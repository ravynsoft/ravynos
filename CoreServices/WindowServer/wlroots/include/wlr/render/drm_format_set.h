#ifndef WLR_RENDER_DRM_FORMAT_SET_H
#define WLR_RENDER_DRM_FORMAT_SET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** A single DRM format, with a set of modifiers attached. */
struct wlr_drm_format {
	// The actual DRM format, from `drm_fourcc.h`
	uint32_t format;
	// The number of modifiers
	size_t len;
	// The capacity of the array; do not use.
	size_t capacity;
	// The actual modifiers
	uint64_t modifiers[];
};

/**
 * A set of DRM formats and modifiers.
 *
 * This is used to describe the supported format + modifier combinations. For
 * instance, backends will report the set they can display, and renderers will
 * report the set they can render to. For a more general overview of formats
 * and modifiers, see:
 * https://lore.kernel.org/dri-devel/20210905122742.86029-1-daniels@collabora.com/
 *
 * For compatibility with legacy drivers which don't support explicit
 * modifiers, the special modifier DRM_FORMAT_MOD_INVALID is used to indicate
 * that implicit modifiers are supported. Legacy drivers can also support the
 * DRM_FORMAT_MOD_LINEAR modifier, which forces the buffer to have a linear
 * layout.
 *
 * Users must not assume that implicit modifiers are supported unless INVALID
 * is listed in the modifier list.
 */
struct wlr_drm_format_set {
	// The number of formats
	size_t len;
	// The capacity of the array; private to wlroots
	size_t capacity;
	// A pointer to an array of `struct wlr_drm_format *` of length `len`.
	struct wlr_drm_format **formats;
};

/**
 * Free all of the DRM formats in the set, making the set empty.  Does not
 * free the set itself.
 */
void wlr_drm_format_set_finish(struct wlr_drm_format_set *set);

/**
 * Return a pointer to a member of this `wlr_drm_format_set` of format
 * `format`, or NULL if none exists.
 */
const struct wlr_drm_format *wlr_drm_format_set_get(
	const struct wlr_drm_format_set *set, uint32_t format);

bool wlr_drm_format_set_has(const struct wlr_drm_format_set *set,
	uint32_t format, uint64_t modifier);

bool wlr_drm_format_set_add(struct wlr_drm_format_set *set, uint32_t format,
	uint64_t modifier);

/**
 * Intersect two DRM format sets `a` and `b`, storing in the destination set
 * `dst` the format + modifier pairs which are in both source sets.
 *
 * Returns false on failure or when the intersection is empty.
 */
bool wlr_drm_format_set_intersect(struct wlr_drm_format_set *dst,
	const struct wlr_drm_format_set *a, const struct wlr_drm_format_set *b);

#endif
