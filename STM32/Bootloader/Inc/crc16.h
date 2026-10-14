#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

/**
 * @brief  Tính CRC-16 Modbus trên mảng dữ liệu
 * @param  buf  Con trỏ tới buffer dữ liệu cần tính CRC
 * @param  len  Số byte trong buffer (uint32_t)
 * @retval Giá trị CRC-16 (polynomial 0xA001, init 0xFFFF)
 */
uint16_t CRC16_Modbus(const uint8_t *buf, uint32_t len);

#endif /* CRC16_H */
