/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1994-1996, 1998-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_json.h>
#include <sudo_queue.h>
#include <sudo_util.h>

static FILE *eventlog_stub_open_log(int type, const char *logfile);
static void eventlog_stub_close_log(int type, FILE *fp);

/* Eventlog config settings (default values). */
static struct eventlog_config evl_conf = {
    EVLOG_NONE,			/* type */
    EVLOG_SUDO,			/* format */
    LOG_NOTICE,			/* syslog_acceptpri */
    LOG_ALERT,			/* syslog_rejectpri */
    LOG_ALERT,			/* syslog_alertpri */
    MAXSYSLOGLEN,		/* syslog_maxlen */
    0,				/* file_maxlen */
    ROOT_UID,			/* mailuid */
    false,			/* omit_hostname */
    _PATH_SUDO_LOGFILE,		/* logpath */
    "%h %e %T",			/* time_fmt */
#ifdef _PATH_SUDO_SENDMAIL
    _PATH_SUDO_SENDMAIL,	/* mailerpath */
#else
    NULL,			/* mailerpath (disabled) */
#endif
    "-t",			/* mailerflags */
    NULL,			/* mailfrom */
    MAILTO,			/* mailto */
    N_(MAILSUBJECT),		/* mailsub */
    eventlog_stub_open_log,	/* open_log */
    eventlog_stub_close_log	/* close_log */
};

static FILE *
eventlog_stub_open_log(int type, const char *logfile)
{
    debug_decl(eventlog_stub_open_log, SUDO_DEBUG_UTIL);
    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	"open_log not set, using stub");
    debug_return_ptr(NULL);
}

static void
eventlog_stub_close_log(int type, FILE *fp)
{
    debug_decl(eventlog_stub_close_log, SUDO_DEBUG_UTIL);
    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	"close_log not set, using stub");
    debug_return;
}

/*
 * eventlog config setters.
 */

void
eventlog_set_type(int type)
{
    evl_conf.type = type;
}

void
eventlog_set_format(enum eventlog_format format)
{
    evl_conf.format = format;
}

void
eventlog_set_syslog_acceptpri(int pri)
{
    evl_conf.syslog_acceptpri = pri;
}

void
eventlog_set_syslog_rejectpri(int pri)
{
    evl_conf.syslog_rejectpri = pri;
}

void
eventlog_set_syslog_alertpri(int pri)
{
    evl_conf.syslog_alertpri = pri;
}

void
eventlog_set_syslog_maxlen(size_t len)
{
    evl_conf.syslog_maxlen = len;
}

void
eventlog_set_file_maxlen(size_t len)
{
    evl_conf.file_maxlen = len;
}

void
eventlog_set_mailuid(uid_t uid)
{
    evl_conf.mailuid = uid;
}

void
eventlog_set_omit_hostname(bool omit_hostname)
{
    evl_conf.omit_hostname = omit_hostname;
}

void
eventlog_set_logpath(const char *path)
{
    evl_conf.logpath = path;
}

void
eventlog_set_time_fmt(const char *fmt)
{
    evl_conf.time_fmt = fmt;
}

void
eventlog_set_mailerpath(const char *path)
{
    evl_conf.mailerpath = path;
}

void
eventlog_set_mailerflags(const char *mflags)
{
    evl_conf.mailerflags = mflags;
}

void
eventlog_set_mailfrom(const char *from_addr)
{
    evl_conf.mailfrom = from_addr;
}

void
eventlog_set_mailto(const char *to_addr)
{
    evl_conf.mailto = to_addr;
}

void
eventlog_set_mailsub(const char *subject)
{
    evl_conf.mailsub = subject;
}

void
eventlog_set_open_log(FILE *(*fn)(int type, const char *))
{
    evl_conf.open_log = fn;
}

void
eventlog_set_close_log(void (*fn)(int type, FILE *))
{
    evl_conf.close_log = fn;
}

/*
 * get eventlog config.
 */
const struct eventlog_config *
eventlog_getconf(void)
{
    return &evl_conf;
}
