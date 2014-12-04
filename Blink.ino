#include <Servo.h>
#include <Serial.h>
#
#define NULL_MASK 255
#
#define RR_SENSOR 0b00000001
#define R_SENSOR   0b00000010
#define C_SENSOR   0b00000100
#define L_SENSOR   0b00001000
#define LL_SENSOR 0b00010000
#
#define SERVO_LEFT_PIN 13
#define SERVO_RIGHT_PIN 12
#
#define SENSOR_LL_PIN 7
#define SENSOR_LEFT_PIN 6
#define SENSOR_CENTER_PIN 5
#define SENSOR_RIGHT_PIN 4
#define SENSOR_RR_PIN 3
#
#define NORMAL_STATE 0
#define PREPARE_LEFT_BRANCH 1
#define PREPARE_RIGHT_BRANCH 2
#define RIGHT_BRANCH 3
#define LEFT_BRANCH 4
#define EXITED_LEFT_BRANCH 5
#define EXITED_RIGHT_BRANCH 6

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



int sensors;
int state;
int mask = 0b11111111;
int mask_pos = 0b00000000;
unsigned long timestamp;

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
  
  
 
  sensors = (sensor[0] << 4) | (sensor[1] << 3) | (sensor[2] << 2 | sensor[3] << 1 | sensor[4]);
  sensors = 0b00011111^sensors;

  Serial.print(!sensor[0]);
  Serial.print(!sensor[1]);
  Serial.print(!sensor[2]);
  Serial.print(!sensor[3]);
  Serial.print(!sensor[4]);
  //Serial.println(state[1]);
  Serial.print(7);
  Serial.print(mask);
//  Serial.println(state[1]);

//Print surrent state
if (state == NORMAL_STATE){
Serial.print("NORMAL_STATE");
}
if (state == PREPARE_RIGHT_BRANCH){
Serial.print("PREPARE_RIGHT_BRANCH");
}
if (state == PREPARE_LEFT_BRANCH){
Serial.print("PREPARE_LEFT_BRANCH");
}
if (state == RIGHT_BRANCH){
Serial.print("RIGHT_BRANCH");
}
if (state == LEFT_BRANCH){
Serial.print("LEFT_BRANCH");
}
if (state == EXITED_LEFT_BRANCH){
Serial.print("EXITED_LEFT_BRANCH");
}
if (state == EXITED_RIGHT_BRANCH){
Serial.print("EXITED_RIGHT_BRANCH");
}
   
  //
  // Jedeme do leva
  //
if ((LL_SENSOR | C_SENSOR) == sensors) {
    if (mask == NULL_MASK) {
      mask = 0b11111101;
      state = PREPARE_LEFT_BRANCH;
    }
  }
  
  //
  // Jedeme do prava
  //
  if (RR_SENSOR & sensors) {
    if (mask != NULL_MASK) {
      mask = 0b11111111;
    }
    if (state == PREPARE_LEFT_BRANCH) {
       state = LEFT_BRANCH;
       timestamp = millis();   
    }
    if(state == LEFT_BRANCH && ( millis() > (timestamp + 700)) ) {
        mask = 0b11111100;
        state  = EXITED_LEFT_BRANCH;
    }
  }
  
  if (state == EXITED_LEFT_BRANCH || state == EXITED_RIGHT_BRANCH) {
    if (sensors == 0) {
      
      // kdyz nic nevidi => zachovej se podle predchoziho branche
      if (state == EXITED_LEFT_BRANCH) {
        mask_pos = R_SENSOR;
      } else {
        mask_pos = L_SENSOR;
      }
      
      
    }
  }
  
  if (C_SENSOR == sensors && (state == EXITED_LEFT_BRANCH || state == EXITED_RIGHT_BRANCH)) {
      mask_pos = 0b0000000;
  }
  
  
  //
  // Jedeme po care bez odbocek
  //
  sensors = mask & sensors;
  sensors = mask_pos | sensors;
  
  if(C_SENSOR == sensors) {
       left_servo_run(1);
       right_servo_run(1);
  } else if( ( R_SENSOR == sensors ||
                (R_SENSOR | C_SENSOR) == sensors ) &&
                 state != LEFT_BRANCH) {
              Serial.print("RIGHT");
        // turn right
        left_servo_run(1);
        right_servo_run(-1);    
    } else if(L_SENSOR == sensors ||
                  (L_SENSOR | C_SENSOR) == sensors ) {
        Serial.print("LEDFT");
        // turn left
        left_servo_run(-1);
        right_servo_run(1);    
    }
  Serial.println();
}
