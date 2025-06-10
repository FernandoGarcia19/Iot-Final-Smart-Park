#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  protected:
    byte pin;
    int threshold;
    int lapsus;
    int lightValue;
    bool isBlocked;
  public:
    Sensor(const byte& pin, const int& threshold, const int& lapsus)
    :pin(pin), threshold(threshold), lapsus(lapsus), lightValue(0), isBlocked(false){
    }
    ~Sensor(){}

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

    void init(){
      pinMode(pin, INPUT);
    }

    void readLightValue(){
      lightValue = analogRead(pin);
    }

    virtual void loop() = 0;

};

#endif