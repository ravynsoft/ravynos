/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2015, 2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>		/* for size_t, ssize_t */
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#ifndef __linux__
# if defined(HAVE_SYS_SYSCTL_H)
#  include <sys/sysctl.h>
# elif defined(HAVE_GETUTXID)
#  include <utmpx.h>
# elif defined(HAVE_GETUTID)
#  include <utmp.h>
# endif
#endif /* !__linux__ */

#include <sudoers.h>

/*
 * Fill in a struct timespec with the time the system booted.
 * Returns 1 on success and 0 on failure.
 */

#if defined(__linux__)
bool
get_boottime(struct timespec *ts)
{
    char *line = NULL;
    size_t linesize = 0;
    bool found = false;
    long long llval;
    ssize_t len;
    FILE *fp;
    debug_decl(get_boottime, SUDOERS_DEBUG_UTIL);

    /* read btime from /proc/stat */
    fp = fopen("/proc/stat", "r");
    if (fp != NULL) {
	while ((len = getdelim(&line, &linesize, '\n', fp)) != -1) {
	    if (strncmp(line, "btime ", 6) == 0) {
		if (line[len - 1] == '\n')
		    line[len - 1] = '\0';
		llval = sudo_strtonum(line + 6, 1, LLONG_MAX, NULL);
		if (llval > 0) {
		    ts->tv_sec = (time_t)llval;
		    ts->tv_nsec = 0;
		    found = true;
		    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
			"found btime in /proc/stat: %lld", llval);
		    break;
		} else {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
			"invalid btime in /proc/stat: %s", line);
		}
	    }
	}
	fclose(fp);
	free(line);
    }

    debug_return_bool(found);
}

#elif defined(HAVE_SYSCTL) && defined(KERN_BOOTTIME)

bool
get_boottime(struct timespec *ts)
{
    size_t size;
    int mib[2];
    struct timeval tv;
    debug_decl(get_boottime, SUDOERS_DEBUG_UTIL);

    mib[0] = CTL_KERN;
    mib[1] = KERN_BOOTTIME;
    size = sizeof(tv);
    if (sysctl(mib, 2, &tv, &size, NULL, 0) != -1) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "KERN_BOOTTIME: %lld, %ld", (long long)tv.tv_sec, (long)tv.tv_usec);
	TIMEVAL_TO_TIMESPEC(&tv, ts);
	debug_return_bool(true);
    }

    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"KERN_BOOTTIME: %s", strerror(errno));
    debug_return_bool(false);
}

#elif defined(HAVE_GETUTXID)

bool
get_boottime(struct timespec *ts)
{
    struct utmpx *ut, key;
    debug_decl(get_boottime, SUDOERS_DEBUG_UTIL);

    memset(&key, 0, sizeof(key));
    key.ut_type = BOOT_TIME;
    setutxent();
    if ((ut = getutxid(&key)) != NULL) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "BOOT_TIME: %lld, %ld", (long long)ut->ut_tv.tv_sec,
	    (long)ut->ut_tv.tv_usec);
	TIMEVAL_TO_TIMESPEC(&ut->ut_tv, ts);
    }
    endutxent();
    debug_return_bool(ut != NULL);
}

#elif defined(HAVE_GETUTID)

bool
get_boottime(struct timespec *ts)
{
    struct utmp *ut, key;
    debug_decl(get_boottime, SUDOERS_DEBUG_UTIL);

    memset(&key, 0, sizeof(key));
    key.ut_type = BOOT_TIME;
    setutent();
    if ((ut = getutid(&key)) != NULL) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "BOOT_TIME: %lld", (long long)ut->ut_time);
	ts->tv_sec = ut->ut_time;
	ts->tv_nsec = 0;
    }
    endutent();
    debug_return_bool(ut != NULL);
}

#else

bool
get_boottime(struct timespec *ts)
{
    debug_decl(get_boottime, SUDOERS_DEBUG_UTIL);
    debug_return_bool(false);
}
#endif
