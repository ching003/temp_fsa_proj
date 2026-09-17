#ifndef DISPLAY_H
#define DISPLAY_H

#include "Common.h"
#include <mutex>
#include <string>

// Forward declarations
class TemperatureController;
class TemperatureSensor;

class Display{
private:
    std::mutex &displayMutex;

    // Layout constants
    static const int DASHBOARD_START_ROW = 1;
    static const int DASHBOARD_END_ROW = 18;
    static const int SEPARATOR_ROW = 19;
    static const int INPUT_START_ROW = 20;
    static const int INPUT_STATUS_ROW = 21;

    // ANSI escape helpers
    void moveCursor(int row, int col) const;
    void clearLine() const;
    void saveCursor() const;
    void restoreCursor() const;
    void hideCursor() const;
    void showCursor() const;

    // Lấy màu sắc theo trạng thái
    std::string getStatusColor(SystemStatus status) const;

public:
    Display(std::mutex &mtx);

    // Khởi tạo màn hình: bật Virtual Terminal (Windows), clear, vẽ separator
    void initScreen();

    // Vẽ dashboard tại vị trí cố định (thread-safe)
    void renderDashboard(const TemperatureController &ctrl,
                         const TemperatureSensor &sensor,
                         Mode currentMode);

    // Vẽ prompt nhập liệu ở input zone
    void renderInputPrompt(const std::string &prompt);

    // Vẽ status của input (đã chấp nhận, lỗi, v.v.)
    void renderInputStatus(const std::string &status);

    // Xóa input zone
    void clearInputZone();

    // Khôi phục console khi thoát
    void cleanup();
};

#endif // DISPLAY_H