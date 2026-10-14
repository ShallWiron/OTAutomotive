#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "stm32f10x.h"                  // Device header

void UART_Setup(void);
void UART_SendChar(char c);
void UART_SendString(const char *str);
void UART_SendHex8(uint8_t val);
void UART_SendHex16(uint16_t val);
void UART_SendHex32(uint32_t val);
void UART_SendDec(uint32_t val);
void UART_SendHexArray8(const uint8_t *data, uint16_t length);
void UART_WaitTxComplete(void);

#endif