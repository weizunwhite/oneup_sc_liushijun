/**
 * @file MotorInterface.h
 * @brief Motor control interface - abstract base class
 * @details User should implement this interface for their specific motor driver
 */

#ifndef MOTOR_INTERFACE_H
#define MOTOR_INTERFACE_H

/**
 * @brief Abstract motor interface
 * @details Implement this interface to control your specific motor hardware
 */
class MotorInterface {
public:
    virtual ~MotorInterface() {}

    /**
     * @brief Move forward
     */
    virtual void forward() = 0;

    /**
     * @brief Move backward
     */
    virtual void backward() = 0;

    /**
     * @brief Turn left
     */
    virtual void turnLeft() = 0;

    /**
     * @brief Turn right
     */
    virtual void turnRight() = 0;

    /**
     * @brief Stop all motors
     */
    virtual void stop() = 0;
};

#endif // MOTOR_INTERFACE_H
