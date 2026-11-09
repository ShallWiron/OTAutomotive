#ifndef OTA_PROTOCOL_H
#define OTA_PROTOCOL_H

#include <stdint.h>
#include "stm32f10x_can.h"

#define ID_QUERY_APP            0b11000100000U
#define ID_START_OTA            0b11001000000U
#define ID_START_FILE           0b11010000000U
#define ID_START_CHUNK          0b11011000000U
#define ID_DATA_CHUNK           0b11100000000U
#define ID_END_CHUNK            0b11101000000U
#define ID_END_FILE             0b11110000000U
#define ID_RESPONSE             0b11111000000U

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
