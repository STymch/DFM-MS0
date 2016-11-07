/*
  The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device) 
 */

#include <SoftwareSerial.h>

const int   LED_PIN = 13;				// LED pin
const int	RX_PIN = 10;				// Software UART RX pin
const int	TX_PIN = 11;				// Software UART TX pin
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM
//const long  DR_SOFTWARE_COM = 9600;	// Data rate for software COM
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM for AT command

SoftwareSerial BTSerial(RX_PIN, TX_PIN); // Software UART RX, TX for Bluetooth HC-05

void setup() {
  // Define pin modes for software TX, RX:
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  
  // Set the data rate and open hardware COM portort:
  Serial.begin(DR_HARDWARE_COM);
  while (!Serial) 
  {
	; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting hardware COM!");
  delay(1000);

  // Set the data rate and open software COM portort:
  BTSerial.begin(DR_SOFTWARE_COM);
  //BTSerial.println("Starting BT software COM");
  delay(1000);
}

void loop()
{
	// Read byte from hardware COM -> write it into software COM
	/*while (Serial.available())
	{
		// Read byte
		byte b = Serial.read();
		// Write byte to hardware COM
		Serial.write(b);
		// Write byte to software COM
		BTSerial.write(b);
	}*/
	// Read result of AT command from software COM
	while (BTSerial.available())
	{
		// Read byte from software COM and then write it into hardware COM
		byte b = BTSerial.read();
		Serial.write(b);	
	}
	 
}

void serialEvent()
{
	// Read byte
	byte b = Serial.read();
	// Write byte to hardware COM
	Serial.write(b);
	// Write byte to software COM
	BTSerial.write(b);
}