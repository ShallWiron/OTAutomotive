#include "App_WebServer.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Preferences.h>
#include "IndexHtml.h"
#include "config.h"

#define WS_SERVER_PORT 81

static WebServer server(WEB_SERVER_PORT);
static WebSocketsServer wsServer(WS_SERVER_PORT);

static Car_Controller *g_car = nullptr;
static OTA_Protocol *g_ota = nullptr;
static SD_Manager *g_sd = nullptr;
static Preferences g_prefs;

#define RAM_BUFFER_MAX_SIZE (58 * 1024)
static uint8_t s_ramBuffer[RAM_BUFFER_MAX_SIZE];
static size_t s_ramSize = 0;
static File s_sdUploadFile;
static bool s_isFlashing = false;
static uint8_t s_activeApp = 1;

static void broadcastOtaProgress(int pct, const String& msg) {
    String escaped = msg;
    escaped.replace("\"", "\\\"");
    String json = "{\"type\":\"ota_progress\",\"percent\":" + String(pct) + ",\"msg\":\"" + escaped + "\"}";
    wsServer.broadcastTXT(json);
    wsServer.loop();
    delay(1);
}

static void handleApiOtaQuery() {
    if (!g_ota) {
        server.send(500, "application/json", "{\"success\":false,\"message\":\"OTA subsystem not initialized\"}");
        return;
    }

    uint8_t app = g_ota->queryCurrentApp(400);
    if (app == 1 || app == 2) {
        s_activeApp = app;
        g_prefs.putUChar("active_app", app);
        uint8_t suggested = (app == 1) ? 2 : 1;

        String json = "{\"type\":\"ota_query\",\"success\":true,\"active_app\":" + String(app) +
                      ",\"target_app\":" + String(suggested) + ",\"source\":\"CAN\"}";
        wsServer.broadcastTXT(json);
        server.send(200, "application/json", json);
    } else {
        uint8_t saved = g_prefs.getUChar("active_app", s_activeApp);
        uint8_t suggested = (saved == 1) ? 2 : 1;
        String json = "{\"type\":\"ota_query\",\"success\":false,\"active_app\":" + String(saved) +
                      ",\"target_app\":" + String(suggested) +
                      ",\"source\":\"NVS\",\"message\":\"CAN Timeout: No response from STM32\"}";
        server.send(200, "application/json", json);
    }
}

static void handleApiOtaSdStatus() {
    if (!g_sd) {
        server.send(500, "application/json", "{\"mounted\":false,\"message\":\"SD subsystem not initialized\",\"files\":[]}");
        return;
    }
    server.send(200, "application/json", g_sd->getStatusJson());
}

static void handleApiOtaUploadSdData() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        if (!g_sd || !g_sd->isReady()) {
            Serial.println("[SD ERR] Card not ready for upload");
            return;
        }

        String fname = upload.filename;
        int lastSlash = fname.lastIndexOf('/');
        if (lastSlash >= 0) fname = fname.substring(lastSlash + 1);
        int lastBackslash = fname.lastIndexOf('\\');
        if (lastBackslash >= 0) fname = fname.substring(lastBackslash + 1);
        fname.trim();
        if (fname.length() == 0) fname = "firmware.bin";
        if (!fname.startsWith("/")) fname = "/" + fname;

        s_sdUploadFile = g_sd->openWrite(fname);
        if (!s_sdUploadFile) {
            Serial.printf("[SD ERR] Cannot create file: %s\n", fname.c_str());
            return;
        }
        Serial.printf("[SD] Receiving upload: %s\n", fname.c_str());
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
        if (s_sdUploadFile) {
            s_sdUploadFile.write(upload.buf, upload.currentSize);
        }
    }
    else if (upload.status == UPLOAD_FILE_END) {
        if (s_sdUploadFile) {
            s_sdUploadFile.close();
            Serial.printf("[SD OK] Saved file: %u Bytes\n", (unsigned int)upload.totalSize);
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (s_sdUploadFile) {
            s_sdUploadFile.close();
            String fname = upload.filename;
            if (!fname.startsWith("/")) fname = "/" + fname;
            g_sd->remove(fname);
        }
    }
}

static void handleApiOtaUploadSdDone() {
    if (!g_sd || !g_sd->isReady()) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"MicroSD card not detected\"}");
        return;
    }
    server.send(200, "application/json", "{\"success\":true,\"message\":\"File saved to MicroSD successfully\"}");
}

static void handleApiOtaDeleteSd() {
    if (!g_sd || !g_sd->isReady()) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"MicroSD card not mounted\"}");
        return;
    }

    if (!server.hasArg("filename")) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing filename argument\"}");
        return;
    }

    String filename = server.arg("filename");
    if (g_sd->remove(filename)) {
        server.send(200, "application/json", "{\"success\":true,\"message\":\"File deleted successfully\"}");
    } else {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Failed to delete file\"}");
    }
}

static void handleApiOtaFlashSd() {
    if (s_isFlashing) {
        server.send(409, "application/json", "{\"success\":false,\"message\":\"OTA flash already in progress\"}");
        return;
    }

    if (!g_sd || !g_sd->isReady()) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"MicroSD card not mounted\"}");
        return;
    }

    if (!server.hasArg("filename")) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing filename argument\"}");
        return;
    }

    String filename = server.arg("filename");
    File f = g_sd->openRead(filename);
    if (!f) {
        server.send(404, "application/json", "{\"success\":false,\"message\":\"File not found on MicroSD\"}");
        return;
    }

    uint8_t targetApp = (s_activeApp == 1) ? 2 : 1;
    if (server.hasArg("target_app")) {
        int t = server.arg("target_app").toInt();
        if (t == 1 || t == 2) targetApp = (uint8_t)t;
    }

    bool testCrc = server.hasArg("test_crc") && (server.arg("test_crc") == "1");
    g_ota->setInjectCrcError(testCrc);

    s_isFlashing = true;
    bool ok = g_ota->flashFromFile(f, targetApp, broadcastOtaProgress);
    f.close();
    s_isFlashing = false;

    if (ok) {
        s_activeApp = targetApp;
        g_prefs.putUChar("active_app", targetApp);
        g_prefs.putString("last_file", filename);
        g_prefs.putString("last_status", "Flash SD Success");

        server.send(200, "application/json",
                    "{\"success\":true,\"active_app\":" + String(targetApp) +
                    ",\"message\":\"Firmware flashed from MicroSD into App " + String(targetApp) + " successfully\"}");
    } else {
        server.send(200, "application/json", "{\"success\":false,\"message\":\"" + g_ota->getLastError() + "\"}");
    }
}

static void handleApiOtaUploadData() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        s_ramSize = 0;
        s_isFlashing = true;
        Serial.printf("[RAM] Receiving HTTP stream: %s\n", upload.filename.c_str());
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
        if (s_ramSize + upload.currentSize <= RAM_BUFFER_MAX_SIZE) {
            memcpy(&s_ramBuffer[s_ramSize], upload.buf, upload.currentSize);
            s_ramSize += upload.currentSize;
        } else {
            Serial.printf("[RAM ERR] Binary exceeds max buffer size (%u bytes)\n", (unsigned int)RAM_BUFFER_MAX_SIZE);
        }
    }
    else if (upload.status == UPLOAD_FILE_END) {
        Serial.printf("[RAM OK] Stream received: %u Bytes\n", (unsigned int)s_ramSize);
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
        s_ramSize = 0;
        s_isFlashing = false;
        Serial.println("[RAM ERR] Upload stream aborted");
    }
}

static void handleApiOtaUploadDone() {
    if (s_ramSize == 0) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Binary buffer empty or transfer aborted\"}");
        s_isFlashing = false;
        return;
    }

    uint8_t targetApp = (s_activeApp == 1) ? 2 : 1;
    if (server.hasArg("target_app")) {
        int t = server.arg("target_app").toInt();
        if (t == 1 || t == 2) targetApp = (uint8_t)t;
    }

    bool testCrc = server.hasArg("test_crc") && (server.arg("test_crc") == "1");
    g_ota->setInjectCrcError(testCrc);

    bool ok = g_ota->flashFromBuffer(s_ramBuffer, s_ramSize, targetApp, broadcastOtaProgress);

    s_ramSize = 0;
    s_isFlashing = false;

    if (ok) {
        s_activeApp = targetApp;
        g_prefs.putUChar("active_app", targetApp);
        g_prefs.putString("last_file", "Direct RAM Upload");
        g_prefs.putString("last_status", "Flash RAM Success");

        server.send(200, "application/json",
                    "{\"success\":true,\"active_app\":" + String(targetApp) +
                    ",\"message\":\"Firmware flashed from RAM into App " + String(targetApp) + " successfully\"}");
    } else {
        server.send(200, "application/json", "{\"success\":false,\"message\":\"" + g_ota->getLastError() + "\"}");
    }
}

static void handleApiCmd() {
    if (!g_car) {
        server.send(500, "application/json", "{\"success\":false,\"message\":\"Car subsystem not initialized\"}");
        return;
    }
    String cmd = "";
    if (server.hasArg("action")) cmd = server.arg("action");
    else if (server.hasArg("cmd")) cmd = server.arg("cmd");
    else if (server.hasArg("code")) cmd = server.arg("code");

    if (cmd.length() == 0) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing cmd parameter\"}");
        return;
    }

    bool ok = g_car->executeCommand(cmd);
    server.send(200, "application/json", "{\"success\":" + String(ok ? "true" : "false") + ",\"action\":\"" + g_car->getLastAction() + "\"}");
}

static void handleApiStatus() {
    if (!g_car) {
        server.send(500, "application/json", "{\"error\":\"Not initialized\"}");
        return;
    }
    server.send(200, "application/json", g_car->getStatusJson());
}

static void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected -> Auto STOP for safety\n", num);
            if (g_car) g_car->stop();
            break;

        case WStype_CONNECTED: {
            uint8_t target = (s_activeApp == 1) ? 2 : 1;
            String json = "{\"type\":\"ota_query\",\"success\":true,\"active_app\":" + String(s_activeApp) +
                          ",\"target_app\":" + String(target) + "}";
            wsServer.sendTXT(num, json);
            if (g_car) {
                String st = g_car->getStatusJson();
                wsServer.sendTXT(num, st);
            }
            break;
        }

        case WStype_TEXT: {
            if (!g_car || length == 0) return;
            char buf[64];
            size_t copyLen = (length < sizeof(buf) - 1) ? length : (sizeof(buf) - 1);
            memcpy(buf, payload, copyLen);
            buf[copyLen] = '\0';

            char *p = buf;
            while (*p == ' ' || *p == '\t') p++;

            bool ok = false;
            if (*p == '{') {
                String msg(p);
                int idx = msg.indexOf("\"cmd\":\"");
                if (idx >= 0) {
                    int endIdx = msg.indexOf("\"", idx + 7);
                    if (endIdx > idx + 7) {
                        String cmdCode = msg.substring(idx + 7, endIdx);
                        ok = g_car->executeCommand(cmdCode);
                    }
                }
            } else {
                ok = g_car->executeCommand(String(p));
            }
            if (ok) {
                String st = g_car->getStatusJson();
                wsServer.broadcastTXT(st);
            }
            break;
        }
        default:
            break;
    }
}

void App_WebServer_Init(Car_Controller &car, OTA_Protocol &ota, SD_Manager &sd) {
    g_car = &car;
    g_ota = &ota;
    g_sd = &sd;

    g_prefs.begin("ota_app", false);
    s_activeApp = g_prefs.getUChar("active_app", 1);
    if (s_activeApp != 1 && s_activeApp != 2) s_activeApp = 1;

    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", INDEX_HTML);
    });

    server.on("/api/ota/query", HTTP_POST, handleApiOtaQuery);
    server.on("/api/ota/query", HTTP_GET, handleApiOtaQuery);
    server.on("/api/ota/sd_status", HTTP_GET, handleApiOtaSdStatus);
    server.on("/api/ota/upload_sd", HTTP_POST, handleApiOtaUploadSdDone, handleApiOtaUploadSdData);
    server.on("/api/ota/delete_sd", HTTP_POST, handleApiOtaDeleteSd);
    server.on("/api/ota/flash_sd", HTTP_POST, handleApiOtaFlashSd);
    server.on("/api/ota/upload", HTTP_POST, handleApiOtaUploadDone, handleApiOtaUploadData);
    server.on("/api/cmd", HTTP_GET, handleApiCmd);
    server.on("/api/cmd", HTTP_POST, handleApiCmd);
    server.on("/api/status", HTTP_GET, handleApiStatus);

    server.begin();

    wsServer.begin();
    wsServer.onEvent(webSocketEvent);

    Serial.println("[WEB OK] HTTP & WebSocket servers initialized on port 80 & 81.");
}

void App_WebServer_Loop() {
    server.handleClient();
    wsServer.loop();
}
