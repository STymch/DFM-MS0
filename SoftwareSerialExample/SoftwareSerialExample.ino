
#include "CommDef.h"

// SoftwareSerial library
#include <SoftwareSerial.h>

const int   LED_PIN = 13;				// LED pin
const int	RX_PIN = 10;				// Software UART RX pin, connect to TX of Bluetooth HC-05 
const int	TX_PIN = 11;				// Software UART TX pin, connect to RX of Bluetooth HC-05
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM

// SoftwareSerial* BTSerial;// (RX_PIN, TX_PIN); // Software UART RX, TX for Bluetooth HC-05
SoftwareSerial BTSerial(RX_PIN, TX_PIN);

// byte pBuff[256]; // = new byte[256];
// UINT i, n;
// long lCount = 0;


void setup() {


// Define pin modes for software TX, RX:
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  
  // Set the data rate and open hardware COM port:
  Serial.begin(DR_HARDWARE_COM);
  BTSerial.begin(DR_SOFTWARE_COM);

  // Wait for serial port to connect. Needed for native USB port only
  while (!Serial);
  Serial.println("Starting hardware COM!");

  // Set the data rate and open software COM port:
  /*if (nTypeSerial == 1) {
	  BTSerial = new SoftwareSerial(RX_PIN, TX_PIN);
	  BTSerial->begin(DR_SOFTWARE_COM);
	  // BTSerial.println("Starting BT software COM");
	  delay(1000);
	}*/
}


void loop()
{
	int nH, nB;
	// Read byte from hardware COM -> write it into software COM
	while (nH=Serial.available())
	{
		Serial.print("H port: nH=");
		Serial.print(nH, HEX);
		// Read byte
		int b = Serial.read();
		Serial.print(",b=");
		// Write byte to hardware COM
		Serial.print(b, HEX);
		Serial.println();
		// Write byte to software COM
		BTSerial.write(b);
	}
	// Read result of AT command from software COM
	while (nB=BTSerial.available())
	{
		Serial.print("BT port: nB=");
		Serial.print(nB, HEX);
		// Read byte
		int b = BTSerial.read();
		Serial.print(",b=");
		Serial.print(b, HEX);
		Serial.println();
		// Read byte from software COM and then write it into hardware COM
		BTSerial.write(b);
	}
	
	/*
	
	
	// BTSerial = new SoftwareSerial(RX_PIN, TX_PIN);
	int nTypeSerial = 1; // 0 - hardware, 1 - software
	if (nTypeSerial == 1) {
		BTSerial->begin(DR_SOFTWARE_COM);
		// BTSerial.println("Starting BT software COM");
		delay(1000);
	}

	i = n = 0;
	if (Serial.available() > 0) {
		// Read length of data
		n = Serial.read()-39;
		Serial.println(n+1);
		pBuff[i++] = (byte)(n+39);
		while (i < n+1) {
			// Read data from hardware COM
			while (Serial.available() > 0) {
				// Read byte and add to buffer
				pBuff[i++] = Serial.read();
			}
		}
	}
	// Write buffer to hardware & software COM
	if (n > 0)
	{
		Serial.print("i1="); Serial.println(n+1);
		Serial.write(pBuff,n+1);
		Serial.println();
		
		BTSerial->write(pBuff, n+1);
		i = n = 0;
	}

	// Read data from software COM
	if (BTSerial->available() > 0) {
		// Read length of data
		n = BTSerial->read();
		Serial.print("N="); Serial.print(n);
		pBuff[i++] = (byte)n;
		while (i < n+1) {
			// Read data from software COM
			while (BTSerial->available() > 0) {
				// Read byte and add to buffer
				pBuff[i++] = BTSerial->read();
			}
		}
	}
	// Write buffer to software COM
	if (n > 0)
	{
		Serial.print(" Count=");	Serial.print(++lCount);
		Serial.print(" Buff=");		Serial.write(pBuff, n+1);
		Serial.println();

		BTSerial->write(pBuff, n+1);
		i = n = 0;
	}
*/
}