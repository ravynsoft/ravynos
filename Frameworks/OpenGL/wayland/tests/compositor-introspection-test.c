/*
 * Copyright © 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>

#include "wayland-client.h"
#include "wayland-server.h"
#include "test-runner.h"

/* Ensure the connection doesn't fail due to lack of XDG_RUNTIME_DIR. */
static const char *
require_xdg_runtime_dir(void)
{
	char *val = getenv("XDG_RUNTIME_DIR");
	assert(val && val[0] == '/' && "set $XDG_RUNTIME_DIR to run this test");

	return val;
}

struct compositor {
	struct wl_display *display;
	struct wl_listener listener;
	struct wl_client *client;
};

static void
client_created(struct wl_listener *listener, void *data)
{
	struct compositor *c = wl_container_of(listener, c, listener);
	c->client = data;
}

static void
check_client_list(struct compositor *compositor)
{
	struct wl_list *client_list;
	struct wl_client *client, *client_it;
	int num_clients = 0;

	client_list = wl_display_get_client_list(compositor->display);
	wl_client_for_each(client_it, client_list) {
		num_clients++;
		client = client_it;
	}
	assert(num_clients == 1);
	/* 'client_it' is not valid here, so we took a copy of the client in the loop.
	 * We could also do this assert in the loop directly, but in case it fails it is
	 * easier to understand the problem when we know that the previous assert passed,
	 * so that there is only one client but the wrong one. */
	assert(compositor->client == client);
}

static const char *
setup_compositor(struct compositor *compositor)
{
	const char *socket;

	require_xdg_runtime_dir();

	compositor->display = wl_display_create();
	socket = wl_display_add_socket_auto(compositor->display);

	compositor->listener.notify = client_created;
	wl_display_add_client_created_listener(compositor->display, &compositor->listener);

	return socket;
}

static void
cleanup_compositor(struct compositor *compositor)
{
	wl_client_destroy(compositor->client);
	wl_display_destroy(compositor->display);
}

TEST(new_client_connect)
{
	const char *socket;
	struct compositor compositor = { 0 };
	struct {
		struct wl_display *display;
	} client;

	socket = setup_compositor(&compositor);

	client.display = wl_display_connect(socket);

	wl_event_loop_dispatch(wl_display_get_event_loop(compositor.display), 100);

	assert(compositor.client != NULL);

	check_client_list(&compositor);



	wl_display_disconnect(client.display);
	cleanup_compositor(&compositor);
}

struct resource_listener {
	struct wl_listener listener;
	int count;
};

static void
resource_created(struct wl_listener *listener, void *data)
{
	struct resource_listener *l;
	l = wl_container_of(listener, l, listener);
	l->count++;
}

TEST(new_resource)
{
	const char *socket;
	struct compositor compositor = { 0 };
	struct {
		struct wl_display *display;
		struct wl_callback *cb;
	} client;
	struct resource_listener resource_listener;

	socket = setup_compositor(&compositor);
	client.display = wl_display_connect(socket);
	wl_event_loop_dispatch(wl_display_get_event_loop(compositor.display), 100);

	resource_listener.count = 0;
	resource_listener.listener.notify = resource_created;
	wl_client_add_resource_created_listener(compositor.client,
						&resource_listener.listener);

	client.cb = wl_display_sync(client.display);
	wl_display_flush(client.display);
	wl_event_loop_dispatch(wl_display_get_event_loop(compositor.display), 100);

	assert(resource_listener.count == 1);

	wl_callback_destroy(client.cb);
	wl_display_disconnect(client.display);
	cleanup_compositor(&compositor);

	/* This is defined to be safe also after client destruction */
	wl_list_remove(&resource_listener.listener.link);
}
