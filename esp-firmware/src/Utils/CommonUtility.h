#pragma once
#include <LittleFS.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <Models/WiFiConfig.h>

class CommonUtility {
public:
    static void wipeAllMemory();        
    static bool checkResetButton();
    static String getMessageType(const String msg);
};



