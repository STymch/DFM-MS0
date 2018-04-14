/*
 Name:		GPS_Test.ino
 Created:	2/18/2018 11:18:50 PM
 Author:	stymc
*/


#include <SoftwareSerial.h>
#include <TinyGPS.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
It requires the use of SoftwareSerial, and assumes that you have a
4800-baud serial GPS device hooked up on pins 12(rx) and 13(tx).
*/

#define __GPS__1__

const unsigned long	COORD_UPD_PERIOD = 1000; // Coordinate update period, ms

TinyGPS gps;
SoftwareSerial ss(12, 13);

uint8_t	state = LOW;
uint8_t	LED = 13;


void setup()
{
	Serial.begin(38400);
	ss.begin(9600);

	Serial.print("---== TinyGPS library v."); Serial.println(TinyGPS::library_version());	
}

#ifdef __GPS__1__
void loop()
{
	bool newData = false;
	unsigned long chars;
	unsigned short sentences, failed;

	// Waiting and parse GPS data and report some key values
	for (unsigned long start = millis(); millis() - start < COORD_UPD_PERIOD && !newData;)
	{
		while (ss.available())
		{	
			// Did a new valid sentence come in?
			char c = ss.read();
			if (gps.encode(c)) newData = true;
		}
	}

	// Check new data present&
	if (newData)
	{
		float flat, flon;
		unsigned long age;
		gps.f_get_position(&flat, &flon, &age);
		
		Serial.print("LAT= ");		// Latitude, Широта
		Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
		Serial.print("\t LON= ");	// Longitude, Долгота
		Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
		Serial.print("\t SAT= ");	// Satellites used in last full GPGGA sentence
		Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
		Serial.print("\t PREC= ");	// Horizontal dilution of precision in 100ths
		Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());

		digitalWrite(LED, state);
		state = !state;
	}

	// GPS statistics
	gps.stats(&chars, &sentences, &failed);
	if (chars == 0) Serial.println("***** #1 NO GPS MODULE *****");
	else {
		Serial.print("\t\t CHARS= ");	Serial.print(chars);
		Serial.print("\t SENTENCES= ");	Serial.print(sentences);
		Serial.print("\t CSUM ERR= ");	Serial.println(failed);
	}
}
#endif __GPS__1__

#ifdef __GPS__2__
// Code for Reading GPS Coordinates
// Building Arduino Projects for the Internet of Things: Experiments with Real-World Applications
// Copyright © 2016 by Adeel Javed

static void smartdelay(unsigned long ms)
{
	unsigned long start = millis();
	do
	{
		while (ss.available())
			gps.encode(ss.read());
	} while (millis() - start < ms);
}

void loop( )
{
	unsigned long age = 0;
	float lat, lon;
	unsigned long chars;
	unsigned short sentences, failed;

	smartdelay(COORD_UPD_PERIOD);
	
	// GPS statistics
	gps.stats(&chars, &sentences, &failed);
	if (chars == 0) Serial.println("***** #2 NO GPS MODULE *****");
	else {
		gps.f_get_position(&lat, &lon, &age);

		lat = (lat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lat);
		lon = (lon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lon);

		Serial.print("\t LAT= ");		Serial.print(lat, 6);
		Serial.print("\t LON= ");	Serial.println(lon, 6);
	}
}
#endif __GPS__2__

#ifdef __GPS__3__
// Проект 33: Модуль GPS. Принцип работы, подключение, примеры
// http://arduino-kit.ru/textpage_ws/pages_ws/proekt-33_--modul-gps.-printsip-rabotyi-podklyuchenie-primeryi
bool newdata = false;
unsigned long start;
long lat, lon;
unsigned long time, date;

void loop()
{
	unsigned long chars;
	unsigned short sentences, failed;

	start = millis();

	// задержка между обновлениями координат
	while (millis() - start < COORD_UPD_PERIOD)
	{
		newdata = readgps();
		if (newdata)
		{
			start = millis();
			gps.get_position(&lat, &lon);
			gps.get_datetime(&date, &time);
			Serial.print("\t Lat: "); Serial.print(lat);
			Serial.print("\t Long: "); Serial.print(lon);
			Serial.print("\t Date: "); Serial.print(date);
			Serial.print("\t Time: "); Serial.println(time);
		}
	}
	// GPS statistics
	gps.stats(&chars, &sentences, &failed);
	if (chars == 0) Serial.println("***** #3 NO GPS MODULE *****");
}

// проверка наличия данных
bool readgps()
{
	while (ss.available())
	{
		int b = ss.read();
		//в TinyGPS есть ошибка: не обрабатываются данные с \r и \n
		if ('\r' != b)
		{
			if (gps.encode(b)) return true;
		}
	}
	return false;
}
#endif __GPS__3__
