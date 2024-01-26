/*********
  Rui Santos
  Complete instructions at: https://RandomNerdTutorials.com/esp32-cam-save-picture-firebase-storage/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Based on the example provided by the ESP Firebase Client Library
*********/

#include "Arduino.h"
#include "WiFi.h"
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <LittleFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>

#include <ESPAsyncWebSrv.h>

//Replace with your network credentials
String sta_ssid = ""; 
String sta_password = "" ;
const String ap_ssid = "ESP_CAM";
const String ap_password = "123456789";
boolean sta_flag = false;
int Person = 0; 
const unsigned long Network_TimeOut = 5000;// Wait 5 seconds to Connect Wifi

const char* ssid = "Tran Hong 1";
const char* password = "88888888";

//Local Server Variable
AsyncWebServer server(80); //Create a web server listening on port 80
AsyncWebSocket ws("/ws");//Create a path for web socket server

// Insert Firebase project API Key
#define API_KEY "AIzaSyCb4L-TqX8VCtKFN5gNBN77B7pT9Eh55k4"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "20521269@gm.uit.edu.vn"
#define USER_PASSWORD "010102ha@"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "smart-desk-using-iot-pla-63534.appspot.com"
// For example:
//#define STORAGE_BUCKET_ID "esp-iot-app.appspot.com"

// Photo File Name to save in LittleFS
#define FILE_PHOTO_PATH "/photo.jpg"
#define BUCKET_PHOTO "/data/photo.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

boolean takeNewPhoto = true;

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

void fcsUploadCallback(FCS_UploadStatusInfo info);

bool taskCompleted = false;

/*------------------------Server---------------------------*/
void notifyClients(const String data) {
  String predata = "{";
  predata += data;
  predata += "}"; 
  ws.textAll(predata);
}

void notifyClient(AsyncWebSocketClient *client)
{
  String data ="{";
  if(WiFi.status() != WL_CONNECTED)   
    data += "Wifi OFF";
  else   
    data += "Wifi ON";
  data += "}";
  ws.text(client->id(),data);
  data.clear();

}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if(String((char*)data).indexOf("Username:") >= 0){
      sta_ssid = String((char*)data).substring(String((char*)data).indexOf(' ')+1,String((char*)data).length());
    }
    if(String((char*)data).indexOf("Password:") >= 0){
      sta_password = String((char*)data).substring(String((char*)data).indexOf(' ')+1,String((char*)data).length());
      sta_flag = true;
    }
    if(String((char*)data).indexOf("Disconnect") >= 0){
      WiFi.disconnect(true,true);
      WiFi.mode(WIFI_AP);
      notifyClients("Wifi OFF");
    }
  }

}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      Person += 1;
      notifyClient(client);
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      Person = (Person > 0)? Person-1 : 0;
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html> 
<head>
  <title>ESP Web Server</title>
  <style>
    *{
      padding: 0;
      margin: 0;
      box-sizing: border-box;
    }
    body{
      overflow: hidden;
      text-align: center;
      height: 100vh;
      width: 100vw;
      background: linear-gradient(to right, #12c2e9, #c471ed, #f64f59);
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-direction: column;
      align-content: center;
      flex-wrap: wrap;
    }
    button{
      background-color: azure;
      border: none;
      outline: none;
      border-radius: 100px;
      font-weight: bolder;
      transition-duration: 0.3s;
      transition-property: color,background-color ;
      -webkit-box-shadow: 0px 11px 22px -9px #000000; 
      box-shadow: 0px 11px 22px -9px #000000;
      cursor: pointer;
      margin: 5px 25px;
    }
    button:hover{
      color: whitesmoke;
      background-color: black;
    }
    #overlay{
      position: fixed;
      z-index: 1;
      width: 100vw;
      height: 100vh;
      top: 0;
      left: 0;
      background: transparent;
      backdrop-filter: blur(5px);
    }
    #Wifi{
      background-color: transparent;
      border: 2px solid rgba(255, 255, 255, 0.5);
      backdrop-filter: blur(15px);
      width: 900px;
      border-radius: 25px;
      display: flex;
      justify-content: center;
      align-items: center;
      flex-direction: column;
      padding: 20px;
      position: fixed;
      transform: scale(0.8);
      transition: 0.5s;
      font-size: xx-large;
      z-index: 3;
      top: 25%;
    }
    #Wifi button{
      width: 440px;
      padding: 12px 0 12px 0;
      font-size: x-large;
    }
    #menu{
      background-color: transparent;
      border: 2px solid rgba(255, 255, 255, 0.5);
      display: flex;
      justify-content:space-around;
      width: 100vw;
      border-radius: 50px;
      z-index: 2;
    }
    #menu button{
      width: 200px;
      padding: 7px 0 7px 0;
    }
    #Router{
      display: flex;
      align-items: center;
      justify-content: center;
    }
    #Action button{
      margin-bottom: 20px;
      border: 0;
      display: block;
      width: 300px;
      padding: 15px 0 15px 0;
    }
    #status{
      width: 10px;
      height: 10px;
      background-color: gray;
      border-radius: 50%;
      margin-left: 5px;
    }
    .hide{
        display: none;
    }
    .outer{
      width: 160px;
      height: 160px;
      border-radius: 50%;
      box-shadow: rgba(0, 0, 0, 0.16) 0px 3px 6px, rgba(0, 0, 0, 0.23) 0px 3px 6px;
      padding: 20px;
    }
    .inner{
      height: 120px;
      width: 120px;
      display: flex;
      align-items: center;
      justify-content: center;
      border-radius: 50%;
      box-shadow: rgba(50, 50, 93, 0.25) 0px 30px 60px -12px inset, rgba(0, 0, 0, 0.3) 0px 18px 36px -18px inset;
    }
    .number{
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: bolder;
      color: #555;
    }
    circle{
      fill: none;
      stroke: url(#GradientColor);
      stroke-width: 20px;
      stroke-dasharray: 472; /*Exact circle*/
      stroke-dashoffset: 200;
      transition: 0.5s;
    }
    svg{
      margin-top: 22px;
      position: absolute;
      top: 0;
      left: 0;
    }
    .container_gauge{
      display: flex;
      justify-content: space-around;
      width: 50%;
    }
    .Sensor_Value{
      display: flex;
      height: 200px;
      width: 100vw;
      justify-content: space-around;
      flex-wrap: wrap;
    }
    .gauge{
      width: 160px;
      height: 160px;
      position: relative;
      margin: 0 10px 100px 10px;
    }
    .gauge h3{
      text-align: center;
    }
    #Bool{
      margin: 200px 0 0 25px;
      display: flex;
      justify-content: space-between;
      width: 100vw;
      height: 300px;
      align-items: center;
      flex-wrap: wrap;
    }
    #NameTree{
      font-size: xx-large;
      font-weight: bolder;
      text-align: center; 
    }
    .Error_Sensor{
      display: flex;
    }
    input, .input_box input{
      border: none;
      outline: none;
      background: transparent;
    }
    .input_box{
      position: relative;
      width: 800px;
      margin: 40px 0;
      border-bottom: 2px solid #fff;
    }
    .input_box label{
      position: absolute;
      top: 50%;
      left: 5px;
      margin-left: 5px;
      transform: translateY(-50%);
      transition: 0.5s;
      font-weight: bolder;
      color: azure;
      pointer-events: none;
    }
    .input_box input:focus~label, .input_box input:valid~label{
      top: -15px; 
    }
    .input_box input{
      font-size: 1em;
      width: 100%;
      height: 50px;
      color: aliceblue;
      padding: 0 35px 0 5px;
    }
    .loading{
      width: 80px;
      height: 80px;
      position: relative;
    }
    .first_circle,.last_circle{
      border-radius: 50%;
    }
    .first_circle{
      width: 100%;
      height: 100%;
      border: 10px solid rgba(255, 255, 255, 0.5);
      border-left: 5px solid transparent;
      border-right: 5px solid transparent;
      animation: spinner 1.5s infinite linear;
    }
    .last_circle{
      width: 75%;
      height: 75%;
      position: absolute;
      top: 12.5%;
      left: 12.5%;
      border: 7px solid rgba(255, 255, 255, 0.25);
      border-top: 2.5px solid transparent;
      border-bottom: 2.5px solid transparent;
      animation: spinner 1.5s infinite linear reverse;
    }
    @keyframes spinner{
      from{
        transform: rotate(-360deg);
      }
      to{
        transform: rotate(360deg);
      }
    }
    @media(max-width: 62em)
    {
      body{
        height: 100vw;
      }
      .container_gauge{
        flex-direction: column;
        align-items: center;
      }
    }
  </style>
</head>
<body>
<div id="Wifi" class="show"> <!--Login Wifi--> 
  <h1>GATEWAY SERVER</h1>
  <h3 id = "Info">%INFO%</h3>
  <div id="wait" class="loading hide">
    <div class="first_circle"></div>
    <div class="last_circle"></div>
  </div>
  <form  id = "LogIn" autocomplete="off">
    <div class="input_box">
      <input type="text" id="ssid" name="ssid" required/><br>
      <label for="ssid">Name Wifi: </label>
    </div>
    <div class="input_box">
      <input type="password" id="password" name="password" required/><br>
      <label for="password">Password: </label>
    </div>
  </form>    
  <button id="Submit">Connect</button>
  <button id = "Disconnect" class="hide">Disconnect</button>
</div>  
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;

    window.addEventListener('load', onLoad);
    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      websocket = new WebSocket(gateway);
      websocket.onopen    = onOpen;
      websocket.onclose   = onClose;
      websocket.onmessage = onMessage; // <-- add this line
    }
    function onOpen(event) {
      console.log('Connection opened');
    }
    function onClose(event) {
      console.log('Connection closed');
      TheEnd()
    }
    function onMessage(event){
        var pre_data = String(event.data).substring(String(event.data).indexOf("{")+1,String(event.data).indexOf("}"))
        if(String(pre_data).indexOf("Wifi") >= 0)
        {
            if(String(pre_data).indexOf("OFF") >= 0)
            {
            document.getElementById("Submit").classList.remove("hide");
            document.getElementById('LogIn').classList.remove("hide");
            document.getElementById("wait").classList.add("hide");
            document.getElementById('Disconnect').classList.add("hide");
            document.getElementById('Info').innerHTML = "Status: Not connected";
            }
            if(String(pre_data).indexOf("ON") >= 0)
            {
            document.getElementById("Submit").classList.add("hide");
            document.getElementById('LogIn').classList.add("hide");
            document.getElementById('Info').innerHTML = "Status: Connected";
            document.getElementById('Disconnect').classList.remove("hide");
            document.getElementById("wait").classList.add("hide");
            }
        }
    }
    function onLoad(event) {
      initWebSocket();
      initButton();
    }
    function initButton() {
      document.getElementById("Submit").addEventListener("click",Log);
      document.getElementById("Disconnect").addEventListener("click",Dis);
    }
    function Dis(){
        websocket.send("Disconnect");
    }
    function Log(){
      websocket.send("Username: " + document.getElementById('ssid').value);
      websocket.send("Password: " + document.getElementById('password').value);
      document.getElementById('ssid').value = "";
      document.getElementById('password').value = "";
      document.getElementById("wait").classList.remove("hide");
      document.getElementById("Submit").classList.add("hide");
      document.getElementById('LogIn').classList.add("hide");
      document.getElementById('Info').innerHTML = "Waiting....";
    }

  </script>
</body>
</html>
)rawliteral";

/*------------------------------End-----------------------------*/



// Capture Photo and Save it to LittleFS
void capturePhotoSaveLittleFS( void ) {
  // Dispose first pictures because of bad quality
  camera_fb_t* fb = NULL;
  // Skip first 3 frames (increase/decrease number as needed).
  for (int i = 0; i < 4; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
    
  // Take a new photo
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }  

  // Photo file name
  Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_WRITE);

  // Insert the data in the photo file
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.print("The picture has been saved in ");
    Serial.print(FILE_PHOTO_PATH);
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  // Close the file
  file.close();
  esp_camera_fb_return(fb);
}

void initWiFi(){

  if(sta_ssid == "")
    return;
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  long current = millis();
  while (WiFi.status() != WL_CONNECTED && (unsigned long) (millis()- current) < Network_TimeOut) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  if(WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_AP);
    if(Person > 0)
      notifyClients("Wifi OFF");
  }
  else
  {
    if(Person > 0)
      notifyClients("Wifi ON");
    
    //Firebase
    // Assign the api key
    configF.api_key = API_KEY;
    //Assign the user sign in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    //Assign the callback function for the long running token generation task
    configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&configF, &auth);
    Firebase.reconnectWiFi(true);
  }

}

void initLittleFS(){
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("LittleFS mounted successfully");
  }
}

void initCamera(){
 // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

void setup() {
  
  // Serial port for debugging purposes
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  initWebSocket();
  

    // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Start server
  server.begin();
  initLittleFS();
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
  initWiFi();
}

void loop() {
  if (takeNewPhoto) {
    capturePhotoSaveLittleFS();
    takeNewPhoto = false;
  }
  delay(1);
  if (Firebase.ready() && !taskCompleted){
    taskCompleted = true;
    Serial.print("Uploading picture... ");

    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO_PATH /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, BUCKET_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */,fcsUploadCallback)){
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
    }
    else{
      Serial.println(fbdo.errorReason());
    }
  }


  if(sta_flag) // Try to connect wifi after receive from form
  {
    WiFi.mode(WIFI_AP_STA);
    initWiFi();
    sta_flag = false;
  } 
  ws.cleanupClients();

}

// The Firebase Storage upload callback function
void fcsUploadCallback(FCS_UploadStatusInfo info){
    if (info.status == firebase_fcs_upload_status_init){
        Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.fileSize, info.remoteFileName.c_str());
    }
    else if (info.status == firebase_fcs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    }
    else if (info.status == firebase_fcs_upload_status_complete)
    {
        Serial.println("Upload completed\n");
        FileMetaInfo meta = fbdo.metaData();
        Serial.printf("Name: %s\n", meta.name.c_str());
        Serial.printf("Bucket: %s\n", meta.bucket.c_str());
        Serial.printf("contentType: %s\n", meta.contentType.c_str());
        Serial.printf("Size: %d\n", meta.size);
        Serial.printf("Generation: %lu\n", meta.generation);
        Serial.printf("Metageneration: %lu\n", meta.metageneration);
        Serial.printf("ETag: %s\n", meta.etag.c_str());
        Serial.printf("CRC32: %s\n", meta.crc32.c_str());
        Serial.printf("Tokens: %s\n", meta.downloadTokens.c_str());
        Serial.printf("Download URL: %s\n\n", fbdo.downloadURL().c_str());
    }
    else if (info.status == firebase_fcs_upload_status_error){
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}