#pragma once
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"
#include "BaseManager.h"
#include <DNSServer.h>

class WebpageManager : public BaseManager
{
public:
    void begin(AsyncWebSocket *ws) override;
    void loop() override;
    void handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg) override;
    void handleESPNowMessage(uint8_t *macAddress, String msg) override;

private:
    void setupCaptivePortalRoutes();
    static void serveIndex(AsyncWebServerRequest *request);

    AsyncWebServer server{80};
    DNSServer dnsServer;
};
