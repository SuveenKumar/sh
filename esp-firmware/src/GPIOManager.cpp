#include "GPIOManager.h"
#include <Utils/Constants.h>
#include <LittleFS.h>
#include "Utils/CommonUtility.h"
#include "Utils/MessageParser.h"

void GPIOManager::begin(AsyncWebSocket *ws)
{
    this->ws = ws;
    SetupGPIOPins();
    mountFileSystem();
    String pinsState = CommonUtility::loadStringFromFile("/leds.txt");
    if (pinsState.isEmpty())
    {
        pinsState = "0000";
        CommonUtility::saveStringToFile("/leds.txt", pinsState);
    }
    updateAllPinsState(pinsState);
    
    // Start Access Point
    WiFi.mode(WIFI_AP_STA);
    String softApSSID = CommonUtility::loadStringFromFile("/device.txt");
    if (softApSSID.isEmpty())
    {
        WiFi.softAP("ESP-Config");
    }
    else
    {
        WiFi.softAP(softApSSID);
    }

    CommonUtility::LogInfo("📡 AP Started: IP = " + WiFi.softAPIP().toString());
    delay(100);

    CommonUtility::broadcastEspNowMessage(GetPinStateMessage(pinsState));
    CommonUtility::LogInfo("Last Pins State: " + pinsState);
    CommonUtility::LogInfo("🚀 GPIOManager started");
}

void GPIOManager::loop()
{
}

void GPIOManager::handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg)
{
    if (eventType == WS_EVT_CONNECT)
    {
        String json = GetPinStateMessage(CommonUtility::loadStringFromFile("/leds.txt"));
        CommonUtility::broadcastEspNowMessage("PinsStatus");
        client->text(json);

        MessageData deviceName;
        deviceName.type = "DeviceName";
        deviceName.values.push_back(WiFi.softAPSSID());
        client->text(MessageParser::build(deviceName));
    }

    if (eventType == WS_EVT_DATA)
    {
        MessageData message = MessageParser::parse(msg);
        if (message.type == "TogglePin")
        {
            if(message.values[0] == WiFi.softAPSSID())
            {
                TogglePin(message);
            }
            else
            {
                CommonUtility::LogInfo("Forwarding message to: " + message.values[0]);
                CommonUtility::broadcastEspNowMessage(msg);
            }
        }
        if(message.type == "ChangeName"){
            message.type = "NameChanged";
            ws->textAll(MessageParser::build(message));
            changeSoftAPName(message.values[0]);
            CommonUtility::LogInfo("📡 AP Started: IP = " + WiFi.softAPIP().toString());
        }
    }
}

void GPIOManager::handleESPNowMessage(uint8_t *macAddress, String msg)
{
    MessageData message = MessageParser::parse(msg);
    if (message.type == "PinsStatus")
    {
        String json = GetPinStateMessage(CommonUtility::loadStringFromFile("/leds.txt"));
        CommonUtility::broadcastEspNowMessage(json);
    }

    if (message.type == "TogglePin" && message.values[0] == WiFi.softAPSSID())
    {
        TogglePin(message);
    }
    if (message.type == "PinsState")
    {
        String json = MessageParser::build(message);
        ws->textAll(json);
    }
}

void GPIOManager::changeSoftAPName(String newSSID, String newPassword) {
  WiFi.softAPdisconnect(true);
  delay(100);

  if (newPassword.length() >= 8) {
    WiFi.softAP(newSSID.c_str(), newPassword.c_str());
  } else {
    WiFi.softAP(newSSID.c_str());
  }
  if (CommonUtility::saveStringToFile("/device.txt", newSSID))
  {
        Serial.println("✅ JSON saved to LittleFS");
  }
  CommonUtility::LogInfo("Changed SoftAP to: "+ newSSID +" "+ WiFi.softAPIP().toString());
}


void GPIOManager::TogglePin(MessageData message)
{
    String deviceName = message.values[0];
    int index = message.values[1][0] - '0';
    char state = message.values[2][0];

    String newStates = CommonUtility::loadStringFromFile("/leds.txt");
    newStates[index] = state;
    updatePinState(index, state - '0');

    CommonUtility::LogInfo("Updated pin State:" + newStates);
    if (CommonUtility::saveStringToFile("/leds.txt", newStates))
    {
        Serial.println("✅ JSON saved to LittleFS");
    }

    String json = GetPinStateMessage(newStates);
    CommonUtility::LogInfo("📣 Notifying all WebSocket clients: " + json);
    ws->textAll(json);
    CommonUtility::broadcastEspNowMessage(json);
}

String GPIOManager::GetPinStateMessage(String pinStates){
    MessageData response;
    response.type = "PinsState";
    response.values.emplace_back(WiFi.softAPSSID());
    response.values.emplace_back(pinStates);

    return MessageParser::build(response);
}

bool GPIOManager::mountFileSystem()
{
    // Mount LittleFS
    if (!LittleFS.begin())
    {
        CommonUtility::LogInfo("❌ LittleFS mount failed!");
        return false;
    }

    CommonUtility::LogInfo("✅ LittleFS mounted");
    // List all files for debugging
    CommonUtility::LogInfo("📂 Listing LittleFS files:");
    Dir dir = LittleFS.openDir("/");
    while (dir.next())
    {
        CommonUtility::LogInfo("  " + dir.fileName());
    }
    return true;
}

void GPIOManager::SetupGPIOPins()
{
    for (int i = 0; i < NUM_PINS; i++)
    {
        pinMode(LED_PINS[i], OUTPUT);
    }
}

void GPIOManager::updateAllPinsState(const String &pinStates)
{
    for (size_t i = 0; i < pinStates.length(); i++)
    {
        // Active Low
        digitalWrite(LED_PINS[i], !(pinStates[i] - '0'));
    }
}

void GPIOManager::updatePinState(int index, bool state)
{
    // Active Low
    CommonUtility::LogInfo("updatePinState: " + String(LED_PINS[index]) + ": " + state);
    digitalWrite(LED_PINS[index], !state);
}
