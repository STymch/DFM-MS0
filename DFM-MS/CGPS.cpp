
#include "CGPS.h"
//////////////////////////////////////////////////////
// <<< CGPS - class for GPS module as serial device
// Use a TinyGPS object and SoftwareSerial
///////////////////////////////////////////////////////
// Get Latitude and Longitude, Return:	0 - OK, -1 - sensor error
INT	CGPS::GetGPS_Position(FLOAT& fLat, FLOAT& fLon)
{
	INT		rc = 0;
	DWORD	chars;
	WORD	sentences, failed;
	
	// Check new data from GPS
	if (WaitNewData())
	{
		m_pGPS->f_get_position(&fLat, &fLon);
		fLat = (fLat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : fLat);
		fLon = (fLon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : fLon);
	}
	
	// Check statistics
	m_pGPS->stats(&chars, &sentences, &failed);
	if (chars == 0)  // No characters received from GPS: check wiring
		rc = -1;

	return rc;
}

// Did a new valid sentence come in, Return: true - new data present, false - no new data
BOOL CGPS::WaitNewData() 
{
	// No new data
	m_isNewData = false;

	// Waiting new data from GPS
	m_pSerialGPS->listen();	// swtch software serial port
	while (m_pSerialGPS->available())
	{
		// Read data from serial port GPS
		char c = m_pSerialGPS->read();
		if (m_pGPS->encode(c)) // Did a new valid sentence come in?
			m_isNewData = true;
	}
	return m_isNewData;
}

