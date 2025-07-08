#include <LittleFS.h>
#include "WiFiManager.h"
#include "WebpageManager.h"
#include "Utils/FileUtility.h"
#include "Utils/PinsUtility.h"
#include <Utils/Constants.h>

// Global Instances
WiFiManager wifiManager;
WebpageManager webpageManager;
AsyncWebSocket ws{"/ws"};


void setup() {
    Serial.begin(115200);
    if(!FileUtility::mountFileSystem()){
        return;
    }
    bool* pinStates = FileUtility::loadPinStates();

    if(pinStates != nullptr){
        PinsUtility::updateDigitalPinsState(pinStates);
    }

    //Start WiFiManager
    wifiManager.begin(&ws);

    // Start WebPage
    webpageManager.begin(&ws);
    ws.enable(true);
    Serial.println("🧩 WebSocket initialized");
} 

void loop() {
    wifiManager.loop();
    webpageManager.loop();
}
