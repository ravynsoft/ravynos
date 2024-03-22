/*
 * Copyright © 2012 Intel Corporation
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "wayland-private.h"
#include "wayland-server.h"
#include "test-runner.h"

struct client_destroy_listener {
	struct wl_listener listener;
	bool done;
	struct wl_listener late_listener;
	bool late_done;
	struct wl_listener resource_listener;
	bool resource_done;
};

static void
client_destroy_notify(struct wl_listener *l, void *data)
{
	struct client_destroy_listener *listener =
		wl_container_of(l, listener, listener);

	listener->done = true;
	assert(!listener->resource_done);
	assert(!listener->late_done);
}

static void
client_resource_destroy_notify(struct wl_listener *l, void *data)
{
	struct client_destroy_listener *listener =
		wl_container_of(l, listener, resource_listener);

	assert(listener->done);
	listener->resource_done = true;
	assert(!listener->late_done);
}

static void
client_late_destroy_notify(struct wl_listener *l, void *data)
{
	struct client_destroy_listener *listener =
		wl_container_of(l, listener, late_listener);

	assert(listener->done);
	assert(listener->resource_done);
	listener->late_done = true;
}

TEST(client_destroy_listener)
{
	struct wl_display *display;
	struct wl_client *client;
	struct wl_resource *resource;
	struct client_destroy_listener a, b;
	int s[2];

	assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, s) == 0);
	display = wl_display_create();
	assert(display);
	client = wl_client_create(display, s[0]);
	assert(client);

	resource = wl_resource_create(client, &wl_callback_interface, 1, 0);
	assert(resource);

	a.listener.notify = client_destroy_notify;
	a.done = false;
	a.resource_listener.notify = client_resource_destroy_notify;
	a.resource_done = false;
	a.late_listener.notify = client_late_destroy_notify;
	a.late_done = false;
	wl_client_add_destroy_listener(client, &a.listener);
	wl_resource_add_destroy_listener(resource, &a.resource_listener);
	wl_client_add_destroy_late_listener(client, &a.late_listener);

	assert(wl_client_get_destroy_listener(client, client_destroy_notify) ==
	       &a.listener);
	assert(wl_resource_get_destroy_listener(resource, client_resource_destroy_notify) ==
	       &a.resource_listener);
	assert(wl_client_get_destroy_late_listener(client, client_late_destroy_notify) ==
	       &a.late_listener);

	b.listener.notify = client_destroy_notify;
	b.done = false;
	b.resource_listener.notify = client_resource_destroy_notify;
	b.resource_done = false;
	b.late_listener.notify = client_late_destroy_notify;
	b.late_done = false;
	wl_client_add_destroy_listener(client, &b.listener);
	wl_resource_add_destroy_listener(resource, &b.resource_listener);
	wl_client_add_destroy_late_listener(client, &b.late_listener);

	wl_list_remove(&a.listener.link);
	wl_list_remove(&a.resource_listener.link);
	wl_list_remove(&a.late_listener.link);

	wl_client_destroy(client);

	assert(!a.done);
	assert(!a.resource_done);
	assert(!a.late_done);
	assert(b.done);
	assert(b.resource_done);
	assert(b.late_done);

	close(s[0]);
	close(s[1]);

	wl_display_destroy(display);
}

