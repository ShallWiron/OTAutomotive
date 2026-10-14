#include "ota_protocol.h"
#include "crc16.h"
#include "flash.h"
#include "bootloader.h"
#include "can.h"
#include "uart.h"
#include "delay_tim2.h"

/* ==================== Biến nội bộ (static) ==================== */

static uint8_t            chunk_buffer[OTA_CHUNK_BUFFER_SIZE];
static uint16_t           buffer_index        = 0;
static uint16_t           expected_chunk_len  = 0;
static uint16_t           chunk_index         = 0;

static OTA_StateTypeDef   ota_state           = OTA_STATE_IDLE;
static Chunk_StateTypeDef chunk_state         = CHUNK_STATE_IDLE;

static uint8_t            target_app_id       = PARTITION_APP1;
static uint32_t           target_base_addr    = FLASH_APP1_ADDRESS;
static uint32_t           flash_offset        = 0;

static uint32_t           expected_file_size  = 0;
static uint16_t           expected_file_crc   = 0;
static uint8_t            has_file_crc        = 0;

static uint8_t            expected_packet_idx = 0;
static uint8_t            chunk_seq_error     = 0;

/* ==================== Hàm nội bộ gửi ACK / NACK ==================== */

static void OTA_SendResponse(uint8_t status)
{
    uint8_t resp[1] = { status };
    CAN_SendMessagePolling(ID_RESPONSE, resp, 1);
}

/* ==================== Khởi tạo OTA ==================== */

void OTA_Init(void)
{
    buffer_index        = 0;
    expected_chunk_len  = 0;
    chunk_index         = 0;
    expected_packet_idx = 0;
    chunk_seq_error     = 0;
    ota_state           = OTA_STATE_IDLE;
    chunk_state         = CHUNK_STATE_IDLE;
    target_app_id       = PARTITION_APP1;
    target_base_addr    = FLASH_APP1_ADDRESS;
    flash_offset        = 0;
    expected_file_size  = 0;
    expected_file_crc   = 0;
    has_file_crc        = 0;
}

/* ==================== Xử lý từng bước giao thức CAN OTA ==================== */

static void OTA_HandleStartOta(const CanRxMsg *rx)
{
    if (rx->DLC < 1)
    {
        UART_SendString("[OTA ERR] START_OTA thieu App ID\r\n");
        OTA_SendResponse(OTA_NACK);
        return;
    }

    uint8_t req_app_id = rx->Data[0];
    uint32_t part_addr = Flash_GetPartitionAddress(req_app_id);
    uint32_t part_size = Flash_GetPartitionSizeKB(req_app_id);

    if (part_addr == 0 || part_size == 0)
    {
        UART_SendString("[OTA ERR] ID partition khong hop le: 0x");
        UART_SendHex8(req_app_id);
        UART_SendString("\r\n");
        OTA_SendResponse(OTA_NACK);
        return;
    }

    UART_SendString("\r\n[OTA] Nhan lenh START_OTA cho App ");
    UART_SendDec(req_app_id);
    UART_SendString(" (0x");
    UART_SendHex32(part_addr);
    UART_SendString("), dang xoa Flash...\r\n");

    if (!Flash_ErasePartition(part_addr, part_size))
    {
        UART_SendString("[OTA ERR] Xoa Flash that bai!\r\n");
        OTA_SendResponse(OTA_NACK);
        return;
    }

    target_app_id      = req_app_id;
    target_base_addr   = part_addr;
    flash_offset       = 0;
    buffer_index       = 0;
    expected_chunk_len = 0;
    chunk_index        = 0;
    expected_file_size = 0;
    expected_file_crc  = 0;
    has_file_crc       = 0;
    chunk_state        = CHUNK_STATE_IDLE;
    ota_state          = OTA_STATE_WAIT_FILE;

    UART_SendString("[OTA] Xoa Flash App ");
    UART_SendDec(target_app_id);
    UART_SendString(" thanh cong -> Gui ACK\r\n");

    OTA_SendResponse(OTA_ACK);
}

static void OTA_HandleStartFile(const CanRxMsg *rx)
{
    if (rx->DLC < 4)
    {
        UART_SendString("[OTA ERR] START_FILE DLC < 4\r\n");
        OTA_SendResponse(OTA_NACK);
        return;
    }

    uint32_t file_size = ((uint32_t)rx->Data[0] << 24) |
                         ((uint32_t)rx->Data[1] << 16) |
                         ((uint32_t)rx->Data[2] << 8)  |
                         ((uint32_t)rx->Data[3]);

    uint32_t max_bytes = Flash_GetPartitionSizeKB(target_app_id) * 1024U;
    if (file_size == 0 || file_size > max_bytes)
    {
        UART_SendString("[OTA ERR] Kich thuoc file khong hop le: ");
        UART_SendDec(file_size);
        UART_SendString(" bytes\r\n");
        OTA_SendResponse(OTA_NACK);
        return;
    }

    expected_file_size = file_size;

    /* Nếu ESP32 gửi kèm 2 byte Global CRC16 ở Data[4..5] (DLC >= 6) */
    if (rx->DLC >= 6)
    {
        expected_file_crc = ((uint16_t)rx->Data[4] << 8) | rx->Data[5];
        has_file_crc      = 1;
    }
    else
    {
        expected_file_crc = 0;
        has_file_crc      = 0;
    }

    flash_offset       = 0;
    buffer_index       = 0;
    expected_chunk_len = 0;
    chunk_index        = 0;
    chunk_state        = CHUNK_STATE_IDLE;
    ota_state          = OTA_STATE_RECEIVING;

    UART_SendString("[OTA] Nhan START_FILE: Size = ");
    UART_SendDec(expected_file_size);
    UART_SendString(" bytes");
    if (has_file_crc)
    {
        UART_SendString(", Global CRC16 = 0x");
        UART_SendHex16(expected_file_crc);
    }
    UART_SendString(" -> Gui ACK\r\n");

    OTA_SendResponse(OTA_ACK);
}

static void OTA_HandleStartChunk(const CanRxMsg *rx)
{
    if (rx->DLC < 2)
    {
        UART_SendString("[OTA ERR] START_CHUNK DLC < 2\r\n");
        return;
    }

    uint16_t len = ((uint16_t)rx->Data[0] << 8) | rx->Data[1];
    uint32_t max_bytes = Flash_GetPartitionSizeKB(target_app_id) * 1024U;

    if (len == 0 || len > OTA_CHUNK_BUFFER_SIZE || (flash_offset + len) > max_bytes)
    {
        UART_SendString("[OTA ERR] Do dai Chunk khong hop le: ");
        UART_SendDec(len);
        UART_SendString("\r\n");
        chunk_state  = CHUNK_STATE_IDLE;
        buffer_index = 0;
        return;
    }

    /* Reset bộ đệm chunk để sẵn sàng nhận mới hoặc nhận lại (Retry) */
    expected_chunk_len  = len;
    buffer_index        = 0;
    expected_packet_idx = 0;
    chunk_seq_error     = 0;
    chunk_state         = CHUNK_STATE_RECEIVING;
}

static void OTA_HandleDataChunk(const CanRxMsg *rx)
{
    if (chunk_state != CHUNK_STATE_RECEIVING)
    {
        return;
    }

    /* Lấy STT gói tin từ 6 bit thấp [5:0] của CAN ID */
    uint8_t pkt_idx = (uint8_t)(rx->StdId & ID_PACKET_INDEX_MASK);

    /* 1. Kiểm tra STT gói tin: nếu sai thứ tự hoặc rớt gói trước đó */
    if (pkt_idx != expected_packet_idx)
    {
        UART_SendString("[OTA ERR] Sai STT goi CAN! Ky vong: #");
        UART_SendDec(expected_packet_idx);
        UART_SendString(", Nhan: #");
        UART_SendDec(pkt_idx);
        UART_SendString(" -> Gui NACK yeu cau truyen lai Chunk!\r\n");

        chunk_seq_error = 1;
        /* Gửi NACK sớm để ESP32 có thể ngắt chuỗi phát và retry ngay */
        OTA_SendResponse(OTA_NACK);
        return;
    }

    /* 2. Nếu Chunk này đã đánh dấu lỗi thì bỏ qua các gói tiếp theo */
    if (chunk_seq_error)
    {
        return;
    }

    /* 3. Lưu dữ liệu gói vào buffer RAM */
    if ((uint32_t)buffer_index + rx->DLC <= OTA_CHUNK_BUFFER_SIZE)
    {
        for (uint8_t i = 0; i < rx->DLC; i++)
        {
            chunk_buffer[buffer_index + i] = rx->Data[i];
        }
        buffer_index += rx->DLC;
        expected_packet_idx++;
    }
    else
    {
        /* Tràn buffer chunk -> đánh dấu lỗi */
        buffer_index    = OTA_CHUNK_BUFFER_SIZE + 1U;
        chunk_seq_error = 1;
        OTA_SendResponse(OTA_NACK);
    }
}

static void OTA_HandleEndChunk(const CanRxMsg *rx)
{
    if (chunk_state != CHUNK_STATE_RECEIVING)
    {
        UART_SendString("[OTA ERR] Nhan END_CHUNK khi chua o trang thai RECEIVING -> Gui NACK\r\n");
        OTA_SendResponse(OTA_NACK);
        return;
    }

    /* 1. Kiểm tra cờ lỗi sai/mất gói STT */
    if (chunk_seq_error)
    {
        UART_SendString("[OTA ERR] Chunk co goi bi sai/mat STT -> Gui NACK (yeu cau gui lai Chunk)\r\n");
        chunk_state         = CHUNK_STATE_IDLE;
        buffer_index        = 0;
        expected_chunk_len  = 0;
        expected_packet_idx = 0;
        chunk_seq_error     = 0;
        OTA_SendResponse(OTA_NACK);
        return;
    }

    /* 2. Kiểm tra đủ số byte của Chunk (phát hiện rớt frame CAN giữa chừng) */
    if (buffer_index != expected_chunk_len)
    {
        UART_SendString("[OTA ERR] Chunk thieu goi CAN! Nhan ");
        UART_SendDec(buffer_index);
        UART_SendString("/");
        UART_SendDec(expected_chunk_len);
        UART_SendString(" bytes -> Gui NACK (yeu cau gui lai Chunk)\r\n");

        chunk_state         = CHUNK_STATE_IDLE;
        buffer_index        = 0;
        expected_chunk_len  = 0;
        expected_packet_idx = 0;
        chunk_seq_error     = 0;
        OTA_SendResponse(OTA_NACK);
        return;
    }

    /* 3. Tính và kiểm tra mã CRC16-Modbus của Chunk (nếu ESP32 gửi kèm 2 byte CRC16) */
    uint16_t calc_crc = CRC16_Modbus(chunk_buffer, expected_chunk_len);
    if (rx->DLC >= 2)
    {
        uint16_t rx_crc = ((uint16_t)rx->Data[0] << 8) | rx->Data[1];
        if (calc_crc != rx_crc)
        {
            UART_SendString("[OTA ERR] Sai Chunk CRC16! Calc=0x");
            UART_SendHex16(calc_crc);
            UART_SendString(", Rx=0x");
            UART_SendHex16(rx_crc);
            UART_SendString(" -> Gui NACK (khong ghi Flash)\r\n");

            chunk_state         = CHUNK_STATE_IDLE;
            buffer_index        = 0;
            expected_chunk_len  = 0;
            expected_packet_idx = 0;
            chunk_seq_error     = 0;
            OTA_SendResponse(OTA_NACK);
            return;
        }
    }

    /* 4. Ghi Chunk hợp lệ vào Flash và đọc ngược lại kiểm tra (Verify) */
    uint32_t write_addr = target_base_addr + flash_offset;
    uint32_t written    = Flash_WriteChunk(write_addr, chunk_buffer, expected_chunk_len);

    if (written != expected_chunk_len || !Flash_VerifyChunk(write_addr, chunk_buffer, expected_chunk_len))
    {
        UART_SendString("[OTA ERR] Loi ghi/verify Flash tai 0x");
        UART_SendHex32(write_addr);
        UART_SendString(" -> Gui NACK\r\n");

        chunk_state         = CHUNK_STATE_IDLE;
        buffer_index        = 0;
        expected_chunk_len  = 0;
        expected_packet_idx = 0;
        chunk_seq_error     = 0;
        OTA_SendResponse(OTA_NACK);
        return;
    }

    flash_offset += expected_chunk_len;
    chunk_index++;

    UART_SendString("[OTA] Chunk #");
    UART_SendDec(chunk_index);
    UART_SendString(" OK (");
    UART_SendDec(expected_chunk_len);
    UART_SendString("B, CRC=0x");
    UART_SendHex16(calc_crc);
    UART_SendString(", Offset=");
    UART_SendDec(flash_offset);
    if (expected_file_size > 0)
    {
        UART_SendString("/");
        UART_SendDec(expected_file_size);
    }
    UART_SendString(")\r\n");

    /* Reset trạng thái Chunk, đảo LED báo hiệu và gửi ACK */
    chunk_state         = CHUNK_STATE_IDLE;
    buffer_index        = 0;
    expected_chunk_len  = 0;
    expected_packet_idx = 0;
    chunk_seq_error     = 0;

    Bootloader_ToggleLED();
    OTA_SendResponse(OTA_ACK);
}

static void OTA_HandleEndFile(const CanRxMsg *rx)
{
    UART_SendString("\r\n[OTA] Nhan lenh END_FILE. Dang kiem tra toan ven Firmware...\r\n");

    /* 1. Kiểm tra tổng dung lượng đã ghi có khớp với khai báo ở START_FILE không */
    if (expected_file_size > 0 && flash_offset != expected_file_size)
    {
        UART_SendString("[OTA ERR] Tong so byte khong khop! Da ghi ");
        UART_SendDec(flash_offset);
        UART_SendString("/");
        UART_SendDec(expected_file_size);
        UART_SendString(" bytes -> Rollback!\r\n");

        OTA_SendResponse(OTA_NACK);
        Flash_ErasePartition(target_base_addr, Flash_GetPartitionSizeKB(target_app_id));
        ota_state = OTA_STATE_IDLE;
        Bootloader_BootActiveOrRollback();
        return;
    }

    /* 2. Kiểm tra Global CRC16 trên toàn bộ vùng Flash vừa nạp */
    if (rx->DLC >= 2)
    {
        expected_file_crc = ((uint16_t)rx->Data[0] << 8) | rx->Data[1];
        has_file_crc      = 1;
    }

    uint16_t flash_crc = CRC16_Modbus((const uint8_t *)target_base_addr, flash_offset);
    UART_SendString("[OTA] Flash Global CRC16 = 0x");
    UART_SendHex16(flash_crc);
    UART_SendString("\r\n");

    if (has_file_crc && (flash_crc != expected_file_crc))
    {
        UART_SendString("[OTA ERR] Sai Global CRC16! Expected=0x");
        UART_SendHex16(expected_file_crc);
        UART_SendString(", Actual=0x");
        UART_SendHex16(flash_crc);
        UART_SendString(" -> Xoa phan vung loi & Rollback!\r\n");

        OTA_SendResponse(OTA_NACK);
        Flash_ErasePartition(target_base_addr, Flash_GetPartitionSizeKB(target_app_id));
        ota_state = OTA_STATE_IDLE;
        Bootloader_BootActiveOrRollback();
        return;
    }

    /* 3. Kiểm tra tính hợp lệ của Vector Table (MSP & Reset_Handler) của Firmware mới */
    if (!Bootloader_CheckAppValid(target_base_addr))
    {
        UART_SendString("[OTA ERR] Firmware moi khong co Vector Table hop le -> Xoa & Rollback!\r\n");

        OTA_SendResponse(OTA_NACK);
        Flash_ErasePartition(target_base_addr, Flash_GetPartitionSizeKB(target_app_id));
        ota_state = OTA_STATE_IDLE;
        Bootloader_BootActiveOrRollback();
        return;
    }

    /* 4. Toàn bộ kiểm tra thành công: Cập nhật Option Bytes sang phân vùng mới, gửi ACK và Jump */
    Flash_SetActivePartition(target_app_id);
    OTA_SendResponse(OTA_ACK);

    UART_SendString("[OTA SUCCESS] Cap nhat hoan tat! Active Partition = App ");
    UART_SendDec(target_app_id);
    UART_SendString(" -> Jumping to 0x");
    UART_SendHex32(target_base_addr);
    UART_SendString("\r\n");

    ota_state = OTA_STATE_IDLE;
    Delay_Ms(20);
    Bootloader_JumpToApp(target_base_addr);
}

/* ==================== Dispatcher chính ==================== */

void OTA_ProcessCanMessage(const CanRxMsg *rx)
{
    uint32_t cmd_id = rx->StdId & ID_CMD_MASK;

    /* Lệnh QUERY_APP: ESP32 truy vấn trạng thái phân vùng App đang Active */
    if (cmd_id == ID_QUERY_APP)
    {
        uint8_t resp[2];
        resp[0] = OTA_ACK;
        resp[1] = Flash_GetActivePartition();
        CAN_SendMessagePolling(ID_RESPONSE, resp, 2);
        return;
    }

    /* Lệnh START_OTA luôn được ưu tiên xử lý ở mọi trạng thái để có thể bắt đầu phiên OTA mới */
    if (cmd_id == ID_START_OTA)
    {
        OTA_HandleStartOta(rx);
        return;
    }

    /* Lệnh START_FILE có thể nhận sau START_OTA hoặc khi gửi lại */
    if (cmd_id == ID_START_FILE)
    {
        OTA_HandleStartFile(rx);
        return;
    }

    if (ota_state == OTA_STATE_RECEIVING)
    {
        if (cmd_id == ID_START_CHUNK)
        {
            OTA_HandleStartChunk(rx);
        }
        else if (cmd_id == ID_DATA_CHUNK)
        {
            OTA_HandleDataChunk(rx);
        }
        else if (cmd_id == ID_END_CHUNK)
        {
            OTA_HandleEndChunk(rx);
        }
        else if (cmd_id == ID_END_FILE)
        {
            OTA_HandleEndFile(rx);
        }
    }
}

void OTA_Run(void)
{
    if (CAN_MessagePending(CAN1, CAN_FIFO1) > 0)
    {
        CanRxMsg rx;
        CAN_Receive(CAN1, CAN_FIFO1, &rx);
        OTA_ProcessCanMessage(&rx);
    }
}
