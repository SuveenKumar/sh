#pragma once
#include <ESPAsyncWebServer.h>

class BaseManager
{
public:
    virtual void begin(AsyncWebSocket *ws) = 0;
    virtual void loop() = 0;
    virtual void handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg) = 0;
    virtual void handleESPNowMessage(uint8_t *macAddress, String msg) = 0;
    virtual ~BaseManager() = default;
};
