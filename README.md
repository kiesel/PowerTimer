PowerTimer
==========

PowerTimer is an ESP8266 based application that controls configurable devices over a 433 Mhz remote control switch. In my case, it control outlets that power various consumers in my household.

It makes use of these other libraries:

* [Time](https://github.com/PaulStoffregen/Time)
* [TimeAlarms](https://github.com/PaulStoffregen/TimeAlarms)
* [NTPClient](https://github.com/arduino-libraries/NTPClient)
* [RCSwitch](https://github.com/sui77/rc-switch)

Currently, there's only support for:
* daily, fixed time alarms
* Intertechno power outlets
* a *breathing* status LED

... more is up to what I need:).

Adaption
--------

The program can be adapted by changing the configuration in `constants.h`, where wifi config & some other stuff lives; the alarm list and socket configuration needs to be changed in `config.h` prior to recompilation. Yes, the configuration is compiled into the program at this time.
