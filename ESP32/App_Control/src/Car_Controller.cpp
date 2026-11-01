#include "Car_Controller.h"

Car_Controller::Car_Controller(CAN_Driver &can_driver)
    : _can(can_driver), _currentMode(CarMode::MODE_MOTOR), _lastAction("IDLE") {}

void Car_Controller::begin() {
    _currentMode = CarMode::MODE_MOTOR;
    _lastAction = "STOP";
    stop();
}

bool Car_Controller::sendCanCommand(uint32_t can_id, const String &action_name) {
    bool ok = _can.sendFrame(can_id, nullptr, 0);
    if (ok) {
        _lastAction = action_name;
    }
    Serial.printf("[CAR] CAN ID: 0x%03X -> %s [%s]\n", can_id, action_name.c_str(), ok ? "SUCCESS" : "FAILED");
    return ok;
}

// ==================== Nhóm 0: An toàn & Hệ thống ====================
bool Car_Controller::emergencyStop() {
    return sendCanCommand(CMD_EMERGENCY_STOP, "EMERGENCY_STOP");
}

bool Car_Controller::setMode(CarMode mode) {
    _currentMode = mode;
    if (mode == CarMode::MODE_MOTOR) {
        return sendCanCommand(CMD_MODE_MOTOR, "MODE_MOTOR");
    } else {
        return sendCanCommand(CMD_MODE_SERVO, "MODE_SERVO");
    }
}

bool Car_Controller::triggerBootloader() {
    const uint8_t security_key[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    Serial.println("[CAR] Gui lenh REQ_BOOT (0x600) sang STM32...");
    bool ok = _can.sendFrame(CAN_ID_REQ_BOOT, security_key, 4);
    if (!ok) return false;

    // Chờ ACK 0x7C0 từ STM32
    if (_can.waitResponse(CAN_ID_RESPONSE, 0x00, 500)) {
        Serial.println("[CAR] STM32 da ACK -> Dang soft-reset sang Bootloader!");
        _lastAction = "BOOTLOADER_TRIGGERED";
        return true;
    }
    Serial.println("[CAR] Khong nhan duoc phan hoi tu STM32!");
    return false;
}

bool Car_Controller::ledOn() {
    _ledState = true;
    return sendCanCommand(CMD_LED_ON, "LED_ON");
}

bool Car_Controller::ledOff() {
    _ledState = false;
    return sendCanCommand(CMD_LED_OFF, "LED_OFF");
}

bool Car_Controller::relayOn() {
    _relayState = true;
    return sendCanCommand(CMD_RELAY_ON, "RELAY_ON");
}

bool Car_Controller::relayOff() {
    _relayState = false;
    return sendCanCommand(CMD_RELAY_OFF, "RELAY_OFF");
}

bool Car_Controller::buzzerBeep() {
    return sendCanCommand(CMD_BUZZER_BEEP, "BUZZER");
}

// ==================== Nhóm 1: Điều khiển Bánh xe ====================
bool Car_Controller::moveForward() {
    return sendCanCommand(CMD_MOTOR_FORWARD, "FORWARD");
}

bool Car_Controller::moveBackward() {
    return sendCanCommand(CMD_MOTOR_BACKWARD, "BACKWARD");
}

bool Car_Controller::turnLeft() {
    return sendCanCommand(CMD_MOTOR_TURN_LEFT, "TURN_LEFT");
}

bool Car_Controller::turnRight() {
    return sendCanCommand(CMD_MOTOR_TURN_RIGHT, "TURN_RIGHT");
}

bool Car_Controller::stop() {
    return sendCanCommand(CMD_MOTOR_STOP, "STOP");
}

bool Car_Controller::leftWheelForward() {
    return sendCanCommand(CMD_MOTOR_LEFT_FWD, "LEFT_FWD");
}

bool Car_Controller::leftWheelBackward() {
    return sendCanCommand(CMD_MOTOR_LEFT_REV, "LEFT_REV");
}

bool Car_Controller::rightWheelForward() {
    return sendCanCommand(CMD_MOTOR_RIGHT_FWD, "RIGHT_FWD");
}

bool Car_Controller::rightWheelBackward() {
    return sendCanCommand(CMD_MOTOR_RIGHT_REV, "RIGHT_REV");
}

// ==================== Nhóm 2: Điều khiển Cánh tay & Kẹp ====================
bool Car_Controller::armUp() {
    return sendCanCommand(CMD_SERVO_ARM_UP, "ARM_UP");
}

bool Car_Controller::armDown() {
    return sendCanCommand(CMD_SERVO_ARM_DOWN, "ARM_DOWN");
}

bool Car_Controller::armExtend() {
    return sendCanCommand(CMD_SERVO_ARM_EXTEND, "ARM_EXTEND");
}

bool Car_Controller::armRetract() {
    return sendCanCommand(CMD_SERVO_ARM_RETRACT, "ARM_RETRACT");
}

bool Car_Controller::armStop() {
    return sendCanCommand(CMD_SERVO_STOP, "ARM_STOP");
}

bool Car_Controller::gripperOpen() {
    return sendCanCommand(CMD_SERVO_GRIP_OPEN, "GRIP_OPEN");
}

bool Car_Controller::gripperClose() {
    return sendCanCommand(CMD_SERVO_GRIP_CLOSE, "GRIP_CLOSE");
}

// ==================== Bộ điều phối lệnh ====================
bool Car_Controller::executeCommand(const String &cmd) {
    String c = cmd;
    c.trim();

    // 1. Nhóm An toàn & Chuyển chế độ
    if (c.equalsIgnoreCase("emergency_stop") || c.equalsIgnoreCase("estop")) {
        return emergencyStop();
    }
    if (c.equalsIgnoreCase("mode_motor") || c == "x") {
        return setMode(CarMode::MODE_MOTOR);
    }
    if (c.equalsIgnoreCase("mode_servo") || c == "X") {
        return setMode(CarMode::MODE_SERVO);
    }
    if (c.equalsIgnoreCase("bootloader") || c.equalsIgnoreCase("ota_boot")) {
        return triggerBootloader();
    }

    // 2. Kẹp gắp, Đèn, Relay, Buzzer (Khả dụng ở cả 2 chế độ)
    if (c.equalsIgnoreCase("grip_open") || c == "W") {
        return gripperOpen();
    }
    if (c.equalsIgnoreCase("grip_close") || c == "w") {
        return gripperClose();
    }
    if (c.equalsIgnoreCase("led_on") || c == "u") {
        return ledOn();
    }
    if (c.equalsIgnoreCase("led_off") || c == "U") {
        return ledOff();
    }
    if (c.equalsIgnoreCase("relay_on") || c == "V") {
        return relayOn();
    }
    if (c.equalsIgnoreCase("relay_off") || c == "v") {
        return relayOff();
    }
    if (c.equalsIgnoreCase("buzzer") || c.equalsIgnoreCase("beep")) {
        return buzzerBeep();
    }

    // 3. Lệnh dừng chung
    if (c.equalsIgnoreCase("stop") || c == "S" || c == "space") {
        return (_currentMode == CarMode::MODE_MOTOR) ? stop() : armStop();
    }

    // 4. Các lệnh trực tiếp hoặc theo chế độ
    if (_currentMode == CarMode::MODE_MOTOR) {
        if (c.equalsIgnoreCase("forward")   || c == "F" || c == "w_key") return moveForward();
        if (c.equalsIgnoreCase("backward")  || c == "B" || c == "s_key") return moveBackward();
        if (c.equalsIgnoreCase("left")      || c.equalsIgnoreCase("spin_left")  || c == "L" || c == "a_key") return turnLeft();
        if (c.equalsIgnoreCase("right")     || c.equalsIgnoreCase("spin_right") || c == "R" || c == "d_key") return turnRight();
        if (c.equalsIgnoreCase("left_fwd")  || c == "G") return leftWheelForward();
        if (c.equalsIgnoreCase("left_rev")  || c == "H") return leftWheelBackward();
        if (c.equalsIgnoreCase("right_fwd") || c == "I") return rightWheelForward();
        if (c.equalsIgnoreCase("right_rev") || c == "J") return rightWheelBackward();
    } else {
        // Chế độ Servo
        if (c.equalsIgnoreCase("arm_up")      || c == "F") return armUp();
        if (c.equalsIgnoreCase("arm_down")    || c == "B") return armDown();
        if (c.equalsIgnoreCase("arm_extend")  || c == "L") return armExtend();
        if (c.equalsIgnoreCase("arm_retract") || c == "R") return armRetract();
        if (c.equalsIgnoreCase("arm_stop"))               return armStop();

        // Vẫn cho phép điều khiển xe khẩn cấp ngay cả khi đang ở tab servo
        if (c.equalsIgnoreCase("forward"))    return moveForward();
        if (c.equalsIgnoreCase("backward"))   return moveBackward();
        if (c.equalsIgnoreCase("spin_left"))  return turnLeft();
        if (c.equalsIgnoreCase("spin_right")) return turnRight();
    }

    Serial.printf("[CAR] Khong nhan dien duoc lenh: %s\n", c.c_str());
    return false;
}

String Car_Controller::getModeString() const {
    return (_currentMode == CarMode::MODE_MOTOR) ? "MOTOR" : "SERVO";
}

String Car_Controller::getStatusJson() const {
    char buf[192];
    snprintf(buf, sizeof(buf),
             "{\"mode\":\"%s\",\"last_action\":\"%s\",\"led\":%s,\"relay\":%s,\"uptime_s\":%lu}",
             getModeString().c_str(), _lastAction.c_str(),
             _ledState ? "true" : "false",
             _relayState ? "true" : "false",
             millis() / 1000UL);
    return String(buf);
}

