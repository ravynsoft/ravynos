#ifndef UTIL_SIGNAL_H
#define UTIL_SIGNAL_H

#include <wayland-server-core.h>

void wlr_signal_emit_safe(struct wl_signal *signal, void *data);

#endif
