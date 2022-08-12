#include <arduinoFFT.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <stdlib.h>

#define d 3400
#define e 3038 // 329 Hz
#define c 3830 // 261 Hz
#define c3 7655 // 130 Hz
#define g 5100 // 196 Hz

#define LED_PIN 53

#define SAMPLES 128
////////////////////////////////////////////////
// APPROVED FOR ECE 474   Spring 2021
//
//  NOTE: modify analogRead() on line 113 according
//   to your setup.
////////////////////////////////////////////////


void TaskBlink( void *pvParameters );
void TaskPlayMelody( void *pvParameters );

QueueHandle_t xQueue1, xQueue2;
TaskHandle_t xTaskRT3p0, xTaskRT3p1, xTaskRT4p0;
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(19200);
  
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
    TaskPlayMelody
    ,  "PlayMelody"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  xTaskCreate(
    TaskRT3p0
    ,  "RT3p0"
    ,  600  // Stack size
    ,  NULL
    ,  0  // Priority
    ,  &xTaskRT3p0);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
  //  (note how the above comment is WRONG!!!)
  vTaskStartScheduler();


}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/


void TaskRT3p0(void *pvParameters)
{

  static double data[SAMPLES];
    
  xQueue1 = xQueueCreate( 1, sizeof( double * ) );
  xQueue2 = xQueueCreate( 1, sizeof( unsigned long ) );
  for (int i = 0; i < SAMPLES; i++) {
    data[i] = rand();
    //Serial.println(data[i]);
  }

  xTaskCreate(
    TaskRT3p1
    ,  "RT4p1"
    ,  128  // Stack size
    ,  (void *) data
    ,  0  // Priority
    ,  &xTaskRT3p1 );
    
  xTaskCreate(
    TaskRT4p0
    ,  "RT4p0"
    ,  650  // Stack size
    ,  NULL
    ,  0  // Priority
    ,  &xTaskRT4p0 );
  
  vTaskSuspend(NULL);
  vTaskDelete(xTaskRT3p1);
  vTaskDelete(xTaskRT4p0);
  vTaskDelete(NULL);
}

void TaskRT3p1(void *pvParameters)
{
  double *dataPtr = (double *) pvParameters;
  unsigned long res;
  unsigned long total = 0;
  for (int j = 0; j < 5; j++) {
    xQueueSendToBack(xQueue1, (void *)&dataPtr, 0);
    xQueueReceive(xQueue2, &res, portMAX_DELAY);
    total += res;
    //Serial.println(res);
  }
  Serial.println((total));
  vTaskResume(xTaskRT3p0);
}

void TaskRT4p0(void *pvParameters)
{
  double *receivedDataPtr;
  double vImag[SAMPLES] = {0};
  unsigned long startTime;
  unsigned long endTime;
  unsigned long elapsedTime;
  for (;;) {
    xQueueReceive(xQueue1, &receivedDataPtr, portMAX_DELAY);
    startTime = millis();
    FFT.Compute(receivedDataPtr, vImag, SAMPLES, FFT_FORWARD); /* Compute FFT */
    endTime = millis();
    elapsedTime = endTime - startTime;
    xQueueSendToBack(xQueue2, (void *)&elapsedTime, 0);
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
