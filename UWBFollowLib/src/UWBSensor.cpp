/**
 * @file UWBSensor.cpp
 * @brief UWB positioning sensor implementation
 */

#include "UWBSensor.h"

// Frame parser structure for binary protocol
struct FrameParser {
    uint8_t state = 0;  // 0=wait header, 1=len, 2=payload, 3=tail
    uint8_t len = 0;
    uint8_t index = 0;
    uint8_t payload[8] = {0};
};

// Helper: convert hex char to value
static int hexDigitValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Helper: parse binary frame
static bool parseFrame(FrameParser& p, uint8_t b, uint16_t& addr, uint16_t& dist, uint8_t& rssi) {
    switch (p.state) {
        case 0:
            if (b == 0xF0) {
                p.state = 1;
            }
            break;
        case 1:
            p.len = b;
            if (p.len == 0x05) {
                p.index = 0;
                p.state = 2;
            } else {
                p.state = 0;
            }
            break;
        case 2:
            p.payload[p.index++] = b;
            if (p.index >= p.len) {
                p.state = 3;
            }
            break;
        case 3:
            if (b == 0xAA) {
                addr = (uint16_t)p.payload[0] | ((uint16_t)p.payload[1] << 8);
                dist = (uint16_t)p.payload[2] | ((uint16_t)p.payload[3] << 8);
                rssi = p.payload[4];
                p.state = 0;
                return true;
            }
            p.state = 0;
            break;
        default:
            p.state = 0;
            break;
    }
    return false;
}

// Helper: check if byte is printable ASCII
static bool isPrintableAscii(uint8_t b) {
    return b >= 0x20 && b <= 0x7E;
}

UWBSensor::UWBSensor() : _debug(false) {
}

bool UWBSensor::begin(const UWBConfig& config) {
    _config = config;
    _data = UWBData();

    // Initialize serial ports
    if (_config.rx0Pin >= 0 && _config.tx0Pin >= 0) {
        _config.serial0->begin(_config.baudRate, SERIAL_8N1, _config.rx0Pin, _config.tx0Pin);
    } else {
        _config.serial0->begin(_config.baudRate);
    }

    if (_config.rx1Pin >= 0 && _config.tx1Pin >= 0) {
        _config.serial1->begin(_config.baudRate, SERIAL_8N1, _config.rx1Pin, _config.tx1Pin);
    } else {
        _config.serial1->begin(_config.baudRate);
    }

    if (_debug) {
        Serial.println("[UWBSensor] Initialized");
        Serial.printf("  UWB0: RX=%d, TX=%d\n", _config.rx0Pin, _config.tx0Pin);
        Serial.printf("  UWB1: RX=%d, TX=%d\n", _config.rx1Pin, _config.tx1Pin);
        Serial.printf("  Baseline: %.1f cm\n", _config.baseline);
    }

    return true;
}

void UWBSensor::update() {
    bool newData = false;
    static FrameParser parser0;
    static FrameParser parser1;
    static unsigned long lastAscii0 = 0;
    static unsigned long lastAscii1 = 0;
    unsigned long now = millis();

    // Read UWB0 data
    while (_config.serial0->available()) {
        uint8_t b = (uint8_t)_config.serial0->read();
        uint16_t addr = 0;
        uint16_t dist = 0;
        uint8_t rssi = 0;

        if (parseFrame(parser0, b, addr, dist, rssi)) {
            if (dist > 0) {
                _data.d0 = (float)dist * _config.distanceScale;
                newData = true;
            }
        } else if (parser0.state == 0) {
            if (b == '\n' || b == '\r') {
                if (_buffer0.length() > 0) {
                    float d = parseDistance(_buffer0);
                    if (d > 0) {
                        _data.d0 = d;
                        newData = true;
                    } else if (_debug) {
                        Serial.printf("[UWB0] raw: %s\n", _buffer0.c_str());
                    }
                    _buffer0 = "";
                }
            } else if (isPrintableAscii(b)) {
                lastAscii0 = now;
                _buffer0 += (char)b;
                if (_buffer0.length() > 200) {
                    float d = parseDistance(_buffer0);
                    if (d > 0) {
                        _data.d0 = d;
                        newData = true;
                    } else if (_debug) {
                        Serial.printf("[UWB0] raw: %s\n", _buffer0.c_str());
                    }
                    _buffer0 = "";
                }
            } else if (_buffer0.length() > 0) {
                _buffer0 = "";
            }
        }
    }

    // Read UWB1 data
    while (_config.serial1->available()) {
        uint8_t b = (uint8_t)_config.serial1->read();
        uint16_t addr = 0;
        uint16_t dist = 0;
        uint8_t rssi = 0;

        if (parseFrame(parser1, b, addr, dist, rssi)) {
            if (dist > 0) {
                _data.d1 = (float)dist * _config.distanceScale;
                newData = true;
            }
        } else if (parser1.state == 0) {
            if (b == '\n' || b == '\r') {
                if (_buffer1.length() > 0) {
                    float d = parseDistance(_buffer1);
                    if (d > 0) {
                        _data.d1 = d;
                        newData = true;
                    } else if (_debug) {
                        Serial.printf("[UWB1] raw: %s\n", _buffer1.c_str());
                    }
                    _buffer1 = "";
                }
            } else if (isPrintableAscii(b)) {
                lastAscii1 = now;
                _buffer1 += (char)b;
                if (_buffer1.length() > 200) {
                    float d = parseDistance(_buffer1);
                    if (d > 0) {
                        _data.d1 = d;
                        newData = true;
                    } else if (_debug) {
                        Serial.printf("[UWB1] raw: %s\n", _buffer1.c_str());
                    }
                    _buffer1 = "";
                }
            } else if (_buffer1.length() > 0) {
                _buffer1 = "";
            }
        }
    }

    // Fallback: flush buffer after idle
    if (_buffer0.length() > 0 && (now - lastAscii0) > 30) {
        float dist = parseDistance(_buffer0);
        if (dist > 0) {
            _data.d0 = dist;
            newData = true;
        } else if (_debug) {
            Serial.printf("[UWB0] raw(noeol): %s\n", _buffer0.c_str());
        }
        _buffer0 = "";
    }
    if (_buffer1.length() > 0 && (now - lastAscii1) > 30) {
        float dist = parseDistance(_buffer1);
        if (dist > 0) {
            _data.d1 = dist;
            newData = true;
        } else if (_debug) {
            Serial.printf("[UWB1] raw(noeol): %s\n", _buffer1.c_str());
        }
        _buffer1 = "";
    }

    if (newData) {
        calculatePosition();
        _data.lastUpdate = millis();
        _data.valid = true;

        // Debug output (throttled)
        if (_debug) {
            static unsigned long lastPrint = 0;
            if (millis() - lastPrint > 500) {
                Serial.printf("[UWB] d0=%.0f, d1=%.0f -> Dist=%.0f, Ang=%.1f\n",
                              _data.d0, _data.d1, _data.distance, _data.angle);
                lastPrint = millis();
            }
        }
    }
}

float UWBSensor::parseDistance(String& line) {
    line.trim();
    if (line.length() == 0) return 0;

    int len = line.length();
    float value = 0;
    bool found = false;
    int lastEnd = -1;

    // Extract numeric value
    for (int i = 0; i < len; i++) {
        char c = line.charAt(i);
        if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+') {
            int j = i;
            bool hasDigit = false;
            if (line.charAt(j) == '-' || line.charAt(j) == '+') j++;
            while (j < len) {
                char cj = line.charAt(j);
                if (cj >= '0' && cj <= '9') {
                    hasDigit = true;
                    j++;
                } else if (cj == '.') {
                    j++;
                } else {
                    break;
                }
            }
            if (hasDigit) {
                value = line.substring(i, j).toFloat();
                found = true;
                lastEnd = j;
            }
            i = j - 1;
        }
    }

    if (found && value > 0) {
        float scale = 1.0f;
        bool unitFound = false;
        int k = lastEnd;

        // Skip whitespace
        while (k < len) {
            char ck = line.charAt(k);
            if (ck != ' ' && ck != '\t' && ck != '\r' && ck != '\n') break;
            k++;
        }

        // Check unit
        if (k < len) {
            char c0 = line.charAt(k);
            if (c0 >= 'A' && c0 <= 'Z') c0 = (char)(c0 - 'A' + 'a');
            char c1 = (k + 1 < len) ? line.charAt(k + 1) : '\0';
            if (c1 >= 'A' && c1 <= 'Z') c1 = (char)(c1 - 'A' + 'a');

            if (c0 == 'm' && c1 == 'm') {
                scale = 0.1f; // mm -> cm
                unitFound = true;
            } else if (c0 == 'c' && c1 == 'm') {
                scale = 1.0f; // cm
                unitFound = true;
            } else if (c0 == 'm') {
                scale = 100.0f; // m -> cm
                unitFound = true;
            }
        }

        if (unitFound) {
            return value * scale;
        }

        return value * _config.distanceScale;
    }

    // Hex fallback
    unsigned long hexValue = 0;
    bool hexFound = false;
    for (int i = 0; i < len; i++) {
        int j = i;
        if (line.charAt(j) == '0' && (j + 1 < len)) {
            char nx = line.charAt(j + 1);
            if (nx == 'x' || nx == 'X') {
                j += 2;
            }
        }

        unsigned long v = 0;
        bool hasHex = false;
        while (j < len) {
            int hv = hexDigitValue(line.charAt(j));
            if (hv >= 0) {
                v = (v << 4) + (unsigned long)hv;
                hasHex = true;
                j++;
            } else {
                break;
            }
        }

        if (hasHex) {
            hexValue = v;
            hexFound = true;
            i = j - 1;
        }
    }

    if (!hexFound || hexValue == 0) return 0;
    return (float)hexValue * _config.hexScale;
}

void UWBSensor::calculatePosition() {
    float d0 = _data.d0;
    float d1 = _data.d1;
    float L = _config.baseline;

    if (d0 <= 0 || d1 <= 0) return;

    // Triangulation algorithm
    // Assuming two UWB modules on a baseline, target in front
    // Calculate x-offset and y-distance
    float x = (d0 * d0 - d1 * d1) / (2 * L);
    float y_sq = d0 * d0 - (x + L / 2) * (x + L / 2);
    float y = (y_sq > 0) ? sqrt(y_sq) : 0;

    _data.distance = y;  // Perpendicular distance

    // Calculate angle (0=forward, +right/-left)
    float angle = atan2(x, y) * 180.0 / PI;

    if (_config.angleInvert) {
        angle = -angle;
    }
    angle += _config.angleOffset;

    _data.angle = angle;
}

bool UWBSensor::isConnected() const {
    return (millis() - _data.lastUpdate) < _config.timeoutMs;
}
