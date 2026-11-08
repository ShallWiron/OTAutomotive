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

// ==================== Bộ điều phối chuẩn Opcode CAN ID ====================
bool Car_Controller::executeCanId(uint16_t can_id) {
    if (can_id == CAN_ID_REQ_BOOT) {
        return triggerBootloader();
    }

    String actionName = "UNKNOWN_CMD";

    switch (can_id) {
        case CMD_EMERGENCY_STOP:    actionName = "EMERGENCY_STOP"; break;
        case CMD_MODE_MOTOR:        actionName = "MODE_MOTOR"; _currentMode = CarMode::MODE_MOTOR; break;
        case CMD_MODE_SERVO:        actionName = "MODE_SERVO"; _currentMode = CarMode::MODE_SERVO; break;

        case CMD_MOTOR_STOP:        actionName = "MOTOR_STOP"; break;
        case CMD_MOTOR_FORWARD:     actionName = "MOTOR_FORWARD"; break;
        case CMD_MOTOR_BACKWARD:    actionName = "MOTOR_BACKWARD"; break;
        case CMD_MOTOR_TURN_LEFT:   actionName = "MOTOR_TURN_LEFT"; break;
        case CMD_MOTOR_TURN_RIGHT:  actionName = "MOTOR_TURN_RIGHT"; break;
        case CMD_MOTOR_LEFT_FWD:    actionName = "MOTOR_LEFT_FWD"; break;
        case CMD_MOTOR_LEFT_REV:    actionName = "MOTOR_LEFT_REV"; break;
        case CMD_MOTOR_RIGHT_FWD:   actionName = "MOTOR_RIGHT_FWD"; break;
        case CMD_MOTOR_RIGHT_REV:   actionName = "MOTOR_RIGHT_REV"; break;

        case CMD_SERVO_STOP:        actionName = "SERVO_STOP"; break;
        case CMD_SERVO_ARM_UP:      actionName = "SERVO_ARM_UP"; break;
        case CMD_SERVO_ARM_DOWN:    actionName = "SERVO_ARM_DOWN"; break;
        case CMD_SERVO_ARM_EXTEND:  actionName = "SERVO_ARM_EXTEND"; break;
        case CMD_SERVO_ARM_RETRACT: actionName = "SERVO_ARM_RETRACT"; break;
        case CMD_SERVO_GRIP_OPEN:   actionName = "SERVO_GRIP_OPEN"; break;
        case CMD_SERVO_GRIP_CLOSE:  actionName = "SERVO_GRIP_CLOSE"; break;

        default:
            actionName = "CMD_0x" + String(can_id, HEX);
            break;
    }

    return sendCanCommand(can_id, actionName);
}

bool Car_Controller::executeCommand(const String &cmd) {
    String c = cmd;
    c.trim();

    // 1. Nếu là chuỗi số Hex dạng "0x041" hoặc số nguyên
    if (c.startsWith("0x") || c.startsWith("0X")) {
        uint16_t id = (uint16_t)strtoul(c.c_str(), nullptr, 16);
        return executeCanId(id);
    }

    // 2. Nhóm An toàn & Chuyển chế độ
    if (c.equalsIgnoreCase("emergency_stop") || c.equalsIgnoreCase("estop")) {
        return emergencyStop();
    }
    if (c.equalsIgnoreCase("mode_motor")) {
        return setMode(CarMode::MODE_MOTOR);
    }
    if (c.equalsIgnoreCase("mode_servo")) {
        return setMode(CarMode::MODE_SERVO);
    }
    if (c.equalsIgnoreCase("bootloader") || c.equalsIgnoreCase("ota_boot")) {
        return triggerBootloader();
    }

    // 3. Kẹp gắp
    if (c.equalsIgnoreCase("grip_open")) {
        return gripperOpen();
    }
    if (c.equalsIgnoreCase("grip_close")) {
        return gripperClose();
    }

    // 4. Lệnh dừng chung
    if (c.equalsIgnoreCase("stop") || c.equalsIgnoreCase("space") || c.equalsIgnoreCase("s") || c.equalsIgnoreCase("x") || c == " ") {
        return (_currentMode == CarMode::MODE_MOTOR) ? stop() : armStop();
    }

    // 5. Điều khiển Bánh xe (Hỗ trợ cả tên đầy đủ và phím tắt F, B, L, R, W, A, S, D)
    if (c.equalsIgnoreCase("forward") || c.equalsIgnoreCase("f") || c.equalsIgnoreCase("w")) return moveForward();
    if (c.equalsIgnoreCase("backward") || c.equalsIgnoreCase("b"))                            return moveBackward();
    if (c.equalsIgnoreCase("left") || c.equalsIgnoreCase("l") || c.equalsIgnoreCase("a"))    return turnLeft();
    if (c.equalsIgnoreCase("right") || c.equalsIgnoreCase("r") || c.equalsIgnoreCase("d"))   return turnRight();
    
    // 4 góc chéo
    if (c.equalsIgnoreCase("left_fwd") || c.equalsIgnoreCase("g") || c.equalsIgnoreCase("q"))  return leftWheelForward();
    if (c.equalsIgnoreCase("right_fwd") || c.equalsIgnoreCase("i") || c.equalsIgnoreCase("e")) return rightWheelForward();
    if (c.equalsIgnoreCase("left_rev") || c.equalsIgnoreCase("h") || c.equalsIgnoreCase("z"))  return leftWheelBackward();
    if (c.equalsIgnoreCase("right_rev") || c.equalsIgnoreCase("j") || c.equalsIgnoreCase("c"))  return rightWheelBackward();

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
    char buf[128];
    snprintf(buf, sizeof(buf),
             "{\"mode\":\"%s\",\"last_action\":\"%s\",\"uptime_s\":%lu}",
             getModeString().c_str(), _lastAction.c_str(),
             millis() / 1000UL);
    return String(buf);
}
