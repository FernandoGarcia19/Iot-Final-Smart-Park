#ifndef GATESENSOR_H
#define GATESENSOR_H
#define GATE_SENSOR_DELAY 5
#include "Sensor.h"
class GateSensor : public Sensor {
  private:
    unsigned long startMillis = 0;
    bool timingBlock = false;
    bool hasTriggered = false;
    String currentState;

  public:
    GateSensor(const byte& pin, const int& threshold, const int& lapsus)
      : Sensor(pin, threshold, lapsus), currentState("CLEAR") {}

    ~GateSensor() {}

    void loop() {
      readLightValue();
      if (lightValue <= threshold) {
        if (!timingBlock) {
          startMillis = millis();
          timingBlock = true;
        } else if (!hasTriggered && (millis() - startMillis >= (unsigned long)(lapsus * 1000))) {
          currentState = "BLOCKED";
          hasTriggered = true;
        }
      } else {
        // Reset when light is restored
        timingBlock = false;
        hasTriggered = false;
        currentState = "CLEAR";
      }
    }

    String getCurrentState() {
      return currentState;
    }
};

#endif