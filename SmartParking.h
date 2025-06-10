#ifndef SMARTPARKING_H
#define SMARTPARKING_H
#include "WifiManager.h"
#include "MQTTClient.h"
#include "./Sensor/GateSensor.h"
#include "./Sensor/SmartCounter.h"
#include "./Actuator/Actuator.h"

#define DEF_THRESHOLD 2000
#define DEF_LAPSUS 1

class SmartParking {
  private:
    WifiManager wifiManager;
    MQTTClient mqttClient;
    Actuator actuator;
    GateSensor gateSensor;
    SmartCounter counterSensor;

    String currentState;
    int currentCarCount;

    const char* updateTopic;
    const char* deltaTopic;

    StaticJsonDocument<JSON_OBJECT_SIZE(64)> inputDoc;
    StaticJsonDocument<JSON_OBJECT_SIZE(16)> outputDoc;
    char outputBuffer[256];

    void reportState()
    {
      outputDoc["state"]["reported"]["gate_actuator"]["gate_state"] = actuator.getState();
      outputDoc["state"]["reported"]["gate_sensor"]["sensor_state"] = gateSensor.getCurrentState();
      outputDoc["state"]["reported"]["car_count"] = counterSensor.getCarCounter();
      outputDoc["state"]["reported"]["config"]["threshold"] = gateSensor.getThreshold();
      outputDoc["state"]["reported"]["config"]["lapsus"] = gateSensor.getLapsus();
      size_t len = serializeJson(outputDoc, outputBuffer, sizeof(outputBuffer));
      outputBuffer[len] = '\0'; 
      mqttClient.publish(updateTopic, outputBuffer);
    }

    void reportGateState(){
      outputDoc["state"]["reported"]["gate_actuator"]["gate_state"] = actuator.getState();
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

    void reportSensorBlocked(){
      outputDoc["state"]["reported"]["gate_sensor"]["sensor_state"] = gateSensor.getCurrentState();
      size_t len = serializeJson(outputDoc, outputBuffer, sizeof(outputBuffer));
      outputBuffer[len] = '\0'; 
      mqttClient.publish(updateTopic, outputBuffer);
    }

    void reportCar(){
      outputDoc["state"]["reported"]["car_count"] = counterSensor.getCarCounter();
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

    void setCarCount(int carCount){
      counterSensor.setCarCounter(carCount);
    }

    void handleDelta(JsonVariant state) {
      if (state.containsKey("gate_actuator")) {
        String gateState = state["gate_actuator"]["gate_state"].as<String>();
        manageServoGate(gateState);
        reportGateState();
      }

      if (state.containsKey("car_count")) {
        int carCount = state["car_count"].as<int>();
        setCarCount(carCount);
        reportCar();
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
          reportConfig();
        }
      }
    }

  public:
    SmartParking(const char* SSID, const char* password, const char* clientId, const char* broker,const int &port, 
      const byte& actuatorPin, const byte& gateSensorPin, const byte& counterSensorPin, const char* updateTopic, const char* deltaTopic)
      : wifiManager(SSID, password),
        mqttClient(broker, port, clientId),
        actuator(actuatorPin), gateSensor(gateSensorPin, DEF_THRESHOLD, DEF_LAPSUS), counterSensor(counterSensorPin, DEF_THRESHOLD, DEF_LAPSUS),
        updateTopic(updateTopic), deltaTopic(deltaTopic), currentState("CLEAR"), currentCarCount(0)
    {
    }


    void init(){
      wifiManager.connect();
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
 
      if(currentState != gateSensor.getCurrentState()){
        currentState = gateSensor.getCurrentState();
        // reportGateState();
        reportSensorBlocked();
      }
      
      if(currentCarCount != counterSensor.getCarCounter()){
        currentCarCount = counterSensor.getCarCounter();
        reportCar();
      }
      delay(1000);
    }
};
#endif