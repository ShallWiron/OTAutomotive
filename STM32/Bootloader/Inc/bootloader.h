#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>

/* ==================== Cờ khởi động OTA ==================== */
#define BKP_OTA_FLAG_MAGIC      0x5A5AU /* Cờ lưu trong BKP_DR1 để yêu cầu vào Bootloader OTA */

/* ==================== API ==================== */

/**
 * @brief  Cấu hình GPIO cho bootloader
 *         - PC13: LED trạng thái (Push-Pull, Active LOW)
 *         - PB2:  Công tắc chế độ boot (Input Pull-Up, đọc BOOT1 jumper)
 */
void Bootloader_GPIO_Init(void);

/**
 * @brief  Kiểm tra trạng thái chân PB2 (BOOT1 jumper)
 * @retval 0 = PB2 LOW  -> Chạy App bình thường
 *         1 = PB2 HIGH -> Vào chế độ Bootloader OTA
 */
uint8_t Bootloader_IsOtaModeRequested(void);

/**
 * @brief  Kiểm tra tính hợp lệ của Vector Table tại địa chỉ phân vùng App
 * @param  app_address  Địa chỉ bắt đầu của partition App (FLASH_APP1_ADDRESS / FLASH_APP2_ADDRESS)
 * @retval 1 = Hợp lệ (MSP nằm trong SRAM và Reset_Handler nằm trong vùng Flash của App)
 *         0 = Không hợp lệ (Flash trống hoặc hỏng)
 */
uint8_t Bootloader_CheckAppValid(uint32_t app_address);

/**
 * @brief  Nhảy sang application tại địa chỉ cho trước
 * @param  app_address  Địa chỉ bắt đầu của partition App (chứa Vector Table)
 * @note   Kiểm tra tính hợp lệ của Vector Table trước khi nhảy.
 *         Nếu thành công, hàm KHÔNG return.
 *         Nếu không hợp lệ, hàm return bình thường (không nhảy).
 */
void Bootloader_JumpToApp(uint32_t app_address);

/**
 * @brief  Khởi động vào phân vùng đang Active trong Option Bytes,
 *         tự động Rollback sang phân vùng còn lại nếu phân vùng Active bị lỗi/trống.
 * @note   Nếu cả 2 phân vùng đều trống/lỗi, hàm sẽ return để Bootloader chờ nhận OTA.
 */
void Bootloader_BootActiveOrRollback(void);

/**
 * @brief  Đảo trạng thái LED PC13 (báo hiệu hoạt động)
 */
void Bootloader_ToggleLED(void);

#endif /* BOOTLOADER_H */
