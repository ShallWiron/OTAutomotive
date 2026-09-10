# Tổng quan
Dự án: Thiết kế mô hình Automotive Gateway cho xe điều khiển phân tán: Tích hợp kiến trúc cập nhật phần mềm không dây tự phục hồi (OTA Dual-Bank Rollback) qua mạng CAN Bus 
- Xe điều khiển qua Wifi bằng ESP32 + STM32
- Tích hợp chức năng Update Firmware cho STM32 qua Wifi dùng ESP32
- Sử dụng giao thức CAN để giao tiếp trên Xe
# Các đầu việc
- Dự án bao gồm các Module theo Phần cứng:
	- STM32
		- Bootloader
		- App - Điều khiển xe
	- ESP32
		- OTA Driver
		- Web điều khiển

| Phần cứng | Phần mềm          | Nguời phụ trách |
| --------- | ----------------- | --------------- |
| STM32     | Bootloader        | Trung           |
|           | App điều khiển xe | Long            |
| ESP32     | OTA Driver        | Trung           |
|           | Web điều khiển    | Long            |
- Báo cáo:
	- A viết: Kiến trúc Bootloader, Flash layout, giao thức OTA, Makefile
	- B viết: Kiến trúc App, motor/servo driver, sơ đồ phần cứng
	- Cả 2: Giao thức CAN, test cases, kết quả
### Trung
- Làm việc trên Linux + Makefile trên tổng dự án (OTA + Ghép nối) 
- [ ] Setup GIT STM(Bootloader, App1, App2, AppKeil), ESP(App_OTA, App_Control), Thêm 1file quản lý dự án
- [ ] Dạy Long dùng Git
- [ ] Học viết Makefile riêng

- [ ] Học CAN, dạy lại Long
- [ ] Code lại OTA theo CAN, cấu hình Frame, lệnh,...
- [ ] Phân tầng code theo các lớp

- [ ] Làm cái Web điều phối OTA
- [ ] Ghép code
### Long
- Làm việc trên Keilc luôn vì chỉ code AppKeil.
- [ ] Học dùng Git
- [ ] Học Timer, PWM, UART để điều khiển động cơ bằng STM
- [ ] Thiết kế sơ đồ nối dây

- [ ] Làm xe chạy được theo từng lệnh giữa STM và ESP
- [ ] Đổi qua dùng CAN để giao tiếp 

- [ ] Làm cái Web điều khiển bằng ESP
# Notes
- Cần thêm:
	- Module cấp nguồn cho STM, ESP : 1 LM2596 = 30.000VND
	- Cần 2 Module SN65HVD230 cho CAN = 2 x 30.000VND = 60.000VND
	- 1 STM32F103C8T6 - Bluepill, 1 mạch nạp ST-Link = 80.000VND
	- Total = 170.000VND
- Phần cứng đã có:
	- Moto Shield L298 cho Arduino
- Cần học dùng PWM
- Dùng Git share code
# Có thể làm thêm
- Áp dụng phương pháp Blackbox test để kiểm chứng luồng cập nhật OTA: tạo các test case cố tình gửi sai mã CRC, ngắt kết nối CAN giữa chừng hoặc ngắt nguồn để đánh giá độ tin cậy của cơ chế Rollback.
- MQTT
- 4G
# Các tiến trình
- Xe: Thay thế Arduino bằng STM32 + ESP32 và có thể điều khiển qua wifi
	- Thiết kế sơ đồ nối giữa STM32 và Hệ thống xe
	- Sử dụng STM32 để điều khiển xe
		- Học và dùng PWM
	- Tích hợp thêm ESP để điều khiển qua Wifi
- Hệ thống OTA: Giao tiếp STM32 và ESP32 bằng CAN, thực hiện OTA. 
	- Giao tiếp CAN STM32 và ESP32
	- Code lại OTA theo CAN và quy ước
- Tổng thể:
	- Làm Makefile cho STM32
- Ghép lại