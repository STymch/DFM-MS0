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
// <<< CDataMS - class for structure of data of FMVI-MS
///////////////////////////////////////////////////////
const INT		DATA_LEN = 19;				// Size of data in bytes
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
			FLOAT	m_fTempr;				// Температура воды									4
			UINT	m_nPowerU;				// Уровень зарда АКБ								2
			FLOAT	m_fQ;					// Мгновенный поток имп/сек							4
			DWORD	m_dwCountFull;			// Общий счетчик импульсов							4
			DWORD	m_dwCountCurr;			// Счетчик импульсов для заданного пролива			4	
		};
	};
public:
	// Constructor, destructor
	CDataMS(INT nDataLen = DATA_LEN) : m_bLen(nDataLen) {}
	~CDataMS() {}

	// Get pointer to data
	BYTE*	GetDataMS()		{ return m_pDataMS;		}
	BYTE	GetLen()		{ return m_bLen;		}
	BYTE	GetStatus()		{ return m_bStatus;		}
	FLOAT	GetTempr()		{ return m_fTempr;		}
	UINT	GetPowerU()		{ return m_nPowerU;		}
	FLOAT	GetQ()			{ return m_fQ;			}
	DWORD	GetCountFull()	{ return m_dwCountFull; }
	DWORD	GetCountCurr()	{ return m_dwCountCurr; }


	// Set data from buffer of bytes
	void	SetDataMS(BYTE b)		{ for (int i = 1; i <= m_bLen; m_pDataMS[i++] = b); }
	void	SetDataMS(BYTE *pBuff)	{ for (int i = 1; i <= m_bLen; m_pDataMS[i++] = (*pBuff)++); }
	void	SetLen(BYTE bLen)		{ m_bLen = bLen;		}
	void	SetStatus(BYTE bStatus) { m_bStatus = bStatus;	}
	void	SetTempr(FLOAT fT)		{ m_fTempr = fT;		}
	void	SetPowerU(UINT nU)		{ m_nPowerU = nU;		}
	void	SetQ(FLOAT fQ)			{ m_fQ = fQ;			}
	void	SetCountFull(DWORD dwC)	{ m_dwCountFull = dwC;	}
	void	SetCountCurr(DWORD dwC)	{ m_dwCountCurr = dwC;	}

};

///////////////////////////////////////////////////////
// <<< CCmndMS - class for structure of commands of FMVI-MS
///////////////////////////////////////////////////////
const INT		CMND_LEN = 5;				// Max size of command in bytes
// Command codes
enum Cmnd { cmndPowerOff = 0x50, cmndSetImpInpPin = 0x69, cmndSetCount = 0x43 };

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

