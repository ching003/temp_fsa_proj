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

mutex dataMutex;
InputMessage sharedInput;
atomic<bool> systemRunning(true);
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

    cout << "\n";
    cout << "  \033[36;1m========================================\033[0m\n";
    cout << "  \033[1;37m  TEMPERATURE CONTROL SYSTEM SIMULATOR\033[0m\n";
    cout << "  \033[36;1m========================================\033[0m\n\n";

    cout << "  \033[90mDefault Configuration:\033[0m\n";
    cout << "    Fan Threshold   : " << config.fanThreshold << " C\n";
    cout << "    Alarm Threshold : " << config.alarmThreshold << " C\n";
    cout << "    Valid Range     : [" << config.minValid << ", " << config.maxValid << "] C\n";
    cout << "    Fault Threshold : " << config.faultThreshold << " cycles\n\n";

    cout << "  \033[1;37mOptions:\033[0m\n";
    cout << "    1. Run with default configuration\n";
    cout << "    2. Customize thresholds\n";
    cout << "    3. Select mode (AUTO/MANUAL)\n\n";

    cout << "  Enter option (1-3): ";
    string input;
    getline(cin, input);

    if (input == "2")
    {
        cout << "\n  \033[1;37m--- Customize Thresholds ---\033[0m\n";

        // Fan threshold
        cout << "  Fan threshold (current: " << config.fanThreshold << " C): ";
        getline(cin, input);
        if (!input.empty())
        {
            try
            {
                double val = stod(input);
                if (val >= config.minValid && val <= config.maxValid)
                {
                    config.fanThreshold = val;
                }
                else
                {
                    cout << "  \033[33m[Warning] Invalid value, keeping default.\033[0m\n";
                }
            }
            catch (...)
            {
                cout << "  \033[33m[Warning] Invalid input, keeping default.\033[0m\n";
            }
        }

        // Alarm threshold
        cout << "  Alarm threshold (current: " << config.alarmThreshold << " C): ";
        getline(cin, input);
        if (!input.empty())
        {
            try
            {
                double val = stod(input);
                if (val > config.fanThreshold && val <= config.maxValid)
                {
                    config.alarmThreshold = val;
                }
                else
                {
                    cout << "  \033[33m[Warning] Must be > fan threshold ("
                         << config.fanThreshold << "). Keeping default.\033[0m\n";
                }
            }
            catch (...)
            {
                cout << "  \033[33m[Warning] Invalid input, keeping default.\033[0m\n";
            }
        }

        // Fault threshold
        cout << "  Fault threshold cycles (current: " << config.faultThreshold << "): ";
        getline(cin, input);
        if (!input.empty())
        {
            try
            {
                int val = stoi(input);
                if (val >= 1 && val <= 20)
                {
                    config.faultThreshold = val;
                }
                else
                {
                    cout << "  \033[33m[Warning] Must be 1-20. Keeping default.\033[0m\n";
                }
            }
            catch (...)
            {
                cout << "  \033[33m[Warning] Invalid input, keeping default.\033[0m\n";
            }
        }

        cout << "\n  \033[32m[OK] Configuration updated.\033[0m\n";
    }

    if (input == "3" || input == "2")
    {
        cout << "\n  \033[1;37m--- Select Mode ---\033[0m\n";
        cout << "    1. AUTO  (random temperature generation)\n";
        cout << "    2. MANUAL (enter temperature manually)\n";
        cout << "  Enter option (1-2): ";
        string modeInput;
        getline(cin, modeInput);
        if (modeInput == "2")
        {
            currentMode = Mode::MANUAL;
            cout << "  \033[32m[OK] Mode set to MANUAL.\033[0m\n";
        }
        else
        {
            currentMode = Mode::AUTO;
            cout << "  \033[32m[OK] Mode set to AUTO.\033[0m\n";
        }
    }

    cout << "\n  \033[90mFinal Configuration:\033[0m\n";
    cout << "    Fan Threshold   : " << config.fanThreshold << " C\n";
    cout << "    Alarm Threshold : " << config.alarmThreshold << " C\n";
    cout << "    Fault Threshold : " << config.faultThreshold << " cycles\n";
    cout << "    Mode            : " << modeToString(currentMode) << "\n\n";
    cout << "  \033[1;37mStarting system in 2 seconds...\033[0m\n";

    this_thread::sleep_for(chrono::seconds(2));

    return config;
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    // 1 Dang ky signal handler
    signal(SIGINT, signalHandler);

    // 2 Startup menu (single-threaded, chua can mutex)
    ThresholdConfig config = showStartupMenu();

    // 3 Khoi tao cac module
    Logger logger("logs/system.log");
    if (!logger.isOpen())
    {
        cerr << "Failed to open log file! Exiting.\n";
        return 1;
    }

    logger.log(LogLevel::INFO, "Mode: " + modeToString(currentMode));

    TemperatureSensor sensor(config, currentMode);
    Fan fan;
    Alarm alarmDevice;
    TemperatureController controller(fan, alarmDevice, logger, config);
    Display display(dataMutex);
    InputHandler inputHandler(dataMutex, sharedInput, systemRunning,
                              currentMode, display);

    // 4 Init screen
    display.initScreen();

    // 5 Spawn input thread
    thread inputThread(&InputHandler::run, &inputHandler);

    // 6 SUPER-LOOP - Main Thread (1s/cycle)
    while (systemRunning.load())
    {
        double temp = 0.0;
        bool hasNewManualInput = false;

        // CHECK INPUT tu input thread (lock mutex)
        {
            lock_guard<mutex> lock(dataMutex);

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
                hasNewManualInput = true;
                sharedInput.hasNewInput = false;
            }
        }

        // READ SENSOR
        if (currentMode == Mode::AUTO)
        {
            temp = sensor.generateRandom();
        }
        else
        {
            // MANUAL: neu khong co input moi, giu lastValidTemp
            lock_guard<mutex> lock(dataMutex);
            if (!hasNewManualInput)
            {
                temp = controller.getLastValidTemp();
            }
        }

        // PROCESS
        controller.process(temp);

        // DISPLAY (lock mutex)
        display.renderDashboard(controller, sensor, currentMode);

        // WAIT
        this_thread::sleep_for(chrono::seconds(1));
    }
    // 7 SHUTDOWN
    logger.log(LogLevel::INFO, "System shutdown requested by user");

    // Cho input thread ket thuc
    if (inputThread.joinable())
    {
        inputThread.join();
    }

    display.cleanup();

    cout << "\n  \033[32m[OK] System shutdown gracefully.\033[0m\n\n";

    return 0;
}