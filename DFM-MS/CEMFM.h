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
	// Width of alarm signals
	INT		m_nALM_FQHWidth;		// Width in millisecond of EMFM ALARM FQH signal
	INT		m_nALM_FQLWidth;		// Width in millisecond of EMFM ALARM FQL signal
	// ALARM signal flags
	BOOL	m_bALM_FQH;				// ALARM FQH signal present - true, no - false
	BOOL	m_bALM_FQL;				// ALARM FQL signal present - true, no - false
	// Pulse signal external interrupt parameters
	INT		m_nEXT_INT_MODE;		// Mode of external interrupt: LOW, CHANGE, RISING, FALLING
	// EMFM pulse counters & flow
	DWORD	m_dwCounterInc;			// Increment counter for input pulses from EMFM/Generator
	DWORD	m_dwCounterDec;			// Decrement counter for input pulses from EMFM/Generator
	FLOAT	m_fQm3h = 0;			// Current flow m3/h
	INT		m_nPulseFactor;			// Number of pulse from EMFM for 1 ltr water
	// For saving time
	DWORD	m_lTime;				// Time in ms
	
public:
	// Constructor, destructor
	CEMFM(	INT nPulse_pin, INT nALM_FQH_pin, INT nALM_FQL_pin, 
			INT nTypePulseFront, INT nTypeALM_FQH, INT nTypeALM_FQL,
			INT nALM_FQHWidth, INT nALM_FQLWidth)
	{
		// PIN
		m_nINP_PULSE_PIN	= nPulse_pin;
		m_nALM_FQH_PIN		= nALM_FQH_pin;
		m_nALM_FQL_PIN		= nALM_FQL_pin;
		
		// Types of signals
		m_nTypePulseFront		= nTypePulseFront;
		m_nTypeALM_FQHSignal	= nTypeALM_FQH;
		m_nTypeALM_FQLSignal	= nTypeALM_FQL;
		
		// Width of pulse
		m_nALM_FQHWidth = nALM_FQHWidth;
		m_nALM_FQLWidth = nALM_FQLWidth;	
	}
	~CEMFM() { }
	
	// --- MODIFYING methods
	// Set counters, define pin modes, set external interrupt ISR	
	void Init(	
			DWORD	dwCountI,		// increment counter for input pulses from EMFM/Generator
			DWORD	dwCountD,		// decrement counter for input pulses from EMFM/Generator
			INT		nPulseFactor,	// number of pulse from EMFM for 1 ltr water
			INT		nINT_MODE,		// mode of external interrupt: LOW, CHANGE, RISING, FALLING
			void	(*ISR)()		// external interrupt ISR
	){
		// Save parameters
		m_dwCounterInc = dwCountI;		// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
		m_dwCounterDec	= dwCountD;		// Current counter for input pulses from EMFM/Generator
		m_nPulseFactor = nPulseFactor;	// Number of pulse from EMFM for 1 ltr water

		// Define pin modes
		pinMode(m_nINP_PULSE_PIN, INPUT);
		pinMode(m_nALM_FQH_PIN, INPUT);
		pinMode(m_nALM_FQL_PIN, INPUT);

		// Initialization time, flow
		m_lTime = millis();
		m_fQm3h = 0.0;

		// Set external interrupt ISR
		m_nEXT_INT_MODE = nINT_MODE;
		attachInterrupt(digitalPinToInterrupt(m_nINP_PULSE_PIN), ISR, m_nEXT_INT_MODE);
	}

	void	SetCounterInc(DWORD dwCountF)	{ m_dwCounterInc = dwCountF;	}
	void	SetCounterDec(DWORD dwCountC)	{ m_dwCounterDec = dwCountC;	}
	void	SetTime(DWORD lTime)			{ m_lTime = lTime;				}
	void	SetQ(FLOAT fQ)					{ m_fQm3h = fQ;					}
	FLOAT	CalculateQ(DWORD dwCounterCurr, DWORD lTimeCurr) 
	{ return m_fQm3h = 3600.0F * (dwCounterCurr - m_dwCounterInc) / m_nPulseFactor / (lTimeCurr - m_lTime); };

	// --- CONSTANT methods
	DWORD	GetCounterInc()	const	{ return m_dwCounterInc;	}
	DWORD	GetCounterDec() const	{ return m_dwCounterDec;	}
	DWORD	GetTime()		const	{ return m_lTime;			}
	FLOAT	GetQ()			const	{ return m_fQm3h;			}
};
#endif

