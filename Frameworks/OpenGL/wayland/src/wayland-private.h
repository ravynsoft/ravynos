/*
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2011 Intel Corporation
 * Copyright © 2013 Jason Ekstrand
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
 */

#ifndef WAYLAND_PRIVATE_H
#define WAYLAND_PRIVATE_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define WL_HIDE_DEPRECATED 1

#include "wayland-util.h"

/* Invalid memory address */
#define WL_ARRAY_POISON_PTR (void *) 4

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

#define WL_MAP_SERVER_SIDE 0
#define WL_MAP_CLIENT_SIDE 1
#define WL_SERVER_ID_START 0xff000000
#define WL_MAP_MAX_OBJECTS 0x00f00000
#define WL_CLOSURE_MAX_ARGS 20

struct wl_object {
	const struct wl_interface *interface;
	const void *implementation;
	uint32_t id;
};

int
wl_interface_equal(const struct wl_interface *iface1,
		   const struct wl_interface *iface2);

/* Flags for wl_map_insert_new and wl_map_insert_at.  Flags can be queried with
 * wl_map_lookup_flags.  The current implementation has room for 1 bit worth of
 * flags.  If more flags are ever added, the implementation of wl_map will have
 * to change to allow for new flags */
enum wl_map_entry_flags {
	WL_MAP_ENTRY_LEGACY = (1 << 0), /* Server side only */
	WL_MAP_ENTRY_ZOMBIE = (1 << 0) /* Client side only */
};

struct wl_map {
	struct wl_array client_entries;
	struct wl_array server_entries;
	uint32_t side;
	uint32_t free_list;
};

typedef enum wl_iterator_result (*wl_iterator_func_t)(void *element,
						      void *data,
						      uint32_t flags);

void
wl_map_init(struct wl_map *map, uint32_t side);

void
wl_map_release(struct wl_map *map);

uint32_t
wl_map_insert_new(struct wl_map *map, uint32_t flags, void *data);

int
wl_map_insert_at(struct wl_map *map, uint32_t flags, uint32_t i, void *data);

int
wl_map_reserve_new(struct wl_map *map, uint32_t i);

void
wl_map_remove(struct wl_map *map, uint32_t i);

void *
wl_map_lookup(struct wl_map *map, uint32_t i);

uint32_t
wl_map_lookup_flags(struct wl_map *map, uint32_t i);

void
wl_map_for_each(struct wl_map *map, wl_iterator_func_t func, void *data);

struct wl_connection *
wl_connection_create(int fd);

int
wl_connection_destroy(struct wl_connection *connection);

void
wl_connection_copy(struct wl_connection *connection, void *data, size_t size);

void
wl_connection_consume(struct wl_connection *connection, size_t size);

int
wl_connection_flush(struct wl_connection *connection);

uint32_t
wl_connection_pending_input(struct wl_connection *connection);

int
wl_connection_read(struct wl_connection *connection);

int
wl_connection_write(struct wl_connection *connection,
		    const void *data, size_t count);

int
wl_connection_queue(struct wl_connection *connection,
		    const void *data, size_t count);

int
wl_connection_get_fd(struct wl_connection *connection);

struct wl_closure {
	int count;
	const struct wl_message *message;
	uint32_t opcode;
	uint32_t sender_id;
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	struct wl_list link;
	struct wl_proxy *proxy;
	struct wl_array extra[0];
};

struct argument_details {
	char type;
	int nullable;
};

const char *
get_next_argument(const char *signature, struct argument_details *details);

int
arg_count_for_signature(const char *signature);

int
wl_message_count_arrays(const struct wl_message *message);

int
wl_message_get_since(const struct wl_message *message);

void
wl_argument_from_va_list(const char *signature, union wl_argument *args,
			 int count, va_list ap);

struct wl_closure *
wl_closure_marshal(struct wl_object *sender,
		    uint32_t opcode, union wl_argument *args,
		    const struct wl_message *message);

struct wl_closure *
wl_closure_vmarshal(struct wl_object *sender,
		    uint32_t opcode, va_list ap,
		    const struct wl_message *message);

struct wl_closure *
wl_connection_demarshal(struct wl_connection *connection,
			uint32_t size,
			struct wl_map *objects,
			const struct wl_message *message);

bool
wl_object_is_zombie(struct wl_map *map, uint32_t id);

int
wl_closure_lookup_objects(struct wl_closure *closure, struct wl_map *objects);

enum wl_closure_invoke_flag {
	WL_CLOSURE_INVOKE_CLIENT = (1 << 0),
	WL_CLOSURE_INVOKE_SERVER = (1 << 1)
};

void
wl_closure_invoke(struct wl_closure *closure, uint32_t flags,
		  struct wl_object *target, uint32_t opcode, void *data);

void
wl_closure_dispatch(struct wl_closure *closure, wl_dispatcher_func_t dispatcher,
		    struct wl_object *target, uint32_t opcode);

int
wl_closure_send(struct wl_closure *closure, struct wl_connection *connection);

int
wl_closure_queue(struct wl_closure *closure, struct wl_connection *connection);

void
wl_closure_print(struct wl_closure *closure,
		 struct wl_object *target, int send, int discarded,
		 uint32_t (*n_parse)(union wl_argument *arg));

void
wl_closure_destroy(struct wl_closure *closure);

extern wl_log_func_t wl_log_handler;

void wl_log(const char *fmt, ...);
void wl_abort(const char *fmt, ...);

struct wl_display;

struct wl_array *
wl_display_get_additional_shm_formats(struct wl_display *display);

static inline void *
zalloc(size_t s)
{
	return calloc(1, s);
}

void
wl_connection_close_fds_in(struct wl_connection *connection, int max);

#endif
