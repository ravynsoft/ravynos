/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>

#include <avahi-common/gccmacro.h>
#include "sigint.h"

static AvahiSimplePoll *simple_poll = NULL;
static struct sigaction old_sigint_sa, old_sigterm_sa;
static int pipe_fds[2] = { -1, -1 };
static AvahiWatch *watch = NULL;

static int set_nonblock(int fd) {
    int n;

    assert(fd >= 0);

    if ((n = fcntl(fd, F_GETFL)) < 0)
        return -1;

    if (n & O_NONBLOCK)
        return 0;

    return fcntl(fd, F_SETFL, n|O_NONBLOCK);
}

static void handler(int s) {
    write(pipe_fds[1], &s, sizeof(s));
}

static void close_pipe_fds(void) {
    if (pipe_fds[0] >= 0)
        close(pipe_fds[0]);
    if (pipe_fds[1] >= 0)
        close(pipe_fds[1]);

    pipe_fds[0] = pipe_fds[1] = -1;
}

static void watch_callback(AvahiWatch *w, int fd, AvahiWatchEvent event, AVAHI_GCC_UNUSED void *userdata) {
    int s;
    ssize_t l;

    assert(w);
    assert(fd == pipe_fds[0]);
    assert(event == AVAHI_WATCH_IN);

    l = read(fd, &s, sizeof(s));
    assert(l == sizeof(s));

    fprintf(stderr, "Got %s, quitting.\n", s == SIGINT ? "SIGINT" : "SIGTERM");
    avahi_simple_poll_quit(simple_poll);
}

int sigint_install(AvahiSimplePoll *spoll) {
    struct sigaction sa;
    const AvahiPoll *p;

    assert(spoll);
    assert(!simple_poll);
    assert(pipe_fds[0] == -1 && pipe_fds[1] == -1);

    if (pipe(pipe_fds) < 0) {
        fprintf(stderr, "pipe() failed: %s\n", strerror(errno));
        return -1;
    }

    set_nonblock(pipe_fds[0]);
    set_nonblock(pipe_fds[1]);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, &old_sigint_sa) < 0) {
        fprintf(stderr, "sigaction() failed: %s\n", strerror(errno));
        close_pipe_fds();
        return -1;
    }

    if (sigaction(SIGTERM, &sa, &old_sigterm_sa) < 0) {
        sigaction(SIGINT, &old_sigint_sa, NULL);
        fprintf(stderr, "sigaction() failed: %s\n", strerror(errno));
        close_pipe_fds();
        return -1;
    }

    p = avahi_simple_poll_get(spoll);
    watch = p->watch_new(p, pipe_fds[0], AVAHI_WATCH_IN, watch_callback, NULL);
    assert(watch);

    simple_poll = spoll;
    return 0;
}

void sigint_uninstall(void) {

    if (!simple_poll)
        return;

    sigaction(SIGTERM, &old_sigterm_sa, NULL);
    sigaction(SIGINT, &old_sigint_sa, NULL);

    close_pipe_fds();

    if (watch) {
        const AvahiPoll *p;

        assert(simple_poll);
        p = avahi_simple_poll_get(simple_poll);

        p->watch_free(watch);
        watch = NULL;
    }

    simple_poll = NULL;
}
