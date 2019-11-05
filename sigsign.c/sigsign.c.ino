

#define CLOCK 8
#define DATA  9
#define LATCH 10


#define BUFFER_SIZE      96
#define DISPLAY_SIZE     6
#define NUMBER_OF_DIGITS 10

///////////////////////////////////////////////////////////////////////////
//
// THIS IS A BUFFER POINTER MAP TO ACCESS INDIVIDUAL DIGIT ELEMENT FLAGS
//
// INDEX INTO THIS BY DIGIT AND THEN BY ELEMENT
//
///////////////////////////////////////////////////////////////////////////

uint16_t DISPLAY_MAP[DISPLAY_SIZE][NUMBER_OF_DIGITS] = {
  // CARD 3 pointers
  { 37,  47,  45,  41,  46,  42,  38,  40,  44,  43},
  { 33,  53,  51,  35,  52,  48,  34,  36,  50,  49},
//  {223, 214, 216, 221, 215, 219, 222, 220, 217, 218},
//  {197, 207, 205, 201, 206, 202, 198, 200, 204, 203},
//  {193, 213, 211, 195, 212, 208, 194, 196, 210, 209},
//  {255, 246, 248, 253, 247, 251, 254, 252, 249, 250},
//  {229, 239, 237, 233, 238, 234, 230, 232, 236, 235},
//  {225, 245, 243, 227, 244, 240, 226, 228, 242, 241},
  
  // CARD 2 pointers
  {  1,  21,  19,   3,  20,  16,   2,   4,  18,  17},
  { 63,  54,  56,  61,  55,  59,  62,  60,  57,  58},
//  { 95,  86,  88,  93,  87,  91,  94,  92,  89,  90},
//  { 69,  79,  77,  73,  78,  74,  70,  72,  76,  75},
//  { 65,  85,  83,  67,  84,  80,  66,  68,  82,  81},
//  {127, 118, 120, 125, 119, 123, 126, 124, 121, 122},
//  {101, 111, 109, 105, 110, 106, 102, 104, 108, 107},
//  { 97, 117, 115,  99, 116, 112,  98, 100, 114, 113},
  
  // CARD 1 pointers
  { 31,  22,  24,  29,  23,  27,  30,  28,  25,  26},
  {  5,  15,  13,   9,  14,  10,   6,   8,  12,  11},
//  {  1,  21,  19,   3,  20,  16,   2,   4,  18,  17},
//  { 63,  54,  56,  61,  55,  59,  62,  60,  57,  58},
//  { 37,  47,  45,  41,  46,  42,  38,  40,  44,  43},
//  { 33,  53,  51,  35,  52,  48,  34,  36,  50,  49},
  

};

///////////////////////////////////////////////////////////////////////////
//
// THIS IS A BUFFER POLARITY MAP
//
// SOME REGISTERS HAVE INVERSE POLARITY
//
///////////////////////////////////////////////////////////////////////////

char BUFFER_POLARITY[] = {
  // card 1
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  
  // card 2
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // card 3
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

///////////////////////////////////////////////////////////////////////////
//
// THIS IS THE DEFAULT DISPLAY BUFFER
//
///////////////////////////////////////////////////////////////////////////

char BUFFER[] = {
  // card 1
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // card 2
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // card 3
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

void setup() {
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(LATCH, OUTPUT);

  //Serial.begin(9600);
}

void clearMap() {
  unsigned idx;
  
  for (idx = 0; idx < BUFFER_SIZE; idx++) {
    BUFFER[idx] = BUFFER_POLARITY[idx]?1:0;
  }
}

void setMap(unsigned digit, unsigned num) { 
  unsigned idx;
  
  idx = DISPLAY_MAP[digit][num];
  BUFFER[idx] = BUFFER_POLARITY[idx]?0:1;
}

void pushMap() {
  unsigned idx;
  for (idx = 0; idx < BUFFER_SIZE; idx++) {
    digitalWrite(CLOCK, LOW);
    delayMicroseconds(10);
    digitalWrite(DATA, BUFFER[BUFFER_SIZE - idx]);
    delayMicroseconds(10);
    digitalWrite(CLOCK, HIGH);
    delayMicroseconds(20);
  }
}

int p = 0;

void writeDigits() {
  unsigned i;
    
  // deassert latch
  digitalWrite(LATCH, HIGH);
  delayMicroseconds(10);

  // clear display buffer
  clearMap();

  //  write digits to buffer
  setMap(0, 6);
  setMap(1, 6);
  setMap(2, 6);
  setMap(3, 6);
  setMap(4, 6);
  setMap(5, 6);
  setMap(6, 6);

  // push buffer to display registers
  pushMap();

  // assert latch (copy registers to latch)
  digitalWrite(LATCH, LOW);
  delayMicroseconds(10);
}


void loop() {
  //Serial.write("transmit\n\r");
  writeDigits();
  p++;
  delay(200);
}
