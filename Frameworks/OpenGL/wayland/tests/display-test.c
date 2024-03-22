/*
 * Copyright © 2012 Intel Corporation
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

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <pthread.h>
#include <poll.h>

#include "wayland-private.h"
#include "wayland-server.h"
#include "wayland-client.h"
#include "test-runner.h"
#include "test-compositor.h"

#include "tests-server-protocol.h"
#include "tests-client-protocol.h"

struct display_destroy_listener {
	struct wl_listener listener;
	int done;
};

static void
display_destroy_notify(struct wl_listener *l, void *data)
{
	struct display_destroy_listener *listener;

	listener = wl_container_of(l, listener, listener);
	listener->done = 1;
}

TEST(display_destroy_listener)
{
	struct wl_display *display;
	struct display_destroy_listener a, b;

	display = wl_display_create();
	assert(display);

	a.listener.notify = &display_destroy_notify;
	a.done = 0;
	wl_display_add_destroy_listener(display, &a.listener);

	assert(wl_display_get_destroy_listener(display, display_destroy_notify) ==
	       &a.listener);

	b.listener.notify = display_destroy_notify;
	b.done = 0;
	wl_display_add_destroy_listener(display, &b.listener);

	wl_list_remove(&a.listener.link);

	wl_display_destroy(display);

	assert(!a.done);
	assert(b.done);
}

/* Fake 'client' which does not use wl_display_connect, and thus leaves the
 * file descriptor passed through WAYLAND_SOCKET intact. This should not
 * trigger an assertion in the leak check. */
static void
empty_client(void)
{
	return;
}

TEST(tc_leaks_tests)
{
	struct display *d = display_create();
	client_create_noarg(d, empty_client);
	display_run(d);
	display_destroy(d);
}

/* This is how pre proxy-version registry binds worked,
 * this should create a proxy that shares the display's
 * version number: 0 */
static void *
old_registry_bind(struct wl_registry *wl_registry,
		  uint32_t name,
		  const struct wl_interface *interface,
		  uint32_t version)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor(
		(struct wl_proxy *) wl_registry, WL_REGISTRY_BIND,
		interface, name, interface->name, version, NULL);

	return (void *) id;
}

struct handler_info {
	struct wl_seat *seat;
	uint32_t bind_version;
	bool use_unversioned;
};

static void
registry_handle_globals(void *data, struct wl_registry *registry,
			uint32_t id, const char *intf, uint32_t ver)
{
	struct handler_info *hi = data;

	/* This is only for the proxy version test */
	if (hi->bind_version)
		ver = hi->bind_version;

	if (strcmp(intf, "wl_seat") == 0) {
		if (hi->use_unversioned)
			hi->seat = old_registry_bind(registry, id,
						     &wl_seat_interface, ver);
		else
			hi->seat = wl_registry_bind(registry, id,
						    &wl_seat_interface, ver);
		assert(hi->seat);
	}
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_globals,
	NULL
};

static struct wl_seat *
client_get_seat_with_info(struct client *c, struct handler_info *hi)
{
	struct wl_registry *reg = wl_display_get_registry(c->wl_display);
	assert(reg);

	assert(hi);
	hi->seat = NULL;
	wl_registry_add_listener(reg, &registry_listener, hi);
	wl_display_roundtrip(c->wl_display);
	assert(hi->seat);

	wl_registry_destroy(reg);

	return hi->seat;
}

static struct wl_seat *
client_get_seat(struct client *c)
{
	struct handler_info hi;

	hi.use_unversioned = false;
	hi.bind_version = 0;

	return client_get_seat_with_info(c, &hi);
}

static void
check_pending_error(struct client *c, struct wl_proxy *proxy)
{
	uint32_t ec, id;
	int err;
	const struct wl_interface *intf;

	err = wl_display_get_error(c->wl_display);
	assert(err == EPROTO);

	ec = wl_display_get_protocol_error(c->wl_display, &intf, &id);
	assert(ec == 23);
	assert(intf == &wl_seat_interface);
	assert(id == wl_proxy_get_id(proxy));
}

static void
check_for_error(struct client *c, struct wl_proxy *proxy)
{
	/* client should be disconnected */
	assert(wl_display_roundtrip(c->wl_display) == -1);

	check_pending_error(c, proxy);
}

static struct client_info *
find_client_info(struct display *d, struct wl_client *client)
{
	struct client_info *ci;

	wl_list_for_each(ci, &d->clients, link) {
		if (ci->wl_client == client)
			return ci;
	}

	return NULL;
}

static void
bind_seat(struct wl_client *client, void *data,
	  uint32_t vers, uint32_t id)
{
	struct display *d = data;
	struct client_info *ci;
	struct wl_resource *res;

	ci = find_client_info(d, client);
	assert(ci);

	res = wl_resource_create(client, &wl_seat_interface, vers, id);
	assert(res);

	/* save the resource as client's info data,
	 * so that we can use it later */
	ci->data = res;
}

static void
client_disconnect_nocheck(struct client *c)
{
	wl_proxy_destroy((struct wl_proxy *) c->tc);
	wl_display_disconnect(c->wl_display);
	free(c);
}

static void
post_error_main(void)
{
	struct client *c = client_connect();
	struct wl_seat *seat = client_get_seat(c);

	/* stop display so that it can post the error.
	 * The function should return -1, because of the posted error */
	assert(stop_display(c, 1) == -1);

	/* display should have posted error, check it! */
	check_for_error(c, (struct wl_proxy *) seat);

	/* don't call client_disconnect(c), because then the test would be
	 * aborted due to checks for error in this function */
	wl_proxy_destroy((struct wl_proxy *) seat);
	client_disconnect_nocheck(c);
}

TEST(post_error_to_one_client)
{
	struct display *d = display_create();
	struct client_info *cl;

	wl_global_create(d->wl_display, &wl_seat_interface,
			 1, d, bind_seat);

	cl = client_create_noarg(d, post_error_main);
	display_run(d);

	/* the display was stopped by client, so it can
	 * proceed in the code and post an error */
	assert(cl->data);
	wl_resource_post_error((struct wl_resource *) cl->data,
			       23, "Dummy error");

	/* this one should be ignored */
	wl_resource_post_error((struct wl_resource *) cl->data,
			       21, "Dummy error (ignore)");

	display_resume(d);
	display_destroy(d);
}

static void
post_error_main2(void)
{
	struct client *c = client_connect();
	struct wl_seat *seat = client_get_seat(c);

	/* the error should not be posted for this client */
	assert(stop_display(c, 2) >= 0);

	wl_proxy_destroy((struct wl_proxy *) seat);
	client_disconnect(c);
}

static void
post_error_main3(void)
{
	struct client *c = client_connect();
	struct wl_seat *seat = client_get_seat(c);

	assert(stop_display(c, 2) == -1);
	check_for_error(c, (struct wl_proxy *) seat);

	/* don't call client_disconnect(c), because then the test would be
	 * aborted due to checks for error in this function */
	wl_proxy_destroy((struct wl_proxy *) seat);
	client_disconnect_nocheck(c);
}

/* all the testcases could be in one TEST, but splitting it
 * apart is better for debugging when the test fails */
TEST(post_error_to_one_from_two_clients)
{
	struct display *d = display_create();
	struct client_info *cl;

	wl_global_create(d->wl_display, &wl_seat_interface,
			 1, d, bind_seat);

	client_create_noarg(d, post_error_main2);
	cl = client_create_noarg(d, post_error_main3);
	display_run(d);

	/* post error only to the second client */
	assert(cl->data);
	wl_resource_post_error((struct wl_resource *) cl->data,
			       23, "Dummy error");
	wl_resource_post_error((struct wl_resource *) cl->data,
			       21, "Dummy error (ignore)");

	display_resume(d);
	display_destroy(d);
}

/* all the testcases could be in one TEST, but splitting it
 * apart is better for debugging when the test fails */
TEST(post_error_to_two_clients)
{
	struct display *d = display_create();
	struct client_info *cl, *cl2;

	wl_global_create(d->wl_display, &wl_seat_interface,
			 1, d, bind_seat);

	cl = client_create_noarg(d, post_error_main3);
	cl2 = client_create_noarg(d, post_error_main3);

	display_run(d);

	/* Try to send the error to both clients */
	assert(cl->data && cl2->data);
	wl_resource_post_error((struct wl_resource *) cl->data,
			       23, "Dummy error");
	wl_resource_post_error((struct wl_resource *) cl->data,
			       21, "Dummy error (ignore)");

	wl_resource_post_error((struct wl_resource *) cl2->data,
			       23, "Dummy error");
	wl_resource_post_error((struct wl_resource *) cl2->data,
			       21, "Dummy error (ignore)");

	display_resume(d);
	display_destroy(d);
}

static void
post_nomem_main(void)
{
	struct client *c = client_connect();
	struct wl_seat *seat = client_get_seat(c);

	assert(stop_display(c, 1) == -1);
	assert(wl_display_get_error(c->wl_display) == ENOMEM);

	wl_proxy_destroy((struct wl_proxy *) seat);
	client_disconnect_nocheck(c);
}

TEST(post_nomem_tst)
{
	struct display *d = display_create();
	struct client_info *cl;

	wl_global_create(d->wl_display, &wl_seat_interface,
			 1, d, bind_seat);

	cl = client_create_noarg(d, post_nomem_main);
	display_run(d);

	assert(cl->data);
	wl_resource_post_no_memory((struct wl_resource *) cl->data);
	display_resume(d);

	/* first client terminated. Run it again,
	 * but post no memory to client */
	cl = client_create_noarg(d, post_nomem_main);
	display_run(d);

	assert(cl->data);
	wl_client_post_no_memory(cl->wl_client);
	display_resume(d);

	display_destroy(d);
}

static void
post_implementation_error_main(void)
{
	struct client *c = client_connect();
	struct wl_seat *seat = client_get_seat(c);
	uint32_t object_id, protocol_error;
	const struct wl_interface *interface;

	assert(stop_display(c, 1) == -1);
	int err = wl_display_get_error(c->wl_display);
	fprintf(stderr, "Err is %i\n", err);
	assert(err == EPROTO);
	protocol_error = wl_display_get_protocol_error(c->wl_display,
						       &interface,
						       &object_id);
	assert(protocol_error == WL_DISPLAY_ERROR_IMPLEMENTATION);
	assert(interface == &wl_display_interface);

	wl_proxy_destroy((struct wl_proxy *) seat);
	client_disconnect_nocheck(c);
}

TEST(post_internal_error_tst)
{
	struct display *d = display_create();
	struct client_info *cl;

	wl_global_create(d->wl_display, &wl_seat_interface,
			 1, d, bind_seat);

	cl = client_create_noarg(d, post_implementation_error_main);
	display_run(d);

	wl_client_post_implementation_error(cl->wl_client, "Error %i", 20);

	display_resume(d);

	display_destroy(d);
}

static void
register_reading(struct wl_display *display)
{
	while(wl_display_prepare_read(display) != 0 && errno == EAGAIN)
		assert(wl_display_dispatch_pending(display) >= 0);
	assert(wl_display_flush(display) >= 0);
}

/* create thread that will call prepare+read so that
 * it will block */
static pthread_t
create_thread(struct client *c, void *(*func)(void*))
{
	pthread_t thread;

	c->display_stopped = 0;
	/* func must set display->stopped to 1 before sleeping */
	assert(pthread_create(&thread, NULL, func, c) == 0);

	/* make sure the thread is sleeping. It's a little bit racy
	 * (setting display_stopped to 1 and calling wl_display_read_events)
	 * so call usleep once again after the loop ends - it should
	 * be sufficient... */
	while (c->display_stopped == 0)
		test_usleep(500);
	test_usleep(10000);

	return thread;
}

static void *
thread_read_error(void *data)
{
	struct client *c = data;

	register_reading(c->wl_display);

	/*
	 * Calling the read right now will block this thread
	 * until the other thread will read the data.
	 * However, after invoking an error, this
	 * thread should be woken up or it will block indefinitely.
	 */
	c->display_stopped = 1;
	assert(wl_display_read_events(c->wl_display) == -1);

	assert(wl_display_dispatch_pending(c->wl_display) == -1);
	assert(wl_display_get_error(c->wl_display));

	pthread_exit(NULL);
}

/* test posting an error in multi-threaded environment. */
static void
threading_post_err(void)
{
	DISABLE_LEAK_CHECKS;

	struct client *c = client_connect();
	pthread_t thread;

	/* register read intention */
	register_reading(c->wl_display);

	/* use this var as an indicator that thread is sleeping */
	c->display_stopped = 0;

	/* create new thread that will register its intention too */
	thread = create_thread(c, thread_read_error);

	/* so now we have sleeping thread waiting for a pthread_cond signal.
	 * The main thread must call wl_display_read_events().
	 * If this call fails, then it won't call broadcast at the
	 * end of the function and the sleeping thread will block indefinitely.
	 * Make the call fail and watch if libwayland will unblock the thread! */

	/* create error on fd, so that wl_display_read_events will fail.
	 * The same can happen when server hangs up */
	close(wl_display_get_fd(c->wl_display));
	/* this read events will fail and will
	 * post an error that should wake the sleeping thread
	 * and dispatch the incoming events */
	assert(wl_display_read_events(c->wl_display) == -1);

	/* kill test in 3 seconds. This should be enough time for the
	 * thread to exit if it's not blocking. If everything is OK, than
	 * the thread was woken up and the test will end before the SIGALRM */
	test_set_timeout(3);
	pthread_join(thread, NULL);

	client_disconnect_nocheck(c);
}

TEST(threading_errors_tst)
{
	struct display *d = display_create();

	client_create_noarg(d, threading_post_err);
	display_run(d);

	display_destroy(d);
}

static void *
thread_prepare_and_read(void *data)
{
	struct client *c = data;

	register_reading(c->wl_display);

	c->display_stopped = 1;

	assert(wl_display_read_events(c->wl_display) == 0);
	assert(wl_display_dispatch_pending(c->wl_display) == 0);

	pthread_exit(NULL);
}

/* test cancel read*/
static void
threading_cancel_read(void)
{
	DISABLE_LEAK_CHECKS;

	struct client *c = client_connect();
	pthread_t th1, th2, th3;

	register_reading(c->wl_display);

	th1 = create_thread(c, thread_prepare_and_read);
	th2 = create_thread(c, thread_prepare_and_read);
	th3 = create_thread(c, thread_prepare_and_read);

	/* all the threads are sleeping, waiting until read or cancel
	 * is called. Cancel the read and let the threads proceed */
	wl_display_cancel_read(c->wl_display);

	/* kill test in 3 seconds. This should be enough time for the
	 * thread to exit if it's not blocking. If everything is OK, than
	 * the thread was woken up and the test will end before the SIGALRM */
	test_set_timeout(3);
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);

	client_disconnect(c);
}

TEST(threading_cancel_read_tst)
{
	struct display *d = display_create();

	client_create_noarg(d, threading_cancel_read);
	display_run(d);

	display_destroy(d);
}

static void
threading_read_eagain(void)
{
	DISABLE_LEAK_CHECKS;

	struct client *c = client_connect();
	pthread_t th1, th2, th3;

	register_reading(c->wl_display);

	th1 = create_thread(c, thread_prepare_and_read);
	th2 = create_thread(c, thread_prepare_and_read);
	th3 = create_thread(c, thread_prepare_and_read);

	/* All the threads are sleeping, waiting until read or cancel
	 * is called. Since we have no data on socket waiting,
	 * the wl_connection_read should end up with error and set errno
	 * to EAGAIN. Check if the threads are woken up in this case. */
	assert(wl_display_read_events(c->wl_display) == 0);
	/* errno should be still set to EAGAIN if wl_connection_read
	 * set it - check if we're testing the right case */
	assert(errno == EAGAIN);

	test_set_timeout(3);
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);

	client_disconnect(c);
}

TEST(threading_read_eagain_tst)
{
	struct display *d = display_create();
	client_create_noarg(d, threading_read_eagain);

	display_run(d);

	display_destroy(d);
}

static void *
thread_prepare_and_read2(void *data)
{
	struct client *c = data;

	while(wl_display_prepare_read(c->wl_display) != 0 && errno == EAGAIN)
		assert(wl_display_dispatch_pending(c->wl_display) == -1);
	assert(wl_display_flush(c->wl_display) == -1);

	c->display_stopped = 1;

	assert(wl_display_read_events(c->wl_display) == -1);
	assert(wl_display_dispatch_pending(c->wl_display) == -1);

	pthread_exit(NULL);
}

static void
threading_read_after_error(void)
{
	DISABLE_LEAK_CHECKS;

	struct client *c = client_connect();
	pthread_t thread;

	/* create an error */
	close(wl_display_get_fd(c->wl_display));
	assert(wl_display_dispatch(c->wl_display) == -1);

	/* try to prepare for reading */
	while(wl_display_prepare_read(c->wl_display) != 0 && errno == EAGAIN)
		assert(wl_display_dispatch_pending(c->wl_display) == -1);
	assert(wl_display_flush(c->wl_display) == -1);

	assert(pthread_create(&thread, NULL,
			      thread_prepare_and_read2, c) == 0);

	/* make sure thread is sleeping */
	while (c->display_stopped == 0)
		test_usleep(500);
	test_usleep(10000);

	assert(wl_display_read_events(c->wl_display) == -1);

	/* kill test in 3 seconds */
	test_set_timeout(3);
	pthread_join(thread, NULL);

	client_disconnect_nocheck(c);
}

TEST(threading_read_after_error_tst)
{
	struct display *d = display_create();

	client_create_noarg(d, threading_read_after_error);
	display_run(d);

	display_destroy(d);
}

static void
wait_for_error_using_dispatch(struct client *c, struct wl_proxy *proxy)
{
	int ret;

	while (true) {
		/* Dispatching should eventually hit the protocol error before
		 * any other error. */
		ret = wl_display_dispatch(c->wl_display);
		if (ret == 0) {
			continue;
		} else {
			assert(errno == EPROTO);
			break;
		}
	}

	check_pending_error(c, proxy);
}

static void
wait_for_error_using_prepare_read(struct client *c, struct wl_proxy *proxy)
{
	int ret = 0;
	struct pollfd pfd[2];

	while (true) {
		while (wl_display_prepare_read(c->wl_display) != 0 &&
		      errno == EAGAIN) {
			assert(wl_display_dispatch_pending(c->wl_display) >= 0);
		}

		/* Flush may fail due to EPIPE if the connection is broken, but
		 * this must not set a fatal display error because that would
		 * result in it being impossible to read a potential protocol
		 * error. */
		do {
			ret = wl_display_flush(c->wl_display);
		} while (ret == -1 && (errno == EINTR || errno == EAGAIN));
		assert(ret >= 0 || errno == EPIPE);
		assert(wl_display_get_error(c->wl_display) == 0);

		pfd[0].fd = wl_display_get_fd(c->wl_display);
		pfd[0].events = POLLIN;
		do {
			ret = poll(pfd, 1, -1);
		} while (ret == -1 && errno == EINTR);
		assert(ret != -1);

		/* We should always manage to read the error before the EPIPE
		 * comes this way. */
		assert(wl_display_read_events(c->wl_display) == 0);

		/* Dispatching should eventually hit the protocol error before
		 * any other error. */
		ret = wl_display_dispatch_pending(c->wl_display);
		if (ret == 0) {
			continue;
		} else {
			assert(errno == EPROTO);
			break;
		}
	}

	check_pending_error(c, proxy);
}

static void
check_error_after_epipe(void *data)
{
	bool use_dispatch_helpers = *(bool *) data;
	struct client *client;
	struct wl_seat *seat;
	struct wl_callback *callback;

	client = client_connect();

	/* This will, according to the implementation below, cause the server
	 * to post an error. */
	seat = client_get_seat(client);
	wl_display_flush(client->wl_display);

	/* The server will not actually destroy the client until it receives
	 * input, so send something to trigger the client destruction. */
	callback = wl_display_sync(client->wl_display);
	wl_callback_destroy(callback);

	/* Sleep some to give the server a chance to react and destroy the
	 * client. */
	test_usleep(200000);

	/* Wait for the protocol error and check that we reached it before
	 * EPIPE. */
	if (use_dispatch_helpers) {
		wait_for_error_using_dispatch(client, (struct wl_proxy *) seat);
	} else {
		wait_for_error_using_prepare_read(client,
						  (struct wl_proxy *) seat);
	}

	wl_seat_destroy(seat);
	client_disconnect_nocheck(client);
}

static void
bind_seat_and_post_error(struct wl_client *client, void *data,
			 uint32_t version, uint32_t id)
{
	struct display *d = data;
	struct client_info *ci;
	struct wl_resource *resource;

	ci = find_client_info(d, client);
	assert(ci);

	resource = wl_resource_create(client, &wl_seat_interface, version, id);
	assert(resource);
	ci->data = resource;

	wl_resource_post_error(ci->data, 23, "Dummy error");
}

TEST(error_code_after_epipe)
{
	struct display *d = display_create();
	bool use_dispatch_helpers;

	wl_global_create(d->wl_display, &wl_seat_interface,
			 1, d, bind_seat_and_post_error);

	use_dispatch_helpers = true;
	client_create(d, check_error_after_epipe, &use_dispatch_helpers);
	display_run(d);

	use_dispatch_helpers = false;
	client_create(d, check_error_after_epipe, &use_dispatch_helpers);
	display_run(d);

	display_destroy(d);
}

static void
check_seat_versions(struct wl_seat *seat, uint32_t ev)
{
	struct wl_pointer *pointer;

	assert(wl_proxy_get_version((struct wl_proxy *) seat) == ev);
	assert(wl_seat_get_version(seat) == ev);

	pointer = wl_seat_get_pointer(seat);
	assert(wl_pointer_get_version(pointer) == ev);
	assert(wl_proxy_get_version((struct wl_proxy *) pointer) == ev);
	wl_proxy_destroy((struct wl_proxy *) pointer);
}

/* Normal client with proxy versions available. */
static void
seat_version(void *data)
{
	struct handler_info *hi = data;
	struct client *c = client_connect();
	struct wl_seat *seat;

	/* display proxy should always be version 0 */
	assert(wl_proxy_get_version((struct wl_proxy *) c->wl_display) == 0);

	seat = client_get_seat_with_info(c, hi);
	if (hi->use_unversioned)
		check_seat_versions(seat, 0);
	else
		check_seat_versions(seat, hi->bind_version);

	wl_proxy_destroy((struct wl_proxy *) seat);

	client_disconnect_nocheck(c);
}

TEST(versions)
{
	struct display *d = display_create();
	struct wl_global *global;
	int i;

	global = wl_global_create(d->wl_display, &wl_seat_interface,
				  5, d, bind_seat);

	for (i = 1; i <= 5; i++) {
		struct handler_info hi;

		hi.bind_version = i;
		hi.use_unversioned = false;
		client_create(d, seat_version, &hi);
		hi.use_unversioned = true;
		client_create(d, seat_version, &hi);
	}

	display_run(d);

	wl_global_destroy(global);

	display_destroy(d);
}

static void
check_error_on_destroyed_object(void *data)
{
	struct client *c;
	struct wl_seat *seat;
	uint32_t id;
	const struct wl_interface *intf;

	c = client_connect();
	seat = client_get_seat(c);

	/* destroy the seat proxy. The display won't know
	 * about it yet, so it will post the error as usual */
	wl_proxy_destroy((struct wl_proxy *) seat);

	/* let display post the error. The error will
	 * be caught in stop_display while dispatching */
	assert(stop_display(c, 1) == -1);

	/* check the returned error. Since the object was destroyed,
	 * we don't know the interface and id */
	assert(wl_display_get_error(c->wl_display) == EPROTO);
	assert(wl_display_get_protocol_error(c->wl_display, &intf, &id) == 23);
	assert(intf == NULL);
	assert(id == 0);

	client_disconnect_nocheck(c);
}

TEST(error_on_destroyed_object)
{
	struct client_info *cl;
	struct display *d = display_create();

	wl_global_create(d->wl_display, &wl_seat_interface,
			 1, d, bind_seat);

	cl = client_create_noarg(d, check_error_on_destroyed_object);
	display_run(d);

	/* did client bind to the seat? */
	assert(cl->data);

	/* post error on the destroyed object */
	wl_resource_post_error((struct wl_resource *) cl->data,
			       23, "Dummy error");
	display_resume(d);
	display_destroy(d);
}

static bool
global_filter(const struct wl_client *client,
	      const struct wl_global *global,
	      void *data)
{
	/* Hide the wl_data_offer interface if no data was provided */
	if (wl_global_get_interface(global) == &wl_data_offer_interface)
		return data != NULL;

	/* Show all the others */
	return true;
}

static void
bind_data_offer(struct wl_client *client, void *data,
		uint32_t vers, uint32_t id)
{
	/* Client should not be able to bind to this interface! */
	assert(false);
}

static void
registry_handle_filtered(void *data, struct wl_registry *registry,
			 uint32_t id, const char *intf, uint32_t ver)
{
	uint32_t *name = data;

	if (strcmp (intf, "wl_data_offer") == 0) {
		assert(name);
		*name = id;
	}
}

static void
registry_handle_remove_filtered(void *data, struct wl_registry *registry,
				uint32_t id)
{
	assert(false);
}

static const struct wl_registry_listener registry_listener_filtered = {
	registry_handle_filtered,
	registry_handle_remove_filtered,
};

static void
get_globals(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;

	registry = wl_display_get_registry(c->wl_display);
	wl_registry_add_listener(registry, &registry_listener_filtered, data);
	wl_display_roundtrip(c->wl_display);

	wl_registry_destroy(registry);
	client_disconnect_nocheck(c);
}

TEST(filtered_global_is_hidden)
{
	struct display *d;
	struct wl_global *g;

	d = display_create();

	g = wl_global_create(d->wl_display, &wl_data_offer_interface,
		      1, d, bind_data_offer);
	wl_display_set_global_filter(d->wl_display, global_filter, NULL);

	client_create_noarg(d, get_globals);
	display_run(d);

	wl_global_destroy(g);

	display_destroy(d);
}

static void
get_dynamic_globals(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;

	registry = wl_display_get_registry(c->wl_display);
	wl_registry_add_listener(registry, &registry_listener_filtered, data);
	wl_display_roundtrip(c->wl_display);

	/* Wait for the server to create a new global */
	assert(stop_display(c, 1) >= 0);

	/* Check that we don't see it */
	wl_display_roundtrip(c->wl_display);

	/* Wait for the server to remove that global */
	assert(stop_display(c, 1) >= 0);

	/* Check that we don't get a global_remove event */
	wl_display_roundtrip(c->wl_display);

	wl_registry_destroy(registry);
	client_disconnect_nocheck(c);
}

TEST(filtered_dynamic_global_is_hidden)
{
	struct display *d;
	struct wl_global *g;

	d = display_create();
	wl_display_set_global_filter(d->wl_display, global_filter, NULL);

	/* Create a client and let it enumerate the globals */
	client_create_noarg(d, get_dynamic_globals);
	display_run(d);

	/* Dynamically create a new global */
	g = wl_global_create(d->wl_display, &wl_data_offer_interface,
			     1, d, bind_data_offer);

	display_resume(d);

	/* Dynamically remove the global */
	wl_global_destroy(g);

	display_resume(d);

	display_destroy(d);
}

static void
check_bind_error(struct client *c)
{
	uint32_t errorcode, id;
	int err;
	const struct wl_interface *intf;

	err = wl_display_get_error(c->wl_display);
	assert(err == EPROTO);

	errorcode = wl_display_get_protocol_error(c->wl_display, &intf, &id);
	assert(errorcode == WL_DISPLAY_ERROR_INVALID_OBJECT);
}

static void
force_bind(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;
	void *ptr;
	uint32_t *name = data;

	registry = wl_display_get_registry(c->wl_display);

	ptr = wl_registry_bind (registry, *name, &wl_data_offer_interface, 1);
	wl_display_roundtrip(c->wl_display);
	check_bind_error(c);

	wl_proxy_destroy((struct wl_proxy *) ptr);
	wl_registry_destroy(registry);

	client_disconnect_nocheck(c);
}

TEST(bind_fails_on_filtered_global)
{
	struct display *d;
	struct wl_global *g;
	uint32_t *name;

	/* Create a anonymous shared memory to pass the interface name */
	name = mmap(NULL, sizeof(uint32_t),
		    PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	d = display_create();

	g = wl_global_create(d->wl_display, &wl_data_offer_interface,
			     1, d, bind_data_offer);
	wl_display_set_global_filter(d->wl_display, global_filter, name);

	client_create(d, get_globals, name);
	*name = 0;

	display_run(d);
	/* wl_data_offer should be 2 */
	assert(*name == 2);
	wl_display_set_global_filter(d->wl_display, global_filter, NULL);

	/* Try to bind to the interface name when a global filter is in place */
	client_create(d, force_bind, name);
	display_run(d);

	wl_global_destroy(g);

	display_destroy(d);
}

static void
pre_fd(void *data, struct fd_passer *fdp)
{
	fd_passer_destroy(fdp);
}

static void
fd(void *data, struct fd_passer *fdp, int32_t fd)
{
	/* We destroyed the resource before this event */
	assert(false);
}

struct fd_passer_listener fd_passer_listener = {
	pre_fd,
	fd,
};

static void
zombie_fd_handle_globals(void *data, struct wl_registry *registry,
			 uint32_t id, const char *intf, uint32_t ver)
{
	struct fd_passer *fdp;

	if (!strcmp(intf, "fd_passer")) {
		fdp = wl_registry_bind(registry, id, &fd_passer_interface, 1);
		fd_passer_add_listener(fdp, &fd_passer_listener, NULL);
	}
}

static const struct wl_registry_listener zombie_fd_registry_listener = {
	zombie_fd_handle_globals,
	NULL
};

static void
zombie_client(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;

	registry = wl_display_get_registry(c->wl_display);
	wl_registry_add_listener(registry, &zombie_fd_registry_listener, NULL);

	/* Gets the registry */
	wl_display_roundtrip(c->wl_display);

	/* push out the fd_passer bind */
	wl_display_roundtrip(c->wl_display);

	/* push out our fd_passer.destroy */
	wl_display_roundtrip(c->wl_display);

	wl_registry_destroy(registry);

	client_disconnect_nocheck(c);
}

struct passer_data {
	struct wl_resource *conjoined_passer;
};

static void
feed_pipe(int fd, char tosend)
{
	int count;

	do {
		count = write(fd, &tosend, 1);
	} while (count != 1 && errno == EAGAIN);
	assert(count == 1);
	close(fd);
}

static void
fd_passer_clobber(struct wl_client *client, struct wl_resource *res)
{
	struct passer_data *pdata = wl_resource_get_user_data(res);
	int pipes1[2], pipes2[2], ret;

	if (pdata->conjoined_passer) {
		ret = pipe(pipes1);
		assert(ret == 0);
		ret = pipe(pipes2);
		assert(ret == 0);

		wl_resource_queue_event(res, FD_PASSER_FD, pipes1[0]);
		fd_passer_send_fd(pdata->conjoined_passer, pipes2[0]);
		feed_pipe(pipes1[1], '1');
		feed_pipe(pipes2[1], '2');
		close(pipes1[0]);
		close(pipes2[0]);
	}
	wl_resource_destroy(res);
}

static void
fd_passer_twin(struct wl_client *client, struct wl_resource *res, struct wl_resource *passer)
{
	struct passer_data *pdata = wl_resource_get_user_data(res);

	pdata->conjoined_passer = passer;
}

static const struct fd_passer_interface fdp_interface = {
	fd_passer_clobber,
	fd_passer_twin
};

static void
pdata_destroy(struct wl_resource *res)
{
	struct passer_data *pdata = wl_resource_get_user_data(res);

	free(pdata);
}

static void
bind_fd_passer(struct wl_client *client, void *data,
	       uint32_t vers, uint32_t id)
{
	struct wl_resource *res;
	struct passer_data *pdata;

	pdata = malloc(sizeof(*pdata));
	assert(pdata);
	pdata->conjoined_passer = NULL;

	res = wl_resource_create(client, &fd_passer_interface, vers, id);
	wl_resource_set_implementation(res, &fdp_interface, pdata, pdata_destroy);
	assert(res);
	if (vers == 1) {
		fd_passer_send_pre_fd(res);
		fd_passer_send_fd(res, fileno(stdin));
	}
}

TEST(zombie_fd)
{
	struct display *d;
	struct wl_global *g;

	d = display_create();

	g = wl_global_create(d->wl_display, &fd_passer_interface,
			     1, d, bind_fd_passer);

	client_create_noarg(d, zombie_client);
	display_run(d);

	wl_global_destroy(g);

	display_destroy(d);
}


static void
double_pre_fd(void *data, struct fd_passer *fdp)
{
	assert(false);
}

static void
double_fd(void *data, struct fd_passer *fdp, int32_t fd)
{
	char buf;
	int count;

	do {
		count = read(fd, &buf, 1);
	} while (count != 1 && errno == EAGAIN);
	assert(count == 1);

	close(fd);
	fd_passer_destroy(fdp);
	assert(buf == '2');
}

struct fd_passer_listener double_fd_passer_listener = {
	double_pre_fd,
	double_fd,
};


static void
double_zombie_fd_handle_globals(void *data, struct wl_registry *registry,
			 uint32_t id, const char *intf, uint32_t ver)
{
	struct fd_passer *fdp1, *fdp2;

	if (!strcmp(intf, "fd_passer")) {
		fdp1 = wl_registry_bind(registry, id, &fd_passer_interface, 2);
		fd_passer_add_listener(fdp1, &double_fd_passer_listener, NULL);
		fdp2 = wl_registry_bind(registry, id, &fd_passer_interface, 2);
		fd_passer_add_listener(fdp2, &double_fd_passer_listener, NULL);
		fd_passer_conjoin(fdp1, fdp2);
		fd_passer_destroy(fdp1);
	}
}

static const struct wl_registry_listener double_zombie_fd_registry_listener = {
	double_zombie_fd_handle_globals,
	NULL
};

static void
double_zombie_client(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;

	registry = wl_display_get_registry(c->wl_display);
	wl_registry_add_listener(registry, &double_zombie_fd_registry_listener, NULL);

	/* Gets the registry */
	wl_display_roundtrip(c->wl_display);

	/* One more so server can respond to conjoin+destroy */
	wl_display_roundtrip(c->wl_display);

	/* And finally push out our last fd_passer.destroy */
	wl_display_roundtrip(c->wl_display);

	wl_registry_destroy(registry);

	client_disconnect_nocheck(c);
}

TEST(zombie_fd_errant_consumption)
{
	struct display *d;
	struct wl_global *g;

	d = display_create();

	g = wl_global_create(d->wl_display, &fd_passer_interface,
			     2, d, bind_fd_passer);

	client_create_noarg(d, double_zombie_client);
	display_run(d);

	wl_global_destroy(g);

	display_destroy(d);
}


static void
registry_bind_interface_mismatch_handle_global(void *data,
					       struct wl_registry *registry,
					       uint32_t id, const char *intf,
					       uint32_t ver)
{
	uint32_t *seat_id_ptr = data;

	if (strcmp(intf, wl_seat_interface.name) == 0) {
		*seat_id_ptr = id;
	}
}

static const struct wl_registry_listener bind_interface_mismatch_registry_listener = {
	registry_bind_interface_mismatch_handle_global,
	NULL
};

static void
registry_bind_interface_mismatch_client(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;
	uint32_t seat_id = 0;
	void *ptr;
	int ret;

	registry = wl_display_get_registry(c->wl_display);
	wl_registry_add_listener(registry,
				 &bind_interface_mismatch_registry_listener,
				 &seat_id);

	ret = wl_display_roundtrip(c->wl_display);
	assert(ret >= 0);
	assert(seat_id != 0);

	/* Bind with a different interface */
	ptr = wl_registry_bind(registry, seat_id, &wl_output_interface, 1);
	ret = wl_display_roundtrip(c->wl_display);
	assert(ret < 0);
	check_bind_error(c);

	wl_proxy_destroy((struct wl_proxy *) ptr);
	wl_registry_destroy(registry);

	client_disconnect_nocheck(c);
}

TEST(registry_bind_interface_mismatch)
{
	struct display *d;
	struct wl_global *seat_global;

	d = display_create();

	seat_global = wl_global_create(d->wl_display, &wl_seat_interface,
				       1, NULL, NULL);

	client_create_noarg(d, registry_bind_interface_mismatch_client);
	display_run(d);

	wl_global_destroy(seat_global);

	display_destroy(d);
}

static void
send_overflow_client(void *data)
{
	struct client *c = client_connect();
	int i, err = 0;
	int *pipes = data;
	char tmp = '\0';
	int sock, optval = 16384;

	/* Limit the send buffer size for the display socket to guarantee
	 * that the test will cause an overflow. */
	sock = wl_display_get_fd(c->wl_display);
	assert(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval)) == 0);

	/* Request to break out of 'display_run' in the main process */
	assert(stop_display(c, 1) >= 0);

	/* On Linux, the actual socket data + metadata space is twice `optval`;
	 * since each noop request requires 8 bytes, the buffer should overflow
	 * within <=4096 iterations. */
	for (i = 0; i < 1000000; i++) {
		noop_request(c);
		err = wl_display_get_error(c->wl_display);
		if (err)
			break;
	}

	/* Do not close the pipe file descriptors afterwards, because the leak
	 * check verifies that the initial/final FD counts are the same */
	assert(write(pipes[1], &tmp, sizeof(tmp)) == (ssize_t)sizeof(tmp));

	/* Expect an error */
	fprintf(stderr, "Send loop failed on try %d, err = %d, %s\n", i, err, strerror(err));
	assert(err == EAGAIN);

	client_disconnect_nocheck(c);
}

TEST(send_overflow_disconnection)
{
	struct display *d;
	char tmp;
	int rpipe[2];
	ssize_t ret;

	assert(pipe(rpipe) != -1);

	d = display_create();

	(void) client_create(d, send_overflow_client, &rpipe);

	/* Close write end of the pipe, so that the later read() call gets
	 * interrupted if the client dies */
	close(rpipe[1]);

	/* Run the display until the client sends a `stop_display`, then
	 * send a resume message but don't actually look at new messages */
	display_run(d);
	display_post_resume_events(d);
	wl_display_flush_clients(d->wl_display);

	/* Wait until all noop requests have been sent (read returns 1), or
	 * until client process aborts (read returns 0) */
	do {
		ret = read(rpipe[0], &tmp, sizeof(tmp));
	} while (ret == -1 && errno == EINTR);
	assert(ret != -1);
	close(rpipe[0]);

	/* For a clean shutdown */
	display_run(d);

	display_destroy(d);
}

static void
registry_global_remove_before_handle_global(void *data,
					    struct wl_registry *registry,
					    uint32_t id, const char *intf,
					    uint32_t ver)
{
	uint32_t *id_ptr = data;

	if (strcmp(intf, wl_seat_interface.name) == 0) {
		assert(*id_ptr == 0);
		*id_ptr = id;
	}
}

static void
registry_global_remove_before_handle_global_remove(void *data,
						   struct wl_registry *registry,
						   uint32_t id)
{
	uint32_t *id_ptr = data;

	if (*id_ptr == id) {
		*id_ptr = 0;
	}
}

/* This listener expects a uint32_t user data pointer, sets it to the wl_seat
 * global ID when receiving a "global" event, and sets it to zero when receiving
 * a "global_remove" event. */
static const struct wl_registry_listener global_remove_before_registry_listener = {
	registry_global_remove_before_handle_global,
	registry_global_remove_before_handle_global_remove,
};

static void
global_remove_before_client(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;
	uint32_t global_id = 0, saved_global_id;
	struct wl_seat *seat;
	int ret;

	registry = wl_display_get_registry(c->wl_display);
	wl_registry_add_listener(registry,
				 &global_remove_before_registry_listener,
				 &global_id);

	ret = wl_display_roundtrip(c->wl_display);
	assert(ret >= 0);
	assert(global_id != 0);
	saved_global_id = global_id;

	/* Wait for the compositor to remove the global */
	assert(stop_display(c, 1) >= 0);

	/* Check binding still works after the global has been removed. Also
	 * check we get the global_remove event. */
	seat = wl_registry_bind(registry, saved_global_id, &wl_seat_interface, 1);
	ret = wl_display_roundtrip(c->wl_display);
	assert(ret >= 0);
	assert(global_id == 0);

	wl_seat_destroy(seat);
	wl_registry_destroy(registry);

	client_disconnect(c);
}

static void
registry_global_remove_after_handle_global(void *data,
					   struct wl_registry *registry,
					   uint32_t id, const char *intf,
					   uint32_t ver)
{
	/* Make sure the global isn't advertised anymore after being removed */
	assert(strcmp(intf, wl_seat_interface.name) != 0);
}

static const struct wl_registry_listener global_remove_after_registry_listener = {
	registry_global_remove_after_handle_global,
	NULL,
};

static void
global_remove_after_client(void *data)
{
	struct client *c = client_connect();
	struct wl_registry *registry;
	uint32_t global_id = 0;
	int ret;

	registry = wl_display_get_registry(c->wl_display);
	wl_registry_add_listener(registry,
				 &global_remove_after_registry_listener,
				 &global_id);

	ret = wl_display_roundtrip(c->wl_display);
	assert(ret >= 0);

	wl_registry_destroy(registry);

	client_disconnect(c);
}

TEST(global_remove)
{
	struct display *d;
	struct wl_global *global;

	d = display_create();

	global = wl_global_create(d->wl_display, &wl_seat_interface,
				  1, d, bind_seat);

	/* Create a client before removing the global */
	client_create_noarg(d, global_remove_before_client);

	display_run(d);

	wl_global_remove(global);

	/* Create another client after removing the global */
	client_create_noarg(d, global_remove_after_client);

	display_resume(d);

	wl_global_destroy(global);

	display_destroy(d);
}

static void
terminate_display(void *arg)
{
	struct wl_display *wl_display = arg;
	wl_display_terminate(wl_display);
}

TEST(no_source_terminate)
{
	struct display *d;
	struct wl_event_loop *loop;

	d = display_create();
	loop = wl_display_get_event_loop(d->wl_display);

	wl_event_loop_add_idle(loop, terminate_display, d->wl_display);

	display_run(d);
	display_destroy(d);
}
