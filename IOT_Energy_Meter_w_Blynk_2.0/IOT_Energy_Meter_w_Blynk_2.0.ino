/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPxxxxxx"
#define BLYNK_TEMPLATE_NAME         "Device"
#define BLYNK_AUTH_TOKEN            "YourAuthToken"

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include "EmonLib.h"
#include <EEPROM.h>
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

EnergyMonitor emon;
#define vCalibration 83.3
#define currCalibration 0.50
 
BlynkTimer timer;

// Wifi Setup
char ssid[] = "**********************";
char pass[] = "**********************";
 
float kWh = 0, cost = 0;
unsigned long lastmillis = millis();
 
void myTimerEvent()
{
  emon.calcVI(20, 2000);
  kWh = kWh + emon.apparentPower * (millis() - lastmillis) / 3600000000.0;
  cost = kWh * 0.218;
  yield();
  Serial.print("Vrms: ");
  Serial.print(emon.Vrms, 2);
  Serial.print("V");
  EEPROM.put(0, emon.Vrms);
  delay(100);
 
  Serial.print("\tIrms: ");
  Serial.print(emon.Irms, 4);
  Serial.print("A");
  EEPROM.put(4, emon.Irms);
  delay(100);
 
  Serial.print("\tPower: ");
  Serial.print(emon.apparentPower, 4);
  Serial.print("W");
  EEPROM.put(8, emon.apparentPower);
  delay(100);
 
  Serial.print("\tkWh: ");
  Serial.print(kWh, 5);
  Serial.println("kWh");
  EEPROM.put(12, kWh);
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("U:");
  lcd.print(emon.Vrms, 1);
  lcd.print("V");
  lcd.setCursor(0, 1);
  lcd.print("I:");
  lcd.print(emon.Irms, 2);
  lcd.print("A");
  lcd.setCursor(9, 0);
  lcd.print("P:");
  lcd.print(emon.apparentPower, 0);
  lcd.print("W");
  lcd.setCursor(9, 1);
  lcd.print("RM:");
  lcd.print(cost, 2);
 
  lastmillis = millis();
 
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, emon.Irms);
  Blynk.virtualWrite(V2, emon.apparentPower);
  Blynk.virtualWrite(V3, kWh);
}
 
void setup()
{
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  lcd.init();
  lcd.backlight();
  emon.voltage(35, vCalibration, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon.current(34, currCalibration);    // Current: input pin, calibration.
 
  timer.setInterval(5000L, myTimerEvent);
  lcd.setCursor(3, 0);
  lcd.print("IoT Energy");
  lcd.setCursor(6, 1);
  lcd.print("Meter");
  delay(3000);
  lcd.clear();
}
 
void loop()
{
  Blynk.run();
  timer.run();
}
