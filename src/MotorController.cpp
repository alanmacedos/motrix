#include <Arduino.h>
#include "MotorController.h"

// --------------------------------------------------
// MOTOR DRIVER PINS
// --------------------------------------------------

namespace
{
    constexpr uint8_t MOTOR_IN1_PIN = 26;
    constexpr uint8_t MOTOR_IN2_PIN = 27;
    constexpr uint8_t MOTOR_IN3_PIN = 14;
    constexpr uint8_t MOTOR_IN4_PIN = 25;

    constexpr uint8_t MOTOR_SLEEP_PIN = 33;

    // PWM CONFIGURATION
    constexpr uint32_t PWM_FREQUENCY = 20000;
    constexpr uint8_t PWM_RESOLUTION = 8;

    constexpr uint8_t PWM_MAX_DUTY = 255;

    // LEDC CHANNELS
    constexpr uint8_t MOTOR_IN1_CHANNEL = 0;
    constexpr uint8_t MOTOR_IN2_CHANNEL = 1;
    constexpr uint8_t MOTOR_IN3_CHANNEL = 2;
    constexpr uint8_t MOTOR_IN4_CHANNEL = 3;
}

// --------------------------------------------------
// INITIALIZATION
// --------------------------------------------------

void MotorController::begin()
{
    if (initialized)
    {
        return;
    }

    // Keep driver disabled during initialization
    pinMode(MOTOR_SLEEP_PIN, OUTPUT);
    digitalWrite(MOTOR_SLEEP_PIN, LOW);

    // Configure PWM channels
    if (
        ledcSetup(MOTOR_IN1_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION) == 0 ||
        ledcSetup(MOTOR_IN2_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION) == 0 ||
        ledcSetup(MOTOR_IN3_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION) == 0 ||
        ledcSetup(MOTOR_IN4_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION) == 0)
    {
        Serial.println("Error configuring motor PWM!");

        return;
    }

    // Attach PWM channels to GPIO pins
    ledcAttachPin(MOTOR_IN1_PIN, MOTOR_IN1_CHANNEL);
    ledcAttachPin(MOTOR_IN2_PIN, MOTOR_IN2_CHANNEL);
    ledcAttachPin(MOTOR_IN3_PIN, MOTOR_IN3_CHANNEL);
    ledcAttachPin(MOTOR_IN4_PIN, MOTOR_IN4_CHANNEL);

    initialized = true;

    // Ensure all motor inputs are inactive
    stop();

    // Enable motor driver
    digitalWrite(MOTOR_SLEEP_PIN, HIGH);

    // Wait for driver wake-up
    delay(1);

    Serial.println("Motor controller initialized!");
}

// --------------------------------------------------
// MOTOR UPDATE
// --------------------------------------------------

void MotorController::update(const ControlState &control)
{
    if (!initialized)
    {
        return;
    }

    // Motor control implementation will be added here.
}

// --------------------------------------------------
// MOTOR STOP
// --------------------------------------------------

void MotorController::stop()
{
    if (!initialized)
    {
        return;
    }

    ledcWrite(MOTOR_IN1_CHANNEL, 0);
    ledcWrite(MOTOR_IN2_CHANNEL, 0);
    ledcWrite(MOTOR_IN3_CHANNEL, 0);
    ledcWrite(MOTOR_IN4_CHANNEL, 0);
}