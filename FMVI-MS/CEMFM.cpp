// 
// 
// 

#include "CEMFM.h"

// Define pin modes, set external interrupt ISR	
void CEMFM::Init(
		DWORD	dwCountF,		// counter of all input pulses from EMFM/Generator from turn on FMVI-MS
		DWORD	dwCountC,		// current counter for input pulses from EMFM/Generator
		UINT	nTInterval4Q,	// interval in millisec for calculate current Q
		UINT	nPulseOnLtr,	// Number of pulse from EMFM for 1 ltr water	
		INT		nINT_NUM,		// number of external interrupt: 0 - digital pin 2, 1 - digital pin 3 
		INT		nINT_MODE,		// mode of external interrupt: LOW, CHANGE, RISING, FALLING
		void	(*ISR)()		// external interrupt ISR
)
{
	// Set counters and interval for calculate flow
	m_dwCountFullPulse = dwCountF;	// Counter of all input pulses from EMFM/Generator from turn on FMVI-MS
	m_dwCountCurrPulse = dwCountC;	// Current counter for input pulses from EMFM/Generator
	m_nTInterval4Q = nTInterval4Q;	// Interval (ms) for calculate current flow

	m_nPulseOnLtr = nPulseOnLtr;	// Number of pulse from EMFM for 1 ltr water

	// Define pin modes
	pinMode(m_nINP_PULSE_PIN, INPUT);
	pinMode(m_nALM_FQH_PIN, INPUT);
	pinMode(m_nALM_FQL_PIN, INPUT);

	// Set external interrupt ISR
	m_nEXT_INT_NUM = nINT_NUM;
	m_nEXT_INT_MODE = nINT_MODE;
	attachInterrupt(m_nEXT_INT_NUM, ISR, m_nEXT_INT_MODE);
}

// Calculate current flow Q
FLOAT	CEMFM::CalculateQ()
{
	UINT nTInterval;

	// Is Q calculated - starting new calculatio process:
	if (m_isQCalculate) {
		m_lTStartQ = millis();				// save time of begin interval of calculation Q
		m_dwCountStartQ = m_dwCountFullPulse;	// save initial value of pulse couner
		m_isQCalculate = !m_isQCalculate;		// reset flag
	}
	// Q is not calculated yet
	else {
		nTInterval = millis() - m_lTStartQ;	// calculate time interval
		// Check time interval for calculation Q 
		if (nTInterval >= m_nTInterval4Q)	// interval full, calculation Q
		{
			m_fQm3h = 36.000*(m_dwCountFullPulse - m_dwCountStartQ) / (10.000*m_nPulseOnLtr*nTInterval);
			m_isQCalculate = !m_isQCalculate;			// reset flag
		}
	}
	return m_fQm3h;
}