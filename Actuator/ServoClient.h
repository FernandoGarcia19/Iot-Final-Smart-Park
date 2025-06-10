#ifndef SERVOCLIENT_H
#define SERVOCLIENT_H
#define CLOSED_ANGLE 90
#define OPEN_ANGLE 0
#include <ESP32Servo.h>

class ServoClient {
  private:
    Servo servo;
    byte pin;
    public:
    ServoClient(byte pin)
    : pin(pin){
    }
    void init(){
      servo.attach(pin);
      servo.write(CLOSED_ANGLE);
    }
    void moveToTarget(const byte& target){
      servo.write(target);
    }

    void open(){
      servo.write(OPEN_ANGLE);
    }
    void close(){
      servo.write(CLOSED_ANGLE);
    }
};

#endif 