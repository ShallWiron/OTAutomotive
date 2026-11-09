#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <Arduino.h>
#include "driver/gpio.h"
#include "driver/twai.h"

class CAN_Driver {
public:
    CAN_Driver(gpio_num_t tx_pin, gpio_num_t rx_pin);

    bool begin();

    bool sendFrame(uint32_t id, const uint8_t* data = nullptr, uint8_t len = 0, uint32_t timeout_ms = 100);

    bool sendWithAck(uint32_t tx_id, const uint8_t* data, uint8_t len,
                     uint32_t ack_id, uint8_t expected_ack,
                     uint32_t timeout_ms = 1000, uint8_t max_retries = 3);

    bool receiveFrame(twai_message_t &rx_msg, uint32_t timeout_ms = 10);

    void clearRxQueue();

    bool waitResponse(uint32_t response_id, uint8_t expected_val, uint32_t timeout_ms = 1000);

private:
    gpio_num_t _tx_pin;
    gpio_num_t _rx_pin;
};

#endif

