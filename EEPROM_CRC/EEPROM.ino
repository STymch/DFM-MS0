/***
Write data into EEPROM for DFM_MS
***/

#include <EEPROM.h>

struct DFM_Data {
	char	m_strAppName[8];	// Applications name
	short	m_nVerMaj;			// Major version number
	short	m_nVerMin;			// Minor version number
	short	m_nVerStatus;		// Status number: 0 - alfa, 1 - beta, 2 - RC, 3 - RTM
	short	m_nVerBuild;		// Build number, from SVC system
	long	m_dwPulseFactor;	// Flowmeter pulse factor, pulses in 1 ltr
	long	m_lLoopMSPeriod;	// DFM-MS main loop period, millis
	long	m_lDebugPrnPeriod;	// Debug print period
	long	m_lInt4CalcQ;		// Interval for calculate instant flow Q, millis
	long	m_lCoordUpdPeriod;	// Delay between coordinate updates (GPS)
	short	m_nTypeRHTSensor;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11	
};

bool isClearEEPROM = false;		// Clear EEPROM flag (if true)

void setup() {

	Serial.begin(38600);
	while (!Serial);	// wait for serial port to connect. Needed for native USB port only

	// Data to store
	DFM_Data myData = {
						"DFM-MS ",	// Applications name
						1,			// Major version number
						0,			// Minor version number
						3,			// Status number: 0 - alfa, 1 - beta, 2 - RC, 3 - RTM
						81,			// Build number, from SVC system
						1000,		// Flowmeter pulse factor, pulses in 1 ltr				(1000)
						100,		// DFM-MS main loop period, millis						(200)
						2000,		// Debug print period									(2000)
						1000,		// Interval for calculate instant flow Q, millis		(1000)
						1000,		// Delay between coordinate updates (GPS)				(1000)
						22,			// Type of RHT DHT sensor: DHT21, DHT22, DHT11			(11, 21, 22)						
	};
	
	// Write data 
	EEPROM.put(0, myData);
	// Print written data
	Serial.println(" -=== WRITTEN data into EEPROM  ===-");
	Serial.print("\t AppName \t\t");		Serial.println(myData.m_strAppName);
	Serial.print("\t VerMaj \t\t");			Serial.println(myData.m_nVerMaj);
	Serial.print("\t VerMin \t\t");			Serial.println(myData.m_nVerMin);
	Serial.print("\t VerStatus \t\t");		Serial.println(myData.m_nVerStatus);
	Serial.print("\t VerBuild \t\t");		Serial.println(myData.m_nVerBuild);
	Serial.print("\t PulseFactor \t\t");	Serial.println(myData.m_dwPulseFactor);
	Serial.print("\t LoopMSPeriod \t\t");	Serial.println(myData.m_lLoopMSPeriod);
	Serial.print("\t DbgPrnPeriod \t\t");	Serial.println(myData.m_lDebugPrnPeriod);
	Serial.print("\t Int4CalcQ \t\t");		Serial.println(myData.m_lInt4CalcQ);
	Serial.print("\t lCoordUpdPeriod \t");	Serial.println(myData.m_lCoordUpdPeriod);
	Serial.print("\t TypeRHTSensor \t\t");	Serial.println(myData.m_nTypeRHTSensor);

	// Clear EEPROM
	if (isClearEEPROM) {
		Serial.println(" -=== CLEARING EEPROM  ===-");
		for (int i = 0; i++ < 1024; EEPROM.put(i, 0));
	}

	// Delay
	delay(2000);

	// Read  data 
	EEPROM.get(0, myData);
	// Print read data
	Serial.println(" -=== READ data from EEPROM  ===-");
	Serial.print("\t AppName \t\t");		Serial.println(myData.m_strAppName);
	Serial.print("\t VerMaj \t\t");			Serial.println(myData.m_nVerMaj);
	Serial.print("\t VerMin \t\t");			Serial.println(myData.m_nVerMin);
	Serial.print("\t VerStatus \t\t");		Serial.println(myData.m_nVerStatus);
	Serial.print("\t VerBuild \t\t");		Serial.println(myData.m_nVerBuild);
	Serial.print("\t PulseFactor \t\t");	Serial.println(myData.m_dwPulseFactor);
	Serial.print("\t LoopMSPeriod \t\t");	Serial.println(myData.m_lLoopMSPeriod);
	Serial.print("\t DbgPrnPeriod \t\t");	Serial.println(myData.m_lDebugPrnPeriod);
	Serial.print("\t Int4CalcQ \t\t");		Serial.println(myData.m_lInt4CalcQ);
	Serial.print("\t lCoordUpdPeriod \t");	Serial.println(myData.m_lCoordUpdPeriod);
	Serial.print("\t TypeRHTSensor \t\t");	Serial.println(myData.m_nTypeRHTSensor);
}

void loop() { }