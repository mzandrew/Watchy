#include <string>
#include "Watchy_mza.h"

#define DARKMODE true
#define TWELVEHOURMODE
#define RESETSTEPSEVERYDAY

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

WatchyMZA::WatchyMZA(){} //constructor

void WatchyMZA::drawWatchFace(){
    display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    drawTime();
    drawDate();
    drawSteps();
    drawWeather();
    drawBattery();
    display.drawBitmap(120, 77, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    if(BLE_CONFIGURED){
        display.drawBitmap(100, 75, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

// modified 2021-07-18 by mza to have the option for 12-hour time (must change the "xadvance" to match that for 0-9 in the font .h file)
// modified 2021-07-18 to have the option to reset the step count every day
void WatchyMZA::drawTime(){
    display.setFont(&DSEG7_Classic_Bold_53_prime);
    display.setCursor(5, 53+5);
    uint8_t minute = currentTime.Minute;
    uint8_t hour = currentTime.Hour;
#ifdef RESETSTEPSEVERYDAY
    uint32_t stepCount = sensor.getCounter(); // potentially upload this somewhere...
    if (hour==0 && minute==0) {
        sensor.resetStepCounter();
    }
#endif
#ifdef TWELVEHOURMODE
    std::string ampm = int(hour/12) ? "pm" : "am";
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

void WatchyMZA::drawDate(){
    display.setFont(&Seven_Segment10pt7b);
    int16_t  x1, y1;
    uint16_t w, h;
    String dayOfWeek = dayStr(currentTime.Wday);
    display.getTextBounds(dayOfWeek, 5, 85, &x1, &y1, &w, &h);
    display.setCursor(85 - w, 85);
    display.println(dayOfWeek);
    String month = monthShortStr(currentTime.Month);
    display.getTextBounds(month, 60, 110, &x1, &y1, &w, &h);
    display.setCursor(85 - w, 110);
    display.println(month);
    display.setFont(&DSEG7_Classic_Bold_25);
    display.setCursor(5, 120);
    if(currentTime.Day < 10){
    display.print("0");      
    }     
    display.println(currentTime.Day);
    display.setCursor(5, 150);
    display.println(currentTime.Year + YEAR_OFFSET);// offset from 1970, since year is stored in uint8_t
}

void WatchyMZA::drawSteps(){
    uint32_t stepCount = sensor.getCounter();
    display.drawBitmap(10, 165, steps, 19, 23, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setCursor(35, 190);
    display.println(stepCount);
}

void WatchyMZA::drawBattery(){
    display.drawBitmap(154, 73, battery, 37, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.fillRect(159, 78, 27, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);//clear battery segments
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if(VBAT > 4.1){
        batteryLevel = 3;
    }
    else if(VBAT > 3.95 && VBAT <= 4.1){
        batteryLevel = 2;
    }
    else if(VBAT > 3.80 && VBAT <= 3.95){
        batteryLevel = 1;
    }    
    else if(VBAT <= 3.80){
        batteryLevel = 0;
    }
    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        display.fillRect(159 + (batterySegments * BATTERY_SEGMENT_SPACING), 78, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

void WatchyMZA::drawWeather(){
    weatherData currentWeather = getWeatherData();
    int8_t temperature = currentWeather.temperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;   
    display.setFont(&DSEG7_Classic_Regular_39);
    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(String(temperature), 100, 150, &x1, &y1, &w, &h);
    display.setCursor(155 - w, 150);
    display.println(temperature);
    display.drawBitmap(165, 110, strcmp(TEMP_UNIT, "metric") == 0 ? celsius : fahrenheit, 26, 20, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
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
    display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}

