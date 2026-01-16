#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Adafruit_SGP30.h"
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- PIN DEFINITIONS (Matched to your Schematic) ---
#define PIN_I2C_SDA     8   //
#define PIN_I2C_SCL     9   //
#define PIN_SD_CS       7   //
#define PIN_SD_MOSI     6   //
#define PIN_SD_MISO     5   //
#define PIN_SD_SCK      4   //
#define PIN_BUTTON      2   //

// --- SETTINGS ---
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32   
#define LOG_INTERVAL    60000 // Log every 1 minute (60,000ms)
#define FLUSH_COUNT     5     // Physically save to SD every 5 logs to save power/card life
#define LONG_PRESS_MS   2000  // 2 seconds to enter Deep Sleep

// --- OBJECTS ---
Adafruit_BME280 bme;
Adafruit_SGP30 sgp;
RTC_DS3231 rtc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- GLOBAL VARIABLES ---
unsigned long lastLogTime = 0;
unsigned long buttonPressTime = 0;
int displayPage = 0; // 0=Temp, 1=Hum/Pres, 2=Air Quality, 3=Time
int logsSinceFlush = 0;
bool isButtonPressed = false;

// --- POWER OFF FUNCTION ---
void powerOff() {
  display.clearDisplay();
  display.setCursor(20, 10);
  display.setTextSize(2);
  display.println("BYE BYE");
  display.display();
  delay(1500);
  
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  
  // Configure the button (Pin 2) to wake the device back up
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0); 
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  // Start I2C with your SuperMini pins
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("ROOTLOG BOOTING...");
  display.display();

  // Initialize Sensors
  if (!bme.begin(0x76, &Wire)) Serial.println("BME280 Error");
  if (!sgp.begin(&Wire)) Serial.println("SGP30 Error");
  if (!rtc.begin(&Wire)) Serial.println("RTC Error");

  // Initialize SD Card
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (!SD.begin(PIN_SD_CS)) {
    display.println("SD CARD FAIL!");
    display.display();
  } else {
    display.println("SD CARD READY");
    display.display();
  }
  delay(1000);
}

void loop() {
  // 1. BUTTON LOGIC (Short press = Page, Long press = Off)
  int buttonState = digitalRead(PIN_BUTTON);
  
  if (buttonState == LOW && !isButtonPressed) {
    buttonPressTime = millis();
    isButtonPressed = true;
  } 
  else if (buttonState == HIGH && isButtonPressed) {
    unsigned long duration = millis() - buttonPressTime;
    if (duration > LONG_PRESS_MS) {
      powerOff();
    } else {
      displayPage++;
      if (displayPage > 3) displayPage = 0;
    }
    isButtonPressed = false;
  }

  // 2. SENSOR READING & LOGGING (Every 1 Minute)
  if (millis() - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = millis();
    
    DateTime now = rtc.now();
    File dataFile = SD.open("/RootLog.csv", FILE_APPEND);
    
    if (dataFile) {
      // If file is new, add headers
      if (dataFile.size() == 0) {
        dataFile.println("Time,Temp,Hum,Pres,eCO2,TVOC");
      }
      
      // Write data row
      dataFile.print(now.timestamp()); dataFile.print(",");
      dataFile.print(bme.readTemperature()); dataFile.print(",");
      dataFile.print(bme.readHumidity()); dataFile.print(",");
      dataFile.print(bme.readPressure() / 100.0); dataFile.print(",");
      dataFile.print(sgp.eCO2); dataFile.print(",");
      dataFile.println(sgp.TVOC);

      // Flush strategy: Only force save every 5 minutes to protect the card
      logsSinceFlush++;
      if (logsSinceFlush >= FLUSH_COUNT) {
        dataFile.flush();
        logsSinceFlush = 0;
      }
      dataFile.close();
    }
  }

  // 3. AIR QUALITY BASELINE (Required by SGP30)
  sgp.IAQmeasure(); 

  // 4. DISPLAY UPDATE
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("ROOTLOG - PAGE "); display.println(displayPage + 1);
  display.drawLine(0, 9, 128, 9, WHITE);
  
  display.setCursor(0, 15);
  display.setTextSize(2);

  switch (displayPage) {
    case 0: // Temperature
      display.print(bme.readTemperature(), 1); display.print(" C");
      break;
    case 1: // Humidity & Pressure
      display.setTextSize(1);
      display.print("Hum: "); display.print(bme.readHumidity(), 0); display.println("%");
      display.print("Pre: "); display.print(bme.readPressure()/100, 0); display.print("hPa");
      break;
    case 2: // Air Quality
      display.setTextSize(1);
      display.print("eCO2: "); display.print(sgp.eCO2); display.println("ppm");
      display.print("TVOC: "); display.print(sgp.TVOC); display.print("ppb");
      break;
    case 3: // Time
      DateTime now = rtc.now();
      display.print(now.hour()); display.print(":"); 
      if(now.minute() < 10) display.print("0");
      display.print(now.minute());
      break;
  }
  display.display();
  delay(100); 
}