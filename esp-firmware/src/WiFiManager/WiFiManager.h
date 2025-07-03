#pragma once
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "Credential/CredentialStorage.h"

class WiFiManager {
public:
    void begin(CredentialStorage* store, std::function<void(const String&)> statusCallback);
    void loop();
    String getStatusHtml();

    String pendingSSID;
    String pendingPASS;
    CredentialStorage* pendingStore = nullptr;
    bool pendingConnect = false;
    void FallbackToDefaultSSID();

private:
    CredentialStorage* storage = nullptr;
    std::function<void(const String&)> notifyCallback;

    void checkPendingConnection();
    unsigned long lastAttempt = 0;
    const unsigned long IDLE_TIMEOUT = 30000;
};