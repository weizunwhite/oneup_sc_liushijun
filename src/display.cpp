/**
 * @file display.cpp
 * @brief OLED显示模块实现 - 使用I2C0总线
 */

#include "display.h"
#include <Wire.h>

// 全局显示对象实例
Display display;

bool Display::begin() {
    // 初始化I2C0总线（OLED专用�?
    Wire.begin(I2C0_SDA_PIN, I2C0_SCL_PIN);
    Wire.setClock(400000);  // 400kHz快速模�?
    delay(100);
    
    // 初始化OLED
    if (!oled.begin(SSD1306_SWITCHCAPVCC, SSD1306_ADDR)) {
        DEBUG_PRINTLN("SSD1306 初始化失�?");
        return false;
    }
    
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.display();
    
    DEBUG_PRINTLN("显示屏初始化完成 - 使用I2C0总线");
    DEBUG_PRINTF("  I2C0: SDA=%d, SCL=%d\n", I2C0_SDA_PIN, I2C0_SCL_PIN);
    return true;
}

void Display::clear() {
    oled.clearDisplay();
}

void Display::update() {
    oled.display();
}

void Display::showSplash() {
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setCursor(16, 10);
    oled.print("Smart");
    oled.setCursor(4, 30);
    oled.print("BackPack");
    oled.display();
}

void Display::drawHeader(WorkMode mode) {
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("Mode ");
    oled.print(mode);
    oled.print(":");
    oled.print(MODE_NAMES_CN[mode]);
}

void Display::showMainScreen(WorkMode mode, float weight, bool isOverweight) {
    oled.clearDisplay();
    drawHeader(mode);
    
    oled.setTextSize(3);
    oled.setCursor(10, 25);
    
    if (weight < 1000) {
        oled.printf("%.0fg", weight);
    } else {
        oled.printf("%.1fkg", weight / 1000.0f);
    }
    
    if (isOverweight) {
        oled.setTextSize(1);
        oled.setCursor(30, 55);
        oled.print("! HEAVY !");
    }
    
    oled.display();
}

void Display::showFollowScreen(WorkMode mode, float distance, float angle, float d0, float d1) {
    oled.clearDisplay();
    drawHeader(mode);
    
    oled.setTextSize(3);
    oled.setCursor(10, 20);
    oled.printf("%.0fcm", distance);
    
    oled.setTextSize(2);
    oled.setCursor(60, 45);
    if (angle > 15) {
        oled.print(">>>");
    } else if (angle < -15) {
        oled.print("<<<");
    } else {
        oled.print(" ^ ");
    }
    
    oled.display();
}

void Display::showCarryingScreen(WorkMode mode, float pitch, float roll, const char* warning) {
    oled.clearDisplay();
    drawHeader(mode);
    
    oled.setTextSize(2);
    oled.setCursor(0, 18);
    oled.printf("P:%.0f", pitch);
    
    oled.setCursor(0, 38);
    oled.printf("R:%.0f", roll);
    
    if (warning && strlen(warning) > 0) {
        oled.setTextSize(1);
        oled.setCursor(0, 56);
        if (strstr(warning, "Forward")) {
            oled.print("! Bend !");
        } else if (strstr(warning, "Shoulder")) {
            oled.print("! Shoulder !");
        }
    }
    
    oled.display();
}

void Display::showTeachingScreen(WorkMode mode, int stepCount, bool isRecording) {
    oled.clearDisplay();
    drawHeader(mode);
    
    if (isRecording) {
        oled.setTextSize(2);
        oled.setCursor(20, 20);
        oled.print("REC");
        
        if ((millis() / 500) % 2) {
            oled.fillCircle(100, 28, 4, SSD1306_WHITE);
        }
    } else {
        oled.setTextSize(2);
        oled.setCursor(10, 20);
        oled.print("Ready");
    }
    
    oled.setTextSize(2);
    oled.setCursor(0, 45);
    oled.print("Step:");
    oled.print(stepCount);
    
    oled.display();
}


void Display::showReturningScreen(WorkMode mode, int totalSteps, int stepsRemaining) {
    oled.clearDisplay();
    drawHeader(mode);
    oled.setTextSize(2);
    oled.setCursor(0, 18);
    oled.print("Go Home");
    oled.setTextSize(1);
    oled.setCursor(0, 38);
    oled.printf("Remain:%d", max(0, stepsRemaining));
    int percent = (totalSteps > 0) ? (100 - (stepsRemaining * 100 / totalSteps)) : 0;
    drawProgressBar(0, 54, 120, 8, percent);
    oled.display();
}

void Display::showPullingScreen(WorkMode mode) {
    oled.clearDisplay();
    drawHeader(mode);
    oled.setTextSize(2);
    oled.setCursor(0, 20);
    oled.print("Manual");
    oled.setCursor(0, 40);
    oled.print("Pull");
    oled.display();
}

void Display::showLine(uint8_t line, const char* text, uint8_t size) {
    oled.setTextSize(size);
    int y = line * 8 * size;
    oled.setCursor(0, y);
    oled.print(text);
}

void Display::showMessage(const char* message, uint16_t durationMs) {
    oled.clearDisplay();
    
    oled.setTextSize(2);
    int textLen = strlen(message);
    int charWidth = 12;
    int x = (SCREEN_WIDTH - textLen * charWidth) / 2;
    int y = (SCREEN_HEIGHT - 16) / 2;
    
    oled.setCursor(max(0, x), y);
    oled.print(message);
    oled.display();
    
    delay(durationMs);
}

void Display::drawProgressBar(int x, int y, int width, int height, int percent) {
    percent = constrain(percent, 0, 100);
    oled.drawRect(x, y, width, height, SSD1306_WHITE);
    int fillWidth = (width - 2) * percent / 100;
    if (fillWidth > 0) {
        oled.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}
