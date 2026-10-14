#ifndef DELAY_TIM2_H
#define DELAY_TIM2_H

#include <stdint.h>
#include "stm32f10x.h"                  // Device header

void Delay1Ms(void);
void Delay_Ms(uint32_t u32DelayInMs);
void Delay_Init();

#endif