// #include <Arduino.h>
#include <RCSwitch.h>
#include "PowerTimer.h"

String socket_toString(socket_t *socket) {
  return "'" + socket->name + "' ["
    + socket->family + ","
    + socket->group + ","
    + socket->number + "]"
  ;
}

String socket_timer_toString(socket_timer_t *timer) {
  String out = "Alarm #" + String(timer->alarm_id);
  out += ", at " + String(timer->hour) + ":" + String(timer->minute) + ":" + String(timer->second);
  out += ", turn " + (timer->on ? String("ON") : String("OFF")) + ", socket ";

  return out + socket_toString(timer->socket);
}

void socket_timer_execute(socket_timer_t *socket_timer) {
  Serial.println("Executing alarm " + socket_timer_toString(socket_timer));

  socket_execute_command(socket_timer->on, socket_timer->socket);
  Serial.println(" ... done.");
}

void socket_execute_command(bool on, socket_t *socket) {
  if (on) {
    Serial.println("Switch ON " + socket_toString(socket));
    rcswitch.switchOn(socket->family, socket->group, socket->number);
  } else {
    Serial.println("Switch OFF " + socket_toString(socket));
    rcswitch.switchOff(socket->family, socket->group, socket->number);
  }
}
