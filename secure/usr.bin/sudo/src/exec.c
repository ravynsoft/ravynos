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

#include <sys/resource.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#ifdef HAVE_LOGIN_CAP_H
# include <login_cap.h>
# ifndef LOGIN_SETENV
#  define LOGIN_SETENV  0
# endif
#endif
#ifdef HAVE_PROJECT_H
# include <project.h>
# include <sys/task.h>
#endif

#include <sudo.h>
#include <sudo_exec.h>
#include <sudo_plugin.h>
#include <sudo_plugin_int.h>

#ifdef HAVE_PTRACE_INTERCEPT
static void
handler(int signo)
{
    /* just return */
}
#endif /* HAVE_PTRACE_INTERCEPT */

static void
close_fds(struct command_details *details, int errfd, int intercept_fd)
{
    int fd, maxfd;
    unsigned char *debug_fds;
    debug_decl(close_fds, SUDO_DEBUG_EXEC);

    if (details->closefrom < 0)
	debug_return;

    /* Preserve debug fds and error pipe as needed. */
    maxfd = sudo_debug_get_fds(&debug_fds);
    for (fd = 0; fd <= maxfd; fd++) {
	if (sudo_isset(debug_fds, fd))
	    add_preserved_fd(&details->preserved_fds, fd);
    }
    if (errfd != -1)
	add_preserved_fd(&details->preserved_fds, errfd);
    if (intercept_fd != -1)
	add_preserved_fd(&details->preserved_fds, intercept_fd);

    /* Close all fds except those explicitly preserved. */
    closefrom_except(details->closefrom, &details->preserved_fds);

    debug_return;
}

/*
 * Setup the execution environment immediately prior to the call to execve().
 * Group setup is performed by policy_init_session(), called earlier.
 * Returns true on success and false on failure.
 */
static bool
exec_setup(struct command_details *details, int intercept_fd, int errfd)
{
    bool ret = false;
    debug_decl(exec_setup, SUDO_DEBUG_EXEC);

#ifdef HAVE_PTRACE_INTERCEPT
    if (ISSET(details->flags, CD_USE_PTRACE)) {
	if (!set_exec_filter())
	    goto done;
    }
#endif /* HAVE_PTRACE_INTERCEPT */

    if (details->pw != NULL) {
#ifdef HAVE_PROJECT_H
	set_project(details->pw);
#endif
#ifdef HAVE_PRIV_SET
	if (details->privs != NULL) {
	    if (setppriv(PRIV_SET, PRIV_INHERITABLE, details->privs) != 0) {
		sudo_warn("%s", U_("unable to set privileges"));
		goto done;
	    }
	}
	if (details->limitprivs != NULL) {
	    if (setppriv(PRIV_SET, PRIV_LIMIT, details->limitprivs) != 0) {
		sudo_warn("%s", U_("unable to set limit privileges"));
		goto done;
	    }
	} else if (details->privs != NULL) {
	    if (setppriv(PRIV_SET, PRIV_LIMIT, details->privs) != 0) {
		sudo_warn("%s", U_("unable to set limit privileges"));
		goto done;
	    }
	}
#endif /* HAVE_PRIV_SET */

#ifdef HAVE_GETUSERATTR
	if (aix_prep_user(details->pw->pw_name, details->tty) != 0) {
	    /* error message displayed by aix_prep_user */
	    goto done;
	}
#endif
#ifdef HAVE_LOGIN_CAP_H
	if (details->login_class) {
	    unsigned int flags;
	    login_cap_t *lc;

	    /*
	     * We only use setusercontext() to set the nice value, rlimits
	     * and umask unless this is a login shell (sudo -i).
	     */
	    lc = login_getclass((char *)details->login_class);
	    if (!lc) {
		sudo_warnx(U_("unknown login class %s"), details->login_class);
		errno = ENOENT;
		goto done;
	    }
	    if (ISSET(details->flags, CD_LOGIN_SHELL)) {
		/* Set everything except user, group and login name. */
		flags = LOGIN_SETALL;
		CLR(flags, LOGIN_SETGROUP|LOGIN_SETLOGIN|LOGIN_SETUSER|LOGIN_SETENV|LOGIN_SETPATH);
	    } else {
		flags = LOGIN_SETRESOURCES|LOGIN_SETPRIORITY|LOGIN_SETUMASK;
	    }
	    if (setusercontext(lc, details->pw, details->pw->pw_uid, flags)) {
		sudo_warn("%s", U_("unable to set user context"));
		if (details->pw->pw_uid != ROOT_UID)
		    goto done;
	    }
	}
#endif /* HAVE_LOGIN_CAP_H */
    }

    if (ISSET(details->flags, CD_SET_GROUPS)) {
	/* set_user_groups() prints error message on failure. */
	if (!set_user_groups(details))
	    goto done;
    }

    if (ISSET(details->flags, CD_SET_PRIORITY)) {
	if (setpriority(PRIO_PROCESS, 0, details->priority) != 0) {
	    sudo_warn("%s", U_("unable to set process priority"));
	    goto done;
	}
    }

    /* Policy may override umask in PAM or login.conf. */
    if (ISSET(details->flags, CD_OVERRIDE_UMASK))
	(void) umask(details->umask);

    /* Apply resource limits specified by the policy, if any. */
    set_policy_rlimits();

    /* Close fds before chroot (need /dev) or uid change (prlimit on Linux). */
    close_fds(details, errfd, intercept_fd);

    if (details->chroot) {
	if (chroot(details->chroot) != 0 || chdir("/") != 0) {
	    sudo_warn(U_("unable to change root to %s"), details->chroot);
	    goto done;
	}
    }

    /*
     * Unlimit the number of processes since Linux's setuid() will
     * return EAGAIN if RLIMIT_NPROC would be exceeded by the uid switch.
     */
    unlimit_nproc();

#if defined(HAVE_SETRESUID)
    if (setresuid(details->cred.uid, details->cred.euid, details->cred.euid) != 0) {
	sudo_warn(U_("unable to change to runas uid (%u, %u)"),
	    (unsigned int)details->cred.uid, (unsigned int)details->cred.euid);
	goto done;
    }
#elif defined(HAVE_SETREUID)
    if (setreuid(details->cred.uid, details->cred.euid) != 0) {
	sudo_warn(U_("unable to change to runas uid (%u, %u)"),
	    (unsigned int)details->cred.uid, (unsigned int)details->cred.euid);
	goto done;
    }
#else
    /* Cannot support real user-ID that is different from effective user-ID. */
    if (setuid(details->cred.euid) != 0) {
	sudo_warn(U_("unable to change to runas uid (%u, %u)"),
	    (unsigned int)details->cred.euid, (unsigned int)details->cred.euid);
	goto done;
    }
#endif /* !HAVE_SETRESUID && !HAVE_SETREUID */

    /* Restore previous value of RLIMIT_NPROC. */
    restore_nproc();

    /*
     * Only change cwd if we have chroot()ed or the policy modules
     * specifies a different cwd.  Must be done after uid change.
     */
    if (details->runcwd != NULL) {
	if (details->chroot != NULL || details->submitcwd == NULL ||
	    strcmp(details->runcwd, details->submitcwd) != 0) {
	    if (ISSET(details->flags, CD_RBAC_ENABLED)) {
		 /* For SELinux, chdir(2) in sesh after the context change. */
		SET(details->flags, CD_RBAC_SET_CWD);
	    } else {
		/* Note: runcwd is relative to the new root, if any. */
		if (chdir(details->runcwd) == -1) {
		    sudo_warn(U_("unable to change directory to %s"),
			details->runcwd);
		    if (!ISSET(details->flags, CD_CWD_OPTIONAL))
			goto done;
		    if (details->chroot != NULL)
			sudo_warnx(U_("starting from %s"), "/");
		}
	    }
	}
    }

    ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Setup the execution environment and execute the command.
 * If SELinux is enabled, run the command via sesh, otherwise
 * execute it directly.
 * If the exec fails, cstat is filled in with the value of errno.
 */
void
exec_cmnd(struct command_details *details, sigset_t *mask,
    int intercept_fd, int errfd)
{
    debug_decl(exec_cmnd, SUDO_DEBUG_EXEC);

#ifdef HAVE_PTRACE_INTERCEPT
    if (ISSET(details->flags, CD_USE_PTRACE)) {
	struct sigaction sa;
	sigset_t set;

	/* Tracer will send us SIGUSR1 when it is time to proceed. */
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = handler;
	if (sudo_sigaction(SIGUSR1, &sa, NULL) != 0) {
	    sudo_warn(U_("unable to set handler for signal %d"),
		SIGUSR1);
	}

	/* Suspend child until tracer seizes control and sends SIGUSR1. */
	sigfillset(&set);
	sigdelset(&set, SIGUSR1);
	sigsuspend(&set);
    }
#endif /* HAVE_PTRACE_INTERCEPT */

    if (mask != NULL)
	sigprocmask(SIG_SETMASK, mask, NULL);
    restore_signals();
    if (exec_setup(details, intercept_fd, errfd) == true) {
	/* headed for execve() */
#ifdef HAVE_SELINUX
	if (ISSET(details->flags, CD_RBAC_ENABLED)) {
	    selinux_execve(details->execfd, details->command, details->argv,
		details->envp, details->runcwd, details->flags);
	} else
#endif
	{
	    sudo_execve(details->execfd, details->command, details->argv,
		details->envp, intercept_fd, details->flags);
	}
    }
    sudo_debug_printf(SUDO_DEBUG_ERROR, "unable to exec %s: %s",
	details->command, strerror(errno));
    debug_return;
}

/*
 * Check for caught signals sent to sudo before command execution.
 * Also suspends the process if SIGTSTP was caught.
 * Returns true if we should terminate, else false.
 */
bool
sudo_terminated(struct command_status *cstat)
{
    int signo;
    bool sigtstp = false;
    debug_decl(sudo_terminated, SUDO_DEBUG_EXEC);

    for (signo = 0; signo < NSIG; signo++) {
	if (signal_pending(signo)) {
	    switch (signo) {
	    case SIGCHLD:
		/* Ignore. */
		break;
	    case SIGTSTP:
		/* Suspend below if not terminated. */
		sigtstp = true;
		break;
	    default:
		/* Terminal signal, do not exec command. */
		cstat->type = CMD_WSTATUS;
		cstat->val = signo + 128;
		debug_return_bool(true);
		break;
	    }
	}
    }
    if (sigtstp) {
	struct sigaction sa;
	sigset_t set, oset;

	/* Send SIGTSTP to ourselves, unblocking it if needed. */
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = SIG_DFL;
	if (sudo_sigaction(SIGTSTP, &sa, NULL) != 0)
	    sudo_warn(U_("unable to set handler for signal %d"), SIGTSTP);
	sigemptyset(&set);
	sigaddset(&set, SIGTSTP);
	sigprocmask(SIG_UNBLOCK, &set, &oset);
	if (kill(getpid(), SIGTSTP) != 0)
	    sudo_warn("kill(%d, SIGTSTP)", (int)getpid());
	sigprocmask(SIG_SETMASK, &oset, NULL);
	/* No need to restore old SIGTSTP handler. */
    }
    debug_return_bool(false);
}

static bool
sudo_needs_pty(const struct command_details *details)
{
    struct plugin_container *plugin;

    if (ISSET(details->flags, CD_USE_PTY))
	return true;

    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->log_ttyin != NULL ||
	    plugin->u.io->log_ttyout != NULL)
	    return true;
    }
    return false;
}

/*
 * Check whether the specified fd matches the device file that
 * corresponds to tty_sb.  If tty_sb is NULL, just check whether
 * fd is a tty.  Always fills in fd_sb (zeroed on error).
 * Returns true on match, else false.
 */
bool
fd_matches_tty(int fd, struct stat *tty_sb, struct stat *fd_sb)
{
    debug_decl(fd_is_user_tty, SUDO_DEBUG_EXEC);

    if (fstat(fd, fd_sb) == -1) {
	 /* Always initialize fd_sb. */
	memset(fd_sb, 0, sizeof(*fd_sb));
	debug_return_bool(false);
    }
    if (!S_ISCHR(fd_sb->st_mode))
	debug_return_bool(false);

    /* Compare with tty_sb if available, else just check that fd is a tty. */
    debug_return_bool(tty_sb ? tty_sb->st_rdev == fd_sb->st_rdev : isatty(fd));
}

/*
 * If we are not running the command in a pty, we were not invoked as
 * sudoedit, there is no command timeout and there is no close function,
 * sudo can exec the command directly (and not wait).
 */
static bool
direct_exec_allowed(const struct command_details *details)
{
    struct plugin_container *plugin;
    debug_decl(direct_exec_allowed, SUDO_DEBUG_EXEC);

    /* Assumes sudo_needs_pty() was already checked. */
    if (ISSET(details->flags, CD_RBAC_ENABLED|CD_SET_TIMEOUT|CD_SUDOEDIT) ||
	    policy_plugin.u.policy->close != NULL)
	debug_return_bool(false);

    TAILQ_FOREACH(plugin, &audit_plugins, entries) {
	if (plugin->u.audit->close != NULL)
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

/*
 * Execute a command, potentially in a pty with I/O logging, and
 * wait for it to finish.
 * This is a little bit tricky due to how POSIX job control works and
 * we fact that we have two different controlling terminals to deal with.
 */
int
sudo_execute(struct command_details *details,
    const struct user_details *user_details,
    struct sudo_event_base *evbase, struct command_status *cstat)
{
    debug_decl(sudo_execute, SUDO_DEBUG_EXEC);

#if defined(HAVE_SELINUX) && !defined(HAVE_PTRACE_INTERCEPT)
    /*
     * SELinux prevents LD_PRELOAD from functioning so we must use
     * ptrace-based intercept mode.
     */
    if (details->selinux_role != NULL || details->selinux_type != NULL) {
	if (ISSET(details->flags, CD_INTERCEPT)) {
	    sudo_warnx("%s",
		U_("intercept mode is not supported with SELinux RBAC on this system"));
	    CLR(details->flags, CD_INTERCEPT);
	}
	if (ISSET(details->flags, CD_LOG_SUBCMDS)) {
	    sudo_warnx("%s",
		U_("unable to log sub-commands with SELinux RBAC on this system"));
	    CLR(details->flags, CD_LOG_SUBCMDS);
	}
    }
#endif /* HAVE_SELINUX && !HAVE_PTRACE_INTERCEPT */

    /* If running in background mode, fork and exit. */
    if (ISSET(details->flags, CD_BACKGROUND)) {
	switch (sudo_debug_fork()) {
	    case -1:
		cstat->type = CMD_ERRNO;
		cstat->val = errno;
		debug_return_int(-1);
	    case 0:
		/*
		 * Child continues in an orphaned process group.
		 * Reads from the terminal fail with EIO.
		 * Writes succeed unless tostop is set on the terminal.
		 */
		(void)setpgid(0, 0);
		break;
	    default:
		/* parent exits (but does not flush buffers) */
		sudo_debug_exit_int(__func__, __FILE__, __LINE__,
		    sudo_debug_subsys, 0);
		_exit(EXIT_SUCCESS);
	}
    }

    /*
     * Restore resource limits before running.
     * We must do this *before* calling the PAM session module.
     */
    restore_limits();

    /*
     * Run the command in a new pty if there is an I/O plugin or the policy
     * has requested a pty.  If /dev/tty is unavailable and no I/O plugin
     * is configured, this returns false and we run the command without a pty.
     */
    if (sudo_needs_pty(details)) {
	if (exec_pty(details, user_details, evbase, cstat))
	    goto done;
    }

    /*
     * If we are not running the command in a pty, we may be able to
     * exec directly, depending on the plugins used.
     */
    if (direct_exec_allowed(details)) {
	if (!sudo_terminated(cstat)) {
	    exec_cmnd(details, NULL, -1, -1);
	    cstat->type = CMD_ERRNO;
	    cstat->val = errno;
	}
	goto done;
    }

    /*
     * Run the command in the existing tty (if any) and wait for it to finish.
     */
    exec_nopty(details, user_details, evbase, cstat);

done:
    /* The caller will run any plugin close functions. */
    debug_return_int(cstat->type == CMD_ERRNO ? -1 : 0);
}

/*
 * Kill command with increasing urgency.
 */
void
terminate_command(pid_t pid, bool use_pgrp)
{
    debug_decl(terminate_command, SUDO_DEBUG_EXEC);

    /* Avoid killing more than a single process or process group. */
    if (pid <= 0)
	debug_return;

    /*
     * Note that SIGCHLD will interrupt the sleep()
     */
    if (use_pgrp) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "killpg %d SIGHUP", (int)pid);
	killpg(pid, SIGHUP);
	sudo_debug_printf(SUDO_DEBUG_INFO, "killpg %d SIGTERM", (int)pid);
	killpg(pid, SIGTERM);
	sleep(2);
	sudo_debug_printf(SUDO_DEBUG_INFO, "killpg %d SIGKILL", (int)pid);
	killpg(pid, SIGKILL);
    } else {
	sudo_debug_printf(SUDO_DEBUG_INFO, "kill %d SIGHUP", (int)pid);
	kill(pid, SIGHUP);
	sudo_debug_printf(SUDO_DEBUG_INFO, "kill %d SIGTERM", (int)pid);
	kill(pid, SIGTERM);
	sleep(2);
	sudo_debug_printf(SUDO_DEBUG_INFO, "kill %d SIGKILL", (int)pid);
	kill(pid, SIGKILL);
    }

    debug_return;
}

/*
 * Free the dynamically-allocated contents of the exec closure.
 */
void
free_exec_closure(struct exec_closure *ec)
{
    debug_decl(free_exec_closure, SUDO_DEBUG_EXEC);

    /* Free any remaining intercept resources. */
    intercept_cleanup(ec);

    sudo_ev_base_free(ec->evbase);
    sudo_ev_free(ec->backchannel_event);
    sudo_ev_free(ec->fwdchannel_event);
    sudo_ev_free(ec->sigint_event);
    sudo_ev_free(ec->sigquit_event);
    sudo_ev_free(ec->sigtstp_event);
    sudo_ev_free(ec->sigterm_event);
    sudo_ev_free(ec->sighup_event);
    sudo_ev_free(ec->sigalrm_event);
    sudo_ev_free(ec->sigpipe_event);
    sudo_ev_free(ec->sigusr1_event);
    sudo_ev_free(ec->sigusr2_event);
    sudo_ev_free(ec->sigchld_event);
    sudo_ev_free(ec->sigcont_event);
    sudo_ev_free(ec->siginfo_event);
    sudo_ev_free(ec->sigwinch_event);
    free(ec->ptyname);

    debug_return;
}
