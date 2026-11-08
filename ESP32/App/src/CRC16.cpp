#include "CRC16.h"

uint16_t crc16_update(uint16_t crc, const uint8_t* data, size_t len) {
    if (!data) return crc;
    for (size_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)data[pos];
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x0001U) {
                crc = (crc >> 1) ^ 0xA001U;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

uint16_t crc16_calculate(const uint8_t* data, size_t len) {
    return crc16_update(0xFFFFU, data, len);
}

