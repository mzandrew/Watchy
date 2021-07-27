#ifndef WATCHY_MZA_H
#define WATCHY_MZA_H

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Watchy.h"
#include "icons.h"

class WatchyMZA : public Watchy {
public:
	WatchyMZA();
		void drawWatchFace();
		void drawTime();
		void drawDate();
		void drawDayName();
		void drawSteps();
		void drawWeather();
		void drawBattery();
		int connectWiFi();
		int MQTT_connect();
		void uploadStepsAndClear();
		void setTimeViaNTP();
		void getWeatherData();
};

#endif

