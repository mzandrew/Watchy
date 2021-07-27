#include "Watchy_mza.h"

WatchyMZA watchy;

void setup() {
	Serial.begin(115200);
	delay(10);
	watchy.init(); // this calls (virtual) drawWatchFace() and then deepsleep();
}

void loop() {}

