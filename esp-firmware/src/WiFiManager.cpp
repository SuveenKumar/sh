#include "WiFiManager.h"
#include <LittleFS.h>
#include "Utils/Constants.h"
#include "Utils/CommonUtility.h"
#include "Utils/MessageParser.h"

void WiFiManager::begin(AsyncWebSocket *ws)
{
    this->ws = ws;
    FallbackToDefaultSSID();
    CommonUtility::LogInfo("🚀 WiFiManager started");
}

void WiFiManager::loop()
{
    checkPendingConnection();
    scanNetworks();
    if (millis() - lastAttempt >= IDLE_TIMEOUT)
    {
        lastAttempt = millis();
        FallbackToDefaultSSID();
    }
}

void WiFiManager::handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg)
{
    if (eventType == WS_EVT_CONNECT)
    {
        CommonUtility::LogInfo("🔌 WebSocket client connected: " + client->remoteIP().toString());
        MessageData response;
        response.type = "Status";
        response.values.emplace_back(String(WiFi.isConnected()));
        response.values.emplace_back(getStatusHtml());
        CommonUtility::LogInfo(MessageParser::build(response));
        client->text(MessageParser::build(response));
        scanInProgress = true;
        scanClientId = client->id();
    }
    if (eventType == WS_EVT_DATA)
    {
        MessageData message = MessageParser::parse(msg);
        if (message.type == "Scan")
        {
            if (!scanInProgress)
            {
                scanInProgress = true;
                scanClientId = client->id();
            }
        }
        if (message.type == "Submit")
        {
            pendingSSID = message.values[0];
            pendingPASS = message.values[1];
            pendingConnect = true;
        }
        if (message.type == "Reset")
        {
            LittleFS.remove("/wifi.json");
            WiFi.disconnect(true);
            notifyStatusToAllClients();
        }
        if (message.type == "Reset")
        {
            LittleFS.remove("/wifi.json");
            WiFi.disconnect(true);
            notifyStatusToAllClients();
        }
    }
}

void WiFiManager::handleESPNowMessage(uint8_t *macAddress, String msg){

}

void WiFiManager::notifyStatusToAllClients()
{
    MessageData response;
    response.type = "Status";
    response.values.emplace_back(String(WiFi.isConnected()));
    response.values.emplace_back(getStatusHtml());
    CommonUtility::LogInfo(MessageParser::build(response));
    ws->textAll(MessageParser::build(response));
}

void WiFiManager::FallbackToDefaultSSID()
{
    if (pendingConnect)
    {
        CommonUtility::LogInfo("⏭️ Skipping fallback: pending connection in progress");
        return;
    }

    if (WiFi.isConnected())
        return;

    std::vector<String> credentials = loadCredentials();

    if (credentials.size() == 0 || credentials[0] == "")
    {
        // CommonUtility::LogInfo("⚠️ No credentials found for fallback");
        return;
    }

    String savedSSID = credentials[0];
    String savedPassword = credentials.size() == 2 ? credentials[1] : "";

    CommonUtility::LogInfo("📶 Attempting fallback connection to SSID: " + savedSSID + " Pass: " + savedPassword);
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
        delay(100);
        yield();
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED)
    {
        CommonUtility::LogInfo("✅ Fallback connected to: " + savedSSID);
        notifyStatusToAllClients();
    }
    else
    {
        CommonUtility::LogInfo("❌ Fallback failed to connect");
        notifyStatusToAllClients();
    }
}

void WiFiManager::checkPendingConnection()
{
    if (!pendingConnect)
        return;

    CommonUtility::LogInfo("⏳ Attempting connection to submitted SSID: " + pendingSSID + " " + pendingPASS);

    notifyStatusToAllClients();

    WiFi.begin(pendingSSID.c_str(), pendingPASS.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
        delay(100);
        yield();
        Serial.print(".");
    }
    Serial.println();
    pendingConnect = false;

    if (WiFi.status() == WL_CONNECTED)
    {
        CommonUtility::LogInfo("✅ Connected to: " + pendingSSID);
        if (saveCredentials(pendingSSID, pendingPASS))
        {
            CommonUtility::LogInfo("💾 Credentials saved successfully");
        }
        else
        {
            CommonUtility::LogInfo("⚠️ Connected, but failed to save credentials");
        }
        notifyStatusToAllClients();
    }
    else
    {
        CommonUtility::LogInfo("❌ Failed to connect to: " + pendingSSID);
        notifyStatusToAllClients();
    }
}

String WiFiManager::getStatusHtml()
{
    if (pendingConnect)
    {
        return "⏳ Attempting connection to: " + pendingSSID;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        return "✅ Connected to: " + WiFi.SSID() + "  " + WiFi.localIP().toString();
    }
    return "❌ Not connected";
}

void WiFiManager::scanNetworks()
{
    int result = WiFi.scanComplete();
    if (scanInProgress && result >= 0)
    {
        CommonUtility::LogInfo("scanNetworks1");
        MessageData response;
        response.values.reserve(result);
        response.type = "ScanResults";
        for (int i = 0; i < result; ++i)
        {
            response.values.emplace_back(String(WiFi.SSID(i).c_str()));
        }
        WiFi.scanDelete();

        AsyncWebSocketClient *target = ws->client(scanClientId);
        if (target && target->canSend())
        {
            auto json = MessageParser::build(response);
            CommonUtility::LogInfo(json);
            target->text(json);
        }

        scanInProgress = false;
        scanClientId = -1;

        // Optionally scan again
        WiFi.scanNetworks(true);
    }
    else if (scanInProgress && result == -2)
    {
        // Scan never started or was cleared; reset and try again
        scanInProgress = false;
        scanClientId = -1;
        WiFi.scanNetworks(true);
    }
}

std::vector<String> WiFiManager::loadCredentials()
{

    std::vector<String> result(2);
    String credentials = CommonUtility::loadStringFromFile("/credentials.txt");

    int sepIndex = credentials.indexOf('|');

    if (sepIndex != -1)
    {
        result[0] = credentials.substring(0, sepIndex);
        result[1] = credentials.substring(sepIndex + 1);
    }

    return result;
}

bool WiFiManager::saveCredentials(const String &ssid, const String &password)
{
    String credentials = ssid + "|" + password;
    return CommonUtility::saveStringToFile("/credentials.txt", credentials);
}
