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
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>		/* for struct winsize on HP-UX */

#include <sudo.h>
#include <sudo_exec.h>
#include <sudo_plugin.h>
#include <sudo_plugin_int.h>

static void handle_sigchld_nopty(struct exec_closure *ec);

/*
 * Handle window size change events.
 */
static void
handle_sigwinch(struct exec_closure *ec, int fd)
{
    struct winsize wsize;
    debug_decl(handle_sigwinch, SUDO_DEBUG_EXEC);

    if (fd != -1 && ioctl(fd, TIOCGWINSZ, &wsize) == 0) {
        if (wsize.ws_row != ec->rows || wsize.ws_col != ec->cols) {
            /* Log window change event. */
            log_winchange(ec, wsize.ws_row, wsize.ws_col);

            /* Update rows/cols. */
            ec->rows = wsize.ws_row;
            ec->cols = wsize.ws_col;
        }
    }
}

/* Note: this is basically the same as mon_errpipe_cb() in exec_monitor.c */
static void
errpipe_cb(int fd, int what, void *v)
{
    struct exec_closure *ec = v;
    ssize_t nread;
    int errval;
    debug_decl(errpipe_cb, SUDO_DEBUG_EXEC);

    /*
     * Read errno from child or EOF when command is executed.
     * Note that the error pipe is *blocking*.
     */
    nread = read(fd, &errval, sizeof(errval));
    switch (nread) {
    case -1:
	if (errno != EAGAIN && errno != EINTR) {
	    if (ec->cstat->val == CMD_INVALID) {
		/* XXX - need a way to distinguish non-exec error. */
		ec->cstat->type = CMD_ERRNO;
		ec->cstat->val = errno;
	    }
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		"%s: failed to read error pipe", __func__);
	    sudo_ev_loopbreak(ec->evbase);
	}
	break;
    default:
	if (nread == 0) {
	    /* The error pipe closes when the command is executed. */
	    sudo_debug_printf(SUDO_DEBUG_INFO, "EOF on error pipe");
	} else {
	    /* Errno value when child is unable to execute command. */
	    sudo_debug_printf(SUDO_DEBUG_INFO, "errno from child: %s",
		strerror(errval));
	    ec->cstat->type = CMD_ERRNO;
	    ec->cstat->val = errval;
	}
	sudo_ev_del(ec->evbase, ec->backchannel_event);
	close(fd);
	break;
    }
    debug_return;
}

/* Signal callback */
static void
signal_cb_nopty(int signo, int what, void *v)
{
    struct sudo_ev_siginfo_container *sc = v;
    struct exec_closure *ec = sc->closure;
    char signame[SIG2STR_MAX];
    pid_t si_pgrp;
    debug_decl(signal_cb_nopty, SUDO_DEBUG_EXEC);

    if (ec->cmnd_pid == -1)
	debug_return;

    if (sig2str(signo, signame) == -1)
	(void)snprintf(signame, sizeof(signame), "%d", signo);
    sudo_debug_printf(SUDO_DEBUG_DIAG,
	"%s: evbase %p, command: %d, signo %s(%d), cstat %p",
	__func__, ec->evbase, (int)ec->cmnd_pid, signame, signo, ec->cstat);

    switch (signo) {
    case SIGCHLD:
	handle_sigchld_nopty(ec);
	if (ec->cmnd_pid == -1) {
	    /* Command exited or was killed, exit event loop. */
	    sudo_ev_loopexit(ec->evbase);
	}
	debug_return;
    case SIGWINCH:
	handle_sigwinch(ec, io_fds[SFD_USERTTY]);
	FALLTHROUGH;
#ifdef SIGINFO
    case SIGINFO:
#endif
    case SIGINT:
    case SIGQUIT:
    case SIGTSTP:
	/*
	 * Only forward user-generated signals not sent by a process other than
	 * the command itself or a member of the command's process group (but
	 * only when either sudo or the command is the process group leader).
	 * Signals sent by the kernel may include SIGTSTP when the user presses
	 * ^Z.  Curses programs often trap ^Z and send SIGTSTP to their own
	 * process group, so we don't want to send an extra SIGTSTP.
	 */
	if (!USER_SIGNALED(sc->siginfo))
	    debug_return;
	if (sc->siginfo->si_pid != 0) {
	    if (sc->siginfo->si_pid == ec->cmnd_pid)
		debug_return;
	    si_pgrp = getpgid(sc->siginfo->si_pid);
	    if (si_pgrp != -1) {
		if (si_pgrp == ec->cmnd_pid || si_pgrp == ec->sudo_pid)
		    debug_return;
	    }
	}
	break;
    default:
	/*
	 * Do not forward signals sent by the command itself or a member of the
	 * command's process group (but only when either sudo or the command is
	 * the process group leader).  We don't want the command to indirectly
	 * kill itself.  For example, this can happen with some versions of
	 * reboot that call kill(-1, SIGTERM) to kill all other processes.
	 */
	if (USER_SIGNALED(sc->siginfo) && sc->siginfo->si_pid != 0) {
	    if (sc->siginfo->si_pid == ec->cmnd_pid)
		debug_return;
	    si_pgrp = getpgid(sc->siginfo->si_pid);
	    if (si_pgrp != -1) {
		if (si_pgrp == ec->cmnd_pid || si_pgrp == ec->sudo_pid)
		    debug_return;
	    }
	}
	break;
    }

    /* Send signal to command. */
    if (signo == SIGALRM) {
	terminate_command(ec->cmnd_pid, false);
    } else if (kill(ec->cmnd_pid, signo) != 0) {
	sudo_warn("kill(%d, SIG%s)", (int)ec->cmnd_pid, signame);
    }

    debug_return;
}


/*
 * Fill in the exec closure and setup initial exec events.
 * Allocates events for the signal pipe and error pipe.
 */
static void
fill_exec_closure(struct exec_closure *ec, struct command_status *cstat,
    struct command_details *details, const struct user_details *user_details,
    struct sudo_event_base *evbase, int errfd)
{
    debug_decl(fill_exec_closure, SUDO_DEBUG_EXEC);

    /* Fill in the non-event part of the closure. */
    ec->sudo_pid = getpid();
    ec->ppgrp = getpgrp();
    ec->cstat = cstat;
    ec->details = details;
    ec->rows = user_details->ts_rows;
    ec->cols = user_details->ts_cols;

    /* Setup event base and events. */
    ec->evbase = evbase;

    /* Event for command status via errfd. */
    ec->backchannel_event = sudo_ev_alloc(errfd,
	SUDO_EV_READ|SUDO_EV_PERSIST, errpipe_cb, ec);
    if (ec->backchannel_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->backchannel_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));
    sudo_debug_printf(SUDO_DEBUG_INFO, "error pipe fd %d\n", errfd);

    /* Events for local signals. */
    ec->sigint_event = sudo_ev_alloc(SIGINT,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigint_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigint_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigquit_event = sudo_ev_alloc(SIGQUIT,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigquit_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigquit_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigtstp_event = sudo_ev_alloc(SIGTSTP,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigtstp_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigtstp_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigterm_event = sudo_ev_alloc(SIGTERM,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigterm_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigterm_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sighup_event = sudo_ev_alloc(SIGHUP,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sighup_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sighup_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigalrm_event = sudo_ev_alloc(SIGALRM,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigalrm_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigalrm_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigpipe_event = sudo_ev_alloc(SIGPIPE,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigpipe_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigpipe_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigusr1_event = sudo_ev_alloc(SIGUSR1,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigusr1_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigusr1_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigusr2_event = sudo_ev_alloc(SIGUSR2,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigusr2_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigusr2_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigchld_event = sudo_ev_alloc(SIGCHLD,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigchld_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigchld_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    ec->sigcont_event = sudo_ev_alloc(SIGCONT,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigcont_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigcont_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

#ifdef SIGINFO
    ec->siginfo_event = sudo_ev_alloc(SIGINFO,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->siginfo_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->siginfo_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));
#endif

    ec->sigwinch_event = sudo_ev_alloc(SIGWINCH,
	SUDO_EV_SIGINFO, signal_cb_nopty, ec);
    if (ec->sigwinch_event == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (sudo_ev_add(ec->evbase, ec->sigwinch_event, NULL, false) == -1)
	sudo_fatal("%s", U_("unable to add event to queue"));

    /* Set the default event base. */
    sudo_ev_base_setdef(ec->evbase);

    debug_return;
}

/*
 * Read an iobuf that is ready.
 */
static void
read_callback(int fd, int what, void *v)
{
    struct io_buffer *iob = v;
    struct sudo_event_base *evbase = sudo_ev_get_base(iob->revent);
    ssize_t n;
    debug_decl(read_callback, SUDO_DEBUG_EXEC);

    n = read(fd, iob->buf + iob->len, sizeof(iob->buf) - iob->len);
    switch (n) {
	case -1:
	    if (errno == EAGAIN || errno == EINTR) {
		/* Not an error, retry later. */
		break;
	    }
	    /* Treat read error as fatal and close the fd. */
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"error reading fd %d: %s", fd, strerror(errno));
	    FALLTHROUGH;
	case 0:
	    /* got EOF */
	    if (n == 0) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "read EOF from fd %d", fd);
	    }
	    safe_close(fd);
	    ev_free_by_fd(evbase, fd);
	    /* If writer already consumed the buffer, close it too. */
	    if (iob->wevent != NULL && iob->off == iob->len) {
		safe_close(sudo_ev_get_fd(iob->wevent));
		ev_free_by_fd(evbase, sudo_ev_get_fd(iob->wevent));
		iob->off = iob->len = 0;
	    }
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"read %zd bytes from fd %d", n, fd);
	    if (!iob->action(iob->buf + iob->len, (unsigned int)n, iob)) {
		terminate_command(iob->ec->cmnd_pid, false);
		iob->ec->cmnd_pid = -1;
	    }
	    iob->len += (unsigned int)n;
	    /* Disable reader if buffer is full. */
	    if (iob->len == sizeof(iob->buf))
		sudo_ev_del(evbase, iob->revent);
	    /* Enable writer now that there is new data in the buffer. */
	    if (iob->wevent != NULL) {
		if (sudo_ev_add(evbase, iob->wevent, NULL, false) == -1)
		    sudo_fatal("%s", U_("unable to add event to queue"));
	    }
	    break;
    }

    debug_return;
}

/*
 * Write an iobuf that is ready.
 */
static void
write_callback(int fd, int what, void *v)
{
    struct io_buffer *iob = v;
    struct sudo_event_base *evbase = sudo_ev_get_base(iob->wevent);
    ssize_t n;
    debug_decl(write_callback, SUDO_DEBUG_EXEC);

    n = write(fd, iob->buf + iob->off, iob->len - iob->off);
    if (n == -1) {
	switch (errno) {
	case EPIPE:
	case EBADF:
	    /* other end of pipe closed */
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"unable to write %u bytes to fd %d",
		iob->len - iob->off, fd);
	    /* Close reader if there is one. */
	    if (iob->revent != NULL) {
		safe_close(sudo_ev_get_fd(iob->revent));
		ev_free_by_fd(evbase, sudo_ev_get_fd(iob->revent));
	    }
	    safe_close(fd);
	    ev_free_by_fd(evbase, fd);
	    break;
	case EINTR:
	case EAGAIN:
	    /* Not an error, retry later. */
	    break;
	default:
	    /* XXX - need a way to distinguish non-exec error. */
	    iob->ec->cstat->type = CMD_ERRNO;
	    iob->ec->cstat->val = errno;
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"error writing fd %d: %s", fd, strerror(errno));
	    sudo_ev_loopbreak(evbase);
	    break;
	}
    } else {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "wrote %zd bytes to fd %d", n, fd);
	iob->off += (unsigned int)n;
	/* Disable writer and reset the buffer if fully consumed. */
	if (iob->off == iob->len) {
	    iob->off = iob->len = 0;
	    sudo_ev_del(evbase, iob->wevent);
	    /* Forward the EOF from reader to writer. */
	    if (iob->revent == NULL) {
		safe_close(fd);
		ev_free_by_fd(evbase, fd);
	    }
	}
	/*
	 * Enable reader if buffer is not full but avoid reading
	 * /dev/tty if the command is no longer running.
	 */
	if (iob->revent != NULL && iob->len != sizeof(iob->buf)) {
	    if (!USERTTY_EVENT(iob->revent) || iob->ec->cmnd_pid != -1) {
		if (sudo_ev_add(evbase, iob->revent, NULL, false) == -1)
		    sudo_fatal("%s", U_("unable to add event to queue"));
	    }
	}
    }

    debug_return;
}

/*
 * If std{in,out,err} are not connected to a terminal, interpose
 * ourselves using a pipe.  Fills in io_pipe[][].
 */
static void
interpose_pipes(struct exec_closure *ec, const char *tty, int io_pipe[3][2])
{
    bool interpose[3] = { false, false, false };
    struct stat sb, tty_sbuf, *tty_sb = NULL;
    struct plugin_container *plugin;
    bool want_winch = false;
    debug_decl(interpose_pipes, SUDO_DEBUG_EXEC);

    /*
     * Determine whether any of std{in,out,err} or window size changes
     * should be logged.
     */
    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->log_stdin)
	    interpose[STDIN_FILENO] = true;
	if (plugin->u.io->log_stdout)
	    interpose[STDOUT_FILENO] = true;
	if (plugin->u.io->log_stderr)
	    interpose[STDERR_FILENO] = true;
	if (plugin->u.io->version >= SUDO_API_MKVERSION(1, 12)) {
	    if (plugin->u.io->change_winsize)
		want_winch = true;
	}
    }

    /*
     * If stdin, stdout or stderr is not the user's tty and logging is
     * enabled, use a pipe to interpose ourselves.
     */
    if (tty != NULL && stat(tty, &tty_sbuf) != -1)
	tty_sb = &tty_sbuf;

    if (interpose[STDIN_FILENO]) {
	if (!fd_matches_tty(STDIN_FILENO, tty_sb, &sb)) {
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"stdin not user's tty, creating a pipe");
	    if (pipe2(io_pipe[STDIN_FILENO], O_CLOEXEC) != 0)
		sudo_fatal("%s", U_("unable to create pipe"));
	    io_buf_new(STDIN_FILENO, io_pipe[STDIN_FILENO][1],
		log_stdin, read_callback, write_callback, ec);
	}
    }
    if (interpose[STDOUT_FILENO]) {
	if (!fd_matches_tty(STDOUT_FILENO, tty_sb, &sb)) {
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"stdout not user's tty, creating a pipe");
	    if (pipe2(io_pipe[STDOUT_FILENO], O_CLOEXEC) != 0)
		sudo_fatal("%s", U_("unable to create pipe"));
	    io_buf_new(io_pipe[STDOUT_FILENO][0], STDOUT_FILENO,
		log_stdout, read_callback, write_callback, ec);
	}
    }
    if (interpose[STDERR_FILENO]) {
	if (!fd_matches_tty(STDERR_FILENO, tty_sb, &sb)) {
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"stderr not user's tty, creating a pipe");
	    if (pipe2(io_pipe[STDERR_FILENO], O_CLOEXEC) != 0)
		sudo_fatal("%s", U_("unable to create pipe"));
	    io_buf_new(io_pipe[STDERR_FILENO][0], STDERR_FILENO,
		log_stderr, read_callback, write_callback, ec);
	}
    }
    if (want_winch) {
	/* Need /dev/tty for SIGWINCH handling. */
	io_fds[SFD_USERTTY] = open(_PATH_TTY, O_RDWR);
    }
}

/*
 * Execute a command and wait for it to finish.
 */
void
exec_nopty(struct command_details *details,
    const struct user_details *user_details,
    struct sudo_event_base *evbase, struct command_status *cstat)
{
    int io_pipe[3][2] = { { -1, -1 }, { -1, -1 }, { -1, -1 } };
    int errpipe[2], intercept_sv[2] = { -1, -1 };
    struct exec_closure ec = { 0 };
    sigset_t set, oset;
    debug_decl(exec_nopty, SUDO_DEBUG_EXEC);

    /*
     * The policy plugin's session init must be run before we fork
     * or certain pam modules won't be able to track their state.
     */
    if (policy_init_session(details) != true)
	sudo_fatalx("%s", U_("policy plugin failed session initialization"));

    /*
     * We use a pipe to get errno if execve(2) fails in the child.
     */
    if (pipe2(errpipe, O_CLOEXEC) != 0)
	sudo_fatal("%s", U_("unable to create pipe"));

    if (ISSET(details->flags, CD_INTERCEPT|CD_LOG_SUBCMDS)) {
	if (!ISSET(details->flags, CD_USE_PTRACE)) {
	    /*
	     * Allocate a socketpair for communicating with sudo_intercept.so.
	     * This must be inherited across exec, hence no FD_CLOEXEC.
	     */
	    if (socketpair(PF_UNIX, SOCK_STREAM, 0, intercept_sv) == -1)
		sudo_fatal("%s", U_("unable to create sockets"));
	}
    }

    /* Interpose std{in,out,err} with pipes if logging I/O. */
    interpose_pipes(&ec, user_details->tty, io_pipe);

    /*
     * Block signals until we have our handlers setup in the parent so
     * we don't miss SIGCHLD if the command exits immediately.
     */
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, &oset);

    /* Check for early termination or suspend signals before we fork. */
    if (sudo_terminated(cstat)) {
	sigprocmask(SIG_SETMASK, &oset, NULL);
	debug_return;
    }

#ifdef HAVE_SELINUX
    if (ISSET(details->flags, CD_RBAC_ENABLED)) {
        if (selinux_relabel_tty(details->tty, -1) == -1) {
	    cstat->type = CMD_ERRNO;
	    cstat->val = errno;
	    debug_return;
	}
	selinux_audit_role_change();
    }
#endif

    ec.cmnd_pid = sudo_debug_fork();
    switch (ec.cmnd_pid) {
    case -1:
	sudo_fatal("%s", U_("unable to fork"));
	break;
    case 0:
	/* child */
	close(errpipe[0]);
	if (intercept_sv[0] != -1)
	    close(intercept_sv[0]);
	/* Replace stdin/stdout/stderr with pipes as needed and exec. */
	if (io_pipe[STDIN_FILENO][0] != -1) {
	    if (dup3(io_pipe[STDIN_FILENO][0], STDIN_FILENO, 0) == -1)
		sudo_fatal("dup3");
	    close(io_pipe[STDIN_FILENO][0]);
	    close(io_pipe[STDIN_FILENO][1]);
	}
	if (io_pipe[STDOUT_FILENO][0] != -1) {
	    if (dup3(io_pipe[STDOUT_FILENO][1], STDOUT_FILENO, 0) == -1)
		sudo_fatal("dup3");
	    close(io_pipe[STDOUT_FILENO][0]);
	    close(io_pipe[STDOUT_FILENO][1]);
	}
	if (io_pipe[STDERR_FILENO][0] != -1) {
	    if (dup3(io_pipe[STDERR_FILENO][1], STDERR_FILENO, 0) == -1)
		sudo_fatal("dup3");
	    close(io_pipe[STDERR_FILENO][0]);
	    close(io_pipe[STDERR_FILENO][1]);
	}
	exec_cmnd(details, &oset, intercept_sv[1], errpipe[1]);
	while (write(errpipe[1], &errno, sizeof(int)) == -1) {
	    if (errno != EINTR)
		break;
	}
	sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys, 1);
	_exit(EXIT_FAILURE);
	/* NOTREACHED */
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "executed %s, pid %d", details->command,
	(int)ec.cmnd_pid);
    /* Close the other end of the pipes and socketpairs. */
    if (io_pipe[STDIN_FILENO][0] != -1)
        close(io_pipe[STDIN_FILENO][0]);
    if (io_pipe[STDOUT_FILENO][1] != -1)
        close(io_pipe[STDOUT_FILENO][1]);
    if (io_pipe[STDERR_FILENO][1] != -1)
        close(io_pipe[STDERR_FILENO][1]);
    close(errpipe[1]);
    if (intercept_sv[1] != -1)
        close(intercept_sv[1]);

    /* No longer need execfd. */
    if (details->execfd != -1) {
	close(details->execfd);
	details->execfd = -1;
    }

    /* Set command timeout if specified. */
    if (ISSET(details->flags, CD_SET_TIMEOUT))
	alarm(details->timeout);

    /*
     * Fill in exec closure, allocate event base, signal events and
     * the error pipe event.
     */
    fill_exec_closure(&ec, cstat, details, user_details, evbase, errpipe[0]);

    if (ISSET(details->flags, CD_INTERCEPT|CD_LOG_SUBCMDS)) {
	int rc = 1;

	/* Create event and closure for intercept mode. */
	ec.intercept = intercept_setup(intercept_sv[0], ec.evbase, details);
	if (ec.intercept == NULL) {
	    rc = -1;
	} else if (ISSET(details->flags, CD_USE_PTRACE)) {
	    /* Try to seize control of the command using ptrace(2). */
	    rc = exec_ptrace_seize(ec.cmnd_pid);
	    if (rc == 0) {
		/* There is another tracer present. */
		CLR(details->flags, CD_INTERCEPT|CD_LOG_SUBCMDS|CD_USE_PTRACE);
	    }
	}
	if (rc == -1)
	    terminate_command(ec.cmnd_pid, false);
    }

    /* Enable any I/O log events. */
    add_io_events(&ec);

    /* Restore signal mask now that signal handlers are setup. */
    sigprocmask(SIG_SETMASK, &oset, NULL);

    /*
     * Non-pty event loop.
     * Wait for command to exit, handles signals and the error pipe.
     */
    if (sudo_ev_dispatch(ec.evbase) == -1)
	sudo_warn("%s", U_("error in event loop"));
    if (sudo_ev_got_break(ec.evbase)) {
	/* error from callback */
	sudo_debug_printf(SUDO_DEBUG_ERROR, "event loop exited prematurely");
	/* kill command */
	terminate_command(ec.cmnd_pid, false);
	ec.cmnd_pid = -1;
    }

#ifdef HAVE_SELINUX
    if (ISSET(details->flags, CD_RBAC_ENABLED)) {
	if (selinux_restore_tty() != 0)
	    sudo_warnx("%s", U_("unable to restore tty label"));
    }
#endif

    /* Flush any remaining output. */
    del_io_events(true);

    /* Free things up. */
    free_io_bufs();
    free_exec_closure(&ec);

    debug_return;
}

/*
 * Wait for command status after receiving SIGCHLD.
 * If the command exits, fill in cstat and stop the event loop.
 * If the command stops, save the tty pgrp, suspend sudo, then restore
 * the tty pgrp when sudo resumes.
 */
static void
handle_sigchld_nopty(struct exec_closure *ec)
{
    pid_t pid;
    int status;
    char signame[SIG2STR_MAX];
    debug_decl(handle_sigchld_nopty, SUDO_DEBUG_EXEC);

    /* There may be multiple children in intercept mode. */
    for (;;) {
	do {
	    pid = waitpid(-1, &status, __WALL|WUNTRACED|WNOHANG);
	} while (pid == -1 && errno == EINTR);
	switch (pid) {
	case -1:
	    if (errno != ECHILD) {
		sudo_warn(U_("%s: %s"), __func__, "waitpid");
		debug_return;
	    }
	    FALLTHROUGH;
	case 0:
	    /* Nothing left to wait for. */
	    debug_return;
	}

	if (WIFSTOPPED(status)) {
	    const int signo = WSTOPSIG(status);

	    if (sig2str(signo, signame) == -1)
		(void)snprintf(signame, sizeof(signame), "%d", signo);
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"%s: process %d stopped, SIG%s", __func__, (int)pid, signame);

	    if (ISSET(ec->details->flags, CD_USE_PTRACE)) {
		/* If not a group-stop signal, just continue. */
		if (!exec_ptrace_stopped(pid, status, ec->intercept))
		    continue;
	    }

	    /* If the main command is suspended, suspend sudo too. */
	    if (pid == ec->cmnd_pid) {
		sudo_suspend_parent(signo, ec->sudo_pid, ec->ppgrp,
		    ec->cmnd_pid, ec, log_suspend);
	    }
	} else {
	    if (WIFSIGNALED(status)) {
		if (sig2str(WTERMSIG(status), signame) == -1) {
		    (void)snprintf(signame, sizeof(signame), "%d",
			WTERMSIG(status));
		}
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "%s: process %d killed, SIG%s", __func__,
		    (int)pid, signame);
	    } else if (WIFEXITED(status)) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "%s: process %d exited: %d", __func__,
		    (int)pid, WEXITSTATUS(status));
	    } else {
		sudo_debug_printf(SUDO_DEBUG_WARN,
		    "%s: unexpected wait status 0x%x for process %d",
		    __func__, status, (int)pid);
	    }

	    /* Only store exit status of the main command. */
	    if (pid != ec->cmnd_pid)
		continue;

	    /* Don't overwrite execve() failure with command exit status. */
	    if (ec->cstat->type == CMD_INVALID) {
		ec->cstat->type = CMD_WSTATUS;
		ec->cstat->val = status;
	    } else {
		sudo_debug_printf(SUDO_DEBUG_WARN,
		    "%s: not overwriting command status %d,%d with %d,%d",
		    __func__, ec->cstat->type, ec->cstat->val,
		    CMD_WSTATUS, status);
	    }
	    ec->cmnd_pid = -1;
	}
    }
}
