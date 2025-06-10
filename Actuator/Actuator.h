#ifndef ACTUATOR_H
#define ACTUATOR_H
#define OPEN_TIME 5000
#include "ServoClient.h"
#include <ArduinoJson.h>

class Actuator
{
private:
  ServoClient servo;
  String state;

  bool isOpenTimerRunning = false;
  unsigned long openStartTime = 0;

public:
  Actuator(const byte &pin)
      : servo(pin), state("CLOSED")
  {
  }

  void init() {
    servo.init();
    closeGate();
  }

  String getState() {
    return state;
  }

  void openGate() {
    if (state == "CLOSED") {
      servo.open();
      state = "OPEN";
      openStartTime = millis();
      isOpenTimerRunning = true;
    }
  }

  void closeGate() {
    servo.close();
    state = "CLOSED";
    isOpenTimerRunning = false;
  }

  void loop() {
    if (state == "OPEN" && isOpenTimerRunning) {
      if (millis() - openStartTime >= OPEN_TIME) {
        closeGate();
        
      }
    }
  }
};
#endif
