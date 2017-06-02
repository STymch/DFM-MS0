#include "CTemperatureSensor.h"

// Detect temperature sensor
// Return:	0 - sensor OK, 
//			-1 - no sensor,
//			1 - no more addresses, 
//			2 - CRC is not valid.
int CTemperatureSensor::Detect()
{
	int		retcode = -1;
	
	// Detect sensor
	if (m_pDS->search(m_pbAddr))	// read adresses
		if (OneWire::crc8(m_pbAddr, 7) == m_pbAddr[7]) 	// check CRC
		{
			// The first ROM byte indicates which chip
			switch (m_pbAddr[0]) {
			case 0x10:	// Chip = DS18S20 or old DS1820
				m_nTypeSensor = 1;
				strcpy(m_strSensorName,"DS18S20");
				break;
			case 0x28:	// Chip = DS18B20
				m_nTypeSensor = 2;
				strcpy(m_strSensorName, "DS18B20");
				break;
			case 0x22:	// Chip = DS1822
				m_nTypeSensor = 3;
				strcpy(m_strSensorName, "DS1822");
				break;
			default:	// Device is not a DS18x20 family
				m_nTypeSensor = -1;
				strcpy(m_strSensorName, "\0");
				break;
			}
			retcode = 0;
		}
		else	// CRC is not valid
			retcode = 2;
	else		// No more addresses
		retcode = 1;

	return retcode;
}

// Get temperature from sensor
// Return:	0 - sensor OK, 
//			1 - sensor error
int	CTemperatureSensor::GetTemperature(float& fTemperature)
{
	int		rc = 1;
	byte	data[12];
	fTemperature = -1.0;
	
	// Detect temperature sensor
	if (!Detect()) {
		m_pDS->reset();
		m_pDS->select(m_pbAddr);
		m_pDS->write(0x44, 1);

		delay(m_lTDelayTempSensor);

		m_pDS->reset();
		m_pDS->select(m_pbAddr);
		m_pDS->write(0xBE);         // read Scratchpad

		// Read data	
		for (int i = 0; i < 9; i++) data[i] = m_pDS->read();	// we need 9 bytes


		// Convert the data to actual temperature
		// because the result is a 16 bit signed integer, it should
		// be stored to an "int16_t" type, which is always 16 bits
		// even when compiled on a 32 bit processor.
		int16_t raw = (data[1] << 8) | data[0];
		if (m_nTypeSensor == 1) {
			raw = raw << 3; // 9 bit resolution default
			if (data[7] == 0x10) {
				// "count remain" gives full 12 bit resolution
				raw = (raw & 0xFFF0) + 12 - data[6];
			}
		}
		else {
			byte cfg = (data[4] & 0x60);
			// at lower res, the low bits are undefined, so let's zero them
			if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
			else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
			else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
												  //// default is 12 bit resolution, 750 ms conversion time
		}

		fTemperature = (float)raw / 16.0;
		rc = 0;
	}
	return rc;
}

