#define d 3400
#define e 3038 // 329 Hz
#define c 3830 // 261 Hz
#define c3 7655 // 130 Hz
#define g 5100 // 196 Hz

#define BIT_2 1 << 2

#define NTASKS 10

int pinA = 34; 
int pinB = 35;
int pinC = 36;
int pinD = 37;
int pinE = 38;
int pinF = 39;
int pinG = 40;
int dp = 41;
int D1 = 25;
int D2 = 27;
int D3 = 29;
int D4 = 31;

int melody[] = {d,e,c,c3,g};
int curr_note = -1;

volatile unsigned long curr_time = 0;
unsigned long light_time = -750;
unsigned long speaker_time = -4000;

void task1();
void task2();

void RR();

void setup() {
  //cli();
  DDRL |= BIT_2;
  
  DDRH |= 1 << 3;
  TCCR4A = _BV(COM4A0); // Toggle
  TCCR4B = _BV(WGM42) | _BV(CS41); // CTC and Scalar
  
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  RR();
}

void RR() {
  task1();
  task2();
  /*
  unsigned long micro = micros();
  delayMicroseconds();
  */
}

void task1() {
  curr_time = millis();
  if ((PORTL & BIT_2) && (curr_time - light_time >= 250)) {
    PORTL &= !BIT_2;
    light_time = curr_time;
  } else if(!(PORTL & BIT_2) && (curr_time - light_time >= 750)){
    PORTL |= BIT_2;
    light_time = curr_time;
  }
}

void task2() {
  curr_time = millis();
  if (curr_time - speaker_time >= 4000) {
    PRR1 &= ~(1 << PRTIM4);
    curr_note = 0;
    OCR4A = melody[curr_note];
    speaker_time = curr_time;
  } else if (curr_time - speaker_time >= 1000 && !(PRR1 & (1 << PRTIM4))) {
    curr_note++;
    if (curr_note == 5){
      PRR1 |= _BV(PRTIM4);
    } else {
      OCR4A = melody[curr_note];
    }
    speaker_time = curr_time;
  }
}
