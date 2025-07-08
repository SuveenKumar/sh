#include "PinsUtility.h"
#include <Utils/Constants.h>
#include <Arduino.h>

void PinsUtility::updateDigitalPinsState(bool* pinStates) {
    Serial.println("Setting Last Pins State");
    for (int i = 0; i < NUM_PINS; i++)
    {
        //Active Low
        digitalWrite(LED_PINS[i], !pinStates[i]);
    }
}