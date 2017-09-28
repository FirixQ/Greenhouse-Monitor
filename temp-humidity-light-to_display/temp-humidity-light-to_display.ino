// Inlcude relevant libraries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Initialise sensors variable
Adafruit_TSL2561_Unified luxMeter = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //address at the end is for 
// adafruit sensor libray

// setup software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK    8
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

long lux;
long greatestLux = 0L;
int ldr;
int integrationTime;

void setup() {
  // turn on serial for error reporting
  Serial.begin(9600);
  
  // initialise sensors
  if(!luxMeter.begin()){
    Serial.println("TSL2561 / Lux meter not detected");
    while(1); //wait until restarted because code won't run without sensor
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
  
  // get sensor event 
  sensors_event_t event;
  luxMeter.getEvent(&event);

  lux = event.light;
  ldr = analogRead(A0);

  // displays output in lux

  if (ldr > 800){
    luxMeter.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
    integrationTime = 13;
  } else if (ldr <= 800 && ldr > 450) {
    luxMeter.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);
    integrationTime = 101;
  } else if (ldr <= 450) {
    luxMeter.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
    integrationTime = 402;
  }
  lux = event.light;
  if (lux > greatestLux) {
    greatestLux = lux;
  }
  display.print("L: ");
  display.print(lux);
  display.println(" l");
  display.print("L+:");
  display.println(greatestLux);
  display.print("RL: " );
  display.println(ldr);
  display.print("T: ");
  display.print(integrationTime);
  display.println("ms");
  display.display();
  
  delay(250);
}
