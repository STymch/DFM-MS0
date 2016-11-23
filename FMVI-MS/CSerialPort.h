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
	const long  DR_COM;			// Data rate for serial COM
	const INT	TYPE_COM_BT;	// Type of Arduino serial COM of Bluetooth modem: 0-Hardware COM (RX=0, TX=1), 1-Software  
	const INT	RX_PIN;			// Software UART RX pin, connect to TX of Bluetooth modem
	const INT	TX_PIN;			// Software UART TX pin, connect to RX of Bluetooth modem
	const BYTE	CONTACT_BYTE;	// Byte for establish contact with remote side
	const UINT	CONTACT_DELAY;	// Delay in mls before repeat send byte

protected:
	SoftwareSerial m_SSerial;	// SoftwareSerial UART

public:
	// Types of Bluetooth modem's COM port
	enum TypeCOMofBT{isCOMofBT_Hardware=0, isCOMofBT_Software = 1};

	// Constructor, destructor
	CSerialPort(long lDR_COM, INT nTypeCOM, INT nRX, INT nTX, BYTE b, UINT delay) :
		DR_COM(lDR_COM), TYPE_COM_BT(nTypeCOM), RX_PIN(nRX), TX_PIN(nTX), 
		CONTACT_BYTE(b), CONTACT_DELAY(delay), m_SSerial(nRX, nTX) {}
	~CSerialPort() {}
	
	// Initialisation of COM port
	void init();
	
	// Sends a byte with the special value and repeats that until it gets a serial response from the remote sidee. 
	void CSerialPort::EstablishContact();

	// Read byte from COM port
	BYTE Read();
	// Reads bytes from the COM port into an array
	BYTE Read(BYTE*, UINT);
	
	// Write byte into COM port
	void Write(BYTE);
	// Write bytes from array into COM port
	void Write(BYTE*, UINT);
};

extern CSerialPort* pCSerialPort;	//			

#endif

