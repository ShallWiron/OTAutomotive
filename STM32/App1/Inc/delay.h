#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>
#include "stm32f10x.h"

/* Biến đếm thời gian hệ thống (tăng mỗi 1ms bởi SysTick_Handler) */
extern volatile uint32_t g_ms;

/**
 * @brief  Khởi tạo bộ đếm SysTick 1ms
 */
void Delay_Init(void);

/**
 * @brief  Hàm tạo trễ mili-giây (đồng thời giữ polling OTA CAN liên tục)
 * @param  ms: Số mili-giây cần delay
 */
void Delay_Ms(uint32_t ms);

/**
 * @brief  Lấy giá trị tick hệ thống (ms)
 * @retval Số ms tính từ lúc khởi động
 */
uint32_t Delay_GetTick(void);

#endif /* DELAY_H */

