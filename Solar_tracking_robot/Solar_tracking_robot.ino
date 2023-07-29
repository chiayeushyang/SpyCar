//Servo motor library
#include <Servo.h>
#include <Stepper.h>
#include "ADS1X15.h"
#include "CytronMotorDriver.h"
#include <SoftwareSerial.h>

#define motor1Pin1 13
#define motor1Pin2 12
#define motor1Pin3 8
#define motor1Pin4 7
#define SERVOPIN 11

// #define Bluetooth
// #define Wifi

ADS1115 ADS(0x48);

CytronMD motor1(PWM_PWM, 10, 9);
CytronMD motor2(PWM_PWM, 6, 5);

SoftwareSerial BTSerial(2, 3);  // RX, TX

char data = "S";

int16_t top_r;
int16_t top_l;
int16_t bot_r;
int16_t bot_l;
int avgtop;
int avgbot;
int avgleft;
int avgright;
int diffelev;
int diffazi;

const int ldrLeftBotChannel = 1;
const int ldrLeftTopChannel = 0;
const int ldrRightTopChannel = 3;
const int ldrRightBotChannel = 2;

int THRESHOLD_VALUE = 800;

int SERVO_MIN = 20;
int SERVO_MAX = 130;
int SERVO_SMOOTH = 15;

int pos = SERVO_MIN;

unsigned long prevTime_T1 = millis();
unsigned long prevTime_T2 = millis();

const int stepsPerRevolution = 2038;
int totalSteps = 0;

Stepper myStepper = Stepper(stepsPerRevolution, motor1Pin1, motor1Pin3, motor1Pin2, motor1Pin4);
Servo servo_updown;

void setup() {

  Serial.begin(115200);
  BTSerial.begin(9600);
  myStepper.setSpeed(10);
  servo_updown.attach(SERVOPIN);
  ADS.begin();
}

void loop() {
  automaticsolartracker();
  // myStepper.setSpeed(10);
  // myStepper.step(stepsPerRevolution);
  // delay(1000);

  // // Rotate CCW quickly at 10 RPM
  // myStepper.setSpeed(10);
  // myStepper.step(-stepsPerRevolution);
  // delay(1000);


  // for (pos = servomin; pos <= servomax; pos += 1) { // goes from 0 degrees to 180 degrees
  //   // in steps of 1 degree
  //   myservo.write(pos);              // tell servo to go to position in variable 'pos'
  //   delay(15);// waits 15ms for the servo to reach the position
  // }

  // for (pos = servomax; pos >= servomin; pos -= 1) { // goes from 180 degrees to 0 degrees
  //   myservo.write(pos);              // tell servo to go to position in variable 'pos'
  //   delay(15);                      // waits 15ms for the servo to reach the position
  // }

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
    if (currentTime_T2 - prevTime_T2 >= 20000) {
      MoveCondition(data);
    }
  }
#endif
}

void robotMove(int sppeedLeft, int speedRight) {
  motor1.setSpeed(sppeedLeft);
  motor2.setSpeed(speedRight);
}

void MoveCondition(char data) {
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

void automaticsolartracker() {
  top_r = ADS.readADC(ldrRightTopChannel);  //top right
  bot_r = ADS.readADC(ldrRightBotChannel);  //bot right
  top_l = ADS.readADC(ldrLeftTopChannel);   //top left
  bot_l = ADS.readADC(ldrLeftBotChannel);   //bot left

  avgtop = (top_r + top_l) / 2;
  avgbot = (bot_r + bot_l) / 2;
  avgleft = (top_l + bot_l) / 2;
  avgright = (top_r + bot_r) / 2;

  diffelev = avgtop - avgbot;
  diffazi = avgright - avgleft;

  if (abs(diffazi) >= THRESHOLD_VALUE) {
    if (abs(totalSteps) >= 2038 * 100 / 360) {
      myStepper.setSpeed(0);  // Stop the stepper motor
      totalSteps = 0;         // Reset the totalSteps variable
    } else {
      if (servo_updown.read() < 75) {
        if (diffazi < 0) {
          myStepper.setSpeed(10);
          myStepper.step(-100);
        } else if (diffazi > 0) {
          myStepper.setSpeed(10);
          myStepper.step(100);
        }
      } else {
        if (diffazi > 0) {
          myStepper.setSpeed(10);
          myStepper.step(-100);
        } else if (diffazi < 0) {
          myStepper.setSpeed(10);
          myStepper.step(100);
        }
      }
    }
  }

  if (abs(diffelev) >= THRESHOLD_VALUE) {
    if (diffelev > 0) {
      Serial.println(servo_updown.read());
      if (servo_updown.read() < SERVO_MAX) {
        servo_updown.write((servo_updown.read() + 2));
        delay(SERVO_SMOOTH);
      }
    }
    if (diffelev < 0) {
      Serial.println(servo_updown.read());
      if (servo_updown.read() > SERVO_MIN) {
        servo_updown.write((servo_updown.read() - 2));
        delay(SERVO_SMOOTH);
      }
    }
  }
}
