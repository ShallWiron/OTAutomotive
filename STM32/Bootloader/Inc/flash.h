#ifndef FLASH_OPS_H
#define FLASH_OPS_H

#include <stdint.h>

#define FLASH_BOOTLOADER_ADDRESS 0x08000000UL
#define FLASH_APP1_ADDRESS       0x08004000UL
#define FLASH_APP2_ADDRESS       0x08012000UL
#define FLASH_APP1_SIZE_KB       56UL
#define FLASH_APP2_SIZE_KB       56UL
#define FLASH_OB_DATA0_ADDR      0x1FFFF804UL

#define PARTITION_APP1           0x01U
#define PARTITION_APP2           0x02U

uint32_t Flash_GetPartitionAddress(uint8_t partition_id);
uint32_t Flash_GetPartitionSizeKB(uint8_t partition_id);
uint8_t Flash_ErasePartition(uint32_t start_address, uint32_t size_in_kb);
uint32_t Flash_WriteChunk(uint32_t address, const uint8_t *data, uint32_t len);
uint8_t Flash_VerifyChunk(uint32_t address, const uint8_t *data, uint32_t len);
uint8_t Flash_GetActivePartition(void);
void Flash_SetActivePartition(uint8_t partition_id);

#endif
