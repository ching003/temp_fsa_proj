#include "Alarm.h"

Alarm::Alarm() : currentState(DeviceState::OFF){
}

bool Alarm::setState(DeviceState newState){
    if (currentState != newState)
    {
        currentState = newState;
        return true; // State thay đổi -> cần log
    }
    return false; // Không thay đổi -> không log
}

DeviceState Alarm::getState() const{
    return currentState;
}

std::string Alarm::getStateString() const{
    return deviceStateToString(currentState);
}