// 
// 
// 

#include "CSerialPort.h"

// Initialisation of COM port
void CSerialPort::init()
{
	if (TYPE_COM_BT == isCOMofBT_Software) m_SSerial.begin(DR_COM);	// Arduino SoftwareSerial COM 
	else
	{
		Serial.begin(DR_COM);	// Arduino Hardware COM port
		while (!Serial) {}		// Wait for serial port to connect. Needed for native USB
	}

}

// Sends a byte with the special value and repeats that until it gets a serial response from the remote sidee. 
void CSerialPort::EstablishContact() {
	if (TYPE_COM_BT == isCOMofBT_Software)		// Arduino SoftwareSerial COM 
		while (m_SSerial.available() <= 0) {
			m_SSerial.write(CONTACT_BYTE);		// send a byte
			delay(CONTACT_DELAY);
		}
	else										// Arduino Hardware COM port	
		while (Serial.available() <= 0) {
			Serial.write(CONTACT_BYTE);			// send a byte
			delay(CONTACT_DELAY);
		}
}
// Read byte from COM port
BYTE CSerialPort::Read() {
	BYTE b = 0;

	if (TYPE_COM_BT == isCOMofBT_Software) {
		if (m_SSerial.available() > 0) b = m_SSerial.read();	// Arduino SoftwareSerial COM 
	}
	else
		if (Serial.available() > 0) b = Serial.read();			// Arduino Hardware COM port
	
	return b;
}
// Reads bytes from the COM port into an array. First byte is quantity of bytes after this 
BYTE Read(BYTE* pBuffer) {
	BYTE n;		// length of array after first byte
	UINT i = 0;

	if (TYPE_COM_BT == isCOMofBT_Software{			// Arduino SoftwareSerial COM 
		n = Read();	pBuffer[i++] = n;
		while (i < b + 1) {

		}
	
	
	}
	else bytes = Serial.readBytes(pBuffer, bLength);	// Arduino Hardware COM port

	return bytes;
}

// Write byte into COM port
void CSerialPort::Write(BYTE bData) {
	if (TYPE_COM_BT == isCOMofBT_Software) m_SSerial.write(bData);	// Arduino SoftwareSerial COM 
	else Serial.write(bData);				// Arduino Hardware COM port
}
// Write bytes from array into COM port
void CSerialPort::Write(BYTE* pBuffer, UINT nLength) {
	if (TYPE_COM_BT == isCOMofBT_Software) m_SSerial.write(pBuffer, nLength);	// Arduino SoftwareSerial COM 
	else Serial.write(pBuffer, nLength);				// Arduino Hardware COM port
}



CSerialPort* pCSerialPort;

