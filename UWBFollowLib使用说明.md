# UWBFollowLib 使用说明

## 📦 库说明

`UWBFollowLib` 是从智能书包项目中提取的 **UWB 跟随功能独立库**，包含：

- **UWB 双基站定位**：三角定位算法
- **跟随控制算法**：带滤波和迟滞的 Bang-bang 控制器
- **电机接口抽象**：支持任意电机驱动器

## 📂 库结构

```
UWBFollowLib/
├── src/                          # 源代码
│   ├── UWBFollow.h              # 主头文件（包含所有功能）
│   ├── UWBSensor.h/.cpp         # UWB 定位传感器
│   ├── FollowController.h/.cpp  # 跟随控制器
│   └── MotorInterface.h         # 电机接口（需要你实现）
├── examples/                     # 示例代码
│   └── BasicFollow/
│       └── BasicFollow.ino      # 完整示例（含 L298N 驱动）
├── README.md                     # 完整文档
├── USAGE.md                      # 详细使用指南
├── QUICK_START.md               # 快速开始
├── LICENSE                       # MIT 许可证
└── library.properties           # 库配置文件
```

## 🚀 如何使用

### 方式一：在 PlatformIO 项目中使用

1. 将 `UWBFollowLib` 文件夹复制到你的项目的 `lib` 目录
2. 在你的代码中引入：

```cpp
#include <UWBFollow.h>
```

### 方式二：在 Arduino IDE 中使用

1. 将 `UWBFollowLib` 复制到 Arduino 库目录：
   - Windows: `Documents/Arduino/libraries/`
   - macOS: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`

2. 重启 Arduino IDE

3. 在你的 sketch 中引入：

```cpp
#include <UWBFollow.h>
```

## 💡 最简使用示例

```cpp
#include <UWBFollow.h>

// 1. 实现你的电机接口
class MyMotor : public MotorInterface {
public:
    void forward() override { /* 前进 */ }
    void backward() override { /* 后退 */ }
    void turnLeft() override { /* 左转 */ }
    void turnRight() override { /* 右转 */ }
    void stop() override { /* 停止 */ }
};

// 2. 创建对象
MyMotor motor;
UWBSensor uwb;
FollowController follow(&motor);

void setup() {
    // 3. 配置并初始化
    UWBConfig uwbCfg;
    uwbCfg.baseline = 28.0f;
    uwb.begin(uwbCfg);

    FollowConfig followCfg;
    follow.begin(followCfg);
}

void loop() {
    // 4. 更新
    uwb.update();
    if (uwb.isConnected()) {
        UWBData data = uwb.getData();
        follow.update(data.distance, data.angle);
    }
}
```

## 📚 文档位置

| 文档 | 位置 | 说明 |
|------|------|------|
| 完整文档 | `UWBFollowLib/README.md` | API 参考、算法说明、故障排除 |
| 使用指南 | `UWBFollowLib/USAGE.md` | 详细使用步骤、电机驱动示例 |
| 快速开始 | `UWBFollowLib/QUICK_START.md` | 三步上手 |
| 示例代码 | `UWBFollowLib/examples/` | 完整可运行示例 |

## 🔧 主要配置参数

### UWB 配置

```cpp
UWBConfig cfg;
cfg.baseline = 28.0f;        // UWB 模块间距 (cm)
cfg.rx0Pin = 16;             // UWB0 RX 引脚
cfg.tx0Pin = 17;             // UWB0 TX 引脚
cfg.rx1Pin = 27;             // UWB1 RX 引脚
cfg.tx1Pin = 13;             // UWB1 TX 引脚
cfg.angleInvert = true;      // 角度反向
```

### 跟随控制配置

```cpp
FollowConfig cfg;
cfg.targetDistance = 80.0f;   // 目标距离 (cm)
cfg.distanceDeadzone = 15.0f; // 距离死区 (cm)
cfg.turnOnAngle = 35.0f;      // 转向触发角度 (度)
cfg.filterAlpha = 0.1f;       // 滤波系数 (0-1)
```

## 🎯 与原项目的集成

如果你想在原项目中使用这个库：

```cpp
// 原来的代码：
#include "uwb.h"
#include "follow.h"
#include "motor.h"

// 改为：
#include <UWBFollow.h>

// 实现一个适配器类：
class MotorAdapter : public MotorInterface {
private:
    Motor* originalMotor;
public:
    MotorAdapter(Motor* m) : originalMotor(m) {}
    void forward() override { originalMotor->forward(); }
    void backward() override { originalMotor->backward(); }
    void turnLeft() override { originalMotor->turnLeft(); }
    void turnRight() override { originalMotor->turnRight(); }
    void stop() override { originalMotor->stop(); }
};
```

## ⚙️ Git 信息

库已经初始化为独立的 Git 仓库：

```bash
cd UWBFollowLib
git log  # 查看提交历史
```

如果需要推送到 GitHub：

```bash
cd UWBFollowLib
git remote add origin https://github.com/your-username/UWBFollowLib.git
git push -u origin master
```

## 📋 特性总结

| 特性 | 说明 |
|------|------|
| 独立性 | 无需原项目依赖，可直接使用 |
| 可配置 | 所有参数都可以自定义 |
| 兼容性 | 支持 Arduino IDE 和 PlatformIO |
| 文档完整 | 包含详细文档和示例 |
| 可扩展 | 电机接口支持任意驱动器 |
| 调试友好 | 内置调试输出功能 |

## 🎓 适用场景

- 跟随机器人
- 自动跟随小车
- 智能购物车
- 跟随式行李箱
- 任何需要 UWB 跟随功能的项目

## 💬 支持

有问题请查看：
1. `UWBFollowLib/README.md` - 完整文档
2. `UWBFollowLib/USAGE.md` - 详细使用指南
3. `UWBFollowLib/examples/` - 示例代码

---

**提取完成日期**：2026-01-07
**库版本**：1.0.0
**基于项目**：智能自动跟随书包
