# Danh Sách Kịch Bản Kiểm Thử & Đánh Giá Toàn Vẹn Hệ Thống (Test Cases & Verification)

Tài liệu thiết kế các kịch bản kiểm thử (Black-box & Fault-injection Test Cases) nhằm đánh giá độ ổn định, tính toàn vẹn dữ liệu và khả năng tự phục hồi (Rollback) của hệ thống Automotive Gateway OTA qua CAN Bus.

---

## 1. Ma trận Kịch bản Kiểm thử

| Mã TC | Tên Kịch bản | Mục tiêu Kiểm thử | Phương pháp Thực hiện | Kết quả Kỳ vọng | Đánh giá |
| :---: | :--- | :--- | :--- | :--- | :---: |
| **TC-01** | Truy vấn Active App | Đọc đúng phân vùng đang chạy từ Option Bytes | Gửi frame CAN `0x620 [0x55]` từ ESP32 | STM32 phản hồi `0x7C0 [0x00, App_ID]`. Khớp đúng ID đang chạy | **PASS** |
| **TC-02** | Nạp Firmware Bình thường | Toàn trình cập nhật từ App 1 sang App 2 | Nạp file `app2.bin` hợp lệ qua Web UI | 100% Chunk ACK, khớp Global CRC, Option Bytes đổi sang 2, Jump sang App 2 | **PASS** |
| **TC-03** | An toàn tiền điều kiện (Pre-condition) | Ngăn chặn OTA khi xe đang chuyển động | Kích hoạt motor chạy $\rightarrow$ Gửi lệnh `0x600 (REQ_BOOT)` | STM32 phản hồi `0x7C0 [0x01] (NACK)`. Xe tiếp tục chạy, không dừng đột ngột | **PASS** |
| **TC-04** | Rơi gói tin CAN (Packet Loss) | Cơ chế phát hiện sai STT và Retry Chunk | ESP32 cố tình bỏ qua gói tin STT #1 trong Chunk | STM32 phát hiện sai thứ tự gói $\rightarrow$ gửi `NACK`. ESP32 truyền lại cả Chunk thành công | **PASS** |
| **TC-05** | Sai mã CRC Chunk | Từ chối ghi Flash khi dữ liệu khối bị hỏng | ESP32 cố tình đảo 1 bit CRC16 ở frame `0x740 (END_CHUNK)` | STM32 từ chối ghi Flash, gửi `NACK`. Chunk được truyền lại | **PASS** |
| **TC-06** | Sai Global CRC (Hỏng toàn file) | Kích hoạt cơ chế Rollback về App cũ | Làm sai lệch 1 byte trong firmware $\rightarrow$ gửi `END_FILE` | STM32 phát hiện sai Global CRC $\rightarrow$ Xóa phân vùng lỗi $\rightarrow$ Rollback về App cũ | **PASS** |
| **TC-07** | Mất nguồn khi đang ghi Flash | Bảo vệ phân vùng không bị brick khi sụt áp | Đang nạp đến 50%, ngắt nguồn pin đột ngột | Cấp nguồn lại: Option Bytes vẫn trỏ bản App cũ $\rightarrow$ Khởi động chạy bình thường | **PASS** |
| **TC-08** | Cứu hộ phần cứng (PB2 Jumper) | Cứu hộ khi cả 2 phân vùng App đều hỏng | Xóa trắng cả 2 App $\rightarrow$ Gạt PB2 lên 3.3V $\rightarrow$ Reset chip | STM32 ở lại Bootloader vĩnh viễn, nhận nạp lại firmware mới qua CAN thành công | **PASS** |
| **TC-09** | Nạp Firmware từ MicroSD | Kiểm tra khả năng nạp trực tiếp file từ thẻ SD qua SPI | Chọn file `.bin` trên thẻ MicroSD $\rightarrow$ Bấm "Flash from SD" | Đọc file streaming từ SD xuống CAN, STM32 nhận đủ dữ liệu và Swap thành công | **PASS** |
| **TC-10** | Mở khóa Tính năng App 2 | Kiểm chứng Feature Differentiation giữa App 1 và App 2 | Quan sát xe khi ở App 1 và sau khi nạp OTA lên App 2 | Ở App 1 không có OLED và còi; lên App 2 có Dashboard OLED 32x32 bitmap tiếng Anh và còi Horn ngắt TIM4 2.5kHz | **PASS** |
| **TC-11** | Chuyển đổi Dark/Light Theme | Đổi màu nền giao diện và ghi nhớ cấu hình | Nhấn nút Theme toggle trên Web UI $\rightarrow$ Tải lại trang | Giao diện đổi tức thì nền Đen $\leftrightarrow$ Trắng, giữ nguyên trạng thái sau F5 | **PASS** |
| **TC-12** | Quản lý File thẻ SD qua Web | Tải file `.bin` mới vào thẻ SD không dây | Upload file `.bin` qua REST API `/api/ota/upload_sd` | File lưu chuẩn xác vào thẻ SD, xuất hiện trên bảng danh sách file | **PASS** |

---

## 2. Chi Tiết Thực Thi Từng Kịch Bản

### TC-02: Cập nhật Firmware Chuẩn (Normal Path)
1. **Trạng thái ban đầu:** STM32 đang chạy App 1 (LED PC13 nháy chu kỳ 500ms). Option Bytes `Data0 = 0x01`.
2. **Thực thi:**
   - Trên Web UI chọn file `app2.bin` (kích thước ~18.5 KB). Bấm "Nạp Firmware".
   - ESP32 gửi `0x600 [0xDE, 0xAD, 0xBE, 0xEF]`.
   - STM32 phản hồi `0x7C0 [0x00]`, Soft-Reset vào Bootloader.
   - Bootloader xóa 56KB phân vùng App 2 (`0x08012000`).
   - ESP32 truyền tuần tự ~74 Chunks (mỗi Chunk 256 bytes). STM32 xác nhận `ACK` từng Chunk.
   - Kết thúc file: STM32 verify Global CRC16 và kiểm tra Vector Table hợp lệ.
   - STM32 ghi Option Bytes `Data0 = 0x02`, jump sang App 2.
3. **Kết quả quan sát:** Màn hình OLED trên xe hiển thị Dashboard của App 2. Quá trình nạp mất ~1.8 giây tại baudrate 500kbps.

---

### TC-03: Kiểm tra Điều kiện An toàn khi Đang Lái Xe
1. **Trạng thái ban đầu:** Xe đang nhận lệnh di chuyển từ Web (Chân PB12/PB14 đang ở mức HIGH, motor quay).
2. **Thực thi:** Trên Web cố tình gửi lệnh OTA `0x600`.
3. **Kết quả quan sát:**
   - Hàm `OTA_IsVehicleBusy()` trong ngắt `CAN1_RX1_IRQHandler` phát hiện thanh ghi `GPIOB->ODR` đang tích cực.
   - STM32 lập tức phản hồi `0x7C0 [0x01] (OTA_NACK)`.
   - Motor không bị ngắt, xe không bị khựng hoặc dừng đột ngột. Hệ thống duy trì tính năng lái an toàn.

---

### TC-06: Tiêm Lỗi Toàn Vẹn File & Tự Động Rollback (Fault Injection)
1. **Trạng thái ban đầu:** Xe đang chạy ổn định trên App 1.
2. **Thực thi:**
   - Cố tình sửa đổi 1 byte ngẫu nhiên trong file `app2.bin`.
   - Thực hiện quy trình nạp OTA qua CAN.
   - Các Chunk vẫn vượt qua kiểm tra CRC cục bộ và ghi vào Flash App 2.
   - Tại lệnh `0x780 (ID_END_FILE)`: Bootloader quét toàn bộ vùng Flash `0x08012000`, tính ra `Global CRC16` thực tế khác với mã `expected_file_crc` gửi từ ESP32.
3. **Phản ứng hệ thống:**
   - STM32 xuất log: `[OTA ERR] Sai Global CRC16 -> Xoa phan vung loi & Rollback!`.
   - Tự động xóa sạch phân vùng App 2 vừa nạp để tránh rác bộ nhớ.
   - Gọi hàm `Bootloader_BootActiveOrRollback()`: Nhảy trở lại App 1 cũ.
   - Xe tiếp tục vận hành bình thường trên bản App 1, không bị brick.

---

### TC-07: Đánh giá Bảo vệ Mất Nguồn (Power-Loss Simulation)
1. **Trạng thái ban đầu:** Đang nạp firmware vào phân vùng App 2.
2. **Thực thi:** Rút dây nguồn pin ở tiến độ ~50% (đang ghi giữa chừng).
3. **Phản ứng hệ thống:**
   - Bộ nhớ phân vùng App 2 mới chỉ được ghi một nửa (chưa hoàn thiện).
   - Tuy nhiên, thanh ghi phần cứng Option Bytes (`0x1FFFF804`) vẫn giữ nguyên giá trị `0x01` (trỏ về App 1) do cơ chế **Atomic Active Switching** chỉ cập nhật ở bước cuối cùng.
4. **Cấp nguồn trở lại:**
   - Bootloader khởi động, đọc Option Bytes thấy `Data0 = 0x01`.
   - Kiểm tra Vector Table của App 1 tại `0x08004000` hoàn toàn hợp lệ.
   - Bootloader jump thẳng vào App 1. Hệ thống phục hồi ngay lập tức.

---

### TC-09: Nạp Firmware Trực Tiếp Từ Thẻ Nhớ MicroSD
1. **Trạng thái ban đầu:** ESP32 gắn thẻ nhớ MicroSD qua giao tiếp SPI (CS: GPIO 15). Thẻ nhớ chứa sẵn tệp `app2.bin`.
2. **Thực thi:**
   - Trên Web UI tại tab **Source: MicroSD Card**, bảng file hiển thị tệp `app2.bin` (~18.5 KB).
   - Bấm nút `SELECT` bên cạnh tệp `app2.bin`.
   - Bấm nút **"FLASH FROM SD CARD"**.
3. **Phản ứng hệ thống:**
   - ESP32 kích hoạt Bootloader qua CAN (`0x600`), xóa Bank 2 (`0x640`).
   - Đọc từng khối từ thẻ SD và truyền gói CAN burst `0x700..0x71F`.
   - Tính toán và xác nhận CRC từng Chunk (`0x740`) và Global CRC (`0x780`).
   - STM32 xác nhận `ACK`, hoán đổi Option Bytes sang Bank 2 và khởi động vào App 2.
   - Web UI báo thành công 100%.

---

### TC-10: Kiểm Chứng Feature Differentiation (Mở Khóa Còi Horn TIM4 & Dashboard OLED HUD)
1. **Trạng thái ban đầu:** Xe đang chạy trên App 1 (bản demo tinh gọn).
2. **Quan sát trước OTA:**
   - App 1 chỉ điều khiển động cơ L298 và 2 Servo cần cẩu (PA6, PA7).
   - Màn hình OLED SSD1306 và còi Buzzer không được trang bị trên App 1 để tiết kiệm dung lượng tối đa. Bấm nút Horn trên Web xe không kêu còi.
3. **Thực thi:** Thực hiện OTA nạp bản `app2.bin` qua RAM hoặc MicroSD.
4. **Quan sát sau OTA:**
   - Hệ thống tự động truy vấn lại sau khi nạp: `Active App: App 2` (Option Bytes `Data0 = 0x02`).
   - Màn hình OLED SSD1306 trên xe sáng lên hiển thị Dashboard 3 vùng chuyên nghiệp:
     - Header: Tag `[CAR]` / `[CRANE]`, icon còi loa `HORN`, chấm nhịp tim Heartbeat.
     - Center: Icon đồ họa 32x32 biểu diễn hướng chuyển động xe / cần cẩu + 2 dòng trạng thái tiếng Anh rõ nét.
     - Footer: Telemetry CAN thời gian thực `APP2 | CAN: 0x... | 500K`.
   - Bấm giữ nút `[HORN]` (hoặc phím `H`): ESP32 gửi CAN ID `0x0C1`, còi Buzzer trên xe phát âm thanh 2.5kHz từ chân PA0 qua ngắt phần cứng `TIM4` (Non-blocking, xe vẫn lái bình thường khi bấm còi), icon còi trên OLED chuyển sang hình phát sóng âm. Nhả nút gửi `0x0C0`, còi tắt ngay lập tức.

---

### TC-11: Chuyển Đổi Dark / Light Theme & Ghi Nhớ Cấu Hình
1. **Thực thi:**
   - Mở Web UI, mặc định giao diện chạy theme tối đen công nghiệp (`#0d0d0d`).
   - Bấm nút chuyển theme `THEME: LIGHT` trên thanh tiêu đề.
   - Toàn bộ nền, bảng điều khiển, nút bấm chuyển sang tông màu sáng tối giản (`#f4f4f5`, viền xám tro).
   - Tải lại trình duyệt (F5) hoặc ngắt kết nối rồi mở lại.
2. **Kết quả quan sát:**
   - Trình duyệt giữ nguyên giao diện Light Theme đã chọn nhờ cơ chế lưu `localStorage.getItem('app_theme')`.
   - Bấm lại nút `THEME: DARK` chuyển về nền tối mượt mà.

---

### TC-12: Quản Lý & Tải Thêm File Vào Thẻ MicroSD Không Dây
1. **Thực thi:**
   - Tại mục "Upload firmware to SD card" trên Web UI, chọn tệp `app1_v2.bin` từ máy tính.
   - Bấm nút tải lên thẻ SD.
2. **Kết quả quan sát:**
   - Trình duyệt gửi request `POST /api/ota/upload_sd` dạng `multipart/form-data`.
   - ESP32 nhận luồng dữ liệu và lưu trực tiếp vào tệp `/app1_v2.bin` trên MicroSD qua SPI.
   - Sau khi hoàn thành, danh sách file trên Web UI tự động làm mới, xuất hiện tệp `/app1_v2.bin` kèm dung lượng chính xác, sẵn sàng để bấm `SELECT` và nạp.
