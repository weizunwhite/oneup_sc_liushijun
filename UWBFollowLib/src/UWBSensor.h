/**
 * @file UWBSensor.h
 * @brief Dual UWB positioning sensor class
 * @details Triangulation positioning using two UWB modules
 */

#ifndef UWB_SENSOR_H
#define UWB_SENSOR_H

#include <Arduino.h>

/**
 * @brief UWB configuration structure
 */
struct UWBConfig {
    // Serial ports
    HardwareSerial* serial0;        ///< Serial for UWB module 0
    HardwareSerial* serial1;        ///< Serial for UWB module 1
    uint32_t baudRate;              ///< Serial baud rate

    // Pin configuration (for ESP32 remappable pins)
    int8_t rx0Pin;                  ///< RX pin for UWB0 (-1 for default)
    int8_t tx0Pin;                  ///< TX pin for UWB0 (-1 for default)
    int8_t rx1Pin;                  ///< RX pin for UWB1 (-1 for default)
    int8_t tx1Pin;                  ///< TX pin for UWB1 (-1 for default)

    // Positioning parameters
    float baseline;                 ///< Distance between two UWB modules (cm)
    float distanceScale;            ///< Distance scaling factor
    float hexScale;                 ///< Hex format distance scaling factor
    bool angleInvert;               ///< Invert angle direction
    float angleOffset;              ///< Angle offset in degrees

    // Timeout
    uint32_t timeoutMs;             ///< Connection timeout (ms)

    /**
     * @brief Default constructor with common values
     */
    UWBConfig() :
        serial0(&Serial2),
        serial1(&Serial1),
        baudRate(115200),
        rx0Pin(16), tx0Pin(17),
        rx1Pin(27), tx1Pin(13),
        baseline(28.0f),
        distanceScale(1.0f),
        hexScale(0.1f),
        angleInvert(true),
        angleOffset(0.0f),
        timeoutMs(2000) {}
};

/**
 * @brief UWB positioning data
 */
struct UWBData {
    float d0;                       ///< Distance from module 0 (cm)
    float d1;                       ///< Distance from module 1 (cm)
    float distance;                 ///< Calculated perpendicular distance (cm)
    float angle;                    ///< Calculated angle (degrees, 0=forward, +right/-left)
    bool valid;                     ///< Data validity flag
    unsigned long lastUpdate;       ///< Last update timestamp (ms)

    UWBData() : d0(0), d1(0), distance(0), angle(0), valid(false), lastUpdate(0) {}
};

/**
 * @brief UWB positioning sensor class
 */
class UWBSensor {
public:
    /**
     * @brief Constructor
     */
    UWBSensor();

    /**
     * @brief Initialize UWB sensor
     * @param config Configuration structure
     * @return true if successful
     */
    bool begin(const UWBConfig& config);

    /**
     * @brief Update sensor data (call in loop)
     */
    void update();

    /**
     * @brief Get current positioning data
     * @return UWB data structure
     */
    UWBData getData() const { return _data; }

    /**
     * @brief Check if UWB is connected
     * @return true if data is recent
     */
    bool isConnected() const;

    /**
     * @brief Enable/disable debug output
     * @param enable Debug flag
     */
    void setDebug(bool enable) { _debug = enable; }

private:
    UWBConfig _config;
    UWBData _data;
    String _buffer0;
    String _buffer1;
    bool _debug;

    float parseDistance(String& line);
    void calculatePosition();
};

#endif // UWB_SENSOR_H
