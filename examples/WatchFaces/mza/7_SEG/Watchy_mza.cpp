// last updated 2022-06-03 by mza

#include "Watchy_mza.h"
#include "DSEG7_Classic_Bold_22.h"
#include "DSEG14_Classic_25.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Bold_53_prime.h"
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "secrets.h" // WLAN_SSID, WLAN_PASS, AIO_USERNAME, AIO_FEED, AIO_KEY, UTC_OFFSET_HOURS, MY_CITY_NAME, COUNTRY_CODE, TEMP_UNIT in secrets.h
#define AIO_SERVER     "io.adafruit.com"
#define AIO_SERVERPORT 8883

#define DARKMODE true
#define TWELVEHOURMODE
#define RESETSTEPSEVERYDAY
//#define WEATHER
//#define DEBUG
#define TIME_SET_DELAY_S (3) // positive fudge factor to allow for upload time, etc. (seconds, YMMV)
#define TIME_SET_DELAY_MS (1850) // extra negative fudge factor to tweak time (milliseconds, should be at least 1000 to wait for the ntp packet response)

#define BATTERY_SEGMENT_WIDTH   (7)
#define BATTERY_SEGMENT_HEIGHT  (11)
#define BATTERY_SEGMENT_SPACING (9)
#define WEATHER_ICON_WIDTH      (48)
#define WEATHER_ICON_HEIGHT     (32)
#define DATE_HEIGHT             (22)
#define TIME_HEIGHT             (53)
#define DAY_NAME_HEIGHT         (25)
#define TEMPERATURE_HEIGHT      (25)
#define HEIGHT_STEPS            (25)

#define Y_POSITION_DATE        (DATE_HEIGHT+6)
#define Y_POSITION_TIME        (Y_POSITION_DATE+TIME_HEIGHT+8)
#define Y_POSITION_DAY_NAME    (Y_POSITION_TIME+DAY_NAME_HEIGHT+6)
#define Y_POSITION_OTHER       (Y_POSITION_DAY_NAME+8)
#define Y_POSITION_WEATHER     (Y_POSITION_OTHER)
#define Y_POSITION_WIFI        (Y_POSITION_OTHER+10)
#define Y_POSITION_BLE         (Y_POSITION_OTHER+10)
#define Y_POSITION_TEMPERATURE (Y_POSITION_OTHER+TEMPERATURE_HEIGHT+10)
#define Y_POSITION_STEPS       (167)
#define Y_POSITION_BATTERY     (Y_POSITION_TEMPERATURE+BATTERY_SEGMENT_HEIGHT-2)

#ifdef TWELVEHOURMODE
	#define X_POSITION_TIME    (0)
#else
	#define X_POSITION_TIME    (5)
#endif
#define X_POSITION_WEATHER (10)
#define X_POSITION_WIFI    (65)
#define X_POSITION_BLE     (90)

extern RTC_DATA_ATTR weatherData currentWeather;
extern RTC_DATA_ATTR int weatherIntervalCounter;
RTC_DATA_ATTR uint32_t oldStepCount = 0;
RTC_DATA_ATTR uint32_t reallyOldStepCount = 0;

RTC_DATA_ATTR bool wifi_active = false;

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets
WiFiUDP UDP;

void sendNTPpacket(IPAddress &address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(packetBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

// sendNTPpacket and NTP function code are from Arduino/libraries/WiFi101/examples/WiFiUdpNtpClient/WiFiUdpNtpClient.ino
void WatchyMZA::setTimeViaNTP() {
	if (connectWiFi()) {
		unsigned int localPort = 2390; // local port to listen for UDP packets
		IPAddress timeServer(132, 163, 97, 4); // ntp1.glb.nist.gov NTP server
		UDP.begin(localPort);
		sendNTPpacket(timeServer); // send an NTP packet to a time server
		delay(TIME_SET_DELAY_MS);
		if ( UDP.parsePacket() ) {
			UDP.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
			unsigned long secsSince1900 = highWord << 16 | lowWord;
			const unsigned long seventyYears = 2208988800UL;
			time_t epoch = secsSince1900 - seventyYears;
			char timestring[20];
	//		Serial.print("Seconds since Jan 1 1900 = " ); Serial.println(secsSince1900);
	//		Serial.print("epoch time = "); Serial.println(epoch);
	//		sprintf(timestring, "%02d:%02d:%02d", (epoch%86400)/3600, (epoch%3600)/60, epoch%60);
	//		Serial.print("UTC time is "); Serial.println(timestring);
			epoch += (UTC_OFFSET_HOURS) * 3600; // UTC_OFFSET_HOURS is a signed quantity
	//		Serial.print("epoch time (local) = "); Serial.println(epoch);
			sprintf(timestring, "%02ld:%02ld:%02ld", (epoch%86400)/3600, (epoch%3600)/60, epoch%60);
			Serial.print("ntp server responded with "); Serial.println(timestring);
			const time_t fudge(TIME_SET_DELAY_S);
			epoch += fudge;
			sprintf(timestring, "%02ld:%02ld:%02ld", (epoch%86400)/3600, (epoch%3600)/60, epoch%60);
			Serial.print("setting time to "); Serial.println(timestring);
			tmElements_t epoch_tm;
			breakTime(epoch, epoch_tm);
			// tmElements_t tm = localtime(&epoch); // error: conversion from 'tm*' to non-scalar type 'tmElements_t' requested
			//tm *epoch_tm = localtime(&epoch);
			// tm epoch_tm = localtime(epoch); // error: invalid conversion from 'time_t' {aka 'long int'} to 'const time_t*' {aka 'const long int*'} [-fpermissive]
			// tm epoch_tm = localtime(&epoch); // error: conversion from 'tm*' to non-scalar type 'tm' requested
			// error: no matching function for call to 'WatchyRTC::set(tm*&)'
			RTC.set(epoch_tm); // set time on RTC
		} else {
			Serial.println("didn't get a response");
		}
		disconnectWiFi();
	}
}

// setup and MQTT_connect below are from https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/examples/adafruitio_anon_time_esp8266/adafruitio_anon_time_esp8266.ino
/***********************************************************************
  Adafruit MQTT Library ESP32 Adafruit IO SSL/TLS example
  Use the latest version of the ESP32 Arduino Core:
    https://github.com/espressif/arduino-esp32
  Works great with Adafruit Huzzah32 Feather and Breakout Board:
    https://www.adafruit.com/product/3405
    https://www.adafruit.com/products/4172
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Tony DiCola for Adafruit Industries.
  Modified by Brent Rubell for Adafruit Industries
  MIT license, all text above must be included in any redistribution
 **********************************************************************/

// WiFiFlientSecure for SSL/TLS support
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/" AIO_FEED);

// io.adafruit.com root CA
const char* adafruitio_root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
    "-----END CERTIFICATE-----\n";

int WatchyMZA::connectWiFi() {
	if (wifi_active) {
		Serial.print("  IP address: "); Serial.println(WiFi.localIP());
		return 1;
	}
	Serial.print("Connecting to " WLAN_SSID "... ");
	WiFi.begin(WLAN_SSID, WLAN_PASS);
#define MAX_RETRIES (60)
	uint8_t retries = MAX_RETRIES;
	while (WiFi.status() != WL_CONNECTED) {
		delay(100);
		Serial.print(".");
		retries--;
		if (retries == 0) { Serial.println("  failed"); return 0; }
	}
	if (WiFi.status()==WL_CONNECTED) {
		Serial.print("  IP address: "); Serial.println(WiFi.localIP());
		client.setCACert(adafruitio_root_ca); // Set Adafruit IO's root CA
		int tenths = MAX_RETRIES - retries;
		Serial.print("wifi connection took "); Serial.print(tenths/10.); Serial.println(" seconds");
		wifi_active = true;
		return tenths;
	} else {
		return 0;
	}
}

void WatchyMZA::disconnectWiFi() {
	if (wifi_active) {
		WiFi.disconnect();
		WiFi.mode(WIFI_OFF);
		Serial.println("wifi disconnected");
		wifi_active = false;
	}
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
int WatchyMZA::MQTT_connect() {
	int8_t ret;
	if (mqtt.connected()) { return 1; } // Stop if already connected.
	Serial.print("Connecting to MQTT... ");
	uint8_t retries = 3;
	while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
		Serial.println(mqtt.connectErrorString(ret));
		mqtt.disconnect();
		delay(5000);  // wait 5 seconds
		Serial.println("Retrying MQTT connection... ");
		retries--;
		if (retries == 0) { return 0; }
	}
	Serial.println("Connected!");
	return 1;
}

WatchyMZA::WatchyMZA(){} //constructor

void WatchyMZA::drawWatchFace(){
	display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
	display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
	drawDate();
	drawTime();
	drawDayName();
	drawBattery();
//	display.drawBitmap(X_POSITION_WIFI, Y_POSITION_WIFI, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
//	if(BLE_CONFIGURED){
//		display.drawBitmap(X_POSITION_BLE, Y_POSITION_BLE, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
//	}
#ifdef RESETSTEPSEVERYDAY
#ifdef DEBUG
	if (currentTime.Minute==58) {
//	if (currentTime.Minute%10==1) {
#else
	if (currentTime.Hour==23 && currentTime.Minute==58) {
#endif
		if (reallyOldStepCount) {
			reallyOldStepCount = uploadSteps(reallyOldStepCount);
		}
		if (reallyOldStepCount) {
			Serial.print("current really old step count: "); Serial.println(reallyOldStepCount);
		}
	}
#ifdef DEBUG
	if (currentTime.Minute==59) {
//	if (currentTime.Minute%10==2) {
#else
	if (currentTime.Hour==23 && currentTime.Minute==59) {
#endif
		oldStepCount = sensor.getCounter();
		if (oldStepCount) {
			sensor.resetStepCounter();
			oldStepCount = uploadSteps(oldStepCount);
			if (oldStepCount) {
				reallyOldStepCount += oldStepCount;
			}
		}
	}
#endif
	drawSteps();
	#ifdef WEATHER
		if (weatherIntervalCounter >= WEATHER_UPDATE_INTERVAL) {
			getWeatherData();
			weatherIntervalCounter = 0;
		} else {
			weatherIntervalCounter++;
		}
		drawWeather();
	#endif
#ifdef DEBUG
	if (currentTime.Minute==57) {
//	if (currentTime.Minute%10==0) {
#else
	if (currentTime.Hour==23 && currentTime.Minute==57) {
#endif
		setTimeViaNTP();
	}
	disconnectWiFi();
	btStop();
}

// modified 2021-07-18 by mza to have the option for 12-hour time (must change the "xadvance" to match that for 0-9 in the font .h file)
// modified 2021-07-18 to have the option to reset the step count every day
// modified 2021-07-26 to publish yesterday's step count on an adafruit IO feed before clearing it
void WatchyMZA::drawTime(){
	display.setFont(&DSEG7_Classic_Bold_53_prime);
	display.setCursor(X_POSITION_TIME, Y_POSITION_TIME);
	uint8_t minute = currentTime.Minute;
	uint8_t hour = currentTime.Hour;
	uint8_t second = currentTime.Second;
	char timestring[12];
#ifdef TWELVEHOURMODE
	String ampm = int(hour/12) ? "pm" : "am";
//  0,1,2,3,4,5,6,7,8,9,10,11  12,13,14,15,16,17,18,19,20,21,22,23 24-hour-mode
// 12,1,2,3,4,5,6,7,8,9,10,11  12, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11 12-hour-mode
	hour %= 12; if (hour==0) { hour = 12; }
	sprintf(timestring, "%2d:%02d", hour, minute);
#else
	sprintf(timestring, "%02d:%02d", hour, minute);
#endif
	display.print(timestring);
	sprintf(timestring, "%2d:%02d:%02d %s", hour, minute, second, ampm.c_str());
	Serial.println(timestring);
}

uint32_t WatchyMZA::uploadSteps(uint32_t steps) {
	if (steps) {
		Serial.print("current step count: "); Serial.println(steps);
		if (connectWiFi() && MQTT_connect()) {
			feed.publish(steps); // upload this somewhere
			mqtt.disconnect();
			disconnectWiFi();
			Serial.print("published step count: "); Serial.println(steps);
			steps = 0;
		} else {
			Serial.println("couldn't publish step count! "); Serial.println(steps);
		}
	}
	return steps;
}

void WatchyMZA::drawDayName(){
	String dayOfWeek = dayStr(currentTime.Wday);
	display.setFont(&DSEG14_Classic_Regular_25);
	int16_t  x1, y1;
	uint16_t w, h;
	display.getTextBounds(dayOfWeek, 0, Y_POSITION_DAY_NAME, &x1, &y1, &w, &h);
	display.setCursor((DISPLAY_WIDTH-w)/2, Y_POSITION_DAY_NAME);
	display.println(dayOfWeek);
}

void WatchyMZA::drawDate(){
	uint16_t year = YEAR_OFFSET + currentTime.Year; // offset from 1970, since year is stored in uint8_t
	uint8_t month_of_year = currentTime.Month;
	uint8_t day_of_month = currentTime.Day;
	char date_string[20];
	sprintf(date_string, "%04d-%02d-%02d", year, month_of_year, day_of_month);
	display.setFont(&DSEG7_Classic_Bold_22);
	display.setCursor(6, Y_POSITION_DATE);
	display.println(date_string);
}

void WatchyMZA::drawSteps(){
    uint32_t stepCount = sensor.getCounter();
    display.drawBitmap(10, Y_POSITION_STEPS, steps, 19, 23, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setCursor(35, Y_POSITION_STEPS+HEIGHT_STEPS);
    display.setFont(&DSEG7_Classic_Bold_25);
    display.println(stepCount);
}

void WatchyMZA::drawBattery(){
    display.drawBitmap(154, Y_POSITION_BATTERY, battery, 37, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.fillRect(159, Y_POSITION_BATTERY+5, 27, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);//clear battery segments
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if(VBAT > 4.1){
        batteryLevel = 3;
    } else if(VBAT > 3.95 && VBAT <= 4.1){
        batteryLevel = 2;
    } else if(VBAT > 3.80 && VBAT <= 3.95){
        batteryLevel = 1;
    } else if(VBAT <= 3.80){
        batteryLevel = 0;
    }
    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        display.fillRect(159 + (batterySegments * BATTERY_SEGMENT_SPACING), Y_POSITION_BATTERY+5, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

void WatchyMZA::getWeatherData() {
	if (connectWiFi()) { // Use Weather API for live data if WiFi is connected
		Serial.print("grabbing weather data for " MY_CITY_NAME "...");
		HTTPClient http;
		http.setConnectTimeout(3000); // 3 second max timeout
		String weatherQueryURL = String(OPENWEATHERMAP_URL) + String(MY_CITY_NAME) + String(",") + String(COUNTRY_CODE) + String("&units=") + String(TEMP_UNIT) + String("&appid=") + String(OPENWEATHERMAP_APIKEY);
		http.begin(weatherQueryURL.c_str());
		int httpResponseCode = http.GET();
		if (httpResponseCode == 200) {
			String payload = http.getString();
			JSONVar responseObject = JSON.parse(payload);
			currentWeather.temperature = int(responseObject["main"]["temp"]);
			currentWeather.weatherConditionCode = int(responseObject["weather"][0]["id"]);
			Serial.println(" done");
		} else {
			Serial.println(" error fetching weather data");
		}
		http.end();
		disconnectWiFi();
	} else { // No WiFi, use RTC Temperature
		uint8_t temperature = RTC.temperature() / 4; //celsius
		if(strcmp(TEMP_UNIT, "imperial") == 0) {
			temperature = temperature * 9. / 5. + 32.; //fahrenheit
		}
		currentWeather.temperature = temperature;
		currentWeather.weatherConditionCode = 800;
	}
}

void WatchyMZA::drawWeather(){
	int8_t temperature = currentWeather.temperature;
	int16_t weatherConditionCode = currentWeather.weatherConditionCode;
	display.setFont(&DSEG7_Classic_Bold_25);
	int16_t  x1, y1;
	uint16_t w, h;
	display.getTextBounds(String(temperature), 0, Y_POSITION_TEMPERATURE, &x1, &y1, &w, &h);
	display.setCursor(155 - w, Y_POSITION_TEMPERATURE);
	display.println(temperature);
	display.drawBitmap(165, Y_POSITION_TEMPERATURE-TEMPERATURE_HEIGHT, strcmp(TEMP_UNIT, "metric") == 0 ? celsius : fahrenheit, 26, 20, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
	const unsigned char* weatherIcon;
	//https://openweathermap.org/weather-conditions
	if (weatherConditionCode > 801) {//Cloudy
		weatherIcon = cloudy;
	} else if (weatherConditionCode == 801) {//Few Clouds
		weatherIcon = cloudsun;
	} else if (weatherConditionCode == 800) {//Clear
		weatherIcon = sunny;
	} else if (weatherConditionCode >=700) {//Atmosphere
		weatherIcon = cloudy;
	} else if (weatherConditionCode >=600) {//Snow
		weatherIcon = snow;
	} else if (weatherConditionCode >=500) {//Rain
		weatherIcon = rain;
	} else if (weatherConditionCode >=300) {//Drizzle
		weatherIcon = rain;
	} else if (weatherConditionCode >=200) {//Thunderstorm
		weatherIcon = rain;
	} else {
		return;
	}
	display.drawBitmap(X_POSITION_WEATHER, Y_POSITION_WEATHER, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}

void WatchyMZA::init(String datetime) {
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause(); // get wake up reason
	Wire.begin(SDA, SCL); // init i2c
	switch (wakeup_reason) {
		#ifdef ESP_RTC
		case ESP_SLEEP_WAKEUP_TIMER: // ESP Internal RTC
			if(guiState == WATCHFACE_STATE){
				RTC.read(currentTime);
				currentTime.Minute++;
				tmElements_t tm;
				tm.Month = currentTime.Month;
				tm.Day = currentTime.Day;
				tm.Year = currentTime.Year;
				tm.Hour = currentTime.Hour;
				tm.Minute = currentTime.Minute;
				tm.Second = 0;
				//time_t t = makeTime(tm); // Watchy v1.2.8 and up does this step for us
				RTC.set(tm);
				RTC.read(currentTime);
				showWatchFace(true); // partial updates on tick
			}
			break;
		#endif
		case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
			RTC.clearAlarm(); // resets the alarm flag in the RTC
			if(guiState == WATCHFACE_STATE){
				RTC.read(currentTime);
//				Serial.println("partial update");
				showWatchFace(true); // partial updates on tick
			}
			break;
		case ESP_SLEEP_WAKEUP_EXT1: // button Press
			handleButtonPress();
			break;
		default: //reset
			#ifndef ESP_RTC
			RTC.config(datetime);
			#endif
			_bmaConfig();
//			Serial.println("full update");
			showWatchFace(false); //full update on reset
			break;
	}
	deepSleep();
}

uint16_t WatchyMZA::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)address, (uint8_t)len);
    uint8_t i = 0;
    while (Wire.available()) {
        data[i++] = Wire.read();
    }
    return 0;
}

uint16_t WatchyMZA::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data, len);
    return (0 !=  Wire.endTransmission());
}

void WatchyMZA::_bmaConfig() {
	if (sensor.begin(_readRegister, _writeRegister, delay) == false) {
		return; //fail to init BMA
	}
	// Accel parameter structure
	Acfg cfg;
	/*!
		Output data rate in Hz, Optional parameters:
			- BMA4_OUTPUT_DATA_RATE_0_78HZ
			- BMA4_OUTPUT_DATA_RATE_1_56HZ
			- BMA4_OUTPUT_DATA_RATE_3_12HZ
			- BMA4_OUTPUT_DATA_RATE_6_25HZ
			- BMA4_OUTPUT_DATA_RATE_12_5HZ
			- BMA4_OUTPUT_DATA_RATE_25HZ
			- BMA4_OUTPUT_DATA_RATE_50HZ
			- BMA4_OUTPUT_DATA_RATE_100HZ
			- BMA4_OUTPUT_DATA_RATE_200HZ
			- BMA4_OUTPUT_DATA_RATE_400HZ
			- BMA4_OUTPUT_DATA_RATE_800HZ
			- BMA4_OUTPUT_DATA_RATE_1600HZ
	*/
	cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
	/*!
		G-range, Optional parameters:
			- BMA4_ACCEL_RANGE_2G
			- BMA4_ACCEL_RANGE_4G
			- BMA4_ACCEL_RANGE_8G
			- BMA4_ACCEL_RANGE_16G
	*/
	cfg.range = BMA4_ACCEL_RANGE_2G;
	/*!
		Bandwidth parameter, determines filter configuration, Optional parameters:
			- BMA4_ACCEL_OSR4_AVG1
			- BMA4_ACCEL_OSR2_AVG2
			- BMA4_ACCEL_NORMAL_AVG4
			- BMA4_ACCEL_CIC_AVG8
			- BMA4_ACCEL_RES_AVG16
			- BMA4_ACCEL_RES_AVG32
			- BMA4_ACCEL_RES_AVG64
			- BMA4_ACCEL_RES_AVG128
	*/
	cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
	/*! Filter performance mode , Optional parameters:
		- BMA4_CIC_AVG_MODE
		- BMA4_CONTINUOUS_MODE
	*/
	cfg.perf_mode = BMA4_CONTINUOUS_MODE;
	// Configure the BMA423 accelerometer
	sensor.setAccelConfig(cfg);
	// Enable BMA423 accelerometer
	// Warning : Need to use feature, you must first enable the accelerometer
	// Warning : Need to use feature, you must first enable the accelerometer
	sensor.enableAccel();
	struct bma4_int_pin_config config ;
	config.edge_ctrl = BMA4_LEVEL_TRIGGER;
	config.lvl = BMA4_ACTIVE_HIGH;
	config.od = BMA4_PUSH_PULL;
	config.output_en = BMA4_OUTPUT_ENABLE;
	config.input_en = BMA4_INPUT_DISABLE;
	// The correct trigger interrupt needs to be configured as needed
	sensor.setINTPinConfig(config, BMA4_INTR1_MAP);
	struct bma423_axes_remap remap_data;
	remap_data.x_axis = 1;
	remap_data.x_axis_sign = 0xFF;
	remap_data.y_axis = 0;
	remap_data.y_axis_sign = 0xFF;
	remap_data.z_axis = 2;
	remap_data.z_axis_sign = 0xFF;
	// Need to raise the wrist function, need to set the correct axis
	sensor.setRemapAxes(&remap_data);
	// Enable BMA423 isStepCounter feature
	sensor.enableFeature(BMA423_STEP_CNTR, true);
	// Enable BMA423 isTilt feature
	sensor.enableFeature(BMA423_TILT, true);
	// Enable BMA423 isDoubleClick feature
	sensor.enableFeature(BMA423_WAKEUP, true);
	// Reset steps
//	sensor.resetStepCounter();
	// Turn on feature interrupt
	sensor.enableStepCountInterrupt();
	sensor.enableTiltInterrupt();
	// It corresponds to isDoubleClick interrupt
	sensor.enableWakeupInterrupt();
}

