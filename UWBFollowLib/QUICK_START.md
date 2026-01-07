# UWBFollow 库 - 快速开始

## 📦 安装

将 `UWBFollowLib` 文件夹复制到你的项目的 `lib` 目录。

## 🚀 三步使用

### 1️⃣ 实现电机接口

```cpp
#include <UWBFollow.h>

class MyMotor : public MotorInterface {
public:
    void forward() override { /* 你的代码 */ }
    void backward() override { /* 你的代码 */ }
    void turnLeft() override { /* 你的代码 */ }
    void turnRight() override { /* 你的代码 */ }
    void stop() override { /* 你的代码 */ }
};
```

### 2️⃣ 初始化

```cpp
MyMotor motor;
UWBSensor uwb;
FollowController follow(&motor);

void setup() {
    UWBConfig uwbCfg;
    uwbCfg.baseline = 28.0f;  // UWB 模块间距
    uwb.begin(uwbCfg);

    FollowConfig followCfg;
    follow.begin(followCfg);
}
```

### 3️⃣ 更新

```cpp
void loop() {
    uwb.update();
    if (uwb.isConnected()) {
        UWBData data = uwb.getData();
        follow.update(data.distance, data.angle);
    }
}
```

## 📝 完整示例

参考 `examples/BasicFollow/BasicFollow.ino`

## 📚 详细文档

- **README.md** - 完整文档
- **USAGE.md** - 详细使用指南
- **examples/** - 示例代码

## ⚙️ 常用配置

```cpp
// UWB 配置
UWBConfig uwbCfg;
uwbCfg.baseline = 28.0f;           // 模块间距 (cm)
uwbCfg.rx0Pin = 16;                // UWB0 RX 引脚
uwbCfg.tx0Pin = 17;                // UWB0 TX 引脚
uwbCfg.rx1Pin = 27;                // UWB1 RX 引脚
uwbCfg.tx1Pin = 13;                // UWB1 TX 引脚

// 跟随配置
FollowConfig followCfg;
followCfg.targetDistance = 80.0f;  // 目标距离 (cm)
followCfg.turnOnAngle = 35.0f;     // 转向角度 (度)
```

## 🔧 调试

```cpp
uwb.setDebug(true);     // 启用 UWB 调试
follow.setDebug(true);  // 启用跟随控制调试
```

## 📌 硬件连接

```
ESP32          UWB0        UWB1
GPIO16  <-->   TX
GPIO17  <-->   RX
GPIO27  <-->              TX
GPIO13  <-->              RX
```

两个 UWB 模块相距 28cm，面向前方。
