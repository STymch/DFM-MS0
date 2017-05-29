// 
// 
// 

#include "CRHTSensor.h"

// Detect RHT sensor
// Return:	0 - sensor OK, 
//			-1 - no sensor,
//			1 - Battery LOW, Level < 2.25v
int CRHTSensor::Detect()
{
	int		retcode = -1;

#if defined(ARDUINO_ARCH_ESP8266) || (ESP8266_NODEMCU)
	if (m_pHTU21D->begin(D1, D2) == true)
#else
	if (m_pHTU21D->begin() == true)
#endif
		retcode = 0;
	
	// Check Battery Status
	if (m_pHTU21D->batteryStatus() != true)
		retcode = 1;	// Battery LOW, Level < 2.25v

	// Fimware version
	m_nFirmwareVersion = m_pHTU21D->readFirmwareVersion();
	// Sensor's name
	m_nSensorID = m_pHTU21D->readDeviceID();

	return retcode;
}

// Get RH and temperature from sensor
void CRHTSensor::GetRHT(float &fHumidity, float &fTemperature)
{
	// Set resolution
	m_pHTU21D->setResolution(HTU21D_RES_RH11_TEMP11);
	
	// Read Compensated Humidity
	fHumidity = m_pHTU21D->readCompensatedHumidity();
	
	// Read Temperature
	fTemperature= m_pHTU21D->readTemperature();
}


