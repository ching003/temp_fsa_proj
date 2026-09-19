cmake_minimum_required(VERSION 3.10)
project(TemperatureSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(temperature_simulator
    src/main.cpp
    src/TemperatureSensor.cpp
    src/Fan.cpp
    src/Alarm.cpp
    src/TemperatureController.cpp
    src/Display.cpp
    src/InputHandler.cpp
    src/Logger.cpp
)

# Link pthread on Linux
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(temperature_simulator PRIVATE Threads::Threads)
endif()