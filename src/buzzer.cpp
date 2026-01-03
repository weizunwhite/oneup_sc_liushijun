/**
 * @file buzzer.cpp
 * @brief 蜂鸣器驱动模块实现
 * 支持高电平有效和低电平有效两种蜂鸣器
 */

#include "buzzer.h"

// 全局蜂鸣器对象实例
Buzzer buzzer;

// 根据配置定义开关状态
#if BUZZER_ACTIVE_LOW
    #define BUZZER_ON  LOW   // 低电平响
    #define BUZZER_OFF HIGH  // 高电平停
#else
    #define BUZZER_ON  HIGH  // 高电平响
    #define BUZZER_OFF LOW   // 低电平停
#endif

void Buzzer::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, BUZZER_OFF);  // 初始关闭

    _isBeeping = false;
    _lastBeepTime = 0;
    _beepState = false;

    DEBUG_PRINTLN("蜂鸣器初始化完成");
    DEBUG_PRINTF("  BUZZER_PIN: %d, ACTIVE_LOW: %d\n", BUZZER_PIN, BUZZER_ACTIVE_LOW);
}

void Buzzer::update() {
    if (!_isBeeping) return;

    unsigned long now = millis();

    if (_beepState) {
        // 当前在响，检查是否到了停止时间
        if (now - _lastBeepTime >= BUZZER_BEEP_DURATION) {
            digitalWrite(BUZZER_PIN, BUZZER_OFF);
            _beepState = false;
            _lastBeepTime = now;
        }
    } else {
        // 当前在静音，检查是否到了下次响的时间
        if (now - _lastBeepTime >= BUZZER_BEEP_INTERVAL) {
            digitalWrite(BUZZER_PIN, BUZZER_ON);
            _beepState = true;
            _lastBeepTime = now;
        }
    }
}

void Buzzer::startBeeping() {
    if (_isBeeping) return;  // 已经在响了

    _isBeeping = true;
    _beepState = true;
    _lastBeepTime = millis();
    digitalWrite(BUZZER_PIN, BUZZER_ON);

    DEBUG_PRINTLN("蜂鸣器开始报警");
}

void Buzzer::stopBeeping() {
    if (!_isBeeping) return;  // 已经停止了

    _isBeeping = false;
    _beepState = false;
    digitalWrite(BUZZER_PIN, BUZZER_OFF);

    DEBUG_PRINTLN("蜂鸣器停止报警");
}

void Buzzer::beep(uint16_t duration) {
    // 单次蜂鸣（不影响间歇蜂鸣状态）
    digitalWrite(BUZZER_PIN, BUZZER_ON);
    delay(duration);
    digitalWrite(BUZZER_PIN, BUZZER_OFF);
}
