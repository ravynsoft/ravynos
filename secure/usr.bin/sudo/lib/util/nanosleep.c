/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2011, 2013, 2017-2018, 2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef HAVE_NANOSLEEP

#include <sys/types.h>
#include <sys/time.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#include <time.h>
#include <errno.h>

#include <sudo_compat.h>
#include <sudo_util.h>

int
sudo_nanosleep(const struct timespec *timeout, struct timespec *remainder)
{
    struct timespec endtime, now;
    struct timeval tv;
    int rval;

    if (timeout->tv_sec == 0 && timeout->tv_nsec < 1000) {
	tv.tv_sec = 0;
	tv.tv_usec = 1;
    } else {
	TIMESPEC_TO_TIMEVAL(&tv, timeout);
    }
    if (remainder != NULL) {
	if (sudo_gettime_real(&endtime) == -1)
	    return -1;
	sudo_timespecadd(&endtime, timeout, &endtime);
    }
    rval = select(0, NULL, NULL, NULL, &tv);
    if (remainder != NULL) {
	if (rval == 0) {
	    /* Timeout expired, no remaining time. */
	    sudo_timespecclear(remainder);
	} else if (errno == EINTR) {
	    /* Interrupted, compute remaining time. */
	    if (sudo_gettime_real(&now) == -1)
		return -1;
	    sudo_timespecsub(&endtime, &now, remainder);
	    if (remainder->tv_sec < 0 || remainder->tv_nsec < 0)
		sudo_timespecclear(remainder);
	}
    }
    return rval;
}
#endif /* HAVE_NANOSLEEP */
