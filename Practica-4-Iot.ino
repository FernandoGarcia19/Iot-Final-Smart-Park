#include "SmartParking.h"
#include "config.h"

SmartParking* parking;

void setup(){
  Serial.begin(115200);
  parking = new SmartParking(SSID, password, clientId, broker, port, actuatorPin, gateSensorPin, counterSensorPin, updateTopic, deltaTopic);
  parking->init();
}

void loop(){
  parking->loop();
}