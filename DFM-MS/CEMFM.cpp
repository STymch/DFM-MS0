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
		{	m_fQm3h = 3600.0F * (m_dwCountFull - m_dwCountStartQ) / (m_nPulseOnLtr * lTInterval);
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