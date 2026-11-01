/**
 * @file    oled.c
 * @brief   Driver màn hình OLED SSD1306 (128x64) I2C (PB6-SCL, PB7-SDA)
 *          cho STM32F103 (App 2).
 */

#include "oled.h"
#include "bitmap.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <string.h>

/* Bộ nhớ đệm đồ họa hiển thị (128 x 64 bit = 1024 bytes) */
static uint8_t s_OLED_Buffer[OLED_BUFFER_SIZE];

/* ===================================================================
 *                    CÁC HÀM GIAO TIẾP SOFTWARE I2C
 * =================================================================== */

#define I2C_SCL_HIGH()   GPIO_SetBits(OLED_I2C_PORT, OLED_SCL_PIN)
#define I2C_SCL_LOW()    GPIO_ResetBits(OLED_I2C_PORT, OLED_SCL_PIN)
#define I2C_SDA_HIGH()   GPIO_SetBits(OLED_I2C_PORT, OLED_SDA_PIN)
#define I2C_SDA_LOW()    GPIO_ResetBits(OLED_I2C_PORT, OLED_SDA_PIN)
#define I2C_SDA_READ()   GPIO_ReadInputDataBit(OLED_I2C_PORT, OLED_SDA_PIN)

static inline void I2C_Delay(void)
{
    /* Trễ ngắn tạo xung nhịp I2C ~300kHz - 400kHz tại 72MHz */
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

    /* Chu kỳ nhận bit ACK từ Slave */
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
    I2C_WriteByte(0x00U); /* Co = 0, D/C# = 0: Lệnh Command */
    I2C_WriteByte(cmd);
    I2C_Stop();
}

/* ===================================================================
 *                    DRIVER KHỞI TẠO & ĐIỀU KHIỂN SSD1306
 * =================================================================== */

void OLED_Init(void)
{
    GPIO_InitTypeDef gpioInit;

    /* 1. Bật clock cho GPIOB */
    RCC_APB2PeriphClockCmd(OLED_I2C_GPIO_CLK, ENABLE);

    /* 2. Cấu hình chân PB6 (SCL) và PB7 (SDA) Open-Drain */
    gpioInit.GPIO_Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(OLED_I2C_PORT, &gpioInit);

    /* Đưa bus về trạng thái rảnh (Idle = High) */
    I2C_SCL_HIGH();
    I2C_SDA_HIGH();

    /* Chờ màn hình ổn định nguồn */
    for (volatile uint32_t i = 0; i < 100000U; i++) {
        __NOP();
    }

    /* 3. Bảng lệnh khởi tạo chuẩn SSD1306 (128x64) */
    static const uint8_t initCommands[] = {
        0xAEU,        /* Tắt hiển thị Display OFF */
        0xD5U, 0x80U, /* Tần số dao động nội OSC Frequency */
        0xA8U, 0x3FU, /* Multiplex Ratio = 63 (128x64) */
        0xD3U, 0x00U, /* Display Offset = 0 */
        0x40U,        /* Display Start Line = 0 */
        0x8DU, 0x14U, /* Bật mạch nhân áp Charge Pump (0x14 = Enable) */
        0x20U, 0x00U, /* Chế độ nạp bộ nhớ Horizontal Addressing Mode */
        0xA1U,        /* Đảo chiều quét cột Segment Re-map */
        0xC8U,        /* Đảo chiều quét hàng COM Output Scan Direction */
        0xDAU, 0x12U, /* Cấu hình chân COM Pins */
        0x81U, 0xCFU, /* Độ tương phản Contrast Control */
        0xD9U, 0xF1U, /* Chu kỳ nạp trước Pre-charge Period */
        0xDBU, 0x40U, /* Mức điện áp VCOMH Deselect Level */
        0xA4U,        /* Resume RAM content display */
        0xA6U,        /* Chế độ hiển thị bình thường Normal Display (1=sáng, 0=tối) */
        0x2EU,        /* Tắt cuộn trang Deactivate Scroll */
        0xAFU         /* Bật hiển thị Display ON */
    };

    for (uint32_t i = 0; i < sizeof(initCommands); i++) {
        OLED_WriteCommand(initCommands[i]);
    }

    /* Xóa sạch màn hình khi mới khởi động */
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
    /* Cấu hình phạm vi cột: 0 đến 127 */
    OLED_WriteCommand(0x21U);
    OLED_WriteCommand(0U);
    OLED_WriteCommand(OLED_WIDTH - 1U);

    /* Cấu hình phạm vi trang (Page): 0 đến 7 (8 trang x 8 bit = 64 pixels) */
    OLED_WriteCommand(0x22U);
    OLED_WriteCommand(0U);
    OLED_WriteCommand((OLED_HEIGHT / 8U) - 1U);

    /* Truyền toàn bộ 1024 byte dữ liệu trong 1 burst truyền liên tục */
    I2C_Start();
    I2C_WriteByte(OLED_I2C_ADDR);
    I2C_WriteByte(0x40U); /* Co = 0, D/C# = 1: Dữ liệu RAM Data */

    for (uint16_t i = 0; i < OLED_BUFFER_SIZE; i++) {
        I2C_WriteByte(s_OLED_Buffer[i]);
    }

    I2C_Stop();
}

/* ===================================================================
 *                    CÁC HÀM VẼ ĐỒ HOẠ CƠ BẢN
 * =================================================================== */

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if ((x >= OLED_WIDTH) || (y >= OLED_HEIGHT)) {
        return;
    }

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
        if ((x0 == x1) && (y0 == y1)) {
            break;
        }
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
    if ((ch < 32) || (ch > 126)) {
        ch = ' ';
    }
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
        if (y + 8U > OLED_HEIGHT) {
            break;
        }
        OLED_ShowChar(x, y, *str, color);
        x += 6U;
        str++;
    }
}

/* ===================================================================
 *                    GIAO DIỆN XE THỂ THAO & DASHBOARD APP 2
 * =================================================================== */

void OLED_ShowDashboard(uint8_t heartbeat_state)
{
    /* 1. Nạp ảnh Xe Thể Thao Công Nghệ Cao (128x64) vào FrameBuffer */
    memcpy(s_OLED_Buffer, Bitmap_SportCar_128x64, sizeof(s_OLED_Buffer));

    /* 2. Tiêu đề thương hiệu trên Header banner (y=1) */
    OLED_ShowString(10U, 1U, "== OTAutomotive ==", OLED_COLOR_WHITE);

    /* 3. Thông tin trạng thái App 2 và CAN Bus tại Footer box */
    OLED_ShowString(4U, 53U, "APP2:ONLINE", OLED_COLOR_WHITE);
    OLED_ShowString(74U, 53U, "CAN:500K", OLED_COLOR_WHITE);

    /* 4. Icon nhịp tim / kết nối nhấp nháy tại góc phải */
    if (heartbeat_state) {
        /* Chấm tròn đặc khi heartbeat = 1 */
        OLED_DrawCircle(123U, 56U, 2U, OLED_COLOR_WHITE, 1U);
    } else {
        /* Vòng tròn rỗng khi heartbeat = 0 */
        OLED_DrawCircle(123U, 56U, 2U, OLED_COLOR_WHITE, 0U);
    }

    /* 5. Đẩy ra màn hình OLED */
    OLED_Update();
}

