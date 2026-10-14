#include "crc16.h"

uint16_t CRC16_Modbus(const uint8_t *buf, uint32_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint32_t pos = 0; pos < len; pos++)
    {
        crc ^= (uint16_t)buf[pos];

        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}
