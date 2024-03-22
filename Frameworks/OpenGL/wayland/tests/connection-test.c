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

#include <math.h>
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
#include <poll.h>

#include "wayland-private.h"
#include "test-runner.h"
#include "test-compositor.h"

static const char message[] = "Hello, world";

static struct wl_connection *
setup(int *s)
{
	struct wl_connection *connection;

	assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, s) == 0);

	connection = wl_connection_create(s[0]);
	assert(connection);

	return connection;
}

TEST(connection_create)
{
	struct wl_connection *connection;
	int s[2];

	connection = setup(s);
	wl_connection_destroy(connection);
	close(s[0]);
	close(s[1]);
}

TEST(connection_write)
{
	struct wl_connection *connection;
	int s[2];
	char buffer[64];

	connection = setup(s);

	assert(wl_connection_write(connection, message, sizeof message) == 0);
	assert(wl_connection_flush(connection) == sizeof message);
	assert(read(s[1], buffer, sizeof buffer) == sizeof message);
	assert(memcmp(message, buffer, sizeof message) == 0);

	wl_connection_destroy(connection);
	close(s[0]);
	close(s[1]);
}

TEST(connection_data)
{
	struct wl_connection *connection;
	int s[2];
	char buffer[64];

	connection = setup(s);

	assert(write(s[1], message, sizeof message) == sizeof message);
	assert(wl_connection_read(connection) == sizeof message);
	wl_connection_copy(connection, buffer, sizeof message);
	assert(memcmp(message, buffer, sizeof message) == 0);
	wl_connection_consume(connection, sizeof message);

	wl_connection_destroy(connection);
	close(s[0]);
	close(s[1]);
}

TEST(connection_queue)
{
	struct wl_connection *connection;
	int s[2];
	char buffer[64];

	connection = setup(s);

	/* Test that wl_connection_queue() puts data in the output
	 * buffer without flush it.  Verify that the data did get in
	 * the buffer by writing another message and making sure that
	 * we receive the two messages on the other fd. */

	assert(wl_connection_queue(connection, message, sizeof message) == 0);
	assert(wl_connection_flush(connection) == 0);
	assert(wl_connection_write(connection, message, sizeof message) == 0);
	assert(wl_connection_flush(connection) == 2 * sizeof message);
	assert(read(s[1], buffer, sizeof buffer) == 2 * sizeof message);
	assert(memcmp(message, buffer, sizeof message) == 0);
	assert(memcmp(message, buffer + sizeof message, sizeof message) == 0);

	wl_connection_destroy(connection);
	close(s[0]);
	close(s[1]);
}

static void
va_list_wrapper(const char *signature, union wl_argument *args, int count, ...)
{
	va_list ap;
	va_start(ap, count);
	wl_argument_from_va_list(signature, args, count, ap);
	va_end(ap);
}

TEST(argument_from_va_list)
{
	union wl_argument args[WL_CLOSURE_MAX_ARGS];
	struct wl_object fake_object, fake_new_object;
	struct wl_array fake_array;

	va_list_wrapper("i", args, 1, 100);
	assert(args[0].i == 100);

	va_list_wrapper("is", args, 2, 101, "value");
	assert(args[0].i == 101);
	assert(strcmp(args[1].s, "value") == 0);

	va_list_wrapper("?iuf?sonah", args, 8,
			102, 103, wl_fixed_from_int(104), "value",
			&fake_object, &fake_new_object, &fake_array, 106);
	assert(args[0].i == 102);
	assert(args[1].u == 103);
	assert(args[2].f == wl_fixed_from_int(104));
	assert(strcmp(args[3].s, "value") == 0);
	assert(args[4].o == &fake_object);
	assert(args[5].o == &fake_new_object);
	assert(args[6].a == &fake_array);
	assert(args[7].h == 106);
}

struct marshal_data {
	struct wl_connection *read_connection;
	struct wl_connection *write_connection;
	int s[2];
	uint32_t buffer[10];
	union {
		uint32_t u;
		int32_t i;
		const char *s;
		int h;
	} value;
};

static void
setup_marshal_data(struct marshal_data *data)
{
	assert(socketpair(AF_UNIX,
			  SOCK_STREAM | SOCK_CLOEXEC, 0, data->s) == 0);
	data->read_connection = wl_connection_create(data->s[0]);
	assert(data->read_connection);
	data->write_connection = wl_connection_create(data->s[1]);
	assert(data->write_connection);
}

static void
release_marshal_data(struct marshal_data *data)
{
	close(wl_connection_destroy(data->read_connection));
	close(wl_connection_destroy(data->write_connection));
}

static void
marshal(struct marshal_data *data, const char *format, int size, ...)
{
	struct wl_closure *closure;
	static const uint32_t opcode = 4444;
	static struct wl_object sender = { NULL, NULL, 1234 };
	struct wl_message message = { "test", format, NULL };
	va_list ap;

	va_start(ap, size);
	closure = wl_closure_vmarshal(&sender, opcode, ap, &message);
	va_end(ap);

	assert(closure);
	assert(wl_closure_send(closure, data->write_connection) == 0);
	wl_closure_destroy(closure);
	assert(wl_connection_flush(data->write_connection) == size);
	assert(read(data->s[0], data->buffer, sizeof data->buffer) == size);

	assert(data->buffer[0] == sender.id);
	assert(data->buffer[1] == (opcode | (size << 16)));
}

TEST(connection_marshal)
{
	struct marshal_data data;
	struct wl_object object;
	struct wl_array array;
	static const char text[] = "curry";

	setup_marshal_data(&data);

	marshal(&data, "i", 12, 42);
	assert(data.buffer[2] == 42);

	marshal(&data, "u", 12, 55);
	assert(data.buffer[2] == 55);

	marshal(&data, "s", 20, "frappo");
	assert(data.buffer[2] == 7);
	assert(strcmp((char *) &data.buffer[3], "frappo") == 0);

	object.id = 557799;
	marshal(&data, "o", 12, &object);
	assert(data.buffer[2] == object.id);

	marshal(&data, "n", 12, &object);
	assert(data.buffer[2] == object.id);

	array.data = (void *) text;
	array.size = sizeof text;
	marshal(&data, "a", 20, &array);
	assert(data.buffer[2] == array.size);
	assert(memcmp(&data.buffer[3], text, array.size) == 0);

	release_marshal_data(&data);
}

static void
expected_fail_marshal(int expected_error, const char *format, ...)
{
	struct wl_closure *closure;
	static const uint32_t opcode = 4444;
	static const struct wl_interface test_interface = {
		.name = "test_object"
	};
	static struct wl_object sender = { 0 };
	struct wl_message message = { "test", format, NULL };

	sender.interface = &test_interface;
	sender.id = 1234;
	va_list ap;

	va_start(ap, format);
	closure = wl_closure_vmarshal(&sender, opcode, ap, &message);
	va_end(ap);

	assert(closure == NULL);
	assert(errno == expected_error);
}

static void
expected_fail_marshal_send(struct marshal_data *data, int expected_error,
			   const char *format, ...)
{
	struct wl_closure *closure;
	static const uint32_t opcode = 4444;
	static struct wl_object sender = { NULL, NULL, 1234 };
	struct wl_message message = { "test", format, NULL };
	va_list ap;

	va_start(ap, format);
	closure = wl_closure_vmarshal(&sender, opcode, ap, &message);
	va_end(ap);

	assert(closure);
	assert(wl_closure_send(closure, data->write_connection) < 0);
	assert(errno == expected_error);

	wl_closure_destroy(closure);
}

TEST(connection_marshal_nullables)
{
	struct marshal_data data;
	struct wl_object object;
	const char text[] = "curry";

	setup_marshal_data(&data);

	expected_fail_marshal(EINVAL, "o", NULL);
	expected_fail_marshal(EINVAL, "s", NULL);
	expected_fail_marshal(EINVAL, "a", NULL);

	marshal(&data, "?o", 12, NULL);
	assert(data.buffer[2] == 0);

	marshal(&data, "?s", 12, NULL);
	assert(data.buffer[2] == 0);

	object.id = 55293;
	marshal(&data, "?o", 12, &object);
	assert(data.buffer[2] == object.id);

	marshal(&data, "?s", 20, text);
	assert(data.buffer[2] == sizeof text);
	assert(strcmp((char *) &data.buffer[3], text) == 0);

	release_marshal_data(&data);
}

static void
validate_demarshal_u(struct marshal_data *data,
		     struct wl_object *object, uint32_t u)
{
	assert(data->value.u == u);
}

static void
validate_demarshal_i(struct marshal_data *data,
		     struct wl_object *object, int32_t i)
{
	assert(data->value.i == i);
}

static void
validate_demarshal_s(struct marshal_data *data,
		     struct wl_object *object, const char *s)
{
	if (data->value.s != NULL)
		assert(strcmp(data->value.s, s) == 0);
	else
		assert(s == NULL);
}

static void
validate_demarshal_h(struct marshal_data *data,
		     struct wl_object *object, int fd)
{
	struct stat buf1, buf2;

	assert(fd != data->value.h);
	fstat(fd, &buf1);
	fstat(data->value.h, &buf2);
	assert(buf1.st_dev == buf2.st_dev);
	assert(buf1.st_ino == buf2.st_ino);
	close(fd);
	close(data->value.h);
}

static void
validate_demarshal_f(struct marshal_data *data,
		     struct wl_object *object, wl_fixed_t f)
{
	assert(data->value.i == f);
}

static void
demarshal(struct marshal_data *data, const char *format,
	  uint32_t *msg, void (*func)(void))
{
	struct wl_message message = { "test", format, NULL };
	struct wl_closure *closure;
	struct wl_map objects;
	struct wl_object object = { NULL, &func, 0 };
	int size = msg[1] >> 16;

	assert(write(data->s[1], msg, size) == size);
	assert(wl_connection_read(data->read_connection) == size);

	wl_map_init(&objects, WL_MAP_SERVER_SIDE);
	object.id = msg[0];
	closure = wl_connection_demarshal(data->read_connection,
					  size, &objects, &message);
	assert(closure);
	wl_closure_invoke(closure, WL_CLOSURE_INVOKE_SERVER, &object, 0, data);
	wl_closure_destroy(closure);
}

TEST(connection_demarshal)
{
	struct marshal_data data;
	uint32_t msg[10];

	setup_marshal_data(&data);

	data.value.u = 8000;
	msg[0] = 400200;	/* object id */
	msg[1] = 12 << 16;		/* size = 12, opcode = 0 */
	msg[2] = data.value.u;
	demarshal(&data, "u", msg, (void *) validate_demarshal_u);

	data.value.i = -557799;
	msg[0] = 400200;
	msg[1] = 12 << 16;
	msg[2] = data.value.i;
	demarshal(&data, "i", msg, (void *) validate_demarshal_i);

	data.value.s = "superdude";
	msg[0] = 400200;
	msg[1] = 24 << 16;
	msg[2] = 10;
	msg[3 + msg[2]/4] = 0;
	memcpy(&msg[3], data.value.s, msg[2]);
	demarshal(&data, "s", msg, (void *) validate_demarshal_s);

	data.value.s = "superdude";
	msg[0] = 400200;
	msg[1] = 24 << 16;
	msg[2] = 10;
	msg[3 + msg[2]/4] = 0;
	memcpy(&msg[3], data.value.s, msg[2]);
	demarshal(&data, "?s", msg, (void *) validate_demarshal_s);

	data.value.i = wl_fixed_from_double(-90000.2390);
	msg[0] = 400200;
	msg[1] = 12 << 16;
	msg[2] = data.value.i;
	demarshal(&data, "f", msg, (void *) validate_demarshal_f);

	data.value.s = NULL;
	msg[0] = 400200;
	msg[1] = 12 << 16;
	msg[2] = 0;
	demarshal(&data, "?s", msg, (void *) validate_demarshal_s);

	release_marshal_data(&data);
}

static void
marshal_demarshal(struct marshal_data *data,
		  void (*func)(void), int size, const char *format, ...)
{
	struct wl_closure *closure;
	static const int opcode = 4444;
	static struct wl_object sender = { NULL, NULL, 1234 };
	struct wl_message message = { "test", format, NULL };
	struct wl_map objects;
	struct wl_object object = { NULL, &func, 0 };
	va_list ap;
	uint32_t msg[1] = { 1234 };

	va_start(ap, format);
	closure = wl_closure_vmarshal(&sender, opcode, ap, &message);
	va_end(ap);

	assert(closure);
	assert(wl_closure_send(closure, data->write_connection) == 0);
	wl_closure_destroy(closure);
	assert(wl_connection_flush(data->write_connection) == size);

	assert(wl_connection_read(data->read_connection) == size);

	wl_map_init(&objects, WL_MAP_SERVER_SIDE);
	object.id = msg[0];
	closure = wl_connection_demarshal(data->read_connection,
					  size, &objects, &message);
	assert(closure);
	wl_closure_invoke(closure, WL_CLOSURE_INVOKE_SERVER, &object, 0, data);
	wl_closure_destroy(closure);
}

TEST(connection_marshal_demarshal)
{
	struct marshal_data data;
	char f[] = "/tmp/wayland-tests-XXXXXX";

	setup_marshal_data(&data);

	data.value.u = 889911;
	marshal_demarshal(&data, (void *) validate_demarshal_u,
			  12, "u", data.value.u);

	data.value.i = -13;
	marshal_demarshal(&data, (void *) validate_demarshal_i,
			  12, "i", data.value.i);

	data.value.s = "cookie robots";
	marshal_demarshal(&data, (void *) validate_demarshal_s,
			  28, "s", data.value.s);

	data.value.s = "cookie robots";
	marshal_demarshal(&data, (void *) validate_demarshal_s,
			  28, "?s", data.value.s);

	data.value.h = mkstemp(f);
	assert(data.value.h >= 0);
	unlink(f);
	marshal_demarshal(&data, (void *) validate_demarshal_h,
			  8, "h", data.value.h);

	data.value.i = wl_fixed_from_double(1234.5678);
	marshal_demarshal(&data, (void *) validate_demarshal_f,
	                  12, "f", data.value.i);

	data.value.i = wl_fixed_from_double(-90000.2390);
	marshal_demarshal(&data, (void *) validate_demarshal_f,
	                  12, "f", data.value.i);

	data.value.i = wl_fixed_from_double((1 << 23) - 1 + 0.0941);
	marshal_demarshal(&data, (void *) validate_demarshal_f,
	                  12, "f", data.value.i);

	release_marshal_data(&data);
}

static void
expected_fail_demarshal(struct marshal_data *data, const char *format,
                        const uint32_t *msg, int expected_error)
{
	struct wl_message message = { "test", format, NULL };
	struct wl_closure *closure;
	struct wl_map objects;
	int size = (msg[1] >> 16);

	assert(write(data->s[1], msg, size) == size);
	assert(wl_connection_read(data->read_connection) == size);

	wl_map_init(&objects, WL_MAP_SERVER_SIDE);
	closure = wl_connection_demarshal(data->read_connection,
					    size, &objects, &message);

	assert(closure == NULL);
	assert(errno == expected_error);
}

TEST(connection_demarshal_null_strings)
{
	struct marshal_data data;
	uint32_t msg[3];

	setup_marshal_data(&data);

	data.value.s = NULL;
	msg[0] = 400200;	/* object id */
	msg[1] = 12 << 16;	/* size = 12, opcode = 0 */
	msg[2] = 0;		/* string length = 0 */
	demarshal(&data, "?s", msg, (void *) validate_demarshal_s);

	expected_fail_demarshal(&data, "s", msg, EINVAL);

	release_marshal_data(&data);
}

/* These tests are verifying that the demarshaling code will gracefully handle
 * clients lying about string and array lengths and giving values near
 * UINT32_MAX. Before fixes f7fdface and f5b9e3b9 this test would crash on
 * 32bit systems.
 */
TEST(connection_demarshal_failures)
{
	struct marshal_data data;
	unsigned int i;
	uint32_t msg[3];

	const uint32_t overflowing_values[] = {
		/* Values very close to UINT32_MAX. Before f5b9e3b9 these
		 * would cause integer overflow in DIV_ROUNDUP. */
		0xffffffff, 0xfffffffe, 0xfffffffd, 0xfffffffc,

		/* Values at various offsets from UINT32_MAX. Before f7fdface
		 * these would overflow the "p" pointer on 32bit systems,
		 * effectively subtracting the offset from it. It had good
		 * chance to cause crash depending on what was stored at that
		 * offset before "p". */
		0xfffff000, 0xffffd000, 0xffffc000, 0xffffb000
	};

	setup_marshal_data(&data);

	/* sender_id, does not matter */
	msg[0] = 0;

	/* (size << 16 | opcode), opcode is 0, does not matter */
	msg[1] = sizeof(msg) << 16;

	for (i = 0; i < ARRAY_LENGTH(overflowing_values); i++) {
		/* length of the string or array */
		msg[2] = overflowing_values[i];

		expected_fail_demarshal(&data, "s", msg, EINVAL);
		expected_fail_demarshal(&data, "a", msg, EINVAL);
	}

	release_marshal_data(&data);
}

TEST(connection_marshal_alot)
{
	struct marshal_data data;
	char f[64];
	int i;

	setup_marshal_data(&data);

	/* We iterate enough to make sure we wrap the circular buffers
	 * for both regular data an fds. */

	for (i = 0; i < 2000; i++) {
		strcpy(f, "/tmp/wayland-tests-XXXXXX");
		data.value.h = mkstemp(f);
		assert(data.value.h >= 0);
		unlink(f);
		marshal_demarshal(&data, (void *) validate_demarshal_h,
				  8, "h", data.value.h);
	}

	release_marshal_data(&data);
}

TEST(connection_marshal_too_big)
{
	struct marshal_data data;
	char *big_string = malloc(5000);

	assert(big_string);

	memset(big_string, ' ', 4999);
	big_string[4999] = '\0';

	setup_marshal_data(&data);

	expected_fail_marshal_send(&data, E2BIG, "s", big_string);

	release_marshal_data(&data);
	free(big_string);
}

static void
marshal_helper(const char *format, void *handler, ...)
{
	struct wl_closure *closure;
	static struct wl_object sender = { NULL, NULL, 1234 };
	struct wl_object object = { NULL, &handler, 0 };
	static const int opcode = 4444;
	struct wl_message message = { "test", format, NULL };
	va_list ap;
	int done;

	va_start(ap, handler);
	closure = wl_closure_vmarshal(&sender, opcode, ap, &message);
	va_end(ap);

	assert(closure);
	done = 0;
	wl_closure_invoke(closure, WL_CLOSURE_INVOKE_SERVER, &object, 0, &done);
	wl_closure_destroy(closure);
	assert(done);
}

static void
suu_handler(void *data, struct wl_object *object,
	    const char *s, uint32_t u1, uint32_t u2)
{
	int *done = data;

	assert(strcmp(s, "foo") == 0);
	assert(u1 == 500);
	assert(u2 == 404040);
	*done = 1;
}

TEST(invoke_closure)
{
	marshal_helper("suu", suu_handler, "foo", 500, 404040);
}

static void
leak_closure(void)
{
	struct wl_callback *cb;
	struct pollfd pfd;
	struct client *c = client_connect();

	cb = wl_display_sync(c->wl_display);
	assert(cb);
	assert(wl_display_flush(c->wl_display) > 0);

	/* we don't need it, it is referenced */
	wl_callback_destroy(cb);

	pfd.fd = wl_display_get_fd(c->wl_display);
	pfd.events = POLLIN;

	test_set_timeout(2);
	assert(poll(&pfd, 1, -1) == 1);

	/* read events, but do not dispatch them */
	assert(wl_display_prepare_read(c->wl_display) == 0);
	assert(wl_display_read_events(c->wl_display) == 0);

	/*
	 * now we have wl_callback.done and wl_display.delete_id queued;
	 * if we now release the queue (in wl_display_disconnect())
	 * we should not leak memory
	 */

	client_disconnect(c);
}

TEST(closure_leaks)
{
	struct display *d = display_create();

	client_create_noarg(d, leak_closure);
	display_run(d);

	display_destroy(d);
}

static void
leak_after_error(void)
{
	struct client *c = client_connect();

	/* this should return -1, because we'll send error
	 * from server. */
	assert(stop_display(c, 1) == -1);
	assert(wl_display_dispatch_pending(c->wl_display) == -1);
	assert(wl_display_get_error(c->wl_display) == ENOMEM);

	/* after we got error, we have display_resume event
	 * in the queue. It should be freed in wl_display_disconnect().
	 * Let's see! */

	wl_proxy_destroy((struct wl_proxy *) c->tc);
	wl_display_disconnect(c->wl_display);
	free(c);
}

TEST(closure_leaks_after_error)
{
	struct display *d = display_create();
	struct client_info *cl;

	cl = client_create_noarg(d, leak_after_error);
	display_run(d);

	wl_client_post_no_memory(cl->wl_client);
	display_resume(d);

	display_destroy(d);
}

/** Raw read from socket expecting wl_display.error
 *
 * \param sockfd The socket to read from.
 * \param expected_error The expected wl_display error code.
 *
 * Reads the socket and manually parses one message, expecting it to be a
 * wl_display.error with the wl_display as the originating object.
 * Asserts that the received error code is expected_error.
 */
static void
expect_error_recv(int sockfd, uint32_t expected_error)
{
	uint32_t buf[1024];
	ssize_t slen;
	uint32_t opcode;
	int str_len;

	slen = recv(sockfd, buf, sizeof buf, 0);
	assert(slen >= 2 * (ssize_t)sizeof (uint32_t));
	opcode = buf[1] & 0xffff;
	fprintf(stderr, "Received %zd bytes, object %u, opcode %u\n",
		slen, buf[0], opcode);

	/* check error event */
	assert(buf[0] == 1);
	assert(opcode == WL_DISPLAY_ERROR);

	str_len = buf[4];
	assert(str_len > 0);
	assert(str_len <= slen - 5 * (ssize_t)sizeof (uint32_t));
	fprintf(stderr, "Error event on object %u, code %u, message \"%*s\"\n",
		buf[2], buf[3], str_len, (const char *)&buf[5]);

	assert(buf[3] == expected_error);
}

/* A test for https://gitlab.freedesktop.org/wayland/wayland/issues/52
 * trying to provoke a read from uninitialized memory in
 * wl_connection_demarshal() for sender_id and opcode.
 *
 * This test might not fail as is even with #52 unfixed, since there is no way
 * to detect what happens and the crash with zero size depends on stack content.
 * However, running under Valgrind would point out invalid reads and use of
 * uninitialized values.
 */
TEST(request_bogus_size)
{
	struct wl_display *display;
	struct wl_client *client;
	int s[2];
	uint32_t msg[3];
	int bogus_size;

	test_set_timeout(1);

	/*
	 * The manufactured message has real size 12. Test all bogus sizes
	 * smaller than that, and zero as the last one since wl_closure_init
	 * handles zero specially and having garbage in the stack makes it more
	 * likely to crash in wl_connection_demarshal.
	 */
	for (bogus_size = 11; bogus_size >= 0; bogus_size--) {
		fprintf(stderr, "* bogus size %d\n", bogus_size);

		assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, s) == 0);
		display = wl_display_create();
		assert(display);
		client = wl_client_create(display, s[0]);
		assert(client);

		/* manufacture a request that lies about its size */
		msg[0] = 1; /* sender id: wl_display */
		msg[1] = (bogus_size << 16) | WL_DISPLAY_SYNC; /* size and opcode */
		msg[2] = 2; /* sync argument: new_id for wl_callback */

		assert(send(s[1], msg, sizeof msg, 0) == sizeof msg);

		wl_event_loop_dispatch(wl_display_get_event_loop(display), 0);

		expect_error_recv(s[1], WL_DISPLAY_ERROR_INVALID_METHOD);

		/* Do not wl_client_destroy, the error already caused it. */
		close(s[1]);
		wl_display_destroy(display);
	}
}
