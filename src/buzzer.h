/**
 * @file buzzer.h
 * @brief 蜂鸣器驱动模块
 * @details 用于姿态异常时的声音提醒
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "config.h"

class Buzzer {
public:
    /**
     * @brief 初始化蜂鸣器
     */
    void begin();

    /**
     * @brief 更新蜂鸣器状态（需要在loop中调用）
     */
    void update();

    /**
     * @brief 开始蜂鸣（间歇响）
     */
    void startBeeping();

    /**
     * @brief 停止蜂鸣
     */
    void stopBeeping();

    /**
     * @brief 单次短促蜂鸣
     * @param duration 持续时间(ms)
     */
    void beep(uint16_t duration = 100);

    /**
     * @brief 是否正在蜂鸣
     */
    bool isBeeping() const { return _isBeeping; }

private:
    bool _isBeeping = false;              // 是否正在间歇蜂鸣
    unsigned long _lastBeepTime = 0;      // 上次蜂鸣时间
    bool _beepState = false;              // 当前蜂鸣状态
};

// 全局蜂鸣器对象
extern Buzzer buzzer;

#endif // BUZZER_H
