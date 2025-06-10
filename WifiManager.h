#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H
#include <WiFi.h>

class WifiManager{
  private:
    const char* password;
    const char* SSID;
  public:
    WifiManager(const char* SSID, const char* password)
    : password(password), SSID(SSID)
    {
      WiFi.mode(WIFI_STA);
    }

    void connect() {
      WiFi.begin(SSID, password);
      Serial.print("Connecting to ");
      Serial.println(SSID);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
      }
      Serial.print("Connected to WiFi. IP address: ");
      Serial.println(WiFi.localIP());
    }

    bool isConnected(){
        return (WiFi.status() == WL_CONNECTED);
    }
};

#endif