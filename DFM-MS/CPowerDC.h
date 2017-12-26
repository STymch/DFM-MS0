// CPowerDC.h

#ifndef _CPOWERDC_h
#define _CPOWERDC_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

///////////////////////////////////////////////////////
// <<< CPowerDC - class for Power DC control
///////////////////////////////////////////////////////
class CPowerDC
{
protected:
	int m_nPowerInput_PIN;		// Power input analog pin
	int	m_nPowerOnOff_PIN;		// Power on/off output digital pin
	int	m_nDelayAfterPowerON;	// Wait for Power ON, millis

public:
	// Constructor, destructor
	CPowerDC(int nPowerInput_PIN, int nPowerOnOff_PIN, int nDelayAfterPowerON) :
		m_nPowerInput_PIN(nPowerInput_PIN), m_nPowerOnOff_PIN(nPowerOnOff_PIN), m_nDelayAfterPowerON(nDelayAfterPowerON)
	{
		// Declare the PowerOnOff pin as an OUTPUT:
		pinMode(m_nPowerOnOff_PIN, OUTPUT);

	}
	~CPowerDC() { }

	// Methods:
	// Power ON
	void PowerON() {
		// Set PowerOnOff pin to HIGH level 
		digitalWrite(m_nPowerOnOff_PIN, HIGH);

		// Wait for Power ON
		delay(m_nDelayAfterPowerON);
	}

	// Power OFF
	void PowerOFF() {
		// Set PowerOnOff pin to LOW level 
		digitalWrite(m_nPowerOnOff_PIN, LOW);

		// Wait for Power OFF
		delay(m_nDelayAfterPowerON);
	}

	// Get Power DC
	int	GetPowerDC() {
		// read the value from Power input pin:
		return analogRead(m_nPowerInput_PIN);
	}
};

#endif

