// CDataEEPROM.h

#ifndef _CDATAEEPROM_h
#define _CDATAEEPROM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "CommDef.h"
#include <EEPROM.h>

class CDataEEPROM
{
protected:
	// Starting address for data location in EEPROM
	const INT START_ADDRESS = 0;

	// Data structure in EEPROM
	struct{
		CHAR	m_strAppName[8];	// Applications name
		UINT	m_nVerMaj;			// Major version number
		UINT	m_nVerMin;			// Minor version number
		UINT	m_nVerStatus;		// Status number: 0 - alpha, 1 - beta, 2 - RC, 3 - RTM
		UINT	m_nVerBuild;		// Build number, from SVC system
		DWORD	m_dwPulseFactor;	// Flow meter pulse factor, pulses in 1 ltr
		INT		m_nVolumeFactor;	// Flow meter volume factor, milliltrs in 1 pulse
		DWORD	m_lLoopMSPeriod;	// DFM-MS main loop period, millis
		DWORD	m_lDebugPrnPeriod;	// Debug print period
		DWORD	m_lInt4CalcQ;		// Interval for calculate instant flow Q, millis
		DWORD	m_lCoordUpdPeriod;	// Delay between coordinate updates (GPS)
		WORD	m_nTypeRHTSensor;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11 (11, 21, 22)
		WORD	m_nTypeDevOnPIN2;	// Type of device on PIN2: 00 - LED, 01,11 - Button (Pulse, LOW, HIGH), 02,12 - Button (State, LOW, HIGH)
	} m_Data;

 public:
	 // --- CONSTANT methods
	 // Write structure into EEPROM
	 void	WriteData()	const	{ EEPROM.put(START_ADDRESS, m_Data);	}
	 // Get data from structure
	 
	 // --- MODIFYING methods
	 // Read structure into EEPROM
	 void	ReadData()			{ EEPROM.get(START_ADDRESS, m_Data);	}
	 // Put data into structure

	 // --- FRIEND function
	 // First initialization of data
	 friend void EEPROM_InitData(CDataEEPROM&);
	 // Read data from EEPROM and copy into global variables
	 friend INT EEPROM_ReadData(CDataEEPROM&);
	 // Print data from EEPROM into Serial console, Return: 0 - data read from EEPROM, -1 - EEPROM empty
	 friend void EEPROM_PrintData(CDataEEPROM&);
};
#endif

