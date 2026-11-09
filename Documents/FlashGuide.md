# Flash Operations — STM32F103 (Linux/Makefile)

---

## Nguyên tắc cơ bản

| Quy tắc | Chi tiết |
|:--------|:---------|
| Unlock/Lock | Bắt buộc `FLASH_Unlock()` trước khi xóa/ghi, `FLASH_Lock()` sau khi xong |
| Xóa trước khi ghi | Flash chỉ ghi được `1→0`. Muốn ghi đè phải xóa trang về `0xFF` trước |
| Page size | 1KB (1024 bytes) — đơn vị xóa nhỏ nhất |
| Đơn vị ghi | Half-Word (16-bit) — `FLASH_ProgramHalfWord()` |
| Đọc Flash | Trực tiếp qua con trỏ, không cần unlock |

---

## Nạp firmware bằng `st-flash`

```bash
# Nạp Bootloader
st-flash write STM32/Bootloader/build/bootloader.bin 0x08000000

# Nạp App1 (bản tinh gọn - Origin: 0x08004000)
st-flash write STM32/App1/build/app1.bin 0x08004000

# Nạp App2 (bản OLED HUD + Còi Horn - Origin: 0x08012000)
st-flash write STM32/App2/build/app2.bin 0x08012000

# Đọc ngược 1KB tại địa chỉ bất kỳ để kiểm tra
st-flash read dump.bin 0x08004000 0x400
xxd dump.bin | head -n 8
```

---

## Debug từng dòng bằng GDB + OpenOCD

```bash
# Terminal 1 — khởi động OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# Terminal 2 — kết nối GDB
gdb-multiarch build/bootloader.elf
(gdb) target remote :3333
(gdb) monitor halt
(gdb) load
(gdb) break Bootloader_JumpToApp
(gdb) continue
```

---

## Hàm ghi Flash chuẩn (tham khảo)

Xem implementation đầy đủ trong dự án: [`flash.c`](../STM32/Bootloader/Src/flash.c)

```c
// Ghi chunk dữ liệu vào Flash (half-word aligned, padding 0xFF nếu len lẻ)
uint32_t Flash_WriteChunk(uint32_t address, const uint8_t *data, uint32_t len)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    for (uint32_t i = 0; i < len; i += 2)
    {
        uint8_t  lo    = data[i];
        uint8_t  hi    = (i + 1 < len) ? data[i + 1] : 0xFFU;  // padding nếu len lẻ
        uint16_t hw    = (uint16_t)lo | ((uint16_t)hi << 8);

        if (FLASH_ProgramHalfWord(address + i, hw) != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return 0;
        }
    }

    FLASH_Lock();
    return len;
}
```

> [!NOTE]
> `address` phải căn chỉnh 2-byte (even). Firmware ARM Thumb luôn có kích thước chẵn nên không xảy ra vấn đề trong thực tế.
