// 
// 
// 

#include "CSerialPort.h"

// Constructor
CSerialPort::CSerialPort(INT nTypeCOM, INT nRX, INT nTX) {
	m_nTypeOfCOM_BT = nTypeCOM;			// Type of Arduino COM of Bluetooth modem: 0-Hardware COM (RX=0, TX=1), 1-Software  
	m_nRX_PIN = nRX;					// Software UART RX pin, connect to TX of Bluetooth modem
	m_nTX_PIN = nTX;					// Software UART TX pin, connect to RX of Bluetooth modem

	// Define pin modes for software TX, RX:
	pinMode(m_nRX_PIN, INPUT);
	pinMode(m_nTX_PIN, OUTPUT);

	// Arduino SoftwareSerial COM 
	if (m_nTypeOfCOM_BT == isCOMofBT_Software)
		m_pSSerial = new SoftwareSerial(m_nRX_PIN, m_nTX_PIN);	// Create SoftwareSerial COM
	else m_pSSerial = NULL;
}
// Initialisation of COM port: set data rate of COM port
void CSerialPort::Init(long lDR_COM, BYTE b, UINT nDelay, long lTimeOut)
{
	m_lDataRate	= lDR_COM;				// Data rate for serial COM
	m_bContactByte = b;					// Byte for establish contact with remote side
	m_nSendContactByteDelay = nDelay;	// Delay in mls before repeat send byte
	m_lSerialTimeout = lTimeOut;		// Maximum milliseconds to wait for serial data when using Read(BYTE*)
	
	
	// Initialization of COM 
	if (m_nTypeOfCOM_BT == isCOMofBT_Software)	// Arduino SoftwareSerial COM 
		m_pSSerial->begin(m_lDataRate);			// Initialization of COM 
	else										// Arduino Hardware COM port
	{
		Serial.begin(m_lDataRate);	// Initialization of COM 
		while (!Serial) {}			// Wait for serial port to connect. Needed for native USB
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
// Return:	-1	- no byte read, else - reading byte 	
INT CSerialPort::Read() {
	INT b = -1;

	if (m_nTypeOfCOM_BT == isCOMofBT_Software) {
		if (m_pSSerial->available() > 0) b = m_pSSerial->read();// Arduino SoftwareSerial COM 
	}
	else
		if (Serial.available() > 0) b = Serial.read();			// Arduino Hardware COM port
	
	return b;
}
// Read data from COM port into byte array. First byte = SIZE of data, array must be [SIZE+1]. 
// Return:	>0 - size of byte array, 
//			-1 - no bytes available in port,
//			-2 - not all bytes read, data must be re-read,
//			-3 - data size is big.
INT CSerialPort::Read(BYTE *pBuffer, INT nMaxBuffLen) {
	INT		len;			// Size of data in array
	INT		b;				// Reading byte
	INT		i = 0;			// Index in array
	DWORD	dwTime;			// Time in microsec
	INT		retcode = -1;	// Return code - no bytes available in port

	// If available - read first byte = size of data
	if ((len = Read()) >= nMaxBuffLen)	retcode = -3;	// Data size is big
	else 
		if (len > 0) {
			pBuffer[i++] = (BYTE)len;				// Save first byte = size of data into output array
			dwTime = millis() + m_lSerialTimeout;	// Time of waiting of end reading in millisec
			// Read len bytes and save it into output array
			do {
				// If available - read byte, else - waiting timeout
				if ((b = Read()) > 0) pBuffer[i++] = (BYTE)b;
			} while ((i < len + 1) && (millis() <= dwTime));

			if (i == len + 1)	retcode = len + 1;	// Read all bytes successfully
			else	retcode = -2;					// Timeout when read byte - not all bytes read
		}

	return retcode;
}

// Write byte into COM port
void CSerialPort::Write(BYTE bData) {
	if (m_nTypeOfCOM_BT == isCOMofBT_Software) m_pSSerial->write(bData);	// Arduino SoftwareSerial COM 
	else Serial.write(bData);												// Arduino Hardware COM port
}
// Write bytes from array into COM port
void CSerialPort::Write(BYTE *pBuffer, INT nLength) {
	if (m_nTypeOfCOM_BT == isCOMofBT_Software) m_pSSerial->write(pBuffer, nLength);	// Arduino SoftwareSerial COM 
	else Serial.write(pBuffer, nLength);											// Arduino Hardware COM port
}
