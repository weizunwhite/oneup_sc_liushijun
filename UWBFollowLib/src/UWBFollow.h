/**
 * @file UWBFollow.h
 * @brief UWB Follow Library - Main header
 * @details Complete library for UWB-based follow control
 *
 * @author Based on Smart Backpack Project
 * @version 1.0.0
 * @date 2026-01-07
 */

#ifndef UWB_FOLLOW_H
#define UWB_FOLLOW_H

// Include all library components
#include "MotorInterface.h"
#include "UWBSensor.h"
#include "FollowController.h"

/**
 * @mainpage UWBFollow Library
 *
 * @section intro_sec Introduction
 * This library provides a complete solution for UWB-based follow control.
 * It includes:
 * - Dual UWB sensor positioning
 * - Triangulation-based distance and angle calculation
 * - Bang-bang follow controller with filtering and hysteresis
 * - Motor interface abstraction
 *
 * @section usage_sec Basic Usage
 *
 * 1. Implement the MotorInterface for your motor driver
 * 2. Create UWBSensor and FollowController objects
 * 3. Configure and initialize
 * 4. Call update() in your main loop
 *
 * @code
 * // 1. Implement motor interface
 * class MyMotor : public MotorInterface {
 *   // Implement forward(), backward(), turnLeft(), turnRight(), stop()
 * };
 *
 * // 2. Create objects
 * MyMotor motor;
 * UWBSensor uwb;
 * FollowController follow(&motor);
 *
 * // 3. Initialize
 * void setup() {
 *   UWBConfig uwbCfg;
 *   uwb.begin(uwbCfg);
 *
 *   FollowConfig followCfg;
 *   follow.begin(followCfg);
 * }
 *
 * // 4. Update in loop
 * void loop() {
 *   uwb.update();
 *   if (uwb.isConnected()) {
 *     UWBData data = uwb.getData();
 *     follow.update(data.distance, data.angle);
 *   }
 * }
 * @endcode
 */

#endif // UWB_FOLLOW_H
