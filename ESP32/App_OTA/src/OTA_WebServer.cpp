#include "OTA_WebServer.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <Preferences.h>
#include <vector>
#include "config.h"

static WebServer server(80);
static OTA_Protocol* g_ota = nullptr;
static Preferences g_prefs;
static File g_uploadFile;

static String g_alertMessage = "";
static String g_alertType = ""; // "success", "error", "info"
static bool g_isFlashing = false;
static String g_appSourceInfo = "Gateway NVS (Bấm 'Truy vấn' để đồng bộ qua CAN)";

struct SdFileInfo {
    String name;
    size_t size;
    bool isBin;
};

// Hàm đảm bảo thẻ nhớ SD luôn sẵn sàng (tự mount lại nếu vừa cắm thẻ)
static bool ensureSdCard() {
    if (SD.cardType() == CARD_NONE) {
        SD.end();
        return SD.begin(SD_CS_PIN);
    }
    return true;
}

// Quét danh sách file trên thẻ nhớ SD
static std::vector<SdFileInfo> getSdFileList() {
    std::vector<SdFileInfo> files;
    if (!ensureSdCard()) {
        return files;
    }

    File root = SD.open("/");
    if (!root || !root.isDirectory()) {
        return files;
    }

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String fname = String(file.name());
            if (!fname.startsWith("/")) {
                fname = "/" + fname;
            }
            bool isBin = fname.endsWith(".bin") || fname.endsWith(".BIN");
            files.push_back({ fname, file.size(), isBin });
        }
        file = root.openNextFile();
    }
    root.close(); // Đóng directory tránh leak file descriptor
    return files;
}

static String buildHtmlPage() {
    uint8_t activeApp = g_prefs.getUChar("active_app", 1);
    String lastFile = g_prefs.getString("last_file", "Chưa có");
    uint32_t lastSize = g_prefs.getUInt("last_size", 0);
    String lastStatus = g_prefs.getString("last_status", "Sẵn sàng");

    std::vector<SdFileInfo> fileList = getSdFileList();
    int binCount = 0;
    for (const auto& f : fileList) {
        if (f.isBin) binCount++;
    }

    String html = "<!DOCTYPE html><html lang='vi'><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>ESP32 CAN OTA Gateway</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 15px; background: #f4f6f8; color: #333; line-height: 1.5; }";
    html += ".container { max-width: 800px; margin: 0 auto; background: #fff; padding: 20px; border-radius: 6px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
    html += "h2 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 8px; margin-top: 0; }";
    html += "fieldset { border: 1px solid #ccc; border-radius: 4px; padding: 12px 15px; margin-bottom: 16px; background: #fafafa; }";
    html += "legend { font-weight: bold; color: #2980b9; padding: 0 5px; }";
    html += "table { width: 100%; border-collapse: collapse; margin-top: 8px; background: #fff; }";
    html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
    html += "th { background-color: #eee; }";
    html += "button, input[type='submit'] { background: #3498db; color: #fff; border: none; padding: 8px 14px; border-radius: 4px; cursor: pointer; font-size: 14px; }";
    html += "button:hover, input[type='submit']:hover { background: #2980b9; }";
    html += "button:disabled, input[type='submit']:disabled { background: #bbb; cursor: not-allowed; }";
    html += ".btn-danger { background: #e74c3c; }";
    html += ".btn-danger:hover { background: #c0392b; }";
    html += ".btn-flash { background: #27ae60; font-weight: bold; font-size: 15px; padding: 10px 18px; }";
    html += ".btn-flash:hover { background: #219150; }";
    html += ".alert { padding: 10px 14px; border-radius: 4px; margin-bottom: 16px; font-weight: bold; }";
    html += ".alert-success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }";
    html += ".alert-error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }";
    html += ".alert-info { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }";
    html += ".badge { display: inline-block; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 13px; }";
    html += ".badge-app1 { background: #3498db; color: #fff; }";
    html += ".badge-app2 { background: #e67e22; color: #fff; }";
    html += "</style>";
    html += "<script>";
    html += "function onFlashSubmit() {";
    html += "  if (!confirm('Xác nhận bắt đầu nạp Firmware qua mạng CAN Bus sang STM32?\\nQuá trình sẽ mất khoảng 5-10 giây.')) return false;";
    html += "  var btn = document.getElementById('btnFlash');";
    html += "  if (btn) { btn.disabled = true; btn.innerText = 'Đang nạp OTA qua CAN Bus... Vui lòng đợi!'; btn.style.background = '#888'; }";
    html += "  return true;";
    html += "}";
    html += "</script>";
    html += "</head><body>";

    html += "<div class='container'>";
    html += "<h2>BỘ ĐIỀU KHIỂN CAN OTA GATEWAY (ESP32 - STM32)</h2>";

    // Khối thông báo Alert
    if (g_alertMessage.length() > 0) {
        String cls = "alert-info";
        if (g_alertType == "success") cls = "alert-success";
        else if (g_alertType == "error") cls = "alert-error";
        html += "<div class='alert " + cls + "'>" + g_alertMessage + "</div>";
        g_alertMessage = ""; // Reset sau khi hiển thị
        g_alertType = "";
    }

    // 1. Khối thông tin hệ thống & STM32
    html += "<fieldset><legend>1. Thông tin hệ thống & STM32 ECU</legend>";
    html += "<p><b>WiFi STA IP:</b> " + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("Chưa kết nối (Đang quét...)")) + "</p>";
    html += "<p><b>WiFi SoftAP:</b> " + String(WIFI_AP_SSID) + " (IP: " + WiFi.softAPIP().toString() + ")</p>";
    html += "<p><b>Thẻ nhớ SD:</b> " + String(SD.cardType() != CARD_NONE ? "Đã sẵn sàng" : "<span style='color:red;'>Chưa nhận thẻ nhớ!</span>") + "</p>";
    html += "<hr style='border:0; border-top:1px dashed #ccc;'>";
    html += "<p><b>Phân vùng App hiện tại của STM32:</b> ";
    if (activeApp == 1) {
        html += "<span class='badge badge-app1'>APP 1 (0x08004000)</span>";
    } else if (activeApp == 2) {
        html += "<span class='badge badge-app2'>APP 2 (0x08012000)</span>";
    } else {
        html += "<span>Chưa xác định (Mặc định: App 1)</span>";
    }
    html += "</p>";
    html += "<p style='font-size:13px; color:#555;'><i>Nguồn xác thực: " + g_appSourceInfo + "</i></p>";
    html += "<p><b>Lần nạp gần nhất:</b> " + lastFile + " (" + String(lastSize) + " Bytes) - Trạng thái: " + lastStatus + "</p>";
    html += "<form method='POST' action='/query' style='margin-top:8px;'>";
    html += "<button type='submit'>Truy vấn App từ STM32 qua CAN</button>";
    html += "</form>";
    html += "</fieldset>";

    // 2. Tải file lên thẻ nhớ SD
    html += "<fieldset><legend>2. Tải file Firmware (.bin) lên thẻ nhớ SD</legend>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='firmware' accept='.bin' required style='margin-right:10px;'>";
    html += "<button type='submit'>Tải lên SD</button>";
    html += "</form>";
    html += "</fieldset>";

    // 3. Danh sách file trong thẻ nhớ SD
    html += "<fieldset><legend>3. Danh sách file trong thẻ nhớ SD (" + String(fileList.size()) + " files)</legend>";
    if (fileList.empty()) {
        html += "<p><i>Chưa có file nào trên thẻ nhớ SD hoặc chưa cắm thẻ nhớ. Vui lòng kiểm tra thẻ nhớ và tải file .bin lên ở Mục 2.</i></p>";
    } else {
        html += "<table><thead><tr><th>Tên file</th><th>Kích thước</th><th>Định dạng</th><th>Hành động</th></tr></thead><tbody>";
        for (const auto& f : fileList) {
            html += "<tr>";
            html += "<td>" + f.name + "</td>";
            html += "<td>" + String(f.size) + " bytes</td>";
            html += "<td>" + String(f.isBin ? "<b style='color:green;'>Firmware .bin</b>" : "File khác") + "</td>";
            html += "<td>";
            html += "<form method='POST' action='/delete' style='display:inline;' onsubmit='return confirm(\"Xác nhận xóa file " + f.name + " trên thẻ nhớ?\");'>";
            html += "<input type='hidden' name='filename' value='" + f.name + "'>";
            html += "<button type='submit' class='btn-danger' style='padding:4px 8px; font-size:12px;'>Xóa</button>";
            html += "</form>";
            html += "</td>";
            html += "</tr>";
        }
        html += "</tbody></table>";
    }
    html += "</fieldset>";

    // 4. Chọn file và phân vùng để Flash OTA
    html += "<fieldset><legend>4. Chọn file & Phân vùng để Flash OTA qua CAN</legend>";
    if (binCount == 0) {
        html += "<p style='color:#c0392b;'><i>Không tìm thấy file firmware (.bin) nào trên thẻ nhớ SD. Vui lòng tải file .bin ở Mục 2 để kích hoạt tính năng nạp.</i></p>";
    } else {
        html += "<form method='POST' action='/flash' onsubmit='return onFlashSubmit();'>";
        html += "<div style='margin-bottom:12px;'>";
        html += "<label><b>Chọn file firmware (.bin):</b></label><br>";
        html += "<select name='filename' style='padding:6px; min-width:260px; margin-top:4px;'>";
        for (const auto& f : fileList) {
            if (f.isBin) {
                html += "<option value='" + f.name + "'>" + f.name + " (" + String(f.size) + " bytes)</option>";
            }
        }
        html += "</select>";
        html += "</div>";

        // Gợi ý phân vùng đích theo kiến trúc Dual-Bank
        uint8_t targetSuggestion = (activeApp == 1) ? 2 : 1;

        html += "<div style='margin-bottom:12px;'>";
        html += "<label><b>Chọn phân vùng đích trên STM32:</b></label><br>";
        html += "<label style='margin-right:20px;'>";
        html += "<input type='radio' name='app_id' value='1' " + String(targetSuggestion == 1 ? "checked" : "") + "> ";
        html += "<b>App 1</b> (Địa chỉ: 0x08004000, 56KB)";
        html += "</label><br>";
        html += "<label>";
        html += "<input type='radio' name='app_id' value='2' " + String(targetSuggestion == 2 ? "checked" : "") + "> ";
        html += "<b>App 2</b> (Địa chỉ: 0x08012000, 56KB)";
        html += "</label>";
        html += "<p style='font-size:12px; color:#666; margin:4px 0 0 0;'>";
        html += "* Gợi ý Dual-Bank: Hiện STM32 đang là App " + String(activeApp) + ", hệ thống tự động chọn phân vùng dự phòng App " + String(targetSuggestion) + ".";
        html += "</p>";
        html += "</div>";

        html += "<div style='margin-bottom:15px;'>";
        html += "<label>";
        html += "<input type='checkbox' name='crc_err' value='1'> ";
        html += "Giả lập lỗi CRC ở Chunk #2 (Blackbox Test kiểm thử cơ chế Fail-safe Rollback)";
        html += "</label>";
        html += "</div>";

        html += "<button type='submit' id='btnFlash' class='btn-flash' " + String(g_isFlashing ? "disabled" : "") + ">";
        html += g_isFlashing ? "ĐANG NẠP OTA... XIN ĐỢI!" : "BẮT ĐẦU FLASH OTA QUA CAN";
        html += "</button>";
        html += "</form>";
    }
    html += "</fieldset>";

    html += "</div></body></html>";
    return html;
}

static void handleRoot() {
    server.send(200, "text/html", buildHtmlPage());
}

static void handleUploadFile() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        ensureSdCard();

        // Lấy tên file gốc (cắt bỏ đường dẫn từ phía client nếu có)
        String filename = upload.filename;
        int lastSlash = filename.lastIndexOf('/');
        if (lastSlash >= 0) filename = filename.substring(lastSlash + 1);
        int lastBackslash = filename.lastIndexOf('\\');
        if (lastBackslash >= 0) filename = filename.substring(lastBackslash + 1);
        filename.trim();
        if (filename.length() == 0) filename = "firmware.bin";
        if (!filename.startsWith("/")) filename = "/" + filename;

        Serial.printf("[HTTP] Bat dau tai file len SD: %s\n", filename.c_str());

        if (SD.exists(filename)) {
            SD.remove(filename);
        }

        g_uploadFile = SD.open(filename, FILE_WRITE);
        if (!g_uploadFile) {
            Serial.printf("[HTTP ERR] Khong the tao file tren SD: %s\n", filename.c_str());
            g_alertMessage = "Khong the tao file tren the nho SD! Kiem tra the nho.";
            g_alertType = "error";
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (g_uploadFile) {
            g_uploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (g_uploadFile) {
            g_uploadFile.close();
            Serial.printf("[HTTP] Tai file len SD hoan tat: %s (%u bytes)\n",
                          upload.filename.c_str(), upload.totalSize);
            g_alertMessage = "Tai len the nho thanh cong: " + upload.filename + " (" + String(upload.totalSize) + " bytes)";
            g_alertType = "success";
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (g_uploadFile) {
            g_uploadFile.close();
            Serial.println("[HTTP ERR] Upload bi huy!");
        }
        g_alertMessage = "Qua trinh tai file bi huy!";
        g_alertType = "error";
    }
}

static void handleUploadDone() {
    server.sendHeader("Location", "/");
    server.send(303);
}

static void handleFlash() {
    if (g_isFlashing) {
        g_alertMessage = "He thong dang thuc hien nap OTA, vui long doi!";
        g_alertType = "error";
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    if (!server.hasArg("filename") || !server.hasArg("app_id")) {
        g_alertMessage = "Thieu thong tin file hoac phan vung dich!";
        g_alertType = "error";
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    String filename = server.arg("filename");
    int appId = server.arg("app_id").toInt();
    bool injectCrc = server.hasArg("crc_err");

    if (appId != 1 && appId != 2) {
        g_alertMessage = "Phan vung khong hop le (chi chap nhan App 1 hoac App 2)!";
        g_alertType = "error";
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    ensureSdCard();

    if (!SD.exists(filename)) {
        g_alertMessage = "File " + filename + " khong ton tai tren the nho SD!";
        g_alertType = "error";
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    File file = SD.open(filename, FILE_READ);
    if (!file) {
        g_alertMessage = "Khong the mo file: " + filename;
        g_alertType = "error";
        server.sendHeader("Location", "/");
        server.send(303);
        return;
    }

    size_t fileSize = file.size();
    g_ota->setInjectCrcError(injectCrc);

    Serial.printf("[WEB] Kich hoat flash file %s vao App %d qua CAN (CRC Error Test=%s)...\n",
                  filename.c_str(), appId, injectCrc ? "ON" : "OFF");

    g_isFlashing = true;
    bool ok = g_ota->flashFromFile(file, (uint8_t)appId);
    file.close();
    g_isFlashing = false;

    if (ok) {
        g_prefs.putUChar("active_app", (uint8_t)appId);
        g_prefs.putString("last_file", filename);
        g_prefs.putUInt("last_size", (uint32_t)fileSize);
        g_prefs.putString("last_status", "Thanh cong (100%)");
        g_appSourceInfo = "Vua nap thanh cong qua CAN vao App " + String(appId);

        g_alertMessage = "FLASH THANH CONG! Da nap " + filename + " vao App " + String(appId) + ". STM32 da boot vao firmware moi!";
        g_alertType = "success";
    } else {
        String err = g_ota->getLastError();
        if (err.isEmpty()) err = "Loi truyen CAN hoac verify firmware that bai!";
        g_prefs.putString("last_status", "That bai: " + err);

        g_alertMessage = "FLASH THAT BAI: " + err;
        g_alertType = "error";
    }

    server.sendHeader("Location", "/");
    server.send(303);
}

static void handleQuery() {
    Serial.println("[WEB] Gui lenh truy van App status sang STM32 qua CAN...");
    uint8_t stmApp = g_ota->queryCurrentApp(400);

    if (stmApp == 1 || stmApp == 2) {
        g_prefs.putUChar("active_app", stmApp);
        g_appSourceInfo = "Xac thuc truc tiep tu STM32 qua CAN Bus (500 kbps)";
        g_alertMessage = "STM32 phan hoi truc tiep qua CAN: Dang hoat dong o App " + String(stmApp);
        g_alertType = "success";
    } else {
        g_appSourceInfo = "Gateway NVS (STM32 khong phan hoi qua CAN luc nay)";
        g_alertMessage = "STM32 khong phan hoi qua CAN (Co the dang ban chay xe hoac chua bat). Hien thi theo bo nho Gateway.";
        g_alertType = "info";
    }

    server.sendHeader("Location", "/");
    server.send(303);
}

static void handleDelete() {
    if (server.hasArg("filename")) {
        ensureSdCard();
        String filename = server.arg("filename");
        if (SD.exists(filename)) {
            SD.remove(filename);
            g_alertMessage = "Da xoa file: " + filename;
            g_alertType = "info";
        }
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void OTA_WebServer_Init(OTA_Protocol& otaProtocol) {
    g_ota = &otaProtocol;
    g_prefs.begin("ota_gw", false);

    // Cấu hình Wi-Fi: Chạy song song cả STA và SoftAP
    WiFi.mode(WIFI_AP_STA);

    // Bật SoftAP để luôn truy cập được kể cả mang ra ngoài xe không có router
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    Serial.println("\n[WIFI AP] Da khoi tao SoftAP:");
    Serial.printf("  -> SSID:     %s\n", WIFI_AP_SSID);
    Serial.printf("  -> Password: %s\n", WIFI_AP_PASSWORD);
    Serial.printf("  -> IP AP:    %s\n", WiFi.softAPIP().toString().c_str());

    // Kết nối vào mạng WiFi phòng thí nghiệm
    Serial.printf("[WIFI STA] Dang ket noi vao %s...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Cài đặt các route cho Web Server
    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, handleUploadDone, handleUploadFile);
    server.on("/flash", HTTP_POST, handleFlash);
    server.on("/query", HTTP_POST, handleQuery);
    server.on("/delete", HTTP_POST, handleDelete);

    server.begin();
    Serial.println("[HTTP] Web Server da khoi dong tren port 80!");
    Serial.println(">> Truy cap Web dieu khien tai:");
    Serial.printf("   - Qua SoftAP: http://%s/\n", WiFi.softAPIP().toString().c_str());
}

void OTA_WebServer_Loop() {
    server.handleClient();
}
