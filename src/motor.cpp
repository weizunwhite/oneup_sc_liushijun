/**
 * @file motor.cpp
 * @brief Motor driver - PWM speed control
 */

#include "motor.h"

Motor motor;

namespace {
constexpr uint8_t CH_LEFT_IN1 = 0;
constexpr uint8_t CH_LEFT_IN2 = 1;
constexpr uint8_t CH_RIGHT_IN1 = 2;
constexpr uint8_t CH_RIGHT_IN2 = 3;

void setupPwm(uint8_t channel, uint8_t pin) {
    ledcSetup(channel, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcAttachPin(pin, channel);
}
}  // namespace

void Motor::begin() {
    setupPwm(CH_LEFT_IN1, MOTOR_LEFT_IN1);
    setupPwm(CH_LEFT_IN2, MOTOR_LEFT_IN2);
    setupPwm(CH_RIGHT_IN1, MOTOR_RIGHT_IN1);
    setupPwm(CH_RIGHT_IN2, MOTOR_RIGHT_IN2);

    stop();
    DEBUG_PRINTLN("Motor init done (PWM)");
}

void Motor::forward() {
    ledcWrite(CH_LEFT_IN1, MOTOR_SPEED_FORWARD);
    ledcWrite(CH_LEFT_IN2, 0);
    ledcWrite(CH_RIGHT_IN1, MOTOR_SPEED_FORWARD);
    ledcWrite(CH_RIGHT_IN2, 0);
}

void Motor::backward() {
    ledcWrite(CH_LEFT_IN1, 0);
    ledcWrite(CH_LEFT_IN2, MOTOR_SPEED_FORWARD);
    ledcWrite(CH_RIGHT_IN1, 0);
    ledcWrite(CH_RIGHT_IN2, MOTOR_SPEED_FORWARD);
}

void Motor::turnLeft() {
    ledcWrite(CH_LEFT_IN1, 0);
    ledcWrite(CH_LEFT_IN2, MOTOR_SPEED_TURN);
    ledcWrite(CH_RIGHT_IN1, MOTOR_SPEED_TURN);
    ledcWrite(CH_RIGHT_IN2, 0);
}

void Motor::turnRight() {
    ledcWrite(CH_LEFT_IN1, MOTOR_SPEED_TURN);
    ledcWrite(CH_LEFT_IN2, 0);
    ledcWrite(CH_RIGHT_IN1, 0);
    ledcWrite(CH_RIGHT_IN2, MOTOR_SPEED_TURN);
}

void Motor::stop() {
    ledcWrite(CH_LEFT_IN1, 0);
    ledcWrite(CH_LEFT_IN2, 0);
    ledcWrite(CH_RIGHT_IN1, 0);
    ledcWrite(CH_RIGHT_IN2, 0);
}
