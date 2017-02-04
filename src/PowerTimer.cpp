#include <Arduino.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <ESP8266Wifi.h>
#include <SPI.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <RCSwitch.h>

#include "constants.h"

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, NTP_SERVER, NTP_TIME_OFFSET, NTP_DEFAULT_SYNC);

RCSwitch rcswitch = RCSwitch();

typedef struct {
  char family;
  int group;
  int number;
} socketmapping_t;

socketmapping_t sockets[] = {
  socketmapping_t{'a', 1, 1},
  socketmapping_t{'a', 1, 2},
  socketmapping_t{'a', 1, 3}
};

bool socket1 = true;
bool ledOn = true;

time_t updateTimeFromNTP() {
  Serial.print("Refresh clock from ntp: ");
  Serial.print(ntpClient.getFormattedTime());

  Serial.print(", epoch time is: ");
  Serial.println(ntpClient.getEpochTime());

  return ntpClient.getEpochTime();
}

void pulse() {
  digitalWrite(D0, ledOn ? HIGH : LOW);
  ledOn = !ledOn;
}

void heartbeat() {
  Serial.print("Heartbeat @ ");
  Serial.print(ntpClient.getFormattedTime());
  Serial.print(", millis = ");
  Serial.println(millis());
}

void socketN(int n, bool on) {
  Serial.print("Turning socket ");
  Serial.print(n);
  Serial.print(on ? " on" : " off");

  if (on) {
    rcswitch.switchOn(sockets[n].family, sockets[n].group, sockets[n].number);
  } else {
    rcswitch.switchOff(sockets[n].family, sockets[n].group, sockets[n].number);
  }

  Serial.println(" ... done.");
}

void socket1PowerOn() { socketN(0, true); }
void socket2PowerOn() { socketN(1, true); }
void socket3PowerOn() { socketN(2, true); }
void socket1PowerOff() { socketN(0, false); }
void socket2PowerOff() { socketN(1, false); }
void socket3PowerOff() { socketN(2, false); }

void socket2Alternate() {
  Serial.print("Alternation: ");
  socketN(1, socket1);
  socket1 = !socket1;
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  pinMode(D0, OUTPUT);

  rcswitch.enableTransmit(D2);
  delay(250);

  Serial.print("Attempting to connect to SSID ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  Serial.print("Connected to SSID ");
  Serial.print(WLAN_SSID);
  Serial.print(" @ ");
  Serial.println(WiFi.localIP());

  ntpClient.begin();
  ntpClient.update();

  Serial.println("Registering NTP update service ...");
  setSyncProvider(updateTimeFromNTP);
  setSyncInterval(60);

  Serial.println("Registering alarms ...");
  Alarm.timerRepeat(3, pulse);
  Alarm.timerRepeat(15, heartbeat);

  // Register triggers for babyphone station
  Alarm.alarmRepeat(12, 15, 0, socket1PowerOn);
  Alarm.alarmRepeat(18, 30, 0, socket1PowerOn);
  Alarm.alarmRepeat(10, 30, 0, socket1PowerOff);

  // Register triggers for night illumination socket
  Alarm.alarmRepeat(17, 30, 0, socket2PowerOn);
  Alarm.alarmRepeat(9, 0, 0, socket2PowerOff);

  // Register triggers for living room lamp
  Alarm.alarmRepeat(17, 30, 0, socket3PowerOn);
  Alarm.alarmRepeat(0, 30, 0, socket3PowerOff);

  Serial.println("Boot up sequence finished.");
}

void loop() {

  // Let NTP client sync
  ntpClient.update();

  // Alarms handler
  Alarm.delay(1000);
}
