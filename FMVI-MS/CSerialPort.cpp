// 
// 
// 

#include "CSerialPort.h"

// Initialisation of COM port: set data rate of COM port
void CSerialPort::init(long lDR_COM, INT nTypeCOM, INT nRX, INT nTX, BYTE b, UINT nDelay, long lTimeout)
{
//	m_lDataRate	= lDR_COM;				// Data rate for serial COM
	m_nTypeOfCOM_BT = nTypeCOM;			// Type of Arduino COM of Bluetooth modem: 0-Hardware COM (RX=0, TX=1), 1-Software  
//	m_nRX_PIN = nRX;					// Software UART RX pin, connect to TX of Bluetooth modem
//	m_nTX_PIN = nTX;					// Software UART TX pin, connect to RX of Bluetooth modem
	m_bContactByte = b;					// Byte for establish contact with remote side
	m_nSendContactByteDelay = nDelay;	// Delay in mls before repeat send byte
	m_lSerialTimeout = lTimeOut;		// Maximum milliseconds to wait for serial data when using Read(BYTE*)
	
	

	if (m_nTypeOfCOM_BT == isCOMofBT_Software)	// Arduino SoftwareSerial COM 
	{
		m_pSSerial = new SoftwareSerial(nRX, nTX);	// Create SoftwareSerial COM
		m_pSSerial->begin(lDR_COM);					// Initialization of COM 
	}
	else										// Arduino Hardware COM port
	{
		Serial.begin(lDR_COM);	// Initialization of COM 
		while (!Serial) {}		// Wait for serial port to connect. Needed for native USB
	}

}

// Sends a byte with the special value and repeats that until it gets a serial response from the remote sidee. 
void CSerialPort::EstablishContact() {
	if (m_nTypeOfCOM_BT == isCOMofBT_Software)		// Arduino SoftwareSerial COM 
		while (m_pSSerial->available() <= 0) {
			m_pSSerial->write(m_bContactByte);		// send a byte
			delay(m_nSendContactByteDelay);
		}
	else										// Arduino Hardware COM port	
		while (Serial.available() <= 0) {
			Serial.write(m_bContactByte);		// send a byte
			delay(m_nSendContactByteDelay);
		}
}
// Read byte from COM port
// Return:	-1	- byte no read, else - reading byte 	
INT CSerialPort::Read() {
	INT b = -1;

	if (m_nTypeOfCOM_BT == isCOMofBT_Software) {
		if (m_pSSerial->available() > 0) b = m_pSSerial.read();	// Arduino SoftwareSerial COM 
	}
	else
		if (Serial.available() > 0) b = Serial.read();			// Arduino Hardware COM port
	
	return b;
}
// Reads bytes from the COM port into an array. First byte is quantity of bytes after this 
// Return:	0 - all bytes read, -1	- bytes no read, -2 - less of data with read 	
INT CSerialPort::Read(BYTE* pBuffer) {
	INT		len;			// length of array after first byte
	INT		b;				// Reading byte
	UINT	i = 0;			// Index in array
	DWORD	dwTime;			// Time in microsec
	INT		retcode = -1;	// Return code - no bytes

	// If available - read first byte = length of data after it
	if ((len = Read()) > 0) {			
		pBuffer[i++] = (BYTE)len;		// Save first byte (length) into output array
		dwTime = micros();				// Get current time in microsec
		// Read len bytes and save it into output array
		while (i < len + 1 && LessEqual (micros(), dwTime+m_lSerialTimeout) {			
			// If available - read byte, else - waiting timeout
			if( (b = Read()) > 0) pBuffer[i++] = (BYTE)b;					
		}
		if (i == len + 1)	
			retcode = 0;	// Read all bytes successfully
		else
			retcode = -2;	// Timeout when read byte
	}
		
	return retcode;
}

// Write byte into COM port
void CSerialPort::Write(BYTE bData) {
	if (m_nTypeOfCOM_BT == isCOMofBT_Software) m_pSSerial->write(bData);	// Arduino SoftwareSerial COM 
	else Serial.write(bData);												// Arduino Hardware COM port
}
// Write bytes from array into COM port
void CSerialPort::Write(BYTE* pBuffer, UINT nLength) {
	if (m_nTypeOfCOM_BT == isCOMofBT_Software) m_pSSerial->write(pBuffer, nLength);	// Arduino SoftwareSerial COM 
	else Serial.write(pBuffer, nLength);											// Arduino Hardware COM port
}



CSerialPort* pSerialPort;

