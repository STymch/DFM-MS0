/*
 Name:		AnalogInputPin.ino
 Created:	3/27/2017 5:35:52 PM
 Author:	Tymchenko
*/


int analogPin = 0;	// potentiometer wiper (middle terminal) connected to analog pin 3
					// outside leads to ground and +5V
int val = 0;		// variable to store the value read

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(38400);
}

// the loop function runs over and over again until power down or reset
void loop() {
	val = analogRead(analogPin);	// read the input pin
	Serial.println(val);            // debug value 
}
