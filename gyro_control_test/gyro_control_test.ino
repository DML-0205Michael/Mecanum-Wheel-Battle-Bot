#include <MPU6050.h>
#include <I2Cdev.h>
#include <Wire.h>

MPU6050 accelgyro;
int16_t gx, gy, gz;
float wz_error;
const int num_of_loop;
float KP, KI, KD; // of yaw
int yaw; // int, -255 ~ 255
float yaw_speed; 
float error_sum;
float error_previous;
const float ss_error_range=10; // deg/s
const float i_limit=200; // PWM
const float tot_limit=255; // PWM



void setup() {
  Wire.begin();
  Serial.begin(115200);

  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100);

  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100);

  calculate_gyro_error();
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100);
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100);
  delay(300);
}

void loop() {
  accelgyro.getRotation(&gx, &gy, &gz);
  float wz=float(gz)/131-wz_error;
}

void PID(float target_value, float actual_value){
  
  float error_current=target_value-actual_value; // current error 

  p_out=error_current*KP; 

  if ((target_value==0) && (pitch!=0)){ // prevent large error sum before take off
    //if ((actual_PID > integral_limit) || (actual_PID < -integral_limit)){ // if not reaching steady state/still have large overshoot
    if (error_current < ss_error_range){ // if within ss error
      i_out=i_out+error_current*KI;
      if (i_out<-i_limit) {
        i_out=-i_limit;
      } else if (i_out>i_limit){
        i_out=i_limit;
      }
    } else {
      i_out=0;
    } 
  } else {i_out=0;}

  d_out=KD*(error_current-error_previous);

  // output = KP*  current err + KI_flag*KI     * error sum   +KD       *(current err - previous err)
  //PID_output= KP*error_current + KI_flag*KI     *error_sum    +KD       *(error_current-error_previous);
  tot_out=p_out+i_out+d_out;
  if (tot_out<-tot_limit) {
    tot_out=-tot_limit;
  } else if (tot_out>tot_limit){
    tot_out=tot_limit;
  }

  error_previous=error_current; // previous = current
}

void calculate_gyro_error(){
  int i=0;
  while (i<num_of_loop){
    accelgyro.getRotation(&gx, &gy, &gz);
    float wz=float(gz)/131;
    wz_error=wz_error+wz;
    i++;
  }
  wz_error=wz_error/num_of_loop;
}