#include <assert.h>
#include <stdlib.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_buffer.h>
#include "render/allocator/allocator.h"
#include "render/drm_format_set.h"
#include "render/swapchain.h"

static void swapchain_handle_allocator_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_swapchain *swapchain =
		wl_container_of(listener, swapchain, allocator_destroy);
	swapchain->allocator = NULL;
}

struct wlr_swapchain *wlr_swapchain_create(
		struct wlr_allocator *alloc, int width, int height,
		const struct wlr_drm_format *format) {
	struct wlr_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		return NULL;
	}
	swapchain->allocator = alloc;
	swapchain->width = width;
	swapchain->height = height;

	swapchain->format = wlr_drm_format_dup(format);
	if (swapchain->format == NULL) {
		free(swapchain);
		return NULL;
	}

	swapchain->allocator_destroy.notify = swapchain_handle_allocator_destroy;
	wl_signal_add(&alloc->events.destroy, &swapchain->allocator_destroy);

	return swapchain;
}

static void slot_reset(struct wlr_swapchain_slot *slot) {
	if (slot->acquired) {
		wl_list_remove(&slot->release.link);
	}
	wlr_buffer_drop(slot->buffer);
	memset(slot, 0, sizeof(*slot));
}

void wlr_swapchain_destroy(struct wlr_swapchain *swapchain) {
	if (swapchain == NULL) {
		return;
	}
	for (size_t i = 0; i < WLR_SWAPCHAIN_CAP; i++) {
		slot_reset(&swapchain->slots[i]);
	}
	wl_list_remove(&swapchain->allocator_destroy.link);
	free(swapchain->format);
	free(swapchain);
}

static void slot_handle_release(struct wl_listener *listener, void *data) {
	struct wlr_swapchain_slot *slot =
		wl_container_of(listener, slot, release);
	wl_list_remove(&slot->release.link);
	slot->acquired = false;
}

static struct wlr_buffer *slot_acquire(struct wlr_swapchain *swapchain,
		struct wlr_swapchain_slot *slot, int *age) {
	assert(!slot->acquired);
	assert(slot->buffer != NULL);

	slot->acquired = true;

	slot->release.notify = slot_handle_release;
	wl_signal_add(&slot->buffer->events.release, &slot->release);

	if (age != NULL) {
		*age = slot->age;
	}

	return wlr_buffer_lock(slot->buffer);
}

struct wlr_buffer *wlr_swapchain_acquire(struct wlr_swapchain *swapchain,
		int *age) {
	struct wlr_swapchain_slot *free_slot = NULL;
	for (size_t i = 0; i < WLR_SWAPCHAIN_CAP; i++) {
		struct wlr_swapchain_slot *slot = &swapchain->slots[i];
		if (slot->acquired) {
			continue;
		}
		if (slot->buffer != NULL) {
			return slot_acquire(swapchain, slot, age);
		}
		free_slot = slot;
	}
	if (free_slot == NULL) {
		wlr_log(WLR_ERROR, "No free output buffer slot");
		return NULL;
	}

	if (swapchain->allocator == NULL) {
		return NULL;
	}

	wlr_log(WLR_DEBUG, "Allocating new swapchain buffer");
	free_slot->buffer = wlr_allocator_create_buffer(swapchain->allocator,
		swapchain->width, swapchain->height, swapchain->format);
	if (free_slot->buffer == NULL) {
		wlr_log(WLR_ERROR, "Failed to allocate buffer");
		return NULL;
	}
	return slot_acquire(swapchain, free_slot, age);
}

static bool swapchain_has_buffer(struct wlr_swapchain *swapchain,
		struct wlr_buffer *buffer) {
	for (size_t i = 0; i < WLR_SWAPCHAIN_CAP; i++) {
		struct wlr_swapchain_slot *slot = &swapchain->slots[i];
		if (slot->buffer == buffer) {
			return true;
		}
	}
	return false;
}

void wlr_swapchain_set_buffer_submitted(struct wlr_swapchain *swapchain,
		struct wlr_buffer *buffer) {
	assert(buffer != NULL);

	if (!swapchain_has_buffer(swapchain, buffer)) {
		return;
	}

	// See the algorithm described in:
	// https://www.khronos.org/registry/EGL/extensions/EXT/EGL_EXT_buffer_age.txt
	for (size_t i = 0; i < WLR_SWAPCHAIN_CAP; i++) {
		struct wlr_swapchain_slot *slot = &swapchain->slots[i];
		if (slot->buffer == buffer) {
			slot->age = 1;
		} else if (slot->age > 0) {
			slot->age++;
		}
	}
}
