#ifndef POWERTIMER_H
#define POWERTIMER_H

#include <Arduino.h>
#include <TimeAlarms.h>

typedef struct {
  char family;
  int group;
  int number;
  String name;
} socket_t;

typedef struct {
  AlarmID_t alarm_id;
  String name;
  bool on;
  socket_t *socket;
  int hour;
  int minute;
  int second;
} socket_timer_t;

extern void setupHttpService();
extern void execute_socket_command(bool, socket_t *);

#endif
