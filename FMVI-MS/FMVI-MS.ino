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
//  - определение и объявление общих глобальных данных
//  - функции:
//		setup()					- инициализация всех обьектов
//		loop()					- бесконечный рабочий цикл системы
//  - локальные функции:
//		ReadTemperatureDS ( )	- запрос температуры среды из датчика типа DS18B20
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
const int	MODE_INT = CHANGE;			// mode of external interrupt

// Serial ports parameters
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM, bps
const long	SERIAL_READ_TIMEOUT = 10;	// Timeout for serial port data read, millisecs

// Time parameters
const int	DELAY_BEFORE_READ_BT = 250;	// Delay before read data from BT COM port, millises
const DWORD	FREQUENCY_TIMER2_MS = 250;	// Timer2 period in milliseconds

// Global variables
volatile DWORD	CEMFM::dwCountFullPulse;	// Counter of all input pulses from EMFM/Generator from start
volatile DWORD	CEMFM::dwCountCurrPulse;	// Current counter of input pulses from EMFM/Generator

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
	static bool led_out = HIGH;
	
	// Write data into BT COM port
	//if (isDataChange) 
	{
		pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);
		isDataChange = FALSE;
	}

	// Наращиваем счетчик тиков таймера
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
	DWORD dwTimeBeginPulse;	// time of begin pulse

	// Increment counter of all EMFM pulse
	//CEMFM::dwCountFullPulse++;

	// Decrement read pulse counter while > 0
	//if (CEMFM::dwCountCurrPulse > 0) CEMFM::dwCountCurrPulse--;

	// Disable all interrupts
	noInterrupts();
	// Read pulse pin state
	if (digitalRead(pEMFM->GetPulsePin()) == pEMFM->GetTypePulseFront()) // pin state is front of pulse - pulse begin
	{
		dwTimeBeginPulse = millis(); // save time of pulse begin
	}
	else	// pulse end, check it width 
	{
		if (millis() - dwTimeBeginPulse >= pEMFM->GetInpPulseWidth() ) // width of pulse correct
		{
			// Increment counter of all EMFM pulse
			pDataMS->SetCountC(++CEMFM::dwCountFullPulse);

			// Decrement current counter while > 0
			if (CEMFM::dwCountCurrPulse > 0)
			{
				pDataMS->SetCountF(--CEMFM::dwCountCurrPulse);
				// If read all pulse - write data into BT COM port
				if (CEMFM::dwCountCurrPulse == 0)
				{
					pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);
				}
			}
		}
	}
	// Enable all interrupts
	interrupts();
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

	CEMFM::dwCountFullPulse = 0;
	CEMFM::dwCountCurrPulse = 10000L;

	pEMFM = new CEMFM(TEST_PIN, ALM_FQH_PIN, ALM_FQH_PIN, LOW, LOW, LOW, 50, 50, 50);
	pEMFM->Init(INT_1, MODE_INT, ISR_InputPulse);

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
	// Press any key for start / stop loop
	if (isKeyHit) 
	{
		
//		noInterrupts();     // disable interrupts

		// Print number of loop
		Serial.println(); Serial.print("Loop="); Serial.print(lCount); 
		Serial.print("\tTimerTick="); Serial.print(dwTimerTick);
		Serial.print("\tCountFullPulse="); Serial.print(CEMFM::dwCountFullPulse);
		Serial.print("\tCountCurrPulse="); Serial.println(CEMFM::dwCountCurrPulse);

		// Fill data
		pDataMS->SetStatus(BYTE(nStatus));
		pDataMS->SetTempr(fT);
		pDataMS->SetPowerU(nU);
		pDataMS->SetQ(fQ);
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
		fQ += 0.01;
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