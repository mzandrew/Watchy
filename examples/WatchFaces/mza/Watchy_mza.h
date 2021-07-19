#ifndef WATCHY_MZA_H
#define WATCHY_MZA_H

#include <Watchy.h>
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "DSEG7_Classic_Bold_53_prime.h"
#include "icons.h"

class WatchyMZA : public Watchy{
    public:
        WatchyMZA();
        void drawWatchFace();
        void drawTime();
        void drawDate();
        void drawSteps();
        void drawWeather();
        void drawBattery();
};

#endif

