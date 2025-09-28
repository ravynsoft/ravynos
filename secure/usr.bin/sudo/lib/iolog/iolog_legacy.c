/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

bool
iolog_parse_loginfo_legacy(FILE *fp, const char *iolog_dir,
    struct eventlog *evlog)
{
    char *buf = NULL, *cp, *ep;
    const char *errstr;
    size_t bufsize = 0, cwdsize = 0, cmdsize = 0;
    bool ret = false;
    debug_decl(iolog_parse_loginfo_legacy, SUDO_DEBUG_UTIL);

    /*
     * Info file has three lines:
     *  1) a log info line
     *  2) cwd
     *  3) command with args
     */
    if (getdelim(&buf, &bufsize, '\n', fp) == -1 ||
	getdelim(&evlog->cwd, &cwdsize, '\n', fp) == -1 ||
	getdelim(&evlog->command, &cmdsize, '\n', fp) == -1) {
	sudo_warn(U_("%s: invalid log file"), iolog_dir);
	goto done;
    }

    /* Strip the newline from the cwd and command. */
    evlog->cwd[strcspn(evlog->cwd, "\n")] = '\0';
    evlog->command[strcspn(evlog->command, "\n")] = '\0';

    /*
     * Crack the log line (lines and cols not present in old versions).
     *	timestamp:user:runas_user:runas_group:tty:lines:cols
     * XXX - probably better to use strtok and switch on the state.
     */
    buf[strcspn(buf, "\n")] = '\0';
    cp = buf;

    /* timestamp */
    if ((ep = strchr(cp, ':')) == NULL) {
	sudo_warn(U_("%s: time stamp field is missing"), iolog_dir);
	goto done;
    }
    *ep = '\0';
    evlog->submit_time.tv_sec =
	(time_t)sudo_strtonum(cp, 0, TIME_T_MAX, &errstr);
    if (errstr != NULL) {
	sudo_warn(U_("%s: time stamp %s: %s"), iolog_dir, cp, errstr);
	goto done;
    }

    /* submit user */
    cp = ep + 1;
    if ((ep = strchr(cp, ':')) == NULL) {
	sudo_warn(U_("%s: user field is missing"), iolog_dir);
	goto done;
    }
    if ((evlog->submituser = strndup(cp, (size_t)(ep - cp))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    /* runas user */
    cp = ep + 1;
    if ((ep = strchr(cp, ':')) == NULL) {
	sudo_warn(U_("%s: runas user field is missing"), iolog_dir);
	goto done;
    }
    if ((evlog->runuser = strndup(cp, (size_t)(ep - cp))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    /* runas group */
    cp = ep + 1;
    if ((ep = strchr(cp, ':')) == NULL) {
	sudo_warn(U_("%s: runas group field is missing"), iolog_dir);
	goto done;
    }
    if (cp != ep) {
	if ((evlog->rungroup = strndup(cp, (size_t)(ep - cp))) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
    }

    /* tty, followed by optional lines + cols */
    cp = ep + 1;
    if ((ep = strchr(cp, ':')) == NULL) {
	/* just the tty */
	if ((evlog->ttyname = strdup(cp)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
    } else {
	/* tty followed by lines + cols */
	if ((evlog->ttyname = strndup(cp, (size_t)(ep - cp))) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	cp = ep + 1;
	/* need to NULL out separator to use sudo_strtonum() */
	/* XXX - use sudo_strtonumx */
	if ((ep = strchr(cp, ':')) != NULL) {
	    *ep = '\0';
	}
	evlog->lines = (int)sudo_strtonum(cp, 1, INT_MAX, &errstr);
	if (errstr != NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"%s: tty lines %s: %s", iolog_dir, cp, errstr);
	}
	if (ep != NULL) {
	    cp = ep + 1;
	    evlog->columns = (int)sudo_strtonum(cp, 1, INT_MAX, &errstr);
	    if (errstr != NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "%s: tty cols %s: %s", iolog_dir, cp, errstr);
	    }
	}
    }

    ret = true;

done:
    free(buf);
    debug_return_bool(ret);
}
