// Tìm các nút cần thêm sự kiện click
var btnInfoStudent = document.querySelector("#btnInfoStudent");
var btnSubject = document.querySelector("#btnSubject");


// Thêm sự kiện click vào các nút
btnInfoStudent.addEventListener("click", function() {
  // Hiển thị trang thông tin sinh viên
  location.href = "ThongTinSV.html";
});

btnSubject.addEventListener("click", function() {
  // Hiển thị trang thông tin môn học
  location.href = "MonHoc.html";
});


