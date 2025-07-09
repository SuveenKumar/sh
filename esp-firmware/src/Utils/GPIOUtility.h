#pragma once
#include <vector>
#include <Models/PinsState.h>

class GPIOUtility {
public:
    static void SetupGPIOPins();
    static void updateAllPinsState(PinsState pinStates);
    static void updatePinState(int index, bool state);
};
