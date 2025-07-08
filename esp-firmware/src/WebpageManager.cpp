#include "WebpageManager.h"
#include <Utils/Constants.h>
#include <LittleFS.h>

void WebpageManager::begin(AsyncWebSocket* ws) {
    this->ws = ws;
    setupCaptivePortalRoutes();
    server.addHandler(ws);

    if (!LittleFS.exists("/index.html")) {
        Serial.println("❌ index.html missing");
        return;
    } 
    Serial.println("✅ index.html found");
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();

    WiFi.mode(WIFI_AP_STA);
    delay(100);
    WiFi.scanNetworks(true);

    Serial.println("🚀 WebPortal started");
}

void WebpageManager::setupCaptivePortalRoutes() {
    // Start DNS Server for captive portal
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    server.on("/generate_204", HTTP_ANY, serveIndex);
    server.on("/hotspot-detect.html", HTTP_ANY, serveIndex);
    server.on("/redirect", HTTP_ANY, serveIndex);
    server.on("/ncsi.txt", HTTP_ANY, serveIndex);
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(204);
    });
    server.on("/leds", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/leds.html", "text/html");
    });
    server.onNotFound(serveIndex);
}

void WebpageManager::serveIndex(AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
}

void WebpageManager::loop(){
        dnsServer.processNextRequest(); // Handle DNS redirects
}

