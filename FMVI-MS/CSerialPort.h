// CSerialPort.h

#ifndef _CSERIALPORT_h
#define _CSERIALPORT_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

///////////////////////////////////////////////////////
// <<< CSerialPort - class for Arduino Serial COM port
///////////////////////////////////////////////////////
class CSerialPort
{
protected:
	//long	m_lDataRate;				// Data rate for serial COM
	INT		m_nTypeOfCOM_BT;			// Type of Arduino COM of Bluetooth modem: 0-Hardware COM (RX=0, TX=1), 1-Software  
	//INT		m_nRX_PIN;					// Software UART RX pin, connect to TX of Bluetooth modem
	//INT		m_nTX_PIN;					// Software UART TX pin, connect to RX of Bluetooth modem
	BYTE	m_bContactByte;				// Byte for establish contact with remote side
	UINT	m_nSendContactByteDelay;	// Delay in mls before repeat send byte
	long	m_lSerialTimeout;			// Maximum milliseconds to wait for serial data when using Read(BYTE*)

	SoftwareSerial* m_pSSerial;			// SoftwareSerial COM

public:
	// Names of types of Bluetooth modem's COM port
	enum TypeCOMofBT{isCOMofBT_Hardware=0, isCOMofBT_Software = 1};

	// Constructor, destructor
	CSerialPort()	{}
	~CSerialPort()	{}
	
	// Initialisation of SerialPort
	void init(); (long, INT, INT, INT, BYTE UINT, long);
	
	// Sends a byte with the special value and repeats that until it gets a serial response from the remote sidee. 
	void EstablishContact();
	
	// Read byte from COM port
	INT Read();
	// Reads bytes from the COM port into an array, first byte is length of data after it
	INT Read(BYTE*);
	// Set the maximum milliseconds to wait for serial data when using Read(BYTE*)
	vod SetReadTimeout(long lTimeout) {m_lSerialTimeout = lTimeout;}

	// Write byte into COM port
	void Write(BYTE);
	// Write bytes from array into COM port
	void Write(BYTE*, UINT);
};

extern CSerialPort* pCSerialPort;	//			

#endif

