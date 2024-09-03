/* PIN CONNECTION
  MAX30102 -> [SCL -> D1], [SDA -> D2]
  DHT22 -> [DATA -> D0]
  LM35 -> [OUT -> A0]
  HX711 -> [HX711_DOUT -> D5], [HX711_CLK -> D6]
*/

#include <ESP8266WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h"
#include <DHT.h>
#include "HX711.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define HX711_DOUT D5
#define HX711_CLK D6	
#define DHTPIN D0       						// DHT sensor signal pin is connected to (D0)
#define DHTTYPE DHT22   						// DHT 22 type is in use

const int LM35 = A0;	              // LM35 pin connected to A0
float resolution = 3300 / 1023;
char ssid[] = SECRET_SSID ;         // your network SSID (name)
char pass[] = SECRET_PASS;          // your network password

WiFiClient  client;
MAX30105 particleSensor;
DHT dht(DHTPIN, DHTTYPE);
HX711 scale;

unsigned long myChannelNumber = SECRET_CH_ID;		    // Insert Channel ID here
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;	   // Insert API Key here

// MAX3010x variables
const byte RATE_SIZE = 4;           //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];              //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;                  //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 20000;   // Wait 20 seconds to update the channel again. This is due to ThingSpeak Free Account limitation.

// Variable to hold sensor readings
float room_temp = 0, room_humidity = 0, body_temp = 0, weight = 0, iv_bag = 0;

// Calibration factor for HX711 scale
float calibration_factor = -535262/500 ;  // user set calibration factor (float)

void setup() {
  Serial.begin(115200);								    //Initialize serial
  Serial.println();
  delay(3000);
  Serial.println(F("Configure ESP8266 into Station Mode"));
  WiFi.mode(WIFI_STA);
  //Check status for connect and reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);						// Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print("*");
      delay(5000);
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  // Initialize ThingSpeak
  Serial.print(F("Initializing ThingSpeak..."));
  ThingSpeak.begin(client);
  Serial.println(F("DONE"));
  // Initialize DHT22
  Serial.println("Initializing DHT22");
  dht.begin();	
  Serial.println(F("DONE"));
  // Initialize HX711 Scale
  Serial.print(F("Initializing HX711 Scale..."));
  scale.begin(HX711_DOUT, HX711_CLK);
  scale.set_scale();
  scale.tare();
  Serial.println(F("DONE"));
  long zero_factor = scale.read_average();			// Get baseline reading
  Serial.print("Scale zero factor: ");
  Serial.println(zero_factor);
  
  // Initialize MAX3010x sensor
  Serial.print(F("Initializing MAX30102..."));
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))	//Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    //while (1);
  }
  Serial.println(F("DONE"));
  Serial.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup();                     //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);  //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);   //Turn off Green LED
}

void loop() {
// Check WiFi connection status.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to restore connection to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);						// Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print("*");
      delay(5000);
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  delay(2000);
  
  // Get reading for patient's heart pulse
  heartbeat_read();
  
  // Get room temperature & humidity sensor reading
  room_temp = dht.readTemperature();
  room_humidity = dht.readHumidity();
  
  // Get patient body temperature reading
  body_temp = ((analogRead(LM35) * resolution) / 10) ;

  // Get IV Fluid bag mass
  iv_scale();
  
  Serial.print("Temp: ");
  Serial.print(room_temp);
  Serial.print("°C");
  Serial.print(" , Humidity: ");
  Serial.print(room_humidity);
  Serial.print("%");
  Serial.print(" , Body Temp: ");
  Serial.print(body_temp++);
  Serial.print("°C");
  Serial.print(" , IV Bag: ");
  Serial.print(weight);
  Serial.print("KG");
  Serial.print(" , BPM: ");
  Serial.println(beatsPerMinute);
  
  // wait until timerDelay for publish data to ThingSpeak
  if ((millis() - lastTime) >= timerDelay) {
    publish_data();
  }
}

void publish_data() {
  
  // set the fields with the values
  ThingSpeak.setField(1, room_temp);
  ThingSpeak.setField(2, room_humidity);
  ThingSpeak.setField(3, body_temp);
  ThingSpeak.setField(4, iv_bag);
  ThingSpeak.setField(5, beatsPerMinute);

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) { //ThinkSpeak server will return HTTP status code of 200 if successful.
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  lastTime = millis();
}

void heartbeat_read() {
  
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //Heartbeat detected!
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable
      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
/* For Debug purpose
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
*/
  if (irValue < 50000)
    Serial.println(F(" No finger detected?"));
	beatsPerMinute = 0;
    beatAvg = 0;
}

void iv_scale() {
  if (scale.wait_ready_timeout(1000)) {
    scale.set_scale(calibration_factor);					// Adjust to the calibration_factor
    weight = scale.get_units(10);
    iv_bag = weight;
  } else {
    Serial.println(F("HX711 not found."));
	  iv_bag = 0;
  }
}
