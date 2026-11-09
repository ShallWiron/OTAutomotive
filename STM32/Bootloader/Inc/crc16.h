#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

uint16_t CRC16_Modbus(const uint8_t *buf, uint32_t len);

#endif
