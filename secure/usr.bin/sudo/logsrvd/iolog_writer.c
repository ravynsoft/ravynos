/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_fatal.h>
#include <sudo_queue.h>
#include <sudo_util.h>

#include <logsrvd.h>

static bool
type_matches(InfoMessage *info, const char *source,
    InfoMessage__ValueCase value_case)
{
    const void *val = info->u.strval;	/* same for strlistval */
    debug_decl(type_matches, SUDO_DEBUG_UTIL);

    if (info->key == NULL) {
	sudo_warnx(U_("%s: protocol error: NULL key"), source);
	debug_return_bool(false);
    }
    if (info->value_case != value_case) {
	sudo_warnx(U_("%s: protocol error: wrong type for %s"),
	    source, info->key);
	debug_return_bool(false);
    }
    if (value_case != INFO_MESSAGE__VALUE_NUMVAL && val == NULL) {
	sudo_warnx(U_("%s: protocol error: NULL value found in %s"),
	    source, info->key);
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Copy the specified string list.
 * The input string list need not be NULL-terminated.
 * Returns a NULL-terminated string vector.
 */
static char **
strlist_copy(InfoMessage__StringList *strlist)
{
    char **dst, **src = strlist->strings;
    size_t i, len = strlist->n_strings;
    debug_decl(strlist_copy, SUDO_DEBUG_UTIL);

    dst = reallocarray(NULL, len + 1, sizeof(char *));
    if (dst == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    for (i = 0; i < len; i++) {
	if ((dst[i] = strdup(src[i])) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
    }
    dst[i] = NULL;
    debug_return_ptr(dst);

bad:
    if (dst != NULL) {
	while (i)
	    free(dst[--i]);
	free(dst);
    }
    debug_return_ptr(NULL);
}

/*
 * Fill in eventlog details from an AcceptMessage
 * Caller is responsible for freeing strings in struct eventlog.
 * Returns true on success and false on failure.
 */
struct eventlog *
evlog_new(TimeSpec *submit_time, InfoMessage **info_msgs, size_t infolen,
    struct connection_closure *closure)
{
    const char *source = closure->journal_path ? closure->journal_path :
	closure->ipaddr;
    struct eventlog *evlog;
    unsigned char uuid[16];
    size_t idx;
    debug_decl(evlog_new, SUDO_DEBUG_UTIL);

    evlog = calloc(1, sizeof(*evlog));
    if (evlog == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }

    /* Create a UUID to store in the event log. */
    sudo_uuid_create(uuid);
    if (sudo_uuid_to_string(uuid, evlog->uuid_str, sizeof(evlog->uuid_str)) == NULL) {
       sudo_warnx("%s", U_("unable to generate UUID"));
       goto bad;
    }

    /* Client/peer IP address. */
    if ((evlog->peeraddr = strdup(closure->ipaddr)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }

    /* Submit time. */
    if (submit_time != NULL) {
	evlog->submit_time.tv_sec = (time_t)submit_time->tv_sec;
	evlog->submit_time.tv_nsec = (long)submit_time->tv_nsec;
    }

    /* Default values */
    evlog->lines = 24;
    evlog->columns = 80;
    evlog->runuid = (uid_t)-1;
    evlog->rungid = (gid_t)-1;
    evlog->exit_value = -1;

    /* Pull out values by key from info array. */
    for (idx = 0; idx < infolen; idx++) {
	InfoMessage *info = info_msgs[idx];
	const char *key = info->key;
	switch (key[0]) {
	case 'c':
	    if (strcmp(key, "columns") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_NUMVAL)) {
		    if (info->u.numval <= 0 || info->u.numval > INT_MAX) {
			errno = ERANGE;
			sudo_warn(U_("%s: %s"), source, "columns");
		    } else {
			evlog->columns = (int)info->u.numval;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "command") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->command = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    break;
	case 'l':
	    if (strcmp(key, "lines") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_NUMVAL)) {
		    if (info->u.numval <= 0 || info->u.numval > INT_MAX) {
			errno = ERANGE;
			sudo_warn(U_("%s: %s"), source, "lines");
		    } else {
			evlog->lines = (int)info->u.numval;
		    }
		}
		continue;
	    }
	    break;
	case 'r':
	    if (strcmp(key, "runargv") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRLISTVAL)) {
		    evlog->runargv = strlist_copy(info->u.strlistval);
		    if (evlog->runargv == NULL)
			goto bad;
		}
		continue;
	    }
	    if (strcmp(key, "runchroot") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->runchroot = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "runcwd") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->runcwd = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "runenv") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRLISTVAL)) {
		    evlog->runenv = strlist_copy(info->u.strlistval);
		    if (evlog->runenv == NULL)
			goto bad;
		}
		continue;
	    }
	    if (strcmp(key, "rungid") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_NUMVAL)) {
		    if (info->u.numval < 0 || info->u.numval > UINT_MAX) {
			errno = ERANGE;
			sudo_warn(U_("%s: %s"), source, "rungid");
		    } else {
			evlog->rungid = (gid_t)info->u.numval;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "rungroup") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->rungroup = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "runuid") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_NUMVAL)) {
		    if (info->u.numval < 0 || info->u.numval > UINT_MAX) {
			errno = ERANGE;
			sudo_warn(U_("%s: %s"), source, "runuid");
		    } else {
			evlog->runuid = (uid_t)info->u.numval;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "runuser") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->runuser = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    break;
	case 's':
	    if (strcmp(key, "source") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->source = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "submitcwd") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->cwd = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "submitenv") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRLISTVAL)) {
		    evlog->submitenv = strlist_copy(info->u.strlistval);
		    if (evlog->submitenv == NULL)
			goto bad;
		}
		continue;
	    }
	    if (strcmp(key, "submitgroup") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->submitgroup = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "submithost") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->submithost = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    if (strcmp(key, "submituser") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->submituser = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    break;
	case 't':
	    if (strcmp(key, "ttyname") == 0) {
		if (type_matches(info, source, INFO_MESSAGE__VALUE_STRVAL)) {
		    if ((evlog->ttyname = strdup(info->u.strval)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			goto bad;
		    }
		}
		continue;
	    }
	    break;
	}
    }

    /* Check for required settings */
    if (evlog->submituser == NULL) {
	sudo_warnx(U_("%s: protocol error: %s missing from AcceptMessage"),
	    source, "submituser");
	goto bad;
    }
    if (evlog->submithost == NULL) {
	sudo_warnx(U_("%s: protocol error: %s missing from AcceptMessage"),
	    source, "submithost");
	goto bad;
    }
    if (evlog->runuser == NULL) {
	sudo_warnx(U_("%s: protocol error: %s missing from AcceptMessage"),
	    source, "runuser");
	goto bad;
    }
    if (evlog->command == NULL) {
	sudo_warnx(U_("%s: protocol error: %s missing from AcceptMessage"),
	    source, "command");
	goto bad;
    }

    /* Other settings that must exist for event logging. */
    if (evlog->cwd == NULL) {
	if ((evlog->cwd = strdup("unknown")) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
    }
    if (evlog->runcwd == NULL) {
	if ((evlog->runcwd = strdup(evlog->cwd)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
    }
    if (evlog->submitgroup == NULL) {
	/* TODO: make submitgroup required */
	if ((evlog->submitgroup = strdup("unknown")) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
    }
    if (evlog->ttyname == NULL) {
	if ((evlog->ttyname = strdup("unknown")) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
    }

    debug_return_ptr(evlog);

bad:
    eventlog_free(evlog);
    debug_return_ptr(NULL);
}

struct iolog_path_closure {
    char *iolog_dir;
    struct eventlog *evlog;
};

static size_t
fill_seq(char *str, size_t strsize, void *v)
{
    struct iolog_path_closure *closure = v;
    char *sessid = closure->evlog->sessid;
    int len;
    debug_decl(fill_seq, SUDO_DEBUG_UTIL);

    if (sessid[0] == '\0') {
	if (!iolog_nextid(closure->iolog_dir, sessid))
	    debug_return_size_t((size_t)-1);
    }

    /* Path is of the form /var/log/sudo-io/00/00/01. */
    len = snprintf(str, strsize, "%c%c/%c%c/%c%c", sessid[0],
	sessid[1], sessid[2], sessid[3], sessid[4], sessid[5]);
    if (len < 0 || len >= (ssize_t)strsize) {
	sudo_warnx(U_("%s: unable to format session id"), __func__);
	debug_return_size_t(strsize); /* handle non-standard snprintf() */
    }
    debug_return_size_t((size_t)len);
}

static size_t
fill_user(char * restrict str, size_t strsize, void * restrict v)
{
    struct iolog_path_closure *closure = v;
    const struct eventlog *evlog = closure->evlog;
    debug_decl(fill_user, SUDO_DEBUG_UTIL);

    if (evlog->submituser == NULL) {
	sudo_warnx(U_("%s: %s is not set"), __func__, "submituser");
	debug_return_size_t(strsize);
    }
    debug_return_size_t(strlcpy(str, evlog->submituser, strsize));
}

static size_t
fill_group(char * restrict str, size_t strsize, void * restrict v)
{
    struct iolog_path_closure *closure = v;
    const struct eventlog *evlog = closure->evlog;
    debug_decl(fill_group, SUDO_DEBUG_UTIL);

    if (evlog->submitgroup == NULL) {
	sudo_warnx(U_("%s: %s is not set"), __func__, "submitgroup");
	debug_return_size_t(strsize);
    }
    debug_return_size_t(strlcpy(str, evlog->submitgroup, strsize));
}

static size_t
fill_runas_user(char * restrict str, size_t strsize, void * restrict v)
{
    struct iolog_path_closure *closure = v;
    const struct eventlog *evlog = closure->evlog;
    debug_decl(fill_runas_user, SUDO_DEBUG_UTIL);

    if (evlog->runuser == NULL) {
	sudo_warnx(U_("%s: %s is not set"), __func__, "runuser");
	debug_return_size_t(strsize);
    }
    debug_return_size_t(strlcpy(str, evlog->runuser, strsize));
}

static size_t
fill_runas_group(char * restrict str, size_t strsize, void * restrict v)
{
    struct iolog_path_closure *closure = v;
    const struct eventlog *evlog = closure->evlog;
    debug_decl(fill_runas_group, SUDO_DEBUG_UTIL);

    /* FIXME: rungroup not guaranteed to be set */
    if (evlog->rungroup == NULL) {
	sudo_warnx(U_("%s: %s is not set"), __func__, "rungroup");
	debug_return_size_t(strsize);
    }
    debug_return_size_t(strlcpy(str, evlog->rungroup, strsize));
}

static size_t
fill_hostname(char * restrict str, size_t strsize, void * restrict v)
{
    struct iolog_path_closure *closure = v;
    const struct eventlog *evlog = closure->evlog;
    debug_decl(fill_hostname, SUDO_DEBUG_UTIL);

    if (evlog->submithost == NULL) {
	sudo_warnx(U_("%s: %s is not set"), __func__, "submithost");
	debug_return_size_t(strsize);
    }
    debug_return_size_t(strlcpy(str, evlog->submithost, strsize));
}

static size_t
fill_command(char * restrict str, size_t strsize, void * restrict v)
{
    struct iolog_path_closure *closure = v;
    const struct eventlog *evlog = closure->evlog;
    debug_decl(fill_command, SUDO_DEBUG_UTIL);

    if (evlog->command == NULL) {
	sudo_warnx(U_("%s: %s is not set"), __func__, "command");
	debug_return_size_t(strsize);
    }
    debug_return_size_t(strlcpy(str, evlog->command, strsize));
}

/* Note: "seq" must be first in the list. */
static const struct iolog_path_escape path_escapes[] = {
    { "seq", fill_seq },
    { "user", fill_user },
    { "group", fill_group },
    { "runas_user", fill_runas_user },
    { "runas_group", fill_runas_group },
    { "hostname", fill_hostname },
    { "command", fill_command },
    { NULL, NULL }
};

/*
 * Create I/O log path
 * Sets iolog_path, iolog_file and iolog_dir_fd in the closure
 */
static bool
create_iolog_path(struct connection_closure *closure)
{
    struct eventlog *evlog = closure->evlog;
    struct iolog_path_closure path_closure;
    char expanded_dir[PATH_MAX], expanded_file[PATH_MAX], pathbuf[PATH_MAX];
    int len;
    debug_decl(create_iolog_path, SUDO_DEBUG_UTIL);

    path_closure.evlog = evlog;
    path_closure.iolog_dir = expanded_dir;

    if (!expand_iolog_path(logsrvd_conf_iolog_dir(), expanded_dir,
	    sizeof(expanded_dir), &path_escapes[1], &path_closure)) {
	sudo_warnx(U_("unable to expand iolog path %s"),
	    logsrvd_conf_iolog_dir());
	goto bad;
    }

    if (!expand_iolog_path(logsrvd_conf_iolog_file(), expanded_file,
	    sizeof(expanded_file), &path_escapes[0], &path_closure)) {
	sudo_warnx(U_("unable to expand iolog path %s"),
	    logsrvd_conf_iolog_file());
	goto bad;
    }

    len = snprintf(pathbuf, sizeof(pathbuf), "%s/%s", expanded_dir,
	expanded_file);
    if (len < 0 || len >= ssizeof(pathbuf)) {
	errno = ENAMETOOLONG;
	sudo_warn("%s/%s", expanded_dir, expanded_file);
	goto bad;
    }

    /*
     * Create log path, along with any intermediate subdirs.
     * Calls mkdtemp() if pathbuf ends in XXXXXX.
     */
    if (!iolog_mkpath(pathbuf)) {
	sudo_warn(U_("unable to create iolog path %s"), pathbuf);
        goto bad;
    }
    if ((evlog->iolog_path = strdup(pathbuf)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    evlog->iolog_file = evlog->iolog_path + strlen(expanded_dir) + 1;

    /* We use iolog_dir_fd in calls to openat(2) */
    closure->iolog_dir_fd =
	iolog_openat(AT_FDCWD, evlog->iolog_path, O_RDONLY);
    if (closure->iolog_dir_fd == -1) {
	sudo_warn("%s", evlog->iolog_path);
	goto bad;
    }

    debug_return_bool(true);
bad:
    free(evlog->iolog_path);
    evlog->iolog_path = NULL;
    debug_return_bool(false);
}

bool
iolog_create(int iofd, struct connection_closure *closure)
{
    debug_decl(iolog_create, SUDO_DEBUG_UTIL);

    if (iofd < 0 || iofd >= IOFD_MAX) {
	sudo_warnx(U_("invalid iofd %d"), iofd);
	debug_return_bool(false);
    }

    closure->iolog_files[iofd].enabled = true;
    debug_return_bool(iolog_open(&closure->iolog_files[iofd],
	closure->iolog_dir_fd, iofd, "w"));
}

void
iolog_close_all(struct connection_closure *closure)
{
    const char *errstr;
    unsigned int i;
    debug_decl(iolog_close_all, SUDO_DEBUG_UTIL);

    for (i = 0; i < IOFD_MAX; i++) {
	if (!closure->iolog_files[i].enabled)
	    continue;
	if (!iolog_close(&closure->iolog_files[i], &errstr)) {
	    sudo_warnx(U_("error closing iofd %u: %s"), i, errstr);
	}
    }
    if (closure->iolog_dir_fd != -1)
	close(closure->iolog_dir_fd);

    debug_return;
}

bool
iolog_flush_all(struct connection_closure *closure)
{
    const char *errstr;
    bool ret = true;
    unsigned int i;
    debug_decl(iolog_flush_all, SUDO_DEBUG_UTIL);

    for (i = 0; i < IOFD_MAX; i++) {
	if (!closure->iolog_files[i].enabled)
	    continue;
	if (!iolog_flush(&closure->iolog_files[i], &errstr)) {
	    sudo_warnx(U_("error flushing iofd %u: %s"), i, errstr);
	    ret = false;
	}
    }

    debug_return_bool(ret);
}

bool
iolog_init(AcceptMessage *msg, struct connection_closure *closure)
{
    struct eventlog *evlog = closure->evlog;
    debug_decl(iolog_init, SUDO_DEBUG_UTIL);

    /* Create I/O log path */
    if (!create_iolog_path(closure))
	debug_return_bool(false);

    /* Write sudo I/O log info file */
    if (!iolog_write_info_file(closure->iolog_dir_fd, evlog))
	debug_return_bool(false);

    /*
     * Create timing, stdout, stderr and ttyout files for sudoreplay.
     * Others will be created on demand.
     */
    if (!iolog_create(IOFD_TIMING, closure) ||
	!iolog_create(IOFD_STDOUT, closure) ||
	!iolog_create(IOFD_STDERR, closure) ||
	!iolog_create(IOFD_TTYOUT, closure))
	debug_return_bool(false);

    /* Ready to log I/O buffers. */
    debug_return_bool(true);
}

/*
 * Copy len bytes from src to dst.
 */
static bool
iolog_copy(struct iolog_file *src, struct iolog_file *dst, off_t remainder,
    const char **errstr)
{
    char buf[64 * 1024];
    ssize_t nread;
    debug_decl(iolog_copy, SUDO_DEBUG_UTIL);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"copying %lld bytes", (long long)remainder);
    while (remainder > 0) {
	const size_t toread = MIN((size_t)remainder, sizeof(buf));
	nread = iolog_read(src, buf, toread, errstr);
	if (nread == -1)
	    debug_return_bool(false);
	remainder -= nread;

	do {
	    ssize_t nwritten = iolog_write(dst, buf, (size_t)nread, errstr);
	    if (nwritten == -1)
		debug_return_bool(false);
	    nread -= nwritten;
	} while (nread > 0);
    }

    debug_return_bool(true);
}

/*
 * Like rename(2) but changes UID as needed.
 */
static bool
iolog_rename(const char *from, const char *to)
{
    bool ok, uid_changed = false;
    debug_decl(iolog_rename, SUDO_DEBUG_UTIL);

    ok = rename(from, to) == 0;
    if (!ok && errno == EACCES) {
	uid_changed = iolog_swapids(false);
	if (uid_changed)
	    ok = rename(from, to) == 0;
    }

    if (uid_changed) {
	if (!iolog_swapids(true))
	    ok = false;
    }
    debug_return_bool(ok);
}

/* Compressed logs don't support random access, need to rewrite them. */
bool
iolog_rewrite(const struct timespec *target, struct connection_closure *closure)
{
    const struct eventlog *evlog = closure->evlog;
    struct iolog_file new_iolog_files[IOFD_MAX];
    off_t iolog_file_sizes[IOFD_MAX] = { 0 };
    struct timing_closure timing;
    int iofd, len, tmpdir_fd = -1;
    const char *name, *errstr;
    char tmpdir[PATH_MAX];
    bool ret = false;
    debug_decl(iolog_rewrite, SUDO_DEBUG_UTIL);

    memset(&timing, 0, sizeof(timing));
    timing.decimal = ".";

    /* Parse timing file until we reach the target point. */
    /* TODO: use iolog_seekto with a callback? */
    for (;;) {
	/* Read next record from timing file. */
	if (iolog_read_timing_record(&closure->iolog_files[IOFD_TIMING], &timing) != 0)
	    goto done;
	sudo_timespecadd(&timing.delay, &closure->elapsed_time,
	    &closure->elapsed_time);
	if (timing.event < IOFD_TIMING) {
	    if (!closure->iolog_files[timing.event].enabled) {
		/* Missing log file. */
		sudo_warnx(U_("invalid I/O log %s: %s referenced but not present"),
		    evlog->iolog_path, iolog_fd_to_name(timing.event));
		goto done;
	    }
	    iolog_file_sizes[timing.event] += (off_t)timing.u.nbytes;
	}

	if (sudo_timespeccmp(&closure->elapsed_time, target, >=)) {
	    if (sudo_timespeccmp(&closure->elapsed_time, target, ==))
		break;

	    /* Mismatch between resume point and stored log. */
	    sudo_warnx(U_("%s: unable to find resume point [%lld, %ld]"),
		evlog->iolog_path, (long long)target->tv_sec, target->tv_nsec);
	    goto done;
	}
    }
    iolog_file_sizes[IOFD_TIMING] =
	iolog_seek(&closure->iolog_files[IOFD_TIMING], 0, SEEK_CUR);
    iolog_rewind(&closure->iolog_files[IOFD_TIMING]);

    /* Create new I/O log files in a temporary directory. */
    len = snprintf(tmpdir, sizeof(tmpdir), "%s/restart.XXXXXX",
	evlog->iolog_path);
    if (len < 0 || len >= ssizeof(tmpdir)) {
	errno = ENAMETOOLONG;
	sudo_warn("%s/restart.XXXXXX", evlog->iolog_path);
	goto done;
    }
    if (!iolog_mkdtemp(tmpdir)) {
	sudo_warn(U_("unable to mkdir %s"), tmpdir);
	goto done;
    }
    if ((tmpdir_fd = iolog_openat(AT_FDCWD, tmpdir, O_RDONLY)) == -1) {
	sudo_warn(U_("unable to open %s"), tmpdir);
	goto done;
    }

    /* Create new copies of the existing iologs */
    memset(new_iolog_files, 0, sizeof(new_iolog_files));
    for (iofd = 0; iofd < IOFD_MAX; iofd++) {
	if (!closure->iolog_files[iofd].enabled)
	    continue;
	new_iolog_files[iofd].enabled = true;
	if (!iolog_open(&new_iolog_files[iofd], tmpdir_fd, iofd, "w")) {
	    if (errno != ENOENT) {
		sudo_warn(U_("unable to open %s/%s"),
		    tmpdir, iolog_fd_to_name(iofd));
		goto done;
	    }
	}
    }

    for (iofd = 0; iofd < IOFD_MAX; iofd++) {
	if (!closure->iolog_files[iofd].enabled)
	    continue;
	if (!iolog_copy(&closure->iolog_files[iofd], &new_iolog_files[iofd],
		iolog_file_sizes[iofd], &errstr)) {
	    name = iolog_fd_to_name(iofd);
	    sudo_warnx(U_("unable to copy %s/%s to %s/%s: %s"),
		evlog->iolog_path, name, tmpdir, name, errstr);
	    goto done;
	}
    }

    /* Move copied log files into place. */
    for (iofd = 0; iofd < IOFD_MAX; iofd++) {
	char from[PATH_MAX], to[PATH_MAX];

	if (!closure->iolog_files[iofd].enabled)
	    continue;

	/* This would be easier with renameat(2), old systems are annoying. */
	name = iolog_fd_to_name(iofd);
	len = snprintf(from, sizeof(from), "%s/%s", tmpdir, name);
	if (len < 0 || len >= ssizeof(from)) {
	    errno = ENAMETOOLONG;
	    sudo_warn("%s/%s", tmpdir, name);
	    goto done;
	}
	len = snprintf(to, sizeof(to), "%s/%s", evlog->iolog_path,
	    name);
	if (len < 0 || len >= ssizeof(from)) {
	    errno = ENAMETOOLONG;
	    sudo_warn("%s/%s", evlog->iolog_path, name);
	    goto done;
	}
	if (!iolog_rename(from, to)) {
	    sudo_warn(U_("unable to rename %s to %s"), from, to);
	    goto done;
	}
    }

    for (iofd = 0; iofd < IOFD_MAX; iofd++) {
	if (!closure->iolog_files[iofd].enabled)
	    continue;
	(void)iolog_close(&closure->iolog_files[iofd], &errstr);
	closure->iolog_files[iofd] = new_iolog_files[iofd];
	new_iolog_files[iofd].enabled = false;
    }

    /* Ready to log I/O buffers. */
    ret = true;
done:
    if (tmpdir_fd != -1) {
	if (!ret) {
	    for (iofd = 0; iofd < IOFD_MAX; iofd++) {
		if (!new_iolog_files[iofd].enabled)
		    continue;
		(void)iolog_close(&new_iolog_files[iofd], &errstr);
		(void)unlinkat(tmpdir_fd, iolog_fd_to_name(iofd), 0);
	    }
	}
	close(tmpdir_fd);
	(void)rmdir(tmpdir);
    }
    debug_return_bool(ret);
}

/*
 * Add given delta to elapsed time.
 * We cannot use timespecadd here since delta is not struct timespec.
 */
void
update_elapsed_time(TimeSpec *delta, struct timespec *elapsed)
{
    debug_decl(update_elapsed_time, SUDO_DEBUG_UTIL);

    /* Cannot use timespecadd since msg doesn't use struct timespec. */
    elapsed->tv_sec += (time_t)delta->tv_sec;
    elapsed->tv_nsec += (long)delta->tv_nsec;
    while (elapsed->tv_nsec >= 1000000000) {
	elapsed->tv_sec++;
	elapsed->tv_nsec -= 1000000000;
    }
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"%s: delta [%lld, %d], elapsed time now [%lld, %ld]",
	__func__, (long long)delta->tv_sec, delta->tv_nsec,
	(long long)elapsed->tv_sec, elapsed->tv_nsec);

    debug_return;
}
