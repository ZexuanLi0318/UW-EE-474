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

int smile_arr[][8] = {{0, 1, 0, 0, 1, 0, 1, 0}, {1, 0, 0, 1, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}, {0, 0, 1, 0, 0, 1, 1, 0}};

int num_displayed;
int smile_showing = 0;
int display_on = 1;
int curr_dig;
int display_ind = 0;
int theme_count = 0;

void displayInt(int n);
void displayDigit(int* dig, int pos);
void clearDisplay();

void task1(void *p);
int task2_arg;
void task2(void *p);
void task3(void *p);
int task3_arg;
void task4(void *p);
void task5(void *p);
void displaySmile(void *p);

void schedule_sync(void *p);
void sleep_474(int i);
void task_self_quit();
void start_function(void (*functionPTR) (void *p));


void DDS();

int task_ind = 0;
unsigned int reload = 0x1F;

enum state {READY, RUNNING, SLEEPING, DEAD};

int states[NTASKS][2];

enum flag {PENDING, DONE};

volatile enum flag sFlag;

typedef struct{
  void (*ftpr)(void *p); // the function pointer
  void *arg_ptr; // the argument pointer
  enum state tcb_state; // the task state
  unsigned int delay_t;
  unsigned int id;
  char task_name[20];
  int start_count;
} TCB;

TCB tcb_list[NTASKS];
TCB dead_list[NTASKS];

void setup() {
  //cli();
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

  num_displayed = 40;
  
  // DDS Initialization

  int curr_id = 0;

  tcb_list[curr_id].ftpr = task1;
  tcb_list[curr_id].arg_ptr = NULL;
  tcb_list[curr_id].tcb_state = READY;
  tcb_list[curr_id].delay_t = 0;
  tcb_list[curr_id].id = curr_id;
  memcpy(tcb_list[curr_id].task_name, "LED Flasher        ", 20);
  tcb_list[curr_id].start_count = 1;
  curr_id++;

  task2_arg = 2;
  tcb_list[curr_id].ftpr = task2;
  tcb_list[curr_id].arg_ptr = (void*)&task2_arg;
  tcb_list[curr_id].tcb_state = READY;
  tcb_list[curr_id].delay_t = 0;
  tcb_list[curr_id].id = curr_id;
  memcpy(tcb_list[curr_id].task_name, "Speaker Tone        ", 20);
  tcb_list[curr_id].start_count = 1;
  curr_id++;
  
  tcb_list[curr_id].ftpr = task3;
  tcb_list[curr_id].arg_ptr = (void*)&task3_arg;
  tcb_list[curr_id].tcb_state = DEAD;
  tcb_list[curr_id].delay_t = 0;
  tcb_list[curr_id].id = curr_id;
  memcpy(tcb_list[curr_id].task_name, "Display             ", 20);
  tcb_list[curr_id].start_count = 0;
  curr_id++;

  tcb_list[curr_id].ftpr = task4;
  tcb_list[curr_id].arg_ptr = NULL;
  tcb_list[curr_id].tcb_state = DEAD;
  tcb_list[curr_id].delay_t = 0;
  tcb_list[curr_id].id = curr_id;
  memcpy(tcb_list[curr_id].task_name, "Combo              ", 20);
  tcb_list[curr_id].start_count = 0;
  curr_id++;

  tcb_list[curr_id].ftpr = task5;
  tcb_list[curr_id].arg_ptr = NULL;
  tcb_list[curr_id].tcb_state = READY;
  tcb_list[curr_id].delay_t = 0;
  tcb_list[curr_id].id = curr_id;
  memcpy(tcb_list[curr_id].task_name, "Combo              ", 20);
  tcb_list[curr_id].start_count = 1;
  curr_id++;

  tcb_list[curr_id].ftpr = displaySmile;
  tcb_list[curr_id].arg_ptr = NULL;
  tcb_list[curr_id].tcb_state = DEAD;
  tcb_list[curr_id].delay_t = 0;
  tcb_list[curr_id].id = curr_id;
  memcpy(tcb_list[curr_id].task_name, "Smile              ", 20);
  tcb_list[curr_id].start_count = 0;
  curr_id++;

  tcb_list[curr_id].ftpr = schedule_sync;
  tcb_list[curr_id].arg_ptr = NULL;
  tcb_list[curr_id].tcb_state = READY;
  tcb_list[curr_id].delay_t = 0;
  tcb_list[curr_id].id = curr_id;
  memcpy(tcb_list[curr_id].task_name, "Schedule Sync       ", 20);
  tcb_list[curr_id].start_count = 1;
  
  curr_id++;
  tcb_list[curr_id].ftpr = NULL;
  Serial.begin(9600);
  SREG |= (1<<7);
  OCR2A = reload;
  //sei();
}

ISR(TIMER2_COMPA_vect) {
  sFlag = DONE;
  OCR2A = reload;
}

void schedule_sync(void *p) {
  while (sFlag == PENDING);
  //curr_time++;
  for (int i = 0; i < NTASKS; i++) {
    if (tcb_list[i].tcb_state == SLEEPING) {
      tcb_list[i].delay_t -= 2;
    }
    if (tcb_list[i].delay_t <= 0 && tcb_list[i].tcb_state != DEAD) {
      tcb_list[i].tcb_state = READY;
    }
  }
  sFlag = PENDING;
  return;
}

void loop() {
  DDS();
}

void DDS() {
  if (tcb_list[task_ind].ftpr == NULL && task_ind != 0) {
    //Serial.print("restart");
    task_ind = 0;
  }
  if (tcb_list[task_ind].ftpr == NULL && task_ind == 0) {
    // do something
  }

  start_function(tcb_list[task_ind].ftpr);
  task_ind++;
  return;
}

void start_function(void (*functionPTR) (void *p)) {
  functionPTR(tcb_list[task_ind].arg_ptr);
}

void sleep_474(int t){
  tcb_list[task_ind].delay_t = t;
  tcb_list[task_ind].tcb_state = SLEEPING;
}

void task_self_quit() {
  //tcb_list[task_ind].delay_t = 0;
  tcb_list[task_ind].tcb_state = DEAD;
  dead_list[task_ind] = tcb_list[task_ind];
}

void task_start(TCB *p) {
  p->start_count++;
  p->tcb_state = READY;
}

void task1(void *p) {
  if (tcb_list[task_ind].tcb_state != READY) {
    return;
  }
  tcb_list[task_ind].tcb_state = RUNNING;
  if ((PORTL & BIT_2)) {
    PORTL &= !BIT_2;
    sleep_474(750);
  } else {
    PORTL |= BIT_2;
    sleep_474(250);
  }
}

void task2(void *p) {
  if (tcb_list[task_ind].tcb_state != READY) {
    return;
  }
  //Serial.print("enter");
  int* arg = (int*)p;
  if (*arg != -1 && (*arg == theme_count && PRR1 | (1 << PRTIM4))) {
    curr_note = -1;
    theme_count = 0;
    task_self_quit();
    return;
  }
  tcb_list[task_ind].tcb_state = RUNNING;
  if (curr_note < 5) {
    curr_note++;
    if (curr_note == 5){
      theme_count++;
      PRR1 |= (1 << PRTIM4);
      sleep_474(4000);
      return;
    } else {
      PRR1 &= ~(1 << PRTIM4);
      OCR4A = melody[curr_note];
    }
  } else {
    PRR1 &= ~(1 << PRTIM4);
    curr_note = 0;
    OCR4A = melody[curr_note];
  }
  sleep_474(1000);
}

void task3(void *p) {
  if (tcb_list[task_ind].tcb_state != READY) {
    return;
  }
  //Serial.print("run");
  tcb_list[task_ind].tcb_state = RUNNING;
  int *arg = (int*)p;
  //Serial.print(*arg);
  //Serial.print("\n");
  displayInt(*arg);
  tcb_list[task_ind].tcb_state = READY;
}

void task4(void *p) {
  if (tcb_list[task_ind].tcb_state != READY) {
    return;
  }
  task2_arg = -1;
  tcb_list[task_ind].tcb_state = RUNNING;
  if (!(PRR1 & (1 << PRTIM4))) {
    task3_arg = melody_hz[curr_note];
    num_displayed = 40;
  } else {
    num_displayed -= 1;
    task3_arg = num_displayed;
    sleep_474(100);
    return;
  }
  tcb_list[task_ind].tcb_state = READY;
}

void task5(void *p) {
  if (tcb_list[task_ind].tcb_state != READY) {
    return;
  }
  tcb_list[task_ind].tcb_state = RUNNING;
  if (tcb_list[1].tcb_state == DEAD && tcb_list[2].tcb_state == DEAD && !smile_showing) {
    task_start(&tcb_list[2]);
    num_displayed = 30;
    task3_arg = 30;
    tcb_list[task_ind].tcb_state = READY;
  } else if (num_displayed > 0) {
    num_displayed -= 1;
    task3_arg = num_displayed;
    sleep_474(100);
  } else if (tcb_list[1].tcb_state == DEAD && num_displayed == 0) {
    task2_arg = 1;
    num_displayed = -1;
    task_start(&tcb_list[1]);
  } else if (tcb_list[1].tcb_state == DEAD && !smile_showing) {
    tcb_list[2].tcb_state = DEAD;
    task_start(&tcb_list[5]);
    sleep_474(2000);
  } else if (smile_showing) {
    smile_showing = 0;
    clearDisplay();
    tcb_list[1].tcb_state = DEAD;
    tcb_list[3].tcb_state = DEAD;
    tcb_list[4].tcb_state = DEAD;
    tcb_list[5].tcb_state = DEAD;
  }
  /*
  if (tcb_list[1].tcb_state == DEAD && tcb_list[2].tcb_state == DEAD) {
    task_start(&tcb_list[2]);
    num_displayed = 30;
    task3_arg = 30;
    tcb_list[task_ind].tcb_state = READY;
    return;
  } else if (num_displayed == 0) {
    task_start(&tcb_list[1]);
  } else { 

  }
  */
}

void displaySmile(void *p) {
  if (tcb_list[task_ind].tcb_state != READY) {
    return;
  }
  smile_showing = 1;
  tcb_list[task_ind].tcb_state = RUNNING;
  if (display_ind == 0) {
    curr_dig = 0;
  }
  if (display_on) {
    clearDisplay();
  } else {
    Serial.print(curr_dig);
    displayDigit(smile_arr[curr_dig], display_ind);
    curr_dig++;
    display_ind++;
  }
  if (curr_dig == 4) {
    display_ind = 0;
  }
  tcb_list[task_ind].tcb_state = READY;
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
