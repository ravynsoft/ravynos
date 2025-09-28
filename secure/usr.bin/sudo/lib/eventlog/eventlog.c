/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1994-1996, 1998-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sudo_lbuf.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_json.h>
#include <sudo_queue.h>
#include <sudo_util.h>

#define IS_SESSID(s) ( \
    isalnum((unsigned char)(s)[0]) && isalnum((unsigned char)(s)[1]) && \
    (s)[2] == '/' && \
    isalnum((unsigned char)(s)[3]) && isalnum((unsigned char)(s)[4]) && \
    (s)[5] == '/' && \
    isalnum((unsigned char)(s)[6]) && isalnum((unsigned char)(s)[7]) && \
    (s)[8] == '\0')

struct eventlog_args {
    const char *reason;
    const char *errstr;
    const struct timespec *event_time;
    eventlog_json_callback_t json_info_cb;
    void *json_info;
};

/*
 * Allocate and fill in a new logline.
 */
static bool
new_logline(int event_type, int flags, struct eventlog_args *args,
    const struct eventlog *evlog, struct sudo_lbuf *lbuf)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    const char *iolog_file;
    const char *tty, *tsid = NULL;
    char exit_str[STRLEN_MAX_SIGNED(int) + 1];
    char sessid[7], offsetstr[64] = "";
    size_t i;
    debug_decl(new_logline, SUDO_DEBUG_UTIL);

    if (ISSET(flags, EVLOG_RAW) || evlog == NULL) {
	if (args->reason != NULL) {
	    if (args->errstr != NULL) {
		sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "%s: %s",
		    args->reason, args->errstr);
	    } else {
		sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "%s", args->reason);
	    }
	    if (sudo_lbuf_error(lbuf))
		goto oom;
	}
	debug_return_bool(true);
    }

    /* A TSID may be a sudoers-style session ID or a free-form string. */
    iolog_file = evlog->iolog_file;
    if (iolog_file != NULL) {
	if (IS_SESSID(iolog_file)) {
	    sessid[0] = iolog_file[0];
	    sessid[1] = iolog_file[1];
	    sessid[2] = iolog_file[3];
	    sessid[3] = iolog_file[4];
	    sessid[4] = iolog_file[6];
	    sessid[5] = iolog_file[7];
	    sessid[6] = '\0';
	    tsid = sessid;
	} else {
	    tsid = iolog_file;
	}
	if (sudo_timespecisset(&evlog->iolog_offset)) {
	    /* Only write up to two significant digits for the decimal part. */
	    if (evlog->iolog_offset.tv_nsec > 10000000) {
		(void)snprintf(offsetstr, sizeof(offsetstr), "@%lld.%02ld",
		    (long long)evlog->iolog_offset.tv_sec,
		    evlog->iolog_offset.tv_nsec / 10000000);
	    } else if (evlog->iolog_offset.tv_sec != 0) {
		(void)snprintf(offsetstr, sizeof(offsetstr), "@%lld",
		    (long long)evlog->iolog_offset.tv_sec);
	    }
	}
    }

    /* Sudo-format logs use the short form of the ttyname. */
    if ((tty = evlog->ttyname) != NULL) {
	if (strncmp(tty, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	    tty += sizeof(_PATH_DEV) - 1;
    }

    /*
     * Format the log line as an lbuf, escaping control characters in
     * octal form (#0nn).  Error checking (ENOMEM) is done at the end.
     */
    if (args->reason != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "%s%s", args->reason,
	    args->errstr ? " : " : " ; ");
    }
    if (args->errstr != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "%s ; ", args->errstr);
    }
    if (evlog->submithost != NULL && !evl_conf->omit_hostname) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "HOST=%s ; ",
	    evlog->submithost);
    }
    if (tty != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "TTY=%s ; ", tty);
    }
    if (evlog->runchroot != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "CHROOT=%s ; ",
	    evlog->runchroot);
    }
    if (evlog->runcwd != NULL || evlog->cwd != NULL) {
	if (ISSET(flags, EVLOG_CWD)) {
	    /* For sudoreplay -l output format. */
	    sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "CWD=%s ; ",
		evlog->runcwd ? evlog->runcwd : evlog->cwd);
	} else if (evlog->runcwd != NULL) {
	    /* For backwards compatibility with sudo log format. */
	    sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "PWD=%s ; ",
		evlog->runcwd);
	}
    }
    if (evlog->runuser != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "USER=%s ; ",
	    evlog->runuser);
    }
    if (evlog->rungroup != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "GROUP=%s ; ",
	    evlog->rungroup);
    }
    if (tsid != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "TSID=%s%s ; ", tsid,
	    offsetstr);
    }
    if (evlog->env_add != NULL && evlog->env_add[0] != NULL) {
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "ENV=%s",
	    evlog->env_add[0]);
	for (i = 1; evlog->env_add[i] != NULL; i++) {
	    sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, " %s",
		evlog->env_add[i]);
	}
	sudo_lbuf_append(lbuf, " ; ");
    }
    if (evlog->command != NULL && evlog->runargv != NULL) {
	/* Command plus argv. */
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL|LBUF_ESC_BLANK,
	    "COMMAND=%s", evlog->command);
	if (evlog->runargv[0] != NULL) {
	    for (i = 1; evlog->runargv[i] != NULL; i++) {
		sudo_lbuf_append(lbuf, " ");
		if (strchr(evlog->runargv[i], ' ') != NULL) {
		    /* Wrap args containing spaces in single quotes. */
		    sudo_lbuf_append(lbuf, "'");
		    sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL|LBUF_ESC_QUOTE,
			"%s", evlog->runargv[i]);
		    sudo_lbuf_append(lbuf, "'");
		} else {
		    /* Escape quotes here too for consistency. */
		    sudo_lbuf_append_esc(lbuf,
			LBUF_ESC_CNTRL|LBUF_ESC_BLANK|LBUF_ESC_QUOTE,
			"%s", evlog->runargv[i]);
		}
	    }
	}
	if (event_type == EVLOG_EXIT) {
	    if (evlog->signal_name != NULL) {
		sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, " ; SIGNAL=%s",
		    evlog->signal_name);
	    }
	    if (evlog->exit_value != -1) {
		(void)snprintf(exit_str, sizeof(exit_str), "%d",
		    evlog->exit_value);
		sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, " ; EXIT=%s",
		    exit_str);
	    }
	}
    } else if (evlog->command != NULL) {
	/* Just the command, no argv. */
	sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "COMMAND=%s",
	    evlog->command);
    }

    if (!sudo_lbuf_error(lbuf))
	debug_return_bool(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_bool(false);
}

bool
eventlog_store_sudo(int event_type, const struct eventlog *evlog,
    struct sudo_lbuf *lbuf)
{
    struct eventlog_args args = { NULL };

    return new_logline(event_type, EVLOG_CWD, &args, evlog, lbuf);
}

static void
closefrom_nodebug(int lowfd)
{
    unsigned char *debug_fds;
    int fd, startfd;
    debug_decl(closefrom_nodebug, SUDO_DEBUG_UTIL);

    startfd = sudo_debug_get_fds(&debug_fds) + 1;
    if (lowfd > startfd)
	startfd = lowfd;

    /* Close fds higher than the debug fds. */
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"closing fds >= %d", startfd);
    closefrom(startfd);

    /* Close fds [lowfd, startfd) that are not in debug_fds. */
    for (fd = lowfd; fd < startfd; fd++) {
	if (sudo_isset(debug_fds, fd))
	    continue;
	sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	    "closing fd %d", fd);
#ifdef __APPLE__
	/* Avoid potential libdispatch crash when we close its fds. */
	(void) fcntl(fd, F_SETFD, FD_CLOEXEC);
#else
	(void) close(fd);
#endif
    }
    debug_return;
}

#define MAX_MAILFLAGS	63

sudo_noreturn static void
exec_mailer(int pipein)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    char *last, *mflags, *p, *argv[MAX_MAILFLAGS + 1];
    const char *mpath = evl_conf->mailerpath;
    size_t i;
    const char * const root_envp[] = {
	"HOME=/",
	"PATH=/usr/bin:/bin:/usr/sbin:/sbin",
	"LOGNAME=root",
	"USER=root",
# ifdef _AIX
	"LOGIN=root",
# endif
	NULL
    };
    debug_decl(exec_mailer, SUDO_DEBUG_UTIL);

    /* Set stdin to read side of the pipe. */
    if (dup3(pipein, STDIN_FILENO, 0) == -1) {
	syslog(LOG_ERR, _("unable to dup stdin: %m")); // -V618
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "unable to dup stdin: %s", strerror(errno));
	sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
	_exit(127);
    }

    /* Build up an argv based on the mailer path and flags */
    if ((mflags = strdup(evl_conf->mailerflags)) == NULL) {
	syslog(LOG_ERR, _("unable to allocate memory")); // -V618
	sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
	_exit(127);
    }
    argv[0] = sudo_basename(mpath);

    i = 1;
    for (p = strtok_r(mflags, " \t", &last); p != NULL;
            p = strtok_r(NULL, " \t", &last)) {
        if (i < MAX_MAILFLAGS)
            argv[i++] = p;
    }
    argv[i] = NULL;

    /*
     * Depending on the config, either run the mailer as root
     * (so user cannot kill it) or as the user (for the paranoid).
     */
    if (setuid(ROOT_UID) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR, "unable to change uid to %u",
	    ROOT_UID);
    }
    if (evl_conf->mailuid != ROOT_UID) {
	if (setuid(evl_conf->mailuid) != 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR, "unable to change uid to %u",
		(unsigned int)evl_conf->mailuid);
	}
    }
    sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
    if (evl_conf->mailuid == ROOT_UID)
	execve(mpath, argv, (char **)root_envp);
    else
	execv(mpath, argv);
    syslog(LOG_ERR, _("unable to execute %s: %m"), mpath); // -V618
    sudo_debug_printf(SUDO_DEBUG_ERROR, "unable to execute %s: %s",
	mpath, strerror(errno));
    _exit(127);
}

/* Send a message to the mailto user */
static bool
send_mail(const struct eventlog *evlog, const char *message)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    const char *cp, *timefmt = evl_conf->time_fmt;
    struct sigaction sa;
    char timebuf[1024];
    sigset_t chldmask;
    struct tm tm;
    time_t now;
    FILE *mail;
    int fd, pfd[2], status;
    size_t len;
    pid_t pid, rv;
    struct stat sb;
#if defined(HAVE_NL_LANGINFO) && defined(CODESET)
    char *locale;
#endif
    debug_decl(send_mail, SUDO_DEBUG_UTIL);

    /* If mailer is disabled just return. */
    if (evl_conf->mailerpath == NULL || evl_conf->mailto == NULL)
	debug_return_bool(true);

    /* Make sure the mailer exists and is a regular file. */
    if (stat(evl_conf->mailerpath, &sb) != 0 || !S_ISREG(sb.st_mode))
	debug_return_bool(false);

    time(&now);
    if (localtime_r(&now, &tm) == NULL)
	debug_return_bool(false);

    /* Block SIGCHLD for the duration since we call waitpid() below. */
    sigemptyset(&chldmask);
    sigaddset(&chldmask, SIGCHLD);
    (void)sigprocmask(SIG_BLOCK, &chldmask, NULL);

    /* Fork and return, child will daemonize. */
    switch (pid = sudo_debug_fork()) {
	case -1:
	    /* Error. */
	    sudo_warn("%s", U_("unable to fork"));

	    /* Unblock SIGCHLD and return. */
	    (void)sigprocmask(SIG_UNBLOCK, &chldmask, NULL);
	    debug_return_bool(false);
	case 0:
	    /* Child. */
	    switch (fork()) {
		case -1:
		    /* Error. */
		    syslog(LOG_ERR, _("unable to fork: %m")); // -V618
		    sudo_debug_printf(SUDO_DEBUG_ERROR, "unable to fork: %s",
			strerror(errno));
		    sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
		    _exit(EXIT_FAILURE);
		    /* NOTREACHED */
		case 0:
		    /* Grandchild continues below. */
		    sudo_debug_enter(__func__, __FILE__, __LINE__, sudo_debug_subsys);
		    break;
		default:
		    /* Parent will wait for us. */
		    _exit(EXIT_SUCCESS);
		    /* NOTREACHED */
	    }
	    break;
	default:
	    /* Parent. */
	    for (;;) {
		rv = waitpid(pid, &status, 0);
		if (rv == -1 && errno != EINTR)
		    break;
		if (rv != -1 && !WIFSTOPPED(status))
		    break;
	    }
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"child (%d) exit value %d", (int)rv, status);

	    /* Unblock SIGCHLD and return. */
	    (void)sigprocmask(SIG_UNBLOCK, &chldmask, NULL);
	    debug_return_bool(true);
    }

    /* Reset SIGCHLD to default and unblock it. */
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_DFL;
    (void)sigaction(SIGCHLD, &sa, NULL);
    (void)sigprocmask(SIG_UNBLOCK, &chldmask, NULL);

    /* Daemonize - disassociate from session/tty. */
    if (setsid() == -1)
      sudo_warn("setsid");
    if (chdir("/") == -1)
      sudo_warn("chdir(/)");
    fd = open(_PATH_DEVNULL, O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd != -1) {
	(void) dup2(fd, STDIN_FILENO);
	(void) dup2(fd, STDOUT_FILENO);
	(void) dup2(fd, STDERR_FILENO);
    }

    /* Close non-debug fds so we don't leak anything. */
    closefrom_nodebug(STDERR_FILENO + 1);

    if (pipe2(pfd, O_CLOEXEC) == -1) {
	syslog(LOG_ERR, _("unable to open pipe: %m")); // -V618
	sudo_debug_printf(SUDO_DEBUG_ERROR, "unable to open pipe: %s",
	    strerror(errno));
	sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
	_exit(EXIT_FAILURE);
    }

    switch (pid = sudo_debug_fork()) {
	case -1:
	    /* Error. */
	    syslog(LOG_ERR, _("unable to fork: %m")); // -V618
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to fork");
	    sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
	    _exit(EXIT_FAILURE);
	    /* NOTREACHED */
	case 0:
	    /* Child. */
	    exec_mailer(pfd[0]);
	    /* NOTREACHED */
    }

    (void) close(pfd[0]);
    if ((mail = fdopen(pfd[1], "w")) == NULL) {
	syslog(LOG_ERR, "fdopen: %m");
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to fdopen pipe");
	sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
	_exit(EXIT_FAILURE);
    }

    /* Pipes are all setup, send message. */
    (void) fprintf(mail, "To: %s\nFrom: %s\nAuto-Submitted: %s\nSubject: ",
	evl_conf->mailto,
	evl_conf->mailfrom ? evl_conf->mailfrom :
	(evlog ? evlog->submituser : "root"),
	"auto-generated");
    for (cp = _(evl_conf->mailsub); *cp; cp++) {
	/* Expand escapes in the subject */
	if (*cp == '%' && *(cp+1) != '%') {
	    switch (*(++cp)) {
		case 'h':
		    if (evlog != NULL)
			(void) fputs(evlog->submithost, mail);
		    break;
		case 'u':
		    if (evlog != NULL)
			(void) fputs(evlog->submituser, mail);
		    break;
		default:
		    cp--;
		    break;
	    }
	} else
	    (void) fputc(*cp, mail);
    }

#if defined(HAVE_NL_LANGINFO) && defined(CODESET)
    locale = setlocale(LC_ALL, NULL);
    if (locale[0] != 'C' || locale[1] != '\0')
	(void) fprintf(mail, "\nContent-Type: text/plain; charset=\"%s\"\nContent-Transfer-Encoding: 8bit", nl_langinfo(CODESET));
#endif /* HAVE_NL_LANGINFO && CODESET */

    timebuf[sizeof(timebuf) - 1] = '\0';
    len = strftime(timebuf, sizeof(timebuf), timefmt, &tm);
    if (len == 0 || timebuf[sizeof(timebuf) - 1] != '\0') {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_ERROR,
	    "strftime() failed to format time: %s", timefmt);
	/* Fall back to default time format string. */
	timebuf[sizeof(timebuf) - 1] = '\0';
	len = strftime(timebuf, sizeof(timebuf), "%h %e %T", &tm);
	if (len == 0 || timebuf[sizeof(timebuf) - 1] != '\0') {
	    timebuf[0] = '\0';		/* give up */
	}
    }
    if (evlog != NULL) {
	(void) fprintf(mail, "\n\n%s : %s : %s : ", evlog->submithost, timebuf,
	    evlog->submituser);
    } else {
	(void) fprintf(mail, "\n\n%s : ", timebuf);
    }
    fputs(message, mail);
    fputs("\n\n", mail);

    fclose(mail);
    for (;;) {
	rv = waitpid(pid, &status, 0);
	if (rv == -1 && errno != EINTR)
	    break;
	if (rv != -1 && !WIFSTOPPED(status))
	    break;
    }
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"child (%d) exit value %d", (int)rv, status);
    sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
    _exit(EXIT_SUCCESS);
}

static bool
json_add_timestamp(struct json_container *jsonc, const char *name,
    const struct timespec *ts, bool format_timestamp)
{
    struct json_value json_value;
    size_t len;
    debug_decl(json_add_timestamp, SUDO_DEBUG_PLUGIN);

    if (!sudo_json_open_object(jsonc, name))
	goto oom;

    json_value.type = JSON_NUMBER;
    json_value.u.number = ts->tv_sec;
    if (!sudo_json_add_value(jsonc, "seconds", &json_value))
	goto oom;

    json_value.type = JSON_NUMBER;
    json_value.u.number = ts->tv_nsec;
    if (!sudo_json_add_value(jsonc, "nanoseconds", &json_value))
	goto oom;

    if (format_timestamp) {
	const struct eventlog_config *evl_conf = eventlog_getconf();
	const char *timefmt = evl_conf->time_fmt;
	time_t secs = ts->tv_sec;
	char timebuf[1024];
	struct tm tm;

	if (gmtime_r(&secs, &tm) != NULL) {
	    timebuf[sizeof(timebuf) - 1] = '\0';
	    len = strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%SZ", &tm);
	    if (len != 0 && timebuf[sizeof(timebuf) - 1] == '\0') {
		json_value.type = JSON_STRING;
		json_value.u.string = timebuf; // -V507
		if (!sudo_json_add_value(jsonc, "iso8601", &json_value))
		    goto oom;
	    }
	}

	if (localtime_r(&secs, &tm) != NULL) {
	    timebuf[sizeof(timebuf) - 1] = '\0';
	    len = strftime(timebuf, sizeof(timebuf), timefmt, &tm);
	    if (len != 0 && timebuf[sizeof(timebuf) - 1] == '\0') {
		json_value.type = JSON_STRING;
		json_value.u.string = timebuf; // -V507
		if (!sudo_json_add_value(jsonc, "localtime", &json_value))
		    goto oom;
	    }
	}
    }

    if (!sudo_json_close_object(jsonc))
	goto oom;

    debug_return_bool(true);
oom:
    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	"%s: %s", __func__, "unable to allocate memory");
    debug_return_bool(false);
}

/*
 * Store the contents of struct eventlog as JSON.
 * The submit_time and iolog_path members are not stored, they should
 * be stored and formatted by the caller.
 */
bool
eventlog_store_json(struct json_container *jsonc, const struct eventlog *evlog)
{
    struct json_value json_value;
    size_t i;
    char *cp;
    debug_decl(eventlog_store_json, SUDO_DEBUG_UTIL);

    /* Required settings. */
    if (evlog == NULL || evlog->submituser == NULL)
	debug_return_bool(false);

    /*
     * The most important values are written first in case
     * the log record gets truncated.
     * Note: submit_time and iolog_path are not stored here.
     */

    json_value.type = JSON_STRING;
    json_value.u.string = evlog->submituser;
    if (!sudo_json_add_value(jsonc, "submituser", &json_value))
	goto oom;

    if (evlog->command != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->command;
	if (!sudo_json_add_value(jsonc, "command", &json_value))
	    goto oom;
    }

    if (evlog->runuser != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->runuser;
	if (!sudo_json_add_value(jsonc, "runuser", &json_value))
	    goto oom;
    }

    if (evlog->rungroup != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->rungroup;
	if (!sudo_json_add_value(jsonc, "rungroup", &json_value))
	    goto oom;
    }

    if (evlog->runchroot != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->runchroot;
	if (!sudo_json_add_value(jsonc, "runchroot", &json_value))
	    goto oom;
    }

    if (evlog->runcwd != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->runcwd;
	if (!sudo_json_add_value(jsonc, "runcwd", &json_value))
	    goto oom;
    }

    if (evlog->source != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->source;
	if (!sudo_json_add_value(jsonc, "source", &json_value))
	    goto oom;
    }

    if (evlog->ttyname != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->ttyname;
	if (!sudo_json_add_value(jsonc, "ttyname", &json_value))
	    goto oom;
    }

    if (evlog->submithost != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->submithost;
	if (!sudo_json_add_value(jsonc, "submithost", &json_value))
	    goto oom;
    }

    if (evlog->cwd != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->cwd;
	if (!sudo_json_add_value(jsonc, "submitcwd", &json_value))
	    goto oom;
    }

    if (evlog->rungroup!= NULL && evlog->rungid != (gid_t)-1) {
	json_value.type = JSON_ID;
	json_value.u.id = evlog->rungid;
	if (!sudo_json_add_value(jsonc, "rungid", &json_value))
	    goto oom;
    }

    if (evlog->runuid != (uid_t)-1) {
	json_value.type = JSON_ID;
	json_value.u.id = evlog->runuid;
	if (!sudo_json_add_value(jsonc, "runuid", &json_value))
	    goto oom;
    }

    json_value.type = JSON_NUMBER;
    json_value.u.number = evlog->columns;
    if (!sudo_json_add_value(jsonc, "columns", &json_value))
        goto oom;

    json_value.type = JSON_NUMBER;
    json_value.u.number = evlog->lines;
    if (!sudo_json_add_value(jsonc, "lines", &json_value))
        goto oom;

    if (evlog->runargv != NULL) {
	if (!sudo_json_open_array(jsonc, "runargv"))
	    goto oom;
	for (i = 0; (cp = evlog->runargv[i]) != NULL; i++) {
	    json_value.type = JSON_STRING;
	    json_value.u.string = cp;
	    if (!sudo_json_add_value(jsonc, NULL, &json_value))
		goto oom;
	}
	if (!sudo_json_close_array(jsonc))
	    goto oom;
    }

    if (evlog->runenv != NULL) {
	if (!sudo_json_open_array(jsonc, "runenv"))
	    goto oom;
	for (i = 0; (cp = evlog->runenv[i]) != NULL; i++) {
	    json_value.type = JSON_STRING;
	    json_value.u.string = cp;
	    if (!sudo_json_add_value(jsonc, NULL, &json_value))
		goto oom;
	}
	if (!sudo_json_close_array(jsonc))
	    goto oom;
    }

    if (evlog->submitenv != NULL) {
	if (!sudo_json_open_array(jsonc, "submitenv"))
	    goto oom;
	for (i = 0; (cp = evlog->submitenv[i]) != NULL; i++) {
	    json_value.type = JSON_STRING;
	    json_value.u.string = cp;
	    if (!sudo_json_add_value(jsonc, NULL, &json_value))
		goto oom;
	}
	if (!sudo_json_close_array(jsonc))
	    goto oom;
    }

    debug_return_bool(true);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_bool(false);
}

static bool
default_json_cb(struct json_container *jsonc, void *v)
{
    return eventlog_store_json(jsonc, v);
}

static char *
format_json(int event_type, struct eventlog_args *args,
    const struct eventlog *evlog, bool compact)
{
    eventlog_json_callback_t info_cb = args->json_info_cb;
    void *info = args->json_info;
    struct json_container jsonc = { 0 };
    struct json_value json_value;
    const char *time_str, *type_str;
    struct timespec now;
    debug_decl(format_json, SUDO_DEBUG_UTIL);

    if (info_cb == NULL) {
	info_cb = default_json_cb;
	info = (void *)evlog;
    }

    if (sudo_gettime_real(&now) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to read the clock");
	debug_return_str(NULL);
    }

    switch (event_type) {
    case EVLOG_ACCEPT:
	type_str = "accept";
	time_str = "submit_time";
	break;
    case EVLOG_REJECT:
	type_str = "reject";
	time_str = "submit_time";
	break;
    case EVLOG_ALERT:
	type_str = "alert";
	time_str = "alert_time";
	break;
    case EVLOG_EXIT:
	type_str = "exit";
	time_str = "exit_time";
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unexpected event type %d", event_type);
	debug_return_str(NULL);
    }

    if (!sudo_json_init(&jsonc, 4, compact, false, false))
	goto bad;
    if (!sudo_json_open_object(&jsonc, type_str))
	goto bad;

    if (evlog != NULL && evlog->uuid_str[0] != '\0') {
	json_value.type = JSON_STRING;
	json_value.u.string = evlog->uuid_str;
	if (!sudo_json_add_value(&jsonc, "uuid", &json_value))
	    goto bad;
    }

    /* Reject and Alert events include a reason and optional error string. */
    if (args->reason != NULL) {
	char *ereason = NULL;

	if (args->errstr != NULL) {
	    const int len = asprintf(&ereason, _("%s: %s"), args->reason,
		args->errstr);
	    if (len == -1) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		goto bad;
	    }
	}
	json_value.type = JSON_STRING;
	json_value.u.string = ereason ? ereason : args->reason;
	if (!sudo_json_add_value(&jsonc, "reason", &json_value)) {
	    free(ereason);
	    goto bad;
	}
	free(ereason);
    }

    /* Log event time on server (set earlier) */
    if (!json_add_timestamp(&jsonc, "server_time", &now, true)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable format timestamp");
	goto bad;
    }

    /* Log event time from client */
    if (args->event_time != NULL) {
	if (!json_add_timestamp(&jsonc, time_str, args->event_time, true)) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable format timestamp");
	    goto bad;
	}
    }

    if (event_type == EVLOG_EXIT && evlog != NULL) {
	/* Exit events don't need evlog details if there is a UUID. */
	if (evlog->uuid_str[0] != '\0') {
	    if (args->json_info == NULL)
		info = NULL;
	}

	if (sudo_timespecisset(&evlog->run_time)) {
	    if (!json_add_timestamp(&jsonc, "run_time", &evlog->run_time, false)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable format timestamp");
		goto bad;
	    }
	}
	if (evlog->signal_name != NULL) {
	    json_value.type = JSON_STRING;
	    json_value.u.string = evlog->signal_name;
	    if (!sudo_json_add_value(&jsonc, "signal", &json_value))
		goto bad;

	    json_value.type = JSON_BOOL;
	    json_value.u.boolean = evlog->dumped_core;
	    if (!sudo_json_add_value(&jsonc, "dumped_core", &json_value))
		goto bad;
	}
	json_value.type = JSON_NUMBER;
	json_value.u.number = evlog->exit_value;
	if (!sudo_json_add_value(&jsonc, "exit_value", &json_value))
	    goto bad;
    }

    /* Event log info may be missing for alert messages. */
    if (evlog != NULL) {
	if (evlog->peeraddr != NULL) {
	    json_value.type = JSON_STRING;
	    json_value.u.string = evlog->peeraddr;
	    if (!sudo_json_add_value(&jsonc, "peeraddr", &json_value))
		goto bad;
	}

	if (evlog->iolog_path != NULL) {
	    json_value.type = JSON_STRING;
	    json_value.u.string = evlog->iolog_path;
	    if (!sudo_json_add_value(&jsonc, "iolog_path", &json_value))
		goto bad;

	    if (sudo_timespecisset(&evlog->iolog_offset)) {
		if (!json_add_timestamp(&jsonc, "iolog_offset", &evlog->iolog_offset, false)) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
			"unable format timestamp");
		    goto bad;
		}
	    }
	}

	if (event_type == EVLOG_EXIT) {
	    /* Exit events don't need evlog details if there is a UUID. */
	    if (evlog->uuid_str[0] != '\0') {
		if (args->json_info == NULL)
		    info = NULL;
	    }

	    if (sudo_timespecisset(&evlog->run_time)) {
		if (!json_add_timestamp(&jsonc, "run_time", &evlog->run_time,
			false)) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
			"unable format timestamp");
		    goto bad;
		}
	    }
	    if (evlog->signal_name != NULL) {
		json_value.type = JSON_STRING;
		json_value.u.string = evlog->signal_name;
		if (!sudo_json_add_value(&jsonc, "signal", &json_value))
		    goto bad;

		json_value.type = JSON_BOOL;
		json_value.u.boolean = evlog->dumped_core;
		if (!sudo_json_add_value(&jsonc, "dumped_core", &json_value))
		    goto bad;
	    }
	    json_value.type = JSON_NUMBER;
	    json_value.u.number = evlog->exit_value;
	    if (!sudo_json_add_value(&jsonc, "exit_value", &json_value))
		goto bad;
	}
    }

    /* Write log info. */
    if (info != NULL) {
	if (!info_cb(&jsonc, info))
	    goto bad;
    }

    if (!sudo_json_close_object(&jsonc))
	goto bad;

    /* Caller is responsible for freeing the buffer. */
    debug_return_str(sudo_json_get_buf(&jsonc));

bad:
    sudo_json_free(&jsonc);
    debug_return_str(NULL);
}

/*
 * Log a message to syslog, prepending the username and splitting the
 * message into parts if it is longer than syslog_maxlen.
 */
static bool
do_syslog_sudo(int pri, char *logline, const struct eventlog *evlog)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    size_t len, maxlen;
    char *p, *tmp, save;
    const char *fmt;
    debug_decl(do_syslog_sudo, SUDO_DEBUG_UTIL);

    evl_conf->open_log(EVLOG_SYSLOG, NULL);

    if (evlog == NULL) {
	/* Not a command, just log it as-is. */
	syslog(pri, "%s", logline);
	goto done;
    }

    /*
     * Log the full line, breaking into multiple syslog(3) calls if necessary
     */
    fmt = _("%8s : %s");
    maxlen = evl_conf->syslog_maxlen -
	(strlen(fmt) - 5 + strlen(evlog->submituser));
    for (p = logline; *p != '\0'; ) {
	len = strlen(p);
	if (len > maxlen) {
	    /*
	     * Break up the line into what will fit on one syslog(3) line
	     * Try to avoid breaking words into several lines if possible.
	     */
	    tmp = memrchr(p, ' ', maxlen);
	    if (tmp == NULL)
		tmp = p + maxlen;

	    /* NULL terminate line, but save the char to restore later */
	    save = *tmp;
	    *tmp = '\0';

	    syslog(pri, fmt, evlog->submituser, p);

	    *tmp = save;			/* restore saved character */

	    /* Advance p and eliminate leading whitespace */
	    for (p = tmp; *p == ' '; p++)
		continue;
	} else {
	    syslog(pri, fmt, evlog->submituser, p);
	    p += len;
	}
	fmt = _("%8s : (command continued) %s");
	maxlen = evl_conf->syslog_maxlen -
	    (strlen(fmt) - 5 + strlen(evlog->submituser));
    }
done:
    evl_conf->close_log(EVLOG_SYSLOG, NULL);

    debug_return_bool(true);
}

static bool
do_syslog_json(int pri, int event_type, struct eventlog_args *args,
    const struct eventlog *evlog)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    char *json_str;
    debug_decl(do_syslog_json, SUDO_DEBUG_UTIL);

    /* Format as a compact JSON message (no newlines) */
    json_str = format_json(event_type, args, evlog, true);
    if (json_str == NULL)
	debug_return_bool(false);

    /* Syslog it in a sudo object with a @cee: prefix. */
    /* TODO: use evl_conf->syslog_maxlen to break up long messages. */
    evl_conf->open_log(EVLOG_SYSLOG, NULL);
    syslog(pri, "@cee:{\"sudo\":{%s}}", json_str);
    evl_conf->close_log(EVLOG_SYSLOG, NULL);
    free(json_str);
    debug_return_bool(true);
}

/*
 * Log a message to syslog in either sudo or JSON format.
 */
static bool
do_syslog(int event_type, int flags, struct eventlog_args *args,
    const struct eventlog *evlog)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    struct sudo_lbuf lbuf;
    bool ret = false;
    int pri;
    debug_decl(do_syslog, SUDO_DEBUG_UTIL);

    sudo_lbuf_init(&lbuf, NULL, 0, NULL, 0);

    /* Sudo format logs and mailed logs use the same log line format. */
    if (evl_conf->format == EVLOG_SUDO || ISSET(flags, EVLOG_MAIL)) {
	if (!new_logline(event_type, flags, args, evlog, &lbuf))
	    goto done;

	if (ISSET(flags, EVLOG_MAIL)) {
	    if (!send_mail(evlog, lbuf.buf)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable to mail log line");
	    }
	    if (ISSET(flags, EVLOG_MAIL_ONLY)) {
		ret = true;
		goto done;
	    }
	}
    }

    switch (event_type) {
    case EVLOG_ACCEPT:
    case EVLOG_EXIT:
	pri = evl_conf->syslog_acceptpri;
	break;
    case EVLOG_REJECT:
	pri = evl_conf->syslog_rejectpri;
	break;
    case EVLOG_ALERT:
	pri = evl_conf->syslog_alertpri;
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unexpected event type %d", event_type);
	pri = -1;
	break;
    }
    if (pri == -1) {
	/* syslog disabled for this message type */
	ret = true;
	goto done;
    }

    switch (evl_conf->format) {
    case EVLOG_SUDO:
	ret = do_syslog_sudo(pri, lbuf.buf, evlog);
	break;
    case EVLOG_JSON:
	ret = do_syslog_json(pri, event_type, args, evlog);
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unexpected eventlog format %d", evl_conf->format);
	break;
    }
done:
    sudo_lbuf_destroy(&lbuf);
    debug_return_bool(ret);
}

static bool
do_logfile_sudo(const char *logline, const struct eventlog *evlog,
    const struct timespec *event_time)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    char *full_line, timebuf[8192], *timestr = NULL;
    const char *timefmt = evl_conf->time_fmt;
    const char *logfile = evl_conf->logpath;
    struct tm tm;
    bool ret = false;
    FILE *fp;
    int len;
    debug_decl(do_logfile_sudo, SUDO_DEBUG_UTIL);

    if ((fp = evl_conf->open_log(EVLOG_FILE, logfile)) == NULL)
	debug_return_bool(false);

    if (!sudo_lock_file(fileno(fp), SUDO_LOCK)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to lock log file %s", logfile);
	goto done;
    }

    if (event_time != NULL) {
	time_t tv_sec = event_time->tv_sec;
	if (localtime_r(&tv_sec, &tm) != NULL) {
	    /* strftime() does not guarantee to NUL-terminate so we must check. */
	    timebuf[sizeof(timebuf) - 1] = '\0';
	    if (strftime(timebuf, sizeof(timebuf), timefmt, &tm) != 0 &&
		    timebuf[sizeof(timebuf) - 1] == '\0') {
		timestr = timebuf;
	    }
	}
    }
    if (evlog != NULL) {
	len = asprintf(&full_line, "%s : %s : %s",
	    timestr ? timestr : "invalid date", evlog->submituser, logline);
    } else {
	len = asprintf(&full_line, "%s : %s",
	    timestr ? timestr : "invalid date", logline);
    }
    if (len == -1) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    eventlog_writeln(fp, full_line, (size_t)len, evl_conf->file_maxlen);
    free(full_line);
    (void)fflush(fp);
    if (ferror(fp)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to write log file %s", logfile);
	goto done;
    }
    ret = true;

done:
    (void)sudo_lock_file(fileno(fp), SUDO_UNLOCK);
    evl_conf->close_log(EVLOG_FILE, fp);
    debug_return_bool(ret);
}

static bool
do_logfile_json(int event_type, struct eventlog_args *args,
    const struct eventlog *evlog)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    const char *logfile = evl_conf->logpath;
    struct stat sb;
    char *json_str;
    int ret = false;
    FILE *fp;
    debug_decl(do_logfile_json, SUDO_DEBUG_UTIL);

    if ((fp = evl_conf->open_log(EVLOG_FILE, logfile)) == NULL)
	debug_return_bool(false);

    json_str = format_json(event_type, args, evlog, false);
    if (json_str == NULL)
	goto done;

    if (!sudo_lock_file(fileno(fp), SUDO_LOCK)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to lock log file %s", logfile);
	goto done;
    }

    /* Note: assumes file ends in "\n}\n" */
    if (fstat(fileno(fp), &sb) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "unable to stat %s", logfile);
	goto done;
    }
    if (sb.st_size == 0) {
	/* New file */
	putc('{', fp);
    } else if (fseeko(fp, -3, SEEK_END) == 0) {
	/* Continue file, overwrite the final "\n}\n" */
	putc(',', fp);
    } else {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "unable to seek %s", logfile);
	goto done;
    }
    fputs(json_str, fp);
    fputs("\n}\n", fp);			/* close JSON */
    fflush(fp);
    /* XXX - check for file error and recover */

    ret = true;

done:
    free(json_str);
    (void)sudo_lock_file(fileno(fp), SUDO_UNLOCK);
    evl_conf->close_log(EVLOG_FILE, fp);
    debug_return_bool(ret);
}

static bool
do_logfile(int event_type, int flags, struct eventlog_args *args,
    const struct eventlog *evlog)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    struct sudo_lbuf lbuf;
    bool ret = false;
    debug_decl(do_logfile, SUDO_DEBUG_UTIL);

    sudo_lbuf_init(&lbuf, NULL, 0, NULL, 0);

    /* Sudo format logs and mailed logs use the same log line format. */
    if (evl_conf->format == EVLOG_SUDO || ISSET(flags, EVLOG_MAIL)) {
	if (!new_logline(event_type, flags, args, evlog, &lbuf))
	    goto done;

	if (ISSET(flags, EVLOG_MAIL)) {
	    if (!send_mail(evlog, lbuf.buf)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable to mail log line");
	    }
	    if (ISSET(flags, EVLOG_MAIL_ONLY)) {
		ret = true;
		goto done;
	    }
	}
    }

    switch (evl_conf->format) {
    case EVLOG_SUDO:
	ret = do_logfile_sudo(lbuf.buf ? lbuf.buf : args->reason, evlog,
	    args->event_time);
	break;
    case EVLOG_JSON:
	ret = do_logfile_json(event_type, args, evlog);
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unexpected eventlog format %d", evl_conf->format);
	break;
    }

done:
    sudo_lbuf_destroy(&lbuf);
    debug_return_bool(ret);
}

bool
eventlog_accept(const struct eventlog *evlog, int flags,
    eventlog_json_callback_t info_cb, void *info)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    const int log_type = evl_conf->type;
    struct eventlog_args args = { NULL };
    bool ret = true;
    debug_decl(eventlog_accept, SUDO_DEBUG_UTIL);

    args.event_time = &evlog->submit_time;
    args.json_info_cb = info_cb;
    args.json_info = info;

    if (ISSET(log_type, EVLOG_SYSLOG)) {
	if (!do_syslog(EVLOG_ACCEPT, flags, &args, evlog))
	    ret = false;
	CLR(flags, EVLOG_MAIL);
    }
    if (ISSET(log_type, EVLOG_FILE)) {
	if (!do_logfile(EVLOG_ACCEPT, flags, &args, evlog))
	    ret = false;
    }

    debug_return_bool(ret);
}

bool
eventlog_reject(const struct eventlog *evlog, int flags, const char *reason,
    eventlog_json_callback_t info_cb, void *info)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    const int log_type = evl_conf->type;
    struct eventlog_args args = { NULL };
    bool ret = true;
    debug_decl(eventlog_reject, SUDO_DEBUG_UTIL);

    args.reason = reason;
    args.event_time = &evlog->submit_time;
    args.json_info_cb = info_cb;
    args.json_info = info;

    if (ISSET(log_type, EVLOG_SYSLOG)) {
	if (!do_syslog(EVLOG_REJECT, flags, &args, evlog))
	    ret = false;
	CLR(flags, EVLOG_MAIL);
    }
    if (ISSET(log_type, EVLOG_FILE)) {
	if (!do_logfile(EVLOG_REJECT, flags, &args, evlog))
	    ret = false;
    }

    debug_return_bool(ret);
}

bool
eventlog_alert(const struct eventlog *evlog, int flags,
    struct timespec *alert_time, const char *reason, const char *errstr)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    const int log_type = evl_conf->type;
    struct eventlog_args args = { NULL };
    bool ret = true;
    debug_decl(eventlog_alert, SUDO_DEBUG_UTIL);

    args.reason = reason;
    args.errstr = errstr;
    args.event_time = alert_time;

    if (ISSET(log_type, EVLOG_SYSLOG)) {
	if (!do_syslog(EVLOG_ALERT, flags, &args, evlog))
	    ret = false;
	CLR(flags, EVLOG_MAIL);
    }
    if (ISSET(log_type, EVLOG_FILE)) {
	if (!do_logfile(EVLOG_ALERT, flags, &args, evlog))
	    ret = false;
    }

    debug_return_bool(ret);
}

bool
eventlog_mail(const struct eventlog *evlog, int flags,
    struct timespec *event_time, const char *reason, const char *errstr,
    char * const extra[])
{
    struct eventlog_args args = { NULL };
    struct sudo_lbuf lbuf;
    bool ret = false;
    debug_decl(eventlog_mail, SUDO_DEBUG_UTIL);

    args.reason = reason;
    args.errstr = errstr;
    args.event_time = event_time;

    sudo_lbuf_init(&lbuf, NULL, 0, NULL, 0);
    if (!new_logline(EVLOG_ALERT, flags, &args, evlog, &lbuf))
	goto done;

    if (extra != NULL) {
	/* Each extra message is written on its own line. */
	while (*extra != NULL) {
	    sudo_lbuf_append(&lbuf, "\n");
	    sudo_lbuf_append_esc(&lbuf, LBUF_ESC_CNTRL, "%s", *extra);
	    if (sudo_lbuf_error(&lbuf)) {
		sudo_debug_printf(
		    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		    "unable to format mail message");
		goto done;
	    }
	    extra++;
	}
    }

    ret = send_mail(evlog, lbuf.buf);
    if (!ret) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to mail log line");
    }

done:
    sudo_lbuf_destroy(&lbuf);
    debug_return_bool(ret);
}

bool
eventlog_exit(const struct eventlog *evlog, int flags)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    const int log_type = evl_conf->type;
    struct eventlog_args args = { NULL };
    struct timespec exit_time;
    bool ret = true;
    debug_decl(eventlog_exit, SUDO_DEBUG_UTIL);

    if (sudo_timespecisset(&evlog->run_time)) {
	sudo_timespecadd(&evlog->submit_time, &evlog->run_time, &exit_time);
	args.event_time = &exit_time;
    }

    if (ISSET(log_type, EVLOG_SYSLOG)) {
	if (!do_syslog(EVLOG_EXIT, flags, &args, evlog))
	    ret = false;
	CLR(flags, EVLOG_MAIL);
    }
    if (ISSET(log_type, EVLOG_FILE)) {
	if (!do_logfile(EVLOG_EXIT, flags, &args, evlog))
	    ret = false;
    }

    debug_return_bool(ret);
}
