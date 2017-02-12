// #include <Arduino.h>
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
