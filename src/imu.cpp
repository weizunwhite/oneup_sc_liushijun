/**
 * @file imu.cpp
 * @brief 陀螺仪/加速度计模块实现 (MPU6050) - 使用I2C1总线
 */

#include "imu.h"
#include <Wire.h>
#include <math.h>

// 全局IMU对象实例
IMU imu;

// 第二个I2C总线
TwoWire I2C_IMU = TwoWire(1);  // 使用I2C1

bool IMU::begin() {
    // 初始化I2C1总线（独立于OLED的I2C0）
    I2C_IMU.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, 400000);  // 400kHz
    
    delay(100);
    
    if (!_mpu.begin(MPU6050_ADDR, &I2C_IMU)) {
        DEBUG_PRINTLN("MPU6050 未找到!");
        return false;
    }
    
    // 配置测量范围
    _mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    // 初始化数据
    _data = {0};
    _data.valid = true;
    
    DEBUG_PRINTLN("IMU (MPU6050) 初始化完成 - 使用I2C1总线");
    DEBUG_PRINTF("  I2C1: SDA=%d, SCL=%d\n", I2C1_SDA_PIN, I2C1_SCL_PIN);
    DEBUG_PRINTF("  加速度范围: ±4G\n");
    DEBUG_PRINTF("  陀螺仪范围: ±500°/s\n");
    
    // 启动时自动校准
    delay(100);
    calibrate();
    
    return true;
}

void IMU::update() {
    sensors_event_t accel, gyro, temp;
    
    if (!_mpu.getEvent(&accel, &gyro, &temp)) {
        _data.valid = false;
        return;
    }
    
    // 存储原始数据
    _data.accelX = accel.acceleration.x;
    _data.accelY = accel.acceleration.y;
    _data.accelZ = accel.acceleration.z;
    
    _data.gyroX = gyro.gyro.x;
    _data.gyroY = gyro.gyro.y;
    _data.gyroZ = gyro.gyro.z;
    
    _data.temperature = temp.temperature;
    
    // 计算姿态角度（基于加速度）
    float ax = _data.accelX;
    float ay = _data.accelY;
    float az = _data.accelZ;
    
    // Pitch: 前后倾斜
    _data.pitch = atan2(ay, sqrt(ax * ax + az * az)) * 180.0f / PI - _pitchOffset;
    
    // Roll: 左右倾斜
    _data.roll = atan2(ax, sqrt(ay * ay + az * az)) * 180.0f / PI - _rollOffset;
    
    // Yaw: 简单积分（会漂移）
    unsigned long now = millis();
    if (_lastUpdate > 0) {
        float dt = (now - _lastUpdate) / 1000.0f;
        _data.yaw += _data.gyroZ * dt * 180.0f / PI;
        
        while (_data.yaw > 180) _data.yaw -= 360;
        while (_data.yaw < -180) _data.yaw += 360;
    }
    _lastUpdate = now;
    
    _data.valid = true;
}

void IMU::calibrate() {
    DEBUG_PRINTLN("IMU 校准中...");
    
    float pitchSum = 0;
    float rollSum = 0;
    const int samples = 20;
    
    for (int i = 0; i < samples; i++) {
        sensors_event_t accel, gyro, temp;
        _mpu.getEvent(&accel, &gyro, &temp);
        
        float ax = accel.acceleration.x;
        float ay = accel.acceleration.y;
        float az = accel.acceleration.z;
        
        pitchSum += atan2(ay, sqrt(ax * ax + az * az)) * 180.0f / PI;
        rollSum += atan2(ax, sqrt(ay * ay + az * az)) * 180.0f / PI;
        
        delay(20);
    }
    
    _pitchOffset = pitchSum / samples;
    _rollOffset = rollSum / samples;
    _data.yaw = 0;
    
    DEBUG_PRINTF("IMU 校准完成: pitch_offset=%.1f, roll_offset=%.1f\n", 
                 _pitchOffset, _rollOffset);
}

PostureWarning IMU::checkPosture() {
    update();
    
    if (!_data.valid) {
        return POSTURE_OK;
    }
    
    // 检测前后倾斜（弯腰/驼背）
    if (_data.pitch > BEND_THRESHOLD) {
        return POSTURE_BENT_FORWARD;
    }
    if (_data.pitch < -BEND_THRESHOLD) {
        return POSTURE_BENT_BACKWARD;
    }
    
    // 检测左右倾斜（高低肩）
    if (_data.roll > SHOULDER_THRESHOLD) {
        return POSTURE_SHOULDER_RIGHT;
    }
    if (_data.roll < -SHOULDER_THRESHOLD) {
        return POSTURE_SHOULDER_LEFT;
    }
    
    return POSTURE_OK;
}

const char* IMU::getWarningText(PostureWarning warning) {
    switch (warning) {
        case POSTURE_BENT_FORWARD:
            return "!! Bent Forward !!";
        case POSTURE_BENT_BACKWARD:
            return "!! Leaning Back !!";
        case POSTURE_SHOULDER_LEFT:
            return "!! Left Shoulder !!";
        case POSTURE_SHOULDER_RIGHT:
            return "!! Right Shoulder !!";
        default:
            return "";
    }
}
