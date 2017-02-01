// 
// 
// 

#include "CEMFM.h"

// Define pin modes, set external interrupt ISR	
void CEMFM::Init(	INT nINT_NUM,	// number of external interrupt: 0 - digital pin 2, 1 - digital pin 3 
					INT nINT_MODE,	// mode of external interrupt: LOW, CHANGE, RISING, FALLING
					void (ISR)()	// external interrupt ISR
				)
{
	m_nEXT_INT_NUM = nINT_NUM;
	m_nEXT_INT_MODE = nINT_MODE;

	// Define pin modes:
	pinMode(m_nINP_PULSE_PIN, INPUT);
	pinMode(m_nALM_FQH_PIN, INPUT);
	pinMode(m_nALM_FQL_PIN, INPUT);

	// Set external interrupt ISR
	attachInterrupt(m_nEXT_INT_NUM, ISR, m_nEXT_INT_MODE);
}

