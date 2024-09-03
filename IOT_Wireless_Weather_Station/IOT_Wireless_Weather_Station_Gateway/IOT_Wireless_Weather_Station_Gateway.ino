#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>
 
 
#define SS 5
#define RST 14
#define DI0 2
 
//#define TX_P 17
#define BAND 433E6
//#define ENCRYPT 0x78
 
String device_id;
String temperature;
String pressure;
String altitude;
String humidity;
String dewPoint;
String rainfall;
String lux;
 
const char* ssid = "************";
const char* password = "********";
 
WebServer server(80);
 
void setup()
{
  Serial.begin(115200);
  Serial.println("LoRa Receiver");
  //LoRa.setTxPower(TX_P);
  //LoRa.setSyncWord(ENCRYPT);
 
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  Serial.println("Connecting to ");
  Serial.println(ssid);
 
  //Connect to your local wi-fi network
  WiFi.begin(ssid, password);
 
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
 
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
 
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop()
{
  // try to parse packet
  int pos1, pos2, pos3, pos4, pos5, pos6, pos7;
 
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packet
    Serial.print("Received packet:  ");
    String LoRaData = LoRa.readString();
    Serial.print(LoRaData);
    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
 
    pos1 = LoRaData.indexOf('/');
    pos2 = LoRaData.indexOf('&');
    pos3 = LoRaData.indexOf('#');
    pos4 = LoRaData.indexOf('@');
    pos5 = LoRaData.indexOf('$');
    pos6 = LoRaData.indexOf('^');
    pos7 = LoRaData.indexOf('!');
 
    device_id = LoRaData.substring(0, pos1);
    temperature = LoRaData.substring(pos1 + 1, pos2);
    pressure = LoRaData.substring(pos2 + 1, pos3);
    altitude = LoRaData.substring(pos3 + 1, pos4);
    humidity = LoRaData.substring(pos4 + 1, pos5);
    dewPoint = LoRaData.substring(pos5 + 1, pos6);
    rainfall = LoRaData.substring(pos6 + 1, pos7);
    lux = LoRaData.substring(pos7 + 1, LoRaData.length());
 
    Serial.print(F("Device ID = "));
    Serial.println(device_id);
 
    Serial.print(F("Temperature = "));
    Serial.print(temperature);
    Serial.println(F("*C"));
 
    Serial.print(F("Pressure = "));
    Serial.print(pressure);
    Serial.println(F("hPa"));
 
    Serial.print(F("Approx. Altitude = "));
    Serial.print(altitude);
    Serial.println(F("m"));
 
    Serial.print(F("Humidity = "));
    Serial.print(humidity);
    Serial.println(F("%"));
 
    Serial.print("Dew point = ");
    Serial.print(dewPoint);
    Serial.println(" *C");
 
    Serial.print(F("Rainfall = "));
    Serial.print(rainfall);
    Serial.println(F("%"));
 
    Serial.print(F("Light = "));
    Serial.print(lux);
    Serial.println(F(" lx"));
 
    Serial.println();
    
    server.handleClient();
  }
}
 
void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML(temperature.toFloat(), humidity.toFloat(), pressure.toFloat(), altitude.toFloat(), dewPoint.toFloat(), rainfall.toFloat(), lux.toFloat()));
}
 
 
void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}
 
String SendHTML(float temperature, float humidity, float pressure, float altitude, float dewPoint, float rainfall, float lux)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Wireless Weather Station</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>Wireless Weather Station</h1>\n";
 
  ptr += "<p>Temperature: ";
  ptr += temperature;
  ptr += "&deg;C</p>";
 
  ptr += "<p>Humidity: ";
  ptr += humidity;
  ptr += "%</p>";
 
  ptr += "<p>Pressure: ";
  ptr += pressure;
  ptr += "hPa</p>";
 
  ptr += "<p>Altitude: ";
  ptr += altitude;
  ptr += "m</p>";
 
  ptr += "<p>Dew Point: ";
  ptr += dewPoint;
  ptr += "&deg;C</p>";
 
  ptr += "<p>Rainfall: ";
  ptr += rainfall;
  ptr += "%</p>";
 
  ptr += "<p>Light: ";
  ptr += lux;
  ptr += "lx</p>";
 
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
