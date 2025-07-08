#pragma once
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"
#include "Utils/FileUtility.h"
#include <DNSServer.h>

class GPIOManager {
public:
    void begin(AsyncWebSocket* ws);

private:
    void subscribeToMessages();
    AsyncWebSocket* ws = nullptr;
};
