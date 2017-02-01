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
	INT		m_nINP_PULSE_PIN;		// Input pin for connect EMFM/Generator out pulse
	INT		m_nALM_FQH_PIN;			// Input pin for EMFM ALARM FQH signal
	INT		m_nALM_FQL_PIN;			// Input pin for EMFM ALARM FQL signal
	
	INT		m_nTypePulseFront;		// Type of EMFM/Generator Output pulse front: LOW or HIGH  
	INT		m_nTypeALM_FQHSignal;	// Type of EMFM ALARM FQH signal: LOW or HIGH  
	INT		m_nTypeALM_FQLSignal;	// Type of EMFM ALARM FQL signal: LOW or HIGH
	
	UINT	m_nInpPulseWidth;		// Width in millisec of EMFM output pulse
	UINT	m_nALM_FQHWidth;		// Width in millisec of EMFM ALARM FQH signal
	UINT	m_nALM_FQLWidth;		// Width in millisec of EMFM ALARM FQL signal
	
	bool	m_bALM_FQH;				// ALARM FQH signal present - true, no - false
	bool	m_bALM_FQL;				// ALARM FQL signal present - true, no - false

	INT		m_nEXT_INT_NUM;			// Number of external interrupt: 0 - digital pin 2, 1 - digital pin 3
	INT		m_nEXT_INT_MODE;		// mode of external interrupt: LOW, CHANGE, RISING, FALLING
	
public:
	// Static data
	volatile static DWORD	dwCountFullPulse;	// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	volatile static DWORD	dwCountCurrPulse;	// Current counter for input pulses from EMFM/Generator
	
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
	
	// Define pin modes, set external interrupt ISR	
	void Init(	INT nINT_NUM,	// number of external interrupt: 0 - digital pin 2, 1 - digital pin 3 
				INT nINT_MODE,	// mode of external interrupt: LOW, CHANGE, RISING, FALLING
				void (ISR)()	// external interrupt ISR
	);

	INT		GetPulsePin() { return m_nINP_PULSE_PIN;  }
	INT		GetTypePulseFront() { return m_nTypePulseFront; }
	UINT	GetInpPulseWidth(){ return m_nInpPulseWidth; }
};



#endif

