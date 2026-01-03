/**
 * @file follow.h
 * @brief 跟随控制模块头文件
 */

#ifndef FOLLOW_H
#define FOLLOW_H

#include <Arduino.h>
#include "config.h"

class Follow {
public:
    void begin();
    
    /**
     * @brief 更新跟随控制
     * @param distance 目标距离 (cm)
     * @param angle 目标角度 (度)
     */
    void update(float distance, float angle);
    
    /**
     * @brief 停止跟随
     */
    void stop();

private:
    enum FollowCmd {
        CMD_STOP = 0,
        CMD_FORWARD,
        CMD_LEFT,
        CMD_RIGHT
    };

    float _kpDistance = 2.0;
    float _kpAngle = 3.0;

    float _distanceFiltered = 0.0f;
    float _angleFiltered = 0.0f;
    bool _filterInit = false;
    FollowCmd _lastCmd = CMD_STOP;
    unsigned long _lastCmdTime = 0;
    unsigned long _lastUpdate;
};

extern Follow follow;

#endif // FOLLOW_H
