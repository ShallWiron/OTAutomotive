#ifndef OTA_TRIGGER_H
#define OTA_TRIGGER_H

#include <stdint.h>
#include "stm32f10x.h"

/* CAN IDs dùng cho lệnh kích hoạt Bootloader */
#define ID_REQ_BOOT         0b11000000000U  /* 0x600: ESP32 yêu cầu vào Bootloader */
#define ID_QUERY_APP        0b11000100000U  /* 0x620: ESP32 truy vấn App hiện tại */
#define ID_RESPONSE         0b11111000000U  /* 0x7C0: Phản hồi từ STM32 -> ESP32 */
#define OTA_ACK             0x00U

#define CURRENT_APP_ID      1               /* App 1 */

/* Cờ Magic Key đồng bộ với Bootloader */
#define BKP_OTA_FLAG_MAGIC  0x5A5AU

/**
 * @brief  Khởi tạo ngoại vi CAN cho Application (PA11 RX, PA12 TX, 500kbps)
 *         và cấu hình bộ lọc nhận frame OTA (ID 0x600).
 */
void OTA_Trigger_Init(void);

/**
 * @brief  Kiểm tra bản tin CAN đến (Polling không chặn).
 *         Nếu nhận được frame 0x600 với đúng 4 byte Security Key (0xDEADBEEF):
 *         1. Phản hồi ACK (0x7C0, Data=0x00) cho ESP32.
 *         2. Ghi cờ Magic Key 0x5A5A vào BKP_DR1.
 *         3. Kích hoạt NVIC_SystemReset() để nhảy vào Bootloader.
 */
void OTA_Trigger_Poll(void);

#endif /* OTA_TRIGGER_H */

