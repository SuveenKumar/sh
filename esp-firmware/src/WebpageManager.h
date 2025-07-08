#pragma once
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"
#include "Utils/FileUtility.h"
#include <ArduinoJson.h>
#include <DNSServer.h>

class WebpageManager {
public:
    void begin(AsyncWebSocket* ws);
    void loop();

private:
    void setupCaptivePortalRoutes();
    static void serveIndex(AsyncWebServerRequest* request);

    AsyncWebSocket* ws = nullptr;
    AsyncWebServer server{80};
    DNSServer dnsServer;
};
