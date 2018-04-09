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
	short	m_nTypeRHTSensor;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11
};

void setup() {

	Serial.begin(38600);
	while (!Serial);	// wait for serial port to connect. Needed for native USB port only

	// Data to store
	DFM_Data myData = {
						"DFM-MS ",
						1,
						0,
						2,
						78,
						1000,
						200,
						2000,
						1000,
						11
	};
	
	// Write data 
	EEPROM.put(0, myData);
	// Print written data
	Serial.println(" -=== WRITTEN data into EEPROM  ===-");
	Serial.print("\t AppName \t");			Serial.println(myData.m_strAppName);
	Serial.print("\t VerMaj \t");			Serial.println(myData.m_nVerMaj);
	Serial.print("\t VerMin \t");			Serial.println(myData.m_nVerMin);
	Serial.print("\t VerStatus \t");		Serial.println(myData.m_nVerStatus);
	Serial.print("\t VerBuild \t");			Serial.println(myData.m_nVerBuild);
	Serial.print("\t PulseFactor \t");		Serial.println(myData.m_dwPulseFactor);
	Serial.print("\t LoopMSPeriod \t");		Serial.println(myData.m_lLoopMSPeriod);
	Serial.print("\t DbgPrnPeriod \t");		Serial.println(myData.m_lDebugPrnPeriod);
	Serial.print("\t Int4CalcQ \t");		Serial.println(myData.m_lInt4CalcQ);
	Serial.print("\t TypeRHTSensor \t");	Serial.println(myData.m_nTypeRHTSensor);

	// Delay
	delay(2000);

	// Read  data 
	EEPROM.get(0, myData);
	// Print read data
	Serial.println(" -=== READ data from EEPROM  ===-");
	Serial.print("\t AppName \t");			Serial.println(myData.m_strAppName);
	Serial.print("\t VerMaj \t");			Serial.println(myData.m_nVerMaj);
	Serial.print("\t VerMin \t");			Serial.println(myData.m_nVerMin);
	Serial.print("\t VerStatus \t");		Serial.println(myData.m_nVerStatus);
	Serial.print("\t VerBuild \t");			Serial.println(myData.m_nVerBuild);
	Serial.print("\t PulseFactor \t");		Serial.println(myData.m_dwPulseFactor);
	Serial.print("\t LoopMSPeriod \t");		Serial.println(myData.m_lLoopMSPeriod);
	Serial.print("\t DbgPrnPeriod \t");		Serial.println(myData.m_lDebugPrnPeriod);
	Serial.print("\t Int4CalcQ \t");		Serial.println(myData.m_lInt4CalcQ);
	Serial.print("\t TypeRHTSensor \t");	Serial.println(myData.m_nTypeRHTSensor);
}

void loop() { }