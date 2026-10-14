#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "driver/twai.h"

// ==================== Cấu hình phần cứng ====================
#define SD_CS_PIN           15
#define CAN_TX_PIN          GPIO_NUM_5
#define CAN_RX_PIN          GPIO_NUM_4

// ==================== Thông tin mạng WiFi ====================
#define WIFI_SSID           "Shall"
#define WIFI_PASSWORD       "0123456789"
#define WIFI_AP_SSID        "ESP32_OTA_Gateway"
#define WIFI_AP_PASSWORD    "12345678"

// ==================== Định nghĩa CAN ID (11-bit) ====================
#define CAN_ID_REQ_BOOT     0x600U  // Pha 0: Yêu cầu Soft-Reset sang Bootloader
#define CAN_ID_QUERY_APP    0x620U  // Truy vấn phân vùng App hiện tại của STM32
#define CAN_ID_START_OTA    0x640U  // Pha 1: Bắt đầu OTA & Xóa Flash
#define CAN_ID_START_FILE   0x680U  // Pha 1: Khai báo kích thước File & Global CRC16
#define CAN_ID_START_CHUNK  0x6C0U  // Pha 2: Khai báo độ dài Chunk
#define CAN_ID_DATA_CHUNK   0x700U  // Pha 2: Dữ liệu Chunk (nhúng STT vào 6 bit thấp)
#define CAN_ID_END_CHUNK    0x740U  // Pha 2: Kết thúc Chunk & Chunk CRC16
#define CAN_ID_END_FILE     0x780U  // Pha 4: Kết thúc File & Verify Flash
#define CAN_ID_RESPONSE     0x7C0U  // Phản hồi từ STM32 -> ESP32

// ==================== Hằng số giao thức OTA ====================
#define OTA_ACK             0x00U
#define OTA_NACK            0x01U

#define CHUNK_SIZE          256U    // Kích thước 1 Chunk (256 bytes = 32 gói CAN)
#define MAX_CHUNK_RETRIES   3U      // Số lần retry tối đa khi Chunk lỗi

#endif // CONFIG_H
