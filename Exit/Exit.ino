#include "../SmartParking.h"
#include "../config.h"

SmartParking* parking;

void setup(){
  Serial.begin(115200);
  parking = new SmartParking(SSIDexit, exitId, broker, port, actuatorPin, gateSensorPin, counterSensorPin, exitUpdateTopic, exitDeltaTopic);
  parking->init();
}

void loop(){
  parking->loop();
}