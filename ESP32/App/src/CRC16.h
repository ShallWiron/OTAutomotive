#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stddef.h>

uint16_t crc16_update(uint16_t crc, const uint8_t* data, size_t len);
uint16_t crc16_calculate(const uint8_t* data, size_t len);

#endif
