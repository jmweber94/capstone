/* Sweep
 by BARRAGAN <http://barraganstudio.com> 
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Sweep
*/ 

#include <Servo.h> 

 
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
 
int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
  Serial.begin(9600);
} 
 
void loop() 
{ 
  if (Serial.available()) {  // Returns true if there is serial input.
    char ch = Serial.read();
    
    if (ch == 'e') {
      // Make sure not to exceed the mechanical limitation.
      if (pos <= 90){
        pos += 90;
      }
    } 
    else if (ch == 'r'){
      // Make sure not to exceed the mechanical limitation.
      if(pos >=30){
      pos -= 30;
      }
    }
    
    // Now ask the servo to move to that position.
    myservo.write(pos);
    // Mechnical limitation to the frequency of commands given.
    delay(50);
  }
} 

