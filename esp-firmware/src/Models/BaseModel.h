#pragma once
#include <ArduinoJson.h>
#include "Utils/Log.h"
class BaseModel {
private:
    virtual void fromJson(JsonDocument& doc) = 0;
    virtual void toJson(JsonDocument& doc) = 0;
    virtual const String getType() const = 0;
public:
    String serialize() {
        StaticJsonDocument<256> doc;
        toJson(doc);
        doc["type"] = getType();
        String json;
        serializeJson(doc, json);
        return json;
    }

    void deserialize(String& json) {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, json);
        if (!error) {
            fromJson(doc);
        } else {
            Log::Info("⚠️ JSON Parse Error: "+String(error.c_str()));
        }
    }

    virtual ~BaseModel() {}
};
