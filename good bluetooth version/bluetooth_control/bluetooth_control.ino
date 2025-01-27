#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

//#include <WiFi.h>
//#include <WiFiClient.h>

#include <ESP32Servo.h>
////////////////////////////////////////// Motors //////////////////////////////////////////
#define M1A 26
#define M1B 25
#define M1P 33

#define M2A 19
#define M2B 18
#define M2P 21

#define M3A 5
#define M3B 17
#define M3P 16

#define M4A 27
#define M4B 14
#define M4P 12

int M_pin[12]={M1A,M1B,M1P,M2A,M2B,M2P,M3A,M3B,M3P,M4A,M4B,M4P};
int Speed[4]={0,0,0,0};
int buffer[4]={0,0,0,0};
float roll;
float pitch;
float yaw;
int yaw_sen=80;
int pitch_roll_sen;

#define S1_pin 32
#define S2_pin 23
Servo S1; // left
Servo S2; // right
int angle_perc;
const float S1_max=126;
const float S1_min=46;
const float S1_range=S1_max-S1_min;
const float S2_max=142;
const float S2_min=60;
const float S2_range=S2_max-S2_min;
////////////////////////////////////////// Motors //////////////////////////////////////////
////////////////////////////////////////// Voltage Monitor //////////////////////////////////////////
#define V1_pin 36
#define V_tot_pin 39
#define R1 10 // Mohms
#define R2 1 // Mohms
#define voltage_constant 3.3/4095*(R1+R2)/R2
// Vb=3.3x/4095*(R1+R2)/R2=voltage_constant*x
float Vb1;
float Vb2;
float Vb_tot;

const float Vb1_max=8.22;
const float Vb1_min=7.5;
const float Vb1_range=Vb1_max-Vb1_min;

const float Vb2_max=8.22;
const float Vb2_min=7.5;
const float Vb2_range=Vb1_max-Vb1_min;

float Vb_tot_max=Vb1_max+Vb2_max;
float Vb_tot_min=Vb1_min+Vb2_min;
float Vb_tot_range=Vb_tot_max-Vb_tot_min;
////////////////////////////////////////// Voltage Monitor //////////////////////////////////////////
////////////////////////////////////////// Bluetooth //////////////////////////////////////////
int update_interval=100; // time interval in ms for updating panel indicators
unsigned long last_time=0; // time of last update
char data_in; // data received from serial link
//String text; // String for text elements
int progress; // Progress Bar Value
////////////////////////////////////////// Bluetooth //////////////////////////////////////////
void setup() {
  pinMode(2,OUTPUT);
  Serial.begin(115200);
  SerialBT.begin(115200);
  build_panel();

  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  delay(400);

  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  delay(400);
  // for (int i=0;1<12;i++){
  //   pinMode(M_pin[i],OUTPUT);
  //   Serial.println(M_pin[i]);
  //   digitalWrite(2,HIGH); delay(1000); digitalWrite(2,LOW); delay(20);
  // }
  pinMode(M1A,OUTPUT); pinMode(M1B,OUTPUT); pinMode(M1P,OUTPUT); Serial.println("M1 done.");
  pinMode(M2A,OUTPUT); pinMode(M2B,OUTPUT); pinMode(M2P,OUTPUT); Serial.println("M2 done.");
  pinMode(M3A,OUTPUT); pinMode(M3B,OUTPUT); pinMode(M3P,OUTPUT); Serial.println("M3 done.");
  pinMode(M4A,OUTPUT); pinMode(M4B,OUTPUT); pinMode(M4P,OUTPUT); Serial.println("M4 done.");

  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  delay(400);

  Serial.println("start servo");
  // digitalWrite(2,HIGH); delay(200); digitalWrite(2,LOW); delay(200); 
  // digitalWrite(2,HIGH); delay(200); digitalWrite(2,LOW); delay(200); 
  // delay(1000);

  S1.setPeriodHertz(50);
  S2.setPeriodHertz(50);
  S1.attach(S1_pin);
  S2.attach(S2_pin);

  Serial.println("servo attach done.");

  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100);  
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  digitalWrite(2,HIGH); delay(100); digitalWrite(2,LOW); delay(100); 
  delay(400);
}

void loop() {
  receive_data();
  speed_calculation();
  motor_output();
  voltage_monitor();
  send_data();

  Serial.println();
}

void speed_calculation(){
  if (roll>-50 && roll<50 && pitch!=0){
    roll=0;
  } else if (pitch>-50 && pitch<50 && roll!=0){
    pitch=0;
  }
  // Serial.print(float(pitch_roll_sen)/100);Serial.print("\t");
  // Serial.print(float(yaw_sen)/100);Serial.print("\t");
  float mpitch=pitch*float(pitch_roll_sen)/100;
  float mroll=roll*float(pitch_roll_sen)/100;
  float myaw=yaw*float(yaw_sen)/100;
  // Serial.print(mroll);Serial.print("\t");
  // Serial.print(mpitch);Serial.print("\t");
  // Serial.print(myaw);Serial.print("\t");
  buffer[0]=mpitch+mroll+myaw;
  buffer[1]=mpitch-mroll-myaw;
  buffer[2]=mpitch+mroll-myaw;
  buffer[3]=mpitch-mroll+myaw;
  // Serial.print(buffer[0]);Serial.print("\t");
  // Serial.print(buffer[1]);Serial.print("\t");
  // Serial.print(buffer[2]);Serial.print("\t");
  // Serial.print(buffer[3]);Serial.print("\t");

  int buffer_max=0;
  for (int i=0;i<3;i++){
    if (abs(buffer[i])>=abs(buffer[i+1])){
      buffer_max=buffer[i];
    }
  }
  //Serial.print(buffer_max);Serial.print("\t");

  if (abs(buffer_max)>255){
    float proportion=abs(255/float(buffer_max));
    //Serial.print(proportion);Serial.print("\t");
    Speed[0]=buffer[0]*proportion;
    Speed[1]=-buffer[1]*proportion;
    Speed[2]=buffer[2]*proportion;
    Speed[3]=-buffer[3]*proportion;
    
  } else {
    Speed[0]=buffer[0];
    Speed[1]=-buffer[1];
    Speed[2]=buffer[2];
    Speed[3]=-buffer[3];
    //Serial.print("prop NA");Serial.print("\t");
  }
  
  Serial.print(Speed[0]);Serial.print("\t");
  Serial.print(Speed[1]);Serial.print("\t");
  Serial.print(Speed[2]);Serial.print("\t");
  Serial.print(Speed[3]);Serial.print("\t");
  Serial.print("|\t");
  
}

void motor_output(){

  if (Speed[0]>=0){
    digitalWrite(M1A, HIGH);
    digitalWrite(M1B, LOW);
    analogWrite(M1P, abs(Speed[0]));
  } else if  (Speed[0]<0) {
    digitalWrite(M1B, HIGH);
    digitalWrite(M1A, LOW);
    analogWrite(M1P, abs(Speed[0]));
  }

  if (Speed[1]>=0){
    digitalWrite(M2A, HIGH);
    digitalWrite(M2B, LOW);
    analogWrite(M2P, abs(Speed[1]));
  } else if  (Speed[1]<0) {
    digitalWrite(M2B, HIGH);
    digitalWrite(M2A, LOW);
    analogWrite(M2P, abs(Speed[1]));
  }

  if (Speed[2]>=0){
    digitalWrite(M3A, LOW);
    digitalWrite(M3B, HIGH);
    analogWrite(M3P, abs(Speed[2]));
  } else if  (Speed[2]<0) {
    digitalWrite(M3B, LOW);
    digitalWrite(M3A, HIGH);
    analogWrite(M3P, abs(Speed[2]));
  }

 if (Speed[3]>=0){
    digitalWrite(M4A, HIGH);
    digitalWrite(M4B, LOW);
    analogWrite(M4P, abs(Speed[3]));
  } else if  (Speed[3]<0) {
    digitalWrite(M4B, HIGH);
    digitalWrite(M4A, LOW);
    analogWrite(M4P, abs(Speed[3]));
  }

  int S1_angle=S1_min+float(angle_perc)/100*S1_range;
  int S2_angle=S2_max-float(angle_perc)/100*S2_range;
  
  if (S1_angle > S1_max){
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
  //if (angle >20)digitalWrite(2,HIGH); else digitalWrite(2,LOW);
  // Serial.print(angle_perc);Serial.print("\t");
  // Serial.print(S1_angle);Serial.print("\t");
  // Serial.print(S2_angle);Serial.print("\t");
}

void voltage_monitor(){
  int x1=analogRead(V1_pin);
  Vb1=voltage_constant*x1;

  int x_tot=analogRead(V_tot_pin);
  Vb_tot=voltage_constant*x_tot;

  Vb2=Vb_tot-Vb1;
}


void build_panel(){
  // Build panel in app
  SerialBT.println("*.kwl");
  SerialBT.println("clear_panel()");
  SerialBT.println("set_grid_size(12,9)");
  SerialBT.println("add_text_box(5,0,2,L,Text,245,240,245,H)");

  SerialBT.println("add_text_box(10,3,2,L,Text,245,240,245,)");
  SerialBT.println("add_text_box(4,3,2,L,Text,245,240,245,)");
  SerialBT.println("add_text_box(6,3,2,L,Text,245,240,245,)");
  SerialBT.println("add_text_box(0,3,2,L,Text,245,240,245,)");
  SerialBT.println("add_text_box(2,0,2,L,Text,245,240,245,)");
  SerialBT.println("add_text_box(8,0,2,L,Text,245,240,245,)");

  SerialBT.println("add_text_box(2,3,2,L,Text,245,240,245,F)");
  SerialBT.println("add_text_box(8,3,2,L,Text,245,240,245,G)");
  SerialBT.println("add_slider(1,9,3,0,100,0,A,A,0)");
  SerialBT.println("add_slider(1,8,2,0,100,0,L,L,0)");
  SerialBT.println("add_slider(8,8,2,0,100,0,R,R,0)");
  SerialBT.println("add_slider(0,5,4,0,40,0,S,S,0)");
  SerialBT.println("add_free_pad(2,5,-255,255,0,0,Y,)");
  SerialBT.println("add_free_pad(8,5,-255,255,0,0,R,P)");
  SerialBT.println("add_gauge(6,2,4,0,100,50,g,,,10,5)");
  SerialBT.println("add_gauge(0,2,4,0,100,0,f,,,10,5)");
  SerialBT.println("add_gauge(3,1,4,0,100,0,h,,,10,5)");
  SerialBT.println("set_panel_notes(-,,,)");
  SerialBT.println("run()");
  SerialBT.println("*");
}

void receive_data(){
  if (SerialBT.available()){
    //digitalWrite(2,HIGH); 
    data_in=SerialBT.read();  //Get next character

    if(data_in=='L'){ //  Slider
      yaw_sen=SerialBT.parseInt();
    }

    if(data_in=='R'){ //  Slider
      pitch_roll_sen=SerialBT.parseInt();
    }

    if(data_in=='S'){ // Servo
      angle_perc=SerialBT.parseInt();
    }

    // Left joy, yaw
    if(data_in=='W'){ //  yaw
      while(true){
        if (SerialBT.available()){
          data_in=SerialBT.read();  //Get next character
          if(data_in=='X') yaw=SerialBT.parseInt();
          if(data_in=='W') break; // exit loop
        }
      }
    } 

    // Right joy, roll and pitch
    if(data_in=='P'){ //  Pad Start Text
      while(true){
        if (SerialBT.available()){
          data_in=SerialBT.read();  //Get next character
          if(data_in=='X') roll=SerialBT.parseInt();
          if(data_in=='Y') pitch=-SerialBT.parseInt();
          if(data_in=='P') break; // End character
        }
      }
    } 

  } //else {
  //   digitalWrite(2,LOW);
  //   yaw = yaw_previous;
  //   roll = roll_previous;
  //   pitch = pitch_previous;
  // }
}

void send_data(){
  unsigned long t=millis();
  if ((t-last_time)>update_interval){
    last_time=t;

    SerialBT.print("*F"+String(Vb1)+"*");
    SerialBT.print("*G"+String(Vb2)+"*");
    SerialBT.print("*H"+String(Vb_tot)+"*");

    // Orange Progress Bar (Range is from 0 to 100)
    progress=(Vb1-Vb1_min)/Vb1_range*100; 
    SerialBT.print("*f"+String(progress)+"*");

    progress=(Vb2-Vb2_min)/Vb2_range*100;; 
    SerialBT.print("*g"+String(progress)+"*");

    progress=(Vb_tot-Vb_tot_min)/Vb_tot_range*100;
    SerialBT.print("*h"+String(progress)+"*");
  }
}