#ifndef ALARM_H
#define ALARM_H

#include "Common.h"
#include <string>

class Alarm{
private:
    DeviceState currentState;

public:
    Alarm();

    // Thay đổi trạng thái - trả về true nếu state THỰC SỰ thay đổi (edge-triggered)
    bool setState(DeviceState newState);

    // Getters
    DeviceState getState() const;
    std::string getStateString() const;
};

#endif // ALARM_H