# UWBFollow Library

UWB-based autonomous follow control library for ESP32. Perfect for robots, autonomous vehicles, and follow-me systems.

## Features

- **Dual UWB Positioning**: Triangulation using two UWB modules for accurate distance and angle calculation
- **Smart Follow Control**: Bang-bang controller with low-pass filtering and hysteresis for smooth tracking
- **Motor Abstraction**: Easy-to-implement motor interface for any motor driver
- **Configurable**: Fully customizable parameters for different use cases
- **Production Ready**: Extracted from a tested smart backpack project

## Hardware Requirements

- ESP32 Development Board
- 2× UWB positioning modules (e.g., DW1000, DWM1000)
- Motor driver (e.g., L298N, TB6612FNG)
- DC motors

## Installation

### Arduino IDE
1. Download this library as ZIP
2. In Arduino IDE: Sketch → Include Library → Add .ZIP Library
3. Select the downloaded ZIP file

### PlatformIO
1. Copy the `UWBFollowLib` folder to your project's `lib` directory
2. Or add to `platformio.ini`:
```ini
lib_deps =
    file://path/to/UWBFollowLib
```

## Quick Start

### 1. Implement Motor Interface

Create a motor class that implements the `MotorInterface`:

```cpp
#include <UWBFollow.h>

class MyMotor : public MotorInterface {
public:
    void forward() override {
        // Your motor forward code
    }

    void backward() override {
        // Your motor backward code
    }

    void turnLeft() override {
        // Your motor turn left code
    }

    void turnRight() override {
        // Your motor turn right code
    }

    void stop() override {
        // Your motor stop code
    }
};
```

### 2. Setup and Use

```cpp
#include <UWBFollow.h>

MyMotor motor;
UWBSensor uwb;
FollowController follow(&motor);

void setup() {
    Serial.begin(115200);

    // Configure UWB
    UWBConfig uwbCfg;
    uwbCfg.baseline = 28.0f;  // Distance between two UWB modules (cm)
    uwb.begin(uwbCfg);

    // Configure follow controller
    FollowConfig followCfg;
    followCfg.targetDistance = 80.0f;  // Target follow distance (cm)
    follow.begin(followCfg);
}

void loop() {
    uwb.update();

    if (uwb.isConnected()) {
        UWBData data = uwb.getData();
        follow.update(data.distance, data.angle);
    } else {
        follow.stop();
    }
}
```

## Configuration

### UWB Configuration

```cpp
UWBConfig uwbCfg;
uwbCfg.serial0 = &Serial2;           // Serial port for UWB0
uwbCfg.serial1 = &Serial1;           // Serial port for UWB1
uwbCfg.baudRate = 115200;            // Serial baud rate
uwbCfg.rx0Pin = 16;                  // RX pin for UWB0
uwbCfg.tx0Pin = 17;                  // TX pin for UWB0
uwbCfg.rx1Pin = 27;                  // RX pin for UWB1
uwbCfg.tx1Pin = 13;                  // TX pin for UWB1
uwbCfg.baseline = 28.0f;             // Distance between modules (cm)
uwbCfg.distanceScale = 1.0f;         // Distance scaling factor
uwbCfg.hexScale = 0.1f;              // Hex format scaling
uwbCfg.angleInvert = true;           // Invert angle direction
uwbCfg.angleOffset = 0.0f;           // Angle offset (degrees)
uwbCfg.timeoutMs = 2000;             // Connection timeout (ms)
```

### Follow Controller Configuration

```cpp
FollowConfig followCfg;
followCfg.targetDistance = 80.0f;    // Target follow distance (cm)
followCfg.distanceDeadzone = 15.0f;  // Distance deadzone (cm)
followCfg.angleDeadzone = 15.0f;     // Angle deadzone (degrees)
followCfg.minDistance = 40.0f;       // Minimum safe distance (cm)
followCfg.enableDistance = 100.0f;   // Stop if closer than this (cm)
followCfg.filterAlpha = 0.1f;        // Low-pass filter coefficient (0-1)
followCfg.turnOnAngle = 35.0f;       // Start turning at this angle (degrees)
followCfg.turnOffAngle = 15.0f;      // Stop turning at this angle (degrees)
followCfg.commandHoldMs = 300;       // Minimum command duration (ms)
```

## Hardware Connection

### Typical ESP32 + L298N Setup

```
ESP32          UWB Module 0
GPIO16  <-->   TX
GPIO17  <-->   RX

ESP32          UWB Module 1
GPIO27  <-->   TX
GPIO13  <-->   RX

ESP32          L298N
GPIO5   <-->   IN1 (Left Motor +)
GPIO14  <-->   IN2 (Left Motor -)
GPIO32  <-->   IN3 (Right Motor +)
GPIO33  <-->   IN4 (Right Motor -)
```

### UWB Module Placement

```
    [UWB0]  28cm  [UWB1]
       |           |
       +-----+-----+
             |
          [Robot]
```

Place two UWB modules on a baseline (default 28cm apart) facing forward.

## API Reference

### UWBSensor Class

#### Methods

- `bool begin(const UWBConfig& config)` - Initialize UWB sensor
- `void update()` - Update sensor data (call in loop)
- `UWBData getData() const` - Get current positioning data
- `bool isConnected() const` - Check if UWB is connected
- `void setDebug(bool enable)` - Enable/disable debug output

#### UWBData Structure

```cpp
struct UWBData {
    float d0;              // Distance from module 0 (cm)
    float d1;              // Distance from module 1 (cm)
    float distance;        // Calculated perpendicular distance (cm)
    float angle;           // Calculated angle (degrees, 0=forward, +right/-left)
    bool valid;            // Data validity flag
    unsigned long lastUpdate;  // Last update timestamp (ms)
};
```

### FollowController Class

#### Methods

- `FollowController(MotorInterface* motor)` - Constructor
- `void begin(const FollowConfig& config)` - Initialize controller
- `void update(float distance, float angle)` - Update follow control
- `void stop()` - Stop following
- `void setDebug(bool enable)` - Enable/disable debug output
- `float getFilteredDistance() const` - Get filtered distance
- `float getFilteredAngle() const` - Get filtered angle

### MotorInterface Class

Pure virtual interface - implement all methods:

- `virtual void forward() = 0` - Move forward
- `virtual void backward() = 0` - Move backward
- `virtual void turnLeft() = 0` - Turn left
- `virtual void turnRight() = 0` - Turn right
- `virtual void stop() = 0` - Stop all motors

## Algorithm Details

### Triangulation

Uses two UWB modules to calculate target position:

```
        Target (x, y)
          /|\
         / | \
       d0  |  d1
       /   y   \
      /    |    \
  [UWB0]--L/2--[UWB1]
```

Calculate:
- `x = (d0² - d1²) / (2L)` - lateral offset
- `y = sqrt(d0² - (x + L/2)²)` - perpendicular distance
- `angle = atan2(x, y)` - angle in degrees

### Follow Control

Bang-bang controller with:
- **Low-pass filtering**: Smooth noisy sensor data
- **Hysteresis**: Prevent rapid switching between turn/straight
- **Command hold**: Maintain commands for minimum duration
- **Safety zones**: Stop if too close or too far

Control logic:
1. If distance ≤ enableDistance: **STOP**
2. If |angle| > turnOnAngle: **TURN** (left/right)
3. If distance > target + deadzone: **FORWARD**
4. Otherwise: **STOP**

## Troubleshooting

### No UWB Data
- Check serial connections (RX/TX)
- Verify baud rate matches UWB module
- Enable debug mode: `uwb.setDebug(true)`
- Check power supply to UWB modules

### Robot Not Moving
- Verify motor driver connections
- Check motor power supply
- Enable debug mode: `follow.setDebug(true)`
- Test motor interface directly

### Unstable Following
- Increase filter alpha (smoother but slower): `0.15-0.2`
- Adjust turn angles (wider hysteresis): `turnOnAngle=40, turnOffAngle=10`
- Increase command hold time: `commandHoldMs=400`

### Wrong Direction
- Invert angle: `angleInvert = !angleInvert`
- Swap UWB modules
- Adjust angle offset: `angleOffset = 180.0f`

## Examples

See `examples/` folder for complete working examples:
- **BasicFollow** - Complete example with L298N motor driver

## Credits

Based on the Smart Backpack Project - an intelligent autonomous following backpack system.

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Please open an issue or pull request.

## Support

For issues and questions:
- GitHub Issues: [Your repo issues page]
- Documentation: [Your documentation link]
