/**
 * @file main.cpp
 * @brief Smart backpack - full feature firmware
 */

#include <Arduino.h>
#include <BluetoothSerial.h>
#include "config.h"
#include "display.h"
#include "imu.h"
#include "motor.h"
#include "weight.h"
#include "uwb.h"
#include "follow.h"
#include "path.h"
#include "buzzer.h"
#include "led.h"

WorkMode currentMode = MODE_STANDBY;
volatile bool buttonPressed = false;
unsigned long lastButtonTime = 0;
unsigned long lastClickTime = 0;
bool pendingSingleClick = false;
bool buttonDown = false;
unsigned long buttonDownTime = 0;
bool longPressTriggered = false;

BluetoothSerial SerialBT;
bool btReady = false;

void IRAM_ATTR buttonISR();
void handleButton();
void handleSerialCommands();
void handleInput(Stream& stream);
void handleCommand(char cmd);
void handleStandbyWeightWarning();
void runCurrentMode();
void updateDisplay();
void setMode(WorkMode nextMode);
void switchMode();

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("==========================================");
    Serial.println("    Smart Backpack System Start (Full Version)");
    Serial.println("==========================================");
    Serial.println();

    btReady = SerialBT.begin(BT_DEVICE_NAME);
    if (btReady) {
        Serial.println("Bluetooth ready");
    } else {
        Serial.println("Bluetooth init failed");
    }

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    // Initialize buzzer early to avoid power-on false beeps
    buzzer.begin();

    if (!display.begin()) Serial.println("OLED Init Failed!");
    else display.showSplash();

    ledStrip.begin();
    motor.begin();

    if (!imu.begin()) Serial.println("IMU Init Failed!");

    weight.begin();
    uwb.begin();
    follow.begin();
    path.begin();

    delay(1000);
    Serial.println("System Ready! Current Mode: 0 (Standby)");
}

void loop() {
    handleButton();
    handleSerialCommands();

    uwb.update();
    buzzer.update();
    ledStrip.update();

    static unsigned long lastSensor = 0;
    if (millis() - lastSensor > 100) {
        if (currentMode == MODE_CARRYING) imu.update();
        if (currentMode == MODE_STANDBY) weight.readWeight();
        lastSensor = millis();
    }

    runCurrentMode();

    static unsigned long lastDisplay = 0;
    if (millis() - lastDisplay > 200) {
        updateDisplay();
        lastDisplay = millis();
    }
}

void runCurrentMode() {
    switch (currentMode) {
        case MODE_STANDBY:
            handleStandbyWeightWarning();
            break;

        case MODE_FOLLOWING: {
            if (uwb.isConnected()) {
                UWBData data = uwb.getData();
                follow.update(data.distance, data.angle);
            } else {
                motor.stop();
            }
            break;
        }

        case MODE_PULLING:
            motor.stop();
            break;

        case MODE_CARRYING: {
            PostureWarning w = imu.checkPosture();
            if (w != POSTURE_OK) {
                buzzer.startBeeping();
            } else {
                buzzer.stopBeeping();
            }
            break;
        }

        case MODE_RETURNING:
            if (!path.updateReturning()) {
                setMode(MODE_STANDBY);
            }
            break;

        case MODE_TEACHING:
            break;

        default:
            break;
    }
}

void setMode(WorkMode nextMode) {
    motor.stop();
    buzzer.stopBeeping();

    if (nextMode == MODE_FOLLOWING) {
        motor.setSpeed(MOTOR_SPEED_FOLLOW_FORWARD, MOTOR_SPEED_FOLLOW_TURN);
    } else if (nextMode == MODE_TEACHING || nextMode == MODE_RETURNING) {
        motor.setSpeed(MOTOR_SPEED_PATH_FORWARD, MOTOR_SPEED_PATH_TURN);
    } else {
        motor.setSpeed(MOTOR_SPEED_FORWARD, MOTOR_SPEED_TURN);
    }

    if (currentMode == MODE_TEACHING && path.isRecording()) {
        path.stopRecording();
    }
    if (currentMode == MODE_RETURNING && path.isReturning()) {
        path.cancelReturning();
    }

    currentMode = nextMode;

    if (currentMode == MODE_TEACHING) {
        path.startRecording();
    } else if (currentMode == MODE_RETURNING) {
        if (!path.startReturning()) {
            currentMode = MODE_STANDBY;
        }
    }

    Serial.print("Switch Mode: ");
    Serial.println(MODE_NAMES[currentMode]);
    display.showMessage(MODE_NAMES[currentMode], 1000);
}

void switchMode() {
    WorkMode nextMode = MODE_STANDBY;

    if (currentMode == MODE_STANDBY) nextMode = MODE_CARRYING;
    else if (currentMode == MODE_CARRYING) nextMode = MODE_FOLLOWING;
    else if (currentMode == MODE_FOLLOWING) nextMode = MODE_TEACHING;
    else if (currentMode == MODE_TEACHING) nextMode = MODE_PULLING;
    else if (currentMode == MODE_PULLING) nextMode = MODE_STANDBY;
    else nextMode = MODE_STANDBY;

    setMode(nextMode);
}

void updateDisplay() {
    UWBData ud;
    PostureWarning pw;

    switch (currentMode) {
        case MODE_STANDBY:
            break;

        case MODE_CARRYING:
            pw = imu.checkPosture();
            display.showCarryingScreen(currentMode, imu.getPitch(), imu.getRoll(), imu.getWarningText(pw));
            break;

        case MODE_FOLLOWING:
            ud = uwb.getData();
            display.showFollowScreen(currentMode, ud.distance, ud.angle, ud.d0, ud.d1);
            break;

        case MODE_RETURNING:
            display.showReturningScreen(currentMode, path.getStepCount(), path.getRemainingSteps());
            break;

        case MODE_TEACHING:
            display.showTeachingScreen(currentMode, path.getStepCount(), path.isRecording());
            break;

        case MODE_PULLING:
            display.showPullingScreen(currentMode);
            break;

        default:
            display.showMessage(MODE_NAMES[currentMode], 0);
            break;
    }
}

void IRAM_ATTR buttonISR() {
    buttonPressed = true;
}

void handleButton() {
    unsigned long now = millis();

    if (pendingSingleClick && (now - lastClickTime > BUTTON_DOUBLE_CLICK_MS)) {
        pendingSingleClick = false;
        switchMode();
    }

    if (buttonPressed && (now - lastButtonTime > 200)) {
        buttonPressed = false;
        if (digitalRead(BUTTON_PIN) == LOW) {
            lastButtonTime = now;
            buttonDown = true;
            buttonDownTime = now;
            longPressTriggered = false;
        }
    }

    if (buttonDown) {
        if (digitalRead(BUTTON_PIN) == HIGH) {
            buttonDown = false;
            if (!longPressTriggered) {
                if (pendingSingleClick && (now - lastClickTime <= BUTTON_DOUBLE_CLICK_MS)) {
                    pendingSingleClick = false;
                    setMode(MODE_RETURNING);
                } else {
                    pendingSingleClick = true;
                    lastClickTime = now;
                }
            }
        } else if (!longPressTriggered && (now - buttonDownTime >= BUTTON_LONG_PRESS_TIME)) {
            longPressTriggered = true;
            pendingSingleClick = false;
            ledStrip.toggle();
        }
    }
}

void handleInput(Stream& stream) {
    while (stream.available()) {
        char cmd = (char)stream.read();
        handleCommand(cmd);
    }
}

void handleCommand(char cmd) {
    if (cmd == '\r' || cmd == '\n') return;

    bool recordAction = false;
    PathActionType actionRec = ACTION_STOP;

    switch (cmd) {
        case 'w': case 'W':
        case 'f': case 'F':
            motor.forward();
            actionRec = ACTION_FORWARD;
            recordAction = true;
            break;
        case 's':
        case 'S':
        case 'b':
        case 'B':
            motor.backward();
            actionRec = ACTION_BACKWARD;
            recordAction = true;
            break;
        case 'a': case 'A':
        case 'l': case 'L':
            motor.turnLeft();
            actionRec = ACTION_LEFT;
            recordAction = true;
            break;
        case 'd': case 'D':
        case 'r': case 'R':
            motor.turnRight();
            actionRec = ACTION_RIGHT;
            recordAction = true;
            break;
        case 'x': case 'X':
            motor.stop();
            actionRec = ACTION_STOP;
            recordAction = true;
            break;
        case 'm': case 'M':
            switchMode();
            break;
        case 't': case 'T':
            weight.tare();
            break;
        case 'c': case 'C':
            imu.calibrate();
            break;
        case '?': case 'h': case 'H':
            Serial.println("Commands: W/A/S/D/X or F/B/L/R/X for movement");
            Serial.println("M: mode, T: tare, C: IMU calibrate, P: teach, E: return");
            if (btReady) {
                SerialBT.println("Commands: W/A/S/D/X or F/B/L/R/X for movement");
                SerialBT.println("M: mode, T: tare, C: IMU calibrate, P: teach, E: return");
            }
            break;
        case 'p': case 'P':
            setMode(MODE_TEACHING);
            break;
        case 'e': case 'E':
            setMode(MODE_RETURNING);
            break;
        default:
            break;
    }

    if (currentMode == MODE_TEACHING && path.isRecording() && recordAction) {
        path.recordStep(actionRec);
    }
}

void handleSerialCommands() {
    handleInput(Serial);
    if (btReady) {
        handleInput(SerialBT);
    }
}

void handleStandbyWeightWarning() {
    static unsigned long lastWarnTime = 0;
    unsigned long now = millis();
    if (weight.getWeight() > WEIGHT_WARNING_THRESHOLD) {
        if (!buzzer.isBusy() && (now - lastWarnTime > WEIGHT_WARNING_COOLDOWN_MS)) {
            buzzer.beepTimes(3, BUZZER_WARN_DURATION, BUZZER_WARN_INTERVAL);
            lastWarnTime = now;
        }
    }
}
