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
//		BTSerialReadCmnd()		-	Read command from DFM-CP by BT Serial Port
//		SerialUI()				-	User interface by srial port (hardware or BT)
/////////////////////////////////////////////////////////////////////////////////

#include "CommDef.h"
#include "CPowerDC.h"
#include "CSerialPort.h"
#include "CDataMS.h"
#include "CEMFM.h"
#include "CRHTSensor.h"
#include "CTemperatureSensor.h"
#include "CGPS.h"
#include "CLED.h"
#include "TimerOne.h"


// ---===--- Global constants ---===---
// Application name & Version
const CHAR strAppName[]			= "DFM-MS ";
const CHAR strVerMajMin[]		= "1.1 ";		// Major, minor versions
const CHAR strVerStatusBuild[]	= "0.77 ";		// Status, build

// Arduino analog GPIO
const INT   POWER_INPUT_PIN	= 0;		// Power analog input pin

// Arduino digital GPIO
const INT   EXT_LED_PIN		= 2;		// External LED ootput pin
const INT	EMFM_PIN		= 3;		// EMFM digital out input pin
const INT   POWER_ON_OFF_PIN= 4;		// Power ON/OFF output pin
const INT   TEMP_PIN		= 5;		// Temperature sensor DS18B20 DQ out input pin
const INT   ALM_FQH_PIN		= 6;		// EEMFM FQH ALARM out input pin
const INT   ALM_FQL_PIN		= 7;		// EEMFM FQL ALARM out input pin
const INT	DHTxx_PIN		= 8;		// RHT sensor DHTxx input pin 
const INT	RX_PIN_BT		= 10;		// Software UART RX pin, connect to TX of Bluetooth HC-0x 
const INT	TX_PIN_BT		= 11;		// Software UART TX pin, connect to RX of Bluetooth HC-0x
const INT	RX_PIN_GPS		= 12;		// Software UART RX pin, connect to TX of serial GPS module 
const INT	TX_PIN_GPS		= 13;		// Software UART TX pin, connect to RX of serial GPS module
const INT   INT_LED_PIN		= 13;		// Internal LED output pin

// Serial ports parameters
const LONG  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const LONG  DR_BT_COM		= 38400;	// Data rate for software COM of Bluetooth HC-0x, bps
const LONG	BT_READ_TIMEOUT = 100;		// Timeout for serial port Bluetooth data read, millis
const LONG  DR_GPS_COM		= 9600;		// Data rate for software COM of serial GPS module, bps
const LONG	GPS_READ_TIMEOUT= 1000;		// Timeout waiting GPS data

// Metric parameters
const DWORD	PULSE_UNIT_LTR	= 1000;		// Quantity pulse in 1 ltr
const INT	TEMPERATURE_PRECISION = 9;	// Temperature sensor resolution: 9-12, 9 - low 

const INT	STATUS_BIT_OK	= 0;		// Status bit: no error state
const INT	STATUS_BIT_ERR	= 1;		// Status bit: error state

// ---===--- Global variables ---===---
DWORD	lT1Period			= 10000;	// Timer1 period, microsec
BOOL	state				= false;	// State for calculation time interval 
volatile DWORD	lT1Counter	= 0;		// Timer 1 tick counter
DWORD	lT1CountStart, lT1CountStop;	// For save timer's ticks

DWORD	lLoopMSPeriod		= 200;		// DFM-MS main loop period, millis
DWORD	lDebugPrnPeriod		= 2000;		// Debug print period
DWORD	lGetSensorsPeriod	= 60000;	// Get sensors of T, RHT Air period
DWORD	lInt4CalcQ			= 1000;		// Interval for calculate instant flow Q, millis
INT		nDelayAfterPowerON	= 2000;		// Wait for Power ON, millis

DWORD	dwCountBadPulse		= 0;		// Counter of bad input pulse packet (pulse front < nPulseWidth)
DWORD	dwCountReceiveErr	= 0;		// Counter of bad received packet

INT		nQMA_Points			= 3;		// Number of points for calculate moving average of instant flow Q
INT		nPulseWidth			= 50;		// Width in millisec of EMFM output pulse
INT		nALM_FQHWidth		= 500;		// Width in millisec of EMFM ALARM FQH signal
INT		nALM_FQLWidth		= 500;		// Width in millisec of EMFM ALARM FQL signal
INT		nPULSE_INT_MODE		= FALLING;	// Mode of interrupt of EMFM pulse out: LOW, CHANGE, RISING, FALLING
void	(*pISR)();						// Pointer to ISR callback function of EMFM pulse out 
BOOL	isAntiTinklingOn	= false;	// Antitinkling flag for EMFM output pulse: true - ON, false - OFF

INT		nTypeRHTSensor		= snsrDHT21;// Type of RHT DHT sensor: DHT21, DHT22, DHT11

BOOL	isMeasuring			= false;	// Measuring state flag: true - Test is ON, false - Test is OFF

INT		nLEDStateInit		= LOW;		// Init LEDs state

BYTE	pBuff[DATA_LEN+1];				// Buffer for save sending data

FLOAT	fTick_ms,				// Millisecs in timer tick
fTAir,					// Air Temperature
fRHumidityAir,			// Air Relative Humidity
fTWater,				// Water Temperature
fLatitude = 0.0,		// 48.476017,	// GPS Latitude
fLongitude = 0.0,		// 35.014864,	// GPS Longitude
fQ = 0.0;				// Current Volume Flow of Water

DWORD	lCount = 0, lCurrCount, lTime;

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

// --==-- GPS module
CGPS *pGPS;

// --==-- Power DC Control object
CPowerDC			*pPowerDC;

// --==-- LED objects
CLED				*pInt_LED;	// Internal LED
CLED				*pExt_LED;	// External LED

///////////////////////////////////////////////////////////////
// Initialisation DFM-MS
///////////////////////////////////////////////////////////////
void setup() 
{
	INT rc;
		// --==-- Arduino Timer 1//	Timer1.initialize(lT1Period);			// initialize Timer 1, and set period//	Timer1.attachInterrupt(Timer1Callback);	// attaches Timer 1 overflow ISR function and enable a timer overflow interrupt
	fTick_ms = lT1Period / 1000.0f;			// ticks in ms

	// --==-- Hardware serial port
	// Set the data rate and open
	Serial.begin(DR_HARDWARE_COM);
	DBG_PRN_LOGO(strAppName, strVerMajMin);	DBG_PRNL(F(": SETUP Starting ..."));
	
	// --==-- Power DC Control object
	// Create object & initialization
	DBG_PRN_LOGO(strAppName, strVerMajMin);	DBG_PRN(F(": PowerDC ON: "));
	pPowerDC = new CPowerDC(POWER_INPUT_PIN, POWER_ON_OFF_PIN, nDelayAfterPowerON);
	// Power ON
	pPowerDC->PowerON();
	DBG_PRN(F("\t PASSED"));

	// --==-- Bluetooth serial port
	// Create object & initialization
	pBTSerialPort = new CSerialPort(1, RX_PIN_BT, TX_PIN_BT);
	pBTSerialPort->Init(DR_BT_COM, 0);
	pBTSerialPort->SetReadTimeout(BT_READ_TIMEOUT);

	// --==-- Send data packet object
	// Create object
	pDataMS = new CDataMS;
	// Clear status byte
	pDataMS->SetStatus(STATUS_BIT_OK);
	// Set Power DC value
	pDataMS->SetPowerU(pPowerDC->GetPowerDC());

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
	DBG_PRN_LOGO(strAppName, strVerMajMin);	DBG_PRN(F(": RHT sensor: "));
	pRHTSensor = new CRHTSensor(DHTxx_PIN, nTypeRHTSensor);
	// Get RH and temperature from sensor
	if (!(rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir)))
	{
		DBG_PRN(F("OK"));
		DBG_PRN(F("\t Model: "));		DBG_PRN(pRHTSensor->GetSensorModel());
		DBG_PRN(F("\t TempAir: "));		DBG_PRN(fTAir, 1);
		DBG_PRN(F("\t Humidity: "));	DBG_PRNL(fRHumidityAir, 1);
	}
	else {
		DBG_PRN(F("NOT DETECTED! \t Model: ")); DBG_PRN(pRHTSensor->GetSensorModel());
	}	

	pDataMS->SetRHTSensorError( !rc ? STATUS_BIT_OK : STATUS_BIT_ERR);	// set status bit RHT sensor
	pDataMS->SetTemprAir(fTAir);										// set temperature of air
	pDataMS->SetRHumidityAir(fRHumidityAir);							// set humidity

	// --==-- Temperature of water sensor
	DBG_PRN_LOGO(strAppName, strVerMajMin);	DBG_PRN(F(": Temperature of water sensor: "));
	pTemperatureSensor = new CTemperatureSensor(TEMP_PIN, TEMPERATURE_PRECISION);
	// Waiting...
	delay(2000);
	// Get temperature from sensor
	if ( !(rc = pTemperatureSensor->GetTemperature(fTWater)) ) {
		DBG_PRN(F("OK"));
		DBG_PRN(F("\t NumOfDev:"));		DBG_PRN(pTemperatureSensor->GetNumberOfDevices());
		DBG_PRN(F("\t Model: "));		DBG_PRN(DS_MODEL_NAME[pTemperatureSensor->GetTypeSensor()]);
		DBG_PRN(F("\t PPawer: "));		DBG_PRN(pTemperatureSensor->isParasitePower());
		DBG_PRN(F("\t TempWater: "));	DBG_PRNL(fTWater, 1);
	}
	else	DBG_PRNL(DS_MODEL_NAME[0]);
	
	pDataMS->SetTempSensorError( !rc ? STATUS_BIT_OK : STATUS_BIT_ERR);	// set status bit
	pDataMS->SetTemprWater(fTWater);									// set water temperature

	// --==-- GPS module
	DBG_PRN_LOGO(strAppName, strVerMajMin);	DBG_PRN(F(": GPS module: "));
	pGPS = new CGPS(RX_PIN_GPS, TX_PIN_GPS, DR_GPS_COM, GPS_READ_TIMEOUT);
	// Waiting...
	delay(2000);
	// Get position from GPS
	if (!(rc = pGPS->GetGPS_Position(fLatitude, fLongitude))) {
		DBG_PRN(F("OK"));
		DBG_PRN(F("\t LAT: "));		DBG_PRN(fLatitude);
		DBG_PRN(F("\t LON: "));		DBG_PRN(fLongitude);
	}
	else	DBG_PRN(F("NOT DETECTED!"));

	pDataMS->SetGPSError (!rc ? STATUS_BIT_OK : STATUS_BIT_ERR);	// set status bit of GPS module
	pDataMS->SetGPS_LAT(fLatitude);									// set GPS Latitude
	pDataMS->SetGPS_LON(fLongitude);								// set GPS Longitude

	// --==-- LEDs 
	pInt_LED = new CLED(INT_LED_PIN, nLEDStateInit);
	pExt_LED = new CLED(EXT_LED_PIN, nLEDStateInit);

	// --==-- Delay before starting main loop
	DBG_PRN_LOGO(strAppName, strVerMajMin);	DBG_PRNL(F(": MAIN Starting ..."));
}

///////////////////////////////////////////////////////////////
// Working loop of DFM-MS
///////////////////////////////////////////////////////////////
void loop()
{
	DWORD lTimeBegin, lTimeInt;
	
	// Save time of begin of loop
	lTimeBegin = millis();

	// Counter of loops
	lCount++;

#ifdef _DEBUG_TRACE	
	// Read and execute command from serial console (hardware COM)
	::SerialReadCmnd();

	// Get data from sensors T, RHT of air
	if (lCount % (lGetSensorsPeriod / lLoopMSPeriod) == 0 && !isMeasuring)
	{
		// Read humidity and temperature from RHT sensor and save in data packet
		INT rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir);
		pDataMS->SetRHTSensorError(!rc ? STATUS_BIT_OK : STATUS_BIT_ERR);			// set status bit of sensor
		pDataMS->SetTemprAir(fTAir);	pDataMS->SetRHumidityAir(fRHumidityAir);

		// Read water temperature and save in data packet
		pDataMS->SetTempSensorError(!pTemperatureSensor->GetTemperature(fTWater) ? STATUS_BIT_OK : STATUS_BIT_ERR);
		pDataMS->SetTemprWater(fTWater);
	}

#endif	

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
	
	// Set moving average of Q into data packet
	pDataMS->SetQ(pEMFM->GetQMA());
		
	// Blink LED, Get Power, Debug print to serial port console
	if (lCount % (lDebugPrnPeriod / lLoopMSPeriod) == 0)
	{
		// Blink internal LED 
		pInt_LED->Blink();
		
		// Get Power DC value and save in data packet
		if (!isMeasuring) pDataMS->SetPowerU(pPowerDC->GetPowerDC());
		
		// Get GPS data
		if (!pGPS->isNewData() && !isMeasuring)
		{
			INT rc = pGPS->GetGPS_Position(fLatitude, fLongitude);
			pDataMS->SetGPSError(!rc ? STATUS_BIT_OK : STATUS_BIT_ERR);	// set status bit of GPS module
			pDataMS->SetGPS_LAT(fLatitude);								// set GPS Latitude
			pDataMS->SetGPS_LON(fLongitude);							// set GPS Longitude
		}

		// Debug Print data
		DBG_PRN_LOGO(strAppName, strVerMajMin);	DBG_PRN(lCount);
		DBG_PRN(F("\tTime="));					DBG_PRN(lTimeBegin, 10);
		DBG_PRN(F("\tSt=0x"));					DBG_PRN(pDataMS->GetStatus(), HEX);
		DBG_PRN(F("\tCF="));					DBG_PRN(pEMFM->GetCountFull(), 10);
		DBG_PRN(F("\tCC="));					DBG_PRN(pEMFM->GetCountCurr(), 10);

		DBG_PRN(F("\tTW="));					DBG_PRN(pDataMS->GetTemprWater(), 1);
		DBG_PRN(F("\tTA="));					DBG_PRN(pDataMS->GetTemprAir(), 1);
		DBG_PRN(F("\tRH="));					DBG_PRN(pDataMS->GetRHumidityAir(), 1);

		DBG_PRN(F("\tLAT="));					DBG_PRN(pDataMS->GetGPS_LAT(), 6);
		DBG_PRN(F("\tLON="));					DBG_PRN(pDataMS->GetGPS_LON(), 6);

		DBG_PRN(F("\tQ="));						DBG_PRN(pEMFM->GetQCurr(), 3);
		DBG_PRN(F("\tQMA="));					DBG_PRN(pEMFM->GetQMA(), 3);
		
		DBG_PRN(F("\tU="));						DBG_PRN(pDataMS->GetPowerU());
		
		DBG_PRN(F("\tRecErr="));				DBG_PRN(dwCountReceiveErr, 10);
	}

	// Calculate time interval from start loop
	lTimeInt = millis() - lTimeBegin;

	// Delay for end of loop period 
	if(lLoopMSPeriod > lTimeInt) delay(lLoopMSPeriod - lTimeInt);
}
///////////////////////////////////////////////////////////////
// Timer 1 overflow ISR function
///////////////////////////////////////////////////////////////
void Timer1Callback() { ++lT1Counter; }
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
/*void ISR_ExtButtonPressAntON()
{
	// Button press: check time between previous button press 
	if (pMSExtButton->isPress(millis())) // button press correct
	{
		// Blink LED 
		pInt_LED->Blink();

		// Reverse star-stop bit in status byte of DFM-MS
//		pDataMS->SetStartStopExt(!pDataMS->GetStartStopExt() ? 1 : 0);
		
		// Save new time of button press
		pMSExtButton->SetTStartPress(millis());
	}
}
*/
///////////////////////////////////////////////////////////////
// Read command from DFM-CP
///////////////////////////////////////////////////////////////
void BTSerialReadCmnd()
{
	DWORD	dwCountCurr;
	INT		nErrCode, rc;

	// Read command from DFM-CP
	if ((nErrCode = pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1)) > 0) {
		DBG_PRN_LOGO(strAppName, strVerMajMin);
		DBG_PRN(F("--->>> BTSerial CMND: "));	DBG_PRN(pCmndMS->GetCode());
		
		// Set ReceiveError status bit to no error
		pDataMS->SetReceiveError(STATUS_BIT_OK);

		// Analize input command
		switch (pCmndMS->GetCode()) 
		{
			// 67 Set current pulse count
			case cmndSetCount: 
				DBG_PRN(F("\t(SetCount) : ")); DBG_PRN(pCmndMS->GetArg_dw());
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
				DBG_PRN(F("\t PASSED"));
				break;

			// 72 Read humidity and temperature from RHT sensor
			case cmndReadRHT:
				DBG_PRN(F("\t(ReadRHT)"));
				if (!isMeasuring) {
					// Read humidity and temperature from RHT sensor and save in data packet
					rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir);
					pDataMS->SetRHTSensorError(!rc ? STATUS_BIT_OK : STATUS_BIT_ERR);			// set status bit of sensor
					pDataMS->SetTemprAir(fTAir);	pDataMS->SetRHumidityAir(fRHumidityAir);
					DBG_PRN(F("\t TAir= "));	DBG_PRN(fTAir, 1);
					DBG_PRN(F("\t RH= "));		DBG_PRN(fRHumidityAir, 1);
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;

			// 84 Read water temperature from sensor
			case cmndReadTemprWater:
				DBG_PRN(F("\t(ReadTemprWater)"));
				if (!isMeasuring) {
					// Read temperature and save in data packet
					pDataMS->SetTempSensorError(!pTemperatureSensor->GetTemperature(fTWater) ? STATUS_BIT_OK : STATUS_BIT_ERR);
					pDataMS->SetTemprWater(fTWater);
					DBG_PRN(F("\t TWater= ")); DBG_PRN(fTWater, 1); DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;


			// 80 Turn off power
			case cmndPowerOff:
				DBG_PRN(F("\t(PowerOff)"));
				// Power OFF
				pPowerDC->PowerOFF();
				DBG_PRN(F("\t PASSED"));
				break;

			// 82 Test Receive
			case cmndTestReceive:
				DBG_PRN(F("\t(TestReceive)"));
				// Do nothing...
				DBG_PRN(F("\t PASSED"));
				break;


			// 160 Set DFM-MS main loop period, millis. Default = 200.
			case cmndSetLoopMSPeriod:
				DBG_PRN(F("\t(SetLoopMSPeriod) : ")); DBG_PRN(pCmndMS->GetArg_dw());
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					lLoopMSPeriod = pCmndMS->GetArg_dw();
					interrupts();
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;

			// 161 Set number of points for calculate moving average of flow Q. Default = 10.
			case cmndSetQMA_Points:
				DBG_PRN(F("\t(SetQMA_Points) : ")); DBG_PRN(pCmndMS->GetArg_n());
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					pEMFM->SetQMA_Points(nQMA_Points = pCmndMS->GetArg_n());
					interrupts();
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;

			// 162 Set interval for calculate instant flow Q, millis. Default = 500.
			case cmndSetInt4CalcQ:
				DBG_PRN(F("\t(SetInt4CalcQ) : ")); DBG_PRN(pCmndMS->GetArg_dw());
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					pEMFM->SetInt4CalcQ(lInt4CalcQ = pCmndMS->GetArg_dw());
					interrupts();
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;

			// 164 Set antitinkling flag for EMFM output pulse: 1 - ON, 0 - OFF. Default = 0.
			case cmndSetAntiTinklingOn:
				DBG_PRN(F("\t(SetAntiTinklingOn) : ")); DBG_PRN(pCmndMS->GetArg_b());
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
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;

			// 165 Set Width in millisec of EMFM output pulse. Default = 50.
			case cmndSetPulseWidth:
				DBG_PRN(F("\t(SetPulseWidth) : ")); DBG_PRN(pCmndMS->GetArg_n());
				if (!isMeasuring) {
					// Change parameter
					noInterrupts();
					pEMFM->SetPulseWidth(nPulseWidth = pCmndMS->GetArg_n());
					interrupts();
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;
		}
	}
	else if (nErrCode != -1) {
		DBG_PRN_LOGO(strAppName, strVerMajMin); 
		DBG_PRN(F("======>>> BTSerial ERROR: "));	DBG_PRN(nErrCode);	
		DBG_PRN(F("\t DATA: "));					DBG_PRN((pCmndMS->GetData())[0]);
		
		if ( (pCmndMS->GetData())[0] <= CMND_LEN && (pCmndMS->GetData())[0] >= 0 )
			for (int j = 1; j <= CMND_LEN; ++j) { DBG_PRN(F(" 0x")); DBG_PRN((pCmndMS->GetData())[j], HEX);	}
		
		DBG_PRN(F("\t SKIPED"));
		
		dwCountReceiveErr++;

		// Set ReceiveError status bit to 1
		pDataMS->SetReceiveError(STATUS_BIT_ERR);
	}
}

#ifdef _DEBUG_TRACE
///////////////////////////////////////////////////////////////
// Read command from serial console (hardware COM)
///////////////////////////////////////////////////////////////
void SerialReadCmnd() 
{
	INT	nInpByte;

	// Ожидание команды из последовательного порта (байт)
	if ((nInpByte = Serial.read()) > 0)
	{
		DBG_PRN_LOGO(strAppName, strVerMajMin);
		DBG_PRN(F("--->>> Serial PRESSED: "));
		DBG_PRN(nInpByte);

		// анализируем принятый байт
		switch (nInpByte)
		{
		case 32:	// SPACE - start/stop calculate elapsed time
			if (!state) // state false - starting counting ticks
			{
				noInterrupts();
				lT1CountStart = lT1Counter;	// save start time
				interrupts();
				lCurrCount = 0;	pEMFM->SetCountFull(0);
			}
			else		// state true - stop counting
			{
				noInterrupts();
				lT1CountStop = lT1Counter;	// save current time
				interrupts();
				lCurrCount = pEMFM->GetCountFull();
				
				DBG_PRN(F("\t COUNT = "));				DBG_PRN(lCurrCount, 10);
				DBG_PRN(F("\t Elapsed time, ms = "));	DBG_PRN((DWORD)((lT1CountStop - lT1CountStart)*fTick_ms), 10);
			}
			state = !state;					// change state			break;
		}
	}

/*	// Ожидание команды из последовательного порта
	if (Serial.available() > 0)
	{
		DBG_PRN_LOGO(strAppName, strVerMajMin);
		DBG_PRN(F("--->>> Serial PRESSED: "));
		
		// считываем байт данных
		nInpByte = Serial.read();
		DBG_PRN(nInpByte);

		// анализируем команду
		if (nInpByte == 32) // SPACE
		{
			lCurrCount = pEMFM->GetCountFull();
			DBG_PRN(F("\t COUNT = "));	DBG_PRN(lCurrCount, 10);
			DBG_PRN(F("\t TIME = "));	DBG_PRNL(millis() - lTime);
			
			lCurrCount = 0;	pEMFM->SetCountFull(0);
			lTime = millis();
		}
	}
*/
}
#endif 