#include "OTA_Protocol.h"
#include "CRC16.h"

OTA_Protocol::OTA_Protocol(CAN_Driver& can_driver)
    : _can(can_driver), _injectCrcError(false), _lastError("") {}

uint8_t OTA_Protocol::queryCurrentApp(uint32_t timeout_ms) {
    _can.clearRxQueue();
    const uint8_t queryPayload[1] = { 0x55 };
    if (!_can.sendFrame(CAN_ID_QUERY_APP, queryPayload, 1)) {
        Serial.println("[OTA] Khong the phat frame QUERY_APP!");
        return 0;
    }

    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        twai_message_t rx;
        if (_can.receiveFrame(rx, 10)) {
            if (rx.identifier == CAN_ID_RESPONSE && rx.data_length_code >= 2) {
                if (rx.data[0] == OTA_ACK) {
                    uint8_t appId = rx.data[1];
                    Serial.printf("[OTA] STM32 phan hoi truc tiep qua CAN: App %d\n", appId);
                    return appId;
                }
            }
        }
    }
    return 0;
}

bool OTA_Protocol::triggerBootloader() {
    Serial.print("[OTA] [0/4] Gui REQ_BOOT qua CAN sang STM32 (ID: 0x600)... ");
    const uint8_t magicKey[4] = { 0xDE, 0xAD, 0xBE, 0xEF };

    if (_can.sendWithAck(CAN_ID_REQ_BOOT, magicKey, 4, CAN_ID_RESPONSE, OTA_ACK, 500, 2)) {
        Serial.println("OK (STM32 da nhan lenh & dang Soft-Reset!)");
        delay(200); // Chờ STM32 Bootloader khởi động hoàn tất
        return true;
    }

    Serial.println("Timeout (Co the STM32 da o san trong Bootloader). Tiep tuc thu START_OTA...");
    return false;
}

bool OTA_Protocol::burstChunkData(const uint8_t* chunkBuf, size_t chunkLen) {
    size_t sent = 0;
    uint8_t packet_idx = 0;

    while (sent < chunkLen) {
        uint8_t len = (chunkLen - sent) >= 8 ? 8 : (chunkLen - sent);
        uint32_t frame_id = CAN_ID_DATA_CHUNK | (packet_idx & 0x3FU);

        if (!_can.sendFrame(frame_id, &chunkBuf[sent], len)) {
            Serial.printf("\n[OTA ERR] Loi phat frame CAN tai offset %u!\n", (unsigned int)sent);
            return false;
        }

        sent += len;
        packet_idx++;

        // Delay 3ms giữa các frame, đồng thời kiểm tra xem STM32 có NACK sớm do sai STT không
        uint32_t wait_start = millis();
        while (millis() - wait_start < 3) {
            twai_message_t rx;
            if (_can.receiveFrame(rx, 0)) {
                if (rx.identifier == CAN_ID_RESPONSE && rx.data_length_code > 0 && rx.data[0] == OTA_NACK) {
                    Serial.printf("\n[OTA CAN] Nhan NACK som o goi STT #%u -> Dung truyen chunk!\n", packet_idx - 1);
                    return false;
                }
            }
            delayMicroseconds(500);
        }
    }
    return true;
}

bool OTA_Protocol::sendChunk(const uint8_t* chunkBuf, size_t chunkLen, uint16_t chunkIndex) {
    uint16_t chunkCrc = crc16_calculate(chunkBuf, chunkLen);

    for (uint8_t attempt = 1; attempt <= MAX_CHUNK_RETRIES; attempt++) {
        _can.clearRxQueue();

        // 1. Gửi START_CHUNK kèm độ dài chunk (2 bytes Big Endian)
        uint8_t startPayload[2] = {
            (uint8_t)((chunkLen >> 8) & 0xFF),
            (uint8_t)(chunkLen & 0xFF)
        };
        if (!_can.sendFrame(CAN_ID_START_CHUNK, startPayload, 2)) {
            delay(30);
            continue;
        }
        delay(5);

        // 2. Bắn chuỗi các frame data của Chunk
        if (!burstChunkData(chunkBuf, chunkLen)) {
            delay(30);
            continue;
        }

        // 3. Gửi END_CHUNK kèm mã Chunk CRC16
        uint16_t txCrc = chunkCrc;
        if (_injectCrcError && chunkIndex == 2 && attempt == 1) {
            txCrc ^= 0xFFFFU; // Giả lập lỗi CRC ở Chunk 2
            Serial.printf("  [BLACKBOX TEST] Gia lap sai CRC Chunk #%u (0x%04X -> 0x%04X)...\n",
                          chunkIndex, chunkCrc, txCrc);
        }

        uint8_t endPayload[2] = {
            (uint8_t)((txCrc >> 8) & 0xFF),
            (uint8_t)(txCrc & 0xFF)
        };
        if (!_can.sendFrame(CAN_ID_END_CHUNK, endPayload, 2)) {
            delay(30);
            continue;
        }

        // 4. Chờ STM32 ghi Flash & gửi ACK
        if (_can.waitResponse(CAN_ID_RESPONSE, OTA_ACK, 3000)) {
            return true;
        }

        Serial.printf("  [OTA RETRY] Chunk #%u bi NACK/Timeout! Thu lai (%u/%u)...\n",
                      chunkIndex, attempt, MAX_CHUNK_RETRIES);
        delay(50);
    }

    return false;
}

bool OTA_Protocol::flashFromFile(File& file, uint8_t appId) {
    if (!file) {
        _lastError = "Khong the mo file tren the nho SD!";
        Serial.println("[OTA ERR] " + _lastError);
        return false;
    }

    size_t totalSize = file.size();
    if (totalSize == 0) {
        _lastError = "File firmware rong (0 bytes)!";
        Serial.println("[OTA ERR] " + _lastError);
        return false;
    }

    // 1. Tính Global CRC16 trước khi gửi
    uint8_t buffer[CHUNK_SIZE];
    uint16_t globalCrc = 0xFFFFU;
    file.seek(0);
    while (file.available()) {
        size_t n = file.read(buffer, sizeof(buffer));
        if (n == 0) break;
        globalCrc = crc16_update(globalCrc, buffer, n);
    }
    file.seek(0);

    Serial.printf("[OTA] Bat dau nap App %d | Size: %uB | Global CRC16: 0x%04X\n",
                  appId, (unsigned int)totalSize, globalCrc);

    return flashFromStream(file, totalSize, globalCrc, appId);
}

bool OTA_Protocol::flashFromStream(Stream& stream, size_t fileSize, uint16_t globalCrc, uint8_t appId) {
    if (fileSize == 0) {
        _lastError = "Kich thuoc firmware bang 0!";
        return false;
    }

    _lastError = "";

    // Pha 0: Kích hoạt STM32 vào Bootloader
    triggerBootloader();

    // Pha 1: Bắt đầu OTA (Xóa Flash)
    Serial.printf("[OTA] [1/4] Gui START_OTA (App %d, cho STM32 xoa Flash)... ", appId);
    uint8_t otaPayload[1] = { appId };
    if (!_can.sendWithAck(CAN_ID_START_OTA, otaPayload, 1, CAN_ID_RESPONSE, OTA_ACK, 5000, 3)) {
        _lastError = "STM32 khong phan hoi ACK khi gui START_OTA (Loi xoa Flash)!";
        Serial.println("\n[OTA ERR] " + _lastError);
        return false;
    }
    Serial.println("OK (Da xoa Flash)");

    // Pha 1.2: Khai báo File (Total Size, Global CRC)
    Serial.printf("[OTA] [2/4] Gui START_FILE (Size=%uB, CRC=0x%04X)... ",
                  (unsigned int)fileSize, globalCrc);
    uint8_t filePayload[6] = {
        (uint8_t)((fileSize >> 24) & 0xFF),
        (uint8_t)((fileSize >> 16) & 0xFF),
        (uint8_t)((fileSize >> 8)  & 0xFF),
        (uint8_t)(fileSize & 0xFF),
        (uint8_t)((globalCrc >> 8) & 0xFF),
        (uint8_t)(globalCrc & 0xFF)
    };
    if (!_can.sendWithAck(CAN_ID_START_FILE, filePayload, 6, CAN_ID_RESPONSE, OTA_ACK, 1500, 3)) {
        _lastError = "STM32 khong phan hoi ACK START_FILE!";
        Serial.println("\n[OTA ERR] " + _lastError);
        return false;
    }
    Serial.println("OK");

    // Pha 2: Đọc và truyền từng Chunk
    Serial.println("[OTA] [3/4] Dang truyen du lieu cac Chunks...");
    uint8_t chunkBuffer[CHUNK_SIZE];
    size_t totalSent = 0;
    uint16_t chunkIndex = 0;

    while (totalSent < fileSize) {
        size_t toRead = (fileSize - totalSent >= CHUNK_SIZE) ? CHUNK_SIZE : (fileSize - totalSent);
        size_t bytesRead = stream.readBytes((char*)chunkBuffer, toRead);
        if (bytesRead == 0) break;

        chunkIndex++;
        if (!sendChunk(chunkBuffer, bytesRead, chunkIndex)) {
            _lastError = "Truyen Chunk #" + String(chunkIndex) + " that bai sau " + String(MAX_CHUNK_RETRIES) + " lan thu!";
            Serial.printf("\n[OTA ERR] %s Huy OTA.\n", _lastError.c_str());
            return false;
        }

        totalSent += bytesRead;
        int progress = (totalSent * 100) / fileSize;
        Serial.printf("  -> Chunk #%u: %uB [%d%%] (%u/%uB)\n",
                      chunkIndex, (unsigned int)bytesRead, progress,
                      (unsigned int)totalSent, (unsigned int)fileSize);
    }

    // Pha 4: Kết thúc File & Verify
    Serial.print("[OTA] [4/4] Gui END_FILE & cho STM32 verify Global CRC16... ");
    uint8_t endFilePayload[2] = {
        (uint8_t)((globalCrc >> 8) & 0xFF),
        (uint8_t)(globalCrc & 0xFF)
    };
    if (!_can.sendWithAck(CAN_ID_END_FILE, endFilePayload, 2, CAN_ID_RESPONSE, OTA_ACK, 3000, 3)) {
        _lastError = "STM32 bao loi Global CRC / Vector Table (Rollback da kich hoat)!";
        Serial.println("\n[OTA ERR] " + _lastError);
        return false;
    }
    Serial.println("OK (STM32 da doi Option Bytes & Jump sang App moi!)");

    Serial.println("==============================================");
    Serial.printf("  --> TRUYEN OTA APP %d THANH CONG 100%%! <--\n", appId);
    Serial.println("==============================================\n");
    _lastError = "";
    return true;
}

