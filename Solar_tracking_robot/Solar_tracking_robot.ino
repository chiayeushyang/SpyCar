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

#define SERVOPIN 10

#define trigPin A3
#define echoPin A2

#define Bluetooth
#define Wifi

ADS1115 ADS(0x48);

CytronMD motor1(PWM_PWM, 11, 3); //M1A, M1B
CytronMD motor2(PWM_PWM, 6, 5); //M2A, M2B

SoftwareSerial BTSerial(9, 4);  // RX, TX

char data = "S";

int16_t top_r;
int16_t top_l;
int16_t bot_r;
int16_t bot_l;

unsigned int avg;
unsigned int avgtop;
unsigned int avgbot;
unsigned int avgleft;
unsigned int avgright;

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

long duration;
int distance;

bool car_move = false;

Stepper myStepper = Stepper(stepsPerRevolution, motor1Pin1, motor1Pin3, motor1Pin2, motor1Pin4);
Servo servo_updown;

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  BTSerial.begin(9600);
  myStepper.setSpeed(10);
  servo_updown.attach(SERVOPIN);
  ADS.begin();
}

void loop() {
  unsigned long currentTime_T3 = millis();
  if (currentTime_T3 - prevTime_T2 >= 10000) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);

    distance = duration * 0.034 / 2;

    // if (distance < 50) {
    //   robotMove(100, 100);
    //   car_move = true;
    // } else if(distance < 70) {
    //   robotMove(0, 0);
    //   car_move = false;
    // } else if(distance < 120) {
    //   robotMove(-100, -100);
    //   car_move = true;
    // }

    if (car_move == false) {
      automaticsolartracker();
    }
  }

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
    case '|':
      robotMove(0, 0);
      car_move = false;
      break;
    case '^':
      robotMove(-230, -230);
      car_move = true;
      break;
    case '!':
      robotMove(230, 230);
      car_move = true;
      break;
    case '<':
      robotMove(120, -120);
      car_move = true;
      break;
    case '>':
      robotMove(-120, 120);
      car_move = true;
      break;
    case '[':
      robotMove(80, -80);
      car_move = true;
      break;
    case ']':
      robotMove(-80, 80);
      car_move = true;
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

  avg = (top_r + bot_r + top_l + bot_l) / 4;

  diffelev = avgtop - avgbot;
  diffazi = avgright - avgleft;

  if (avg > 2000) {
    if (abs(diffazi) >= THRESHOLD_VALUE) {
      if (servo_updown.read() < 75) {
        if (diffazi < 0) {
          myStepper.setSpeed(10);
          myStepper.step(-10);
          totalSteps -= 10;
        } else if (diffazi > 0) {
          myStepper.setSpeed(10);
          myStepper.step(10);
          totalSteps += 10;
        }
      } else {
        if (diffazi > 0) {
          myStepper.setSpeed(10);
          myStepper.step(-10);
          totalSteps -= 10;
        } else if (diffazi < 0) {
          myStepper.setSpeed(10);
          myStepper.step(10);
          totalSteps += 10;
        }
      }
    }

    if (abs(diffelev) >= THRESHOLD_VALUE) {
      if (diffelev > 0) {
        if (servo_updown.read() < SERVO_MAX) {
          servo_updown.write((servo_updown.read() + 2));
          delay(SERVO_SMOOTH);
        }
      }
      if (diffelev < 0) {
        if (servo_updown.read() > SERVO_MIN) {
          servo_updown.write((servo_updown.read() - 2));
          delay(SERVO_SMOOTH);
        }
      }
    }
  }
}
