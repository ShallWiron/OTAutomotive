#ifndef OTA_PROTOCOL_H
#define OTA_PROTOCOL_H

#include <Arduino.h>
#include <FS.h>
#include "CAN_Driver.h"
#include "config.h"

/**
 * @brief Lớp Middleware quản lý giao thức cập nhật CAN OTA xuống STM32
 */
class OTA_Protocol {
public:
    explicit OTA_Protocol(CAN_Driver& can_driver);

    /**
     * @brief Kích hoạt STM32 tự Soft-Reset chuyển sang Bootloader qua mạng CAN
     */
    bool triggerBootloader();

    /**
     * @brief Nạp firmware từ File (Thẻ nhớ SD, LittleFS, SPIFFS...) xuống STM32
     * @param file   File firmware (.bin) đã mở
     * @param appId  Mã phân vùng đích (0x01 = App 1, 0x02 = App 2)
     */
    bool flashFromFile(File& file, uint8_t appId);

    /**
     * @brief Nạp firmware từ Stream (phù hợp cả cho Web Server upload trực tiếp)
     * @param stream     Luồng dữ liệu nhị phân
     * @param fileSize   Tổng kích thước file (bytes)
     * @param globalCrc  Mã Global CRC16 của toàn file
     * @param appId      Mã phân vùng đích (0x01 hoặc 0x02)
     */
    bool flashFromStream(Stream& stream, size_t fileSize, uint16_t globalCrc, uint8_t appId);

    /**
     * @brief Truy vấn phân vùng App đang chạy từ STM32 qua CAN Bus
     * @param timeout_ms Thời gian chờ phản hồi (ms)
     * @return 1 = App 1, 2 = App 2, 0 = Không phản hồi (Timeout)
     */
    uint8_t queryCurrentApp(uint32_t timeout_ms = 300);

    // Chế độ Blackbox Test: Giả lập gửi sai CRC tại Chunk #2
    void setInjectCrcError(bool enable) { _injectCrcError = enable; }
    bool isInjectCrcError() const { return _injectCrcError; }

    String getLastError() const { return _lastError; }
    void setLastError(const String& err) { _lastError = err; }

private:
    CAN_Driver& _can;
    bool _injectCrcError;
    String _lastError;

    /**
     * @brief Gửi một khối Chunk (tối đa 256 bytes) kèm cơ chế Retry tự động
     */
    bool sendChunk(const uint8_t* chunkBuf, size_t chunkLen, uint16_t chunkIndex);

    /**
     * @brief Phát chuỗi frame dữ liệu của Chunk (8 bytes/frame) kèm check NACK sớm
     */
    bool burstChunkData(const uint8_t* chunkBuf, size_t chunkLen);
};

#endif // OTA_PROTOCOL_H

