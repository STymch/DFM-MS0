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
const INT		DATA_LEN = 39;				// Size of data in bytes

class CDataMS
{
protected:
	union									// Union for access to data
	{
		BYTE		m_pDataMS[DATA_LEN + 1];// Array of bytes for data
		struct								// Structure of data
		{
			BYTE	m_bLen;					// Data packet length = DATA_LEN				1
			union
			{
				BYTE	m_bStatus;			// Status bits									1
				struct
				{
					INT	m_btReceiveError		: 1;// 0 - receive data error
					INT	m_btRHTSensorError		: 1;// 1 - RHT sensor alarm
					INT	m_btTempSensorError		: 1;// 2 - temperature sensor alarm
					INT m_btEMFM_FQH			: 1;// 3 - flow high limit alarm
					INT m_btEMFM_FQL			: 1;// 4 - flow low limit alarm
					INT m_btGPSError			: 1;// 5 - GPS module alarm
					INT	m_btNU1					: 1;// 6 - not used
					INT m_btTestRun				: 1;// 7 - test run bit: 1 - test is run, 0 - test not run
				};
			};
			FLOAT	m_fTemprAir;	// Temperature of air, C							4
			FLOAT	m_fRHumidityAir;// Compensated Humidity, %							4
			FLOAT	m_fTemprWater;	// Temperature of water, C							4
			UINT	m_nPowerU;		// Power voltage, V									2
			FLOAT	m_fLAT;			// GPS Latitude										4
			FLOAT	m_fLON;			// GPS Longitude									4
			FLOAT	m_fQ;			// Instant flow m3/h								4
			DWORD	m_dwTime;		// Time, millis										4
			DWORD	m_dwCounterInc;	// Increment pulse counter							4
			DWORD	m_dwCounterDec;	// Decrement pulse counter for tests				4	
		};
	};
public:
	// Constructor, destructor
	CDataMS(INT nDataLen = DATA_LEN) : m_bLen(nDataLen) {}
	~CDataMS() {}

	// --- CONSTANT methods
	// Get data from object
	BYTE	GetLen()			const	{ return m_bLen;		 }
	FLOAT	GetTemprAir()		const	{ return m_fTemprAir;	 }
	FLOAT	GetRHumidityAir()	const	{ return m_fRHumidityAir;}
	FLOAT	GetTemprWater()		const	{ return m_fTemprWater;	 }
	UINT	GetPowerU()			const	{ return m_nPowerU;		 }
	FLOAT	GetGPS_LAT()		const	{ return m_fLAT;		 }
	FLOAT	GetGPS_LON()		const	{ return m_fLON;		 }
	FLOAT	GetQ()				const	{ return m_fQ;			 }
	DWORD	GetTime()			const	{ return m_dwTime;		 }
	DWORD	GetCounterInc()		const	{ return m_dwCounterInc; }
	DWORD	GetCounterDec()		const	{ return m_dwCounterDec; }

	// Get status bits
	BYTE	Get_bStatus()			const	{ return m_bStatus;				}
	INT		Get_btReceiveError()	const	{ return m_btReceiveError;		}
	INT		Get_btRHTSensorError()	const	{ return m_btRHTSensorError;	}
	INT		Get_btTempSensorError()	const	{ return m_btTempSensorError;	}
	INT		Get_btEMFM_FQH()		const	{ return m_btEMFM_FQH;			}
	INT		Get_btEMFM_FQL()		const	{ return m_btEMFM_FQL;			}
	INT		Get_btGPSError()		const	{ return m_btGPSError;			}
	INT		Get_btTestRun()			const	{ return m_btTestRun;			}

	// --- MODIFYING methods
	BYTE*	GetDataMS()					{ return m_pDataMS; }
	// Set data for object
	void	SetDataMS(BYTE b)			{ for (INT i = 1; i <= m_bLen; m_pDataMS[i++] = b);			}
	void	SetDataMS(BYTE *pBuff)		{ for (INT i = 1; i <= m_bLen; m_pDataMS[i++] = (*pBuff)++);}
	void	SetLen(BYTE bLen)			{ m_bLen = bLen;				}
	void	SetTemprAir(FLOAT fT)		{ m_fTemprAir = fT;				}
	void	SetRHumidityAir(FLOAT fRH)	{ m_fRHumidityAir = fRH;		}
	void	SetTemprWater(FLOAT fT)		{ m_fTemprWater = fT;			}
	void	SetPowerU(UINT nU)			{ m_nPowerU = nU;				}
	void	SetGPS_LAT(FLOAT fLAT)		{ m_fLAT = fLAT;				}
	void	SetGPS_LON(FLOAT fLON)		{ m_fLON = fLON;				}
	void	SetQ(FLOAT fQ)				{ m_fQ = fQ;					}
	void	SetTime(DWORD dwTime)		{ m_dwTime = dwTime;			}
	void	SetCounterInc(DWORD dwC)	{ m_dwCounterInc = dwC;			}
	void	SetCounterDec(DWORD dwC)	{ m_dwCounterDec = dwC;			}

	// Set status bits
	void	Set_bStatus(BYTE bStatus)		{ m_bStatus = bStatus;			}
	void	Set_btReceiveError(INT bit)		{ m_btReceiveError = bit;		}
	void	Set_btRHTSensorError(INT bit)	{ m_btRHTSensorError = bit;		}
	void	Set_btTempSensorError(INT bit)	{ m_btTempSensorError = bit;	}
	void	Set_btEMFM_FQH(INT bit)			{ m_btEMFM_FQH = bit;			}
	void	Set_btEMFM_FQL(INT bit)			{ m_btEMFM_FQL = bit;			}
	void	Set_btGPSError(INT bit)			{ m_btGPSError = bit;			}
	void	Set_btTestRun(INT bit)			{ m_btTestRun = bit;			}
};

///////////////////////////////////////////////////////
// <<< CCmndMS - class for structure of commands of DFM-MS
///////////////////////////////////////////////////////
const INT		CMND_LEN = 5;				// Max size of command in bytes
// Command codes
enum Cmnd	{	// DFM_MS control commands
				cmndSetCounter		= 0x43,	// DWORD. Set current counter, if 0 - be set DWORD(-1)
				cmndReadRHT			= 0x48,	// Read humidity and temperature of air from sensor	
				cmndGetLocation		= 0x4C,	// Get location from GPS	
				cmndPowerOff		= 0x50,	// Execute DFM-MS Power OFF
				cmndReqDataMS		= 0x52,	// Request data packet from MS
				cmndReadTemprWater	= 0x54,	// Read water temperature from sensor
				
				// Set parameters value commands
				cmndSetLoopMSPeriod		= 0xA0,	// DWORD. Set DFM-MS main loop frequency, millis. Default = 200.
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

	// --- CONSTANT methods
	// Get command code
	BYTE	GetCode() const		{ return m_bCmndCode;	}

	// Get command arguments
	BYTE	GetArg_b() const	{ return m_bCmndArg;	}
	UINT	GetArg_n() const	{ return m_nCmndArg;	}
	DWORD	GetArg_dw()const	{ return m_dwCmndArg;	}
	FLOAT	GetArg_f() const	{ return m_bCmndArg;	}

	// --- MODIFYING methods
	// Get pointer to array of data
	BYTE*	GetData() { return m_pCmnd; }
	// Write data from array
	void	SetData(BYTE *pBuff) { for (INT i = 1; i <= m_bCmndLen; m_pCmnd[i++] = (*pBuff)++); }
};

#endif

