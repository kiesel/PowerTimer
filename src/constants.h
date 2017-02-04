#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>

#define WLAN_SSID ""
#define WLAN_PASS ""

#define NTP_SERVER "europe.pool.ntp.org"
#define NTP_TIME_OFFSET 3600
#define NTP_DEFAULT_SYNC 600000 // all 10 minutes

const int ILED = D4;
const int RCSWITCH_PIN = D3;

#endif
