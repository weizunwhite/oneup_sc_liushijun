/**
 * @file led.h
 * @brief LED灯条驱动模块
 * @details 用于夜间照明和安全提醒
 */

#ifndef LED_H
#define LED_H

#include "config.h"
#include <Adafruit_NeoPixel.h>

class LedStrip {
public:
    LedStrip() : _strip(LED_STRIP_COUNT, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800) {}

    /**
     * @brief 初始化LED灯条
     */
    void begin();

    /**
     * @brief 打开LED灯条
     */
    void turnOn();

    /**
     * @brief 关闭LED灯条
     */
    void turnOff();

    /**
     * @brief 切换LED灯条开关状态
     */
    void toggle();

    /**
     * @brief 设置LED颜色
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief 设置LED亮度
     * @param brightness 亮度 (0-255)
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief LED是否开启
     */
    bool isOn() const { return _isOn; }

    /**
     * @brief 更新LED显示（呼吸灯效果，可选）
     */
    void update();

private:
    Adafruit_NeoPixel _strip;
    bool _isOn = false;
    uint8_t _r = LED_COLOR_R;
    uint8_t _g = LED_COLOR_G;
    uint8_t _b = LED_COLOR_B;
};

// 全局LED灯条对象
extern LedStrip ledStrip;

#endif // LED_H
