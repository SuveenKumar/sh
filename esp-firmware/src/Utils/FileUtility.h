#pragma once
#include <ArduinoJson.h>
#include "Models/PinsState.h"

class FileUtility {
public:
    static bool mountFileSystem();
    static PinsState loadPinStates();
    static void savePinStates(PinsState ledStates);
    static bool saveCredentials(const String &ssid, const String &password);
    static bool loadCredentials(String &ssid, String &password);
};
