To toggle between Dark Mode/Light Mode, change `#define DARKMODE true` in `Watchy_mza.cpp`

To use 12-hour mode, add `#define TWELVEHOURMODE` in `Watchy_mza.cpp`

To clear the step count every morning at midnight, add `#define RESETSTEPSEVERYDAY` in `Watchy_mza.cpp`

To upload yesterday's step count just before clearing it, define the following in `secrets.h`:
`WLAN_SSID`, `WLAN_PASS`, `AIO_USERNAME`, `AIO_FEED`, `AIO_KEY`

secrets.h should also contain `MY_CITY_NAME` and `TEMP_UNIT` to get appropriate weather updates

Sets time via NTP every morning at 1 minute past midnight.  Put your `UTC_OFFSET_HOURS` (a signed quantity) in `secrets.h`

platformio setup:
pip install -U platformio
platformio lib --global install 6864 1092

platformio compile:
pio run
pio run -e esp32dev
pio run -e debug

platformio compile & program:
pio run -e esp32dev -t upload
pio run -e debug -t upload

platformio serial monitor:
pio device monitor

