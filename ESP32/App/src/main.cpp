#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "CAN_Driver.h"
#include "SD_Manager.h"
#include "Car_Controller.h"
#include "OTA_Protocol.h"
#include "App_WebServer.h"

CAN_Driver can(CAN_TX_PIN, CAN_RX_PIN);
SD_Manager sd(SD_CS_PIN);
Car_Controller car(can);
OTA_Protocol ota(can);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(200);

    Serial.println("\n[INIT] ESP32 Automotive Gateway starting...");

    if (can.begin()) {
        Serial.println("[CAN OK] TWAI 500kbps initialized.");
    } else {
        Serial.println("[CAN ERR] TWAI initialization failed!");
    }

    car.begin();

    if (sd.begin()) {
        Serial.printf("[SD OK] MicroSD card initialized (%s).\n", sd.getCardType().c_str());
    } else {
        Serial.println("[SD INFO] No MicroSD card detected. Operating in direct RAM buffer mode.");
    }

    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    Serial.printf("[WIFI AP] %s | IP: %s\n", WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());

    if (String(WIFI_SSID).length() > 0) {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    App_WebServer_Init(car, ota, sd);
    Serial.println("[SYS OK] Initialization complete.\n");
}

void loop() {
    App_WebServer_Loop();

    static uint32_t lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 5000) {
        lastWifiCheck = millis();
        static bool wasConnected = false;
        bool isConnected = (WiFi.status() == WL_CONNECTED);
        if (isConnected && !wasConnected) {
            Serial.printf("[WIFI STA OK] IP: %s\n", WiFi.localIP().toString().c_str());
            can.sendFrame(CMD_WIFI_CONNECTED, nullptr, 0);
        }
        wasConnected = isConnected;
    }

    if (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '?') {
            Serial.printf("[STATUS] %s\n", car.getStatusJson().c_str());
        } else if (c == '!') {
            car.emergencyStop();
        } else if (c == 'q' || c == 'Q') {
            uint8_t app = ota.queryCurrentApp(500);
            Serial.printf("[OTA] Active App: %d\n", app);
        } else if (c == 'e' || c == 'E') {
            ota.setInjectCrcError(!ota.isInjectCrcError());
            Serial.printf("[TEST] CRC Error Injection: %s\n", ota.isInjectCrcError() ? "ENABLED" : "DISABLED");
        }
    }

    delay(2);
}
