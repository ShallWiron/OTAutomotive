#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Cập nhật mã CRC16-Modbus (Poly 0xA001, Init 0xFFFF)
 */
uint16_t crc16_update(uint16_t crc, const uint8_t* data, size_t len);

/**
 * @brief Tính mã CRC16-Modbus cho toàn bộ buffer
 */
uint16_t crc16_calculate(const uint8_t* data, size_t len);

#endif // CRC16_H

