
const int   POWER_INPUT = 0;	// Power analog input pin
const int   POWER_ON_OFF = 4;   // Power off output pin
const int   LED_PIN = 13;		// LED output pin

long int	lCount = 0;         // counter
long int	lCountA1 = 0;       // counter alarm 1
long int	lCountA2 = 0;       // counter alarm 2
int			nLEDState = LOW;    // LED state
int			nAlarm = 0;			// Alarm

int			nFactor = 891;		// Max value on pin: calibrated
float		fUmax	= 12.3;		// Power max value: calibrated
float		fDisch1 = 0.9;		// Power discharge threshold 1 (alarm)	
float		fDisch2 = 0.8;		// Power discharge threshold 2 (power OFF)	
int			nDelayAfterPowerON = 2000;		// Wait for Power ON, millis

void setup() {
	Serial.begin(38400);

	// declare the pin as an OUTPUT:
	pinMode(LED_PIN, OUTPUT);
	pinMode(POWER_ON_OFF, OUTPUT);

	// Power ON
	digitalWrite(POWER_ON_OFF, HIGH);
	delay(nDelayAfterPowerON);
}

void loop() {
	// Power input, Volts
	float fUin;
	int val;

	if (nAlarm == 0 || nAlarm == 1) {
		// read the value from Power input pin:
		val = analogRead(POWER_INPUT);
		fUin = (fUmax * val) / nFactor;
		
		if (fUin <= fUmax * fDisch1 && nAlarm == 0) {
			nAlarm = 1;
			lCountA1 = lCount;
		}

		if (fUin <= fUmax * fDisch2 && nAlarm != 2) {
			nAlarm = 2;
			lCountA2 = lCount;
		}

		Serial.print("\tCount=\t"); Serial.print(lCount);
		Serial.print("\tA1=\t");	Serial.print(lCountA1);
		Serial.print("\tA2=\t");	Serial.print(lCountA2);
		Serial.print("\tInp=\t");   Serial.print(val);
		Serial.print("\tU=\t");		Serial.println(fUin);
	}
	
	if (nAlarm == 2) 
	{
		// Power OFF
		digitalWrite(POWER_ON_OFF, LOW);
		delay(nDelayAfterPowerON);
		nAlarm = 3;
	}

	nLEDState = !nLEDState;
	digitalWrite(LED_PIN, nLEDState);

	delay(1000);
	lCount++;

	if (nAlarm >= 3 && nAlarm <= 13) {
		// read the value from Power input pin:
		val = analogRead(POWER_INPUT);
		fUin = (fUmax * val) / nFactor;
		nAlarm++;
		
		Serial.print("\tCount=\t"); Serial.print(lCount);
		Serial.print("\tA1=\t");	Serial.print(lCountA1);
		Serial.print("\tA2=\t");	Serial.print(lCountA2);
		Serial.print("\tInp=\t");   Serial.print(val);
		Serial.print("\tU=\t");		Serial.println(fUin);
	}
}
