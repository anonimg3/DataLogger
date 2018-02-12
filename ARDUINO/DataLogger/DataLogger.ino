/******************************** LIBRARIES ********************************/
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <Adafruit_INA219.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include <SdFat.h>
/*************************** GLOBAL VARIABLES ***************************/
/* SD CARD */
SdFat SD;
File file;
String fileName;

/* OLED DISPLAY */
SSD1306AsciiAvrI2c oled;

/* RTC */
RTC_DS3231 RTC;
String currentDate;
String currentTime;

/* DS18B20 TEMPERATURE SENSOR */
byte sensorAddress[8] = {0x28, 0x5B, 0xC4, 0x8, 0xA, 0x0, 0x0, 0x83};
OneWire onewire(8);
DS18B20 sensors(&onewire);

/* INA219 CURRENT SENSOR */
Adafruit_INA219 ina219;
int sampleCnt = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
float temperature = 0;
/****************************** FUNCTIONS ******************************/
void setup() {
  /* SERIAL INITIALIZE */
  Serial.begin(115200);
  while (!Serial) {
    // wait for serial port to connect. Needed for native USB port only
  }

  /* SD CARD INITIALIZE */
  SD.begin(10);

  /* CURRENT SENSOR INITIALIZE */
  ina219.begin();

  /* OLED DISPLAYINITIALIZE */
  oled.begin(&Adafruit128x64, 0x3C);
  oled.clear();
  oled.setFont(Stang5x7);

  /* TEMPERATURE SENSOR INITIALIZE */
  sensors.begin();
  sensors.request(sensorAddress);

  /* SET CURRENT DATE AND TIME (LAUNCH ONLY ONCE) */
  //    DateTime now = RTC.now();
  //    DateTime compiled = DateTime(__DATE__, __TIME__);
  //    if (now.unixtime() < compiled.unixtime()) {
  //      RTC.adjust(DateTime(__DATE__, __TIME__));
  //    }
  
  /* GET THE CURRENT DATE AND TIME */
  DateTime now = RTC.now();
  
  /* FORMATTING DATE AND TIME */
  currentDate = String(now.year()) + "-" +  formatting(now.month()) + "-" + formatting(now.day());
  currentTime = formatting(now.hour()) + "-" + formatting(now.minute())  + "-" + formatting(now.second());
  
  /* CREATE A UNIQUE FILE NAME */
  fileName = currentDate + " " + currentTime + ".txt";;
}

void loop() {
  /* INCREMENT SAMPLE COUNTER */
  sampleCnt++;

  /* READS VOLTAGE AND CURRENT FROM SENSOR*/
  current_mA = ina219.getCurrent_mA();
  loadvoltage = ina219.getBusVoltage_V() + ( ina219.getShuntVoltage_mV() / 1000);
  power_mW = ina219.getPower_mW();

  /* GET THE CURRENT DATE AND TIME */
  DateTime now = RTC.now();

  /* WAIT FOR RESULTS */
  delay(500);

  /* FORMATTING DATE AND TIME */
  currentDate = String(now.year()) + "-" +  formatting(now.month()) + "-" + formatting(now.day());
  currentTime = formatting(now.hour()) + ":" + formatting(now.minute())  + ":" + formatting(now.second());

  /* READS THE TEMPERATURE FROM SENSORE */
  if (sensors.available()) {
    temperature = sensors.readTemperature(sensorAddress);
    sensors.request(sensorAddress);
  }

  /* DISPLAY MEASUREMENTS ON OLED */
  oled.clear();
  oled.println("Voltage: " + String(loadvoltage) + " V");
  oled.println("Current: " + String(current_mA) + " mA");
  oled.println("Power:   " + String(power_mW) + " mW");
  oled.println("Temp:    " + String(temperature) + " C");
  oled.println("Time:    " + currentTime);
  oled.println("Date:    " + currentDate);
  oled.println("Sample:  " + String(sampleCnt));
  oled.println("Author:  Anonimg3");

  /* SAVE MEASUREMENTS ON THE SD CARD*/
  file = SD.open(fileName, FILE_WRITE);
  if (file) {
    String fileBuf = String(sampleCnt) + ";" + loadvoltage + ";" + current_mA + ";" + power_mW + ";" + temperature + ";" + currentDate + ";" + currentTime;
    file.println(fileBuf );
    file.close();
  }

  /* SEND MEASUREMENTS VIA UART */
  String serialBuf = String(loadvoltage) + ',' + String(current_mA) + ',' + String(power_mW) + ',' + String(currentDate) + ',' + String(currentTime) + ',' + String(temperature) + ',';
  Serial.println(serialBuf);
}

String formatting(int value) {
  return value < 10 ? "0" + String(value) : String(value);
}


