#include "Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

Logger::Logger(const std::string& filepath) {
    logFile.open(filepath, std::ios::app);
    if (logFile.is_open()) {
        logFile << "\n========================================\n";
        log(LogLevel::INFO, "=== System Started ===");
    } else {
        std::cerr << "[Logger] ERROR: Cannot open log file: " << filepath << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        log(LogLevel::INFO, "=== System Shutdown ===");
        logFile << "========================================\n";
        logFile.close();
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!logFile.is_open()) return;

    logFile << "[" << getTimestamp() << "] "
            << "[" << logLevelToString(level) << "] "
            << message << std::endl;

    // flush ngay lap tuc de dam bao du lieu ghi vao file
    logFile.flush();
}

bool Logger::isOpen() const {
    return logFile.is_open();
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
