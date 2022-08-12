#include <dht.h>

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <stdlib.h>
#include <LiquidCrystal.h>

#define d 3400
#define e 3038 // 329 Hz
#define c 3830 // 261 Hz
#define c3 7655 // 130 Hz
#define g 5100 // 196 Hz

#define LED_PIN 53
#define DHT11_PIN 7
#define BUTTON_PIN 5
#define TRIG_PIN 41
#define ECHO_PIN 43
#define SAMPLES 128

QueueHandle_t xQueue1, xQueue2, xQueue3;

int tempFlag = 0;
int switched = 0;


dht DHT;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  } 

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  "Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
    
  xTaskCreate(
    TaskButton
    ,  "Button"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskTemp
    ,  "Temp"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskRange
    ,  "Range"   // A name just for humans
    ,  300  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskSpeaker
    ,  "Speaker"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
  
  xTaskCreate(
    TaskDisplay
    ,  "Display"   // A name just for humans
    ,  500  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
    

  vTaskStartScheduler();


}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskSpeaker(void *pvParameters) {
  DDRH |= 1 << 3;
  TCCR4A = _BV(COM4A0); // Toggle
  TCCR4B = _BV(WGM42) | _BV(CS41); // CTC and Scalar
  

  int distance;

  for (;;)
  {
    xQueueReceive(xQueue1, &distance, 0);
    if (distance < 100) {
      PRR1 &= ~(1 << PRTIM4);
      OCR4A = c3 - (100 * (50 - distance));
    }else{
      PRR1 |= (1 << PRTIM4);
    }
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }
}

void TaskRange(void *pvParameters) {
  /*
  unsigned long t1;
  unsigned long t2;
  unsigned long pulse_width;
  float cm;
  float inches;
  */
  long duration; // variable for the duration of sound wave travel
  int distance; // variable for the distance measurement


   // The Trigger pin will tell the sensor to range find
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);

  //Set Echo pin as input to measure the duration of 
  //pulses coming back from the distance sensor
  pinMode(ECHO_PIN, INPUT);

  xQueue1 = xQueueCreate( 1, sizeof( int ) );

  for(;;) {
    
       // Clears the trigPin condition
      digitalWrite(TRIG_PIN, LOW);
      delayMicroseconds(2);
      // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN, LOW);
      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(ECHO_PIN, HIGH);
      // Calculating the distance
      distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
      // Displays the distance on the Serial Monitor

      xQueueSendToBack(xQueue1, (void *)&distance, 0);

      /*
      // Hold the trigger pin high for at least 10 us
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN, LOW);
    
      // Wait for pulse on echo pin
      while ( digitalRead(ECHO_PIN) == 0 );
    
      // Measure how long the echo pin was held high (pulse width)
      // Note: the micros() counter will overflow after ~70 min
      t1 = micros();
      while ( digitalRead(ECHO_PIN) == 1);
      t2 = micros();
      pulse_width = t2 - t1;
    
      // Calculate distance in centimeters and inches. The constants
      // are found in the datasheet, and calculated from the assumed speed
      //of sound in air at sea level (~340 m/s).
      cm = pulse_width / 58.0;
      inches = pulse_width / 148.0;
    
      // Print out results
      if ( pulse_width > MAX_DIST ) {
        Serial.println("Out of range");
      } else {
        Serial.print(cm);
        Serial.print(" cm \t");
        Serial.print(inches);
        Serial.println(" in");
      }
      */
    
      // Wait at least 60ms before next measurement
      vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

void TaskButton(void *pvParameters) {
  vTaskDelay(100/portTICK_PERIOD_MS);

  int lastState = HIGH;
  int currentState;
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  for (;;) {
    // read the state of the switch/button:
    currentState = digitalRead(BUTTON_PIN);
  
    if(lastState == LOW && currentState == HIGH) {
      
      tempFlag = !tempFlag;
      switched = 1;
      
    }
      
  
    // save the last state
    lastState = currentState;
  }
}

void TaskDisplay(void *pvParameters) {

  const int rs = 23, en = 25, d4 = 27, d5 = 29, d6 = 31, d7 = 33;
  LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

  vTaskDelay(50/portTICK_PERIOD_MS); 

  lcd.begin(16, 2);
  lcd.print("Temperature (F)");
  //lcd.setCursor(0,1);
  float res = 0;
  float inter = 0;
  for (;;) {
    xQueueReceive(xQueue3, &inter, 0);
    if (inter != res) {
      lcd.clear();
      if (!tempFlag) {
        lcd.print("Temperature (C)");
      } else {
        lcd.print("Temperature (F)");
        inter = inter * 1.8 + 32;
      }
      lcd.setCursor(0,1);
      lcd.print(inter);
      res = inter;
    }
    if (switched) {
      int temp;
      lcd.clear();
      if (!tempFlag) {
        lcd.print("Temperature (C)");
        temp = res;
      } else {
        lcd.print("Temperature (F)");
        temp = res * 1.8 + 32;
      }
      lcd.setCursor(0,1);
      lcd.print(temp);
      switched = 0;
    }
  }
}

void TaskTemp(void *pvParameters) {
  vTaskDelay(0/portTICK_PERIOD_MS); 
  xQueue3 = xQueueCreate( 1, sizeof( float ) );
  for (;;) {
    int chk = DHT.read11(DHT11_PIN);
    Serial.println(DHT.temperature);
    xQueueSendToBack(xQueue3, (void *)&DHT.temperature, 0);
    
    vTaskDelay( 200 / portTICK_PERIOD_MS );
  }
}

void TaskBlink(void *pvParameters)  // This is a task.
{
 // (void) pvParameters;  // allocate stack space for params

/*
  Blink
*/

  pinMode(LED_PIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 100 / portTICK_PERIOD_MS );
    digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 200 / portTICK_PERIOD_MS ); 
  }
}

void TaskPlayMelody(void *pvParameters)  // This is a task.
{
 // (void) pvParameters;

  DDRH |= 1 << 3;
  TCCR4A = _BV(COM4A0); // Toggle
  TCCR4B = _BV(WGM42) | _BV(CS41); // CTC and Scalar
  

  int melody[] = {d,e,c,c3,g};
  int curr_note = 0;
  int times_played = 0;

  for (;;)
  {
    if (curr_note == 5) {
      PRR1 |= (1 << PRTIM4);
      curr_note = 0;
      times_played++;
      if (times_played == 3) {
        vTaskDelete(NULL);
      }else {
        vTaskDelay( 1500 / portTICK_PERIOD_MS ); 
      }
    }
    PRR1 &= ~(1 << PRTIM4);
    OCR4A = melody[curr_note];
    curr_note++;
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); 
  }
}
