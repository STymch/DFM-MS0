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
		void	(*ISR)()		// external interrupt ISR
)
{
	// Set counters and interval for calculate flow
	m_dwCountFull = dwCountF;	// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	m_dwCountCurr = dwCountC;	// Current counter for input pulses from EMFM/Generator
	m_lTInterval4Q = lTInterval4Q;	// Interval (ms) for calculate current flow

	m_nPulseOnLtr = nPulseOnLtr;	// Number of pulse from EMFM for 1 ltr water

	// Define pin modes
	pinMode(m_nINP_PULSE_PIN,	INPUT);
	pinMode(m_nALM_FQH_PIN,		INPUT);
	pinMode(m_nALM_FQL_PIN,		INPUT);

	// Set external interrupt ISR
	m_nEXT_INT_MODE = nINT_MODE;
	attachInterrupt(digitalPinToInterrupt(m_nINP_PULSE_PIN), ISR, m_nEXT_INT_MODE);
}

// Calculate current flow Q
FLOAT	CEMFM::CalculateQ()
{
	DWORD	lTInterval;

	// Is Q calculated - starting new calculatio process:
	if (m_isQCalculate) {
		m_lTStartQ = millis();				// save time of begin interval of calculation Q
		m_dwCountStartQ = m_dwCountFull;	// save initial value of pulse couner
		m_isQCalculate = !m_isQCalculate;	// reset flag
	}
	// Q is not calculated yet
	else {
		// Calculate time interval
		lTInterval = millis() - m_lTStartQ;
		// Check full time interval for calculation Q 
		if (lTInterval >= m_lTInterval4Q)		// calculate Q
		{	m_fQm3h = 3600.0 * (m_dwCountFull - m_dwCountStartQ) / (m_nPulseOnLtr * lTInterval);
			m_isQCalculate = !m_isQCalculate;	// reset flag Q calculated
		}
	}
	return m_fQm3h;
}