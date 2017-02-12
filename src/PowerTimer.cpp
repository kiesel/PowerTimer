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
  analogWrite(ILED, 255-(exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0);
}

void execute_socket_timer(socket_timer_t *socket_timer) {
  Serial.printf("Executing alarm '%s' on socket '%s'\n", socket_timer->name.c_str(), socket_timer->socket->name.c_str());

  execute_socket_command(socket_timer->on, socket_timer->socket);
  Serial.println(" ... done.");
}

void execute_socket_command(bool on, socket_t *socket) {
  if (on) {
    Serial.printf("switchOn('%c', %d, %d)\n", socket->family, socket->group, socket->number);
    rcswitch.switchOn(socket->family, socket->group, socket->number);
  } else {
    Serial.printf("switchOff('%c', %d, %d)\n", socket->family, socket->group, socket->number);
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

  Serial.println("* Registered " + socket_timer_toString(socket_timer));
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  pinMode(ILED, OUTPUT);
  analogWriteRange(255);
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

  for (int i = 0; i < sizeof(timers) / sizeof(socket_timer_t); i++) {
    register_socket_timer(&timers[i]);
  }

  Serial.println("Boot up sequence finished.");
}

void setupHttpService() {
  Serial.println("Starting HTTP service ...");

  server.on("/", []() {
    String page = "<html><body>Power service running<br/>Control sockets:<br/>";
    page += "<form method='get'><ul>";

    for (int i = 0; i < sizeof(sockets) / sizeof(socket_t); i++) {
      page += "<li>Socket " + sockets[i].name + " <a href='/socket?n=" + i + "&amp;s=on'>ON</a> | <a href='/socket?n=" + i + "&amp;s=off'>OFF</a></li>";
    }

    page += "</ul></body></html>";
    server.send(200, "text/html", page);
  });

  server.on("/socket", HTTP_GET, []() {
    String number = server.arg("n");
    String cmd = server.arg("s");

    int index = number.toInt();

    if (index < 0 || index > sizeof(sockets) / sizeof(socket_t)) {
      server.send(404, "text/plain", "Socket index not found.");
      return;
    }

    bool enable = (0 == cmd.compareTo("on"));
    execute_socket_command(enable, &sockets[index]);
    server.send(200, "text/html", "<html><header><meta http-equiv='refresh' content='1, url=/'/><body>Ok, switched socket '" + sockets[index].name + "</body></html>'\n");
  });

  server.begin();
  Serial.println("HTTP ready.");
}

void loop() {

  pulse();

  // Let NTP client sync
  ntpClient.update();

  // Alarms handler
  Alarm.delay(10);

  // Service HTTP clients
  server.handleClient();
}
