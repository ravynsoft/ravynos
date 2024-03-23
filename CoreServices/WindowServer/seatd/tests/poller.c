#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "poller.h"
#include "test.h"

static void test_poller_init(void) {
	struct poller poller;
	test_assert(poller_init(&poller) == 0);
	poller_finish(&poller);
}

struct test_fd {
	int fd;
	uint32_t events;
};

static int test_fd_event(int fd, uint32_t mask, void *data) {
	struct test_fd *d = data;
	d->fd = fd;
	d->events = mask;
	return 0;
}

static void test_poller_single_fd(void) {
	struct poller poller;
	test_assert(poller_init(&poller) == 0);

	int fds[2];
	test_assert(pipe(fds) == 0);

	struct test_fd evd;
	struct event_source_fd *ev =
		poller_add_fd(&poller, fds[0], EVENT_READABLE, test_fd_event, &evd);
	test_assert(ev != NULL);

	evd.fd = 0;
	evd.events = 0;
	test_assert(write(fds[1], "\0", 1) == 1);
	test_assert(poller_poll(&poller) == 0);
	test_assert(evd.fd == fds[0]);
	test_assert(evd.events == EVENT_READABLE);

	evd.fd = 0;
	evd.events = 0;
	test_assert(write(fds[1], "\0", 1) == 1);
	test_assert(poller_poll(&poller) == 0);
	test_assert(evd.fd == fds[0]);
	test_assert(evd.events == EVENT_READABLE);

	close(fds[0]);
	close(fds[1]);
	poller_finish(&poller);
}

static void test_poller_multi_fd(void) {
	struct poller poller;
	test_assert(poller_init(&poller) == 0);

	char dummy[8];
	int fdsa[2], fdsb[2];
	test_assert(pipe(fdsa) == 0);
	test_assert(pipe(fdsb) == 0);

	struct test_fd evd1, evd2;
	struct event_source_fd *ev1 =
		poller_add_fd(&poller, fdsa[0], EVENT_READABLE, test_fd_event, &evd1);
	struct event_source_fd *ev2 =
		poller_add_fd(&poller, fdsb[0], EVENT_READABLE, test_fd_event, &evd2);
	test_assert(ev1 != NULL);
	test_assert(ev2 != NULL);

	evd1.fd = evd2.fd = 0;
	evd1.events = evd2.events = 0;
	test_assert(write(fdsa[1], "\0", 1) == 1);
	test_assert(poller_poll(&poller) == 0);
	test_assert(read(fdsa[0], &dummy, sizeof dummy) == 1);
	test_assert(evd1.fd == fdsa[0]);
	test_assert(evd1.events == EVENT_READABLE);
	test_assert(evd2.fd == 0);
	test_assert(evd2.events == 0);

	evd1.fd = evd2.fd = 0;
	evd1.events = evd2.events = 0;
	test_assert(write(fdsb[1], "\0", 1) == 1);
	test_assert(poller_poll(&poller) == 0);
	test_assert(read(fdsb[0], &dummy, sizeof dummy) == 1);
	test_assert(evd1.fd == 0);
	test_assert(evd1.events == 0);
	test_assert(evd2.fd == fdsb[0]);
	test_assert(evd2.events == EVENT_READABLE);

	evd1.fd = evd2.fd = 0;
	evd1.events = evd2.events = 0;
	test_assert(write(fdsa[1], "\0", 1) == 1);
	test_assert(write(fdsb[1], "\0", 1) == 1);
	test_assert(poller_poll(&poller) == 0);
	test_assert(read(fdsa[0], &dummy, sizeof dummy) == 1);
	test_assert(read(fdsb[0], &dummy, sizeof dummy) == 1);
	test_assert(evd1.fd == fdsa[0]);
	test_assert(evd1.events == EVENT_READABLE);
	test_assert(evd2.fd == fdsb[0]);
	test_assert(evd2.events == EVENT_READABLE);

	close(fdsa[0]);
	close(fdsa[1]);
	close(fdsb[0]);
	close(fdsb[1]);
	poller_finish(&poller);
}

struct test_signal {
	int signal;
};

static int test_signal_event(int signal, void *data) {
	struct test_signal *d = data;
	d->signal = signal;
	return 0;
}

static void test_poller_single_signal(void) {
	struct poller poller;
	test_assert(poller_init(&poller) == 0);

	struct test_signal evd;
	struct event_source_signal *ev =
		poller_add_signal(&poller, SIGRTMIN, test_signal_event, &evd);
	test_assert(ev != NULL);

	evd.signal = 0;
	test_assert(kill(getpid(), SIGRTMIN) == 0);
	test_assert(poller_poll(&poller) == 0);
	test_assert(evd.signal == SIGRTMIN);

	evd.signal = 0;
	test_assert(kill(getpid(), SIGRTMIN) == 0);
	test_assert(poller_poll(&poller) == 0);
	test_assert(evd.signal == SIGRTMIN);

	poller_finish(&poller);
}

static void test_poller_multi_signal(void) {
	struct poller poller;
	test_assert(poller_init(&poller) == 0);

	struct test_signal evd1, evd2;
	struct event_source_signal *ev1 =
		poller_add_signal(&poller, SIGRTMIN, test_signal_event, &evd1);
	struct event_source_signal *ev2 =
		poller_add_signal(&poller, SIGRTMIN + 1, test_signal_event, &evd2);
	test_assert(ev1 != NULL);
	test_assert(ev2 != NULL);

	evd1.signal = evd2.signal = 0;
	test_assert(kill(getpid(), SIGRTMIN) == 0);
	test_assert(poller_poll(&poller) == 0);
	test_assert(evd1.signal == SIGRTMIN);
	test_assert(evd2.signal == 0);

	evd1.signal = evd2.signal = 0;
	test_assert(kill(getpid(), SIGRTMIN + 1) == 0);
	test_assert(poller_poll(&poller) == 0);
	test_assert(evd1.signal == 0);
	test_assert(evd2.signal == SIGRTMIN + 1);

	evd1.signal = evd2.signal = 0;
	test_assert(kill(getpid(), SIGRTMIN) == 0);
	test_assert(kill(getpid(), SIGRTMIN + 1) == 0);
	test_assert(poller_poll(&poller) == 0);
	test_assert(evd1.signal == SIGRTMIN);
	test_assert(evd2.signal == SIGRTMIN + 1);

	poller_finish(&poller);
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	test_run(test_poller_init);
	test_run(test_poller_single_fd);
	test_run(test_poller_multi_fd);
	test_run(test_poller_single_signal);
	test_run(test_poller_multi_signal);
}
