
#include "CGPS.h"
//////////////////////////////////////////////////////
// <<< CGPS - class for GPS module as serial device
// Use a TinyGPS object and SoftwareSerial
///////////////////////////////////////////////////////
// Get Latitude and Longitude, Return:	0 - OK, 1 - no data, -1 - no GPS module, check wiring
INT	CGPS::GetGPS_Position(FLOAT& fLat, FLOAT& fLon)
{
	INT		rc = 0;
	DWORD	gpsChars;
	WORD	gpsGoodSentences, gpsFailedCheckSum;
	
	// Check new data from GPS
	if (WaitNewData()) {	
		// Get coordinates of position
		m_pGPS->f_get_position(&fLat, &fLon);
		fLat = (fLat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : fLat);
		fLon = (fLon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : fLon);
	}
	else {
		fLat = fLon = 0.0;
		rc = 1;
	}

	// Check statistics
	m_pGPS->stats(&gpsChars, &gpsGoodSentences, &gpsFailedCheckSum);
	// No characters received from GPS: no GPS module, check wiring
	if (gpsChars == 0) {
		fLat = fLon = -1.0;
		rc = -1;
	}

	return rc;
}

// Did a new valid sentence come in, Return: true - new data present, false - no new data
BOOL CGPS::WaitNewData() 
{
	DWORD	lStartTime = millis();	// start time
	BOOL	isNewData = false;		// no new data

	// Switch software serial port
	m_pSerialGPS->listen();
	
	// Waiting new data from GPS
	do {
		while (m_pSerialGPS->available() && !isNewData) {
			// Read data from serial port GPS, did a new valid sentence come in?
			if (m_pGPS->encode(m_pSerialGPS->read())) isNewData = true;
		}
	} while (millis() - lStartTime < m_lUpdPeriod && !isNewData);

	// Stop listening. Returns true if we were actually listening.
//	while(m_pSerialGPS->stopListening());

	return isNewData;
}

