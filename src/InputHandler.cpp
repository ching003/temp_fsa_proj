#include "InputHandler.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#endif

// ============================================================
// Constructor
// ============================================================

InputHandler::InputHandler(std::mutex& mtx,
                           InputMessage& input,
                           std::atomic<bool>& run,
                           Mode& mode,
                           Display& disp)
    : dataMutex(mtx),
      sharedInput(input),
      running(run),
      currentMode(mode),
      display(disp)
{
}

// ============================================================
// Main thread entry point
// ============================================================

void InputHandler::run() {
    while (running.load()) {
        if (currentMode == Mode::AUTO) {
            handleAutoMode();
        } else {
            handleManualMode();
        }
    }
}

// ============================================================
// AUTO mode: non-blocking key detection
// ============================================================

void InputHandler::handleAutoMode() {
#ifdef _WIN32
    // Windows: dung _kbhit() de kiem tra phim khong blocking
    if (_kbhit()) {
        int ch = _getch();
        if (ch == 'q' || ch == 'Q') {
            running.store(false);
            return;
        }
        if (ch == 'm' || ch == 'M') {
            std::lock_guard<std::mutex> lock(dataMutex);
            sharedInput.modeToggleRequested = true;
        }
    }
#else
    // Linux/macOS: dung select() voi timeout
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 50000; // 50ms timeout

    if (select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0) {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) > 0) {
            if (ch == 'q' || ch == 'Q') {
                running.store(false);
                return;
            }
            if (ch == 'm' || ch == 'M') {
                std::lock_guard<std::mutex> lock(dataMutex);
                sharedInput.modeToggleRequested = true;
            }
        }
    }
#endif

    // Sleep 50ms de giam CPU usage
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

// ============================================================
// MANUAL mode: blocking input
// ============================================================

void InputHandler::handleManualMode() {
    // Kiem tra xem co con chay khong truoc khi block
    if (!running.load()) return;

    // Hien thi prompt o input zone
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        display.renderInputPrompt("[MANUAL] Enter temperature (C): ");
    }

    // Doc gia tri tu nguoi dung (BLOCKING - nhung o thread rieng nen OK)
    std::string line;
    std::getline(std::cin, line);

    // Kiem tra lai trang thai sau khi unblock
    if (!running.load()) return;

    // Kiem tra xem co phai lenh dac biet khong
    if (line == "q" || line == "Q") {
        running.store(false);
        return;
    }

    if (line == "m" || line == "M") {
        std::lock_guard<std::mutex> lock(dataMutex);
        sharedInput.modeToggleRequested = true;
        return;
    }

    // Parse so thuc
    try {
        double value = std::stod(line);

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            sharedInput.temperature = value;
            sharedInput.hasNewInput = true;
        }

        // Hien thi status
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            std::ostringstream oss;
            oss << "[Status] Input accepted: " << std::fixed
                << std::setprecision(1) << value << " C";
            display.renderInputStatus(oss.str());
        }

    } catch (const std::exception&) {
        // Input khong hop le
        std::lock_guard<std::mutex> lock(dataMutex);
        display.renderInputStatus("[Error] Invalid input! Enter a number.");
    }
}

// ============================================================
// Enable raw mode (Windows)
// ============================================================

void InputHandler::enableRawMode() {
#ifdef _WIN32
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        GetConsoleMode(hIn, &mode);
        // Bat Virtual Terminal Input
        mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        SetConsoleMode(hIn, mode);
    }
#else
    // Linux: chuyen sang non-canonical mode
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}
