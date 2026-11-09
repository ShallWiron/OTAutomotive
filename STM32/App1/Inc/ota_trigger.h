#ifndef OTA_TRIGGER_H
#define OTA_TRIGGER_H

#include <stdint.h>
#include "stm32f10x.h"

/* Các CAN ID dùng cho giao thức CAN OTA và điều khiển */
#define ID_REQ_BOOT         0x600U  /* ESP32 yêu cầu vào Bootloader */
#define ID_QUERY_APP        0x620U  /* ESP32 truy vấn App hiện tại */
#define ID_RESPONSE         0x7C0U  /* Phản hồi từ STM32 -> ESP32 */
#define OTA_ACK             0x00U

#define CURRENT_APP_ID      1       /* App 1 */

/* Cờ Magic Key nhảy vào Bootloader */
#define BKP_OTA_FLAG_MAGIC  0x5A5AU

void OTA_Trigger_Init(void);
void OTA_Trigger_Check(CanRxMsg *rx);

#endif /* OTA_TRIGGER_H */
