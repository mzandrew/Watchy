// last updated 2021-07-25 by mza
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

WatchyMZA::WatchyMZA(){} //constructor

void WatchyMZA::drawWatchFace(){
	display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
	display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
	drawDate();
	drawTime();
	drawDayName();
	drawWeather();
	drawSteps();
	drawBattery();
	display.drawBitmap(X_POSITION_WIFI, Y_POSITION_WIFI, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
	if(BLE_CONFIGURED){
		display.drawBitmap(X_POSITION_BLE, Y_POSITION_BLE, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
	}
}

// modified 2021-07-18 by mza to have the option for 12-hour time (must change the "xadvance" to match that for 0-9 in the font .h file)
// modified 2021-07-18 to have the option to reset the step count every day
void WatchyMZA::drawTime(){
    display.setFont(&DSEG7_Classic_Bold_53_prime);
    display.setCursor(5, Y_POSITION_TIME);
    uint8_t minute = currentTime.Minute;
    uint8_t hour = currentTime.Hour;
#ifdef RESETSTEPSEVERYDAY
    static uint32_t oldStepCount = 0;
    if (hour==0 && minute==0) {
        oldStepCount = sensor.getCounter(); // potentially upload this somewhere...
        sensor.resetStepCounter();
    }
#endif
#ifdef TWELVEHOURMODE
    String ampm = int(hour/12) ? "pm" : "am";
//  0,1,2,3,4,5,6,7,8,9,10,11  12,13,14,15,16,17,18,19,20,21,22,23 24-hour-mode
// 12,1,2,3,4,5,6,7,8,9,10,11  12, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11 12-hour-mode
    hour %= 12;
    if (hour==0) {
        hour = 12;
    }
    if(hour < 10){
        display.print(" ");
    }
#else
    if(hour < 10){
        display.print("0");
    }
#endif
    display.print(hour);
    display.print(":");
    if(minute < 10){
        display.print("0");
    }  
    display.println(minute);
}

void WatchyMZA::drawDayName(){
	String dayOfWeek = dayStr(currentTime.Wday);
	//String dayOfWeek = "ABCDEFGHI";
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

