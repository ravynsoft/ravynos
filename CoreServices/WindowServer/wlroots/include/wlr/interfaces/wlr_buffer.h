/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_BUFFER_H
#define WLR_INTERFACES_WLR_BUFFER_H

#include <wlr/types/wlr_buffer.h>

struct wlr_buffer_impl {
	void (*destroy)(struct wlr_buffer *buffer);
	bool (*get_dmabuf)(struct wlr_buffer *buffer,
		struct wlr_dmabuf_attributes *attribs);
	bool (*get_shm)(struct wlr_buffer *buffer,
		struct wlr_shm_attributes *attribs);
	bool (*begin_data_ptr_access)(struct wlr_buffer *buffer, uint32_t flags,
		void **data, uint32_t *format, size_t *stride);
	void (*end_data_ptr_access)(struct wlr_buffer *buffer);
};

struct wlr_buffer_resource_interface {
	const char *name;
	bool (*is_instance)(struct wl_resource *resource);
	struct wlr_buffer *(*from_resource)(struct wl_resource *resource);
};

/**
 * Initialize a buffer. This function should be called by producers. The
 * initialized buffer is referenced: once the producer is done with the buffer
 * they should call wlr_buffer_drop.
 */
void wlr_buffer_init(struct wlr_buffer *buffer,
	const struct wlr_buffer_impl *impl, int width, int height);

/**
 * Allows the registration of a wl_resource implementation.
 *
 * The matching function will be called for the wl_resource when creating a
 * wlr_buffer from a wl_resource.
 */
void wlr_buffer_register_resource_interface(
	const struct wlr_buffer_resource_interface *iface);

#endif
