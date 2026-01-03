/**
 * @file weight.h
 * @brief 称重模块头文件 (HX711)
 */

#ifndef WEIGHT_H
#define WEIGHT_H

#include <Arduino.h>
#include <HX711.h>
#include "config.h"

class Weight {
public:
    /**
     * @brief 初始化称重模块
     */
    void begin();

    /**
     * @brief 去皮（将当前重量设为零点）
     */
    void tare();

    /**
     * @brief 校准
     * @param knownWeight 已知重量 (g)
     */
    void calibrate(float knownWeight);

    /**
     * @brief 读取重量（非阻塞）
     * @return 重量 (g)
     */
    float readWeight();

    /**
     * @brief 获取最后读取的重量
     * @return 重量 (g)
     */
    float getWeight() const { return _currentWeight; }

    /**
     * @brief 检查是否超重
     * @return true 超重
     */
    bool isOverweight() const { return _currentWeight > WEIGHT_OVERLOAD_THRESHOLD; }

    /**
     * @brief 设置校准系数
     */
    void setCalibrationFactor(float factor) { _calibrationFactor = factor; }

    /**
     * @brief 获取校准系数
     */
    float getCalibrationFactor() const { return _calibrationFactor; }

    /**
     * @brief 检查模块是否可用
     */
    bool isAvailable() const { return _available; }

    /**
     * @brief 检查模块是否就绪
     */
    bool isReady();

private:
    HX711 _hx711;
    float _calibrationFactor = WEIGHT_CALIBRATION_FACTOR;
    float _currentWeight = 0;
    bool _available = false;
};

// 全局称重对象
extern Weight weight;

#endif // WEIGHT_H
