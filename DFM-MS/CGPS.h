// CGPS.h

#ifndef _CGPS_h
#define _CGPS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "CommDef.h"
#include <SoftwareSerial.h>
#include <TinyGPS.h>

///////////////////////////////////////////////////////
// <<< CGPS - class for GPS module as serial device
// Use a TinyGPS object and SoftwareSerial
///////////////////////////////////////////////////////
class CGPS
{
 protected:
	TinyGPS			*m_pGPS;		// TinyGPS object for access GPS function
	SoftwareSerial	*m_pSerialGPS;	// Software Serial object for communicate with GPS module
	
	LONG			m_lDataRate;	// Data rate for software serial COM of GPS
	INT				m_nRX_PIN;		// Software UART RX pin, connect to TX of GPS module
	INT				m_nTX_PIN;		// Software UART TX pin, connect to RX of GPS module
	DWORD			m_lUpdPeriod;	// Delay between coordinate updates (GPS)
	
public:
	// Constructor, destructor
	CGPS(INT nRX, INT nTX, LONG lDR_COM, DWORD lPeriod): m_lUpdPeriod (lPeriod)
	{
		// Initialization
		m_nRX_PIN = nRX;
		m_nTX_PIN = nTX;
		m_lDataRate = lDR_COM;

		// Create objects
		m_pGPS			= new TinyGPS;
		m_pSerialGPS	= new SoftwareSerial(m_nRX_PIN, m_nTX_PIN);

		// Initialization of serial port GPS
		m_pSerialGPS->begin(m_lDataRate);
	}
	~CGPS() { }

	// --- MODIFYING methods
	// Did a new valid sentence come in, Return: true - new data present, false - no new data
	BOOL WaitNewData();
	
	// Get Latitude and Longitude, Return:	0 - OK, -1 - no GPS module, check wiring
	INT	GetGPS_Position(FLOAT& fLat, FLOAT& fLon);
};

#endif

