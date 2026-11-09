#include "oled.h"
#include "bitmap.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <string.h>

static uint8_t s_OLED_Buffer[OLED_BUFFER_SIZE];
static uint8_t s_heartbeat = 0;

/* Software I2C Low Level */
#define I2C_SCL_HIGH()   GPIO_SetBits(OLED_I2C_PORT, OLED_SCL_PIN)
#define I2C_SCL_LOW()    GPIO_ResetBits(OLED_I2C_PORT, OLED_SCL_PIN)
#define I2C_SDA_HIGH()   GPIO_SetBits(OLED_I2C_PORT, OLED_SDA_PIN)
#define I2C_SDA_LOW()    GPIO_ResetBits(OLED_I2C_PORT, OLED_SDA_PIN)

static inline void I2C_Delay(void)
{
    for (volatile uint32_t i = 0; i < 6U; i++) {
        __NOP();
    }
}

static void I2C_Start(void)
{
    I2C_SDA_HIGH();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_LOW();
    I2C_Delay();
    I2C_SCL_LOW();
    I2C_Delay();
}

static void I2C_Stop(void)
{
    I2C_SDA_LOW();
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_HIGH();
    I2C_Delay();
}

static void I2C_WriteByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8U; i++) {
        if (byte & 0x80U) {
            I2C_SDA_HIGH();
        } else {
            I2C_SDA_LOW();
        }
        I2C_Delay();
        I2C_SCL_HIGH();
        I2C_Delay();
        I2C_SCL_LOW();
        byte <<= 1U;
    }
    I2C_SDA_HIGH();
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SCL_LOW();
    I2C_Delay();
}

static void OLED_WriteCommand(uint8_t cmd)
{
    I2C_Start();
    I2C_WriteByte(OLED_I2C_ADDR);
    I2C_WriteByte(0x00U);
    I2C_WriteByte(cmd);
    I2C_Stop();
}

void OLED_Init(void)
{
    GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(OLED_I2C_GPIO_CLK, ENABLE);

    gpioInit.GPIO_Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(OLED_I2C_PORT, &gpioInit);

    I2C_SCL_HIGH();
    I2C_SDA_HIGH();

    for (volatile uint32_t i = 0; i < 100000U; i++) {
        __NOP();
    }

    static const uint8_t initCommands[] = {
        0xAEU, 0xD5U, 0x80U, 0xA8U, 0x3FU, 0xD3U, 0x00U, 0x40U,
        0x8DU, 0x14U, 0x20U, 0x00U, 0xA1U, 0xC8U, 0xDAU, 0x12U,
        0x81U, 0xCFU, 0xD9U, 0xF1U, 0xDBU, 0x40U, 0xA4U, 0xA6U,
        0x2EU, 0xAFU
    };

    for (uint32_t i = 0; i < sizeof(initCommands); i++) {
        OLED_WriteCommand(initCommands[i]);
    }

    OLED_Clear();
    OLED_Update();
}

void OLED_Clear(void)
{
    memset(s_OLED_Buffer, 0, sizeof(s_OLED_Buffer));
}

void OLED_SetBrightness(uint8_t contrast)
{
    OLED_WriteCommand(0x81U);
    OLED_WriteCommand(contrast);
}

void OLED_Update(void)
{
    OLED_WriteCommand(0x21U);
    OLED_WriteCommand(0U);
    OLED_WriteCommand(OLED_WIDTH - 1U);

    OLED_WriteCommand(0x22U);
    OLED_WriteCommand(0U);
    OLED_WriteCommand((OLED_HEIGHT / 8U) - 1U);

    I2C_Start();
    I2C_WriteByte(OLED_I2C_ADDR);
    I2C_WriteByte(0x40U);

    for (uint16_t i = 0; i < OLED_BUFFER_SIZE; i++) {
        I2C_WriteByte(s_OLED_Buffer[i]);
    }
    I2C_Stop();
}

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if ((x >= OLED_WIDTH) || (y >= OLED_HEIGHT)) return;

    uint16_t byteIndex = (uint16_t)x + ((uint16_t)(y / 8U) * OLED_WIDTH);
    uint8_t bitMask = (uint8_t)(1U << (y % 8U));

    if (color == OLED_COLOR_WHITE) {
        s_OLED_Buffer[byteIndex] |= bitMask;
    } else if (color == OLED_COLOR_BLACK) {
        s_OLED_Buffer[byteIndex] &= (uint8_t)(~bitMask);
    } else if (color == OLED_COLOR_INVERT) {
        s_OLED_Buffer[byteIndex] ^= bitMask;
    }
}

void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color)
{
    int16_t dx = (int16_t)(x1 > x0 ? x1 - x0 : x0 - x1);
    int16_t dy = (int16_t)(y1 > y0 ? y1 - y0 : y0 - y1);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        OLED_DrawPixel(x0, y0, color);
        if ((x0 == x1) && (y0 == y1)) break;
        int16_t e2 = (int16_t)(2 * err);
        if (e2 > -dy) {
            err -= dy;
            x0 = (uint8_t)(x0 + sx);
        }
        if (e2 < dx) {
            err += dx;
            y0 = (uint8_t)(y0 + sy);
        }
    }
}

void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, uint8_t fill)
{
    if (fill) {
        for (uint8_t i = 0; i < h; i++) {
            OLED_DrawLine(x, y + i, x + w - 1U, y + i, color);
        }
    } else {
        OLED_DrawLine(x, y, x + w - 1U, y, color);
        OLED_DrawLine(x, y + h - 1U, x + w - 1U, y + h - 1U, color);
        OLED_DrawLine(x, y, x, y + h - 1U, color);
        OLED_DrawLine(x + w - 1U, y, x + w - 1U, y + h - 1U, color);
    }
}

void OLED_DrawCircle(uint8_t xc, uint8_t yc, uint8_t r, uint8_t color, uint8_t fill)
{
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r;

    while (y >= x) {
        if (fill) {
            OLED_DrawLine((uint8_t)(xc - x), (uint8_t)(yc - y), (uint8_t)(xc + x), (uint8_t)(yc - y), color);
            OLED_DrawLine((uint8_t)(xc - x), (uint8_t)(yc + y), (uint8_t)(xc + x), (uint8_t)(yc + y), color);
            OLED_DrawLine((uint8_t)(xc - y), (uint8_t)(yc - x), (uint8_t)(xc + y), (uint8_t)(yc - x), color);
            OLED_DrawLine((uint8_t)(xc - y), (uint8_t)(yc + x), (uint8_t)(xc + y), (uint8_t)(yc + x), color);
        } else {
            OLED_DrawPixel((uint8_t)(xc + x), (uint8_t)(yc + y), color);
            OLED_DrawPixel((uint8_t)(xc - x), (uint8_t)(yc + y), color);
            OLED_DrawPixel((uint8_t)(xc + x), (uint8_t)(yc - y), color);
            OLED_DrawPixel((uint8_t)(xc - x), (uint8_t)(yc - y), color);
            OLED_DrawPixel((uint8_t)(xc + y), (uint8_t)(yc + x), color);
            OLED_DrawPixel((uint8_t)(xc - y), (uint8_t)(yc + x), color);
            OLED_DrawPixel((uint8_t)(xc + y), (uint8_t)(yc - x), color);
            OLED_DrawPixel((uint8_t)(xc - y), (uint8_t)(yc - x), color);
        }
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap, uint8_t color)
{
    for (uint8_t j = 0; j < h; j++) {
        for (uint8_t i = 0; i < w; i++) {
            uint16_t byteIdx = (uint16_t)(i + ((j / 8U) * w));
            uint8_t bitIdx = j % 8U;
            if (bitmap[byteIdx] & (1U << bitIdx)) {
                OLED_DrawPixel(x + i, y + j, color);
            }
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t color)
{
    if ((ch < 32) || (ch > 126)) ch = ' ';
    uint8_t charIndex = (uint8_t)(ch - 32);

    for (uint8_t col = 0; col < 6U; col++) {
        uint8_t line = Font_6x8[charIndex][col];
        for (uint8_t row = 0; row < 8U; row++) {
            if (line & 0x01U) {
                OLED_DrawPixel(x + col, y + row, color);
            }
            line >>= 1U;
        }
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t color)
{
    while (*str) {
        if (x + 6U > OLED_WIDTH) {
            x = 0;
            y += 8U;
        }
        if (y + 8U > OLED_HEIGHT) break;
        OLED_ShowChar(x, y, *str, color);
        x += 6U;
        str++;
    }
}

static void FormatHex3(uint16_t val, char *buf)
{
    const char hex[] = "0123456789ABCDEF";
    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = hex[(val >> 8) & 0x0F];
    buf[3] = hex[(val >> 4) & 0x0F];
    buf[4] = hex[val & 0x0F];
    buf[5] = '\0';
}

void OLED_ShowState(uint8_t mode, uint16_t cmdId, uint8_t hornActive)
{
    const uint8_t *icon = Bitmap_Stop_32x32;
    const char *title = "STOP";
    const char *sub = "MOTORS IDLE";

    s_heartbeat = !s_heartbeat;

    /* Mode 0: Car Navigation */
    if (mode == 0)
    {
        switch (cmdId)
        {
            case 0x000: /* CMD_EMERGENCY_STOP */
                icon = Bitmap_EStop_32x32;
                title = "EMERGENCY";
                sub = "MOTORS HALTED";
                break;
            case 0x040: /* CMD_MOTOR_STOP */
                icon = Bitmap_Stop_32x32;
                title = "STOP";
                sub = "MOTORS IDLE";
                break;
            case 0x041: /* CMD_MOTOR_FORWARD */
                icon = Bitmap_Forward_32x32;
                title = "FORWARD";
                sub = "RUNNING AHEAD";
                break;
            case 0x042: /* CMD_MOTOR_BACKWARD */
                icon = Bitmap_Backward_32x32;
                title = "REVERSE";
                sub = "RUNNING BACK";
                break;
            case 0x043: /* CMD_MOTOR_TURN_LEFT */
                icon = Bitmap_TurnLeft_32x32;
                title = "TURN LEFT";
                sub = "PIVOT LEFT";
                break;
            case 0x044: /* CMD_MOTOR_TURN_RIGHT */
                icon = Bitmap_TurnRight_32x32;
                title = "TURN RIGHT";
                sub = "PIVOT RIGHT";
                break;
            case 0x045: /* CMD_MOTOR_LEFT_FWD (NE) */
                icon = Bitmap_NE_32x32;
                title = "FWD RIGHT";
                sub = "DIAGONAL NE";
                break;
            case 0x046: /* CMD_MOTOR_LEFT_REV (SW) */
                icon = Bitmap_SW_32x32;
                title = "REV LEFT";
                sub = "DIAGONAL SW";
                break;
            case 0x047: /* CMD_MOTOR_RIGHT_FWD (NW) */
                icon = Bitmap_NW_32x32;
                title = "FWD LEFT";
                sub = "DIAGONAL NW";
                break;
            case 0x048: /* CMD_MOTOR_RIGHT_REV (SE) */
                icon = Bitmap_SE_32x32;
                title = "REV RIGHT";
                sub = "DIAGONAL SE";
                break;
            default:
                break;
        }
    }
    /* Mode 1: Robotic Arm / Crane */
    else
    {
        switch (cmdId)
        {
            case 0x000: /* CMD_EMERGENCY_STOP */
                icon = Bitmap_EStop_32x32;
                title = "EMERGENCY";
                sub = "CRANE HALTED";
                break;
            case 0x040:
            case 0x080: /* CMD_SERVO_STOP */
                icon = Bitmap_ArmHold_32x32;
                title = "ARM HOLD";
                sub = "SERVO READY";
                break;
            case 0x041:
            case 0x081: /* CMD_SERVO_ARM_UP */
                icon = Bitmap_ArmUp_32x32;
                title = "ARM UP";
                sub = "LIFTING UP";
                break;
            case 0x042:
            case 0x082: /* CMD_SERVO_ARM_DOWN */
                icon = Bitmap_ArmDown_32x32;
                title = "ARM DOWN";
                sub = "LOWERING";
                break;
            case 0x043:
            case 0x083: /* CMD_SERVO_ARM_EXTEND */
                icon = Bitmap_ArmExtend_32x32;
                title = "ARM EXTEND";
                sub = "REACHING OUT";
                break;
            case 0x044:
            case 0x084: /* CMD_SERVO_ARM_RETRACT */
                icon = Bitmap_ArmRetract_32x32;
                title = "ARM RETRACT";
                sub = "FOLDING IN";
                break;
            default:
                icon = Bitmap_ArmHold_32x32;
                title = "ARM READY";
                sub = "CRANE ACTIVE";
                break;
        }
    }

    OLED_Clear();

    /* 1. Header (Y: 0..11) */
    if (mode == 0) {
        OLED_ShowString(2, 2, "[CAR]", OLED_COLOR_WHITE);
    } else {
        OLED_ShowString(2, 2, "[CRANE]", OLED_COLOR_WHITE);
    }

    if (hornActive) {
        OLED_DrawBitmap(88, 2, 16, 8, Bitmap_HornActive_16x8, OLED_COLOR_WHITE);
    } else {
        OLED_DrawBitmap(88, 2, 16, 8, Bitmap_HornIdle_16x8, OLED_COLOR_WHITE);
    }

    OLED_DrawCircle(122, 5, 2, OLED_COLOR_WHITE, s_heartbeat ? 1 : 0);
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_WHITE);

    /* 2. Center Body (Y: 13..49) */
    OLED_DrawBitmap(8, 15, 32, 32, icon, OLED_COLOR_WHITE);
    OLED_ShowString(46, 20, title, OLED_COLOR_WHITE);
    OLED_ShowString(46, 33, sub, OLED_COLOR_WHITE);

    /* 3. Footer (Y: 50..63) */
    OLED_DrawLine(0, 50, 127, 50, OLED_COLOR_WHITE);

    char hexBuf[6];
    FormatHex3(cmdId, hexBuf);
    char footerBuf[24];
    /* Build: "APP2 | CAN: 0x... | 500K" */
    footerBuf[0] = 'A'; footerBuf[1] = 'P'; footerBuf[2] = 'P'; footerBuf[3] = '2';
    footerBuf[4] = ' '; footerBuf[5] = '|'; footerBuf[6] = ' ';
    footerBuf[7] = hexBuf[0]; footerBuf[8] = hexBuf[1]; footerBuf[9] = hexBuf[2];
    footerBuf[10] = hexBuf[3]; footerBuf[11] = hexBuf[4];
    footerBuf[12] = ' '; footerBuf[13] = '|'; footerBuf[14] = ' ';
    footerBuf[15] = '5'; footerBuf[16] = '0'; footerBuf[17] = '0'; footerBuf[18] = 'K';
    footerBuf[19] = '\0';

    OLED_ShowString(6, 53, footerBuf, OLED_COLOR_WHITE);

    OLED_Update();
}

void OLED_ShowWaitingWiFi(void)
{
    OLED_Clear();
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_WHITE);
    OLED_ShowString(22, 2, "== OTAutomotive ==", OLED_COLOR_WHITE);

    OLED_DrawBitmap(8, 15, 32, 32, Bitmap_WaitingWiFi_32x32, OLED_COLOR_WHITE);
    OLED_ShowString(46, 20, "CONNECTING...", OLED_COLOR_WHITE);
    OLED_ShowString(46, 33, "WIFI AP / STA", OLED_COLOR_WHITE);

    OLED_DrawLine(0, 50, 127, 50, OLED_COLOR_WHITE);
    OLED_ShowString(14, 53, "APP2 | CAN: 500K", OLED_COLOR_WHITE);
    OLED_Update();
}

void OLED_ShowConnectedSuccess(void)
{
    OLED_Clear();
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_WHITE);
    OLED_ShowString(22, 2, "== OTAutomotive ==", OLED_COLOR_WHITE);

    OLED_DrawBitmap(8, 15, 32, 32, Bitmap_WifiConnected_32x32, OLED_COLOR_WHITE);
    OLED_ShowString(46, 20, "ONLINE", OLED_COLOR_WHITE);
    OLED_ShowString(46, 33, "WIFI READY", OLED_COLOR_WHITE);

    OLED_DrawLine(0, 50, 127, 50, OLED_COLOR_WHITE);
    OLED_ShowString(14, 53, "APP2 | SYSTEM OK", OLED_COLOR_WHITE);
    OLED_Update();
}

void OLED_ShowOTAFlashing(void)
{
    OLED_Clear();
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_WHITE);
    OLED_ShowString(22, 2, "== OTAutomotive ==", OLED_COLOR_WHITE);

    OLED_DrawBitmap(8, 15, 32, 32, Bitmap_OtaFlashing_32x32, OLED_COLOR_WHITE);
    OLED_ShowString(46, 20, "OTA FLASHING", OLED_COLOR_WHITE);
    OLED_ShowString(46, 33, "UPDATING FIRMWARE", OLED_COLOR_WHITE);

    OLED_DrawLine(0, 50, 127, 50, OLED_COLOR_WHITE);
    OLED_ShowString(14, 53, "DO NOT POWER OFF", OLED_COLOR_WHITE);
    OLED_Update();
}
