// last updated 2021-07-27 by mza
#include "Watchy_mza.h"
#include "DSEG7_Classic_Bold_22.h"
#include "DSEG14_Classic_25.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Bold_53_prime.h"

#define DARKMODE true
#define TWELVEHOURMODE
#define RESETSTEPSEVERYDAY

#define DISPLAY_HEIGHT (200)
#define DISPLAY_WIDTH  (200)

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

#define X_POSITION_WEATHER (10)
#define X_POSITION_WIFI    (65)
#define X_POSITION_BLE     (90)

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
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "secrets.h" // WLAN_SSID, WLAN_PASS, AIO_USERNAME, AIO_FEED, AIO_KEY
#define AIO_SERVER     "io.adafruit.com"
#define AIO_SERVERPORT 8883

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

int WatchyMZA::setupMQTT() {
	Serial.print("Connecting to " WLAN_SSID "...");
//	delay(1000);
	WiFi.begin(WLAN_SSID, WLAN_PASS);
	delay(500);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.print("  IP address: "); Serial.println(WiFi.localIP());
	client.setCACert(adafruitio_root_ca); // Set Adafruit IO's root CA
	return WiFi.status()==WL_CONNECTED;
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
	drawWeather();
	drawBattery();
//	display.drawBitmap(X_POSITION_WIFI, Y_POSITION_WIFI, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
//	if(BLE_CONFIGURED){
//		display.drawBitmap(X_POSITION_BLE, Y_POSITION_BLE, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
//	}
	uploadStepsAndClear();
	drawSteps();
}

uint32_t oldStepCount = 0;

// modified 2021-07-18 by mza to have the option for 12-hour time (must change the "xadvance" to match that for 0-9 in the font .h file)
// modified 2021-07-18 to have the option to reset the step count every day
// modified 2021-07-26 to publish yesterday's step count on an adafruit IO feed before clearing it
void WatchyMZA::drawTime(){
    display.setFont(&DSEG7_Classic_Bold_53_prime);
    display.setCursor(5, Y_POSITION_TIME);
    uint8_t minute = currentTime.Minute;
    uint8_t hour = currentTime.Hour;
#ifdef TWELVEHOURMODE
    String ampm = int(hour/12) ? "pm" : "am";
//  0,1,2,3,4,5,6,7,8,9,10,11  12,13,14,15,16,17,18,19,20,21,22,23 24-hour-mode
// 12,1,2,3,4,5,6,7,8,9,10,11  12, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11 12-hour-mode
    hour %= 12;
    if (hour==0) { hour = 12; }
    if(hour < 10){ display.print(" "); }
#else
    if(hour < 10){ display.print("0"); }
#endif
    display.print(hour);
    display.print(":");
    if(minute < 10){ display.print("0"); }
    display.println(minute);
#ifdef RESETSTEPSEVERYDAY
	if (hour==0 && minute==0) {
		oldStepCount = sensor.getCounter();
	}
#endif
}

void WatchyMZA::uploadStepsAndClear() {
	if (oldStepCount) {
		if (setupMQTT() && MQTT_connect()) {
			feed.publish(oldStepCount); // upload this somewhere
			mqtt.disconnect();
			WiFi.disconnect();
			Serial.print("published yesterday's step count: "); Serial.println(oldStepCount);
			sensor.resetStepCounter();
			oldStepCount = 0;
		} else {
			Serial.println("couldn't publish yesterday's step count!");
		}
	}
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

void WatchyMZA::drawWeather(){
    weatherData currentWeather = getWeatherData();
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
    if(weatherConditionCode > 801){//Cloudy
    weatherIcon = cloudy;
    }else if(weatherConditionCode == 801){//Few Clouds
    weatherIcon = cloudsun;  
    }else if(weatherConditionCode == 800){//Clear
    weatherIcon = sunny;  
    }else if(weatherConditionCode >=700){//Atmosphere
    weatherIcon = cloudy; 
    }else if(weatherConditionCode >=600){//Snow
    weatherIcon = snow;
    }else if(weatherConditionCode >=500){//Rain
    weatherIcon = rain;  
    }else if(weatherConditionCode >=300){//Drizzle
    weatherIcon = rain;
    }else if(weatherConditionCode >=200){//Thunderstorm
    weatherIcon = rain; 
    }else
    return;
    display.drawBitmap(X_POSITION_WEATHER, Y_POSITION_WEATHER, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}

