#ifndef APP_WEBSERVER_H
#define APP_WEBSERVER_H

#include <Arduino.h>
#include "Car_Controller.h"
#include "OTA_Protocol.h"

/**
 * @brief Khởi tạo mạng WiFi (Dual-Mode AP + STA) và Web Server tích hợp 2 trong 1:
 *        - Tab Lái xe & Tay gắp: / (hoặc /control)
 *        - Tab Nạp Firmware OTA: /ota
 * @param car Tham chiếu đối tượng Car_Controller
 * @param ota Tham chiếu đối tượng OTA_Protocol
 */
void App_WebServer_Init(Car_Controller &car, OTA_Protocol &ota);

/**
 * @brief Hàm chạy trong loop() để xử lý client request
 */
void App_WebServer_Loop();

#endif // APP_WEBSERVER_H

