/*
 Name:		ExtInterruptGen.ino
 Created:	1/23/2018 4:46:46 PM
 Author:	sitymchenko
*/

int nOutPin = 9;
int nMaxScale = 255;
int nPercentOfScale = 50;

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(nOutPin, OUTPUT);
	analogWrite(nOutPin, nMaxScale*nPercentOfScale / 100);
}

// the loop function runs over and over again until power down or reset
void loop() {
	//analogWrite(nOutPin, nMaxScale*nPercentOfScale/100);
}
