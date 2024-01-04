var btnInfoStudent = document.getElementById("btnInfoLecturer");
var btnSubject = document.getElementById("btnClasses");
var btnSubject = document.getElementById("btnManage");
var btnSearch = document.getElementById("btnSearch");

// Thêm sự kiện click vào các nút
btnInfoStudent.addEventListener("click", function() {
  // Hiển thị trang thông tin sinh viên
  location.href = "ThongTinGV.html";
});

btnSubject.addEventListener("click", function() {
  // Hiển thị trang lớp dạy
  location.href = "LopDay.html";
});

btnSubject.addEventListener("click", function() {
    // Hiển thị trang quản lý 
    location.href = "QuanLy.html";
  });

btnSearch.addEventListener("click", function() {
  // Hiển thị trang tra cứu
  location.href = "QuanLySV.html";
});