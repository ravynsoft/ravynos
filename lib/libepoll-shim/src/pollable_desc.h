#ifndef POLLABLE_DESC_H
#define POLLABLE_DESC_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <poll.h>

struct pollable_desc_vtable;
typedef struct {
	void *ptr;
	struct pollable_desc_vtable const *vtable;
} PollableDesc;

typedef void (*pollable_desc_poll_t)(void *pollable_desc, /**/
    int fd, uint32_t *revents);
typedef void (*pollable_desc_ref_t)(void *pollable_desc);
typedef void (*pollable_desc_unref_t)(void *pollable_desc);

struct pollable_desc_vtable {
	pollable_desc_poll_t poll_fun;
	pollable_desc_ref_t ref_fun;
	pollable_desc_unref_t unref_fun;
};

static inline void
pollable_desc_poll(PollableDesc pollable_desc, int fd, uint32_t *revents)
{
	if (pollable_desc.ptr == NULL) {
		return;
	}

	assert(pollable_desc.vtable != NULL);
	assert(pollable_desc.vtable->poll_fun != NULL);

	pollable_desc.vtable->poll_fun(pollable_desc.ptr, fd, revents);
}

static inline void
pollable_desc_ref(PollableDesc pollable_desc)
{
	if (pollable_desc.ptr == NULL) {
		return;
	}

	assert(pollable_desc.vtable != NULL);
	assert(pollable_desc.vtable->ref_fun != NULL);

	pollable_desc.vtable->ref_fun(pollable_desc.ptr);
}

static inline void
pollable_desc_unref(PollableDesc pollable_desc)
{
	if (pollable_desc.ptr == NULL) {
		return;
	}

	assert(pollable_desc.vtable != NULL);
	assert(pollable_desc.vtable->unref_fun != NULL);

	pollable_desc.vtable->unref_fun(pollable_desc.ptr);
}

#endif
