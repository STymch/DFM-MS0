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
#include "CSerialPort.h"
#include <EEPROM.h>		
#include <OneWire.h>


const int   LED_PIN = 13;				// LED pin
const int	RX_PIN = 10;				// Software UART RX pin, connect to TX of Bluetooth HC-05 
const int	TX_PIN = 11;				// Software UART TX pin, connect to RX of Bluetooth HC-05
const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const long  DR_SOFTWARE_COM = 38400;	// Data rate for software COM, bps

const int	MAX_BUFF_SIZE = 21;			// Max size of bytes data array buffer
const long	SERIAL_READ_TIMEOUT = 10;	// Timeout for serial port data read, millisecs

// Global variables:
BYTE pBuff[MAX_BUFF_SIZE];
INT i, nLen;
long lCount = 0;
int nTypeSerial = 1; // 0 - hardware, 1 - software

// Global objects
// Serial ports
CSerialPort		*pBTSerialPort,	// For bluetooth modem 
				*pHSerialPort;	// Yardware port

void setup() 
{
	// Create objects of FMVI-MS:
	pHSerialPort = new CSerialPort();
	pBTSerialPort = new CSerialPort(1, RX_PIN, TX_PIN);

	pHSerialPort->Init(DR_HARDWARE_COM, 0);
	pBTSerialPort->Init(DR_SOFTWARE_COM, 0);
	pBTSerialPort->SetReadTimeout(SERIAL_READ_TIMEOUT);

	#ifdef _DEBUG_TRACE
		pHSerialPort->Write((BYTE*)"Starting hardware COM!\n\r", 25);
		pBTSerialPort->Write((BYTE *)"Starting BT software COM!\n\r", 28);
	#endif
	
	delay(1000);
}

void loop()
{

	// Read data from hardware port, write into bluetooth port
	if ((nLen = pHSerialPort->Read(pBuff)) > 0) {
		#ifdef _DEBUG_TRACE
			Serial.print("Len="); Serial.print(nLen); 
			Serial.print(" Buff="); Serial.write(pBuff, nLen); Serial.println();
		#endif
	}
	#ifdef _DEBUG_TRACE
	else if (nLen != -1){ Serial.print("H Len="); Serial.println(nLen); }
	#endif
	
	// Read data from bluetooth port, write into hardware port and back to bluetooth
	if ((nLen = pBTSerialPort->Read(pBuff, MAX_BUFF_SIZE)) > 0) {
		lCount++;
		#ifdef _DEBUG_TRACE
			Serial.print("Count="); Serial.print(lCount);
			Serial.print("\tLen=");	Serial.print(nLen);
			Serial.print("\tBuff="); Serial.write(pBuff, nLen); Serial.println();
		#endif
		//pHSerialPort->Write(pBuff, n);
		pBTSerialPort->Write(pBuff, nLen);
	}
	#ifdef _DEBUG_TRACE
	else if (nLen != -1) { Serial.print("BT Len="); Serial.println(nLen); }
	#endif
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