#include <LittleFS.h>
#include "Credential/CredentialStorage.h"
#include "WiFiManager/WiFiManager.h"
#include "WebPortal/WebPortal.h"
#include "Utils/Utility.h"

CredentialStorage storage;
WiFiManager wifiManager;
WebPortal webPortal;
Utility util;

void setup() {
    Serial.begin(115200);
    // util.wipeAllMemory();

    if (!LittleFS.begin()) {
        Serial.println("❌ LittleFS mount failed!");
        return;
    }

    Serial.println("📂 Listing LittleFS files:");
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        Serial.println("  " + dir.fileName());
    }

    WiFi.softAP("ESP-Config");
    Serial.println("📡 AP Started: IP = " + WiFi.softAPIP().toString());

    // Inject status WebSocket callback into WiFiManager
    wifiManager.begin(&storage, [](const String& status) {
        webPortal.notifyAll(status);
    });

    wifiManager.FallbackToDefaultSSID();
    webPortal.begin(&storage, &wifiManager);
}

void loop() {
    wifiManager.loop();
    webPortal.loop();
}
