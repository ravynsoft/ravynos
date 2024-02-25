/*
 * Copyright © 2008 Kristian Høgsberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Kristian Høgsberg <krh@bitplanet.net>
 *    Benjamin Franzke <benjaminfranzke@googlemail.com>
 *
 */

#define _GNU_SOURCE

#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#include "wayland-os.h"
#include "wayland-util.h"
#include "wayland-private.h"
#include "wayland-server.h"

/* This once_t is used to synchronize installing the SIGBUS handler
 * and creating the TLS key. This will be done in the first call
 * wl_shm_buffer_begin_access which can happen from any thread */
static pthread_once_t wl_shm_sigbus_once = PTHREAD_ONCE_INIT;
static pthread_key_t wl_shm_sigbus_data_key;
static struct sigaction wl_shm_old_sigbus_action;

struct wl_shm_pool {
	struct wl_resource *resource;
	int internal_refcount;
	int external_refcount;
	char *data;
	ssize_t size;
	ssize_t new_size;
#ifndef MREMAP_MAYMOVE
	/* The following three fields are needed for mremap() emulation. */
	int mmap_fd;
	int mmap_flags;
	int mmap_prot;
#endif
	bool sigbus_is_impossible;
};

/** \class wl_shm_buffer
 *
 * \brief A SHM buffer
 *
 * wl_shm_buffer provides a helper for accessing the contents of a wl_buffer
 * resource created via the wl_shm interface.
 *
 * A wl_shm_buffer becomes invalid as soon as its #wl_resource is destroyed.
 */
struct wl_shm_buffer {
	struct wl_resource *resource;
	int32_t width, height;
	int32_t stride;
	uint32_t format;
	int offset;
	struct wl_shm_pool *pool;
};

struct wl_shm_sigbus_data {
	struct wl_shm_pool *current_pool;
	int access_count;
	int fallback_mapping_used;
};

static void *
shm_pool_grow_mapping(struct wl_shm_pool *pool)
{
	void *data;

#ifdef MREMAP_MAYMOVE
	data = mremap(pool->data, pool->size, pool->new_size, MREMAP_MAYMOVE);
#else
	data = wl_os_mremap_maymove(pool->mmap_fd, pool->data, &pool->size,
				    pool->new_size, pool->mmap_prot,
				    pool->mmap_flags);
	if (pool->size != 0 && pool->resource != NULL) {
		wl_resource_post_error(pool->resource,
				       WL_SHM_ERROR_INVALID_FD,
				       "leaked old mapping");
	}
#endif
	return data;
}

static void
shm_pool_finish_resize(struct wl_shm_pool *pool)
{
	void *data;

	if (pool->size == pool->new_size)
		return;

	data = shm_pool_grow_mapping(pool);
	if (data == MAP_FAILED) {
		if (pool->resource != NULL)
			wl_resource_post_error(pool->resource,
					       WL_SHM_ERROR_INVALID_FD,
					       "failed mremap");
		return;
	}

	pool->data = data;
	pool->size = pool->new_size;
}

static void
shm_pool_unref(struct wl_shm_pool *pool, bool external)
{
	if (external) {
		pool->external_refcount--;
		assert(pool->external_refcount >= 0);
		if (pool->external_refcount == 0)
			shm_pool_finish_resize(pool);
	} else {
		pool->internal_refcount--;
		assert(pool->internal_refcount >= 0);
	}

	if (pool->internal_refcount + pool->external_refcount > 0)
		return;

	munmap(pool->data, pool->size);
#ifndef MREMAP_MAYMOVE
	close(pool->mmap_fd);
#endif
	free(pool);
}

static void
destroy_buffer(struct wl_resource *resource)
{
	struct wl_shm_buffer *buffer = wl_resource_get_user_data(resource);

	shm_pool_unref(buffer->pool, false);
	free(buffer);
}

static void
shm_buffer_destroy(struct wl_client *client, struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static const struct wl_buffer_interface shm_buffer_interface = {
	shm_buffer_destroy
};

static bool
format_is_supported(struct wl_client *client, uint32_t format)
{
	struct wl_display *display = wl_client_get_display(client);
	struct wl_array *formats;
	uint32_t *p;

	switch (format) {
	case WL_SHM_FORMAT_ARGB8888:
	case WL_SHM_FORMAT_XRGB8888:
		return true;
	default:
		formats = wl_display_get_additional_shm_formats(display);
		wl_array_for_each(p, formats)
			if (*p == format)
				return true;
	}

	return false;
}

static void
shm_pool_create_buffer(struct wl_client *client, struct wl_resource *resource,
		       uint32_t id, int32_t offset,
		       int32_t width, int32_t height,
		       int32_t stride, uint32_t format)
{
	struct wl_shm_pool *pool = wl_resource_get_user_data(resource);
	struct wl_shm_buffer *buffer;

	if (!format_is_supported(client, format)) {
		wl_resource_post_error(resource,
				       WL_SHM_ERROR_INVALID_FORMAT,
				       "invalid format 0x%x", format);
		return;
	}

	if (offset < 0 || width <= 0 || height <= 0 || stride < width ||
	    INT32_MAX / stride < height ||
	    offset > pool->size - stride * height) {
		wl_resource_post_error(resource,
				       WL_SHM_ERROR_INVALID_STRIDE,
				       "invalid width, height or stride (%dx%d, %u)",
				       width, height, stride);
		return;
	}

	buffer = zalloc(sizeof *buffer);
	if (buffer == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	buffer->width = width;
	buffer->height = height;
	buffer->format = format;
	buffer->stride = stride;
	buffer->offset = offset;
	buffer->pool = pool;
	pool->internal_refcount++;

	buffer->resource =
		wl_resource_create(client, &wl_buffer_interface, 1, id);
	if (buffer->resource == NULL) {
		wl_client_post_no_memory(client);
		shm_pool_unref(pool, false);
		free(buffer);
		return;
	}

	wl_resource_set_implementation(buffer->resource,
				       &shm_buffer_interface,
				       buffer, destroy_buffer);
}

static void
destroy_pool(struct wl_resource *resource)
{
	struct wl_shm_pool *pool = wl_resource_get_user_data(resource);

	pool->resource = NULL;
	shm_pool_unref(pool, false);
}

static void
shm_pool_destroy(struct wl_client *client, struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
shm_pool_resize(struct wl_client *client, struct wl_resource *resource,
		int32_t size)
{
	struct wl_shm_pool *pool = wl_resource_get_user_data(resource);

	if (size < pool->size) {
		wl_resource_post_error(resource,
				       WL_SHM_ERROR_INVALID_FD,
				       "shrinking pool invalid");
		return;
	}

	pool->new_size = size;

	/* If the compositor has taken references on this pool it
	 * may be caching pointers into it. In that case we
	 * defer the resize (which may move the entire mapping)
	 * until the compositor finishes dereferencing the pool.
	 */
	if (pool->external_refcount == 0)
		shm_pool_finish_resize(pool);
}

static const struct wl_shm_pool_interface shm_pool_interface = {
	shm_pool_create_buffer,
	shm_pool_destroy,
	shm_pool_resize
};

static void
shm_create_pool(struct wl_client *client, struct wl_resource *resource,
		uint32_t id, int fd, int32_t size)
{
	struct wl_shm_pool *pool;
	struct stat statbuf;
	int seals;
	int prot;
	int flags;

	if (size <= 0) {
		wl_resource_post_error(resource,
				       WL_SHM_ERROR_INVALID_STRIDE,
				       "invalid size (%d)", size);
		goto err_close;
	}

	pool = zalloc(sizeof *pool);
	if (pool == NULL) {
		wl_client_post_no_memory(client);
		goto err_close;
	}

#ifdef HAVE_MEMFD_CREATE
	seals = fcntl(fd, F_GET_SEALS);
	if (seals == -1)
		seals = 0;

	if ((seals & F_SEAL_SHRINK) && fstat(fd, &statbuf) >= 0)
		pool->sigbus_is_impossible = statbuf.st_size >= size;
	else
		pool->sigbus_is_impossible = false;
#else
	pool->sigbus_is_impossible = false;
#endif

	pool->internal_refcount = 1;
	pool->external_refcount = 0;
	pool->size = size;
	pool->new_size = size;
	prot = PROT_READ | PROT_WRITE;
	flags = MAP_SHARED;
	pool->data = mmap(NULL, size, prot, flags, fd, 0);
	if (pool->data == MAP_FAILED) {
		wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FD,
				       "failed mmap fd %d: %s", fd,
				       strerror(errno));
		goto err_free;
	}
#ifndef MREMAP_MAYMOVE
	/* We may need to keep the fd, prot and flags to emulate mremap(). */
	pool->mmap_fd = fd;
	pool->mmap_prot = prot;
	pool->mmap_flags = flags;
#else
	close(fd);
#endif
	pool->resource =
		wl_resource_create(client, &wl_shm_pool_interface, 1, id);
	if (!pool->resource) {
		wl_client_post_no_memory(client);
		munmap(pool->data, pool->size);
		free(pool);
		return;
	}

	wl_resource_set_implementation(pool->resource,
				       &shm_pool_interface,
				       pool, destroy_pool);

	return;

err_free:
	free(pool);
err_close:
	close(fd);
}

static const struct wl_shm_interface shm_interface = {
	shm_create_pool
};

static void
bind_shm(struct wl_client *client,
	 void *data, uint32_t version, uint32_t id)
{
	struct wl_resource *resource;
	struct wl_display *display = wl_client_get_display(client);
	struct wl_array *additional_formats;
	uint32_t *p;

	resource = wl_resource_create(client, &wl_shm_interface, 1, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource, &shm_interface, data, NULL);

	wl_shm_send_format(resource, WL_SHM_FORMAT_ARGB8888);
	wl_shm_send_format(resource, WL_SHM_FORMAT_XRGB8888);

	additional_formats = wl_display_get_additional_shm_formats(display);
	wl_array_for_each(p, additional_formats)
		wl_shm_send_format(resource, *p);
}

WL_EXPORT int
wl_display_init_shm(struct wl_display *display)
{
	if (!wl_global_create(display, &wl_shm_interface, 1, NULL, bind_shm))
		return -1;

	return 0;
}

WL_EXPORT struct wl_shm_buffer *
wl_shm_buffer_get(struct wl_resource *resource)
{
	if (resource == NULL)
		return NULL;

	if (wl_resource_instance_of(resource, &wl_buffer_interface,
				    &shm_buffer_interface))
		return wl_resource_get_user_data(resource);
	else
		return NULL;
}

WL_EXPORT int32_t
wl_shm_buffer_get_stride(struct wl_shm_buffer *buffer)
{
	return buffer->stride;
}


/** Get a pointer to the memory for the SHM buffer
 *
 * \param buffer The buffer object
 *
 * Returns a pointer which can be used to read the data contained in
 * the given SHM buffer.
 *
 * As this buffer is memory-mapped, reading from it may generate
 * SIGBUS signals. This can happen if the client claims that the
 * buffer is larger than it is or if something truncates the
 * underlying file. To prevent this signal from causing the compositor
 * to crash you should call wl_shm_buffer_begin_access and
 * wl_shm_buffer_end_access around code that reads from the memory.
 *
 * \memberof wl_shm_buffer
 */
WL_EXPORT void *
wl_shm_buffer_get_data(struct wl_shm_buffer *buffer)
{
	if (buffer->pool->external_refcount &&
	    (buffer->pool->size != buffer->pool->new_size))
		wl_log("Buffer address requested when its parent pool "
		       "has an external reference and a deferred resize "
		       "pending.\n");
	return buffer->pool->data + buffer->offset;
}

WL_EXPORT uint32_t
wl_shm_buffer_get_format(struct wl_shm_buffer *buffer)
{
	return buffer->format;
}

WL_EXPORT int32_t
wl_shm_buffer_get_width(struct wl_shm_buffer *buffer)
{
	return buffer->width;
}

WL_EXPORT int32_t
wl_shm_buffer_get_height(struct wl_shm_buffer *buffer)
{
	return buffer->height;
}

/** Get a reference to a shm_buffer's shm_pool
 *
 * \param buffer The buffer object
 *
 * Returns a pointer to a buffer's shm_pool and increases the
 * shm_pool refcount.
 *
 * The compositor must remember to call wl_shm_pool_unref when
 * it no longer needs the reference to ensure proper destruction
 * of the pool.
 *
 * \memberof wl_shm_buffer
 * \sa wl_shm_pool_unref
 */
WL_EXPORT struct wl_shm_pool *
wl_shm_buffer_ref_pool(struct wl_shm_buffer *buffer)
{
	assert(buffer->pool->internal_refcount +
	       buffer->pool->external_refcount);

	buffer->pool->external_refcount++;
	return buffer->pool;
}

/** Unreference a shm_pool
 *
 * \param pool The pool object
 *
 * Drops a reference to a wl_shm_pool object.
 *
 * This is only necessary if the compositor has explicitly
 * taken a reference with wl_shm_buffer_ref_pool(), otherwise
 * the pool will be automatically destroyed when appropriate.
 *
 * \memberof wl_shm_pool
 * \sa wl_shm_buffer_ref_pool
 */
WL_EXPORT void
wl_shm_pool_unref(struct wl_shm_pool *pool)
{
	shm_pool_unref(pool, true);
}

static void
reraise_sigbus(void)
{
	/* If SIGBUS is raised for some other reason than accessing
	 * the pool then we'll uninstall the signal handler so we can
	 * reraise it. This would presumably kill the process */
	sigaction(SIGBUS, &wl_shm_old_sigbus_action, NULL);
	raise(SIGBUS);
}

static void
sigbus_handler(int signum, siginfo_t *info, void *context)
{
	struct wl_shm_sigbus_data *sigbus_data =
		pthread_getspecific(wl_shm_sigbus_data_key);
	struct wl_shm_pool *pool;

	if (sigbus_data == NULL) {
		reraise_sigbus();
		return;
	}

	pool = sigbus_data->current_pool;

	/* If the offending address is outside the mapped space for
	 * the pool then the error is a real problem so we'll reraise
	 * the signal */
	if (pool == NULL ||
	    (char *) info->si_addr < pool->data ||
	    (char *) info->si_addr >= pool->data + pool->size) {
		reraise_sigbus();
		return;
	}

	sigbus_data->fallback_mapping_used = 1;

	/* This should replace the previous mapping */
	if (mmap(pool->data, pool->size, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, 0, 0) == MAP_FAILED) {
		reraise_sigbus();
		return;
	}
}

static void
destroy_sigbus_data(void *data)
{
	struct wl_shm_sigbus_data *sigbus_data = data;

	free(sigbus_data);
}

static void
init_sigbus_data_key(void)
{
	struct sigaction new_action = {
		.sa_sigaction = sigbus_handler,
		.sa_flags = SA_SIGINFO | SA_NODEFER
	};

	sigemptyset(&new_action.sa_mask);

	sigaction(SIGBUS, &new_action, &wl_shm_old_sigbus_action);

	pthread_key_create(&wl_shm_sigbus_data_key, destroy_sigbus_data);
}

/** Mark that the given SHM buffer is about to be accessed
 *
 * \param buffer The SHM buffer
 *
 * An SHM buffer is a memory-mapped file given by the client.
 * According to POSIX, reading from a memory-mapped region that
 * extends off the end of the file will cause a SIGBUS signal to be
 * generated. Normally this would cause the compositor to terminate.
 * In order to make the compositor robust against clients that change
 * the size of the underlying file or lie about its size, you should
 * protect access to the buffer by calling this function before
 * reading from the memory and call wl_shm_buffer_end_access
 * afterwards. This will install a signal handler for SIGBUS which
 * will prevent the compositor from crashing.
 *
 * After calling this function the signal handler will remain
 * installed for the lifetime of the compositor process. Note that
 * this function will not work properly if the compositor is also
 * installing its own handler for SIGBUS.
 *
 * If a SIGBUS signal is received for an address within the range of
 * the SHM pool of the given buffer then the client will be sent an
 * error event when wl_shm_buffer_end_access is called. If the signal
 * is for an address outside that range then the signal handler will
 * reraise the signal which would will likely cause the compositor to
 * terminate.
 *
 * It is safe to nest calls to these functions as long as the nested
 * calls are all accessing the same buffer. The number of calls to
 * wl_shm_buffer_end_access must match the number of calls to
 * wl_shm_buffer_begin_access. These functions are thread-safe and it
 * is allowed to simultaneously access different buffers or the same
 * buffer from multiple threads.
 *
 * \memberof wl_shm_buffer
 */
WL_EXPORT void
wl_shm_buffer_begin_access(struct wl_shm_buffer *buffer)
{
	struct wl_shm_pool *pool = buffer->pool;
	struct wl_shm_sigbus_data *sigbus_data;

	if (pool->sigbus_is_impossible)
		return;

	pthread_once(&wl_shm_sigbus_once, init_sigbus_data_key);

	sigbus_data = pthread_getspecific(wl_shm_sigbus_data_key);
	if (sigbus_data == NULL) {
		sigbus_data = zalloc(sizeof *sigbus_data);
		if (sigbus_data == NULL)
			return;

		pthread_setspecific(wl_shm_sigbus_data_key, sigbus_data);
	}

	assert(sigbus_data->current_pool == NULL ||
	       sigbus_data->current_pool == pool);

	sigbus_data->current_pool = pool;
	sigbus_data->access_count++;
}

/** Ends the access to a buffer started by wl_shm_buffer_begin_access
 *
 * \param buffer The SHM buffer
 *
 * This should be called after wl_shm_buffer_begin_access once the
 * buffer is no longer being accessed. If a SIGBUS signal was
 * generated in-between these two calls then the resource for the
 * given buffer will be sent an error.
 *
 * \memberof wl_shm_buffer
 */
WL_EXPORT void
wl_shm_buffer_end_access(struct wl_shm_buffer *buffer)
{
	struct wl_shm_pool *pool = buffer->pool;
	struct wl_shm_sigbus_data *sigbus_data;

	if (pool->sigbus_is_impossible)
		return;

	sigbus_data = pthread_getspecific(wl_shm_sigbus_data_key);
	assert(sigbus_data && sigbus_data->access_count >= 1);

	if (--sigbus_data->access_count == 0) {
		if (sigbus_data->fallback_mapping_used) {
			wl_resource_post_error(buffer->resource,
					       WL_SHM_ERROR_INVALID_FD,
					       "error accessing SHM buffer");
			sigbus_data->fallback_mapping_used = 0;
		}

		sigbus_data->current_pool = NULL;
	}
}

/** \cond */ /* Deprecated functions below. */

WL_EXPORT struct wl_shm_buffer *
wl_shm_buffer_create(struct wl_client *client,
		     uint32_t id, int32_t width, int32_t height,
		     int32_t stride, uint32_t format)
{
	return NULL;
}

/** \endcond */

/* Functions at the end of this file are deprecated.  Instead of adding new
 * code here, add it before the comment above that states:
 * Deprecated functions below.
 */
