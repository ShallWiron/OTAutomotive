#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include "stm32f10x.h"

/* Hardware I2C Configuration (PB6-SCL, PB7-SDA) */
#define OLED_I2C_GPIO_CLK        RCC_APB2Periph_GPIOB
#define OLED_I2C_PORT            GPIOB
#define OLED_SCL_PIN             GPIO_Pin_6
#define OLED_SDA_PIN             GPIO_Pin_7

#define OLED_I2C_ADDR            0x78U

#define OLED_WIDTH               128U
#define OLED_HEIGHT              64U
#define OLED_BUFFER_SIZE         (OLED_WIDTH * OLED_HEIGHT / 8U)

#define OLED_COLOR_BLACK         0U
#define OLED_COLOR_WHITE         1U
#define OLED_COLOR_INVERT        2U

/* Basic Functions */
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Update(void);
void OLED_SetBrightness(uint8_t contrast);
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, uint8_t fill);
void OLED_DrawCircle(uint8_t xc, uint8_t yc, uint8_t r, uint8_t color, uint8_t fill);
void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap, uint8_t color);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t color);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t color);

/* High-level Dashboard & System Screens */
void OLED_ShowState(uint8_t mode, uint16_t cmdId, uint8_t hornActive);
void OLED_ShowWaitingWiFi(void);
void OLED_ShowConnectedSuccess(void);
void OLED_ShowOTAFlashing(void);

#endif /* OLED_H */
