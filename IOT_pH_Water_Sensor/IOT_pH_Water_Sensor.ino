#define BLYNK_TEMPLATE_ID "*************"
#define BLYNK_TEMPLATE_NAME "***** *****"
#define BLYNK_AUTH_TOKEN "*********************************"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "************"; // Replace with your Wi-Fi network name
char pass[] = "************"; // Replace with your Wi-Fi password

// Analog pin for your pH sensor (connected to TO pin on the module)
const int pHSensorPin = 12;  // Replace with the pin connected to your sensor

// Digital pin for the relay control (replace with your specific pin)
const int relayPin = 2;  // Controlling the relay

// Variables to store pH readings
float pHValue = 0.0;

BlynkTimer timer;

void readSensor(){
  // Read pH value from the analog pin
  int sensorValue = analogRead(pHSensorPin);
  
  // Convert the sensor value to pH level (adjust these values based on your sensor's specifications)
  pHValue = map(sensorValue, 0, 4096, 0.0, 14.0);
  
  // Print pH value to the serial monitor
  Serial.print("pH Value: ");
  Serial.println(pHValue);
  
  // Send pH value to Blynk app. Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, pHValue);  // Use a Value Display Widget in the app to display pHValue.
  
  // Check pH value and control the relay
  if (pHValue > 7.0){
    // If pH is greater than 7, turn on the relay
    digitalWrite(relayPin, HIGH);
    Serial.println("Relay ON");
    } else {
    // If pH is 7 or lower, turn off the relay
    digitalWrite(relayPin, LOW);
    Serial.println("Relay OFF");
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Initialize the relay pin and pH sensor pin
  pinMode(pHSensorPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Ensure relay is initially off
  // Setup a function to be called every second
  timer.setInterval(1000L, readSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}
