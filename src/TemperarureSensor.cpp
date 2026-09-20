#include "TemperatureSensor.h"
#include <chrono>

TemperatureSensor::TemperatureSensor(const ThresholdConfig& cfg, Mode mode)
    : config(cfg),
      currentMode(mode),
      rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count())),
      tempDist(cfg.minValid, cfg.maxValid),
      failureDist(0.0, 1.0),
      failureRate(0.05)  
{
}

double TemperatureSensor::readTemperature() {
    if (currentMode == Mode::AUTO) {
        return generateRandom();
    }
    // MANUAL mode: tra ve 0, main loop se dung gia tri tu sharedInput
    return 0.0;
}

double TemperatureSensor::generateRandom() {
    // Kiem tra xem co xay ra loi sensor ngau nhien khong
    if (failureDist(rng) < failureRate) {
        // Sinh gia tri loi: ngoai khoang hop le
        // 50% tra ve -999, 50% tra ve 120
        if (failureDist(rng) < 0.5) {
            return -999.0;
        } else {
            return 120.0;
        }
    }

    // Sinh nhiet do binh thuong trong khoang [minValid, maxValid]
    return tempDist(rng);
}

void TemperatureSensor::setMode(Mode mode) {
    currentMode = mode;
}

Mode TemperatureSensor::getMode() const {
    return currentMode;
}

void TemperatureSensor::setFailureRate(double rate) {
    if (rate >= 0.0 && rate <= 1.0) {
        failureRate = rate;
    }
}

double TemperatureSensor::getFailureRate() const {
    return failureRate;
}