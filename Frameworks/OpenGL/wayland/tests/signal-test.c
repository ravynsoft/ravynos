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

#include "wayland-server.h"
#include "test-runner.h"

static void
signal_notify(struct wl_listener *listener, void *data)
{
	/* only increase counter*/
	++(*((int *) data));
}

TEST(signal_init)
{
	struct wl_signal signal;

	wl_signal_init(&signal);

	/* Test if listeners' list is initialized */
	assert(&signal.listener_list == signal.listener_list.next
		&& "Maybe wl_signal implementation changed?");
	assert(signal.listener_list.next == signal.listener_list.prev
		&& "Maybe wl_signal implementation changed?");
}

TEST(signal_add_get)
{
	struct wl_signal signal;

	/* we just need different values of notify */
	struct wl_listener l1 = {.notify = (wl_notify_func_t) 0x1};
	struct wl_listener l2 = {.notify = (wl_notify_func_t) 0x2};
	struct wl_listener l3 = {.notify = (wl_notify_func_t) 0x3};
	/* one real, why not */
	struct wl_listener l4 = {.notify = signal_notify};

	wl_signal_init(&signal);

	wl_signal_add(&signal, &l1);
	wl_signal_add(&signal, &l2);
	wl_signal_add(&signal, &l3);
	wl_signal_add(&signal, &l4);

	assert(wl_signal_get(&signal, signal_notify) == &l4);
	assert(wl_signal_get(&signal, (wl_notify_func_t) 0x3) == &l3);
	assert(wl_signal_get(&signal, (wl_notify_func_t) 0x2) == &l2);
	assert(wl_signal_get(&signal, (wl_notify_func_t) 0x1) == &l1);

	/* get should not be destructive */
	assert(wl_signal_get(&signal, signal_notify) == &l4);
	assert(wl_signal_get(&signal, (wl_notify_func_t) 0x3) == &l3);
	assert(wl_signal_get(&signal, (wl_notify_func_t) 0x2) == &l2);
	assert(wl_signal_get(&signal, (wl_notify_func_t) 0x1) == &l1);
}

TEST(signal_emit_to_one_listener)
{
	int count = 0;
	int counter;

	struct wl_signal signal;
	struct wl_listener l1 = {.notify = signal_notify};

	wl_signal_init(&signal);
	wl_signal_add(&signal, &l1);

	for (counter = 0; counter < 100; counter++)
		wl_signal_emit(&signal, &count);

	assert(counter == count);
}

TEST(signal_emit_to_more_listeners)
{
	int count = 0;
	int counter;

	struct wl_signal signal;
	struct wl_listener l1 = {.notify = signal_notify};
	struct wl_listener l2 = {.notify = signal_notify};
	struct wl_listener l3 = {.notify = signal_notify};

	wl_signal_init(&signal);
	wl_signal_add(&signal, &l1);
	wl_signal_add(&signal, &l2);
	wl_signal_add(&signal, &l3);

	for (counter = 0; counter < 100; counter++)
		wl_signal_emit(&signal, &count);

	assert(3 * counter == count);
}

struct signal_emit_mutable_data {
	int count;
	struct wl_listener *remove_listener;
};

static void
signal_notify_mutable(struct wl_listener *listener, void *data)
{
	struct signal_emit_mutable_data *test_data = data;
	test_data->count++;
}

static void
signal_notify_and_remove_mutable(struct wl_listener *listener, void *data)
{
	struct signal_emit_mutable_data *test_data = data;
	signal_notify_mutable(listener, test_data);
	wl_list_remove(&test_data->remove_listener->link);
}

TEST(signal_emit_mutable)
{
	struct signal_emit_mutable_data data = {0};

	/* l2 will remove l3 before l3 is notified */
	struct wl_signal signal;
	struct wl_listener l1 = {.notify = signal_notify_mutable};
	struct wl_listener l2 = {.notify = signal_notify_and_remove_mutable};
	struct wl_listener l3 = {.notify = signal_notify_mutable};

	wl_signal_init(&signal);
	wl_signal_add(&signal, &l1);
	wl_signal_add(&signal, &l2);
	wl_signal_add(&signal, &l3);

	data.remove_listener = &l3;
	wl_signal_emit_mutable(&signal, &data);

	assert(data.count == 2);
}
