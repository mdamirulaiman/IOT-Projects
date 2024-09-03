/*
	DHT11 Sensors
	Lux Sensor BH1750
	Curennt Sensor ACS712 20A
	Rain Sensor
*/

// Define pin connection
#define currentSensorPin A0 // ADC0 (A0)
#define BH1750_SCL 5 // GPIO5 (D1)
#define BH1750_SDA 4 // GPIO4 (D2)
#define DHTSensorPin 14 // GPIO14 (D5)
#define rainSensorPin 12 // GPIO12 (D6)

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID       "***************"
#define BLYNK_TEMPLATE_NAME     "***************"
#define BLYNK_AUTH_TOKEN        "*********************************"

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "*********";
char pass[] = "*********";

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
#include <BH1750.h>
#include <Wire.h>

BlynkTimer timer;
DHT dht(DHTSensorPin, DHT11);
BH1750 lightMeter;

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	Wire.begin();
	Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
	
	// Initialize pin
	pinMode(currentSensorPin, INPUT);
	pinMode(DHTSensorPin, INPUT);
	pinMode(rainSensorPin, INPUT);
	dht.begin();
	lightMeter.begin();
	// Function callback for every second
	timer.setInterval(1000L, readSensor);
}

void readSensor() {
	float rawADC = 0.0, rawSamples = 0.0, rawAverage = 0.0, actualCurrent = 0.0;
	
	for (unsigned int x = 0; x < 150; x++) { // Get 150 samples
		rawADC = analogRead(currentSensorPin);
		rawSamples = rawSamples + rawADC; // Add samples all together
		delay(3); // let ADC settle between samples
	}
	rawAverage = rawSamples / 150.0; // Get the average sample value
	actualCurrent = abs((2.46 - (rawAverage * (3.3 / 1024.0))) / 0.066);
	
	Serial.print(F("Current: "));
	Serial.print(actualCurrent);
	Serial.println(F(" Amps"));
	
	float h = dht.readHumidity();
	float t = dht.readTemperature();
	// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t)) {
		Serial.println(F("Failed to read from DHT sensor!"));
		return;
	}
	
	Serial.print(F("Humidity: "));
	Serial.print(h);
	Serial.print(F("%  Temperature: "));
	Serial.print(t);
	Serial.println(F("°C"));
	
	float lux = lightMeter.readLightLevel(); 
	
	Serial.print(F("Light Intensity: "));
	Serial.print(lux);
	Serial.println(F(" lux"));
	
	Blynk.virtualWrite(V0, actualCurrent); // Current from ACS712
	Blynk.virtualWrite(V1, h); // Humidity from DHT11
	Blynk.virtualWrite(V2, t); // Temperature from DHT11
	Blynk.virtualWrite(V3, lux); // Lux from BH1750
	if (digitalRead(rainSensorPin) == HIGH) { // Rain from rain sensor
		Serial.print(F("Rain Status: "));
		Serial.println(F("NO RAIN"));
		Blynk.virtualWrite(V4, "NO RAIN");
		} else {
		Serial.print(F("Rain Status: "));
		Serial.println(F("RAIN"));
		Blynk.virtualWrite(V4, "RAIN");
	}
}

void loop() {
	// put your main code here, to run repeatedly:
	Blynk.run();
	timer.run();
}
