/**
 * @file path.h
 * @brief 路径示教模块头文件
 */

#ifndef PATH_H
#define PATH_H

#include <Arduino.h>
#include "config.h"

// 路径动作类型
enum PathActionType {
    ACTION_STOP = 0,
    ACTION_FORWARD,
    ACTION_BACKWARD,
    ACTION_LEFT,
    ACTION_RIGHT
};

struct PathStep {
    PathActionType action;
    unsigned long duration; // ms
};

class Path {
public:
    void begin();
    
    /**
     * @brief 开始录制
     */
    void startRecording();
    
    /**
     * @brief 停止录制
     */
    void stopRecording();
    
    /**
     * @brief 记录一步动作
     */
    void recordStep(PathActionType action);
    
    /**
     * @brief 开始反向归位
     */
    bool startReturning();

    /**
     * @brief 中止归位流程
     */
    void cancelReturning();
    
    /**
     * @brief 更新归位逻辑 (在loop中调用)
     * @return true=正在归位, false=归位完成
     */
    bool updateReturning();
    
    int getStepCount() const { return _stepCount; }
    bool isRecording() const { return _isRecording; }
    bool isReturning() const { return _isReturning; }
    int getRemainingSteps() const { return (_isReturning && _currentReturnStep >= 0) ? _currentReturnStep + 1 : 0; }

private:
    PathStep _steps[PATH_MAX_STEPS];
    int _stepCount = 0;
    
    bool _isRecording = false;
    bool _isReturning = false;
    
    unsigned long _currentStepStartTime = 0;
    PathActionType _lastAction = ACTION_STOP;
    unsigned long _lastActionTime = 0;
    
    int _currentReturnStep = -1;
    unsigned long _returnStepStartTime = 0;
    
    void executeAction(PathActionType action);
    PathActionType getReverseAction(PathActionType action);
};

extern Path path;

#endif // PATH_H
