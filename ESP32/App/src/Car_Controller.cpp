#include "Car_Controller.h"

Car_Controller::Car_Controller(CAN_Driver &can_driver)
    : _can(can_driver),
      _currentMode(CarMode::MODE_MOTOR),
      _lastAction("STOP"),
      _lastCanId(0x040),
      _ledState(false),
      _hornState(false) {}

void Car_Controller::begin() {
    stop();
}

bool Car_Controller::sendCanCommand(uint32_t can_id, const String &action_name) {
    _lastAction = action_name;
    _lastCanId = can_id;
    bool ok = _can.sendFrame(can_id, nullptr, 0);
    Serial.printf("[CAR] CAN ID: 0x%03X -> %s [%s]\n", can_id, action_name.c_str(), ok ? "SUCCESS" : "FAILED");
    return ok;
}

// ==================== Nhóm 0: An toàn & Chuyển chế độ ====================
bool Car_Controller::emergencyStop() {
    stop();
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
    stop();
    Serial.println("[CAR] Gui lenh Soft-Reset STM32 vao Bootloader (CAN 0x600)...");
    const uint8_t magicKey[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    if (_can.sendFrame(CAN_ID_REQ_BOOT, magicKey, 4)) {
        _lastAction = "TRIGGER_BOOTLOADER";
        return true;
    }
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

bool Car_Controller::toggleLed() {
    return _ledState ? ledOff() : ledOn();
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
    // NW: Đảo theo thực tế kiểm thử xe
    return sendCanCommand(CMD_MOTOR_RIGHT_FWD, "NW");
}

bool Car_Controller::leftWheelBackward() {
    return sendCanCommand(CMD_MOTOR_LEFT_REV, "SW");
}

bool Car_Controller::rightWheelForward() {
    // NE: Đảo theo thực tế kiểm thử xe
    return sendCanCommand(CMD_MOTOR_LEFT_FWD, "NE");
}

bool Car_Controller::rightWheelBackward() {
    return sendCanCommand(CMD_MOTOR_RIGHT_REV, "SE");
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

// ==================== Nhóm 3: Còi báo hiệu ====================
bool Car_Controller::hornOn() {
    _hornState = true;
    return sendCanCommand(CMD_HORN_ON, "HORN_ON");
}

bool Car_Controller::hornOff() {
    _hornState = false;
    return sendCanCommand(CMD_HORN_OFF, "HORN_OFF");
}

// ==================== Bộ điều phối lệnh chuẩn Opcode CAN ID ====================
bool Car_Controller::executeCanId(uint16_t can_id) {
    String action = "CMD_0x" + String(can_id, HEX);

    switch (can_id) {
        // Nhóm 0
        case CMD_EMERGENCY_STOP:
            action = "ESTOP";
            stop();
            break;
        case CMD_MODE_MOTOR:
            action = "MODE_MOTOR";
            _currentMode = CarMode::MODE_MOTOR;
            break;
        case CMD_MODE_SERVO:
            action = "MODE_SERVO";
            _currentMode = CarMode::MODE_SERVO;
            break;
        case CMD_LED_ON:
            action = "LED_ON";
            _ledState = true;
            break;
        case CMD_LED_OFF:
            action = "LED_OFF";
            _ledState = false;
            break;
        case CMD_WIFI_CONNECTED:
            action = "WIFI_CONNECTED";
            break;

        // Nhóm 1: Bánh xe
        case CMD_MOTOR_STOP:       action = "MOTOR_STOP"; break;
        case CMD_MOTOR_FORWARD:    action = "FORWARD"; break;
        case CMD_MOTOR_BACKWARD:   action = "BACKWARD"; break;
        case CMD_MOTOR_TURN_LEFT:  action = "LEFT"; break;
        case CMD_MOTOR_TURN_RIGHT: action = "RIGHT"; break;
        case CMD_MOTOR_LEFT_FWD:   action = "LEFT_FWD"; break;
        case CMD_MOTOR_LEFT_REV:   action = "LEFT_REV"; break;
        case CMD_MOTOR_RIGHT_FWD:  action = "RIGHT_FWD"; break;
        case CMD_MOTOR_RIGHT_REV:  action = "RIGHT_REV"; break;

        // Nhóm 2: Servo
        case CMD_SERVO_STOP:       action = "SERVO_STOP"; break;
        case CMD_SERVO_ARM_UP:     action = "ARM_UP"; break;
        case CMD_SERVO_ARM_DOWN:   action = "ARM_DOWN"; break;
        case CMD_SERVO_ARM_EXTEND: action = "ARM_EXT"; break;
        case CMD_SERVO_ARM_RETRACT:action = "ARM_RET"; break;

        // Nhóm 3: Còi
        case CMD_HORN_ON:          action = "HORN_ON"; _hornState = true; break;
        case CMD_HORN_OFF:         action = "HORN_OFF"; _hornState = false; break;

        default:
            break;
    }

    return sendCanCommand(can_id, action);
}

bool Car_Controller::executeCommand(const String &cmd) {
    String c = cmd;
    c.trim();

    // 1. Số Hex dạng "0x041"
    if (c.startsWith("0x") || c.startsWith("0X")) {
        uint16_t id = (uint16_t)strtoul(c.c_str(), nullptr, 16);
        return executeCanId(id);
    }

    // 2. An toàn & Chế độ
    if (c.equalsIgnoreCase("emergency_stop") || c.equalsIgnoreCase("estop")) {
        return emergencyStop();
    }
    if (c.equalsIgnoreCase("mode_motor")) {
        return setMode(CarMode::MODE_MOTOR);
    }
    if (c.equalsIgnoreCase("mode_servo")) {
        return setMode(CarMode::MODE_SERVO);
    }
    if (c.equalsIgnoreCase("mode_toggle")) {
        return setMode((_currentMode == CarMode::MODE_MOTOR) ? CarMode::MODE_SERVO : CarMode::MODE_MOTOR);
    }
    if (c.equalsIgnoreCase("bootloader") || c.equalsIgnoreCase("ota_boot")) {
        return triggerBootloader();
    }
    if (c.equalsIgnoreCase("led_on")) {
        return ledOn();
    }
    if (c.equalsIgnoreCase("led_off")) {
        return ledOff();
    }
    if (c.equalsIgnoreCase("led_toggle")) {
        return toggleLed();
    }

    // 3. Còi báo hiệu
    if (c.equalsIgnoreCase("horn_on") || c.equalsIgnoreCase("horn") || c.equalsIgnoreCase("h")) {
        return hornOn();
    }
    if (c.equalsIgnoreCase("horn_off")) {
        return hornOff();
    }

    // 4. Lệnh dừng chung
    if (c.equalsIgnoreCase("stop") || c.equalsIgnoreCase("space") || c.equalsIgnoreCase("x") || c == " ") {
        return (_currentMode == CarMode::MODE_MOTOR) ? stop() : armStop();
    }

    // 5. Điều khiển Bánh xe (Hỗ trợ cả tên đầy đủ và phím tắt F, B, L, R, W, A, S, D)
    if (c.equalsIgnoreCase("forward") || c.equalsIgnoreCase("f") || c.equalsIgnoreCase("w")) return moveForward();
    if (c.equalsIgnoreCase("backward") || c.equalsIgnoreCase("b") || c.equalsIgnoreCase("s")) return moveBackward();
    if (c.equalsIgnoreCase("left") || c.equalsIgnoreCase("l") || c.equalsIgnoreCase("a"))    return turnLeft();
    if (c.equalsIgnoreCase("right") || c.equalsIgnoreCase("r") || c.equalsIgnoreCase("d"))   return turnRight();

    // 4 góc chéo
    if (c.equalsIgnoreCase("left_fwd") || c.equalsIgnoreCase("q") || c.equalsIgnoreCase("nw"))  return leftWheelForward();
    if (c.equalsIgnoreCase("right_fwd") || c.equalsIgnoreCase("e") || c.equalsIgnoreCase("ne")) return rightWheelForward();
    if (c.equalsIgnoreCase("left_rev") || c.equalsIgnoreCase("z") || c.equalsIgnoreCase("sw"))  return leftWheelBackward();
    if (c.equalsIgnoreCase("right_rev") || c.equalsIgnoreCase("c") || c.equalsIgnoreCase("se")) return rightWheelBackward();

    // 6. Điều khiển Tay gắp
    if (c.equalsIgnoreCase("arm_up"))      return armUp();
    if (c.equalsIgnoreCase("arm_down"))    return armDown();
    if (c.equalsIgnoreCase("arm_extend"))  return armExtend();
    if (c.equalsIgnoreCase("arm_retract")) return armRetract();
    if (c.equalsIgnoreCase("arm_stop"))    return armStop();

    Serial.printf("[CAR] Khong nhan dien duoc lenh: %s\n", c.c_str());
    return false;
}

String Car_Controller::getModeString() const {
    return (_currentMode == CarMode::MODE_MOTOR) ? "MOTOR" : "SERVO";
}

String Car_Controller::getStatusJson() const {
    char buf[256];
    snprintf(buf, sizeof(buf),
             "{\"mode\":\"%s\",\"action\":\"%s\",\"last_id\":\"0x%03X\",\"led\":%s,\"horn\":%s,\"uptime_s\":%lu}",
             (_currentMode == CarMode::MODE_MOTOR) ? "MOTOR" : "SERVO",
             _lastAction.c_str(),
             (unsigned int)_lastCanId,
             _ledState ? "true" : "false",
             _hornState ? "true" : "false",
             millis() / 1000UL);
    return String(buf);
}
