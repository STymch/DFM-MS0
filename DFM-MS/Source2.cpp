// CEMFM.h

#ifndef _CEMFM_h
#define _CEMFM_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "CommDef.h"

const int   MA_NVAL = 6;		// Number of point for calculate moving average of Q

								///////////////////////////////////////////////////////
								// <<< CEMFM - class for EMFM QTLD-15 / Generator
								///////////////////////////////////////////////////////
class CEMFM
{
protected:
	// GPIO pins
	int		m_nINP_PULSE_PIN;			// Input pin for connect EMFM/Generator out pulse
	int		m_nALM_FQH_PIN;				// Input pin for EMFM ALARM FQH signal
	int		m_nALM_FQL_PIN;				// Input pin for EMFM ALARM FQL signal
										// Signals
	int		m_nTypePulseFront;			// Type of EMFM/Generator Output pulse front: LOW or HIGH  
	int		m_nTypeALM_FQHSignal;		// Type of EMFM ALARM FQH signal: LOW or HIGH  
	int		m_nTypeALM_FQLSignal;		// Type of EMFM ALARM FQL signal: LOW or HIGH
										// Width of signals
	int		m_nInpPulseWidth;			// Width in millisec of EMFM output pulse
	int		m_nALM_FQHWidth;			// Width in millisec of EMFM ALARM FQH signal
	int		m_nALM_FQLWidth;			// Width in millisec of EMFM ALARM FQL signal
										// ALARM signal flags
	volatile bool	m_bALM_FQH;			// ALARM FQH signal present - true, no - false
	volatile bool	m_bALM_FQL;			// ALARM FQL signal present - true, no - false
										// Pulse signal externel interrupt parameters
	int		m_nEXT_INT_MODE;			// Mode of external interrupt: LOW, CHANGE, RISING, FALLING
										// EMFM pulse counters & flow
	volatile DWORD	m_dwCountFull;		// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	volatile DWORD	m_dwCountCurr;		// Current counter for input pulses from EMFM/Generator
	volatile DWORD	m_dwCountStartQ;	// Begin value of pulse couner for calculate current flow Q
	volatile float	m_fQm3h = 0;		// Current flow m3/h
	volatile float	m_fQMAm3h = 0;		// Moving average of flow m3/h
	float			m_pfQ[MA_NVAL];		// Array for save current Q for calculate moving average of Q
	volatile DWORD	m_lTStartPulseFront;// Time in ms of starting front of pulse
	volatile DWORD	m_lTStartQ;			// Time in ms of starting calculation current flow Q
	DWORD	m_lTInterval4Q;				// Interval (ms) for calculate current flow
	int		m_nPulseOnLtr;				// Number of pulse from EMFM for 1 ltr water
	bool	m_isQCalculate = TRUE;		// Flag: 1 - current Q is calculated, 0 - current Q not calculated.
										// Timer for calculate time

	volatile bool	m_isTimerStart = FALSE;		// Flag: 1 - timer is start, 0 - timer is stop.
	volatile DWORD	m_lTStartTimer;				// Time in ms of starting timer
	volatile DWORD	m_lTTimerInterval = 0;		// Interval (ms) for timer

public:
	// Constructor, destructor
	CEMFM(INT nPulse_pin, INT nALM_FQH_pin, INT nALM_FQL_pin, INT nTypePulseFront, INT nTypeALM_FQH, INT nTypeALM_FQL,
		INT nPulseWidth, INT nALM_FQHWidth, INT nALM_FQLWidth)
	{
		// PIN
		m_nINP_PULSE_PIN = nPulse_pin;
		m_nALM_FQH_PIN = nALM_FQH_pin;
		m_nALM_FQL_PIN = nALM_FQL_pin;

		// Types of signals
		m_nTypePulseFront = nTypePulseFront;
		m_nTypeALM_FQHSignal = nTypeALM_FQH;
		m_nTypeALM_FQLSignal = nTypeALM_FQL;

		// Width of pulse
		m_nInpPulseWidth = nPulseWidth;
		m_nALM_FQHWidth = nALM_FQHWidth;
		m_nALM_FQLWidth = nALM_FQLWidth;

		for (int i = 0; i < MA_NVAL; m_pfQ[i++] = 0.0);

	}
	~CEMFM() { }

	// Set counters, define pin modes, set external interrupt ISR	
	void Init(
		DWORD	dwCountFull,	// counter of all input pulses from EMFM/Generator from turn on FMVI-MS
		DWORD	dwCountCurr,	// current counter for input pulses from EMFM/Generator
		DWORD	lTInterval4Q,	// interval in millisec for calculate current Q
		int		nPulseOnLtr,	// number of pulse from EMFM for 1 ltr water
		int		nINT_MODE,		// mode of external interrupt: LOW, CHANGE, RISING, FALLING
		void(*ISR)()		// external interrupt ISR
	);

	bool	isPulseFront() { return (digitalRead(m_nINP_PULSE_PIN) == m_nTypePulseFront) ? TRUE : FALSE; }
	bool	isPulse(DWORD lTCurr) { return (lTCurr >= m_lTStartPulseFront + m_nInpPulseWidth) ? TRUE : FALSE; }
	void	SetTStartPulse(DWORD lTStart) { m_lTStartPulseFront = lTStart; }
	void	SetCountFull(DWORD dwCountF) { m_dwCountFull = dwCountF; }
	DWORD	GetCountFull() { return m_dwCountFull; }
	void	SetCountCurr(DWORD dwCountC) { m_dwCountCurr = dwCountC; }
	DWORD	GetCountCurr() { return m_dwCountCurr; }

	float	CalculateQ();
	float	CalculateQ(DWORD lTimeInt);
	float	CalculateQMA(float fQcurr);

	float	GetQCurr() { return m_fQm3h; }
	float	GetQMA() { return m_fQMAm3h; }
	void	SetQCurr(float fQ) { m_fQm3h = fQ; }

	void	StartTimer() { m_lTStartTimer = millis(); m_lTTimerInterval = 0; m_isTimerStart = true; }
	void	StopTimer() { m_isTimerStart = false; m_lTTimerInterval = millis() - m_lTStartTimer; }
	DWORD	GetTimer() {
		if (m_isTimerStart == true) m_lTTimerInterval = millis() - m_lTStartTimer;
		return 	m_lTTimerInterval;
	}
};



#endif



// 
// 
// 

#include "CEMFM.h"

// Define pin modes, set external interrupt ISR	
void CEMFM::Init(
	DWORD	dwCountF,		// counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	DWORD	dwCountC,		// current counter for input pulses from EMFM/Generator
	DWORD	lTInterval4Q,	// interval in millisec for calculate current Q
	int		nPulseOnLtr,	// Number of pulse from EMFM for 1 ltr water	
	int		nINT_MODE,		// mode of external interrupt: LOW, CHANGE, RISING, FALLING
	void(*ISR)()		// external interrupt ISR
)
{
	// Set counters and interval for calculate flow
	m_dwCountFull = dwCountF;	// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	m_dwCountCurr = dwCountC;	// Current counter for input pulses from EMFM/Generator
	m_lTInterval4Q = lTInterval4Q;	// Interval (ms) for calculate current flow

	m_nPulseOnLtr = nPulseOnLtr;	// Number of pulse from EMFM for 1 ltr water

									// Define pin modes
	pinMode(m_nINP_PULSE_PIN, INPUT);
	pinMode(m_nALM_FQH_PIN, INPUT);
	pinMode(m_nALM_FQL_PIN, INPUT);

	// Set external interrupt ISR
	m_nEXT_INT_MODE = nINT_MODE;
	attachInterrupt(digitalPinToInterrupt(m_nINP_PULSE_PIN), ISR, m_nEXT_INT_MODE);
}

// Calculate current and moving average of flow Q
FLOAT	CEMFM::CalculateQ()
{
	DWORD	lTInterval;

	// Starting calculation Q process, at start m_isQCalculate = 1
	if (m_isQCalculate) {
		m_lTStartQ = millis();				// save time of begin interval of calculation Q
		m_dwCountStartQ = m_dwCountFull;	// save initial value of pulse couner
		m_isQCalculate = !m_isQCalculate;	// reset flag
	}
	// Calculation Q loop
	else {
		// Calculate time interval
		lTInterval = millis() - m_lTStartQ;
		// Check full time interval for calculation Q 
		if (lTInterval >= m_lTInterval4Q)		// time expiried,  calculate Q
		{
			m_fQm3h = 3600.0F * (m_dwCountFull - m_dwCountStartQ) / (m_nPulseOnLtr * lTInterval);
			m_lTStartQ = millis();				// save time of begin interval for new calculation of Q
			m_dwCountStartQ = m_dwCountFull;	// save value of pulse couner

			CalculateQMA(m_fQm3h);				// calculate  moving average of flow Q
		}
	}
	return m_fQm3h;
}

// Calculate current and moving average of flow Q
FLOAT	CEMFM::CalculateQ(DWORD lTimeInt)
{
	// Starting calculation Q process, at start m_isQCalculate = 1
	if (m_isQCalculate) {
		m_dwCountStartQ = m_dwCountFull;	// save initial value of pulse couner
		m_isQCalculate = !m_isQCalculate;	// reset flag
	}
	// Calculation Q loop
	else {
		m_fQm3h = 3600.0F * (m_dwCountFull - m_dwCountStartQ) / (m_nPulseOnLtr * lTimeInt);
		m_dwCountStartQ = m_dwCountFull;	// save value of pulse couner
		CalculateQMA(m_fQm3h);				// calculate  moving average of flow Q
	}
	return m_fQm3h;
}

// Calculate moving average of flow Q
FLOAT	CEMFM::CalculateQMA(float fQcurr)
{
	m_fQMAm3h = 0;
	for (int i = 0; i < MA_NVAL - 1; ++i) {
		m_pfQ[i] = m_pfQ[i + 1];
		m_fQMAm3h += m_pfQ[i] / MA_NVAL;
	}
	m_pfQ[MA_NVAL - 1] = fQcurr;
	m_fQMAm3h += fQcurr / MA_NVAL;

	return m_fQMAm3h;
}



const INT NUM_OF_SERIES;	// Number of point for calculate average
class CAverageOfSeries
{
protected:
	FLOAT m_pfPoints[NUM_OF_SERIES];	// Array for save current points for calculate average
	FLOAT m_fAverageValue;				// Current Average Value of Series

public:
	CAverage()	{ for (INT i = 0; i < NUM_OF_SERIES; m_pfPoints[++i] = 0.0) }
	~CAverage() {}

	// Calculate moving average
	FLOAT CAverage::MA(float fCurrVal)
	{
		// Calculate moving average
		m_fAverageValue += (fCurrVal - m_pfPoints[0]) / NUM_OF_SERIES;
		
		// Move points in array on 1 position left
		for (INT i = 0; i < NUM_OF_SERIES - 1; ++i)
			m_pfPoints[i] = m_pfPoints[i + 1];
		
		// Save current value in array
		m_pfPoints[NUM_OF_SERIES - 1] = fCurrVal;
		
		return m_fAverage;
	}
};