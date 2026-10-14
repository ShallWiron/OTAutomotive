#include "uart.h"
#include "stm32f10x.h"                  // Device header
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void UART_SendChar(char c) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint16_t)c);
}

void UART_SendString(const char *str) {
    while (*str) {
        UART_SendChar(*str++);
    }
}

void UART_SendHex8(uint8_t val) {
    const char hex[] = "0123456789ABCDEF";
    UART_SendChar(hex[(val >> 4) & 0x0F]);
    UART_SendChar(hex[val & 0x0F]);
}

void UART_SendHex16(uint16_t val) {
    UART_SendHex8((uint8_t)((val >> 8) & 0xFF));
    UART_SendHex8((uint8_t)(val & 0xFF));
}

void UART_SendHex32(uint32_t val) {
    UART_SendHex16((uint16_t)((val >> 16) & 0xFFFF));
    UART_SendHex16((uint16_t)(val & 0xFFFF));
}

void UART_SendDec(uint32_t val) {
    char buf[12];
    int i = 0;

    if (val == 0) {
        UART_SendChar('0');
        return;
    }

    while (val > 0 && i < (int)sizeof(buf)) {
        buf[i++] = (char)('0' + (val % 10));
        val /= 10;
    }

    while (i > 0) {
        UART_SendChar(buf[--i]);
    }
}

void UART_SendHexArray8(const uint8_t *data, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        UART_SendHex8(data[i]);
        UART_SendChar(' ');

        if ((i + 1) % 8 == 0) {
            UART_SendChar('\r');
            UART_SendChar('\n');
        }
    }

    if (length % 8 != 0) {
        UART_SendChar('\r');
        UART_SendChar('\n');
    }
}

void UART_WaitTxComplete(void) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

void UART_Setup(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    GPIO_InitTypeDef GPIO_Structure;
    GPIO_Structure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Structure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_Structure);

    GPIO_Structure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_Structure);

    USART_InitTypeDef USART_Structure;
    USART_Structure.USART_BaudRate = 115200;
    USART_Structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Structure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Structure.USART_Parity = USART_Parity_No;
    USART_Structure.USART_StopBits = USART_StopBits_1;
    USART_Structure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_Structure);
    USART_Cmd(USART1, ENABLE);
}
