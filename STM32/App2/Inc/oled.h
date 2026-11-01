/**
 * @file    oled.h
 * @brief   Driver màn hình OLED SSD1306 (128x64) giao tiếp I2C qua chân PB6 (SCL) và PB7 (SDA)
 *          cho STM32F103 (App 2).
 */

#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include "stm32f10x.h"

/* Cấu hình phần cứng Software I2C trên Port B */
#define OLED_I2C_GPIO_CLK        RCC_APB2Periph_GPIOB
#define OLED_I2C_PORT            GPIOB
#define OLED_SCL_PIN             GPIO_Pin_6
#define OLED_SDA_PIN             GPIO_Pin_7

/* Địa chỉ I2C của màn hình OLED SSD1306 (0x3C dịch trái 1 bit) */
#define OLED_I2C_ADDR            0x78U

/* Kích thước màn hình */
#define OLED_WIDTH               128U
#define OLED_HEIGHT              64U
#define OLED_BUFFER_SIZE         (OLED_WIDTH * OLED_HEIGHT / 8U)

/* Định nghĩa màu sắc */
#define OLED_COLOR_BLACK         0U
#define OLED_COLOR_WHITE         1U
#define OLED_COLOR_INVERT        2U

/**
 * @brief  Khởi tạo chân GPIO (PB6, PB7) và gửi chuỗi lệnh thiết lập ban đầu cho SSD1306.
 */
void OLED_Init(void);

/**
 * @brief  Xóa sạch bộ đệm hiển thị trong RAM.
 */
void OLED_Clear(void);

/**
 * @brief  Đẩy toàn bộ 1024 byte bộ đệm từ RAM ra màn hình OLED qua I2C.
 */
void OLED_Update(void);

/**
 * @brief  Điều chỉnh độ sáng tương phản màn hình (0 - 255).
 * @param  contrast: Mức tương phản (khuyên dùng 0xCF).
 */
void OLED_SetBrightness(uint8_t contrast);

/**
 * @brief  Vẽ một điểm ảnh (Pixel) tại toạ độ (x, y).
 */
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief  Vẽ đoạn thẳng từ (x0, y0) đến (x1, y1) bằng thuật toán Bresenham.
 */
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);

/**
 * @brief  Vẽ hình chữ nhật.
 * @param  fill: 0 = chỉ vẽ viền, 1 = tô đặc bên trong.
 */
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color, uint8_t fill);

/**
 * @brief  Vẽ hình tròn tâm (xc, yc) bán kính r.
 */
void OLED_DrawCircle(uint8_t xc, uint8_t yc, uint8_t r, uint8_t color, uint8_t fill);

/**
 * @brief  Vẽ mảng dữ liệu Bitmap đơn sắc lên màn hình.
 */
void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap, uint8_t color);

/**
 * @brief  Hiển thị một ký tự ASCII (kích thước 6x8) tại toạ độ (x, y).
 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t color);

/**
 * @brief  Hiển thị chuỗi ký tự ASCII tại toạ độ (x, y).
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t color);

/**
 * @brief  Vẽ giao diện bảng điều khiển xe thể thao (Cyber Sportscar Dashboard) cho App 2.
 * @param  heartbeat_state: 0 hoặc 1 để tạo hiệu ứng nhịp tim / kết nối nhấp nháy.
 */
void OLED_ShowDashboard(uint8_t heartbeat_state);

#endif /* OLED_H */

