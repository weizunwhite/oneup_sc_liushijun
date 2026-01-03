/**
 * @file display.h
 * @brief OLED显示模块头文�?
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

class Display {
public:
    bool begin();
    void clear();
    void showSplash();
    void showMainScreen(WorkMode mode, float weight, bool isOverweight);
    void showFollowScreen(WorkMode mode, float distance, float angle, float d0, float d1);
    void showCarryingScreen(WorkMode mode, float pitch, float roll, const char* warning);
    void showTeachingScreen(WorkMode mode, int stepCount, bool isRecording);
    void showReturningScreen(WorkMode mode, int totalSteps, int stepsRemaining);
    void showPullingScreen(WorkMode mode);
    void showLine(uint8_t line, const char* text, uint8_t size = 1);
    void showMessage(const char* message, uint16_t durationMs = 1000);
    void update();

private:
    Adafruit_SSD1306 oled = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    void drawHeader(WorkMode mode);
    void drawProgressBar(int x, int y, int width, int height, int percent);
};

extern Display display;

#endif // DISPLAY_H
