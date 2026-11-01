#include <Arduino.h>
#include "config.h"
#include "CAN_Driver.h"
#include "Car_Controller.h"
#include "Control_WebServer.h"

// ==================== Khởi tạo các Module ====================
CAN_Driver can(CAN_TX_PIN, CAN_RX_PIN);
Car_Controller car(can);

void printHelpMenu() {
    Serial.println("\n==================================================");
    Serial.println("           ESP32 AUTOMOTIVE CAR CONTROLLER        ");
    Serial.println("==================================================");
    Serial.println("Phim tat Serial (Khong can Web van test duoc xe):");
    Serial.println("  [x] : Chuyen che do BANH XE (Motor)");
    Serial.println("  [X] : Chuyen che do TAY GAP (Servo)");
    Serial.println("  [!] : DUNG KHAN CAP (Emergency Stop)");
    Serial.println("  --- O che do Banh xe ---");
    Serial.println("  [F] : Tien          | [B] : Lui");
    Serial.println("  [L] : Quay Trai     | [R] : Quay Phai");
    Serial.println("  [S] : Dung banh xe");
    Serial.println("  [G/H] : Banh trai Tien/Lui");
    Serial.println("  [I/J] : Banh phai Tien/Lui");
    Serial.println("  --- O che do Tay gap ---");
    Serial.println("  [F] : Nang tay      | [B] : Ha tay");
    Serial.println("  [L] : Keo dai tay   | [R] : Thu tay ve");
    Serial.println("  [W] : Mo kep        | [w] : Dong kep");
    Serial.println("  --- He thong ---");
    Serial.println("  [O] : Kich hoat Bootloader OTA tren STM32");
    Serial.println("  [?] : Xem trang thai JSON hien tai");
    Serial.println("==================================================\n");
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);

    printHelpMenu();

    // 1. Khởi tạo ngoại vi CAN (TWAI) 500kbps
    if (!can.begin()) {
        Serial.println("[ERR] Khoi tao TWAI/CAN that bai! Kiem tra day noi SN65HVD230.");
    } else {
        Serial.println("[OK] CAN Bus da san sang truyen nhan voi STM32.");
    }

    // 2. Khởi tạo bộ điều khiển xe
    car.begin();

    // 3. Khởi tạo WiFi Access Point & Web Server điều khiển
    Control_WebServer_Init(car);
}

void loop() {
    // 1. Xu ly cac request tu trinh duyet Web
    Control_WebServer_Loop();

    // 2. Xu ly lenh dieu khien tu Serial Monitor (Thu nghiem truc tiep)
    if (Serial.available() > 0) {
        char c = (char)Serial.read();
        
        // Bo qua ky tu xuong dong
        if (c == '\r' || c == '\n') {
            return;
        }

        if (c == '!') {
            car.emergencyStop();
        } else if (c == 'O') {
            car.triggerBootloader();
        } else if (c == '?') {
            Serial.printf("[STATUS] %s\n", car.getStatusJson().c_str());
        } else {
            String cmdStr(c);
            car.executeCommand(cmdStr);
        }
    }

    // 3. Kiem tra ban tin CAN phan hoi hoac telemetry tu STM32 (neu co)
    twai_message_t rx_msg;
    if (can.receiveFrame(rx_msg, 0)) {
        Serial.printf("[CAN RX] ID: 0x%03X, DLC: %d\n", rx_msg.identifier, rx_msg.data_length_code);
    }

    delay(2);
}