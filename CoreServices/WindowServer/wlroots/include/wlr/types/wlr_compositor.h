/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_COMPOSITOR_H
#define WLR_TYPES_WLR_COMPOSITOR_H

#include <pixman.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/addon.h>
#include <wlr/util/box.h>

enum wlr_surface_state_field {
	WLR_SURFACE_STATE_BUFFER = 1 << 0,
	WLR_SURFACE_STATE_SURFACE_DAMAGE = 1 << 1,
	WLR_SURFACE_STATE_BUFFER_DAMAGE = 1 << 2,
	WLR_SURFACE_STATE_OPAQUE_REGION = 1 << 3,
	WLR_SURFACE_STATE_INPUT_REGION = 1 << 4,
	WLR_SURFACE_STATE_TRANSFORM = 1 << 5,
	WLR_SURFACE_STATE_SCALE = 1 << 6,
	WLR_SURFACE_STATE_FRAME_CALLBACK_LIST = 1 << 7,
	WLR_SURFACE_STATE_VIEWPORT = 1 << 8,
};

struct wlr_surface_state {
	uint32_t committed; // enum wlr_surface_state_field
	// Sequence number of the surface state. Incremented on each commit, may
	// overflow.
	uint32_t seq;

	struct wlr_buffer *buffer;
	int32_t dx, dy; // relative to previous position
	pixman_region32_t surface_damage, buffer_damage; // clipped to bounds
	pixman_region32_t opaque, input;
	enum wl_output_transform transform;
	int32_t scale;
	struct wl_list frame_callback_list; // wl_resource

	int width, height; // in surface-local coordinates
	int buffer_width, buffer_height;

	struct wl_list subsurfaces_below;
	struct wl_list subsurfaces_above;

	/**
	 * The viewport is applied after the surface transform and scale.
	 *
	 * If has_src is true, the surface content is cropped to the provided
	 * rectangle. If has_dst is true, the surface content is scaled to the
	 * provided rectangle.
	 */
	struct {
		bool has_src, has_dst;
		// In coordinates after scale/transform are applied, but before the
		// destination rectangle is applied
		struct wlr_fbox src;
		int dst_width, dst_height; // in surface-local coordinates
	} viewport;

	// Number of locks that prevent this surface state from being committed.
	size_t cached_state_locks;
	struct wl_list cached_state_link; // wlr_surface.cached
};

struct wlr_surface_role {
	const char *name;
	void (*commit)(struct wlr_surface *surface);
	void (*precommit)(struct wlr_surface *surface,
		const struct wlr_surface_state *state);
};

struct wlr_surface_output {
	struct wlr_surface *surface;
	struct wlr_output *output;

	struct wl_list link; // wlr_surface::current_outputs
	struct wl_listener bind;
	struct wl_listener destroy;
};

struct wlr_surface {
	struct wl_resource *resource;
	struct wlr_renderer *renderer;
	/**
	 * The surface's buffer, if any. A surface has an attached buffer when it
	 * commits with a non-null buffer in its pending state. A surface will not
	 * have a buffer if it has never committed one, has committed a null buffer,
	 * or something went wrong with uploading the buffer.
	 */
	struct wlr_client_buffer *buffer;
	/**
	 * The buffer position, in surface-local units.
	 */
	int sx, sy;
	/**
	 * The last commit's buffer damage, in buffer-local coordinates. This
	 * contains both the damage accumulated by the client via
	 * `wlr_surface_state.surface_damage` and `wlr_surface_state.buffer_damage`.
	 * If the buffer has been resized, the whole buffer is damaged.
	 *
	 * This region needs to be scaled and transformed into output coordinates,
	 * just like the buffer's texture. In addition, if the buffer has shrunk the
	 * old size needs to be damaged and if the buffer has moved the old and new
	 * positions need to be damaged.
	 */
	pixman_region32_t buffer_damage;
	/**
	 * The last commit's damage caused by surface and its subsurfaces'
	 * movement, in surface-local coordinates.
	 */
	pixman_region32_t external_damage;
	/**
	 * The current opaque region, in surface-local coordinates. It is clipped to
	 * the surface bounds. If the surface's buffer is using a fully opaque
	 * format, this is set to the whole surface.
	 */
	pixman_region32_t opaque_region;
	/**
	 * The current input region, in surface-local coordinates. It is clipped to
	 * the surface bounds.
	 */
	pixman_region32_t input_region;
	/**
	 * `current` contains the current, committed surface state. `pending`
	 * accumulates state changes from the client between commits and shouldn't
	 * be accessed by the compositor directly.
	 */
	struct wlr_surface_state current, pending;

	struct wl_list cached; // wlr_surface_state.cached_link

	const struct wlr_surface_role *role; // the lifetime-bound role or NULL
	void *role_data; // role-specific data

	struct {
		struct wl_signal client_commit;
		struct wl_signal commit;
		struct wl_signal new_subsurface;
		struct wl_signal destroy;
	} events;

	struct wl_list current_outputs; // wlr_surface_output::link

	struct wlr_addon_set addons;
	void *data;

	// private state

	struct wl_listener renderer_destroy;

	struct {
		int32_t scale;
		enum wl_output_transform transform;
		int width, height;
		int buffer_width, buffer_height;
	} previous;
};

struct wlr_renderer;

struct wlr_compositor {
	struct wl_global *global;
	struct wlr_renderer *renderer;

	struct wl_listener display_destroy;

	struct {
		struct wl_signal new_surface;
		struct wl_signal destroy;
	} events;
};

typedef void (*wlr_surface_iterator_func_t)(struct wlr_surface *surface,
	int sx, int sy, void *data);

/**
 * Set the lifetime role for this surface. Returns 0 on success or -1 if the
 * role cannot be set.
 */
bool wlr_surface_set_role(struct wlr_surface *surface,
		const struct wlr_surface_role *role, void *role_data,
		struct wl_resource *error_resource, uint32_t error_code);

/**
 * Whether or not this surface currently has an attached buffer. A surface has
 * an attached buffer when it commits with a non-null buffer in its pending
 * state. A surface will not have a buffer if it has never committed one, has
 * committed a null buffer, or something went wrong with uploading the buffer.
 */
bool wlr_surface_has_buffer(struct wlr_surface *surface);

/**
 * Get the texture of the buffer currently attached to this surface. Returns
 * NULL if no buffer is currently attached or if something went wrong with
 * uploading the buffer.
 */
struct wlr_texture *wlr_surface_get_texture(struct wlr_surface *surface);

/**
 * Get the root of the subsurface tree for this surface. Can return NULL if
 * a surface in the tree has been destroyed.
 */
struct wlr_surface *wlr_surface_get_root_surface(struct wlr_surface *surface);

/**
 * Check if the surface accepts input events at the given surface-local
 * coordinates. Does not check the surface's subsurfaces.
 */
bool wlr_surface_point_accepts_input(struct wlr_surface *surface,
		double sx, double sy);

/**
 * Find a surface in this surface's tree that accepts input events and has all
 * parents mapped (except this surface, which can be unmapped) at the given
 * surface-local coordinates. Returns the surface and coordinates in the leaf
 * surface coordinate system or NULL if no surface is found at that location.
 */
struct wlr_surface *wlr_surface_surface_at(struct wlr_surface *surface,
		double sx, double sy, double *sub_x, double *sub_y);

void wlr_surface_send_enter(struct wlr_surface *surface,
		struct wlr_output *output);

void wlr_surface_send_leave(struct wlr_surface *surface,
		struct wlr_output *output);

void wlr_surface_send_frame_done(struct wlr_surface *surface,
		const struct timespec *when);

/**
 * Get the bounding box that contains the surface and all subsurfaces in
 * surface coordinates.
 * X and y may be negative, if there are subsurfaces with negative position.
 */
void wlr_surface_get_extends(struct wlr_surface *surface, struct wlr_box *box);

/**
 * Get the wlr_surface corresponding to a wl_surface resource. This asserts
 * that the resource is a valid wl_surface resource created by wlroots and
 * will never return NULL.
 */
struct wlr_surface *wlr_surface_from_resource(struct wl_resource *resource);

/**
 * Call `iterator` on each mapped surface in the surface tree (whether or not
 * this surface is mapped), with the surface's position relative to the root
 * surface. The function is called from root to leaves (in rendering order).
 */
void wlr_surface_for_each_surface(struct wlr_surface *surface,
	wlr_surface_iterator_func_t iterator, void *user_data);

/**
 * Get the effective surface damage in surface-local coordinate space. Besides
 * buffer damage, this includes damage induced by resizing and moving the
 * surface and its subsurfaces. The resulting damage is not expected to be
 * bounded by the surface itself.
 */
void wlr_surface_get_effective_damage(struct wlr_surface *surface,
	pixman_region32_t *damage);

/**
 * Get the source rectangle describing the region of the buffer that needs to
 * be sampled to render this surface's current state. The box is in
 * buffer-local coordinates.
 *
 * If the viewport's source rectangle is unset, the position is zero and the
 * size is the buffer's.
 */
void wlr_surface_get_buffer_source_box(struct wlr_surface *surface,
	struct wlr_fbox *box);

/**
 * Acquire a lock for the pending surface state.
 *
 * The state won't be committed before the caller releases the lock. Instead,
 * the state becomes cached. The caller needs to use wlr_surface_unlock_cached
 * to release the lock.
 *
 * Returns a surface commit sequence number for the cached state.
 */
uint32_t wlr_surface_lock_pending(struct wlr_surface *surface);

/**
 * Release a lock for a cached state.
 *
 * Callers should not assume that the cached state will immediately be
 * committed. Another caller may still have an active lock.
 */
void wlr_surface_unlock_cached(struct wlr_surface *surface, uint32_t seq);

struct wlr_compositor *wlr_compositor_create(struct wl_display *display,
	struct wlr_renderer *renderer);

#endif
