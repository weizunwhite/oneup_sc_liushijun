# UWBFollow 库使用指南

## 如何在你的项目中使用这个库

### 方式一：PlatformIO 本地库

1. 将 `UWBFollowLib` 文件夹复制到你的项目的 `lib` 目录下：

```
your_project/
├── lib/
│   └── UWBFollowLib/        # 复制整个文件夹到这里
│       ├── src/
│       ├── examples/
│       ├── library.properties
│       └── README.md
├── src/
│   └── main.cpp
└── platformio.ini
```

2. 在你的代码中引入：

```cpp
#include <UWBFollow.h>
```

PlatformIO 会自动发现并编译这个库。

### 方式二：Arduino IDE

1. 将 `UWBFollowLib` 文件夹复制到 Arduino 库目录：
   - Windows: `Documents/Arduino/libraries/`
   - macOS: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`

2. 重启 Arduino IDE

3. 在你的 sketch 中：

```cpp
#include <UWBFollow.h>
```

## 基本使用步骤

### 第一步：实现电机接口

根据你的电机驱动器，实现 `MotorInterface`：

```cpp
class MyMotorDriver : public MotorInterface {
public:
    void begin() {
        // 初始化你的电机驱动器
        // 设置引脚、PWM等
    }

    void forward() override {
        // 前进的代码
    }

    void backward() override {
        // 后退的代码
    }

    void turnLeft() override {
        // 左转的代码
    }

    void turnRight() override {
        // 右转的代码
    }

    void stop() override {
        // 停止的代码
    }
};
```

### 第二步：在 setup() 中初始化

```cpp
MyMotorDriver motor;
UWBSensor uwb;
FollowController follow(&motor);

void setup() {
    Serial.begin(115200);

    // 1. 初始化电机
    motor.begin();

    // 2. 配置并初始化 UWB
    UWBConfig uwbCfg;
    uwbCfg.baseline = 28.0f;  // 两个 UWB 模块的间距 (cm)
    uwbCfg.rx0Pin = 16;       // 根据你的接线修改
    uwbCfg.tx0Pin = 17;
    uwbCfg.rx1Pin = 27;
    uwbCfg.tx1Pin = 13;
    uwb.begin(uwbCfg);

    // 3. 配置并初始化跟随控制器
    FollowConfig followCfg;
    followCfg.targetDistance = 80.0f;  // 目标距离 80cm
    follow.begin(followCfg);

    Serial.println("系统就绪!");
}
```

### 第三步：在 loop() 中更新

```cpp
void loop() {
    // 更新 UWB 数据
    uwb.update();

    // 如果 UWB 连接正常
    if (uwb.isConnected()) {
        // 获取定位数据
        UWBData data = uwb.getData();

        // 更新跟随控制器
        follow.update(data.distance, data.angle);
    } else {
        // UWB 未连接，停止
        follow.stop();
    }
}
```

## 常见电机驱动器示例

### L298N 电机驱动器

```cpp
class L298NMotor : public MotorInterface {
private:
    const uint8_t IN1 = 5, IN2 = 14, IN3 = 32, IN4 = 33;
    const uint8_t PWM_CH[4] = {0, 1, 2, 3};
    uint8_t speed = 150;

public:
    void begin() {
        for (int i = 0; i < 4; i++) {
            ledcSetup(PWM_CH[i], 2000, 8);
        }
        ledcAttachPin(IN1, PWM_CH[0]);
        ledcAttachPin(IN2, PWM_CH[1]);
        ledcAttachPin(IN3, PWM_CH[2]);
        ledcAttachPin(IN4, PWM_CH[3]);
        stop();
    }

    void forward() override {
        ledcWrite(PWM_CH[0], speed);
        ledcWrite(PWM_CH[1], 0);
        ledcWrite(PWM_CH[2], speed);
        ledcWrite(PWM_CH[3], 0);
    }

    void backward() override {
        ledcWrite(PWM_CH[0], 0);
        ledcWrite(PWM_CH[1], speed);
        ledcWrite(PWM_CH[2], 0);
        ledcWrite(PWM_CH[3], speed);
    }

    void turnLeft() override {
        ledcWrite(PWM_CH[0], 0);
        ledcWrite(PWM_CH[1], speed);
        ledcWrite(PWM_CH[2], speed);
        ledcWrite(PWM_CH[3], 0);
    }

    void turnRight() override {
        ledcWrite(PWM_CH[0], speed);
        ledcWrite(PWM_CH[1], 0);
        ledcWrite(PWM_CH[2], 0);
        ledcWrite(PWM_CH[3], speed);
    }

    void stop() override {
        for (int i = 0; i < 4; i++) {
            ledcWrite(PWM_CH[i], 0);
        }
    }
};
```

### TB6612FNG 电机驱动器

```cpp
class TB6612Motor : public MotorInterface {
private:
    const uint8_t AIN1 = 5, AIN2 = 14, BIN1 = 32, BIN2 = 33;
    const uint8_t PWMA = 18, PWMB = 19;
    uint8_t speed = 200;

public:
    void begin() {
        pinMode(AIN1, OUTPUT);
        pinMode(AIN2, OUTPUT);
        pinMode(BIN1, OUTPUT);
        pinMode(BIN2, OUTPUT);

        ledcSetup(0, 2000, 8);
        ledcSetup(1, 2000, 8);
        ledcAttachPin(PWMA, 0);
        ledcAttachPin(PWMB, 1);
        stop();
    }

    void forward() override {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        ledcWrite(0, speed);
        ledcWrite(1, speed);
    }

    void backward() override {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        ledcWrite(0, speed);
        ledcWrite(1, speed);
    }

    void turnLeft() override {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        ledcWrite(0, speed);
        ledcWrite(1, speed);
    }

    void turnRight() override {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        ledcWrite(0, speed);
        ledcWrite(1, speed);
    }

    void stop() override {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, LOW);
        ledcWrite(0, 0);
        ledcWrite(1, 0);
    }
};
```

## 调试技巧

### 启用调试输出

```cpp
uwb.setDebug(true);      // UWB 传感器调试信息
follow.setDebug(true);   // 跟随控制器调试信息
```

### 打印定位数据

```cpp
void loop() {
    uwb.update();

    if (uwb.isConnected()) {
        UWBData data = uwb.getData();

        Serial.printf("距离: %.1f cm, 角度: %.1f 度\n",
                      data.distance, data.angle);
        Serial.printf("UWB0: %.1f cm, UWB1: %.1f cm\n",
                      data.d0, data.d1);

        follow.update(data.distance, data.angle);
    }
}
```

### 测试电机

在使用跟随功能前，先单独测试电机：

```cpp
void testMotors() {
    Serial.println("测试前进");
    motor.forward();
    delay(2000);

    Serial.println("测试停止");
    motor.stop();
    delay(1000);

    Serial.println("测试后退");
    motor.backward();
    delay(2000);

    motor.stop();
    delay(1000);

    Serial.println("测试左转");
    motor.turnLeft();
    delay(2000);

    motor.stop();
    delay(1000);

    Serial.println("测试右转");
    motor.turnRight();
    delay(2000);

    motor.stop();
}
```

## 参数调优

### 跟随距离调整

```cpp
FollowConfig cfg;
cfg.targetDistance = 100.0f;     // 增加目标距离
cfg.distanceDeadzone = 20.0f;    // 增加死区，减少频繁启停
cfg.minDistance = 50.0f;         // 提高最小安全距离
```

### 转向灵敏度调整

```cpp
FollowConfig cfg;
cfg.turnOnAngle = 30.0f;   // 降低值 = 更灵敏（容易转向）
cfg.turnOffAngle = 10.0f;  // 降低值 = 更灵敏
```

### 响应速度调整

```cpp
FollowConfig cfg;
cfg.filterAlpha = 0.2f;      // 增加值 = 响应更快但更抖
cfg.commandHoldMs = 200;     // 减少值 = 响应更快
```

## 故障排除

### 问题：UWB 无数据
- 检查串口接线 (RX/TX)
- 检查波特率是否匹配
- 启用 `uwb.setDebug(true)` 查看原始数据
- 检查 UWB 模块供电

### 问题：机器人不动
- 测试电机驱动器是否工作
- 检查电机供电
- 启用 `follow.setDebug(true)` 查看控制指令
- 检查是否在跟随范围内（距离 > 100cm）

### 问题：跟随不稳定
- 增加滤波系数 `filterAlpha = 0.15`
- 增大转向迟滞 `turnOnAngle=40, turnOffAngle=10`
- 增加指令保持时间 `commandHoldMs=400`

### 问题：方向错误
- 切换 `angleInvert` 的值
- 调整 `angleOffset`
- 交换两个 UWB 模块的位置

## 完整示例

参考 `examples/BasicFollow/BasicFollow.ino` 获取完整的可运行示例。
