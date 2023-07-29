// rx -> espcam tx
// tx -> espcam rx
//gnd -> gnd
//5v to 5v
#include "CytronMotorDriver.h"
#include <SoftwareSerial.h>

#define Bluetooth
#define Wifi

char data = "S";
unsigned long prevTime_T1 = millis();
unsigned long prevTime_T2 = millis();

CytronMD motor1(PWM_PWM, 10, 9);   
CytronMD motor2(PWM_PWM, 6, 5); 

SoftwareSerial BTSerial(2, 3); // RX, TX

void setup() {
  Serial.begin(115200);
  BTSerial.begin(9600);
}

void loop() {
    #ifdef Bluetooth
  if (BTSerial.available()) {
    unsigned long currentTime_T1 = millis();
    data = BTSerial.read();
    Serial.print(data);
    if (currentTime_T1 - prevTime_T1 >= 5000) {
      MoveCondition(data);
    }
  }
  
  #endif

  #ifdef Wifi
  if (Serial.available()) {
    unsigned long currentTime_T2 = millis();
    data = Serial.read();
    Serial.print(data);
    if (currentTime_T2 - prevTime_T2 >= 20000){
      MoveCondition(data);
    }
  }
  #endif 
}

void robotMove (int sppeedLeft, int speedRight) {
  motor1.setSpeed(sppeedLeft);   
  motor2.setSpeed(speedRight); 
}

void MoveCondition (char data) {
  switch (data) {
        case 'S':
          robotMove(0, 0);
          break;
        case 'F':
          robotMove(-230, -230);
          break;
        case 'B':
          robotMove(230, 230);
          break;
        case 'L':
          robotMove(170, -170);
          break;
        case 'R':
          robotMove(-170, 170);
          break;
        default:
          break;
      }
}
