///////////////////////////////////////////////////////
// <<< CEMFM - class for EMFM QTLD-15 / Generator
///////////////////////////////////////////////////////

#include "CEMFM.h"
/*
// Define pin modes, set external interrupt ISR	
void CEMFM::Init(
		DWORD	dwCountF,		// counter of all input pulses from EMFM/Generator from turn on FMVI-MS
//		DWORD	dwCountC,		// current counter for input pulses from EMFM/Generator
		INT		nPulseFactor,	// Number of pulse from EMFM for 1 ltr water	
		INT		nINT_MODE,		// mode of external interrupt: LOW, CHANGE, RISING, FALLING
		void	(*ISR)()		// external interrupt ISR
)
{
	// Save parameters
	m_dwCounterAll	= dwCountF;		// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
//	m_dwCounterCurr	= dwCountC;		// Current counter for input pulses from EMFM/Generator
	m_nPulseFactor	= nPulseFactor;	// Number of pulse from EMFM for 1 ltr water

	// Define pin modes
	pinMode(m_nINP_PULSE_PIN,	INPUT);
	pinMode(m_nALM_FQH_PIN,		INPUT);
	pinMode(m_nALM_FQL_PIN,		INPUT);

	// Initialization time, flow
	m_lTime = millis();
	m_fQm3h = 0.0;
	
	// Set external interrupt ISR
	m_nEXT_INT_MODE = nINT_MODE;
	attachInterrupt(digitalPinToInterrupt(m_nINP_PULSE_PIN), ISR, m_nEXT_INT_MODE);
}
*/