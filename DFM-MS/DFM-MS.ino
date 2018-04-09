/////////////////////////////////////////////////////////////////////////////////
// ---Working Measurement Standard - Digital Flowmeter (DFM-WMS)  ---
/////////////////////////////////////////////////////////////////////////////////
// --- Digital Flowmeter Measuring System: DFM-MS
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
// Application name
CHAR strAppName[]			= "DFM-MS ";

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

// Metric parameters
const INT	TEMPERATURE_PRECISION = 9;	// Temperature sensor resolution: 9-12, 9 - low 

const INT	STATUS_BIT_OK	= 0;		// Status bit: no error state
const INT	STATUS_BIT_ERR	= 1;		// Status bit: error state

// ---===--- Global variables ---===---
// Application version information
UINT		nVerMaj			= 1;		// Major version number
UINT		nVerMin			= 0;		// Minor version number
UINT		nVerStatus		= 2;		// Status number: 0 - alfa, 1 - beta, 2 - RC, 3 - RTM
UINT		nVerBuild		= 78;		// Build number, from SVC system
CHAR		strVer[28]		= "";		// Full version info string

volatile DWORD	dwCounterPulseAll = 0;	// Counter of all output pulses from EMFM/Generator from turn on DFM-MS
DWORD			dwCounterPulseAllCopy;	// For copy
volatile DWORD	dwCounterPulseCurr = 0;	// Current counter for output pulses from EMFM/Generator
DWORD			dwCounterPulseCurrCopy;	// For copy

DWORD	dwPulseFactor		= 1000;		// Flowmeter pulse factor, pulses in 1 ltr
DWORD	lLoopMSPeriod		= 200;		// DFM-MS main loop period, millis
DWORD	lDebugPrnPeriod		= 2000;		// Debug print period
DWORD	lGetSensorsPeriod	= 60000;	// Get sensors of T, RHT Air period
DWORD	lInt4CalcQ			= 1000;		// Interval for calculate instant flow Q, millis
INT		nDelayAfterPowerON	= 2000;		// Wait for Power ON, millis

DWORD	dwCountReceiveErr	= 0;		// Counter of bad received packet

INT		nALM_FQHWidth		= 500;		// Width in millisec of EMFM ALARM FQH signal
INT		nALM_FQLWidth		= 500;		// Width in millisec of EMFM ALARM FQL signal
INT		nPULSE_INT_MODE		= FALLING;	// Mode of interrupt of EMFM pulse out: LOW, CHANGE, RISING, FALLING
void	(*pISR)();						// Pointer to ISR callback function of EMFM pulse out 

INT		nTypeRHTSensor		= snsrDHT21;// Type of RHT DHT sensor: DHT21, DHT22, DHT11

volatile BOOL	isTestRun = false;		// Test state flag: true - test is running, false - test not running now
BOOL			isTestRunCopy;			// For copy
volatile BOOL	isReqData = false;		// Request for send data: true - request received, no request
BOOL			isReqDataCopy;			// Request for send data: true - request received, no request

BYTE			pBuff[DATA_LEN+1];		// Buffer for save sending data

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
			fLatitude = 0.0,		// GPS Latitude
			fLongitude = 0.0;		// GPS Longitude
	INT		nLEDStateInit = LOW;	// Init LEDs state
	INT		rc;						// return code
	CHAR	strTemp[6];				// temporary string	// --==-- Save application version info into string
	strcat(strVer, itoa(nVerMaj,	strTemp, 10));	strcat(strVer, ".");
	strcat(strVer, itoa(nVerMin,	strTemp, 10));	strcat(strVer, ".");
	if(nVerStatus == 0) strcat(strVer, "a");
	else	if (nVerStatus == 1) strcat(strVer, "b");
			else	if (nVerStatus == 2) strcat(strVer, "RC");
					else strcat(strVer, "RTM");
	strcat(strVer, ".");
	strcat(strVer, itoa(nVerBuild,	strTemp, 10)); strcat(strVer, " ");

	// --==-- Hardware serial port
	// Set the data rate and open
	Serial.begin(DR_HARDWARE_COM);
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRNL(F(": SETUP Starting ..."));
	
	// --==-- EEPROM object
	pCDataEEPROM = new CDataEEPROM;
	// Read data from EEPROM
	EEPROM_ReadData(*pCDataEEPROM);
	
	// --==-- Power DC Control object
	// Create object & initialization
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRN(F(": PowerDC ON: "));
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
	pDataMS->Set_bStatus(STATUS_BIT_OK);
	// Set ReceiveError bit to error state
	pDataMS->Set_btReceiveError(STATUS_BIT_ERR);
	
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
	
	pDataMS->SetCounterAll(pEMFM->GetCounterAll());
	pDataMS->SetCounterCurr(pEMFM->GetCounterCurr());
	pDataMS->SetTime(pEMFM->GetTime());
	pDataMS->SetQ(pEMFM->GetQ());

		// --==-- Humidity & temperature sensor
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRN(F(": RHT sensor: "));
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

	pDataMS->Set_btRHTSensorError( !rc ? STATUS_BIT_OK : STATUS_BIT_ERR);	// set status bit RHT sensor
	pDataMS->SetTemprAir(fTAir);										// set temperature of air
	pDataMS->SetRHumidityAir(fRHumidityAir);							// set humidity

	// --==-- Temperature of water sensor
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRN(F(": Temperature of water sensor: "));
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
	
	pDataMS->Set_btTempSensorError( !rc ? STATUS_BIT_OK : STATUS_BIT_ERR);	// set status bit
	pDataMS->SetTemprWater(fTWater);									// set water temperature

	// --==-- GPS module
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRN(F(": GPS module: "));
	pGPS = new CGPS(RX_PIN_GPS, TX_PIN_GPS, DR_GPS_COM);
	// Waiting...
	delay(2000);
	// Get position from GPS
	if (!(rc = pGPS->GetGPS_Position(fLatitude, fLongitude))) {
		DBG_PRN(F("OK"));
		DBG_PRN(F("\t LAT: "));		DBG_PRN(fLatitude);
		DBG_PRN(F("\t LON: "));		DBG_PRN(fLongitude);
	}
	else { // GPS module not present
		fLatitude = fLongitude = -1.0;
		DBG_PRN(F("NOT DETECTED!"));
	}

	pDataMS->Set_btGPSError (!rc ? STATUS_BIT_OK : STATUS_BIT_ERR);	// set status bit of GPS module
	pDataMS->SetGPS_LAT(fLatitude);									// set GPS Latitude
	pDataMS->SetGPS_LON(fLongitude);								// set GPS Longitude

	// --==-- Delay before starting main loop
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRNL(F(": MAIN Starting ..."));
}

///////////////////////////////////////////////////////////////
// Working loop of DFM-MS
///////////////////////////////////////////////////////////////
void loop() {
	// Loops counter
	static DWORD	lCounterLoops = 0;
	// For save copy of isReqData
	BOOL isReqDataCopy;

	// Save time of begin of loop
	DWORD lTimeBegin = millis();

	// Increment counter of loops
	lCounterLoops++;

	// Read and execute command from DFM-CP BT serial port
	::BTSerialReadCmnd();

	// Calculate data for DataMS packet
	if (lCounterLoops % (lInt4CalcQ / lLoopMSPeriod) == 0) {
		// Get Power DC value and save in data packet
		pDataMS->SetPowerU(pPowerDC->GetPowerDC());

		// Get counters values from ISR
		COPY_NOINT(dwCounterPulseAllCopy, dwCounterPulseAll);
		COPY_NOINT(dwCounterPulseCurrCopy, dwCounterPulseCurr);

		// Get current time
		DWORD lTimeCurr = millis();

		// Calculate flow Q (m3/h)
		pEMFM->CalculateQ(dwCounterPulseAllCopy, lTimeCurr);

		// Save counters and time in EMFM object
		pEMFM->SetCounterAll(dwCounterPulseAllCopy);
		pEMFM->SetCounterCurr(dwCounterPulseCurrCopy);
		pEMFM->SetTime(lTimeCurr);

		// Save counters, time and flow in data packet
		pDataMS->SetCounterAll(dwCounterPulseAllCopy);
		pDataMS->SetCounterCurr(dwCounterPulseCurrCopy);
		pDataMS->SetTime(lTimeCurr);
		pDataMS->SetQ(pEMFM->GetQ());
	}

	// Check request data flag
	COPY_NOINT(isReqDataCopy, isReqData);
	if (isReqDataCopy) { 
		// Write data into BT serial port
		pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);

		// Clear request data flag
		COPY_NOINT(isReqData, false);
	}

	// Blink LEDs
	if (lCounterLoops % (lDebugPrnPeriod / lLoopMSPeriod) == 0) {
		// Blink internal LED 
		pInt_LED->Blink();
	}

	// Calculate time interval from start loop
	DWORD lTimeInt = millis() - lTimeBegin;

	// Delay for end of loop period 
	if(lLoopMSPeriod > lTimeInt) delay(lLoopMSPeriod - lTimeInt);

#ifdef _DEBUG_TRACE
	FLOAT	fTAir,					// Air Temperature
			fRHumidityAir,			// Air Relative Humidity
			fTWater,				// Water Temperature
			fLatitude = 0.0,		// GPS Latitude
			fLongitude = 0.0;		// GPS Longitude

	// Get data from sensors T, RHT of air, GPS
	if (lCounterLoops % (lGetSensorsPeriod / lLoopMSPeriod) == 0 && !isTestRun)
	{	// Read RHT
		if (pDataMS->Get_btRHTSensorError() == STATUS_BIT_OK)
		{
			INT rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir);
			pDataMS->Set_btRHTSensorError(!rc ? STATUS_BIT_OK : STATUS_BIT_ERR);			// set status bit of sensor
			pDataMS->SetTemprAir(fTAir);	pDataMS->SetRHumidityAir(fRHumidityAir);
		}

		// Read water temperature and save in data packet
		if (pDataMS->Get_btTempSensorError() == STATUS_BIT_OK)
		{
			pDataMS->Set_btTempSensorError(!pTemperatureSensor->GetTemperature(fTWater) ? STATUS_BIT_OK : STATUS_BIT_ERR);
			pDataMS->SetTemprWater(fTWater);
		}

		// Get GPS data
		if (pDataMS->Get_btGPSError() == STATUS_BIT_OK && !pGPS->isNewData())
		{
			pGPS->GetGPS_Position(fLatitude, fLongitude);	// get latitude, longitude
			pDataMS->SetGPS_LAT(fLatitude);					// set GPS Latitude
			pDataMS->SetGPS_LON(fLongitude);				// set GPS Longitude
		}
	}

	// Read and execute command from serial console (hardware COM) 
	::SerialReadCmnd();

	// Print data
	if (lCounterLoops % (lDebugPrnPeriod / lLoopMSPeriod) == 0 && !isTestRun) ::SerialPrintData(lCounterLoops);
#endif
}

///////////////////////////////////////////////////////////////
// ISR callback function for pulse out of EMFM
///////////////////////////////////////////////////////////////
void ISR_FlowmeterPulseOut()
{
	// Increment counter of all EMFM pulse
	++dwCounterPulseAll;

	// Decrement current counter while > 0
	if (dwCounterPulseCurr > 0)
	{
		// Decrement counter
		--dwCounterPulseCurr;

		// If current counter == 0
		if (dwCounterPulseCurr == 0) {
			isTestRun = false;	// test completed
			isReqData = true;	// set request data flag
		}
	}
}

///////////////////////////////////////////////////////////////
// Read command from DFM-CP
///////////////////////////////////////////////////////////////
void BTSerialReadCmnd()
{
	FLOAT	fTAir,					// Air Temperature
			fRHumidityAir,			// Air Relative Humidity
			fTWater,				// Water Temperature
			fLatitude = 0.0,		// GPS Latitude
			fLongitude = 0.0;		// GPS Longitude
	DWORD	dwCounterCurr;
	BOOL	isTestRunCopy, isReqDataCopy;
	INT		nErrCode, rc;

	// Read command from DFM-CP
	if ((nErrCode = pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1)) > 0) {
		DBG_PRN_LOGO(strAppName, strVer);
		DBG_PRN(F("--->>> BTSerial CMND: "));	DBG_PRN(pCmndMS->GetCode());
		
		// Set ReceiveError status bit to no error
		pDataMS->Set_btReceiveError(STATUS_BIT_OK);

		// Analize code of command
		switch (pCmndMS->GetCode()) 
		{
			// 67 Set current pulse count
			case cmndSetCounter: {
				DBG_PRN(F("\t(SetCount) : ")); DBG_PRN(pCmndMS->GetArg_dw());
				// 0 - stop test
				if ((dwCounterCurr = pCmndMS->GetArg_dw()) == 0) // stop test
				{
					dwCounterCurr = DWORD(-1);	// set max dword (-1) for current counter
					isTestRunCopy = false;		// clear flag TestRun
				}
				else	// start test
				{
					isTestRunCopy = true;		// set flag TestRun
				}
				// set request data flag
				isReqDataCopy = true;	
			
				COPY_NOINT(dwCounterPulseCurr, dwCounterCurr);	// save current pulse counter
				COPY_NOINT(isTestRun, isTestRunCopy);			// save flag
				COPY_NOINT(isReqData, isReqDataCopy);			// save flag
				
				DBG_PRN(F("\t PASSED"));
				break;
			}

			// 72 Read humidity and temperature from RHT sensor
			case cmndReadRHT: {
				DBG_PRN(F("\t(ReadRHT)"));
				// Check is now test runing
				COPY_NOINT(isTestRunCopy, isTestRun);			// save flag
				if (!isTestRunCopy) {
					// Read humidity and temperature from RHT sensor and save in data packet
					rc = pRHTSensor->GetRHT(fRHumidityAir, fTAir);
					pDataMS->Set_btRHTSensorError(!rc ? STATUS_BIT_OK : STATUS_BIT_ERR);			
					pDataMS->SetTemprAir(fTAir);	pDataMS->SetRHumidityAir(fRHumidityAir);
					
					// set request data flag
					COPY_NOINT(isReqData, true);

					DBG_PRN(F("\t TAir= "));	DBG_PRN(fTAir, 1);
					DBG_PRN(F("\t RH= "));		DBG_PRN(fRHumidityAir, 1);
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;
			}
			
			// 84 Read water temperature from sensor
			case cmndReadTemprWater: {
				DBG_PRN(F("\t(ReadTemprWater)"));
				// Check is now test running
				COPY_NOINT(isTestRunCopy, isTestRun);			// save flag
				if (!isTestRunCopy) {
					// Read temperature and save in data packet
					pDataMS->Set_btTempSensorError(!pTemperatureSensor->GetTemperature(fTWater) ? STATUS_BIT_OK : STATUS_BIT_ERR);
					pDataMS->SetTemprWater(fTWater);
					
					// set request data flag
					COPY_NOINT(isReqData, true);

					DBG_PRN(F("\t TWater= ")); DBG_PRN(fTWater, 1); DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;
			}

			// 76 Get location from GPS
			case cmndGetLocation: {
				DBG_PRN(F("\t(GetLocation)"));
				// Check is now test running
				COPY_NOINT(isTestRunCopy, isTestRun);			// save flag
				if (!isTestRunCopy) {
					if (pDataMS->Get_btGPSError() == STATUS_BIT_OK && !pGPS->isNewData())
					{
						// Get location 
						pGPS->GetGPS_Position(fLatitude, fLongitude);	// get latitude, longitude
						pDataMS->SetGPS_LAT(fLatitude);					// set GPS Latitude
						pDataMS->SetGPS_LON(fLongitude);				// set GPS Longitude
					}
					// set request data flag
					COPY_NOINT(isReqData, true);
					
					// Print info
					DBG_PRN(F("\t LAT= ")); DBG_PRN(pDataMS->GetGPS_LAT(), 6);
					DBG_PRN(F("\t LON= "));	DBG_PRN(pDataMS->GetGPS_LON(), 6);
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;
			}

			// 80 Turn off power
			case cmndPowerOff: {
				DBG_PRN(F("\t(PowerOff)"));
				// Power OFF
				pPowerDC->PowerOFF();
				
				DBG_PRN(F("\t PASSED"));
				break;
			}

			// 82 Request new data packet from MS to CP
			case cmndReqDataMS: {
				DBG_PRN(F("\t(ReqDataMS)"));
				
				// set request data flag
				COPY_NOINT(isReqData, true);
																
				DBG_PRN(F("\t PASSED"));
				break;
			}

			// 160 Set DFM-MS main loop period, millis. Default = 200.
			case cmndSetLoopMSPeriod: {
				DBG_PRN(F("\t(SetLoopMSPeriod) : ")); DBG_PRN(pCmndMS->GetArg_dw());
				COPY_NOINT(isTestRunCopy, isTestRun);			// save flag
				if (!isTestRunCopy) {
					// Change parameter
					noInterrupts();
					lLoopMSPeriod = pCmndMS->GetArg_dw();
					interrupts();
					DBG_PRN(F("\t PASSED"));
				}
				else DBG_PRN(F("\t SKIPED"));
				break;
			}

			// non-existent code
			default: {
				DBG_PRN(F("\t DO NOTHING PASSED"));
				break;
			}
		}
	}
	else if (nErrCode != -1) {
		DBG_PRN_LOGO(strAppName, strVer); 
		DBG_PRN(F("======>>> BTSerial ERROR: "));	DBG_PRN(nErrCode);	
		DBG_PRN(F("\t DATA: "));					DBG_PRN((pCmndMS->GetData())[0]);
		
		if ( (pCmndMS->GetData())[0] <= CMND_LEN && (pCmndMS->GetData())[0] >= 0 )
			for (int j = 1; j <= CMND_LEN; ++j) { DBG_PRN(F(" 0x")); DBG_PRN((pCmndMS->GetData())[j], HEX);	}
		
		DBG_PRN(F("\t SKIPED"));
		
		dwCountReceiveErr++;

		// Set ReceiveError status bit to 1
		pDataMS->Set_btReceiveError(STATUS_BIT_ERR);
	}
}

///////////////////////////////////////////////////////////////
// Read data from EEPROM and save in global variables
///////////////////////////////////////////////////////////////
void EEPROM_ReadData(CDataEEPROM& data) {

	// Read data from EEPROM into structure m_Data
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
	nTypeRHTSensor = data.m_Data.m_nTypeRHTSensor;	// Type of RHT DHT sensor: DHT21, DHT22, DHT11

	// Printing data
	DBG_PRNL(F(" -=== READ data from EEPROM  ===-"));
	DBG_PRN(F("\t AppName \t"));		DBG_PRNL(data.m_Data.m_strAppName);
	DBG_PRN(F("\t VerMaj \t"));			DBG_PRNL(data.m_Data.m_nVerMaj);
	DBG_PRN(F("\t VerMin \t"));			DBG_PRNL(data.m_Data.m_nVerMin);
	DBG_PRN(F("\t VerStatus \t"));		DBG_PRNL(data.m_Data.m_nVerStatus);
	DBG_PRN(F("\t VerBuild \t"));		DBG_PRNL(data.m_Data.m_nVerBuild);
	DBG_PRN(F("\t PulseFactor \t"));	DBG_PRNL(data.m_Data.m_dwPulseFactor);
	DBG_PRN(F("\t LoopMSPeriod \t"));	DBG_PRNL(data.m_Data.m_lLoopMSPeriod);
	DBG_PRN(F("\t DbgPrnPeriod \t"));	DBG_PRNL(data.m_Data.m_lDebugPrnPeriod);
	DBG_PRN(F("\t Int4CalcQ \t"));		DBG_PRNL(data.m_Data.m_lInt4CalcQ);
	DBG_PRN(F("\t TypeRHTSensor \t"));	DBG_PRNL(data.m_Data.m_nTypeRHTSensor);
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
					COPY_NOINT(dwCounterPulseCurr, INIT_COUNTER);
				}	
				else			// stop stopwatch
				{	
					// save stop time, ms
					lTimeStop = millis();
					// read pulse counter 
					COPY_NOINT(lCounterPulse, dwCounterPulseCurr);
					// print data
					DBG_PRN(F("\t COUNT = "));				DBG_PRN((DWORD)(INIT_COUNTER - lCounterPulse), 10);
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
void SerialPrintData( DWORD loops ) {
	// Print data
	// Loops counter
	DBG_PRN_LOGO(strAppName, strVer);	DBG_PRN(loops);
	
	// Status byte
	DBG_PRN(F("\tSt=0x"));					DBG_PRN(pDataMS->Get_bStatus(), HEX);

	// Voltage
	DBG_PRN(F("\tU="));						DBG_PRN(pDataMS->GetPowerU());
	
	// Water temperature, air RHT
	DBG_PRN(F("\tTW="));					DBG_PRN(pDataMS->GetTemprWater(), 1);
	DBG_PRN(F("\tTA="));					DBG_PRN(pDataMS->GetTemprAir(), 1);
	DBG_PRN(F("\tRH="));					DBG_PRN(pDataMS->GetRHumidityAir(), 1);

	// Location
	DBG_PRN(F("\tLAT="));					DBG_PRN(pDataMS->GetGPS_LAT(), 6);
	DBG_PRN(F("\tLON="));					DBG_PRN(pDataMS->GetGPS_LON(), 6);

	// Counters
	DBG_PRN(F("\tCA="));					DBG_PRN(pDataMS->GetCounterAll(), 10);
	DBG_PRN(F("\tCC="));					DBG_PRN(pDataMS->GetCounterCurr(), 10);

	// Time
	DBG_PRN(F("\tT="));						DBG_PRN(pDataMS->GetTime(), 10);

	// Flow Q, m3/h
	DBG_PRN(F("\tQ="));						DBG_PRN(pDataMS->GetQ(), 3);

	// Counter of read data errors
	DBG_PRN(F("\tRecErr="));				DBG_PRN(dwCountReceiveErr, 10);
}
#endif 