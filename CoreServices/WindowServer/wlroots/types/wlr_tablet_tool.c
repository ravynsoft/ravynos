#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/types/wlr_tablet_tool.h>

#include "interfaces/wlr_input_device.h"

void wlr_tablet_init(struct wlr_tablet *tablet,
		const struct wlr_tablet_impl *impl, const char *name) {
	wlr_input_device_init(&tablet->base, WLR_INPUT_DEVICE_TABLET_TOOL, name);
	tablet->base.tablet = tablet;

	tablet->impl = impl;
	wl_signal_init(&tablet->events.axis);
	wl_signal_init(&tablet->events.proximity);
	wl_signal_init(&tablet->events.tip);
	wl_signal_init(&tablet->events.button);
	wl_array_init(&tablet->paths);
}

void wlr_tablet_finish(struct wlr_tablet *tablet) {
	wlr_input_device_finish(&tablet->base);

	char **path_ptr;
	wl_array_for_each(path_ptr, &tablet->paths) {
		free(*path_ptr);
	}
	wl_array_release(&tablet->paths);
}
