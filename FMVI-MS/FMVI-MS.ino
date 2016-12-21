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
#include "CDataMS.h"
#include "CSerialPort.h"
#include <EEPROM.h>		
#include <OneWire.h>


const int   LED_PIN = 13;				// LED pin
const int	RX_PIN = 10;				// Software UART RX pin, connect to TX of Bluetooth HC-05 
const int	TX_PIN = 11;				// Software UART TX pin, connect to RX of Bluetooth HC-05
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM, bps

const long	SERIAL_READ_TIMEOUT = 10;	// Timeout for serial port data read, millisecs
const int	DELAY_BEFORE_READ_BT = 500;	// Delay before read data from BT COM port, millises

// Global variables:
BYTE pBuff[DATA_LEN+1];
INT i, nLen;
long lCount = 0, lCountR = 0;
long lError = 0;
int nTypeSerial = 1; // 0 - hardware, 1 - software

// Global objects
// Serial ports
CSerialPort		*pBTSerialPort;	// For bluetooth modem 
				
// Data structure of data of FMVI-MS
CDataMS			*pDataMS;

DWORD	dwCFull = 0, dwCCurr = 1000000L;
INT		nStatus = 0;
FLOAT	fT = 0.5, fQ = 0.01;
UINT	nU = 2;

bool	isKeyHit = FALSE;
bool	isDataCompare = FALSE;


void setup() 
{
	#ifdef _DEBUG_TRACE
		// Set the data rate and open hardware COM port:
		Serial.begin(DR_HARDWARE_COM);
		while (!Serial);
	#endif

	// Create objects of FMVI-MS:
	pBTSerialPort = new CSerialPort(1, RX_PIN, TX_PIN);

	pBTSerialPort->Init(DR_SOFTWARE_COM, 0);
	pBTSerialPort->SetReadTimeout(SERIAL_READ_TIMEOUT);

	pDataMS = new CDataMS;
		
			
	#ifdef _DEBUG_TRACE
		Serial.println("----- Starting, please press any key! -----");
		Serial.print(" SERIAL_READ_TIMEOUT = "); Serial.print(SERIAL_READ_TIMEOUT);
		Serial.print(" DELAY_BEFORE_READ_BT = "); Serial.print(DELAY_BEFORE_READ_BT);
		Serial.println();
	#endif
			
}

void loop()
{
	// Press any key for start / stop loop
	if (isKeyHit) {
		
		// Print number of loop
		Serial.println(); 
		Serial.print("Counts: Write="); Serial.print(lCount); Serial.print(" Read="); Serial.print(lCountR);
		Serial.print(" W-R="); Serial.println(lCount-lCountR);
				
		// Fill data
		pDataMS->SetStatus(BYTE(nStatus));
		pDataMS->SetTempr(fT);
		pDataMS->SetPowerU(nU);
		pDataMS->SetQ(fQ);
		pDataMS->SetCountC(dwCCurr);
		pDataMS->SetCountF(dwCFull);

		// Write data into BT COM port
		pBTSerialPort->Write(pDataMS->GetDataMS(), DATA_LEN + 1);
		lCount++;
		Serial.print(" Write ->");

		// Delay
		delay(DELAY_BEFORE_READ_BT); 
		
		// Read data from BT port, compare write and read data
		if ((nLen = pBTSerialPort->Read(pBuff, DATA_LEN + 1)) > 0) {
			lCountR++;
			Serial.print("<- Read ");
			isDataCompare = TRUE;
			for (int i = 0; i <= DATA_LEN && isDataCompare; i++)
				isDataCompare = (pBuff[i] == pDataMS->GetDataMS()[i]);
			if (!isDataCompare) {
				isKeyHit = FALSE;
				lError++; 
				Serial.println();
				Serial.print("----- Write / Read compare error -----"); Serial.println();
				Serial.print("- Write data: Buffer ");
				for (int i = 0; i <= DATA_LEN; Serial.print(i), Serial.print("="),
					Serial.print(pDataMS->GetDataMS()[i++], HEX), Serial.print(" "));
				Serial.println();
				
				Serial.print("- Read data:  Buffer ");
				for (int i = 0; i <= DATA_LEN; Serial.print(i), Serial.print("="),
					Serial.print(pBuff[i++], HEX), Serial.print(" "));
				Serial.println();
			}
			Serial.print(" Error = "); Serial.print(lError);
			
			// Change data
			nStatus++;
			fT += 0.01;
			nU++;
			fQ += 0.01;
			dwCFull++;
			dwCCurr--;
		}
		#ifdef _DEBUG_TRACE
		else if (nLen != -1) { Serial.print("BT Read data: Error="); Serial.println(nLen); isKeyHit = FALSE; }
		#endif

		/*		Serial.println();
		Serial.print("Write data: Buffer ");
		for (int i = 0; i <= DATA_LEN; Serial.print(i), Serial.print("="),
			Serial.print(pDataMS->GetDataMS()[i++], HEX), Serial.print(" "));
		Serial.println();

		Serial.print("Status=");	Serial.print(pDataMS->GetStatus());	Serial.println();
		Serial.print("Tempr=");		Serial.print(pDataMS->GetTempr());	Serial.println();
		Serial.print("U=");			Serial.print(pDataMS->GetPowerU()); Serial.println();
		Serial.print("Q=");			Serial.print(pDataMS->GetQ());		Serial.println();
		Serial.print("CountC=");	Serial.print(pDataMS->GetCountC()); Serial.println();
		Serial.print("CountF=");	Serial.print(pDataMS->GetCountF()); Serial.println();
		Serial.println();
*/
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