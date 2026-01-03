/**
 * @file config.h
 * @brief 智能自动跟随书包 - 最终全功能版
 * @details 包含所有引脚定义、参数配置（全速模式，无PWM）
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== 系统配置 ====================

#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

// ==================== 工作模式定义 ====================

enum WorkMode {
    MODE_STANDBY = 0,       // 待机模式
    MODE_CARRYING = 1,      // 背负模式
    MODE_FOLLOWING = 2,     // 自跟随模式
    MODE_PULLING = 3,       // 手拉车模式
    MODE_RETURNING = 4,     // 归位模式
    MODE_TEACHING = 5,      // 路径示教模式
    MODE_COUNT = 6          // 模式总数
};

const char* const MODE_NAMES_CN[] = {
    "待机", "背负", "跟随", "手拉", "归位", "示教"
};

const char* const MODE_NAMES[] = {
    "Standby", "Carrying", "Following", "Pulling", "Returning", "Teaching"
};

// ==================== 引脚定义 ====================

// --- I2C 总线 ---
#define I2C0_SDA_PIN 21     // OLED
#define I2C0_SCL_PIN 22
#define I2C1_SDA_PIN 25     // MPU6050
#define I2C1_SCL_PIN 26

#define MPU6050_ADDR 0x68
#define SSD1306_ADDR 0x3C

// --- UWB模块 ---
// UWB0 (右前): Serial2 (默认)
#define UWB0_RX_PIN 16
#define UWB0_TX_PIN 17
// UWB1 (左前): Serial1 (重映射)
#define UWB1_RX_PIN 27
#define UWB1_TX_PIN 13  // 注意：文档中TX接这里

#define UWB_BAUD_RATE 115200

// --- 电机控制 (L298N 全速模式) ---
// ⚠️ ENA/ENB 插上跳线帽，不接ESP32
#define MOTOR_LEFT_IN1  5   // 左电机正转
#define MOTOR_LEFT_IN2  14  // 左电机反转
#define MOTOR_RIGHT_IN1 32  // 右电机正转
#define MOTOR_RIGHT_IN2 33  // 右电机反转

// --- HX711称重 ---
#define HX711_DOUT_PIN 18
#define HX711_SCK_PIN  19
#define WEIGHT_CALIBRATION_FACTOR 420.0f // 默认校准系数

// --- 按钮 ---
#define BUTTON_PIN 4
#define BUTTON_LONG_PRESS_TIME 1000  // 长按时间阈值 (ms)
#define BUTTON_DOUBLE_CLICK_MS 400   // 双击间隔 (ms)

// --- 蜂鸣器 ---
// ⚠️ 避免使用strapping pins (0,2,4,5,12,15)
#define BUZZER_PIN 23  // 有源蜂鸣器 (GPIO23安全)
#define BUZZER_ACTIVE_LOW 1  // 1=低电平有效(LOW响), 0=高电平有效(HIGH响)

// --- LED灯条 ---
// 暂时禁用，如需使用请更换到空闲引脚
#define LED_STRIP_PIN 12
#define LED_STRIP_COUNT 30
#define LED_STRIP_ENABLED 1

// --- OLED 参数 ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // QT-PY / XIAO using internal reset

// ==================== 算法参数 ====================

// UWB定位参数
#define UWB_BASELINE 28.0f          // 基站间距 (cm)
#define UWB_TAG_OFFSET 12.5f        // 标签中心偏移 (cm)
#define UWB_DISTANCE_SCALE 1.0f     // 无单位输出时的距离缩放(默认cm；若输出为m，改为100)
#define UWB_HEX_SCALE 0.1f          // 十六进制距离缩放(常见为mm -> cm)
#define UWB_ANGLE_INVERT 1          // 1=角度取反(左右传感器相反时)
#define UWB_ANGLE_OFFSET 0.0f       // 角度零点偏置(度)

// 跟随控制参数
#define FOLLOW_DIST_TARGET 80.0f    // 目标距离 (cm)
#define FOLLOW_DIST_DEADZONE 15.0f  // 距离死区 (cm)
#define FOLLOW_ANGLE_DEADZONE 15.0f // 角度死区 (度)
#define FOLLOW_ENABLE_DISTANCE 100.0f // 停止距离阈值: <=该值停止 (cm)
#define FOLLOW_MIN_DISTANCE 40.0f   // 过近停止距离 (cm)
#define FOLLOW_FILTER_ALPHA 0.1f    // 跟随滤波系数 (0-1)
#define FOLLOW_TURN_ON 35.0f        // 开始转向角度 (度)
#define FOLLOW_TURN_OFF 15.0f       // 结束转向角度 (度)
#define FOLLOW_CMD_HOLD_MS 300      // 指令最短保持时间 (ms)

// 电机PWM参数
#define MOTOR_PWM_FREQ 2000         // PWM频率 (Hz)
#define MOTOR_PWM_RESOLUTION 8      // PWM分辨率 (bits)
#define MOTOR_SPEED_FORWARD 100     // 前进速度 (0-255)
#define MOTOR_SPEED_TURN 80         // 转向速度 (0-255)
#define MOTOR_SPEED_FOLLOW_FORWARD 150
#define MOTOR_SPEED_FOLLOW_TURN 80
#define MOTOR_SPEED_PATH_FORWARD 204
#define MOTOR_SPEED_PATH_TURN 204

// 姿态检测参数
#define BEND_THRESHOLD 25.0f        // 弯腰阈值
#define SHOULDER_THRESHOLD 15.0f    // 高低肩阈值

// 称重参数
#define WEIGHT_OVERLOAD_THRESHOLD 5000.0f // 5kg
#define WEIGHT_WARNING_THRESHOLD 1000.0f  // 1kg
#define WEIGHT_WARNING_COOLDOWN_MS 3000   // 超重提示间隔 (ms)

// 路径记录参数
#define PATH_MAX_STEPS 100

// 蜂鸣器参数
#define BUZZER_BEEP_DURATION 200    // 蜂鸣持续时间 (ms)
#define BUZZER_BEEP_INTERVAL 1000   // 蜂鸣间隔时间 (ms)
#define BUZZER_WARN_DURATION 120    // 超重提示蜂鸣时长 (ms)
#define BUZZER_WARN_INTERVAL 120    // 超重提示间隔 (ms)

// LED灯条参数
#define LED_BRIGHTNESS 128          // LED亮度 (0-255)
#define LED_COLOR_R 255            // 默认颜色-红
#define LED_COLOR_G 0
#define LED_COLOR_B 0

// ==================== 蓝牙指令 ====================
#define BT_DEVICE_NAME "UWB_Backpack"
#define BT_CMD_FORWARD  'F'
#define BT_CMD_BACKWARD 'B'
#define BT_CMD_LEFT     'L'
#define BT_CMD_RIGHT    'R'
#define BT_CMD_STOP     'S'

#endif // CONFIG_H
