#ifndef LOGGER_H
#define LOGGER_H

#include "Common.h"
#include <fstream>
#include <string>

class Logger {
private:
    std::ofstream logFile;

    // Lay timestamp hien tai: YYYY-MM-DD HH:MM:SS
    std::string getTimestamp() const;

public:
    // Mo file log o che do append
    Logger(const std::string& filepath);

    // Destructor: ghi shutdown message va dong file
    ~Logger();

    // Ghi log voi level va message
    void log(LogLevel level, const std::string& message);

    // Kiem tra file co mo thanh cong khong
    bool isOpen() const;
};

#endif // LOGGER_H
