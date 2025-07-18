#pragma once
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"
#include "BaseManager.h"
#include <DNSServer.h>
#include "Utils/MessageParser.h"

class GPIOManager : public BaseManager
{
public:
    void begin(AsyncWebSocket *ws) override;
    void loop() override;
    void handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg) override;
    void handleESPNowMessage(uint8_t *macAddress, String msg) override;
private:
    bool mountFileSystem();
    static void SetupGPIOPins();
    static void updateAllPinsState(const String &states);
    static void updatePinState(int index, bool state);
    void TogglePin(MessageData MessageData);
    String GetPinStateMessage(String pinStates);
    AsyncWebSocket *ws = nullptr;
    void changeSoftAPName(String newSSID, String newPassword = "");
};
