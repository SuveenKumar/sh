#pragma once
#include "BaseModel.h"

class Status : public BaseModel {
public:
    String status;
    bool connected;
    
private:
    void fromJson(JsonDocument& doc) override {
        status = doc["status"] | "";
        connected = doc["connected"];
    }

    void toJson(JsonDocument& doc) override {
        doc["status"] = status;
        doc["connected"] = connected;
    }

    const String getType() const override {
        return "Status";
    }
};
