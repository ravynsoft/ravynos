/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudoers.h>
#include <sudo_eventlog.h>
#include <sudo_iolog.h>
#include <sudo_plugin.h>

#include <def_data.c>		/* for iolog_path.c */

extern struct io_plugin sudoers_io;

sudo_printf_t sudo_printf;
sudo_conv_t sudo_conv;
struct sudo_plugin_event * (*plugin_event_alloc)(void);

static struct sudoers_context io_ctx = SUDOERS_CONTEXT_INITIALIZER;

sudo_dso_public int main(int argc, char *argv[], char *envp[]);

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v] pathname\n", getprogname());
    exit(EXIT_FAILURE);
}

static int
sudo_printf_int(int msg_type, const char * restrict fmt, ...)
{
    va_list ap;
    int len;

    switch (msg_type) {
    case SUDO_CONV_INFO_MSG:
	va_start(ap, fmt);
	len = vfprintf(stdout, fmt, ap);
	va_end(ap);
	break;
    case SUDO_CONV_ERROR_MSG:
	va_start(ap, fmt);
	len = vfprintf(stderr, fmt, ap);
	va_end(ap);
	break;
    default:
	len = -1;
	errno = EINVAL;
	break;
    }

    return len;
}

static bool
validate_iolog_info(const char *log_dir, bool legacy)
{
    struct eventlog *evlog;
    time_t now;

    time(&now);

    /* Parse log file. */
    if ((evlog = iolog_parse_loginfo(-1, log_dir)) == NULL)
	return false;

    if (evlog->cwd == NULL || strcmp(evlog->cwd, "/") != 0) {
	sudo_warnx("bad cwd: want \"/\", got \"%s\"",
	    evlog->cwd ? evlog->cwd : "NULL");
	return false;
    }

    /* No host in the legacy log file. */
    if (!legacy) {
	if (evlog->submithost == NULL || strcmp(evlog->submithost, "localhost") != 0) {
	    sudo_warnx("bad host: want \"localhost\", got \"%s\"",
		evlog->submithost ? evlog->submithost : "NULL");
	    return false;
	}
    }

    if (evlog->submituser == NULL || strcmp(evlog->submituser, "nobody") != 0) {
	sudo_warnx("bad user: want \"nobody\" got \"%s\"",
	    evlog->submituser ? evlog->submituser : "NULL");
	return false;
    }

    if (evlog->runuser == NULL || strcmp(evlog->runuser, "root") != 0) {
	sudo_warnx("bad runuser: want \"root\" got \"%s\"",
	    evlog->runuser ? evlog->runuser : "NULL");
	return false;
    }

    /* No runas group specified, should be NULL. */
    if (evlog->rungroup != NULL) {
	sudo_warnx("bad rungroup: want \"\" got \"%s\"", evlog->rungroup);
	return false;
    }

    if (evlog->ttyname == NULL || strcmp(evlog->ttyname, "/dev/console") != 0) {
	sudo_warnx("bad tty: want \"/dev/console\" got \"%s\"",
	    evlog->ttyname ? evlog->ttyname : "NULL");
	return false;
    }

    if (evlog->command == NULL || strcmp(evlog->command, "/usr/bin/id") != 0) {
	sudo_warnx("bad command: want \"/usr/bin/id\" got \"%s\"",
	    evlog->command ? evlog->command : "NULL");
	return false;
    }

    if (evlog->lines != 24) {
	sudo_warnx("bad lines: want 24 got %d", evlog->lines);
	return false;
    }

    if (evlog->columns != 80) {
	sudo_warnx("bad columns: want 80 got %d", evlog->columns);
	return false;
    }

    if (evlog->submit_time.tv_sec < now - 10 || evlog->submit_time.tv_sec > now + 10) {
	sudo_warnx("bad submit_time: want %lld got %lld", (long long)now,
	    (long long)evlog->submit_time.tv_sec);
	return false;
    }

    eventlog_free(evlog);

    return true;
}

static bool
validate_timing(FILE *fp, int recno, int type, unsigned int p1, unsigned int p2)
{
    struct timing_closure timing;
    char buf[LINE_MAX];

    if (!fgets(buf, sizeof(buf), fp)) {
	sudo_warn("unable to read timing file");
	return false;
    }
    buf[strcspn(buf, "\n")] = '\0';
    if (!iolog_parse_timing(buf, &timing)) {
	sudo_warnx("invalid timing file line: %s", buf);
	return false;
    }
    if (timing.event != type) {
	sudo_warnx("record %d: want type %d, got type %d", recno, type,
	    timing.event);
	return false;
    }
    if (type == IO_EVENT_WINSIZE) {
	if (timing.u.winsize.lines != (int)p1) {
	    sudo_warnx("record %d: want %u lines, got %u", recno, p1,
		timing.u.winsize.lines);
	    return false;
	}
	if (timing.u.winsize.cols != (int)p2) {
	    sudo_warnx("record %d: want %u cols, got %u", recno, p2,
		timing.u.winsize.cols);
	    return false;
	}
    } else {
	if (timing.u.nbytes != p1) {
	    sudo_warnx("record %d: want len %u, got type %zu", recno, p1,
		timing.u.nbytes);
	    return false;
	}
    }
    if (timing.delay.tv_sec != 0) {
	sudo_warnx("record %d: got excessive delay %lld.%09ld", recno,
	    (long long)timing.delay.tv_sec, timing.delay.tv_nsec);
	return false;
    }

    return true;
}


/*
 * Test sudoers I/O log plugin endpoints.
 */
static void
test_endpoints(const struct sudoers_context *ctx, int *ntests, int *nerrors,
    const char *iolog_dir, char *envp[])
{
    int rc, cmnd_argc = 1;
    const char *errstr = NULL;
    char buf[1024], iolog_path[PATH_MAX];
    char runas_gid[64], runas_uid[64];
    FILE *fp;
    const char *cmnd_argv[] = {
	"/usr/bin/id",
	NULL
    };
    const char *user_info[] = {
	"cols=80",
	"lines=24",
	"cwd=/",
	"host=localhost",
	"tty=/dev/console",
	"user=nobody",
	NULL
    };
    const char *command_info[] = {
	"command=/usr/bin/id",
	iolog_path,
	"iolog_stdin=true",
	"iolog_stdout=true",
	"iolog_stderr=true",
	"iolog_ttyin=true",
	"iolog_ttyout=true",
	"iolog_compress=false",
	"iolog_mode=0644",
	runas_gid,
	runas_uid,
	NULL
    };
    char *settings[] = {
	NULL
    };
    const char output[] = "uid=0(root) gid=0(wheel)\r\n";
    const unsigned int outlen = sizeof(output) - 1;

    /* Set runas uid/gid to root. */
    snprintf(runas_uid, sizeof(runas_uid), "runas_uid=%u",
	(unsigned int)ctx->runas.pw->pw_uid);
    snprintf(runas_gid, sizeof(runas_gid), "runas_gid=%u",
	(unsigned int)ctx->runas.pw->pw_gid);

    /* Set path to the iolog directory the user passed in. */
    snprintf(iolog_path, sizeof(iolog_path), "iolog_path=%s", iolog_dir);

    /* Test open endpoint. */
    rc = sudoers_io.open(SUDO_API_VERSION, NULL, sudo_printf_int, settings,
	(char **)user_info, (char **)command_info, cmnd_argc,
	(char **)cmnd_argv, envp, NULL, &errstr);
    (*ntests)++;
    if (rc != 1) {
	sudo_warnx("I/O log open endpoint failed");
	(*nerrors)++;
	return;
    }

    /* Test log_ttyout endpoint. */
    rc = sudoers_io.log_ttyout(output, outlen, &errstr);
    (*ntests)++;
    if (rc != 1) {
	sudo_warnx("I/O log_ttyout endpoint failed");
	(*nerrors)++;
	return;
    }

    /* Test change_winsize endpoint (twice). */
    rc = sudoers_io.change_winsize(32, 128, &errstr);
    (*ntests)++;
    if (rc != 1) {
	sudo_warnx("I/O change_winsize endpoint failed");
	(*nerrors)++;
	return;
    }
    rc = sudoers_io.change_winsize(24, 80, &errstr);
    (*ntests)++;
    if (rc != 1) {
	sudo_warnx("I/O change_winsize endpoint failed");
	(*nerrors)++;
	return;
    }

    /* Close the plugin. */
    sudoers_io.close(0, 0);

    /* Validate I/O log info file (json). */
    (*ntests)++;
    if (!validate_iolog_info(iolog_dir, false))
	(*nerrors)++;

    /* Validate I/O log info file (legacy). */
    snprintf(iolog_path, sizeof(iolog_path), "%s/log.json", iolog_dir);
    unlink(iolog_path);
    (*ntests)++;
    if (!validate_iolog_info(iolog_dir, true))
	(*nerrors)++;

    /* Validate the timing file. */
    snprintf(iolog_path, sizeof(iolog_path), "%s/timing", iolog_dir);
    (*ntests)++;
    if ((fp = fopen(iolog_path, "r")) == NULL) {
	sudo_warn("unable to open %s", iolog_path);
	(*nerrors)++;
	return;
    }

    /* Line 1: output of id command. */
    if (!validate_timing(fp, 1, IO_EVENT_TTYOUT, outlen, 0)) {
	(*nerrors)++;
	return;
    }

    /* Line 2: window size change. */
    if (!validate_timing(fp, 2, IO_EVENT_WINSIZE, 32, 128)) {
	(*nerrors)++;
	return;
    }

    /* Line 3: window size change. */
    if (!validate_timing(fp, 3, IO_EVENT_WINSIZE, 24, 80)) {
	(*nerrors)++;
	return;
    }

    /* Validate ttyout log file. */
    snprintf(iolog_path, sizeof(iolog_path), "%s/ttyout", iolog_dir);
    (*ntests)++;
    fclose(fp);
    if ((fp = fopen(iolog_path, "r")) == NULL) {
	sudo_warn("unable to open %s", iolog_path);
	(*nerrors)++;
	return;
    }
    if (!fgets(buf, sizeof(buf), fp)) {
	sudo_warn("unable to read %s", iolog_path);
	(*nerrors)++;
	return;
    }
    if (strcmp(buf, output) != 0) {
	sudo_warnx("ttylog mismatch: want \"%s\", got \"%s\"", output, buf);
	(*nerrors)++;
	return;
    }
}

int
main(int argc, char *argv[], char *envp[])
{
    struct passwd *tpw;
    int ch, tests = 0, errors = 0;
    const char *iolog_dir;

    initprogname(argc > 0 ? argv[0] : "check_iolog_plugin");

    while ((ch = getopt(argc, argv, "v")) != -1) {
        switch (ch) {
        case 'v':
            /* ignored */
            break;
        default:
            usage();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1)
	usage();
    iolog_dir = argv[0];

    /* Set runas user. */
    if ((tpw = getpwuid(0)) == NULL) {
	if ((tpw = getpwnam("root")) == NULL)
	    sudo_fatalx("unable to look up uid 0 or root");
    }
    io_ctx.runas.pw = pw_dup(tpw);

    /* Set invoking user. */
    if ((tpw = getpwuid(geteuid())) == NULL)
	sudo_fatalx("unable to look up invoking user's uid");
    io_ctx.user.pw = pw_dup(tpw);

    /* Set iolog uid/gid to invoking user. */
    iolog_set_owner(io_ctx.user.pw->pw_uid, io_ctx.user.pw->pw_gid);

    test_endpoints(&io_ctx, &tests, &errors, iolog_dir, envp);

    if (tests != 0) {
	printf("check_iolog_plugin: %d test%s run, %d errors, %d%% success rate\n",
	    tests, tests == 1 ? "" : "s", errors,
	    (tests - errors) * 100 / tests);
    }

    exit(errors);
}

/* Stub functions */

bool
set_perms(const struct sudoers_context *ctx, int perm)
{
    return true;
}

bool
restore_perms(void)
{
    return true;
}

bool
log_warning(const struct sudoers_context *ctx, unsigned int flags,
    const char * restrict fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    sudo_vwarn_nodebug(fmt, ap);
    va_end(ap);

    return true;
}

bool
log_warningx(const struct sudoers_context *ctx, unsigned int flags,
    const char * restrict fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    sudo_vwarnx_nodebug(fmt, ap);
    va_end(ap);

    return true;
}

const struct sudoers_context *
sudoers_get_context(void)
{
    return &io_ctx;
}
