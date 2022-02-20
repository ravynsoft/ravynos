#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_tablet_tool.h>
#include <wlr/types/wlr_tablet_tool.h>

void wlr_tablet_init(struct wlr_tablet *tablet,
		const struct wlr_tablet_impl *impl) {
	tablet->impl = impl;
	wl_signal_init(&tablet->events.axis);
	wl_signal_init(&tablet->events.proximity);
	wl_signal_init(&tablet->events.tip);
	wl_signal_init(&tablet->events.button);
	wl_array_init(&tablet->paths);
}

void wlr_tablet_destroy(struct wlr_tablet *tablet) {
	if (!tablet) {
		return;
	}

	char **path_ptr;
	wl_array_for_each(path_ptr, &tablet->paths) {
		free(*path_ptr);
	}
	wl_array_release(&tablet->paths);

	if (tablet->impl && tablet->impl->destroy) {
		tablet->impl->destroy(tablet);
	} else {
		free(tablet);
	}
}
