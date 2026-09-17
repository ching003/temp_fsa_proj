#include "TemperatureSensor.h"
#include <chrono>

TemperatureSensor::TemperatureSensor(const ThresholdConfig& cfg, Mode mode)
    : config(cfg),
      currentMode(mode),
      rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count())),
      tempDist(cfg.minValid, cfg.maxValid)
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
    // Sinh nhiet do binh thuong trong khoang [minValid, maxValid]
    return tempDist(rng);
}

void TemperatureSensor::setMode(Mode mode) {
    currentMode = mode;
}

Mode TemperatureSensor::getMode() const {
    return currentMode;
}