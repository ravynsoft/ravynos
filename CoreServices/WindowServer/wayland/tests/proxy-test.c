/*
 * Copyright (c) 2019 Red Hat, Inc.
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

#include <assert.h>
#include <string.h>

#include "wayland-server.h"
#include "wayland-client.h"
#include "test-runner.h"

static struct {
	struct wl_display *display;
	struct wl_event_loop *loop;
	int sync_count;
} server;

static struct {
	struct wl_display *display;
	struct wl_callback *callback_a;
	struct wl_callback *callback_b;
	int callback_count;
} client;

static const char *tag_a = "tag";
static const char *tag_b = "tag";

static void
callback_done(void *data, struct wl_callback *cb, uint32_t time)
{
	const char * const *expected_tag;
	const char * const *tag;

	if (cb == client.callback_a)
		expected_tag = &tag_a;
	else if (cb == client.callback_b)
		expected_tag = &tag_b;
	else
		assert(!"unexpected callback");

	tag = wl_proxy_get_tag((struct wl_proxy *) cb);

	assert(tag == expected_tag);
	assert(strcmp(*tag, "tag") == 0);

	wl_callback_destroy(cb);

	client.callback_count++;
}

static const struct wl_callback_listener callback_listener = {
	callback_done,
};

static void
logger_func(void *user_data,
	    enum wl_protocol_logger_type type,
	    const struct wl_protocol_logger_message *message)
{
	if (type != WL_PROTOCOL_LOGGER_REQUEST)
		return;

	assert(strcmp(wl_resource_get_class(message->resource),
		      "wl_display") == 0);
	assert(strcmp(message->message->name, "sync") == 0);

	server.sync_count++;
}

TEST(proxy_tag)
{
	const char *socket;
	struct wl_protocol_logger *logger;

	assert(&tag_a != &tag_b);

	server.display = wl_display_create();
	assert(server.display);
	server.loop = wl_display_get_event_loop(server.display);
	assert(server.loop);
	socket = wl_display_add_socket_auto(server.display);
	assert(socket);
	logger = wl_display_add_protocol_logger(server.display,
						logger_func, NULL);
	assert(logger);

	client.display = wl_display_connect(socket);
	assert(client.display);

	client.callback_a = wl_display_sync(client.display);
	wl_callback_add_listener(client.callback_a, &callback_listener, NULL);
	wl_proxy_set_tag((struct wl_proxy *) client.callback_a,
			 &tag_a);

	client.callback_b = wl_display_sync(client.display);
	wl_callback_add_listener(client.callback_b, &callback_listener, NULL);
	wl_proxy_set_tag((struct wl_proxy *) client.callback_b,
			 &tag_b);

	wl_display_flush(client.display);

	while (server.sync_count < 2) {
		wl_event_loop_dispatch(server.loop, -1);
		wl_display_flush_clients(server.display);
	}

	wl_display_dispatch(client.display);

	assert(client.callback_count == 2);

	wl_protocol_logger_destroy(logger);
	wl_display_disconnect(client.display);
	wl_event_loop_dispatch(server.loop, 100);

	wl_display_destroy(server.display);
}
