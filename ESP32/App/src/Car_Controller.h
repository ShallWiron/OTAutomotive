#ifndef CAR_CONTROLLER_H
#define CAR_CONTROLLER_H

#include <Arduino.h>
#include "CAN_Driver.h"
#include "Car_Protocol.h"

enum class CarMode {
    MODE_MOTOR = 0,
    MODE_SERVO = 1
};

class Car_Controller {
public:
    explicit Car_Controller(CAN_Driver &can_driver);

    void begin();

    bool emergencyStop();
    bool setMode(CarMode mode);
    bool triggerBootloader();
    bool ledOn();
    bool ledOff();
    bool toggleLed();
    bool isLedOn() const { return _ledState; }

    bool moveForward();
    bool moveBackward();
    bool turnLeft();
    bool turnRight();
    bool stop();
    bool leftWheelForward();
    bool leftWheelBackward();
    bool rightWheelForward();
    bool rightWheelBackward();

    bool armUp();
    bool armDown();
    bool armExtend();
    bool armRetract();
    bool armStop();

    bool hornOn();
    bool hornOff();
    bool isHornOn() const { return _hornState; }

    bool executeCanId(uint16_t can_id);
    bool executeCommand(const String &cmd);

    CarMode getMode() const { return _currentMode; }
    String getModeString() const;
    String getLastAction() const { return _lastAction; }
    String getStatusJson() const;

private:
    CAN_Driver &_can;
    CarMode _currentMode;
    String _lastAction;
    uint32_t _lastCanId = 0x040;
    bool _ledState;
    bool _hornState = false;

    bool sendCanCommand(uint32_t can_id, const String &action_name);
};

#endif
