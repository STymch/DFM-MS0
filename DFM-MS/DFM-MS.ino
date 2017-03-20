/////////////////////////////////////////////////////////////////////////////////
// ---Working Measurement Standard - Digital Flowmeter (DFM-WMS)  ---
/////////////////////////////////////////////////////////////////////////////////
// --- Digital Flowmeter Measuring System: DFM-MS
/////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 by Sergiy Tymchenko
// All rights reserved.
// DNIPRO, UKRAINE
// Tel/Fax: +380 56 7916040 / +380 56 7916066
// Mob/Viber/WhatsApp/Telegram: +380 67 6361855
// Skype: stymch2008
// E-mail : stymch@gmail.com
/////////////////////////////////////////////////////////////////////////////////
// Filename: DFM-MS
// Content:
//  - declare global variables & const
//  - functions:
//		setup()				-	Initialisation FMVI-MS
//		loop()				-	Working loop of FMVI-MS
//		ISR_Timer2()		-	Timer2 ISR callback function
//		ISR_InputPulse1()	-	External interrupt ISR callback function, antitinkling ON
//		ISR_InputPulse2()	-	External interrupt ISR callback function, antitinkling OFF
//		BTSerialReadCmnd()	-	Read command from FMVI-CP by BT Serial Port
//		SerialUI()			-	User interface by srial port
/////////////////////////////////////////////////////////////////////////////////

#include "CommDef.h"
//#include <EEPROM.h>		
#include <OneWire.h>
#include <MsTimer2.h>

#include "CSerialPort.h"
#include "CDataMS.h"
#include "CEMFM.h"
#include "CTemperatureSensor.h"

// Global parameters
// Arduino analog GPIO
const int   POWER_ON_PIN = 0;			// Power analog input pin

// Arduino digital GPIO
const int   EMFM_PIN = 3;				// EMFM digital out input pin
const int   TEST_PIN = 2;				// Test digital generator out input pin
const int   TEMP_PIN = 4;				// Temperature sensor DS18B20 DQ out input pin
const int   ALM_FQH_PIN = 5;			// EEMFM FQH ALARM out input pin
const int   ALM_FQL_PIN = 6;			// EEMFM FQL ALARM out input pin
const int   POWER_OFF_PIN = 7;			// Power off output pin

const int	RX_PIN = 10;				// Software UART RX pin, connect to TX of Bluetooth HC-05 
const int	TX_PIN = 11;				// Software UART TX pin, connect to RX of Bluetooth HC-05
const int   LED_PIN = 13;				// LED output pin

// Serial ports parameters
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM, bps
const long	SERIAL_READ_TIMEOUT = 10;	// Timeout for serial port data read, millisecs

// Time parameters
const DWORD	DELAY_LOOP_MS = 100;		// Delay for main loop, millises
const DWORD	FREQ_TIMER2_MS = 100;		// Timer2 period in milliseconds

// Metric parameters
const DWORD	PULSE_UNIT_LTR = 1000;		// Quantity pulse in 1 ltr

// Global variables
DWORD	dwCountBadPulse = 0;			// Counter bad input pulse packet (pulse front < width)
int		nPulseWidth = 1;				// Width in millisec of EMFM output pulse
int		nALM_FQHWidth = 50;				// Width in millisec of EMFM ALARM FQH signal
int		nALM_FQLWidth = 50;				// Width in millisec of EMFM ALARM FQL signal
DWORD	lTInterval4Q = 500;				// Interval (ms) for calculate current flow Q
int		nEXT_INT_MODE = CHANGE;			// Mode of external interrupt: LOW, CHANGE, RISING, FALLING
void(*pISR)();							// Pointer to external interrupt ISR function 

bool	isMeasuring = false;			// Measuring state flag: true - measuring ON, false - measuring OFF
bool	isAntiTinklingOn = false;		// Antitinkling flag: true - ON, false - OFF

BYTE	pBuff[DATA_LEN+1];

bool	isSerialPrn = true;

INT		i, nLen;
long	lCount = 0;
int		nTypeSerial = 1; // 0 - hardware, 1 - software
DWORD	dwTimerTick = 0;

// Global objects
// Serial ports
CSerialPort		*pBTSerialPort;	// For bluetooth modem 
				
// Data structure of data of FMVI-MS
CDataMS	*volatile pDataMS;
CCmndMS *pCmndMS;

// EMFM/Generator
CEMFM *pEMFM;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire DSTempSensor(TEMP_PIN);

INT		nStatus = 0;
FLOAT	fT = 0.5, fQ = 0.01;
UINT	nU = 2;

///////////////////////////////////////////////////////////////
// Initialisation FMVI-MS
///////////////////////////////////////////////////////////////
void setup() 
{
	// Set the data rate and open hardware COM port:
	Serial.begin(DR_HARDWARE_COM);
	while (!Serial);
	
	// Set pin mode for LED pin
	pinMode(LED_PIN, OUTPUT);

	// Create objects of FMVI-MS:
	pBTSerialPort = new CSerialPort(1, RX_PIN, TX_PIN);

	pBTSerialPort->Init(DR_SOFTWARE_COM, 0);
	pBTSerialPort->SetReadTimeout(SERIAL_READ_TIMEOUT);

	pDataMS = new CDataMS;
	pCmndMS = new CCmndMS;

	pEMFM = new CEMFM(TEST_PIN, ALM_FQH_PIN, ALM_FQH_PIN, LOW, LOW, LOW, nPulseWidth, nALM_FQHWidth, nALM_FQLWidth);
	isAntiTinklingOn = false;
	if (isAntiTinklingOn) {	// antitinkling is ON
		nEXT_INT_MODE = CHANGE;
		pISR = ISR_InputPulse1;
	}
	else {					// antitinkling is OFF
		nEXT_INT_MODE = FALLING;
		pISR = ISR_InputPulse2;
	}
	pEMFM->Init(0, DWORD(-1), lTInterval4Q, PULSE_UNIT_LTR, nEXT_INT_MODE, pISR);


	// Set Timer2 period milliseconds
//	MsTimer2::set(FREQ_TIMER2_MS, ISR_Timer2);
	// Enable Timer2 interrupt
//	MsTimer2::start();
}

///////////////////////////////////////////////////////////////
// Working loop of FMVI-MS
///////////////////////////////////////////////////////////////
void loop()
{
	// Read command from FMVI-CP BT serial port
	::BTSerialReadCmnd();
	
	// Write data into BT COM port
	noInterrupts();
	pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);
	interrupts();
	
	// Read and execute command from serial port
	::SerialUI();
	
	// Calculate current flow Q
	//pDataMS->SetQ(pEMFM->CalculateQ());

	// Test print
	if (lCount % 10 == 0 && isSerialPrn)
//	if (dwTimerTick % 10 == 0 && isSerialPrn)
	{
		// Print number of loop
		Serial.println();		Serial.print(""); Serial.print(lCount);
		Serial.print("\tTT=");	Serial.print(dwTimerTick);
		Serial.print("\tCF=");	Serial.print(pEMFM->GetCountFull());
		Serial.print("\tCC=");	Serial.print(pEMFM->GetCountCurr());
		Serial.print("\tCB=");	Serial.print(dwCountBadPulse);
		Serial.print("\tQ=");	Serial.print(pEMFM->GetQCurr(), 6);
		isSerialPrn = !isSerialPrn;
	}
	else 
//		if (dwTimerTick % 10 != 0)
		if (lCount % 10 != 0)
			if (!isSerialPrn) isSerialPrn = !isSerialPrn;

	// Change data
	nStatus++;
	fT += 0.01;
	nU++;

	// Fill data
	pDataMS->SetStatus(BYTE(nStatus));
	pDataMS->SetTempr(fT);
	pDataMS->SetPowerU(nU);

	// Counter of loops
	lCount++;
	
	// Delay for other tasks
	delay(DELAY_LOOP_MS);
}

///////////////////////////////////////////////////////////////
// Timer2 ISR callback function
///////////////////////////////////////////////////////////////
void ISR_Timer2() {
	static	bool	led_out = HIGH;
	
	// Calculate current flow Q
//	pDataMS->SetQ(pEMFM->CalculateQ());

	// Increment timer's ticks
	dwTimerTick++;

	// Switch on/off LED
	digitalWrite(LED_PIN, led_out);
	led_out = !led_out;
}

///////////////////////////////////////////////////////////////
// External interrupt ISR callback function, antitinkling is on
///////////////////////////////////////////////////////////////
void ISR_InputPulse1()
{
	// Read pulse pin state - is pin state is front?
	if (pEMFM->isPulseFront()) // pulse begin
		pEMFM->SetTStartPulse(millis()); // save time of pulse begin
	else	// pulse end, check correct pulse 
	{
		if (pEMFM->isPulse(millis())) // pulse correct
		{
			// Increment counter of all EMFM pulse
			pEMFM->SetCountFull(pEMFM->GetCountFull() + 1);
			// Save new counter in data packet
			pDataMS->SetCountFull(pEMFM->GetCountFull());

			// Decrement current counter while > 0
			if (pEMFM->GetCountCurr() > 0)
			{
				// Decrement counter
				pEMFM->SetCountCurr(pEMFM->GetCountCurr() - 1);
				// Save new counter in data packet
				pDataMS->SetCountCurr(pEMFM->GetCountCurr());
				
				// If current counter == 0 - set flag of measuring OFF
				if (pEMFM->GetCountCurr() == 0)	isMeasuring = false;
			}
		}
		else	// incorrect pulse, counting!
		{
			dwCountBadPulse++;
		}
	}

	// Calculate current flow Q
	pDataMS->SetQ(pEMFM->CalculateQ());
}
///////////////////////////////////////////////////////////////
// External interrupt ISR callback function, antitinkling is off
///////////////////////////////////////////////////////////////
void ISR_InputPulse2()
{
	// Increment counter of all EMFM pulse
	pEMFM->SetCountFull(pEMFM->GetCountFull() + 1);
	// Save new counter in data packet
	pDataMS->SetCountFull(pEMFM->GetCountFull());

	// Decrement current counter while > 0
	if (pEMFM->GetCountCurr() > 0)
	{
		// Decrement counter
		pEMFM->SetCountCurr(pEMFM->GetCountCurr() - 1);
		// Save new counter in data packet
		pDataMS->SetCountCurr(pEMFM->GetCountCurr());

		// If current counter == 0 - set flag of measuring OFF
		if (pEMFM->GetCountCurr() == 0) isMeasuring = false;
	}

	// Calculate current flow Q
	pDataMS->SetQ(pEMFM->CalculateQ());
}

///////////////////////////////////////////////////////////////
// Read command from FMVI-CP
///////////////////////////////////////////////////////////////
void BTSerialReadCmnd()
{
	DWORD	dwCountCurr;

	// Read command from FMVI-CP
	if (pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1) > 0) {
		// Analize input command
		switch (pCmndMS->GetCode()) 
		{
			case cmndSetCount:			// Set current pulse count 
				// if 0 - set count to DWORD(-1)
				if ((dwCountCurr = pCmndMS->GetArg_dw()) == 0)
				{
					dwCountCurr = DWORD(-1);// set -1 for current counter
					isMeasuring = false;	// set flag of measuring OFF
				}
				// Save current pulse count
				pEMFM->SetCountCurr(dwCountCurr);
				// Save current pulse count into data packet
				pDataMS->SetCountCurr(dwCountCurr);
				// Set flag of measuring ON
				isMeasuring = true;
				break;

			case cmndPowerOff:			// Turn off power

				break;
		
		
			case cmndReadTempr:			// Read temperature from sensor

				break;
		}

	}
}

///////////////////////////////////////////////////////////////
// User interface for srial port
///////////////////////////////////////////////////////////////
void SerialUI()
{
	// Read command from serial monitor 
	if (Serial.available()) {
		char ch = Serial.read();
		switch (ch) {
		case '0'...'9':
			// v = v * 10 + ch - '0';
			break;

		case 'p':
			
			break;
		}
	}


}
