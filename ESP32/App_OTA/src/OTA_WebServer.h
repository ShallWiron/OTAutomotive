#ifndef OTA_WEBSERVER_H
#define OTA_WEBSERVER_H

#include <Arduino.h>
#include "OTA_Protocol.h"

/**
 * @brief Khởi tạo WiFi (STA + SoftAP) và Web Server quản lý CAN OTA
 * @param otaProtocol Tham chiếu đối tượng OTA_Protocol đã khởi tạo
 */
void OTA_WebServer_Init(OTA_Protocol& otaProtocol);

/**
 * @brief Hàm chạy trong loop() để xử lý các request WebServer
 */
void OTA_WebServer_Loop();

#endif // OTA_WEBSERVER_H

