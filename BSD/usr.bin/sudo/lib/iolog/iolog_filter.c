/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <regex.h>
#include <string.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_queue.h>
#include <sudo_util.h>

struct pwfilt_regex {
    TAILQ_ENTRY(pwfilt_regex) entries;
    char *pattern;
    regex_t regex;
};
TAILQ_HEAD(pwfilt_regex_list, pwfilt_regex);

struct pwfilt_handle {
    struct pwfilt_regex_list filters;
    bool is_filtered;
};

/*
 * Allocate a new filter handle.
 */
void *
iolog_pwfilt_alloc(void)
{
    struct pwfilt_handle *handle;
    debug_decl(iolog_pwfilt_alloc, SUDO_DEBUG_UTIL);

    handle = malloc(sizeof(*handle));
    if (handle != NULL) {
	TAILQ_INIT(&handle->filters);
	handle->is_filtered = false;
    }

    debug_return_ptr(handle);
}

/*
 * Unlink filt from filters and free it.
 */
static void
iolog_pwfilt_free_filter(struct pwfilt_regex_list *filters,
    struct pwfilt_regex *filt)
{
    debug_decl(iolog_pwfilt_free_filter, SUDO_DEBUG_UTIL);

    if (filt != NULL) {
	TAILQ_REMOVE(filters, filt, entries);
	regfree(&filt->regex);
	free(filt->pattern);
	free(filt);
    }

    debug_return;
}

/*
 * Free the given password filter handle.
 */
void
iolog_pwfilt_free(void *vhandle)
{
    struct pwfilt_handle *handle = vhandle;
    struct pwfilt_regex *filt;
    debug_decl(iolog_pwfilt_free, SUDO_DEBUG_UTIL);

    if (handle != NULL) {
	while ((filt = TAILQ_FIRST(&handle->filters)) != NULL) {
	    iolog_pwfilt_free_filter(&handle->filters, filt);
	}
	free(handle);
    }
    debug_return;
}

/*
 * Add a pattern to the password filter list.
 */
bool
iolog_pwfilt_add(void *vhandle, const char *pattern)
{
    struct pwfilt_handle *handle = vhandle;
    struct pwfilt_regex *filt;
    const char *errstr;
    debug_decl(iolog_pwfilt_add, SUDO_DEBUG_UTIL);

    filt = malloc(sizeof(*filt));
    if (filt == NULL)
	goto oom;
    filt->pattern = strdup(pattern);
    if (filt->pattern == NULL)
	goto oom;

    if (!sudo_regex_compile(&filt->regex, filt->pattern, &errstr)) {
	sudo_warnx(U_("invalid regular expression \"%s\": %s"),
	    pattern, U_(errstr));
	goto bad;
    }

    TAILQ_INSERT_TAIL(&handle->filters, filt, entries);
    debug_return_bool(true);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
bad:
    if (filt != NULL) {
	free(filt->pattern);
	free(filt);
    }
    debug_return_bool(false);
}

/*
 * Remove a pattern from the password filter list.
 */
bool
iolog_pwfilt_remove(void *vhandle, const char *pattern)
{
    struct pwfilt_handle *handle = vhandle;
    struct pwfilt_regex *filt, *next;
    bool ret = false;
    debug_decl(iolog_pwfilt_remove, SUDO_DEBUG_UTIL);

    TAILQ_FOREACH_SAFE(filt, &handle->filters, entries, next) {
	if (strcmp(filt->pattern, pattern) == 0) {
	    iolog_pwfilt_free_filter(&handle->filters, filt);
	    ret = true;
	}
    }
    debug_return_bool(ret);
}

/*
 * If logging output and filtering is _not_ enabled, match buf against the
 * password filter list patterns and, if there is a match, enable filtering.
 * If logging output and filtering _is_ enabled, disable filtering.
 * If logging input and filtering is enabled, replace all characters in
 * buf with stars ('*') up to the next linefeed or carriage return.
 */
bool
iolog_pwfilt_run(void *vhandle, int event, const char *buf,
    size_t len, char **newbuf)
{
    struct pwfilt_handle *handle = vhandle;
    struct pwfilt_regex *filt;
    char *copy;
    debug_decl(iolog_pwfilt_run, SUDO_DEBUG_UTIL);

    /*
     * We only filter ttyin/ttyout.  It is only possible to disable
     * echo when a tty is present.  Filtering passwords in the input
     * log when they appear in the output is pointless.  This does
     * assume that the password prompt is written to the tty as well.
     */
    switch (event) {
    case IO_EVENT_TTYOUT:
	/* If filtering passwords and we receive output, disable it. */
	if (handle->is_filtered)
	    handle->is_filtered = false;

	/* Make a copy of buf that is NUL-terminated. */
	copy = malloc(len + 1);
	if (copy == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_bool(false);
	}
	memcpy(copy, buf, len);
	copy[len] = '\0';

	/* Check output for a password prompt. */
	TAILQ_FOREACH(filt, &handle->filters, entries) {
	    if (regexec(&filt->regex, copy, 0, NULL, 0) == 0) {
		handle->is_filtered = true;
		break;
	    }
	}
	free(copy);
	break;
    case IO_EVENT_TTYIN:
	if (handle->is_filtered) {
	    unsigned int i;

	    for (i = 0; i < len; i++) {
		/* We will stop filtering after reaching cr/nl. */
		if (buf[i] == '\r' || buf[i] == '\n') {
		    handle->is_filtered = false;
		    break;
		}
	    }
	    if (i != 0) {
		/* Filtered, replace buffer with '*' chars. */
		copy = malloc(len);
		if (copy == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_bool(false);
		}
		memset(copy, '*', i);
		if (i != len) {
		    /* Done filtering, copy cr/nl and subsequent characters. */
		    memcpy(copy + i, buf + i, len - i);
		}
		*newbuf = copy;
	    }
	}
	break;
    }

    debug_return_bool(true);
}
