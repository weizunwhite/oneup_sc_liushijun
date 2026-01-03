/**
 * @file path.cpp
 * @brief 路径示教模块实现
 */

#include "path.h"
#include "motor.h"

Path path;

void Path::begin() {
    _stepCount = 0;
    _isRecording = false;
    _isReturning = false;
    _currentReturnStep = -1;
}

void Path::startRecording() {
    cancelReturning();
    _stepCount = 0;
    _isRecording = true;
    _lastAction = ACTION_STOP;
    _lastActionTime = millis();
    DEBUG_PRINTLN("开始路径录制");
}

void Path::stopRecording() {
    if (!_isRecording) return;

    // 记录最后一步，确保上一动作的持续时间被保存
    recordStep(ACTION_STOP);
    _isRecording = false;
    _lastAction = ACTION_STOP;
    _lastActionTime = millis();

    DEBUG_PRINTF("录制结束，共 %d 步\n", _stepCount);
}

void Path::recordStep(PathActionType action) {
    if (!_isRecording) return;

    // 如果动作改变，保存上一段动作的持续时间
    if (action != _lastAction) {
        unsigned long duration = millis() - _lastActionTime;

        // 忽略太短的动作 (<100ms)
        if (duration > 100 && _stepCount < PATH_MAX_STEPS) {
            _steps[_stepCount].action = _lastAction;
            _steps[_stepCount].duration = duration;
            _stepCount++;
            DEBUG_PRINTF("记录步骤 %d: Act=%d, Time=%lu\n", _stepCount, _lastAction, duration);
        }

        _lastAction = action;
        _lastActionTime = millis();
    }
}

bool Path::startReturning() {
    if (_stepCount == 0) {
        DEBUG_PRINTLN("?????????");
        _isReturning = false;
        return false;
    }

    if (_isRecording) {
        stopRecording();
    }

    _isReturning = true;
    _currentReturnStep = 0; // ????????
    _returnStepStartTime = millis();

    PathStep step = _steps[_currentReturnStep];
    executeAction(step.action);

    DEBUG_PRINTLN("????????..");
    return true;
}





void Path::cancelReturning() {
    if (!_isReturning) return;
    _isReturning = false;
    _currentReturnStep = -1;
    motor.stop();
    DEBUG_PRINTLN("取消自动归位");
}

bool Path::updateReturning() {
    if (!_isReturning) return false;

    if (_currentReturnStep >= _stepCount) {
        motor.stop();
        _isReturning = false;
        DEBUG_PRINTLN("????");
        return false;
    }

    PathStep currentStep = _steps[_currentReturnStep];

    // ????????????
    if (millis() - _returnStepStartTime >= currentStep.duration) {
        _currentReturnStep++;

        if (_currentReturnStep < _stepCount) {
            _returnStepStartTime = millis();
            PathStep nextStep = _steps[_currentReturnStep];
            executeAction(nextStep.action);

            DEBUG_PRINTF("???? %d: Act=%d\n", _currentReturnStep, nextStep.action);
        }
    }

    return true;
}





void Path::executeAction(PathActionType action) {
    switch (action) {
        case ACTION_FORWARD:  motor.forward(); break;
        case ACTION_BACKWARD: motor.backward(); break;
        case ACTION_LEFT:     motor.turnLeft(); break;
        case ACTION_RIGHT:    motor.turnRight(); break;
        case ACTION_STOP:     motor.stop(); break;
    }
}

PathActionType Path::getReverseAction(PathActionType action) {
    switch (action) {
        case ACTION_FORWARD:  return ACTION_BACKWARD;
        case ACTION_BACKWARD: return ACTION_FORWARD;
        case ACTION_LEFT:     return ACTION_RIGHT;
        case ACTION_RIGHT:    return ACTION_LEFT;
        default:              return ACTION_STOP;
    }
}
