#pragma once
#include <ESPAsyncWebServer.h>
#include "Credential/CredentialStorage.h"
#include "WiFiManager/WiFiManager.h"

class WebPortal {
public:
    void begin(CredentialStorage* store, WiFiManager* wifi);
    void notifyAll(const String& message);
    void loop();

private:
    void setupWebSocket();
    void setupCaptivePortalRoutes();
    static void serveIndex(AsyncWebServerRequest* request);

    int scanClientId = -1;
    bool scanInProgress = false;

    CredentialStorage* credentialStorage = nullptr;
    WiFiManager* wifiManager = nullptr;

    AsyncWebSocket ws{"/ws"};
    AsyncWebServer server{80};
};
