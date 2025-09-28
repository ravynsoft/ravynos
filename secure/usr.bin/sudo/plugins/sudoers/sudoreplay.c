/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <regex.h>
#include <signal.h>
#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
# else
# include <compat/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_event.h>
#include <sudo_eventlog.h>
#include <sudo_lbuf.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_plugin.h>
#include <sudo_queue.h>
#include <sudo_util.h>

#include <logging.h>

struct replay_closure {
    const char *iolog_dir;
    struct sudo_event_base *evbase;
    struct sudo_event *delay_ev;
    struct sudo_event *keyboard_ev;
    struct sudo_event *output_ev;
    struct sudo_event *sighup_ev;
    struct sudo_event *sigint_ev;
    struct sudo_event *sigquit_ev;
    struct sudo_event *sigterm_ev;
    struct sudo_event *sigtstp_ev;
    struct timespec *offset;
    struct timespec *max_delay;
    struct timing_closure timing;
    int iolog_dir_fd;
    bool interactive;
    bool suspend_wait;
    struct io_buffer {
	size_t len; /* buffer length (how much produced) */
	size_t off; /* write position (how much already consumed) */
	size_t toread; /* how much remains to be read */
	int lastc;	  /* last char written */
	char buf[64 * 1024];
    } iobuf;
};

/*
 * Handle expressions like:
 * ( user millert or user root ) and tty console and command /bin/sh
 */
STAILQ_HEAD(search_node_list, search_node);
struct search_node {
    STAILQ_ENTRY(search_node) entries;
#define ST_EXPR		1
#define ST_TTY		2
#define ST_USER		3
#define ST_PATTERN	4
#define ST_RUNASUSER	5
#define ST_RUNASGROUP	6
#define ST_FROMDATE	7
#define ST_TODATE	8
#define ST_CWD		9
#define ST_HOST		10
    char type;
    bool negated;
    bool or;
    union {
	regex_t cmdre;
	struct timespec tstamp;
	char *cwd;
	char *host;
	char *tty;
	char *user;
	char *runas_group;
	char *runas_user;
	struct search_node_list expr;
	void *ptr;
    } u;
};

static struct search_node_list search_expr = STAILQ_HEAD_INITIALIZER(search_expr);

static double speed_factor = 1.0;

static const char *session_dir = _PATH_SUDO_IO_LOGDIR;

static bool terminal_can_resize, terminal_was_resized, follow_mode;

static int terminal_lines, terminal_cols;

static int ttyfd = -1;

static struct iolog_file iolog_files[] = {
    { false },	/* IOFD_STDIN */
    { false },	/* IOFD_STDOUT */
    { false },	/* IOFD_STDERR */
    { false },	/* IOFD_TTYIN  */
    { false },	/* IOFD_TTYOUT */
    { true, },	/* IOFD_TIMING */
};

static const char short_opts[] =  "d:f:Fhlm:nRSs:V";
static struct option long_opts[] = {
    { "directory",	required_argument,	NULL,	'd' },
    { "filter",		required_argument,	NULL,	'f' },
    { "follow",		no_argument,		NULL,	'F' },
    { "help",		no_argument,		NULL,	'h' },
    { "list",		no_argument,		NULL,	'l' },
    { "max-wait",	required_argument,	NULL,	'm' },
    { "non-interactive", no_argument,		NULL,	'n' },
    { "no-resize",	no_argument,		NULL,	'R' },
    { "suspend-wait",	no_argument,		NULL,	'S' },
    { "speed",		required_argument,	NULL,	's' },
    { "version",	no_argument,		NULL,	'V' },
    { NULL,		no_argument,		NULL,	'\0' },
};

/* XXX move to separate header? (currently in sudoers.h) */
extern char *get_timestr(time_t, int);
extern time_t get_date(char *);

static int list_sessions(int, char **, const char *, const char *, const char *);
static int parse_expr(struct search_node_list *, char **, bool);
static void read_keyboard(int fd, int what, void *v);
static int replay_session(int iolog_dir_fd, const char *iolog_dir,
    struct timespec *offset, struct timespec *max_wait, const char *decimal,
    bool interactive, bool suspend_wait);
static void sudoreplay_cleanup(void);
static void write_output(int fd, int what, void *v);
static void restore_terminal_size(void);
static void setup_terminal(struct eventlog *evlog, bool interactive, bool resize);
sudo_noreturn static void help(void);
sudo_noreturn static void usage(void);

#define VALID_ID(s) (isalnum((unsigned char)(s)[0]) && \
    isalnum((unsigned char)(s)[1]) && isalnum((unsigned char)(s)[2]) && \
    isalnum((unsigned char)(s)[3]) && isalnum((unsigned char)(s)[4]) && \
    isalnum((unsigned char)(s)[5]) && (s)[6] == '\0')

sudo_dso_public int main(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    int ch, i, iolog_dir_fd, len, exitcode = EXIT_FAILURE;
    bool def_filter = true, listonly = false;
    bool interactive = true, suspend_wait = false, resize = true;
    const char *decimal, *id, *user = NULL, *pattern = NULL, *tty = NULL;
    char *cp, *ep, iolog_dir[PATH_MAX];
    struct timespec offset = { 0, 0};
    struct eventlog *evlog;
    struct timespec max_delay_storage, *max_delay = NULL;
    double dval;
    debug_decl(main, SUDO_DEBUG_MAIN);

#if defined(SUDO_DEVEL) && defined(__OpenBSD__)
    {
	extern char *malloc_options;
	malloc_options = "S";
    }
#endif

    initprogname(argc > 0 ? argv[0] : "sudoreplay");
    setlocale(LC_ALL, "");
    decimal = localeconv()->decimal_point;
    bindtextdomain("sudoers", LOCALEDIR); /* XXX - should have sudoreplay domain */
    textdomain("sudoers");

    /* Register fatal/fatalx callback. */
    sudo_fatal_callback_register(sudoreplay_cleanup);

    /* Read sudo.conf and initialize the debug subsystem. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG) == -1)
	return EXIT_FAILURE;
    sudo_debug_register(getprogname(), NULL, NULL,
	sudo_conf_debug_files(getprogname()), -1);

    while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
	switch (ch) {
	case 'd':
	    session_dir = optarg;
	    break;
	case 'f':
	    /* Set the replay filter. */
	    def_filter = false;
	    for (cp = strtok_r(optarg, ",", &ep); cp; cp = strtok_r(NULL, ",", &ep)) {
		if (strcmp(cp, "stdin") == 0)
		    iolog_files[IOFD_STDIN].enabled = true;
		else if (strcmp(cp, "stdout") == 0)
		    iolog_files[IOFD_STDOUT].enabled = true;
		else if (strcmp(cp, "stderr") == 0)
		    iolog_files[IOFD_STDERR].enabled = true;
		else if (strcmp(cp, "ttyin") == 0)
		    iolog_files[IOFD_TTYIN].enabled = true;
		else if (strcmp(cp, "ttyout") == 0)
		    iolog_files[IOFD_TTYOUT].enabled = true;
		else
		    sudo_fatalx(U_("invalid filter option: %s"), optarg);
	    }
	    break;
	case 'F':
	    follow_mode = true;
	    break;
	case 'h':
	    help();
	    /* NOTREACHED */
	case 'l':
	    listonly = true;
	    break;
	case 'm':
	    errno = 0;
	    dval = strtod(optarg, &ep);
	    if (*ep != '\0' || errno != 0)
		sudo_fatalx(U_("invalid max wait: %s"), optarg);
	    if (dval <= 0.0) {
		sudo_timespecclear(&max_delay_storage);
	    } else {
		max_delay_storage.tv_sec = (time_t)dval;
		max_delay_storage.tv_nsec = (long)
		    ((dval - (double)max_delay_storage.tv_sec) * 1000000000.0);
	    }
	    max_delay = &max_delay_storage;
	    break;
	case 'n':
	    interactive = false;
	    break;
	case 'R':
	    resize = false;
	    break;
	case 'S':
	    suspend_wait = true;
	    break;
	case 's':
	    errno = 0;
	    speed_factor = strtod(optarg, &ep);
	    if (*ep != '\0' || errno != 0)
		sudo_fatalx(U_("invalid speed factor: %s"), optarg);
	    break;
	case 'V':
	    (void) printf(_("%s version %s\n"), getprogname(), PACKAGE_VERSION);
	    exitcode = EXIT_SUCCESS;
	    goto done;
	default:
	    usage();
	    /* NOTREACHED */
	}

    }
    argc -= optind;
    argv += optind;

    if (listonly) {
	exitcode = list_sessions(argc, argv, pattern, user, tty);
	goto done;
    }

    if (argc != 1)
	usage();

    /* By default we replay stdout, stderr and ttyout. */
    if (def_filter) {
	iolog_files[IOFD_STDOUT].enabled = true;
	iolog_files[IOFD_STDERR].enabled = true;
	iolog_files[IOFD_TTYOUT].enabled = true;
    }

    /* Check for offset in @sec.nsec form at the end of the id. */
    id = argv[0];
    if ((cp = strchr(id, '@')) != NULL) {
	ep = iolog_parse_delay(cp + 1, &offset, decimal);
	if (ep == NULL || *ep != '\0')
	    sudo_fatalx(U_("invalid time offset %s"), cp + 1);
	*cp = '\0';
    }

    /* 6 digit ID in base 36, e.g. 01G712AB or free-form name */
    if (VALID_ID(id)) {
	len = snprintf(iolog_dir, sizeof(iolog_dir), "%s/%.2s/%.2s/%.2s",
	    session_dir, id, &id[2], &id[4]);
	if (len < 0 || len >= ssizeof(iolog_dir))
	    sudo_fatalx(U_("%s/%.2s/%.2s/%.2s: %s"), session_dir,
		id, &id[2], &id[4], strerror(ENAMETOOLONG));
    } else if (id[0] == '/') {
	len = snprintf(iolog_dir, sizeof(iolog_dir), "%s", id);
	if (len < 0 || len >= ssizeof(iolog_dir))
	    sudo_fatalx(U_("%s/timing: %s"), id, strerror(ENAMETOOLONG));
    } else {
	len = snprintf(iolog_dir, sizeof(iolog_dir), "%s/%s", session_dir, id);
	if (len < 0 || len >= ssizeof(iolog_dir)) {
	    sudo_fatalx(U_("%s/%s: %s"), session_dir, id,
		strerror(ENAMETOOLONG));
	}
    }

    /* Open files for replay, applying replay filter for the -f flag. */
    if ((iolog_dir_fd = iolog_openat(AT_FDCWD, iolog_dir, O_RDONLY)) == -1)
	sudo_fatal("%s", iolog_dir);
    for (i = 0; i < IOFD_MAX; i++) {
	if (!iolog_open(&iolog_files[i], iolog_dir_fd, i, "r")) {
	    if (errno != ENOENT) {
		sudo_fatal(U_("unable to open %s/%s"), iolog_dir,
		    iolog_fd_to_name(i));
	    }
	}
    }
    if (!iolog_files[IOFD_TIMING].enabled) {
	sudo_fatal(U_("unable to open %s/%s"), iolog_dir,
	    iolog_fd_to_name(IOFD_TIMING));
    }

    /* Parse log file. */
    if ((evlog = iolog_parse_loginfo(iolog_dir_fd, iolog_dir)) == NULL)
	goto done;
    printf(_("Replaying sudo session: %s"), evlog->command);
    if (evlog->runargv != NULL && evlog->runargv[0] != NULL) {
	for (i = 1; evlog->runargv[i] != NULL; i++)
	    printf(" %s", evlog->runargv[i]);
    }

    /* Setup terminal if appropriate. */
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
	interactive = false;
    setup_terminal(evlog, interactive, resize);
    putchar('\r');
    putchar('\n');

    /* Done with parsed log file. */
    eventlog_free(evlog);
    evlog = NULL;

    /* Replay session corresponding to iolog_files[]. */
    exitcode = replay_session(iolog_dir_fd, iolog_dir, &offset, max_delay,
	decimal, interactive, suspend_wait);

    restore_terminal_size();
    sudo_term_restore(ttyfd, true);
done:
    sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys, exitcode);
    return exitcode;
}

/*
 * List of terminals that support xterm-like resizing.
 * This is not an exhaustive list.
 * For a list of VT100 style escape codes, see:
 *  http://invisible-island.net/xterm/ctlseqs/ctlseqs.html#VT100%20Mode
 */
struct term_names {
    const char *name;
    unsigned int len;
} compatible_terms[] = {
    { "Eterm", 5 },
    { "aterm", 5 },
    { "dtterm", 6 },
    { "gnome", 5 },
    { "konsole", 7 },
    { "kvt\0", 4 },
    { "mlterm", 6 },
    { "rxvt", 4 },
    { "xterm", 5 },
    { NULL, 0 }
};

struct getsize_closure {
    int nums[2];
    int nums_depth;
    int nums_maxdepth;
    int state;
    const char *cp;
    struct sudo_event *ev;
    struct timespec timeout;
};

/* getsize states */
#define INITIAL		0x00
#define NEW_NUMBER	0x01
#define NUMBER		0x02
#define GOTSIZE		0x04
#define READCHAR	0x10

/*
 * Callback for reading the terminal size response.
 * We use an event for this to support timeouts.
 */
static void
getsize_cb(int fd, int what, void *v)
{
    struct getsize_closure *gc = v;
    unsigned char ch = '\0';
    debug_decl(getsize_cb, SUDO_DEBUG_UTIL);

    for (;;) {
	if (gc->cp[0] == '\0') {
	    gc->state = GOTSIZE;
	    goto done;
	}
	if (ISSET(gc->state, READCHAR)) {
	    ssize_t nread = read(ttyfd, &ch, 1);
	    switch (nread) {
	    case -1:
		if (errno == EAGAIN)
		    goto another;
		FALLTHROUGH;
	    case 0:
		goto done;
	    default:
		CLR(gc->state, READCHAR);
		break;
	    }
	}
	switch (gc->state) {
	case INITIAL:
	    if (ch == 0233 && gc->cp[0] == '\033') {
		/* meta escape, equivalent to ESC[ */
		ch = '[';
		gc->cp++;
	    }
	    if (gc->cp[0] == '%' && gc->cp[1] == 'd') {
		gc->state = NEW_NUMBER;
		continue;
	    }
	    if (gc->cp[0] != ch) {
		sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		    "got %d, expected %d", ch, gc->cp[0]);
		goto done;
	    }
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"got %d", ch);
	    SET(gc->state, READCHAR);
	    gc->cp++;
	    break;
	case NEW_NUMBER:
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"parsing number");
	    if (!isdigit(ch))
		goto done;
	    gc->cp += 2;
	    if (gc->nums_depth > gc->nums_maxdepth)
		goto done;
	    gc->nums[gc->nums_depth] = 0;
	    gc->state = NUMBER;
	    FALLTHROUGH;
	case NUMBER:
	    if (!isdigit(ch)) {
		/* done with number, reparse ch */
		sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		    "number %d (ch %d)", gc->nums[gc->nums_depth], ch);
		gc->nums_depth++;
		gc->state = INITIAL;
		continue;
	    }
	    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
		"got %d", ch);
	    if (gc->nums[gc->nums_depth] > INT_MAX / 10)
		goto done;
	    gc->nums[gc->nums_depth] *= 10;
	    gc->nums[gc->nums_depth] += (ch - '0');
	    SET(gc->state, READCHAR);
	    break;
	}
    }

another:
    if (sudo_ev_add(NULL, gc->ev, &gc->timeout, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));
done:
    debug_return;
}


/*
 * Get the terminal size using vt100 terminal escapes.
 */
static bool
xterm_get_size(int *new_lines, int *new_cols)
{
    struct sudo_event_base *evbase;
    struct getsize_closure gc;
    const char getsize_request[] = "\0337\033[r\033[999;999H\033[6n";
    const char getsize_response[] = "\033[%d;%dR";
    bool ret = false;
    debug_decl(xterm_get_size, SUDO_DEBUG_UTIL);

    /* request the terminal's size */
    if (write(ttyfd, getsize_request, strlen(getsize_request)) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "%s: error writing xterm size request", __func__);
	goto done;
    }

    /*
     * Callback info for reading back the size with a 10 second timeout.
     * We expect two numbers (lines and cols).
     */
    gc.state = INITIAL|READCHAR;
    gc.nums_depth = 0;
    gc.nums_maxdepth = 1;
    gc.cp = getsize_response;
    gc.timeout.tv_sec = 10;
    gc.timeout.tv_nsec = 0;

    /* Setup an event for reading the terminal size */
    evbase = sudo_ev_base_alloc();
    if (evbase == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    gc.ev = sudo_ev_alloc(ttyfd, SUDO_EV_READ, getsize_cb, &gc);
    if (gc.ev == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    /* Read back terminal size response */
    if (sudo_ev_add(evbase, gc.ev, &gc.timeout, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));
    sudo_ev_dispatch(evbase);

    if (gc.state == GOTSIZE) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "terminal size %d x %x", gc.nums[0], gc.nums[1]);
	*new_lines = gc.nums[0];
	*new_cols = gc.nums[1];
	ret = true;
    }

    sudo_ev_base_free(evbase);
    sudo_ev_free(gc.ev);

done:
    debug_return_bool(ret);
}

/*
 * Set the size of the text area to lines and cols.
 * Depending on the terminal implementation, the window itself may
 * or may not shrink to a smaller size.
 */
static bool
xterm_set_size(int lines, int cols)
{
    const char setsize_fmt[] = "\033[8;%d;%dt";
    int len, new_lines, new_cols;
    bool ret = false;
    char buf[1024];
    debug_decl(xterm_set_size, SUDO_DEBUG_UTIL);

    /* XXX - save cursor and position restore after resizing */
    len = snprintf(buf, sizeof(buf), setsize_fmt, lines, cols);
    if (len < 0 || len >= ssizeof(buf)) {
	/* not possible due to size of buf */
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: internal error, buffer too small?", __func__);
	goto done;
    }
    if (write(ttyfd, buf, strlen(buf)) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "%s: error writing xterm resize request", __func__);
	goto done;
    }
    /* XXX - keyboard input will interfere with this */
    if (!xterm_get_size(&new_lines, &new_cols))
	goto done;
    if (lines == new_lines && cols == new_cols)
	ret = true;

done:
    debug_return_bool(ret);
}

static void
setup_terminal(struct eventlog *evlog, bool interactive, bool resize)
{
    const char *term;
    debug_decl(check_terminal, SUDO_DEBUG_UTIL);

    fflush(stdout);

    /* Open fd for /dev/tty and set to raw mode. */
    if (interactive) {
	ttyfd = open(_PATH_TTY, O_RDWR);
	if (ttyfd == -1)
	    sudo_fatal("%s", U_("unable to set tty to raw mode"));
	while (!sudo_term_raw(ttyfd, SUDO_TERM_ISIG)) {
	    if (errno != EINTR)
		sudo_fatal("%s", U_("unable to set tty to raw mode"));
	    kill(getpid(), SIGTTOU);
	}
    }

    /* Find terminal size if the session has size info. */
    if (evlog->lines == 0 && evlog->columns == 0) {
	/* no tty size info, hope for the best... */
	debug_return;
    }

    if (resize && ttyfd != -1) {
	term = getenv("TERM");
	if (term != NULL && *term != '\0') {
	    struct term_names *tn;

	    for (tn = compatible_terms; tn->name != NULL; tn++) {
		if (strncmp(term, tn->name, tn->len) == 0) {
		    /* xterm-like terminals can resize themselves. */
		    if (xterm_get_size(&terminal_lines, &terminal_cols))
			terminal_can_resize = true;
		    break;
		}
	    }
	}
    }

    if (!terminal_can_resize) {
	/* either not xterm or not interactive */
	sudo_get_ttysize(ttyfd, &terminal_lines, &terminal_cols);
    }

    if (evlog->lines == terminal_lines && evlog->columns == terminal_cols) {
	/* nothing to change */
	debug_return;
    }

    if (terminal_can_resize) {
	/* session terminal size is different, try to resize ours */
	if (xterm_set_size(evlog->lines, evlog->columns)) {
	    /* success */
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"resized terminal to %d x %x", evlog->lines, evlog->columns);
	    terminal_was_resized = true;
	    debug_return;
	}
	/* resize failed, don't try again */
	terminal_can_resize = false;
    }

    if (evlog->lines > terminal_lines || evlog->columns > terminal_cols) {
	puts(_("Warning: your terminal is too small to properly replay the log."));
	printf(_("Log geometry is %d x %d, your terminal's geometry is %d x %d."), evlog->lines, evlog->columns, terminal_lines, terminal_cols);
    }
    debug_return;
}

static void
resize_terminal(int lines, int cols)
{
    debug_decl(resize_terminal, SUDO_DEBUG_UTIL);

    if (terminal_can_resize) {
	if (xterm_set_size(lines, cols))
	    terminal_was_resized = true;
	else
	    terminal_can_resize = false;
    }

    debug_return;
}

static void
restore_terminal_size(void)
{
    debug_decl(restore_terminal, SUDO_DEBUG_UTIL);

    if (terminal_was_resized) {
	/* We are still in raw mode, hence the carriage return. */
	putchar('\r');
	fputs(U_("Replay finished, press any key to restore the terminal."),
	    stdout);
	fflush(stdout);
	(void)getchar();
	xterm_set_size(terminal_lines, terminal_cols);
	putchar('\r');
	putchar('\n');
    }

    debug_return;
}

static bool
iolog_complete(struct replay_closure *closure)
{
    struct stat sb;
    debug_decl(iolog_complete, SUDO_DEBUG_UTIL);

    if (fstatat(closure->iolog_dir_fd, "timing", &sb, 0) != -1) {
	if (ISSET(sb.st_mode, S_IWUSR|S_IWGRP|S_IWOTH))
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

/*
 * Read the next record from the timing file and schedule a delay
 * event with the specified timeout.
 * In follow mode, ignore EOF and just delay for a short time.
 * Return 0 on success, 1 on EOF and -1 on error.
 */
static int
get_timing_record(struct replay_closure *closure)
{
    struct timing_closure *timing = &closure->timing;
    bool nodelay = false;
    debug_decl(get_timing_record, SUDO_DEBUG_UTIL);

    if (follow_mode && timing->event == IO_EVENT_COUNT) {
	/* In follow mode, we already waited. */
	nodelay = true;
    }

    switch (iolog_read_timing_record(&iolog_files[IOFD_TIMING], timing)) {
    case -1:
	/* error */
	debug_return_int(-1);
    case 1:
	/* EOF */
	if (!follow_mode || iolog_complete(closure)) {
	    debug_return_int(1);
	}
	/* Follow mode, keep reading until done. */
	iolog_clearerr(&iolog_files[IOFD_TIMING]);
	timing->delay.tv_sec = 0;
	timing->delay.tv_nsec = 1000000;
	timing->iol = NULL;
	timing->event = IO_EVENT_COUNT;
	break;
    default:
	/* Record number bytes to read. */
	if (timing->event != IO_EVENT_WINSIZE &&
		timing->event != IO_EVENT_SUSPEND) {
	    closure->iobuf.len = 0;
	    closure->iobuf.off = 0;
	    closure->iobuf.lastc = '\0';
	    closure->iobuf.toread = timing->u.nbytes;
	}

	if (sudo_timespecisset(closure->offset)) {
	    if (sudo_timespeccmp(&timing->delay, closure->offset, >)) {
		sudo_timespecsub(&timing->delay, closure->offset, &timing->delay);
		sudo_timespecclear(closure->offset);
	    } else {
		sudo_timespecsub(closure->offset, &timing->delay, closure->offset);
		sudo_timespecclear(&timing->delay);
	    }
	}

	if (nodelay) {
	    /* Already waited, fire immediately. */
	    timing->delay.tv_sec = 0;
	    timing->delay.tv_nsec = 0;
	} else {
	    /* Adjust delay using speed factor and max_delay. */
	    iolog_adjust_delay(&timing->delay, closure->max_delay,
		speed_factor);
	}
	break;
    }

    /* Schedule the delay event. */
    if (sudo_ev_add(closure->evbase, closure->delay_ev, &timing->delay, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    debug_return_int(0);
}

/*
 * Read next timing record.
 * Exits the event loop on EOF, breaks out on error.
 */
static void
next_timing_record(struct replay_closure *closure)
{
    debug_decl(next_timing_record, SUDO_DEBUG_UTIL);

again:
    switch (get_timing_record(closure)) {
    case 0:
	/* success */
	if (closure->timing.event == IO_EVENT_SUSPEND &&
	    closure->timing.u.signo == SIGCONT && !closure->suspend_wait) {
	    /* Ignore time spent suspended. */
	    goto again;
	}
	break;
    case 1:
	/* EOF */
	sudo_ev_loopexit(closure->evbase);
	break;
    default:
	/* error */
	sudo_ev_loopbreak(closure->evbase);
	break;
    }
    debug_return;
}

static bool
fill_iobuf(struct replay_closure *closure)
{
    const size_t space = sizeof(closure->iobuf.buf) - closure->iobuf.len;
    const struct timing_closure *timing = &closure->timing;
    const char *errstr;
    debug_decl(fill_iobuf, SUDO_DEBUG_UTIL);

    if (closure->iobuf.toread != 0 && space != 0) {
	const size_t len =
	    closure->iobuf.toread < space ? closure->iobuf.toread : space;
	ssize_t nread = iolog_read(timing->iol,
	    closure->iobuf.buf + closure->iobuf.off, len, &errstr);
	if (nread <= 0) {
	    if (nread == 0) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "%s/%s: premature EOF, expected %zu bytes",
		    closure->iolog_dir, iolog_fd_to_name(timing->event),
		    closure->iobuf.toread);
	    } else {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "%s/%s: read error: %s", closure->iolog_dir,
		    iolog_fd_to_name(timing->event), errstr);
	    }
	    sudo_warnx(U_("unable to read %s/%s: %s"),
		closure->iolog_dir, iolog_fd_to_name(timing->event), errstr);
	    debug_return_bool(false);
	}
	closure->iobuf.toread -= (size_t)nread;
	closure->iobuf.len += (size_t)nread;
    }

    debug_return_bool(true);
}

/*
 * Called when the inter-record delay has expired.
 * Depending on the record type, either reads the next
 * record or changes window size.
 */
static void
delay_cb(int fd, int what, void *v)
{
    struct replay_closure *closure = v;
    struct timing_closure *timing = &closure->timing;
    debug_decl(delay_cb, SUDO_DEBUG_UTIL);

    switch (timing->event) {
    case IO_EVENT_WINSIZE:
	resize_terminal(timing->u.winsize.lines, timing->u.winsize.cols);
	break;
    case IO_EVENT_STDIN:
	if (iolog_files[IOFD_STDIN].enabled)
	    timing->iol = &iolog_files[IOFD_STDIN];
	break;
    case IO_EVENT_STDOUT:
	if (iolog_files[IOFD_STDOUT].enabled)
	    timing->iol = &iolog_files[IOFD_STDOUT];
	break;
    case IO_EVENT_STDERR:
	if (iolog_files[IOFD_STDERR].enabled)
	    timing->iol = &iolog_files[IOFD_STDERR];
	break;
    case IO_EVENT_TTYIN:
	if (iolog_files[IOFD_TTYIN].enabled)
	    timing->iol = &iolog_files[IOFD_TTYIN];
	break;
    case IO_EVENT_TTYOUT:
	if (iolog_files[IOFD_TTYOUT].enabled)
	    timing->iol = &iolog_files[IOFD_TTYOUT];
	break;
    }

    if (timing->iol != NULL) {
	/* If the stream is open, enable the write event. */
	if (sudo_ev_add(closure->evbase, closure->output_ev, NULL, false) == -1)
	    sudo_fatal("%s", U_("unable to add event to queue"));
    } else {
	/* Not replaying, get the next timing record and continue. */
	next_timing_record(closure);
    }

    debug_return;
}

static void
replay_closure_free(struct replay_closure *closure)
{
    /*
     * Free events and event base, then the closure itself.
     */
    if (closure->iolog_dir_fd != -1)
	close(closure->iolog_dir_fd);
    sudo_ev_free(closure->delay_ev);
    sudo_ev_free(closure->keyboard_ev);
    sudo_ev_free(closure->output_ev);
    sudo_ev_free(closure->sighup_ev);
    sudo_ev_free(closure->sigint_ev);
    sudo_ev_free(closure->sigquit_ev);
    sudo_ev_free(closure->sigterm_ev);
    sudo_ev_free(closure->sigtstp_ev);
    sudo_ev_base_free(closure->evbase);
    free(closure);
}

static void
signal_cb(int signo, int what, void *v)
{
    struct replay_closure *closure = v;
    debug_decl(signal_cb, SUDO_DEBUG_UTIL);

    switch (signo) {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
	/* Free the event base and restore signal handlers. */
	replay_closure_free(closure);

	/* Restore the terminal and die. */
	sudoreplay_cleanup();
	kill(getpid(), signo);
	break;
    case SIGTSTP:
	/* Ignore ^Z since we have no way to restore the screen. */
	break;
    }

    debug_return;
}

static struct replay_closure *
replay_closure_alloc(int iolog_dir_fd, const char *iolog_dir,
    struct timespec *offset, struct timespec *max_delay, const char *decimal,
    bool interactive, bool suspend_wait)
{
    struct replay_closure *closure;
    debug_decl(replay_closure_alloc, SUDO_DEBUG_UTIL);

    if ((closure = calloc(1, sizeof(*closure))) == NULL)
	debug_return_ptr(NULL);

    closure->iolog_dir_fd = iolog_dir_fd;
    closure->iolog_dir = iolog_dir;
    closure->interactive = interactive;
    closure->offset = offset;
    closure->suspend_wait = suspend_wait;
    closure->max_delay = max_delay;
    closure->timing.decimal = decimal;

    /*
     * Setup event base and delay, input and output events.
     * If interactive, take input from and write to /dev/tty.
     * If not interactive there is no input event.
     */
    closure->evbase = sudo_ev_base_alloc();
    if (closure->evbase == NULL)
	goto bad;
    closure->delay_ev = sudo_ev_alloc(-1, SUDO_EV_TIMEOUT, delay_cb, closure);
    if (closure->delay_ev == NULL)
        goto bad;
    if (interactive) {
	closure->keyboard_ev = sudo_ev_alloc(ttyfd, SUDO_EV_READ|SUDO_EV_PERSIST,
	    read_keyboard, closure);
	if (closure->keyboard_ev == NULL)
	    goto bad;
	if (sudo_ev_add(closure->evbase, closure->keyboard_ev, NULL, false) == -1)
	    sudo_fatal("%s", U_("unable to add event to queue"));
    }
    closure->output_ev = sudo_ev_alloc(interactive ? ttyfd : STDOUT_FILENO,
	SUDO_EV_WRITE, write_output, closure);
    if (closure->output_ev == NULL)
        goto bad;

    /*
     * Setup signal events, we need to restore the terminal if killed.
     */
    closure->sighup_ev = sudo_ev_alloc(SIGHUP, SUDO_EV_SIGNAL, signal_cb,
	closure);
    if (closure->sighup_ev == NULL)
	goto bad;
    if (sudo_ev_add(closure->evbase, closure->sighup_ev, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    closure->sigint_ev = sudo_ev_alloc(SIGINT, SUDO_EV_SIGNAL, signal_cb,
	closure);
    if (closure->sigint_ev == NULL)
	goto bad;
    if (sudo_ev_add(closure->evbase, closure->sigint_ev, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    closure->sigquit_ev = sudo_ev_alloc(SIGQUIT, SUDO_EV_SIGNAL, signal_cb,
	closure);
    if (closure->sigquit_ev == NULL)
	goto bad;
    if (sudo_ev_add(closure->evbase, closure->sigquit_ev, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    closure->sigterm_ev = sudo_ev_alloc(SIGTERM, SUDO_EV_SIGNAL, signal_cb,
	closure);
    if (closure->sigterm_ev == NULL)
	goto bad;
    if (sudo_ev_add(closure->evbase, closure->sigterm_ev, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    closure->sigtstp_ev = sudo_ev_alloc(SIGTSTP, SUDO_EV_SIGNAL, signal_cb,
	closure);
    if (closure->sigtstp_ev == NULL)
	goto bad;
    if (sudo_ev_add(closure->evbase, closure->sigtstp_ev, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    debug_return_ptr(closure);
bad:
    replay_closure_free(closure);
    debug_return_ptr(NULL);
}

static int
replay_session(int iolog_dir_fd, const char *iolog_dir, struct timespec *offset,
    struct timespec *max_delay, const char *decimal, bool interactive,
    bool suspend_wait)
{
    struct replay_closure *closure;
    int ret = 0;
    debug_decl(replay_session, SUDO_DEBUG_UTIL);

    /* Allocate the delay closure and read the first timing record. */
    closure = replay_closure_alloc(iolog_dir_fd, iolog_dir, offset, max_delay,
	decimal, interactive, suspend_wait);
    if (get_timing_record(closure) != 0) {
	ret = 1;
	goto done;
    }

    /* Run event loop. */
    sudo_ev_dispatch(closure->evbase);
    if (sudo_ev_got_break(closure->evbase))
	ret = 1;

done:
    /* Clean up and return. */
    replay_closure_free(closure);
    debug_return_int(ret);
}

/*
 * Write the I/O buffer.
 */
static void
write_output(int fd, int what, void *v)
{
    struct replay_closure *closure = v;
    const struct timing_closure *timing = &closure->timing;
    struct io_buffer *iobuf = &closure->iobuf;
    int iovcnt = 1;
    struct iovec iov[2];
    bool added_cr = false;
    size_t nbytes;
    ssize_t nwritten;
    debug_decl(write_output, SUDO_DEBUG_UTIL);

    /* Refill iobuf if there is more to read and buf is empty. */
    if (!fill_iobuf(closure)) {
	sudo_ev_loopbreak(closure->evbase);
	debug_return;
    }

    nbytes = iobuf->len - iobuf->off;
    iov[0].iov_base = iobuf->buf + iobuf->off;
    iov[0].iov_len = nbytes;

    if (closure->interactive &&
	(timing->event == IO_EVENT_STDOUT || timing->event == IO_EVENT_STDERR)) {
	char *nl;

	/*
	 * We may need to insert a carriage return before the newline.
	 * Note that the carriage return may have already been written.
	 */
	nl = memchr(iov[0].iov_base, '\n', iov[0].iov_len);
	if (nl != NULL) {
	    size_t len = (size_t)(nl - (char *)iov[0].iov_base);
	    if ((nl == iov[0].iov_base && iobuf->lastc != '\r') ||
		(nl != iov[0].iov_base && nl[-1] != '\r')) {
		iov[0].iov_len = len;
		iov[1].iov_base = (char *)"\r\n";
		iov[1].iov_len = 2;
		iovcnt = 2;
		nbytes = iov[0].iov_len + iov[1].iov_len;
		added_cr = true;
	    }
	}
    }

    nwritten = writev(fd, iov, iovcnt);
    switch (nwritten) {
    case -1:
	if (errno != EINTR && errno != EAGAIN)
	    sudo_fatal(U_("unable to write to %s"), "stdout");
	break;
    case 0:
	/* Should not happen. */
	break;
    default:
	if (added_cr && (size_t)nwritten >= nbytes - 1) {
	    /* The last char written was either '\r' or '\n'. */
	    iobuf->lastc = (size_t)nwritten == nbytes ? '\n' : '\r';
	} else {
	    /* Stash the last char written. */
	    iobuf->lastc = *((char *)iov[0].iov_base + nwritten);
	}
	if (added_cr) {
	    /* Subtract one for the carriage return we added above. */
	    nwritten--;
	}
	iobuf->off += (size_t)nwritten;
	break;
    }

    if (iobuf->off == iobuf->len) {
	/* Write complete, go to next timing entry if possible. */
	switch (get_timing_record(closure)) {
	case 0:
	    /* success */
	    break;
	case 1:
	    /* EOF */
	    sudo_ev_loopexit(closure->evbase);
	    break;
	default:
	    /* error */
	    sudo_ev_loopbreak(closure->evbase);
	    break;
	}
    } else {
	/* Reschedule event to write remainder. */
	if (sudo_ev_add(NULL, closure->output_ev, NULL, false) == -1)
	    sudo_fatal("%s", U_("unable to add event to queue"));
    }
    debug_return;
}

/*
 * Build expression list from search args
 */
static int
parse_expr(struct search_node_list *head, char *argv[], bool sub_expr)
{
    bool or = false, not = false;
    struct search_node *sn;
    char type, **av;
    const char *errstr;
    debug_decl(parse_expr, SUDO_DEBUG_UTIL);

    for (av = argv; *av != NULL; av++) {
	switch (av[0][0]) {
	case 'a': /* and (ignore) */
	    if (strncmp(*av, "and", strlen(*av)) != 0)
		goto bad;
	    continue;
	case 'o': /* or */
	    if (strncmp(*av, "or", strlen(*av)) != 0)
		goto bad;
	    or = true;
	    continue;
	case '!': /* negate */
	    if (av[0][1] != '\0')
		goto bad;
	    not = true;
	    continue;
	case 'c': /* cwd or command */
	    if (av[0][1] == '\0')
		sudo_fatalx(U_("ambiguous expression \"%s\""), *av);
	    if (strncmp(*av, "cwd", strlen(*av)) == 0)
		type = ST_CWD;
	    else if (strncmp(*av, "command", strlen(*av)) == 0)
		type = ST_PATTERN;
	    else
		goto bad;
	    break;
	case 'f': /* from date */
	    if (strncmp(*av, "fromdate", strlen(*av)) != 0)
		goto bad;
	    type = ST_FROMDATE;
	    break;
	case 'g': /* runas group */
	    if (strncmp(*av, "group", strlen(*av)) != 0)
		goto bad;
	    type = ST_RUNASGROUP;
	    break;
	case 'h': /* host */
	    if (strncmp(*av, "host", strlen(*av)) != 0)
		goto bad;
	    type = ST_HOST;
	    break;
	case 'r': /* runas user */
	    if (strncmp(*av, "runas", strlen(*av)) != 0)
		goto bad;
	    type = ST_RUNASUSER;
	    break;
	case 't': /* tty or to date */
	    if (av[0][1] == '\0')
		sudo_fatalx(U_("ambiguous expression \"%s\""), *av);
	    if (strncmp(*av, "todate", strlen(*av)) == 0)
		type = ST_TODATE;
	    else if (strncmp(*av, "tty", strlen(*av)) == 0)
		type = ST_TTY;
	    else
		goto bad;
	    break;
	case 'u': /* user */
	    if (strncmp(*av, "user", strlen(*av)) != 0)
		goto bad;
	    type = ST_USER;
	    break;
	case '(': /* start sub-expression */
	    if (av[0][1] != '\0')
		goto bad;
	    type = ST_EXPR;
	    break;
	case ')': /* end sub-expression */
	    if (av[0][1] != '\0')
		goto bad;
	    if (!sub_expr)
		sudo_fatalx("%s", U_("unmatched ')' in expression"));
	    debug_return_int((int)(av - argv) + 1);
	default:
	bad:
	    sudo_fatalx(U_("unknown search term \"%s\""), *av);
	    /* NOTREACHED */
	}

	/* Allocate new search node */
	if ((sn = calloc(1, sizeof(*sn))) == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	sn->type = type;
	sn->or = or;
	sn->negated = not;
	if (type == ST_EXPR) {
	    STAILQ_INIT(&sn->u.expr);
	    av += parse_expr(&sn->u.expr, av + 1, true);
	} else {
	    if (*(++av) == NULL)
		sudo_fatalx(U_("%s requires an argument"), av[-1]);
	    if (type == ST_PATTERN) {
		if (!sudo_regex_compile(&sn->u.cmdre, *av, &errstr)) {
		    sudo_fatalx(U_("invalid regular expression \"%s\": %s"),
			*av, U_(errstr));
		}
	    } else if (type == ST_TODATE || type == ST_FROMDATE) {
		sn->u.tstamp.tv_sec = get_date(*av);
		sn->u.tstamp.tv_nsec = 0;
		if (sn->u.tstamp.tv_sec == -1)
		    sudo_fatalx(U_("could not parse date \"%s\""), *av);
	    } else {
		sn->u.ptr = *av;
	    }
	}
	not = or = false; /* reset state */
	STAILQ_INSERT_TAIL(head, sn, entries);
    }
    if (sub_expr)
	sudo_fatalx("%s", U_("unmatched '(' in expression"));
    if (or)
	sudo_fatalx("%s", U_("illegal trailing \"or\""));
    if (not)
	sudo_fatalx("%s", U_("illegal trailing \"!\""));

    debug_return_int((int)(av - argv));
}

static char *
expand_command(struct eventlog *evlog, char **newbuf)
{
    size_t len, bufsize = strlen(evlog->command) + 1;
    char *cp, *buf;
    int ac;
    debug_decl(expand_command, SUDO_DEBUG_UTIL);

    if (evlog->runargv == NULL || evlog->runargv[0] == NULL || evlog->runargv[1] == NULL) {
	/* No arguments, we can use the command as-is. */
	*newbuf = NULL;
	debug_return_str(evlog->command);
    }

    /* Skip argv[0], we use evlog->command instead. */
    for (ac = 1; evlog->runargv[ac] != NULL; ac++)
	bufsize += strlen(evlog->runargv[ac]) + 1;

    if ((buf = malloc(bufsize)) == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    cp = buf;

    len = strlcpy(cp, evlog->command, bufsize);
    if (len >= bufsize)
	sudo_fatalx(U_("internal error, %s overflow"), __func__);
    cp += len;
    bufsize -= len;

    for (ac = 1; evlog->runargv[ac] != NULL; ac++) {
	if (bufsize < 2)
	    sudo_fatalx(U_("internal error, %s overflow"), __func__);
	*cp++ = ' ';
	bufsize--;

	len = strlcpy(cp, evlog->runargv[ac], bufsize);
	if (len >= bufsize)
	    sudo_fatalx(U_("internal error, %s overflow"), __func__);
	cp += len;
	bufsize -= len;
    }

    *newbuf = buf;
    debug_return_str(buf);
}

static bool
match_expr(struct search_node_list *head, struct eventlog *evlog, bool last_match)
{
    struct search_node *sn;
    bool res = false, matched = last_match;
    char *tofree;
    int rc;
    debug_decl(match_expr, SUDO_DEBUG_UTIL);

    STAILQ_FOREACH(sn, head, entries) {
	switch (sn->type) {
	case ST_EXPR:
	    res = match_expr(&sn->u.expr, evlog, matched);
	    break;
	case ST_CWD:
	    if (evlog->cwd != NULL)
		res = strcmp(sn->u.cwd, evlog->cwd) == 0;
	    break;
	case ST_HOST:
	    if (evlog->submithost != NULL)
		res = strcmp(sn->u.host, evlog->submithost) == 0;
	    break;
	case ST_TTY:
	    if (evlog->ttyname != NULL)
		res = strcmp(sn->u.tty, evlog->ttyname) == 0;
	    break;
	case ST_RUNASGROUP:
	    if (evlog->rungroup != NULL)
		res = strcmp(sn->u.runas_group, evlog->rungroup) == 0;
	    break;
	case ST_RUNASUSER:
	    if (evlog->runuser != NULL)
		res = strcmp(sn->u.runas_user, evlog->runuser) == 0;
	    break;
	case ST_USER:
	    if (evlog->submituser != NULL)
		res = strcmp(sn->u.user, evlog->submituser) == 0;
	    break;
	case ST_PATTERN:
	    rc = regexec(&sn->u.cmdre, expand_command(evlog, &tofree),
		0, NULL, 0);
	    if (rc && rc != REG_NOMATCH) {
		char buf[BUFSIZ];
		regerror(rc, &sn->u.cmdre, buf, sizeof(buf));
		sudo_fatalx("%s", buf);
	    }
	    res = rc == REG_NOMATCH ? 0 : 1;
	    free(tofree);
	    break;
	case ST_FROMDATE:
	    res = sudo_timespeccmp(&evlog->submit_time, &sn->u.tstamp, >=);
	    break;
	case ST_TODATE:
	    res = sudo_timespeccmp(&evlog->submit_time, &sn->u.tstamp, <=);
	    break;
	default:
	    sudo_fatalx(U_("unknown search type %d"), sn->type);
	    /* NOTREACHED */
	}
	if (sn->negated)
	    res = !res;
	matched = sn->or ? (res || last_match) : (res && last_match);
	last_match = matched;
    }
    debug_return_bool(matched);
}

static int
list_session(struct sudo_lbuf *lbuf, char *log_dir, regex_t *re,
    const char *user, const char *tty)
{
    struct eventlog *evlog = NULL;
    const char *timestr;
    int ret = -1;
    debug_decl(list_session, SUDO_DEBUG_UTIL);

    if ((evlog = iolog_parse_loginfo(-1, log_dir)) == NULL)
	goto done;

    if (evlog->command == NULL || evlog->submituser == NULL ||
	    evlog->runuser == NULL) {
	goto done;
    }
    evlog->iolog_file = log_dir + strlen(session_dir) + 1;

    /* Match on search expression if there is one. */
    if (!STAILQ_EMPTY(&search_expr) && !match_expr(&search_expr, evlog, true))
	goto done;

    timestr = get_timestr(evlog->submit_time.tv_sec, 1);
    sudo_lbuf_append_esc(lbuf, LBUF_ESC_CNTRL, "%s : %s : ",
	timestr ? timestr : "invalid date", evlog->submituser);

    if (eventlog_store_sudo(EVLOG_ACCEPT, evlog, lbuf)) {
	puts(lbuf->buf);
	ret = 0;
    }

done:
    lbuf->error = 0;
    lbuf->len = 0;
    eventlog_free(evlog);
    debug_return_int(ret);
}

static int
session_compare(const void *v1, const void *v2)
{
    const char *s1 = *(const char **)v1;
    const char *s2 = *(const char **)v2;
    return strcmp(s1, s2);
}

/* XXX - always returns 0, calls sudo_fatal() on failure */
static int
find_sessions(const char *dir, regex_t *re, const char *user, const char *tty)
{
    DIR *d;
    struct dirent *dp;
    struct stat sb;
    struct sudo_lbuf lbuf;
    size_t sdlen, sessions_len = 0, sessions_size = 0;
    size_t i;
    int len;
    char pathbuf[PATH_MAX], **sessions = NULL;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    bool checked_type = true;
#else
    const bool checked_type = false;
#endif
    debug_decl(find_sessions, SUDO_DEBUG_UTIL);

    sudo_lbuf_init(&lbuf, NULL, 0, NULL, 0);

    d = opendir(dir);
    if (d == NULL)
	sudo_fatal(U_("unable to open %s"), dir);

    /* XXX - would be faster to use openat() and relative names */
    sdlen = strlcpy(pathbuf, dir, sizeof(pathbuf));
    if (sdlen + 1 >= sizeof(pathbuf)) {
	errno = ENAMETOOLONG;
	sudo_fatal("%s/", dir);
    }
    pathbuf[sdlen++] = '/';
    pathbuf[sdlen] = '\0';

    /* Store potential session dirs for sorting. */
    while ((dp = readdir(d)) != NULL) {
	/* Skip "." and ".." */
	if (dp->d_name[0] == '.' && (dp->d_name[1] == '\0' ||
	    (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))
	    continue;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (checked_type) {
	    if (dp->d_type != DT_DIR) {
		/* Not all file systems support d_type. */
		if (dp->d_type != DT_UNKNOWN)
		    continue;
		checked_type = false;
	    }
	}
#endif

	/* Add name to session list. */
	if (sessions_len + 1 > sessions_size) {
	    if (sessions_size == 0)
		sessions_size = 36 * 36 / 2;
	    sessions = reallocarray(sessions, sessions_size, 2 * sizeof(char *));
	    if (sessions == NULL)
		sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    sessions_size *= 2;
	}
	if ((sessions[sessions_len] = strdup(dp->d_name)) == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	sessions_len++;
    }
    closedir(d);

    /* Sort and list the sessions. */
    if (sessions != NULL) {
	qsort(sessions, sessions_len, sizeof(char *), session_compare);
	for (i = 0; i < sessions_len; i++) {
	    len = snprintf(&pathbuf[sdlen], sizeof(pathbuf) - sdlen,
		"%s/log", sessions[i]);
	    if (len < 0 || (size_t)len >= sizeof(pathbuf) - sdlen) {
		errno = ENAMETOOLONG;
		sudo_fatal("%s/%s/log", dir, sessions[i]);
	    }
	    free(sessions[i]);

	    /* Check for dir with a log file. */
	    if (lstat(pathbuf, &sb) == 0 && S_ISREG(sb.st_mode)) {
		pathbuf[sdlen + (size_t)len - 4] = '\0';
		list_session(&lbuf, pathbuf, re, user, tty);
	    } else {
		/* Strip off "/log" and recurse if a non-log dir. */
		pathbuf[sdlen + (size_t)len - 4] = '\0';
		if (checked_type ||
		    (lstat(pathbuf, &sb) == 0 && S_ISDIR(sb.st_mode)))
		    find_sessions(pathbuf, re, user, tty);
	    }
	}
	free(sessions);
    }
    sudo_lbuf_destroy(&lbuf);

    debug_return_int(0);
}

/* XXX - always returns 0, calls sudo_fatal() on failure */
static int
list_sessions(int argc, char **argv, const char *pattern, const char *user,
    const char *tty)
{
    regex_t rebuf, *re = NULL;
    const char *errstr;
    debug_decl(list_sessions, SUDO_DEBUG_UTIL);

    /* Parse search expression if present */
    parse_expr(&search_expr, argv, false);

    /* optional regex */
    if (pattern) {
	re = &rebuf;
	if (!sudo_regex_compile(re, pattern, &errstr)) {
	    sudo_fatalx(U_("invalid regular expression \"%s\": %s"),
		pattern, U_(errstr));
	}
    }

    debug_return_int(find_sessions(session_dir, re, user, tty));
}

/*
 * Check keyboard for ' ', '<', '>', return
 * pause, slow, fast, next
 */
static void
read_keyboard(int fd, int what, void *v)
{
    struct replay_closure *closure = v;
    static bool paused = false;
    struct timespec ts;
    ssize_t nread;
    char ch;
    debug_decl(read_keyboard, SUDO_DEBUG_UTIL);

    nread = read(fd, &ch, 1);
    switch (nread) {
    case -1:
	if (errno != EINTR && errno != EAGAIN)
	    sudo_fatal(U_("unable to read %s"), "stdin");
	break;
    case 0:
	/* Ignore EOF. */
	break;
    default:
	if (paused) {
	    /* Any key will unpause, run the delay callback directly. */
	    paused = false;
	    delay_cb(-1, SUDO_EV_TIMEOUT, closure);
	    debug_return;
	}
	switch (ch) {
	case ' ':
	    paused = true;
	    /* Disable the delay event until we unpause. */
	    sudo_ev_del(closure->evbase, closure->delay_ev);
	    break;
	case '<':
	    speed_factor /= 2;
	    if (sudo_ev_pending(closure->delay_ev, SUDO_EV_TIMEOUT, &ts)) {
		/* Double remaining timeout. */
		ts.tv_sec *= 2;
		ts.tv_nsec *= 2;
		if (ts.tv_nsec >= 1000000000) {
		    ts.tv_sec++;
		    ts.tv_nsec -= 1000000000;
		}
		if (sudo_ev_add(NULL, closure->delay_ev, &ts, false) == -1) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
			"failed to double remaining delay timeout");
		}
            }
	    break;
	case '>':
	    speed_factor *= 2;
	    if (sudo_ev_pending(closure->delay_ev, SUDO_EV_TIMEOUT, &ts)) {
		/* Halve remaining timeout. */
		if (ts.tv_sec & 1)
		    ts.tv_nsec += 500000000;
		ts.tv_sec /= 2;
		ts.tv_nsec /= 2;
		if (sudo_ev_add(NULL, closure->delay_ev, &ts, false) == -1) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
			"failed to halve remaining delay timeout");
		}
            }
	    break;
	case '\r':
	case '\n':
	    /* Cancel existing delay, run callback directly. */
	    sudo_ev_del(closure->evbase, closure->delay_ev);
	    delay_cb(-1, SUDO_EV_TIMEOUT, closure);
	    break;
	default:
	    /* Unknown key, nothing to do. */
	    break;
	}
	break;
    }
    debug_return;
}

static void
display_usage(FILE *fp)
{
    fprintf(fp, _("usage: %s [-hnRS] [-d dir] [-m num] [-s num] ID\n"),
	getprogname());
    fprintf(fp, _("usage: %s [-h] [-d dir] -l [search expression]\n"),
	getprogname());
}

sudo_noreturn static void
usage(void)
{
    display_usage(stderr);
    exit(EXIT_FAILURE);
}

sudo_noreturn static void
help(void)
{
    (void) printf(_("%s - replay sudo session logs\n\n"), getprogname());
    display_usage(stdout);
    (void) puts(_("\nOptions:\n"
	"  -d, --directory=dir    specify directory for session logs\n"
	"  -f, --filter=filter    specify which I/O type(s) to display\n"
	"  -h, --help             display help message and exit\n"
	"  -l, --list             list available session IDs, with optional expression\n"
	"  -m, --max-wait=num     max number of seconds to wait between events\n"
	"  -n, --non-interactive  no prompts, session is sent to the standard output\n"
	"  -R, --no-resize        do not attempt to re-size the terminal\n"
	"  -S, --suspend-wait     wait while the command was suspended\n"
	"  -s, --speed=num        speed up or slow down output\n"
	"  -V, --version          display version information and exit"));
    exit(EXIT_SUCCESS);
}

/*
 * Cleanup hook for sudo_fatal()/sudo_fatalx()
  */
static void
sudoreplay_cleanup(void)
{
    restore_terminal_size();
    sudo_term_restore(ttyfd, false);
}
