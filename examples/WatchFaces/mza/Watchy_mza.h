#ifndef WATCHY_MZA_H
#define WATCHY_MZA_H

#include <Watchy.h>
#include "icons.h"

class WatchyMZA : public Watchy{
    public:
        WatchyMZA();
        void drawWatchFace();
        void drawTime();
        void drawDate();
        void drawDayName();
        void drawSteps();
        void drawWeather();
        void drawBattery();
};

#endif

