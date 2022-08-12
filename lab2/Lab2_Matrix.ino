#define c 3830 // 261 Hz
#define d 3400 // 294 Hz
#define e 3038 // 329 Hz
#define f 2864 // 349 Hz
#define g 2550 // 392 Hz
#define a 2272 // 440 Hz
#define b 2028 // 493 Hz
#define C 1912 // 523 Hz
#define R 0

#define OP_DECODEMODE  8
#define OP_SCANLIMIT   10
#define OP_SHUTDOWN    11
#define OP_DISPLAYTEST 14
#define OP_INTENSITY   10

//Transfers 1 SPI command to LED Matrix for given row
//Input: row - row in LED matrix
//       data - bit representation of LEDs in a given row; 1 indicates ON, 0 indicates OFF
void spiTransfer(volatile byte row, volatile byte data);

// change these pins as necessary
int DIN = 50;
int CS =  51;
int CLK = 52;

byte spidata[2]; //spi shift register uses 16 bits, 8 for ctrl and 8 for data

int x, y;
int erase = 0;

// Mary Song
int melody[] = { e, R, d, R, c, R, d, R, e, R,e, R,e, R,d, 
R,d, R,d, R,e, R,g, R,g, R,e, R,d, R,c, R,d, R,e, R,e, R,e, 
R,e, R,d, R,d, R,e, R,d, R,c, R,c, R };
int currTime = 0;
int currNote = 0;
int prevSpeakerMillis = 0;
int prevLEDTime = 0;

int convertInput(int n) {
 return (n - 0) * (8 - 0) / (1024 - 0) + 0;
}

void mary() {
  if (currTime - prevSpeakerMillis >= 100) {
    OCR4A = melody[currNote];
    currNote++;
    if (currNote == 54) {
      currNote = 0;
    }
    prevSpeakerMillis = currTime;
  }
}

void setup(){

  //must do this setup
  pinMode(DIN, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);
  digitalWrite(CS, HIGH);
  spiTransfer(OP_DISPLAYTEST,0);
  spiTransfer(OP_SCANLIMIT,7);
  spiTransfer(OP_DECODEMODE,0);
  spiTransfer(OP_SHUTDOWN,1);

  //Stick Input
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  
  //Speaker
  DDRH |= 1 << 3;
  TCCR4A = _BV(COM4A0); // Toggle
  TCCR4B = _BV(WGM42) | _BV(CS40); // CTC and Scalar

  for (int i = 0; i < 8; i++) {
    spiTransfer(i, 0);
  }
  
  Serial.begin(9600);
}

void loop(){
  currTime = millis();
  //mary();
  if (!erase) {
    x = convertInput(analogRead(A0));
    y = convertInput(analogRead(A1));
    spiTransfer(x,1 << y);
    erase = 1;
  } else if (currTime - prevLEDTime >= 10) {
    spiTransfer(x,0);
    prevLEDTime = currTime;
    erase = 0;
  }
}

void spiTransfer(volatile byte opcode, volatile byte data){
  int offset = 0; //only 1 device
  int maxbytes = 2; //16 bits per SPI command
  
  for(int i = 0; i < maxbytes; i++) { //zero out spi data
    spidata[i] = (byte)0;
  }
  //load in spi data
  spidata[offset+1] = opcode+1;
  spidata[offset] = data;
  digitalWrite(CS, LOW); //
  for(int i=maxbytes;i>0;i--)
    shiftOut(DIN,CLK,MSBFIRST,spidata[i-1]); //shift out 1 byte of data starting with leftmost bit
  digitalWrite(CS,HIGH);
}
