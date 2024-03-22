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

#ifndef WAYLAND_SERVER_PRIVATE_H
#define WAYLAND_SERVER_PRIVATE_H

#include "wayland-server-core.h"

struct wl_priv_signal {
	struct wl_list listener_list;
	struct wl_list emit_list;
};

void
wl_priv_signal_init(struct wl_priv_signal *signal);

void
wl_priv_signal_add(struct wl_priv_signal *signal, struct wl_listener *listener);

struct wl_listener *
wl_priv_signal_get(struct wl_priv_signal *signal, wl_notify_func_t notify);

void
wl_priv_signal_emit(struct wl_priv_signal *signal, void *data);

void
wl_priv_signal_final_emit(struct wl_priv_signal *signal, void *data);

#endif
