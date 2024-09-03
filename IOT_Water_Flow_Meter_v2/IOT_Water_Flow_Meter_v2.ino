#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

/*--------------- THINKSPEAK SETUP ----------------------*/
#define SECRET_SSID "*********"		// replace with your WiFi network name
#define SECRET_PASS "*********"		// replace with your WiFi password
#define SECRET_CH_ID ******			// replace with your channel number
#define SECRET_WRITE_APIKEY "*****************"   // replace with your channel write API Key
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

/*--------------- OLED SETUP ----------------------*/
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*--------------- WIFI SETUP ----------------------*/
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

/*--------------- VARIABLE AND GPIO SETUP ----------------------*/
#define LED_BLINK 16
#define SENSOR  2
long currentMillis = 0, previousMillis = 0, currentMillis2 = 0, previousMillis2 = 0;
int interval = 1000, interval2 = 20000;
boolean ledState = LOW;
float calibrationFactor = 4.5, flowRate = 0, flowLitres = 0, totalLitres = 0;
volatile byte pulseCount;
byte pulse1Sec = 0;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
String myStatus = "";

void IRAM_ATTR pulseCounter() {
  pulseCount++;
  digitalWrite(LED_BLINK, !digitalRead(LED_BLINK));
}

void setup() {
  Serial.begin(115200);  // Initialize serial
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
  delay(10);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  pinMode(LED_BLINK, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("WiFi:");
    display.println(SECRET_SSID);
    display.print("Pass:");
    display.println(SECRET_PASS);
    display.print("Connecting");
    display.display();
    display.setCursor(60, 16);

    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      display.print(".");
      display.display();
      delay(2000);
    }
    Serial.println("\nWiFi connection Successful");
    Serial.print("The IP Address of ESP8266 Module is: ");
    Serial.print(WiFi.localIP());// Print the IP address

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("CONNECTED");
    display.println("TO WIFI");
    display.setTextSize(1);
    display.print("IP:");
    display.print(WiFi.localIP());
    display.display();
    delay(3000);
  }

  currentMillis = millis();
  currentMillis2 = millis();
  if (currentMillis - previousMillis > interval)
  {

    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(float(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    display.clearDisplay();

    display.setCursor(24, 0); //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("Projek FYP INA");

    display.setCursor(0, 20); //oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("R:");
    display.print(float(flowRate));
    display.setCursor(100, 28); //oled display
    display.setTextSize(1);
    display.print("L/M");

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");

    display.setCursor(0, 45); //oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("V:");
    display.print(totalLitres);
    display.setCursor(100, 53); //oled display
    display.setTextSize(1);
    display.print("L");
    display.display();

    // Set the fields with the values
    ThingSpeak.setField(1, flowRate);
    ThingSpeak.setField(2, totalLitres);
  }

  if (currentMillis2 - previousMillis2 > interval2) {
    // Write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    }
    else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    previousMillis2 = millis();
  }

}
