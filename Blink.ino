#include <Servo.h>
#
#
#define SERVO_LEFT_PIN 13
#define SERVO_RIGHT_PIN 12
#
#define SENSOR_LEFT_PIN 6
#define SENSOR_CENTER_PIN 5
#define SENSOR_RIGHT_PIN 4

Servo servo_left;
Servo servo_right;



void left_servo_run(int d) {
  if(d > 0) {
    servo_left.attach(SERVO_LEFT_PIN);
    servo_left.write(180);
  } else if (d < 0) {
    servo_left.attach(SERVO_LEFT_PIN);
    servo_left.write(0);
  } else {
    servo_left.detach();
  }
}

void right_servo_run(int d) {
  if(d > 0) {
    servo_right.attach(SERVO_RIGHT_PIN);
    servo_right.write(0);
  } else if (d < 0) {
     servo_left.attach(SERVO_RIGHT_PIN);
     servo_left.write(180);
  } else {
    servo_right.detach();
  }
}



void setup() {
  pinMode(SENSOR_LEFT_PIN, INPUT);
  pinMode(SENSOR_CENTER_PIN, INPUT);
  pinMode(SENSOR_RIGHT_PIN, INPUT);
  
  left_servo_run(1);
  right_servo_run(1);
}



int state[2];

void loop() {
  int sensor[3];
  sensor[0] = digitalRead(SENSOR_LEFT_PIN);
  sensor[1] = digitalRead(SENSOR_CENTER_PIN);
  sensor[2] = digitalRead(SENSOR_RIGHT_PIN);
  
  state[0] = state[1];
  state[1] = (sensor[0] << 0) | (sensor[1] << 1) | (sensor[2] << 2);
  //int change = (~state[1]) & (state[0]);
  
 // if(state[0] != state[1]) {
    if(0b00000010 == state[1]) {
      left_servo_run(0);
      right_servo_run(0);
      delay(1000);
      left_servo_run(1);
      right_servo_run(1);
      delay(1000);
    } else if(0b00000011 == state[1]) {
      left_servo_run(1);
      right_servo_run(0);
    } else if(0b00000110 == state[1]) {
      left_servo_run(0);
      right_servo_run(1);
    }
 // }
  
  delay(10);
}
