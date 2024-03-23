#ifndef _SEATD_EVDEV_H
#define _SEATD_EVDEV_H

int evdev_revoke(int fd);
int path_is_evdev(const char *path);

#endif
