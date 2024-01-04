var btnInfoStudent = document.getElementById("btnInfoStudent");
var btnSubject = document.getElementById("btnSubject");


// Thêm sự kiện click vào các nút
btnInfoStudent.addEventListener("click", function() {
  // Hiển thị trang thông tin sinh viên
  location.href = "ThongTinSV.html";
});

btnSubject.addEventListener("click", function() {
  // Hiển thị trang thông tin môn học
  location.href = "MonHoc.html";
});

