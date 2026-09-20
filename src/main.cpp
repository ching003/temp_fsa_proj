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

using namespace std;
// ============================================================
// Global: shared state giua main thread va input thread
// ============================================================

std::mutex dataMutex;
InputMessage sharedInput;
std::atomic<bool> systemRunning(true);
Mode currentMode = Mode::AUTO;

// ============================================================
// Signal handler: Ctrl+C -> thoat an toan
// ============================================================

void signalHandler(int signal)
{
    systemRunning.store(false);
}

// ============================================================
// Startup Menu: cau hinh truoc khi chay
// ============================================================
ThresholdConfig showStartupMenu()
{
    ThresholdConfig config;

    std::cout << "\n";
    std::cout << "  \033[36;1m========================================\033[0m\n";
    std::cout << "  \033[1;37m  TEMPERATURE CONTROL SYSTEM SIMULATOR\033[0m\n";
    std::cout << "  \033[36;1m========================================\033[0m\n\n";

    std::cout << "  \033[90mDefault Configuration:\033[0m\n";
    std::cout << "    Fan Threshold   : " << config.fanThreshold << " C\n";
    std::cout << "    Alarm Threshold : " << config.alarmThreshold << " C\n";
    std::cout << "    Valid Range     : [" << config.minValid << ", " << config.maxValid << "] C\n";
    std::cout << "    Fault Threshold : " << config.faultThreshold << " cycles\n\n";

    std::cout << "  \033[1;37mOptions:\033[0m\n";
    std::cout << "    1. Run with default configuration\n";
    std::cout << "    2. Customize thresholds\n";
    std::cout << "    3. Select mode (AUTO/MANUAL)\n\n";

    std::cout << "  Enter option (1-3): ";
    std::string input;
    std::getline(std::cin, input);

    if (input == "2")
    {
        std::cout << "\n  \033[1;37m--- Customize Thresholds ---\033[0m\n";

        // Fan threshold
        std::cout << "  Fan threshold (current: " << config.fanThreshold << " C): ";
        std::getline(std::cin, input);
        if (!input.empty())
        {
            try
            {
                double val = std::stod(input);
                if (val >= config.minValid && val <= config.maxValid)
                {
                    config.fanThreshold = val;
                }
                else
                {
                    std::cout << "  \033[33m[Warning] Invalid value, keeping default.\033[0m\n";
                }
            }
            catch (...)
            {
                std::cout << "  \033[33m[Warning] Invalid input, keeping default.\033[0m\n";
            }
        }

        // Alarm threshold
        std::cout << "  Alarm threshold (current: " << config.alarmThreshold << " C): ";
        std::getline(std::cin, input);
        if (!input.empty())
        {
            try
            {
                double val = std::stod(input);
                if (val > config.fanThreshold && val <= config.maxValid)
                {
                    config.alarmThreshold = val;
                }
                else
                {
                    std::cout << "  \033[33m[Warning] Must be > fan threshold ("
                              << config.fanThreshold << "). Keeping default.\033[0m\n";
                }
            }
            catch (...)
            {
                std::cout << "  \033[33m[Warning] Invalid input, keeping default.\033[0m\n";
            }
        }

        // Fault threshold
        std::cout << "  Fault threshold cycles (current: " << config.faultThreshold << "): ";
        std::getline(std::cin, input);
        if (!input.empty())
        {
            try
            {
                int val = std::stoi(input);
                if (val >= 1 && val <= 20)
                {
                    config.faultThreshold = val;
                }
                else
                {
                    std::cout << "  \033[33m[Warning] Must be 1-20. Keeping default.\033[0m\n";
                }
            }
            catch (...)
            {
                std::cout << "  \033[33m[Warning] Invalid input, keeping default.\033[0m\n";
            }
        }

        std::cout << "\n  \033[32m[OK] Configuration updated.\033[0m\n";
    }

    if (input == "3" || input == "2")
    {
        std::cout << "\n  \033[1;37m--- Select Mode ---\033[0m\n";
        std::cout << "    1. AUTO  (random temperature generation)\n";
        std::cout << "    2. MANUAL (enter temperature manually)\n";
        std::cout << "  Enter option (1-2): ";
        std::string modeInput;
        std::getline(std::cin, modeInput);
        if (modeInput == "2")
        {
            currentMode = Mode::MANUAL;
            std::cout << "  \033[32m[OK] Mode set to MANUAL.\033[0m\n";
        }
        else
        {
            currentMode = Mode::AUTO;
            std::cout << "  \033[32m[OK] Mode set to AUTO.\033[0m\n";
        }
    }

    std::cout << "\n  \033[90mFinal Configuration:\033[0m\n";
    std::cout << "    Fan Threshold   : " << config.fanThreshold << " C\n";
    std::cout << "    Alarm Threshold : " << config.alarmThreshold << " C\n";
    std::cout << "    Fault Threshold : " << config.faultThreshold << " cycles\n";
    std::cout << "    Mode            : " << modeToString(currentMode) << "\n\n";
    std::cout << "  \033[1;37mStarting system in 2 seconds...\033[0m\n";

    std::this_thread::sleep_for(std::chrono::seconds(2));

    return config;
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    // [1] Dang ky signal handler
    signal(SIGINT, signalHandler);

    // [2] Startup menu (single-threaded, chua can mutex)
    ThresholdConfig config = showStartupMenu();

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

        // [a] CHECK INPUT tu input thread (lock mutex)
        {
            std::lock_guard<std::mutex> lock(dataMutex);

            // Toggle mode?
            if (sharedInput.modeToggleRequested)
            {
                if (currentMode == Mode::AUTO)
                {
                    currentMode = Mode::MANUAL;
                    sensor.setMode(Mode::MANUAL);
                }
                else
                {
                    currentMode = Mode::AUTO;
                    sensor.setMode(Mode::AUTO);
                }
                sharedInput.modeToggleRequested = false;
                logger.log(LogLevel::INFO, "Mode changed to: " + modeToString(currentMode));
            }

            // Co du lieu moi tu MANUAL mode?
            if (sharedInput.hasNewInput)
            {
                temp = sharedInput.temperature;
                sharedInput.hasNewInput = false;
            }
        }

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
    // [7] SHUTDOWN
    logger.log(LogLevel::INFO, "System shutdown requested by user");

    // Cho input thread ket thuc
    if (inputThread.joinable())
    {
        inputThread.join();
    }

    display.cleanup();

    std::cout << "\n  \033[32m[OK] System shutdown gracefully.\033[0m\n\n";

    return 0;
}