/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
int LED_10 = 10;
unsigned long currTime;
unsigned long prevTime;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_10, OUTPUT);
  pinMode(2, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
//Part 2
//  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(200);                       // wait for a second
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(200);                       // wait for a second

//Part 3
//  digitalWrite(LED_10, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(200);                       
//  digitalWrite(LED_10, LOW);    // turn the LED off by making the voltage LOW
//  delay(200);                       

//Part 4
//  tone(2, 250);
//  digitalWrite(LED_10, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);  
//  noTone(2);                     
//  digitalWrite(LED_10, LOW);    // turn the LED off by making the voltage LOW
//  tone(2, 250);
//  digitalWrite(LED_BUILTIN, HIGH);
//  delay(1000);          
//  noTone(2);             
//  digitalWrite(LED_BUILTIN, LOW);

//Part 5
    currTime = millis();
    if(currTime < 100000) {
      if(currTime - prevTime >= 0) {
        digitalWrite(2, !digitalRead(2));
        prevTime = currTime;
      }
    }
    digitalWrite(LED_10, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       
    digitalWrite(LED_10, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);                       
    digitalWrite(LED_BUILTIN, LOW);
    
}
