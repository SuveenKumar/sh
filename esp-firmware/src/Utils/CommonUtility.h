#pragma once
#include <LittleFS.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

class CommonUtility
{
public:
    static void wipeAllMemory();
    static bool checkResetButton();
    static bool saveStringToFile(const String &path, const String &content);
    static String loadStringFromFile(const String &path);
    static void LogInfo(const String &info);
};
