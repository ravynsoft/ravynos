/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef HAVE_LINUX_AUDIT

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <libaudit.h>

#include <sudoers.h>
#include <linux_audit.h>

#define AUDIT_NOT_CONFIGURED	-2

/*
 * Open audit connection if possible.
 * Returns audit fd on success and -1 on failure.
 */
static int
linux_audit_open(void)
{
    static int au_fd = -1;
    debug_decl(linux_audit_open, SUDOERS_DEBUG_AUDIT);

    if (au_fd != -1)
	debug_return_int(au_fd);
    au_fd = audit_open();
    if (au_fd == -1) {
	/* Kernel may not have audit support. */
	if (errno == EINVAL || errno == EPROTONOSUPPORT || errno == EAFNOSUPPORT)
	    au_fd = AUDIT_NOT_CONFIGURED;
	else
	    sudo_warn("%s", U_("unable to open audit system"));
    } else if (fcntl(au_fd, F_SETFD, FD_CLOEXEC) == -1) {
	sudo_warn("%s", U_("unable to open audit system"));
	audit_close(au_fd);
	au_fd = -1;
    }
    debug_return_int(au_fd);
}

int
linux_audit_command(char *const argv[], int result)
{
    int au_fd, rc = -1;
    char *cp, *command = NULL;
    char * const *av;
    size_t size, n;
    debug_decl(linux_audit_command, SUDOERS_DEBUG_AUDIT);

    /* Don't return an error if auditing is not configured. */
    if ((au_fd = linux_audit_open()) < 0)
	debug_return_int(au_fd == AUDIT_NOT_CONFIGURED ? 0 : -1);

    /* Convert argv to a flat string. */
    for (size = 0, av = argv; *av != NULL; av++)
	size += strlen(*av) + 1;
    if (size != 0)
	command = malloc(size);
    if (command == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    for (av = argv, cp = command; *av != NULL; av++) {
	const size_t rem = size - (size_t)(cp - command);
	n = strlcpy(cp, *av, rem);
	if (n >= rem) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    goto done;
	}
	cp += n;
	*cp++ = ' ';
    }
    *--cp = '\0';

    /* Log command, ignoring ECONNREFUSED on error. */
    if (audit_log_user_command(au_fd, AUDIT_USER_CMD, command, NULL, result) <= 0) {
	if (errno != ECONNREFUSED) {
	    sudo_warn("%s", U_("unable to send audit message"));
	    goto done;
	}
    }

    rc = 0;

done:
    free(command);

    debug_return_int(rc);
}

#endif /* HAVE_LINUX_AUDIT */
