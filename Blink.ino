#include <Servo.h>
#include <SoftwareSerial.h>
#
#define NULL_MASK 255
#
#define RR_SENSOR 0b00000001
#define R_SENSOR   0b00000010
#define C_SENSOR   0b00000100
#define L_SENSOR   0b00001000
#define LL_SENSOR 0b00010000
#define LL_L_C_SENSOR 0b00011100
#define RR_R_C_SENSOR 0b00000111
#define LL_C_RR_SENSOR 0b00010101
#define L_C_R_SENSOR 0b00001110
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
#define START 0
#define NORMAL_STATE 1
#define PREPARE_BRANCH 2
#define BRANCH 3
#define EXITING_BRANCH 4
#define STOPPED 5
#
#define LEFT_BRANCH 0
#define RIGHT_BRANCH 1
#
#define ARCHIVE_SIZE 6

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


int archived_sensors[ARCHIVE_SIZE] = {0};
int sensors;
int state;
int branch;
int mask = 0b11111111;
int sensor[5];
int history;

void loop() {
  sensor[0] = digitalRead(SENSOR_LL_PIN);
  sensor[1] = digitalRead(SENSOR_LEFT_PIN);
  sensor[2] = digitalRead(SENSOR_CENTER_PIN);
  sensor[3] = digitalRead(SENSOR_RIGHT_PIN);
  sensor[4] = digitalRead(SENSOR_RR_PIN);

  //
  // Cerna pohlcuje ---> 0
  // Bila odrazi    ---> 1
  //
  // Je nutno provest korekci...
  
  sensors = (sensor[0] << 4) | (sensor[1] << 3) | (sensor[2] << 2 | sensor[3] << 1 | sensor[4]);
  sensors = 0b00011111^sensors;

  shift(archived_sensors);
  archived_sensors[ARCHIVE_SIZE-1] = sensors;
  history = get_sensors_history();

  Serial.print(!sensor[0]);
  Serial.print(!sensor[1]);
  Serial.print(!sensor[2]);
  Serial.print(!sensor[3]);
  Serial.print(!sensor[4]);
  //Serial.println(state[1]);
  Serial.print(" ");
  for (int i = 4; i >= 0; --i) {
   Serial.print((mask >> i) & 1); 
  }
  Serial.print(" ");
//  Serial.println(state[1]);
   
   if (state == START) {
    if (sensors == C_SENSOR){
     state = NORMAL_STATE;
    } 
   }
   
   if (state == NORMAL_STATE) {
     Serial.print("NORMAL_STATE");
    if ( (sensors & 0b00001110) == C_SENSOR) {
      check_sides();
    }
   }
   if (state == PREPARE_BRANCH) {
     Serial.print("PREPARE_BRANCH");
     Serial.print(sensors & R_SENSOR != 0);
     if ( (branch == LEFT_BRANCH) && (history & R_SENSOR) ) {
       state = BRANCH;
     }
     if ( (branch == RIGHT_BRANCH) && (history & L_SENSOR) ) {
       state = BRANCH;
     }
   }
   
   if ( (state == BRANCH)) {
     Serial.print("BRANCH");
     // we just entered the branch (passed the junction)
     if (sensors == C_SENSOR) {
       mask = NULL_MASK;
     }
     // we get into closing junction (end of branch)
     
   if (mask == NULL_MASK) {
     check_for_closing_junction();
   }
     
   }
   
   if (state == EXITING_BRANCH) {
     Serial.print("EXITING_BRANCH");
     // exiting closing junction
     if (sensors == C_SENSOR) {
       close_branch();
     } 
   }
   
   if (history == LL_C_RR_SENSOR) {
     state = STOPPED;
     stop_servos();
   }
    
  if (state != NORMAL_STATE) {
    Serial.print(" ");
   if (branch == LEFT_BRANCH) {
    Serial.print("L");
   } else {
    Serial.print("R");
   }
  }
  Serial.print(" = ");
  for (int i = 4; i >= 0; --i) {
   Serial.print((history >> i) & 1); 
  }
  Serial.print(" = ");
   sensors = sensors & mask;
   follow_line(); 
  Serial.println(); 
}

void check_for_closing_junction() {
  Serial.print("*");
  Serial.print(sensors == RR_R_C_SENSOR);
  Serial.print("*");
  Serial.print(branch == LEFT_BRANCH);
  Serial.print("*");
  if ( (sensors & RR_R_C_SENSOR == RR_R_C_SENSOR) && (branch == LEFT_BRANCH) ) {
    mask = 0b00011101; 
    state = EXITING_BRANCH;
  } else if ( (sensors & LL_L_C_SENSOR == LL_L_C_SENSOR) && (branch == RIGHT_BRANCH) ) {
    mask = 0b00010111; 
    state = EXITING_BRANCH;
  }
}

void close_branch() {
   mask = NULL_MASK;
   state = NORMAL_STATE;
}

void check_sides() {
  if (sensors & (LL_SENSOR | RR_SENSOR) ) {
    state = PREPARE_BRANCH; 
    // determine which branch will follow
    if (sensors & LL_SENSOR) {
      branch = LEFT_BRANCH;
      mask = 0b00011101;
    } else {
      branch = RIGHT_BRANCH;
      mask = 0b00010111;
    }
    
  }
}

void follow_line() {
    
  Serial.print("\t\t\t GO  ->  "); 

  if (state == STOPPED){
   Serial.print("STOPPED");
    return;
  }
  
  if ((history & L_C_R_SENSOR) == L_C_R_SENSOR) {
   go(1,1);
   Serial.print("STRAIGHT"); 
   return;
  }
  Serial.print(sensors);
  /*if (sensors == 0) {
   if (branch == LEFT_BRANCH){
     Serial.print("RIGHT");
    go(1,-1);
   } else {
     Serial.print("RIGHT");
    go(-1,1);
   } 
  }*/
 
  if (sensors & L_SENSOR) {
    Serial.print("LEFT");
    go(-1,1);
  } else
  if(sensors & R_SENSOR) {
    Serial.print("RIGHT");
    go(1,-1);
  } else
  if (sensors & C_SENSOR) { 
    Serial.print("STRAIGHT");
    go(1,1);
  }
}

void go(int left_speed, int right_speed) {
  left_servo_run(left_speed);
  right_servo_run(right_speed);
}

 void stop_servos() {
  left_servo_run(0);
  right_servo_run(0); 
 }
 
 void shift(int *arr) {
  for (int i = 0; i < ARCHIVE_SIZE-1; ++i) {
   arr[i] = arr[i+1];
  } 
 }
 
 int get_sensors_history() {
   int buf;
   for (int i =0; i < ARCHIVE_SIZE; ++i) {
    buf |= archived_sensors[i]; 
   }
   return buf;
 }
