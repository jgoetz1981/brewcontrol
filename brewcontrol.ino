//PIN DECLARATIONS
int TEMP_SET_LATCH = 30;
int TEMP_READ_LATCH = 31;
int MASH_SET_CLOCK = 32;
int MASH_SET_DATA = 33;
int BOIL_SET_CLOCK = 34;
int BOIL_SET_DATA = 35;
int MASH_READ_CLOCK = 36;
int MASH_READ_DATA = 37;
int BOIL_READ_CLOCK = 38;
int BOIL_READ_DATA = 39;

int MASH_POT = 40;
int MASH_ENABLE = 41;
int BOIL_POT = 42;
int BOIL_ENABLE = 43;

int TEMP_ONE_WIRE = 44;

void initPinModes(){
  pinMode(TEMP_SET_LATCH, OUTPUT);
  pinMode(TEMP_READ_LATCH, OUTPUT);
  pinMode(MASH_SET_CLOCK, OUTPUT);
  pinMode(MASH_SET_DATA, OUTPUT);
  pinMode(BOIL_SET_CLOCK, OUTPUT);
  pinMode(BOIL_SET_DATA, OUTPUT);
  pinMode(MASH_READ_CLOCK, OUTPUT);
  pinMode(MASH_READ_DATA, OUTPUT);
  pinMode(BOIL_READ_CLOCK, OUTPUT);
  pinMode(BOIL_READ_DATA, OUTPUT);
  pinMode(MASH_POT, INPUT);
  pinMode(MASH_ENABLE, INPUT);
  pinMode(BOIL_POT, INPUT);
  pinMode(BOIL_ENABLE, INPUT);
}

//END DECLARATIONS

//VARIABLES
//Used to translate real numbers into 7 segment bytes
byte segmentCounter[10];

//Temperature of the mash tun
int mashTemp;
//Set temp for the mash tun
int mashSetTemp;

//Temperature of the boil kettle
int boilTemp;
//Set temp for the boil kettle
int boilSetTemp;

//SETUP

void setup(){
 Serial.begin(9600); 
 initPinModes();
 fillSegmentCounter();
}

void fillSegmentCounter(){
  segmentCounter[0] = 63;//0,1,2,3,4,5  00111111
  segmentCounter[1]= 6;//1,2 00000110
  segmentCounter[2] = 91;//0,1,3,4,6 001011011
  segmentCounter[3] = 79;//0,1,2,3,6 001001111
  segmentCounter[4] = 102;//1,2,5,6 001100110
  segmentCounter[5] = 109;//0,2,3,5,6 001101101
  segmentCounter[6] = 125;//0,2,3,4,5,6 001111101
  segmentCounter[7] = 7;//0,1,2 000000111
  segmentCounter[8] = 127;//0,1,2,3,4,5,6 001111111
  segmentCounter[9] = 111;//0,1,2,3,5,6 001101111
}

//LOOP

void loop(){
  displayCurrentTemps();
  displaySetTemps();
}

//BEGIN FUNCTIONS

//Functions from loop

/**
Reads the temperatures from the sensors and displays on the 7 segment displays
*/
void displayCurrentTemps(){
   //read Mash temp
  mashTemp = 100;
  //read boil temp
  boilTemp = 100; 
  buildAndWrite32Bit(mashTemp, TEMP_READ_LATCH, MASH_READ_DATA, MASH_READ_CLOCK);
  buildAndWrite32Bit(boilTemp, TEMP_READ_LATCH, BOIL_READ_DATA, BOIL_READ_CLOCK);
}

/**
Checks if the temperateure pots are turned on and if so displays the value set
by the user
*/
void displaySetTemps(){
   //check mash temp enabled
  if(checkTempSet(MASH_ENABLE) == 1){
    
  } else {
    
  }
  //Check boil temp enabled
  if(checkTempSet(BOIL_ENABLE) == 1){
    
  } else {
    
  }
}

// 7-Segment Display Functions
void buildAndWrite32Bit(int toTrans, int latchPin, int dataPin, int clockPin){
  unsigned long toRet = 0;
  int hundreds = int(toTrans/100);
  int tens = int((toTrans%100)/10);
  int ones = (toTrans%10);
  
  byte shiftH = segmentCounter[hundreds] << 1;
  byte shiftT = segmentCounter[tens] << 1;
  byte shiftO = segmentCounter[ones] << 1;

  Serial.print("I: ");
  Serial.println(toTrans);
  Serial.print("hundreds: ");
  Serial.println(shiftH);
  Serial.print("Tens: ");
  Serial.println(shiftT);
  Serial.print("Ones: ");
  Serial.println(shiftO);

  digitalWrite(latchPin,LOW);
  shiftOut(dataPin, clockPin, MSBFIRST,shiftO);
  shiftOut(dataPin, clockPin, MSBFIRST,shiftT);
  shiftOut(dataPin, clockPin, MSBFIRST,shiftH);
  digitalWrite(latchPin, HIGH);
}

boolean checkTempSet(int pinToCheck){
  return true;
}
