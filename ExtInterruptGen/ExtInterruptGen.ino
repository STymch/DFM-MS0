/*
 Name:		ExtInterruptGen.ino
 Created:	1/23/2018 4:46:46 PM
 Author:	sitymchenko
*/

#include "TimerOne.h"

const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const int	PWM_PIN			= 9;		// Use PWM on this pin
const int	PWM_DUTY		= 256;		// PWM duty cycle, 25%

long	lTimerPeriod	= 1400;			// Timer1 period, microsec.
bool	state = false;					// State for calculation time interval 
float	fTick_ms;						// Millisecs in timer tick

volatile long	lCounter = 0;			// Timer's tick counter
long			lCountStart, lCountStop;// For save timer's ticks  void setup(){	// Initialization pin, Timer1, PWM	pinMode(PWM_PIN, OUTPUT);			// Set pin mode	Timer1.initialize(lTimerPeriod);	// initialize timer1, and set period	Timer1.pwm(PWM_PIN, PWM_DUTY);      // setup pwm on pin 9 and set duty cycle	Timer1.attachInterrupt(callback);	// attaches callback() and enable a timer overflow interrupt
	fTick_ms = lTimerPeriod / 1000.0f;

	// Printing info
	Serial.begin(DR_HARDWARE_COM);				Serial.println();
	Serial.print(F("\nPWM on PIN =\t\t"));		Serial.println(PWM_PIN);
	Serial.print(F("Timer1 period, mcs =\t"));	Serial.println(lTimerPeriod);
	Serial.print(F("Pulse, ms =\t\t"));			Serial.println((PWM_DUTY / 1024.0) * lTimerPeriod / 1000, 2);	Serial.print(F("Ticks, ms =\t\t"));			Serial.println(fTick_ms, 2);
}
void callback(){	++lCounter;}
void loop(){	int		nInpByte;

	// Ожидание команды из последовательного порта (байт)
	if ((nInpByte = Serial.read()) > 0) 
	{
		Serial.print(" PRESSED: ");	Serial.print(nInpByte);
		
		// анализируем принятый байт
		switch(nInpByte)
		{
			case 32:	// SPACE - start/stop calculate elapsed time
				if (!state) // state false - starting counting ticks
				{ 
					noInterrupts();
					lCountStart = lCounter;	// save start time
					interrupts();
				}
				else		// state true - stop counting
				{		
					noInterrupts();
					lCountStop = lCounter;	// save current time
					interrupts();
					
					Serial.print("\tElapsed time, ms = ");	
					Serial.println((unsigned long)((lCountStop-lCountStart)*fTick_ms));
				}
				state = !state;				// change state				break;			case 51:	// 3 - set Q3 flow			case 50:	// 2 - set Q2 flow			case 49:	// 1 - set Q1 flow				// Set new parameters for Q: lTimerPeriod = 3600/Q				if(nInpByte == 51)			lTimerPeriod = 2400;	// Q3=1.500 m3/h					else	if (nInpByte == 50) lTimerPeriod = 24000;	// Q2=0.157 m3/h						else				lTimerPeriod = 56000;	// Q1=0.066 m3/h				fTick_ms = lTimerPeriod / 1000.0f;
				noInterrupts();				Timer1.setPeriod(lTimerPeriod);			// set new timer period				Timer1.setPwmDuty(PWM_PIN, PWM_DUTY);	// setup pwm on pin 9 and set new duty cycle				interrupts();				// Printing info				Serial.println();
				Serial.print(F("\nPWM on PIN =\t\t"));		Serial.println(PWM_PIN);
				Serial.print(F("Timer1 period, mcs =\t"));	Serial.println(lTimerPeriod);
				Serial.print(F("Pulse, ms =\t\t"));			Serial.println((PWM_DUTY / 1024.0) * lTimerPeriod / 1000, 2);				Serial.print(F("Ticks, ms =\t\t"));			Serial.println(fTick_ms, 2);				break;		}	}}
