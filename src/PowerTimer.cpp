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
} socket_t;

socket_t sockets[] = {
  socket_t{'a', 1, 1},
  socket_t{'a', 1, 2},
  socket_t{'a', 1, 3}
};

typedef struct {
  AlarmID_t alarm_id;
  String name;
  bool on;
  socket_t *socket;
  int hour;
  int minute;
  int second;
} socket_timer_t;

bool socket1 = true;
bool ledOn = true;
const int ILED = D4;
const int RCSWITCH_PIN = D3;

socket_timer_t timers[] = {
  socket_timer_t{dtINVALID_ALARM_ID, "Livingroom lamp ON", true, &sockets[0], 17, 30, 0},
  socket_timer_t{dtINVALID_ALARM_ID, "Livingroom lamp OFF", false, &sockets[0], 0, 1, 0},
  socket_timer_t{dtINVALID_ALARM_ID, "Night illumination ON", true, &sockets[1], 17, 30, 0},
  socket_timer_t{dtINVALID_ALARM_ID, "Night illumination OFF", false, &sockets[1], 9, 0, 0},
  socket_timer_t{dtINVALID_ALARM_ID, "Babyphone Station Power ON", true, &sockets[2], 18, 30, 0},
  socket_timer_t{dtINVALID_ALARM_ID, "Babyphone Station Power OFF", false, &sockets[2], 10, 0, 0},
};

time_t updateTimeFromNTP() {
  Serial.print("Refresh clock from ntp: ");
  Serial.print(ntpClient.getFormattedTime());

  Serial.print(", epoch time is: ");
  Serial.println(ntpClient.getEpochTime());

  return ntpClient.getEpochTime();
}

void pulse() {
  digitalWrite(ILED, LOW);
  Alarm.delay(50);
  digitalWrite(ILED, HIGH);
}

void execute_socket_timer(socket_timer_t *socket_timer) {
  Serial.print("Executing alarm '");
  Serial.print(socket_timer->name);
  Serial.print("' on socket ");
  Serial.print(socket_timer->socket->family);
  Serial.print(", ");
  Serial.print(socket_timer->socket->group);
  Serial.print(", ");
  Serial.println(socket_timer->socket->number);

  if (socket_timer->on) {
    rcswitch.switchOn(socket_timer->socket->family, socket_timer->socket->group, socket_timer->socket->number);
  } else {
    rcswitch.switchOff(socket_timer->socket->family, socket_timer->socket->group, socket_timer->socket->number);
  }

  Serial.println(" ... done.");
}

void socket_timer_callback() {
  AlarmID_t alarm_id = Alarm.getTriggeredAlarmId();

  Serial.print("Triggered alarm is ");
  Serial.println(alarm_id);

  for (int i = 0; i < sizeof(timers) / sizeof(socket_timer_t); i++) {
    if (timers[i].alarm_id == alarm_id) {
      execute_socket_timer(&timers[i]);
      return;
    }
  }

  Serial.print("Unable to resolve alarm w/ id ");
  Serial.println(alarm_id);
}

void register_socket_timer(socket_timer_t *socket_timer) {
  AlarmID_t alarm_id = Alarm.alarmRepeat(socket_timer->hour, socket_timer->minute, socket_timer->second, socket_timer_callback);
  socket_timer->alarm_id = alarm_id;

  Serial.print("Registered alarm '");
  Serial.print(socket_timer->name);
  Serial.print("' w/ id ");
  Serial.println(alarm_id);
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  pinMode(ILED, OUTPUT);
  digitalWrite(ILED, HIGH);

  rcswitch.enableTransmit(RCSWITCH_PIN);
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
  Alarm.timerRepeat(2, pulse);

  for (int i = 0; i < sizeof(timers) / sizeof(socket_timer_t); i++) {
    register_socket_timer(&timers[i]);
  }

  Serial.println("Boot up sequence finished.");
}

void loop() {

  // Let NTP client sync
  ntpClient.update();

  // Alarms handler
  Alarm.delay(1000);
}
