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
 */

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dlfcn.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "wayland-util.h"
#include "wayland-private.h"
#include "wayland-server-private.h"
#include "wayland-server.h"
#include "wayland-os.h"

/* This is the size of the char array in struct sock_addr_un.
 * No Wayland socket can be created with a path longer than this,
 * including the null terminator.
 */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX	108
#endif

#define LOCK_SUFFIX	".lock"
#define LOCK_SUFFIXLEN	5

struct wl_socket {
	int fd;
	int fd_lock;
	struct sockaddr_un addr;
	char lock_addr[UNIX_PATH_MAX + LOCK_SUFFIXLEN];
	struct wl_list link;
	struct wl_event_source *source;
	char *display_name;
};

struct wl_client {
	struct wl_connection *connection;
	struct wl_event_source *source;
	struct wl_display *display;
	struct wl_resource *display_resource;
	struct wl_list link;
	struct wl_map objects;
	struct wl_priv_signal destroy_signal;
	struct wl_priv_signal destroy_late_signal;
	pid_t pid;
	uid_t uid;
	gid_t gid;
	int error;
	struct wl_priv_signal resource_created_signal;
};

struct wl_display {
	struct wl_event_loop *loop;
	int run;

	uint32_t next_global_name;
	uint32_t serial;

	struct wl_list registry_resource_list;
	struct wl_list global_list;
	struct wl_list socket_list;
	struct wl_list client_list;
	struct wl_list protocol_loggers;

	struct wl_priv_signal destroy_signal;
	struct wl_priv_signal create_client_signal;

	struct wl_array additional_shm_formats;

	wl_display_global_filter_func_t global_filter;
	void *global_filter_data;

	int terminate_efd;
	struct wl_event_source *term_source;
};

struct wl_global {
	struct wl_display *display;
	const struct wl_interface *interface;
	uint32_t name;
	uint32_t version;
	void *data;
	wl_global_bind_func_t bind;
	struct wl_list link;
	bool removed;
};

struct wl_resource {
	struct wl_object object;
	wl_resource_destroy_func_t destroy;
	struct wl_list link;
	/* Unfortunately some users of libwayland (e.g. mesa) still use the
	 * deprecated wl_resource struct, even if creating it with the new
	 * wl_resource_create(). So we cannot change the layout of the struct
	 * unless after the data field. */
	struct wl_signal deprecated_destroy_signal;
	struct wl_client *client;
	void *data;
	int version;
	wl_dispatcher_func_t dispatcher;
	struct wl_priv_signal destroy_signal;
};

struct wl_protocol_logger {
	struct wl_list link;
	wl_protocol_logger_func_t func;
	void *user_data;
};

static int debug_server = 0;

static void
log_closure(struct wl_resource *resource,
	    struct wl_closure *closure, int send)
{
	struct wl_object *object = &resource->object;
	struct wl_display *display = resource->client->display;
	struct wl_protocol_logger *protocol_logger;
	struct wl_protocol_logger_message message;

	if (debug_server)
		wl_closure_print(closure, object, send, false, NULL);

	if (!wl_list_empty(&display->protocol_loggers)) {
		message.resource = resource;
		message.message_opcode = closure->opcode;
		message.message = closure->message;
		message.arguments_count = closure->count;
		message.arguments = closure->args;
		wl_list_for_each(protocol_logger,
				 &display->protocol_loggers, link) {
			protocol_logger->func(protocol_logger->user_data,
					      send ? WL_PROTOCOL_LOGGER_EVENT :
						     WL_PROTOCOL_LOGGER_REQUEST,
					      &message);
		}
	}
}

static bool
verify_objects(struct wl_resource *resource, uint32_t opcode,
	       union wl_argument *args)
{
	struct wl_object *object = &resource->object;
	const char *signature = object->interface->events[opcode].signature;
	struct argument_details arg;
	struct wl_resource *res;
	int count, i;

	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		switch (arg.type) {
		case 'n':
		case 'o':
			res = (struct wl_resource *) (args[i].o);
			if (res && res->client != resource->client) {
				wl_log("compositor bug: The compositor "
				       "tried to use an object from one "
				       "client in a '%s.%s' for a different "
				       "client.\n", object->interface->name,
				       object->interface->events[opcode].name);
				return false;
			}
		}
	}
	return true;
}

static void
handle_array(struct wl_resource *resource, uint32_t opcode,
	     union wl_argument *args,
	     int (*send_func)(struct wl_closure *, struct wl_connection *))
{
	struct wl_closure *closure;
	struct wl_object *object = &resource->object;

	if (resource->client->error)
		return;

	if (!verify_objects(resource, opcode, args)) {
		resource->client->error = 1;
		return;
	}

	closure = wl_closure_marshal(object, opcode, args,
				     &object->interface->events[opcode]);

	if (closure == NULL) {
		resource->client->error = 1;
		return;
	}

	log_closure(resource, closure, true);

	if (send_func(closure, resource->client->connection))
		resource->client->error = 1;

	wl_closure_destroy(closure);
}

WL_EXPORT void
wl_resource_post_event_array(struct wl_resource *resource, uint32_t opcode,
			     union wl_argument *args)
{
	handle_array(resource, opcode, args, wl_closure_send);
}

WL_EXPORT void
wl_resource_post_event(struct wl_resource *resource, uint32_t opcode, ...)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	struct wl_object *object = &resource->object;
	va_list ap;

	va_start(ap, opcode);
	wl_argument_from_va_list(object->interface->events[opcode].signature,
				 args, WL_CLOSURE_MAX_ARGS, ap);
	va_end(ap);

	wl_resource_post_event_array(resource, opcode, args);
}


WL_EXPORT void
wl_resource_queue_event_array(struct wl_resource *resource, uint32_t opcode,
			      union wl_argument *args)
{
	handle_array(resource, opcode, args, wl_closure_queue);
}

WL_EXPORT void
wl_resource_queue_event(struct wl_resource *resource, uint32_t opcode, ...)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	struct wl_object *object = &resource->object;
	va_list ap;

	va_start(ap, opcode);
	wl_argument_from_va_list(object->interface->events[opcode].signature,
				 args, WL_CLOSURE_MAX_ARGS, ap);
	va_end(ap);

	wl_resource_queue_event_array(resource, opcode, args);
}

static void
wl_resource_post_error_vargs(struct wl_resource *resource,
			     uint32_t code, const char *msg, va_list argp)
{
	struct wl_client *client = resource->client;
	char buffer[128];

	vsnprintf(buffer, sizeof buffer, msg, argp);

	/*
	 * When a client aborts, its resources are destroyed in id order,
	 * which means the display resource is destroyed first. If destruction
	 * of any later resources results in a protocol error, we end up here
	 * with a NULL display_resource. Do not try to send errors to an
	 * already dead client.
	 */
	if (client->error || !client->display_resource)
		return;

	wl_resource_post_event(client->display_resource,
			       WL_DISPLAY_ERROR, resource, code, buffer);
	client->error = 1;

}

WL_EXPORT void
wl_resource_post_error(struct wl_resource *resource,
		       uint32_t code, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	wl_resource_post_error_vargs(resource, code, msg, ap);
	va_end(ap);
}

static void
destroy_client_with_error(struct wl_client *client, const char *reason)
{
	wl_log("%s (pid %u)\n", reason, client->pid);
	wl_client_destroy(client);
}

static int
wl_client_connection_data(int fd, uint32_t mask, void *data)
{
	struct wl_client *client = data;
	struct wl_connection *connection = client->connection;
	struct wl_resource *resource;
	struct wl_object *object;
	struct wl_closure *closure;
	const struct wl_message *message;
	uint32_t p[2];
	uint32_t resource_flags;
	int opcode, size, since;
	int len;

	if (mask & WL_EVENT_HANGUP) {
		wl_client_destroy(client);
		return 1;
	}

	if (mask & WL_EVENT_ERROR) {
		destroy_client_with_error(client, "socket error");
		return 1;
	}

	if (mask & WL_EVENT_WRITABLE) {
		len = wl_connection_flush(connection);
		if (len < 0 && errno != EAGAIN) {
			destroy_client_with_error(
			    client, "failed to flush client connection");
			return 1;
		} else if (len >= 0) {
			wl_event_source_fd_update(client->source,
						  WL_EVENT_READABLE);
		}
	}

	len = 0;
	if (mask & WL_EVENT_READABLE) {
		len = wl_connection_read(connection);
		if (len == 0 || (len < 0 && errno != EAGAIN)) {
			destroy_client_with_error(
			    client, "failed to read client connection");
			return 1;
		}
	}

	while (len >= 0 && (size_t) len >= sizeof p) {
		wl_connection_copy(connection, p, sizeof p);
		opcode = p[1] & 0xffff;
		size = p[1] >> 16;
		if (len < size)
			break;

		resource = wl_map_lookup(&client->objects, p[0]);
		resource_flags = wl_map_lookup_flags(&client->objects, p[0]);
		if (resource == NULL) {
			wl_resource_post_error(client->display_resource,
					       WL_DISPLAY_ERROR_INVALID_OBJECT,
					       "invalid object %u", p[0]);
			break;
		}

		object = &resource->object;
		if (opcode >= object->interface->method_count) {
			wl_resource_post_error(client->display_resource,
					       WL_DISPLAY_ERROR_INVALID_METHOD,
					       "invalid method %d, object %s@%u",
					       opcode,
					       object->interface->name,
					       object->id);
			break;
		}

		message = &object->interface->methods[opcode];
		since = wl_message_get_since(message);
		if (!(resource_flags & WL_MAP_ENTRY_LEGACY) &&
		    resource->version > 0 && resource->version < since) {
			wl_resource_post_error(client->display_resource,
					       WL_DISPLAY_ERROR_INVALID_METHOD,
					       "invalid method %d (since %d < %d)"
					       ", object %s@%u",
					       opcode, resource->version, since,
					       object->interface->name,
					       object->id);
			break;
		}


		closure = wl_connection_demarshal(client->connection, size,
						  &client->objects, message);

		if (closure == NULL && errno == ENOMEM) {
			wl_resource_post_no_memory(resource);
			break;
		} else if (closure == NULL ||
			   wl_closure_lookup_objects(closure, &client->objects) < 0) {
			wl_resource_post_error(client->display_resource,
					       WL_DISPLAY_ERROR_INVALID_METHOD,
					       "invalid arguments for %s@%u.%s",
					       object->interface->name,
					       object->id,
					       message->name);
			wl_closure_destroy(closure);
			break;
		}

		log_closure(resource, closure, false);

		if ((resource_flags & WL_MAP_ENTRY_LEGACY) ||
		    resource->dispatcher == NULL) {
			wl_closure_invoke(closure, WL_CLOSURE_INVOKE_SERVER,
					  object, opcode, client);
		} else {
			wl_closure_dispatch(closure, resource->dispatcher,
					    object, opcode);
		}

		wl_closure_destroy(closure);

		if (client->error)
			break;

		len = wl_connection_pending_input(connection);
	}

	if (client->error) {
		destroy_client_with_error(client,
					  "error in client communication");
	}

	return 1;
}

/** Flush pending events to the client
 *
 * \param client The client object
 *
 * Events sent to clients are queued in a buffer and written to the
 * socket later - typically when the compositor has handled all
 * requests and goes back to block in the event loop.  This function
 * flushes all queued up events for a client immediately.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_flush(struct wl_client *client)
{
	wl_connection_flush(client->connection);
}

/** Get the display object for the given client
 *
 * \param client The client object
 * \return The display object the client is associated with.
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_display *
wl_client_get_display(struct wl_client *client)
{
	return client->display;
}

static int
bind_display(struct wl_client *client, struct wl_display *display);

/** Create a client for the given file descriptor
 *
 * \param display The display object
 * \param fd The file descriptor for the socket to the client
 * \return The new client object or NULL on failure.
 *
 * Given a file descriptor corresponding to one end of a socket, this
 * function will create a wl_client struct and add the new client to
 * the compositors client list.  At that point, the client is
 * initialized and ready to run, as if the client had connected to the
 * servers listening socket.  When the client eventually sends
 * requests to the compositor, the wl_client argument to the request
 * handler will be the wl_client returned from this function.
 *
 * The other end of the socket can be passed to
 * wl_display_connect_to_fd() on the client side or used with the
 * WAYLAND_SOCKET environment variable on the client side.
 *
 * Listeners added with wl_display_add_client_created_listener() will
 * be notified by this function after the client is fully constructed.
 *
 * On failure this function sets errno accordingly and returns NULL.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_client *
wl_client_create(struct wl_display *display, int fd)
{
	struct wl_client *client;

	client = zalloc(sizeof *client);
	if (client == NULL)
		return NULL;

	wl_priv_signal_init(&client->resource_created_signal);
	client->display = display;
	client->source = wl_event_loop_add_fd(display->loop, fd,
					      WL_EVENT_READABLE,
					      wl_client_connection_data, client);

	if (!client->source)
		goto err_client;

	if (wl_os_socket_peercred(fd, &client->uid, &client->gid,
				  &client->pid) != 0)
		goto err_source;

	client->connection = wl_connection_create(fd);
	if (client->connection == NULL)
		goto err_source;

	wl_map_init(&client->objects, WL_MAP_SERVER_SIDE);

	if (wl_map_insert_at(&client->objects, 0, 0, NULL) < 0)
		goto err_map;

	wl_priv_signal_init(&client->destroy_signal);
	wl_priv_signal_init(&client->destroy_late_signal);
	if (bind_display(client, display) < 0)
		goto err_map;

	wl_list_insert(display->client_list.prev, &client->link);

	wl_priv_signal_emit(&display->create_client_signal, client);

	return client;

err_map:
	wl_map_release(&client->objects);
	wl_connection_destroy(client->connection);
err_source:
	wl_event_source_remove(client->source);
err_client:
	free(client);
	return NULL;
}

/** Return Unix credentials for the client
 *
 * \param client The display object
 * \param pid Returns the process ID
 * \param uid Returns the user ID
 * \param gid Returns the group ID
 *
 * This function returns the process ID, the user ID and the group ID
 * for the given client.  The credentials come from getsockopt() with
 * SO_PEERCRED, on the client socket fd.  All the pointers can be
 * NULL, if the caller is not interested in a particular ID.
 *
 * Note, process IDs are subject to race conditions and are not a reliable way
 * to identify a client.
 *
 * Be aware that for clients that a compositor forks and execs and
 * then connects using socketpair(), this function will return the
 * credentials for the compositor.  The credentials for the socketpair
 * are set at creation time in the compositor.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_get_credentials(struct wl_client *client,
			  pid_t *pid, uid_t *uid, gid_t *gid)
{
	if (pid)
		*pid = client->pid;
	if (uid)
		*uid = client->uid;
	if (gid)
		*gid = client->gid;
}

/** Get the file descriptor for the client
 *
 * \param client The display object
 * \return The file descriptor to use for the connection
 *
 * This function returns the file descriptor for the given client.
 *
 * Be sure to use the file descriptor from the client for inspection only.
 * If the caller does anything to the file descriptor that changes its state,
 * it will likely cause problems.
 *
 * See also wl_client_get_credentials().
 * It is recommended that you evaluate whether wl_client_get_credentials()
 * can be applied to your use case instead of this function.
 *
 * If you would like to distinguish just between the client and the compositor
 * itself from the client's request, it can be done by getting the client
 * credentials and by checking the PID of the client and the compositor's PID.
 * Regarding the case in which the socketpair() is being used, you need to be
 * careful. Please note the documentation for wl_client_get_credentials().
 *
 * This function can be used for a compositor to validate a request from
 * a client if there are additional information provided from the client's
 * file descriptor. For instance, suppose you can get the security contexts
 * from the client's file descriptor. The compositor can validate the client's
 * request with the contexts and make a decision whether it permits or deny it.
 *
 * \memberof wl_client
 */
WL_EXPORT int
wl_client_get_fd(struct wl_client *client)
{
	return wl_connection_get_fd(client->connection);
}

/** Look up an object in the client name space
 *
 * \param client The client object
 * \param id The object id
 * \return The object or NULL if there is not object for the given ID
 *
 * This looks up an object in the client object name space by its
 * object ID.
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_resource *
wl_client_get_object(struct wl_client *client, uint32_t id)
{
	return wl_map_lookup(&client->objects, id);
}

WL_EXPORT void
wl_client_post_no_memory(struct wl_client *client)
{
	wl_resource_post_error(client->display_resource,
			       WL_DISPLAY_ERROR_NO_MEMORY, "no memory");
}

/** Report an internal server error
 *
 * \param client The client object
 * \param msg A printf-style format string
 * \param ... Format string arguments
 *
 * Report an unspecified internal implementation error and disconnect
 * the client.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_post_implementation_error(struct wl_client *client,
				    char const *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	wl_resource_post_error_vargs(client->display_resource,
				     WL_DISPLAY_ERROR_IMPLEMENTATION,
				     msg, ap);
	va_end(ap);
}

WL_EXPORT void
wl_resource_post_no_memory(struct wl_resource *resource)
{
	wl_resource_post_error(resource->client->display_resource,
			       WL_DISPLAY_ERROR_NO_MEMORY, "no memory");
}

/** Detect if a wl_resource uses the deprecated public definition.
 *
 * Before Wayland 1.2.0, the definition of struct wl_resource was public.
 * It was made opaque just before 1.2.0, and later new fields were added.
 * The new fields cannot be accessed if a program is using the deprecated
 * definition, as there would not be memory allocated for them.
 *
 * The creation pattern for the deprecated definition was wl_resource_init()
 * followed by wl_client_add_resource(). wl_resource_init() was an inline
 * function and no longer exists, but binaries might still carry it.
 * wl_client_add_resource() still exists for ABI compatibility.
 */
static bool
resource_is_deprecated(struct wl_resource *resource)
{
	struct wl_map *map = &resource->client->objects;
	int id = resource->object.id;

	/* wl_client_add_resource() marks deprecated resources with the flag. */
	if (wl_map_lookup_flags(map, id) & WL_MAP_ENTRY_LEGACY)
		return true;

	return false;
}

static enum wl_iterator_result
destroy_resource(void *element, void *data, uint32_t flags)
{
	struct wl_resource *resource = element;

	wl_signal_emit(&resource->deprecated_destroy_signal, resource);
	/* Don't emit the new signal for deprecated resources, as that would
	 * access memory outside the bounds of the deprecated struct */
	if (!resource_is_deprecated(resource))
		wl_priv_signal_final_emit(&resource->destroy_signal, resource);

	if (resource->destroy)
		resource->destroy(resource);

	if (!(flags & WL_MAP_ENTRY_LEGACY))
		free(resource);

	return WL_ITERATOR_CONTINUE;
}

WL_EXPORT void
wl_resource_destroy(struct wl_resource *resource)
{
	struct wl_client *client = resource->client;
	uint32_t id;
	uint32_t flags;

	id = resource->object.id;
	flags = wl_map_lookup_flags(&client->objects, id);
	destroy_resource(resource, NULL, flags);

	if (id < WL_SERVER_ID_START) {
		if (client->display_resource) {
			wl_resource_queue_event(client->display_resource,
						WL_DISPLAY_DELETE_ID, id);
		}
		wl_map_insert_at(&client->objects, 0, id, NULL);
	} else {
		wl_map_remove(&client->objects, id);
	}
}

WL_EXPORT uint32_t
wl_resource_get_id(struct wl_resource *resource)
{
	return resource->object.id;
}

WL_EXPORT struct wl_list *
wl_resource_get_link(struct wl_resource *resource)
{
	return &resource->link;
}

WL_EXPORT struct wl_resource *
wl_resource_from_link(struct wl_list *link)
{
	struct wl_resource *resource;

	return wl_container_of(link, resource, link);
}

WL_EXPORT struct wl_resource *
wl_resource_find_for_client(struct wl_list *list, struct wl_client *client)
{
	struct wl_resource *resource;

	if (client == NULL)
		return NULL;

	wl_list_for_each(resource, list, link) {
		if (resource->client == client)
			return resource;
	}

	return NULL;
}

WL_EXPORT struct wl_client *
wl_resource_get_client(struct wl_resource *resource)
{
	return resource->client;
}

WL_EXPORT void
wl_resource_set_user_data(struct wl_resource *resource, void *data)
{
	resource->data = data;
}

WL_EXPORT void *
wl_resource_get_user_data(struct wl_resource *resource)
{
	return resource->data;
}

WL_EXPORT int
wl_resource_get_version(struct wl_resource *resource)
{
	return resource->version;
}

WL_EXPORT void
wl_resource_set_destructor(struct wl_resource *resource,
			   wl_resource_destroy_func_t destroy)
{
	resource->destroy = destroy;
}

WL_EXPORT int
wl_resource_instance_of(struct wl_resource *resource,
			const struct wl_interface *interface,
			const void *implementation)
{
	return wl_interface_equal(resource->object.interface, interface) &&
		resource->object.implementation == implementation;
}

WL_EXPORT void
wl_resource_add_destroy_listener(struct wl_resource *resource,
				 struct wl_listener * listener)
{
	if (resource_is_deprecated(resource))
		wl_signal_add(&resource->deprecated_destroy_signal, listener);
	else
		wl_priv_signal_add(&resource->destroy_signal, listener);
}

WL_EXPORT struct wl_listener *
wl_resource_get_destroy_listener(struct wl_resource *resource,
				 wl_notify_func_t notify)
{
	if (resource_is_deprecated(resource))
		return wl_signal_get(&resource->deprecated_destroy_signal, notify);
	return wl_priv_signal_get(&resource->destroy_signal, notify);
}

/** Retrieve the interface name (class) of a resource object.
 *
 * \param resource The resource object
 *
 * \memberof wl_resource
 */
WL_EXPORT const char *
wl_resource_get_class(struct wl_resource *resource)
{
	return resource->object.interface->name;
}

/**
 * Add a listener to be called at the beginning of wl_client destruction
 *
 * The listener provided will be called when wl_client destroy has begun,
 * before any of that client's resources have been destroyed.
 *
 * There is no requirement to remove the link of the wl_listener when the
 * signal is emitted.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_add_destroy_listener(struct wl_client *client,
			       struct wl_listener *listener)
{
	wl_priv_signal_add(&client->destroy_signal, listener);
}

WL_EXPORT struct wl_listener *
wl_client_get_destroy_listener(struct wl_client *client,
			       wl_notify_func_t notify)
{
	return wl_priv_signal_get(&client->destroy_signal, notify);
}

/**
 * Add a listener to be called at the end of wl_client destruction
 *
 * The listener provided will be called when wl_client destroy is nearly
 * complete, after all of that client's resources have been destroyed.
 *
 * There is no requirement to remove the link of the wl_listener when the
 * signal is emitted.
 *
 * \memberof wl_client
 * \since 1.22.0
 */
WL_EXPORT void
wl_client_add_destroy_late_listener(struct wl_client *client,
				    struct wl_listener *listener)
{
	wl_priv_signal_add(&client->destroy_late_signal, listener);
}

WL_EXPORT struct wl_listener *
wl_client_get_destroy_late_listener(struct wl_client *client,
				    wl_notify_func_t notify)
{
	return wl_priv_signal_get(&client->destroy_late_signal, notify);
}

WL_EXPORT void
wl_client_destroy(struct wl_client *client)
{
	uint32_t serial = 0;

	wl_priv_signal_final_emit(&client->destroy_signal, client);

	wl_client_flush(client);
	wl_map_for_each(&client->objects, destroy_resource, &serial);
	wl_map_release(&client->objects);
	wl_event_source_remove(client->source);
	close(wl_connection_destroy(client->connection));

	wl_priv_signal_final_emit(&client->destroy_late_signal, client);

	wl_list_remove(&client->link);
	wl_list_remove(&client->resource_created_signal.listener_list);
	free(client);
}

/* Check if a global filter is registered and use it if any.
 *
 * If no wl_global filter has been registered, this function will
 * return true, allowing the wl_global to be visible to the wl_client
 */
static bool
wl_global_is_visible(const struct wl_client *client,
	      const struct wl_global *global)
{
	struct wl_display *display = client->display;

	return (display->global_filter == NULL ||
		display->global_filter(client, global, display->global_filter_data));
}

static void
registry_bind(struct wl_client *client,
	      struct wl_resource *resource, uint32_t name,
	      const char *interface, uint32_t version, uint32_t id)
{
	struct wl_global *global;
	struct wl_display *display = resource->data;

	wl_list_for_each(global, &display->global_list, link)
		if (global->name == name)
			break;

	if (&global->link == &display->global_list)
		wl_resource_post_error(resource,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "invalid global %s (%d)", interface, name);
	else if (strcmp(global->interface->name, interface) != 0)
		wl_resource_post_error(resource,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "invalid interface for global %u: "
				       "have %s, wanted %s",
				       name, interface, global->interface->name);
	else if (version == 0)
		wl_resource_post_error(resource,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "invalid version for global %s (%d): 0 is not a valid version",
				       interface, name);
	else if (global->version < version)
		wl_resource_post_error(resource,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "invalid version for global %s (%d): have %d, wanted %d",
				       interface, name, global->version, version);
	else if (!wl_global_is_visible(client, global))
		wl_resource_post_error(resource,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "invalid global %s (%d)", interface, name);
	else
		global->bind(client, global->data, version, id);
}

static const struct wl_registry_interface registry_interface = {
	registry_bind
};

static void
display_sync(struct wl_client *client,
	     struct wl_resource *resource, uint32_t id)
{
	struct wl_resource *callback;
	uint32_t serial;

	callback = wl_resource_create(client, &wl_callback_interface, 1, id);
	if (callback == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	serial = wl_display_get_serial(client->display);
	wl_callback_send_done(callback, serial);
	wl_resource_destroy(callback);
}

static void
unbind_resource(struct wl_resource *resource)
{
	wl_list_remove(&resource->link);
}

static void
display_get_registry(struct wl_client *client,
		     struct wl_resource *resource, uint32_t id)
{
	struct wl_display *display = resource->data;
	struct wl_resource *registry_resource;
	struct wl_global *global;

	registry_resource =
		wl_resource_create(client, &wl_registry_interface, 1, id);
	if (registry_resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(registry_resource,
				       &registry_interface,
				       display, unbind_resource);

	wl_list_insert(&display->registry_resource_list,
		       &registry_resource->link);

	wl_list_for_each(global, &display->global_list, link)
		if (wl_global_is_visible(client, global) && !global->removed)
			wl_resource_post_event(registry_resource,
					       WL_REGISTRY_GLOBAL,
					       global->name,
					       global->interface->name,
					       global->version);
}

static const struct wl_display_interface display_interface = {
	display_sync,
	display_get_registry
};

static void
destroy_client_display_resource(struct wl_resource *resource)
{
	resource->client->display_resource = NULL;
}

static int
bind_display(struct wl_client *client, struct wl_display *display)
{
	client->display_resource =
		wl_resource_create(client, &wl_display_interface, 1, 1);
	if (client->display_resource == NULL) {
		/* DON'T send no-memory error to client - it has no
		 * resource to which it could post the event */
		return -1;
	}

	wl_resource_set_implementation(client->display_resource,
				       &display_interface, display,
				       destroy_client_display_resource);
	return 0;
}

static int
handle_display_terminate(int fd, uint32_t mask, void *data) {
	uint64_t term_event;

	if (read(fd, &term_event, sizeof(term_event)) < 0 && errno != EAGAIN)
		return -1;

	return 0;
}

/** Create Wayland display object.
 *
 * \return The Wayland display object. Null if failed to create
 *
 * This creates the wl_display object.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_display *
wl_display_create(void)
{
	struct wl_display *display;
	const char *debug;

	debug = getenv("WAYLAND_DEBUG");
	if (debug && (strstr(debug, "server") || strstr(debug, "1")))
		debug_server = 1;

	display = zalloc(sizeof *display);
	if (display == NULL)
		return NULL;

	display->loop = wl_event_loop_create();
	if (display->loop == NULL) {
		free(display);
		return NULL;
	}

	display->terminate_efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (display->terminate_efd < 0)
		goto err_eventfd;

	display->term_source = wl_event_loop_add_fd(display->loop,
						    display->terminate_efd,
						    WL_EVENT_READABLE,
						    handle_display_terminate,
						    NULL);

	if (display->term_source == NULL)
		goto err_term_source;

	wl_list_init(&display->global_list);
	wl_list_init(&display->socket_list);
	wl_list_init(&display->client_list);
	wl_list_init(&display->registry_resource_list);
	wl_list_init(&display->protocol_loggers);

	wl_priv_signal_init(&display->destroy_signal);
	wl_priv_signal_init(&display->create_client_signal);

	display->next_global_name = 1;
	display->serial = 0;

	display->global_filter = NULL;
	display->global_filter_data = NULL;

	wl_array_init(&display->additional_shm_formats);

	return display;

err_term_source:
	close(display->terminate_efd);
err_eventfd:
	wl_event_loop_destroy(display->loop);
	free(display);
	return NULL;
}

static void
wl_socket_destroy(struct wl_socket *s)
{
	if (s->source)
		wl_event_source_remove(s->source);
	if (s->addr.sun_path[0])
		unlink(s->addr.sun_path);
	if (s->fd >= 0)
		close(s->fd);
	if (s->lock_addr[0])
		unlink(s->lock_addr);
	if (s->fd_lock >= 0)
		close(s->fd_lock);

	free(s);
}

static struct wl_socket *
wl_socket_alloc(void)
{
	struct wl_socket *s;

	s = zalloc(sizeof *s);
	if (!s)
		return NULL;

	s->fd = -1;
	s->fd_lock = -1;

	return s;
}

/** Destroy Wayland display object.
 *
 * \param display The Wayland display object which should be destroyed.
 *
 * This function emits the wl_display destroy signal, releases
 * all the sockets added to this display, free's all the globals associated
 * with this display, free's memory of additional shared memory formats and
 * destroy the display object.
 *
 * \sa wl_display_add_destroy_listener
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_destroy(struct wl_display *display)
{
	struct wl_socket *s, *next;
	struct wl_global *global, *gnext;

	wl_priv_signal_final_emit(&display->destroy_signal, display);

	wl_list_for_each_safe(s, next, &display->socket_list, link) {
		wl_socket_destroy(s);
	}

	close(display->terminate_efd);
	wl_event_source_remove(display->term_source);

	wl_event_loop_destroy(display->loop);

	wl_list_for_each_safe(global, gnext, &display->global_list, link)
		free(global);

	wl_array_release(&display->additional_shm_formats);

	wl_list_remove(&display->protocol_loggers);

	free(display);
}

/** Set a filter function for global objects
 *
 * \param display The Wayland display object.
 * \param filter  The global filter function.
 * \param data User data to be associated with the global filter.
 *
 * Set a filter for the wl_display to advertise or hide global objects
 * to clients.
 * The set filter will be used during wl_global advertisement to
 * determine whether a global object should be advertised to a
 * given client, and during wl_global binding to determine whether
 * a given client should be allowed to bind to a global.
 *
 * Clients that try to bind to a global that was filtered out will
 * have an error raised.
 *
 * Setting the filter NULL will result in all globals being
 * advertised to all clients. The default is no filter.
 *
 * The filter should be installed before any client connects and should always
 * take the same decision given a client and a global. Not doing so will result
 * in inconsistent filtering and broken wl_registry event sequences.
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_set_global_filter(struct wl_display *display,
			     wl_display_global_filter_func_t filter,
			     void *data)
{
	display->global_filter = filter;
	display->global_filter_data = data;
}

WL_EXPORT struct wl_global *
wl_global_create(struct wl_display *display,
		 const struct wl_interface *interface, int version,
		 void *data, wl_global_bind_func_t bind)
{
	struct wl_global *global;
	struct wl_resource *resource;

	if (version < 1) {
		wl_log("wl_global_create: failing to create interface "
		       "'%s' with version %d because it is less than 1\n",
			interface->name, version);
		return NULL;
	}

	if (version > interface->version) {
		wl_log("wl_global_create: implemented version for '%s' "
		       "higher than interface version (%d > %d)\n",
		       interface->name, version, interface->version);
		return NULL;
	}

	if (display->next_global_name >= UINT32_MAX) {
		wl_log("wl_global_create: ran out of global names\n");
		return NULL;
	}

	global = zalloc(sizeof *global);
	if (global == NULL)
		return NULL;

	global->display = display;
	global->name = display->next_global_name++;
	global->interface = interface;
	global->version = version;
	global->data = data;
	global->bind = bind;
	global->removed = false;
	wl_list_insert(display->global_list.prev, &global->link);

	wl_list_for_each(resource, &display->registry_resource_list, link)
		if (wl_global_is_visible(resource->client, global))
			wl_resource_post_event(resource,
					       WL_REGISTRY_GLOBAL,
					       global->name,
					       global->interface->name,
					       global->version);

	return global;
}

/** Remove the global
 *
 * \param global The Wayland global.
 *
 * Broadcast a global remove event to all clients without destroying the
 * global. This function can only be called once per global.
 *
 * wl_global_destroy() removes the global and immediately destroys it. On
 * the other end, this function only removes the global, allowing clients
 * that have not yet received the global remove event to continue to bind to
 * it.
 *
 * This can be used by compositors to mitigate clients being disconnected
 * because a global has been added and removed too quickly. Compositors can call
 * wl_global_remove(), then wait an implementation-defined amount of time, then
 * call wl_global_destroy(). Note that the destruction of a global is still
 * racy, since clients have no way to acknowledge that they received the remove
 * event.
 *
 * \since 1.17.90
 */
WL_EXPORT void
wl_global_remove(struct wl_global *global)
{
	struct wl_display *display = global->display;
	struct wl_resource *resource;

	if (global->removed)
		wl_abort("wl_global_remove: called twice on the same "
			 "global '%s@%"PRIu32"'", global->interface->name,
			 global->name);

	wl_list_for_each(resource, &display->registry_resource_list, link)
		if (wl_global_is_visible(resource->client, global))
			wl_resource_post_event(resource, WL_REGISTRY_GLOBAL_REMOVE,
					       global->name);

	global->removed = true;
}

WL_EXPORT void
wl_global_destroy(struct wl_global *global)
{
	if (!global->removed)
		wl_global_remove(global);
	wl_list_remove(&global->link);
	free(global);
}

WL_EXPORT const struct wl_interface *
wl_global_get_interface(const struct wl_global *global)
{
	return global->interface;
}

/** Get the name of the global.
 *
 * \param global The global object.
 * \param client Client for which to look up the global.
 * \return The name of the global, or 0 if the global is not visible to the
 *         client.
 *
 * \memberof wl_global
 * \since 1.22
 */
WL_EXPORT uint32_t
wl_global_get_name(const struct wl_global *global,
		   const struct wl_client *client)
{
	return wl_global_is_visible(client, global) ? global->name : 0;
}

/** Get the version of the given global.
 *
 * \param global The global object.
 * \return The version advertised by the global.
 *
 * \memberof wl_global
 * \since 1.21
 */
WL_EXPORT uint32_t
wl_global_get_version(const struct wl_global *global)
{
	return global->version;
}

/** Get the display object for the given global
 *
 * \param global The global object
 * \return The display object the global is associated with.
 *
 * \memberof wl_global
 * \since 1.20
 */
WL_EXPORT struct wl_display *
wl_global_get_display(const struct wl_global *global)
{
	return global->display;
}

WL_EXPORT void *
wl_global_get_user_data(const struct wl_global *global)
{
	return global->data;
}

/** Set the global's user data
 *
 * \param global The global object
 * \param data The user data pointer
 *
 * \since 1.17.90
 */
WL_EXPORT void
wl_global_set_user_data(struct wl_global *global, void *data)
{
	global->data = data;
}

/** Get the current serial number
 *
 * \param display The display object
 *
 * This function returns the most recent serial number, but does not
 * increment it.
 *
 * \memberof wl_display
 */
WL_EXPORT uint32_t
wl_display_get_serial(struct wl_display *display)
{
	return display->serial;
}

/** Get the next serial number
 *
 * \param display The display object
 *
 * This function increments the display serial number and returns the
 * new value.
 *
 * \memberof wl_display
 */
WL_EXPORT uint32_t
wl_display_next_serial(struct wl_display *display)
{
	display->serial++;

	return display->serial;
}

WL_EXPORT struct wl_event_loop *
wl_display_get_event_loop(struct wl_display *display)
{
	return display->loop;
}

WL_EXPORT void
wl_display_terminate(struct wl_display *display)
{
	int ret;
	uint64_t terminate = 1;

	display->run = 0;

	ret = write(display->terminate_efd, &terminate, sizeof(terminate));
	assert (ret >= 0 || errno == EAGAIN);
}

WL_EXPORT void
wl_display_run(struct wl_display *display)
{
	display->run = 1;

	while (display->run) {
		wl_display_flush_clients(display);
		wl_event_loop_dispatch(display->loop, -1);
	}
}

WL_EXPORT void
wl_display_flush_clients(struct wl_display *display)
{
	struct wl_client *client, *next;
	int ret;

	wl_list_for_each_safe(client, next, &display->client_list, link) {
		ret = wl_connection_flush(client->connection);
		if (ret < 0 && errno == EAGAIN) {
			wl_event_source_fd_update(client->source,
						  WL_EVENT_WRITABLE |
						  WL_EVENT_READABLE);
		} else if (ret < 0) {
			wl_client_destroy(client);
		}
	}
}

/** Destroy all clients connected to the display
 *
 * \param display The display object
 *
 * This function should be called right before wl_display_destroy() to ensure
 * all client resources are closed properly. Destroying a client from within
 * wl_display_destroy_clients() is safe, but creating one will leak resources
 * and raise a warning.
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_destroy_clients(struct wl_display *display)
{
	struct wl_list tmp_client_list, *pos;
	struct wl_client *client;

	/* Move the whole client list to a temporary head because some new clients
	 * might be added to the original head. */
	wl_list_init(&tmp_client_list);
	wl_list_insert_list(&tmp_client_list, &display->client_list);
	wl_list_init(&display->client_list);

	/* wl_list_for_each_safe isn't enough here: it fails if the next client is
	 * destroyed by the destroy handler of the current one. */
	while (!wl_list_empty(&tmp_client_list)) {
		pos = tmp_client_list.next;
		client = wl_container_of(pos, client, link);

		wl_client_destroy(client);
	}

	if (!wl_list_empty(&display->client_list)) {
		wl_log("wl_display_destroy_clients: cannot destroy all clients because "
			   "new ones were created by destroy callbacks\n");
	}
}

static int
socket_data(int fd, uint32_t mask, void *data)
{
	struct wl_display *display = data;
	struct sockaddr_un name;
	socklen_t length;
	int client_fd;

	length = sizeof name;
	client_fd = wl_os_accept_cloexec(fd, (struct sockaddr *) &name,
					 &length);
	if (client_fd < 0)
		wl_log("failed to accept: %s\n", strerror(errno));
	else
		if (!wl_client_create(display, client_fd))
			close(client_fd);

	return 1;
}

static int
wl_socket_lock(struct wl_socket *socket)
{
	struct stat socket_stat;

	snprintf(socket->lock_addr, sizeof socket->lock_addr,
		 "%s%s", socket->addr.sun_path, LOCK_SUFFIX);

	socket->fd_lock = open(socket->lock_addr, O_CREAT | O_CLOEXEC | O_RDWR,
			       (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));

	if (socket->fd_lock < 0) {
		wl_log("unable to open lockfile %s check permissions\n",
			socket->lock_addr);
		goto err;
	}

	if (flock(socket->fd_lock, LOCK_EX | LOCK_NB) < 0) {
		wl_log("unable to lock lockfile %s, maybe another compositor is running\n",
			socket->lock_addr);
		goto err_fd;
	}

	if (lstat(socket->addr.sun_path, &socket_stat) < 0 ) {
		if (errno != ENOENT) {
			wl_log("did not manage to stat file %s\n",
				socket->addr.sun_path);
			goto err_fd;
		}
	} else if (socket_stat.st_mode & S_IWUSR ||
		   socket_stat.st_mode & S_IWGRP) {
		unlink(socket->addr.sun_path);
	}

	return 0;
err_fd:
	close(socket->fd_lock);
	socket->fd_lock = -1;
err:
	*socket->lock_addr = 0;
	/* we did not set this value here, but without lock the
	 * socket won't be created anyway. This prevents the
	 * wl_socket_destroy from unlinking already existing socket
	 * created by other compositor */
	*socket->addr.sun_path = 0;

	return -1;
}

static int
wl_socket_init_for_display_name(struct wl_socket *s, const char *name)
{
	int name_size;
	const char *runtime_dir = "";
	const char *separator = "";

	if (name[0] != '/') {
		runtime_dir = getenv("XDG_RUNTIME_DIR");
		if (!runtime_dir || runtime_dir[0] != '/') {
			wl_log("error: XDG_RUNTIME_DIR is invalid or not set in"
			       " the environment\n");

			/* to prevent programs reporting
			 * "failed to add socket: Success" */
			errno = ENOENT;
			return -1;
		}
		separator = "/";
	}

	s->addr.sun_family = AF_LOCAL;
	name_size = snprintf(s->addr.sun_path, sizeof s->addr.sun_path,
			     "%s%s%s", runtime_dir, separator, name) + 1;

	assert(name_size > 0);
	if (name_size > (int)sizeof s->addr.sun_path) {
		wl_log("error: socket path \"%s%s%s\" plus null terminator"
		       " exceeds 108 bytes\n", runtime_dir, separator, name);
		*s->addr.sun_path = 0;
		/* to prevent programs reporting
		 * "failed to add socket: Success" */
		errno = ENAMETOOLONG;
		return -1;
	}

	s->display_name = (s->addr.sun_path + name_size - 1) - strlen(name);

	return 0;
}

static int
_wl_display_add_socket(struct wl_display *display, struct wl_socket *s)
{
	socklen_t size;

	s->fd = wl_os_socket_cloexec(PF_LOCAL, SOCK_STREAM, 0);
	if (s->fd < 0) {
		return -1;
	}

	size = offsetof (struct sockaddr_un, sun_path) + strlen(s->addr.sun_path);
	if (bind(s->fd, (struct sockaddr *) &s->addr, size) < 0) {
		wl_log("bind() failed with error: %s\n", strerror(errno));
		return -1;
	}

	if (listen(s->fd, 128) < 0) {
		wl_log("listen() failed with error: %s\n", strerror(errno));
		return -1;
	}

	s->source = wl_event_loop_add_fd(display->loop, s->fd,
					 WL_EVENT_READABLE,
					 socket_data, display);
	if (s->source == NULL) {
		return -1;
	}

	wl_list_insert(display->socket_list.prev, &s->link);
	return 0;
}

WL_EXPORT const char *
wl_display_add_socket_auto(struct wl_display *display)
{
	struct wl_socket *s;
	int displayno = 0;
	char display_name[20] = "";

	/* A reasonable number of maximum default sockets. If
	 * you need more than this, use the explicit add_socket API. */
	const int MAX_DISPLAYNO = 32;

	s = wl_socket_alloc();
	if (s == NULL)
		return NULL;

	do {
		snprintf(display_name, sizeof display_name, "wayland-%d", displayno);
		if (wl_socket_init_for_display_name(s, display_name) < 0) {
			wl_socket_destroy(s);
			return NULL;
		}

		if (wl_socket_lock(s) < 0)
			continue;

		if (_wl_display_add_socket(display, s) < 0) {
			wl_socket_destroy(s);
			return NULL;
		}

		return s->display_name;
	} while (displayno++ < MAX_DISPLAYNO);

	/* Ran out of display names. */
	wl_socket_destroy(s);
	errno = EINVAL;
	return NULL;
}

/**  Add a socket with an existing fd to Wayland display for the clients to connect.
 *
 * \param display Wayland display to which the socket should be added.
 * \param sock_fd The existing socket file descriptor to be used
 * \return 0 if success. -1 if failed.
 *
 * The existing socket fd must already be created, opened, and locked.
 * The fd must be properly set to CLOEXEC and bound to a socket file
 * with both bind() and listen() already called.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_add_socket_fd(struct wl_display *display, int sock_fd)
{
	struct wl_socket *s;
	struct stat buf;

	/* Require a valid fd or fail */
	if (sock_fd < 0 || fstat(sock_fd, &buf) < 0 || !S_ISSOCK(buf.st_mode)) {
		return -1;
	}

	s = wl_socket_alloc();
	if (s == NULL)
		return -1;

	s->source = wl_event_loop_add_fd(display->loop, sock_fd,
					 WL_EVENT_READABLE,
					 socket_data, display);
	if (s->source == NULL) {
		wl_log("failed to establish event source\n");
		wl_socket_destroy(s);
		return -1;
	}

	/* Reuse the existing fd */
	s->fd = sock_fd;

	wl_list_insert(display->socket_list.prev, &s->link);

	return 0;
}

/** Add a socket to Wayland display for the clients to connect.
 *
 * \param display Wayland display to which the socket should be added.
 * \param name Name of the Unix socket.
 * \return 0 if success. -1 if failed.
 *
 * This adds a Unix socket to Wayland display which can be used by clients to
 * connect to Wayland display.
 *
 * If NULL is passed as name, then it would look for WAYLAND_DISPLAY env
 * variable for the socket name. If WAYLAND_DISPLAY is not set, then default
 * wayland-0 is used.
 *
 * If the socket name is a relative path, the Unix socket will be created in
 * the directory pointed to by environment variable XDG_RUNTIME_DIR. If
 * XDG_RUNTIME_DIR is invalid or not set, then this function fails and returns -1.
 *
 * If the socket name is an absolute path, then it is used as-is for the
 * the Unix socket.
 *
 * The length of the computed socket path must not exceed the maximum length
 * of a Unix socket path.
 * The function also fails if the user does not have write permission in the
 * directory or if the path is already in use.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_add_socket(struct wl_display *display, const char *name)
{
	struct wl_socket *s;

	s = wl_socket_alloc();
	if (s == NULL)
		return -1;

	if (name == NULL)
		name = getenv("WAYLAND_DISPLAY");
	if (name == NULL)
		name = "wayland-0";

	if (wl_socket_init_for_display_name(s, name) < 0) {
		wl_socket_destroy(s);
		return -1;
	}

	if (wl_socket_lock(s) < 0) {
		wl_socket_destroy(s);
		return -1;
	}

	if (_wl_display_add_socket(display, s) < 0) {
		wl_socket_destroy(s);
		return -1;
	}

	return 0;
}

WL_EXPORT void
wl_display_add_destroy_listener(struct wl_display *display,
				struct wl_listener *listener)
{
	wl_priv_signal_add(&display->destroy_signal, listener);
}

/** Registers a listener for the client connection signal.
 *  When a new client object is created, \a listener will be notified, carrying
 *  a pointer to the new wl_client object.
 *
 *  \ref wl_client_create
 *  \ref wl_display
 *  \ref wl_listener
 *
 * \param display The display object
 * \param listener Signal handler object
 */
WL_EXPORT void
wl_display_add_client_created_listener(struct wl_display *display,
					struct wl_listener *listener)
{
	wl_priv_signal_add(&display->create_client_signal, listener);
}

WL_EXPORT struct wl_listener *
wl_display_get_destroy_listener(struct wl_display *display,
				wl_notify_func_t notify)
{
	return wl_priv_signal_get(&display->destroy_signal, notify);
}

WL_EXPORT void
wl_resource_set_implementation(struct wl_resource *resource,
			       const void *implementation,
			       void *data, wl_resource_destroy_func_t destroy)
{
	resource->object.implementation = implementation;
	resource->data = data;
	resource->destroy = destroy;
	resource->dispatcher = NULL;
}

WL_EXPORT void
wl_resource_set_dispatcher(struct wl_resource *resource,
			   wl_dispatcher_func_t dispatcher,
			   const void *implementation,
			   void *data, wl_resource_destroy_func_t destroy)
{
	resource->dispatcher = dispatcher;
	resource->object.implementation = implementation;
	resource->data = data;
	resource->destroy = destroy;
}

/** Create a new resource object
 *
 * \param client The client owner of the new resource.
 * \param interface The interface of the new resource.
 * \param version The version of the new resource.
 * \param id The id of the new resource. If 0, an available id will be used.
 *
 * Listeners added with \a wl_client_add_resource_created_listener will be
 * notified at the end of this function.
 *
 * \memberof wl_resource
 */
WL_EXPORT struct wl_resource *
wl_resource_create(struct wl_client *client,
		   const struct wl_interface *interface,
		   int version, uint32_t id)
{
	struct wl_resource *resource;

	resource = zalloc(sizeof *resource);
	if (resource == NULL)
		return NULL;

	if (id == 0) {
		id = wl_map_insert_new(&client->objects, 0, NULL);
		if (id == 0) {
			free(resource);
			return NULL;
		}
	}

	resource->object.id = id;
	resource->object.interface = interface;
	resource->object.implementation = NULL;

	wl_signal_init(&resource->deprecated_destroy_signal);
	wl_priv_signal_init(&resource->destroy_signal);

	resource->destroy = NULL;
	resource->client = client;
	resource->data = NULL;
	resource->version = version;
	resource->dispatcher = NULL;

	if (wl_map_insert_at(&client->objects, 0, id, resource) < 0) {
		if (errno == EINVAL) {
			wl_resource_post_error(client->display_resource,
					       WL_DISPLAY_ERROR_INVALID_OBJECT,
					       "invalid new id %d", id);
		}
		free(resource);
		return NULL;
	}

	wl_priv_signal_emit(&client->resource_created_signal, resource);
	return resource;
}

WL_EXPORT void
wl_log_set_handler_server(wl_log_func_t handler)
{
	wl_log_handler = handler;
}

/** Adds a new protocol logger.
 *
 * When a new protocol message arrives or is sent from the server
 * all the protocol logger functions will be called, carrying the
 * \a user_data pointer, the type of the message (request or
 * event) and the actual message.
 * The lifetime of the messages passed to the logger function ends
 * when they return so the messages cannot be stored and accessed
 * later.
 *
 * \a errno is set on error.
 *
 * \param display The display object
 * \param func The function to call to log a new protocol message
 * \param user_data The user data pointer to pass to \a func
 *
 * \return The protol logger object on success, NULL on failure.
 *
 * \sa wl_protocol_logger_destroy
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_protocol_logger *
wl_display_add_protocol_logger(struct wl_display *display,
			       wl_protocol_logger_func_t func, void *user_data)
{
	struct wl_protocol_logger *logger;

	logger = zalloc(sizeof *logger);
	if (!logger)
		return NULL;

	logger->func = func;
	logger->user_data = user_data;
	wl_list_insert(&display->protocol_loggers, &logger->link);

	return logger;
}

/** Destroys a protocol logger.
 *
 * This function destroys a protocol logger and removes it from the display
 * it was added to with \a wl_display_add_protocol_logger.
 * The \a logger object becomes invalid after calling this function.
 *
 * \sa wl_display_add_protocol_logger
 *
 * \memberof wl_protocol_logger
 */
WL_EXPORT void
wl_protocol_logger_destroy(struct wl_protocol_logger *logger)
{
	wl_list_remove(&logger->link);
	free(logger);
}

/** Add support for a wl_shm pixel format
 *
 * \param display The display object
 * \param format The wl_shm pixel format to advertise
 * \return A pointer to the wl_shm format that was added to the list
 * or NULL if adding it to the list failed.
 *
 * Add the specified wl_shm format to the list of formats the wl_shm
 * object advertises when a client binds to it.  Adding a format to
 * the list means that clients will know that the compositor supports
 * this format and may use it for creating wl_shm buffers.  The
 * compositor must be able to handle the pixel format when a client
 * requests it.
 *
 * The compositor by default supports WL_SHM_FORMAT_ARGB8888 and
 * WL_SHM_FORMAT_XRGB8888.
 *
 * \memberof wl_display
 */
WL_EXPORT uint32_t *
wl_display_add_shm_format(struct wl_display *display, uint32_t format)
{
	uint32_t *p = NULL;

	p = wl_array_add(&display->additional_shm_formats, sizeof *p);

	if (p != NULL)
		*p = format;
	return p;
}

/**
 * Get list of additional wl_shm pixel formats
 *
 * \param display The display object
 *
 * This function returns the list of addition wl_shm pixel formats
 * that the compositor supports.  WL_SHM_FORMAT_ARGB8888 and
 * WL_SHM_FORMAT_XRGB8888 are always supported and not included in the
 * array, but all formats added through wl_display_add_shm_format()
 * will be in the array.
 *
 * \sa wl_display_add_shm_format()
 *
 * \private
 *
 * \memberof wl_display
 */
struct wl_array *
wl_display_get_additional_shm_formats(struct wl_display *display)
{
	return &display->additional_shm_formats;
}

/** Get the list of currently connected clients
 *
 * \param display The display object
 *
 * This function returns a pointer to the list of clients currently
 * connected to the display. You can iterate on the list by using
 * the \a wl_client_for_each macro.
 * The returned value is valid for the lifetime of the \a display.
 * You must not modify the returned list, but only access it.
 *
 * \sa wl_client_for_each()
 * \sa wl_client_get_link()
 * \sa wl_client_from_link()
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_list *
wl_display_get_client_list(struct wl_display *display)
{
	return &display->client_list;
}

/** Get the link by which a client is inserted in the client list
 *
 * \param client The client object
 *
 * \sa wl_client_for_each()
 * \sa wl_display_get_client_list()
 * \sa wl_client_from_link()
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_list *
wl_client_get_link(struct wl_client *client)
{
	return &client->link;
}

/** Get a wl_client by its link
 *
 * \param link The link of a wl_client
 *
 * \sa wl_client_for_each()
 * \sa wl_display_get_client_list()
 * \sa wl_client_get_link()
 *
 * \memberof wl_client
 */
WL_EXPORT struct wl_client *
wl_client_from_link(struct wl_list *link)
{
	struct wl_client *client;

	return wl_container_of(link, client, link);
}

/** Add a listener for the client's resource creation signal
 *
 * \param client The client object
 * \param listener The listener to be added
 *
 * When a new resource is created for this client the listener
 * will be notified, carrying the new resource as the data argument.
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_add_resource_created_listener(struct wl_client *client,
					struct wl_listener *listener)
{
	wl_priv_signal_add(&client->resource_created_signal, listener);
}

struct wl_resource_iterator_context {
	void *user_data;
	wl_client_for_each_resource_iterator_func_t it;
};

static enum wl_iterator_result
resource_iterator_helper(void *res, void *user_data, uint32_t flags)
{
	struct wl_resource_iterator_context *context = user_data;
	struct wl_resource *resource = res;

	return context->it(resource, context->user_data);
}

/** Iterate over all the resources of a client
 *
 * \param client The client object
 * \param iterator The iterator function
 * \param user_data The user data pointer
 *
 * The function pointed by \a iterator will be called for each
 * resource owned by the client. The \a user_data will be passed
 * as the second argument of the iterator function.
 * If the \a iterator function returns \a WL_ITERATOR_CONTINUE the iteration
 * will continue, if it returns \a WL_ITERATOR_STOP it will stop.
 *
 * Creating and destroying resources while iterating is safe, but new
 * resources may or may not be picked up by the iterator.
 *
 * \sa wl_iterator_result
 *
 * \memberof wl_client
 */
WL_EXPORT void
wl_client_for_each_resource(struct wl_client *client,
			    wl_client_for_each_resource_iterator_func_t iterator,
			    void *user_data)
{
	struct wl_resource_iterator_context context = {
		.user_data = user_data,
		.it = iterator,
	};

	wl_map_for_each(&client->objects, resource_iterator_helper, &context);
}

static void
handle_noop(struct wl_listener *listener, void *data)
{
	/* Do nothing */
}

/** Emits this signal, notifying all registered listeners.
 *
 * A safer version of wl_signal_emit() which can gracefully handle additions
 * and deletions of any signal listener from within listener notification
 * callbacks.
 *
 * Listeners deleted during a signal emission and which have not already been
 * notified at the time of deletion are not notified by that emission.
 *
 * Listeners added (or readded) during signal emission are ignored by that
 * emission.
 *
 * Note that repurposing a listener without explicitly removing it and readding
 * it is not supported and can lead to unexpected behavior.
 *
 * \param signal The signal object that will emit the signal
 * \param data The data that will be emitted with the signal
 *
 * \memberof wl_signal
 * \since 1.20.90
 */
WL_EXPORT void
wl_signal_emit_mutable(struct wl_signal *signal, void *data)
{
	struct wl_listener cursor;
	struct wl_listener end;

	/* Add two special markers: one cursor and one end marker. This way, we
	 * know that we've already called listeners on the left of the cursor
	 * and that we don't want to call listeners on the right of the end
	 * marker. The 'it' function can remove any element it wants from the
	 * list without troubles.
	 *
	 * There was a previous attempt that used to steal the whole list of
	 * listeners but then that broke wl_signal_get().
	 *
	 * wl_list_for_each_safe tries to be safe but it fails: it works fine
	 * if the current item is removed, but not if the next one is. */
	wl_list_insert(&signal->listener_list, &cursor.link);
	cursor.notify = handle_noop;
	wl_list_insert(signal->listener_list.prev, &end.link);
	end.notify = handle_noop;

	while (cursor.link.next != &end.link) {
		struct wl_list *pos = cursor.link.next;
		struct wl_listener *l = wl_container_of(pos, l, link);

		wl_list_remove(&cursor.link);
		wl_list_insert(pos, &cursor.link);

		l->notify(l, data);
	}

	wl_list_remove(&cursor.link);
	wl_list_remove(&end.link);
}

/** \cond INTERNAL */

/** Initialize a wl_priv_signal object
 *
 * wl_priv_signal is a safer implementation of a signal type, with the same API
 * as wl_signal, but kept as a private utility of libwayland-server.
 * It is safer because listeners can be removed from within wl_priv_signal_emit()
 * without corrupting the signal's list.
 *
 * Before passing a wl_priv_signal object to any other function it must be
 * initialized by using wl_priv_signal_init().
 *
 * \memberof wl_priv_signal
 */
void
wl_priv_signal_init(struct wl_priv_signal *signal)
{
	wl_list_init(&signal->listener_list);
	wl_list_init(&signal->emit_list);
}

/** Add a listener to a signal
 *
 * The new listener will be called when calling wl_signal_emit(). If a listener is
 * added to the signal while wl_signal_emit() is running it will be called from
 * the next time wl_priv_signal_emit() is called.
 * To remove a listener call wl_list_remove() on its link member.
 *
 * \memberof wl_priv_signal
 */
void
wl_priv_signal_add(struct wl_priv_signal *signal, struct wl_listener *listener)
{
	wl_list_insert(signal->listener_list.prev, &listener->link);
}

/** Get a listener added to a signal
 *
 * Returns the listener added to the given \a signal and with the given
 * \a notify function, or NULL if there isn't any.
 * Calling this function from within wl_priv_signal_emit() is safe and will
 * return the correct value.
 *
 * \memberof wl_priv_signal
 */
struct wl_listener *
wl_priv_signal_get(struct wl_priv_signal *signal, wl_notify_func_t notify)
{
	struct wl_listener *l;

	wl_list_for_each(l, &signal->listener_list, link)
		if (l->notify == notify)
			return l;
	wl_list_for_each(l, &signal->emit_list, link)
		if (l->notify == notify)
			return l;

	return NULL;
}

/** Emit the signal, calling all the installed listeners
 *
 * Iterate over all the listeners added to this \a signal and call
 * their \a notify function pointer, passing on the given \a data.
 * Removing or adding a listener from within wl_priv_signal_emit()
 * is safe.
 */
void
wl_priv_signal_emit(struct wl_priv_signal *signal, void *data)
{
	struct wl_listener *l;
	struct wl_list *pos;

	wl_list_insert_list(&signal->emit_list, &signal->listener_list);
	wl_list_init(&signal->listener_list);

	/* Take every element out of the list and put them in a temporary list.
	 * This way, the 'it' func can remove any element it wants from the list
	 * without troubles, because we always get the first element, not the
	 * one after the current, which may be invalid.
	 * wl_list_for_each_safe tries to be safe but it fails: it works fine
	 * if the current item is removed, but not if the next one is. */
	while (!wl_list_empty(&signal->emit_list)) {
		pos = signal->emit_list.next;
		l = wl_container_of(pos, l, link);

		wl_list_remove(pos);
		wl_list_insert(&signal->listener_list, pos);

		l->notify(l, data);
	}
}

/** Emit the signal for the last time, calling all the installed listeners
 *
 * Iterate over all the listeners added to this \a signal and call
 * their \a notify function pointer, passing on the given \a data.
 * Removing or adding a listener from within wl_priv_signal_emit()
 * is safe, as is freeing the structure containing the listener.
 *
 * A large body of external code assumes it's ok to free a destruction
 * listener without removing that listener from the list.  Mixing code
 * that acts like this and code that doesn't will result in list
 * corruption.
 *
 * We resolve this by removing each item from the list and isolating it
 * in another list.  We discard it completely after firing the notifier.
 * This should allow interoperability between code that unlinks its
 * destruction listeners and code that just frees structures they're in.
 *
 */
void
wl_priv_signal_final_emit(struct wl_priv_signal *signal, void *data)
{
	struct wl_listener *l;
	struct wl_list *pos;

	/* During a destructor notifier isolate every list item before
	 * notifying.  This renders harmless the long standing misuse
	 * of freeing listeners without removing them, but allows
	 * callers that do choose to remove them to interoperate with
	 * ones that don't. */
	while (!wl_list_empty(&signal->listener_list)) {
		pos = signal->listener_list.next;
		l = wl_container_of(pos, l, link);

		wl_list_remove(pos);
		wl_list_init(pos);

		l->notify(l, data);
	}
}

/** \endcond INTERNAL */

/** \cond */ /* Deprecated functions below. */

uint32_t
wl_client_add_resource(struct wl_client *client,
		       struct wl_resource *resource) WL_DEPRECATED;

WL_EXPORT uint32_t
wl_client_add_resource(struct wl_client *client,
		       struct wl_resource *resource)
{
	if (resource->object.id == 0) {
		resource->object.id =
			wl_map_insert_new(&client->objects,
					  WL_MAP_ENTRY_LEGACY, resource);
		if (resource->object.id == 0)
			return 0;
	} else if (wl_map_insert_at(&client->objects, WL_MAP_ENTRY_LEGACY,
				  resource->object.id, resource) < 0) {
		if (errno == EINVAL) {
			wl_resource_post_error(client->display_resource,
					       WL_DISPLAY_ERROR_INVALID_OBJECT,
					       "invalid new id %d",
					       resource->object.id);
		}
		return 0;
	}

	resource->client = client;
	wl_signal_init(&resource->deprecated_destroy_signal);

	return resource->object.id;
}

struct wl_resource *
wl_client_add_object(struct wl_client *client,
		     const struct wl_interface *interface,
		     const void *implementation,
		     uint32_t id, void *data) WL_DEPRECATED;

WL_EXPORT struct wl_resource *
wl_client_add_object(struct wl_client *client,
		     const struct wl_interface *interface,
		     const void *implementation, uint32_t id, void *data)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, interface, -1, id);
	if (resource == NULL)
		wl_client_post_no_memory(client);
	else
		wl_resource_set_implementation(resource,
					       implementation, data, NULL);

	return resource;
}

struct wl_resource *
wl_client_new_object(struct wl_client *client,
		     const struct wl_interface *interface,
		     const void *implementation, void *data) WL_DEPRECATED;

WL_EXPORT struct wl_resource *
wl_client_new_object(struct wl_client *client,
		     const struct wl_interface *interface,
		     const void *implementation, void *data)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, interface, -1, 0);
	if (resource == NULL)
		wl_client_post_no_memory(client);
	else
		wl_resource_set_implementation(resource,
					       implementation, data, NULL);

	return resource;
}

struct wl_global *
wl_display_add_global(struct wl_display *display,
		      const struct wl_interface *interface,
		      void *data, wl_global_bind_func_t bind) WL_DEPRECATED;

WL_EXPORT struct wl_global *
wl_display_add_global(struct wl_display *display,
		      const struct wl_interface *interface,
		      void *data, wl_global_bind_func_t bind)
{
	return wl_global_create(display, interface, interface->version, data, bind);
}

void
wl_display_remove_global(struct wl_display *display,
			 struct wl_global *global) WL_DEPRECATED;

WL_EXPORT void
wl_display_remove_global(struct wl_display *display, struct wl_global *global)
{
	wl_global_destroy(global);
}

/** \endcond */

/* Functions at the end of this file are deprecated.  Instead of adding new
 * code here, add it before the comment above that states:
 * Deprecated functions below.
 */
