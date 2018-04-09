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
		UINT	m_nVerStatus;		// Status number: 0 - alfa, 1 - beta, 2 - RC, 3 - RTM
		UINT	m_nVerBuild;		// Build number, from SVC system
		DWORD	m_dwPulseFactor;	// Flowmeter pulse factor, pulses in 1 ltr
		DWORD	m_lLoopMSPeriod;	// DFM-MS main loop period, millis
		DWORD	m_lDebugPrnPeriod;	// Debug print period
		DWORD	m_lInt4CalcQ;		// Interval for calculate instant flow Q, millis
		WORD	m_nTypeRHTSensor;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11
	} m_Data;

 public:
	 // --- CONSTANT methods
	 // Write structure into EEPROM
	 void	WriteData()			const	{ EEPROM.put(START_ADDRESS, m_Data);				}
	 // Get data from structure
	 const CHAR* getAppName()	const	{ return m_Data.m_strAppName;			}
	 UINT	getVerMaj()			const	{ return m_Data.m_nVerMaj;				}
	 UINT	getVerMin()			const	{ return m_Data.m_nVerMin;				}
	 UINT	getVerStatus()		const	{ return m_Data.m_nVerStatus;			}
	 UINT	getVerBuild()		const	{ return m_Data.m_nVerBuild;			}
	 DWORD	getPulseFactor()	const	{ return m_Data.m_dwPulseFactor;		}
	 DWORD	getLoopMSPeriod()	const	{ return m_Data.m_lLoopMSPeriod;		}
	 DWORD	getDebugPrnPeriod()	const	{ return m_Data.m_lDebugPrnPeriod;		}	
	 DWORD	getInt4CalcQ()		const	{ return m_Data.m_lInt4CalcQ;			}
	 WORD	getTypeRHTSensor()	const	{ return m_Data.m_nTypeRHTSensor;		}
	 
	 // --- MODIFYING methods
	 // Write structure into EEPROM
	 void	ReadData()						{ EEPROM.get(START_ADDRESS, m_Data);				}
	 // Put data into structure
	 void	putAppName(const CHAR* str)		{ strcpy(m_Data.m_strAppName, str);		}
	 void	putVerMaj(UINT maj)				{ m_Data.m_nVerMaj = maj;				}
	 void	putVerMin(UINT min)				{ m_Data.m_nVerMin = min;				}
	 void	putVerStatus(UINT status)		{ m_Data.m_nVerStatus = status;			}
	 void	putVerBuild(UINT build)			{ m_Data.m_nVerBuild = build;			}
	 void	putPulseFactor(DWORD pf)		{ m_Data.m_dwPulseFactor = pf;			}
	 void	putLoopMSPeriod(DWORD loop)		{ m_Data.m_lLoopMSPeriod = loop;		}
	 void	putDebugPrnPeriod(DWORD period) { m_Data.m_lDebugPrnPeriod = period;	}
	 void	putInt4CalcQ(DWORD interval)	{ m_Data.m_lInt4CalcQ = interval;		}
	 void	putTypeRHTSensor(WORD type)		{ m_Data.m_nTypeRHTSensor = type;		}

	 // --- FRIEND function
	 // Read data from EEPROM and copy into global variables
	 friend void EEPROM_ReadData(CDataEEPROM&);
};

#endif

