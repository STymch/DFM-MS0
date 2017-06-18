// CRHTSensor.h

#ifndef _CRHTSENSOR_h
#define _CRHTSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
/*
#if defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny26__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny45__) || (__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
#include <TinyWireM.h>
#define Wire TinyWireM
#else defined
#include <Wire.h>
#endif
*/

#include <HTU21D.h>

class CRHTSensor
{
 protected:
	 int	m_nSensorID;		// Sensor name
	 int	m_nFirmwareVersion;	// Firmware Version
	 HTU21D *m_pHTU21D;			// HTU21D object

 public:
	// Constructor, destructor
	CRHTSensor() {
		m_pHTU21D = new HTU21D;
	}
	~CRHTSensor() { delete m_pHTU21D; }
	
	// Methods
	// Detect RHT sensor
	// Return:	0 - sensor OK, -1 - no sensor, 1 - Battery LOW
	int		Detect();

	// Get RH and temperature from sensor
	// Return:	0 - sensor OK, -1 - no sensor, 1 - Battery LOW
	int		GetRHT(float &fHumidity, float &fTemperature);
};


#endif

