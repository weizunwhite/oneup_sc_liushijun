/**
 * @file follow.cpp
 * @brief Follow control module
 */

#include "follow.h"
#include "motor.h"

Follow follow;

void Follow::begin() {
    stop();
    _filterInit = false;
    _lastCmd = CMD_STOP;
    _lastCmdTime = 0;
}

void Follow::update(float distance, float angle) {
    // Bang-bang control with filtering and hysteresis
    if (distance <= 0) {
        stop();
        _lastCmd = CMD_STOP;
        return;
    }

    if (!_filterInit) {
        _distanceFiltered = distance;
        _angleFiltered = angle;
        _filterInit = true;
    }

    _distanceFiltered += FOLLOW_FILTER_ALPHA * (distance - _distanceFiltered);
    _angleFiltered += FOLLOW_FILTER_ALPHA * (angle - _angleFiltered);

    float d = _distanceFiltered;
    float a = _angleFiltered;
    float absA = (a < 0) ? -a : a;

    if (d <= FOLLOW_ENABLE_DISTANCE) {
        stop();
        _lastCmd = CMD_STOP;
        return;
    }
    if (d < FOLLOW_MIN_DISTANCE) {
        stop();
        _lastCmd = CMD_STOP;
        return;
    }

    bool turning = false;
    if (_lastCmd == CMD_LEFT || _lastCmd == CMD_RIGHT) {
        turning = absA > FOLLOW_TURN_OFF;
    } else {
        turning = absA > FOLLOW_TURN_ON;
    }

    FollowCmd cmd = CMD_STOP;
    if (turning) {
        cmd = (a >= 0) ? CMD_RIGHT : CMD_LEFT;
    } else if (d > FOLLOW_DIST_TARGET + FOLLOW_DIST_DEADZONE) {
        cmd = CMD_FORWARD;
    } else {
        cmd = CMD_STOP;
    }

    unsigned long now = millis();
    if (cmd != _lastCmd && _lastCmd != CMD_STOP && (now - _lastCmdTime) < FOLLOW_CMD_HOLD_MS) {
        cmd = _lastCmd;
    }

    switch (cmd) {
        case CMD_FORWARD:
            motor.forward();
            break;
        case CMD_LEFT:
            motor.turnLeft();
            break;
        case CMD_RIGHT:
            motor.turnRight();
            break;
        default:
            stop();
            break;
    }

    if (cmd != _lastCmd) {
        _lastCmd = cmd;
        _lastCmdTime = now;
    }

    // Serial.printf("Follow: Dist=%.0f, Ang=%.1f\n", distance, angle);
}

void Follow::stop() {
    motor.stop();
}
