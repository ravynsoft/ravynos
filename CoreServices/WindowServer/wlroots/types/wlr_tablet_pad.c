#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_tablet_pad.h>
#include <wlr/types/wlr_tablet_pad.h>

void wlr_tablet_pad_init(struct wlr_tablet_pad *pad,
		struct wlr_tablet_pad_impl *impl) {
	pad->impl = impl;
	wl_signal_init(&pad->events.button);
	wl_signal_init(&pad->events.ring);
	wl_signal_init(&pad->events.strip);
	wl_signal_init(&pad->events.attach_tablet);

	wl_list_init(&pad->groups);
	wl_array_init(&pad->paths);
}

void wlr_tablet_pad_destroy(struct wlr_tablet_pad *pad) {
	if (!pad) {
		return;
	}

	char **path_ptr;
	wl_array_for_each(path_ptr, &pad->paths) {
		free(*path_ptr);
	}
	wl_array_release(&pad->paths);

	if (pad->impl && pad->impl->destroy) {
		pad->impl->destroy(pad);
	} else {
		free(pad);
	}
}
