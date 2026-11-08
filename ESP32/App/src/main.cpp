#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include "config.h"
#include "CAN_Driver.h"
#include "Car_Controller.h"
#include "OTA_Protocol.h"
#include "App_WebServer.h"

/*
 * ============================================================================
 * CẤU HÌNH CÔ LẬP LỖI - TEST TRỰC TIẾP ĐỘNG CƠ QUA CAN (KHÔNG CẦN WIFI)
 * ============================================================================
 * - Đặt ENABLE_WIFI = 0: TẮT hoàn toàn WiFi và WebServer để cô lập lỗi.
 *   ESP32 gửi lệnh trực tiếp qua CAN bus 500kbps, không trễ, không lag.
 * - Đặt ENABLE_WIFI = 1: Bật lại WiFi và WebServer / WebSocket.
 */
#define ENABLE_WIFI             0

// ==================== Khởi tạo các Module dùng chung ====================
CAN_Driver can(CAN_TX_PIN, CAN_RX_PIN);
Car_Controller car(can);
OTA_Protocol ota(can);

// Biến trạng thái cho chế độ Test Tuần tự Tự động
static bool g_autoTestRunning = false;
static uint32_t g_autoTestStepStart = 0;
static uint8_t g_autoTestStep = 0;

void runOtaFromSD(const char* filepath, uint8_t appId) {
    File file = SD.open(filepath, FILE_READ);
    if (!file) {
        Serial.printf("[SD ERR] Khong the mo file: %s!\n", filepath);
        return;
    }
    ota.flashFromFile(file, appId);
    file.close();
}

void printHelpMenu() {
    Serial.println("\n==================================================");
    Serial.println("  ESP32 AUTOMOTIVE GATEWAY - CHE DO TEST CO LAP   ");
    Serial.println("==================================================");
    Serial.printf("  WiFi & WebServer: %s\n", (ENABLE_WIFI ? "BAT (ON)" : "DA TAT (OFF)"));
    Serial.println(">> Phim tat Serial dieu khien dong co:");
    Serial.println("  [W] hoac [F] : TIEN           (CAN 0x041)");
    Serial.println("  [S] hoac [B] : LUI            (CAN 0x042)");
    Serial.println("  [A] hoac [L] : QUAY TRAI      (CAN 0x043)");
    Serial.println("  [D] hoac [R] : QUAY PHAI      (CAN 0x044)");
    Serial.println("  [X] hoac [Space] : DUNG XE    (CAN 0x040)");
    Serial.println("  -- 4 huong re cheo --");
    Serial.println("  [Q] hoac [G] : TIEN - TRAI    (CAN 0x045)");
    Serial.println("  [E] hoac [I] : TIEN - PHAI    (CAN 0x047)");
    Serial.println("  [Z] hoac [H] : LUI - TRAI     (CAN 0x046)");
    Serial.println("  [C] hoac [J] : LUI - PHAI     (CAN 0x048)");
    Serial.println("  -- Kich ban Test tu dong --");
    Serial.println("  [T] : BAT / TAT Chu trinh Test tu dong 4 huong");
    Serial.println("  [!] : DUNG KHAN CAP (E-STOP)");
    Serial.println("  [?] : Xem trang thai xe hien tai");
    Serial.println("==================================================\n");
}

/*
 * Kịch bản Test Cố định (Fix cứng code):
 * Chạy tuần tự không cần mạng để kiểm tra chiều quay 4 bánh L298N.
 */
void processAutoTest() {
    if (!g_autoTestRunning) return;

    uint32_t now = millis();
    uint32_t elapsed = now - g_autoTestStepStart;

    switch (g_autoTestStep) {
        case 0:
            Serial.println("[AUTO TEST] >>> BUOC 1: TIEN (2 giay) [CAN 0x041]...");
            car.moveForward();
            g_autoTestStepStart = now;
            g_autoTestStep++;
            break;

        case 1:
            if (elapsed >= 2000) {
                Serial.println("[AUTO TEST] >>> DUNG (1 giay) [CAN 0x040]...");
                car.stop();
                g_autoTestStepStart = now;
                g_autoTestStep++;
            }
            break;

        case 2:
            if (elapsed >= 1000) {
                Serial.println("[AUTO TEST] >>> BUOC 2: LUI (2 giay) [CAN 0x042]...");
                car.moveBackward();
                g_autoTestStepStart = now;
                g_autoTestStep++;
            }
            break;

        case 3:
            if (elapsed >= 2000) {
                Serial.println("[AUTO TEST] >>> DUNG (1 giay) [CAN 0x040]...");
                car.stop();
                g_autoTestStepStart = now;
                g_autoTestStep++;
            }
            break;

        case 4:
            if (elapsed >= 1000) {
                Serial.println("[AUTO TEST] >>> BUOC 3: QUAY TRAI (1.5 giay) [CAN 0x043]...");
                car.turnLeft();
                g_autoTestStepStart = now;
                g_autoTestStep++;
            }
            break;

        case 5:
            if (elapsed >= 1500) {
                Serial.println("[AUTO TEST] >>> DUNG (1 giay) [CAN 0x040]...");
                car.stop();
                g_autoTestStepStart = now;
                g_autoTestStep++;
            }
            break;

        case 6:
            if (elapsed >= 1000) {
                Serial.println("[AUTO TEST] >>> BUOC 4: QUAY PHAI (1.5 giay) [CAN 0x044]...");
                car.turnRight();
                g_autoTestStepStart = now;
                g_autoTestStep++;
            }
            break;

        case 7:
            if (elapsed >= 1500) {
                Serial.println("[AUTO TEST] >>> DUNG TOAN BO XE [CAN 0x040] -> HOAN THANH TEST!");
                car.stop();
                g_autoTestRunning = false;
                g_autoTestStep = 0;
            }
            break;

        default:
            g_autoTestRunning = false;
            g_autoTestStep = 0;
            break;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);

    printHelpMenu();

    // 1. Khởi tạo ngoại vi CAN (TWAI) 500kbps dùng chung
    if (!can.begin()) {
        Serial.println("[CAN ERR] Loi khoi tao TWAI/CAN! Kiem tra transceiver SN65HVD230.");
    } else {
        Serial.println("[CAN OK] CAN Bus 500kbps da san sang giao tiep voi STM32.");
    }

    // 2. Khởi tạo Car Controller
    car.begin();

#if ENABLE_WIFI
    // Khởi tạo thẻ nhớ SD & Web Server khi ENABLE_WIFI = 1
    if (SD.begin(SD_CS_PIN)) {
        Serial.println("[SD OK] Khoi tao SD Card thanh cong!");
    }
    App_WebServer_Init(car, ota);
#else
    // Tắt hoàn toàn sóng vô tuyến WiFi để tiết kiệm năng lượng và cô lập lỗi
    WiFi.mode(WIFI_OFF);
    Serial.println("[WIFI] Da TAT hoan toan WiFi. He thong dang chay o che do TEST CAN truc tiep!");
#endif
}

void loop() {
#if ENABLE_WIFI
    // 1. Xử lý các request từ trình duyệt Web
    App_WebServer_Loop();

    // 2. Theo dõi trạng thái kết nối WiFi STA định kỳ
    static uint32_t lastWifiCheck = 0;
    if (millis() - lastWifiCheck > 5000) {
        lastWifiCheck = millis();
        static bool wasConnected = false;
        bool isConnected = (WiFi.status() == WL_CONNECTED);
        if (isConnected && !wasConnected) {
            Serial.printf("\n[WIFI STA] Da ket noi: %s | IP: %s\n",
                          WIFI_SSID, WiFi.localIP().toString().c_str());
        }
        wasConnected = isConnected;
    }
#endif

    // 3. Chạy máy trạng thái Test Tuần tự Tự động (nếu được kích hoạt)
    processAutoTest();

    // 4. Nhận lệnh kích hoạt qua Serial Monitor
    if (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c != '\r' && c != '\n') {
            // Nếu người dùng can thiệp khi đang test tự động
            if (g_autoTestRunning && c != 't' && c != 'T') {
                g_autoTestRunning = false;
                g_autoTestStep = 0;
                Serial.println("[AUTO TEST] Nguoi dung can thiep -> Da huy chu trinh test tu dong!");
            }

            if (c == 't' || c == 'T') {
                g_autoTestRunning = !g_autoTestRunning;
                g_autoTestStep = 0;
                g_autoTestStepStart = millis();
                if (g_autoTestRunning) {
                    Serial.println("\n[AUTO TEST] BAT CHU TRINH TEST TU DONG (Tien -> Dung -> Lui -> Dung -> Trai -> Dung -> Phai -> Dung)...");
                } else {
                    car.stop();
                    Serial.println("\n[AUTO TEST] DA TAT CHU TRINH TEST TU DONG!");
                }
            } else if (c == '1') {
                runOtaFromSD("/app1.bin", 0x01);
            } else if (c == '2') {
                runOtaFromSD("/app2.bin", 0x02);
            } else if (c == 'e' || c == 'E') {
                ota.setInjectCrcError(!ota.isInjectCrcError());
                Serial.printf("[TEST] Che do Test loi CRC: %s\n",
                              ota.isInjectCrcError() ? "BAT (ON)" : "TAT (OFF)");
            } else if (c == '!') {
                car.emergencyStop();
            } else if (c == 'O' || c == 'o') {
                car.triggerBootloader();
            } else if (c == '?') {
                Serial.printf("[STATUS] %s\n", car.getStatusJson().c_str());
            } else {
                String cmdStr(c);
                car.executeCommand(cmdStr);
            }
        }
    }

    // 5. Lắng nghe bus CAN để debug monitor (Non-blocking: timeout = 0)
    twai_message_t rx;
    if (can.receiveFrame(rx, 0)) {
        Serial.printf("[CAN RX] ID=0x%03X, DLC=%d, Data: ", rx.identifier, rx.data_length_code);
        for (int i = 0; i < rx.data_length_code; i++) {
            Serial.printf("0x%02X ", rx.data[i]);
        }
        Serial.println();
    }

    delay(2);
}
