#pragma once
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

class WiFiManager {
public:
    void begin(AsyncWebSocket* ws);
    void loop();

    String pendingSSID;
    String pendingPASS;
    bool pendingConnect = false;

private:
    void checkPendingConnection();
    void scanNetworks();
    void subscribeToMessages();
    unsigned long lastAttempt = 0;
    AsyncWebSocket* ws = nullptr;
    int scanClientId = -1;
    bool scanInProgress = false;
    void FallbackToDefaultSSID();
    String getStatusHtml();
};