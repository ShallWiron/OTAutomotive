#ifndef OTA_PROTOCOL_H
#define OTA_PROTOCOL_H

#include <stdint.h>
#include "stm32f10x_can.h"

/* ==================== Định nghĩa CAN ID (11-bit) ==================== */
/*
 * Cấu trúc 11-bit CAN ID cho OTA:
 * - Bit [10:9] = 11 (Luồng OTA, lọc vào FIFO1)
 * - Bit [8:6]  = Mã lệnh CMD (001 -> 111)
 * - Bit [5:0]  = 000000 (hoặc STT gói tin)
 */
#define ID_QUERY_APP            0b11000100000U  /* 0x620: ESP32 truy vấn App hiện tại */
#define ID_START_OTA            0b11001000000U  /* 0x640: Bắt đầu OTA & xóa Flash [Data0: AppID] */
#define ID_START_FILE           0b11010000000U  /* 0x680: Thông tin File [Data0..3: Size, Data4..5: Global CRC16] */
#define ID_START_CHUNK          0b11011000000U  /* 0x6C0: Bắt đầu Chunk [Data0..1: Chunk Length] */
#define ID_DATA_CHUNK           0b11100000000U  /* 0x700: Gói dữ liệu Chunk (tối đa 8 bytes/frame) */
#define ID_END_CHUNK            0b11101000000U  /* 0x740: Kết thúc Chunk [Data0..1: Chunk CRC16] */
#define ID_END_FILE             0b11110000000U  /* 0x780: Kết thúc File [Optional Data0..1: Global CRC16] */
#define ID_RESPONSE             0b11111000000U  /* 0x7C0: Phản hồi từ STM32 -> ESP32 [Data0: ACK/NACK] */

#define ID_CMD_MASK             0b11111000000U  
#define ID_PACKET_INDEX_MASK    0b00000111111U

#define OTA_ACK                 0x00U
#define OTA_NACK                0x01U
#define OTA_CHUNK_BUFFER_SIZE   256U

typedef enum
{
    OTA_STATE_IDLE = 0,
    OTA_STATE_WAIT_FILE,
    OTA_STATE_RECEIVING
} OTA_StateTypeDef;

typedef enum
{
    CHUNK_STATE_IDLE = 0,
    CHUNK_STATE_RECEIVING
} Chunk_StateTypeDef;

void OTA_Init(void);

void OTA_ProcessCanMessage(const CanRxMsg *rx);

void OTA_Run(void);

#endif
