#ifndef WATCHY_MZA_H
#define WATCHY_MZA_H

//#define ARDUINO_WATCHY_V10
//#define ARDUINO_WATCHY_V15
//#define ARDUINO_WATCHY_V20

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Watchy.h>
#include "icons.h"

class WatchyMZA : public Watchy {
	public:
		using Watchy::Watchy;
		void showWatchFace(bool partialRefresh);
		void drawWatchFace();
		void drawTime();
		void drawDate();
		void drawDayName();
		void drawSteps();
		void drawOldSteps();
		void drawWeather();
		void drawBattery();
		int connectWiFi();
		void disconnectWiFi();
		int MQTT_connect();
		void upload_old_step_count_and_clear();
		void upload_step_count_and_clear();
		uint32_t uploadSteps(uint32_t);
		void setTimeViaNTP();
		void getWeatherData();
		void init(String = "");
		void handleButtonPress();
	private:
		static uint16_t _readRegister(uint8_t, uint8_t, uint8_t *, uint16_t);
		static uint16_t _writeRegister(uint8_t, uint8_t, uint8_t *, uint16_t);
		void _bmaConfig();
};

#endif

