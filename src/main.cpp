#include "Common.h"
#include "TemperatureSensor.h"
#include "Fan.h"
#include "Alarm.h"
#include "TemperatureController.h"
#include "Display.h"
#include "InputHandler.h"
#include "Logger.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <csignal>
#include <string>
#include <sstream>
#include <limits>

int main()
{

    // [3] Khoi tao cac module
    Logger logger("logs/system.log");
    if (!logger.isOpen())
    {
        std::cerr << "Failed to open log file! Exiting.\n";
        return 1;
    }

    logger.log(LogLevel::INFO, "Mode: " + modeToString(currentMode));

    TemperatureSensor sensor(config, currentMode);
    Fan fan;
    Alarm alarmDevice;
    TemperatureController controller(fan, alarmDevice, logger, config);
    Display display(dataMutex);

    // [4] Init screen
    display.initScreen();

    // [5] Spawn input thread
    std::thread inputThread(&InputHandler::run, &inputHandler);

    // [6] SUPER-LOOP - Main Thread (1s/cycle)
    while (systemRunning.load())
    {
        double temp = 0.0;

        // [b] READ SENSOR
        if (currentMode == Mode::AUTO)
        {
            temp = sensor.generateRandom();
        }
        else
        {
            // MANUAL: neu khong co input moi, giu lastValidTemp
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!sharedInput.hasNewInput && temp == 0.0)
            {
                temp = controller.getLastValidTemp();
            }
        }

        // [c] PROCESS
        controller.process(temp);

        // [d] DISPLAY (lock mutex)
        display.renderDashboard(controller, sensor, currentMode);

        // [e] WAIT
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    display.cleanup();

    std::cout << "\n  \033[32m[OK] System shutdown gracefully.\033[0m\n\n";

    return 0;
}