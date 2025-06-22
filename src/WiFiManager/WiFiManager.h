#pragma once
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <DNSServer.h>
#include "Credential/CredentialStorage.h"

class WiFiManager {
public:
    void begin(CredentialStorage* store);
    void startCaptivePortal();
    void loop();
    String getStatusHtml();
    String pendingSSID;
    String pendingPASS;
    bool pendingConnect = false;
    CredentialStorage* pendingStore = nullptr;
    void FallbackToDefaultSSID();
private:
    DNSServer dnsServer;
    CredentialStorage* storage;
    void checkPendingConnection();
    unsigned long lastAttempt = 0;
    const unsigned long IDLE_TIMEOUT = 10000; // 10 seconds
};