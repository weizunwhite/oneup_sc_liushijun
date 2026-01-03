/**
 * @file motor.h
 * @brief Motor driver - PWM speed control
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "config.h"

class Motor {
public:
    void begin();
    void setSpeed(uint8_t forward, uint8_t turn);
    void forward();
    void backward();
    void turnLeft();
    void turnRight();
    void stop();
};

extern Motor motor;

#endif // MOTOR_H
