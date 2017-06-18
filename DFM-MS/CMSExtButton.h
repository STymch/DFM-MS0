// CMSExtButton.h

#ifndef _CMSEXTBUTTON_h
#define _CMSEXTBUTTON_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

///////////////////////////////////////////////////////
// <<< CMSExtButton - class for external button of DFM-MS
///////////////////////////////////////////////////////
class CMSExtButton
{
	protected:
		int		m_nINP_EXT_BUTTON_PIN;		// Input pin for connect external button
		int		m_nTypePressButtonFront;	// Type of ptress button front: LOW or HIGH 
		int		m_nExtButtonPressWidth;		// Width in millisec of external button press
		int		m_nExtButtonIntMode;		// Mode of interrupt of press button: LOW, CHANGE, RISING, FALLING
		DWORD	m_lTStartPressFront;		// Time in ms of starting front of press button

	public:
		// Constructor, destructor
		CMSExtButton(
			int nButton_PIN,		// Input pin for connect external button
			int nTypePressFront,	// Type of ptress button front: LOW or HIGH
			int nPressWidth,		// Width in millisec of external button press
			int	nINT_MODE,			// Mode of external interrupt: LOW, CHANGE, RISING, FALLING
			void(*ISR)()			// ISR callback function
		)
		{
			// Save parameters
			m_nINP_EXT_BUTTON_PIN	= nButton_PIN;
			m_nTypePressButtonFront = nTypePressFront;
			m_nExtButtonPressWidth	= nPressWidth;
			m_nExtButtonIntMode		= nINT_MODE;


			// Define pin mode
			pinMode(m_nINP_EXT_BUTTON_PIN, INPUT);

			// Set ISR callback function
			attachInterrupt(digitalPinToInterrupt(m_nExtButtonIntMode), ISR, m_nExtButtonIntMode);
		}
	
		~CMSExtButton() { }

		// Methods
		// Is front of button press?
		bool	isPressFront() { return (digitalRead(m_nINP_EXT_BUTTON_PIN) == m_nTypePressButtonFront) ? TRUE : FALSE; }
		// Is button press correct?
		bool	isPress(DWORD lTime) { return (lTime >= m_lTStartPressFront + m_nExtButtonPressWidth) ? TRUE : FALSE; }
		// Save time of start button press
		void	SetTStartPress(DWORD lTime) { m_lTStartPressFront = lTime; }
};


#endif

