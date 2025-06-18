#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  protected:
    byte pin;
    int threshold;
    int lapsus;
    int lightValue;
    bool isBlocked;
    unsigned long startMillis = 0;
    bool timingBlock = false;
    bool hasTriggered = false;
    String currentState;
  public:
    Sensor(const byte& pin, const int& threshold, const int& lapsus)
    :pin(pin), threshold(threshold), lapsus(lapsus), lightValue(0), isBlocked(false), currentState("CLEAR"){
    }
    ~Sensor(){}

    void init(){
      pinMode(pin, INPUT);
    }

    int getLightValue(){
      return this->lightValue;
    }

    int getLapsus(){
      return this->lapsus;
    }

    void setLapsus(const int& lapsus){
      this->lapsus = lapsus;
    }

    int getThreshold(){
      return this->threshold;
    }

    void setThreshold(const int& threshold){
      this->threshold = threshold;
    }

    bool getIsBlocked(){
      return isBlocked;
    }

    void readLightValue(){
      lightValue = analogRead(pin);
    }

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
