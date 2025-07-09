#pragma once
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"
#include "BaseManager.h"
#include "Utils/FileUtility.h"
#include <DNSServer.h>

class GPIOManager : public BaseManager {
public:
    void begin(AsyncWebSocket* ws) override;
    void loop() override;
    void handleWebSocketMessage(AsyncWebSocketClient* client, AwsEventType eventType, String msg) override;
private:
    AsyncWebSocket* ws = nullptr;
};
