#include "Fan.h"

Fan::Fan() : currentState(DeviceState::OFF){
}

bool Fan::setState(DeviceState newState){
    if (currentState != newState){
        currentState = newState;
        return true; // State thay đổi -> cần log
    }
    return false; // Không thay đổi -> không log
}

DeviceState Fan::getState() const{
    return currentState;
}

std::string Fan::getStateString() const{
    return deviceStateToString(currentState);
}