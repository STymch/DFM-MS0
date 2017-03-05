/////////////////////////////////////////////////////////////////////////////////
// --- Flowmeter verification installation - working measurement standard  ---
/////////////////////////////////////////////////////////////////////////////////
// --- Flowmeter verification installation measuring system: FMVI-MS
/////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 by Sergiy Tymchenko
// All rights reserved.
// DNIPRO, UKRAINE
// Tel/Fax: +380 56 7916040 / +380 56 7916066
// Mob/Viber/WhatsApp/Telegram: +380 67 6361855
// Skype: stymch2008
// E-mail : stymch@gmail.com
/////////////////////////////////////////////////////////////////////////////////
// Filename: FMVI-MS
// Content:
//  - declare global variables & const
//  - functions:
//		setup()				-	Initialisation FMVI-MS
//		loop()				-	Working loop of FMVI-MS
//		ISR_Timer2()		-	Timer2 ISR callback function
//		ISR_InputPulse()	-	External interrupt ISR callback function
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

// Arduino external interrupts
//const int	INT_0 = 0;					// external interrupt 0 (connect to digital pin 2)
//const int	INT_1 = 1;					// external interrupt 1 (connect to digital pin 3)
//const int	MODE_INT = CHANGE;			// mode of external interrupt: on change state pin


// Serial ports parameters
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM, bps
const long	SERIAL_READ_TIMEOUT = 10;	// Timeout for serial port data read, millisecs

// Time parameters
const int	DELAY_BEFORE_READ_BT = 250;	// Delay before read data from BT COM port, millises
const DWORD	FREQUENCY_TIMER2_MS = 250;	// Timer2 period in milliseconds

// Metric parameters
const DWORD	PULSE_UNIT_LTR = 1000;		// Quantity pulse in 1 ltr

// Global variables
DWORD	dwCountBadPulse = 0;			// Counter bad input pulse packet (front less width)
int		nPulseWidth = 0;				// Width in millisec of EMFM output pulse
int		nALM_FQHWidth = 50;				// Width in millisec of EMFM ALARM FQH signal
int		nALM_FQLWidth = 50;				// Width in millisec of EMFM ALARM FQL signal
DWORD	lTInterval4Q = 750;				// Interval (ms) for calculate current flow Q
int		nEXT_INT_MODE = CHANGE;			// Mode of external interrupt: LOW, CHANGE, RISING, FALLING

bool	isAntiTinklingOn = true;		// Antitinkling flag: true - On, false - Off
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
	pEMFM->Init(0, DWORD(-1), lTInterval4Q, PULSE_UNIT_LTR, nEXT_INT_MODE, ISR_InputPulse);

	// Set Timer2 period milliseconds
	MsTimer2::set(FREQUENCY_TIMER2_MS, ISR_Timer2);
	// Enable Timer2 interrupt
	MsTimer2::start();
}

///////////////////////////////////////////////////////////////
// Working loop of FMVI-MS
///////////////////////////////////////////////////////////////
void loop()
{
	
	// Read and execute command from serial port
	::SerialUserInterface();

	
	// Test print
	//if (lCount % 100 == 0)
	if (dwTimerTick % 10 == 0 && isSerialPrn)
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
	else if (dwTimerTick % 10 != 0)
			if (!isSerialPrn) isSerialPrn = !isSerialPrn;

	// Fill data
	pDataMS->SetStatus(BYTE(nStatus));
	pDataMS->SetTempr(fT);
	pDataMS->SetPowerU(nU);
		

	// Counter of loops
	lCount++;
		
	// Change data
	nStatus++;
	fT += 0.01;
	nU++;
}

///////////////////////////////////////////////////////////////
// Timer2 ISR callback function
///////////////////////////////////////////////////////////////
void ISR_Timer2() {
	static	bool	led_out = HIGH;
	static	DWORD	dwCountPulse1 = 0, dwCountPulse2 = 0;
	float	fQ;
	DWORD	dwCountCurr;

	// Read command from FMVI-CP
	if (pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1) > 0) {
		// Analize input command
		switch (pCmndMS->GetCode()) {
		case cmndSetCount:			// Set current pulse count 
			// if 0 - set count to DWORD(-1)
			if ((dwCountCurr = pCmndMS->GetArg_dw()) == 0)	dwCountCurr = DWORD(-1);
			// Save current pulse count
			pEMFM->SetCountCurr(dwCountCurr);
			// Save current pulse count into data packet
			pDataMS->SetCountCurr(dwCountCurr);
			break;

		case cmndPowerOff:			// Turn off power

			break;
		}

	}

	// Calculate current flow Q
//	dwCountPulse2 = pEMFM->GetCountFull();
	pDataMS->SetQ(pEMFM->CalculateQ());
//	fQ = 3600.0F * (dwCountPulse2 - dwCountPulse1) / (PULSE_UNIT_LTR * FREQUENCY_TIMER2_MS);
//	pDataMS->SetQ(fQ);
//	pEMFM->SetQCurr(fQ);
//	dwCountPulse1 = dwCountPulse2;

	// Write data into BT COM port
	pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);

	// Increment timer's ticks
	dwTimerTick++;

	// Switch on/off LED
	digitalWrite(LED_PIN, led_out);
	led_out = !led_out;
}

///////////////////////////////////////////////////////////////
// External interrupt ISR callback function
///////////////////////////////////////////////////////////////
void ISR_InputPulse()
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
				// If current counter == 0 - write data packet into BT COM port
				if (pEMFM->GetCountCurr() == 0)
					pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);
			}
		}
		else	// incorrect pulse, counting!
		{
			dwCountBadPulse++;
		}
	}
}

///////////////////////////////////////////////////////////////
// User interface for srial port
///////////////////////////////////////////////////////////////
void SerialUserInterface()
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
