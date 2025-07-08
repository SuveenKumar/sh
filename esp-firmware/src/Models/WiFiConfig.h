#pragma once
#include "BaseModel.h"

class WiFiConfig : public BaseModel {
public:
    String ssid;
    String password;
    
private:
    void fromJson(JsonDocument& doc) override {
        ssid = doc["ssid"] | "";
        password = doc["password"] | "";
    }

    void toJson(JsonDocument& doc) override {
        doc["ssid"] = ssid;
        doc["password"] = password;
    }

    const String getType() const override {
        return "WiFiConfig";
    }
};
