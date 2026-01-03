/**
 * @file uwb.h
 * @brief UWB定位模块头文件
 */

#ifndef UWB_H
#define UWB_H

#include <Arduino.h>
#include "config.h"

// UWB数据结构
struct UWBData {
    float d0;           // 基站0距离 (cm)
    float d1;           // 基站1距离 (cm)
    float distance;     // 目标综合距离 (cm)
    float angle;        // 目标角度 (度)
    bool valid;         // 数据有效性
    unsigned long lastUpdate;
};

class UWB {
public:
    void begin();
    void update();
    
    /**
     * @brief 获取当前数据
     */
    UWBData getData() const { return _data; }
    
    /**
     * @brief 检查UWB是否连接正常
     */
    bool isConnected();

private:
    UWBData _data;
    
    // 串口解析相关
    String _buffer0;
    String _buffer1;
    
    float parseDistance(String& line);
    void calculatePosition();
};

extern UWB uwb;

#endif // UWB_H
