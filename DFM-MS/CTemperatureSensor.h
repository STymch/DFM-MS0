// CTemperatureSensor.h

#ifndef _CTEMPERATURESENSOR_h
#define _CTEMPERATURESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <OneWire.h>
#include "CommDef.h"

class CTemperatureSensor
{
 protected:
	 int		m_nDS_PIN;				// Input pin for connect temperature sensor Maxim/Dallas ICs
	 int		m_nTypeSensor;			// Type of sensor: 0 - not detected, 1 - DS18S20 or DS1820, 2 - DS18B20, 3 - DS1822	
	 char		m_strSensorName[10];	// Name of sensor
	 byte		m_pbAddr[8];			// For temperature sensor adresses
	 DWORD		m_lTDelayTempSensor;	// Delay (ms) for read temperature sensor
	 OneWire	*m_pDS;					// A oneWire instance to communicate with OneWire devices

public:
	// Constructor, destructor
	CTemperatureSensor( int nDS_pin, DWORD lTDelayTempSensor) {
		m_nDS_PIN = nDS_pin;
		m_nTypeSensor = 0;
		m_lTDelayTempSensor = lTDelayTempSensor;
		
		m_pDS = new OneWire(m_nDS_PIN);
			
		pinMode(m_nDS_PIN, INPUT);
	}
	~CTemperatureSensor() { delete m_pDS; }
	
	// Methods
	int	Detect();							// Detect temperature sensor
	int	GetTemperature(float& fTemperature);// Get temperature from sensor: 0 - celsius, 1 - fahrenheit
};


#endif

