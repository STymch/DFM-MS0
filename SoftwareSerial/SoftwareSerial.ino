/*
 Name:		SoftwareSerial.ino
 Created:	05.06.2017 13:43:41
 Author:	Sergiy Tymchenko
 
 For work in AT-command mode for HC-0x bluetooth modules

*/

// SoftwareSerial library
#include <SoftwareSerial.h>

const int   LED_PIN = 13;				// LED pin
const int	RX_PIN = 10;				// Software UART RX pin, connect to TX of Bluetooth HC-0x 
const int	TX_PIN = 11;				// Software UART TX pin, connect to RX of Bluetooth HC-0x
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM. Default for HC-05=38400, for HC-06,08=9600
int			nLEDState = LOW;			// LED state

SoftwareSerial BTSerial(RX_PIN, TX_PIN);

void setup() {
	// Define pin modes for software TX, RX:
	pinMode(RX_PIN, INPUT);
	pinMode(TX_PIN, OUTPUT);

	// Set the data rate and open hardware COM port:
	Serial.begin(DR_HARDWARE_COM);
	
	// Set the data rate and open software COM port:
	BTSerial.begin(DR_SOFTWARE_COM);
	
	// Wait for hardware serial port to connect. Needed for native USB port only
	while (!Serial);
	Serial.print("Starting hardware COM =");	Serial.println(DR_HARDWARE_COM); 
	Serial.print("Starting software COM =");	Serial.println(DR_SOFTWARE_COM);
	
	// Set pin mode for LED pin
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, nLEDState);

	delay(1000);
}

void loop() {
	// Read byte from software port => translate into hardware port
	if (BTSerial.available()) {
		Serial.write(BTSerial.read());
		nLEDState = !nLEDState;
		digitalWrite(LED_PIN, nLEDState);
	}
	
	// Read byte from hardware port => translate into software port
	if (Serial.available()) {
		BTSerial.write(Serial.read());
		nLEDState = !nLEDState;
		digitalWrite(LED_PIN, nLEDState);
	}
}
