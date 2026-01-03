# 自动跟随书包项目 -  开发指南



## 项目概述

这是一个五年级学生的科技创新项目：**智能自动跟随书包**。书包固定在带轮底盘上，通过UWB定位技术实现自动跟随人行走的功能。

### 项目目标
解决小学生书包过重的问题，让书包能够自动跟随学生移动，减轻学生负担。

---

## 工作模式

系统有6种工作模式，通过按钮循环切换，OLED屏幕显示当前模式：

| 模式编号 | 模式名称 | 功能描述 |
|---------|---------|---------|
| 0 | 待机模式 | 屏幕显示模式和重量，重量超标时提醒切换模式 |
| 1 | 背负模式 | 陀螺仪检测学生姿态（弯腰、驼背、高低肩） |
| 2 | 自跟随模式 | UWB工作，小车自动跟随人移动 |
| 3 | 手拉车模式 | 所有传感器停止，纯手动拉车 |
| 4 | 归位模式 | 按照示教路径自动归位 |
| 5 | 路径示教模式 | 蓝牙连接手机APP遥控，小车学习路径 |

---

## 硬件清单

### 主控
- **ESP32 Dev Module**（普通版，非S3）

### 传感器模块
- **UWB模块**: 硅传 MK8000TR7.9-GC × 3个
  - 2个配置为从机（基站），安装在小车前端左右两侧
  - 1个配置为主机（标签），佩戴在人身上
- **陀螺仪**: MPU6050（I2C接口）
- **称重模块**: HX711 + 压力传感器

### 执行器
- **电机驱动**: L298N（带ENA/ENB使能引脚，支持PWM调速）
- **电机**: 12V直流减速电机 × 2（工作电流约0.7A，无编码器）

### 显示与交互
- **显示屏**: SSD1306 OLED 0.96寸（I2C接口）
- **按钮**: 轻触按钮 × 1

### 电源
- **UPS电池模块**: 输出12V（电机）和5V（主控）

---

## 底盘结构

```
俯视图：
        【前方 - 跟随方向】
    
    [UWB从机1]          [UWB从机0]
         ┌─────────────────┐
    ○────┤                 ├────○  ← 驱动轮（后轮）
         │   [称重模块]     │
         │   [陀螺仪]       │
         │                 │
         └───────┬─────────┘
             ○   │   ○  ← 万向轮（前轮）
                 │
              [拉杆]
        
        【后方 - 拉杆方向】
```

- 两个UWB从机（基站）安装在小车前端，间距约25-30cm
- 后轮为驱动轮（差速驱动），前轮为万向轮
- 陀螺仪和称重模块安装在底盘中部

---

## 接线方案（ESP32 Dev Module - 双I2C总线版）

### ⚠️ 重要说明

**为解决 OLED 和 MPU6050 共用 I2C 总线导致的显示卡顿问题，现采用 ESP32 双 I2C 总线方案：**
- **I2C0** (Wire) - 专用于 OLED 显示屏
- **I2C1** (Wire1) - 专用于 MPU6050 陀螺仪

### 电源接线

| 模块 | 电源 |
|------|------|
| ESP32 | UPS 5V → VIN引脚 |
| L298N | UPS 12V → 12V输入端 |
| UWB模块 ×2 | ESP32 3.3V |
| MPU6050 | ESP32 3.3V |
| SSD1306 | ESP32 3.3V |
| HX711 | ESP32 5V（VIN） |

### 信号接线

| 模块 | 模块引脚 | ESP32引脚 | 备注 |
|------|----------|-----------|------|
| **UWB从机0** | VCC | 3.3V | 小车右前方 |
| | GND | GND | |
| | GPIO1 (TX→) | **GPIO16 (RX)** | UWB发→ESP32收，Serial2 |
| | GPIO0 (←RX) | **GPIO17 (TX)** | UWB收←ESP32发，Serial2 |
| **UWB从机1** | VCC | 3.3V | 小车左前方 |
| | GND | GND | |
| | GPIO1 (TX→) | **GPIO27 (RX)** | UWB发→ESP32收，Serial1 |
| | GPIO0 (←RX) | **GPIO13 (TX)** | UWB收←ESP32发，Serial1 |
| **L298N** | IN1 | **GPIO5** | 左电机正转 (GPIO12易冲突，已改) |
| | IN2 | **GPIO14** | 左电机反转 |
| | IN3 | **GPIO32** | 右电机正转 |
| | IN4 | **GPIO33** | 右电机反转 |
| | ENA | **(不接ESP32)** | 跳线帽插上，接5V 全速模式 |
| | ENB | **(不接ESP32)** | 跳线帽插上，接5V 全速模式 |
| | GND | GND | 必须共地 |
| **MPU6050** | VCC | 3.3V | 使用 I2C1 总线 |
| | GND | GND | |
| | SDA | **GPIO25** | I2C1 SDA（独立） |
| | SCL | **GPIO26** | I2C1 SCL（独立） |
| **SSD1306** | VCC | 3.3V | 使用 I2C0 总线 |
| | GND | GND | |
| | SDA | **GPIO21** | I2C0 SDA（独立） |
| | SCL | **GPIO22** | I2C0 SCL（独立） |
| **HX711** | VCC | 5V | |
| | GND | GND | |
| | DOUT | **GPIO18** | 数据 |
| | SCK | **GPIO19** | 时钟 |
| **按钮** | 一端 | **GPIO4** | 内部上拉，按下为LOW |
| | 另一端 | GND | |

### 引脚分配汇总

```
ESP32 引脚使用情况（最终版）：
├── 串口
│   ├── GPIO16 (RX2) ← UWB从机0 TX
│   ├── GPIO17 (TX2) → UWB从机0 RX
│   ├── GPIO27 (RX1) ← UWB从机1 TX
│   └── GPIO13 (TX1) → UWB从机1 RX
├── I2C0 总线（OLED专用）
│   ├── GPIO21 (SDA) ↔ SSD1306
│   └── GPIO22 (SCL) ↔ SSD1306
├── I2C1 总线（IMU专用）
│   ├── GPIO25 (SDA) ↔ MPU6050
│   └── GPIO26 (SCL) ↔ MPU6050
├── 电机控制 (全速模式)
│   ├── GPIO5  → L298N IN1
│   ├── GPIO14 → L298N IN2
│   ├── GPIO32 → L298N IN3
│   └── GPIO33 → L298N IN4
│   * ENA/ENB 插跳线帽，不接ESP32
├── HX711
│   ├── GPIO18 → DOUT
│   └── GPIO19 → SCK
└── 按钮
    └── GPIO4 → 按钮一端 (另一端接地)
```

### 引脚变更说明

从旧版本到双I2C版本的主要变更：

| 信号 | 旧引脚 | 新引脚 | 变更原因 |
|------|--------|--------|----------|
| MPU6050 SDA | GPIO21 (共用) | **GPIO25** | 改用独立I2C1总线 |
| MPU6050 SCL | GPIO22 (共用) | **GPIO26** | 改用独立I2C1总线 |
| UWB1 TX | GPIO26 | **GPIO13** | 让出给MPU6050 I2C1 |
| 电机 IN1 | GPIO13 | **GPIO12** | 让出给UWB1 |
| 电机 IN3 | GPIO14 | **GPIO32** | 调整 |
| 电机 IN4 | GPIO25 | **GPIO33** | 让出给MPU6050 |
| 电机 ENA | GPIO32 | **GPIO15** | 调整 |
| 电机 ENB | GPIO33 | **GPIO2** | 调整 |
```

---

## UWB模块配置

### 模块角色说明
- **主机（基站/Anchor）**: 佩戴在人身上，主动发起测距
- **从机（标签/Tag）**: 安装在小车上，响应测距请求并获取距离

### 配置架构：1主2从

| 位置 | 角色 | 地址 | 说明 |
|------|------|------|------|
| 人身上 | 主机 | 0000 | 发起测距请求 |
| 小车右前 | 从机0 | 0001 | 输出到ESP32的Serial2 |
| 小车左前 | 从机1 | 0002 | 输出到ESP32的Serial1 |

小车上的两个从机模块会各自通过串口输出它们到主机（人）的距离。

### 配置参数

#### 人身上的UWB模块（主机）

```
AT+MODE=0        // 进入AT配置模式
AT+ROLE=1        // 设置为主机模式
AT+PID=255       // 网络ID
AT+PERIOD=5      // 测距周期50ms
AT+MADDR=0000    // 本机地址
AT+SADDR0=0001   // 从机0地址（小车右前）
AT+SADDR1=0002   // 从机1地址（小车左前）
AT+UART=115200   // 波特率
AT+RST           // 复位生效
```

#### 小车右前方的UWB模块（从机0）

```
AT+MODE=0
AT+ROLE=0        // 从机模式
AT+PID=255       // 网络ID必须相同
AT+PERIOD=5      // 测距周期必须相同
AT+MADDR=0001    // 本机地址（从机模式下MADDR是本机地址）
AT+SADDR0=0000   // 主机地址
AT+UART=115200
AT+RST
```

#### 小车左前方的UWB模块（从机1）

```
AT+MODE=0
AT+ROLE=0        // 从机模式
AT+PID=255
AT+PERIOD=5
AT+MADDR=0002    // 本机地址
AT+SADDR0=0000   // 主机地址
AT+UART=115200
AT+RST
```

### UWB数据帧格式

模块输出的距离数据为二进制格式：

| 字节 | 内容 | 说明 |
|------|------|------|
| 0 | 0xF0 | 帧头 |
| 1 | 0x05 | 有效数据长度 |
| 2 | 地址低位 | 发送方地址 |
| 3 | 地址高位 | |
| 4 | 距离低位 | 单位：厘米 |
| 5 | 距离高位 | |
| 6 | 信号强度 | RSSI = 值 - 256 (dBm) |
| 7 | 0xAA | 帧尾 |

**解析示例：**
```cpp
// 接收到：F0 05 00 00 2D 00 B7 AA
// 地址：0x0000
// 距离：0x002D = 45cm
// RSSI：0xB7 - 256 = -73dBm
```

---

## UWB双基站定位算法

### 几何原理

两个UWB基站安装在小车前端，间距为 L（约25-30cm），分别测得到标签的距离 d0 和 d1。

```
        人（标签）
           *
          /|\
         / | \
      d0/  |  \d1
       /   |y  \
      /    |    \
   [从机0]----[从机1]
      ←--- L ---→
         小车前端
```

### 坐标计算

以两个基站连线中点为原点，建立坐标系：

```cpp
// 基站间距 L (cm)
const float L = 25.0;

// 计算标签相对于小车的坐标
float x = (d0*d0 - d1*d1) / (2*L);  // 左右偏移，正值偏右
float y_sq = d0*d0 - (x + L/2)*(x + L/2);
float y = (y_sq > 0) ? sqrt(y_sq) : 0;  // 前方距离
```

### 跟随控制策略

```cpp
// 参数
const float FOLLOW_DISTANCE = 80.0;   // 目标跟随距离 (cm)
const float DEAD_ZONE = 20.0;         // 死区范围 (cm)
const float ANGLE_DEAD_ZONE = 10.0;   // 角度死区 (度)

// 计算距离和角度
float distance = sqrt(x*x + y*y);
float angle = atan2(x, y) * 180.0 / PI;  // 正值表示目标在右边

// 控制逻辑
if (distance > FOLLOW_DISTANCE + DEAD_ZONE) {
    // 太远了，前进
    if (angle > ANGLE_DEAD_ZONE) {
        // 目标在右边，右转前进
        turnRight(speed);
    } else if (angle < -ANGLE_DEAD_ZONE) {
        // 目标在左边，左转前进
        turnLeft(speed);
    } else {
        // 直行
        goForward(speed);
    }
} else if (distance < FOLLOW_DISTANCE - DEAD_ZONE) {
    // 太近了，后退
    goBackward(speed);
} else {
    // 在合适范围内，停止
    stop();
}
```

---

## 姿态检测算法（背负模式）

使用MPU6050检测学生背书包时的姿态问题。

### 检测指标

1. **弯腰/驼背检测**：Y轴倾斜角度
2. **高低肩检测**：X轴倾斜角度

```cpp
// 阈值设定
const float BEND_THRESHOLD = 20.0;     // 弯腰阈值（度）
const float SHOULDER_THRESHOLD = 10.0; // 高低肩阈值（度）

// 从MPU6050读取加速度
float ax = accel.x;
float ay = accel.y;
float az = accel.z;

// 计算倾斜角度
float pitch = atan2(ay, sqrt(ax*ax + az*az)) * 180.0 / PI;  // 前后倾斜
float roll = atan2(ax, sqrt(ay*ay + az*az)) * 180.0 / PI;   // 左右倾斜

// 检测
if (abs(pitch) > BEND_THRESHOLD) {
    // 提醒：检测到弯腰驼背
}
if (abs(roll) > SHOULDER_THRESHOLD) {
    // 提醒：检测到高低肩
}
```

---

## 路径示教与归位（简化版）

由于没有编码器，使用**时间记录法**实现简化版路径示教。

### 示教模式

通过蓝牙接收手机APP的遥控指令，记录每个动作和持续时间：

```cpp
struct PathStep {
    uint8_t action;    // 动作: 0=停止, 1=前进, 2=后退, 3=左转, 4=右转
    uint16_t duration; // 持续时间 (ms)
};

PathStep path[100];  // 最多记录100步
int pathLength = 0;
```

### 归位模式

按记录的路径反向执行：

```cpp
void executeReturn() {
    for (int i = pathLength - 1; i >= 0; i--) {
        uint8_t reverseAction = reverseActionMap[path[i].action];
        executeAction(reverseAction, path[i].duration);
    }
}

// 动作反转映射
// 前进 ↔ 后退
// 左转 ↔ 右转
```

**注意**：这种方法精度有限，误差会累积，仅适用于短距离（1-2米）归位。

---

## 蓝牙通信协议

用于路径示教模式，手机APP通过蓝牙发送遥控指令。

### 指令格式

| 指令 | 含义 |
|------|------|
| 'F' | 前进 |
| 'B' | 后退 |
| 'L' | 左转 |
| 'R' | 右转 |
| 'S' | 停止 |
| 'P' | 开始记录路径 |
| 'E' | 结束记录路径 |

推荐手机APP：**蓝牙调试器**（Android）

---

## 代码架构建议

```
project/
├── src/
│   ├── main.cpp              // 主程序，模式切换逻辑
│   ├── config.h              // 引脚定义、参数配置
│   ├── uwb.h / uwb.cpp       // UWB数据读取和解析
│   ├── motor.h / motor.cpp   // 电机控制
│   ├── imu.h / imu.cpp       // MPU6050姿态检测
│   ├── weight.h / weight.cpp // HX711称重
│   ├── display.h / display.cpp // OLED显示
│   ├── follow.h / follow.cpp // 跟随算法
│   └── path.h / path.cpp     // 路径示教和归位
├── platformio.ini            // PlatformIO配置
└── README.md
```

### 推荐库

```ini
; platformio.ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    adafruit/Adafruit SSD1306
    adafruit/Adafruit GFX Library
    adafruit/Adafruit MPU6050
    bogde/HX711
```

---

## 开发优先级

建议按以下顺序开发和测试：

1. **基础框架**：模式切换 + OLED显示
2. **电机控制**：前进、后退、转向、PWM调速
3. **称重功能**：HX711读取和校准
4. **姿态检测**：MPU6050读取和角度计算
5. **UWB通信**：数据接收和解析
6. **跟随算法**：定位计算和控制逻辑
7. **蓝牙遥控**：路径示教功能
8. **归位功能**：路径记录和回放

---

## 注意事项

1. **串口初始化**：ESP32的Serial1需要手动指定引脚
   ```cpp
   Serial1.begin(115200, SERIAL_8N1, 27, 26);  // RX=27, TX=26
   Serial2.begin(115200, SERIAL_8N1, 16, 17);  // RX=16, TX=17
   ```

2. **UWB模块供电**：必须使用3.3V，不能接5V

3. **I2C地址**：
   - MPU6050: 0x68 (AD0接GND) 或 0x69 (AD0接VCC)
   - SSD1306: 0x3C 或 0x3D

4. **电机共地**：L298N的GND必须与ESP32共地

5. **PWM频率**：ESP32建议使用5000Hz，分辨率8位

6. **UWB测距精度**：标称30cm，实际使用中可能有波动，建议加滤波

---

## 测试检查清单

- [ ] ESP32能正常上电启动
- [ ] OLED显示正常
- [ ] 按钮能切换模式
- [ ] 电机能正反转
- [ ] PWM调速有效
- [ ] HX711能读取重量
- [ ] MPU6050能读取姿态
- [ ] UWB从机0能收到距离数据
- [ ] UWB从机1能收到距离数据
- [ ] 跟随模式能正常工作
- [ ] 蓝牙能连接手机
- [ ] 路径示教能记录
- [ ] 归位能执行
