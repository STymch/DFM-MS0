/////////////////////////////////////////////////////////////////////////////////
// --- Flowmeter verification installation - working measurement standard  ---
/////////////////////////////////////////////////////////////////////////////////
// --- S U B S Y S T E M ---
// ---  Flowmeter verification installation measuring system: FMVI-MS
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
// Contents:
//  - определение и объявление общих глобальных данных
//  - функции:
//		setup()					- инициализация всех обьектов
//		loop()					- бесконечный рабочий цикл системы
//  - локальные функции:
//		ReadTemperatureDS ( )	- запрос температуры среды из датчика типа DS18B20
//		
/////////////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>		
#include <OneWire.h>
#include <SoftwareSerial.h>

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
  /* чиатем данные из последовательного порта
   *  
   */
  
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

