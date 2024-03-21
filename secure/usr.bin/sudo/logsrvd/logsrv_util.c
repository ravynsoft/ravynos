/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <errno.h>
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
#include <time.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

#include "logsrv_util.h"

/*
 * Expand buf as needed or just reset it.
 */
bool
expand_buf(struct connection_buffer *buf, size_t needed)
{
    void *newdata;
    debug_decl(expand_buf, SUDO_DEBUG_UTIL);

    if (buf->size < needed) {
	/* Expand buffer. */
	const size_t newsize = sudo_pow2_roundup(needed);
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "expanding buffer from %zu to %zu", buf->size, newsize);
	if (newsize < needed) {
	    /* overflow */
	    errno = ENOMEM;
	    goto oom;
	}
	if ((newdata = malloc(newsize)) == NULL)
	    goto oom;
	if (buf->len != buf->off)
	    memcpy(newdata, buf->data + buf->off, buf->len - buf->off);
	free(buf->data);
	buf->data = newdata;
	buf->size = newsize;
    } else {
	/* Just reset existing buffer. */
	if (buf->len != buf->off) {
	    memmove(buf->data, buf->data + buf->off,
		buf->len - buf->off);
	}
    }
    buf->len -= buf->off;
    buf->off = 0;

    debug_return_bool(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_bool(false);
}

/*
 * Open any I/O log files that are present.
 * The timing file must always exist.
 */
bool
iolog_open_all(int dfd, const char *iolog_dir, struct iolog_file *iolog_files,
    const char *mode)
{
    int iofd;
    debug_decl(iolog_open_all, SUDO_DEBUG_UTIL);

    for (iofd = 0; iofd < IOFD_MAX; iofd++) {
	iolog_files[iofd].enabled = true;
        if (!iolog_open(&iolog_files[iofd], dfd, iofd, mode)) {
	    if (errno != ENOENT) {
		sudo_warn(U_("unable to open %s/%s"), iolog_dir,
		    iolog_fd_to_name(iofd));
		debug_return_bool(false);
	    }
	}
    }
    if (!iolog_files[IOFD_TIMING].enabled) {
	sudo_warn(U_("unable to open %s/%s"), iolog_dir,
	    iolog_fd_to_name(IOFD_TIMING));
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Seek to the specified point in time in the I/O logs.
 */
bool
iolog_seekto(int iolog_dir_fd, const char *iolog_path,
    struct iolog_file *iolog_files, struct timespec *elapsed_time,
    const struct timespec *target)
{
    struct timing_closure timing;
    off_t pos;
    debug_decl(iolog_seekto, SUDO_DEBUG_UTIL);

    if (!sudo_timespecisset(target)) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "resuming at start of file [0, 0]");
	debug_return_bool(true);
    }

    memset(&timing, 0, sizeof(timing));
    timing.decimal = ".";

    /* Parse timing file until we reach the target point. */
    for (;;) {
	switch (iolog_read_timing_record(&iolog_files[IOFD_TIMING], &timing)) {
	case 0:
	    break;
	case 1:
	    /* EOF reading timing file. */
	    sudo_warnx(U_("%s/%s: unable to find resume point [%lld, %ld]"),
		iolog_path, "timing", (long long)target->tv_sec,
		target->tv_nsec);
	    goto bad;
	default:
	    /* Error printed by iolog_read_timing_record(). */
	    goto bad;
	}
	sudo_timespecadd(elapsed_time, &timing.delay, elapsed_time);
	if (timing.event < IOFD_TIMING) {
	    if (!iolog_files[timing.event].enabled) {
		/* Missing log file. */
		sudo_warn(U_("missing I/O log file %s/%s"), iolog_path,
		    iolog_fd_to_name(timing.event));
		goto bad;
	    }
	    pos = iolog_seek(&iolog_files[timing.event], (off_t)timing.u.nbytes,
		SEEK_CUR);
	    if (pos == -1) {
		sudo_warn(U_("%s/%s: unable to seek forward %zu"), iolog_path,
		    iolog_fd_to_name(timing.event), timing.u.nbytes);
		goto bad;
	    }
	}
	if (sudo_timespeccmp(elapsed_time, target, >=)) {
	    if (sudo_timespeccmp(elapsed_time, target, ==))
		break;

	    /* Mismatch between resume point and stored log. */
	    sudo_warnx(U_("%s/%s: unable to find resume point [%lld, %ld]"),
		iolog_path, "timing", (long long)target->tv_sec,
		target->tv_nsec);
	    goto bad;
	}
    }
    debug_return_bool(true);
bad:
    debug_return_bool(false);
}
