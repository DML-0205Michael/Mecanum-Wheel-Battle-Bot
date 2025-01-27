// https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
// https://examples.blynk.cc/?board=ESP32&shield=ESP32%20WiFi&example=GettingStarted%2FBlynkSimpleDemo

#define BLYNK_TEMPLATE_ID "TMPL2zD8rsyAP"
#define BLYNK_TEMPLATE_NAME "Mini Car"
#define BLYNK_AUTH_TOKEN "h-HIrPRaHcAuGcKCTKIqMjYyVgvvT51n"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

#define M1A 25
#define M1B 26
#define M1P 27

#define M2A 19
#define M2B 18
#define M2P 15

#define M3A 17
#define M3B 16
#define M3P 4

#define M4A 14
#define M4B 12
#define M4P 13

int M_pin[12]={M1A,M1B,M1P,M2A,M2B,M2P,M3A,M3B,M3P,M4A,M4B,M4P};
int Speed[4]={0,0,0,0};
int buffer[4]={0,0,0,0};
int roll;
int pitch;
int yaw;

#define S1_pin 32
#define S2_pin 21
Servo S1; // left
Servo S2; // right
int angle;
#define S1_max 120;
#define S1_min 80;
#define S2_max 120;
#define S2_min 80;

#define V1_pin 36
#define V_tot_pin 39
#define R1 10 // Mohms
#define R2 1 // Mohms
#define voltage_constant 3.3/4095*(R1+R2)/R2
// Vb=3.3x/4095*(R1+R2)/R2=voltage_constant*x
float Vb1;
float Vb2;
float Vb_tot;

void setup() {
  Blynk.begin(BLYNK_AUTH_TOKEN, "DMLINS16", "ldmzuishuai");
  for (int i=1;1<12;i++){
    pinMode(M_pin[i],OUTPUT);
  }
  S1.setPeriodHertz(50);
  S2.setPeriodHertz(50);
  S1.attach(S1_pin);
}

void loop() {
  Blynk.run();
  speed_calculation();
  motor_output();
  //voltage_monitor();

}

void speed_calculation(){
  buffer[1]=pitch+roll+yaw;
  buffer[2]=pitch-roll-yaw;
  buffer[3]=pitch+roll-yaw;
  buffer[4]=pitch-roll+yaw;
  
  int buffer_max=0;
  for (int i=1;i<4;i++){
    if (buffer[i]>=buffer_max){
      buffer_max=buffer[i];
    }
  }

  if (buffer_max>255){
    float proportion=255/buffer_max;
    Speed[1]=buffer[1]*proportion;
    Speed[2]=-buffer[2]*proportion;
    Speed[3]=buffer[3]*proportion;
    Speed[4]=-buffer[4]*proportion;
  } else {
    Speed[1]=buffer[1];
    Speed[2]=-buffer[2];
    Speed[3]=buffer[3];
    Speed[4]=-buffer[4];
  }
}

void motor_output(){
  if (Speed[1]>=0){
    digitalWrite(M1A, HIGH);
    digitalWrite(M1B, LOW);
    analogWrite(M1P, abs(Speed[1]));
  } else if  (Speed[1]<0) {
    digitalWrite(M1B, HIGH);
    digitalWrite(M1A, LOW);
    analogWrite(M1P, abs(Speed[1]));
  }

  if (Speed[2]>=0){
    digitalWrite(M2A, HIGH);
    digitalWrite(M2B, LOW);
    analogWrite(M1P, abs(Speed[2]));
  } else if  (Speed[2]<0) {
    digitalWrite(M2B, HIGH);
    digitalWrite(M2A, LOW);
    analogWrite(M2P, abs(Speed[2]));
  }

 if (Speed[3]>=0){
    digitalWrite(M3A, HIGH);
    digitalWrite(M3B, LOW);
    analogWrite(M3P, abs(Speed[3]));
  } else if  (Speed[3]<0) {
    digitalWrite(M3B, HIGH);
    digitalWrite(M3A, LOW);
    analogWrite(M3P, abs(Speed[3]));
  }

  if (Speed[4]>=0){
    digitalWrite(M4A, HIGH);
    digitalWrite(M4B, LOW);
    analogWrite(M4P, abs(Speed[4]));
  } else if  (Speed[4]<0) {
    digitalWrite(M4B, HIGH);
    digitalWrite(M4A, LOW);
    analogWrite(M4P, abs(Speed[4]));
  }

  int S1_angle=S1_min+angle;
  int S2_angle=S2_max-angle;
  if (S1_angle>S1_max){
    S1_angle=S1_max;
  } else if (S1_angle<S1_min){
    S1_angle=S1_min;
  }
  if (S2_angle>S2_max){
    S2_angle=S2_max;
  } else if (S2_angle<S2_min){
    S2_angle=S2_min;
  }
  S1.write(S1_angle);
  S2.write(S2_angle);
}

void voltage_monitor(){
  int x1=analogRead(V1_pin);
  Vb1=voltage_constant*x1;

  int x_tot=analogRead(V_tot_pin);
  Vb_tot=voltage_constant*x_tot;

  Vb2=Vb_tot-Vb1;

  Blynk.virtualWrite(V4, Vb1);
  Blynk.virtualWrite(V5, Vb2);
  Blynk.virtualWrite(V6, Vb_tot);

}

BLYNK_WRITE(V0){
  roll= param.asInt();
}
BLYNK_WRITE(V1){
  pitch= param.asInt();
}
BLYNK_WRITE(V2){
  yaw= param.asInt();
}
BLYNK_WRITE(V3){
  angle= param.asInt();
}