#ifndef FAN_H
#define FAN_H

#include "Common.h"
#include <string>

class Fan{
private:
    DeviceState currentState;

public:
    Fan();

    // Thay đổi trạng thái - trả về true nếu state THỰC SỰ thay đổi  (edge-triggered)
    bool setState(DeviceState newState);

    // Getters
    DeviceState getState() const;
    std::string getStateString() const;
};

#endif // FAN_H