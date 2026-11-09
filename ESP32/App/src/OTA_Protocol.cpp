#include "OTA_Protocol.h"
#include "CRC16.h"

OTA_Protocol::OTA_Protocol(CAN_Driver& can_driver)
    : _can(can_driver), _injectCrcError(false), _lastError("") {}

uint8_t OTA_Protocol::queryCurrentApp(uint32_t timeout_ms) {
    _can.clearRxQueue();
    const uint8_t queryPayload[1] = { 0x55 };
    if (!_can.sendFrame(CAN_ID_QUERY_APP, queryPayload, 1)) {
        return 0;
    }

    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        twai_message_t rx;
        if (_can.receiveFrame(rx, 10)) {
            if (rx.identifier == CAN_ID_RESPONSE && rx.data_length_code >= 2) {
                if (rx.data[0] == OTA_ACK) {
                    return rx.data[1];
                }
            }
        }
    }
    return 0;
}

bool OTA_Protocol::triggerBootloader() {
    const uint8_t magicKey[4] = { 0xDE, 0xAD, 0xBE, 0xEF };

    _can.clearRxQueue();
    if (_can.sendWithAck(CAN_ID_REQ_BOOT, magicKey, 4, CAN_ID_RESPONSE, OTA_ACK, 500, 2)) {
        delay(200);
        return true;
    }

    return false;
}

bool OTA_Protocol::burstChunkData(const uint8_t* chunkBuf, size_t chunkLen) {
    size_t sent = 0;
    uint8_t packet_idx = 0;

    while (sent < chunkLen) {
        uint8_t len = (chunkLen - sent) >= 8 ? 8 : (chunkLen - sent);
        uint32_t frame_id = CAN_ID_DATA_CHUNK | (packet_idx & 0x3FU);

        if (!_can.sendFrame(frame_id, &chunkBuf[sent], len)) {
            return false;
        }

        sent += len;
        packet_idx++;

        uint32_t wait_start = millis();
        while (millis() - wait_start < 3) {
            twai_message_t rx;
            if (_can.receiveFrame(rx, 0)) {
                if (rx.identifier == CAN_ID_RESPONSE && rx.data_length_code > 0 && rx.data[0] == OTA_NACK) {
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

        uint8_t startPayload[2] = {
            (uint8_t)((chunkLen >> 8) & 0xFF),
            (uint8_t)(chunkLen & 0xFF)
        };
        if (!_can.sendFrame(CAN_ID_START_CHUNK, startPayload, 2)) {
            delay(30);
            continue;
        }
        delay(5);

        if (!burstChunkData(chunkBuf, chunkLen)) {
            delay(30);
            continue;
        }

        uint16_t crcToSend = chunkCrc;
        if (_injectCrcError && chunkIndex == 2 && attempt == 1) {
            crcToSend ^= 0xFFFFU;
        }

        uint8_t endPayload[2] = {
            (uint8_t)((crcToSend >> 8) & 0xFF),
            (uint8_t)(crcToSend & 0xFF)
        };
        if (!_can.sendFrame(CAN_ID_END_CHUNK, endPayload, 2)) {
            delay(30);
            continue;
        }

        if (_can.waitResponse(CAN_ID_RESPONSE, OTA_ACK, 3000)) {
            return true;
        }

        delay(50);
    }

    return false;
}

bool OTA_Protocol::executeOtaSequence(size_t fileSize, uint16_t globalCrc, uint8_t appId,
                                      OTA_ProgressCallback cb,
                                      const std::function<size_t(uint8_t*, size_t)>& readChunkFn) {
    if (fileSize == 0 || fileSize > 56 * 1024) {
        _lastError = "Invalid firmware binary size (" + String(fileSize) + " bytes, max 56KB)";
        return false;
    }

    _lastError = "";

    if (cb) cb(2, "Connecting Bootloader (CAN 0x600)...");
    triggerBootloader();

    if (cb) cb(10, "Erasing target Flash sector App " + String(appId) + " (56 KB)...");
    uint8_t otaPayload[1] = { appId };
    if (!_can.sendWithAck(CAN_ID_START_OTA, otaPayload, 1, CAN_ID_RESPONSE, OTA_ACK, 5000, 3)) {
        _lastError = "STM32 flash erase timeout or refused";
        return false;
    }

    if (cb) cb(18, "Registering image: " + String(fileSize) + " B, CRC: 0x" + String(globalCrc, HEX));
    uint8_t filePayload[6] = {
        (uint8_t)((fileSize >> 24) & 0xFF),
        (uint8_t)((fileSize >> 16) & 0xFF),
        (uint8_t)((fileSize >> 8)  & 0xFF),
        (uint8_t)(fileSize & 0xFF),
        (uint8_t)((globalCrc >> 8) & 0xFF),
        (uint8_t)(globalCrc & 0xFF)
    };
    if (!_can.sendWithAck(CAN_ID_START_FILE, filePayload, 6, CAN_ID_RESPONSE, OTA_ACK, 1500, 3)) {
        _lastError = "STM32 rejected START_FILE parameters";
        return false;
    }

    uint8_t chunkBuffer[CHUNK_SIZE];
    size_t totalSent = 0;
    uint16_t chunkIndex = 0;
    size_t totalChunks = (fileSize + CHUNK_SIZE - 1) / CHUNK_SIZE;

    while (totalSent < fileSize) {
        size_t toRead = (fileSize - totalSent >= CHUNK_SIZE) ? CHUNK_SIZE : (fileSize - totalSent);
        size_t bytesRead = readChunkFn(chunkBuffer, toRead);
        if (bytesRead == 0) break;

        chunkIndex++;
        if (!sendChunk(chunkBuffer, bytesRead, chunkIndex)) {
            _lastError = "Transmission failed at Chunk #" + String(chunkIndex) + " after retries";
            return false;
        }

        totalSent += bytesRead;
        int pct = 18 + (int)((totalSent * 74) / fileSize);
        if (cb) {
            cb(pct, "Transmitting Chunk #" + String(chunkIndex) + "/" + String(totalChunks) + " (" + String(totalSent) + "/" + String(fileSize) + " B)");
        }
    }

    if (cb) cb(95, "Validating Global CRC16 and swapping Option Bytes...");
    uint8_t endFilePayload[2] = {
        (uint8_t)((globalCrc >> 8) & 0xFF),
        (uint8_t)(globalCrc & 0xFF)
    };
    if (!_can.sendWithAck(CAN_ID_END_FILE, endFilePayload, 2, CAN_ID_RESPONSE, OTA_ACK, 3000, 3)) {
        _lastError = "STM32 Global CRC check failed or rejected. Rollback triggered.";
        return false;
    }

    if (cb) cb(100, "OTA UPDATE SUCCESSFUL! Swapped to App " + String(appId));
    _lastError = "";
    return true;
}

bool OTA_Protocol::flashFromBuffer(const uint8_t* buffer, size_t fileSize, uint8_t appId, OTA_ProgressCallback cb) {
    if (buffer == nullptr || fileSize == 0) {
        _lastError = "RAM firmware buffer is empty";
        return false;
    }
    if (fileSize > 56 * 1024) {
        _lastError = "Firmware exceeds max partition limit (56KB)";
        return false;
    }

    uint16_t globalCrc = crc16_calculate(buffer, fileSize);
    size_t offset = 0;

    return executeOtaSequence(fileSize, globalCrc, appId, cb, [&](uint8_t* dst, size_t len) {
        size_t toCopy = (offset + len <= fileSize) ? len : (fileSize - offset);
        if (toCopy > 0) {
            memcpy(dst, &buffer[offset], toCopy);
            offset += toCopy;
        }
        return toCopy;
    });
}

bool OTA_Protocol::flashFromFile(File& file, uint8_t appId, OTA_ProgressCallback cb) {
    if (!file) {
        _lastError = "Unable to open firmware file";
        return false;
    }

    size_t totalSize = file.size();
    if (totalSize == 0 || totalSize > 56 * 1024) {
        _lastError = "Invalid file size (" + String(totalSize) + " bytes, max 56KB)";
        return false;
    }

    uint16_t globalCrc = 0xFFFFU;
    uint8_t buffer[CHUNK_SIZE];
    file.seek(0);
    while (file.available()) {
        size_t n = file.read(buffer, sizeof(buffer));
        if (n == 0) break;
        globalCrc = crc16_update(globalCrc, buffer, n);
    }
    file.seek(0);

    return executeOtaSequence(totalSize, globalCrc, appId, cb, [&](uint8_t* dst, size_t len) {
        return file.readBytes((char*)dst, len);
    });
}
