
#include "CommDef.h"

// SoftwareSerial library
#include <SoftwareSerial.h>

const int   LED_PIN = 13;				// LED pin
const int	RX_PIN = 10;				// Software UART RX pin, connect to TX of Bluetooth HC-05 
const int	TX_PIN = 11;				// Software UART TX pin, connect to RX of Bluetooth HC-05
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM

SoftwareSerial* BTSerial;// (RX_PIN, TX_PIN); // Software UART RX, TX for Bluetooth HC-05


byte pBuff[256]; // = new byte[256];
UINT i, n;
long lCount = 0;

struct StatusByte {
	UINT b0 : 1;
	UINT b1 : 1;
	UINT b2 : 1;
	UINT b3 : 1;
	UINT b4_7 : 4;
};

StatusByte bits;
union UStatusByte {
	byte bByte;
	StatusByte bStatus;
};
UStatusByte uByte;
// int nTypeSerial = 1; // 0 - hardware, 1 - software



void setup() {

uByte.bByte = 0; uByte.bStatus.b0 = 1; uByte.bStatus.b1 = 1; uByte.bStatus.b2 = 1; uByte.bStatus.b4_7 = 15;
int nTypeSerial = 1; // 0 - hardware, 1 - software



// Define pin modes for software TX, RX:
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  
  // Set the data rate and open hardware COM port:
  Serial.begin(DR_HARDWARE_COM);
  
  // Wait for serial port to connect. Needed for native USB port only
  while (!Serial);
  Serial.println("Starting hardware COM!");
  Serial.println(uByte.bByte, HEX);
  Serial.println(sizeof(BYTE));
  Serial.println(sizeof(WORD));
  Serial.println(sizeof(UINT));
  Serial.println(sizeof(INT));
  Serial.println(sizeof(DWORD));
  Serial.println(sizeof(FLOAT));
  Serial.println(sizeof(QWORD));

  Serial.println();

  Serial.println(sizeof(uint8_t));
  Serial.println(sizeof(uint16_t));
  Serial.println(sizeof(uint32_t));
  Serial.println(sizeof(uint64_t));
  
  delay(1000);

  // Set the data rate and open software COM port:
  if (nTypeSerial == 1) {
	  BTSerial = new SoftwareSerial(RX_PIN, TX_PIN);
	  BTSerial->begin(DR_SOFTWARE_COM);
	  // BTSerial.println("Starting BT software COM");
	  delay(1000);
	}
}

void loop()
{
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
}