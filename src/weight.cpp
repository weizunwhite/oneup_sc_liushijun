/**
 * @file weight.cpp
 * @brief 称重模块实现 (HX711) - 优化版
 */

#include "weight.h"

// 全局称重对象实例
Weight weight;

void Weight::begin() {
    _hx711.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
    
    // 设置初始校准系数
    _hx711.set_scale(_calibrationFactor);
    
    // 等待模块就绪（短超时）
    unsigned long startTime = millis();
    while (!_hx711.is_ready()) {
        if (millis() - startTime > 1000) {
            DEBUG_PRINTLN("HX711 未连接或超时!");
            _available = false;
            return;
        }
        delay(10);
    }
    
    _available = true;
    
    // 自动去皮
    DEBUG_PRINTLN("HX711 正在去皮...");
    _hx711.tare(5);  // 取5个样本
    
    DEBUG_PRINTLN("称重模块初始化完成");
    DEBUG_PRINTF("  DOUT=%d, SCK=%d\n", HX711_DOUT_PIN, HX711_SCK_PIN);
    DEBUG_PRINTF("  校准系数: %.2f\n", _calibrationFactor);
}

void Weight::tare() {
    if (_available && _hx711.is_ready()) {
        _hx711.tare(5);
        DEBUG_PRINTLN("称重去皮完成");
    }
}

void Weight::calibrate(float knownWeight) {
    if (!_available || !_hx711.is_ready()) {
        DEBUG_PRINTLN("HX711 未就绪，无法校准!");
        return;
    }
    
    _hx711.tare(5);
    delay(500);
    
    DEBUG_PRINTLN("请放置已知重量物品...");
    delay(3000);
    
    float rawValue = _hx711.get_units(5);
    
    if (rawValue != 0) {
        float newFactor = rawValue / knownWeight;
        _calibrationFactor = newFactor;
        _hx711.set_scale(_calibrationFactor);
        DEBUG_PRINTF("新校准系数: %.2f\n", _calibrationFactor);
    }
}

float Weight::readWeight() {
    // 如果模块不可用，直接返回
    if (!_available) {
        return _currentWeight;
    }
    
    // 非阻塞读取：只有在数据就绪时才读取
    if (_hx711.is_ready()) {
        // 只读取1个样本，避免阻塞
        float reading = _hx711.get_units(1);
        
        // 低通滤波
        _currentWeight = _currentWeight * 0.8f + reading * 0.2f;
        
        // 限制负值
        if (_currentWeight < 0) {
            _currentWeight = 0;
        }
    }
    
    return _currentWeight;
}

bool Weight::isReady() {
    return _available && _hx711.is_ready();
}
