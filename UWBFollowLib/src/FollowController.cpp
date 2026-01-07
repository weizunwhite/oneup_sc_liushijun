/**
 * @file FollowController.cpp
 * @brief Follow controller implementation
 */

#include "FollowController.h"

FollowController::FollowController(MotorInterface* motor) :
    _motor(motor),
    _debug(false),
    _distanceFiltered(0.0f),
    _angleFiltered(0.0f),
    _filterInit(false),
    _lastCmd(CMD_STOP),
    _lastCmdTime(0) {
}

void FollowController::begin(const FollowConfig& config) {
    _config = config;
    _filterInit = false;
    _lastCmd = CMD_STOP;
    _lastCmdTime = 0;
    stop();

    if (_debug) {
        Serial.println("[FollowController] Initialized");
        Serial.printf("  Target distance: %.1f cm\n", _config.targetDistance);
        Serial.printf("  Turn angles: %.1f/%.1f deg\n", _config.turnOnAngle, _config.turnOffAngle);
    }
}

void FollowController::update(float distance, float angle) {
    // Check validity
    if (distance <= 0) {
        stop();
        _lastCmd = CMD_STOP;
        return;
    }

    // Initialize filter
    if (!_filterInit) {
        _distanceFiltered = distance;
        _angleFiltered = angle;
        _filterInit = true;
    }

    // Low-pass filter
    _distanceFiltered += _config.filterAlpha * (distance - _distanceFiltered);
    _angleFiltered += _config.filterAlpha * (angle - _angleFiltered);

    float d = _distanceFiltered;
    float a = _angleFiltered;
    float absA = (a < 0) ? -a : a;

    // Safety: stop if too close
    if (d <= _config.enableDistance) {
        stop();
        _lastCmd = CMD_STOP;
        if (_debug) {
            Serial.printf("[Follow] Too close: %.1f cm\n", d);
        }
        return;
    }

    if (d < _config.minDistance) {
        stop();
        _lastCmd = CMD_STOP;
        if (_debug) {
            Serial.printf("[Follow] Below minimum: %.1f cm\n", d);
        }
        return;
    }

    // Hysteresis for turning
    bool turning = false;
    if (_lastCmd == CMD_LEFT || _lastCmd == CMD_RIGHT) {
        // Already turning, keep turning until angle < turnOffAngle
        turning = absA > _config.turnOffAngle;
    } else {
        // Not turning, start turning if angle > turnOnAngle
        turning = absA > _config.turnOnAngle;
    }

    // Decide command
    FollowCommand cmd = CMD_STOP;
    if (turning) {
        cmd = (a >= 0) ? CMD_RIGHT : CMD_LEFT;
    } else if (d > _config.targetDistance + _config.distanceDeadzone) {
        cmd = CMD_FORWARD;
    } else {
        cmd = CMD_STOP;
    }

    // Command hold: prevent rapid switching
    unsigned long now = millis();
    if (cmd != _lastCmd && _lastCmd != CMD_STOP && (now - _lastCmdTime) < _config.commandHoldMs) {
        cmd = _lastCmd;  // Hold previous command
    }

    // Execute command
    switch (cmd) {
        case CMD_FORWARD:
            _motor->forward();
            if (_debug && cmd != _lastCmd) {
                Serial.println("[Follow] Forward");
            }
            break;
        case CMD_LEFT:
            _motor->turnLeft();
            if (_debug && cmd != _lastCmd) {
                Serial.println("[Follow] Turn Left");
            }
            break;
        case CMD_RIGHT:
            _motor->turnRight();
            if (_debug && cmd != _lastCmd) {
                Serial.println("[Follow] Turn Right");
            }
            break;
        default:
            stop();
            if (_debug && cmd != _lastCmd) {
                Serial.println("[Follow] Stop");
            }
            break;
    }

    // Update state
    if (cmd != _lastCmd) {
        _lastCmd = cmd;
        _lastCmdTime = now;
    }
}

void FollowController::stop() {
    _motor->stop();
}
