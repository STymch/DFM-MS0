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
			BYTE	m_bLen;					// Data packet length = DATA_LEN				1
			union
			{
				BYTE	m_bStatus;			// Status bits									1
				struct
				{
					int	m_btReceiveDataError	: 1;// 0 - not used
					int	m_btTempSensorError		: 1;// 1 - temperature sensor alarm
					int m_btEMFM_FQH			: 1;// 2 - flow high limit alarm
					int m_btEMFM_FQL			: 1;// 3 - flow low limit alarm
					int	m_btLowBatteryPower		: 1;// 4 - not used
					int	m_btRHTSensorError		: 1;// 5 - RHT sensor alarm
					int m_btEndBatteryRHTSensor	: 1;// 6 - end of battery RHT Sensor: VDD < 2.25V
					int m_btStartStopExt		: 1;// 7 - start of test from ext interrupt: 1, 0 - stopped
				};
			};
			FLOAT	m_fTemprAir;			// Temperature of air, C							4
			FLOAT	m_fRHumidityAir;		// Compensated Humidity, %							4
			FLOAT	m_fTemprWater;			// Temperature of water, C							4
			UINT	m_nPowerU;				// Power voltage, V									2
			volatile FLOAT	m_fQ;			// Instant flow m3/h								4
			volatile DWORD	m_dwTimeInt;	// Time's interval, millis							4
			volatile DWORD	m_dwCountFull;	// Full pulse counter, incremental					4
			volatile DWORD	m_dwCountCurr;	// Current pulse counter for tests, decremental		4	
		};
	};
public:
	// Constructor, destructor
	CDataMS(INT nDataLen = DATA_LEN) : m_bLen(nDataLen) {}
	~CDataMS() {}

	// Get data from object
	BYTE*	GetDataMS()			{ return m_pDataMS;			}
	BYTE	GetLen()			{ return m_bLen;			}
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
	void	SetTemprAir(FLOAT fT)		{ m_fTemprAir = fT;		}
	void	SetRHumidityAir(FLOAT fRH)	{ m_fRHumidityAir = fRH;}
	void	SetTemprWater(FLOAT fT)		{ m_fTemprWater = fT;	}
	void	SetPowerU(UINT nU)			{ m_nPowerU = nU;		}
	void	SetQ(FLOAT fQ)				{ m_fQ = fQ;			}
	void	SetTimeInt(DWORD dwTime)	{ m_dwTimeInt = dwTime; }
	void	SetCountFull(DWORD dwC)		{ m_dwCountFull = dwC;	}
	void	SetCountCurr(DWORD dwC)		{ m_dwCountCurr = dwC;	}

	// Set status bits
	void	SetStatus(BYTE bStatus)			{ m_bStatus = bStatus;				}
	void	SetTempSensorError(int bit)		{ m_btTempSensorError = bit;		}
	void	SetEMFM_FQH(int bit)			{ m_btEMFM_FQH = bit;				}
	void	SetEMFM_FQL(int bit)			{ m_btEMFM_FQL = bit;				}
	void	SetRHTSensorError(int bit)		{ m_btRHTSensorError = bit;			}
	void	SetEndBatteryRHTSensor(int bit) { m_btEndBatteryRHTSensor = bit;	}
	void	SetStartStopExt(int bit)		{ m_btStartStopExt = bit;			}

	// Get status bits
	BYTE	GetStatus()				{ return m_bStatus;					}
	int		GetTempSensorError()	{ return m_btTempSensorError;		}
	int		GetEMFM_FQH()			{ return m_btEMFM_FQH;				}
	int		GetEMFM_FQL()			{ return m_btEMFM_FQL;				}
	int		GetRHTSensorError()		{ return m_btRHTSensorError;		}
	int		GetEndBatteryRHTSensor(){ return m_btEndBatteryRHTSensor;	}
	int		GetStartStopExt()		{ return m_btStartStopExt;			}

};

///////////////////////////////////////////////////////
// <<< CCmndMS - class for structure of commands of DFM-MS
///////////////////////////////////////////////////////
const INT		CMND_LEN = 5;				// Max size of command in bytes
// Command codes
enum Cmnd	{	// DFM_MS control commands
				cmndPowerOff		= 0x50,	// Execte DFM-MS Power OFF
				cmndSetCount		= 0x43,	// DWORD. Set current counter, if 0 - be set in DWORD(-1)
				cmndReadRHT			= 0x48,	// Read compensated humidity and temperature of air from sensor			
				cmndReadTemprWater	= 0x54,	// Read water temperature from sensor
				
				// Set parameters value commands
				cmndSetLoopMSFreq		= 0xA0,	// DWORD. Set DFM-MS main loop frequancy, millis. Default = 200.
				cmndSetQMA_Points		= 0xA1,	// UINT. Set number of points for calculate moving average of flow Q. Default = 10.
				cmndSetInt4CalcQ		= 0xA2,	// DWORD. Interval for calculate instant flow Q, millis. Default = 500.
				cmndSetButtonPressWidth = 0xA3,	// UINT. Width in millisec of external button press. Default = 100.
				cmndSetAntiTinklingOn	= 0xA4,	// BYTE. Antitinkling flag for EMFM output pulse: 1 - ON, 0 - OFF. Default = 0.
				cmndSetPulseWidth		= 0xA5,	// UINT. Width in millisec of EMFM output pulse. Default = 50.
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

