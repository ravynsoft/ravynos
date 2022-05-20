#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <wayland-server-core.h>
#include <xf86drm.h>

#include <wlr/backend/interface.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/util/log.h>

#include "backend/wayland.h"
#include "render/drm_format_set.h"
#include "render/pixel_format.h"
#include "util/signal.h"

#include "drm-client-protocol.h"
#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "pointer-gestures-unstable-v1-client-protocol.h"
#include "presentation-time-client-protocol.h"
#include "xdg-activation-v1-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "tablet-unstable-v2-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"

struct wlr_wl_linux_dmabuf_feedback_v1 {
	struct wlr_wl_backend *backend;
	dev_t main_device_id;
	struct wlr_wl_linux_dmabuf_v1_table_entry *format_table;
	size_t format_table_size;

	dev_t tranche_target_device_id;
};

struct wlr_wl_linux_dmabuf_v1_table_entry {
	uint32_t format;
	uint32_t pad; /* unused */
	uint64_t modifier;
};

struct wlr_wl_backend *get_wl_backend_from_backend(struct wlr_backend *backend) {
	assert(wlr_backend_is_wl(backend));
	return (struct wlr_wl_backend *)backend;
}

static int dispatch_events(int fd, uint32_t mask, void *data) {
	struct wlr_wl_backend *wl = data;

	if ((mask & WL_EVENT_HANGUP) || (mask & WL_EVENT_ERROR)) {
		if (mask & WL_EVENT_ERROR) {
			wlr_log(WLR_ERROR, "Failed to read from remote Wayland display");
		}
		wl_display_terminate(wl->local_display);
		return 0;
	}

	int count = 0;
	if (mask & WL_EVENT_READABLE) {
		count = wl_display_dispatch(wl->remote_display);
	}
	if (mask & WL_EVENT_WRITABLE) {
		wl_display_flush(wl->remote_display);
	}
	if (mask == 0) {
		count = wl_display_dispatch_pending(wl->remote_display);
		wl_display_flush(wl->remote_display);
	}

	if (count < 0) {
		wlr_log(WLR_ERROR, "Failed to dispatch remote Wayland display");
		wl_display_terminate(wl->local_display);
		return 0;
	}
	return count;
}

static void xdg_wm_base_handle_ping(void *data,
		struct xdg_wm_base *base, uint32_t serial) {
	xdg_wm_base_pong(base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	xdg_wm_base_handle_ping,
};

static void presentation_handle_clock_id(void *data,
		struct wp_presentation *presentation, uint32_t clock) {
	struct wlr_wl_backend *wl = data;
	wl->presentation_clock = clock;
}

static const struct wp_presentation_listener presentation_listener = {
	.clock_id = presentation_handle_clock_id,
};

static void linux_dmabuf_v1_handle_format(void *data,
		struct zwp_linux_dmabuf_v1 *linux_dmabuf_v1, uint32_t format) {
	// Note, this event is deprecated
	struct wlr_wl_backend *wl = data;

	wlr_drm_format_set_add(&wl->linux_dmabuf_v1_formats, format,
		DRM_FORMAT_MOD_INVALID);
}

static void linux_dmabuf_v1_handle_modifier(void *data,
		struct zwp_linux_dmabuf_v1 *linux_dmabuf_v1, uint32_t format,
		uint32_t modifier_hi, uint32_t modifier_lo) {
	struct wlr_wl_backend *wl = data;

	uint64_t modifier = ((uint64_t)modifier_hi << 32) | modifier_lo;
	wlr_drm_format_set_add(&wl->linux_dmabuf_v1_formats, format, modifier);
}

static const struct zwp_linux_dmabuf_v1_listener linux_dmabuf_v1_listener = {
	.format = linux_dmabuf_v1_handle_format,
	.modifier = linux_dmabuf_v1_handle_modifier,
};

static void linux_dmabuf_feedback_v1_handle_done(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *feedback) {
	// This space is intentionally left blank
}

static void linux_dmabuf_feedback_v1_handle_format_table(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *feedback, int fd, uint32_t size) {
	struct wlr_wl_linux_dmabuf_feedback_v1 *feedback_data = data;

	feedback_data->format_table = NULL;

	void *table_data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (table_data == MAP_FAILED) {
		wlr_log_errno(WLR_ERROR, "failed to mmap DMA-BUF format table");
	} else {
		feedback_data->format_table = table_data;
		feedback_data->format_table_size = size;
	}
	close(fd);
}

static void linux_dmabuf_feedback_v1_handle_main_device(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *feedback,
		struct wl_array *dev_id_arr) {
	struct wlr_wl_linux_dmabuf_feedback_v1 *feedback_data = data;

	dev_t dev_id;
	assert(dev_id_arr->size == sizeof(dev_id));
	memcpy(&dev_id, dev_id_arr->data, sizeof(dev_id));

	feedback_data->main_device_id = dev_id;

	drmDevice *device = NULL;
	if (drmGetDeviceFromDevId(dev_id, 0, &device) != 0) {
		wlr_log_errno(WLR_ERROR, "drmGetDeviceFromDevId failed");
		return;
	}

	const char *name = NULL;
	if (device->available_nodes & (1 << DRM_NODE_RENDER)) {
		name = device->nodes[DRM_NODE_RENDER];
	} else {
		// Likely a split display/render setup. Pick the primary node and hope
		// Mesa will open the right render node under-the-hood.
		assert(device->available_nodes & (1 << DRM_NODE_PRIMARY));
		name = device->nodes[DRM_NODE_PRIMARY];
		wlr_log(WLR_DEBUG, "DRM device %s has no render node, "
			"falling back to primary node", name);
	}

	feedback_data->backend->drm_render_name = strdup(name);

	drmFreeDevice(&device);
}

static void linux_dmabuf_feedback_v1_handle_tranche_done(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *feedback) {
	struct wlr_wl_linux_dmabuf_feedback_v1 *feedback_data = data;
	feedback_data->tranche_target_device_id = 0;
}

static void linux_dmabuf_feedback_v1_handle_tranche_target_device(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *feedback,
		struct wl_array *dev_id_arr) {
	struct wlr_wl_linux_dmabuf_feedback_v1 *feedback_data = data;

	dev_t dev_id;
	assert(dev_id_arr->size == sizeof(dev_id));
	memcpy(&dev_id, dev_id_arr->data, sizeof(dev_id));

	feedback_data->tranche_target_device_id = dev_id;
}

static void linux_dmabuf_feedback_v1_handle_tranche_formats(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *feedback,
		struct wl_array *indices_arr) {
	struct wlr_wl_linux_dmabuf_feedback_v1 *feedback_data = data;

	if (feedback_data->format_table == NULL) {
		return;
	}
	if (feedback_data->tranche_target_device_id != feedback_data->main_device_id) {
		return;
	}

	size_t table_cap = feedback_data->format_table_size /
		sizeof(struct wlr_wl_linux_dmabuf_v1_table_entry);
	uint16_t *index_ptr;
	wl_array_for_each(index_ptr, indices_arr) {
		assert(*index_ptr < table_cap);
		const struct wlr_wl_linux_dmabuf_v1_table_entry *entry =
			&feedback_data->format_table[*index_ptr];
		wlr_drm_format_set_add(&feedback_data->backend->linux_dmabuf_v1_formats,
			entry->format, entry->modifier);
	}
}

static void linux_dmabuf_feedback_v1_handle_tranche_flags(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *feedback, uint32_t flags) {
	// TODO: handle SCANOUT flag
}

static const struct zwp_linux_dmabuf_feedback_v1_listener
		linux_dmabuf_feedback_v1_listener = {
	.done = linux_dmabuf_feedback_v1_handle_done,
	.format_table = linux_dmabuf_feedback_v1_handle_format_table,
	.main_device = linux_dmabuf_feedback_v1_handle_main_device,
	.tranche_done = linux_dmabuf_feedback_v1_handle_tranche_done,
	.tranche_target_device = linux_dmabuf_feedback_v1_handle_tranche_target_device,
	.tranche_formats = linux_dmabuf_feedback_v1_handle_tranche_formats,
	.tranche_flags = linux_dmabuf_feedback_v1_handle_tranche_flags,
};

static bool device_has_name(const drmDevice *device, const char *name) {
	for (size_t i = 0; i < DRM_NODE_MAX; i++) {
		if (!(device->available_nodes & (1 << i))) {
			continue;
		}
		if (strcmp(device->nodes[i], name) == 0) {
			return true;
		}
	}
	return false;
}

static char *get_render_name(const char *name) {
	uint32_t flags = 0;
	int devices_len = drmGetDevices2(flags, NULL, 0);
	if (devices_len < 0) {
		wlr_log(WLR_ERROR, "drmGetDevices2 failed: %s", strerror(-devices_len));
		return NULL;
	}
	drmDevice **devices = calloc(devices_len, sizeof(drmDevice *));
	if (devices == NULL) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}
	devices_len = drmGetDevices2(flags, devices, devices_len);
	if (devices_len < 0) {
		free(devices);
		wlr_log(WLR_ERROR, "drmGetDevices2 failed: %s", strerror(-devices_len));
		return NULL;
	}

	const drmDevice *match = NULL;
	for (int i = 0; i < devices_len; i++) {
		if (device_has_name(devices[i], name)) {
			match = devices[i];
			break;
		}
	}

	char *render_name = NULL;
	if (match == NULL) {
		wlr_log(WLR_ERROR, "Cannot find DRM device %s", name);
	} else if (!(match->available_nodes & (1 << DRM_NODE_RENDER))) {
		// Likely a split display/render setup. Pick the primary node and hope
		// Mesa will open the right render node under-the-hood.
		wlr_log(WLR_DEBUG, "DRM device %s has no render node, "
			"falling back to primary node", name);
		assert(match->available_nodes & (1 << DRM_NODE_PRIMARY));
		render_name = strdup(match->nodes[DRM_NODE_PRIMARY]);
	} else {
		render_name = strdup(match->nodes[DRM_NODE_RENDER]);
	}

	for (int i = 0; i < devices_len; i++) {
		drmFreeDevice(&devices[i]);
	}
	free(devices);

	return render_name;
}

static void legacy_drm_handle_device(void *data, struct wl_drm *drm,
		const char *name) {
	struct wlr_wl_backend *wl = data;
	wl->drm_render_name = get_render_name(name);
}

static void legacy_drm_handle_format(void *data, struct wl_drm *drm,
		uint32_t format) {
	// This space is intentionally left blank
}

static void legacy_drm_handle_authenticated(void *data, struct wl_drm *drm) {
	// This space is intentionally left blank
}

static void legacy_drm_handle_capabilities(void *data, struct wl_drm *drm,
		uint32_t caps) {
	// This space is intentionally left blank
}

static const struct wl_drm_listener legacy_drm_listener = {
	.device = legacy_drm_handle_device,
	.format = legacy_drm_handle_format,
	.authenticated = legacy_drm_handle_authenticated,
	.capabilities = legacy_drm_handle_capabilities,
};

static void shm_handle_format(void *data, struct wl_shm *shm,
		uint32_t shm_format) {
	struct wlr_wl_backend *wl = data;
	uint32_t drm_format = convert_wl_shm_format_to_drm(shm_format);
	wlr_drm_format_set_add(&wl->shm_formats, drm_format, DRM_FORMAT_MOD_INVALID);
}

static const struct wl_shm_listener shm_listener = {
	.format = shm_handle_format,
};

static void registry_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *iface, uint32_t version) {
	struct wlr_wl_backend *wl = data;

	wlr_log(WLR_DEBUG, "Remote wayland global: %s v%" PRIu32, iface, version);

	if (strcmp(iface, wl_compositor_interface.name) == 0) {
		wl->compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, 4);
	} else if (strcmp(iface, wl_seat_interface.name) == 0) {
		struct wl_seat *wl_seat = wl_registry_bind(registry, name,
			&wl_seat_interface, 5);
		if (!create_wl_seat(wl_seat, wl)) {
			wl_seat_destroy(wl_seat);
		}
	} else if (strcmp(iface, xdg_wm_base_interface.name) == 0) {
		wl->xdg_wm_base = wl_registry_bind(registry, name,
			&xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wl->xdg_wm_base, &xdg_wm_base_listener, NULL);
	} else if (strcmp(iface, zxdg_decoration_manager_v1_interface.name) == 0) {
		wl->zxdg_decoration_manager_v1 = wl_registry_bind(registry, name,
			&zxdg_decoration_manager_v1_interface, 1);
	} else if (strcmp(iface, zwp_pointer_gestures_v1_interface.name) == 0) {
		wl->zwp_pointer_gestures_v1 = wl_registry_bind(registry, name,
			&zwp_pointer_gestures_v1_interface, version < 3 ? version : 3);
	} else if (strcmp(iface, wp_presentation_interface.name) == 0) {
		wl->presentation = wl_registry_bind(registry, name,
			&wp_presentation_interface, 1);
		wp_presentation_add_listener(wl->presentation,
			&presentation_listener, wl);
	} else if (strcmp(iface, zwp_tablet_manager_v2_interface.name) == 0) {
		wl->tablet_manager = wl_registry_bind(registry, name,
			&zwp_tablet_manager_v2_interface, 1);
	} else if (strcmp(iface, zwp_linux_dmabuf_v1_interface.name) == 0 &&
			version >= 3) {
		wl->zwp_linux_dmabuf_v1 = wl_registry_bind(registry, name,
			&zwp_linux_dmabuf_v1_interface, version >= 4 ? 4 : version);
		zwp_linux_dmabuf_v1_add_listener(wl->zwp_linux_dmabuf_v1,
			&linux_dmabuf_v1_listener, wl);
	} else if (strcmp(iface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
		wl->zwp_relative_pointer_manager_v1 = wl_registry_bind(registry, name,
			&zwp_relative_pointer_manager_v1_interface, 1);
	} else if (strcmp(iface, wl_drm_interface.name) == 0) {
		wl->legacy_drm = wl_registry_bind(registry, name, &wl_drm_interface, 1);
		wl_drm_add_listener(wl->legacy_drm, &legacy_drm_listener, wl);
	} else if (strcmp(iface, wl_shm_interface.name) == 0) {
		wl->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
		wl_shm_add_listener(wl->shm, &shm_listener, wl);
	} else if (strcmp(iface, xdg_activation_v1_interface.name) == 0) {
		wl->activation_v1 = wl_registry_bind(registry, name,
			&xdg_activation_v1_interface, 1);
	}
}

static void registry_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// TODO
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove
};

/*
 * Initializes the wayland backend. Opens a connection to a remote wayland
 * compositor and creates surfaces for each output, then registers globals on
 * the specified display.
 */
static bool backend_start(struct wlr_backend *backend) {
	struct wlr_wl_backend *wl = get_wl_backend_from_backend(backend);
	wlr_log(WLR_INFO, "Starting Wayland backend");

	wl->started = true;

	struct wlr_wl_seat *seat;
	wl_list_for_each(seat, &wl->seats, link) {
		if (seat->keyboard) {
			create_wl_keyboard(seat);
		}

		if (wl->tablet_manager) {
			wl_add_tablet_seat(wl->tablet_manager, seat);
		}
	}

	for (size_t i = 0; i < wl->requested_outputs; ++i) {
		wlr_wl_output_create(&wl->backend);
	}

	return true;
}

static void backend_destroy(struct wlr_backend *backend) {
	if (!backend) {
		return;
	}

	struct wlr_wl_backend *wl = get_wl_backend_from_backend(backend);

	struct wlr_wl_output *output, *tmp_output;
	wl_list_for_each_safe(output, tmp_output, &wl->outputs, link) {
		wlr_output_destroy(&output->wlr_output);
	}

	struct wlr_wl_input_device *input_device, *tmp_input_device;
	wl_list_for_each_safe(input_device, tmp_input_device, &wl->devices, link) {
		wlr_input_device_destroy(&input_device->wlr_input_device);
	}

	struct wlr_wl_buffer *buffer, *tmp_buffer;
	wl_list_for_each_safe(buffer, tmp_buffer, &wl->buffers, link) {
		destroy_wl_buffer(buffer);
	}

	wlr_backend_finish(backend);

	wl_list_remove(&wl->local_display_destroy.link);

	wl_event_source_remove(wl->remote_display_src);

	close(wl->drm_fd);

	wlr_drm_format_set_finish(&wl->shm_formats);
	wlr_drm_format_set_finish(&wl->linux_dmabuf_v1_formats);

	destroy_wl_seats(wl);
	if (wl->zxdg_decoration_manager_v1) {
		zxdg_decoration_manager_v1_destroy(wl->zxdg_decoration_manager_v1);
	}
	if (wl->zwp_pointer_gestures_v1) {
		zwp_pointer_gestures_v1_destroy(wl->zwp_pointer_gestures_v1);
	}
	if (wl->presentation) {
		wp_presentation_destroy(wl->presentation);
	}
	if (wl->zwp_linux_dmabuf_v1) {
		zwp_linux_dmabuf_v1_destroy(wl->zwp_linux_dmabuf_v1);
	}
	if (wl->shm) {
		wl_shm_destroy(wl->shm);
	}
	if (wl->zwp_relative_pointer_manager_v1) {
		zwp_relative_pointer_manager_v1_destroy(wl->zwp_relative_pointer_manager_v1);
	}
	free(wl->drm_render_name);
	free(wl->activation_token);
	xdg_wm_base_destroy(wl->xdg_wm_base);
	wl_compositor_destroy(wl->compositor);
	wl_registry_destroy(wl->registry);
	wl_display_flush(wl->remote_display);
	wl_display_disconnect(wl->remote_display);
	free(wl);
}

static clockid_t backend_get_presentation_clock(struct wlr_backend *backend) {
	struct wlr_wl_backend *wl = get_wl_backend_from_backend(backend);
	return wl->presentation_clock;
}

static int backend_get_drm_fd(struct wlr_backend *backend) {
	struct wlr_wl_backend *wl = get_wl_backend_from_backend(backend);
	return wl->drm_fd;
}

static uint32_t get_buffer_caps(struct wlr_backend *backend) {
	struct wlr_wl_backend *wl = get_wl_backend_from_backend(backend);
	return (wl->zwp_linux_dmabuf_v1 ? WLR_BUFFER_CAP_DMABUF : 0)
		| (wl->shm ? WLR_BUFFER_CAP_SHM : 0);
}

static const struct wlr_backend_impl backend_impl = {
	.start = backend_start,
	.destroy = backend_destroy,
	.get_presentation_clock = backend_get_presentation_clock,
	.get_drm_fd = backend_get_drm_fd,
	.get_buffer_caps = get_buffer_caps,
};

bool wlr_backend_is_wl(struct wlr_backend *b) {
	return b->impl == &backend_impl;
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_wl_backend *wl =
		wl_container_of(listener, wl, local_display_destroy);
	backend_destroy(&wl->backend);
}

struct wlr_backend *wlr_wl_backend_create(struct wl_display *display,
		const char *remote) {
	wlr_log(WLR_INFO, "Creating wayland backend");

	struct wlr_wl_backend *wl = calloc(1, sizeof(*wl));
	if (!wl) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}

	wlr_backend_init(&wl->backend, &backend_impl);

	wl->local_display = display;
	wl_list_init(&wl->devices);
	wl_list_init(&wl->outputs);
	wl_list_init(&wl->seats);
	wl_list_init(&wl->buffers);
	wl->presentation_clock = CLOCK_MONOTONIC;

	wl->remote_display = wl_display_connect(remote);
	if (!wl->remote_display) {
		wlr_log_errno(WLR_ERROR, "Could not connect to remote display");
		goto error_wl;
	}

	wl->registry = wl_display_get_registry(wl->remote_display);
	if (!wl->registry) {
		wlr_log_errno(WLR_ERROR, "Could not obtain reference to remote registry");
		goto error_display;
	}
	wl_registry_add_listener(wl->registry, &registry_listener, wl);

	wl_display_roundtrip(wl->remote_display); // get globals

	if (!wl->compositor) {
		wlr_log(WLR_ERROR,
			"Remote Wayland compositor does not support wl_compositor");
		goto error_registry;
	}
	if (!wl->xdg_wm_base) {
		wlr_log(WLR_ERROR,
			"Remote Wayland compositor does not support xdg-shell");
		goto error_registry;
	}

	struct zwp_linux_dmabuf_feedback_v1 *linux_dmabuf_feedback_v1 = NULL;
	struct wlr_wl_linux_dmabuf_feedback_v1 feedback_data = { .backend = wl };
	if (wl->zwp_linux_dmabuf_v1 != NULL &&
			zwp_linux_dmabuf_v1_get_version(wl->zwp_linux_dmabuf_v1) >=
			ZWP_LINUX_DMABUF_V1_GET_DEFAULT_FEEDBACK_SINCE_VERSION) {
		linux_dmabuf_feedback_v1 =
			zwp_linux_dmabuf_v1_get_default_feedback(wl->zwp_linux_dmabuf_v1);
		if (linux_dmabuf_feedback_v1 == NULL) {
			wlr_log(WLR_ERROR, "Allocation failed");
			goto error_registry;
		}
		zwp_linux_dmabuf_feedback_v1_add_listener(linux_dmabuf_feedback_v1,
			&linux_dmabuf_feedback_v1_listener, &feedback_data);

		if (wl->legacy_drm != NULL) {
			wl_drm_destroy(wl->legacy_drm);
			wl->legacy_drm = NULL;
		}
	}

	wl_display_roundtrip(wl->remote_display); // get linux-dmabuf formats

	if (feedback_data.format_table != NULL) {
		munmap(feedback_data.format_table, feedback_data.format_table_size);
	}
	if (linux_dmabuf_feedback_v1 != NULL) {
		zwp_linux_dmabuf_feedback_v1_destroy(linux_dmabuf_feedback_v1);
	}

	struct wl_event_loop *loop = wl_display_get_event_loop(wl->local_display);
	int fd = wl_display_get_fd(wl->remote_display);
	wl->remote_display_src = wl_event_loop_add_fd(loop, fd, WL_EVENT_READABLE,
		dispatch_events, wl);
	if (!wl->remote_display_src) {
		wlr_log(WLR_ERROR, "Failed to create event source");
		goto error_registry;
	}
	wl_event_source_check(wl->remote_display_src);

	if (wl->drm_render_name != NULL) {
		wlr_log(WLR_DEBUG, "Opening DRM render node %s", wl->drm_render_name);
		wl->drm_fd = open(wl->drm_render_name, O_RDWR | O_NONBLOCK | O_CLOEXEC);
		if (wl->drm_fd < 0) {
			wlr_log_errno(WLR_ERROR, "Failed to open DRM render node %s",
				wl->drm_render_name);
			goto error_remote_display_src;
		}
	} else {
		wl->drm_fd = -1;
	}

	wl->local_display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &wl->local_display_destroy);

	const char *token = getenv("XDG_ACTIVATION_TOKEN");
	if (token != NULL) {
		wl->activation_token = strdup(token);
		unsetenv("XDG_ACTIVATION_TOKEN");
	}

	return &wl->backend;

error_remote_display_src:
	wl_event_source_remove(wl->remote_display_src);
error_registry:
	free(wl->drm_render_name);
	if (wl->compositor) {
		wl_compositor_destroy(wl->compositor);
	}
	if (wl->xdg_wm_base) {
		xdg_wm_base_destroy(wl->xdg_wm_base);
	}
	wl_registry_destroy(wl->registry);
error_display:
	wl_display_disconnect(wl->remote_display);
error_wl:
	wlr_backend_finish(&wl->backend);
	free(wl);
	return NULL;
}

struct wl_display *wlr_wl_backend_get_remote_display(struct wlr_backend *backend) {
	struct wlr_wl_backend *wl = get_wl_backend_from_backend(backend);
	return wl->remote_display;
}
