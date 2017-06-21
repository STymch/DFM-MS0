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
	pinMode(2, INPUT);
	attachInterrupt(0, blink, CHANGE); // 0 = pin 2, 1 = pin 3

}
 
// the loop function runs over and over again until power down or reset
void loop() {
	digitalWrite(pin, state);
}

void blink()
{
	state = !state;
}
