#ifndef COMPONENTS_H
#define COMPONENTS_H

class Motor {
  private:
  int pin1, pin2, pwm;
  int currentSpeed = 0;
  float calibration;
  public:
  Motor(int pin1, int pin2, int pwm, float calibration = 1) : pin1(pin1), pin2(pin2), pwm(pwm), calibration(calibration) {
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pwm, OUTPUT);
  }
  void drive(bool forward, float s) {
    digitalWrite(pin1, forward);
    digitalWrite(pin2, !forward);
    int requestedSpeed = (int)(calibration*s*255);
    if(currentSpeed != requestedSpeed) {
        analogWrite(pwm, requestedSpeed);
        currentSpeed = requestedSpeed;
    }
  }
};

class Ultra {
  private:
  int trigger, echo;
  float lastDistance = 0;
  //Async code
  bool sendingPulse = false;
  bool sentPulse = false;
  bool receivingPulse = false;
  unsigned long timer = 0;
  
  public:
  Ultra(int echo, int trigger) : echo(echo), trigger(trigger) {
    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);
  }
  //Can take up to 2 seconds -- but guarantees accurate distance
  float getDistanceSync(unsigned long timeout) {  
    digitalWrite(trigger, 1);
    delayMicroseconds(10);
    digitalWrite(trigger, 0);
    
    float duration = pulseIn(echo, 1, timeout);
    if(duration != 0) {
      lastDistance = duration*0.034/2;
      return lastDistance;
    }
    return 300;
    return lastDistance;
  }
  float getDistance(unsigned long timeout) {
    unsigned long currentTime = micros();
    if(!sendingPulse && !sentPulse) {
      sendingPulse = true;
      timer = currentTime;
      digitalWrite(trigger, 1);
    } else if(sendingPulse && currentTime - timer > 10) {
      sentPulse = true;
      digitalWrite(trigger, 0);
    } else if(sentPulse && !receivingPulse && digitalRead(echo)) {
      timer = currentTime;
      receivingPulse = true;
    } else if(receivingPulse && !digitalRead(echo)) {
      float duration = currentTime - timer;
      lastDistance = duration*0.034/2;
      sentPulse = receivingPulse = sendingPulse = false;
    } else if(currentTime - timer > timeout) {
      sentPulse = receivingPulse = sendingPulse = false;
    }
    return lastDistance;
  }
};

class IR {
  private:
  int pin;
  public:
  IR(int pin) : pin(pin) {
    pinMode(pin, INPUT_PULLUP);
  }
  bool isCovered() {
    return !digitalRead(pin);
  }
};

class Button {
  private:
  int pin;
  bool pressed = false;
  unsigned long changeTime = 0;
  
  public:
  Button(int pin) : pin(pin) {
    pinMode(pin, INPUT);
  }
  bool isClicked() {
    unsigned long currentTime = millis();
    bool timeoutExpired = currentTime - changeTime > 50;
    
    bool isDown = digitalRead(pin);
    if(isDown) {
      changeTime = currentTime;
      if(!pressed && timeoutExpired) {
        pressed = true;
        return true;
      }
    }
    pressed = pressed && !timeoutExpired;
    return false;
  }
  bool isDown() {
    return pressed;
  }
};

#endif
