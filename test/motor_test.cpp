/**
 * @file 电机测试程序
 * @brief 仅测试 L298N 电机驱动，排查电机不转的问题
 * 
 * 接线检查：
 * L298N IN1  → ESP32 GPIO5
 * L298N IN2  → ESP32 GPIO14
 * L298N IN3  → ESP32 GPIO32
 * L298N IN4  → ESP32 GPIO33
 * L298N ENA  → ESP32 GPIO15
 * L298N ENB  → ESP32 GPIO2
 * L298N 12V  → UPS 12V
 * L298N GND  → ESP32 GND (共地！)
 * 
 * ⚠️ 重要：L298N 的 ENA 和 ENB 跳线帽必须拔掉！
 */

#include <Arduino.h>

// 引脚定义
#define MOTOR_LEFT_IN1  5
#define MOTOR_LEFT_IN2  14
#define MOTOR_RIGHT_IN1 32
#define MOTOR_RIGHT_IN2 33
#define MOTOR_LEFT_ENA  15
#define MOTOR_RIGHT_ENB 2

// PWM配置
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8
#define PWM_CHANNEL_LEFT 0
#define PWM_CHANNEL_RIGHT 1

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n=========================================");
    Serial.println("       L298N 电机驱动测试程序");
    Serial.println("=========================================\n");
    
    // 配置所有引脚为输出
    pinMode(MOTOR_LEFT_IN1, OUTPUT);
    pinMode(MOTOR_LEFT_IN2, OUTPUT);
    pinMode(MOTOR_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR_RIGHT_IN2, OUTPUT);
    pinMode(MOTOR_LEFT_ENA, OUTPUT);
    pinMode(MOTOR_RIGHT_ENB, OUTPUT);
    
    Serial.println("引脚配置:");
    Serial.printf("  左电机: IN1=%d, IN2=%d, ENA=%d\n", MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, MOTOR_LEFT_ENA);
    Serial.printf("  右电机: IN1=%d, IN2=%d, ENB=%d\n", MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, MOTOR_RIGHT_ENB);
    
    // 配置PWM
    ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_LEFT_ENA, PWM_CHANNEL_LEFT);
    ledcAttachPin(MOTOR_RIGHT_ENB, PWM_CHANNEL_RIGHT);
    
    Serial.println("\nPWM配置:");
    Serial.printf("  频率: %dHz, 分辨率: %d位\n", PWM_FREQ, PWM_RESOLUTION);
    
    // 初始状态：全部停止
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_LEFT, 0);
    ledcWrite(PWM_CHANNEL_RIGHT, 0);
    
    Serial.println("\n检查清单:");
    Serial.println("  [ ] L298N 12V有供电");
    Serial.println("  [ ] L298N GND 与 ESP32 GND 共地");
    Serial.println("  [ ] L298N ENA跳线帽已拔除");
    Serial.println("  [ ] L298N ENB跳线帽已拔除");
    Serial.println("  [ ] 电机已接到 L298N 输出端");
    Serial.println("\n=========================================");
    Serial.println("3秒后开始测试...");
    Serial.println("=========================================\n");
    delay(3000);
}

void loop() {
    // 测试1: 左电机正转
    Serial.println("\n[测试1] 左电机正转 (速度200, 持续2秒)");
    Serial.println("  IN1=HIGH, IN2=LOW, ENA=200");
    digitalWrite(MOTOR_LEFT_IN1, HIGH);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_LEFT, 200);
    delay(2000);
    
    // 停止
    Serial.println("  停止");
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_LEFT, 0);
    delay(1000);
    
    // 测试2: 左电机反转
    Serial.println("\n[测试2] 左电机反转 (速度200, 持续2秒)");
    Serial.println("  IN1=LOW, IN2=HIGH, ENA=200");
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, HIGH);
    ledcWrite(PWM_CHANNEL_LEFT, 200);
    delay(2000);
    
    // 停止
    Serial.println("  停止");
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_LEFT, 0);
    delay(1000);
    
    // 测试3: 右电机正转
    Serial.println("\n[测试3] 右电机正转 (速度200, 持续2秒)");
    Serial.println("  IN3=HIGH, IN4=LOW, ENB=200");
    digitalWrite(MOTOR_RIGHT_IN1, HIGH);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_RIGHT, 200);
    delay(2000);
    
    // 停止
    Serial.println("  停止");
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_RIGHT, 0);
    delay(1000);
    
    // 测试4: 右电机反转
    Serial.println("\n[测试4] 右电机反转 (速度200, 持续2秒)");
    Serial.println("  IN3=LOW, IN4=HIGH, ENB=200");
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, HIGH);
    ledcWrite(PWM_CHANNEL_RIGHT, 200);
    delay(2000);
    
    // 停止
    Serial.println("  停止");
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_RIGHT, 0);
    delay(1000);
    
    // 测试5: 两电机同时正转
    Serial.println("\n[测试5] 两电机同时正转 (速度200, 持续2秒)");
    digitalWrite(MOTOR_LEFT_IN1, HIGH);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    digitalWrite(MOTOR_RIGHT_IN1, HIGH);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_LEFT, 200);
    ledcWrite(PWM_CHANNEL_RIGHT, 200);
    delay(2000);
    
    // 全部停止
    Serial.println("  全部停止");
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);
    ledcWrite(PWM_CHANNEL_LEFT, 0);
    ledcWrite(PWM_CHANNEL_RIGHT, 0);
    
    Serial.println("\n=========================================");
    Serial.println("测试循环完成，3秒后重新开始...");
    Serial.println("=========================================");
    delay(3000);
}
