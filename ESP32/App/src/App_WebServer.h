#ifndef APP_WEBSERVER_H
#define APP_WEBSERVER_H

#include <Arduino.h>
#include "Car_Controller.h"
#include "OTA_Protocol.h"
#include "SD_Manager.h"

void App_WebServer_Init(Car_Controller &car, OTA_Protocol &ota, SD_Manager &sd);
void App_WebServer_Loop();

#endif // APP_WEBSERVER_H
