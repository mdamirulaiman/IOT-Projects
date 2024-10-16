
/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6VLpbEL46"
#define BLYNK_TEMPLATE_NAME "IOT Water Tank Controller"
#define BLYNK_AUTH_TOKEN "3wmxYNqmYYpZIKJw9c7KMCToR3MIQtoW"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#define TRIGGER_PIN  14  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     27  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define EMPTY_DISTANCE 350 // Water tank full height
#define LOWER_THRESHOLD 50 // Low water level height
#define UPPER_THRESHOLD 250 // Full water level height
#define BUTTON_PIN 26
#define RELAY_PIN 25

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
int waterLevel = 0;
bool pumpState = false, manualState = false;
unsigned long time_now = 0;

BlynkTimer timer;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, EMPTY_DISTANCE);

void setup()
{
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setBitmapMode(1);

  // Debug console
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

}

void loop()
{
  if (millis() - time_now > 100) {
    time_now = millis();
    waterLevel = readWaterLevel();

    char buffer[50];
    sprintf(buffer, "Current water level is: %dcm", waterLevel);
    Serial.println(buffer);

    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(4, 11, "Water level monitor");
    u8g2.drawRFrame(0, 0, 102, 16, 5);
    u8g2.drawXBMP(107, 1, 15, 16, image_network_4_bars_bits);
    u8g2.sendBuffer();


    delay(50);
  }

  if (digitalRead(BUTTON_PIN) == 0) {
    manualState = true;
  }

  // Auto mode
  if ((waterLevel <= LOWER_THRESHOLD) && pumpState == false) {
    digitalWrite(RELAY_PIN, HIGH);
    pumpState = true;
  } else if (waterLevel >= UPPER_THRESHOLD && pumpState == true) {
    digitalWrite(RELAY_PIN, LOW);
    pumpState = false;
  }
  // Manual mode
  if ((waterLevel <= UPPER_THRESHOLD) && manualState == true) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if ((waterLevel >= UPPER_THRESHOLD) && manualState == true) {
    digitalWrite(RELAY_PIN, LOW);
    manualState = false;
  }

}

int readWaterLevel()
{
  int current_distance = sonar.ping_cm();
  int actual_level = EMPTY_DISTANCE - current_distance;
  return actual_level;
}
