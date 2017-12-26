#ifndef _CRHTSENSOR_h
#define _CRHTSENSOR_h

#include <SparkFunHTU21D.h>
/* HTU21D humidity/temperature sensor
By : Nathan Seidle
SparkFun Electronics
Date : September 15th, 2013
License : This code is public domain but you buy me a beer if you use this and we meet someday(Beerware license).

Uses the HTU21D library to display the current humidity and temperature

Hardware Connections(Breakoutboard to Arduino) :
	- VCC = 3.3V
	- GND = GND
	- SDA = A4(use inline 330 ohm resistor if your board is 5V)
	- SCL = A5(use inline 330 ohm resistor if your board is 5V)
*/

#include <DHT.h>
/* DHT humidity/temperature sensors
Types: DHT11, DHT21, DHT22
Hardware Connections(Breakoutboard to Arduino) :
	- VCC (red)		= 5V
	- GND (black)	= GND
	- SIG (yellow)	= Digital PIN
*/

///////////////////////////////////////////////////////
// <<<CRHTSensor - class for RH & temperature sensor HTU21D, DHTxx 
///////////////////////////////////////////////////////
// Types of RHT Sensors
enum eRHT_SensorType{	
	snsrAUTO	= 0,	// Autodetect sensor
	snsrHTU21D	= 1,	// HTU21D
	snsrDHT11	= DHT11,// DHT 11			
	snsrDHT21	= DHT21,// DHT 21			
	snsrDHT22	= DHT22,// DHT 22			
};

class CRHTSensor
{
 protected:
	HTU21D	*m_pHTU;		// HTU21D sensor object
	
	DHT		*m_pDHT;		// DHT sensor object
	int		m_nDHT_PIN;		// DHT digital pin we're connected to
	int		m_nDHT_TYPE;	// DHT sensors type
	
	int		m_nSensorModel;	// Autodetect Model of sensor: 0 - HTU21D, 1 - DHTxx, -1 - no sensors
		
public:
	// Constructor, destructor
	CRHTSensor(int nDHT_PIN, int nDHT_TYPE) : m_nDHT_PIN(nDHT_PIN), m_nDHT_TYPE(nDHT_TYPE)
	{
		// Create HTU21D sensor object
		m_pHTU = new HTU21D();
		m_pHTU->begin();
		
		// Create DHTxx sensor object
		m_pDHT = new DHT(m_nDHT_PIN, m_nDHT_TYPE);
		m_pDHT->begin();
	}
	~CRHTSensor() { }
	
	// Get RH and temperature from all sensors
	// Return:	0 - OK, -1 - sensor error
	int	GetRHT(float& fHumidity, float& fTemperature);
		
	// Get RHT sensor model
	// Return:	0 - HTU, 1 - DHTxx,  -1 - No sensor
	int GetSensorModel() { return m_nSensorModel; }
};

#endif

