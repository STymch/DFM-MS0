// CTemperatureSensor.h

#ifndef _CTEMPERATURESENSOR_h
#define _CTEMPERATURESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <OneWire.h>

class CTemperatureSensor
{
 protected:


 public:
	void Init();

	// Constructor, destructor
	CTemperatureSensor() {}
	~CTemperatureSensor() {}
};


#endif

