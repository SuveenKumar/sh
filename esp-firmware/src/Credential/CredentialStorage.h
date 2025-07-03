#pragma once
#include <Arduino.h>

class CredentialStorage {
public:
    bool begin();
    bool save(const String& ssid, const String& password);
    bool load(String& ssid, String& password);
};
