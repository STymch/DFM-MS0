// CDataMS.h

#ifndef _CDATAMS_h
#define _CDATAMS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "CommDef.h"


///////////////////////////////////////////////////////
// <<< CDataMS - class for structure of data of DFM-MS
///////////////////////////////////////////////////////
const INT		DATA_LEN = 31;				// Size of data in bytes
class CDataMS
{
protected:
	union									// Union for access to data
	{
		BYTE		m_pDataMS[DATA_LEN + 1];// Array of bytes for data
		struct								// Structure of data
		{
			BYTE	m_bLen;					// Длина пакета информации = DATA_LEN				1
			BYTE	m_bStatus;				// Биты состояни элементов измерительной системы	1
			FLOAT	m_fTemprAir;			// Температура воздуха, C							4
			FLOAT	m_fRHumidityAir;		// Compensated Humidity, %							4
			FLOAT	m_fTemprWater;			// Температура воды, C								4
			UINT	m_nPowerU;				// Уровень зарда АКБ								2
			FLOAT	m_fQ;					// Мгновенный поток имп/сек							4
			DWORD	m_dwTimeInt;			// Интервал времени, миллисекунды					4
			DWORD	m_dwCountFull;			// Общий счетчик импульсов							4
			DWORD	m_dwCountCurr;			// Счетчик импульсов для заданного пролива			4	
		};
	};
public:
	// Constructor, destructor
	CDataMS(INT nDataLen = DATA_LEN) : m_bLen(nDataLen) {}
	~CDataMS() {}

	// Get data from object
	BYTE*	GetDataMS()			{ return m_pDataMS;			}
	BYTE	GetLen()			{ return m_bLen;			}
	BYTE	GetStatus()			{ return m_bStatus;			}
	FLOAT	GetTemprAir()		{ return m_fTemprAir;		}
	FLOAT	GetRHumidityAir()	{ return m_fRHumidityAir;	}
	FLOAT	GetTemprWater()		{ return m_fTemprWater;		}
	UINT	GetPowerU()			{ return m_nPowerU;			}
	FLOAT	GetQ()				{ return m_fQ;				}
	DWORD	GetTimeInt()		{ return m_dwTimeInt;		}
	DWORD	GetCountFull()		{ return m_dwCountFull;		}
	DWORD	GetCountCurr()		{ return m_dwCountCurr;		}


	// Set data for object
	void	SetDataMS(BYTE b)			{ for (int i = 1; i <= m_bLen; m_pDataMS[i++] = b);			}
	void	SetDataMS(BYTE *pBuff)		{ for (int i = 1; i <= m_bLen; m_pDataMS[i++] = (*pBuff)++);}
	void	SetLen(BYTE bLen)			{ m_bLen = bLen;		}
	void	SetStatus(BYTE bStatus)		{ m_bStatus = bStatus;	}
	void	SetTemprAir(FLOAT fT)		{ m_fTemprAir = fT;		}
	void	SetRHumidityAir(FLOAT fRH)	{ m_fRHumidityAir = fRH;}
	void	SetTemprWater(FLOAT fT)		{ m_fTemprWater = fT;	}
	void	SetPowerU(UINT nU)			{ m_nPowerU = nU;		}
	void	SetQ(FLOAT fQ)				{ m_fQ = fQ;			}
	void	SetTimeInt(DWORD dwTime)	{ m_dwTimeInt = dwTime; }
	void	SetCountFull(DWORD dwC)		{ m_dwCountFull = dwC;	}
	void	SetCountCurr(DWORD dwC)		{ m_dwCountCurr = dwC;	}

};

///////////////////////////////////////////////////////
// <<< CCmndMS - class for structure of commands of DFM-MS
///////////////////////////////////////////////////////
const INT		CMND_LEN = 5;				// Max size of command in bytes
// Command codes
enum Cmnd	{	cmndPowerOff		= 0x50,	// DFM-MS Power OFF
				cmndSetImpInpPin	= 0x69,	// Set number of pulse input pin	
				cmndSetCount		= 0x43,	// Set current counter (DWORD arg), if 0 - be set in DWORD(-1)
				cmndReadTemprWater	= 0x54,	// Read water temperature from sensor
				cmndReadRHT			= 0x48,	// Read compensated humidity and temperature of air from sensor
			};

class CCmndMS
{
protected:
	union									// Union for access to data
	{
		BYTE		m_pCmnd[CMND_LEN + 1];	// Array of bytes for data packet with command
		struct								// Structure of data
		{
			BYTE	m_bCmndLen;				// Size of command <= CMND_LEN				1
			BYTE	m_bCmndCode;			// Command code								1
			union							// Command arguments:						1-4
			{
				BYTE	m_bCmndArg;			// BYTE		(1)
				UINT	m_nCmndArg;			// UINT		(2)
				DWORD	m_dwCmndArg;		// DWORD	(4)		
				FLOAT	m_fCmndArg;			// FLOAT	(4)
			};
		};
	};
public:
	// Constructor, destructor
	CCmndMS(INT nDataLen = CMND_LEN) : m_bCmndLen(nDataLen) {}
	~CCmndMS() {}

	// Get command code
	BYTE	GetCode()	{ return m_bCmndCode;	}

	// Get command arguments
	BYTE	GetArg_b()	{ return m_bCmndArg;	}
	UINT	GetArg_n()	{ return m_nCmndArg;	}
	DWORD	GetArg_dw()	{ return m_dwCmndArg;	}
	FLOAT	GetArg_f()	{ return m_bCmndArg;	}

	// Get pointer to array of data
	BYTE*	GetData() { return m_pCmnd; }
	void	SetData(BYTE *pBuff) { for (int i = 1; i <= m_bCmndLen; m_pCmnd[i++] = (*pBuff)++); }
};

#endif

