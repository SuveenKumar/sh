#include "WebpageManager.h"
#include <Utils/Constants.h>
#include <LittleFS.h>
#include "Utils/CommonUtility.h"

void WebpageManager::begin(AsyncWebSocket *ws)
{
    setupCaptivePortalRoutes();
    server.addHandler(ws);

    if (!LittleFS.exists("/index.html"))
    {
        CommonUtility::LogInfo("❌ index.html missing");
        return;
    }
    CommonUtility::LogInfo("✅ index.html found");
    server.serveStatic("/", LittleFS, "/").setDefaultFile("leds.html");
    server.begin();

    CommonUtility::LogInfo("🚀 WebpageManager started");
}

void WebpageManager::loop()
{
    dnsServer.processNextRequest();
}

void WebpageManager::handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg)
{
}

void WebpageManager::handleESPNowMessage(uint8_t *macAddress, String msg){

}

void WebpageManager::setupCaptivePortalRoutes()
{
    // Start DNS Server for captive portal
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    server.on("/generate_204", HTTP_ANY, serveIndex);
    server.on("/hotspot-detect.html", HTTP_ANY, serveIndex);
    server.on("/redirect", HTTP_ANY, serveIndex);
    server.on("/ncsi.txt", HTTP_ANY, serveIndex);
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(204); });
    server.on("/leds", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/leds.html", "text/html"); });
    server.onNotFound(serveIndex);
}

void WebpageManager::serveIndex(AsyncWebServerRequest *request)
{
    request->send(LittleFS, "/leds.html", "text/html");
}