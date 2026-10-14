#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include "config.h"
#include "CAN_Driver.h"
#include "OTA_Protocol.h"
#include "OTA_WebServer.h"

// Khởi tạo các module
CAN_Driver can(CAN_TX_PIN, CAN_RX_PIN);
OTA_Protocol ota(can);

void runOtaFromSD(const char* filepath, uint8_t appId) {
    File file = SD.open(filepath, FILE_READ);
    if (!file) {
        Serial.printf("[SD ERR] Khong the mo file: %s!\n", filepath);
        return;
    }
    ota.flashFromFile(file, appId);
    file.close();
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n==============================================");
    Serial.println("       ESP32 AUTOMOTIVE CAN OTA GATEWAY       ");
    Serial.println("==============================================");

    // Khởi tạo thẻ nhớ SD qua SPI
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[SD ERR] Khoi tao SD Card that bai! Kiem tra the nho.");
    } else {
        Serial.println("[SD OK] Khoi tao SD Card thanh cong!");
    }

    // Khởi tạo ngoại vi CAN
    if (!can.begin()) {
        Serial.println("[CAN ERR] Loi khoi tao TWAI/CAN! Dung chuong trinh.");
        while (1) { delay(100); }
    }

    // Khởi tạo Web Server điều khiển OTA và WiFi (STA + AP)
    OTA_WebServer_Init(ota);

    Serial.println("\n>> San sang nhan lenh:");
    Serial.println(">> - Qua Web:   Truy cap dia chi IP cua ESP32");
    Serial.println(">> - Qua Serial: Go '1' (App1), '2' (App2), 'e' (Test CRC)\n");
}

static uint32_t lastWifiCheck = 0;

void loop() {
    // 1. Xử lý các yêu cầu Web Server
    OTA_WebServer_Loop();

    // 2. Theo dõi trạng thái kết nối WiFi STA định kỳ
    if (millis() - lastWifiCheck > 5000) {
        lastWifiCheck = millis();
        static bool wasConnected = false;
        bool isConnected = (WiFi.status() == WL_CONNECTED);
        if (isConnected && !wasConnected) {
            Serial.printf("[WIFI STA] Da ket noi thanh cong! IP: %s\n",
                          WiFi.localIP().toString().c_str());
            Serial.printf(">> Truy cap Web tai: http://%s/\n",
                          WiFi.localIP().toString().c_str());
        }
        wasConnected = isConnected;
    }

    // 3. Nhận lệnh kích hoạt qua Serial Monitor
    if (Serial.available()) {
        char ch = Serial.read();
        if (ch == '1' || ch == 's' || ch == 'S') {
            runOtaFromSD("/app1.bin", 0x01);
        } else if (ch == '2') {
            runOtaFromSD("/app2.bin", 0x02);
        } else if (ch == 'e' || ch == 'E') {
            ota.setInjectCrcError(!ota.isInjectCrcError());
            Serial.printf("[TEST] Che do Blackbox Test loi CRC: %s\n",
                          ota.isInjectCrcError() ? "BAT (ON)" : "TAT (OFF)");
        }
    }

    // 4. Lắng nghe bus CAN để debug monitor (Non-blocking: timeout = 0)
    twai_message_t rx;
    if (can.receiveFrame(rx, 0)) {
        Serial.printf("[CAN RX] ID=0x%03X, DLC=%d, Data: ", rx.identifier, rx.data_length_code);
        for (int i = 0; i < rx.data_length_code; i++) {
            Serial.printf("0x%02X ", rx.data[i]);
        }
        Serial.println();
    }
}