/*
 * Copyright © 2014 Jonas Ådahl
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

#include "wayland-client.h"
#include "wayland-private.h"
#include "wayland-server.h"
#include "test-runner.h"

TEST(message_version)
{
	unsigned int i;
	const struct {
		const struct wl_message *message;
		int expected_version;
	} messages[] = {
		{ &wl_pointer_interface.events[WL_POINTER_ENTER], 1 },
		{ &wl_surface_interface.events[WL_SURFACE_ENTER], 1 },
		{ &wl_pointer_interface.methods[WL_POINTER_SET_CURSOR], 1 },
		{ &wl_pointer_interface.methods[WL_POINTER_RELEASE], 3 },
		{ &wl_surface_interface.methods[WL_SURFACE_DESTROY], 1 },
		{ &wl_surface_interface.methods[WL_SURFACE_SET_BUFFER_TRANSFORM], 2 },
		{ &wl_surface_interface.methods[WL_SURFACE_SET_BUFFER_SCALE], 3 },
	};

	for (i = 0; i < ARRAY_LENGTH(messages); ++i) {
		assert(wl_message_get_since(messages[i].message) ==
		       messages[i].expected_version);
	}
}

TEST(message_count_arrays)
{
	unsigned int i;
	struct wl_message fake_messages[] = {
		{ "empty", "", NULL },
		{ "non_present", "iufsonh", NULL },
		{ "leading", "aiufsonh", NULL},
		{ "trailing", "iufsonha", NULL },
		{ "middle", "iufasonh", NULL },
		{ "multiple", "aaiufaasonhaa", NULL },
		{ "leading_version", "2aaiufaasonhaa", NULL },
		{ "among_nullables", "iufsa?oa?sah", NULL },
		{ "all_mixed", "2aiufas?oa?sa", NULL },
	};
	const struct {
		const struct wl_message *message;
		int expected_array_count;
	} messages[] = {
		{ &wl_pointer_interface.events[WL_POINTER_ENTER], 0 },
		{ &wl_keyboard_interface.events[WL_KEYBOARD_ENTER], 1 },
		{ &fake_messages[0], 0 },
		{ &fake_messages[1], 0 },
		{ &fake_messages[2], 1 },
		{ &fake_messages[3], 1 },
		{ &fake_messages[4], 1 },
		{ &fake_messages[5], 6 },
		{ &fake_messages[6], 6 },
		{ &fake_messages[7], 3 },
		{ &fake_messages[8], 4 },
	};

	for (i = 0; i < ARRAY_LENGTH(messages); ++i) {
		assert(wl_message_count_arrays(messages[i].message) ==
		       messages[i].expected_array_count);
	}
}
