#pragma once
#include <WiFi.h>

class WifiManager {
public:
    void begin(const char* ssid, const char* pass) {
        _ssid = ssid;
        _pass = pass;
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, pass);
        Serial.print("WiFi baglaniyor");
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.printf("\nWiFi bagli: %s\n", WiFi.localIP().toString().c_str());
    }

    bool connected() {
        return WiFi.status() == WL_CONNECTED;
    }

    void reconnectIfNeeded() {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi koptu, yeniden baglaniyor...");
            WiFi.disconnect();
            WiFi.begin(_ssid, _pass);
            int tries = 0;
            while (WiFi.status() != WL_CONNECTED && tries < 20) {
                delay(500);
                tries++;
            }
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("WiFi yeniden baglandi");
            }
        }
    }

    String chipId() {
        uint64_t mac = ESP.getEfuseMac();
        char id[17];
        snprintf(id, sizeof(id), "%04X%08X",
            (uint16_t)(mac >> 32), (uint32_t)mac);
        return String(id);
    }

private:
    const char* _ssid;
    const char* _pass;
};
