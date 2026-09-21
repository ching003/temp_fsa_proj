#include "Display.h"
#include "TemperatureController.h"
#include "TemperatureSensor.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif
const int BOX_WIDTH = 44;

// Lặp ký tự Unicode theo số lượng ký tự hiển thị.
// Không dùng std::string(count, '═') vì '═' là UTF-8 nhiều byte.
std::string repeatChar(const std::string &ch, int count)
{
    std::string result;
    for (int i = 0; i < count; ++i)
        result += ch;
    return result;
}
// ============================================================
// ANSI escape helpers
// ============================================================

void Display::moveCursor(int row, int col) const
{
    std::cout << "\033[" << row << ";" << col << "H";
}

void Display::clearLine() const
{
    std::cout << "\033[2K";
}

void Display::saveCursor() const
{
    std::cout << "\033[s";
}

void Display::restoreCursor() const
{
    std::cout << "\033[u";
}

void Display::hideCursor() const
{
    std::cout << "\033[?25l";
}

void Display::showCursor() const
{
    std::cout << "\033[?25h";
}

std::string Display::getStatusColor(SystemStatus status) const
{
    switch (status)
    {
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

Display::Display(std::mutex &mtx) : displayMutex(mtx)
{
}

// ============================================================
// Init screen
// ============================================================

void Display::initScreen()
{
#ifdef _WIN32
    // Bật Virtual Terminal Processing trên Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
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
                              Mode currentMode)
{
    std::lock_guard<std::mutex> lock(displayMutex);

    SystemStatus status = ctrl.getStatus();
    std::string color = getStatusColor(status);
    const std::string reset = "\033[0m";

    const TemperatureStats &stats = ctrl.getStats();

    // Xóa toàn bộ dashboard cũ
    moveCursor(DASHBOARD_START_ROW, 1);

    for (int i = DASHBOARD_START_ROW; i <= DASHBOARD_END_ROW; ++i)
    {
        moveCursor(i, 1);
        clearLine();
    }

    // Hàm tạo một dòng trong khung
    auto printRow = [&](const std::string &content)
    {
        // BOX_WIDTH là số ký tự nằm giữa 2 cạnh của khung.
        int visibleWidth = static_cast<int>(content.length());
        int padding = BOX_WIDTH - visibleWidth;

        if (padding < 0)
            padding = 0;

        std::cout << "  \033[36;1m║"
                  << reset
                  << content
                  << std::string(padding, ' ')
                  << "\033[36;1m║"
                  << reset
                  << '\n';
    };

    // =========================
    // Top
    // =========================

    std::cout << "  \033[36;1m╔"
              << repeatChar("═", BOX_WIDTH)
              << "╗"
              << reset
              << '\n';

    // =========================
    // Title
    // =========================

    std::string title = "TEMPERATURE CONTROL SYSTEM";

    std::string titleContent = "   " + title;

    printRow(titleContent);

    // =========================
    // Separator
    // =========================

    std::cout << "  \033[36;1m╠"
              << repeatChar("═", BOX_WIDTH)
              << "╣"
              << reset
              << '\n';

    // =========================
    // Mode
    // =========================

    std::string modeStr = modeToString(currentMode);

    printRow("  Mode        : " + modeStr);

    // =========================
    // Temperature
    // =========================

    std::ostringstream ossTemp;

    if (status == SystemStatus::SENSOR_ERROR ||
        status == SystemStatus::FAILSAFE)
    {

        ossTemp << "  Temperature : "
                << color
                << "*** SENSOR FAULT ***"
                << reset;

        // Không dùng printRow vì có ANSI color code
        // nên xử lý riêng
        std::string visibleText =
            "  Temperature : *** SENSOR FAULT ***";

        int padding =
            BOX_WIDTH - static_cast<int>(visibleText.length());

        if (padding < 0)
            padding = 0;

        std::cout << "  \033[36;1m║"
                  << reset
                  << "  Temperature : "
                  << color
                  << "*** SENSOR FAULT ***"
                  << reset
                  << std::string(padding, ' ')
                  << "\033[36;1m║"
                  << reset
                  << '\n';
    }
    else
    {

        ossTemp << std::fixed
                << std::setprecision(1)
                << ctrl.getLastValidTemp()
                << " C";

        std::string tempStr = ossTemp.str();

        std::string visibleText =
            "  Temperature : " + tempStr;

        int padding =
            BOX_WIDTH - static_cast<int>(visibleText.length());

        if (padding < 0)
            padding = 0;

        std::cout << "  \033[36;1m║"
                  << reset
                  << "  Temperature : "
                  << color
                  << tempStr
                  << reset
                  << std::string(padding, ' ')
                  << "\033[36;1m║"
                  << reset
                  << '\n';
    }

    // =========================
    // Status
    // =========================

    printRow("  Status      : " + statusToString(status));

    // =========================
    // Fan
    // =========================

    std::string fanStr = ctrl.getFanStateString();

    if (status == SystemStatus::FAILSAFE &&
        ctrl.getFanState() == DeviceState::ON)
    {
        fanStr += "  (FORCED)";
    }

    {
        std::string visibleText = "  Fan         : " + fanStr;

        int padding =
            BOX_WIDTH - static_cast<int>(visibleText.length());

        if (padding < 0)
            padding = 0;

        std::cout << "  \033[36;1m║"
                  << reset
                  << "  Fan         : ";

        if (ctrl.getFanState() == DeviceState::ON)
        {
            std::cout << "\033[32;1m"
                      << fanStr
                      << reset;
        }
        else
        {
            std::cout << fanStr;
        }

        std::cout << std::string(padding, ' ')
                  << "\033[36;1m║"
                  << reset
                  << '\n';
    }

    // =========================
    // Alarm
    // =========================

    std::string alarmStr = ctrl.getAlarmStateString();

    if (status == SystemStatus::FAILSAFE &&
        ctrl.getAlarmState() == DeviceState::ON)
    {
        alarmStr += "  (FORCED)";
    }

    {
        std::string visibleText = "  Alarm       : " + alarmStr;

        int padding =
            BOX_WIDTH - static_cast<int>(visibleText.length());

        if (padding < 0)
            padding = 0;

        std::cout << "  \033[36;1m║"
                  << reset
                  << "  Alarm       : ";

        if (ctrl.getAlarmState() == DeviceState::ON)
        {
            std::cout << "\033[31;1m"
                      << alarmStr
                      << reset;
        }
        else
        {
            std::cout << alarmStr;
        }

        std::cout << std::string(padding, ' ')
                  << "\033[36;1m║"
                  << reset
                  << '\n';
    }

    // =========================
    // Separator
    // =========================

    std::cout << "  \033[36;1m╠"
              << repeatChar("═", BOX_WIDTH)
              << "╣"
              << reset
              << '\n';

    // =========================
    // Statistics
    // =========================

    printRow("  Statistics:");

    if (stats.count > 0)
    {

        std::ostringstream ossStats;

        ossStats << "    Min: "
                 << std::fixed
                 << std::setprecision(1)
                 << stats.minTemp
                 << " C  Max: "
                 << stats.maxTemp
                 << " C";

        printRow(ossStats.str());

        std::ostringstream ossAvg;

        ossAvg << "    Avg: "
               << std::fixed
               << std::setprecision(1)
               << stats.average()
               << " C  Samples: "
               << stats.count;

        printRow(ossAvg.str());
    }
    else
    {

        printRow("    No data yet");
        printRow("");
    }

    // =========================
    // Separator
    // =========================

    std::cout << "  \033[36;1m╠"
              << repeatChar("═", BOX_WIDTH)
              << "╣"
              << reset
              << '\n';

    // =========================
    // Fault Counter
    // =========================

    std::ostringstream ossFault;

    ossFault << "  Fault Counter: "
             << ctrl.getFaultCounter();

    if (status == SystemStatus::FAILSAFE)
    {
        ossFault << " (FAILSAFE!)";
    }

    printRow(ossFault.str());

    // =========================
    // Bottom
    // =========================

    std::cout << "  \033[36;1m╚"
              << repeatChar("═", BOX_WIDTH)
              << "╝"
              << reset
              << '\n';

    // =========================
    // Hotkeys
    // =========================

    std::cout << "    \033[90mQ = Quit | M = Toggle Mode\033[0m";

    std::cout.flush();
}

// ============================================================
// Input zone rendering
// ============================================================

void Display::renderInputPrompt(const std::string &prompt)
{
    // Không lock mutex ở đây - caller phải lock
    moveCursor(INPUT_START_ROW, 1);
    clearLine();
    std::cout << "  " << prompt;
    std::cout.flush();
}

void Display::renderInputStatus(const std::string &status)
{
    // Không lock mutex ở đây - caller phải lock
    moveCursor(INPUT_STATUS_ROW, 1);
    clearLine();
    std::cout << "  \033[90m" << status << "\033[0m";
    std::cout.flush();
}

void Display::clearInputZone()
{
    std::lock_guard<std::mutex> lock(displayMutex);
    moveCursor(INPUT_START_ROW, 1);
    clearLine();
    moveCursor(INPUT_STATUS_ROW, 1);
    clearLine();
    std::cout.flush();
}

void Display::cleanup()
{
    showCursor();
    std::cout << "\033[0m"; // Reset all attributes
    moveCursor(INPUT_START_ROW + 3, 1);
    std::cout.flush();
}