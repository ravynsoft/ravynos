#ifndef XWAYLAND_SOCKETS_H
#define XWAYLAND_SOCKETS_H

bool set_cloexec(int fd, bool cloexec);
void unlink_display_sockets(int display);
int open_display_sockets(int socks[2]);

#endif
