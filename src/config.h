#ifndef CONFIG_H
#define CONFIG_H

#include <TimeAlarms.h>
#include "PowerTimer.h"

socket_t sockets[] = {
  socket_t{'a', 1, 1, "Livingroom lamp"},
  socket_t{'a', 1, 2, "Night illumination LED"},
  socket_t{'a', 1, 3, "Babyphone Station"}
};

socket_timer_t timers[] = {
  socket_timer_t{dtINVALID_ALARM_ID, true, &sockets[0], 17, 30, 0},
  socket_timer_t{dtINVALID_ALARM_ID, false, &sockets[0], 0, 1, 0},
  socket_timer_t{dtINVALID_ALARM_ID, true, &sockets[1], 17, 30, 0},
  socket_timer_t{dtINVALID_ALARM_ID, false, &sockets[1], 9, 0, 0},
  socket_timer_t{dtINVALID_ALARM_ID, true, &sockets[2], 18, 30, 0},
  socket_timer_t{dtINVALID_ALARM_ID, false, &sockets[2], 10, 0, 0},
};

#endif
