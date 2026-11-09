#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "stm32f10x.h"

/*
 * CẤU HÌNH LED ONBOARD TRÊN CHÂN PC13 (Active-Low)
 */
#define LED_PORT    GPIOC
#define LED_PIN     GPIO_Pin_13
#define LED_CLK     RCC_APB2Periph_GPIOC

void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif /* LED_H */
