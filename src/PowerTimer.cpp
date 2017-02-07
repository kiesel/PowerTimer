#include <Arduino.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <ESP8266Wifi.h>
#include <SPI.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <RCSwitch.h>

#include "PowerTimer.h"
#include "constants.h"
#include "config.h"

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, NTP_SERVER, NTP_TIME_OFFSET, NTP_DEFAULT_SYNC);

RCSwitch rcswitch = RCSwitch();
ESP8266WebServer server(80);

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

  execute_socket_command(socket_timer->on, socket_timer->socket);
  Serial.println(" ... done.");
}

void execute_socket_command(bool on, socket_t *socket) {
  if (on) {
    rcswitch.switchOn(socket->family, socket->group, socket->number);
  } else {
    rcswitch.switchOff(socket->family, socket->group, socket->number);
  }
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

  setupHttpService();

  Serial.println("Registering alarms ...");
  Alarm.timerRepeat(2, pulse);

  for (int i = 0; i < sizeof(timers) / sizeof(socket_timer_t); i++) {
    register_socket_timer(&timers[i]);
  }

  Serial.println("Boot up sequence finished.");
}

void setupHttpService() {
  Serial.println("Starting HTTP service ...");

  server.on("/", []() {
    server.send(200, "text/html", "<html><body>Power service running ...</body></html>");
  });

  server.on("/socket", HTTP_POST, []() {
    String number = server.arg("n");
    String cmd = server.arg("s");

    int index = number.toInt();

    if (index < 0 || index > sizeof(sockets) / sizeof(socket_t)) {
      server.send(400, "text/plain", "Socket index not found.");
      return;
    }

    bool enable = (0 == cmd.compareTo("on"));
    execute_socket_command(enable, &sockets[0]);
    server.send(200, "text/plain", "Ok, switched socket.");
  });

  server.begin();
  Serial.println("HTTP ready.");
}

void loop() {

  // Let NTP client sync
  ntpClient.update();

  // Alarms handler
  Alarm.delay(100);

  // Service HTTP clients
  server.handleClient();
}
