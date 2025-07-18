#pragma once
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include "BaseManager.h"

class WiFiManager : public BaseManager
{
public:
    void begin(AsyncWebSocket *ws) override;
    void loop() override;
    void handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg) override;
    void handleESPNowMessage(uint8_t *macAddress, String msg) override;

    String pendingSSID;
    String pendingPASS;
    bool pendingConnect = false;

private:
    void checkPendingConnection();
    void scanNetworks();
    unsigned long lastAttempt = 0;
    AsyncWebSocket *ws = nullptr;
    int scanClientId = -1;
    bool scanInProgress = false;
    void FallbackToDefaultSSID();
    String getStatusHtml();
    static bool saveCredentials(const String &ssid, const String &password);
    static std::vector<String> loadCredentials();
    void notifyStatusToAllClients();
};