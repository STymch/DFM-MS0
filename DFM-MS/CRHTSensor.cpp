#include "CRHTSensor.h"

// Get RH and temperature from all sensors
// Return:	0 - OK, -1 - sensor error
int	CRHTSensor::GetRHT(float& fHumidity, float& fTemperature)
{
	int rc = 0;		// OK
					
	// No sensor detected
	m_nSensorModel = -1;
	
	// Get RH and temperature from HTU21D sensor
	// Read Air Humidity
	fHumidity = m_pHTU->readHumidity();
	if (fHumidity == ERROR_I2C_TIMEOUT || fHumidity == ERROR_BAD_CRC)
	{
		rc = -1; fHumidity = -1.0;
	}

	// Read Air Temperature
	fTemperature = m_pHTU->readTemperature();
	if (fTemperature == ERROR_I2C_TIMEOUT || fTemperature == ERROR_BAD_CRC)
	{
		rc = -1; fTemperature = -1.0;
	}

	// Detect Model of sensor HTU21D
	if (!rc) m_nSensorModel = 0;
	// Get RH and temperature from DHTxx sensor
	else
	{
		rc = 0;

		// Read Air Humidity
		fHumidity = m_pDHT->readHumidity();
		if (isnan(fHumidity))
		{
			rc = -1; fHumidity = -1.0;
		}

		// Read Air Temperature
		fTemperature = m_pDHT->readTemperature();
		if (isnan(fTemperature))
		{
			rc = -1; fTemperature = -1.0;
		}

		// Detect Model of sensor DHTxx
		if (!rc) m_nSensorModel = 1;
	}
	
	return rc;
}
