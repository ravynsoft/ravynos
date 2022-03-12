/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_BUFFER_H
#define WLR_TYPES_WLR_BUFFER_H

#include <pixman.h>
#include <wayland-server-core.h>
#include <wlr/render/dmabuf.h>
#include <wlr/util/addon.h>

struct wlr_buffer;
struct wlr_renderer;

struct wlr_shm_attributes {
	int fd;
	uint32_t format;
	int width, height, stride;
	off_t offset;
};

/**
 * Buffer capabilities.
 *
 * These bits indicate the features supported by a wlr_buffer. There is one bit
 * per function in wlr_buffer_impl.
 */
enum wlr_buffer_cap {
	WLR_BUFFER_CAP_DATA_PTR = 1 << 0,
	WLR_BUFFER_CAP_DMABUF = 1 << 1,
	WLR_BUFFER_CAP_SHM = 1 << 2,
};

/**
 * A buffer containing pixel data.
 *
 * A buffer has a single producer (the party who created the buffer) and
 * multiple consumers (parties reading the buffer). When all consumers are done
 * with the buffer, it gets released and can be re-used by the producer. When
 * the producer and all consumers are done with the buffer, it gets destroyed.
 */
struct wlr_buffer {
	const struct wlr_buffer_impl *impl;

	int width, height;

	bool dropped;
	size_t n_locks;
	bool accessing_data_ptr;

	struct {
		struct wl_signal destroy;
		struct wl_signal release;
	} events;

	struct wlr_addon_set addons;
};

/**
 * Unreference the buffer. This function should be called by producers when
 * they are done with the buffer.
 */
void wlr_buffer_drop(struct wlr_buffer *buffer);
/**
 * Lock the buffer. This function should be called by consumers to make
 * sure the buffer can be safely read from. Once the consumer is done with the
 * buffer, they should call wlr_buffer_unlock.
 */
struct wlr_buffer *wlr_buffer_lock(struct wlr_buffer *buffer);
/**
 * Unlock the buffer. This function should be called by consumers once they are
 * done with the buffer.
 */
void wlr_buffer_unlock(struct wlr_buffer *buffer);
/**
 * Reads the DMA-BUF attributes of the buffer. If this buffer isn't a DMA-BUF,
 * returns false.
 *
 * The returned DMA-BUF attributes are valid for the lifetime of the
 * wlr_buffer. The caller isn't responsible for cleaning up the DMA-BUF
 * attributes.
 */
bool wlr_buffer_get_dmabuf(struct wlr_buffer *buffer,
	struct wlr_dmabuf_attributes *attribs);
/**
 * Read shared memory attributes of the buffer. If this buffer isn't shared
 * memory, returns false.
 *
 * The returned shared memory attributes are valid for the lifetime of the
 * wlr_buffer. The caller isn't responsible for cleaning up the shared memory
 * attributes.
 */
bool wlr_buffer_get_shm(struct wlr_buffer *buffer,
	struct wlr_shm_attributes *attribs);
/**
 * Transforms a wl_resource into a wlr_buffer and locks it. Once the caller is
 * done with the buffer, they must call wlr_buffer_unlock.
 *
 * The provided wl_resource must be a wl_buffer.
 */
struct wlr_buffer *wlr_buffer_from_resource(struct wl_resource *resource);

/**
 * Buffer data pointer access flags.
 */
enum wlr_buffer_data_ptr_access_flag {
	/**
	 * The buffer contents can be read back.
	 */
	WLR_BUFFER_DATA_PTR_ACCESS_READ = 1 << 0,
	/**
	 * The buffer contents can be written to.
	 */
	WLR_BUFFER_DATA_PTR_ACCESS_WRITE = 1 << 1,
};

/**
 * Get a pointer to a region of memory referring to the buffer's underlying
 * storage. The format and stride can be used to interpret the memory region
 * contents.
 *
 * The returned pointer should be pointing to a valid memory region for the
 * operations specified in the flags. The returned pointer is only valid up to
 * the next buffer_end_data_ptr_access call.
 */
bool wlr_buffer_begin_data_ptr_access(struct wlr_buffer *buffer, uint32_t flags,
	void **data, uint32_t *format, size_t *stride);
void wlr_buffer_end_data_ptr_access(struct wlr_buffer *buffer);

/**
 * A client buffer.
 */
struct wlr_client_buffer {
	struct wlr_buffer base;

	/**
	 * The buffer's texture, if any. A buffer will not have a texture if the
	 * client destroys the buffer before it has been released.
	 */
	struct wlr_texture *texture;
	/**
	 * The buffer this client buffer was created from. NULL if destroyed.
	 */
	struct wlr_buffer *source;

	// private state

	struct wl_listener source_destroy;

	// If the client buffer has been created from a wl_shm buffer
	uint32_t shm_source_format;
};

/**
 * Creates a wlr_client_buffer from a given wlr_buffer by creating a texture
 * from it, and copying its wl_resource.
 */
struct wlr_client_buffer *wlr_client_buffer_create(struct wlr_buffer *buffer,
	struct wlr_renderer *renderer);

/**
 * Get a client buffer from a generic buffer. If the buffer isn't a client
 * buffer, returns NULL.
 */
struct wlr_client_buffer *wlr_client_buffer_get(struct wlr_buffer *buffer);
/**
 * Check if a resource is a wl_buffer resource.
 */
bool wlr_resource_is_buffer(struct wl_resource *resource);
/**
 * Try to update the buffer's content.
 *
 * Fails if there's more than one reference to the buffer or if the texture
 * isn't mutable.
 */
bool wlr_client_buffer_apply_damage(struct wlr_client_buffer *client_buffer,
	struct wlr_buffer *next, pixman_region32_t *damage);

#endif
