#include "flash.h"
#include "stm32f10x.h"
#include "stm32f10x_flash.h"

uint32_t Flash_GetPartitionAddress(uint8_t partition_id)
{
    if (partition_id == PARTITION_APP1)
    {
        return FLASH_APP1_ADDRESS;
    }
    if (partition_id == PARTITION_APP2)
    {
        return FLASH_APP2_ADDRESS;
    }
    return 0;
}

uint32_t Flash_GetPartitionSizeKB(uint8_t partition_id)
{
    if (partition_id == PARTITION_APP1)
    {
        return FLASH_APP1_SIZE_KB;
    }
    if (partition_id == PARTITION_APP2)
    {
        return FLASH_APP2_SIZE_KB;
    }
    return 0;
}

uint8_t Flash_ErasePartition(uint32_t start_address, uint32_t size_in_kb)
{
    uint8_t ok = 1;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    for (uint32_t i = 0; i < size_in_kb; i++)
    {
        uint32_t page_addr = start_address + (i * 1024U);
        if (FLASH_ErasePage(page_addr) != FLASH_COMPLETE)
        {
            ok = 0;
            break;
        }
    }

    FLASH_Lock();
    return ok;
}

uint32_t Flash_WriteChunk(uint32_t address, const uint8_t *data, uint32_t len)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    for (uint32_t i = 0; i < len; i += 2)
    {
        uint8_t low_byte  = data[i];
        uint8_t high_byte = (i + 1 < len) ? data[i + 1] : 0xFFU;
        uint16_t data16   = (uint16_t)low_byte | ((uint16_t)high_byte << 8);

        if (FLASH_ProgramHalfWord(address + i, data16) != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return 0;
        }
    }

    FLASH_Lock();
    return len;
}

uint8_t Flash_VerifyChunk(uint32_t address, const uint8_t *data, uint32_t len)
{
    const volatile uint8_t *flash_ptr = (const volatile uint8_t *)address;

    for (uint32_t i = 0; i < len; i++)
    {
        if (flash_ptr[i] != data[i])
        {
            return 0;
        }
    }

    return 1;
}

uint8_t Flash_GetActivePartition(void)
{
    uint8_t active_app = *(volatile uint8_t *)FLASH_OB_DATA0_ADDR;

    if (active_app != PARTITION_APP1 && active_app != PARTITION_APP2)
    {
        active_app = PARTITION_APP1;
    }

    return active_app;
}

void Flash_SetActivePartition(uint8_t partition_id)
{
    if (partition_id != PARTITION_APP1 && partition_id != PARTITION_APP2)
    {
        return;
    }

    uint8_t current_id = *(volatile uint8_t *)FLASH_OB_DATA0_ADDR;
    if (current_id == partition_id)
    {
        return;
    }

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_EraseOptionBytes();
    FLASH_ProgramOptionByteData(FLASH_OB_DATA0_ADDR, partition_id);
    FLASH_Lock();
}
