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
//#include <OneWire.h>
//#include <MsTimer2.h>

#include "CSerialPort.h"
#include "CDataMS.h"
#include "CEMFM.h"
#include "CTemperatureSensor.h"
#include "CRHTSensor.h"

// Global parameters
// Arduino analog GPIO
const int   POWER_ON_PIN = 0;			// Power analog input pin

// Arduino digital GPIO
const int   EMFM_PIN = 3;				// EMFM digital out input pin
const int   TEST_PIN = 2;				// Test digital generator out input pin
const int   TEMP_PIN = 5;				// Temperature sensor DS18B20 DQ out input pin
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
const DWORD	DELAY_TEMP_SENSOR = 1000;	// Delay for measuring temperature
//const DWORD	FREQ_TIMER2_MS = 100;	// Timer2 period in milliseconds

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

byte	pBuff[DATA_LEN+1];

bool	isSerialPrn = true;

int		i, nLen;
long	lCount = 0;
int		nTypeSerial = 1; // 0 - hardware, 1 - software
DWORD	dwTimerTick = 0;

// Global objects
// Serial ports
CSerialPort		*pBTSerialPort;	// For bluetooth modem 
				
// Data structure of data of DFM-MS
CDataMS	*pDataMS;
CCmndMS	*pCmndMS;

// EMFM/Generator
CEMFM	*pEMFM;

// Air compensated humidity and temperature sensor
CRHTSensor	*pRHTSensor;

// Water temperature sensor
CTemperatureSensor	*pTemperatureSensor;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire DSTempSensor(TEMP_PIN);

byte	bStatus = 0;
FLOAT	fTAir, fRHumidityAir, fTWater, fQ = 0.0;
DWORD	lTimeInt;
UINT	nU = 749;

///////////////////////////////////////////////////////////////
// Initialisation FMVI-MS
///////////////////////////////////////////////////////////////
void setup() 
{
	int		rc;
	
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
	pDataMS->SetStatus(0);

	pCmndMS = new CCmndMS;

	pEMFM = new CEMFM(TEST_PIN, ALM_FQH_PIN, ALM_FQH_PIN, LOW, LOW, LOW, nPulseWidth, nALM_FQHWidth, nALM_FQLWidth);
	
	if (isAntiTinklingOn == true) {	// antitinkling is ON
		nEXT_INT_MODE = CHANGE;
		pISR = ISR_InputPulse1;
	}
	else {					// antitinkling is OFF
		nEXT_INT_MODE = FALLING;
		pISR = ISR_InputPulse2;
	}
	pEMFM->Init(0, DWORD(-1), lTInterval4Q, PULSE_UNIT_LTR, nEXT_INT_MODE, pISR);


	pRHTSensor = new CRHTSensor();
	if (rc = pRHTSensor->Detect())
	{
		Serial.println();	Serial.print("--== RHT Sensor (HTU21D, SHT21 or Si70xx) Error="); Serial.print(rc);
		pDataMS->SetRHTSensorError(1);	// set status bit
		fTAir = -1;						// temperature = -1
		fRHumidityAir = -1;				// humidity
	}
	else
	{
		Serial.println();	Serial.print("--== RHT Sensor (HTU21D, SHT21 or Si70xx) OK!");
		pDataMS->SetRHTSensorError(0);	// set status bit
		pRHTSensor->GetRHT(fRHumidityAir,fTAir);	// Get RH and temperature from sensor
	}
	pDataMS->SetTemprAir(fTAir);
	pDataMS->SetRHumidityAir(fRHumidityAir);


	pTemperatureSensor = new CTemperatureSensor(TEMP_PIN, DELAY_TEMP_SENSOR);
	if (rc = pTemperatureSensor->Detect()) 
	{
		Serial.println();		Serial.print("--== Temperature Sensor Error="); Serial.print(rc);
		pDataMS->SetTempSensorError(1);	// set status bit
		fTWater = -1;					// temperature = -1
	}
	else 
	{
		Serial.println();		Serial.print("--== Temperature Sensor OK!");
		pDataMS->SetTempSensorError(0);	// set status bit
		fTWater = pTemperatureSensor->GetTemperature();		
	}
	pDataMS->SetTemprWater(fTWater);
	

	pDataMS->SetPowerU(nU);
}

///////////////////////////////////////////////////////////////
// Working loop of DFM-MS
///////////////////////////////////////////////////////////////
void loop()
{
	
	// Read and execute command from DFM-CP BT serial port
	::BTSerialReadCmnd();

	// Read and execute command from serial port console
	::SerialUI();

	// Write data into BT serial port
	noInterrupts();
	pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);
	interrupts();

	// Calculate current flow Q
	pDataMS->SetQ(pEMFM->CalculateQ());

/*	// Change data
	if (lCount % 100 == 0) bStatus = ~bStatus;
	fTAir += 0.01;
	fTWater += 0.01;
	fRHumidityAir += 0.01;
	if (lCount % 200 == 0) nU--;

	// Fill data
	pDataMS->SetStatus(bStatus);
	pDataMS->SetTemprWater(fTWater);
	pDataMS->SetTemprAir(fTAir);
	pDataMS->SetRHumidityAir(fRHumidityAir);
	pDataMS->SetPowerU(nU);
//	pDataMS->SetTimeInt(lTimeInt);
	
*/

	// Debug print to serial port console
	if (lCount % 10 == 0 && isSerialPrn)
		//	if (dwTimerTick % 10 == 0 && isSerialPrn)
	{
		// Print number of loop
		Serial.println();		Serial.print(""); Serial.print(lCount);
		//		Serial.print("\tTT=");	Serial.print(dwTimerTick);
		Serial.print("\tCF=");	Serial.print(pEMFM->GetCountFull(), 10);
		Serial.print("\tCC=");	Serial.print(pEMFM->GetCountCurr(), 10);
		//		Serial.print("\tCB=");	Serial.print(dwCountBadPulse);
		Serial.print("\tQ=");	Serial.print(pEMFM->GetQCurr(), 3);
		Serial.print("\tTW=");	Serial.print(pDataMS->GetTemprWater(), 2);
		Serial.print("\tTA=");	Serial.print(pDataMS->GetTemprAir(), 2);
		Serial.print("\tRH=");	Serial.print(pDataMS->GetRHumidityAir(), 2);
		Serial.print("\tU=");	Serial.print(pDataMS->GetPowerU());
		//		isSerialPrn = !isSerialPrn;
	}
//	else
		//		if (dwTimerTick % 10 != 0)
		//		if (lCount % 10 != 0)
		//			if (!isSerialPrn) isSerialPrn = !isSerialPrn;

	// Counter of loops
	lCount++;
	
	// Delay for other tasks
	delay(DELAY_LOOP_MS);
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

		// Read Timer and save it in data packet
		pDataMS->SetTimeInt(pEMFM->GetTimer());

		// If current counter == 0
		if (pEMFM->GetCountCurr() == 0) {
			isMeasuring = false;	// set flag of measuring OFF
			pEMFM->StopTimer();		// stop Timer
		}
	}

	// Calculate current flow Q
//	pDataMS->SetQ(pEMFM->CalculateQ());
}

///////////////////////////////////////////////////////////////
// Read command from FMVI-CP
///////////////////////////////////////////////////////////////
void BTSerialReadCmnd()
{
	DWORD	dwCountCurr;
	int		nErrCode;

	// Read command from FMVI-CP
	//Serial.println();	Serial.print("Read:");
	if ((nErrCode = pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1)) > 0) {
		// Analize input command
		Serial.println();
		Serial.print(" Cmnd=");	Serial.print(pCmndMS->GetCode());
		Serial.print("\tArg=");	Serial.print(pCmndMS->GetArg_dw());

		switch (pCmndMS->GetCode()) 
		{
			case cmndSetCount:			// Set current pulse count 
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
				
				Serial.print("\tPASSED");
				
				break;

			case cmndReadTemprWater:		// Read water temperature from sensor
				if (!isMeasuring) {
					// Read temperature and save in data packet
					pDataMS->SetTemprWater(pTemperatureSensor->GetTemperature());
					Serial.print("\tPASSED");
				}
				else Serial.print("\tSKIP");

				break;

			case cmndReadRHT:		// Read humidity and temperature from RHT sensor
				if (!isMeasuring) {
					// Read humidity and temperature from RHT sensor and save in data packet
					pRHTSensor->GetRHT(fRHumidityAir, fTAir);	// Get RH and temperature from sensor
					pDataMS->SetTemprAir(fTAir);
					pDataMS->SetRHumidityAir(fRHumidityAir);
					Serial.print("\tPASSED");
				}
				else Serial.print("\tSKIP");

				break;

			case cmndPowerOff:			// Turn off power

				break;

		}

	}
	else if (nErrCode != -1) {
		Serial.print(" Err=");	Serial.print(nErrCode);
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
