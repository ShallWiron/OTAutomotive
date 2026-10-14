#ifndef FLASH_OPS_H
#define FLASH_OPS_H

#include <stdint.h>

/* ==================== Cấu hình phân vùng Flash ==================== */

#define FLASH_BOOTLOADER_ADDRESS 0x08000000UL /* Địa chỉ bắt đầu Bootloader (16KB) */
#define FLASH_APP1_ADDRESS       0x08004000UL /* Địa chỉ bắt đầu App 1 (Page 16) */
#define FLASH_APP2_ADDRESS       0x08012000UL /* Địa chỉ bắt đầu App 2 (Page 72) */
#define FLASH_APP1_SIZE_KB       56UL         /* Kích thước App 1 (KB) */
#define FLASH_APP2_SIZE_KB       56UL         /* Kích thước App 2 (KB) */
#define FLASH_OB_DATA0_ADDR      0x1FFFF804UL /* User Data 0 trong Option Bytes */

#define PARTITION_APP1           0x01U
#define PARTITION_APP2           0x02U

/* ==================== API ==================== */

/**
 * @brief  Lấy địa chỉ bắt đầu Flash tương ứng với ID phân vùng
 * @param  partition_id  PARTITION_APP1 (1) hoặc PARTITION_APP2 (2)
 * @retval Địa chỉ bắt đầu phân vùng (0 nếu ID không hợp lệ)
 */
uint32_t Flash_GetPartitionAddress(uint8_t partition_id);

/**
 * @brief  Lấy kích thước phân vùng (KB) theo ID phân vùng
 * @param  partition_id  PARTITION_APP1 (1) hoặc PARTITION_APP2 (2)
 * @retval Kích thước tính bằng KB (0 nếu ID không hợp lệ)
 */
uint32_t Flash_GetPartitionSizeKB(uint8_t partition_id);

/**
 * @brief  Xóa toàn bộ partition Flash theo địa chỉ bắt đầu và kích thước
 * @param  start_address  Địa chỉ trang Flash đầu tiên cần xóa
 * @param  size_in_kb     Kích thước vùng cần xóa (đơn vị KB = số trang)
 * @retval 1 = Xóa thành công, 0 = Có lỗi khi xóa
 */
uint8_t Flash_ErasePartition(uint32_t start_address, uint32_t size_in_kb);

/**
 * @brief  Ghi chunk dữ liệu vào Flash theo Half-Word (16-bit)
 * @param  address  Địa chỉ Flash bắt đầu ghi (phải căn 2 byte)
 * @param  data     Con trỏ tới buffer dữ liệu cần ghi
 * @param  len      Số byte cần ghi (nếu lẻ byte cuối sẽ tự đệm 0xFF)
 * @retval Số byte đã ghi thành công (0 nếu xảy ra lỗi ghi Flash)
 */
uint32_t Flash_WriteChunk(uint32_t address, const uint8_t *data, uint32_t len);

/**
 * @brief  Đối chiếu (Verify) dữ liệu đã ghi trong Flash với buffer RAM
 * @param  address  Địa chỉ Flash bắt đầu kiểm tra
 * @param  data     Con trỏ tới buffer dữ liệu gốc trong RAM
 * @param  len      Số byte cần kiểm tra
 * @retval 1 = Khớp hoàn toàn, 0 = Sai lệch dữ liệu
 */
uint8_t Flash_VerifyChunk(uint32_t address, const uint8_t *data, uint32_t len);

/**
 * @brief  Đọc partition đang active từ Option Bytes (Data0)
 * @retval 1 = App 1, 2 = App 2 (mặc định 1 nếu giá trị trống 0xFF hoặc không hợp lệ)
 */
uint8_t Flash_GetActivePartition(void);

/**
 * @brief  Lưu partition active mới vào Option Bytes (Data0)
 * @param  partition_id  ID partition mới (1 hoặc 2)
 */
void Flash_SetActivePartition(uint8_t partition_id);

#endif /* FLASH_OPS_H */
