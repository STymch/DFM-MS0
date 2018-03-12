// CTemperatureSensor.h

#ifndef _CTEMPERATURESENSOR_h
#define _CTEMPERATURESENSOR_h

#include "CommDef.h"
#include <DallasTemperature.h>

///////////////////////////////////////////////////////
// <<< CTemperatureSensor - class for Dallas temperature sensor
///////////////////////////////////////////////////////

// Names of sensors
const CHAR	*const DS_MODEL_NAME[6] = { "NOT DETECTED", "DS18S20", "DS18B20", "DS1822", "DS1825", "DS28EA00" };

class CTemperatureSensor
{
protected:
	INT					m_nDS_PIN;				// Input pin for connect temperature sensor Maxim/Dallas ICs
	INT					m_nSensorPrecision;		// Sensor resolution
	INT					m_nTypeSensor;			// Type of sensor: 0 - not detected, 1 - DS18S20, 2 - DS18B20, 3 - DS1822, 4 - DS1825, 5 - DS28EA00
	INT					m_nNumberOfDevices;		// Count of devices on the wire
	BOOL				m_isParasitePowerMode;	// Parasite power requirements: 1 - ON, 0 - OFF
	DeviceAddress		m_DevAddr;				// Temperature sensor adresses
	OneWire				*m_pOneWire;			// OneWire instance to communicate with OneWire devices
	DallasTemperature	*m_pDT;					// Dallas Temperature

public:
	// Constructor, destructor
	CTemperatureSensor(INT nDS_pin, INT nSensorPrecision);
	~CTemperatureSensor() { delete m_pDT; delete m_pOneWire; }

	// --- CONSTANT methods
	INT	GetTypeSensor()		const	{ return m_nTypeSensor;			}
	INT GetNumberOfDevices()const	{ return m_nNumberOfDevices;	}
	BOOL isParasitePower()	const	{ return m_isParasitePowerMode; }
		
	// Get temperature from sensor in celsius, Return:	0 - OK, -1 - sensor error, fTemperature = -1.0
	INT	GetTemperature(FLOAT& fTemperature) const
	{
		INT		rc = 0;

		// Send the command to get temperatures
		m_pDT->requestTemperatures();

		// Get the temperature from the sensor
		if ((fTemperature = m_pDT->getTempC(m_DevAddr)) == DEVICE_DISCONNECTED_C) {
			fTemperature = -1.0;	rc = -1;
		}
			
		return rc;
	}
};

#endif

