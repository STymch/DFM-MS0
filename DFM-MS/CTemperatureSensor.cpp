#include "CTemperatureSensor.h"

// Constructor
CTemperatureSensor::CTemperatureSensor(INT nDS_pin, INT nSensorPrecision)
{
	// Initialisate variables
	m_nDS_PIN = nDS_pin;
	m_nSensorPrecision = nSensorPrecision;
	pinMode(m_nDS_PIN, INPUT);
	
	// Create objects
	m_pOneWire = new OneWire(m_nDS_PIN);
	m_pDT = new DallasTemperature(m_pOneWire);

	// Start up the Dallas Temperature library
	m_pDT->begin();

	// Grab a count of devices on the wire
	if ( (m_nNumberOfDevices = m_pDT->getDeviceCount()) > 0) {
		// Report parasite power requirements
		m_isParasitePowerMode = m_pDT->isParasitePowerMode();

		// Search the wire for address
		m_pDT->getAddress(m_DevAddr, 0);
		
		// Set the resolution
		m_pDT->setResolution(m_DevAddr, m_nSensorPrecision);

		// The first ROM byte indicates model
		switch (m_DevAddr[0]) {
		case DS18S20MODEL:	m_nTypeSensor = 1;
			break;
		case DS18B20MODEL:	m_nTypeSensor = 2;
			break;
		case DS1822MODEL:	m_nTypeSensor = 3;
			break;
		case DS1825MODEL:	m_nTypeSensor = 4;
			break;
		case DS28EA00MODEL:	m_nTypeSensor = 5;
			break;
		default:			m_nTypeSensor = 0;
			break;
		}
	}
}
