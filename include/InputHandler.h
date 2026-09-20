#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "Common.h"
#include "Display.h"
#include <mutex>
#include <atomic>

class InputHandler {
private:
    std::mutex&         dataMutex;
    InputMessage&       sharedInput;
    std::atomic<bool>&  running;
    Mode&               currentMode;
    Display&            display;

    // Xu ly input khi AUTO mode (non-blocking)
    void handleAutoMode();

    // Xu ly input khi MANUAL mode (blocking cin)
    void handleManualMode();

public:
    InputHandler(std::mutex& mtx,
                 InputMessage& input,
                 std::atomic<bool>& run,
                 Mode& mode,
                 Display& disp);

    // Entry point cho thread - chay trong while(running)
    void run();

    // Bat raw mode cho console (Windows)
    static void enableRawMode();
};

#endif // INPUT_HANDLER_H
