/**
 * @file BasicFollow.ino
 * @brief Basic example for UWBFollow library
 * @details Shows how to implement motor interface and use the library
 */

#include <UWBFollow.h>

// ============================================
// Step 1: Implement MotorInterface
// ============================================

class L298NMotor : public MotorInterface {
private:
    // L298N motor driver pins
    const uint8_t LEFT_IN1 = 5;
    const uint8_t LEFT_IN2 = 14;
    const uint8_t RIGHT_IN1 = 32;
    const uint8_t RIGHT_IN2 = 33;

    // PWM channels
    const uint8_t CH_LEFT_IN1 = 0;
    const uint8_t CH_LEFT_IN2 = 1;
    const uint8_t CH_RIGHT_IN1 = 2;
    const uint8_t CH_RIGHT_IN2 = 3;

    const uint32_t PWM_FREQ = 2000;
    const uint8_t PWM_RES = 8;
    uint8_t speed = 150;  // 0-255

public:
    void begin() {
        // Setup PWM channels
        ledcSetup(CH_LEFT_IN1, PWM_FREQ, PWM_RES);
        ledcSetup(CH_LEFT_IN2, PWM_FREQ, PWM_RES);
        ledcSetup(CH_RIGHT_IN1, PWM_FREQ, PWM_RES);
        ledcSetup(CH_RIGHT_IN2, PWM_FREQ, PWM_RES);

        // Attach pins
        ledcAttachPin(LEFT_IN1, CH_LEFT_IN1);
        ledcAttachPin(LEFT_IN2, CH_LEFT_IN2);
        ledcAttachPin(RIGHT_IN1, CH_RIGHT_IN1);
        ledcAttachPin(RIGHT_IN2, CH_RIGHT_IN2);

        stop();
        Serial.println("Motor initialized");
    }

    void setSpeed(uint8_t spd) {
        speed = spd;
    }

    void forward() override {
        ledcWrite(CH_LEFT_IN1, speed);
        ledcWrite(CH_LEFT_IN2, 0);
        ledcWrite(CH_RIGHT_IN1, speed);
        ledcWrite(CH_RIGHT_IN2, 0);
    }

    void backward() override {
        ledcWrite(CH_LEFT_IN1, 0);
        ledcWrite(CH_LEFT_IN2, speed);
        ledcWrite(CH_RIGHT_IN1, 0);
        ledcWrite(CH_RIGHT_IN2, speed);
    }

    void turnLeft() override {
        ledcWrite(CH_LEFT_IN1, 0);
        ledcWrite(CH_LEFT_IN2, speed);
        ledcWrite(CH_RIGHT_IN1, speed);
        ledcWrite(CH_RIGHT_IN2, 0);
    }

    void turnRight() override {
        ledcWrite(CH_LEFT_IN1, speed);
        ledcWrite(CH_LEFT_IN2, 0);
        ledcWrite(CH_RIGHT_IN1, 0);
        ledcWrite(CH_RIGHT_IN2, speed);
    }

    void stop() override {
        ledcWrite(CH_LEFT_IN1, 0);
        ledcWrite(CH_LEFT_IN2, 0);
        ledcWrite(CH_RIGHT_IN1, 0);
        ledcWrite(CH_RIGHT_IN2, 0);
    }
};

// ============================================
// Step 2: Create objects
// ============================================

L298NMotor motor;
UWBSensor uwb;
FollowController follow(&motor);

// ============================================
// Step 3: Setup
// ============================================

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n========================================");
    Serial.println("   UWBFollow Library - Basic Example   ");
    Serial.println("========================================\n");

    // Initialize motor
    motor.begin();
    motor.setSpeed(150);  // Set speed (0-255)

    // Configure UWB sensor
    UWBConfig uwbCfg;
    uwbCfg.serial0 = &Serial2;
    uwbCfg.serial1 = &Serial1;
    uwbCfg.baudRate = 115200;
    uwbCfg.rx0Pin = 16;
    uwbCfg.tx0Pin = 17;
    uwbCfg.rx1Pin = 27;
    uwbCfg.tx1Pin = 13;
    uwbCfg.baseline = 28.0f;         // Distance between UWB modules (cm)
    uwbCfg.distanceScale = 1.0f;     // Distance scaling factor
    uwbCfg.hexScale = 0.1f;          // Hex format scaling
    uwbCfg.angleInvert = true;       // Invert angle direction
    uwbCfg.angleOffset = 0.0f;       // Angle offset
    uwbCfg.timeoutMs = 2000;         // Connection timeout

    // Initialize UWB
    uwb.begin(uwbCfg);
    uwb.setDebug(false);  // Set to true for debug output

    // Configure follow controller
    FollowConfig followCfg;
    followCfg.targetDistance = 80.0f;      // Target follow distance (cm)
    followCfg.distanceDeadzone = 15.0f;    // Distance deadzone (cm)
    followCfg.angleDeadzone = 15.0f;       // Angle deadzone (degrees)
    followCfg.minDistance = 40.0f;         // Minimum distance (cm)
    followCfg.enableDistance = 100.0f;     // Stop distance threshold (cm)
    followCfg.filterAlpha = 0.1f;          // Filter coefficient (0-1)
    followCfg.turnOnAngle = 35.0f;         // Start turning angle (degrees)
    followCfg.turnOffAngle = 15.0f;        // Stop turning angle (degrees)
    followCfg.commandHoldMs = 300;         // Command hold time (ms)

    // Initialize follow controller
    follow.begin(followCfg);
    follow.setDebug(false);  // Set to true for debug output

    Serial.println("System Ready!");
    Serial.println("Place UWB tag in front and watch the robot follow.\n");
}

// ============================================
// Step 4: Main loop
// ============================================

void loop() {
    // Update UWB sensor
    uwb.update();

    // Check if UWB is connected
    if (uwb.isConnected()) {
        // Get UWB data
        UWBData data = uwb.getData();

        // Update follow controller
        follow.update(data.distance, data.angle);

        // Optional: print status
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 500) {
            Serial.printf("Distance: %.1f cm, Angle: %.1f deg\n",
                          data.distance, data.angle);
            lastPrint = millis();
        }
    } else {
        // UWB not connected, stop
        follow.stop();

        static unsigned long lastWarn = 0;
        if (millis() - lastWarn > 1000) {
            Serial.println("UWB not connected!");
            lastWarn = millis();
        }
    }

    delay(10);  // Small delay for stability
}
