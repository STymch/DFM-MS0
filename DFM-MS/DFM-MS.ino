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
//		setup()					-	Initialisation DFM-MS
//		loop()					-	Working loop of DFM-MS
//		ISR_InputPulseAntOFF	-	ISR callback function for pulse out of EMFM, antitinkling is off
//		ISR_InputPulseAntON		-	ISR callback function for pulse out of EMFM, antitinkling is on
//		ISR_ExtButtonPressAntON -	ISR callback function for press external button, antitinkling is on
//		BTSerialReadCmnd()		-	Read command from DFM-CP by BT Serial Port
//		SerialUI()				-	User interface by srial port (hardware or BT)
/////////////////////////////////////////////////////////////////////////////////

#include "CommDef.h"
//#include <EEPROM.h>		

#include "CSerialPort.h"
#include "CDataMS.h"
#include "CEMFM.h"
#include "CTemperatureSensor.h"
#include "CRHTSensor.h"
#include "CMSExtButton.h"
#include "CLED.h"

// Global constatnts
// Arduino analog GPIO
const int   POWER_ON_PIN	= 0;		// Power analog input pin

// Arduino digital GPIO
const int   EXT_BUTTON_PIN	= 2;		// External button input pin
const int   EMFM_PIN		= 3;		// EMFM digital out input pin

const int   POWER_OFF_PIN	= 4;		// Power off output pin
const int   TEMP_PIN		= 5;		// Temperature sensor DS18B20 DQ out input pin
const int   ALM_FQH_PIN		= 6;		// EEMFM FQH ALARM out input pin
const int   ALM_FQL_PIN		= 7;		// EEMFM FQL ALARM out input pin

const int	RX_PIN			= 10;		// Software UART RX pin, connect to TX of Bluetooth HC-05 
const int	TX_PIN			= 11;		// Software UART TX pin, connect to RX of Bluetooth HC-05
const int   LED_PIN			= 13;		// LED output pin

// Serial ports parameters
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM, bps
const long	COM_READ_TIMEOUT= 10;		// Timeout for serial port data read, millis

// Metric parameters
const DWORD	PULSE_UNIT_LTR	= 1000;		// Quantity pulse in 1 ltr

// Global variables
DWORD	lLoopMSFreq			= 200;		// DFM-MS main loop frequancy, millis
DWORD	lDelayTemprSensor	= 1000;		// Wait for measuring water temperature, millis
DWORD	lInt4CalcQ			= 500;		// Interval for calculate instant flow Q, millis
DWORD	dwCountBadPulse		= 0;		// Counter bad input pulse packet (pulse front < nPulseWidth)
int		nQMA_Points			= 10;		// Number of points for calculate moving average of instant flow Q
int		nPulseWidth			= 50;		// Width in millisec of EMFM output pulse
int		nALM_FQHWidth		= 500;		// Width in millisec of EMFM ALARM FQH signal
int		nALM_FQLWidth		= 500;		// Width in millisec of EMFM ALARM FQL signal
int		nPULSE_INT_MODE		= FALLING;	// Mode of interrupt of EMFM pulse out: LOW, CHANGE, RISING, FALLING
void	(*pISR)();						// Pointer to ISR callback function of EMFM pulse out 
bool	isAntiTinklingOn	= false;	// Antitinkling flag for EMFM output pulse: true - ON, false - OFF
int		nExtButtonPressWidth= 100;		// Width in millisec of external button press
int		nEXT_BUTTON_INT_MODE= CHANGE;	// Mode of interrupt of external button in: LOW, CHANGE, RISING, FALLING

bool	isMeasuring			= false;	// Measuring state flag: true - Test is ON, false - Test is OFF

int		nLEDStateInit		= LOW;		// Init LED state

byte	pBuff[DATA_LEN+1];				// Buffer for save sending data

bool	isSerialPrn = true;
int		i, nLen;
long	lCount = 0;
int		nTypeSerial = 1; // 0 - hardware, 1 - software
DWORD	dwTimerTick = 0;

// ---===--- Global objects ---===---
// --==-- Bluetooth serial port
CSerialPort			*pBTSerialPort;	// For bluetooth modem 
				
// --==-- Send data packet object
CDataMS				*pDataMS;
// --==-- Receive commands object
CCmndMS				*pCmndMS;

// --==-- Flowmeter EMFM
CEMFM				*pEMFM;

// --==-- Humidity & temperature sensor
CRHTSensor			*pRHTSensor;

// --==-- Temperature of water sensor
CTemperatureSensor	*pTemperatureSensor;

// --==-- External button of DFM-MS object
CMSExtButton		*pMSExtButton;

// --==-- LED objects
CLED				*pLED2Loop,		// LED in mail loop
					*pLED2ExtButton;// LED for press button


byte	bStatus = 0;
FLOAT	fTAir, fRHumidityAir, fTWater, fQ = 0.0;
//DWORD	lTimeInt;
UINT	nU = 749;

///////////////////////////////////////////////////////////////
// Initialisation DFM-MS
///////////////////////////////////////////////////////////////
void setup() 
{
	int rc;

	// --==-- Hardware serial port
	// Set the data rate and open
	Serial.begin(DR_HARDWARE_COM);
	while (!Serial);
		
	// --==-- Bluetooth serial port
	// Create object & initialization
	pBTSerialPort = new CSerialPort(1, RX_PIN, TX_PIN);
	pBTSerialPort->Init(DR_SOFTWARE_COM, 0);
	pBTSerialPort->SetReadTimeout(COM_READ_TIMEOUT);

	// --==-- Send data packet object
	// Create object
	pDataMS = new CDataMS;
	// Clear status byte
	pDataMS->SetStatus(0);

	// --==-- Receive commands object
	// Create object
	pCmndMS = new CCmndMS;

	// --==-- Flowmeter EMFM
	// Create object
	pEMFM = new CEMFM(EMFM_PIN, ALM_FQH_PIN, ALM_FQH_PIN, LOW, LOW, LOW, nPulseWidth, nALM_FQHWidth, nALM_FQLWidth);
	if (isAntiTinklingOn == true) {	// antitinkling is ON
		nPULSE_INT_MODE = CHANGE;
		pISR = ISR_InputPulseAntON;
	}
	else {							// antitinkling is OFF
		nPULSE_INT_MODE = FALLING;
		pISR = ISR_InputPulseAntOFF;
	}
	// Initialization
	pEMFM->Init(0, DWORD(-1), lInt4CalcQ, nQMA_Points, PULSE_UNIT_LTR, nPULSE_INT_MODE, pISR);

	// --==-- Humidity & temperature sensor
	pRHTSensor = new CRHTSensor();
	// Get RH and temperature from sensor
	if ( !(rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir)) ) {
		Serial.println();	Serial.print("--==-- DFM-MS: RHT Sensor (HTU21D, SHT21 or Si70xx) OK!");
	}
	else {
		Serial.println();	Serial.print("--==-- DFM-MS: RHT Sensor (HTU21D, SHT21 or Si70xx) Error = "); Serial.print(rc);
	}	
	pDataMS->SetRHTSensorError( !rc ? 0 : 1 );			// set status bit RHT sensor
	pDataMS->SetEndBatteryRHTSensor(rc == 1 ? 1 : 0);	// set status bit end of battery RHT sensor
	pDataMS->SetTemprAir(fTAir);						// set temperature of air
	pDataMS->SetRHumidityAir(fRHumidityAir);			// set humidity

	// --==-- Temperature of water sensor
	pTemperatureSensor = new CTemperatureSensor(TEMP_PIN, lDelayTemprSensor);
	// Get temperature from sensor
	if ( !(rc = pTemperatureSensor->GetTemperature(fTWater)) ) {
		Serial.println();		Serial.print("--==-- DFM-MS: Temperature Sensor OK!");
	}
	else {
		Serial.println();		Serial.print("--==-- DFM-MS: Temperature Sensor Error =\t"); Serial.print(rc);
	}
	pDataMS->SetTempSensorError( !rc ? 0 : 1);	// set status bit
	pDataMS->SetTemprWater(fTWater);			// set water temperature
	
	// --==-- External button of DFM-MS object
	// Create object
	pMSExtButton = new CMSExtButton(EXT_BUTTON_PIN, LOW, nExtButtonPressWidth, nEXT_BUTTON_INT_MODE, ISR_ExtButtonPressAntON);

	// Set initial U battery
	pDataMS->SetPowerU(nU);

	// --==-- LED in mail loop
	pLED2Loop = new CLED(LED_PIN, nLEDStateInit);

	// --==--  LED for press button
	pLED2ExtButton = new CLED(LED_PIN, nLEDStateInit);

	// --==-- Delay before starting main loop
	Serial.println();	Serial.print("--==-- DFM-MS: Starting main loop after 5 sec ...");
	delay(5000);
}

///////////////////////////////////////////////////////////////
// Working loop of DFM-MS
///////////////////////////////////////////////////////////////
void loop()
{
	
	DWORD lTimeBegin, lTimeInt;

	// Save time of begin of loop
	lTimeBegin = millis();

	// Read and execute command from DFM-CP BT serial port
	::BTSerialReadCmnd();

	// Write data into BT serial port
	noInterrupts();
	pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);
	interrupts();

	// Calculate current and moving average of flow Q: method 1
	noInterrupts();
	pEMFM->CalculateQ();
	interrupts();

/*
	// Calculate current and moving average of flow Q: method 2
	if (lCount % (TIME_INT4Q / DELAY_LOOP_MS) == 0)
	{
		noInterrupts();
		pEMFM->CalculateQ(TIME_INT4Q);
		interrupts();
	}
*/
	// Set moving average of Q into data packet
	pDataMS->SetQ(pEMFM->GetQMA());
//	pDataMS->SetQ(pEMFM->GetQCurr());

	// Counter of loops
	lCount++;

	// Generate test flow
	for( int i=0; i++ < 12; ISR_InputPulseAntOFF());
		
	// Blink LED and debug print to serial port console
	if (lCount % 10 == 0 && isSerialPrn)
	{
		// Blink LED 
		pLED2Loop->Blink();

/*		// Print data
		Serial.println();		Serial.print(""); Serial.print(lCount);
		Serial.print("\tCF=");	Serial.print(pEMFM->GetCountFull(), 10);
		Serial.print("\tCC=");	Serial.print(pEMFM->GetCountCurr(), 10);
		//Serial.print("\tCB=");	Serial.print(dwCountBadPulse);
		Serial.print("\tQ=");	Serial.print(pEMFM->GetQCurr(), 3);
		Serial.print("\tQMA=");	Serial.print(pEMFM->GetQMA(), 3);
		Serial.print("\tU=");	Serial.print(pDataMS->GetPowerU());
*/
		}

	// Loop time interval
	lTimeInt = millis() - lTimeBegin;

	// Delay for other tasks
	delay(lLoopMSFreq - lTimeInt);
}

///////////////////////////////////////////////////////////////
// ISR callback function for pulse out of EMFM, antitinkling is off
///////////////////////////////////////////////////////////////
void ISR_InputPulseAntOFF()
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

		// Read Timer and save it in data packet
		pDataMS->SetTimeInt(pEMFM->GetTimer());

		// If current counter == 0
		if (pEMFM->GetCountCurr() == 0) {
			isMeasuring = false;	// set flag of measuring OFF
			pEMFM->StopTimer();		// stop Timer
		}
	}
}

///////////////////////////////////////////////////////////////
// ISR callback function for pulse out of EMFM, antitinkling is on
///////////////////////////////////////////////////////////////
void ISR_InputPulseAntON()
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

				// Read Timer and save it in data packet
				pDataMS->SetTimeInt(pEMFM->GetTimer());

				// If current counter == 0
				if (pEMFM->GetCountCurr() == 0) {
					isMeasuring = false;	// set flag of measuring OFF
					pEMFM->StopTimer();		// stop Timer
				}
			}
		}
		else	// incorrect pulse, counting!
		{
			dwCountBadPulse++;
		}
	}
}

///////////////////////////////////////////////////////////////
// ISR callback function for press external button, antitinkling is on
///////////////////////////////////////////////////////////////
void ISR_ExtButtonPressAntON()
{
	// Read press button pin state - is pin state is front?
	if (pMSExtButton->isPressFront())			// button press begin
		pMSExtButton->SetTStartPress(millis()); // save time of button press begin
	else	// button press end, check correct width of button press 
		if (pMSExtButton->isPress(millis())) // button press correct
		{
			// Blink LED 
			pLED2ExtButton->Blink();
			
			// Reverse star-stop bit in status byte of DFM-MS
			pDataMS->SetStartStopExt(!pDataMS->GetStartStopExt() ? 1 : 0);
		}
}

///////////////////////////////////////////////////////////////
// Read command from DFM-CP
///////////////////////////////////////////////////////////////
void BTSerialReadCmnd()
{
	DWORD	dwCountCurr;
	int		nErrCode, rc;

	// Read command from DFM-CP
	if ((nErrCode = pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1)) > 0) {
//		Serial.println();
//		Serial.print(" Cmnd=");	Serial.print(pCmndMS->GetCode());
//		Serial.print("\tArg=");	Serial.print(pCmndMS->GetArg_dw());
		
		// Analize input command
		switch (pCmndMS->GetCode()) 
		{
			// 67 Set current pulse count
			case cmndSetCount: 
				// if 0 - set count to DWORD(-1)
				if ((dwCountCurr = pCmndMS->GetArg_dw()) == 0)
				{
					dwCountCurr = DWORD(-1);// set -1 for current counter
					isMeasuring = false;	// set flag of measuring OFF
					pEMFM->StopTimer();		// stop Timer
				}
				else
				{
					isMeasuring = true;		// set flag of measuring ON
					pEMFM->StartTimer();	// start timer
				}

				// Save current pulse count
				noInterrupts();
				pEMFM->SetCountCurr(dwCountCurr);
				// Save current pulse count into data packet
				pDataMS->SetCountCurr(dwCountCurr);
				interrupts();
//					Serial.print("\tPASSED");				
				break;

			// 84 Read water temperature from sensor
			case cmndReadTemprWater:
				if (!isMeasuring) {
					// Read temperature and save in data packet
					pDataMS->SetTempSensorError(!pTemperatureSensor->GetTemperature(fTWater) ? 0 : 1);
					pDataMS->SetTemprWater(fTWater);
//					Serial.print("\tPASSED");				
				}
//				else Serial.print("\tSKIP");
				break;

			// 72 Read humidity and temperature from RHT sensor
			case cmndReadRHT:
				if (!isMeasuring) {
					// Read humidity and temperature from RHT sensor and save in data packet
					rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir);
					pDataMS->SetRHTSensorError(!rc ? 0 : 1);			// set status bit of sensor
					pDataMS->SetEndBatteryRHTSensor(rc == 1 ? 1 : 0);	// set status bit end of battery RHT sensor
					pDataMS->SetTemprAir(fTAir);
					pDataMS->SetRHumidityAir(fRHumidityAir);
//					Serial.print("\tPASSED");
				}
//				else Serial.print("\tSKIP");
				break;

			// 80 Turn off power
			case cmndPowerOff:
				break;


			// 160 Set DFM-MS main loop frequancy, millis. Default = 200.
			case cmndSetLoopMSFreq:
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					lLoopMSFreq = pCmndMS->GetArg_dw();
					interrupts();
//					Serial.print("\tPASSED");
				}
//				else Serial.print("\tSKIP");
				break;

			// 161 Set number of points for calculate moving average of flow Q. Default = 10.
			case cmndSetQMA_Points:
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					pEMFM->SetQMA_Points(nQMA_Points = pCmndMS->GetArg_n());
					interrupts();
//					Serial.print("\tPASSED");
				}
//				else Serial.print("\tSKIP");
				break;

			// 162 Set interval for calculate instant flow Q, millis. Default = 500.
			case cmndSetInt4CalcQ:
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					pEMFM->SetInt4CalcQ(lInt4CalcQ = pCmndMS->GetArg_dw());
					interrupts();
//					Serial.print("\tPASSED");
				}
//				else Serial.print("\tSKIP");
				break;

			// 163 Set width in millisec of external button press. Default = 100.
			case cmndSetButtonPressWidth:
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					pMSExtButton->SetExtButtonPressWidth(nExtButtonPressWidth = pCmndMS->GetArg_n());
					interrupts();
//					Serial.print("\tPASSED");
				}
//				else Serial.print("\tSKIP");
				break;

			// 164 Set antitinkling flag for EMFM output pulse: 1 - ON, 0 - OFF. Default = 0.
			case cmndSetAntiTinklingOn:
				if (!isMeasuring) {
					// Change parameter
					isAntiTinklingOn = pCmndMS->GetArg_b();
					if (isAntiTinklingOn == true) {	// antitinkling is ON
						nPULSE_INT_MODE = CHANGE;
						pISR = ISR_InputPulseAntON;
					}
					else {							// antitinkling is OFF
						nPULSE_INT_MODE = FALLING;
						pISR = ISR_InputPulseAntOFF;
					}
					// Re-Initialization
					detachInterrupt(digitalPinToInterrupt(EMFM_PIN));
					pEMFM->Init(0, DWORD(-1), lInt4CalcQ, nQMA_Points, PULSE_UNIT_LTR, nPULSE_INT_MODE, pISR);
//					Serial.print("\tPASSED");
				}
//				else Serial.print("\tSKIP");
				break;

			// 165 Set Width in millisec of EMFM output pulse. Default = 50.
			case cmndSetPulseWidth:
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					pEMFM->SetPulseWidth(nPulseWidth = pCmndMS->GetArg_n());
					interrupts();
//					Serial.print("\tPASSED");
				}
//				else Serial.print("\tSKIP");
				break;

		}
	}
	else if (nErrCode != -1) {
//		Serial.print(" Err=");	Serial.print(nErrCode);
	}
}