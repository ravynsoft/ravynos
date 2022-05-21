#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/log.h>
#include "render/allocator/allocator.h"
#include "render/swapchain.h"
#include "types/wlr_output.h"
#include "util/signal.h"

static bool output_set_hardware_cursor(struct wlr_output *output,
		struct wlr_buffer *buffer, int hotspot_x, int hotspot_y) {
	if (!output->impl->set_cursor) {
		return false;
	}

	if (!output->impl->set_cursor(output, buffer, hotspot_x, hotspot_y)) {
		return false;
	}

	wlr_buffer_unlock(output->cursor_front_buffer);
	output->cursor_front_buffer = NULL;

	if (buffer != NULL) {
		output->cursor_front_buffer = wlr_buffer_lock(buffer);
	}

	return true;
}

static void output_cursor_damage_whole(struct wlr_output_cursor *cursor);

void wlr_output_lock_software_cursors(struct wlr_output *output, bool lock) {
	if (lock) {
		++output->software_cursor_locks;
	} else {
		assert(output->software_cursor_locks > 0);
		--output->software_cursor_locks;
	}
	wlr_log(WLR_DEBUG, "%s hardware cursors on output '%s' (locks: %d)",
		lock ? "Disabling" : "Enabling", output->name,
		output->software_cursor_locks);

	if (output->software_cursor_locks > 0 && output->hardware_cursor != NULL) {
		output_set_hardware_cursor(output, NULL, 0, 0);
		output_cursor_damage_whole(output->hardware_cursor);
		output->hardware_cursor = NULL;
	}

	// If it's possible to use hardware cursors again, don't switch immediately
	// since a recorder is likely to lock software cursors for the next frame
	// again.
}

static void output_scissor(struct wlr_output *output, pixman_box32_t *rect) {
	struct wlr_renderer *renderer = output->renderer;
	assert(renderer);

	struct wlr_box box = {
		.x = rect->x1,
		.y = rect->y1,
		.width = rect->x2 - rect->x1,
		.height = rect->y2 - rect->y1,
	};

	int ow, oh;
	wlr_output_transformed_resolution(output, &ow, &oh);

	enum wl_output_transform transform =
		wlr_output_transform_invert(output->transform);
	wlr_box_transform(&box, &box, transform, ow, oh);

	wlr_renderer_scissor(renderer, &box);
}

/**
 * Returns the cursor box, scaled for its output.
 */
static void output_cursor_get_box(struct wlr_output_cursor *cursor,
		struct wlr_box *box) {
	box->x = cursor->x - cursor->hotspot_x;
	box->y = cursor->y - cursor->hotspot_y;
	box->width = cursor->width;
	box->height = cursor->height;
}

static void output_cursor_render(struct wlr_output_cursor *cursor,
		pixman_region32_t *damage) {
	struct wlr_renderer *renderer = cursor->output->renderer;
	assert(renderer);

	struct wlr_texture *texture = cursor->texture;
	if (cursor->surface != NULL) {
		texture = wlr_surface_get_texture(cursor->surface);
	}
	if (texture == NULL) {
		return;
	}

	struct wlr_box box;
	output_cursor_get_box(cursor, &box);

	pixman_region32_t surface_damage;
	pixman_region32_init(&surface_damage);
	pixman_region32_union_rect(&surface_damage, &surface_damage, box.x, box.y,
		box.width, box.height);
	pixman_region32_intersect(&surface_damage, &surface_damage, damage);
	if (!pixman_region32_not_empty(&surface_damage)) {
		goto surface_damage_finish;
	}

	float matrix[9];
	wlr_matrix_project_box(matrix, &box, WL_OUTPUT_TRANSFORM_NORMAL, 0,
		cursor->output->transform_matrix);

	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(&surface_damage, &nrects);
	for (int i = 0; i < nrects; ++i) {
		output_scissor(cursor->output, &rects[i]);
		wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0f);
	}
	wlr_renderer_scissor(renderer, NULL);

surface_damage_finish:
	pixman_region32_fini(&surface_damage);
}

void wlr_output_render_software_cursors(struct wlr_output *output,
		pixman_region32_t *damage) {
	int width, height;
	wlr_output_transformed_resolution(output, &width, &height);

	pixman_region32_t render_damage;
	pixman_region32_init(&render_damage);
	pixman_region32_union_rect(&render_damage, &render_damage, 0, 0,
		width, height);
	if (damage != NULL) {
		// Damage tracking supported
		pixman_region32_intersect(&render_damage, &render_damage, damage);
	}

	if (pixman_region32_not_empty(&render_damage)) {
		struct wlr_output_cursor *cursor;
		wl_list_for_each(cursor, &output->cursors, link) {
			if (!cursor->enabled || !cursor->visible ||
					output->hardware_cursor == cursor) {
				continue;
			}
			output_cursor_render(cursor, &render_damage);
		}
	}

	pixman_region32_fini(&render_damage);
}

static void output_cursor_damage_whole(struct wlr_output_cursor *cursor) {
	struct wlr_box box;
	output_cursor_get_box(cursor, &box);

	pixman_region32_t damage;
	pixman_region32_init_rect(&damage, box.x, box.y, box.width, box.height);

	struct wlr_output_event_damage event = {
		.output = cursor->output,
		.damage = &damage,
	};
	wlr_signal_emit_safe(&cursor->output->events.damage, &event);

	pixman_region32_fini(&damage);
}

static void output_cursor_reset(struct wlr_output_cursor *cursor) {
	if (cursor->output->hardware_cursor != cursor) {
		output_cursor_damage_whole(cursor);
	}
	if (cursor->surface != NULL) {
		wl_list_remove(&cursor->surface_commit.link);
		wl_list_remove(&cursor->surface_destroy.link);
		if (cursor->visible) {
			wlr_surface_send_leave(cursor->surface, cursor->output);
		}
		cursor->surface = NULL;
	}
}

static void output_cursor_update_visible(struct wlr_output_cursor *cursor) {
	struct wlr_box output_box;
	output_box.x = output_box.y = 0;
	wlr_output_transformed_resolution(cursor->output, &output_box.width,
		&output_box.height);

	struct wlr_box cursor_box;
	output_cursor_get_box(cursor, &cursor_box);

	struct wlr_box intersection;
	bool visible =
		wlr_box_intersection(&intersection, &output_box, &cursor_box);

	if (cursor->surface != NULL) {
		if (cursor->visible && !visible) {
			wlr_surface_send_leave(cursor->surface, cursor->output);
		}
		if (!cursor->visible && visible) {
			wlr_surface_send_enter(cursor->surface, cursor->output);
		}
	}

	cursor->visible = visible;
}

static struct wlr_drm_format *output_pick_cursor_format(struct wlr_output *output) {
	struct wlr_allocator *allocator = output->allocator;
	assert(allocator != NULL);

	const struct wlr_drm_format_set *display_formats = NULL;
	if (output->impl->get_cursor_formats) {
		display_formats =
			output->impl->get_cursor_formats(output, allocator->buffer_caps);
		if (display_formats == NULL) {
			wlr_log(WLR_ERROR, "Failed to get cursor display formats");
			return NULL;
		}
	}

	return output_pick_format(output, display_formats, DRM_FORMAT_ARGB8888);
}

static struct wlr_buffer *render_cursor_buffer(struct wlr_output_cursor *cursor) {
	struct wlr_output *output = cursor->output;

	float scale = output->scale;
	enum wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL;
	struct wlr_texture *texture = cursor->texture;
	if (cursor->surface != NULL) {
		texture = wlr_surface_get_texture(cursor->surface);
		scale = cursor->surface->current.scale;
		transform = cursor->surface->current.transform;
	}
	if (texture == NULL) {
		return NULL;
	}

	struct wlr_allocator *allocator = output->allocator;
	struct wlr_renderer *renderer = output->renderer;
	assert(allocator != NULL && renderer != NULL);

	int width = texture->width;
	int height = texture->height;
	if (output->impl->get_cursor_size) {
		// Apply hardware limitations on buffer size
		output->impl->get_cursor_size(cursor->output, &width, &height);
		if ((int)texture->width > width || (int)texture->height > height) {
			wlr_log(WLR_DEBUG, "Cursor texture too large (%dx%d), "
				"exceeds hardware limitations (%dx%d)", texture->width,
				texture->height, width, height);
			return NULL;
		}
	}

	if (output->cursor_swapchain == NULL ||
			output->cursor_swapchain->width != width ||
			output->cursor_swapchain->height != height) {
		struct wlr_drm_format *format =
			output_pick_cursor_format(output);
		if (format == NULL) {
			wlr_log(WLR_ERROR, "Failed to pick cursor format");
			return NULL;
		}

		wlr_swapchain_destroy(output->cursor_swapchain);
		output->cursor_swapchain = wlr_swapchain_create(allocator,
			width, height, format);
		free(format);
		if (output->cursor_swapchain == NULL) {
			wlr_log(WLR_ERROR, "Failed to create cursor swapchain");
			return NULL;
		}
	}

	struct wlr_buffer *buffer =
		wlr_swapchain_acquire(output->cursor_swapchain, NULL);
	if (buffer == NULL) {
		return NULL;
	}

	struct wlr_box cursor_box = {
		.width = texture->width * output->scale / scale,
		.height = texture->height * output->scale / scale,
	};

	float output_matrix[9];
	wlr_matrix_identity(output_matrix);
	if (output->transform != WL_OUTPUT_TRANSFORM_NORMAL) {
		struct wlr_box tr_size = {
			.width = buffer->width,
			.height = buffer->height,
		};
		wlr_box_transform(&tr_size, &tr_size, output->transform, 0, 0);

		wlr_matrix_translate(output_matrix, buffer->width / 2.0,
			buffer->height / 2.0);
		wlr_matrix_transform(output_matrix, output->transform);
		wlr_matrix_translate(output_matrix, - tr_size.width / 2.0,
			- tr_size.height / 2.0);
	}

	float matrix[9];
	wlr_matrix_project_box(matrix, &cursor_box, transform, 0, output_matrix);

	if (!wlr_renderer_begin_with_buffer(renderer, buffer)) {
		wlr_buffer_unlock(buffer);
		return NULL;
	}

	wlr_renderer_clear(renderer, (float[]){ 0.0, 0.0, 0.0, 0.0 });
	wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0);

	wlr_renderer_end(renderer);

	return buffer;
}

static bool output_cursor_attempt_hardware(struct wlr_output_cursor *cursor) {
	struct wlr_output *output = cursor->output;

	if (!output->impl->set_cursor ||
			output->software_cursor_locks > 0) {
		return false;
	}

	struct wlr_output_cursor *hwcur = output->hardware_cursor;
	if (hwcur != NULL && hwcur != cursor) {
		return false;
	}

	struct wlr_texture *texture = cursor->texture;
	if (cursor->surface != NULL) {
		// TODO: try using the surface buffer directly
		texture = wlr_surface_get_texture(cursor->surface);
	}

	// If the cursor was hidden or was a software cursor, the hardware
	// cursor position is outdated
	output->impl->move_cursor(cursor->output,
		(int)cursor->x, (int)cursor->y);

	struct wlr_buffer *buffer = NULL;
	if (texture != NULL) {
		buffer = render_cursor_buffer(cursor);
		if (buffer == NULL) {
			wlr_log(WLR_ERROR, "Failed to render cursor buffer");
			return false;
		}
	}

	struct wlr_box hotspot = {
		.x = cursor->hotspot_x,
		.y = cursor->hotspot_y,
	};
	wlr_box_transform(&hotspot, &hotspot,
		wlr_output_transform_invert(output->transform),
		buffer ? buffer->width : 0, buffer ? buffer->height : 0);

	bool ok = output_set_hardware_cursor(output, buffer, hotspot.x, hotspot.y);
	wlr_buffer_unlock(buffer);
	if (ok) {
		output->hardware_cursor = cursor;
	}
	return ok;
}

bool wlr_output_cursor_set_image(struct wlr_output_cursor *cursor,
		const uint8_t *pixels, int32_t stride, uint32_t width, uint32_t height,
		int32_t hotspot_x, int32_t hotspot_y) {
	struct wlr_renderer *renderer = cursor->output->renderer;
	if (!renderer) {
		// if the backend has no renderer, we can't draw a cursor, but this is
		// actually okay, for ex. with the noop backend
		return true;
	}

	output_cursor_reset(cursor);

	cursor->width = width;
	cursor->height = height;
	cursor->hotspot_x = hotspot_x;
	cursor->hotspot_y = hotspot_y;
	output_cursor_update_visible(cursor);

	wlr_texture_destroy(cursor->texture);
	cursor->texture = NULL;

	cursor->enabled = false;
	if (pixels != NULL) {
		cursor->texture = wlr_texture_from_pixels(renderer,
			DRM_FORMAT_ARGB8888, stride, width, height, pixels);
		if (cursor->texture == NULL) {
			return false;
		}
		cursor->enabled = true;
	}

	if (output_cursor_attempt_hardware(cursor)) {
		return true;
	}

	wlr_log(WLR_DEBUG, "Falling back to software cursor on output '%s'",
		cursor->output->name);
	output_cursor_damage_whole(cursor);
	return true;
}

static void output_cursor_commit(struct wlr_output_cursor *cursor,
		bool update_hotspot) {
	if (cursor->output->hardware_cursor != cursor) {
		output_cursor_damage_whole(cursor);
	}

	struct wlr_surface *surface = cursor->surface;
	assert(surface != NULL);

	// Some clients commit a cursor surface with a NULL buffer to hide it.
	cursor->enabled = wlr_surface_has_buffer(surface);
	cursor->width = surface->current.width * cursor->output->scale;
	cursor->height = surface->current.height * cursor->output->scale;
	output_cursor_update_visible(cursor);
	if (update_hotspot) {
		cursor->hotspot_x -= surface->current.dx * cursor->output->scale;
		cursor->hotspot_y -= surface->current.dy * cursor->output->scale;
	}

	if (output_cursor_attempt_hardware(cursor)) {
		return;
	}

	// Fallback to software cursor
	output_cursor_damage_whole(cursor);
}

static void output_cursor_handle_commit(struct wl_listener *listener,
		void *data) {
	struct wlr_output_cursor *cursor =
		wl_container_of(listener, cursor, surface_commit);
	output_cursor_commit(cursor, true);
}

static void output_cursor_handle_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_output_cursor *cursor = wl_container_of(listener, cursor,
		surface_destroy);
	output_cursor_reset(cursor);
}

void wlr_output_cursor_set_surface(struct wlr_output_cursor *cursor,
		struct wlr_surface *surface, int32_t hotspot_x, int32_t hotspot_y) {
	hotspot_x *= cursor->output->scale;
	hotspot_y *= cursor->output->scale;

	if (surface && surface == cursor->surface) {
		// Only update the hotspot: surface hasn't changed

		if (cursor->output->hardware_cursor != cursor) {
			output_cursor_damage_whole(cursor);
		}
		cursor->hotspot_x = hotspot_x;
		cursor->hotspot_y = hotspot_y;
		if (cursor->output->hardware_cursor != cursor) {
			output_cursor_damage_whole(cursor);
		} else {
			struct wlr_buffer *buffer = cursor->output->cursor_front_buffer;

			struct wlr_box hotspot = {
				.x = cursor->hotspot_x,
				.y = cursor->hotspot_y,
			};
			wlr_box_transform(&hotspot, &hotspot,
				wlr_output_transform_invert(cursor->output->transform),
				buffer ? buffer->width : 0, buffer ? buffer->height : 0);

			output_set_hardware_cursor(cursor->output, buffer,
				hotspot.x, hotspot.y);
		}
		return;
	}

	output_cursor_reset(cursor);

	cursor->surface = surface;
	cursor->hotspot_x = hotspot_x;
	cursor->hotspot_y = hotspot_y;

	if (surface != NULL) {
		wl_signal_add(&surface->events.commit, &cursor->surface_commit);
		wl_signal_add(&surface->events.destroy, &cursor->surface_destroy);

		cursor->visible = false;
		output_cursor_commit(cursor, false);
	} else {
		cursor->enabled = false;
		cursor->width = 0;
		cursor->height = 0;

		if (cursor->output->hardware_cursor == cursor) {
			output_set_hardware_cursor(cursor->output, NULL, 0, 0);
		}
	}
}

bool wlr_output_cursor_move(struct wlr_output_cursor *cursor,
		double x, double y) {
	if (cursor->x == x && cursor->y == y) {
		return true;
	}

	if (cursor->output->hardware_cursor != cursor) {
		output_cursor_damage_whole(cursor);
	}

	bool was_visible = cursor->visible;
	x *= cursor->output->scale;
	y *= cursor->output->scale;
	cursor->x = x;
	cursor->y = y;
	output_cursor_update_visible(cursor);

	if (!was_visible && !cursor->visible) {
		// Cursor is still hidden, do nothing
		return true;
	}

	if (cursor->output->hardware_cursor != cursor) {
		output_cursor_damage_whole(cursor);
		return true;
	}

	assert(cursor->output->impl->move_cursor);
	return cursor->output->impl->move_cursor(cursor->output, (int)x, (int)y);
}

struct wlr_output_cursor *wlr_output_cursor_create(struct wlr_output *output) {
	struct wlr_output_cursor *cursor =
		calloc(1, sizeof(struct wlr_output_cursor));
	if (cursor == NULL) {
		return NULL;
	}
	cursor->output = output;
	wl_signal_init(&cursor->events.destroy);
	wl_list_init(&cursor->surface_commit.link);
	cursor->surface_commit.notify = output_cursor_handle_commit;
	wl_list_init(&cursor->surface_destroy.link);
	cursor->surface_destroy.notify = output_cursor_handle_destroy;
	wl_list_insert(&output->cursors, &cursor->link);
	cursor->visible = true; // default position is at (0, 0)
	return cursor;
}

void wlr_output_cursor_destroy(struct wlr_output_cursor *cursor) {
	if (cursor == NULL) {
		return;
	}
	output_cursor_reset(cursor);
	wlr_signal_emit_safe(&cursor->events.destroy, cursor);
	if (cursor->output->hardware_cursor == cursor) {
		// If this cursor was the hardware cursor, disable it
		output_set_hardware_cursor(cursor->output, NULL, 0, 0);
		cursor->output->hardware_cursor = NULL;
	}
	wlr_texture_destroy(cursor->texture);
	wl_list_remove(&cursor->link);
	free(cursor);
}
