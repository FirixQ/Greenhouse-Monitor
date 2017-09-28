// Inlcude relevant libraries
#include <AM2320.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

// Initialise sensors variable
AM2320 tempAndHumid;
Adafruit_TSL2561_Unified luxMeter = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //address at the end is for 
// adafruit sensor libray
RTC_DS1307 rtc;

// setup software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK    8
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// SD card on hardware SPI
const int chipSelect = 53;

// vars for saving data every minute
int lastUnixMin;
int nowUnixMin;
int nextUnixMin;
float temp;
float humidity;
int lux;

void setup() {
  // turn on serial for error reporting
  Serial.begin(9600);

  // setup sd card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
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

  //configure display
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  // get start time for timer
  DateTime start = rtc.now();
  lastUnixMin = start.unixtime(); 
}

void loop() {
  display.setCursor(0,0);
  display.clearDisplay();
  if (tempAndHumid.measure()){ // .measure returns true if measurement was successful
    display.print("T: ");
    temp = tempAndHumid.getTemperature() - 1.5;
    display.print(temp); // retrieves the temp value
    display.print("C\nH: ");
    humidity = tempAndHumid.getHumidity();
    display.print(humidity); // retrieves the humidity value
    display.println("%");
  } else {
    int errorCode = tempAndHumid.getErrorCode();
    switch (errorCode) {
      case 1: Serial.println("ERR: Sensor is offline"); break;
      case 2: Serial.println("ERR: CRC validation failed."); break;
    }    
  }

  // get sensor event 
  sensors_event_t event;
  luxMeter.getEvent(&event);

  // displays output in lux
  if (event.light){
    display.print("L: ");
    lux = (int)event.light;
    display.print(lux);
    display.println(" l");
  } else {
    Serial.println("TSL2561 overloaded - no reliable info");
    display.println("LuxOverload");
  }

  
  //get current time
  DateTime now = rtc.now();
  
  //convert to 12 hour time
  int twentyfourhr = now.hour();
  int twelvehr;
  char partOfDay[4];
  if(twentyfourhr < 12){
    twelvehr = twentyfourhr;
    sprintf(partOfDay, " AM");
  } else {
    twelvehr = twentyfourhr - 12;
    sprintf(partOfDay, " PM");
  }
  display.print(twelvehr); display.print(":");
  display.print(now.minute(), DEC); display.print(partOfDay);
  display.display();

  // check if a minute has elasped. if it has then store data on sd card.
  nowUnixMin = now.unixtime();
  nextUnixMin = lastUnixMin + 60;

  if(nowUnixMin > nextUnixMin){
    lastUnixMin = nowUnixMin;
    String currentTime = String(now.minute()) + ":" + String(now.hour()) + " - " + String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
    String dataString = String(temp) + "," + String(humidity) + "," + String(lux) + "," + currentTime;
    File dataFile = SD.open("data.log", FILE_WRITE);
    if(dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    }
  }
  delay(1000);
}
