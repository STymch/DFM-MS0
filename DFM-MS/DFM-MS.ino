/////////////////////////////////////////////////////////////////////////////////
// ---Working Measurement Standard - Digital Flowmeter (DFM)  ---
/////////////////////////////////////////////////////////////////////////////////
// --- Digital Flowmeter: Measuring System (DFM-MS)
/////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 by Sergiy Tymchenko from BOST Labs
// All rights reserved.
// DNIPRO, UKRAINE
// Tel/Fax: +380 56 791 6040 / +380 56 791 6066
// Mob/Viber/WhatsApp/Telegram: +380 67 636 1855
// Skype: stymch2008
// E-mail : stymch@gmail.com
/////////////////////////////////////////////////////////////////////////////////
// Filename: DFM-MS
// Content:
//  - declare global variables & const
//  - functions:
//		setup()					-	Initialisation DFM-MS
//		loop()					-	Working loop of DFM-MS
//		ISR_FlowmeterPulseOut()	-	ISR callback function for pulse out of EMFM
//		BTSerialReadCmnd()		-	Read command from DFM-CP by BT Serial Port
//		EEPROM_RadData()		-	Read data from EEPROM
//		SerialReadCmnd()		-	Read command from serial console (hardware COM)
//		SerialPrintData()		-	Print debug information into serial console (hardware COM)
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
#include "CDataEEPROM.h"


// ---===--- Global constants ---===---
// Arduino analog GPIO
const INT   POWER_INPUT_PIN = 0;			// Power analog input pin

// Arduino digital GPIO
const INT   EXT_LED_PIN		= 2;			// External LED ootput pin
const INT	EMFM_PIN		= 3;			// EMFM digital out input pin
const INT   POWER_ON_OFF_PIN= 4;			// Power ON/OFF output pin
const INT   TEMP_PIN		= 5;			// Temperature sensor DS18B20 DQ out input pin
const INT   ALM_FQH_PIN		= 6;			// EEMFM FQH ALARM out input pin
const INT   ALM_FQL_PIN		= 7;			// EEMFM FQL ALARM out input pin
const INT	DHTxx_PIN		= 8;			// RHT sensor DHTxx input pin 
const INT	RX_PIN_BT		= SS;			// Software UART RX pin, connect to TX of Bluetooth HC-0x 
const INT	TX_PIN_BT		= MOSI;			// Software UART TX pin, connect to RX of Bluetooth HC-0x
const INT	RX_PIN_GPS		= MISO;			// Software UART RX pin, connect to TX of serial GPS module 
const INT	TX_PIN_GPS		= LED_BUILTIN;	// Software UART TX pin, connect to RX of serial GPS module
const INT   INT_LED_PIN		= LED_BUILTIN;	// Internal LED output pin

// Serial ports parameters
const LONG  DR_HARDWARE_COM = 38400;// Data rate for hardware COM, bps
const LONG  DR_BT_COM		= 38400;// Data rate for software COM of Bluetooth HC-0x, bps
const LONG	BT_READ_TIMEOUT = 100;	// Timeout for serial port Bluetooth data read, millis
const LONG  DR_GPS_COM		= 9600;	// Data rate for software COM of serial GPS module, bps

// Metric constants
const INT	TEMP_SENSOR_RES = 9;	// Temperature water sensor resolution: 9-12, 9 - low 
const INT	P_FACTOR		= 1000;	// EMFM pulse factor - pulses per 1 ltr

// Status bits state
const INT	ST_BIT_OK			= 0;// Status bit: no error state
const INT	ST_BIT_ERR			= 1;// Status bit: error state
const INT	ST_BIT_TEST_RUN		= 1;// Status bit test run: test is running
const INT	ST_BIT_TEST_NOT_RUN	= 0;// Status bit test run: test is not running

// ---===--- Global variables ---===---
// Application name
CHAR	strAppName[] = "DFM-MS ";
// Application version information
UINT	nVerMaj		= 1;	// Major version number
UINT	nVerMin		= 0;	// Minor version number
UINT	nVerStatus	= 3;	// Status number: 0 - alfa, 1 - beta, 2 - RC, 3 - RTM
UINT	nVerBuild	= 81;	// Build number, from SVC system
CHAR	strVer[30]	= "";	// Application version full information

// Counters of EMFM pulses
volatile DWORD	dwCounterPulseInc;	// Increment counter for output pulses from EMFM/Generator
volatile DWORD	dwCounterPulseDec;	// Decrement counter for output pulses from EMFM/Generator

DWORD	dwPulseFactor = P_FACTOR;	// EMFM pulse factor, pulses in 1 ltr

// Times parameters
DWORD	lLoopMSPeriod		= 200;	// DFM-MS main loop period, millis
DWORD	lDebugPrnPeriod		= 2000;	// Debug print period
DWORD	lGetSensorsPeriod	= 30000;// Get sensors of T, RHT Air period
DWORD	lInt4CalcQ			= 1000;	// Interval for calculate instant flow Q, millis
DWORD	lCoordUpdPeriod		= 1000;	// Delay between coordinate updates (GPS)
INT		nDelayAfterPowerON	= 2000;	// Wait for Power ON, millis
INT		nALM_FQHWidth		= 500;	// Width in millisec of EMFM ALARM FQH signal
INT		nALM_FQLWidth		= 500;	// Width in millisec of EMFM ALARM FQL signal

// ISR parameters
INT		nPULSE_INT_MODE = FALLING;	// Mode of interrupt of EMFM pulse out: LOW, CHANGE, RISING, FALLING
void	(*pISR)();					// Pointer to ISR callback function of EMFM pulse out 

// Sensors parameters
INT		nTypeRHTSensor = snsrDHT21;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11
INT		nStGPS = -1;				// State of GPS subsystem: -1 - module not present; 1 - module present, no location data; 0 - success, present location data

// Flags
volatile BOOL	isTestRun = false;	// Test state flag: true - test is running, false - test not running now
volatile BOOL	isReqData = false;	// Request for send data flag: true - request received, no request

// For receive packets
BYTE	pBuff[DATA_LEN + 1];		// Buffer for save sending data
DWORD	dwCountReceiveErr = 0;		// Counter of bad received packet

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
CGPS				*pGPS;

// --==-- Power DC Control object
CPowerDC			*pPowerDC;

// --==-- LED objects
CLED				*pInt_LED;	// Internal LED
CLED				*pExt_LED;	// External LED

// --==-- EEPROM object
CDataEEPROM *pCDataEEPROM;

///////////////////////////////////////////////////////////////
// Initialisation DFM-MS
///////////////////////////////////////////////////////////////
void setup()
{
	FLOAT	fTAir,					// Air Temperature
			fRHumidityAir,			// Air Relative Humidity
			fTWater,				// Water Temperature
			fLatitude = -1.0,		// GPS Latitude
			fLongitude = -1.0;		// GPS Longitude
	INT		nLEDStateInit = LOW;	// Init LEDs state
	INT		rc;						// return code
	CHAR	strTemp[6];				// temporary string	// --==-- Hardware serial port
	// Set the data rate and open
	Serial.begin(DR_HARDWARE_COM);

	// --==-- EEPROM object
	pCDataEEPROM = new CDataEEPROM;
	// Read data from EEPROM
	EEPROM_InitData(*pCDataEEPROM);
	rc = EEPROM_ReadData(*pCDataEEPROM);

	// --==-- Save application version info into string
	strcat(strVer, itoa(nVerMaj, strTemp, 10));	strcat(strVer, ".");
	strcat(strVer, itoa(nVerMin, strTemp, 10));	strcat(strVer, ".");
	if (nVerStatus == 0) strcat(strVer, "a");
	else	if (nVerStatus == 1) strcat(strVer, "b");
			else	if (nVerStatus == 2) strcat(strVer, "RC");
					else strcat(strVer, "RTM");
	strcat(strVer, ".");
	strcat(strVer, itoa(nVerBuild, strTemp, 10)); strcat(strVer, " ");
	STP_PRN_LOGO(strAppName, strVer);	STP_PRNL(F(": SETUP STARTING ----->>>>>"));

	// Print data was read from EEPROM
	STP_PRN_LOGO(strAppName, strVer);
	if (!rc) { STP_PRNL(F(": DATA WAS READ FROM EEPROM:")); }
	else { STP_PRNL(F(": EEPROM EMPTY, DATA INITIALIZATION:")); }
	EEPROM_PrintData(*pCDataEEPROM);

	// --==-- Power DC Control object
	// Create object & initialization
	STP_PRN_LOGO(strAppName, strVer);	STP_PRN(F(": SUBSYSTEM -> POWER DC:"));
	pPowerDC = new CPowerDC(POWER_INPUT_PIN, POWER_ON_OFF_PIN, nDelayAfterPowerON);
	// Power ON
	pPowerDC->PowerON();
	STP_PRN(F("\t\t PASSED (ON)"));

	// --==-- Bluetooth serial port
	// Create object & initialization
	pBTSerialPort = new CSerialPort(1, RX_PIN_BT, TX_PIN_BT);
	pBTSerialPort->Init(DR_BT_COM, 0);
	pBTSerialPort->SetReadTimeout(BT_READ_TIMEOUT);

	// --==-- Send data packet object
	// Create object
	pDataMS = new CDataMS;

	// Clear status byte
	pDataMS->Set_bStatus(ST_BIT_OK);
	// Set ReceiveError bit to error state
	pDataMS->Set_btReceiveError(ST_BIT_ERR);

	// Set Power DC value
	pDataMS->SetPowerU(pPowerDC->GetPowerDC());

	// --==-- Receive commands object
	// Create object
	pCmndMS = new CCmndMS;

	// --==-- LEDs 
	pInt_LED = new CLED(INT_LED_PIN, nLEDStateInit);
	pExt_LED = new CLED(EXT_LED_PIN, nLEDStateInit);

	// --==-- Flowmeter EMFM
	// Create object
	pEMFM = new CEMFM(EMFM_PIN, ALM_FQH_PIN, ALM_FQH_PIN, LOW, LOW, LOW, nALM_FQHWidth, nALM_FQLWidth);
	pISR = ISR_FlowmeterPulseOut; nPULSE_INT_MODE = FALLING;
	// Initialization, attache interrupt
	pEMFM->Init(0, DWORD(-1), dwPulseFactor, nPULSE_INT_MODE, pISR);

	pDataMS->SetCounterInc(pEMFM->GetCounterInc());
	pDataMS->SetCounterDec(pEMFM->GetCounterDec());
	pDataMS->SetTime(pEMFM->GetTime());
	pDataMS->SetQ(pEMFM->GetQ());

	// --==-- Humidity & temperature sensor
	STP_PRN_LOGO(strAppName, strVer);	STP_PRN(F(": SUBSYSTEM -> RHT AIR SENSOR:"));
	pRHTSensor = new CRHTSensor(DHTxx_PIN, nTypeRHTSensor);
	// Get RH and temperature from sensor
	if (!(rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir)))
	{
		STP_PRN(F("\t PASSED"));
		STP_PRN(F("\t\t MODEL: "));			STP_PRN(pRHTSensor->GetSensorModel());
		STP_PRN(F("\t TAir (C): "));		STP_PRN(fTAir, 1);
		STP_PRN(F("\t Humidity (%): "));	STP_PRN(fRHumidityAir, 1);
	}
	else {
		STP_PRN(F("\t\t NOT DETECTED! \t MODEL: ")); STP_PRN(pRHTSensor->GetSensorModel());
	}

	pDataMS->Set_btRHTSensorError(!rc ? ST_BIT_OK : ST_BIT_ERR);	// set status bit RHT sensor
	pDataMS->SetTemprAir(fTAir);									// set temperature of air
	pDataMS->SetRHumidityAir(fRHumidityAir);						// set humidity

	// --==-- Temperature of water sensor
	STP_PRN_LOGO(strAppName, strVer);	STP_PRN(F(": SUBSYSTEM -> T WATER SENSOR: "));
	pTemperatureSensor = new CTemperatureSensor(TEMP_PIN, TEMP_SENSOR_RES);
	// Waiting...
	delay(2000);
	// Get temperature from sensor
	if (!(rc = pTemperatureSensor->GetTemperature(fTWater))) {
		STP_PRN(F("\t PASSED"));
		STP_PRN(F("\t\t NUMBER OF DEV:"));	STP_PRN(pTemperatureSensor->GetNumberOfDevices());
		STP_PRN(F("\t MODEL: "));			STP_PRN(DS_MODEL_NAME[pTemperatureSensor->GetTypeSensor()]);
		STP_PRN(F("\t PPOWER: "));			STP_PRN(pTemperatureSensor->isParasitePower());
		STP_PRN(F("\t T (C): "));			STP_PRN(fTWater, 1);
	}
	else	STP_PRN(F("\t NOT DETECTED"));

	pDataMS->Set_btTempSensorError(!rc ? ST_BIT_OK : ST_BIT_ERR);	// set status bit
	pDataMS->SetTemprWater(fTWater);								// set water temperature

	// --==-- GPS subsystem
	STP_PRN_LOGO(strAppName, strVer);	STP_PRN(F(": SUBSYSTEM -> GPS: "));
	pGPS = new CGPS(RX_PIN_GPS, TX_PIN_GPS, DR_GPS_COM, lCoordUpdPeriod);
	// Waiting...
	delay(lCoordUpdPeriod);
	// Get position from GPS
	if ((nStGPS = pGPS->GetGPS_Position(fLatitude, fLongitude)) >= 0) {
		STP_PRN(F("\t\t\t PASSED"));
		STP_PRN(F("\t\t LAT: "));	STP_PRN(fLatitude);
		STP_PRN(F("\t LON: "));		STP_PRNL(fLongitude);
	}
	else { // GPS module not present
		fLatitude = fLongitude = -1.0;
		STP_PRNL(F("\t\t\t NOT DETECTED"));
	}

	pDataMS->Set_btGPSError(nStGPS >= 0 ? ST_BIT_OK : ST_BIT_ERR);	// set status bit of GPS module
	pDataMS->SetGPS_LAT(fLatitude);									// set GPS Latitude
	pDataMS->SetGPS_LON(fLongitude);								// set GPS Longitude
	
	// --==-- Starting main loop
	STP_PRN_LOGO(strAppName, strVer);	STP_PRNL(F(": LOOP STARTING ----->>>>>"));
}

///////////////////////////////////////////////////////////////
// Working loop of DFM-MS
///////////////////////////////////////////////////////////////
void loop() {
	// Loops counter
	static DWORD	lCounterLoops = 0;
	// Flag - was there a data request from DFM-CP
	static BOOL isReqDataGet = false;

	// For save copy of isReqData
	BOOL isReqDataCpy;
	// For save copy of increment counter
	DWORD dwCounterPulseIncCpy;
	// For save copy of decrement counter
	DWORD dwCounterPulseDecCpy;

	// Save time of begin of loop
	DWORD lTimeBegin = millis();

	// Increment counter of loops
	lCounterLoops++;

	// Read and execute command from DFM-CP BT serial port
	INT rc = ::BTSerialReadCmnd();

	// Check request data flag
	COPY_NOINT(isReqDataCpy, isReqData);
	if (isReqDataCpy) { // Set data for DataMS packet and write it into serial port
		// Clear request data flag before read counters!
		COPY_NOINT(isReqData, false);

		// Get Power DC value and save in data packet
		pDataMS->SetPowerU(pPowerDC->GetPowerDC());

		// Get counters values from ISR
		COPY_NOINT(dwCounterPulseIncCpy, dwCounterPulseInc);
		COPY_NOINT(dwCounterPulseDecCpy, dwCounterPulseDec);

		// Get current time
		DWORD lTimeCurr = millis();

		// Calculate flow Q (m3/h) and save it in data packet only if get cmndReqDataMS
		if (rc == cmndReqDataMS) pEMFM->CalculateQ(dwCounterPulseIncCpy, lTimeCurr);

		// Save counters and time in EMFM object
		pEMFM->SetCounterInc(dwCounterPulseIncCpy);
		pEMFM->SetCounterDec(dwCounterPulseDecCpy);
		pEMFM->SetTime(lTimeCurr);

		// Save counters, time and flow in data packet
		pDataMS->SetCounterInc(dwCounterPulseIncCpy);
		pDataMS->SetCounterDec(dwCounterPulseDecCpy);
		pDataMS->SetTime(lTimeCurr);
		pDataMS->SetQ(pEMFM->GetQ());

		// Write data into BT serial port
		pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);

		// Set flag - was there a data request from DFM-CP
		isReqDataGet = true;
	}

	// GPS subsystem: get location
	if (lCounterLoops % (lCoordUpdPeriod / lLoopMSPeriod) == 0 && !isReqDataGet && nStGPS != 0)
	{
		FLOAT fLatitude, fLongitude; // Latitude, longitude

		// Get location and set state of GPS subsystem
		nStGPS = pGPS->GetGPS_Position(fLatitude, fLongitude);

		// Get location, set into DataMS & set GPS status bit
		pDataMS->Set_btGPSError(nStGPS >= 0 ? ST_BIT_OK : ST_BIT_ERR);
		pDataMS->SetGPS_LAT(fLatitude);		// set GPS Latitude
		pDataMS->SetGPS_LON(fLongitude);	// set GPS Longitude
	}

	// LEDs control
	if (lCounterLoops % (lCoordUpdPeriod / lLoopMSPeriod) == 0) {
		INT factor;		// external LED blink factor
		
		// Check state of GPS module
		switch (nStGPS) {
			case -1: {
				factor = 3;	// no GPS module (3 sec period blink)
				break;
			}
			case 1: {
				factor = 2;	// GPS module present, no coordinates (2 sec period blink)
				break;
			}
			default: {
				factor = 1;	// coordinates present (1 sec period blink)
				break;
			}
		}
		// Blink external LED
		if (lCounterLoops % (factor * lCoordUpdPeriod / lLoopMSPeriod) == 0) {
			pInt_LED->Blink();
			pExt_LED->Blink();
		}
	}

	// Calculate time interval from start loop
	DWORD lTimeInt = millis() - lTimeBegin;

	// Delay for end of loop period 
	if (lLoopMSPeriod > lTimeInt) delay(lLoopMSPeriod - lTimeInt);

#ifdef _DEBUG_TRACE
	// Get data from sensors T, RHT of air, GPS
	if (lCounterLoops % (lGetSensorsPeriod / lLoopMSPeriod) == 0 && !isTestRun)
	{
		// Read RHT
		FLOAT	fTAir;			// Air Temperature
		FLOAT	fRHumidityAir;	// Air Relative Humidity
								// Get RHT, set status bit of sensor
		pDataMS->Set_btRHTSensorError(!pRHTSensor->GetRHT(fRHumidityAir, fTAir) ? ST_BIT_OK : ST_BIT_ERR);
		pDataMS->SetTemprAir(fTAir);	pDataMS->SetRHumidityAir(fRHumidityAir);

		// Read water temperature and save in data packet
		FLOAT fTWater;	// Water Temperature
						// Get water temperature, set status bit of sensor
		pDataMS->Set_btTempSensorError(!pTemperatureSensor->GetTemperature(fTWater) ? ST_BIT_OK : ST_BIT_ERR);
		pDataMS->SetTemprWater(fTWater);
	}

	// Read and execute command from serial console (hardware COM) 
	::SerialReadCmnd();

	// Additional print data packet into serial console, if get dwCounterSet command
	if (rc == cmndStStpTest) ::SerialPrintData(lCounterLoops);

	// The next print data packet into serial console, if no test running
//	BOOL	isTestRunCpy;
//	COPY_NOINT(isTestRunCpy, isTestRun);
//	if (lCounterLoops % (lDebugPrnPeriod / lLoopMSPeriod) == 0 && !isTestRunCpy) ::SerialPrintData(lCounterLoops);
	if (lCounterLoops % (lDebugPrnPeriod / lLoopMSPeriod) == 0) ::SerialPrintData(lCounterLoops);
#endif
}

///////////////////////////////////////////////////////////////
// ISR callback function for pulse out of EMFM
///////////////////////////////////////////////////////////////
void ISR_FlowmeterPulseOut()
{
	// Increment counter of all EMFM pulse
	++dwCounterPulseInc;

	// Decrement current counter while > 0
	if (dwCounterPulseDec > 0)
	{
		// Decrement counter
		--dwCounterPulseDec;

		// If current counter == 0
		if (dwCounterPulseDec == 0) {
			isTestRun = false;	// test completed
			isReqData = true;	// set request data flag
		}
	}
}

///////////////////////////////////////////////////////////////
// Read command from DFM-CP
///////////////////////////////////////////////////////////////
INT BTSerialReadCmnd()
{
	BOOL	isTestRunCpy;
	INT		rc, nErrCode;

	// Read command from DFM-CP
	if ((nErrCode = pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1)) > 0) {
		DBG_PRN_LOGO(strAppName, strVer);
		DBG_PRN(F("--->>> BTSerial CMND: "));	DBG_PRN(pCmndMS->GetCode());

		// Set ReceiveError status bit to no error
		pDataMS->Set_btReceiveError(ST_BIT_OK);

		// Retcode - command code
		rc = (pCmndMS->GetCode());

		// Analize code of command
		switch (rc)
		{
			// 0x43 (67) START / STOP test
			case cmndStStpTest: {
				DBG_PRN(F("\t(SetCount) : ")); DBG_PRN(pCmndMS->GetArg_dw());
				DWORD	dwCounterSet;
				// Argument == 0 - STOP test
				if ((dwCounterSet = pCmndMS->GetArg_dw()) == 0) // stop test
				{
					dwCounterSet = DWORD(-1);	// set max dword (-1) for current counter
					isTestRunCpy = false;		// clear flag TestRun
				}
				else	// Argument != 0 - START test
				{
					isTestRunCpy = true;		// set flag TestRun
				}

				// Set test run status bit
				pDataMS->Set_btTestRun(isTestRunCpy ? ST_BIT_TEST_RUN : ST_BIT_TEST_NOT_RUN);
				
				// Print info
				COPY_NOINT(dwCounterPulseDec, dwCounterSet);	// set new decrement counter
				COPY_NOINT(isTestRun, isTestRunCpy);			// set TestRun flag
				COPY_NOINT(isReqData, true);					// set ReqData flag

				DBG_PRNL(F("\t PASSED"));
				break;
			}

			// 0x48 (72) Read humidity and temperature from RHT sensor
			case cmndReadRHT: {
				DBG_PRN(F("\t(ReadRHT)"));
				// Check is now test runing
				COPY_NOINT(isTestRunCpy, isTestRun);	// save flag
				if (!isTestRunCpy) {
					FLOAT	fTAir;			// Air Temperature
					FLOAT	fRHumidityAir;	// Air Relative Humidity

					// Read humidity and temperature from RHT sensor and save in data packet
					pDataMS->Set_btRHTSensorError(!pRHTSensor->GetRHT(fRHumidityAir, fTAir) ? ST_BIT_OK : ST_BIT_ERR);
					pDataMS->SetTemprAir(fTAir);	pDataMS->SetRHumidityAir(fRHumidityAir);

					// Print info
					DBG_PRN(F("\t TAir= "));	DBG_PRN(fTAir, 1);
					DBG_PRN(F("\t RH= "));		DBG_PRN(fRHumidityAir, 1);
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRNL(F("\t SKIPED"));
				break;
			}

			// 0x54 (84) Read water temperature from sensor
			case cmndReadTemprWater: {
				DBG_PRN(F("\t(ReadTemprWater)"));
				// Check is now test running
				COPY_NOINT(isTestRunCpy, isTestRun);			// save flag
				if (!isTestRunCpy) {
					FLOAT	fTWater;	// Water Temperature

					// Read temperature and save in data packet
					pDataMS->Set_btTempSensorError(!pTemperatureSensor->GetTemperature(fTWater) ? ST_BIT_OK : ST_BIT_ERR);
					pDataMS->SetTemprWater(fTWater);
					
					// Print info
					DBG_PRN(F("\t TWater= ")); DBG_PRN(fTWater, 1);
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRNL(F("\t SKIPED"));
				break;
			}

			// 0x4C (76) Get location from GPS
			case cmndGetLocation: {
				DBG_PRN(F("\t(GetLocation)"));
				// Check is now test running
				COPY_NOINT(isTestRunCpy, isTestRun);	// save flag
				if (!isTestRunCpy) {
					FLOAT fLatitude, fLongitude;		// Latitude, longitude

					// Get location and set state of GPS subsystem
					nStGPS = pGPS->GetGPS_Position(fLatitude, fLongitude);

					// Set location into DataMS & set GPS status bit
					pDataMS->Set_btGPSError(nStGPS >= 0 ? ST_BIT_OK : ST_BIT_ERR);
					pDataMS->SetGPS_LAT(fLatitude);		// set GPS Latitude
					pDataMS->SetGPS_LON(fLongitude);	// set GPS Longitude

					// Print info
					DBG_PRN(F("\t LAT= ")); DBG_PRN(pDataMS->GetGPS_LAT(), 6);
					DBG_PRN(F("\t LON= "));	DBG_PRN(pDataMS->GetGPS_LON(), 6);
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRNL(F("\t SKIPED"));
				break;
			}

			// 0x50 (80) Turn off power
			case cmndPowerOff: {
				DBG_PRN(F("\t(PowerOff)"));
				// Power OFF
				pPowerDC->PowerOFF();

				DBG_PRNL(F("\t PASSED"));
				break;
			}

			// 0x52 (82) Request new data packet from MS to CP
			case cmndReqDataMS: {
				DBG_PRN(F("\t(ReqDataMS)"));

				// set request data flag
				COPY_NOINT(isReqData, true);

				DBG_PRNL(F("\t PASSED"));
				break;
			}

			// 0xA0 (160) Set DFM-MS main loop period, millis. Default = 200.
			case cmndSetLoopMSPeriod: {
				DBG_PRN(F("\t(SetLoopMSPeriod) : ")); DBG_PRN(pCmndMS->GetArg_dw());
				COPY_NOINT(isTestRunCpy, isTestRun);			// save flag
				if (!isTestRunCpy) {
					// Change parameter
					COPY_NOINT(lLoopMSPeriod, pCmndMS->GetArg_dw());
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRNL(F("\t SKIPED"));
				break;
			}

			// non-existent code
			default: {
				DBG_PRNL(F("\t DO NOTHING PASSED"));
				break;
			}
		}
	}
	else {
		if (nErrCode != -1) {
			// Print info
			DBG_PRN_LOGO(strAppName, strVer);
			DBG_PRN(F("======>>> BTSerial ERROR: "));	DBG_PRN(nErrCode);
			DBG_PRN(F("\t DATA: "));					DBG_PRN((pCmndMS->GetData())[0]);

			if ((pCmndMS->GetData())[0] <= CMND_LEN && (pCmndMS->GetData())[0] >= 0)
				for (int j = 1; j <= CMND_LEN; ++j) { DBG_PRN(F(" 0x")); DBG_PRN((pCmndMS->GetData())[j], HEX); }

			DBG_PRNL(F("\t SKIPED"));

			// Increment counter of bad received packet
			dwCountReceiveErr++;

			// Set ReceiveError status bit to 1
			pDataMS->Set_btReceiveError(ST_BIT_ERR);
		}
		rc = nErrCode;
	}
	return rc;
}
///////////////////////////////////////////////////////////////
// Initialization of data before read from EEPROM
///////////////////////////////////////////////////////////////
void EEPROM_InitData(CDataEEPROM& data) {
	strcpy(data.m_Data.m_strAppName, strAppName);	// Applications name
	data.m_Data.m_nVerMaj = nVerMaj;				// Major version number
	data.m_Data.m_nVerMin = nVerMin;				// Minor version number
	data.m_Data.m_nVerStatus = nVerStatus;			// Status number: 0 - alpha, 1 - beta, 2 - RC, 3 - RTM
	data.m_Data.m_nVerBuild = nVerBuild;			// Build number, from SVC system

	data.m_Data.m_dwPulseFactor = dwPulseFactor;	// Flow meter pulse factor, pulses in 1 ltr
	data.m_Data.m_lLoopMSPeriod = lLoopMSPeriod;	// DFM-MS main loop period, millis
	data.m_Data.m_lDebugPrnPeriod = lDebugPrnPeriod;// Debug print period
	data.m_Data.m_lInt4CalcQ = lInt4CalcQ;			// Interval for calculate instant flow Q, millis
	data.m_Data.m_lCoordUpdPeriod = lCoordUpdPeriod;// Delay between coordinate updates (GPS)
	data.m_Data.m_nTypeRHTSensor = nTypeRHTSensor;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11
}
///////////////////////////////////////////////////////////////
// Read data from EEPROM and save in global variables
// Return: 0 - data read from EEPROM, -1 - EEPROM empty
///////////////////////////////////////////////////////////////
INT EEPROM_ReadData(CDataEEPROM& data) {

	INT rc = 0;

	// Read data from EEPROM into structure m_Data, if EEPROM not empty
	if (EEPROM.read(0) == 0) rc = -1;	// EEPROM empty
	else {								// EEPROM not empty
		data.ReadData();

		// Save read data into global variables
		strcpy(strAppName, data.m_Data.m_strAppName);	// Applications name
		nVerMaj = data.m_Data.m_nVerMaj;				// Major version number
		nVerMin = data.m_Data.m_nVerMin;				// Minor version number
		nVerStatus = data.m_Data.m_nVerStatus;			// Status number: 0 - alfa, 1 - beta, 2 - RC, 3 - RTM
		nVerBuild = data.m_Data.m_nVerBuild;			// Build number, from SVC system

		dwPulseFactor = data.m_Data.m_dwPulseFactor;	// Flowmeter pulse factor, pulses in 1 ltr
		lLoopMSPeriod = data.m_Data.m_lLoopMSPeriod;	// DFM-MS main loop period, millis
		lDebugPrnPeriod = data.m_Data.m_lDebugPrnPeriod;// Debug print period
		lInt4CalcQ = data.m_Data.m_lInt4CalcQ;			// Interval for calculate instant flow Q, millis
		lCoordUpdPeriod = data.m_Data.m_lCoordUpdPeriod;// Delay between coordinate updates (GPS)
		nTypeRHTSensor = data.m_Data.m_nTypeRHTSensor;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11
	}
	return rc;
}
///////////////////////////////////////////////////////////////
// Print data from EEPROM into Serial console
///////////////////////////////////////////////////////////////
void EEPROM_PrintData(CDataEEPROM& data) {
	// Printing data
	STP_PRN(F(" AppName \t\t"));		STP_PRNL(data.m_Data.m_strAppName);
	STP_PRN(F(" VerMaj \t\t"));			STP_PRNL(data.m_Data.m_nVerMaj);
	STP_PRN(F(" VerMin \t\t"));			STP_PRNL(data.m_Data.m_nVerMin);
	STP_PRN(F(" VerStatus \t\t"));		STP_PRNL(data.m_Data.m_nVerStatus);
	STP_PRN(F(" VerBuild \t\t"));		STP_PRNL(data.m_Data.m_nVerBuild);
	STP_PRN(F(" PulseFactor \t\t"));	STP_PRNL(data.m_Data.m_dwPulseFactor);
	STP_PRN(F(" LoopMSPeriod \t\t"));	STP_PRNL(data.m_Data.m_lLoopMSPeriod);
	STP_PRN(F(" DbgPrnPeriod \t\t"));	STP_PRNL(data.m_Data.m_lDebugPrnPeriod);
	STP_PRN(F(" Int4CalcQ \t\t"));		STP_PRNL(data.m_Data.m_lInt4CalcQ);
	STP_PRN(F(" lCoordUpdPeriod \t"));	STP_PRNL(data.m_Data.m_lCoordUpdPeriod);
	STP_PRN(F(" TypeRHTSensor \t\t"));	STP_PRNL(data.m_Data.m_nTypeRHTSensor);
}

#ifdef _DEBUG_TRACE
///////////////////////////////////////////////////////////////
// Read command from serial console (hardware COM)
///////////////////////////////////////////////////////////////
void SerialReadCmnd()
{
	const DWORD		INIT_COUNTER = 3600000000L;	// initial counter for calculations
	INT				nInpByte;					// input byte
	static BOOL		isStopwatch = false;		// stopwatch flag
	static DWORD	lCounterPulse;				// counter of EMFM pulse 
	static DWORD	lTimeStart, lTimeStop;		// start / stop time, ms

												// Read byte from serial port
	if ((nInpByte = Serial.read()) > 0)
	{
		DBG_PRN_LOGO(strAppName, strVer);
		DBG_PRN(F("--->>> Serial PRESSED: "));
		DBG_PRN(nInpByte);

		// Check byte
		switch (nInpByte)
		{
			case 32: {	// SPACE: start/stop stopwatch
				if (!isStopwatch) // start stopwatch
				{
					// save start time, ms
					lTimeStart = millis();
					// set current counter 1 000 hour
					COPY_NOINT(dwCounterPulseDec, INIT_COUNTER);
				}
				else			// stop stopwatch
				{
					// save stop time, ms
					lTimeStop = millis();
					// read pulse counter 
					COPY_NOINT(lCounterPulse, dwCounterPulseDec);
					// print data
					DBG_PRN(F("\t Pulse = "));				DBG_PRN((DWORD)(INIT_COUNTER - lCounterPulse), 10);
					DBG_PRN(F("\t Elapsed time, ms = "));	DBG_PRN((DWORD)(lTimeStop - lTimeStart), 10);
				}
				isStopwatch = !isStopwatch;	// invert flag				break;
			}
			case 49: { // 1: set request data flag
				COPY_NOINT(isReqData, true);
				break;
			}
		}
	}
}
///////////////////////////////////////////////////////////////
// Print debug information into serial console (hardware COM)
///////////////////////////////////////////////////////////////
void SerialPrintData(DWORD loops) {
	// Print data
	// Loops counter
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRN(loops);

	// Status byte
	DBG_PRN(F("\tSt=0x"));				DBG_PRN(pDataMS->Get_bStatus(), HEX);

	// Voltage
	DBG_PRN(F("\tU="));					DBG_PRN(pDataMS->GetPowerU());

	// Water temperature, air RHT
	DBG_PRN(F("\tTW="));				DBG_PRN(pDataMS->GetTemprWater(), 1);
	DBG_PRN(F("\tTA="));				DBG_PRN(pDataMS->GetTemprAir(), 1);
	DBG_PRN(F("\tRH="));				DBG_PRN(pDataMS->GetRHumidityAir(), 1);

	// Location
	DBG_PRN(F("\tGPS="));				DBG_PRN(nStGPS);
	DBG_PRN(F("\tLAT="));				DBG_PRN(pDataMS->GetGPS_LAT(), 6);
	DBG_PRN(F("\tLON="));				DBG_PRN(pDataMS->GetGPS_LON(), 6);

	// Counters
	DBG_PRN(F("\tC+="));				DBG_PRN(pDataMS->GetCounterInc(), 10);
	DBG_PRN(F("\tC-="));				DBG_PRN(pDataMS->GetCounterDec(), 10);

	// Time
	DBG_PRN(F("\tT="));					DBG_PRN(pDataMS->GetTime(), 10);

	// Flow Q, m3/h
	DBG_PRN(F("\tQ="));					DBG_PRN(pDataMS->GetQ(), 3);

	// Counter of read data errors
	DBG_PRN(F("\tRecErr="));			DBG_PRN(dwCountReceiveErr, 10);
}
#endif 