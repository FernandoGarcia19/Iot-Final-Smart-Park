#ifndef SMARTCOUNTER_H
#define SMARTCOUNTER_H
#define SMART_COUNTER_DELAY 5
#include "Sensor.h"

class SmartCounter: public Sensor{
  private:
    int carCounter; 
    unsigned long startLapsus; 
    unsigned long endLapsus; 
  public:
    SmartCounter(const byte& pin, const int& threshold, const int& lapsus)
    : Sensor(pin, threshold, lapsus), carCounter(0), startLapsus(0), endLapsus(0){
    }
    ~SmartCounter(){}

    int getCarCounter(){
      return carCounter;
    }
    
    void loop(){
      readLightValue();
      if(lightValue <= threshold && !isBlocked){
        startLapsus = millis();
        isBlocked = true;
      } else if (lightValue > threshold && isBlocked){
        endLapsus = millis();
        isBlocked = false;
        int blockedTime = (endLapsus - startLapsus) / 1000;
        if (blockedTime >= lapsus){
          carCounter++;
        }
      }
      //delay(SMART_COUNTER_DELAY);
    }

    void setCarCounter(int carCount){
      this->carCounter = carCount;
    }

};
#endif