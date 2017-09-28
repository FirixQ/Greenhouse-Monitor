// Inlcude relevant libraries
#include <AM2320.h>
#include <Wire.h>
#include <SPI.h>
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
//bool rtcOn; //set false if rtc isnt running

// setup software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK    8
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  // turn on serial for error reporting
  Serial.begin(9600);
  
  // initialise sensors
  tempAndHumid.begin();

  if(!luxMeter.begin()){
    Serial.println("TSL2561 / Lux meter not detected");
    while(1); //wait until restarted because code won't run without sensor
  }
  // initialise rtc
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    //rtcOn = false;
  } else {
    if (!rtc.isrunning()) {
      Serial.println("RTC not running, starting now.");
      // update rtc time
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      //rtcOn = true;
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
}

void loop() {
  display.setCursor(0,0);
  display.clearDisplay();
  if (tempAndHumid.measure()){ // .measure returns true if measurement was successful
    display.print("T: ");
    display.print(tempAndHumid.getTemperature()); // retrieves the temp value
    display.print("C\nH: ");
    display.print(tempAndHumid.getHumidity()); // retrieves the humidity value
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
    display.print((int)event.light);
    display.println(" l");
  } else {
    Serial.println("TSL2561 overloaded - no reliable info");
    display.println("LuxOverload");
  }

  
  //get current time and print to monitor
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
  
  delay(1000);
}
