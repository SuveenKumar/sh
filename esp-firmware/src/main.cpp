#include "GPIOManager.h"
#include "WiFiManager.h"
#include "WebpageManager.h"
#include "BaseManager.h"
#include "Utils/CommonUtility.h"

// Global Instances
WiFiManager wifiManager;
WebpageManager webpageManager;
AsyncWebSocket ws{"/ws"};
GPIOManager gpioManager;

void setup()
{
    Serial.begin(115200);

    // Start GPIOManager
    gpioManager.begin(&ws);

    // Start WiFiManager
    wifiManager.begin(&ws);

    // Start WebPage
    webpageManager.begin(&ws);

    // Setup central WebSocket event dispatcher
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType eventType, void *arg, uint8_t *data, size_t len)
               {
        String msg;
        for (size_t i = 0; i < len; i++) msg += (char)data[i];
        // Dispatch to all managers
        gpioManager.handleWebSocketMessage(client, eventType, msg);
        wifiManager.handleWebSocketMessage(client, eventType, msg);
        webpageManager.handleWebSocketMessage(client, eventType, msg); });

    ws.enable(true);
    CommonUtility::LogInfo("🧩 WebSocket initialized");
}

void loop()
{
    gpioManager.loop();
    wifiManager.loop();
    webpageManager.loop();
}
