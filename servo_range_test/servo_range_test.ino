#include <ESP32Servo.h>

Servo S1;  // create servo object to control a servo
Servo S2;
int max_angle=120;    
int min_angle=67;
void setup() {
  S2.setPeriodHertz(50);
  S2.attach(32);

  S1.setPeriodHertz(50);
  S1.attach(23);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  // LIFT
    S1.write(123);   
    S2.write(63);  
    delay(2000);

  // Down
    S1.write(50); 
    S2.write(138);            
delay(2000);
}
