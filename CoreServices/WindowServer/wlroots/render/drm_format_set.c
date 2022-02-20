#include <assert.h>
#include <drm_fourcc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/render/drm_format_set.h>
#include <wlr/util/log.h>
#include "render/drm_format_set.h"

void wlr_drm_format_set_finish(struct wlr_drm_format_set *set) {
	for (size_t i = 0; i < set->len; ++i) {
		free(set->formats[i]);
	}
	free(set->formats);

	set->len = 0;
	set->capacity = 0;
	set->formats = NULL;
}

static struct wlr_drm_format **format_set_get_ref(struct wlr_drm_format_set *set,
		uint32_t format) {
	for (size_t i = 0; i < set->len; ++i) {
		if (set->formats[i]->format == format) {
			return &set->formats[i];
		}
	}

	return NULL;
}

const struct wlr_drm_format *wlr_drm_format_set_get(
		const struct wlr_drm_format_set *set, uint32_t format) {
	struct wlr_drm_format **ptr =
		format_set_get_ref((struct wlr_drm_format_set *)set, format);
	return ptr ? *ptr : NULL;
}

bool wlr_drm_format_set_has(const struct wlr_drm_format_set *set,
		uint32_t format, uint64_t modifier) {
	const struct wlr_drm_format *fmt = wlr_drm_format_set_get(set, format);
	if (!fmt) {
		return false;
	}
	return wlr_drm_format_has(fmt, modifier);
}

bool wlr_drm_format_set_add(struct wlr_drm_format_set *set, uint32_t format,
		uint64_t modifier) {
	assert(format != DRM_FORMAT_INVALID);

	struct wlr_drm_format **ptr = format_set_get_ref(set, format);
	if (ptr) {
		return wlr_drm_format_add(ptr, modifier);
	}

	struct wlr_drm_format *fmt = wlr_drm_format_create(format);
	if (!fmt) {
		return false;
	}
	if (!wlr_drm_format_add(&fmt, modifier)) {
		return false;
	}

	if (set->len == set->capacity) {
		size_t new = set->capacity ? set->capacity * 2 : 4;

		struct wlr_drm_format **tmp = realloc(set->formats,
			sizeof(*fmt) + sizeof(fmt->modifiers[0]) * new);
		if (!tmp) {
			wlr_log_errno(WLR_ERROR, "Allocation failed");
			free(fmt);
			return false;
		}

		set->capacity = new;
		set->formats = tmp;
	}

	set->formats[set->len++] = fmt;
	return true;
}

struct wlr_drm_format *wlr_drm_format_create(uint32_t format) {
	size_t capacity = 4;
	struct wlr_drm_format *fmt =
		calloc(1, sizeof(*fmt) + sizeof(fmt->modifiers[0]) * capacity);
	if (!fmt) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}
	fmt->format = format;
	fmt->capacity = capacity;
	return fmt;
}

bool wlr_drm_format_has(const struct wlr_drm_format *fmt, uint64_t modifier) {
	for (size_t i = 0; i < fmt->len; ++i) {
		if (fmt->modifiers[i] == modifier) {
			return true;
		}
	}
	return false;
}

bool wlr_drm_format_add(struct wlr_drm_format **fmt_ptr, uint64_t modifier) {
	struct wlr_drm_format *fmt = *fmt_ptr;

	if (wlr_drm_format_has(fmt, modifier)) {
		return true;
	}

	if (fmt->len == fmt->capacity) {
		size_t capacity = fmt->capacity ? fmt->capacity * 2 : 4;

		fmt = realloc(fmt, sizeof(*fmt) + sizeof(fmt->modifiers[0]) * capacity);
		if (!fmt) {
			wlr_log_errno(WLR_ERROR, "Allocation failed");
			return false;
		}

		fmt->capacity = capacity;
		*fmt_ptr = fmt;
	}

	fmt->modifiers[fmt->len++] = modifier;
	return true;
}

struct wlr_drm_format *wlr_drm_format_dup(const struct wlr_drm_format *format) {
	assert(format->len <= format->capacity);
	size_t format_size = sizeof(struct wlr_drm_format) +
		format->capacity * sizeof(format->modifiers[0]);
	struct wlr_drm_format *duped_format = malloc(format_size);
	if (duped_format == NULL) {
		return NULL;
	}
	memcpy(duped_format, format, format_size);
	return duped_format;
}

struct wlr_drm_format *wlr_drm_format_intersect(
		const struct wlr_drm_format *a, const struct wlr_drm_format *b) {
	assert(a->format == b->format);

	size_t format_cap = a->len < b->len ? a->len : b->len;
	size_t format_size = sizeof(struct wlr_drm_format) +
		format_cap * sizeof(a->modifiers[0]);
	struct wlr_drm_format *format = calloc(1, format_size);
	if (format == NULL) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}
	format->format = a->format;
	format->capacity = format_cap;

	for (size_t i = 0; i < a->len; i++) {
		for (size_t j = 0; j < b->len; j++) {
			if (a->modifiers[i] == b->modifiers[j]) {
				assert(format->len < format->capacity);
				format->modifiers[format->len] = a->modifiers[i];
				format->len++;
				break;
			}
		}
	}

	// If the intersection is empty, then the formats aren't compatible with
	// each other.
	if (format->len == 0) {
		free(format);
		return NULL;
	}

	return format;
}

bool wlr_drm_format_set_intersect(struct wlr_drm_format_set *dst,
		const struct wlr_drm_format_set *a, const struct wlr_drm_format_set *b) {
	assert(dst != a && dst != b);

	struct wlr_drm_format_set out = {0};
	out.capacity = a->len < b->len ? a->len : b->len;
	out.formats = calloc(out.capacity, sizeof(struct wlr_drm_format *));
	if (out.formats == NULL) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return false;
	}

	for (size_t i = 0; i < a->len; i++) {
		for (size_t j = 0; j < b->len; j++) {
			if (a->formats[i]->format == b->formats[j]->format) {
				// When the two formats have no common modifier, keep
				// intersecting the rest of the formats: they may be compatible
				// with each other
				struct wlr_drm_format *format =
					wlr_drm_format_intersect(a->formats[i], b->formats[j]);
				if (format != NULL) {
					out.formats[out.len] = format;
					out.len++;
				}
				break;
			}
		}
	}

	if (out.len == 0) {
		wlr_drm_format_set_finish(&out);
		return false;
	}

	*dst = out;
	return true;
}
