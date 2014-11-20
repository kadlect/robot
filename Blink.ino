#include <Servo.h>
#include <Serial.h>
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
    servo_left.detach();
    servo_left.attach(SERVO_LEFT_PIN);
    servo_left.write(180);
  } else if (d < 0) {
    servo_left.detach();
    servo_left.attach(SERVO_LEFT_PIN);
    servo_left.write(0);
  } else {
    servo_left.detach();
  }
}

void right_servo_run(int d) {
  if(d > 0) {
    servo_right.detach();
    servo_right.attach(SERVO_RIGHT_PIN);
    servo_right.write(0);
  } else if (d < 0) {
     servo_right.detach();
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
  
  Serial.begin(9600);
  
  left_servo_run(1);
  right_servo_run(1);
}



int state[2];

void loop() {
  int sensor[3];
  sensor[0] = digitalRead(SENSOR_LEFT_PIN);
  sensor[1] = digitalRead(SENSOR_CENTER_PIN);
  sensor[2] = digitalRead(SENSOR_RIGHT_PIN);
  
  Serial.print(sensor[0]);
  Serial.print(sensor[1]);
  Serial.println(sensor[2]);

  
  state[0] = state[1];
  state[1] = (sensor[0] << 2) | (sensor[1] << 1) | (sensor[2] << 0);
  //int change = (~state[1]) & (state[0]);
  
  if(state[0] != state[1]) {
    if(0b00000101 == state[1]) {
      left_servo_run(1);
      right_servo_run(1);
    } else if(0b00000100 == state[1] || 0b00000110 == state[1] ) {
      // turn right
      left_servo_run(1);
      right_servo_run(-1);
    } else if(0b00000001 == state[1] || 0b00000011 == state[1]) {
      // turn left
      left_servo_run(-1);
      right_servo_run(1);
    }
  }
}
