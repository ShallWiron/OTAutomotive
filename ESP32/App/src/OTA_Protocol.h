#ifndef OTA_PROTOCOL_H
#define OTA_PROTOCOL_H

#include <Arduino.h>
#include <FS.h>
#include <functional>
#include "CAN_Driver.h"
#include "config.h"

typedef std::function<void(int percent, const String& stepDesc)> OTA_ProgressCallback;

class OTA_Protocol {
public:
    explicit OTA_Protocol(CAN_Driver& can_driver);

    uint8_t queryCurrentApp(uint32_t timeout_ms = 500);
    bool triggerBootloader();

    bool flashFromBuffer(const uint8_t* buffer, size_t fileSize, uint8_t appId, OTA_ProgressCallback cb = nullptr);
    bool flashFromFile(File& file, uint8_t appId, OTA_ProgressCallback cb = nullptr);

    void setInjectCrcError(bool enable) { _injectCrcError = enable; }
    bool isInjectCrcError() const { return _injectCrcError; }

    String getLastError() const { return _lastError; }
    void setLastError(const String& err) { _lastError = err; }

private:
    CAN_Driver& _can;
    bool _injectCrcError;
    String _lastError;

    bool executeOtaSequence(size_t fileSize, uint16_t globalCrc, uint8_t appId,
                            OTA_ProgressCallback cb,
                            const std::function<size_t(uint8_t*, size_t)>& readChunkFn);

    bool sendChunk(const uint8_t* chunkBuf, size_t chunkLen, uint16_t chunkIndex);
    bool burstChunkData(const uint8_t* chunkBuf, size_t chunkLen);
};

#endif
