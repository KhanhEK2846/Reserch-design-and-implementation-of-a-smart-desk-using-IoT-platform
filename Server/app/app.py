from flask import Flask,render_template,request, redirect, session
from firebase_admin import initialize_app, credentials, db
import pyrebase


cred = credentials.Certificate("app/key.json")
default_app = initialize_app(credential=cred,options={'databaseURL':'https://smart-desk-using-iot-pla-63534-default-rtdb.firebaseio.com/'})
rtdb = db.reference('/Test')

app = Flask(__name__)

config = {
  'apiKey': "AIzaSyCb4L-TqX8VCtKFN5gNBN77B7pT9Eh55k4",
  'authDomain': "smart-desk-using-iot-pla-63534.firebaseapp.com",
  'databaseURL': "https://smart-desk-using-iot-pla-63534-default-rtdb.firebaseio.com",
  'projectId': "smart-desk-using-iot-pla-63534",
  'storageBucket': "smart-desk-using-iot-pla-63534.appspot.com",
  'messagingSenderId': "1094898119298",
  'appId': "1:1094898119298:web:b9f8b31bdb53a3be6b3a9b"
}

firebase = pyrebase.initialize_app(config)
auth = firebase.auth()

email = 'test@gmail.com'
password = '123456'

app.secret_key = 'secret'

@app.route('/',methods=['GET', 'POST'])
def Index():
    if('user' in session):
        return 'Hi, {}'.format(session ['user'])
    if request.method =='POST':
        email = request.form.get('email')
        password = request.form.get('password')    
        try:
            user = auth.sign_in_with_email_and_password(email, password)
            session['user'] = email
        except:
            return 'Failed to login'
    return render_template('Trangchu.html')

@app.route('/logout')
def logout():
    session.pop('user')     
    return redirect('/')                                 

@app.route('/Test',methods=['GET', 'POST'])
def Test():
    rtdb.set({'hello': 'nothing'})
    return rtdb.get()

@app.route('/',methods=['GET', 'POST'])
def LogIn():
    if request.method == "GET":
        return render_template('TrangChu.html')
    if request.method == "POST":
        username = request.form['Username']
        password = request.form['Password']
        return username + " " + password

@app.route("/DangKy",methods=['GET', 'POST'])
def DangKy():
    if request.method == "GET":
        return render_template('DangKy.html')
    if request.method == "POST":
        return


@app.errorhandler(404)
def page_not_found(error):
    return render_template("404.html"), 404

if __name__ == '__main__':
    app.run(debug=True)