#ifndef _SEATD_TERMINAL_H
#define _SEATD_TERMINAL_H

#include <stdbool.h>

int terminal_open(int vt);

int terminal_set_process_switching(int fd, bool enable);
int terminal_current_vt(int fd);
int terminal_switch_vt(int fd, int vt);
int terminal_ack_release(int fd);
int terminal_ack_acquire(int fd);
int terminal_set_keyboard(int fd, bool enable);
int terminal_set_graphics(int fd, bool enable);

#endif
