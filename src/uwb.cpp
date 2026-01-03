/**
 * @file uwb.cpp
 * @brief UWB定位模块实现
 * 双基站定位算法
 */

#include "uwb.h"

UWB uwb;

struct FrameParser {
    uint8_t state = 0;  // 0=wait header, 1=len, 2=payload, 3=tail
    uint8_t len = 0;
    uint8_t index = 0;
    uint8_t payload[8] = {0};
};

static int hexDigitValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

void UWB::begin() {
    // 开启两个串口
    // Serial2 (UWB0): 可配置引脚
    Serial2.begin(UWB_BAUD_RATE, SERIAL_8N1, UWB0_RX_PIN, UWB0_TX_PIN);
    
    // Serial1 (UWB1): 重映射引脚 RX=27, TX=13
    Serial1.begin(UWB_BAUD_RATE, SERIAL_8N1, UWB1_RX_PIN, UWB1_TX_PIN);
    
    _data = {0, 0, 0, 0, false, 0};
    
    DEBUG_PRINTLN("UWB模块初始化完成");
    DEBUG_PRINTF("  UWB0: RX=16, TX=17 (Serial2)\n");
    DEBUG_PRINTF("  UWB1: RX=%d, TX=%d (Serial1)\n", UWB1_RX_PIN, UWB1_TX_PIN);
}

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

static bool isPrintableAscii(uint8_t b) {
    return b >= 0x20 && b <= 0x7E;
}

void UWB::update() {
    bool newData = false;
    static FrameParser parser0;
    static FrameParser parser1;
    static unsigned long lastAscii0 = 0;
    static unsigned long lastAscii1 = 0;
    unsigned long now = millis();
    
    // 读取 UWB0 数据 (Serial2)
    while (Serial2.available()) {
        uint8_t b = (uint8_t)Serial2.read();
        uint16_t addr = 0;
        uint16_t dist = 0;
        uint8_t rssi = 0;
        if (parseFrame(parser0, b, addr, dist, rssi)) {
            if (dist > 0) {
                _data.d0 = (float)dist * UWB_DISTANCE_SCALE;
                newData = true;
            }
        } else if (parser0.state == 0) {
            if (b == '\n' || b == '\r') {
                if (_buffer0.length() > 0) {
                    float d = parseDistance(_buffer0);
                    if (d > 0) {
                        _data.d0 = d;
                        newData = true;
                    } else {
                        DEBUG_PRINTF("UWB0 raw: %s\n", _buffer0.c_str());
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
                    } else {
                        DEBUG_PRINTF("UWB0 raw: %s\n", _buffer0.c_str());
                    }
                    _buffer0 = "";
                }
            } else if (_buffer0.length() > 0) {
                _buffer0 = "";
            }
        }
    }
    
    // 读取 UWB1 数据 (Serial1)
    while (Serial1.available()) {
        uint8_t b = (uint8_t)Serial1.read();
        uint16_t addr = 0;
        uint16_t dist = 0;
        uint8_t rssi = 0;
        if (parseFrame(parser1, b, addr, dist, rssi)) {
            if (dist > 0) {
                _data.d1 = (float)dist * UWB_DISTANCE_SCALE;
                newData = true;
            }
        } else if (parser1.state == 0) {
            if (b == '\n' || b == '\r') {
                if (_buffer1.length() > 0) {
                    float d = parseDistance(_buffer1);
                    if (d > 0) {
                        _data.d1 = d;
                        newData = true;
                    } else {
                        DEBUG_PRINTF("UWB1 raw: %s\n", _buffer1.c_str());
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
                    } else {
                        DEBUG_PRINTF("UWB1 raw: %s\n", _buffer1.c_str());
                    }
                    _buffer1 = "";
                }
            } else if (_buffer1.length() > 0) {
                _buffer1 = "";
            }
        }
    }

    // Fallback: no newline, flush after short idle
    if (_buffer0.length() > 0 && (now - lastAscii0) > 30) {
        float dist = parseDistance(_buffer0);
        if (dist > 0) {
            _data.d0 = dist;
            newData = true;
        } else {
            DEBUG_PRINTF("UWB0 raw(noeol): %s\n", _buffer0.c_str());
        }
        _buffer0 = "";
    }
    if (_buffer1.length() > 0 && (now - lastAscii1) > 30) {
        float dist = parseDistance(_buffer1);
        if (dist > 0) {
            _data.d1 = dist;
            newData = true;
        } else {
            DEBUG_PRINTF("UWB1 raw(noeol): %s\n", _buffer1.c_str());
        }
        _buffer1 = "";
    }
    
    if (newData) {
        calculatePosition();
        _data.lastUpdate = millis();
        _data.valid = true;
        
        // 调试输出（限制频率）
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 500) {
            DEBUG_PRINTF("UWB: d0=%.0f, d1=%.0f -> Dist=%.0f, Ang=%.1f\n", 
                         _data.d0, _data.d1, _data.distance, _data.angle);
            lastPrint = millis();
        }
    }
}

float UWB::parseDistance(String& line) {
    // 假设数据格式为 "D:123.45" 或类似
    // 需要根据实际UWB模块输出格式调整
    // 这里假设格式为: "DIST: 1.23m" 或纯数字或 "mc 0f 00000xxx" (如果是AT命令模式)
    
    // 简化处理：查找数字
    // 如果是 "mid 0x0000xxxx ..." 这种十六进制格式，需要专门解析
    // 假设是LinkTrack或通常的 "Distance: 120 cm"
    
    // ⚠️ 临时方案：模拟解析，期待格式中直接包含距离数值
    // 这里需要根据您实际的UWB输出格式来写
    // 假设格式是 "d: 120"
    
    // Extract the last numeric token and convert to cm
    line.trim();
    if (line.length() == 0) return 0;

    int len = line.length();
    float value = 0;
    bool found = false;
    int lastEnd = -1;

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
        while (k < len) {
            char ck = line.charAt(k);
            if (ck != ' ' && ck != '\t' && ck != '\r' && ck != '\n') break;
            k++;
        }
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

        return value * UWB_DISTANCE_SCALE;
    }

    // Hex fallback: find last hex token (e.g., "0f 00000a3c")
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
    return (float)hexValue * UWB_HEX_SCALE;
}

void UWB::calculatePosition() {
    float d0 = _data.d0;
    float d1 = _data.d1;
    float L = UWB_BASELINE;
    
    if (d0 <= 0 || d1 <= 0) return;
    
    // 所有的计算单位为 cm
    
    // 三角定位算法
    // 设车中心为原点 (x, y)
    // d0^2 = (x + L/2)^2 + y^2
    // d1^2 = (x - L/2)^2 + y^2
    
    // 简化公式计算相对坐标 (距离和角度)
    // 余弦定理：
    // cos(theta) = (d0^2 + L^2 - d1^2) / (2 * d0 * L)
    
    float cos_theta = (d0 * d0 + L * L - d1 * d1) / (2 * d0 * L);
    cos_theta = constrain(cos_theta, -1.0f, 1.0f);
    
    // x偏离中心距离
    // 这是一个近似计算，假设两个传感器在一条直线上，目标在前方
    
    // 计算左右偏移
    // 如果 d0 > d1，说明目标偏向左边 (UWB1更近)
    // 如果 d0 < d1，说明目标偏向右边 (UWB0更近)
    
    // 简单计算：
    float x = (d0 * d0 - d1 * d1) / (2 * L);
    float y_sq = d0 * d0 - (x + L/2) * (x + L/2);
    float y = (y_sq > 0) ? sqrt(y_sq) : 0;
    
    _data.distance = y;  // 前方垂直距离
    
    // 计算角度 (正前方为0，左负右正)
    // x 正值表示偏右，负值表示偏左
    float angle = atan2(x, y) * 180.0 / PI;
#if UWB_ANGLE_INVERT
    angle = -angle;
#endif
    angle += UWB_ANGLE_OFFSET;
    _data.angle = angle;
}

bool UWB::isConnected() {
    return (millis() - _data.lastUpdate) < 2000; // 2秒超时
}
