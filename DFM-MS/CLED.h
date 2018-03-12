// CLED.h

#ifndef _CLED_h
#define _CLED_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

///////////////////////////////////////////////////////
// <<< CLED - class for work with LED Arduino
///////////////////////////////////////////////////////
class CLED
{
	protected:
		INT	m_nLED_PIN;		// LED output pin
		INT m_nLED_State;	// Current LED state: LOW, HIGH

	public:
		// Constructor, destructor
		CLED(
			INT nLED_PIN,			// LED output pin
			INT nLED_State = LOW	// Current LED state: LOW, HIGH
		)
		{
			// Save parameters
			m_nLED_PIN		= nLED_PIN;
			m_nLED_State	= nLED_State;
			
			// Define pin mode for LED
			pinMode(m_nLED_PIN, OUTPUT);
			// Write LED state
			digitalWrite(m_nLED_PIN, m_nLED_State);
		}

		~CLED() { }

		// Methods
		// Blink LED
		void	Blink() { m_nLED_State = !m_nLED_State;	digitalWrite(m_nLED_PIN, m_nLED_State); }
};
#endif

