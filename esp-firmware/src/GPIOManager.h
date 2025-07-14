#pragma once
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"
#include "BaseManager.h"
#include <DNSServer.h>

class GPIOManager : public BaseManager {
public:
    void begin(AsyncWebSocket* ws) override;
    void loop() override;
    void handleWebSocketMessage(AsyncWebSocketClient* client, AwsEventType eventType, String msg) override;
private:
    bool mountFileSystem();
    static void SetupGPIOPins();
    static void updateAllPinsState(const String &states);
    static void updatePinState(int index, bool state);
    AsyncWebSocket *ws = nullptr;
};
