#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "connection.h"
#include "test.h"

static void test_send_one_byte(void) {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	struct connection c1 = {.fd = fds[0]};
	struct connection c2 = {.fd = fds[1]};

	char in = 85, out = 0;

	test_assert(connection_put(&c1, &in, sizeof in) == 0);
	test_assert(connection_flush(&c1) == sizeof in);

	test_assert(connection_read(&c2) == sizeof out);
	test_assert(connection_pending(&c2) == sizeof out);
	test_assert(connection_get(&c2, &out, sizeof out) == sizeof out);
	test_assert(out == in);

	test_assert(connection_pending(&c2) == 0);
	close(fds[0]);
	close(fds[1]);
}

static void test_short_read(void) {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	struct connection c1 = {.fd = fds[0]};
	struct connection c2 = {.fd = fds[1]};

	char in = 85, out = 0;
	int out_large = 0;

	test_assert(connection_put(&c1, &in, sizeof in) == 0);
	test_assert(connection_flush(&c1) > 0);

	test_assert(connection_read(&c2) > 0);

	test_assert(connection_pending(&c2) == sizeof out);
	test_assert(connection_get(&c2, &out, sizeof out_large) == -1);
	test_assert(errno == EAGAIN);

	test_assert(connection_pending(&c2) == sizeof out);
	test_assert(connection_get(&c2, &out, sizeof out) == sizeof out);

	test_assert(out == in);

	test_assert(connection_pending(&c2) == 0);
	close(fds[0]);
	close(fds[1]);
}

static void test_long_write(void) {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	struct connection c1 = {.fd = fds[0]};
	struct connection c2 = {.fd = fds[1]};

	char in[CONNECTION_BUFFER_SIZE + 1];
	memset(in, 0, sizeof in);

	test_assert(connection_put(&c1, &in, sizeof in) == -1);
	test_assert(errno = EAGAIN);

	test_assert(connection_read(&c2) == -1 && errno == EAGAIN);
	test_assert(connection_pending(&c2) == 0);
	close(fds[0]);
	close(fds[1]);
}

static void test_send_one_int(void) {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	struct connection c1 = {.fd = fds[0]};
	struct connection c2 = {.fd = fds[1]};

	int in = 0xDEADBEEF, out = 0;

	test_assert(connection_put(&c1, &in, sizeof in) == 0);
	test_assert(connection_flush(&c1) == sizeof in);

	test_assert(connection_read(&c2) == sizeof out);
	test_assert(connection_pending(&c2) == sizeof out);
	test_assert(connection_get(&c2, &out, sizeof out) == sizeof out);

	test_assert(out == in);

	test_assert(connection_pending(&c2) == 0);
	close(fds[0]);
	close(fds[1]);
}

static void test_restore(void) {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	struct connection c1 = {.fd = fds[0]};
	struct connection c2 = {.fd = fds[1]};

	int in = 0xDEADBEEF, out = 0;

	test_assert(connection_put(&c1, &in, sizeof in) == 0);
	test_assert(connection_flush(&c1) == sizeof in);

	test_assert(connection_read(&c2) == sizeof out);
	test_assert(connection_pending(&c2) == sizeof out);
	test_assert(connection_get(&c2, &out, sizeof out) == sizeof out);

	test_assert(out == in);

	test_assert(connection_pending(&c2) == 0);

	connection_restore(&c2, sizeof out);
	test_assert(connection_pending(&c2) == sizeof out);
	test_assert(connection_get(&c2, &out, sizeof out) == sizeof out);

	test_assert(out == in);

	test_assert(connection_pending(&c2) == 0);
	close(fds[0]);
	close(fds[1]);
}

static void test_send_variable_sequence(void) {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	struct connection c1 = {.fd = fds[0]};
	struct connection c2 = {.fd = fds[1]};

	char in1 = 85, out1 = 0;
	int in2 = 0xDEADBEEF, out2 = 0;
	uint64_t in3 = 0xCAFEDEADBEEF, out3 = 0;
	char in4 = 85, out4 = 0;

	test_assert(connection_put(&c1, &in1, sizeof in1) == 0);
	test_assert(connection_put(&c1, &in2, sizeof in2) == 0);
	test_assert(connection_put(&c1, &in3, sizeof in3) == 0);
	test_assert(connection_put(&c1, &in4, sizeof in4) == 0);
	test_assert(connection_flush(&c1) > 0);

	test_assert(connection_read(&c2) > 0);
	test_assert(connection_pending(&c2) > 0);
	test_assert(connection_get(&c2, &out1, sizeof out1) == sizeof out1);
	test_assert(connection_get(&c2, &out2, sizeof out2) == sizeof out2);
	test_assert(connection_get(&c2, &out3, sizeof out3) == sizeof out3);
	test_assert(connection_get(&c2, &out4, sizeof out4) == sizeof out4);

	test_assert(out1 == in1);
	test_assert(out2 == in2);
	test_assert(out3 == in3);
	test_assert(out4 == in4);

	test_assert(connection_pending(&c2) == 0);
	close(fds[0]);
	close(fds[1]);
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	test_run(test_send_one_byte);
	test_run(test_short_read);
	test_run(test_long_write);
	test_run(test_send_one_int);
	test_run(test_restore);
	test_run(test_send_variable_sequence);
}
