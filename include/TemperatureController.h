#ifndef TEMPERATURE_CONTROLLER_H
#define TEMPERATURE_CONTROLLER_H

#include "Common.h"
#include "Fan.h"
#include "Alarm.h"
#include "Logger.h"

class TemperatureController
{
private:
    Fan &fan;
    Alarm &alarm;
    Logger &logger;
    ThresholdConfig config;

    SystemStatus currentStatus;

    // Phan loai nhiet do va dieu khien actuators
    void classifyAndControl(double temp);

    // Kiem tra nhiet do hop le
    bool isValidTemperature(double temp) const;

public:
    TemperatureController(Fan &f, Alarm &a, Logger &log,
                          const ThresholdConfig &cfg);

    // Entry point xu ly moi chu ky
    void process(double rawTemp);

    // Getters
    SystemStatus getStatus() const;
    double getLastValidTemp() const;

    // Lay trang thai actuators (cho Display)
    DeviceState getFanState() const;
    DeviceState getAlarmState() const;
    std::string getFanStateString() const;
    std::string getAlarmStateString() const;
};

#endif
