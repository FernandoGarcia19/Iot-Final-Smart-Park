#include "../SmartParking.h"
#include "../config.h"

SmartParking* parking;

void setup(){
  Serial.begin(115200);
  parking = new SmartParking(SSIDentrance, entranceId, broker, port, actuatorPin, gateSensorPin, counterSensorPin, entrUpdateTopic, entrDeltaTopic);
  parking->init();
}

void loop(){
  parking->loop();
}