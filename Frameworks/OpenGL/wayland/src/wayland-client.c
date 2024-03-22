/*
 * Copyright © 2008-2012 Kristian Høgsberg
 * Copyright © 2010-2012 Intel Corporation
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

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>

#include "wayland-util.h"
#include "wayland-os.h"
#include "wayland-client.h"
#include "wayland-private.h"

/** \cond */

enum wl_proxy_flag {
	WL_PROXY_FLAG_ID_DELETED = (1 << 0),
	WL_PROXY_FLAG_DESTROYED = (1 << 1),
	WL_PROXY_FLAG_WRAPPER = (1 << 2),
};

struct wl_zombie {
	int event_count;
	int *fd_count;
};

struct wl_proxy {
	struct wl_object object;
	struct wl_display *display;
	struct wl_event_queue *queue;
	uint32_t flags;
	int refcount;
	void *user_data;
	wl_dispatcher_func_t dispatcher;
	uint32_t version;
	const char * const *tag;
	struct wl_list queue_link; /**< in struct wl_event_queue::proxy_list */
};

struct wl_event_queue {
	struct wl_list event_list;
	struct wl_list proxy_list; /**< struct wl_proxy::queue_link */
	struct wl_display *display;
};

struct wl_display {
	struct wl_proxy proxy;
	struct wl_connection *connection;

	/* errno of the last wl_display error */
	int last_error;

	/* When display gets an error event from some object, it stores
	 * information about it here, so that client can get this
	 * information afterwards */
	struct {
		/* Code of the error. It can be compared to
		 * the interface's errors enumeration. */
		uint32_t code;
		/* interface (protocol) in which the error occurred */
		const struct wl_interface *interface;
		/* id of the proxy that caused the error. There's no warranty
		 * that the proxy is still valid. It's up to client how it will
		 * use it */
		uint32_t id;
	} protocol_error;
	int fd;
	struct wl_map objects;
	struct wl_event_queue display_queue;
	struct wl_event_queue default_queue;
	pthread_mutex_t mutex;

	int reader_count;
	uint32_t read_serial;
	pthread_cond_t reader_cond;
};

/** \endcond */

static int debug_client = 0;

/**
 * This helper function wakes up all threads that are
 * waiting for display->reader_cond (i. e. when reading is done,
 * canceled, or an error occurred)
 *
 * NOTE: must be called with display->mutex locked
 */
static void
display_wakeup_threads(struct wl_display *display)
{
	/* Thread can get sleeping only in read_events(). If we're
	 * waking it up, it means that the read completed or was
	 * canceled, so we must increase the read_serial.
	 * This prevents from indefinite sleeping in read_events().
	 */
	++display->read_serial;

	pthread_cond_broadcast(&display->reader_cond);
}

/**
 * This function is called for local errors (no memory, server hung up)
 *
 * \param display
 * \param error    error value (EINVAL, EFAULT, ...)
 *
 * \note this function is called with display mutex locked
 */
static void
display_fatal_error(struct wl_display *display, int error)
{
	if (display->last_error)
		return;

	if (!error)
		error = EFAULT;

	display->last_error = error;

	display_wakeup_threads(display);
}

/**
 * This function is called for error events
 * and indicates that in some object an error occurred.
 * The difference between this function and display_fatal_error()
 * is that this one handles errors that will come by wire,
 * whereas display_fatal_error() is called for local errors.
 *
 * \param display
 * \param code    error code
 * \param id      id of the object that generated the error
 * \param intf    protocol interface
 */
static void
display_protocol_error(struct wl_display *display, uint32_t code,
		       uint32_t id, const struct wl_interface *intf)
{
	int err;

	if (display->last_error)
		return;

	/* set correct errno */
	if (intf && wl_interface_equal(intf, &wl_display_interface)) {
		switch (code) {
		case WL_DISPLAY_ERROR_INVALID_OBJECT:
		case WL_DISPLAY_ERROR_INVALID_METHOD:
			err = EINVAL;
			break;
		case WL_DISPLAY_ERROR_NO_MEMORY:
			err = ENOMEM;
			break;
		case WL_DISPLAY_ERROR_IMPLEMENTATION:
			err = EPROTO;
			break;
		default:
			err = EFAULT;
		}
	} else {
		err = EPROTO;
	}

	pthread_mutex_lock(&display->mutex);

	display->last_error = err;

	display->protocol_error.code = code;
	display->protocol_error.id = id;
	display->protocol_error.interface = intf;

	/*
	 * here it is not necessary to wake up threads like in
	 * display_fatal_error, because this function is called from
	 * an event handler and that means that read_events() is done
	 * and woke up all threads. Since wl_display_prepare_read()
	 * fails when there are events in the queue, no threads
	 * can sleep in read_events() during dispatching
	 * (and therefore during calling this function), so this is safe.
	 */

	pthread_mutex_unlock(&display->mutex);
}

static void
wl_event_queue_init(struct wl_event_queue *queue, struct wl_display *display)
{
	wl_list_init(&queue->event_list);
	wl_list_init(&queue->proxy_list);
	queue->display = display;
}

static void
wl_proxy_unref(struct wl_proxy *proxy)
{
	assert(proxy->refcount > 0);
	if (--proxy->refcount > 0)
		return;

	/* If we get here, the client must have explicitly requested
	 * deletion. */
	assert(proxy->flags & WL_PROXY_FLAG_DESTROYED);
	free(proxy);
}

static void
validate_closure_objects(struct wl_closure *closure)
{
	const char *signature;
	struct argument_details arg;
	int i, count;
	struct wl_proxy *proxy;

	signature = closure->message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		switch (arg.type) {
		case 'n':
		case 'o':
			proxy = (struct wl_proxy *) closure->args[i].o;
			if (proxy && proxy->flags & WL_PROXY_FLAG_DESTROYED)
				closure->args[i].o = NULL;
			break;
		default:
			break;
		}
	}
}

/* Destroys a closure which was demarshaled for dispatch; unrefs all the
 * proxies in its arguments, as well as its own proxy, and destroys the
 * closure itself. */
static void
destroy_queued_closure(struct wl_closure *closure)
{
	const char *signature;
	struct argument_details arg;
	struct wl_proxy *proxy;
	int i, count;

	signature = closure->message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		switch (arg.type) {
		case 'n':
		case 'o':
			proxy = (struct wl_proxy *) closure->args[i].o;
			if (proxy)
				wl_proxy_unref(proxy);
			break;
		default:
			break;
		}
	}

	wl_proxy_unref(closure->proxy);
	wl_closure_destroy(closure);
}

static void
wl_event_queue_release(struct wl_event_queue *queue)
{
	struct wl_closure *closure;

	if (!wl_list_empty(&queue->proxy_list)) {
		struct wl_proxy *proxy, *tmp;

		if (queue != &queue->display->default_queue) {
			wl_log("warning: queue %p destroyed while proxies "
			       "still attached:\n", queue);
		}

		wl_list_for_each_safe(proxy, tmp, &queue->proxy_list,
				      queue_link) {
			if (queue != &queue->display->default_queue) {
				wl_log("  %s@%u still attached\n",
				       proxy->object.interface->name,
				       proxy->object.id);
			}
			proxy->queue = NULL;
			wl_list_remove(&proxy->queue_link);
			wl_list_init(&proxy->queue_link);
		}
	}

	while (!wl_list_empty(&queue->event_list)) {
		closure = wl_container_of(queue->event_list.next,
					  closure, link);
		wl_list_remove(&closure->link);
		destroy_queued_closure(closure);
	}
}

/** Destroy an event queue
 *
 * \param queue The event queue to be destroyed
 *
 * Destroy the given event queue. Any pending event on that queue is
 * discarded.
 *
 * The \ref wl_display object used to create the queue should not be
 * destroyed until all event queues created with it are destroyed with
 * this function.
 *
 * \memberof wl_event_queue
 */
WL_EXPORT void
wl_event_queue_destroy(struct wl_event_queue *queue)
{
	struct wl_display *display = queue->display;

	pthread_mutex_lock(&display->mutex);
	wl_event_queue_release(queue);
	free(queue);
	pthread_mutex_unlock(&display->mutex);
}

/** Create a new event queue for this display
 *
 * \param display The display context object
 * \return A new event queue associated with this display or NULL on
 * failure.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_event_queue *
wl_display_create_queue(struct wl_display *display)
{
	struct wl_event_queue *queue;

	queue = zalloc(sizeof *queue);
	if (queue == NULL)
		return NULL;

	wl_event_queue_init(queue, display);

	return queue;
}

static int
message_count_fds(const char *signature)
{
	unsigned int count, i, fds = 0;
	struct argument_details arg;

	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		if (arg.type == 'h')
			fds++;
	}

	return fds;
}

static struct wl_zombie *
prepare_zombie(struct wl_proxy *proxy)
{
	const struct wl_interface *interface = proxy->object.interface;
	const struct wl_message *message;
	int i, count;
	struct wl_zombie *zombie = NULL;

	/* If we hit an event with an FD, ensure we have a zombie object and
	 * fill the fd_count slot for that event with the number of FDs for
	 * that event. Interfaces with no events containing FDs will not have
	 * zombie objects created. */
	for (i = 0; i < interface->event_count; i++) {
		message = &interface->events[i];
		count = message_count_fds(message->signature);

		if (!count)
			continue;

		if (!zombie) {
			zombie = zalloc(sizeof(*zombie) +
				        (interface->event_count * sizeof(int)));
			if (!zombie)
				return NULL;

			zombie->event_count = interface->event_count;
			zombie->fd_count = (int *) &zombie[1];
		}

		zombie->fd_count[i] = count;
	}

	return zombie;
}

static enum wl_iterator_result
free_zombies(void *element, void *data, uint32_t flags)
{
	if (flags & WL_MAP_ENTRY_ZOMBIE)
		free(element);

	return WL_ITERATOR_CONTINUE;
}

static struct wl_proxy *
proxy_create(struct wl_proxy *factory, const struct wl_interface *interface,
	     uint32_t version)
{
	struct wl_proxy *proxy;
	struct wl_display *display = factory->display;

	proxy = zalloc(sizeof *proxy);
	if (proxy == NULL)
		return NULL;

	proxy->object.interface = interface;
	proxy->display = display;
	proxy->queue = factory->queue;
	proxy->refcount = 1;
	proxy->version = version;

	proxy->object.id = wl_map_insert_new(&display->objects, 0, proxy);
	if (proxy->object.id == 0) {
		free(proxy);
		return NULL;
	}

	wl_list_insert(&proxy->queue->proxy_list, &proxy->queue_link);

	return proxy;
}

/** Create a proxy object with a given interface
 *
 * \param factory Factory proxy object
 * \param interface Interface the proxy object should use
 * \return A newly allocated proxy object or NULL on failure
 *
 * This function creates a new proxy object with the supplied interface. The
 * proxy object will have an id assigned from the client id space. The id
 * should be created on the compositor side by sending an appropriate request
 * with \ref wl_proxy_marshal().
 *
 * The proxy will inherit the display and event queue of the factory object.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \sa wl_display, wl_event_queue, wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_create(struct wl_proxy *factory, const struct wl_interface *interface)
{
	struct wl_display *display = factory->display;
	struct wl_proxy *proxy;

	pthread_mutex_lock(&display->mutex);
	proxy = proxy_create(factory, interface, factory->version);
	pthread_mutex_unlock(&display->mutex);

	return proxy;
}

/* The caller should hold the display lock */
static struct wl_proxy *
wl_proxy_create_for_id(struct wl_proxy *factory,
		       uint32_t id, const struct wl_interface *interface)
{
	struct wl_proxy *proxy;
	struct wl_display *display = factory->display;

	proxy = zalloc(sizeof *proxy);
	if (proxy == NULL)
		return NULL;

	proxy->object.interface = interface;
	proxy->object.id = id;
	proxy->display = display;
	proxy->queue = factory->queue;
	proxy->refcount = 1;
	proxy->version = factory->version;

	if (wl_map_insert_at(&display->objects, 0, id, proxy) == -1) {
		free(proxy);
		return NULL;
	}

	wl_list_insert(&proxy->queue->proxy_list, &proxy->queue_link);

	return proxy;
}

static void
proxy_destroy(struct wl_proxy *proxy)
{
	if (proxy->flags & WL_PROXY_FLAG_ID_DELETED) {
		wl_map_remove(&proxy->display->objects, proxy->object.id);
	} else if (proxy->object.id < WL_SERVER_ID_START) {
		struct wl_zombie *zombie = prepare_zombie(proxy);

		/* The map now contains the zombie entry, until the delete_id
		 * event arrives. */
		wl_map_insert_at(&proxy->display->objects,
				 WL_MAP_ENTRY_ZOMBIE,
				 proxy->object.id,
				 zombie);
	} else {
		wl_map_insert_at(&proxy->display->objects, 0,
				 proxy->object.id, NULL);
	}

	proxy->flags |= WL_PROXY_FLAG_DESTROYED;

	proxy->queue = NULL;
	wl_list_remove(&proxy->queue_link);
	wl_list_init(&proxy->queue_link);

	wl_proxy_unref(proxy);
}

static void
wl_proxy_destroy_caller_locks(struct wl_proxy *proxy)
{
	if (proxy->flags & WL_PROXY_FLAG_WRAPPER)
		wl_abort("Tried to destroy wrapper with wl_proxy_destroy()\n");

	proxy_destroy(proxy);
}

/** Destroy a proxy object
 *
 * \param proxy The proxy to be destroyed
 *
 * \c proxy must not be a proxy wrapper.
 *
 * \note This function will abort in response to egregious
 * errors, and will do so with the display lock held. This means
 * SIGABRT handlers must not perform any actions that would
 * attempt to take that lock, or a deadlock would occur.
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_destroy(struct wl_proxy *proxy)
{
	struct wl_display *display = proxy->display;

	pthread_mutex_lock(&display->mutex);

	wl_proxy_destroy_caller_locks(proxy);

	pthread_mutex_unlock(&display->mutex);
}

/** Set a proxy's listener
 *
 * \param proxy The proxy object
 * \param implementation The listener to be added to proxy
 * \param data User data to be associated with the proxy
 * \return 0 on success or -1 on failure
 *
 * Set proxy's listener to \c implementation and its user data to
 * \c data. If a listener has already been set, this function
 * fails and nothing is changed.
 *
 * \c implementation is a vector of function pointers. For an opcode
 * \c n, \c implementation[n] should point to the handler of \c n for
 * the given object.
 *
 * \c proxy must not be a proxy wrapper.
 *
 * \memberof wl_proxy
 */
WL_EXPORT int
wl_proxy_add_listener(struct wl_proxy *proxy,
		      void (**implementation)(void), void *data)
{
	if (proxy->flags & WL_PROXY_FLAG_WRAPPER)
		wl_abort("Proxy %p is a wrapper\n", proxy);

	if (proxy->object.implementation || proxy->dispatcher) {
		wl_log("proxy %p already has listener\n", proxy);
		return -1;
	}

	proxy->object.implementation = implementation;
	proxy->user_data = data;

	return 0;
}

/** Get a proxy's listener
 *
 * \param proxy The proxy object
 * \return The address of the proxy's listener or NULL if no listener is set
 *
 * Gets the address to the proxy's listener; which is the listener set with
 * \ref wl_proxy_add_listener.
 *
 * This function is useful in clients with multiple listeners on the same
 * interface to allow the identification of which code to execute.
 *
 * \memberof wl_proxy
 */
WL_EXPORT const void *
wl_proxy_get_listener(struct wl_proxy *proxy)
{
	return proxy->object.implementation;
}

/** Set a proxy's listener (with dispatcher)
 *
 * \param proxy The proxy object
 * \param dispatcher The dispatcher to be used for this proxy
 * \param implementation The dispatcher-specific listener implementation
 * \param data User data to be associated with the proxy
 * \return 0 on success or -1 on failure
 *
 * Set proxy's listener to use \c dispatcher_func as its dispatcher and \c
 * dispatcher_data as its dispatcher-specific implementation and its user data
 * to \c data. If a listener has already been set, this function
 * fails and nothing is changed.
 *
 * The exact details of dispatcher_data depend on the dispatcher used.  This
 * function is intended to be used by language bindings, not user code.
 *
 * \c proxy must not be a proxy wrapper.
 *
 * \memberof wl_proxy
 */
WL_EXPORT int
wl_proxy_add_dispatcher(struct wl_proxy *proxy,
			wl_dispatcher_func_t dispatcher,
			const void *implementation, void *data)
{
	if (proxy->flags & WL_PROXY_FLAG_WRAPPER)
		wl_abort("Proxy %p is a wrapper\n", proxy);

	if (proxy->object.implementation || proxy->dispatcher) {
		wl_log("proxy %p already has listener\n", proxy);
		return -1;
	}

	proxy->object.implementation = implementation;
	proxy->dispatcher = dispatcher;
	proxy->user_data = data;

	return 0;
}

static struct wl_proxy *
create_outgoing_proxy(struct wl_proxy *proxy, const struct wl_message *message,
		      union wl_argument *args,
		      const struct wl_interface *interface, uint32_t version)
{
	int i, count;
	const char *signature;
	struct argument_details arg;
	struct wl_proxy *new_proxy = NULL;

	signature = message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);

		switch (arg.type) {
		case 'n':
			new_proxy = proxy_create(proxy, interface, version);
			if (new_proxy == NULL)
				return NULL;

			args[i].o = &new_proxy->object;
			break;
		}
	}

	return new_proxy;
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param args Extra arguments for the given request
 * \param interface The interface to use for the new proxy
 *
 * This function translates a request given an opcode, an interface and a
 * wl_argument array to the wire format and writes it to the connection
 * buffer.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will inherit their version from their parent.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_array_constructor(struct wl_proxy *proxy,
				   uint32_t opcode, union wl_argument *args,
				   const struct wl_interface *interface)
{
	return wl_proxy_marshal_array_constructor_versioned(proxy, opcode,
							    args, interface,
							    proxy->version);
}


/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param args Extra arguments for the given request
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version for the new proxy
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.  This version takes an
 * array of the union type wl_argument.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_array_constructor_versioned(struct wl_proxy *proxy,
					     uint32_t opcode,
					     union wl_argument *args,
					     const struct wl_interface *interface,
					     uint32_t version)
{
	return wl_proxy_marshal_array_flags(proxy, opcode, interface, version, 0, args);
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version of the new proxy
 * \param flags Flags that modify marshalling behaviour
 * \param ... Extra arguments for the given request
 * \return A new wl_proxy for the new_id argument or NULL on error
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * The flag WL_MARSHAL_FLAG_DESTROY may be passed to ensure the proxy
 * is destroyed atomically with the marshalling in order to prevent
 * races that can occur if the display lock is dropped between the
 * marshal and destroy operations.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_flags(struct wl_proxy *proxy, uint32_t opcode,
		       const struct wl_interface *interface, uint32_t version,
		       uint32_t flags, ...)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	va_list ap;

	va_start(ap, flags);
	wl_argument_from_va_list(proxy->object.interface->methods[opcode].signature,
				 args, WL_CLOSURE_MAX_ARGS, ap);
	va_end(ap);

	return wl_proxy_marshal_array_flags(proxy, opcode, interface, version, flags, args);
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version for the new proxy
 * \param flags Flags that modify marshalling behaviour
 * \param args Extra arguments for the given request
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.  This version takes an
 * array of the union type wl_argument.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * The flag WL_MARSHAL_FLAG_DESTROY may be passed to ensure the proxy
 * is destroyed atomically with the marshalling in order to prevent
 * races that can occur if the display lock is dropped between the
 * marshal and destroy operations.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal_flags()
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_array_flags(struct wl_proxy *proxy, uint32_t opcode,
			     const struct wl_interface *interface, uint32_t version,
			     uint32_t flags, union wl_argument *args)
{
	struct wl_closure *closure;
	struct wl_proxy *new_proxy = NULL;
	const struct wl_message *message;
	struct wl_display *disp = proxy->display;

	pthread_mutex_lock(&disp->mutex);

	message = &proxy->object.interface->methods[opcode];
	if (interface) {
		new_proxy = create_outgoing_proxy(proxy, message,
						  args, interface,
						  version);
		if (new_proxy == NULL)
			goto err_unlock;
	}

	if (proxy->display->last_error) {
		goto err_unlock;
	}

	closure = wl_closure_marshal(&proxy->object, opcode, args, message);
	if (closure == NULL) {
		wl_log("Error marshalling request: %s\n", strerror(errno));
		display_fatal_error(proxy->display, errno);
		goto err_unlock;
	}

	if (debug_client)
		wl_closure_print(closure, &proxy->object, true, false, NULL);

	if (wl_closure_send(closure, proxy->display->connection)) {
		wl_log("Error sending request: %s\n", strerror(errno));
		display_fatal_error(proxy->display, errno);
	}

	wl_closure_destroy(closure);

 err_unlock:
	if (flags & WL_MARSHAL_FLAG_DESTROY)
		wl_proxy_destroy_caller_locks(proxy);

	pthread_mutex_unlock(&disp->mutex);

	return new_proxy;
}


/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param ... Extra arguments for the given request
 *
 * This function is similar to wl_proxy_marshal_constructor(), except
 * it doesn't create proxies for new-id arguments.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \sa wl_proxy_create()
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_marshal(struct wl_proxy *proxy, uint32_t opcode, ...)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	va_list ap;

	va_start(ap, opcode);
	wl_argument_from_va_list(proxy->object.interface->methods[opcode].signature,
				 args, WL_CLOSURE_MAX_ARGS, ap);
	va_end(ap);

	wl_proxy_marshal_array_constructor(proxy, opcode, args, NULL);
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param ... Extra arguments for the given request
 * \return A new wl_proxy for the new_id argument or NULL on error
 *
 * This function translates a request given an opcode, an interface and extra
 * arguments to the wire format and writes it to the connection buffer. The
 * types of the extra arguments must correspond to the argument types of the
 * method associated with the opcode in the interface.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will inherit their version from their parent.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_constructor(struct wl_proxy *proxy, uint32_t opcode,
			     const struct wl_interface *interface, ...)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	va_list ap;

	va_start(ap, interface);
	wl_argument_from_va_list(proxy->object.interface->methods[opcode].signature,
				 args, WL_CLOSURE_MAX_ARGS, ap);
	va_end(ap);

	return wl_proxy_marshal_array_constructor(proxy, opcode,
						  args, interface);
}


/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param interface The interface to use for the new proxy
 * \param version The protocol object version of the new proxy
 * \param ... Extra arguments for the given request
 * \return A new wl_proxy for the new_id argument or NULL on error
 *
 * Translates the request given by opcode and the extra arguments into the
 * wire format and write it to the connection buffer.
 *
 * For new-id arguments, this function will allocate a new wl_proxy
 * and send the ID to the server.  The new wl_proxy will be returned
 * on success or NULL on error with errno set accordingly.  The newly
 * created proxy will have the version specified.
 *
 * \note This should not normally be used by non-generated code.
 *
 * \memberof wl_proxy
 */
WL_EXPORT struct wl_proxy *
wl_proxy_marshal_constructor_versioned(struct wl_proxy *proxy, uint32_t opcode,
				       const struct wl_interface *interface,
				       uint32_t version, ...)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	va_list ap;

	va_start(ap, version);
	wl_argument_from_va_list(proxy->object.interface->methods[opcode].signature,
				 args, WL_CLOSURE_MAX_ARGS, ap);
	va_end(ap);

	return wl_proxy_marshal_array_constructor_versioned(proxy, opcode,
							    args, interface,
							    version);
}

/** Prepare a request to be sent to the compositor
 *
 * \param proxy The proxy object
 * \param opcode Opcode of the request to be sent
 * \param args Extra arguments for the given request
 *
 * This function is similar to wl_proxy_marshal_array_constructor(), except
 * it doesn't create proxies for new-id arguments.
 *
 * \note This is intended to be used by language bindings and not in
 * non-generated code.
 *
 * \sa wl_proxy_marshal()
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_marshal_array(struct wl_proxy *proxy, uint32_t opcode,
		       union wl_argument *args)
{
	wl_proxy_marshal_array_constructor(proxy, opcode, args, NULL);
}

static void
display_handle_error(void *data,
		     struct wl_display *display, void *object,
		     uint32_t code, const char *message)
{
	struct wl_proxy *proxy = object;
	uint32_t object_id;
	const struct wl_interface *interface;

	if (proxy) {
		wl_log("%s@%u: error %d: %s\n",
		       proxy->object.interface->name,
		       proxy->object.id,
		       code, message);

		object_id = proxy->object.id;
		interface = proxy->object.interface;
	} else {
		wl_log("[destroyed object]: error %d: %s\n",
		       code, message);

		object_id = 0;
		interface = NULL;
	}

	display_protocol_error(display, code, object_id, interface);
}

static void
display_handle_delete_id(void *data, struct wl_display *display, uint32_t id)
{
	struct wl_proxy *proxy;

	pthread_mutex_lock(&display->mutex);

	proxy = wl_map_lookup(&display->objects, id);

	if (wl_object_is_zombie(&display->objects, id)) {
		/* For zombie objects, the 'proxy' is actually the zombie
		 * event-information structure, which we can free. */
		free(proxy);
		wl_map_remove(&display->objects, id);
	} else if (proxy) {
		proxy->flags |= WL_PROXY_FLAG_ID_DELETED;
	} else {
		wl_log("error: received delete_id for unknown id (%u)\n", id);
	}

	pthread_mutex_unlock(&display->mutex);
}

static const struct wl_display_listener display_listener = {
	display_handle_error,
	display_handle_delete_id
};

static int
connect_to_socket(const char *name)
{
	struct sockaddr_un addr;
	socklen_t size;
	const char *runtime_dir;
	int name_size, fd;
	bool path_is_absolute;

	if (name == NULL)
		name = getenv("WAYLAND_DISPLAY");
	if (name == NULL)
		name = "wayland-0";

	path_is_absolute = name[0] == '/';

	runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (((!runtime_dir || runtime_dir[0] != '/') && !path_is_absolute)) {
		wl_log("error: XDG_RUNTIME_DIR is invalid or not set in the environment.\n");
		/* to prevent programs reporting
		 * "failed to create display: Success" */
		errno = ENOENT;
		return -1;
	}

	fd = wl_os_socket_cloexec(PF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0)
		return -1;

	memset(&addr, 0, sizeof addr);
	addr.sun_family = AF_LOCAL;
	if (!path_is_absolute) {
		name_size =
			snprintf(addr.sun_path, sizeof addr.sun_path,
			         "%s/%s", runtime_dir, name) + 1;
	} else {
		/* absolute path */
		name_size =
			snprintf(addr.sun_path, sizeof addr.sun_path,
			         "%s", name) + 1;
	}

	assert(name_size > 0);
	if (name_size > (int)sizeof addr.sun_path) {
		if (!path_is_absolute) {
			wl_log("error: socket path \"%s/%s\" plus null terminator"
			       " exceeds %i bytes\n", runtime_dir, name, (int) sizeof(addr.sun_path));
		} else {
			wl_log("error: socket path \"%s\" plus null terminator"
			       " exceeds %i bytes\n", name, (int) sizeof(addr.sun_path));
		}
		close(fd);
		/* to prevent programs reporting
		 * "failed to add socket: Success" */
		errno = ENAMETOOLONG;
		return -1;
	};

	size = offsetof (struct sockaddr_un, sun_path) + name_size;

	if (connect(fd, (struct sockaddr *) &addr, size) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

/** Connect to Wayland display on an already open fd
 *
 * \param fd The fd to use for the connection
 * \return A \ref wl_display object or \c NULL on failure
 *
 * The wl_display takes ownership of the fd and will close it when the
 * display is destroyed.  The fd will also be closed in case of
 * failure.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_display *
wl_display_connect_to_fd(int fd)
{
	struct wl_display *display;
	const char *debug;

	debug = getenv("WAYLAND_DEBUG");
	if (debug && (strstr(debug, "client") || strstr(debug, "1")))
		debug_client = 1;

	display = zalloc(sizeof *display);
	if (display == NULL) {
		close(fd);
		return NULL;
	}

	display->fd = fd;
	wl_map_init(&display->objects, WL_MAP_CLIENT_SIDE);
	wl_event_queue_init(&display->default_queue, display);
	wl_event_queue_init(&display->display_queue, display);
	pthread_mutex_init(&display->mutex, NULL);
	pthread_cond_init(&display->reader_cond, NULL);
	display->reader_count = 0;

	if (wl_map_insert_at(&display->objects, 0, 0, NULL) == -1)
		goto err_connection;

	display->proxy.object.id =
		wl_map_insert_new(&display->objects, 0, display);

	if (display->proxy.object.id == 0)
		goto err_connection;

	display->proxy.object.interface = &wl_display_interface;
	display->proxy.display = display;
	display->proxy.object.implementation = (void(**)(void)) &display_listener;
	display->proxy.user_data = display;
	display->proxy.queue = &display->default_queue;
	display->proxy.flags = 0;
	display->proxy.refcount = 1;

	/* We set this version to 0 for backwards compatibility.
	 *
	 * If a client is using old versions of protocol headers,
	 * it will use unversioned API to create proxies.  Those
	 * proxies will inherit this 0.
	 *
	 * A client could be passing these proxies into library
	 * code newer than the headers that checks proxy
	 * versions.  When the proxy version is reported as 0
	 * the library will know that it can't reliably determine
	 * the proxy version, and should do whatever fallback is
	 * required.
	 *
	 * This trick forces wl_display to always report 0, but
	 * since it's a special object that we can't bind
	 * specific versions of anyway, this should be fine.
	 */
	display->proxy.version = 0;

	display->connection = wl_connection_create(display->fd);
	if (display->connection == NULL)
		goto err_connection;

	return display;

 err_connection:
	pthread_mutex_destroy(&display->mutex);
	pthread_cond_destroy(&display->reader_cond);
	wl_map_release(&display->objects);
	close(display->fd);
	free(display);

	return NULL;
}

/** Connect to a Wayland display
 *
 * \param name Name of the Wayland display to connect to
 * \return A \ref wl_display object or \c NULL on failure
 *
 * Connect to the Wayland display named \c name. If \c name is \c NULL,
 * its value will be replaced with the WAYLAND_DISPLAY environment
 * variable if it is set, otherwise display "wayland-0" will be used.
 *
 * If WAYLAND_SOCKET is set, it's interpreted as a file descriptor number
 * referring to an already opened socket. In this case, the socket is used
 * as-is and \c name is ignored.
 *
 * If \c name is a relative path, then the socket is opened relative to
 * the XDG_RUNTIME_DIR directory.
 *
 * If \c name is an absolute path, then that path is used as-is for
 * the location of the socket at which the Wayland server is listening;
 * no qualification inside XDG_RUNTIME_DIR is attempted.
 *
 * If \c name is \c NULL and the WAYLAND_DISPLAY environment variable
 * is set to an absolute pathname, then that pathname is used as-is
 * for the socket in the same manner as if \c name held an absolute
 * path. Support for absolute paths in \c name and WAYLAND_DISPLAY
 * is present since Wayland version 1.15.
 *
 * \memberof wl_display
 */
WL_EXPORT struct wl_display *
wl_display_connect(const char *name)
{
	char *connection, *end;
	int flags, fd;

	connection = getenv("WAYLAND_SOCKET");
	if (connection) {
		int prev_errno = errno;
		errno = 0;
		fd = strtol(connection, &end, 10);
		if (errno != 0 || connection == end || *end != '\0')
			return NULL;
		errno = prev_errno;

		flags = fcntl(fd, F_GETFD);
		if (flags == -1 && errno == EBADF)
			return NULL;
		else if (flags != -1)
			fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
		unsetenv("WAYLAND_SOCKET");
	} else {
		fd = connect_to_socket(name);
		if (fd < 0)
			return NULL;
	}

	return wl_display_connect_to_fd(fd);
}

/** Close a connection to a Wayland display
 *
 * \param display The display context object
 *
 * Close the connection to \c display. The \ref wl_proxy and
 * \ref wl_event_queue objects need to be manually destroyed by the caller
 * before disconnecting.
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_disconnect(struct wl_display *display)
{
	wl_connection_destroy(display->connection);
	wl_map_for_each(&display->objects, free_zombies, NULL);
	wl_map_release(&display->objects);
	wl_event_queue_release(&display->default_queue);
	wl_event_queue_release(&display->display_queue);
	pthread_mutex_destroy(&display->mutex);
	pthread_cond_destroy(&display->reader_cond);
	close(display->fd);

	free(display);
}

/** Get a display context's file descriptor
 *
 * \param display The display context object
 * \return Display object file descriptor
 *
 * Return the file descriptor associated with a display so it can be
 * integrated into the client's main loop.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_get_fd(struct wl_display *display)
{
	return display->fd;
}

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
	int *done = data;

	*done = 1;
	wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
	sync_callback
};

/** Block until all pending request are processed by the server
 *
 * \param display The display context object
 * \param queue The queue on which to run the roundtrip
 * \return The number of dispatched events on success or -1 on failure
 *
 * This function blocks until the server has processed all currently issued
 * requests by sending a request to the display server and waiting for a
 * reply before returning.
 *
 * This function uses wl_display_dispatch_queue() internally. It is not allowed
 * to call this function while the thread is being prepared for reading events,
 * and doing so will cause a dead lock.
 *
 * \note This function may dispatch other events being received on the given
 * queue.
 *
 * \sa wl_display_roundtrip()
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_roundtrip_queue(struct wl_display *display, struct wl_event_queue *queue)
{
	struct wl_display *display_wrapper;
	struct wl_callback *callback;
	int done, ret = 0;

	done = 0;

	display_wrapper = wl_proxy_create_wrapper(display);
	if (!display_wrapper)
		return -1;

	wl_proxy_set_queue((struct wl_proxy *) display_wrapper, queue);
	callback = wl_display_sync(display_wrapper);
	wl_proxy_wrapper_destroy(display_wrapper);

	if (callback == NULL)
		return -1;

	wl_callback_add_listener(callback, &sync_listener, &done);
	while (!done && ret >= 0)
		ret = wl_display_dispatch_queue(display, queue);

	if (ret == -1 && !done)
		wl_callback_destroy(callback);

	return ret;
}

/** Block until all pending request are processed by the server
 *
 * \param display The display context object
 * \return The number of dispatched events on success or -1 on failure
 *
 * This function blocks until the server has processed all currently issued
 * requests by sending a request to the display server and waiting for a reply
 * before returning.
 *
 * This function uses wl_display_dispatch_queue() internally. It is not allowed
 * to call this function while the thread is being prepared for reading events,
 * and doing so will cause a dead lock.
 *
 * \note This function may dispatch other events being received on the default
 * queue.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_roundtrip(struct wl_display *display)
{
	return wl_display_roundtrip_queue(display, &display->default_queue);
}

static int
create_proxies(struct wl_proxy *sender, struct wl_closure *closure)
{
	struct wl_proxy *proxy;
	const char *signature;
	struct argument_details arg;
	uint32_t id;
	int i;
	int count;

	signature = closure->message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		switch (arg.type) {
		case 'n':
			id = closure->args[i].n;
			if (id == 0) {
				closure->args[i].o = NULL;
				break;
			}
			proxy = wl_proxy_create_for_id(sender, id,
						       closure->message->types[i]);
			if (proxy == NULL)
				return -1;
			closure->args[i].o = (struct wl_object *)proxy;
			break;
		default:
			break;
		}
	}

	return 0;
}

static void
increase_closure_args_refcount(struct wl_closure *closure)
{
	const char *signature;
	struct argument_details arg;
	int i, count;
	struct wl_proxy *proxy;

	signature = closure->message->signature;
	count = arg_count_for_signature(signature);
	for (i = 0; i < count; i++) {
		signature = get_next_argument(signature, &arg);
		switch (arg.type) {
		case 'n':
		case 'o':
			proxy = (struct wl_proxy *) closure->args[i].o;
			if (proxy)
				proxy->refcount++;
			break;
		default:
			break;
		}
	}

	closure->proxy->refcount++;
}

static int
queue_event(struct wl_display *display, int len)
{
	uint32_t p[2], id;
	int opcode, size;
	struct wl_proxy *proxy;
	struct wl_closure *closure;
	const struct wl_message *message;
	struct wl_event_queue *queue;
	struct timespec tp;
	unsigned int time;
	int num_zombie_fds;

	wl_connection_copy(display->connection, p, sizeof p);
	id = p[0];
	opcode = p[1] & 0xffff;
	size = p[1] >> 16;
	if (len < size)
		return 0;

	/* If our proxy is gone or a zombie, just eat the event (and any FDs,
	 * if applicable). */
	proxy = wl_map_lookup(&display->objects, id);
	if (!proxy || wl_object_is_zombie(&display->objects, id)) {
		struct wl_zombie *zombie = wl_map_lookup(&display->objects, id);
		num_zombie_fds = (zombie && opcode < zombie->event_count) ?
			zombie->fd_count[opcode] : 0;

		if (debug_client) {
			clock_gettime(CLOCK_REALTIME, &tp);
			time = (tp.tv_sec * 1000000L) + (tp.tv_nsec / 1000);

			fprintf(stderr, "[%7u.%03u] discarded [%s]@%d.[event %d]"
				"(%d fd, %d byte)\n",
				time / 1000, time % 1000,
				zombie ? "zombie" : "unknown",
				id, opcode,
				num_zombie_fds, size);
		}
		if (num_zombie_fds > 0)
			wl_connection_close_fds_in(display->connection,
						   num_zombie_fds);

		wl_connection_consume(display->connection, size);
		return size;
	}

	if (opcode >= proxy->object.interface->event_count) {
		wl_log("interface '%s' has no event %u\n",
		       proxy->object.interface->name, opcode);
		return -1;
	}

	message = &proxy->object.interface->events[opcode];
	closure = wl_connection_demarshal(display->connection, size,
					  &display->objects, message);
	if (!closure)
		return -1;

	if (create_proxies(proxy, closure) < 0) {
		wl_closure_destroy(closure);
		return -1;
	}

	if (wl_closure_lookup_objects(closure, &display->objects) != 0) {
		wl_closure_destroy(closure);
		return -1;
	}

	closure->proxy = proxy;
	increase_closure_args_refcount(closure);

	if (proxy == &display->proxy)
		queue = &display->display_queue;
	else
		queue = proxy->queue;

	if (!queue)
		wl_abort("Tried to add event to destroyed queue\n");

	wl_list_insert(queue->event_list.prev, &closure->link);

	return size;
}

static uint32_t
id_from_object(union wl_argument *arg)
{
	struct wl_proxy *proxy;

	if (arg->o) {
		proxy = (struct wl_proxy *)arg->o;
		return proxy->object.id;
	}

	return 0;
}

static void
dispatch_event(struct wl_display *display, struct wl_event_queue *queue)
{
	struct wl_closure *closure;
	struct wl_proxy *proxy;
	int opcode;
	bool proxy_destroyed;

	closure = wl_container_of(queue->event_list.next, closure, link);
	wl_list_remove(&closure->link);
	opcode = closure->opcode;

	/* Verify that the receiving object is still valid by checking if has
	 * been destroyed by the application. */
	validate_closure_objects(closure);
	proxy = closure->proxy;
	proxy_destroyed = !!(proxy->flags & WL_PROXY_FLAG_DESTROYED);
	if (proxy_destroyed) {
		if (debug_client)
			wl_closure_print(closure, &proxy->object, false, true, id_from_object);
		destroy_queued_closure(closure);
		return;
	}

	pthread_mutex_unlock(&display->mutex);

	if (proxy->dispatcher) {
		if (debug_client)
			wl_closure_print(closure, &proxy->object, false, false, id_from_object);

		wl_closure_dispatch(closure, proxy->dispatcher,
				    &proxy->object, opcode);
	} else if (proxy->object.implementation) {
		if (debug_client)
			wl_closure_print(closure, &proxy->object, false, false, id_from_object);

		wl_closure_invoke(closure, WL_CLOSURE_INVOKE_CLIENT,
				  &proxy->object, opcode, proxy->user_data);
	}

	pthread_mutex_lock(&display->mutex);

	destroy_queued_closure(closure);
}

static int
read_events(struct wl_display *display)
{
	int total, rem, size;
	uint32_t serial;

	display->reader_count--;
	if (display->reader_count == 0) {
		total = wl_connection_read(display->connection);
		if (total == -1) {
			if (errno == EAGAIN) {
				/* we must wake up threads whenever
				 * the reader_count dropped to 0 */
				display_wakeup_threads(display);

				return 0;
			}

			display_fatal_error(display, errno);
			return -1;
		} else if (total == 0) {
			/* The compositor has closed the socket. This
			 * should be considered an error so we'll fake
			 * an errno */
			errno = EPIPE;
			display_fatal_error(display, errno);
			return -1;
		}

		for (rem = total; rem >= 8; rem -= size) {
			size = queue_event(display, rem);
			if (size == -1) {
				display_fatal_error(display, errno);
				return -1;
			} else if (size == 0) {
				break;
			}
		}

		display_wakeup_threads(display);
	} else {
		serial = display->read_serial;
		while (display->read_serial == serial)
			pthread_cond_wait(&display->reader_cond,
					  &display->mutex);

		if (display->last_error) {
			errno = display->last_error;
			return -1;
		}
	}

	return 0;
}

static void
cancel_read(struct wl_display *display)
{
	display->reader_count--;
	if (display->reader_count == 0)
		display_wakeup_threads(display);
}

/** Read events from display file descriptor
 *
 * \param display The display context object
 * \return 0 on success or -1 on error.  In case of error errno will
 * be set accordingly
 *
 * Calling this function will result in data available on the display file
 * descriptor being read and read events will be queued on their corresponding
 * event queues.
 *
 * Before calling this function, depending on what thread it is to be called
 * from, wl_display_prepare_read_queue() or wl_display_prepare_read() needs to
 * be called. See wl_display_prepare_read_queue() for more details.
 *
 * When being called at a point where other threads have been prepared to read
 * (using wl_display_prepare_read_queue() or wl_display_prepare_read()) this
 * function will sleep until all other prepared threads have either been
 * cancelled (using wl_display_cancel_read()) or them self entered this
 * function. The last thread that calls this function will then read and queue
 * events on their corresponding event queues, and finally wake up all other
 * wl_display_read_events() calls causing them to return.
 *
 * If a thread cancels a read preparation when all other threads that have
 * prepared to read has either called wl_display_cancel_read() or
 * wl_display_read_events(), all reader threads will return without having read
 * any data.
 *
 * To dispatch events that may have been queued, call
 * wl_display_dispatch_pending() or wl_display_dispatch_queue_pending().
 *
 * \sa wl_display_prepare_read(), wl_display_cancel_read(),
 * wl_display_dispatch_pending(), wl_display_dispatch()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_read_events(struct wl_display *display)
{
	int ret;

	pthread_mutex_lock(&display->mutex);

	if (display->last_error) {
		cancel_read(display);
		pthread_mutex_unlock(&display->mutex);

		errno = display->last_error;
		return -1;
	}

	ret = read_events(display);

	pthread_mutex_unlock(&display->mutex);

	return ret;
}

static int
dispatch_queue(struct wl_display *display, struct wl_event_queue *queue)
{
	int count;

	if (display->last_error)
		goto err;

	count = 0;
	while (!wl_list_empty(&display->display_queue.event_list)) {
		dispatch_event(display, &display->display_queue);
		if (display->last_error)
			goto err;
		count++;
	}

	while (!wl_list_empty(&queue->event_list)) {
		dispatch_event(display, queue);
		if (display->last_error)
			goto err;
		count++;
	}

	return count;

err:
	errno = display->last_error;

	return -1;
}

/** Prepare to read events from the display's file descriptor to a queue
 *
 * \param display The display context object
 * \param queue The event queue to use
 * \return 0 on success or -1 if event queue was not empty
 *
 * This function (or wl_display_prepare_read()) must be called before reading
 * from the file descriptor using wl_display_read_events(). Calling
 * wl_display_prepare_read_queue() announces the calling thread's intention to
 * read and ensures that until the thread is ready to read and calls
 * wl_display_read_events(), no other thread will read from the file descriptor.
 * This only succeeds if the event queue is empty, and if not -1 is returned and
 * errno set to EAGAIN.
 *
 * If a thread successfully calls wl_display_prepare_read_queue(), it must
 * either call wl_display_read_events() when it's ready or cancel the read
 * intention by calling wl_display_cancel_read().
 *
 * Use this function before polling on the display fd or integrate the fd into a
 * toolkit event loop in a race-free way. A correct usage would be (with most
 * error checking left out):
 *
 * \code
 * while (wl_display_prepare_read_queue(display, queue) != 0)
 *         wl_display_dispatch_queue_pending(display, queue);
 * wl_display_flush(display);
 *
 * ret = poll(fds, nfds, -1);
 * if (has_error(ret))
 *         wl_display_cancel_read(display);
 * else
 *         wl_display_read_events(display);
 *
 * wl_display_dispatch_queue_pending(display, queue);
 * \endcode
 *
 * Here we call wl_display_prepare_read_queue(), which ensures that between
 * returning from that call and eventually calling wl_display_read_events(), no
 * other thread will read from the fd and queue events in our queue. If the call
 * to wl_display_prepare_read_queue() fails, we dispatch the pending events and
 * try again until we're successful.
 *
 * The wl_display_prepare_read_queue() function doesn't acquire exclusive access
 * to the display's fd. It only registers that the thread calling this function
 * has intention to read from fd. When all registered readers call
 * wl_display_read_events(), only one (at random) eventually reads and queues
 * the events and the others are sleeping meanwhile. This way we avoid races and
 * still can read from more threads.
 *
 * \sa wl_display_cancel_read(), wl_display_read_events(),
 * wl_display_prepare_read()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_prepare_read_queue(struct wl_display *display,
			      struct wl_event_queue *queue)
{
	int ret;

	pthread_mutex_lock(&display->mutex);

	if (!wl_list_empty(&queue->event_list)) {
		errno = EAGAIN;
		ret = -1;
	} else {
		display->reader_count++;
		ret = 0;
	}

	pthread_mutex_unlock(&display->mutex);

	return ret;
}

/** Prepare to read events from the display's file descriptor
 *
 * \param display The display context object
 * \return 0 on success or -1 if event queue was not empty
 *
 * This function does the same thing as wl_display_prepare_read_queue()
 * with the default queue passed as the queue.
 *
 * \sa wl_display_prepare_read_queue
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_prepare_read(struct wl_display *display)
{
	return wl_display_prepare_read_queue(display, &display->default_queue);
}

/** Cancel read intention on display's fd
 *
 * \param display The display context object
 *
 * After a thread successfully called wl_display_prepare_read() it must
 * either call wl_display_read_events() or wl_display_cancel_read().
 * If the threads do not follow this rule it will lead to deadlock.
 *
 * \sa wl_display_prepare_read(), wl_display_read_events()
 *
 * \memberof wl_display
 */
WL_EXPORT void
wl_display_cancel_read(struct wl_display *display)
{
	pthread_mutex_lock(&display->mutex);

	cancel_read(display);

	pthread_mutex_unlock(&display->mutex);
}

static int
wl_display_poll(struct wl_display *display, short int events)
{
	int ret;
	struct pollfd pfd[1];

	pfd[0].fd = display->fd;
	pfd[0].events = events;
	do {
		ret = poll(pfd, 1, -1);
	} while (ret == -1 && errno == EINTR);

	return ret;
}

/** Dispatch events in an event queue
 *
 * \param display The display context object
 * \param queue The event queue to dispatch
 * \return The number of dispatched events on success or -1 on failure
 *
 * Dispatch events on the given event queue.
 *
 * If the given event queue is empty, this function blocks until there are
 * events to be read from the display fd. Events are read and queued on
 * the appropriate event queues. Finally, events on given event queue are
 * dispatched. On failure -1 is returned and errno set appropriately.
 *
 * In a multi threaded environment, do not manually wait using poll() (or
 * equivalent) before calling this function, as doing so might cause a dead
 * lock. If external reliance on poll() (or equivalent) is required, see
 * wl_display_prepare_read_queue() of how to do so.
 *
 * This function is thread safe as long as it dispatches the right queue on the
 * right thread. It is also compatible with the multi thread event reading
 * preparation API (see wl_display_prepare_read_queue()), and uses the
 * equivalent functionality internally. It is not allowed to call this function
 * while the thread is being prepared for reading events, and doing so will
 * cause a dead lock.
 *
 * It can be used as a helper function to ease the procedure of reading and
 * dispatching events.
 *
 * \note Since Wayland 1.5 the display has an extra queue
 * for its own events (i. e. delete_id). This queue is dispatched always,
 * no matter what queue we passed as an argument to this function.
 * That means that this function can return non-0 value even when it
 * haven't dispatched any event for the given queue.
 *
 * \sa wl_display_dispatch(), wl_display_dispatch_pending(),
 * wl_display_dispatch_queue_pending(), wl_display_prepare_read_queue()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_dispatch_queue(struct wl_display *display,
			  struct wl_event_queue *queue)
{
	int ret;

	if (wl_display_prepare_read_queue(display, queue) == -1)
		return wl_display_dispatch_queue_pending(display, queue);

	while (true) {
		ret = wl_display_flush(display);

		if (ret != -1 || errno != EAGAIN)
			break;

		if (wl_display_poll(display, POLLOUT) == -1) {
			wl_display_cancel_read(display);
			return -1;
		}
	}

	/* Don't stop if flushing hits an EPIPE; continue so we can read any
	 * protocol error that may have triggered it. */
	if (ret < 0 && errno != EPIPE) {
		wl_display_cancel_read(display);
		return -1;
	}

	if (wl_display_poll(display, POLLIN) == -1) {
		wl_display_cancel_read(display);
		return -1;
	}

	if (wl_display_read_events(display) == -1)
		return -1;

	return wl_display_dispatch_queue_pending(display, queue);
}

/** Dispatch pending events in an event queue
 *
 * \param display The display context object
 * \param queue The event queue to dispatch
 * \return The number of dispatched events on success or -1 on failure
 *
 * Dispatch all incoming events for objects assigned to the given
 * event queue. On failure -1 is returned and errno set appropriately.
 * If there are no events queued, this function returns immediately.
 *
 * \memberof wl_display
 * \since 1.0.2
 */
WL_EXPORT int
wl_display_dispatch_queue_pending(struct wl_display *display,
				  struct wl_event_queue *queue)
{
	int ret;

	pthread_mutex_lock(&display->mutex);

	ret = dispatch_queue(display, queue);

	pthread_mutex_unlock(&display->mutex);

	return ret;
}

/** Process incoming events
 *
 * \param display The display context object
 * \return The number of dispatched events on success or -1 on failure
 *
 * Dispatch events on the default event queue.
 *
 * If the default event queue is empty, this function blocks until there are
 * events to be read from the display fd. Events are read and queued on
 * the appropriate event queues. Finally, events on the default event queue
 * are dispatched. On failure -1 is returned and errno set appropriately.
 *
 * In a multi threaded environment, do not manually wait using poll() (or
 * equivalent) before calling this function, as doing so might cause a dead
 * lock. If external reliance on poll() (or equivalent) is required, see
 * wl_display_prepare_read_queue() of how to do so.
 *
 * This function is thread safe as long as it dispatches the right queue on the
 * right thread. It is also compatible with the multi thread event reading
 * preparation API (see wl_display_prepare_read_queue()), and uses the
 * equivalent functionality internally. It is not allowed to call this function
 * while the thread is being prepared for reading events, and doing so will
 * cause a dead lock.
 *
 * \note It is not possible to check if there are events on the queue
 * or not. For dispatching default queue events without blocking, see \ref
 * wl_display_dispatch_pending().
 *
 * \sa wl_display_dispatch_pending(), wl_display_dispatch_queue(),
 * wl_display_read_events()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_dispatch(struct wl_display *display)
{
	return wl_display_dispatch_queue(display, &display->default_queue);
}

/** Dispatch default queue events without reading from the display fd
 *
 * \param display The display context object
 * \return The number of dispatched events or -1 on failure
 *
 * This function dispatches events on the main event queue. It does not
 * attempt to read the display fd and simply returns zero if the main
 * queue is empty, i.e., it doesn't block.
 *
 * \sa wl_display_dispatch(), wl_display_dispatch_queue(),
 * wl_display_flush()
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_dispatch_pending(struct wl_display *display)
{
	return wl_display_dispatch_queue_pending(display,
						 &display->default_queue);
}

/** Retrieve the last error that occurred on a display
 *
 * \param display The display context object
 * \return The last error that occurred on \c display or 0 if no error occurred
 *
 * Return the last error that occurred on the display. This may be an error sent
 * by the server or caused by the local client.
 *
 * \note Errors are \b fatal. If this function returns non-zero the display
 * can no longer be used.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_get_error(struct wl_display *display)
{
	int ret;

	pthread_mutex_lock(&display->mutex);

	ret = display->last_error;

	pthread_mutex_unlock(&display->mutex);

	return ret;
}

/** Retrieves the information about a protocol error:
 *
 * \param display    The Wayland display
 * \param interface  if not NULL, stores the interface where the error occurred,
 *                   or NULL, if unknown.
 * \param id         if not NULL, stores the object id that generated
 *                   the error, or 0, if the object id is unknown. There's no
 *                   guarantee the object is still valid; the client must know
 *                   if it deleted the object.
 * \return           The error code as defined in the interface specification.
 *
 * \code
 * int err = wl_display_get_error(display);
 *
 * if (err == EPROTO) {
 *        code = wl_display_get_protocol_error(display, &interface, &id);
 *        handle_error(code, interface, id);
 * }
 *
 * ...
 * \endcode
 * \memberof wl_display
 */
WL_EXPORT uint32_t
wl_display_get_protocol_error(struct wl_display *display,
			      const struct wl_interface **interface,
			      uint32_t *id)
{
	uint32_t ret;

	pthread_mutex_lock(&display->mutex);

	ret = display->protocol_error.code;

	if (interface)
		*interface = display->protocol_error.interface;
	if (id)
		*id = display->protocol_error.id;

	pthread_mutex_unlock(&display->mutex);

	return ret;
}


/** Send all buffered requests on the display to the server
 *
 * \param display The display context object
 * \return The number of bytes sent on success or -1 on failure
 *
 * Send all buffered data on the client side to the server. Clients should
 * always call this function before blocking on input from the display fd.
 * On success, the number of bytes sent to the server is returned. On
 * failure, this function returns -1 and errno is set appropriately.
 *
 * wl_display_flush() never blocks.  It will write as much data as
 * possible, but if all data could not be written, errno will be set
 * to EAGAIN and -1 returned.  In that case, use poll on the display
 * file descriptor to wait for it to become writable again.
 *
 * \memberof wl_display
 */
WL_EXPORT int
wl_display_flush(struct wl_display *display)
{
	int ret;

	pthread_mutex_lock(&display->mutex);

	if (display->last_error) {
		errno = display->last_error;
		ret = -1;
	} else {
		/* We don't make EPIPE a fatal error here, so that we may try to
		 * read events after the failed flush. When the compositor sends
		 * an error it will close the socket, and if we make EPIPE fatal
		 * here we don't get a chance to process the error. */
		ret = wl_connection_flush(display->connection);
		if (ret < 0 && errno != EAGAIN && errno != EPIPE)
			display_fatal_error(display, errno);
	}

	pthread_mutex_unlock(&display->mutex);

	return ret;
}

/** Set the user data associated with a proxy
 *
 * \param proxy The proxy object
 * \param user_data The data to be associated with proxy
 *
 * Set the user data associated with \c proxy. When events for this
 * proxy are received, \c user_data will be supplied to its listener.
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_set_user_data(struct wl_proxy *proxy, void *user_data)
{
	proxy->user_data = user_data;
}

/** Get the user data associated with a proxy
 *
 * \param proxy The proxy object
 * \return The user data associated with proxy
 *
 * \memberof wl_proxy
 */
WL_EXPORT void *
wl_proxy_get_user_data(struct wl_proxy *proxy)
{
	return proxy->user_data;
}

/** Get the protocol object version of a proxy object
 *
 * \param proxy The proxy object
 * \return The protocol object version of the proxy or 0
 *
 * Gets the protocol object version of a proxy object, or 0
 * if the proxy was created with unversioned API.
 *
 * A returned value of 0 means that no version information is
 * available, so the caller must make safe assumptions about
 * the object's real version.
 *
 * wl_display's version will always return 0.
 *
 * \memberof wl_proxy
 */
WL_EXPORT uint32_t
wl_proxy_get_version(struct wl_proxy *proxy)
{
	return proxy->version;
}

/** Get the id of a proxy object
 *
 * \param proxy The proxy object
 * \return The id the object associated with the proxy
 *
 * \memberof wl_proxy
 */
WL_EXPORT uint32_t
wl_proxy_get_id(struct wl_proxy *proxy)
{
	return proxy->object.id;
}

/** Set the tag of a proxy object
 *
 * A toolkit or application can set a unique tag on a proxy in order to
 * identify whether an object is managed by itself or some external part.
 *
 * To create a tag, the recommended way is to define a statically allocated
 * constant char array containing some descriptive string. The tag will be the
 * pointer to the non-const pointer to the beginning of the array.
 *
 * For example, to define and set a tag on a surface managed by a certain
 * subsystem:
 *
 * 	static const char *my_tag = "my tag";
 *
 * 	wl_proxy_set_tag((struct wl_proxy *) surface, &my_tag);
 *
 * Then, in a callback with wl_surface as an argument, in order to check
 * whether it's a surface managed by the same subsystem.
 *
 * 	const char * const *tag;
 *
 * 	tag = wl_proxy_get_tag((struct wl_proxy *) surface);
 * 	if (tag != &my_tag)
 *		return;
 *
 *	...
 *
 * For debugging purposes, a tag should be suitable to be included in a debug
 * log entry, e.g.
 *
 * 	const char * const *tag;
 *
 * 	tag = wl_proxy_get_tag((struct wl_proxy *) surface);
 * 	printf("Got a surface with the tag %p (%s)\n",
 * 	       tag, (tag && *tag) ? *tag : "");
 *
 * \param proxy The proxy object
 * \param tag The tag
 *
 * \memberof wl_proxy
 * \since 1.17.90
 */
WL_EXPORT void
wl_proxy_set_tag(struct wl_proxy *proxy,
		 const char * const *tag)
{
	proxy->tag = tag;
}

/** Get the tag of a proxy object
 *
 * See wl_proxy_set_tag for details.
 *
 * \param proxy The proxy object
 *
 * \memberof wl_proxy
 * \since 1.17.90
 */
WL_EXPORT const char * const *
wl_proxy_get_tag(struct wl_proxy *proxy)
{
	return proxy->tag;
}

/** Get the interface name (class) of a proxy object
 *
 * \param proxy The proxy object
 * \return The interface name of the object associated with the proxy
 *
 * \memberof wl_proxy
 */
WL_EXPORT const char *
wl_proxy_get_class(struct wl_proxy *proxy)
{
	return proxy->object.interface->name;
}

/** Assign a proxy to an event queue
 *
 * \param proxy The proxy object
 * \param queue The event queue that will handle this proxy or NULL
 *
 * Assign proxy to event queue. Events coming from \c proxy will be
 * queued in \c queue from now. If queue is NULL, then the display's
 * default queue is set to the proxy.
 *
 * In order to guarantee proper handing of all events which were queued
 * before the queue change takes effect, it is required to dispatch the
 * proxy's old event queue after setting a new event queue.
 *
 * This is particularly important for multi-threaded setups, where it is
 * possible for events to be queued to the proxy's old queue from a
 * different thread during the invocation of this function.
 *
 * To ensure that all events for a newly created proxy are dispatched
 * on a particular queue, it is necessary to use a proxy wrapper if
 * events are read and dispatched on more than one thread. See
 * wl_proxy_create_wrapper() for more details.
 *
 * \note By default, the queue set in proxy is the one inherited from parent.
 *
 * \sa wl_display_dispatch_queue()
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_set_queue(struct wl_proxy *proxy, struct wl_event_queue *queue)
{
	pthread_mutex_lock(&proxy->display->mutex);

	wl_list_remove(&proxy->queue_link);

	if (queue) {
		assert(proxy->display == queue->display);
		proxy->queue = queue;
	} else {
		proxy->queue = &proxy->display->default_queue;
	}

	wl_list_insert(&proxy->queue->proxy_list, &proxy->queue_link);

	pthread_mutex_unlock(&proxy->display->mutex);
}

/** Create a proxy wrapper for making queue assignments thread-safe
 *
 * \param proxy The proxy object to be wrapped
 * \return A proxy wrapper for the given proxy or NULL on failure
 *
 * A proxy wrapper is type of 'struct wl_proxy' instance that can be used when
 * sending requests instead of using the original proxy. A proxy wrapper does
 * not have an implementation or dispatcher, and events received on the
 * object is still emitted on the original proxy. Trying to set an
 * implementation or dispatcher will have no effect but result in a warning
 * being logged.
 *
 * Setting the proxy queue of the proxy wrapper will make new objects created
 * using the proxy wrapper use the set proxy queue.
 * Even though there is no implementation nor dispatcher, the proxy queue can
 * be changed. This will affect the default queue of new objects created by
 * requests sent via the proxy wrapper.
 *
 * A proxy wrapper can only be destroyed using wl_proxy_wrapper_destroy().
 *
 * A proxy wrapper must be destroyed before the proxy it was created from.
 *
 * If a user reads and dispatches events on more than one thread, it is
 * necessary to use a proxy wrapper when sending requests on objects when the
 * intention is that a newly created proxy is to use a proxy queue different
 * from the proxy the request was sent on, as creating the new proxy and then
 * setting the queue is not thread safe.
 *
 * For example, a module that runs using its own proxy queue that needs to
 * do display roundtrip must wrap the wl_display proxy object before sending
 * the wl_display.sync request. For example:
 *
 * \code
 *
 *   struct wl_event_queue *queue = ...;
 *   struct wl_display *wrapped_display;
 *   struct wl_callback *callback;
 *
 *   wrapped_display = wl_proxy_create_wrapper(display);
 *   wl_proxy_set_queue((struct wl_proxy *) wrapped_display, queue);
 *   callback = wl_display_sync(wrapped_display);
 *   wl_proxy_wrapper_destroy(wrapped_display);
 *   wl_callback_add_listener(callback, ...);
 *
 * \endcode
 *
 * \memberof wl_proxy
 */
WL_EXPORT void *
wl_proxy_create_wrapper(void *proxy)
{
	struct wl_proxy *wrapped_proxy = proxy;
	struct wl_proxy *wrapper;

	wrapper = zalloc(sizeof *wrapper);
	if (!wrapper)
		return NULL;

	pthread_mutex_lock(&wrapped_proxy->display->mutex);

	wrapper->object.interface = wrapped_proxy->object.interface;
	wrapper->object.id = wrapped_proxy->object.id;
	wrapper->version = wrapped_proxy->version;
	wrapper->display = wrapped_proxy->display;
	wrapper->queue = wrapped_proxy->queue;
	wrapper->flags = WL_PROXY_FLAG_WRAPPER;
	wrapper->refcount = 1;

	wl_list_insert(&wrapper->queue->proxy_list, &wrapper->queue_link);

	pthread_mutex_unlock(&wrapped_proxy->display->mutex);

	return wrapper;
}

/** Destroy a proxy wrapper
 * \param proxy_wrapper The proxy wrapper to be destroyed
 *
 * \memberof wl_proxy
 */
WL_EXPORT void
wl_proxy_wrapper_destroy(void *proxy_wrapper)
{
	struct wl_proxy *wrapper = proxy_wrapper;

	if (!(wrapper->flags & WL_PROXY_FLAG_WRAPPER))
		wl_abort("Tried to destroy non-wrapper proxy with "
			 "wl_proxy_wrapper_destroy\n");

	assert(wrapper->refcount == 1);

	pthread_mutex_lock(&wrapper->display->mutex);

	wl_list_remove(&wrapper->queue_link);

	pthread_mutex_unlock(&wrapper->display->mutex);

	free(wrapper);
}

WL_EXPORT void
wl_log_set_handler_client(wl_log_func_t handler)
{
	wl_log_handler = handler;
}
