/*
 Name:		ExternalInterrupt.ino
 Created:	11.10.2016 17:32:52
 Author:	sitymchenko
*/

const long  DR_HARDWARE_COM = 38400;	// Data rate for hardware COM, bps
const int	LED = 13;
const int	EXT_INT = 3;

bool isReadPulseDuration = false;	// Flag for read pulse duration
unsigned long lPulseDuration;		// Pulse duration
unsigned long lCurrCount, lTime;

volatile int state = LOW;
volatile unsigned long lCount = 0;

// the setup function runs once when you press reset or power the board
void setup() 
{
	pinMode(LED, OUTPUT);
	pinMode(EXT_INT, INPUT);
	attachInterrupt(1, blink, FALLING); // 0 = pin 2, 1 = pin 3
	
	Serial.begin(DR_HARDWARE_COM);
}
 
// the loop function runs over and over again until power down or reset
void loop() 
{
	int   nInpByte;

	digitalWrite(LED, state);
	
	// Ожидание команды из последовательного порта
	Serial.println("Waiting command...");

	while (Serial.available() <= 0) {
		if (isReadPulseDuration) {
			lPulseDuration = pulseIn(EXT_INT, HIGH);
			Serial.print("\tPulse, mcs = ");	Serial.println(lPulseDuration);
		}
	};

	Serial.print("PRESSED: ");
	// считываем байт данных
	nInpByte = Serial.read();
	Serial.println(nInpByte);

	// анализируем команду
	if (nInpByte == 32) // SPACE - calculate pulse and elapsed time
	{
		lCurrCount = lCount;

		Serial.print("\tCOUNT = ");	Serial.print(lCurrCount, 10);
		Serial.print("\tTIME = ");	Serial.println(millis() - lTime);
		lCurrCount = lCount = 0;
		lTime = millis();
	}
	else if (nInpByte == 49) // 1 - start/stop reading pulse duration
			isReadPulseDuration = !isReadPulseDuration;
}

void blink()
{
	state = !state;
	lCount++;
}
