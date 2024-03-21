/*
 * Copyright © 2013 Marek Chalupa
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
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>

#include "wayland-server.h"
#include "test-runner.h"

TEST(create_resource_tst)
{
	struct wl_display *display;
	struct wl_client *client;
	struct wl_resource *res;
	struct wl_list *link;
	int s[2];
	uint32_t id;

	assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, s) == 0);
	display = wl_display_create();
	assert(display);
	client = wl_client_create(display, s[0]);
	assert(client);

	res = wl_resource_create(client, &wl_seat_interface, 4, 0);
	assert(res);

	/* setters/getters */
	assert(wl_resource_get_version(res) == 4);

	assert(client == wl_resource_get_client(res));
	id = wl_resource_get_id(res);
	assert(wl_client_get_object(client, id) == res);

	link = wl_resource_get_link(res);
	assert(link);
	assert(wl_resource_from_link(link) == res);

	wl_resource_set_user_data(res, (void *) 0xbee);
	assert(wl_resource_get_user_data(res) == (void *) 0xbee);

	wl_resource_destroy(res);
	wl_client_destroy(client);
	wl_display_destroy(display);
	close(s[1]);
}

static void
res_destroy_func(struct wl_resource *res)
{
	assert(res);

	_Bool *destr = wl_resource_get_user_data(res);
	*destr = 1;
}

static _Bool notify_called = 0;
static void
destroy_notify(struct wl_listener *l, void *data)
{
	assert(l && data);
	notify_called = 1;

	/* In real code it's common to free the structure holding the
	 * listener at this point, but not to remove it from the list.
	 *
	 * That's fine since this is a destruction notification and
	 * it's the last time this signal can fire.  We set these
	 * to NULL so we can check them later to ensure no write after
	 * "free" occurred.
	 */
	l->link.prev = NULL;
	l->link.next = NULL;
}

TEST(destroy_res_tst)
{
	struct wl_display *display;
	struct wl_client *client;
	struct wl_resource *res;
	int s[2];
	unsigned id;
	struct wl_list *link;

	_Bool destroyed = 0;
	struct wl_listener destroy_listener = {
		.notify = &destroy_notify
	};

	assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, s) == 0);
	display = wl_display_create();
	assert(display);
	client = wl_client_create(display, s[0]);
	assert(client);

	res = wl_resource_create(client, &wl_seat_interface, 4, 0);
	assert(res);
	wl_resource_set_implementation(res, NULL, &destroyed, res_destroy_func);
	wl_resource_add_destroy_listener(res, &destroy_listener);

	id = wl_resource_get_id(res);
	link = wl_resource_get_link(res);
	assert(link);

	wl_resource_destroy(res);
	assert(destroyed);
	assert(notify_called); /* check if signal was emitted */
	assert(wl_client_get_object(client, id) == NULL);
	assert(destroy_listener.link.prev == NULL);
	assert(destroy_listener.link.next == NULL);

	res = wl_resource_create(client, &wl_seat_interface, 2, 0);
	assert(res);
	destroyed = 0;
	notify_called = 0;
	wl_resource_set_destructor(res, res_destroy_func);
	wl_resource_set_user_data(res, &destroyed);
	wl_resource_add_destroy_listener(res, &destroy_listener);
	/* client should destroy the resource upon its destruction */
	wl_client_destroy(client);
	assert(destroyed);
	assert(notify_called);
	assert(destroy_listener.link.prev == NULL);
	assert(destroy_listener.link.next == NULL);

	wl_display_destroy(display);
	close(s[1]);
}

TEST(create_resource_with_same_id)
{
	struct wl_display *display;
	struct wl_client *client;
	struct wl_resource *res, *res2;
	int s[2];
	uint32_t id;

	assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, s) == 0);
	display = wl_display_create();
	assert(display);
	client = wl_client_create(display, s[0]);
	assert(client);

	res = wl_resource_create(client, &wl_seat_interface, 2, 0);
	assert(res);
	id = wl_resource_get_id(res);
	assert(wl_client_get_object(client, id) == res);

	/* this one should replace the old one */
	res2 = wl_resource_create(client, &wl_seat_interface, 1, id);
	assert(res2 != NULL);
	assert(wl_client_get_object(client, id) == res2);

	wl_resource_destroy(res2);
	wl_resource_destroy(res);

	wl_client_destroy(client);
	wl_display_destroy(display);
	close(s[1]);
}

static void
display_destroy_notify(struct wl_listener *l, void *data)
{
	l->link.prev = l->link.next = NULL;
}

TEST(free_without_remove)
{
	struct wl_display *display;
	struct wl_listener a, b;

	display = wl_display_create();
	a.notify = display_destroy_notify;
	b.notify = display_destroy_notify;

	wl_display_add_destroy_listener(display, &a);
	wl_display_add_destroy_listener(display, &b);

	wl_display_destroy(display);

	assert(a.link.next == a.link.prev && a.link.next == NULL);
	assert(b.link.next == b.link.prev && b.link.next == NULL);
}
