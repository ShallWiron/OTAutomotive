#include "CAN_Driver.h"

CAN_Driver::CAN_Driver(gpio_num_t tx_pin, gpio_num_t rx_pin)
    : _tx_pin(tx_pin), _rx_pin(rx_pin) {}

bool CAN_Driver::begin() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(_tx_pin, _rx_pin, TWAI_MODE_NORMAL);
    twai_timing_config_t  t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        Serial.println("[CAN DRV] Loi cai dat TWAI driver!");
        return false;
    }

    if (twai_start() != ESP_OK) {
        Serial.println("[CAN DRV] Loi khoi dong TWAI!");
        return false;
    }

    Serial.println("[CAN DRV] TWAI khoi tao thanh cong (500 kbps)");
    return true;
}

void CAN_Driver::clearRxQueue() {
    twai_message_t dummy;
    while (twai_receive(&dummy, 0) == ESP_OK) {
        // Xả sạch các frame cũ còn tồn trong buffer
    }
}

bool CAN_Driver::sendFrame(uint32_t id, const uint8_t* data, uint8_t len, uint32_t timeout_ms) {
    if (len > 8) len = 8;

    twai_message_t tx = {};
    tx.identifier = id;
    tx.extd = 0;
    tx.rtr = 0;
    tx.data_length_code = len;

    if (data != nullptr) {
        for (uint8_t i = 0; i < len; i++) {
            tx.data[i] = data[i];
        }
    }

    return (twai_transmit(&tx, pdMS_TO_TICKS(timeout_ms)) == ESP_OK);
}

bool CAN_Driver::receiveFrame(twai_message_t &rx_msg, uint32_t timeout_ms) {
    return (twai_receive(&rx_msg, pdMS_TO_TICKS(timeout_ms)) == ESP_OK);
}

bool CAN_Driver::waitResponse(uint32_t response_id, uint8_t expected_val, uint32_t timeout_ms) {
    uint32_t start_time = millis();
    while (millis() - start_time < timeout_ms) {
        twai_message_t rx;
        if (receiveFrame(rx, 10)) {
            if (rx.identifier == response_id) {
                if (rx.data_length_code > 0 && rx.data[0] == expected_val) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    }
    return false; // Timeout
}

bool CAN_Driver::sendWithAck(uint32_t tx_id, const uint8_t* data, uint8_t len,
                             uint32_t ack_id, uint8_t expected_ack,
                             uint32_t timeout_ms, uint8_t max_retries) {
    for (uint8_t attempt = 1; attempt <= max_retries; attempt++) {
        clearRxQueue();
        if (sendFrame(tx_id, data, len)) {
            if (waitResponse(ack_id, expected_ack, timeout_ms)) {
                return true;
            }
        }
        delay(30);
    }
    return false;
}

