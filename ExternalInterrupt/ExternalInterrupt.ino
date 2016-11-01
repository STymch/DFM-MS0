/*
 Name:		ExternalInterrupt.ino
 Created:	11.10.2016 17:32:52
 Author:	sitymchenko
*/

int pin = 13;
volatile int state = LOW;

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(pin, OUTPUT);
	attachInterrupt(1, blink, FALLING);

}

// the loop function runs over and over again until power down or reset
void loop() {
	digitalWrite(pin, state);
}

void blink()
{
	state = !state;
}
