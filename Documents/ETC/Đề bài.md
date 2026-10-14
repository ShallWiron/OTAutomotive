# Tổng quan
Dự án: Thiết kế mô hình Automotive Gateway cho xe điều khiển phân tán: Tích hợp kiến trúc cập nhật phần mềm không dây tự phục hồi (OTA Dual-Bank Rollback) qua mạng CAN Bus 
- Xe điều khiển qua Wifi bằng ESP32 + STM32
- Tích hợp chức năng Update Firmware cho STM32 qua Wifi dùng ESP32
- Sử dụng giao thức CAN để giao tiếp trên Xe
# Đề bài chi tiết
- Hệ thống phần cứng bao gồm: 
	- Khung xe điều khiển
	- Vi điều khiển STM32F103C8T6 (đóng vai trò ECU điều khiển và an toàn)
	- ESP32 (đóng vai trò TCU kết nối mạng Wi-Fi)
	- 02 module thu phát CAN (SN65HVD230 hoặc tương đương)
	- Module điều khiển động cơ L298
	- Hệ thống cấp nguồn
	- Motor và Servor.
- Thiết kế mô hình kết nối phần cứng và sơ đồ phân vùng bộ nhớ Flash cho STM32 theo cơ chế Dual-Bank. Sinh viên phải xác định rõ địa chỉ bắt đầu của vùng Bootloader, vùng Application 1 và 2 (1 App chứa Firmware hiện tại, 1 App làm nơi Update). Trình bày sơ đồ luồng quá trình khởi động và nhảy (jump) từ Bootloader sang phân vùng ứng dụng tương ứng.
- Thiết kế kiến trúc phần mềm cho STM32 theo mô hình phân lớp rõ ràng:
	- Lớp IP (Low-level Driver): thực hiện cấu hình trực tiếp thanh ghi hoặc sử dụng thư viện SPL đối với bộ điều khiển bxCAN, Timer (cho PWM) và Flash. Tuyệt đối không phụ thuộc vào thư viện HAL.
    - Lớp Middleware: cung cấp các dịch vụ xử lý ghép mảnh gói tin CAN, dịch vụ đọc/ghi/xóa Flash theo block, tính toán mã CRC, và điều khiển tốc độ/chiều quay động cơ.
    - Lớp Application: thực thi logic điều khiển xe hoặc logic cập nhật phần mềm tùy thuộc vào trạng thái hệ thống.
- Xây dựng chương trình trên ESP32 đóng vai trò là Web Server và CAN Gateway. ESP32 phát mạng Wi-Fi và cung cấp một giao diện điều khiển trên trình duyệt web gồm 2 chức năng chính: Bảng điều khiển hướng di chuyển của xe (Tiến, Lùi, Trái, Phải) và tính năng chọn file nạp Firmware (.bin) để cập nhật. Khi có lệnh, ESP32 phải băm nhỏ dữ liệu và đóng gói gửi xuống STM32 thông qua giao thức CAN Bus.
- Xây dựng giao thức truyền nhận trên mạng CAN Bus giữa ESP32 và STM32. Nhóm phải định nghĩa các CAN ID riêng biệt để phân loại luồng dữ liệu (Ví dụ: ID dành riêng cho lệnh lái xe và ID dành riêng cho dữ liệu OTA). Trên STM32, tiến hành cấu hình bộ lọc (CAN Filter) để vi điều khiển chỉ nhận đúng các ID đã quy định, loại bỏ hoàn toàn các gói tin rác trên mạng.
- Xây dựng chương trình Bootloader trên STM32 có khả năng tự phục hồi (Fail-safe/Rollback). Khi nhận được luồng dữ liệu OTA từ CAN, Bootloader tiến hành ghi dữ liệu vào phân vùng OTA dự phòng. Sau khi nhận đủ, hệ thống phải thực hiện kiểm tra tính toàn vẹn (CRC) của firmware mới trước khi Swap (đổi) phân vùng để chạy. Thiết kế cơ chế bảo vệ: nếu quá trình cập nhật bị lỗi (mất kết nối, mất nguồn giữa chừng) hoặc mã CRC không khớp, hệ thống phải tự động Rollback về bản firmware cũ đang hoạt động ổn định để xe không bị tê liệt.
- Xây dựng môi trường kiểm thử phần mềm một cách bài bản. Thiết kế Makefile để biên dịch toàn bộ chương trình của STM32 bằng GCC toolchain (arm-none-eabi-gcc), sinh file thực thi dạng BIN, ELF và HEX độc lập hoàn toàn với IDE KeilC. 
- Thiết kế cơ chế bảo vệ hệ thống trong quá trình cập nhật, bao gồm: xử lý mất nguồn khi đang ghi Flash, phát hiện firmware lỗi và không cho chạy chương trình sai.
- Xây dựng báo cáo kết quả, trong đó trình bày kiến trúc phần mềm, sơ đồ luồng dữ liệu CAN, giao thức phân mảnh OTA, danh sách test case và kết quả kiểm thử. Báo cáo phải kèm theo hướng dẫn sử dụng Makefile và tài liệu mô tả quá trình vận hành của xe.