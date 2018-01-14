/*
 Name:		ExternalInterrupt.ino
 Created:	11.10.2016 17:32:52
 Author:	sitymchenko
*/

int LED = 13;
int EXT_INT = 3;
volatile int state = LOW;
volatile unsigned long lCount = 0;

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(LED, OUTPUT);
	pinMode(EXT_INT, INPUT);
	attachInterrupt(1, blink, FALLING); // 0 = pin 2, 1 = pin 3
	
	Serial.begin(38400);
}
 
// the loop function runs over and over again until power down or reset
void loop() {
	unsigned long lCurrCount, lTime=millis();	
	int   nInpByte;

	digitalWrite(LED, state);
	
	// Ожидание команды из последовательного порта
	Serial.println("Waiting command...");

	while (Serial.available() <= 0);

	Serial.print("PRESSED: ");
	// считываем байт данных
	nInpByte = Serial.read();
	Serial.println(nInpByte);

	// анализируем команду
	if (nInpByte == 32) // SPACE
	{
		lCurrCount = lCount;	
		
		Serial.print("\tCOUNT = ");	Serial.print(lCurrCount, 10);
		Serial.print("\tTIME = ");	Serial.println(millis()-lTime);
		lCurrCount = lCount = 0;
		lTime = millis();
	}
}

void blink()
{
	state = !state;
	lCount++;
}
