/*
 Name:		AnalogInputPin.ino
 Created:	3/27/2017 5:35:52 PM
 Author:	Tymchenko
*/


int analogPin = 1;	// potentiometer wiper (middle terminal) connected to analog pin 3
					// outside leads to ground and +5V
int val = 0;		// variable to store the value read

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(38400);
}

// the loop function runs over and over again until power down or reset
void loop() {
	float fV;											// volts
	val =	analogRead(analogPin);						// read the input pin
	fV = 5.0f * val / 1024.0f;
	Serial.print("Inp="); Serial.print(val);	        
	Serial.print("\tV="); Serial.println(fV);	        

	delay(1000);
}
