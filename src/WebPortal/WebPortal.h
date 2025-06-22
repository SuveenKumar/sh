#pragma once
#include <ESPAsyncWebServer.h>
#include "Credential/CredentialStorage.h"
#include "WiFiManager/WiFiManager.h"

class WebPortal {
public:
    void begin(CredentialStorage* store, WiFiManager* wifi);
private:
    AsyncWebServer server{80};
};
