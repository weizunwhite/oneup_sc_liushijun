/**
 * @file FollowController.h
 * @brief Follow control algorithm
 * @details Bang-bang controller with filtering and hysteresis
 */

#ifndef FOLLOW_CONTROLLER_H
#define FOLLOW_CONTROLLER_H

#include <Arduino.h>
#include "MotorInterface.h"

/**
 * @brief Follow controller configuration
 */
struct FollowConfig {
    float targetDistance;           ///< Target follow distance (cm)
    float distanceDeadzone;         ///< Distance deadzone (cm)
    float angleDeadzone;            ///< Angle deadzone (degrees)
    float minDistance;              ///< Minimum distance - stop if closer (cm)
    float enableDistance;           ///< Distance threshold - stop if closer (cm)
    float filterAlpha;              ///< Low-pass filter coefficient (0-1)
    float turnOnAngle;              ///< Angle to start turning (degrees)
    float turnOffAngle;             ///< Angle to stop turning (degrees)
    uint32_t commandHoldMs;         ///< Minimum command duration (ms)

    /**
     * @brief Default constructor with recommended values
     */
    FollowConfig() :
        targetDistance(80.0f),
        distanceDeadzone(15.0f),
        angleDeadzone(15.0f),
        minDistance(40.0f),
        enableDistance(100.0f),
        filterAlpha(0.1f),
        turnOnAngle(35.0f),
        turnOffAngle(15.0f),
        commandHoldMs(300) {}
};

/**
 * @brief Follow controller class
 */
class FollowController {
public:
    /**
     * @brief Constructor
     * @param motor Motor interface implementation
     */
    FollowController(MotorInterface* motor);

    /**
     * @brief Initialize controller
     * @param config Configuration structure
     */
    void begin(const FollowConfig& config);

    /**
     * @brief Update follow control (call in loop)
     * @param distance Target distance (cm)
     * @param angle Target angle (degrees, 0=forward, +right/-left)
     */
    void update(float distance, float angle);

    /**
     * @brief Stop following
     */
    void stop();

    /**
     * @brief Enable/disable debug output
     * @param enable Debug flag
     */
    void setDebug(bool enable) { _debug = enable; }

    /**
     * @brief Get filtered distance
     * @return Filtered distance (cm)
     */
    float getFilteredDistance() const { return _distanceFiltered; }

    /**
     * @brief Get filtered angle
     * @return Filtered angle (degrees)
     */
    float getFilteredAngle() const { return _angleFiltered; }

private:
    enum FollowCommand {
        CMD_STOP = 0,
        CMD_FORWARD,
        CMD_LEFT,
        CMD_RIGHT
    };

    MotorInterface* _motor;
    FollowConfig _config;
    bool _debug;

    float _distanceFiltered;
    float _angleFiltered;
    bool _filterInit;
    FollowCommand _lastCmd;
    unsigned long _lastCmdTime;
};

#endif // FOLLOW_CONTROLLER_H
