#include "KickSat_Sensor.h"
#include <RTCZero.h>
#include <SPI.h>
#include <SdFat.h>
SdFat SD;
File logfile;


KickSat_Sensor kSensor(ADC_CS, ADC_RST, SD_CS, "config.txt");



/*-----------SLEEP VARRIABLES----------*/
RTCZero rtc;                     // Create RTC object
               // Array for file name data logged to named in setup
int NextAlarmSec;                // Variable to hold next alarm time seconds value
int NextAlarmMin;                // Variable to hold next alarm time minute value


/*----------INITAL START TIME---------*/
const byte hours = 10;
const byte minutes = 30;
const byte seconds = 00;
const byte day = 13;
const byte month = 8;
const byte year = 18;

/*---------------SETUP---------------*/
void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(GREEN, OUTPUT);
  SPI.begin();
//  rtc.begin();                                            // Start the RTC in 24hr mode
//  rtc.setTime(hours, minutes, seconds);                   // Set the time
//  rtc.setDate(day, month, year);                          // Set the date
  
  kSensor.CreateFile();                                           //MUST do - init SD card and creates unique filename
  kSensor.writeHeader();
  
}

/*---------------LOOP---------------*/
void loop() {
  float buf[10];                                          // holds data from XTB    


  kSensor.operate(buf);                                   // function that actually measures the devices
                                                          // ----------------------------------------------

  blink(GREEN,1);                                         // Quick blink to show we survived the operate function
                                                          // ----------------------------------------------
                                                            
//  if( CurrentCycleCount >= SamplesPerCycle ) {          // Decide if we need to save to SD (not used)       
//    logfile.flush();
//    CurrentCycleCount = 0;
//  }
//  if ( SampleIntMin == 0 ) {                              // calculate next alarm
//    NextAlarmSec = (NextAlarmSec + SampleIntSec) % 60;    // modulus to deal with rollover 65 becomes 5
//    rtc.setAlarmSeconds(NextAlarmSec);                    // RTC time to wake, currently seconds only
//    rtc.enableAlarm(rtc.MATCH_SS);                        // Match seconds only
//  }
//  else {  
//    NextAlarmMin = (rtc.getMinutes() + SampleIntMin) % 60;    // modulus to deal with rollover 65 becomes 5
//    NextAlarmMin = NextAlarmMin + int ((rtc.getSeconds() + SampleIntSec) / 60);    // find rollover minute, i.e. 65 secs becomes 1 minute extra
//    NextAlarmSec = (rtc.getSeconds() + SampleIntSec) % 60;    // i.e. 65 becomes 5
//
//    rtc.setAlarmSeconds(NextAlarmSec);                    // Set RTC time to wake in seconds
//    rtc.setAlarmMinutes(NextAlarmMin);                    // Set RTC time to wake in minutes
//    rtc.enableAlarm(rtc.MATCH_MMSS);                      // Match minutes & seconds
//  }
//  rtc.attachInterrupt(alarmMatch);                        // Attaches function to be called, currently blank
//  delay(5);                                               // Brief delay prior to sleeping not really sure its required  
//  rtc.standbyMode();                                      // Sleep until next alarm match   
}
/*-----------------------------------*/
/*-----------------------------------*/





void blink(uint8_t LED, uint8_t flashes) {  // blink for a certain number of flashes
  uint8_t i;
  for (i=0; i<flashes; i++) {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(200);
  }
}


void alarmMatch() // Do something when interrupt called
{

}
