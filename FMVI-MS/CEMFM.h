// CEMFM.h

#ifndef _CEMFM_h
#define _CEMFM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "CommDef.h"

///////////////////////////////////////////////////////
// <<< CEMFM - class for EMFM QTLD-15 / Generator
///////////////////////////////////////////////////////
class CEMFM
{
protected:
	// GPIO pins
	INT		m_nINP_PULSE_PIN;		// Input pin for connect EMFM/Generator out pulse
	INT		m_nALM_FQH_PIN;			// Input pin for EMFM ALARM FQH signal
	INT		m_nALM_FQL_PIN;			// Input pin for EMFM ALARM FQL signal
	// Signals
	INT		m_nTypePulseFront;		// Type of EMFM/Generator Output pulse front: LOW or HIGH  
	INT		m_nTypeALM_FQHSignal;	// Type of EMFM ALARM FQH signal: LOW or HIGH  
	INT		m_nTypeALM_FQLSignal;	// Type of EMFM ALARM FQL signal: LOW or HIGH
	// Width of signals
	UINT	m_nInpPulseWidth;		// Width in millisec of EMFM output pulse
	UINT	m_nALM_FQHWidth;		// Width in millisec of EMFM ALARM FQH signal
	UINT	m_nALM_FQLWidth;		// Width in millisec of EMFM ALARM FQL signal
	// ALARM signal flags
	bool	m_bALM_FQH;				// ALARM FQH signal present - true, no - false
	bool	m_bALM_FQL;				// ALARM FQL signal present - true, no - false
	// Pulse signal externel interrupt parameters
	INT		m_nEXT_INT_NUM;			// Number of external interrupt: 0 - digital pin 2, 1 - digital pin 3
	INT		m_nEXT_INT_MODE;		// Mode of external interrupt: LOW, CHANGE, RISING, FALLING
	// EMFM pulse counters & flow
	DWORD	m_dwCountFullPulse;		// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	DWORD	m_dwCountCurrPulse;		// Current counter for input pulses from EMFM/Generator
	DWORD	m_dwCountStartQ;		// Begin value of pulse couner for calculate current flow Q
	UINT	m_nPulseOnLtr;			// Number of pulse from EMFM for 1 ltr water
	FLOAT	m_fQm3h = 0;			// Current flow m3/h
	UINT	m_nTInterval4Q;			// Interval (ms) for calculate current flow
	DWORD	m_lTStartPulseFront;	// Time in ms of starting front of pulse
	DWORD	m_lTStartQ;				// Time in ms of starting calculation current flow Q
	bool	m_isQCalculate = TRUE;	// Flag: 1 - current Q is calculated, 0 - current Q not calculated.
	
public:
	// Static data
	volatile static DWORD	dwCountFullPulse;	// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	volatile static DWORD	dwCountCurrPulse;	// Current counter for input pulses from EMFM/Generator
	volatile static FLOAT	fQm3h;				// Current flow m3/h
	
	// Constructor, destructor
	CEMFM(	INT nPulse_pin, INT nALM_FQH_pin, INT nALM_FQL_pin, INT nTypePulseFront, INT nTypeALM_FQH, INT nTypeALM_FQL,
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
		
	}
	~CEMFM() { }
		
	// Set counters, define pin modes, set external interrupt ISR	
	void Init(	
			DWORD	dwCountF,		// counter of all input pulses from EMFM/Generator from turn on FMVI-MS
			DWORD	dwCountC,		// current counter for input pulses from EMFM/Generator
			UINT	nTInterval4Q,	// interval in millisec for calculate current Q
			UINT	nPulseOnLtr,	// Number of pulse from EMFM for 1 ltr water
			INT		nINT_NUM,		// number of external interrupt: 0 - digital pin 2, 1 - digital pin 3 
			INT		nINT_MODE,		// mode of external interrupt: LOW, CHANGE, RISING, FALLING
			void	(*ISR)()		// external interrupt ISR
	);

	
	
//	INT		GetPulsePin()		{ return m_nINP_PULSE_PIN;  }
//	INT		ReadPulseState()	{ return digitalRead(m_nINP_PULSE_PIN); }

	bool	isPulseFront()					{ return (digitalRead(m_nINP_PULSE_PIN) == m_nTypePulseFront) ? TRUE : FALSE; }
	bool	isPulse(DWORD lTCurr)			{ return (lTCurr >= m_lTStartPulseFront + m_nInpPulseWidth) ? TRUE : FALSE; }
	void	SetTStartPulse(DWORD lTStart)	{ m_lTStartPulseFront = lTStart; }
	void	SetCountFull(DWORD dwCountF)	{ m_dwCountFullPulse = dwCountF; }
	DWORD	GetCountFull()					{ return m_dwCountFullPulse; }
	void	SetCountCurr(DWORD dwCountC)	{ m_dwCountCurrPulse = dwCountC; }
	DWORD	GetCountCurr()					{ return m_dwCountCurrPulse; }

	FLOAT	CalculateQ();
	FLOAT	GetQCurr()						{ return m_fQm3h; }

//	INT		GetTypePulseFront() { return m_nTypePulseFront; }
//	UINT	GetInpPulseWidth()	{ return m_nInpPulseWidth;	}
};



#endif

