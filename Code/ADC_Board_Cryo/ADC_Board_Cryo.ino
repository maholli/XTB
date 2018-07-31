/*

  XTB-20 board configured for a mini breakout (samd21)
  Pinout designed for custom flat cable:
  --------------------------
   Mini Breakout |  XTB  |
  --------------------------
        GND      |  GND   |  
        AREF     |        | 
        3V3      |  3V3   |  
        A3       |        |       
        A2       |  RST   |  
        A1       |  CS    | 
        A0       |  START |  
        13       |  SCK   | 
        12       |  MISO  | 
        11       |  MOSI  | 
                 |  GND   |
  --------------------------

  Operating instructions at:
  https://github.com/maholli/xtb
  Max Holliday & Connor Settle - July 2018


  https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
  https://adafruit.github.io/arduino-board-index/package_adafruit_index.json

  Operate via GUI using PythonGUI.py
  Dump data to text file using serialSave.py
*/

#include <SPI.h>
#define chipSelectPin A1
#define startSync A0
#define resetPin A2
#define LEDPIN 13

bool PGAen = false;
bool startUP = false;
bool resetMe= false;
byte address, thisValue, address2, address3 = 0;
byte readOut0, readOut1, readOut2, readOut3, readOut4, readOut5, readOut6, readOut7, readOut8, readOut9, readOut10, readOut11, readOut12, readOut13, readOut14, readOut15, readOut16, readOut17 = 0;
float lastTemp =0;
int cnt = 0;



/*------------------------------------------------*/
/*---------- varriables you need to set! ---------*/ 
const float refV = 2.5; //reference voltage for the ADC. Usually use internal 2.5V by setting the registers below
const float pgaGain = 1;
const float FSR = (refV*2)/pgaGain;
const float LSBsize = FSR/pow(2,24);
bool showHex = true;
/*------------------------------------------------*/
/*------------------------------------------------*/
bool active = false;
bool current_spinning_mode = false;
int cs_pin_N, cs_pin_E, cs_pin_W, cs_pin_S;

byte data_filter_chop_reg = 0x17;
byte vbias_reg = 0x0;
byte gain_pga_reg = 0x8;



void setup() {
  Serial.begin(115200);
  delay(3);
  Serial.println("Serial Connected");
  pinMode(resetPin,OUTPUT);
  pinMode(chipSelectPin,OUTPUT); 
  pinMode(startSync,OUTPUT);
  pinMode(LEDPIN, OUTPUT); 
  digitalWrite(startSync, LOW);
  digitalWrite(resetPin, HIGH);
  delayMicroseconds(1);
  delay(500);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  
  /* inital startup routine (including reset)*/ 
  delay(100);
  resetADC();
  delay(10);
  lastTemp = readTemp();
     
}




/*------------------------------------------------*/
/*------------------------------------------------*/
void loop() {  
  if (Serial.available() > 0) {
    handleCommand();
  }
  
  if (!active) {
    if (!startUP){
      delay(5000);
      Serial.print("Time(us)"),Serial.print(","),
      Serial.print(" Temperature(C)"),Serial.print(","),
      Serial.print(" Vth(nmos)"),Serial.print(","),Serial.println(" Vth(pmos)"); 
      startUP = true;
    }
      digitalWrite(LEDPIN,HIGH);
      float nn = mosfetV(0x04);
      delay(10);
      float pp = mosfetV(0x05);
      delay(10);
      digitalWrite(LEDPIN,LOW);
      lastTemp = readTemp();
      Serial.print(micros()),Serial.print(","),
      Serial.print(lastTemp,2),Serial.print(","),
      Serial.print(nn,4),Serial.print(","),Serial.println(pp,4);
      
  }
  
//  hallSpin(50); 
//  if (active) {
//    if (current_spinning_mode) {
//        hallSpin(25);      
//    } else {
//      readData1(showHex = false, 1000, true);
//      delay(75);
//    }
//  }
}
/*------------------------------------------------*/
/*------------------------------------------------*/

void hallSpin2(int dTime) {
//  N
//W + E
//  S
  /* HALL SPIN -- phase 1 --*/
//  float tPhaseTot = micros();
//  float tPhase1 = micros();
  byte inByteA, inByteB, inByteC, inByteD, inByteE, inByteF, inByteG, inByteH, inByteI, inByteJ, inByteK, inByteL, inByteM = 0;
  byte inByteA2, inByteB2, inByteC2, inByteD2, inByteE2, inByteF2, inByteG2, inByteH2, inByteI2, inByteJ2, inByteK2, inByteL2, inByteM2 = 0;
  float decVal_A, decVal_B, decVal_C, decVal_D = 0;
  float decVal_A2, decVal_B2, decVal_C2, decVal_D2 = 0;
  delay(dTime);
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(0x31);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x9C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x03);   //0x46  IDACMAG
  SPI.transfer(0xF2);   //0x47  IDACMUX
  SPI.transfer(0x01);   //0x48  VBIAS
//  SPI.transfer(0x19);   //system calibration
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(5);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteA = SPI.transfer(0x00);
  inByteB = SPI.transfer(0x00);
  inByteC = SPI.transfer(0x00);
  delay(5);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x01);   //how many registers to write to
  SPI.transfer(0x20);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  delay(5);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteA2 = SPI.transfer(0x00);
  inByteB2 = SPI.transfer(0x00);
  inByteC2 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  decVal_A = dataConvert(inByteA,inByteB,inByteC);
  decVal_A2 = dataConvert(inByteA2,inByteB2,inByteC2);
  printTimeStamp();
  Serial.print(",");
  Serial.print(decVal_A,DEC),Serial.print(",");;
  Serial.println(decVal_A2,DEC);
}


void hallSpin(int dTime) {
//  N
//W + E
//  S
  /* HALL SPIN -- phase 1 --*/
//  float tPhaseTot = micros();
//  float tPhase1 = micros();
  byte inByteA, inByteB, inByteC, inByteD, inByteE, inByteF, inByteG, inByteH, inByteI, inByteJ, inByteK, inByteL, inByteM = 0;
  byte inByteA2, inByteB2, inByteC2, inByteD2, inByteE2, inByteF2, inByteG2, inByteH2, inByteI2, inByteJ2, inByteK2, inByteL2, inByteM2 = 0;
  float decVal_A, decVal_B, decVal_C, decVal_D = 0;
  float decVal_A2, decVal_B2, decVal_C2, decVal_D2 = 0;
  delay(dTime);
  
//  if (cnt % 50 == 0){
//    lastTemp = readTemp();
//  }
/* ------- HALL SPIN -- phase 1 ------- */
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(0x31);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x0C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x04);   //0x46  IDACMAG
  SPI.transfer(0xF2);   //0x47  IDACMUX
  SPI.transfer(0x01);   //0x48  VBIAS
//  SPI.transfer(0x19);   //system calibration
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(50);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteA = SPI.transfer(0x00);
  inByteB = SPI.transfer(0x00);
  inByteC = SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x01);   //how many registers to write to
  SPI.transfer(0x20);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  delay(5);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteA2 = SPI.transfer(0x00);
  inByteB2 = SPI.transfer(0x00);
  inByteC2 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  decVal_A = dataConvert(inByteA,inByteB,inByteC);
  decVal_A2 = dataConvert(inByteA2,inByteB2,inByteC2);
  delay(1);  
  /* HALL SPIN -- phase 4 --*/
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);   
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(0x20);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x0C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x04);   //0x46  IDACMAG
  SPI.transfer(0xF1);   //0x47  IDACMUX
  SPI.transfer(0x08);   //0x48  VBIAS
//  SPI.transfer(0x19);   //system calibration
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(50);
  SPI.transfer(0x00);   //NOP to get rid of junk?
  SPI.transfer(0x12); //transfer read command  
  inByteD = SPI.transfer(0x00);
  inByteE = SPI.transfer(0x00);
  inByteF = SPI.transfer(0x00);
  delay(5);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x01);   //how many registers to write to
  SPI.transfer(0x13);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  delay(50);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteD2 = SPI.transfer(0x00);
  inByteE2 = SPI.transfer(0x00);
  inByteF2 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  decVal_B = dataConvert(inByteD,inByteE,inByteF);
  decVal_B2 = dataConvert(inByteD2,inByteE2,inByteF2);
  delay(1);
 /* HALL SPIN -- phase 2 --*/
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(0x02);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x0C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x04);   //0x46  IDACMAG
  SPI.transfer(0xF3);   //0x47  IDACMUX
  SPI.transfer(0x02);   //0x48  VBIAS
//  SPI.transfer(0x19);   //system calibration
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(50);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteH = SPI.transfer(0x00);
  inByteI = SPI.transfer(0x00);
  inByteJ = SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x01);   //how many registers to write to
  SPI.transfer(0x13);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  delay(5);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteH2 = SPI.transfer(0x00);
  inByteI2 = SPI.transfer(0x00);
  inByteJ2 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  decVal_C = dataConvert(inByteH,inByteI,inByteJ);
  decVal_C2 = dataConvert(inByteH2,inByteI2,inByteJ2);
  delay(1);
 /* HALL SPIN -- phase 3 --*/
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(0x13);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x0C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x04);   //0x46  IDACMAG
  SPI.transfer(0xF0);   //0x47  IDACMUX
  SPI.transfer(0x04);   //0x48  VBIAS
//  SPI.transfer(0x19);   //system calibration
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(50);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteK = SPI.transfer(0x00);
  inByteL = SPI.transfer(0x00);
  inByteM = SPI.transfer(0x00);
  delay(5);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x01);   //how many registers to write to
  SPI.transfer(0x02);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  delay(5);
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  inByteK2 = SPI.transfer(0x00);
  inByteL2 = SPI.transfer(0x00);
  inByteM2 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  decVal_D = dataConvert(inByteK,inByteL,inByteM);
  decVal_D2 = dataConvert(inByteK2,inByteL2,inByteM2);
      
  float dataSpin = ((decVal_A+decVal_C+decVal_D+decVal_B)/4)*1000; //output in mV
  float vApplied = ((abs(decVal_A2)+abs(decVal_C2)+abs(decVal_D2)+abs(decVal_B2))/4)*1000; //output in mV
  
  printTimeStamp();
  Serial.print(",");
//  Serial.print(lastTemp,2), Serial.print(","),
  Serial.print(decVal_A,8),Serial.print(","),
  Serial.print(decVal_B,8),Serial.print(","),
  Serial.print(decVal_C,8),Serial.print(","),
  Serial.print(decVal_D,8),Serial.print(","),
  Serial.print(vApplied,8),Serial.print(","),
  Serial.println(dataSpin, DEC);
  cnt++; 
}
/*------------------------------------------------*/


void handleCommand() {
  String message = "";
  while (Serial.available() > 0) {
    byte inByte = Serial.read();
    message += (char)inByte;
  }
  message = message.substring(0, message.length() - 2);
  //Serial.println(message);
  String argv[7];
  parseMessage(message, argv);

  if (argv[0] == "test") {
    //Serial.println("Recieved");
    delay(3000);
  } else if (argv[0] == "stop") {
    stopADC();
  } else if (argv[0] == "start") {
    startADC();
  } else if (argv[0] == "reset") {
    resetADC();
  } else if (argv[0] == "readout") {
    regReadout();
    delay(5000);
  } else if (argv[0] == "setinputpins") {
    int aaa, bbb;
    if (argv[1] == "COM") {
      aaa = 12;
    } else {
      aaa = argv[1].toInt();
    }
    if (argv[2] == "COM") {
      bbb = 12;
    } else {
      bbb = argv[2].toInt();
    }
    setInputMUX(aaa, bbb);
  } else if (argv[0] == "setdatarate") {
    setDataRate(argv[1].toInt());
  } else if (argv[0] == "setgain") {
    setGain(argv[1].toInt());
  } else if (argv[0] == "setidacpins") {
    int a, b, c;
    if (argv[1] == "Off") {
      a = -1;
    } else {
      a = argv[1].toInt();
    }
    if (argv[2] == "Off") {
      b = -1;
    } else {
      b = argv[2].toInt();
    }
    if (argv[3] == "Off") {
      c = 0;
    } else {
      c = argv[3].toInt();
    }
    setIDAC(a, b, c);
  } else if (argv[0] == "setpga") {
    if (argv[1] == "Bypassed") {
      setPGA(0);
    } else if (argv[1] == "IP_Buffer_Bypassed") {
      setPGA(2);
    } else {
      setPGA(1);
    }
  } else if (argv[0] == "setfilter") {
    if (argv[1] == "SINC3") {
      setFilter(false);
    } else {
      setFilter(true);
    }
  } else if (argv[0] == "setchop") {
    if (argv[1] == "On") {
      setChop(true);
    } else {
      setChop(false);
    }
  } else if (argv[0] == "setvbias") {
    if (argv[1] == "1.65V") {
      vbias_reg |= 0x80;
    } else {
      vbias_reg &= 0x7f;
    }
    writeReg(0x48, vbias_reg);
  } else if (argv[0] == "setvbiaspins") {
    setVbiasPins(argv[1].toInt(), argv[2].toInt(), argv[3].toInt(), argv[4].toInt(), argv[5].toInt(), argv[6].toInt());
  } else if (argv[0] == "setreadmode") {
    if (argv[1] == "currentspin") {
      current_spinning_mode = true;
    } else {
      current_spinning_mode = false;
    }
  } else if (argv[0] == "setcspins") {
    cs_pin_N = argv[1].toInt();
    cs_pin_E = argv[2].toInt();
    cs_pin_W = argv[3].toInt();
    cs_pin_S = argv[4].toInt();
  } else if (argv[0] == "systest") {
    sysTest();
  } else if (argv[0] == "readtemp") {
    readTemp();
  }
}

float dataConvert( byte a, byte b, byte c){
  int rawDataIN = 0; //create an empty 24 bit integer for the data
  float dataOUT = 0;
  rawDataIN |= a; //shift the data in one byte at a time
  rawDataIN = (rawDataIN << 8) | b;
  rawDataIN = (rawDataIN << 8) | c;
  if (((1 << 23) & rawDataIN) != 0){ //If the rawData has bit 24 on, then it's a negative value
    int maskIN = (1 << 24) - 1;
    rawDataIN = ((~rawDataIN) & maskIN) + 1;
    dataOUT = rawDataIN * LSBsize * -1;}
  else{ //if it's not negative
    dataOUT = float(rawDataIN)*LSBsize;} //then just multiply by LSBsize
  return dataOUT;
}

float mosfetV(byte pinNum){
  byte mosDat1, mosDat2, mosDat3 = 0;
  byte IDACpin = pinNum | 0xF0;
  pinNum |= 0xC0;

  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);   
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(pinNum); //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x0C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x04);   //0x46  IDACMAG
  SPI.transfer(IDACpin);//0x47  IDACMUX
  SPI.transfer(0x00);   //0x48  VBIAS
//  SPI.transfer(0x19);   //system calibration
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(5);
  SPI.transfer(0x00);   //NOP to get rid of junk?
  SPI.transfer(0x12); //transfer read command  
  mosDat1 = SPI.transfer(0x00);
  mosDat2 = SPI.transfer(0x00);
  mosDat3 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  float mosData = -1*dataConvert(mosDat1,mosDat2,mosDat3);
  return mosData;
}

void sysTest(){
  Serial.println("");
  delay(20); 
  setInputMUX(5,6);
  delay(20);  
  writeReg(0x49, 0x30); //inputs shorted to mid-supply
  delay(50);  
  if (active) {
    for (int j=1; j<=50; j++){
      readData1(showHex = false, 1000, true);
      delay(75);    
    }
  }
  writeReg(0x49, 0x10);
  delay(20); 
  setInputMUX(12,12);
  delay(20);
  Serial.println("");
}


float readTemp() {
  byte aa, bb, cc = 0;
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);   
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x07);   //how many registers to write to
  SPI.transfer(0xCC);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x15);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x00);   //0x46  IDACMAG
  SPI.transfer(0xFF);   //0x47  IDACMUX
  SPI.transfer(0x00);   //0x48  VBIAS
  SPI.transfer(0x50);   //0x49  SYS
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(50);
  SPI.transfer(0x00);   //send NOPS
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  aa = SPI.transfer(0x00);
  bb = SPI.transfer(0x00);
  cc = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);  
  float temp = dataConvert(aa, bb, cc)*1000.0;
  float dd = 0;
  Serial.print("Temperature: ");
  if (temp < 129){
    Serial.print(temp,DEC),Serial.print("  ");
    dd = ((-1*(129.00-temp)*0.403)+25);
    Serial.print(dd, 2), Serial.println(" degrees C");
  }
  else {
    Serial.print(temp,DEC),Serial.print("  ");
    dd = ((-1*(129.00-temp)*0.403)+25);
    Serial.print(dd, 2), Serial.println(" degrees C");
}
  delay(1);  
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);   
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x07);   //how many registers to write to
  SPI.transfer(0xCC);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x0C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x00);   //0x46  IDACMAG
  SPI.transfer(0xFF);   //0x47  IDACMUX
  SPI.transfer(0x00);   //0x48  VBIAS
  SPI.transfer(0x00);   //0x49  SYS
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  delay(1);
  return dd;
}


void parseMessage(String msg, String arg[]) {
  int index = 0;

  int wordIndex = 0;
  int wordStart = 0;
  while (index < msg.length()) {
    if (msg.charAt(index) == ' ') {
      arg[wordIndex] = msg.substring(wordStart, index);
      //Serial.println(arg[wordIndex]);
      wordStart = index + 1;
      wordIndex++;
    }
    index++;
  }
  arg[wordIndex] = msg.substring(wordStart, index);
}


void setDataRate(int sps) {
  byte val = 0x0;
  
  if (sps == 60) {
    val = 0x16;
  } else if (sps == 100) {
    val = 0x17;
  } else if (sps == 200) {
    val = 0x18;
  } else if (sps == 400) {
    val = 0x19;
  } else if (sps == 800) {
    val = 0x1a;
  } else if (sps == 1000) {
    val = 0x1b;
  } else if (sps == 2000) {
    val = 0x1c;
  } else if (sps == 4000) {
    val = 0x1d;
  }

  data_filter_chop_reg &= 0xf0;
  data_filter_chop_reg |= val;
  writeReg(0x44, data_filter_chop_reg);
  //regReadout();
}

void setChop(bool c) {
  if (c) {
    data_filter_chop_reg |= 0x80;
  } else {
    data_filter_chop_reg &= 0x7f;
  }
  writeReg(0x44, data_filter_chop_reg);
}

void setFilter(bool latency) {
  if (latency) {
    data_filter_chop_reg |= 0x10;
  } else {
    data_filter_chop_reg &= 0xef;
  }
  writeReg(0x44, data_filter_chop_reg);
}

void setVbiasPins(int a, int b, int c, int d, int e, int f) {
  byte val = 0x0;
  val |= (a | (b << 1) | (c << 2) | (d << 3) | (e << 4) | (f << 5));
  vbias_reg &= 0x80;
  vbias_reg |= val;
  writeReg(0x48, vbias_reg);
}

void setIDAC(int a, int b, int c) {
  int AINx, AINy;
    if (a == -1) {
      AINx = 15;
    } else {
      AINx = a;
    }
    if (b == -1) {
      AINy = 15;
    } else {
      AINy = b;
    }
    
    byte val;
    val |= AINy;
    val |= (AINx << 4);
    writeReg(0x47, val);
    
    val = 0x0;
    if (c == 10) {
      val = 0x1;
    } else if (c == 50) {
      val = 0x2;
    } else if (c == 100) {
      val = 0x3;
    } else if (c == 250) {
      val = 0x4;
    } else if (c == 500) {
      val = 0x5;
    } else if (c == 750) {
      val = 0x6;
    } else if (c == 1000) {
      val = 0x7;
    } else if (c == 1500) {
      val = 0x8;
    } else if (c == 2000) {
      val = 0x9;
    }
    
    writeReg(0x46, val);
}

void setPGA(int mode) {
  gain_pga_reg &= 0xe7;
  gain_pga_reg |= (mode << 3);
  writeReg(0x43, gain_pga_reg);
}

void setGain(int g) {
  byte val = 0x0;
  switch (g) {
    case 1:
      val = 0x0;
      break;
    case 2:
      val = 0x1;
      break;
    case 4:
      val = 0x2;
      break;
    case 8:
      val = 0x3;
      break;
    case 16:
      val = 0x4;
      break;
    case 32:
      val = 0x5;
      break;
    case 64:
      val = 0x6;
      break;
    case 128:
      val = 0x7;
      break;
  }

  gain_pga_reg &= 0xf8;
  gain_pga_reg |= val;
  writeReg(0x43, gain_pga_reg);
}

void setInputMUX(int AINx, int AINy) {
  byte val;
  val |= AINy;
  val |= (AINx << 4);
  writeReg(0x42, val);
}

void hallVbias(int pin) {
  byte val = (1 << pin);
  writeReg(0x48, val);
}

/*Start ADC with command + SYNC pin --- WORKING ---*/
void startADC() {
  //Serial.println("---Starting ADC---");
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x0A); //send stop byte
  SPI.transfer(0x08); //send start byte
//  digitalWrite(startSync, LOW);
//  delay(4*1/10000000);
//  digitalWrite(startSync, HIGH);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  active = true;
}

/*Stop ADC with command + SYNC pin --- UNKNOWN ---*/
void stopADC() {
  //Serial.println("---Stopping ADC---");
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x0A); //send stop byte
//  digitalWrite(startSync, LOW);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  active = false;
}

/*Configure current excitation source --- WORKING ---*/
void IDAC(bool setIDAC, byte valIDAC, byte pinIDAC){
  if(setIDAC == 1){
    digitalWrite(chipSelectPin, LOW);
    delayMicroseconds(1);
    SPI.transfer(0x46);   //Send register START location
    SPI.transfer(0x01);   //how many registers to write to
    SPI.transfer(valIDAC);   //set 0x46 register to ENABLE @ 100uA
    SPI.transfer(pinIDAC);   //set register to specified value
    delay(1);
    digitalWrite(chipSelectPin, HIGH); 
    //Serial.println("---IDAC ENABLED----"); 
    }
  else{
    digitalWrite(chipSelectPin, LOW);
    delayMicroseconds(1);
    SPI.transfer(0x46);   //Send register START location
    SPI.transfer(0x01);   //how many registers to write to
    SPI.transfer(0x00);   //set 0x46 register to DISABLE
    SPI.transfer(0xFF);   //set 0x47 register to DISCONNECT all IDAC pins
    delay(1);
    digitalWrite(chipSelectPin, HIGH);
    //Serial.println("---IDAC DISABLED----"); 
    }
}



/*Reset ADC with command + reset pin --- WORKING ---*/
void resetADC() {
  //Serial.println("---Resetting ADC---");
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x06); //send reset byte
  digitalWrite(resetPin, LOW);
  delayMicroseconds(1);
  digitalWrite(resetPin, HIGH);
  delay(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x07);   //how many registers to write to
  SPI.transfer(0xCC);   //0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x1D);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x00);   //0x46  IDACMAG
  SPI.transfer(0xFF);   //0x47  IDACMUX
  SPI.transfer(0x00);   //0x48  VBIAS
  SPI.transfer(0x10);   //0x49  SYS
//  SPI.transfer(0x19);   //system calibration
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  delay(20);
  digitalWrite(chipSelectPin, HIGH);
  delay(1000); 
}
void printTimeStamp() {
  //TODO: don't use millis
  Serial.print(micros());
}
/*Read 24 bit data from ADC --- WORKING ---*/
float readData1(bool showHex, int scalar, bool printData) { //read the ADC data when STATUS and CRC bits are NOT enabled
//  Serial.print("Data Read: ");
  byte inByte1, inByte2, inByte3, inByte4, inByte5, inByte6 = 0;
  int result;
  float decVal = 0;
  /*Read the three bytes of 2's complement data from the ADC*/ 
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x00);  
  SPI.transfer(0x12); //transfer read command  
  inByte1 = SPI.transfer(0x00);
  inByte2 = SPI.transfer(0x00);
  inByte3 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);

  /*Print the HEX value (if showHex = true)*/
  if (showHex == 1){
    //Serial.print("HEX Value: ");
    Serial.print(inByte1,HEX), Serial.print(inByte2,HEX), Serial.println(inByte3,HEX);
  }
  
  /*Convert the three bytes into readable data*/
  float dataNEW = dataConvert(inByte1,inByte2,inByte3);
  if (printData) {
    printTimeStamp();
    Serial.print(",");
    Serial.println(dataNEW, DEC);
  }
  return dataNEW;
}

/*Write register value --- WORKING ---*/
void writeReg(byte address, byte thisValue) {
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(address); //Send register location
  SPI.transfer(0x00);
  SPI.transfer(thisValue);  //Send value to write 
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  address = 0;
  thisValue = 0;
}

/*Read register value --- UNKNOWN ---*/
void readReg(byte address2) {  
  byte inByte = 0x00;
  byte result = 0x00;
  byte rawRegVal = 0x00;
  Serial.print(address2, HEX);
  Serial.print("\t");
  digitalWrite(chipSelectPin, LOW); 
  delayMicroseconds(1);
  SPI.transfer(address2);
  inByte = SPI.transfer(0x00);
  result = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  rawRegVal = (rawRegVal << 8 | result);
  Serial.print(rawRegVal, HEX);
  Serial.print("\t");
  Serial.println(result, HEX);
}


/*Read ALL of the registers in a row --- WORKING ---*/
void regReadout(){
  Serial.println("------Register Readout-------");
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x20);   //Send register START location
  SPI.transfer(0x12);   //how many registers we want to read (0x12 = all 18)
  readOut0 = SPI.transfer(0x00);
  readOut1 = SPI.transfer(0x00);
  readOut2 = SPI.transfer(0x00);
  readOut3 = SPI.transfer(0x00);
  readOut4 = SPI.transfer(0x00);
  readOut5 = SPI.transfer(0x00);
  readOut6 = SPI.transfer(0x00);
  readOut7 = SPI.transfer(0x00);
  readOut8 = SPI.transfer(0x00);
  readOut9 = SPI.transfer(0x00);
  readOut10 = SPI.transfer(0x00);
  readOut11 = SPI.transfer(0x00);
  readOut12 = SPI.transfer(0x00);
  readOut13 = SPI.transfer(0x00);
  readOut14 = SPI.transfer(0x00);
  readOut15 = SPI.transfer(0x00);
  readOut16 = SPI.transfer(0x00);
  readOut17 = SPI.transfer(0x00);  
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("Register 0x00 (ID):        "), Serial.println(readOut0, HEX);
  Serial.print("Register 0x01 (STATUS):    "), Serial.println(readOut1, HEX);
  Serial.print("Register 0x02 (INPMUX):    "), Serial.println(readOut2, HEX);
  Serial.print("Register 0x03 (PGA):       "), Serial.println(readOut3, HEX);
  Serial.print("Register 0x04 (DATARATE):  "), Serial.println(readOut4, HEX);
  Serial.print("Register 0x05 (REF):       "), Serial.println(readOut5, HEX);
  Serial.print("Register 0x06 (IDACMAG):   "), Serial.println(readOut6, HEX);
  Serial.print("Register 0x07 (IDACMUX):   "), Serial.println(readOut7, HEX);
  Serial.print("Register 0x08 (VBIAS):     "), Serial.println(readOut8, HEX);
  Serial.print("Register 0x09 (SYS):       "), Serial.println(readOut9, HEX);
  Serial.print("Register 0x0A (OFCAL0):    "), Serial.println(readOut10, HEX);
  Serial.print("Register 0x0B (OFCAL1):    "), Serial.println(readOut11, HEX);
  Serial.print("Register 0x0C (OFCAL2):    "), Serial.println(readOut12, HEX);
  Serial.print("Register 0x0D (FSCAL0):    "), Serial.println(readOut13, HEX);
  Serial.print("Register 0x0E (FSCAL1):    "), Serial.println(readOut14, HEX);
  Serial.print("Register 0x0F (FSCAL2):    "), Serial.println(readOut15, HEX);
  Serial.print("Register 0x10 (GPIODAT):   "), Serial.println(readOut16, HEX);
  Serial.print("Register 0x11 (GPIOCON):   "), Serial.println(readOut17, HEX);
  Serial.println("-----------------------------");
}

/*Initiate Self Calibration --- WORKING (kinda?) ---*/
void SFOCAL() {
//  Serial.println("Self Calibration");
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x19); //send self offset command
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  delay(100);
}

