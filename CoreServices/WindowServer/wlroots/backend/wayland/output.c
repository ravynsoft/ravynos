#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <wayland-client.h>

#include <wlr/interfaces/wlr_output.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/util/log.h>

#include "backend/wayland.h"
#include "render/pixel_format.h"
#include "render/swapchain.h"
#include "render/wlr_renderer.h"
#include "util/signal.h"

#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "presentation-time-client-protocol.h"
#include "xdg-activation-v1-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

static const uint32_t SUPPORTED_OUTPUT_STATE =
	WLR_OUTPUT_STATE_BACKEND_OPTIONAL |
	WLR_OUTPUT_STATE_BUFFER |
	WLR_OUTPUT_STATE_MODE;

static struct wlr_wl_output *get_wl_output_from_output(
		struct wlr_output *wlr_output) {
	assert(wlr_output_is_wl(wlr_output));
	return (struct wlr_wl_output *)wlr_output;
}

static void surface_frame_callback(void *data, struct wl_callback *cb,
		uint32_t time) {
	struct wlr_wl_output *output = data;
	assert(output);
	wl_callback_destroy(cb);
	output->frame_callback = NULL;

	wlr_output_send_frame(&output->wlr_output);
}

static const struct wl_callback_listener frame_listener = {
	.done = surface_frame_callback
};

static void presentation_feedback_destroy(
		struct wlr_wl_presentation_feedback *feedback) {
	wl_list_remove(&feedback->link);
	wp_presentation_feedback_destroy(feedback->feedback);
	free(feedback);
}

static void presentation_feedback_handle_sync_output(void *data,
		struct wp_presentation_feedback *feedback, struct wl_output *output) {
	// This space is intentionally left blank
}

static void presentation_feedback_handle_presented(void *data,
		struct wp_presentation_feedback *wp_feedback, uint32_t tv_sec_hi,
		uint32_t tv_sec_lo, uint32_t tv_nsec, uint32_t refresh_ns,
		uint32_t seq_hi, uint32_t seq_lo, uint32_t flags) {
	struct wlr_wl_presentation_feedback *feedback = data;

	struct timespec t = {
		.tv_sec = ((uint64_t)tv_sec_hi << 32) | tv_sec_lo,
		.tv_nsec = tv_nsec,
	};
	struct wlr_output_event_present event = {
		.commit_seq = feedback->commit_seq,
		.presented = true,
		.when = &t,
		.seq = ((uint64_t)seq_hi << 32) | seq_lo,
		.refresh = refresh_ns,
		.flags = flags,
	};
	wlr_output_send_present(&feedback->output->wlr_output, &event);

	presentation_feedback_destroy(feedback);
}

static void presentation_feedback_handle_discarded(void *data,
		struct wp_presentation_feedback *wp_feedback) {
	struct wlr_wl_presentation_feedback *feedback = data;

	struct wlr_output_event_present event = {
		.commit_seq = feedback->commit_seq,
		.presented = false,
	};
	wlr_output_send_present(&feedback->output->wlr_output, &event);

	presentation_feedback_destroy(feedback);
}

static const struct wp_presentation_feedback_listener
		presentation_feedback_listener = {
	.sync_output = presentation_feedback_handle_sync_output,
	.presented = presentation_feedback_handle_presented,
	.discarded = presentation_feedback_handle_discarded,
};

static bool output_set_custom_mode(struct wlr_output *wlr_output,
		int32_t width, int32_t height, int32_t refresh) {
	wlr_output_update_custom_mode(wlr_output, width, height, 0);
	return true;
}

void destroy_wl_buffer(struct wlr_wl_buffer *buffer) {
	if (buffer == NULL) {
		return;
	}
	wl_list_remove(&buffer->buffer_destroy.link);
	wl_list_remove(&buffer->link);
	wl_buffer_destroy(buffer->wl_buffer);
	free(buffer);
}

static void buffer_handle_release(void *data, struct wl_buffer *wl_buffer) {
	struct wlr_wl_buffer *buffer = data;
	buffer->released = true;
	wlr_buffer_unlock(buffer->buffer); // might free buffer
}

static const struct wl_buffer_listener buffer_listener = {
	.release = buffer_handle_release,
};

static void buffer_handle_buffer_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_wl_buffer *buffer =
		wl_container_of(listener, buffer, buffer_destroy);
	destroy_wl_buffer(buffer);
}

static bool test_buffer(struct wlr_wl_backend *wl,
		struct wlr_buffer *wlr_buffer) {
	struct wlr_dmabuf_attributes dmabuf;
	struct wlr_shm_attributes shm;
	if (wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf)) {
		return wlr_drm_format_set_has(&wl->linux_dmabuf_v1_formats,
			dmabuf.format, dmabuf.modifier);
	} else if (wlr_buffer_get_shm(wlr_buffer, &shm)) {
		return wlr_drm_format_set_has(&wl->shm_formats, shm.format,
			DRM_FORMAT_MOD_INVALID);
	} else {
		return false;
	}
}

static struct wl_buffer *import_dmabuf(struct wlr_wl_backend *wl,
		struct wlr_dmabuf_attributes *dmabuf) {
	uint32_t modifier_hi = dmabuf->modifier >> 32;
	uint32_t modifier_lo = (uint32_t)dmabuf->modifier;
	struct zwp_linux_buffer_params_v1 *params =
		zwp_linux_dmabuf_v1_create_params(wl->zwp_linux_dmabuf_v1);
	for (int i = 0; i < dmabuf->n_planes; i++) {
		zwp_linux_buffer_params_v1_add(params, dmabuf->fd[i], i,
			dmabuf->offset[i], dmabuf->stride[i], modifier_hi, modifier_lo);
	}

	struct wl_buffer *wl_buffer = zwp_linux_buffer_params_v1_create_immed(
		params, dmabuf->width, dmabuf->height, dmabuf->format, 0);
	// TODO: handle create() errors
	return wl_buffer;
}

static struct wl_buffer *import_shm(struct wlr_wl_backend *wl,
		struct wlr_shm_attributes *shm) {
	enum wl_shm_format wl_shm_format = convert_drm_format_to_wl_shm(shm->format);
	uint32_t size = shm->stride * shm->height;
	struct wl_shm_pool *pool = wl_shm_create_pool(wl->shm, shm->fd, size);
	if (pool == NULL) {
		return NULL;
	}
	struct wl_buffer *wl_buffer = wl_shm_pool_create_buffer(pool, shm->offset,
		shm->width, shm->height, shm->stride, wl_shm_format);
	wl_shm_pool_destroy(pool);
	return wl_buffer;
}

static struct wlr_wl_buffer *create_wl_buffer(struct wlr_wl_backend *wl,
		struct wlr_buffer *wlr_buffer) {
	if (!test_buffer(wl, wlr_buffer)) {
		return NULL;
	}

	struct wlr_dmabuf_attributes dmabuf;
	struct wlr_shm_attributes shm;
	struct wl_buffer *wl_buffer;
	if (wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf)) {
		wl_buffer = import_dmabuf(wl, &dmabuf);
	} else if (wlr_buffer_get_shm(wlr_buffer, &shm)) {
		wl_buffer = import_shm(wl, &shm);
	} else {
		return NULL;
	}
	if (wl_buffer == NULL) {
		return NULL;
	}

	struct wlr_wl_buffer *buffer = calloc(1, sizeof(struct wlr_wl_buffer));
	if (buffer == NULL) {
		wl_buffer_destroy(wl_buffer);
		return NULL;
	}
	buffer->wl_buffer = wl_buffer;
	buffer->buffer = wlr_buffer_lock(wlr_buffer);
	wl_list_insert(&wl->buffers, &buffer->link);

	wl_buffer_add_listener(wl_buffer, &buffer_listener, buffer);

	buffer->buffer_destroy.notify = buffer_handle_buffer_destroy;
	wl_signal_add(&wlr_buffer->events.destroy, &buffer->buffer_destroy);

	return buffer;
}

static struct wlr_wl_buffer *get_or_create_wl_buffer(struct wlr_wl_backend *wl,
		struct wlr_buffer *wlr_buffer) {
	struct wlr_wl_buffer *buffer;
	wl_list_for_each(buffer, &wl->buffers, link) {
		// We can only re-use a wlr_wl_buffer if the parent compositor has
		// released it, because wl_buffer.release is per-wl_buffer, not per
		// wl_surface.commit.
		if (buffer->buffer == wlr_buffer && buffer->released) {
			buffer->released = false;
			wlr_buffer_lock(buffer->buffer);
			return buffer;
		}
	}

	return create_wl_buffer(wl, wlr_buffer);
}

static bool output_test(struct wlr_output *wlr_output) {
	struct wlr_wl_output *output =
		get_wl_output_from_output(wlr_output);

	uint32_t unsupported =
		wlr_output->pending.committed & ~SUPPORTED_OUTPUT_STATE;
	if (unsupported != 0) {
		wlr_log(WLR_DEBUG, "Unsupported output state fields: 0x%"PRIx32,
			unsupported);
		return false;
	}

	if (wlr_output->pending.committed & WLR_OUTPUT_STATE_MODE) {
		assert(wlr_output->pending.mode_type == WLR_OUTPUT_STATE_MODE_CUSTOM);
	}

	if ((wlr_output->pending.committed & WLR_OUTPUT_STATE_BUFFER) &&
			!test_buffer(output->backend, wlr_output->pending.buffer)) {
		return false;
	}

	return true;
}

static bool output_commit(struct wlr_output *wlr_output) {
	struct wlr_wl_output *output =
		get_wl_output_from_output(wlr_output);

	if (!output_test(wlr_output)) {
		return false;
	}

	if (wlr_output->pending.committed & WLR_OUTPUT_STATE_MODE) {
		if (!output_set_custom_mode(wlr_output,
				wlr_output->pending.custom_mode.width,
				wlr_output->pending.custom_mode.height,
				wlr_output->pending.custom_mode.refresh)) {
			return false;
		}
	}

	if (wlr_output->pending.committed & WLR_OUTPUT_STATE_BUFFER) {
		struct wp_presentation_feedback *wp_feedback = NULL;
		if (output->backend->presentation != NULL) {
			wp_feedback = wp_presentation_feedback(output->backend->presentation,
				output->surface);
		}

		pixman_region32_t *damage = NULL;
		if (wlr_output->pending.committed & WLR_OUTPUT_STATE_DAMAGE) {
			damage = &wlr_output->pending.damage;
		}

		if (output->frame_callback != NULL) {
			wlr_log(WLR_ERROR, "Skipping buffer swap");
			return false;
		}

		output->frame_callback = wl_surface_frame(output->surface);
		wl_callback_add_listener(output->frame_callback, &frame_listener, output);

		struct wlr_buffer *wlr_buffer = wlr_output->pending.buffer;
		struct wlr_wl_buffer *buffer =
			get_or_create_wl_buffer(output->backend, wlr_buffer);
		if (buffer == NULL) {
			return false;
		}

		wl_surface_attach(output->surface, buffer->wl_buffer, 0, 0);

		if (damage == NULL) {
			wl_surface_damage_buffer(output->surface,
				0, 0, INT32_MAX, INT32_MAX);
		} else {
			int rects_len;
			pixman_box32_t *rects =
				pixman_region32_rectangles(damage, &rects_len);
			for (int i = 0; i < rects_len; i++) {
				pixman_box32_t *r = &rects[i];
				wl_surface_damage_buffer(output->surface, r->x1, r->y1,
					r->x2 - r->x1, r->y2 - r->y1);
			}
		}

		wl_surface_commit(output->surface);

		if (wp_feedback != NULL) {
			struct wlr_wl_presentation_feedback *feedback =
				calloc(1, sizeof(*feedback));
			if (feedback == NULL) {
				wp_presentation_feedback_destroy(wp_feedback);
				return false;
			}
			feedback->output = output;
			feedback->feedback = wp_feedback;
			feedback->commit_seq = output->wlr_output.commit_seq + 1;
			wl_list_insert(&output->presentation_feedbacks, &feedback->link);

			wp_presentation_feedback_add_listener(wp_feedback,
				&presentation_feedback_listener, feedback);
		} else {
			struct wlr_output_event_present present_event = {
				.commit_seq = wlr_output->commit_seq + 1,
				.presented = true,
			};
			wlr_output_send_present(wlr_output, &present_event);
		}
	}

	wl_display_flush(output->backend->remote_display);

	return true;
}

static bool output_set_cursor(struct wlr_output *wlr_output,
		struct wlr_buffer *wlr_buffer, int hotspot_x, int hotspot_y) {
	struct wlr_wl_output *output = get_wl_output_from_output(wlr_output);
	struct wlr_wl_backend *backend = output->backend;

	output->cursor.hotspot_x = hotspot_x;
	output->cursor.hotspot_y = hotspot_y;

	if (output->cursor.surface == NULL) {
		output->cursor.surface =
			wl_compositor_create_surface(backend->compositor);
	}
	struct wl_surface *surface = output->cursor.surface;

	if (wlr_buffer != NULL) {
		struct wlr_wl_buffer *buffer =
			get_or_create_wl_buffer(output->backend, wlr_buffer);
		if (buffer == NULL) {
			return false;
		}

		wl_surface_attach(surface, buffer->wl_buffer, 0, 0);
		wl_surface_damage_buffer(surface, 0, 0, INT32_MAX, INT32_MAX);
		wl_surface_commit(surface);
	} else {
		wl_surface_attach(surface, NULL, 0, 0);
		wl_surface_commit(surface);
	}

	update_wl_output_cursor(output);
	wl_display_flush(backend->remote_display);
	return true;
}

static const struct wlr_drm_format_set *output_get_formats(
		struct wlr_output *wlr_output, uint32_t buffer_caps) {
	struct wlr_wl_output *output = get_wl_output_from_output(wlr_output);
	if (buffer_caps & WLR_BUFFER_CAP_DMABUF) {
		return &output->backend->linux_dmabuf_v1_formats;
	} else if (buffer_caps & WLR_BUFFER_CAP_SHM) {
		return &output->backend->shm_formats;
	}
	return NULL;
}

static void output_destroy(struct wlr_output *wlr_output) {
	struct wlr_wl_output *output = get_wl_output_from_output(wlr_output);
	if (output == NULL) {
		return;
	}

	wl_list_remove(&output->link);

	if (output->cursor.surface) {
		wl_surface_destroy(output->cursor.surface);
	}

	if (output->frame_callback) {
		wl_callback_destroy(output->frame_callback);
	}

	struct wlr_wl_presentation_feedback *feedback, *feedback_tmp;
	wl_list_for_each_safe(feedback, feedback_tmp,
			&output->presentation_feedbacks, link) {
		presentation_feedback_destroy(feedback);
	}

	if (output->zxdg_toplevel_decoration_v1) {
		zxdg_toplevel_decoration_v1_destroy(output->zxdg_toplevel_decoration_v1);
	}
	xdg_toplevel_destroy(output->xdg_toplevel);
	xdg_surface_destroy(output->xdg_surface);
	wl_surface_destroy(output->surface);
	wl_display_flush(output->backend->remote_display);
	free(output);
}

void update_wl_output_cursor(struct wlr_wl_output *output) {
	struct wlr_wl_pointer *pointer = output->cursor.pointer;
	if (pointer) {
		assert(pointer->output == output);
		assert(output->enter_serial);
		wl_pointer_set_cursor(pointer->wl_pointer, output->enter_serial,
			output->cursor.surface, output->cursor.hotspot_x,
			output->cursor.hotspot_y);
	}
}

static bool output_move_cursor(struct wlr_output *_output, int x, int y) {
	// TODO: only return true if x == current x and y == current y
	return true;
}

static const struct wlr_output_impl output_impl = {
	.destroy = output_destroy,
	.test = output_test,
	.commit = output_commit,
	.set_cursor = output_set_cursor,
	.move_cursor = output_move_cursor,
	.get_cursor_formats = output_get_formats,
	.get_primary_formats = output_get_formats,
};

bool wlr_output_is_wl(struct wlr_output *wlr_output) {
	return wlr_output->impl == &output_impl;
}

static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
	struct wlr_wl_output *output = data;
	assert(output && output->xdg_surface == xdg_surface);

	xdg_surface_ack_configure(xdg_surface, serial);

	// nothing else?
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data,
		struct xdg_toplevel *xdg_toplevel,
		int32_t width, int32_t height, struct wl_array *states) {
	struct wlr_wl_output *output = data;
	assert(output && output->xdg_toplevel == xdg_toplevel);

	if (width == 0 || height == 0) {
		return;
	}
	// loop over states for maximized etc?
	output_set_custom_mode(&output->wlr_output, width, height, 0);
}

static void xdg_toplevel_handle_close(void *data,
		struct xdg_toplevel *xdg_toplevel) {
	struct wlr_wl_output *output = data;
	assert(output && output->xdg_toplevel == xdg_toplevel);

	wlr_output_destroy(&output->wlr_output);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_handle_configure,
	.close = xdg_toplevel_handle_close,
};

struct wlr_output *wlr_wl_output_create(struct wlr_backend *wlr_backend) {
	struct wlr_wl_backend *backend = get_wl_backend_from_backend(wlr_backend);
	if (!backend->started) {
		++backend->requested_outputs;
		return NULL;
	}

	struct wlr_wl_output *output;
	if (!(output = calloc(sizeof(struct wlr_wl_output), 1))) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_wl_output");
		return NULL;
	}
	wlr_output_init(&output->wlr_output, &backend->backend, &output_impl,
		backend->local_display);
	struct wlr_output *wlr_output = &output->wlr_output;

	wlr_output_update_custom_mode(wlr_output, 1280, 720, 0);
	strncpy(wlr_output->make, "wayland", sizeof(wlr_output->make));
	strncpy(wlr_output->model, "wayland", sizeof(wlr_output->model));

	char name[64];
	snprintf(name, sizeof(name), "WL-%zu", ++backend->last_output_num);
	wlr_output_set_name(wlr_output, name);

	char description[128];
	snprintf(description, sizeof(description),
		"Wayland output %zu", backend->last_output_num);
	wlr_output_set_description(wlr_output, description);

	output->backend = backend;
	wl_list_init(&output->presentation_feedbacks);

	output->surface = wl_compositor_create_surface(backend->compositor);
	if (!output->surface) {
		wlr_log_errno(WLR_ERROR, "Could not create output surface");
		goto error;
	}
	wl_surface_set_user_data(output->surface, output);
	output->xdg_surface =
		xdg_wm_base_get_xdg_surface(backend->xdg_wm_base, output->surface);
	if (!output->xdg_surface) {
		wlr_log_errno(WLR_ERROR, "Could not get xdg surface");
		goto error;
	}
	output->xdg_toplevel =
		xdg_surface_get_toplevel(output->xdg_surface);
	if (!output->xdg_toplevel) {
		wlr_log_errno(WLR_ERROR, "Could not get xdg toplevel");
		goto error;
	}

	if (backend->zxdg_decoration_manager_v1) {
		output->zxdg_toplevel_decoration_v1 =
			zxdg_decoration_manager_v1_get_toplevel_decoration(
			backend->zxdg_decoration_manager_v1, output->xdg_toplevel);
		if (!output->zxdg_toplevel_decoration_v1) {
			wlr_log_errno(WLR_ERROR, "Could not get xdg toplevel decoration");
			goto error;
		}
		zxdg_toplevel_decoration_v1_set_mode(output->zxdg_toplevel_decoration_v1,
			ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}

	wlr_wl_output_set_title(wlr_output, NULL);

	xdg_toplevel_set_app_id(output->xdg_toplevel, "wlroots");
	xdg_surface_add_listener(output->xdg_surface,
			&xdg_surface_listener, output);
	xdg_toplevel_add_listener(output->xdg_toplevel,
			&xdg_toplevel_listener, output);
	wl_surface_commit(output->surface);

	wl_display_roundtrip(output->backend->remote_display);

	wl_list_insert(&backend->outputs, &output->link);
	wlr_output_update_enabled(wlr_output, true);

	wlr_signal_emit_safe(&backend->backend.events.new_output, wlr_output);

	struct wlr_wl_seat *seat;
	wl_list_for_each(seat, &backend->seats, link) {
		if (seat->pointer) {
			create_wl_pointer(seat, output);
		}
	}

	// TODO: let the compositor do this bit
	if (backend->activation_v1 && backend->activation_token) {
		xdg_activation_v1_activate(backend->activation_v1,
				backend->activation_token, output->surface);
	}

	// Start the rendering loop by requesting the compositor to render a frame
	wlr_output_schedule_frame(wlr_output);

	return wlr_output;

error:
	wlr_output_destroy(&output->wlr_output);
	return NULL;
}

void wlr_wl_output_set_title(struct wlr_output *output, const char *title) {
	struct wlr_wl_output *wl_output = get_wl_output_from_output(output);

	char wl_title[32];
	if (title == NULL) {
		if (snprintf(wl_title, sizeof(wl_title), "wlroots - %s", output->name) <= 0) {
			return;
		}
		title = wl_title;
	}

	xdg_toplevel_set_title(wl_output->xdg_toplevel, title);
	wl_display_flush(wl_output->backend->remote_display);
}

struct wl_surface *wlr_wl_output_get_surface(struct wlr_output *output) {
	struct wlr_wl_output *wl_output = get_wl_output_from_output(output);
	return wl_output->surface;
}
