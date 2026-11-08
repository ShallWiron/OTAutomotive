#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "driver/twai.h"

// ==================== Cấu hình chân phần cứng CAN ====================
// SN65HVD230 nối với ESP32: TX -> GPIO 5, RX -> GPIO 4
#define CAN_TX_PIN              GPIO_NUM_5
#define CAN_RX_PIN              GPIO_NUM_4

// ==================== Cấu hình WiFi (Station - Bắt WiFi ngoài) ====================
#define WIFI_SSID               "Shall"                    // Thay tên WiFi / Hotspot điện thoại tại đây
#define WIFI_PASSWORD           "0123456789"               // Thay mật khẩu WiFi tại đây
#define WEB_SERVER_PORT         80

// ==================== Cấu hình Serial Debug ====================
#define SERIAL_BAUD_RATE        115200

#endif // CONFIG_H

