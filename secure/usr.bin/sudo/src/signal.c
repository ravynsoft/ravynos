/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sudo.h>
#include <sudo_exec.h>

static struct signal_state {
    int signo;
    int restore;
    struct sigaction sa;
} saved_signals[] = {
    { SIGALRM },	/* SAVED_SIGALRM */
    { SIGCHLD },	/* SAVED_SIGCHLD */
    { SIGCONT },	/* SAVED_SIGCONT */
    { SIGHUP },		/* SAVED_SIGHUP */
    { SIGINT },		/* SAVED_SIGINT */
    { SIGPIPE },	/* SAVED_SIGPIPE */
    { SIGQUIT },	/* SAVED_SIGQUIT */
    { SIGTERM },	/* SAVED_SIGTERM */
    { SIGTSTP },	/* SAVED_SIGTSTP */
    { SIGTTIN },	/* SAVED_SIGTTIN */
    { SIGTTOU },	/* SAVED_SIGTTOU */
    { SIGUSR1 },	/* SAVED_SIGUSR1 */
    { SIGUSR2 },	/* SAVED_SIGUSR2 */
    { -1 }
};

static sig_atomic_t pending_signals[NSIG];

static void
sudo_handler(int signo)
{
    /* Mark signal as pending. */
    pending_signals[signo] = 1;
}

bool
signal_pending(int signo)
{
    return pending_signals[signo] == 1;
}

/*
 * Save signal handler state so it can be restored before exec.
 */
void
save_signals(void)
{
    struct signal_state *ss;
    debug_decl(save_signals, SUDO_DEBUG_MAIN);

    for (ss = saved_signals; ss->signo != -1; ss++) {
	if (sigaction(ss->signo, NULL, &ss->sa) != 0)
	    sudo_warn(U_("unable to save handler for signal %d"), ss->signo);
    }

    debug_return;
}

/*
 * Restore signal handlers to initial state for exec.
 */
void
restore_signals(void)
{
    struct signal_state *ss;
    debug_decl(restore_signals, SUDO_DEBUG_MAIN);

    for (ss = saved_signals; ss->signo != -1; ss++) {
	if (ss->restore) {
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"restoring handler for signal %d: %s", ss->signo,
		ss->sa.sa_handler == SIG_IGN ? "SIG_IGN" :
		ss->sa.sa_handler == SIG_DFL ? "SIG_DFL" : "???");
	    if (sigaction(ss->signo, &ss->sa, NULL) != 0) {
		sudo_warn(U_("unable to restore handler for signal %d"),
		    ss->signo);
	    }
	}
    }

    debug_return;
}

/*
 * Trap tty-generated (and other) signals so we can't be killed before
 * calling the policy close function.  The signal pipe will be drained
 * in sudo_execute() before running the command and new handlers will
 * be installed in the parent.
 */
void
init_signals(void)
{
    struct sigaction sa;
    struct signal_state *ss;
    debug_decl(init_signals, SUDO_DEBUG_MAIN);

    memset(&sa, 0, sizeof(sa));
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sudo_handler;

    for (ss = saved_signals; ss->signo > 0; ss++) {
	switch (ss->signo) {
	    case SIGCONT:
	    case SIGPIPE:
	    case SIGTTIN:
	    case SIGTTOU:
		/* Don't install these until exec time. */
		break;
	    case SIGCHLD:
		/* Sudo needs to be able to catch SIGCHLD. */
		if (ss->sa.sa_handler == SIG_IGN) {
		    sudo_debug_printf(SUDO_DEBUG_INFO,
			"will restore signal %d on exec", SIGCHLD);
		    ss->restore = true;
		}
		if (sigaction(SIGCHLD, &sa, NULL) != 0) {
		    sudo_warn(U_("unable to set handler for signal %d"),
			SIGCHLD);
		}
		break;
	    default:
		if (ss->sa.sa_handler != SIG_IGN) {
		    if (sigaction(ss->signo, &sa, NULL) != 0) {
			sudo_warn(U_("unable to set handler for signal %d"),
			    ss->signo);
		    }
		}
		break;
	}
    }
    /* Ignore SIGPIPE until exec. */
    if (saved_signals[SAVED_SIGPIPE].sa.sa_handler != SIG_IGN) {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "will restore signal %d on exec", SIGPIPE);
	saved_signals[SAVED_SIGPIPE].restore = true;
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) != 0)
	    sudo_warn(U_("unable to set handler for signal %d"), SIGPIPE);
    }

    debug_return;
}

/*
 * Like sigaction() but sets restore flag in saved_signals[]
 * if needed.
 */
int
sudo_sigaction(int signo, struct sigaction *sa, struct sigaction *osa)
{
    struct signal_state *ss;
    int ret;
    debug_decl(sudo_sigaction, SUDO_DEBUG_MAIN);

    for (ss = saved_signals; ss->signo > 0; ss++) {
	if (ss->signo == signo) {
	    /* If signal was or now is ignored, restore old handler on exec. */
	    if (ss->sa.sa_handler == SIG_IGN || sa->sa_handler == SIG_IGN) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "will restore signal %d on exec", signo);
		ss->restore = true;
	    }
	    break;
	}
    }
    ret = sigaction(signo, sa, osa);

    debug_return_int(ret);
}
