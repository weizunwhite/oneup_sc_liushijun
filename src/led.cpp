#include "led.h"

LedStrip ledStrip;

void LedStrip::begin() {
#if LED_STRIP_ENABLED
    _strip.begin();
    _strip.setBrightness(LED_BRIGHTNESS);
    _strip.show();
    _isOn = false;
#endif
}

void LedStrip::setColor(uint8_t r, uint8_t g, uint8_t b) {
#if LED_STRIP_ENABLED
    _r = r;
    _g = g;
    _b = b;
    if (_isOn) {
        for (uint16_t i = 0; i < _strip.numPixels(); ++i) {
            _strip.setPixelColor(i, _strip.Color(_r, _g, _b));
        }
        _strip.show();
    }
#endif
}

void LedStrip::setBrightness(uint8_t brightness) {
#if LED_STRIP_ENABLED
    _strip.setBrightness(brightness);
    if (_isOn) {
        _strip.show();
    }
#endif
}

void LedStrip::turnOn() {
#if LED_STRIP_ENABLED
    _isOn = true;
    for (uint16_t i = 0; i < _strip.numPixels(); ++i) {
        _strip.setPixelColor(i, _strip.Color(_r, _g, _b));
    }
    _strip.show();
#endif
}

void LedStrip::turnOff() {
#if LED_STRIP_ENABLED
    _isOn = false;
    _strip.clear();
    _strip.show();
#endif
}

void LedStrip::toggle() {
#if LED_STRIP_ENABLED
    if (_isOn) {
        turnOff();
    } else {
        turnOn();
    }
#endif
}

void LedStrip::update() {
    // Reserved for future effects.
}
