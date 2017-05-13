// CRHTSensor.h

#ifndef _CRHTSENSOR_h
#define _CRHTSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
/*
#if defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny26__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny45__) || (__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
#include <TinyWireM.h>
#define Wire TinyWireM
#else defined
#include <Wire.h>
#endif
*/

//#include <Wire.h>
#include <HTU21D.h>

class CRHTSensor
{
 protected:


 public:
	void Init();
	
	// Constructor, destructor
	CRHTSensor() {
		HTU21D myHTU21D;

		// Setup
		Serial.begin(38400);
		Serial.println(F(""));

#if defined(ARDUINO_ARCH_ESP8266) || (ESP8266_NODEMCU)
		while (myHTU21D.begin(D1, D2) != true)
#else
		while (myHTU21D.begin() != true)
#endif
		{
			Serial.println(F("HTU21D, SHT21 or Si70xx sensor is not present..."));
			delay(5000);
		}

		Serial.println(F("HTU21D, SHT21 or Si70xx sensor is present"));


		// Loop

		Serial.println(F("..."));
		Serial.println(F("<< RHTSensor DEMO: %RH - 12Bit, Temperature - 14Bit (default settings) >>"));
		Serial.print(F("Humidity: "));
		Serial.print(myHTU21D.readHumidity());
		Serial.println(F(" +-2%RH"));
		Serial.print(F("Compensated Humidity: "));
		Serial.print(myHTU21D.readCompensatedHumidity());
		Serial.println(F(" +-2%RH"));
		Serial.print(F("Temperature: "));
		Serial.print(myHTU21D.readTemperature());
		Serial.println(F(" +-0.5deg.C"));

		Serial.println(F("..."));
		Serial.println(F("<< DEMO: %RH - 11Bit, Temperature - 11Bit >>"));
		myHTU21D.setResolution(HTU21D_RES_RH11_TEMP11);
		Serial.print(F("Humidity: "));
		Serial.print(myHTU21D.readHumidity());
		Serial.println(F(" +-2%RH"));
		Serial.print(F("Compensated Humidity: "));
		Serial.print(myHTU21D.readCompensatedHumidity());
		Serial.println(F(" +-2%RH"));
		Serial.print(F("Temperature: "));
		Serial.print(myHTU21D.readTemperature());
		Serial.println(F(" +-0.5deg.C"));

		Serial.println(F("..."));
		Serial.println(F("<< DEMO: Battery Status >>"));
		if (myHTU21D.batteryStatus() == true)
		{
			Serial.println(F("Battery OK. Level > 2.25v"));
		}
		else
		{
			Serial.println(F("Battery LOW. Level < 2.25v"));
		}

		Serial.println(F("..."));
		Serial.println(F("<< DEMO: Fimware version >>"));
		Serial.print(F("FW version: "));
		Serial.println(myHTU21D.readFirmwareVersion());

		Serial.println(F("..."));
		Serial.println(F("<< DEMO: Device ID >>"));
		Serial.print(F("Sensor's name: "));
		Serial.println(myHTU21D.readDeviceID());
	}
	~CRHTSensor() {}

};


#endif

