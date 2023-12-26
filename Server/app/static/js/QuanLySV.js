var btnInfoStudent = document.querySelector("#btnInfoLecturer");
var btnSubject = document.querySelector("#btnClasses");
var btnSubject = document.querySelector("#btnManage");
var btnSearch = document.querySelector("#btnSearch");

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