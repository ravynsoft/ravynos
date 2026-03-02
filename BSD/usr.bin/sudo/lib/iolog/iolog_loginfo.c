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
#include <unistd.h>
#include <fcntl.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_json.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

struct eventlog *
iolog_parse_loginfo(int dfd, const char *iolog_dir)
{
    struct eventlog *evlog = NULL;
    FILE *fp = NULL;
    int fd = -1;
    int tmpfd = -1;
    bool ok, legacy = false;
    debug_decl(iolog_parse_loginfo, SUDO_DEBUG_UTIL);

    if (dfd == -1) {
	if ((tmpfd = open(iolog_dir, O_RDONLY)) == -1) {
	    sudo_warn("%s", iolog_dir);
	    goto bad;
	}
	dfd = tmpfd;
    }
    if ((fd = openat(dfd, "log.json", O_RDONLY, 0)) == -1) {
	fd = openat(dfd, "log", O_RDONLY, 0);
	legacy = true;
    }
    if (tmpfd != -1)
	close(tmpfd);
    if (fd == -1 || (fp = fdopen(fd, "r")) == NULL) {
	sudo_warn("%s/log", iolog_dir);
	goto bad;
    }
    fd = -1;

    if ((evlog = calloc(1, sizeof(*evlog))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    evlog->runuid = (uid_t)-1;
    evlog->rungid = (gid_t)-1;
    evlog->exit_value = -1;

    ok = legacy ? iolog_parse_loginfo_legacy(fp, iolog_dir, evlog) :
	iolog_parse_loginfo_json(fp, iolog_dir, evlog);
    if (ok) {
	fclose(fp);
	debug_return_ptr(evlog);
    }

bad:
    if (fd != -1)
	close(fd);
    if (fp != NULL)
	fclose(fp);
    eventlog_free(evlog);
    debug_return_ptr(NULL);
}

/*
 * Write the legacy I/O log file that contains the user and command info.
 * This file is not compressed.
 */
static bool
iolog_write_info_file_legacy(int dfd, struct eventlog *evlog)
{
    char * const *av;
    FILE *fp;
    int error, fd;
    debug_decl(iolog_info_write_log, SUDO_DEBUG_UTIL);

    fd = iolog_openat(dfd, "log", O_CREAT|O_TRUNC|O_WRONLY);
    if (fd == -1 || (fp = fdopen(fd, "w")) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to %sopen %s/log", fd == -1 ? "" : "fd", evlog->iolog_path);
	if (fd != -1)
	    close(fd);
	debug_return_bool(false);
    }
    if (fchown(fd, iolog_get_uid(), iolog_get_gid()) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to fchown %d:%d %s/log", __func__,
	    (int)iolog_get_uid(), (int)iolog_get_gid(), evlog->iolog_path);
    }

    fprintf(fp, "%lld:%s:%s:%s:%s:%d:%d\n%s\n",
	(long long)evlog->submit_time.tv_sec,
	evlog->submituser ? evlog->submituser : "unknown",
	evlog->runuser ? evlog->runuser : RUNAS_DEFAULT,
	evlog->rungroup ? evlog->rungroup : "",
	evlog->ttyname ? evlog->ttyname : "unknown",
	evlog->lines, evlog->columns,
	evlog->cwd ? evlog->cwd : "unknown");
    fputs(evlog->command ? evlog->command : "unknown", fp);
    for (av = evlog->runargv + 1; *av != NULL; av++) {
	fputc(' ', fp);
	fputs(*av, fp);
    }
    fputc('\n', fp);
    fflush(fp);
    if ((error = ferror(fp))) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to write to I/O log file %s/log", evlog->iolog_path);
    }
    fclose(fp);

    debug_return_bool(!error);
}

/*
 * Write the "log.json" file that contains the user and command info.
 * This file is not compressed.
 */
static bool
iolog_write_info_file_json(int dfd, struct eventlog *evlog)
{
    struct json_container jsonc;
    struct json_value json_value;
    bool ret = false;
    FILE *fp = NULL;
    int fd = -1;
    debug_decl(iolog_write_info_file_json, SUDO_DEBUG_UTIL);

    if (!sudo_json_init(&jsonc, 4, false, false, false))
	debug_return_bool(false);

    /* Timestamp */
    if (!sudo_json_open_object(&jsonc, "timestamp"))
	goto oom;

    json_value.type = JSON_NUMBER;
    json_value.u.number = evlog->submit_time.tv_sec;
    if (!sudo_json_add_value(&jsonc, "seconds", &json_value))
        goto oom;

    json_value.type = JSON_NUMBER;
    json_value.u.number = evlog->submit_time.tv_nsec;
    if (!sudo_json_add_value(&jsonc, "nanoseconds", &json_value))
        goto oom;

    if (!sudo_json_close_object(&jsonc))
	goto oom;

    if (!eventlog_store_json(&jsonc, evlog))
	goto done;

    fd = iolog_openat(dfd, "log.json", O_CREAT|O_TRUNC|O_WRONLY);
    if (fd == -1 || (fp = fdopen(fd, "w")) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to %sopen %s/log.json", fd == -1 ? "" : "fd",
	    evlog->iolog_path);
        goto done;
    }
    if (fchown(fd, iolog_get_uid(), iolog_get_gid()) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to fchown %d:%d %s/log.json", __func__,
	    (int)iolog_get_uid(), (int)iolog_get_gid(), evlog->iolog_path);
    }
    fd = -1;

    fprintf(fp, "{%s\n}\n", sudo_json_get_buf(&jsonc));
    fflush(fp);
    if (ferror(fp)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
            "unable to write to I/O log file %s/log.json", evlog->iolog_path);
	goto done;
    }

    ret = true;
    goto done;

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
done:
    sudo_json_free(&jsonc);
    if (fp != NULL)
	fclose(fp);
    if (fd != -1)
	close(fd);

    debug_return_bool(ret);
}

/*
 * Write the I/O log and log.json files that contain user and command info.
 * These files are not compressed.
 */
bool
iolog_write_info_file(int dfd, struct eventlog *evlog)
{
    debug_decl(iolog_write_info_file, SUDO_DEBUG_UTIL);

    if (!iolog_write_info_file_legacy(dfd, evlog))
	debug_return_bool(false);
    if (!iolog_write_info_file_json(dfd, evlog))
	debug_return_bool(false);

    debug_return_bool(true);
}
