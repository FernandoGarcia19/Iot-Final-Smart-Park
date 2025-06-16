#ifndef SMARTPARKING_H
#define SMARTPARKING_H
#include <WiFiManager.h>
#include "MQTTClient.h"
#include "./Sensor/Sensor.h"
#include "./Actuator/Actuator.h"

#define DEF_THRESHOLD 1600
#define DEF_LAPSUS 2

class SmartParking {
  private:
    MQTTClient mqttClient;
    Actuator actuator;
    Sensor gateSensor;
    Sensor counterSensor;
    WiFiManager wifiManager;

    String currentCounterState;
    String currentGateState;

    const char* updateTopic;
    const char* deltaTopic;
    const char* SSID;

    StaticJsonDocument<JSON_OBJECT_SIZE(64)> inputDoc;
    StaticJsonDocument<JSON_OBJECT_SIZE(16)> outputDoc;
    char outputBuffer[256];

    void reportState()
    {
      outputDoc.clear();
      outputDoc["state"]["reported"]["gate_actuator"]= actuator.getState();
      // outputDoc["state"]["reported"]["gate_sensor"] = gateSensor.getCurrentState();
      // outputDoc["state"]["reported"]["counter_sensor"] = counterSensor.getCurrentState();
      outputDoc["state"]["reported"]["config"]["threshold"] = gateSensor.getThreshold();
      outputDoc["state"]["reported"]["config"]["lapsus"] = gateSensor.getLapsus();
      size_t len = serializeJson(outputDoc, outputBuffer, sizeof(outputBuffer));
      outputBuffer[len] = '\0'; 
      mqttClient.publish(updateTopic, outputBuffer);
    }

    void reportGateState(){
      outputDoc["state"]["reported"]["gate_actuator"] = actuator.getState();
      size_t len = serializeJson(outputDoc, outputBuffer, sizeof(outputBuffer));
      outputBuffer[len] = '\0'; 
      mqttClient.publish(updateTopic, outputBuffer);
    }

    void reportConfig(){
      outputDoc["state"]["reported"]["config"]["threshold"] = gateSensor.getThreshold();
      outputDoc["state"]["reported"]["config"]["lapsus"] = gateSensor.getLapsus();
      size_t len = serializeJson(outputDoc, outputBuffer, sizeof(outputBuffer));
      outputBuffer[len] = '\0'; 
      mqttClient.publish(updateTopic, outputBuffer);
    }

    void reportGateBlocked(){
      outputDoc.clear();
      outputDoc["state"]["reported"]["gate_sensor"] = gateSensor.getCurrentState();
      outputDoc["state"]["reported"]["gate_actuator"]= actuator.getState();
      size_t len = serializeJson(outputDoc, outputBuffer, sizeof(outputBuffer));
      outputBuffer[len] = '\0'; 
      mqttClient.publish(updateTopic, outputBuffer);
    }

    void reportCounterBlocked(){
      outputDoc.clear();
      outputDoc["state"]["reported"]["counter_sensor"]= counterSensor.getCurrentState();
      outputDoc["state"]["reported"]["gate_actuator"]= actuator.getState();
      size_t len = serializeJson(outputDoc, outputBuffer, sizeof(outputBuffer));
      outputBuffer[len] = '\0'; 
      mqttClient.publish(updateTopic, outputBuffer);
    }

    void manageServoGate(const String &gateState){
      (gateState == "OPEN") ? actuator.openGate() : actuator.closeGate();
    }

    void setSensorThreshold(int threshold){
      gateSensor.setThreshold(threshold);
      counterSensor.setThreshold(threshold);
    }

    void setSensorLapsus(int lapsus){
      gateSensor.setLapsus(lapsus);
      counterSensor.setLapsus(lapsus);
    }

    void handleDelta(JsonVariant state) {
      if (state.containsKey("gate_actuator")) {
        String gateState = state["gate_actuator"].as<String>();
        manageServoGate(gateState);
        //reportGateState();
      }

      bool configChanged = false;

      if (state.containsKey("config")) {
        JsonVariant config = state["config"];

        if (config.containsKey("threshold")) {
          int threshold = config["threshold"];
          setSensorThreshold(threshold);
          configChanged = true;
        }

        if (config.containsKey("lapsus")) {
          int lapsus = config["lapsus"];
          setSensorLapsus(lapsus);
          configChanged = true;
        }

        if (configChanged) {
          //reportConfig();
        }
      }
    }

  public:
    SmartParking(const char* SSID, const char* clientId, const char* broker,const int &port, 
      const byte& actuatorPin, const byte& gateSensorPin, const byte& counterSensorPin, const char* updateTopic, const char* deltaTopic)
      : SSID(SSID),
        mqttClient(broker, port, clientId),
        actuator(actuatorPin), gateSensor(gateSensorPin, DEF_THRESHOLD, DEF_LAPSUS), counterSensor(counterSensorPin, DEF_THRESHOLD, DEF_LAPSUS),
        updateTopic(updateTopic), deltaTopic(deltaTopic), currentCounterState("CLEAR"), currentGateState("CLEAR")
    {
    }


    void init(){
      WiFi.mode(WIFI_AP_STA);
      wifiManager.autoConnect(SSID);
      actuator.init();
      gateSensor.init();
      counterSensor.init();

      mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length) {
        String message;
        for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

        Serial.print("MESSAGE ARRIVED ON TOPIC: ");
        Serial.println(topic);
        Serial.print("PAYLOAD: ");
        Serial.println(message);
      
        DeserializationError err = deserializeJson(inputDoc, payload);
        if (err) {
          Serial.print("deserializeJson failed: ");
          Serial.println(err.c_str());
          return;
        }
        JsonVariant deltaState = inputDoc["state"];
        this->handleDelta(deltaState);  // your custom processor
        reportState();
        inputDoc.clear();
      });
    }

    void loop(){
      if(!mqttClient.isConnected()){
        mqttClient.connect();
        mqttClient.subscribe(deltaTopic);
        reportState();
      }

      mqttClient.loop();
      gateSensor.loop();
      counterSensor.loop();
      //actuator.loop();
      // Serial.print("Gate: ");
      // Serial.println(gateSensor.getLightValue());
      if(currentGateState != gateSensor.getCurrentState()){
        currentGateState = gateSensor.getCurrentState();
        // reportGateState();
        reportGateBlocked();
      }
      // Serial.print("Counter: ");
      // Serial.println(counterSensor.getLightValue());
      if(currentCounterState != counterSensor.getCurrentState()){
        currentCounterState = counterSensor.getCurrentState();
        reportCounterBlocked();
      }
      
      delay(1000);
    }
};
#endif
