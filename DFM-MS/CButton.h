// CButton.h

#ifndef _CBUTTON_h
#define _CBUTTON_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "CommDef.h"

///////////////////////////////////////////////////////
// <<< CButton - class for external button of DFM-MS
///////////////////////////////////////////////////////
class CButton
{
protected:
	INT		m_nINP_EXT_BUTTON_PIN;	// Input pin for connect external button
	INT		m_nTypePressButtonFront;// Type of ptress button front: LOW or HIGH 
	INT		m_nButtonPressWidth;	// Width in millisec of external button press for Pulse type of Button
	INT		m_nButtonIntMode;		// Mode of interrupt of press button: LOW, CHANGE, RISING, FALLING
	DWORD	m_lTStartPressFront;	// Time in ms of starting front of press button for Pulse type

public:
	// Constructor, destructor
	CButton(
		INT nButton_PIN,		// Input pin for connect external button
		INT nTypePressFront,	// Type of ptress button front: LOW or HIGH
		INT nPressWidth,		// Width in millisec of external button press
		INT	nINT_MODE,			// Mode of external interrupt: LOW, CHANGE, RISING, FALLING
		void(*ISR)()			// ISR callback function
	)
	{
		// Save parameters
		m_nINP_EXT_BUTTON_PIN	= nButton_PIN;
		m_nTypePressButtonFront = nTypePressFront;
		m_nButtonPressWidth		= nPressWidth;
		m_nButtonIntMode		= nINT_MODE;

		// Define pin mode
		pinMode(m_nINP_EXT_BUTTON_PIN, INPUT);

		// Set ISR callback function
		attachInterrupt(digitalPinToInterrupt(m_nINP_EXT_BUTTON_PIN), ISR, m_nButtonIntMode);
	}

	~CButton() { }

	// Methods
	// Is front of button press?
	bool	isPressFront() { return (digitalRead(m_nINP_EXT_BUTTON_PIN) == m_nTypePressButtonFront) ? true : false; }
	// Is button press correct?
	bool	isPress(DWORD lTime) { return (lTime >= m_lTStartPressFront + m_nButtonPressWidth) ? true : false; }
	// Save time of start button press for Pulse type of Button
	void	SetTStartPress(DWORD lTime) { m_lTStartPressFront = lTime; }
};

#endif

