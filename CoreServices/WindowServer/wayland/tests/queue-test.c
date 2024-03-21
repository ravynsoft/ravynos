/*
 * Copyright © 2012 Jonas Ådahl
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

#define _GNU_SOURCE /* For memrchr */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>

#include "wayland-client.h"
#include "wayland-server.h"
#include "test-runner.h"
#include "test-compositor.h"

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

static void
registry_handle_global(void *data, struct wl_registry *registry,
		       uint32_t id, const char *interface, uint32_t version)
{
	int *pcounter = data;
	(*pcounter)++;
	assert(*pcounter == 1);
	wl_registry_destroy(registry);
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	NULL
};

/* Test that destroying a proxy object doesn't result in any more
 * callback being invoked, even though were many queued. */
static void
client_test_proxy_destroy(void)
{
	struct wl_display *display;
	struct wl_registry *registry;
	int counter = 0;

	display = wl_display_connect(NULL);
	assert(display);

	registry = wl_display_get_registry(display);
	assert(registry != NULL);
	wl_registry_add_listener(registry, &registry_listener,
				 &counter);
	assert(wl_display_roundtrip(display) != -1);

	assert(counter == 1);

	/* don't destroy the registry, we have already destroyed them
	 * in the global handler */
	wl_display_disconnect(display);
}

struct multiple_queues_state {
	struct wl_display *display;
	struct wl_callback* callback2;
	bool done;
};

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
	struct multiple_queues_state *state = data;

	state->done = true;
	wl_callback_destroy(callback);

	wl_display_dispatch_pending(state->display);

	wl_callback_destroy(state->callback2);
}

static const struct wl_callback_listener sync_listener = {
	sync_callback
};

/* Test that when receiving the first of two synchronization
 * callback events, destroying the second one doesn't cause any
 * errors even if the delete_id event is handled out of order. */
static void
client_test_multiple_queues(void)
{
	struct wl_event_queue *queue;
	struct wl_callback *callback1;
	struct multiple_queues_state state;
	int ret = 0;

	state.display = wl_display_connect(NULL);
	assert(state.display);

	queue = wl_display_create_queue(state.display);
	assert(queue);

	state.done = false;
	callback1 = wl_display_sync(state.display);
	assert(callback1 != NULL);
	wl_callback_add_listener(callback1, &sync_listener, &state);
	wl_proxy_set_queue((struct wl_proxy *) callback1, queue);

	state.callback2 = wl_display_sync(state.display);
	assert(state.callback2 != NULL);
	wl_callback_add_listener(state.callback2, &sync_listener, NULL);
	wl_proxy_set_queue((struct wl_proxy *) state.callback2, queue);

	wl_display_flush(state.display);

	while (!state.done && !ret)
		ret = wl_display_dispatch_queue(state.display, queue);

	wl_event_queue_destroy(queue);
	wl_display_disconnect(state.display);

	exit(ret == -1 ? -1 : 0);
}

static void
sync_callback_roundtrip(void *data, struct wl_callback *callback, uint32_t serial)
{
	bool *done = data;
	*done = true;
}

static const struct wl_callback_listener sync_listener_roundtrip = {
	sync_callback_roundtrip
};

/* Test that doing a roundtrip on a queue only the events on that
 * queue get dispatched. */
static void
client_test_queue_roundtrip(void)
{
	struct wl_event_queue *queue;
	struct wl_callback *callback1;
	struct wl_callback *callback2;
	struct wl_display *display;
	bool done1 = false;
	bool done2 = false;

	display = wl_display_connect(NULL);
	assert(display);

	queue = wl_display_create_queue(display);
	assert(queue);

	/* arm a callback on the default queue */
	callback1 = wl_display_sync(display);
	assert(callback1 != NULL);
	wl_callback_add_listener(callback1, &sync_listener_roundtrip, &done1);

	/* arm a callback on the other queue */
	callback2 = wl_display_sync(display);
	assert(callback2 != NULL);
	wl_callback_add_listener(callback2, &sync_listener_roundtrip, &done2);
	wl_proxy_set_queue((struct wl_proxy *) callback2, queue);

	/* roundtrip on default queue must not dispatch the other queue. */
	wl_display_roundtrip(display);
	assert(done1 == true);
	assert(done2 == false);

	/* re-arm the sync callback on the default queue, so we see that
	 * wl_display_roundtrip_queue() does not dispatch the default queue. */
	wl_callback_destroy(callback1);
	done1 = false;
	callback1 = wl_display_sync(display);
	assert(callback1 != NULL);
	wl_callback_add_listener(callback1, &sync_listener_roundtrip, &done1);

	wl_display_roundtrip_queue(display, queue);
	assert(done1 == false);
	assert(done2 == true);

	wl_callback_destroy(callback1);
	wl_callback_destroy(callback2);
	wl_event_queue_destroy(queue);

	wl_display_disconnect(display);
}

static void
client_test_queue_proxy_wrapper(void)
{
	struct wl_event_queue *queue;
	struct wl_display *display;
	struct wl_display *display_wrapper;
	struct wl_callback *callback;
	bool done = false;

	/*
	 * For an illustration of what usage would normally fail without using
	 * proxy wrappers, see the `client_test_queue_set_queue_race' test case.
	 */

	display = wl_display_connect(NULL);
	assert(display);

	/* Pretend we are in a separate thread where a thread-local queue is
	 * used. */
	queue = wl_display_create_queue(display);
	assert(queue);

	display_wrapper = wl_proxy_create_wrapper(display);
	assert(display_wrapper);
	wl_proxy_set_queue((struct wl_proxy *) display_wrapper, queue);
	callback = wl_display_sync(display_wrapper);
	wl_proxy_wrapper_destroy(display_wrapper);
	assert(callback != NULL);

	/* Pretend we are now another thread and dispatch the dispatch the main
	 * queue while also knowing our callback is read and queued. */
	wl_display_roundtrip(display);

	/* Make sure that the pretend-to-be main thread didn't dispatch our
	 * callback, behind our back. */
	wl_callback_add_listener(callback, &sync_listener_roundtrip, &done);
	wl_display_flush(display);

	assert(!done);

	/* Make sure that we eventually end up dispatching our callback. */
	while (!done)
		assert(wl_display_dispatch_queue(display, queue) != -1);

	wl_callback_destroy(callback);
	wl_event_queue_destroy(queue);

	wl_display_disconnect(display);
}

static void
client_test_queue_set_queue_race(void)
{
	struct wl_event_queue *queue;
	struct wl_display *display;
	struct wl_callback *callback;
	bool done = false;

	/*
	 * This test illustrates the multi threading scenario which would fail
	 * without doing what is done in the `client_test_queue_proxy_wrapper'
	 * test.
	 */

	display = wl_display_connect(NULL);
	assert(display);

	/* Pretend we are in a separate thread where a thread-local queue is
	 * used. */
	queue = wl_display_create_queue(display);
	assert(queue);

	callback = wl_display_sync(display);
	assert(callback != NULL);

	/* Pretend we are now another thread and dispatch the dispatch the main
	 * queue while also knowing our callback is read, queued on the wrong
	 * queue, and dispatched. */
	wl_display_roundtrip(display);

	/* Pretend we are back in the separate thread, and continue with setting
	 * up our callback. */
	wl_callback_add_listener(callback, &sync_listener_roundtrip, &done);
	wl_proxy_set_queue((struct wl_proxy *) callback, queue);

	/* Roundtrip our separate thread queue to make sure any events are
	 * dispatched. */
	wl_display_roundtrip_queue(display, queue);

	/* Verify that the callback has indeed been dropped. */
	assert(!done);

	wl_callback_destroy(callback);
	wl_event_queue_destroy(queue);

	wl_display_disconnect(display);
}

static char *
maybe_map_file(int fd, size_t *len)
{
	char *data;

	*len = lseek(fd, 0, SEEK_END);
	data = mmap(0, *len, PROT_READ, MAP_PRIVATE, fd, 0);

	return data;
}

static char *
map_file(int fd, size_t *len)
{
	char *data;

	data = maybe_map_file(fd, len);
	assert(data != MAP_FAILED && "Failed to mmap file");

	return data;
}

static char *
last_line_of(char *s)
{
	size_t len = strlen(s);
	char *last;

	last = memrchr(s, '\n', len);
	/* If we found a newline at end of string, find the previous one. */
	if (last && last[1] == 0)
		last = memrchr(s, '\n', len - 1);
	/* If we have a newline, the last line starts after the newline.
	 * Otherwise, the whole string is the last line. */
	if (last)
		last += 1;
	else
		last = s;

	return last;
}

static void
client_test_queue_destroy_with_attached_proxies(void)
{
	struct wl_event_queue *queue;
	struct wl_display *display;
	struct wl_display *display_wrapper;
	struct wl_callback *callback;
	char *log;
	size_t log_len;
	char callback_name[24];
	int ret;

	display = wl_display_connect(NULL);
	assert(display);

	/* Pretend we are in a separate thread where a thread-local queue is
	 * used. */
	queue = wl_display_create_queue(display);
	assert(queue);

	/* Create a sync dispatching events on the thread-local queue. */
	display_wrapper = wl_proxy_create_wrapper(display);
	assert(display_wrapper);
	wl_proxy_set_queue((struct wl_proxy *) display_wrapper, queue);
	callback = wl_display_sync(display_wrapper);
	wl_proxy_wrapper_destroy(display_wrapper);
	assert(callback != NULL);

	/* Destroy the queue before the attached object. */
	wl_event_queue_destroy(queue);

	/* Check that the log contains some information about the attached
	 * wl_callback proxy. */
	log = map_file(client_log_fd, &log_len);
	ret = snprintf(callback_name, sizeof(callback_name), "wl_callback@%u",
		       wl_proxy_get_id((struct wl_proxy *) callback));
	assert(ret > 0 && ret < (int)sizeof(callback_name) &&
	       "callback name creation failed (possibly truncated)");
	assert(strstr(last_line_of(log), callback_name));
	munmap(log, log_len);

	wl_callback_destroy(callback);

	wl_display_disconnect(display);
}

static void
client_test_queue_proxy_event_to_destroyed_queue(void)
{
	struct wl_event_queue *queue;
	struct wl_display *display;
	struct wl_display *display_wrapper;
	struct wl_callback *callback;

	display = wl_display_connect(NULL);
	assert(display);

	/* Pretend we are in a separate thread where a thread-local queue is
	 * used. */
	queue = wl_display_create_queue(display);
	assert(queue);

	/* Create a sync dispatching events on the thread-local queue. */
	display_wrapper = wl_proxy_create_wrapper(display);
	assert(display_wrapper);
	wl_proxy_set_queue((struct wl_proxy *) display_wrapper, queue);
	callback = wl_display_sync(display_wrapper);
	wl_proxy_wrapper_destroy(display_wrapper);
	assert(callback != NULL);
	wl_display_flush(display);

	/* Destroy the queue before the attached object. */
	wl_event_queue_destroy(queue);

	/* During this roundtrip we should receive the done event on 'callback',
	 * try to queue it to the destroyed queue, and abort. */
	wl_display_roundtrip(display);

	wl_callback_destroy(callback);

	wl_display_disconnect(display);
}

static void
client_test_queue_destroy_default_with_attached_proxies(void)
{
	struct wl_display *display;
	struct wl_callback *callback;
	char *log;
	size_t log_len;
	char callback_name[24];
	int ret;

	display = wl_display_connect(NULL);
	assert(display);

	/* Create a sync dispatching events on the default queue. */
	callback = wl_display_sync(display);
	assert(callback != NULL);

	/* Destroy the default queue (by disconnecting) before the attached
	 * object. */
	wl_display_disconnect(display);

	/* Check that the log does not contain any warning about the attached
	 * wl_callback proxy. */
	log = maybe_map_file(client_log_fd, &log_len);
	ret = snprintf(callback_name, sizeof(callback_name), "wl_callback@%u",
		       wl_proxy_get_id((struct wl_proxy *) callback));
	assert(ret > 0 && ret < (int)sizeof(callback_name) &&
	       "callback name creation failed (possibly truncated)");
	assert(log == MAP_FAILED || strstr(log, callback_name) == NULL);
	if (log != MAP_FAILED)
		munmap(log, log_len);

	/* HACK: Directly free the memory of the wl_callback proxy to appease
	 * ASan. We would normally use wl_callback_destroy(), but since we have
	 * destroyed the associated wl_display, using this function would lead
	 * to memory errors. */
	free(callback);
}

static void
dummy_bind(struct wl_client *client,
	   void *data, uint32_t version, uint32_t id)
{
}

TEST(queue_proxy_destroy)
{
	struct display *d;
	const struct wl_interface *dummy_interfaces[] = {
		&wl_seat_interface,
		&wl_pointer_interface,
		&wl_keyboard_interface,
		&wl_surface_interface
	};
	unsigned int i;

	d = display_create();

	for (i = 0; i < ARRAY_LENGTH(dummy_interfaces); i++)
		wl_global_create(d->wl_display, dummy_interfaces[i],
				 dummy_interfaces[i]->version,
				 NULL, dummy_bind);

	test_set_timeout(2);

	client_create_noarg(d, client_test_proxy_destroy);
	display_run(d);

	display_destroy(d);
}

TEST(queue_multiple_queues)
{
	struct display *d = display_create();

	test_set_timeout(2);

	client_create_noarg(d, client_test_multiple_queues);
	display_run(d);

	display_destroy(d);
}

TEST(queue_roundtrip)
{
	struct display *d = display_create();

	test_set_timeout(2);

	client_create_noarg(d, client_test_queue_roundtrip);
	display_run(d);

	display_destroy(d);
}

TEST(queue_set_queue_proxy_wrapper)
{
	struct display *d = display_create();

	test_set_timeout(2);

	client_create_noarg(d, client_test_queue_proxy_wrapper);
	display_run(d);

	display_destroy(d);
}

TEST(queue_set_queue_race)
{
	struct display *d = display_create();

	test_set_timeout(2);

	client_create_noarg(d, client_test_queue_set_queue_race);
	display_run(d);

	display_destroy(d);
}

TEST(queue_destroy_with_attached_proxies)
{
	struct display *d = display_create();

	test_set_timeout(2);

	client_create_noarg(d, client_test_queue_destroy_with_attached_proxies);
	display_run(d);

	display_destroy(d);
}

TEST(queue_proxy_event_to_destroyed_queue)
{
	struct display *d = display_create();
	struct client_info *ci;
	char *client_log;
	size_t client_log_len;

	test_set_timeout(2);

	ci = client_create_noarg(d, client_test_queue_proxy_event_to_destroyed_queue);
	display_run(d);

	/* Check that the final line in the log mentions the expected reason
	 * for the abort. */
	client_log = map_file(ci->log_fd, &client_log_len);
	assert(!strcmp(last_line_of(client_log),
		       "Tried to add event to destroyed queue\n"));
	munmap(client_log, client_log_len);

	/* Check that the client aborted. */
	display_destroy_expect_signal(d, SIGABRT);
}

TEST(queue_destroy_default_with_attached_proxies)
{
	struct display *d = display_create();

	test_set_timeout(2);

	client_create_noarg(d, client_test_queue_destroy_default_with_attached_proxies);
	display_run(d);

	display_destroy(d);
}
