To toggle between Dark Mode/Light Mode, change `#define DARKMODE true` in `Watchy_mza.cpp`

To use 12-hour mode, add `#define TWELVEHOURMODE` in `Watchy_mza.cpp`

To clear the step count every morning at midnight, add `#define RESETSTEPSEVERYDAY` in `Watchy_mza.cpp`

To upload yesterday's step count just before clearing it, define the following in `secrets.h`:
`WLAN_SSID`, `WLAN_PASS`, `AIO_USERNAME`, `AIO_FEED`, `AIO_KEY`

