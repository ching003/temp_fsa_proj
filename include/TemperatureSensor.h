#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "Common.h"
#include <random>

class TemperatureSensor {
private:
    const ThresholdConfig& config;
    Mode currentMode;

    // Random engine
    std::mt19937 rng;
    std::uniform_real_distribution<double> tempDist;
    std::uniform_real_distribution<double> failureDist;

    double failureRate;  // Ty le loi ngau nhien (0.0 - 1.0)

public:
    // Constructor
    TemperatureSensor(const ThresholdConfig& cfg, Mode mode = Mode::AUTO);

    // Doc nhiet do (AUTO: random, MANUAL: khong lam gi - dung sharedInput)
    double readTemperature();

    // Sinh nhiet do ngau nhien (AUTO mode)
    double generateRandom();

    // Getter & Setter
    void setMode(Mode mode);
    Mode getMode() const;
    void setFailureRate(double rate);
    double getFailureRate() const;
};

#endif 