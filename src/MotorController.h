#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "ControlState.h"

class MotorController
{
public:
    void begin();
    void update(const ControlState &control);
    void stop();

private:
    bool initialized = false;
};

#endif