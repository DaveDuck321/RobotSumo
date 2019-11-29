#include "components.h"

#define MOTOR_A_1 4
#define MOTOR_A_2 5
#define MOTOR_A_PWM 3

#define MOTOR_B_1 8
#define MOTOR_B_2 7
#define MOTOR_B_PWM 6

#define ENABLE_PIN 2

#define ULTRA_1_ECHO 12
#define ULTRA_1_TRIG 11
#define ULTRA_R_ECHO A1
#define ULTRA_L_ECHO A2

#define IR_SENSOR_L 9
#define IR_SENSOR_R 10

#define STOP_BTN A0

bool enabled = false;

Motor *motorL, *motorR;
Ultra *ultra, *ultraR, *ultraL;
IR *lightL, *lightR;
Button *button;

void DisableAll() {
  digitalWrite(ENABLE_PIN, 0);
}

void EnableAll() {
  digitalWrite(ENABLE_PIN, 1);
}

void ToggleMovement() {
  enabled = !enabled;
  digitalWrite(ENABLE_PIN, enabled);
}

void Spin(bool clockwise, float s) {
  motorL->drive(!clockwise, s);
  motorR->drive(clockwise, s);
}

void Move(bool forward, float s) {
  motorL->drive(forward, s);
  motorR->drive(forward, s);
}

void MoveTurn(bool forward, bool clockwise, float s) {
  if(clockwise) {
    motorL->drive(forward, s*0.9f);
    motorR->drive(forward, s);
  } else {
    motorL->drive(forward, s);
    motorR->drive(forward, s*0.9f);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println("Hello world");
  motorL = new Motor(MOTOR_A_2, MOTOR_A_1, MOTOR_A_PWM, 0.95
  );
  motorR = new Motor(MOTOR_B_2, MOTOR_B_1, MOTOR_B_PWM);
  ultra = new Ultra(ULTRA_1_ECHO, ULTRA_1_TRIG);
  ultraR = new Ultra(ULTRA_R_ECHO, ULTRA_1_TRIG);
  ultraL = new Ultra(ULTRA_L_ECHO, ULTRA_1_TRIG);
  button = new Button(STOP_BTN);

  lightL = new IR(IR_SENSOR_L);
  lightR = new IR(IR_SENSOR_R);
  
  pinMode(ENABLE_PIN, OUTPUT);
  DisableAll();
  motorL->drive(true, 1);
  motorR->drive(true, 1);
}

int spinDirection = 0;

void loop() {
  if(button->isClicked()) {
    ToggleMovement();
  }
  switch((!lightL->isCovered() << 1) + (!lightR->isCovered())) {
    case 3:
      spinDirection = 0;
      break;
    case 1:
      spinDirection = 2;
      break;
    case 2:
      spinDirection = 3;
      break;
    case 0:
      //Serial.println(ultra->getDistanceSync(300000));
      float d = ultra->getDistanceSync(60000);
      //Serial.print("Center: ");
      Serial.println(d);
      //Serial.print("Right:  ");
      //Serial.println(dR);
      //Serial.print("Left:  ");
      //Serial.println(dL);
      if(d < 60) {
        if(ultraR->getDistanceSync(60000)<d) {
          spinDirection = 4;
        } else if(ultraL->getDistanceSync(60000) < d){
          spinDirection = 5;
        } else {
          spinDirection = 1;
        }
      } else if(spinDirection==1){
        spinDirection = 2;
      }
      break;
  }

  switch(spinDirection) {
    case 0: case 1:
      Move(spinDirection, 1);
      break;
    case 4: case 5:
      MoveTurn(true, spinDirection==4, 1);
      break;
    default:
      Spin(spinDirection-2, 0.5);
  }
}
