#ifndef POWERTIMER_H
#define POWERTIMER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <TimeAlarms.h>

typedef struct {
  char family;
  int group;
  int number;
  String name;
} socket_t;

typedef struct {
  AlarmID_t alarm_id;
  bool on;
  socket_t *socket;
  int hour;
  int minute;
  int second;
} socket_timer_t;

extern void setupHttpService();
extern void socket_execute_command(bool, socket_t *);
extern void socket_timer_execute(socket_timer_t *);
extern String socket_toString(socket_t *);
extern String socket_timer_toString(socket_timer_t *);

extern socket_timer_t* get_socket_timer(AlarmID_t);

extern RCSwitch rcswitch;

#endif
