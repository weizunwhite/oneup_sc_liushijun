/**
 * @file main.cpp
 * @brief Smart backpack - full feature firmware
 */

#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "imu.h"
#include "motor.h"
#include "weight.h"
#include "uwb.h"
#include "follow.h"
#include "path.h"
#include "buzzer.h"

WorkMode currentMode = MODE_STANDBY;
volatile bool buttonPressed = false;
unsigned long lastButtonTime = 0;

void IRAM_ATTR buttonISR();
void handleButton();
void handleSerialCommands();
void runCurrentMode();
void updateDisplay();

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("==========================================");
    Serial.println("    Smart Backpack System Start (Full Version)");
    Serial.println("==========================================");
    Serial.println();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    // Initialize buzzer early to avoid power-on false beeps
    buzzer.begin();

    if (!display.begin()) Serial.println("OLED Init Failed!");
    else display.showSplash();

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
            buzzer.stopBeeping();
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
                currentMode = MODE_STANDBY;
            }
            break;

        case MODE_TEACHING:
            break;

        default:
            break;
    }
}

void switchMode() {
    motor.stop();
    buzzer.stopBeeping();
    path.stopRecording();
    path.cancelReturning();

    if (currentMode == MODE_STANDBY) currentMode = MODE_CARRYING;
    else if (currentMode == MODE_CARRYING) currentMode = MODE_FOLLOWING;
    else if (currentMode == MODE_FOLLOWING) currentMode = MODE_PULLING;
    else currentMode = MODE_STANDBY;

    Serial.print("Switch Mode: ");
    Serial.println(MODE_NAMES[currentMode]);
    display.showMessage(MODE_NAMES[currentMode], 1000);
}

void updateDisplay() {
    UWBData ud;
    PostureWarning pw;

    switch (currentMode) {
        case MODE_STANDBY:
            buzzer.stopBeeping();
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
    if (buttonPressed && (millis() - lastButtonTime > 200)) {
        buttonPressed = false;
        if (digitalRead(BUTTON_PIN) == LOW) {
            lastButtonTime = millis();
            switchMode();
        }
    }
}

void handleSerialCommands() {
    if (!Serial.available()) return;

    char cmd = Serial.read();
    bool teachingActive = (currentMode == MODE_TEACHING);
    bool recordAction = false;
    PathActionType actionRec = ACTION_STOP;

    switch (cmd) {
        case 'w': case 'W':
        case 'f': case 'F':
            motor.forward();
            actionRec = ACTION_FORWARD;
            recordAction = true;
            break;
        case 's': case 'S':
            if (cmd == 'S' && teachingActive) {
                motor.stop();
                actionRec = ACTION_STOP;
            } else {
                motor.backward();
                actionRec = ACTION_BACKWARD;
            }
            recordAction = true;
            break;
        case 'b': case 'B':
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
            Serial.println("Commands: W/S/A/D/X or F/B/L/R/S (teaching) for movement");
            Serial.println("M: mode, T: tare, C: IMU calibrate, P: start teach, E: end teach/return");
            break;
        case 'p': case 'P':
            currentMode = MODE_TEACHING;
            motor.stop();
            path.startRecording();
            break;
        case 'e': case 'E':
            path.stopRecording();
            if (path.startReturning()) {
                currentMode = MODE_RETURNING;
            } else {
                currentMode = MODE_STANDBY;
            }
            break;
    }

    if (currentMode == MODE_TEACHING && path.isRecording() && recordAction) {
        path.recordStep(actionRec);
    }
}
