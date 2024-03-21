/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <sudo.h>

/*
 * Add an fd to preserve.
 */
int
add_preserved_fd(struct preserved_fd_list *pfds, int fd)
{
    struct preserved_fd *pfd, *pfd_new;
    debug_decl(add_preserved_fd, SUDO_DEBUG_UTIL);

    pfd_new = malloc(sizeof(*pfd));
    if (pfd_new == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    pfd_new->lowfd = fd;
    pfd_new->highfd = fd;
    pfd_new->flags = fcntl(fd, F_GETFD);
    if (pfd_new->flags == -1) {
	free(pfd_new);
	debug_return_int(-1);
    }

    TAILQ_FOREACH(pfd, pfds, entries) {
	if (fd == pfd->highfd) {
	    /* already preserved */
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"fd %d already preserved", fd);
	    free(pfd_new);
	    pfd_new = NULL;
	    break;
	}
	if (fd < pfd->highfd) {
	    TAILQ_INSERT_BEFORE(pfd, pfd_new, entries);
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"preserving fd %d", fd);
	    pfd_new = NULL;
	    break;
	}
    }
    if (pfd_new != NULL) {
	TAILQ_INSERT_TAIL(pfds, pfd_new, entries);
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "preserving fd %d", fd);
    }

    debug_return_int(0);
}

/*
 * Close all descriptors, startfd and higher except those listed
 * in pfds.
 */
void
closefrom_except(int startfd, struct preserved_fd_list *pfds)
{
    int fd, lastfd = -1;
    struct preserved_fd *pfd, *pfd_next;
    unsigned char *fdbits;
    debug_decl(closefrom_except, SUDO_DEBUG_UTIL);

    /* First, relocate preserved fds to be as contiguous as possible.  */
    TAILQ_FOREACH_REVERSE_SAFE(pfd, pfds, preserved_fd_list, entries, pfd_next) {
	if (pfd->highfd < startfd)
	    continue;
	fd = dup(pfd->highfd);
	if (fd == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"dup %d", pfd->highfd);
	    if (errno == EBADF) {
		TAILQ_REMOVE(pfds, pfd, entries);
		continue;
	    }
	    /* NOTE: still need to adjust lastfd below with unchanged lowfd. */
	} else if (fd < pfd->highfd) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"dup %d -> %d", pfd->highfd, pfd->lowfd);
	    sudo_debug_update_fd(pfd->highfd, pfd->lowfd);
	    pfd->lowfd = fd;
	    fd = pfd->highfd;
	}
	if (fd != -1)
	    (void) close(fd);

	if (pfd->lowfd > lastfd)
	    lastfd = pfd->lowfd;	/* highest (relocated) preserved fd */
    }

    if (lastfd == -1) {
	/* No fds to preserve. */
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "closefrom(%d)", startfd);
	closefrom(startfd);
	debug_return;
    }

    /* Create bitmap of preserved (relocated) fds.  */
    fdbits = calloc((size_t)(lastfd + NBBY) / NBBY, 1);
    if (fdbits == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    TAILQ_FOREACH(pfd, pfds, entries) {
	sudo_setbit(fdbits, pfd->lowfd);
    }

    /*
     * Close any unpreserved fds [startfd,lastfd]
     */
    for (fd = startfd; fd <= lastfd; fd++) {
	if (!sudo_isset(fdbits, fd)) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"closing fd %d", fd);
#ifdef __APPLE__
	    /* Avoid potential libdispatch crash when we close its fds. */
	    (void) fcntl(fd, F_SETFD, FD_CLOEXEC);
#else
	    (void) close(fd);
#endif
	}
    }
    free(fdbits);

    /* Let closefrom() do the rest for us. */
    if (lastfd + 1 > startfd)
	startfd = lastfd + 1;
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"closefrom(%d)", startfd);
    closefrom(startfd);

    /* Restore preserved fds and set flags. */
    TAILQ_FOREACH_REVERSE(pfd, pfds, preserved_fd_list, entries) {
	if (pfd->lowfd != pfd->highfd) {
	    if (dup2(pfd->lowfd, pfd->highfd) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "dup2(%d, %d): %s", pfd->lowfd, pfd->highfd,
		    strerror(errno));
	    } else {
		sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		    "dup2(%d, %d)", pfd->lowfd, pfd->highfd);
	    }
	    if (fcntl(pfd->highfd, F_SETFD, pfd->flags) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "fcntl(%d, F_SETFD, %d): %s", pfd->highfd,
		    pfd->flags, strerror(errno));
	    } else {
		sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		    "fcntl(%d, F_SETFD, %d)", pfd->highfd, pfd->flags);
	    }
	    sudo_debug_update_fd(pfd->lowfd, pfd->highfd);
	    (void) close(pfd->lowfd);
	    pfd->lowfd = pfd->highfd;
	}
    }
    debug_return;
}

/*
 * Parse a comma-separated list of fds and add them to preserved_fds.
 */
void
parse_preserved_fds(struct preserved_fd_list *pfds, const char *fdstr)
{
    const char *cp = fdstr;
    long lval;
    char *ep;
    debug_decl(parse_preserved_fds, SUDO_DEBUG_UTIL);

    do {
	errno = 0;
	lval = strtol(cp, &ep, 10);
	if (ep == cp || (*ep != ',' && *ep != '\0')) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to parse fd string %s", cp);
	    break;
	}
	if ((errno == ERANGE && lval == LONG_MAX) || lval < 0 || lval > INT_MAX) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"range error parsing fd string %s", cp);
	} else {
	    add_preserved_fd(pfds, (int)lval);
	}
	cp = ep + 1;
    } while (*ep != '\0');

    debug_return;
}
