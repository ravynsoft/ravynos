/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_event.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_queue.h>
#include <sudo_util.h>

#include <logsrvd.h>

#if defined(HAVE_STRUCT_DIRENT_D_NAMLEN) && HAVE_STRUCT_DIRENT_D_NAMLEN
# define NAMLEN(dirent) (dirent)->d_namlen
#else
# define NAMLEN(dirent) strlen((dirent)->d_name)
#endif

static struct outgoing_journal_queue outgoing_journal_queue =
    TAILQ_HEAD_INITIALIZER(outgoing_journal_queue);

static struct sudo_event *outgoing_queue_event;

/*
 * Callback that runs when the outgoing queue retry timer fires.
 * Tries to relay the first entry in the outgoing queue.
 */
static void
outgoing_queue_cb(int unused, int what, void *v)
{
    struct connection_closure *closure;
    struct outgoing_journal *oj, *next;
    struct sudo_event_base *evbase = v;
    bool success = false;
    debug_decl(outgoing_queue_cb, SUDO_DEBUG_UTIL);

    /* Must have at least one relay server. */
    if (TAILQ_EMPTY(logsrvd_conf_relay_address()))
	debug_return;

    /* Process first journal. */
    TAILQ_FOREACH_SAFE(oj, &outgoing_journal_queue, entries, next) {
	FILE *fp;
	int fd;

	fd = open(oj->journal_path, O_RDWR);
	if (fd == -1) {
	    if (errno == ENOENT) {
		TAILQ_REMOVE(&outgoing_journal_queue, oj, entries);
		free(oj->journal_path);
		free(oj);
	    }
	    continue;
	}
	if (!sudo_lock_file(fd, SUDO_TLOCK)) {
	    sudo_warn(U_("unable to lock %s"), oj->journal_path);
	    close(fd);
	    continue;
	}
	fp = fdopen(fd, "r");
	if (fp == NULL) {
	    sudo_warn(U_("unable to open %s"), oj->journal_path);
	    close(fd);
	    break;
	}

	/* Allocate a connection closure and fill in journal vars. */
	closure = connection_closure_alloc(fd, false, true, evbase);
	if (closure == NULL) {
	    fclose(fp);
	    break;
	}
	closure->journal = fp;
	closure->journal_path = oj->journal_path;

	/* Done with oj now, closure owns journal_path. */
	TAILQ_REMOVE(&outgoing_journal_queue, oj, entries);
	free(oj);

	success = connect_relay(closure);
	if (!success) {
	    sudo_warnx("%s", U_("unable to connect to relay"));
	    connection_close(closure);
	}
	break;
    }
}

/*
 * Schedule the outgoing_queue_event, creating it as necessary.
 * The event will fire after the specified timeout elapses.
 */
bool
logsrvd_queue_enable(time_t timeout, struct sudo_event_base *evbase)
{
    debug_decl(logsrvd_queue_enable, SUDO_DEBUG_UTIL);

    if (!TAILQ_EMPTY(&outgoing_journal_queue)) {
	struct timespec tv = { timeout, 0 };

	if (outgoing_queue_event == NULL) {
	    outgoing_queue_event = sudo_ev_alloc(-1, SUDO_EV_TIMEOUT,
		outgoing_queue_cb, evbase);
	    if (outgoing_queue_event == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		debug_return_bool(false);
	    }
	}
	if (sudo_ev_add(evbase, outgoing_queue_event, &tv, false) == -1) {
	    sudo_warnx("%s", U_("unable to add event to queue"));
	    debug_return_bool(false);
	}
    }
    debug_return_bool(true);
}

/*
 * Allocate a queue item based on the connection and push it on
 * the outgoing queue.
 * Consumes journal_path from the closure.
 */
bool
logsrvd_queue_insert(struct connection_closure *closure)
{
    struct outgoing_journal *oj;
    debug_decl(logsrvd_queue_insert, SUDO_DEBUG_UTIL);

    if (closure->journal_path == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "missing journal_path for closure %p", closure);
	debug_return_bool(false);
    }

    if ((oj = malloc(sizeof(*oj))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    oj->journal_path = closure->journal_path;
    closure->journal_path = NULL;
    TAILQ_INSERT_TAIL(&outgoing_journal_queue, oj, entries);

    if (!logsrvd_queue_enable(logsrvd_conf_relay_retry_interval(),
	    closure->evbase))
	debug_return_bool(false);

    debug_return_bool(true);
}

/*
 * Scan the outgoing queue at startup and populate the
 * outgoing_journal_queue.
 */
bool
logsrvd_queue_scan(struct sudo_event_base *evbase)
{
    char path[PATH_MAX];
    struct dirent *dent;
    size_t prefix_len;
    int dirlen;
    DIR *dirp;
    debug_decl(logsrvd_queue_scan, SUDO_DEBUG_UTIL);

    /* Must have at least one relay server. */
    if (TAILQ_EMPTY(logsrvd_conf_relay_address()))
	debug_return_bool(true);

    dirlen = snprintf(path, sizeof(path), "%s/outgoing/%s",
	logsrvd_conf_relay_dir(), RELAY_TEMPLATE);
    if (dirlen >= ssizeof(path)) {
	errno = ENAMETOOLONG;
	sudo_warn("%s/outgoing/%s", logsrvd_conf_relay_dir(), RELAY_TEMPLATE);
	debug_return_bool(false);
    }
    dirlen -= (int)sizeof(RELAY_TEMPLATE) - 1;
    path[dirlen] = '\0';

    dirp = opendir(path);
    if (dirp == NULL) {
	sudo_warn("opendir %s", path);
	debug_return_bool(false);
    }
    prefix_len = strcspn(RELAY_TEMPLATE, "X");
    while ((dent = readdir(dirp)) != NULL) {
	struct outgoing_journal *oj;

	/* Skip anything that is not a relay temp file. */
	if (NAMLEN(dent) != sizeof(RELAY_TEMPLATE) - 1)
	    continue;
	if (strncmp(dent->d_name, RELAY_TEMPLATE, prefix_len) != 0)
	    continue;

	/* Add to queue. */
	path[dirlen] = '\0';
	if (strlcat(path, dent->d_name, sizeof(path)) >= sizeof(path))
	    continue;
	if ((oj = malloc(sizeof(*oj))) == NULL)
	    goto oom;
	if ((oj->journal_path = strdup(path)) == NULL) {
	    free(oj);
	    goto oom;
	}
	TAILQ_INSERT_TAIL(&outgoing_journal_queue, oj, entries);
    }
    closedir(dirp);

    /* Process the queue immediately. */
    if (!logsrvd_queue_enable(0, evbase))
	debug_return_bool(false);

    debug_return_bool(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    closedir(dirp);
    debug_return_bool(false);
}

/*
 * Dump outgoing queue in response to SIGUSR1.
 */
void
logsrvd_queue_dump(void)
{
    struct outgoing_journal *oj;
    debug_decl(logsrvd_queue_dump, SUDO_DEBUG_UTIL);

    if (TAILQ_EMPTY(&outgoing_journal_queue))
	debug_return;

    sudo_debug_printf(SUDO_DEBUG_INFO, "outgoing journal queue:");
    TAILQ_FOREACH(oj, &outgoing_journal_queue, entries) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "  %s", oj->journal_path);
    }
}
