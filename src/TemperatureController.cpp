#include "TemperatureController.h"
#include <sstream>
#include <iomanip>

// ============================================================
// Constructor
// ============================================================

TemperatureController::TemperatureController(Fan &f, Alarm &a, Logger &log,
                                             const ThresholdConfig &cfg)
    : fan(f),
      alarm(a),
      logger(log),
      config(cfg),
      currentStatus(SystemStatus::NORMAL),
      lastValidTemp(25.0)
{
    // Log cau hinh khoi tao
    std::ostringstream oss;
    oss << "Controller initialized - Fan threshold: " << config.fanThreshold
        << "C, Alarm threshold: " << config.alarmThreshold
        << "C, Valid range: [" << config.minValid << ", " << config.maxValid
        << "]C, Fault threshold: " << config.faultThreshold << " cycles";
    logger.log(LogLevel::INFO, oss.str());
}

// ============================================================
// Main processing entry point (moi chu ky)
// ============================================================

void TemperatureController::process(double rawTemp)
{
    if (isValidTemperature(rawTemp))
    {
        // === VALID TEMPERATURE ===

        // Cap nhat nhiet do hop le va thong ke
        lastValidTemp = rawTemp;
        stats.update(rawTemp);

        // Phan loai va dieu khien
        classifyAndControl(rawTemp);

        // Log nhiet do moi chu ky
        std::ostringstream oss;
        oss << "Temperature: " << std::fixed << std::setprecision(1)
            << rawTemp << "C | Status: " << statusToString(currentStatus)
            << " | Fan: " << fan.getStateString()
            << " | Alarm: " << alarm.getStateString();
        logger.log(LogLevel::INFO, oss.str());
    }
}

// ============================================================
// Phan loai nhiet do va dieu khien actuators
// ============================================================

void TemperatureController::classifyAndControl(double temp)
{
    SystemStatus prevStatus = currentStatus;

    if (temp < config.fanThreshold)
    {
        // NORMAL: < fanThreshold
        currentStatus = SystemStatus::NORMAL;

        bool fanChanged = fan.setState(DeviceState::OFF);
        bool alarmChanged = alarm.setState(DeviceState::OFF);

        // Log state transition (edge-triggered)
        if (fanChanged)
        {
            logger.log(LogLevel::INFO, "Fan turned OFF");
        }
        if (alarmChanged)
        {
            logger.log(LogLevel::INFO, "Alarm turned OFF");
        }
    }
    else if (temp < config.alarmThreshold)
    {
        // WARNING: fanThreshold <= temp < alarmThreshold
        currentStatus = SystemStatus::WARNING;

        bool fanChanged = fan.setState(DeviceState::ON);
        bool alarmChanged = alarm.setState(DeviceState::OFF);

        if (fanChanged)
        {
            logger.log(LogLevel::WARNING, "Fan turned ON - Temperature WARNING");
        }
        if (alarmChanged)
        {
            logger.log(LogLevel::INFO, "Alarm turned OFF");
        }
    }
    else
    {
        // CRITICAL: temp >= alarmThreshold
        currentStatus = SystemStatus::CRITICAL;

        bool fanChanged = fan.setState(DeviceState::ON);
        bool alarmChanged = alarm.setState(DeviceState::ON);

        if (fanChanged)
        {
            logger.log(LogLevel::CRITICAL, "Fan turned ON - Temperature CRITICAL");
        }
        if (alarmChanged)
        {
            logger.log(LogLevel::CRITICAL, "Alarm turned ON - Temperature CRITICAL!");
        }
    }

    // Log status transition
    if (prevStatus != currentStatus)
    {
        std::ostringstream oss;
        oss << "Status changed: " << statusToString(prevStatus)
            << " -> " << statusToString(currentStatus);
        logger.log(LogLevel::INFO, oss.str());
    }
}

// ============================================================
// Validate temperature
// ============================================================

bool TemperatureController::isValidTemperature(double temp) const
{
    return temp >= config.minValid && temp <= config.maxValid;
}

// ============================================================
// Getters
// ============================================================

SystemStatus TemperatureController::getStatus() const
{
    return currentStatus;
}

double TemperatureController::getLastValidTemp() const
{
    return lastValidTemp;
}

DeviceState TemperatureController::getFanState() const
{
    return fan.getState();
}

DeviceState TemperatureController::getAlarmState() const
{
    return alarm.getState();
}

std::string TemperatureController::getFanStateString() const
{
    return fan.getStateString();
}

std::string TemperatureController::getAlarmStateString() const
{
    return alarm.getStateString();
}
