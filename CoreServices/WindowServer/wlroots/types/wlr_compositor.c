#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/render/interface.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_region.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>
#include <wlr/util/region.h>
#include "types/wlr_region.h"
#include "util/signal.h"
#include "util/time.h"

#define COMPOSITOR_VERSION 4
#define CALLBACK_VERSION 1

static int min(int fst, int snd) {
	if (fst < snd) {
		return fst;
	} else {
		return snd;
	}
}

static int max(int fst, int snd) {
	if (fst > snd) {
		return fst;
	} else {
		return snd;
	}
}

static void surface_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void surface_handle_attach(struct wl_client *client,
		struct wl_resource *resource,
		struct wl_resource *buffer_resource, int32_t dx, int32_t dy) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);

	struct wlr_buffer *buffer = NULL;
	if (buffer_resource != NULL) {
		buffer = wlr_buffer_from_resource(buffer_resource);
		if (buffer == NULL) {
			wl_resource_post_error(buffer_resource, 0, "unknown buffer type");
			return;
		}
	}

	surface->pending.committed |= WLR_SURFACE_STATE_BUFFER;
	surface->pending.dx = dx;
	surface->pending.dy = dy;

	wlr_buffer_unlock(surface->pending.buffer);
	surface->pending.buffer = buffer;
}

static void surface_handle_damage(struct wl_client *client,
		struct wl_resource *resource,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);
	if (width < 0 || height < 0) {
		return;
	}
	surface->pending.committed |= WLR_SURFACE_STATE_SURFACE_DAMAGE;
	pixman_region32_union_rect(&surface->pending.surface_damage,
		&surface->pending.surface_damage,
		x, y, width, height);
}

static void callback_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void surface_handle_frame(struct wl_client *client,
		struct wl_resource *resource, uint32_t callback) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);

	struct wl_resource *callback_resource = wl_resource_create(client,
		&wl_callback_interface, CALLBACK_VERSION, callback);
	if (callback_resource == NULL) {
		wl_resource_post_no_memory(resource);
		return;
	}
	wl_resource_set_implementation(callback_resource, NULL, NULL,
		callback_handle_resource_destroy);

	wl_list_insert(surface->pending.frame_callback_list.prev,
		wl_resource_get_link(callback_resource));

	surface->pending.committed |= WLR_SURFACE_STATE_FRAME_CALLBACK_LIST;
}

static void surface_handle_set_opaque_region(struct wl_client *client,
		struct wl_resource *resource,
		struct wl_resource *region_resource) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);
	surface->pending.committed |= WLR_SURFACE_STATE_OPAQUE_REGION;
	if (region_resource) {
		pixman_region32_t *region = wlr_region_from_resource(region_resource);
		pixman_region32_copy(&surface->pending.opaque, region);
	} else {
		pixman_region32_clear(&surface->pending.opaque);
	}
}

static void surface_handle_set_input_region(struct wl_client *client,
		struct wl_resource *resource,
		struct wl_resource *region_resource) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);
	surface->pending.committed |= WLR_SURFACE_STATE_INPUT_REGION;
	if (region_resource) {
		pixman_region32_t *region = wlr_region_from_resource(region_resource);
		pixman_region32_copy(&surface->pending.input, region);
	} else {
		pixman_region32_fini(&surface->pending.input);
		pixman_region32_init_rect(&surface->pending.input,
			INT32_MIN, INT32_MIN, UINT32_MAX, UINT32_MAX);
	}
}

static void surface_state_transformed_buffer_size(struct wlr_surface_state *state,
		int *out_width, int *out_height) {
	int width = state->buffer_width;
	int height = state->buffer_height;
	if ((state->transform & WL_OUTPUT_TRANSFORM_90) != 0) {
		int tmp = width;
		width = height;
		height = tmp;
	}
	*out_width = width;
	*out_height = height;
}

/**
 * Computes the surface viewport source size, ie. the size after applying the
 * surface's scale, transform and cropping (via the viewport's source
 * rectangle) but before applying the viewport scaling (via the viewport's
 * destination rectangle).
 */
static void surface_state_viewport_src_size(struct wlr_surface_state *state,
		int *out_width, int *out_height) {
	if (state->buffer_width == 0 && state->buffer_height == 0) {
		*out_width = *out_height = 0;
		return;
	}

	if (state->viewport.has_src) {
		*out_width = state->viewport.src.width;
		*out_height = state->viewport.src.height;
	} else {
		surface_state_transformed_buffer_size(state,
			out_width, out_height);
		*out_width /= state->scale;
		*out_height /= state->scale;
	}
}

static void surface_finalize_pending(struct wlr_surface *surface) {
	struct wlr_surface_state *pending = &surface->pending;

	if ((pending->committed & WLR_SURFACE_STATE_BUFFER)) {
		if (pending->buffer != NULL) {
			pending->buffer_width = pending->buffer->width;
			pending->buffer_height = pending->buffer->height;
		} else {
			pending->buffer_width = pending->buffer_height = 0;
		}
	}

	if (!pending->viewport.has_src &&
			(pending->buffer_width % pending->scale != 0 ||
			pending->buffer_height % pending->scale != 0)) {
		// TODO: send WL_SURFACE_ERROR_INVALID_SIZE error once this issue is
		// resolved:
		// https://gitlab.freedesktop.org/wayland/wayland/-/issues/194
		wlr_log(WLR_DEBUG, "Client bug: submitted a buffer whose size (%dx%d) "
			"is not divisible by scale (%d)", pending->buffer_width,
			pending->buffer_height, pending->scale);
	}

	if (pending->viewport.has_dst) {
		if (pending->buffer_width == 0 && pending->buffer_height == 0) {
			pending->width = pending->height = 0;
		} else {
			pending->width = pending->viewport.dst_width;
			pending->height = pending->viewport.dst_height;
		}
	} else {
		surface_state_viewport_src_size(pending, &pending->width, &pending->height);
	}

	pixman_region32_intersect_rect(&pending->surface_damage,
		&pending->surface_damage, 0, 0, pending->width, pending->height);

	pixman_region32_intersect_rect(&pending->buffer_damage,
		&pending->buffer_damage, 0, 0, pending->buffer_width,
		pending->buffer_height);
}

static void surface_update_damage(pixman_region32_t *buffer_damage,
		struct wlr_surface_state *current, struct wlr_surface_state *pending) {
	pixman_region32_clear(buffer_damage);

	if (pending->width != current->width ||
			pending->height != current->height ||
			pending->viewport.src.x != current->viewport.src.x ||
			pending->viewport.src.y != current->viewport.src.y ||
			pending->viewport.src.width != current->viewport.src.width ||
			pending->viewport.src.height != current->viewport.src.height) {
		// Damage the whole buffer on resize or viewport source box change
		pixman_region32_union_rect(buffer_damage, buffer_damage, 0, 0,
			pending->buffer_width, pending->buffer_height);
	} else {
		// Copy over surface damage + buffer damage
		pixman_region32_t surface_damage;
		pixman_region32_init(&surface_damage);

		pixman_region32_copy(&surface_damage, &pending->surface_damage);

		if (pending->viewport.has_dst) {
			int src_width, src_height;
			surface_state_viewport_src_size(pending, &src_width, &src_height);
			float scale_x = (float)pending->viewport.dst_width / src_width;
			float scale_y = (float)pending->viewport.dst_height / src_height;
			wlr_region_scale_xy(&surface_damage, &surface_damage,
				1.0 / scale_x, 1.0 / scale_y);
		}
		if (pending->viewport.has_src) {
			// This is lossy: do a best-effort conversion
			pixman_region32_translate(&surface_damage,
				floor(pending->viewport.src.x),
				floor(pending->viewport.src.y));
		}

		wlr_region_scale(&surface_damage, &surface_damage, pending->scale);

		int width, height;
		surface_state_transformed_buffer_size(pending, &width, &height);
		wlr_region_transform(&surface_damage, &surface_damage,
			wlr_output_transform_invert(pending->transform),
			width, height);

		pixman_region32_union(buffer_damage,
			&pending->buffer_damage, &surface_damage);

		pixman_region32_fini(&surface_damage);
	}
}

/**
 * Append pending state to current state and clear pending state.
 */
static void surface_state_move(struct wlr_surface_state *state,
		struct wlr_surface_state *next) {
	state->width = next->width;
	state->height = next->height;
	state->buffer_width = next->buffer_width;
	state->buffer_height = next->buffer_height;

	if (next->committed & WLR_SURFACE_STATE_SCALE) {
		state->scale = next->scale;
	}
	if (next->committed & WLR_SURFACE_STATE_TRANSFORM) {
		state->transform = next->transform;
	}
	if (next->committed & WLR_SURFACE_STATE_BUFFER) {
		state->dx = next->dx;
		state->dy = next->dy;
		next->dx = next->dy = 0;

		wlr_buffer_unlock(state->buffer);
		state->buffer = NULL;
		if (next->buffer) {
			state->buffer = wlr_buffer_lock(next->buffer);
		}
		wlr_buffer_unlock(next->buffer);
		next->buffer = NULL;
	} else {
		state->dx = state->dy = 0;
	}
	if (next->committed & WLR_SURFACE_STATE_SURFACE_DAMAGE) {
		pixman_region32_copy(&state->surface_damage, &next->surface_damage);
		pixman_region32_clear(&next->surface_damage);
	} else {
		pixman_region32_clear(&state->surface_damage);
	}
	if (next->committed & WLR_SURFACE_STATE_BUFFER_DAMAGE) {
		pixman_region32_copy(&state->buffer_damage, &next->buffer_damage);
		pixman_region32_clear(&next->buffer_damage);
	} else {
		pixman_region32_clear(&state->buffer_damage);
	}
	if (next->committed & WLR_SURFACE_STATE_OPAQUE_REGION) {
		pixman_region32_copy(&state->opaque, &next->opaque);
	}
	if (next->committed & WLR_SURFACE_STATE_INPUT_REGION) {
		pixman_region32_copy(&state->input, &next->input);
	}
	if (next->committed & WLR_SURFACE_STATE_VIEWPORT) {
		memcpy(&state->viewport, &next->viewport, sizeof(state->viewport));
	}
	if (next->committed & WLR_SURFACE_STATE_FRAME_CALLBACK_LIST) {
		wl_list_insert_list(&state->frame_callback_list,
			&next->frame_callback_list);
		wl_list_init(&next->frame_callback_list);
	}

	state->committed |= next->committed;
	next->committed = 0;

	state->seq = next->seq;

	state->cached_state_locks = next->cached_state_locks;
	next->cached_state_locks = 0;
}

static void surface_apply_damage(struct wlr_surface *surface) {
	if (surface->current.buffer == NULL) {
		// NULL commit
		if (surface->buffer != NULL) {
			wlr_buffer_unlock(&surface->buffer->base);
		}
		surface->buffer = NULL;
		return;
	}

	if (surface->buffer != NULL) {
		if (wlr_client_buffer_apply_damage(surface->buffer,
				surface->current.buffer, &surface->buffer_damage)) {
			wlr_buffer_unlock(surface->current.buffer);
			surface->current.buffer = NULL;
			return;
		}
	}

	struct wlr_client_buffer *buffer = wlr_client_buffer_create(
			surface->current.buffer, surface->renderer);

	wlr_buffer_unlock(surface->current.buffer);
	surface->current.buffer = NULL;

	if (buffer == NULL) {
		wlr_log(WLR_ERROR, "Failed to upload buffer");
		return;
	}

	if (surface->buffer != NULL) {
		wlr_buffer_unlock(&surface->buffer->base);
	}
	surface->buffer = buffer;
}

static void surface_update_opaque_region(struct wlr_surface *surface) {
	struct wlr_texture *texture = wlr_surface_get_texture(surface);
	if (texture == NULL) {
		pixman_region32_clear(&surface->opaque_region);
		return;
	}

	if (wlr_texture_is_opaque(texture)) {
		pixman_region32_init_rect(&surface->opaque_region,
			0, 0, surface->current.width, surface->current.height);
		return;
	}

	pixman_region32_intersect_rect(&surface->opaque_region,
		&surface->current.opaque,
		0, 0, surface->current.width, surface->current.height);
}

static void surface_update_input_region(struct wlr_surface *surface) {
	pixman_region32_intersect_rect(&surface->input_region,
		&surface->current.input,
		0, 0, surface->current.width, surface->current.height);
}

static void surface_state_init(struct wlr_surface_state *state);

static void subsurface_parent_commit(struct wlr_subsurface *subsurface);

static void surface_cache_pending(struct wlr_surface *surface) {
	struct wlr_surface_state *cached = calloc(1, sizeof(*cached));
	if (!cached) {
		wl_resource_post_no_memory(surface->resource);
		return;
	}

	surface_state_init(cached);
	surface_state_move(cached, &surface->pending);

	wl_list_insert(surface->cached.prev, &cached->cached_state_link);

	surface->pending.seq++;
}

static void surface_commit_state(struct wlr_surface *surface,
		struct wlr_surface_state *next) {
	assert(next->cached_state_locks == 0);

	if (surface->role && surface->role->precommit) {
		surface->role->precommit(surface, next);
	}

	bool invalid_buffer = next->committed & WLR_SURFACE_STATE_BUFFER;

	surface->sx += next->dx;
	surface->sy += next->dy;
	surface_update_damage(&surface->buffer_damage, &surface->current, next);

	pixman_region32_clear(&surface->external_damage);
	if (surface->current.width > next->width ||
			surface->current.height > next->height ||
			next->dx != 0 || next->dy != 0) {
		pixman_region32_union_rect(&surface->external_damage,
			&surface->external_damage, -next->dx, -next->dy,
			surface->current.width, surface->current.height);
	}

	surface->previous.scale = surface->current.scale;
	surface->previous.transform = surface->current.transform;
	surface->previous.width = surface->current.width;
	surface->previous.height = surface->current.height;
	surface->previous.buffer_width = surface->current.buffer_width;
	surface->previous.buffer_height = surface->current.buffer_height;

	surface_state_move(&surface->current, next);

	if (invalid_buffer) {
		surface_apply_damage(surface);
	}
	surface_update_opaque_region(surface);
	surface_update_input_region(surface);

	// commit subsurface order
	struct wlr_subsurface *subsurface;
	wl_list_for_each_reverse(subsurface, &surface->pending.subsurfaces_above,
			pending.link) {
		wl_list_remove(&subsurface->current.link);
		wl_list_insert(&surface->current.subsurfaces_above,
			&subsurface->current.link);

		subsurface_parent_commit(subsurface);
	}
	wl_list_for_each_reverse(subsurface, &surface->pending.subsurfaces_below,
			pending.link) {
		wl_list_remove(&subsurface->current.link);
		wl_list_insert(&surface->current.subsurfaces_below,
			&subsurface->current.link);

		subsurface_parent_commit(subsurface);
	}

	// If we're committing the pending state, bump the pending sequence number
	// here, to allow commit listeners to lock the new pending state.
	if (next == &surface->pending) {
		surface->pending.seq++;
	}

	if (surface->role && surface->role->commit) {
		surface->role->commit(surface);
	}

	wlr_signal_emit_safe(&surface->events.commit, surface);
}

static void collect_subsurface_damage_iter(struct wlr_surface *surface,
		int sx, int sy, void *data) {
	struct wlr_subsurface *subsurface = data;
	pixman_region32_t *damage = &subsurface->parent->external_damage;
	pixman_region32_union_rect(damage, damage,
		subsurface->current.x + sx,
		subsurface->current.y + sy,
		surface->current.width, surface->current.height);
}

// TODO: untangle from wlr_surface
static void subsurface_parent_commit(struct wlr_subsurface *subsurface) {
	struct wlr_surface *surface = subsurface->surface;

	bool moved = subsurface->current.x != subsurface->pending.x ||
		subsurface->current.y != subsurface->pending.y;
	if (subsurface->mapped && moved) {
		wlr_surface_for_each_surface(surface,
			collect_subsurface_damage_iter, subsurface);
	}

	if (subsurface->synchronized && subsurface->has_cache) {
		wlr_surface_unlock_cached(surface, subsurface->cached_seq);
		subsurface->has_cache = false;
	}

	subsurface->current.x = subsurface->pending.x;
	subsurface->current.y = subsurface->pending.y;
	if (subsurface->mapped && (moved || subsurface->reordered)) {
		subsurface->reordered = false;
		wlr_surface_for_each_surface(surface,
			collect_subsurface_damage_iter, subsurface);
	}

	if (!subsurface->added) {
		subsurface->added = true;
		wlr_signal_emit_safe(&subsurface->parent->events.new_subsurface,
			subsurface);
	}
}

static void surface_handle_commit(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);
	surface_finalize_pending(surface);

	wlr_signal_emit_safe(&surface->events.client_commit, NULL);

	if (surface->pending.cached_state_locks > 0 || !wl_list_empty(&surface->cached)) {
		surface_cache_pending(surface);
	} else {
		surface_commit_state(surface, &surface->pending);
	}
}

static void surface_handle_set_buffer_transform(struct wl_client *client,
		struct wl_resource *resource, int32_t transform) {
	if (transform < WL_OUTPUT_TRANSFORM_NORMAL ||
			transform > WL_OUTPUT_TRANSFORM_FLIPPED_270) {
		wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_TRANSFORM,
			"Specified transform value (%d) is invalid", transform);
		return;
	}
	struct wlr_surface *surface = wlr_surface_from_resource(resource);
	surface->pending.committed |= WLR_SURFACE_STATE_TRANSFORM;
	surface->pending.transform = transform;
}

static void surface_handle_set_buffer_scale(struct wl_client *client,
		struct wl_resource *resource, int32_t scale) {
	if (scale <= 0) {
		wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_SCALE,
			"Specified scale value (%d) is not positive", scale);
		return;
	}
	struct wlr_surface *surface = wlr_surface_from_resource(resource);
	surface->pending.committed |= WLR_SURFACE_STATE_SCALE;
	surface->pending.scale = scale;
}

static void surface_handle_damage_buffer(struct wl_client *client,
		struct wl_resource *resource,
		int32_t x, int32_t y, int32_t width,
		int32_t height) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);
	if (width < 0 || height < 0) {
		return;
	}
	surface->pending.committed |= WLR_SURFACE_STATE_BUFFER_DAMAGE;
	pixman_region32_union_rect(&surface->pending.buffer_damage,
		&surface->pending.buffer_damage,
		x, y, width, height);
}

static const struct wl_surface_interface surface_implementation = {
	.destroy = surface_handle_destroy,
	.attach = surface_handle_attach,
	.damage = surface_handle_damage,
	.frame = surface_handle_frame,
	.set_opaque_region = surface_handle_set_opaque_region,
	.set_input_region = surface_handle_set_input_region,
	.commit = surface_handle_commit,
	.set_buffer_transform = surface_handle_set_buffer_transform,
	.set_buffer_scale = surface_handle_set_buffer_scale,
	.damage_buffer = surface_handle_damage_buffer
};

struct wlr_surface *wlr_surface_from_resource(struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_surface_interface,
		&surface_implementation));
	return wl_resource_get_user_data(resource);
}

static void surface_state_init(struct wlr_surface_state *state) {
	state->scale = 1;
	state->transform = WL_OUTPUT_TRANSFORM_NORMAL;

	wl_list_init(&state->subsurfaces_above);
	wl_list_init(&state->subsurfaces_below);

	wl_list_init(&state->frame_callback_list);

	pixman_region32_init(&state->surface_damage);
	pixman_region32_init(&state->buffer_damage);
	pixman_region32_init(&state->opaque);
	pixman_region32_init_rect(&state->input,
		INT32_MIN, INT32_MIN, UINT32_MAX, UINT32_MAX);
}

static void surface_state_finish(struct wlr_surface_state *state) {
	wlr_buffer_unlock(state->buffer);

	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &state->frame_callback_list) {
		wl_resource_destroy(resource);
	}

	pixman_region32_fini(&state->surface_damage);
	pixman_region32_fini(&state->buffer_damage);
	pixman_region32_fini(&state->opaque);
	pixman_region32_fini(&state->input);
}

static void surface_state_destroy_cached(struct wlr_surface_state *state) {
	surface_state_finish(state);
	wl_list_remove(&state->cached_state_link);
	free(state);
}

static void surface_output_destroy(struct wlr_surface_output *surface_output);

static void surface_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_surface *surface = wlr_surface_from_resource(resource);

	struct wlr_surface_output *surface_output, *surface_output_tmp;
	wl_list_for_each_safe(surface_output, surface_output_tmp,
			&surface->current_outputs, link) {
		surface_output_destroy(surface_output);
	}

	wlr_signal_emit_safe(&surface->events.destroy, surface);

	wlr_addon_set_finish(&surface->addons);

	struct wlr_surface_state *cached, *cached_tmp;
	wl_list_for_each_safe(cached, cached_tmp, &surface->cached, cached_state_link) {
		surface_state_destroy_cached(cached);
	}

	wl_list_remove(&surface->renderer_destroy.link);
	surface_state_finish(&surface->pending);
	surface_state_finish(&surface->current);
	pixman_region32_fini(&surface->buffer_damage);
	pixman_region32_fini(&surface->external_damage);
	pixman_region32_fini(&surface->opaque_region);
	pixman_region32_fini(&surface->input_region);
	if (surface->buffer != NULL) {
		wlr_buffer_unlock(&surface->buffer->base);
	}
	free(surface);
}

static void surface_handle_renderer_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_surface *surface =
		wl_container_of(listener, surface, renderer_destroy);
	wl_resource_destroy(surface->resource);
}

static struct wlr_surface *surface_create(struct wl_client *client,
		uint32_t version, uint32_t id, struct wlr_renderer *renderer) {
	struct wlr_surface *surface = calloc(1, sizeof(struct wlr_surface));
	if (!surface) {
		wl_client_post_no_memory(client);
		return NULL;
	}
	surface->resource = wl_resource_create(client, &wl_surface_interface,
		version, id);
	if (surface->resource == NULL) {
		free(surface);
		wl_client_post_no_memory(client);
		return NULL;
	}
	wl_resource_set_implementation(surface->resource, &surface_implementation,
		surface, surface_handle_resource_destroy);

	wlr_log(WLR_DEBUG, "New wlr_surface %p (res %p)", surface, surface->resource);

	surface->renderer = renderer;

	surface_state_init(&surface->current);
	surface_state_init(&surface->pending);
	surface->pending.seq = 1;

	wl_signal_init(&surface->events.client_commit);
	wl_signal_init(&surface->events.commit);
	wl_signal_init(&surface->events.destroy);
	wl_signal_init(&surface->events.new_subsurface);
	wl_list_init(&surface->current_outputs);
	wl_list_init(&surface->cached);
	pixman_region32_init(&surface->buffer_damage);
	pixman_region32_init(&surface->external_damage);
	pixman_region32_init(&surface->opaque_region);
	pixman_region32_init(&surface->input_region);
	wlr_addon_set_init(&surface->addons);

	wl_signal_add(&renderer->events.destroy, &surface->renderer_destroy);
	surface->renderer_destroy.notify = surface_handle_renderer_destroy;

	return surface;
}

struct wlr_texture *wlr_surface_get_texture(struct wlr_surface *surface) {
	if (surface->buffer == NULL) {
		return NULL;
	}
	return surface->buffer->texture;
}

bool wlr_surface_has_buffer(struct wlr_surface *surface) {
	return wlr_surface_get_texture(surface) != NULL;
}

bool wlr_surface_set_role(struct wlr_surface *surface,
		const struct wlr_surface_role *role, void *role_data,
		struct wl_resource *error_resource, uint32_t error_code) {
	assert(role != NULL);

	if (surface->role != NULL && surface->role != role) {
		if (error_resource != NULL) {
			wl_resource_post_error(error_resource, error_code,
				"Cannot assign role %s to wl_surface@%" PRIu32 ", already has role %s\n",
				role->name, wl_resource_get_id(surface->resource),
				surface->role->name);
		}
		return false;
	}
	if (surface->role_data != NULL && surface->role_data != role_data) {
		wl_resource_post_error(error_resource, error_code,
			"Cannot reassign role %s to wl_surface@%" PRIu32 ","
			"role object still exists", role->name,
			wl_resource_get_id(surface->resource));
		return false;
	}

	surface->role = role;
	surface->role_data = role_data;
	return true;
}

uint32_t wlr_surface_lock_pending(struct wlr_surface *surface) {
	surface->pending.cached_state_locks++;
	return surface->pending.seq;
}

void wlr_surface_unlock_cached(struct wlr_surface *surface, uint32_t seq) {
	if (surface->pending.seq == seq) {
		assert(surface->pending.cached_state_locks > 0);
		surface->pending.cached_state_locks--;
		return;
	}

	bool found = false;
	struct wlr_surface_state *cached;
	wl_list_for_each(cached, &surface->cached, cached_state_link) {
		if (cached->seq == seq) {
			found = true;
			break;
		}
	}
	assert(found);

	assert(cached->cached_state_locks > 0);
	cached->cached_state_locks--;

	if (cached->cached_state_locks != 0) {
		return;
	}

	if (cached->cached_state_link.prev != &surface->cached) {
		// This isn't the first cached state. This means we're blocked on a
		// previous cached state.
		return;
	}

	// TODO: consider merging all committed states together
	struct wlr_surface_state *next, *tmp;
	wl_list_for_each_safe(next, tmp, &surface->cached, cached_state_link) {
		if (next->cached_state_locks > 0) {
			break;
		}

		surface_commit_state(surface, next);
		surface_state_destroy_cached(next);
	}
}

struct wlr_surface *wlr_surface_get_root_surface(struct wlr_surface *surface) {
	while (wlr_surface_is_subsurface(surface)) {
		struct wlr_subsurface *subsurface =
			wlr_subsurface_from_wlr_surface(surface);
		if (subsurface == NULL) {
			break;
		}
		surface = subsurface->parent;
	}
	return surface;
}

bool wlr_surface_point_accepts_input(struct wlr_surface *surface,
		double sx, double sy) {
	return sx >= 0 && sx < surface->current.width &&
		sy >= 0 && sy < surface->current.height &&
		pixman_region32_contains_point(&surface->current.input, floor(sx), floor(sy), NULL);
}

struct wlr_surface *wlr_surface_surface_at(struct wlr_surface *surface,
		double sx, double sy, double *sub_x, double *sub_y) {
	struct wlr_subsurface *subsurface;
	wl_list_for_each_reverse(subsurface, &surface->current.subsurfaces_above,
			current.link) {
		if (!subsurface->mapped) {
			continue;
		}

		double _sub_x = subsurface->current.x;
		double _sub_y = subsurface->current.y;
		struct wlr_surface *sub = wlr_surface_surface_at(subsurface->surface,
			sx - _sub_x, sy - _sub_y, sub_x, sub_y);
		if (sub != NULL) {
			return sub;
		}
	}

	if (wlr_surface_point_accepts_input(surface, sx, sy)) {
		if (sub_x) {
			*sub_x = sx;
		}
		if (sub_y) {
			*sub_y = sy;
		}
		return surface;
	}

	wl_list_for_each_reverse(subsurface, &surface->current.subsurfaces_below,
			current.link) {
		if (!subsurface->mapped) {
			continue;
		}

		double _sub_x = subsurface->current.x;
		double _sub_y = subsurface->current.y;
		struct wlr_surface *sub = wlr_surface_surface_at(subsurface->surface,
			sx - _sub_x, sy - _sub_y, sub_x, sub_y);
		if (sub != NULL) {
			return sub;
		}
	}

	return NULL;
}

static void surface_output_destroy(struct wlr_surface_output *surface_output) {
	wl_list_remove(&surface_output->bind.link);
	wl_list_remove(&surface_output->destroy.link);
	wl_list_remove(&surface_output->link);

	free(surface_output);
}

static void surface_handle_output_bind(struct wl_listener *listener,
		void *data) {
	struct wlr_output_event_bind *evt = data;
	struct wlr_surface_output *surface_output =
		wl_container_of(listener, surface_output, bind);
	struct wl_client *client = wl_resource_get_client(
			surface_output->surface->resource);
	if (client == wl_resource_get_client(evt->resource)) {
		wl_surface_send_enter(surface_output->surface->resource, evt->resource);
	}
}

static void surface_handle_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_surface_output *surface_output =
		wl_container_of(listener, surface_output, destroy);
	surface_output_destroy(surface_output);
}

void wlr_surface_send_enter(struct wlr_surface *surface,
		struct wlr_output *output) {
	struct wl_client *client = wl_resource_get_client(surface->resource);
	struct wlr_surface_output *surface_output;
	struct wl_resource *resource;

	wl_list_for_each(surface_output, &surface->current_outputs, link) {
		if (surface_output->output == output) {
			return;
		}
	}

	surface_output = calloc(1, sizeof(struct wlr_surface_output));
	if (surface_output == NULL) {
		return;
	}
	surface_output->bind.notify = surface_handle_output_bind;
	surface_output->destroy.notify = surface_handle_output_destroy;

	wl_signal_add(&output->events.bind, &surface_output->bind);
	wl_signal_add(&output->events.destroy, &surface_output->destroy);

	surface_output->surface = surface;
	surface_output->output = output;
	wl_list_insert(&surface->current_outputs, &surface_output->link);

	wl_resource_for_each(resource, &output->resources) {
		if (client == wl_resource_get_client(resource)) {
			wl_surface_send_enter(surface->resource, resource);
		}
	}
}

void wlr_surface_send_leave(struct wlr_surface *surface,
		struct wlr_output *output) {
	struct wl_client *client = wl_resource_get_client(surface->resource);
	struct wlr_surface_output *surface_output, *tmp;
	struct wl_resource *resource;

	wl_list_for_each_safe(surface_output, tmp,
			&surface->current_outputs, link) {
		if (surface_output->output == output) {
			surface_output_destroy(surface_output);
			wl_resource_for_each(resource, &output->resources) {
				if (client == wl_resource_get_client(resource)) {
					wl_surface_send_leave(surface->resource, resource);
				}
			}
			break;
		}
	}
}

void wlr_surface_send_frame_done(struct wlr_surface *surface,
		const struct timespec *when) {
	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp,
			&surface->current.frame_callback_list) {
		wl_callback_send_done(resource, timespec_to_msec(when));
		wl_resource_destroy(resource);
	}
}

static void surface_for_each_surface(struct wlr_surface *surface, int x, int y,
		wlr_surface_iterator_func_t iterator, void *user_data) {
	struct wlr_subsurface *subsurface;
	wl_list_for_each(subsurface, &surface->current.subsurfaces_below, current.link) {
		if (!subsurface->mapped) {
			continue;
		}

		struct wlr_subsurface_parent_state *state = &subsurface->current;
		int sx = state->x;
		int sy = state->y;

		surface_for_each_surface(subsurface->surface, x + sx, y + sy,
			iterator, user_data);
	}

	iterator(surface, x, y, user_data);

	wl_list_for_each(subsurface, &surface->current.subsurfaces_above, current.link) {
		if (!subsurface->mapped) {
			continue;
		}

		struct wlr_subsurface_parent_state *state = &subsurface->current;
		int sx = state->x;
		int sy = state->y;

		surface_for_each_surface(subsurface->surface, x + sx, y + sy,
			iterator, user_data);
	}
}

void wlr_surface_for_each_surface(struct wlr_surface *surface,
		wlr_surface_iterator_func_t iterator, void *user_data) {
	surface_for_each_surface(surface, 0, 0, iterator, user_data);
}

struct bound_acc {
	int32_t min_x, min_y;
	int32_t max_x, max_y;
};

static void handle_bounding_box_surface(struct wlr_surface *surface,
		int x, int y, void *data) {
	struct bound_acc *acc = data;

	acc->min_x = min(x, acc->min_x);
	acc->min_y = min(y, acc->min_y);

	acc->max_x = max(x + surface->current.width, acc->max_x);
	acc->max_y = max(y + surface->current.height, acc->max_y);
}

void wlr_surface_get_extends(struct wlr_surface *surface, struct wlr_box *box) {
	struct bound_acc acc = {
		.min_x = 0,
		.min_y = 0,
		.max_x = surface->current.width,
		.max_y = surface->current.height,
	};

	wlr_surface_for_each_surface(surface, handle_bounding_box_surface, &acc);

	box->x = acc.min_x;
	box->y = acc.min_y;
	box->width = acc.max_x - acc.min_x;
	box->height = acc.max_y - acc.min_y;
}

static void crop_region(pixman_region32_t *dst, pixman_region32_t *src,
		const struct wlr_box *box) {
	pixman_region32_intersect_rect(dst, src,
		box->x, box->y, box->width, box->height);
	pixman_region32_translate(dst, -box->x, -box->y);
}

void wlr_surface_get_effective_damage(struct wlr_surface *surface,
		pixman_region32_t *damage) {
	pixman_region32_clear(damage);

	// Transform and copy the buffer damage in terms of surface coordinates.
	wlr_region_transform(damage, &surface->buffer_damage,
		surface->current.transform, surface->current.buffer_width,
		surface->current.buffer_height);
	wlr_region_scale(damage, damage, 1.0 / (float)surface->current.scale);

	if (surface->current.viewport.has_src) {
		struct wlr_box src_box = {
			.x = floor(surface->current.viewport.src.x),
			.y = floor(surface->current.viewport.src.y),
			.width = ceil(surface->current.viewport.src.width),
			.height = ceil(surface->current.viewport.src.height),
		};
		crop_region(damage, damage, &src_box);
	}
	if (surface->current.viewport.has_dst) {
		int src_width, src_height;
		surface_state_viewport_src_size(&surface->current,
			&src_width, &src_height);
		float scale_x = (float)surface->current.viewport.dst_width / src_width;
		float scale_y = (float)surface->current.viewport.dst_height / src_height;
		wlr_region_scale_xy(damage, damage, scale_x, scale_y);
	}

	pixman_region32_union(damage, damage, &surface->external_damage);
}

void wlr_surface_get_buffer_source_box(struct wlr_surface *surface,
		struct wlr_fbox *box) {
	box->x = box->y = 0;
	box->width = surface->current.buffer_width;
	box->height = surface->current.buffer_height;

	if (surface->current.viewport.has_src) {
		box->x = surface->current.viewport.src.x * surface->current.scale;
		box->y = surface->current.viewport.src.y * surface->current.scale;
		box->width = surface->current.viewport.src.width * surface->current.scale;
		box->height = surface->current.viewport.src.height * surface->current.scale;

		int width, height;
		surface_state_transformed_buffer_size(&surface->current, &width, &height);
		wlr_fbox_transform(box, box,
			wlr_output_transform_invert(surface->current.transform),
			width, height);
	}
}

static const struct wl_compositor_interface compositor_impl;

static struct wlr_compositor *compositor_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_compositor_interface,
		&compositor_impl));
	return wl_resource_get_user_data(resource);
}

static void compositor_create_surface(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	struct wlr_compositor *compositor = compositor_from_resource(resource);

	struct wlr_surface *surface = surface_create(client,
		wl_resource_get_version(resource), id, compositor->renderer);
	if (surface == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wlr_signal_emit_safe(&compositor->events.new_surface, surface);
}

static void compositor_create_region(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	region_create(client, wl_resource_get_version(resource), id);
}

static const struct wl_compositor_interface compositor_impl = {
	.create_surface = compositor_create_surface,
	.create_region = compositor_create_region,
};

static void compositor_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_compositor *compositor = data;

	struct wl_resource *resource =
		wl_resource_create(wl_client, &wl_compositor_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(resource, &compositor_impl, compositor, NULL);
}

static void compositor_handle_display_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_compositor *compositor =
		wl_container_of(listener, compositor, display_destroy);
	wlr_signal_emit_safe(&compositor->events.destroy, NULL);
	wl_list_remove(&compositor->display_destroy.link);
	wl_global_destroy(compositor->global);
	free(compositor);
}

struct wlr_compositor *wlr_compositor_create(struct wl_display *display,
		struct wlr_renderer *renderer) {
	struct wlr_compositor *compositor = calloc(1, sizeof(*compositor));
	if (!compositor) {
		return NULL;
	}

	compositor->global = wl_global_create(display, &wl_compositor_interface,
		COMPOSITOR_VERSION, compositor, compositor_bind);
	if (!compositor->global) {
		free(compositor);
		return NULL;
	}
	compositor->renderer = renderer;

	wl_signal_init(&compositor->events.new_surface);
	wl_signal_init(&compositor->events.destroy);

	compositor->display_destroy.notify = compositor_handle_display_destroy;
	wl_display_add_destroy_listener(display, &compositor->display_destroy);

	return compositor;
}
