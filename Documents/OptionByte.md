# Hướng Dẫn Sử Dụng Option Bytes Trên STM32 Để Quản Lý Phân Vùng OTA

Tài liệu này tóm tắt cách thức hoạt động và lập trình thanh ghi Option Bytes của STM32 để chuyển đổi phân vùng khởi động (`App 1` vs `App 2`) khi nạp OTA, đảm bảo không mất dữ liệu khi mất nguồn điện và bảo vệ an toàn hệ thống.

---

## 1. Tổng Quan Về Option Bytes
* **Bản chất:** Là vùng nhớ Flash phi bốc hơi (Non-volatile) đặc biệt trên STM32, độc lập vật lý với 128KB bộ nhớ chính (Main Flash) và **không bị mất dữ liệu khi rút nguồn điện**.
* **Ưu điểm cho OTA:**
  * Không tốn dung lượng bộ nhớ chính của ứng dụng.
  * Không cần kết nối pin Backup (VBAT).
  * Chống ghi đè nhầm từ ứng dụng chính nhờ cơ chế khóa lớp kép.
  * Tự động khởi động lại (Reset) hệ thống sau khi lưu cấu hình thành công.
* **Địa chỉ lưu cấu hình:** Sử dụng thanh ghi dữ liệu người dùng **`User Data 0` (`Data0`)** tại địa chỉ **`0x1FFF F804`**.

---

## 2. Cách Đọc Trạng Thái Phân Vùng (Khi Boot)

Trong hàm `main()` của **Bootloader**, đọc trực tiếp giá trị `Data0` từ địa chỉ Option Bytes để quyết định nhảy vào phân vùng mong muốn.

```c
#define OB_DATA0_ADDR   0x1FFFF804 // Địa chỉ User Data 0
#define APP1_ADDRESS    0x08004000 // Địa chỉ bắt đầu App 1
#define APP2_ADDRESS    0x08012000 // Địa chỉ bắt đầu App 2

int main(void) {
    // 1. Khởi tạo ngoại vi mặc định...
    
    // 2. Đọc giá trị active_app trực tiếp bằng con trỏ C
    uint8_t active_app = *(volatile uint8_t*)OB_DATA0_ADDR;

    // 3. Nếu là lần đầu chạy hoặc dữ liệu trống (0xFF), mặc định chọn App 1
    if (active_app != 1 && active_app != 2) {
        active_app = 1;
    }

    // 4. Nhảy vào App tương ứng (khi không kích hoạt chế độ nạp thủ công qua chân PB2)
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2) != 0) {
        if (active_app == 2) {
            Jump_To_App(APP2_ADDRESS);
        } else {
            Jump_To_App(APP1_ADDRESS);
        }
    }

    // 5. Nếu PB2 = 0 -> Đi vào vòng lặp chờ nhận lệnh OTA từ ESP32
    while(1) {
        // Luồng nhận dữ liệu UART...
    }
}
```

---

## 3. Cách Ghi/Chuyển Đổi Phân Vùng (Khi Nạp Thành Công)

Khi nhận được gói tin kết thúc OTA thành công (`CMD_END`), thực hiện quy trình mở khóa, xóa, ghi giá trị mới và nạp lại cấu hình (Launch) để tự động reset STM32.

```c
#include "stm32f10x_flash.h"

void Set_Active_Partition(uint8_t app_id) {
    // 1. Mở khóa bộ điều khiển Flash chính
    FLASH_Unlock();
    
    // 2. Mở khóa vùng cấu hình Option Bytes
    FLASH_OB_Unlock();
    
    // 3. Xóa sạch vùng Option Bytes cũ (Bắt buộc phải xóa trước khi ghi)
    FLASH_OB_Erase();
    
    // 4. Ghi giá trị phân vùng mới (1 hoặc 2) vào User Data 0
    // Thư viện tự động tính toán byte đảo (complement) ghi vào 0x1FFFF805 để check lỗi
    FLASH_ProgramOptionByteData(0x1FFFF804, app_id);
    
    // 5. Khóa lại, nạp cấu hình mới và tự động kích hoạt SYSTEM RESET ngay lập tức
    FLASH_OB_Launch(); 
    
    // 6. Chốt khóa bảo vệ Flash (thực tế chip đã reset tại bước 5)
    FLASH_Lock();
}
```

---

## 4. Cơ Chế Chống Brick (Atomic Active Switching)
Quy trình nạp chỉ đổi biến chỉ định khởi động khi toàn bộ file đã được kiểm tra khớp hoàn toàn:

```mermaid
flowchart TD
    Start([ESP32 truyền dữ liệu OTA]) --> WriteFlash[Ghi vào Phân vùng phụ\n(ví dụ: App 2)]
    WriteFlash --> CheckError{Có lỗi xảy ra?\n(Mất điện, rớt mạng, lỗi CRC,...)}
    CheckError -- "Có" --> KeepConfig[Giữ nguyên cấu hình cũ\n(Option Bytes = 1)]
    KeepConfig --> ResetOld[STM32 Reset:\nTiếp tục chạy App 1 (cũ)]
    CheckError -- "Không" --> WriteOB[Ghi đè Option Bytes = 2]
    WriteOB --> ResetNew[STM32 Reset:\nTự động chạy App 2 (mới)]
```
