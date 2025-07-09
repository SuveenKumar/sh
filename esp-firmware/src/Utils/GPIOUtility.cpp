#include "GPIOUtility.h"
#include <Utils/Constants.h>
#include <Arduino.h>
#include "Log.h"

void GPIOUtility::SetupGPIOPins() {
    for (int i = 0; i < NUM_PINS; i++) {
       pinMode(LED_PINS[i], OUTPUT);
    }
}

void GPIOUtility::updateAllPinsState(PinsState pinStates) {
    for (size_t i = 0; i < pinStates.pinsState.size(); i++)
    {
        //Active Low
        digitalWrite(LED_PINS[i], pinStates.pinsState[i]);
    }
}

void GPIOUtility::updatePinState(int index, bool state){
    //Active Low
    Log::Info("updatePinState: "+String(LED_PINS[index])+ ": " + state);
    digitalWrite(LED_PINS[index], !state);
}
