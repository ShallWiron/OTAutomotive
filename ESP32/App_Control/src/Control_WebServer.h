#ifndef CONTROL_WEBSERVER_H
#define CONTROL_WEBSERVER_H

#include <Arduino.h>
#include "Car_Controller.h"

/**
 * @brief Khởi tạo WiFi Access Point và Web Server điều khiển xe qua giao diện Web
 * @param car Tham chiếu đối tượng Car_Controller đã khởi tạo
 */
void Control_WebServer_Init(Car_Controller &car);

/**
 * @brief Hàm chạy trong loop() để xử lý client request
 */
void Control_WebServer_Loop();

#endif // CONTROL_WEBSERVER_H

