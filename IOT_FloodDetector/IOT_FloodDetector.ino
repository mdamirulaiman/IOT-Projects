#include <NewPing.h>

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define RELAY_CH1    2
#define RELAY_CH2    3
#define RELAY_CH3    4
#define RELAY_CH4    5
#define buzzer       6
int distance = 0, setpoint_1 = 40, setpoint_2 = 50;
int ping_time = 50;
unsigned long time_now = 0, time_buzzer = 0;
bool beep = false;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup() {
	Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
	pinMode(RELAY_CH1, OUTPUT);
	pinMode(RELAY_CH2, OUTPUT);
	pinMode(RELAY_CH3, OUTPUT);
	pinMode(RELAY_CH4, OUTPUT);
	pinMode(buzzer, OUTPUT);
}

void loop() {
	
	if(millis() - time_now > ping_time){
		time_now = millis();					// Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
		Serial.print("Flood level: ");
		distance = sonar.ping_cm();
		Serial.print(distance);				// Send ping, get distance in cm and print result (0 = outside set distance range)
		Serial.println("cm");
	}
	
	if(distance < setpoint){
		digitalWrite(RELAY_CH1, LOW);
		digitalWrite(RELAY_CH2, LOW);
		digitalWrite(RELAY_CH3, LOW);
		digitalWrite(RELAY_CH4, LOW);
		tone(8, 262, 250);
	}else{
		digitalWrite(RELAY_CH1, HIGH);
		digitalWrite(RELAY_CH2, HIGH);
		digitalWrite(RELAY_CH3, HIGH);
		digitalWrite(RELAY_CH4, HIGH);
		noTone(buzzer);
	}

	if(millis() - time_buzzer > 500){
		beep = !beep;
		if (beep){
			tone(8, 262, 250);
		}else{
			noTone(buzzer);
		}

	}
}