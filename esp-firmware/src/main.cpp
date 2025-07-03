#include <LittleFS.h>
#include "Credential/CredentialStorage.h"
#include "WiFiManager/WiFiManager.h"
#include "WebPortal/WebPortal.h"
#include <DNSServer.h>

// Global Instances
CredentialStorage storage;
WiFiManager wifiManager;
WebPortal webPortal;

const byte DNS_PORT = 53;
DNSServer dnsServer;

void setup() {
    Serial.begin(115200);

    // Mount LittleFS
    if (!LittleFS.begin()) {
        Serial.println("❌ LittleFS mount failed!");
        return;
    }
    Serial.println("✅ LittleFS mounted");

    // List all files for debugging
    Serial.println("📂 Listing LittleFS files:");
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        Serial.println("  " + dir.fileName());
    }

    // Start Access Point
    WiFi.softAP("ESP-Config");
    Serial.println("📡 AP Started: IP = " + WiFi.softAPIP().toString());

    // Start DNS Server for captive portal
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // Start WiFiManager
    wifiManager.begin(&storage, [](const String& status) {
        webPortal.notifyAll(status);
    });

    wifiManager.FallbackToDefaultSSID();

    // Start WebPortal
    webPortal.begin(&storage, &wifiManager);
}

void loop() {
    wifiManager.loop();
    webPortal.loop();
    dnsServer.processNextRequest(); // Handle DNS redirects
}
