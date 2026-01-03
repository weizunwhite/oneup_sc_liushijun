/**
 * @file imu.h
 * @brief 陀螺仪/加速度计模块头文件 (MPU6050)
 */

#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "config.h"

// 姿态数据结构
struct IMUData {
    float pitch;        // 前后倾斜 (度)
    float roll;         // 左右倾斜 (度)
    float yaw;          // 偏航角 (度)
    
    float accelX;       // X轴加速度 (m/s²)
    float accelY;       // Y轴加速度
    float accelZ;       // Z轴加速度
    
    float gyroX;        // X轴角速度 (rad/s)
    float gyroY;        // Y轴角速度
    float gyroZ;        // Z轴角速度
    
    float temperature;  // 温度 (°C)
    
    bool valid;         // 数据是否有效
};

// 姿态警告类型
enum PostureWarning {
    POSTURE_OK = 0,
    POSTURE_BENT_FORWARD,   // 前倾/弯腰
    POSTURE_BENT_BACKWARD,  // 后仰
    POSTURE_SHOULDER_LEFT,  // 左肩低
    POSTURE_SHOULDER_RIGHT  // 右肩低
};

class IMU {
public:
    bool begin();
    void update();
    IMUData getData() const { return _data; }
    float getPitch() const { return _data.pitch; }
    float getRoll() const { return _data.roll; }
    PostureWarning checkPosture();
    const char* getWarningText(PostureWarning warning);
    void calibrate();

private:
    Adafruit_MPU6050 _mpu;
    IMUData _data;
    float _pitchOffset = 0;
    float _rollOffset = 0;
    unsigned long _lastUpdate = 0;
};

extern IMU imu;

#endif // IMU_H
