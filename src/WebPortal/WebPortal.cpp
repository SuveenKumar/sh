#include "WebPortal.h"
#include <LittleFS.h>

void WebPortal::begin(CredentialStorage* store, WiFiManager* wifi) {
    // Serve static index.html
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Handle form submission from the web page
    // Handle form submission from the web page
server.on("/submit", HTTP_POST, [store, wifi](AsyncWebServerRequest* request) {
    if (!request->hasArg("ssid") || !request->hasArg("password")) {
        Serial.println("Missing ssid or password");
        request->send(400, "text/plain", "Missing ssid or password");
        return;
    }

    String ssid = request->arg("ssid");
    String pass = request->arg("password");

    Serial.println("Received SSID: " + ssid + ", Password: " + pass);

    // Store in WiFiManager for background connection
    wifi->pendingSSID = ssid;
    wifi->pendingPASS = pass;
    wifi->pendingStore = store;
    wifi->pendingConnect = true;
});


    // Return WiFi connection status
    server.on("/status", HTTP_GET, [wifi](AsyncWebServerRequest* request) 
    {
        request->send(200, "text/html", wifi->getStatusHtml());
    });

    // Return available SSIDs in JSON
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest* request) {
        int n = WiFi.scanComplete();

        if (n == WIFI_SCAN_RUNNING) {
            request->send(200, "application/json", "[]");
            return;
        }

        if (n < 0) {
            Serial.println("Scan failed. Retrying...");
            WiFi.scanNetworks(true);  // Restart scan
            request->send(200, "application/json", "[]");
            return;
        }

        String json = "[";
        for (int i = 0; i < n; ++i) {
            if (i > 0) json += ",";
            json += "\"" + WiFi.SSID(i) + "\"";
        }
        json += "]";
        Serial.println(json);
        WiFi.scanDelete();
        WiFi.scanNetworks(true);  // Start new scan for next request
        request->send(200, "application/json", json);
    });

    // Reset credentials and disconnect WiFi
server.on("/reset", HTTP_POST, [store](AsyncWebServerRequest* request) {
    Serial.println("🔄 Reset requested");
    LittleFS.remove("/wifi.json");
    WiFi.disconnect(true); // Disconnect and erase STA config
    request->send(200, "text/html", "✅ WiFi credentials reset. Please reconnect.");
});

//For captive Portal
server.onNotFound([](AsyncWebServerRequest* request){
    //Serial.println("Redirecting unknown request to /index.html");
    request->send(LittleFS, "/index.html", "text/html");
});
server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* request){
    request->redirect("/index.html");
});
server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* request){
    request->redirect("/index.html");
});
server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest* request){
    request->redirect("/index.html");
});
server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
  request->redirect("/index.html");
});

    // Start web server
    server.begin();

    // Enable both AP and STA mode
    WiFi.mode(WIFI_AP_STA);
    delay(100);                   // Let WiFi mode settle
    WiFi.scanNetworks(true);      // Start async scan
}
