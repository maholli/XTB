//XTB-20 board configured for a mini breakout (samd21)

#include <SPI.h>
#define chipSelectPin 3
#define startSync 9
#define resetPin 8

bool PGAen = false;
bool startUP = false;
bool resetMe= false;
byte address, thisValue, address2, address3 = 0;
byte readOut0, readOut1, readOut2, readOut3, readOut4, readOut5, readOut6, readOut7, readOut8, readOut9, readOut10, readOut11, readOut12, readOut13, readOut14, readOut15, readOut16, readOut17 = 0;
byte inByte1, inByte2, inByte3, inByte4, inByte5, inByte6 = 0;


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
  SerialUSB.begin(115200);
  delay(3);
  SerialUSB.println("Serial Connected");
  pinMode(resetPin,OUTPUT);
  pinMode(chipSelectPin,OUTPUT); 
  pinMode(startSync,OUTPUT); 
  digitalWrite(startSync, LOW);
  digitalWrite(resetPin, HIGH);
  delayMicroseconds(1);
  delay(500);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  
  /* inital startup routine (including reset)*/ 
  delay(100);
  resetADC();
  
}




/*------------------------------------------------*/
/*------------------------------------------------*/
void loop() {  
  if (SerialUSB.available() > 0) {
    handleCommand();
  }

  if (active) {
    hallSpin(10);
  }
  
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





void hallSpin(int dTime) {
//  N
//W + E
//  S

  /* HALL SPIN -- phase 1 --*/
//  float tPhaseTot = micros();
//  float tPhase1 = micros();
  delay(dTime);
  float rDataA = 0;
  float decVal_1 = 0;
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(0x13);   //0x42  INPMUX 
  SPI.transfer(0xE8);   //0x43  PGA
  SPI.transfer(0x1C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x03);   //0x46  IDACMAG
  SPI.transfer(0xF2);   //0x47  IDACMUX
  SPI.transfer(0x81);   //0x48  VBIAS
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  SPI.transfer(0x25);   //RREG trick to force DRDY high
  delay(1);
  digitalWrite(chipSelectPin, HIGH); 
  delay(5);
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1); 
  if (digitalRead(12)==0){
    SPI.transfer(0x12); //transfer read command  
    inByte1 = SPI.transfer(0x00);
    inByte2 = SPI.transfer(0x00);
    inByte3 = SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x25);   //RREG trick to force DRDY high
    delay(1);
    digitalWrite(chipSelectPin, HIGH);
  }
  else{
    SerialUSB.println("conversion error");
    delay(1);
    digitalWrite(chipSelectPin, HIGH);
  }
  /* convert data from phase 1 --*/
  int rawData1 = 0; //create an empty 24 bit integer for the data
  rawData1 |= inByte1; //shift the data in one byte at a time
  rawData1 = (rawData1 << 8) | inByte2;
  rawData1 = (rawData1 << 8) | inByte3;
  if (((1 << 23) & rawData1) != 0){ //If the rawData has bit 24 on, then it's a negative value
    int mask = (1 << 24) - 1;
    rawData1 = ((~rawData1) & mask) + 1;
    decVal_1 = rawData1 * LSBsize * -1;}
  else{ //if it's not negative
    decVal_1 = float(rawData1)*LSBsize;} //then just multiply by LSBsize
  delay(50);
  
  /* HALL SPIN -- phase 2 --*/
  float decVal_2 = 0;
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);   
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(0x02);   //0x42  INPMUX 
  SPI.transfer(0xE8);   //0x43  PGA
  SPI.transfer(0x1C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(0x03);   //0x46  IDACMAG
  SPI.transfer(0xF3);   //0x47  IDACMUX
  SPI.transfer(0x82);   //0x48  VBIAS
  SPI.transfer(0x0A);   //send stop byte
  SPI.transfer(0x08);   //send start byte
  SPI.transfer(0x25);   //RREG trick to force DRDY high
  delay(1);
  digitalWrite(chipSelectPin, HIGH); 
  delay(5);
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1); 
  if (digitalRead(12)==0){
    SPI.transfer(0x12); //transfer read command  
    inByte1 = SPI.transfer(0x00);
    inByte2 = SPI.transfer(0x00);
    inByte3 = SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x25);   //RREG trick to force DRDY high
    delay(1);
    digitalWrite(chipSelectPin, HIGH);
  }
  else{
    SerialUSB.println("conversion error");
    delay(1);
    digitalWrite(chipSelectPin, HIGH);
  }
  /* convert data from phase 2 --*/
  int rawData2 = 0; //create an empty 24 bit integer for the data
  rawData2 |= inByte4; //shift the data in one byte at a time
  rawData2 = (rawData2 << 8) | inByte5;
  rawData2 = (rawData2 << 8) | inByte6;
  if (((1 << 23) & rawData2) != 0){ //If the rawData has bit 24 on, then it's a negative value
    int mask = (1 << 24) - 1;
    rawData2 = ((~rawData2) & mask) + 1;
    decVal_2 = rawData2 * LSBsize * -1;}
  else{ //if it's not negative
    decVal_2 = float(rawData2)*LSBsize;} //then just multiply by LSBsize
  float dataSpin = (decVal_1-decVal_2)*1000; //output in mV
  printTimeStamp();
  SerialUSB.print(",");
  SerialUSB.print(dataSpin, DEC),SerialUSB.println(rawData2, HEX);
  delay(dTime);
}
/*------------------------------------------------*/


void handleCommand() {
  String message = "";
  while (SerialUSB.available() > 0) {
    byte inByte = SerialUSB.read();
    message += (char)inByte;
  }
  message = message.substring(0, message.length() - 2);
  //SerialUSB.println(message);
  String argv[7];
  parseMessage(message, argv);

  if (argv[0] == "test") {
    //SerialUSB.println("Recieved");
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


void sysTest(){
  SerialUSB.println("");
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
  SerialUSB.println("");
}

void readTemp() {
  delay(20);
  writeReg(0x49, 0x50); //enable internal temp monitor
  delay(100);
  float a = readData1(false,1000,false); //read adc in mV
  SerialUSB.print("Temperature: ");
  if (a < 129){
//    SerialUSB.print(a,DEC),SerialUSB.print("  ");
    SerialUSB.print((-1*(129.00-a)*0.403)+25), SerialUSB.println(" degrees C");
  }
  else {
//    SerialUSB.print(a,DEC),SerialUSB.print("  ");
    SerialUSB.print(((129.00-a)*0.403)+25), SerialUSB.println(" degrees C");
  }
  writeReg(0x49, 0x10); //disable internal temp monitor
  delay(50);  
}

void parseMessage(String msg, String arg[]) {
  int index = 0;

  int wordIndex = 0;
  int wordStart = 0;
  while (index < msg.length()) {
    if (msg.charAt(index) == ' ') {
      arg[wordIndex] = msg.substring(wordStart, index);
      //SerialUSB.println(arg[wordIndex]);
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
  //SerialUSB.println("---Starting ADC---");
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
  //SerialUSB.println("---Stopping ADC---");
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
    //SerialUSB.println("---IDAC ENABLED----"); 
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
    //SerialUSB.println("---IDAC DISABLED----"); 
    }
}



/*Reset ADC with command + reset pin --- WORKING ---*/
void resetADC() {
  //SerialUSB.println("---Resetting ADC---");
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x06); //send reset byte
  digitalWrite(resetPin, LOW);
  delayMicroseconds(1);
  digitalWrite(resetPin, HIGH);
//  delayMicroseconds(1);
//  digitalWrite(chipSelectPin, HIGH); 
//  /* register configuration - use excel calculator!*/
//  delay(1000);
//  digitalWrite(chipSelectPin, LOW);
  delay(1);
//  SPI.transfer(0x42);   //Send register START location
//  SPI.transfer(0x07);   //how many registers to write to
//  SPI.transfer(0x02);   //0x42  INPMUX 
//  SPI.transfer(0x08);   //0x43  PGA
//  SPI.transfer(0x17);   //0x44  DATARATE
//  SPI.transfer(0x39);   //0x45  REF
//  SPI.transfer(0x03);   //0x46  IDACMAG
//  SPI.transfer(0xF3);   //0x47  IDACMUX
//  SPI.transfer(0x82);   //0x48  VBIAS
//  SPI.transfer(0x10);   //0x49  SYS
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
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  delay(3000); 
}
void printTimeStamp() {
  //TODO: don't use millis
  SerialUSB.print(micros(),4);
}
/*Read 24 bit data from ADC --- WORKING ---*/
float readData1(bool showHex, int scalar, bool printData) { //read the ADC data when STATUS and CRC bits are NOT enabled
//  SerialUSB.print("Data Read: ");
  int result;
  float decVal = 0;
  /*Read the three bytes of 2's complement data from the ADC*/ 
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);  
  SPI.transfer(0x12); //transfer read command  
  inByte1 = SPI.transfer(0x00);
  inByte2 = SPI.transfer(0x00);
  inByte3 = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);

  /*Print the HEX value (if showHex = true)*/
  if (showHex == 1){
    //SerialUSB.print("HEX Value: ");
    SerialUSB.print(inByte1,HEX), SerialUSB.print(inByte2,HEX), SerialUSB.println(inByte3,HEX);
  }
  
  /*Convert the three bytes into readable data*/
  int rawData = 0; //create an empty 24 bit integer for the data
  rawData |= inByte1; //shift the data in one byte at a time
  rawData = (rawData << 8) | inByte2;
  rawData = (rawData << 8) | inByte3;
  
  if (((1 << 23) & rawData) != 0){ //If the rawData has bit 24 on, then it's a negative value
    int mask = (1 << 24) - 1;
    rawData = ((~rawData) & mask) + 1;
    decVal = rawData * scalar * LSBsize * -1;

//    decVal = float(rawData-(1 << 23))*-1*LSBsize*scalar; //if it's negative, then subtract 2^23 from it and multiple by LSB   
  }
  else{ //if it's not negative
    decVal = float(rawData)*LSBsize*scalar; //then just multiply by LSBsize
  }
//  SerialUSB.print("Voltage (V): ");
  if (printData) {
    printTimeStamp();
    SerialUSB.print(",");
    SerialUSB.println(decVal, DEC);
  }
  return decVal;
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
  SerialUSB.print(address2, HEX);
  SerialUSB.print("\t");
  digitalWrite(chipSelectPin, LOW); 
  delayMicroseconds(1);
  SPI.transfer(address2);
  inByte = SPI.transfer(0x00);
  result = SPI.transfer(0x00);
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  rawRegVal = (rawRegVal << 8 | result);
  SerialUSB.print(rawRegVal, HEX);
  SerialUSB.print("\t");
  SerialUSB.println(result, HEX);
}


/*Read ALL of the registers in a row --- WORKING ---*/
void regReadout(){
  SerialUSB.println("------Register Readout-------");
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
  SerialUSB.print("Register 0x00 (ID):        "), SerialUSB.println(readOut0, HEX);
  SerialUSB.print("Register 0x01 (STATUS):    "), SerialUSB.println(readOut1, HEX);
  SerialUSB.print("Register 0x02 (INPMUX):    "), SerialUSB.println(readOut2, HEX);
  SerialUSB.print("Register 0x03 (PGA):       "), SerialUSB.println(readOut3, HEX);
  SerialUSB.print("Register 0x04 (DATARATE):  "), SerialUSB.println(readOut4, HEX);
  SerialUSB.print("Register 0x05 (REF):       "), SerialUSB.println(readOut5, HEX);
  SerialUSB.print("Register 0x06 (IDACMAG):   "), SerialUSB.println(readOut6, HEX);
  SerialUSB.print("Register 0x07 (IDACMUX):   "), SerialUSB.println(readOut7, HEX);
  SerialUSB.print("Register 0x08 (VBIAS):     "), SerialUSB.println(readOut8, HEX);
  SerialUSB.print("Register 0x09 (SYS):       "), SerialUSB.println(readOut9, HEX);
  SerialUSB.print("Register 0x0A (OFCAL0):    "), SerialUSB.println(readOut10, HEX);
  SerialUSB.print("Register 0x0B (OFCAL1):    "), SerialUSB.println(readOut11, HEX);
  SerialUSB.print("Register 0x0C (OFCAL2):    "), SerialUSB.println(readOut12, HEX);
  SerialUSB.print("Register 0x0D (FSCAL0):    "), SerialUSB.println(readOut13, HEX);
  SerialUSB.print("Register 0x0E (FSCAL1):    "), SerialUSB.println(readOut14, HEX);
  SerialUSB.print("Register 0x0F (FSCAL2):    "), SerialUSB.println(readOut15, HEX);
  SerialUSB.print("Register 0x10 (GPIODAT):   "), SerialUSB.println(readOut16, HEX);
  SerialUSB.print("Register 0x11 (GPIOCON):   "), SerialUSB.println(readOut17, HEX);
  SerialUSB.println("-----------------------------");
}

/*Initiate Self Calibration --- WORKING (kinda?) ---*/
void SFOCAL() {
//  SerialUSB.println("Self Calibration");
  digitalWrite(chipSelectPin, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x19); //send self offset command
  delay(1);
  digitalWrite(chipSelectPin, HIGH);
  delay(100);
}

