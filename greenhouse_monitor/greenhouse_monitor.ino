// Inlcude relevant libraries
#include <AM2320.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <RTClib.h>

// Initialise sensors variable
AM2320 tempAndHumid;
Adafruit_TSL2561_Unified luxMeter = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //address at the end is for 
// adafruit sensor libray
RTC_DS1307 rtc;

// SD card on hardware SPI
const int chipSelect = 53;

//for storing data
float temp;
float humidity;
unsigned int lux;

void setup() {
  // turn on serial for error reporting
  Serial.begin(9600);

  // setup sd card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while(1);
  }
  Serial.println("SD card initialized.");
  
  // initialise sensors
  tempAndHumid.begin();

  if(!luxMeter.begin()){
    Serial.println("TSL2561 / Lux meter not detected");
    while(1); //wait until restarted because code won't run without sensor
  }
  // initialise rtc
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while(1);
  } else {
    if (!rtc.isrunning()) {
      Serial.println("RTC not running, starting now.");
      // update rtc time
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }
  // configure luxMeter
  luxMeter.setGain(TSL2561_GAIN_1X); //sets sensitivity to 1x
  luxMeter.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS); //same as camera shutter speed; options are 13ms, 101ms and 402ms.
  // lower values allow for greater accuracy in bright condition, higher for dark conditions.

  // get start time for timer
  DateTime start = rtc.now(); 

  // add boot time and date to file
  File dataFile = SD.open("data.log", FILE_WRITE);
  if(dataFile) {
    String initData = "booted,,," + String(start.unixtime());
    dataFile.println(initData);
    dataFile.close();
  }

  // set up pin 13 (led) to switch on when reading is taking place
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // start
  Serial.println("Starting now");
}

void loop() {
  // get current millis to calculate amount of time to deduct from the delay
  unsigned long start = millis();

  // switch on LED to warn user from powering off
  digitalWrite(13, HIGH);

  // begin collecting data
  if (tempAndHumid.measure()){ // measure returns true if measurement was successful
    temp = tempAndHumid.getTemperature() - 1.5;
    humidity = tempAndHumid.getHumidity();
  } else {
    int errorCode = tempAndHumid.getErrorCode();
    temp = 999;
    humidity = 0;
    switch (errorCode) {
      case 1: Serial.println("ERR: Sensor is offline"); break;
      case 2: Serial.println("ERR: CRC validation failed."); break;
    }    
  }

  // get sensor event 
  sensors_event_t event;
  luxMeter.getEvent(&event);

  // records in lux
  if (event.light){
    lux = (int)event.light;
  } else {
    Serial.println("TSL2561 overloaded - no reliable info");
    //check LDR to see if its bright.
    int ldrVal = analogRead(A0);
    if(ldrVal > 500) {
      lux = 100000L; // value definately out of range but light
    } else {
      lux = 0; // ldr thinks its dark, set lux to 0
    }
  }

  //get current time
  DateTime now = rtc.now();
  File dataFile = SD.open("data.log", FILE_WRITE);
  String dataString = String(temp) + "," + String(humidity) + "," + String(lux) + "," + String(now.unixtime());
  if(dataFile) {
      dataFile.println(dataString);
      dataFile.close();
  }

  // get the end time and calculate the time taken
  unsigned long timeTaken = millis()-start;
  int reduceTime = (int)timeTaken * 1000;

  //turn led off again to indicate board is no longer taking data
  digitalWrite(13, LOW);
  
  delay(600000-reduceTime); //10 minutes, subtracting time taken for this operation
}
