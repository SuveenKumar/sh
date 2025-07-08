#pragma once
#include <ArduinoJson.h>

class FileUtility {
public:
    static bool mountFileSystem();
    static bool* loadPinStates();
    static void savePinStates(bool pinStates[4]);
    static bool saveCredentials(const String &ssid, const String &password);
    static bool loadCredentials(String &ssid, String &password);
};
