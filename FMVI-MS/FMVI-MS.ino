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
const int   EMFM_PIN = 2;				// EMFM digital out input pin
const int   TEST_PIN = 3;				// Test digital generator out input pin
const int   TEMP_PIN = 4;				// Temperature sensor DS18B20 DQ out input pin
const int   ALM_FQH_PIN = 5;			// EEMFM FQH ALARM out input pin
const int   ALM_FQL_PIN = 6;			// EEMFM FQL ALARM out input pin
const int   POWER_OFF_PIN = 7;			// Power off output pin

const int	RX_PIN = 10;				// Software UART RX pin, connect to TX of Bluetooth HC-05 
const int	TX_PIN = 11;				// Software UART TX pin, connect to RX of Bluetooth HC-05
const int   LED_PIN = 13;				// LED output pin

// Arduino external interrupts
const int	INT_0 = 0;					// external interrupt 0 (connect to digital pin 2)
const int	INT_1 = 1;					// external interrupt 1 (connect to digital pin 3)
const int	MODE_INT = CHANGE;			// mode of external interrupt: on change state pin

// Serial ports parameters
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM, bps
const long	SERIAL_READ_TIMEOUT = 10;	// Timeout for serial port data read, millisecs

// Time parameters
const int	DELAY_BEFORE_READ_BT = 250;	// Delay before read data from BT COM port, millises
const DWORD	FREQUENCY_TIMER2_MS = 250;	// Timer2 period in milliseconds

// Global variables
DWORD	dwCountBadPulse = 0;				// Counter bad input pulse packet (front less width)

//volatile DWORD	CEMFM::dwCountFullPulse;	// Counter of all input pulses from EMFM/Generator from start
//volatile DWORD	CEMFM::dwCountCurrPulse;	// Current counter of input pulses from EMFM/Generator
//volatile FLOAT	CEMFM::fQm3h;				// Current flow m3/h
//volatile UINT	CEMFM::nTimeInterval4Q;		// 
	
BYTE pBuff[DATA_LEN+1];


INT i, nLen;
long lCount = 0, lCountR = 0;
long lError = 0;
int nTypeSerial = 1; // 0 - hardware, 1 - software
volatile DWORD	dwTimerTick = 0;

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

bool	isKeyHit = TRUE;
bool	isDataCompare = FALSE;
volatile bool	isDataChange = FALSE;
decltype (isKeyHit) isKeyPress;

///////////////////////////////////////////////////////////////
// Timer2 ISR callback function
///////////////////////////////////////////////////////////////
void ISR_Timer2() {
	static	bool led_out = HIGH;
	DWORD	dwCountCurrPulse;
	
	// Read command from FMVI-CP
	if (pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1) > 0) {
		// Analize input command
		switch (pCmndMS->GetCode()) {
		
		// Set current pulse count 
		case cmndSetCount:									
//			noInterrupts();									// disable interrupts
			dwCountCurrPulse = pCmndMS->GetArg_dw();// current pulse count value
			if (dwCountCurrPulse == 0)				// if 0 - set count to -1
				dwCountCurrPulse = DWORD(-1);
			
			// Save current pulse count
			pEMFM->SetCountCurr(dwCountCurrPulse);	
			// Save current pulse count into data packet
			pDataMS->SetCountC(dwCountCurrPulse);
			//			interrupts();									// enable interrupts
			break;

		case cmndPowerOff:									// Turn off power

			break;
		}

	}

	// Calculate current flow Q
	pDataMS->SetQ(pEMFM->CalculateQ());

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
	// Disable all interrupts
	//noInterrupts();
	// Read pulse pin state - is pin state is front?
	if (pEMFM->isPulseFront()) // pulse begin
		pEMFM->SetTStartPulse(millis()); // save time of pulse begin
	else	// pulse end, check correct pulse 
	{
		if (pEMFM->isPulse(millis()) ) // pulse correct
		{
			// Increment counter of all EMFM pulse
			pEMFM->SetCountFull(pEMFM->GetCountFull() + 1);
			// Save new counter in data packet
			pDataMS->SetCountF(pEMFM->GetCountFull());

			// Decrement current counter while > 0
			if (pEMFM->GetCountCurr() > 0)
			{
				// Decrement counter
				pEMFM->SetCountCurr(pEMFM->GetCountCurr() - 1);
				// Save new counter in data packet
				pDataMS->SetCountC(pEMFM->GetCountCurr());
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
	// Enable all interrupts
	//interrupts();
}


///////////////////////////////////////////////////////////////
// Initialisation FMVI-MS
///////////////////////////////////////////////////////////////
void setup() 
{
	#ifdef _DEBUG_TRACE
		// Set the data rate and open hardware COM port:
		Serial.begin(DR_HARDWARE_COM);
		while (!Serial);
	#endif

	// Set pin mode for LED pin
	pinMode(LED_PIN, OUTPUT);

	// Create objects of FMVI-MS:
	pBTSerialPort = new CSerialPort(1, RX_PIN, TX_PIN);

	pBTSerialPort->Init(DR_SOFTWARE_COM, 0);
	pBTSerialPort->SetReadTimeout(SERIAL_READ_TIMEOUT);

	pDataMS = new CDataMS;
	pCmndMS = new CCmndMS;

	//CEMFM::dwCountFullPulse = 0;
	//CEMFM::dwCountCurrPulse = DWORD(-1);

	pEMFM = new CEMFM(TEST_PIN, ALM_FQH_PIN, ALM_FQH_PIN, LOW, LOW, LOW, 5, 50, 50);
	pEMFM->Init(0, DWORD(-1), 500, 2*FREQUENCY_TIMER2_MS, INT_1, MODE_INT, ISR_InputPulse);

	// Set Timer2 period milliseconds
	MsTimer2::set(FREQUENCY_TIMER2_MS, ISR_Timer2);
	// Enable Timer2 interrupt
	MsTimer2::start();

							
	#ifdef _DEBUG_TRACE
		Serial.println("----- Starting, please press any key! -----");
		Serial.print(" SERIAL_READ_TIMEOUT = "); Serial.print(SERIAL_READ_TIMEOUT);
//		Serial.print(" DELAY_BEFORE_READ_BT = "); Serial.print(DELAY_BEFORE_READ_BT);
		Serial.print(" FREQUENCY_TIMER2_MS = "); Serial.print(FREQUENCY_TIMER2_MS);
		Serial.println();
	#endif
			
}

///////////////////////////////////////////////////////////////
// Working loop of FMVI-MS
///////////////////////////////////////////////////////////////
void loop()
{
	
	// Read command from FMVI-CP
/*	if (pBTSerialPort->Read(pCmndMS->GetData(), CMND_LEN + 1) > 0) {
		// Analize input command
		Serial.print("************************** INPUT COMMAND: "); 
		Serial.print(pCmndMS->GetCode(), HEX); 
		Serial.print(" ");
		Serial.print(pCmndMS->GetArg_dw());
		Serial.println("****************************************************************************************");
		switch (pCmndMS->GetCode()) {
		case cmndSetCount:		// Set current pulse count 
			noInterrupts();     // disable interrupts
			CEMFM::dwCountCurrPulse = pCmndMS->GetArg_dw();
			if (CEMFM::dwCountCurrPulse == 0) CEMFM::dwCountCurrPulse = DWORD(-1);
			interrupts();       // enable interrupts
			break;

		case cmndPowerOff:	// Turn off power

			break;

		case cmndSetImpInpPin:	// Set number input pin for input pulse

			break;

		}

	}
*/

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
	
	// Press any key for start / stop loop
	if (isKeyHit) 
	{
		
//		noInterrupts();     // disable interrupts
		//if (lCount % 100 == 0)
		if (dwTimerTick % 10 == 0)
		{
			// Print number of loop
			Serial.println();		Serial.print(""); Serial.print(lCount);
			Serial.print("\tTT=");	Serial.print(dwTimerTick);
			Serial.print("\tCF=");	Serial.print(pEMFM->GetCountFull());
			Serial.print("\tCC=");	Serial.print(pEMFM->GetCountCurr());
			Serial.print("\tCB=");	Serial.print(dwCountBadPulse);
			Serial.print("\tQ=");	Serial.print(pEMFM->GetQCurr());
		}

		// Fill data
		pDataMS->SetStatus(BYTE(nStatus));
		pDataMS->SetTempr(fT);
		pDataMS->SetPowerU(nU);
//		pDataMS->SetQ(fQ);
		//pDataMS->SetCountC(CEMFM::dwCountFullPulse);
		//pDataMS->SetCountF(CEMFM::dwCountCurrPulse);
		
		isDataChange = TRUE;

		//interrupts();       // enable interrupts

		// Counter of loops
		lCount++;
		
		// Change data
		nStatus++;
		fT += 0.01;
		nU++;
//		fQ += 0.01;
	}
	// Check press key
	if (Serial.available()){
		isKeyHit = !isKeyHit; 
		Serial.read();
	}
	while (Serial.available()) { Serial.read(); }
}
	

/*
// Глобальные константы
const int   LED_PIN         = 13;     // номер выхода, к которому подключен светодиод
//const int   OPTO_SENSOR_PIN = 2;      // номер входа, к которому подключен оптоэлектронный датчик    
const int   OPTO_SENSOR_PIN = 8;      // номер входа, к которому датчик EMFM    
const long  DEFAULT_SPEED   = 115200; // нкорость передачи данных последовательного порта бит/с по умолчанию
const int   BEAM_ASTERISK   = 10;     // количество лучей сигнальной звездочки

// Глобальные параметры конфигурации
long  lSerialSpeed;   // скорость передачи данных последовательного порта бит/с
int   nBeamAsterisk;  // количество лучей сигнальной звездочки
long  lK1;            // передаточный коэффициент числитель
long  lK2;            // передаточный коэффициент знаменатель

// Глобальные переменные
long  lCounter;       // счетчик срабатываний оптоэлектронного датчика
int   nSensorStatus;  // статус оптоэлектронного датчика: HIGH - нет событий , LOW - срабатывание 

// Стартовая функция
void setup()
{
  // Инициализация входов/выходов
  pinMode(OPTO_SENSOR_PIN, INPUT);  // вход оптоэлектронного датчика
  pinMode(LED_PIN, OUTPUT);         // выход светодиода

  // Инициализация последовательного порта
  Serial.begin(DEFAULT_SPEED);
}

// Бесконечный рабочий цикл системы
void loop()
{
  // Локальные переменные
  int nInpByte;         // хранение полученного из порта байта
  boolean bLED = false; // индикация светодиода 
  
  // Получение параметров водосчетчика
  
  // Ожидание команды из последовательного порта на начало подсчета оборотов сигнальной звездочки
  // ожидаем появления данных в последовательном порту
  Serial.println("Waiting command...");
  
  while (Serial.available() <= 0);
  //  Serial.print("Waiting...\n");
  
  Serial.print("Received command:");
  nInpByte = Serial.read();
  Serial.println(nInpByte);
  
  // считываем байт данных
  if (nInpByte == 49)
  {  
	Serial.println("Counting sensor pulse...");
	lCounter = 0;
	while (nInpByte != 50)
	{
	  do{
		  nSensorStatus = digitalRead(OPTO_SENSOR_PIN);
		  nInpByte = Serial.read();
	  } while (nSensorStatus == HIGH && nInpByte != 50);
	  if (nInpByte == 50) break;
	  else {digitalWrite(LED_PIN, bLED); bLED = !bLED;} // индикация светодиода

	  while (nSensorStatus == LOW && nInpByte != 50)
	  {
		  nSensorStatus = digitalRead(OPTO_SENSOR_PIN);
		  nInpByte = Serial.read();
	  }
	  if (nInpByte == 50) break;
	  else 
	  {
		digitalWrite(LED_PIN, bLED); bLED = !bLED;
		lCounter++; Serial.println (lCounter);
	  } 
	}
	
	Serial.print("Finished:");
	// Serial.println (nInpByte);
	Serial.println (lCounter);
  
  }

}

*/