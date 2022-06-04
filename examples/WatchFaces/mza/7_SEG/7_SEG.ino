//#include "Watchy_7_SEG.h"
#include "Watchy_mza.h"
#include "settings.h"

//Watchy7SEG watchy(settings);
WatchyMZA watchy(settings);

void setup(){
	Serial.begin(115200);
	delay(10);
	// this calls (virtual) drawWatchFace() and then deepsleep():
	watchy.init();
}

void loop(){}



