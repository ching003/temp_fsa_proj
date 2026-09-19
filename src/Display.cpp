#include "Display.h"
#include "TemperatureController.h"
#include "TemperatureSensor.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// ANSI escape helpers
// ============================================================

void Display::moveCursor(int row, int col) const{
    std::cout << "\033[" << row << ";" << col << "H";
}

void Display::clearLine() const{
    std::cout << "\033[2K";
}

void Display::saveCursor() const{
    std::cout << "\033[s";
}

void Display::restoreCursor() const{
    std::cout << "\033[u";
}

void Display::hideCursor() const{
    std::cout << "\033[?25l";
}

void Display::showCursor() const{
    std::cout << "\033[?25h";
}

std::string Display::getStatusColor(SystemStatus status) const{
    switch (status){
    case SystemStatus::NORMAL:
        return "\033[32m"; // Green
    case SystemStatus::WARNING:
        return "\033[33m"; // Yellow
    case SystemStatus::CRITICAL:
        return "\033[31m"; // Red
    case SystemStatus::SENSOR_ERROR:
        return "\033[35m"; // Magenta
    case SystemStatus::FAILSAFE:
        return "\033[31;1m"; // Bold Red
    default:
        return "\033[0m";
    }
}

// ============================================================
// Constructor
// ============================================================

Display::Display(std::mutex &mtx) : displayMutex(mtx){
}

// ============================================================
// Init screen
// ============================================================

void Display::initScreen(){
#ifdef _WIN32
    // Bật Virtual Terminal Processing trên Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE){
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif
    // Clear toàn bộ màn hình (Chỉ lần đầu)
    std::cout << "\033[2J\033[H";
    std::cout.flush();
}

// ============================================================
// Render Dashboard
// Thread-safe rendering at fixed console positions
// ============================================================

void Display::renderDashboard(const TemperatureController &ctrl,
                              const TemperatureSensor &sensor,
                              Mode currentMode){
    std::lock_guard<std::mutex> lock(displayMutex);

    saveCursor();
    hideCursor();

    SystemStatus status = ctrl.getStatus();
    std::string color = getStatusColor(status);
    std::string reset = "\033[0m";

    int row = DASHBOARD_START_ROW;

    // Title
    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\x94\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x97" << reset;

    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\x91\033[1;37m   TEMPERATURE CONTROL SYSTEM       \033[36;1m\xE2\x95\x91" << reset;

    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\xA0\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\xA3" << reset;

    // Mode
    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\x91" << reset
              << "  Mode        : \033[1;37m" << modeToString(currentMode)
              << reset << std::string(23 - modeToString(currentMode).length(), ' ')
              << "\033[36;1m\xE2\x95\x91" << reset;

    // Temperature
    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\x91" << reset << "  Temperature : ";
    if (status == SystemStatus::SENSOR_ERROR || status == SystemStatus::FAILSAFE){
        std::cout << color << "*** SENSOR FAULT ***" << reset << "   ";
    }
    else{
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << ctrl.getLastValidTemp() << " C";
        std::string tempStr = oss.str();
        std::cout << color << tempStr << reset
                  << std::string(23 - tempStr.length(), ' ');
    }
    std::cout << "\033[36;1m\xE2\x95\x91" << reset;

    // Status
    moveCursor(row++, 1);
    clearLine();
    std::string statusStr = statusToString(status);
    std::cout << "  \033[36;1m\xE2\x95\x91" << reset
              << "  Status      : " << color << statusStr << reset
              << std::string(23 - statusStr.length(), ' ')
              << "\033[36;1m\xE2\x95\x91" << reset;

    // Fan
    moveCursor(row++, 1);
    clearLine();
    std::string fanStr = ctrl.getFanStateString();
    if (status == SystemStatus::FAILSAFE && ctrl.getFanState() == DeviceState::ON){
        fanStr += "  (FORCED)";
    }
    std::cout << "  \033[36;1m\xE2\x95\x91" << reset
              << "  Fan         : ";
    if (ctrl.getFanState() == DeviceState::ON){
        std::cout << "\033[32;1m" << fanStr << "\033[0m";
    }
    else{
        std::cout << fanStr;
    }
    std::cout << std::string(23 - fanStr.length(), ' ')
              << "\033[36;1m\xE2\x95\x91" << reset;

    // Alarm
    moveCursor(row++, 1);
    clearLine();
    std::string alarmStr = ctrl.getAlarmStateString();
    if (status == SystemStatus::FAILSAFE && ctrl.getAlarmState() == DeviceState::ON){
        alarmStr += "  (FORCED)";
    }
    std::cout << "  \033[36;1m\xE2\x95\x91" << reset
              << "  Alarm       : ";
    if (ctrl.getAlarmState() == DeviceState::ON){
        std::cout << "\033[31;1m" << alarmStr << "\033[0m";
    }
    else{
        std::cout << alarmStr;
    }
    std::cout << std::string(23 - alarmStr.length(), ' ')
              << "\033[36;1m\xE2\x95\x91" << reset;

    // Separator
    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\xA0\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\xA3" << reset;

    // Statistics
    const TemperatureStats &stats = ctrl.getStats();
    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\x91" << reset << "  Statistics:                          \033[36;1m\xE2\x95\x91" << reset;

    moveCursor(row++, 1);
    clearLine();
    if (stats.count > 0){
        std::ostringstream ossStats;
        ossStats << "    Min: " << std::fixed << std::setprecision(1) << stats.minTemp
                 << " C  Max: " << stats.maxTemp << " C";
        std::string statsStr = ossStats.str();
        std::cout << "  \033[36;1m\xE2\x95\x91" << reset << statsStr
                  << std::string(39 - statsStr.length(), ' ')
                  << "\033[36;1m\xE2\x95\x91" << reset;
    }
    else{
        std::cout << "  \033[36;1m\xE2\x95\x91" << reset << "    No data yet                        \033[36;1m\xE2\x95\x91" << reset;
    }

    moveCursor(row++, 1);
    clearLine();
    if (stats.count > 0){
        std::ostringstream ossAvg;
        ossAvg << "    Avg: " << std::fixed << std::setprecision(1) << stats.average()
               << " C  Samples: " << stats.count;
        std::string avgStr = ossAvg.str();
        std::cout << "  \033[36;1m\xE2\x95\x91" << reset << avgStr
                  << std::string(39 - avgStr.length(), ' ')
                  << "\033[36;1m\xE2\x95\x91" << reset;
    }
    else{
        std::cout << "  \033[36;1m\xE2\x95\x91" << reset << "                                       \033[36;1m\xE2\x95\x91" << reset;
    }

    // Separator
    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\xA0\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\xA3" << reset;

    // Fault counter
    moveCursor(row++, 1);
    clearLine();
    std::ostringstream ossFault;
    ossFault << "  Fault Counter: " << ctrl.getFaultCounter();
    if (status == SystemStatus::FAILSAFE){
        ossFault << " (FAILSAFE!)";
    }
    std::string faultStr = ossFault.str();
    std::cout << "  \033[36;1m\xE2\x95\x91" << reset << faultStr
              << std::string(39 - faultStr.length(), ' ')
              << "\033[36;1m\xE2\x95\x91" << reset;

    // Bottom border
    moveCursor(row++, 1);
    clearLine();
    std::cout << "  \033[36;1m\xE2\x95\x9A\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x90\xE2\x95\x9D" << reset;

    // Hotkeys
    moveCursor(row++, 1);
    clearLine();
    std::cout << "    \033[90mQ = Quit | M = Toggle Mode\033[0m";

    // Separator line
    moveCursor(SEPARATOR_ROW, 1);
    clearLine();
    std::cout << "  \033[90m" << std::string(42, '-') << "\033[0m";

    restoreCursor();
    showCursor();
    std::cout.flush();
}

// ============================================================
// Input zone rendering
// ============================================================

void Display::renderInputPrompt(const std::string &prompt){
    // Không lock mutex ở đây - caller phải lock
    moveCursor(INPUT_START_ROW, 1);
    clearLine();
    std::cout << "  " << prompt;
    std::cout.flush();
}

void Display::renderInputStatus(const std::string &status){
    // Không lock mutex ở đây - caller phải lock
    moveCursor(INPUT_STATUS_ROW, 1);
    clearLine();
    std::cout << "  \033[90m" << status << "\033[0m";
    std::cout.flush();
}

void Display::clearInputZone(){
    std::lock_guard<std::mutex> lock(displayMutex);
    moveCursor(INPUT_START_ROW, 1);
    clearLine();
    moveCursor(INPUT_STATUS_ROW, 1);
    clearLine();
    std::cout.flush();
}

void Display::cleanup(){
    showCursor();
    std::cout << "\033[0m"; // Reset all attributes
    moveCursor(INPUT_START_ROW + 3, 1);
    std::cout.flush();
}