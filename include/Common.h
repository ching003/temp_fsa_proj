#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <limits>

// ============================================================
// Enums
// ============================================================

// Che do doc sensor
enum class Mode
{
    AUTO,
    MANUAL
};

// Trang thai thiet bi (Fan, Alarm)
enum class DeviceState
{
    OFF,
    ON
};

// Trang thai he thong
enum class SystemStatus
{
    NORMAL,   // < fanThreshold
    WARNING,  // fanThreshold -> alarmThreshold
    CRITICAL, // >= alarmThreshold
    // SENSOR_ERROR, // Du lieu khong hop le (transient, 1-3 chu ky)
    // FAILSAFE      // Sensor loi lien tuc > faultThreshold chu ky
};

// Muc do log
enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// ============================================================
// Structs
// ============================================================

// Cau hinh nguong - tranh magic number
struct ThresholdConfig
{
    double fanThreshold = 30.0;   // Nguong bat quat
    double alarmThreshold = 40.0; // Nguong bat coi
    double minValid = 0.0;        // Nhiet do hop le min
    double maxValid = 60.0;       // Nhiet do hop le max
    // int faultThreshold = 3;       // So chu ky loi truoc khi FAILSAFE
};

// Thong ke nhiet do (running statistics - O(1) memory)
struct TemperatureStats {
    double minTemp = std::numeric_limits<double>::max();
    double maxTemp = std::numeric_limits<double>::lowest();
    double sum     = 0.0;
    int    count   = 0;

    void update(double temp) {
        if (temp < minTemp) minTemp = temp;
        if (temp > maxTemp) maxTemp = temp;
        sum += temp;
        count++;
    }

    double average() const {
        return count > 0 ? sum / count : 0.0;
    }

    void reset() {
        minTemp = std::numeric_limits<double>::max();
        maxTemp = std::numeric_limits<double>::lowest();
        sum     = 0.0;
        count   = 0;
    }
};

// ============================================================
// Helper functions: enum -> string (cho log/display)
// ============================================================

inline std::string statusToString(SystemStatus status)
{
    switch (status)
    {
    case SystemStatus::NORMAL:
        return "NORMAL";
    case SystemStatus::WARNING:
        return "WARNING";
    case SystemStatus::CRITICAL:
        return "CRITICAL";
    // case SystemStatus::SENSOR_ERROR:
    //     return "SENSOR_ERROR";
    // case SystemStatus::FAILSAFE:
    //     return "FAILSAFE";
    default:
        return "UNKNOWN";
    }
}

inline std::string deviceStateToString(DeviceState state)
{
    switch (state)
    {
    case DeviceState::OFF:
        return "OFF";
    case DeviceState::ON:
        return "ON";
    default:
        return "UNKNOWN";
    }
}

inline std::string modeToString(Mode mode)
{
    switch (mode)
    {
    case Mode::AUTO:
        return "AUTO";
    case Mode::MANUAL:
        return "MANUAL";
    default:
        return "UNKNOWN";
    }
}

inline std::string logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

#endif // COMMON_H