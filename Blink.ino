#include <Servo.h>
#include <Serial.h>
#
#
#define SERVO_LEFT_PIN 13
#define SERVO_RIGHT_PIN 12
#
#define SENSOR_LL_PIN 8
#define SENSOR_LEFT_PIN 11
#define SENSOR_CENTER_PIN 9
#define SENSOR_RIGHT_PIN 10
#define SENSOR_RR_PIN 7

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
     servo_right.attach(SERVO_RIGHT_PIN);
     servo_right.write(180);
  } else {
    servo_right.detach();
  }
}



void setup() {
  pinMode(SENSOR_LL_PIN, INPUT);
  pinMode(SENSOR_LEFT_PIN, INPUT);
  pinMode(SENSOR_CENTER_PIN, INPUT);
  pinMode(SENSOR_RIGHT_PIN, INPUT);
  pinMode(SENSOR_RR_PIN, INPUT);
  
  Serial.begin(9600);
  
  left_servo_run(1);
  right_servo_run(1);
}



int state[2];

void loop() {
  int sensor[3];
  sensor[0] = digitalRead(SENSOR_LL_PIN);
  sensor[1] = digitalRead(SENSOR_LEFT_PIN);
  sensor[2] = digitalRead(SENSOR_CENTER_PIN);
  sensor[3] = digitalRead(SENSOR_RIGHT_PIN);
  sensor[4] = digitalRead(SENSOR_RR_PIN);
  //
  // Cerna pohlcuje ---> 0
  // Bila odrazi    ---> 1
  //
  // Je nutno provest krekci...
  
  
 
  state[0] = state[1];
  state[1] = (sensor[0] << 4) | (sensor[1] << 3) | (sensor[2] << 2 | sensor[3] << 1 | sensor[4]);
  
  state[1] = 0b00011111^state[1];

  Serial.print(sensor[0]);
  Serial.print(sensor[1]);
    Serial.print(sensor[2]);
      Serial.print(sensor[3]);
  Serial.println(sensor[4]);
  //Serial.println(state[1]);
  
  if(state[0] != state[1]) {
    if(0b00001010 == state[1]) {
      left_servo_run(1);
      right_servo_run(1);
    } else if(0b00001000 == state[1] || 0b00001100 == state[1] ) {
      // turn right
      left_servo_run(1);
      right_servo_run(-1);
    } else if(0b00000010 == state[1] || 0b00000110 == state[1]) {
      // turn left
      left_servo_run(-1);
      right_servo_run(1);
    }
  }
}
