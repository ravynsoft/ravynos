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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_fatal.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * Note: we do not test SIGUNUSED as it may not appear in sys_sigabbrev[]
 *       on Linux.  FreeBSD is missing SIGLWP (aka SIGTHR) in sys_signame[].
 */
static struct signal_data {
    int rval;
    int signo;
    const char *sigstr;
    const char *altstr;
} signal_data[] = {
#ifdef SIGHUP
    { 0, SIGHUP, "HUP", NULL },
#endif
#ifdef SIGINT
    { 0, SIGINT, "INT", NULL },
#endif
#ifdef SIGQUIT
    { 0, SIGQUIT, "QUIT", NULL },
#endif
#ifdef SIGILL
    { 0, SIGILL, "ILL", NULL },
#endif
#ifdef SIGTRAP
    { 0, SIGTRAP, "TRAP", NULL },
#endif
#ifdef SIGABRT
    { 0, SIGABRT, "ABRT", "IOT" },
#endif
#ifdef SIGIOT
    { 0, SIGIOT, "IOT", "ABRT" },
#endif
#ifdef SIGEMT
    { 0, SIGEMT, "EMT", NULL },
#endif
#ifdef SIGFPE
    { 0, SIGFPE, "FPE", NULL },
#endif
#ifdef SIGKILL
    { 0, SIGKILL, "KILL", NULL },
#endif
#ifdef SIGBUS
    { 0, SIGBUS, "BUS", NULL },
#endif
#ifdef SIGSEGV
    { 0, SIGSEGV, "SEGV", NULL },
#endif
#ifdef SIGSYS
    { 0, SIGSYS, "SYS", NULL },
#endif
#ifdef SIGPIPE
    { 0, SIGPIPE, "PIPE", NULL },
#endif
#ifdef SIGALRM
    { 0, SIGALRM, "ALRM", NULL },
#endif
#ifdef SIGTERM
    { 0, SIGTERM, "TERM", NULL },
#endif
#ifdef SIGSTKFLT
    { 0, SIGSTKFLT, "STKFLT", NULL },
#endif
#ifdef SIGIO
    { 0, SIGIO, "IO", "POLL"},
#endif
#ifdef SIGXCPU
    { 0, SIGXCPU, "XCPU", NULL },
#endif
#ifdef SIGXFSZ
    { 0, SIGXFSZ, "XFSZ", NULL },
#endif
#ifdef SIGVTALRM
    { 0, SIGVTALRM, "VTALRM", NULL },
#endif
#ifdef SIGPROF
    { 0, SIGPROF, "PROF", NULL },
#endif
#ifdef SIGWINCH
    { 0, SIGWINCH, "WINCH", NULL },
#endif
#ifdef SIGLOST
    { 0, SIGLOST, "LOST", NULL },
#endif
#ifdef SIGUSR1
    { 0, SIGUSR1, "USR1", NULL },
#endif
#ifdef SIGUSR2
    { 0, SIGUSR2, "USR2", NULL },
#endif
#ifdef SIGPWR
    { 0, SIGPWR, "PWR", NULL },
#endif
#ifdef SIGPOLL
    { 0, SIGPOLL, "POLL", "IO" },
#endif
#ifdef SIGSTOP
    { 0, SIGSTOP, "STOP", NULL },
#endif
#ifdef SIGTSTP
    { 0, SIGTSTP, "TSTP", NULL },
#endif
#ifdef SIGCONT
    { 0, SIGCONT, "CONT", NULL },
#endif
#ifdef SIGCHLD
    { 0, SIGCHLD, "CHLD", "CLD" },
#endif
#ifdef SIGCLD
    { 0, SIGCLD, "CLD", "CHLD" },
#endif
#ifdef SIGTTIN
    { 0, SIGTTIN, "TTIN", NULL },
#endif
#ifdef SIGTTOU
    { 0, SIGTTOU, "TTOU", NULL },
#endif
#ifdef SIGINFO
    { 0, SIGINFO, "INFO", NULL },
#endif
#ifdef SIGURG
    { 0, SIGURG, "URG", NULL },
#endif
#ifdef SIGWAITING
    { 0, SIGWAITING, "WAITING", NULL },
#endif
#if defined(SIGLWP) && !defined(__FreeBSD__)
    { 0, SIGLWP, "LWP", NULL },
#endif
#ifdef SIGFREEZE
    { 0, SIGFREEZE, "FREEZE", NULL },
#endif
#ifdef SIGTHAW
    { 0, SIGTHAW, "THAW", NULL },
#endif
#ifdef SIGCANCEL
    { 0, SIGCANCEL, "CANCEL", NULL },
#endif
#if defined(SIGRTMIN) && defined(SIGRTMAX)
    { 0, -1, "RTMIN",   NULL },
    { 0, -1, "RTMIN+1", NULL },
    { 0, -1, "RTMIN+2", NULL },
    { 0, -1, "RTMIN+3", NULL },
    { 0, -1, "RTMAX-3", NULL },
    { 0, -1, "RTMAX-2", NULL },
    { 0, -1, "RTMAX-1", NULL },
    { 0, -1, "RTMAX",   NULL },
#endif
    { -1, 1024, "QWERT", NULL },	/* invalid */
    { -1, 0, NULL, NULL }
};

#ifndef HAVE_SIG2STR
static int
test_sig2str(int *ntests)
{
    struct signal_data *d;
    int rval, errors = 0;
    char sigstr[SIG2STR_MAX];

    for (d = signal_data; d->signo != 0; d++) {
	(*ntests)++;
	rval = sudo_sig2str(d->signo, sigstr);
	if (rval != d->rval) {
	    sudo_warnx_nodebug("FAIL: sig2str(SIG%s): %d != %d",
		d->sigstr, rval, d->rval);
	    errors++;
	    continue;
	}
	if (rval != 0)
	    continue;
	if (strcmp(sigstr, d->sigstr) != 0 &&
	    (d->altstr != NULL && strcmp(sigstr, d->altstr) != 0)) {
	    sudo_warnx_nodebug("FAIL: signal %d: %s != %s", d->signo,
		sigstr, d->sigstr);
	    errors++;
	    continue;
	}
    }

    return errors;
}
#else
static int
test_sig2str(int *ntests)
{
    return 0;
}
#endif /* HAVE_SIG2STR */

#ifndef HAVE_STR2SIG
static int
test_str2sig(int *ntests)
{
    struct signal_data *d;
    int rval, errors = 0;
    int signo;

    for (d = signal_data; d->sigstr != NULL; d++) {
	(*ntests)++;
	rval = sudo_str2sig(d->sigstr, &signo);
	if (rval != d->rval) {
	    sudo_warnx_nodebug("FAIL: str2sig(SIG%s): %d != %d",
		d->sigstr, rval, d->rval);
	    errors++;
	    continue;
	}
	if (rval != 0)
	    continue;
	if (signo != d->signo) {
	    sudo_warnx_nodebug("FAIL: signal SIG%s: %d != %d", d->sigstr,
		signo, d->signo);
	    errors++;
	    continue;
	}
    }

    return errors;
}
#else
static int
test_str2sig(int *ntests)
{
    return 0;
}
#endif /* HAVE_STR2SIG */

#if defined(SIGRTMIN) && defined(SIGRTMAX)
static
void init_sigrt(void)
{
    int i;

    /* Initialize real-time signal values. */
    for (i = 0; signal_data[i].signo != -1; i++)
	continue;
    signal_data[i++].signo = SIGRTMIN;
    signal_data[i++].signo = SIGRTMIN + 1;
    signal_data[i++].signo = SIGRTMIN + 2;
    signal_data[i++].signo = SIGRTMIN + 3;
    signal_data[i++].signo = SIGRTMAX - 3;
    signal_data[i++].signo = SIGRTMAX - 2;
    signal_data[i++].signo = SIGRTMAX - 1;
    signal_data[i++].signo = SIGRTMAX;

}
#else
static
void init_sigrt(void)
{
    /* No real-time signals. */
    return;
}
#endif

/*
 * Simple tests for sig2str() and str2sig().
 */
int
main(int argc, char *argv[])
{
    int ch, errors = 0, ntests = 0;

    initprogname(argc > 0 ? argv[0] : "strsig_test");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    init_sigrt();
    errors += test_sig2str(&ntests);
    errors += test_str2sig(&ntests);

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    return errors;
}
