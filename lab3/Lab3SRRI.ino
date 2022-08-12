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

int zero[] = {1, 1, 1, 1, 1, 1, 0, 0};
int one[] = {0, 1, 1, 0, 0, 0, 0, 0};
int two[] = {1, 1, 0, 1, 1, 0, 1, 0};
int three[] = {1, 1, 1, 1, 0, 0, 1, 0};
int four[] = {0, 1, 1, 0, 0, 1, 1, 0};
int five[] = {1, 0, 1, 1, 0, 1, 1, 0};
int six[] = {1, 0, 1, 1, 1, 1, 1, 0};
int seven[] = {1, 1, 1, 0, 0, 0, 0, 0};
int eight[] = {1, 1, 1, 1, 1, 1, 1, 0};
int nine[] = {1, 1, 1, 1, 0, 1, 1, 0};

int melody[] = {d,e,c,c3,g};
int melody_hz[] = {293, 329, 261, 130, 196};
int curr_note = -1;

int arrDigs[][8] = {{1, 1, 1, 1, 1, 1, 0, 0}, {0, 1, 1, 0, 0, 0, 0, 0}, 
{1, 1, 0, 1, 1, 0, 1, 0}, {1, 1, 1, 1, 0, 0, 1, 0},
{0, 1, 1, 0, 0, 1, 1, 0}, {1, 0, 1, 1, 0, 1, 1, 0}, 
{1, 0, 1, 1, 1, 1, 1, 0}, {1, 1, 1, 0, 0, 0, 0, 0}, 
{1, 1, 1, 1, 1, 1, 1, 0}, {1, 1, 1, 1, 0, 1, 1, 0}};

int num_displayed;
int display_on = 1;
int curr_dig;
int display_ind = 0;

volatile unsigned long curr_time = 0;
unsigned long display_time = -50;
unsigned long light_time = -750;
unsigned long speaker_time = -4000;
unsigned long light_delay = 0;

void displayInt(int n);
void displayDigit(int* dig, int pos);
void clearDisplay();

void task1(void *p);
void task2(void *p);
void task3(void *p);
int task3_arg;
void task4(void *p);

void schedule_sync(void *p);
void sleep_474(int i);
void start_function(void (*functionPTR) (void *p));

void RR();
void SRRI();
void DDS();

void (*tasks[NTASKS]) (void *p);
int task_ind = 0;
unsigned int reload = 0x1F;

enum state {READY, RUNNING, SLEEPING, DEAD};

int states[NTASKS][2];

enum flag {PENDING, DONE};

volatile enum flag sFlag;

ISR(TIMER2_COMPA_vect) {
  sFlag = DONE;
  OCR2A = reload;
}

void schedule_sync(void *p) {
  while (sFlag == PENDING);
  //curr_time++;
  for (int i = 0; i < NTASKS; i++) {
    if (states[i][0] == SLEEPING) {
      states[i][1] -= 2;
    }
    if (states[i][1] <= 0) {
      states[i][0]= READY;
    }
  }
  sFlag = PENDING;
  return;
}

void setup() {
  DDRL |= BIT_2;
  
  DDRH |= 1 << 3;
  TCCR4A = _BV(COM4A0); // Toggle
  TCCR4B = _BV(WGM42) | _BV(CS41); // CTC and Scalar

  TCCR2B = 0; 
  TCCR2A = 1<<WGM21;
  TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20);
  TIMSK2 = (1<<OCIE2A);
  
  // initialize the digital pins as outputs.
  pinMode(pinA, OUTPUT);      
  pinMode(pinB, OUTPUT);     
  pinMode(pinC, OUTPUT);     
  pinMode(pinD, OUTPUT);     
  pinMode(pinE, OUTPUT);     
  pinMode(pinF, OUTPUT);     
  pinMode(pinG, OUTPUT);
  pinMode(dp, OUTPUT);
   
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);  
  pinMode(D3, OUTPUT);  
  pinMode(D4, OUTPUT);   

  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  
  digitalWrite(pinA, LOW);   
  digitalWrite(pinB, LOW);   
  digitalWrite(pinC, LOW);   
  digitalWrite(pinD, LOW);   
  digitalWrite(pinE, LOW);   
  digitalWrite(pinF, LOW);   
  digitalWrite(pinG, LOW);

  num_displayed = 0;

  // SRRI Initialization
  
  tasks[0] = task1;
  tasks[1] = task2;
  tasks[2] = task3;
  tasks[3] = task4;
  tasks[4] = schedule_sync;
  tasks[5] = NULL;

  states[0][0] = READY;
  states[1][0] = READY;
  states[2][0] = READY;
  states[3][0] = READY;
  
  states[0][1] = 0;
  states[1][1] = 0;
  states[2][1] = 0;
  states[3][1] = 0;

  Serial.begin(9600);
  SREG |= (1<<7);
  OCR2A = reload;
}

// the loop routine runs over and over again forever:
void loop() {
  SRRI();
}

void SRRI() {
  if (tasks[task_ind] == NULL && task_ind != 0) {
    //Serial.print("restart");
    task_ind = 0;
  }
  if (tasks[task_ind] == NULL && task_ind == 0) {
    // do something
  }

  start_function(tasks[task_ind]);
  task_ind++;
  return;
}

void start_function(void (*functionPTR) (void *p)) {
  functionPTR(NULL);
}

void sleep_474(int t){
  states[task_ind][0] = SLEEPING;
  states[task_ind][1] = t;
}

void task1(void *p) {
  if (states[task_ind][0] != READY) {
    return;
  }
  states[task_ind][0] = RUNNING;
  if ((PORTL & BIT_2)) {
    PORTL &= !BIT_2;
    sleep_474(750);
  } else {
    PORTL |= BIT_2;
    sleep_474(250);
  }
}

void task2(void *p) {
  if (states[task_ind][0] != READY) {
    return;
  }
  states[task_ind][0] = RUNNING;
  if (curr_note < 5) {
    curr_note++;
    if (curr_note == 5){
      PRR1 |= (1 << PRTIM4);
      sleep_474(4000);
      return;
    } else {
      OCR4A = melody[curr_note];
    }
    sleep_474(1000);
  } else {
    PRR1 &= ~(1 << PRTIM4);
    curr_note = 0;
    OCR4A = melody[curr_note];
    sleep_474(1000);
  }
}

void task3(void *p) {
  if (states[task_ind][0] != READY) {
    return;
  }
  states[task_ind][0] = RUNNING;
  displayInt(num_displayed);
  states[task_ind][0] = READY;
}

void task4(void *p) {
  if (states[task_ind][0] != READY) {
    return;
  }
  states[task_ind][0] = RUNNING;
  num_displayed++;
  sleep_474(100);
}

void displayInt(int n) {
  //curr_time = millis();
  if (display_ind == 0) {
    curr_dig = n;
  }
  if (display_on) {
    clearDisplay();
  } else {
    int curr = curr_dig % 10;
    displayDigit(arrDigs[curr], display_ind);
    curr_dig = curr_dig / 10;
    display_ind++;
  }
  if (curr_dig == 0) {
    display_ind = 0;
  }
}

void displayDigit(int dig[], int pos) {
  for (int i = 0; i < 8; i++){
    digitalWrite(pinA + i, dig[i]);
  }
  digitalWrite((2 * pos) + D1, LOW);
  display_on = 1;
}

void clearDisplay() {
  for (int i = pinA; i < dp + 1; i++) {
    digitalWrite(i, LOW);
  }
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  display_on = 0;
}
