#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "CAN_Driver.h"
#include "Car_Controller.h"
#include "Control_WebServer.h"

/*
 * ============================================================================
 * CẤU HÌNH CÔ LẬP LỖI - TEST TRỰC TIẾP ĐỘNG CƠ QUA CAN (KHÔNG CẦN WIFI)
 * ============================================================================
 * - Đặt ENABLE_WIFI = 0: TẮT hoàn toàn WiFi và WebServer để cô lập lỗi.
 *   ESP32 gửi lệnh trực tiếp qua CAN bus 500kbps, không trễ, không lag.
 * - Đặt ENABLE_WIFI = 1: Bật lại WiFi và WebServer / WebSocket.
 */
#define ENABLE_WIFI             0

// ==================== Khởi tạo các Module ====================
CAN_Driver can(CAN_TX_PIN, CAN_RX_PIN);
Car_Controller car(can);

// Biến trạng thái cho chế độ Test Tuần tự Tự động
static bool g_autoTestRunning = false;
static uint32_t g_autoTestStepStart = 0;
static uint8_t g_autoTestStep = 0;

void printHelpMenu() {
    Serial.println("\n==================================================");
    Serial.println("  ESP32 CAR CONTROLLER - CHE DO TEST CO LAP LOI  ");
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

    // 1. Khởi tạo ngoại vi CAN (TWAI) 500kbps
    if (!can.begin()) {
        Serial.println("[CAN ERR] Khoi tao TWAI/CAN that bai! Kiem tra day noi SN65HVD230.");
    } else {
        Serial.println("[CAN OK] CAN Bus 500kbps da san sang giao tiep voi STM32.");
    }

    // 2. Khởi tạo bộ điều khiển xe
    car.begin();

#if ENABLE_WIFI
    // 3. Khởi tạo WiFi Station & Web Server điều khiển (khi ENABLE_WIFI = 1)
    Control_WebServer_Init(car);
#else
    // Tắt hoàn toàn sóng vô tuyến WiFi để tiết kiệm năng lượng và cô lập lỗi
    WiFi.mode(WIFI_OFF);
    Serial.println("[WIFI] Da TAT hoan toan WiFi. He thong dang chay o che do TEST CAN truc tiep!");
#endif
}

void loop() {
#if ENABLE_WIFI
    // 1. Xử lý các request từ trình duyệt Web
    Control_WebServer_Loop();

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

    // 4. Xử lý lệnh điều khiển từ bàn phím Serial Monitor
    if (Serial.available() > 0) {
        char c = (char)Serial.read();
        
        // Bỏ qua ký tự xuống dòng
        if (c == '\r' || c == '\n') {
            return;
        }

        // Nếu người dùng bấm phím điều khiển khi đang chạy test tự động thì dừng test
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

    // 5. Kiểm tra bản tin CAN phản hồi từ STM32 (nếu có)
    twai_message_t rx_msg;
    if (can.receiveFrame(rx_msg, 0)) {
        Serial.printf("[CAN RX] ID: 0x%03X, DLC: %d\n", rx_msg.identifier, rx_msg.data_length_code);
    }

    delay(2);
}
