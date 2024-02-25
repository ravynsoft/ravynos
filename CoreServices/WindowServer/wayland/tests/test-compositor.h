/*
 * Copyright (c) 2014 Red Hat, Inc.
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

#include <stdint.h>
#include <unistd.h>
#include <stdatomic.h>

#include "wayland-server.h"
#include "wayland-client.h"

/* info about a client on server side */
struct client_info {
	struct display *display;
	struct wl_client *wl_client;
	struct wl_listener destroy_listener;
	const char *name; /* for debugging */

	int pipe;
	pid_t pid;
	int exit_code;
	int kill_code;

	struct wl_list link;
	void *data; /* for arbitrary use */
	int log_fd;
};

struct display {
	struct wl_display *wl_display;
	struct wl_global *test_global;

	struct wl_list clients;
	uint32_t clients_no;
	uint32_t clients_terminated_no;

	/* list of clients waiting for display_resumed event */
	struct wl_list waiting_for_resume;
	uint32_t wfr_num;
};

/* This is a helper structure for clients.
 * Instead of calling wl_display_connect() and all the other stuff,
 * client can use client_connect and it will return this structure
 * filled. */
struct client {
	struct wl_display *wl_display;
	struct test_compositor *tc;

	atomic_bool display_stopped;
};

struct client *client_connect(void);
void client_disconnect(struct client *);
int stop_display(struct client *, int);
void noop_request(struct client *);

/**
 * Usual workflow:
 *
 *    d = display_create();
 *
 *    wl_global_create(d->wl_display, ...);
 *    ... other setups ...
 *
 *    client_create(d, client_main, data);
 *    client_create(d, client_main2, data);
 *
 *    display_run(d);
 *    display_destroy(d);
 */
struct display *display_create(void);
void display_destroy(struct display *d);
void display_destroy_expect_signal(struct display *d, int signum);
void display_run(struct display *d);

/* This function posts the display_resumed event to all waiting clients,
 * so that after flushing events the clients will stop waiting and continue.
 *
 * (Calling `display_run` after this function will resume the display loop.)
 */
void display_post_resume_events(struct display *d);
/* After n clients called stop_display(..., n), the display
 * is stopped and can process the code after display_run().
 *
 * This function posts the display_resumed event to the waiting
 * clients, so that the clients will stop waiting and continue;
 * it then reruns the display. */
void display_resume(struct display *d);

/* The file descriptor containing the client log. This is only valid in the
 * test client processes. */
extern int client_log_fd;

struct client_info *client_create_with_name(struct display *d,
					    void (*client_main)(void *data),
					    void *data,
					    const char *name);
#define client_create(d, c, data) client_create_with_name((d), (c), data, (#c))
#define client_create_noarg(d, c) \
	client_create_with_name((d), (void(*)(void *)) (c), NULL, (#c))
