/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6VLpbEL46"
#define BLYNK_TEMPLATE_NAME "IOT Water Tank Controller"
#define BLYNK_AUTH_TOKEN "3wmxYNqmYYpZIKJw9c7KMCToR3MIQtoW"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#define TRIGGER_PIN 14      // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 27         // Arduino pin tied to echo pin on the ultrasonic sensor.
#define EMPTY_DISTANCE 350  // Measure the height of the water tank from BOTTOM-TO-TOP until reaches the ultrasonic sensor.
#define LOWER_THRESHOLD 50  // Height of water in tank when it is LOW.
#define UPPER_THRESHOLD 250 // Height of water in tank when it is FULL.
#define BUTTON_PIN 26
#define PUMP_RELAY 25
#define FULL_LED 33
#define EMPTY_LED 32
#define I2C_SCL 22
#define I2C_SDA 21

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <NewPing.h>
#include "U8g2lib.h"
#include "A-Bitmap.h"

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
String ipAddress;
int waterLevel = 0, blynkInput = 0;
bool pumpState = false, manualState = false;
unsigned long time_now = 0;

BlynkTimer timer;
WidgetLED pumpLed(V3);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,       // Rotation
                                         U8X8_PIN_NONE, // Reset
                                         I2C_SCL,       // I2C clock
                                         I2C_SDA);      // I2C data
NewPing sonar(TRIGGER_PIN,
              ECHO_PIN,
              EMPTY_DISTANCE);

int readWaterLevel()
{
  int current_distance = sonar.ping_cm();
  int actual_level = EMPTY_DISTANCE - current_distance;
  return actual_level;
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(10, 25, "Connecting to WiFi");
    u8g2.drawXBMP(55, 31, 19, 16, image_wifi_not_connected_bits);
    u8g2.drawRFrame(5, 12, 118, 39, 5);
    u8g2.sendBuffer();
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  ipAddress = WiFi.localIP().toString();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(23, 25, "Wifi Connected");
  u8g2.drawRFrame(5, 12, 118, 39, 5);
  u8g2.drawXBMP(55, 31, 19, 16, image_wifi_bits);
  u8g2.sendBuffer();
  delay(2000);
}

void drawTank(int value, bool operatingMode, bool pumpMode)
{
  char buffer[32];
  u8g2.clearBuffer();
  u8g2.drawFrame(2, 2, 34, 53);
  u8g2.setDrawColor(2);
  u8g2.drawXBMP(4, 3, 6, 51, image_water_scale_bits);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_t0_12b_tr);
  u8g2.drawStr(40, 11, "IOT WATER TANK");
  u8g2.drawBox(0, 56, 128, 8);
  u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(1, 63, "IP:");
  u8g2.drawStr(16, 63, ipAddress.c_str());
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_t0_11b_tr);
  if (operatingMode == true)
  {
    u8g2.drawStr(41, 51, "Manual Mode");
  }
  else
  {
    u8g2.drawStr(47, 51, "Auto Mode");
  }
  if (pumpMode == true)
  {
    u8g2.drawXBMP(110, 39, 15, 15, image_choice_bullet_on_bits);
  }
  else
  {
    u8g2.drawXBMP(110, 39, 15, 15, image_choice_bullet_off_bits);
  }
  u8g2.setDrawColor(2);
  u8g2.drawBox(3,
               map(constrain(value, 0, UPPER_THRESHOLD), 0, UPPER_THRESHOLD, 52, 3),
               32,
               map(constrain(value, 0, UPPER_THRESHOLD), 0, UPPER_THRESHOLD, 2, 51));
  sprintf(buffer, "%d", value);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_profont29_tr);
  u8g2.drawStr(45, 37, buffer);
  u8g2.setFont(u8g2_font_profont22_tr);
  u8g2.drawStr(97, 37, "cm");
  u8g2.sendBuffer();
}

void updateData()
{
  // Push data to Blynk server
  Blynk.virtualWrite(V1, waterLevel);
  if (manualState == true)
  {
    Blynk.virtualWrite(V2, "Manual Mode");
  }
  else
  {
    Blynk.virtualWrite(V2, "Auto Mode");
  }
  if (pumpState == true)
  {
    pumpLed.on();
  }
  else
  {
    pumpLed.off();
  }
}

BLYNK_WRITE(V0)
{
  // Retrieve data from Blynk server
  blynkInput = param.asInt();
  if (blynkInput == 1)
  {
    manualState = true;
  }
}

void setup()
{
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);

  // Debug console
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(FULL_LED, OUTPUT);
  pinMode(EMPTY_LED, OUTPUT);
  digitalWrite(PUMP_RELAY, LOW);
  digitalWrite(EMPTY_LED, LOW);
  digitalWrite(FULL_LED, LOW);
  initWiFi();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, updateData);
}

void loop()
{
  Blynk.run();
  timer.run();
  if (millis() - time_now > 150)
  {
    time_now = millis();
    waterLevel = readWaterLevel();

    char buffer[50];
    sprintf(buffer, "Current water level is: %dcm", waterLevel);
    Serial.println(buffer);
    drawTank(waterLevel, manualState, pumpState);
  }

  if (digitalRead(BUTTON_PIN) == 0)
  {
    manualState = true;
  }

  // Auto mode
  if (waterLevel <= LOWER_THRESHOLD)
  {
    digitalWrite(PUMP_RELAY, HIGH);
    pumpState = true;
  }
  else if (waterLevel >= UPPER_THRESHOLD)
  {
    digitalWrite(PUMP_RELAY, LOW);
    pumpState = false;
  }
  // Manual mode
  if ((waterLevel <= UPPER_THRESHOLD) && manualState == true)
  {
    digitalWrite(PUMP_RELAY, HIGH);
    pumpState = true;
  }
  else if ((waterLevel >= UPPER_THRESHOLD) && manualState == true)
  {
    digitalWrite(PUMP_RELAY, LOW);
    manualState = false;
    pumpState = false;
  }
  // Water Level State
  if (waterLevel <= UPPER_THRESHOLD)
  {
    digitalWrite(EMPTY_LED, HIGH);
    digitalWrite(FULL_LED, LOW);
  }
  else
  {
    digitalWrite(EMPTY_LED, LOW);
    digitalWrite(FULL_LED, HIGH);
  }
}
