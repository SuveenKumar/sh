#include "GPIOManager.h"
#include "WiFiManager.h"
#include "WebpageManager.h"
#include "BaseManager.h"
#include "Utils/CommonUtility.h"
#include <espnow.h>
#include "Utils/Constants.h"

void setupWebSocket();
void setupESPNow();
void onEspNowMessageReceive(uint8_t *mac, uint8_t *data, uint8_t len);
void onEspNowMessageSent(uint8_t *mac_addr, uint8_t status);

// WiFiManager wifiManager;
WebpageManager webpageManager;
AsyncWebSocket ws{"/ws"};
GPIOManager gpioManager;

void setup()
{
    Serial.begin(115200);

    // Start GPIOManager
    gpioManager.begin(&ws);

    // Start WiFiManager
    // wifiManager.begin(&ws);

    // Start WebPage
    webpageManager.begin(&ws);

    setupWebSocket();
    setupESPNow();
}

void loop()
{
    gpioManager.loop();
    // wifiManager.loop();
    webpageManager.loop();
}

void setupWebSocket()
{
    // Setup central WebSocket event dispatcher
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType eventType, void *arg, uint8_t *data, size_t len)
               {
        String msg;
        for (size_t i = 0; i < len; i++) msg += (char)data[i];

        if (eventType == WS_EVT_CONNECT)
        {
            CommonUtility::LogInfo("🔌 WebSocket client connected: " + client->remoteIP().toString());
        }
        if (eventType == WS_EVT_DATA)
        {
            CommonUtility::LogInfo("📩 WebSocket received message: " + msg);
        }
        // Dispatch to all managers
        gpioManager.handleWebSocketMessage(client, eventType, msg);
        //wifiManager.handleWebSocketMessage(client, eventType, msg);
        webpageManager.handleWebSocketMessage(client, eventType, msg); });

    ws.enable(true);
    CommonUtility::LogInfo("🧩 WebSocket initialized");
}

void setupESPNow()
{
    if (esp_now_init() != 0)
    {
        Serial.println("ESP-NOW Init Failed");
        return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_recv_cb(onEspNowMessageReceive);
    esp_now_register_send_cb(onEspNowMessageSent);
    esp_now_add_peer(BROADCAST_MAC_ADDRESS, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void onEspNowMessageReceive(uint8_t *mac, uint8_t *data, uint8_t len)
{
    String msg = "";
    for (int i = 0; i < len; i++)
        msg += (char)data[i];
    CommonUtility::LogInfo("Received: " + msg);
    gpioManager.handleESPNowMessage(mac, msg);
    // wifiManager.handleESPNowMessage(macAddress, msg);
    webpageManager.handleESPNowMessage(mac, msg);
}

void onEspNowMessageSent(uint8_t *mac_addr, uint8_t status)
{
    Serial.print("📤 Broadcast sent: ");
    Serial.println(status == 0 ? "✔️ Success" : "❌ Failed");
}
