// Define pin connection
#define currentSensorPin A0 // ADC0 (A0)
#define DHTSensorPin 5 // GPIO5 (D1)

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID       "**************"
#define BLYNK_TEMPLATE_NAME     "***************"
#define BLYNK_AUTH_TOKEN        "********************************"

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "*********";
char pass[] = "*********";

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
BlynkTimer timer;
DHT dht(DHTSensorPin, DHT11);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Initialize pin
  pinMode(currentSensorPin, INPUT);
  pinMode(DHTSensorPin, INPUT);
  dht.begin();
  // Function callback for every second
  timer.setInterval(1000L, readSensor);
}

void readSensor() {
  float rawADC = 0.0, rawSamples = 0.0, rawAverage = 0.0, actualCurrent = 0.0, actualPower = 0.0;

  for (unsigned int x = 0; x < 150; x++) { // Get 150 samples
    rawADC = analogRead(currentSensorPin);
    rawSamples = rawSamples + rawADC; // Add samples all together
    delay(3); // let ADC settle between samples
  }
  rawAverage = rawSamples / 150.0; // Get the average sample value
  actualCurrent = abs((2.46 - (rawAverage * (3.3 / 1024.0))) / 0.066);
  actualPower = 240 * actualCurrent;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Blynk.virtualWrite(V0, actualCurrent);
  Blynk.virtualWrite(V1, actualPower);
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V3, t);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}
