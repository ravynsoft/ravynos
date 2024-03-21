/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014, Oracle and/or its affiliates.
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

#ifdef HAVE_SOLARIS_AUDIT

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include <bsm/adt.h>
#include <bsm/adt_event.h>

#include <sudoers.h>
#include <solaris_audit.h>

static adt_session_data_t *ah;		/* audit session handle */
static adt_event_data_t	*event;		/* event to be generated */
static char		cwd[PATH_MAX];
static char		cmdpath[PATH_MAX];

static int
adt_sudo_common(const struct sudoers_context *ctx, char *const argv[])
{
	int argc;

	if (adt_start_session(&ah, NULL, ADT_USE_PROC_DATA) != 0) {
		log_warning(ctx, SLOG_NO_STDERR, "adt_start_session");
		return -1;
	}
	if ((event = adt_alloc_event(ah, ADT_sudo)) == NULL) {
		log_warning(ctx, SLOG_NO_STDERR, "alloc_event");
		(void) adt_end_session(ah);
		return -1;
	}
	if ((event->adt_sudo.cwdpath = getcwd(cwd, sizeof(cwd))) == NULL) {
		log_warning(ctx, SLOG_NO_STDERR,
		    _("unable to get current working directory"));
	}

	/* get the real executable name */
	if (ctx->user.cmnd != NULL) {
		if (strlcpy(cmdpath, ctx->user.cmnd,
		    sizeof(cmdpath)) >= sizeof(cmdpath)) {
			log_warningx(ctx, SLOG_NO_STDERR,
			    _("truncated audit path ctx->user.cmnd: %s"),
			    ctx->user.cmnd);
		}
	} else {
		if (strlcpy(cmdpath, argv[0],
		    sizeof(cmdpath)) >= sizeof(cmdpath)) {
			log_warningx(ctx, SLOG_NO_STDERR,
			    _("truncated audit path argv[0]: %s"),
			    argv[0]);
		}
	}

	for (argc = 0; argv[argc] != NULL; argc++)
		continue;

	event->adt_sudo.cmdpath = cmdpath;
	event->adt_sudo.argc = argc - 1;
	event->adt_sudo.argv = (char **)&argv[1];
	event->adt_sudo.envp = env_get();

	return 0;
}


/*
 * Returns 0 on success or -1 on error.
 */
int
solaris_audit_success(const struct sudoers_context *ctx, char *const argv[])
{
	int rc = -1;

	if (adt_sudo_common(ctx, argv) != 0) {
		return -1;
	}
	if (adt_put_event(event, ADT_SUCCESS, ADT_SUCCESS) != 0) {
		log_warning(ctx, SLOG_NO_STDERR, "adt_put_event(ADT_SUCCESS)");
	} else {
		rc = 0;
	}
	adt_free_event(event);
	(void) adt_end_session(ah);

	return rc;
}

/*
 * Returns 0 on success or -1 on error.
 */
int
solaris_audit_failure(const struct sudoers_context *ctx, char *const argv[],
    const char *errmsg)
{
	int rc = -1;

	if (adt_sudo_common(ctx, argv) != 0) {
		return -1;
	}

	event->adt_sudo.errmsg = (char *)errmsg;
	if (adt_put_event(event, ADT_FAILURE, ADT_FAIL_VALUE_PROGRAM) != 0) {
		log_warning(ctx, SLOG_NO_STDERR, "adt_put_event(ADT_FAILURE)");
	} else {
		rc = 0;
	}
	adt_free_event(event);
	(void) adt_end_session(ah);

	return rc;
}

#endif /* HAVE_SOLARIS_AUDIT */
