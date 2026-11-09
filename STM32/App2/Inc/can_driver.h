#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdint.h>
#include "stm32f10x.h"

#define CAN_RX_PIN          GPIO_Pin_11
#define CAN_TX_PIN          GPIO_Pin_12
#define CAN_GPIO_PORT       GPIOA
#define CAN_GPIO_CLK        RCC_APB2Periph_GPIOA

void CAN_Driver_Init(void);
uint8_t CAN_Driver_Receive(CanRxMsg *rxMsg);

#endif
