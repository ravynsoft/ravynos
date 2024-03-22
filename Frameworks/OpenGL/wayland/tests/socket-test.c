/*
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
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>

#include "wayland-client.h"
#include "wayland-os.h"
#include "wayland-server.h"
#include "test-runner.h"

/* Paths longer than what the .sun_path array can contain must be rejected.
 * This is a hard limitation of assigning a name to AF_UNIX/AF_LOCAL sockets.
 * See `man 7 unix`.
 */

static struct sockaddr_un example_sockaddr_un;

#define TOO_LONG (1 + sizeof example_sockaddr_un.sun_path)

/* Ensure the connection doesn't fail due to lack of XDG_RUNTIME_DIR. */
static const char *
require_xdg_runtime_dir(void)
{
	char *val = getenv("XDG_RUNTIME_DIR");
	assert(val && val[0] == '/' && "set $XDG_RUNTIME_DIR to run this test");

	return val;
}

TEST(socket_path_overflow_client_connect)
{
	char path[TOO_LONG];
	struct wl_display *d;

	require_xdg_runtime_dir();

	memset(path, 'a', sizeof path);
	path[sizeof path - 1] = '\0';

	d = wl_display_connect(path);
	assert(d == NULL);
	assert(errno == ENAMETOOLONG);

	/* This is useless, but prevents a warning about example_sockaddr_un
	 * being discarded from the compilation unit. */
	strcpy(example_sockaddr_un.sun_path, "happy now clang?");
	assert(example_sockaddr_un.sun_path[0] != '\0');
}

TEST(socket_path_overflow_server_create)
{
	char path[TOO_LONG];
	struct wl_display *d;
	int ret;

	require_xdg_runtime_dir();

	memset(path, 'a', sizeof path);
	path[sizeof path - 1] = '\0';

	d = wl_display_create();
	assert(d != NULL);

	ret = wl_display_add_socket(d, path);
	assert(ret < 0);
	assert(errno == ENAMETOOLONG);

	wl_display_destroy(d);
}

TEST(add_existing_socket)
{
	char path[sizeof example_sockaddr_un.sun_path];
	const char *name = "wayland-test-0";
	const char *xdg_runtime_dir;
	struct wl_display *d;
	int ret;
	size_t len;

	xdg_runtime_dir = require_xdg_runtime_dir();

	d = wl_display_create();
	assert(d != NULL);

	/* this one should be OK */
	ret = wl_display_add_socket(d, name);
	assert(ret == 0);

	/* this one should fail */
	ret = wl_display_add_socket(d, name);
	assert(ret < 0);

	/* the original socket should still exist.
	 * this was a bug introduced in e2c0d47b0c77f18cd90e9c6eabb358c4d89681c8 */
	len = snprintf(path, sizeof example_sockaddr_un.sun_path, "%s/%s",
		       xdg_runtime_dir, name);
	assert(len < sizeof example_sockaddr_un.sun_path
	       && "Bug in test. Path too long");

	assert(access(path, F_OK) != -1);

	/* the original socket should still exist */
	ret = wl_display_add_socket(d, name);
	assert(ret < 0);

	wl_display_destroy(d);
}

TEST(add_socket_auto)
{
	/* the number of auto sockets is currently 32,
	 * set in wayland-server.c.
	 */
	const int MAX_SOCKETS = 32;

	char path[sizeof example_sockaddr_un.sun_path];
	const char *name;
	const char *xdg_runtime_dir;
	struct wl_display *d;
	int i;
	size_t len;

	xdg_runtime_dir = require_xdg_runtime_dir();

	d = wl_display_create();
	assert(d != NULL);

	for (i = 0; i <= MAX_SOCKETS; ++i) {
		name = wl_display_add_socket_auto(d);
		assert(name != NULL);

		len = snprintf(path, sizeof example_sockaddr_un.sun_path,
			       "%s/%s", xdg_runtime_dir, name);
		assert(len < sizeof example_sockaddr_un.sun_path
		       && "Bug in test. Path too long");

		/* was the socket created correctly? */
		assert(access(path, F_OK) != -1);

		/* is the name sequential? */
		len = snprintf(path, sizeof example_sockaddr_un.sun_path,
			       "wayland-%d", i);
		assert(strcmp(name, path) == 0);
	}

	/* next addition should return NULL */
	name = wl_display_add_socket_auto(d);
	assert(name == NULL);

	/* check if the socket was not deleted the last time */
	name = wl_display_add_socket_auto(d);
	assert(name == NULL);

	wl_display_destroy(d);
}

struct client_create_listener {
	struct wl_listener listener;
	struct wl_display *display;
};

struct client_destroy_listener {
	struct wl_listener listener;
	struct wl_display *display;
};

static void
client_destroy_notify(struct wl_listener *l, void *data)
{
	struct client_destroy_listener *listener =
		wl_container_of(l, listener, listener);
	wl_display_terminate(listener->display);
	free(listener);
}

static void
client_create_notify(struct wl_listener *l, void *data)
{
	struct wl_client *client = data;
	struct client_create_listener *listener =
		wl_container_of(l, listener, listener);
	struct client_destroy_listener *destroy_listener = (struct client_destroy_listener *)malloc(sizeof *destroy_listener);
	assert(destroy_listener != NULL);
	destroy_listener->display = listener->display;
	destroy_listener->listener.notify = client_destroy_notify;
	wl_client_add_destroy_listener(client, &destroy_listener->listener);
}

TEST(absolute_socket_path)
{
	struct wl_display *display;
	struct client_create_listener client_create_listener;
	struct sockaddr_un addr;
	int fd;
	socklen_t size;
	const char *xdg_runtime_dir;
	size_t len;
	int ret;
	pid_t pid;

	/* It's a little weird that this test about absolute socket paths
	 * uses XDG_RUNTIME_DIR, but that's the only location guaranteed
	 * by test-runner to be both writable and unique. This isn't
	 * really a problem; we'll just take care that the leaf-level
	 * filename used for the socket isn't anything that would
	 * accidentally be generated by a default usage of wl_display_connect(). */
	xdg_runtime_dir = require_xdg_runtime_dir();
	memset(&addr, 0, sizeof addr);
	len = snprintf(addr.sun_path, sizeof addr.sun_path,
		       "%s/%s", xdg_runtime_dir, "wayland-absolute-0");
	assert(len < sizeof addr.sun_path
	       && "Bug in test. Path too long");

	/* The path must not exist prior to binding. */
	assert(access(addr.sun_path, F_OK) == -1);

	size = offsetof (struct sockaddr_un, sun_path) + strlen(addr.sun_path);
	addr.sun_family = AF_LOCAL;
	fd = wl_os_socket_cloexec(PF_LOCAL, SOCK_STREAM, 0);
	assert(fd >= 0 );
	ret = bind(fd, (struct sockaddr *) &addr, size);
	assert(ret >= 0);
	ret = listen(fd, 128);
	assert(ret >= 0);

	/* Start server display. Be careful (by avoiding wl_display_add_socket_auto()
	 * to offer only the absolutely qualified socket made above. */
	display = wl_display_create();
	assert(display != NULL);
	client_create_listener.listener.notify = client_create_notify;
	client_create_listener.display = display;
	wl_display_add_client_created_listener(display, &client_create_listener.listener);
	ret = wl_display_add_socket_fd(display, fd);
	assert(ret == 0);

	/* Execute client that connects to the absolutely qualified server socket path. */
	pid = fork();
	assert(pid != -1);

	if (pid == 0) {
		ret = setenv("WAYLAND_DISPLAY", addr.sun_path, 1);
		assert(ret == 0);
		struct wl_display *client_display = wl_display_connect(NULL);
		assert(client_display != NULL);
		ret = wl_display_roundtrip(client_display);
		assert(ret != -1);
		wl_display_disconnect(client_display);
		exit(0);
		assert(false);
	}

	wl_display_run(display);
	ret = waitpid(pid, NULL, 0);
	assert(ret == pid);

	wl_display_destroy(display);

	ret = unlink(addr.sun_path);
	assert(ret == 0);
}
