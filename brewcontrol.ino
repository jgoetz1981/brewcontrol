#include <OneWire.h>

//***PROGRAM CONSTANTS***
//This is the max number of devices that the program will support.  Just to simplify adding more in the future
int MAX_ONEWIRE_DEVICES = 3;

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

int MASH_POT = A0;
int MASH_ENABLE = 41;
int BOIL_POT = A1;
int BOIL_ENABLE = 43;

int TEMP_ONE_WIRE = 44;
/**
Sets the pin modes of the various pin constants to their appropriate
value
*/
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

//END PIN DECLARATIONS

//VARIABLES
//Used to translate real numbers into 7 segment bytes
byte segmentCounter[10];

/*The minimum and maximum values that the pots used for controling temperature
set points will return
*/
int MIN_POT_VALUE = 0;
int MAX_POT_VALUE = 1023;

/*The minimum and maximum settable temperatures for the mash tun*/
int MASH_TUN_MINIMUM = 60;
int MASH_TUN_MAXIMUM = 170;

/*The minimum and maximum settable temperatures for the boil kettle*/
int BOIL_KETTLE_MINIMUM = 50;
int BOIL_KETTLE_MAXIMUM = 212;

//The byte value for --- on a 3 digit 7 segment display
byte noValue = 128; //Just segment 6 10000000

//The unique ID of the boil kettle one wire sensor
//40,95,253,126,3,0,0,7
byte BOIL_TEMP_SENSOR[8] = {0x28,0x5f,0xfd,0x7e,0x3,0x0,0x0,0x7};
//The unique ID of the mash temp sensor;
//40,46,214,245,5,0,0,188
byte MASH_TEMP_SENSOR[8] = {0x28,0x2e,0xd6,0xf5,0x5,0x0,0x0,0xbc};
//The unique ID of the HLT one wire sensor
//40,95,253,126,3,0,0,7
byte HLT_TEMP_SENSOR[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

//THe number of miliseconds to delay while converting temps
int TEMP_CONVERSION_DELAY = 1000;

/**************************
***END CONSTANTS***********
**************************/
//Temperature of the HLT
int hltTemp;
//Temperature of the mash tun
int mashTemp;
//Set temp for the mash tun
int mashSetTemp;

//Temperature of the boil kettle
int boilTemp;
//Set temp for the boil kettle
int boilSetTemp;

//Onewire heler class
OneWire tempHelper(TEMP_ONE_WIRE);

//Is the HLT temperature sensor found?
boolean hltTempEnabled = false;
//Is the Mash temperature sensor found?
boolean mashTempEnabled = false;
//Is the Boil kettle temperature sensor found?
boolean boilTempEnabled = false;


//SETUP
void setup(){
 Serial.begin(9600); 
 initPinModes();
 fillSegmentCounter();
 verifyTempSensors();
}

//LOOP
void loop(){
  displayCurrentTemps();
  displaySetTemps();
  delay(500);
}

//BEGIN FUNCTIONS

//SETUP FUNCTIONS

/**
Sets the values in the segement counter
*/
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


/**
Looks for the one wire devices hooked up and turns on the ones that its expeciting
*/
void verifyTempSensors(){
   tempHelper.reset();
  byte devices[MAX_ONEWIRE_DEVICES][8];
  int deviceCount = findDevices(devices);
  for(int i = 0; i < deviceCount; i++){
    if(!hltTempEnabled && checkIfDeviceMatches(devices[i],HLT_TEMP_SENSOR)){
      hltTempEnabled = true;
    } else if(!mashTempEnabled && checkIfDeviceMatches(devices[i], MASH_TEMP_SENSOR)){
      mashTempEnabled = true;
    } else if(!boilTempEnabled && checkIfDeviceMatches(devices[i], BOIL_TEMP_SENSOR)){
     boilTempEnabled = true;
    }
  }
}

boolean checkIfDeviceMatches(byte readDevice[8], byte toCheck[8]){
  boolean toRet = true;
  for(int i = 0; i < 8 && toRet; i++){
    if(readDevice[i] != toCheck[i]){
      toRet = false;
    }
  }
  return toRet;
}

int findDevices(byte devices[][8]){
  int finished = 1;
  int deviceCount = 0;
  for(int i = 0; finished == 1; i++){
    finished = tempHelper.search(devices[i]);
    if(deviceCount == 1){
      deviceCount++;
    }
  }
  return deviceCount;
}


//Functions from loop

/**
Reads the temperatures from the sensors and displays on the 7 segment displays
*/
void displayCurrentTemps(){
  readTemperatures();
  if(mashTempEnabled){
    buildAndWrite32Bit(mashTemp, TEMP_READ_LATCH, MASH_READ_DATA, MASH_READ_CLOCK);
  }
  if(boilTempEnabled){
    buildAndWrite32Bit(boilTemp, TEMP_READ_LATCH, BOIL_READ_DATA, BOIL_READ_CLOCK);
  }
}

/**
Checks if the temperateure pots are turned on and if so displays the value set
by the user
*/
void displaySetTemps(){
   //check mash temp enabled
  if(checkTempSet(MASH_ENABLE) == 1){
    displayTemp(MASH_TUN_MINIMUM, MASH_TUN_MAXIMUM, MASH_POT, TEMP_SET_LATCH, MASH_SET_DATA, MASH_SET_CLOCK);
  } else {
    displayTemp(-1, -1, -1, TEMP_SET_LATCH, MASH_SET_DATA, MASH_SET_CLOCK);
  }
  //Check boil temp enabled
  if(checkTempSet(BOIL_ENABLE) == 1){
    displayTemp(BOIL_KETTLE_MINIMUM, BOIL_KETTLE_MAXIMUM, BOIL_POT, TEMP_SET_LATCH, BOIL_SET_DATA, BOIL_SET_CLOCK);
  } else {
    displayTemp(-1, -1, -1, TEMP_SET_LATCH, BOIL_SET_DATA, BOIL_SET_CLOCK);
  }
}

// 7-Segment Display Functions

/**
toTrans - an integeger from 0-999 to display
latchPin - the latch pin corresponding to the shift register array that you want to display on
dataPin the dataPin to feed bits to
clockPin the pin connected to the clock for your shift registers
*/
void buildAndWrite32Bit(int toTrans, int latchPin, int dataPin, int clockPin){
  unsigned long toRet = 0;
  int hundreds = int(toTrans/100);
  int tens = int((toTrans%100)/10);
  int ones = (toTrans%10);
  
  byte shiftH = segmentCounter[hundreds] << 1;
  byte shiftT = segmentCounter[tens] << 1;
  byte shiftO = segmentCounter[ones] << 1;

  digitalWrite(latchPin,LOW);
  shiftOut(dataPin, clockPin, MSBFIRST,shiftO);
  shiftOut(dataPin, clockPin, MSBFIRST,shiftT);
  shiftOut(dataPin, clockPin, MSBFIRST,shiftH);
  digitalWrite(latchPin, HIGH);
}

void writeNoData(int latchPin, int dataPin, int clockPin){
  digitalWrite(latchPin,LOW);
  shiftOut(dataPin, clockPin, MSBFIRST,noValue);
  shiftOut(dataPin, clockPin, MSBFIRST,noValue);
  shiftOut(dataPin, clockPin, MSBFIRST,noValue);
  digitalWrite(latchPin, HIGH);
}

/**
Checks if the switch connected to pinToCheck is closed or open
Closed is true, Open is false
*/
boolean checkTempSet(int pinToCheck){
  return digitalRead(pinToCheck);
}

/**
Displays the temp given on the pins given
*/
void displayTemp(int minTemp, int maxTemp, int potPin,  int latch, int data, int clock){
  if(potPin != -1){
    int potValue = analogRead(potPin);
    float percentage = potValue/float(MAX_POT_VALUE);
    //Max temp - mintemp gives us a 0-X range, which we can multiply by the percentage to find the actual value when we re-add minTemp
    int temp = ((maxTemp - minTemp) * percentage) + minTemp;
    buildAndWrite32Bit(temp, latch, data, clock);
  } else {
    writeNoData(latch, data, clock);
  }
}

/**
Reads the temperatures from the one wire devices
*/
void readTemperatures(){
  tempHelper.reset();
  tempHelper.write(0xCC);
  tempHelper.write(0x44);
  delay(TEMP_CONVERSION_DELAY);
  if(hltTempEnabled){
    readTemp(HLT_TEMP_SENSOR, &hltTemp);
  }
  if(mashTempEnabled){
    readTemp(MASH_TEMP_SENSOR, &mashTemp);
  }
  if(boilTempEnabled){
    readTemp(BOIL_TEMP_SENSOR, &boilTemp);
  }
}

void readTemp(byte sensor[], int* tempVar){
  tempHelper.reset();
  tempHelper.select(sensor);
  tempHelper.write(0xBE);
  byte data[12];
  for(int i = 0; i < 9; i++){
    data[i] = tempHelper.read();
  }
  byte MSB = data[1];
  byte LSB = data[0];
  int tempRead = ((MSB << 8) | LSB);
  *tempVar = covertCToF(tempRead/16);
}

int covertCToF(int C){
 return ((int)(C*1.8) + 32);
}

