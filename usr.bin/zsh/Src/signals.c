/*
 * signals.c - signals handling code
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zsh.mdh"
#include "signals.pro"
 
/* Array describing the state of each signal: an element contains *
 * 0 for the default action or some ZSIG_* flags ored together.   */

/**/
mod_export int sigtrapped[VSIGCOUNT];

/*
 * Trap programme lists for each signal.
 *
 * If (sigtrapped[sig] & ZSIG_FUNC) is set, this isn't used.
 * The corresponding shell function is used instead.
 *
 * Otherwise, if sigtrapped[sig] is not zero, this is NULL when a signal
 * is to be ignored, and if not NULL contains the programme list to be
 * eval'd.
 */

/**/
mod_export Eprog siglists[VSIGCOUNT];

/* Total count of trapped signals */

/**/
mod_export volatile int nsigtrapped;

/* Running an exit trap? */

/**/
int in_exit_trap;

/*
 * Flag that exit trap has been set in POSIX mode.
 * The setter's expectation is therefore that it is run
 * on programme exit, not function exit.
 */

/**/
static int exit_trap_posix;

/* Variables used by signal queueing */

/**/
mod_export volatile int queueing_enabled, queue_front, queue_rear;
/**/
mod_export int signal_queue[MAX_QUEUE_SIZE];
/**/
mod_export sigset_t signal_mask_queue[MAX_QUEUE_SIZE];
#ifdef DEBUG
/**/
mod_export volatile int queue_in;
#endif

/* Variables used by trap queueing */

/**/
mod_export volatile int trap_queueing_enabled, trap_queue_front, trap_queue_rear;
/**/
mod_export int trap_queue[MAX_QUEUE_SIZE];

/* This is only used on machines that don't understand signal sets.  *
 * On SYSV machines this will represent the signals that are blocked *
 * (held) using sighold.  On machines which can't block signals at   *
 * all, we will simulate this by ignoring them and remembering them  *
 * in this variable.                                                 */
#if !defined(POSIX_SIGNALS) && !defined(BSD_SIGNALS)
static sigset_t blocked_set;
#endif

#ifdef POSIX_SIGNALS
# define signal_jmp_buf       sigjmp_buf
# define signal_setjmp(b)     sigsetjmp((b),1)
# define signal_longjmp(b,n)  siglongjmp((b),(n))
#else
# define signal_jmp_buf       jmp_buf
# define signal_setjmp(b)     setjmp(b)
# define signal_longjmp(b,n)  longjmp((b),(n))
#endif
 
#ifdef NO_SIGNAL_BLOCKING
# define signal_process(sig)  signal_ignore(sig)
# define signal_reset(sig)    install_handler(sig)
#else
# define signal_process(sig)  ;
# define signal_reset(sig)    ;
#endif

/* Install signal handler for given signal.           *
 * If possible, we want to make sure that interrupted *
 * system calls are not restarted.                    */

/**/
mod_export void
install_handler(int sig)
{
#ifdef POSIX_SIGNALS
    struct sigaction act;
 
    act.sa_handler = (SIGNAL_HANDTYPE) zhandler;
    sigemptyset(&act.sa_mask);        /* only block sig while in handler */
    act.sa_flags = 0;
# ifdef SA_INTERRUPT                  /* SunOS 4.x */
    if (interact)
        act.sa_flags |= SA_INTERRUPT; /* make sure system calls are not restarted */
# endif
    sigaction(sig, &act, (struct sigaction *)NULL);
#else
# ifdef BSD_SIGNALS
    struct sigvec vec;
 
    vec.sv_handler = (SIGNAL_HANDTYPE) zhandler;
    vec.sv_mask = sigmask(sig);    /* mask out this signal while in handler    */
#  ifdef SV_INTERRUPT
    vec.sv_flags = SV_INTERRUPT;   /* make sure system calls are not restarted */
#  endif
    sigvec(sig, &vec, (struct sigvec *)NULL);
# else
#  ifdef SYSV_SIGNALS
    /* we want sigset rather than signal because it will   *
     * block sig while in handler.  signal usually doesn't */
    sigset(sig, zhandler);
#  else  /* NO_SIGNAL_BLOCKING (bummer) */
    signal(sig, zhandler);

#  endif /* SYSV_SIGNALS  */
# endif  /* BSD_SIGNALS   */
#endif   /* POSIX_SIGNALS */
}

/* enable ^C interrupts */
 
/**/
mod_export void
intr(void)
{
    if (interact)
        install_handler(SIGINT);
}

/* disable ^C interrupts */
 
#if 0 /**/
void
nointr(void)
{
    if (interact)
        signal_ignore(SIGINT);
}
#endif
 
/* temporarily block ^C interrupts */
 
/**/
mod_export void
holdintr(void)
{
    if (interact)
        signal_block(signal_mask(SIGINT));
}

/* release ^C interrupts */
 
/**/
mod_export void
noholdintr(void)
{
    if (interact)
        signal_unblock(signal_mask(SIGINT));
}
 
/* create a signal mask containing *
 * only the given signal           */
 
/**/
mod_export sigset_t
signal_mask(int sig)
{
    sigset_t set;
 
    sigemptyset(&set);
    if (sig)
        sigaddset(&set, sig);
    return set;
}

/* Block the signals in the given signal *
 * set. Return the old signal set.       */

/**/
#ifndef BSD_SIGNALS

/**/
mod_export sigset_t
signal_block(sigset_t set)
{
    sigset_t oset;
 
#ifdef POSIX_SIGNALS
    sigprocmask(SIG_BLOCK, &set, &oset);

#else
# ifdef SYSV_SIGNALS
    int i;
 
    oset = blocked_set;
    for (i = 1; i <= NSIG; ++i) {
        if (sigismember(&set, i) && !sigismember(&blocked_set, i)) {
            sigaddset(&blocked_set, i);
            sighold(i);
        }
    }
# else  /* NO_SIGNAL_BLOCKING */
/* We will just ignore signals if the system doesn't have *
 * the ability to block them.                             */
    int i;

    oset = blocked_set;
    for (i = 1; i <= NSIG; ++i) {
        if (sigismember(&set, i) && !sigismember(&blocked_set, i)) {
            sigaddset(&blocked_set, i);
            signal_ignore(i);
        }
   }
# endif /* SYSV_SIGNALS */
#endif /* POSIX_SIGNALS */
 
    return oset;
}

/**/
#endif /* BSD_SIGNALS */

/* Unblock the signals in the given signal *
 * set. Return the old signal set.         */

/**/
mod_export sigset_t
signal_unblock(sigset_t set)
{
    sigset_t oset;

#ifdef POSIX_SIGNALS
    sigprocmask(SIG_UNBLOCK, &set, &oset);
#else
# ifdef BSD_SIGNALS
    sigfillset(&oset);
    oset = sigsetmask(oset);
    sigsetmask(oset & ~set);
# else
#  ifdef SYSV_SIGNALS
    int i;
 
    oset = blocked_set;
    for (i = 1; i <= NSIG; ++i) {
        if (sigismember(&set, i) && sigismember(&blocked_set, i)) {
            sigdelset(&blocked_set, i);
            sigrelse(i);
        }
    }
#  else  /* NO_SIGNAL_BLOCKING */
/* On systems that can't block signals, we are just ignoring them.  So *
 * to unblock signals, we just reenable the signal handler for them.   */
    int i;

    oset = blocked_set;
    for (i = 1; i <= NSIG; ++i) {
        if (sigismember(&set, i) && sigismember(&blocked_set, i)) {
            sigdelset(&blocked_set, i);
            install_handler(i);
        }
   }
#  endif /* SYSV_SIGNALS  */
# endif  /* BSD_SIGNALS   */
#endif   /* POSIX_SIGNALS */
 
    return oset;
}

/* set the process signal mask to *
 * be the given signal mask       */

/**/
mod_export sigset_t
signal_setmask(sigset_t set)
{
    sigset_t oset;
 
#ifdef POSIX_SIGNALS
    sigprocmask(SIG_SETMASK, &set, &oset);
#else
# ifdef BSD_SIGNALS
    oset = sigsetmask(set);
# else
#  ifdef SYSV_SIGNALS
    int i;
 
    oset = blocked_set;
    for (i = 1; i <= NSIG; ++i) {
        if (sigismember(&set, i) && !sigismember(&blocked_set, i)) {
            sigaddset(&blocked_set, i);
            sighold(i);
        } else if (!sigismember(&set, i) && sigismember(&blocked_set, i)) {
            sigdelset(&blocked_set, i);
            sigrelse(i);
        }
    }
#  else  /* NO_SIGNAL_BLOCKING */
    int i;

    oset = blocked_set;
    for (i = 1; i < NSIG; ++i) {
        if (sigismember(&set, i) && !sigismember(&blocked_set, i)) {
            sigaddset(&blocked_set, i);
            signal_ignore(i);
        } else if (!sigismember(&set, i) && sigismember(&blocked_set, i)) {
            sigdelset(&blocked_set, i);
            install_handler(i);
        }
    }
#  endif /* SYSV_SIGNALS  */
# endif  /* BSD_SIGNALS   */
#endif   /* POSIX_SIGNALS */
 
    return oset;
}

#if defined(NO_SIGNAL_BLOCKING)
static int suspend_longjmp = 0;
static signal_jmp_buf suspend_jmp_buf;
#endif

/**/
int
signal_suspend(UNUSED(int sig), int wait_cmd)
{
    int ret;

#if defined(POSIX_SIGNALS) || defined(BSD_SIGNALS)
    sigset_t set;
# if defined(POSIX_SIGNALS) && defined(BROKEN_POSIX_SIGSUSPEND)
    sigset_t oset;
# endif

    sigemptyset(&set);

    /* SIGINT from the terminal driver needs to interrupt "wait"
     * and to cause traps to fire, but otherwise should not be
     * handled by the shell until after any foreground job has
     * a chance to decide whether to exit on that signal.
     */
    if (!(wait_cmd || isset(TRAPSASYNC) ||
	  (sigtrapped[SIGINT] & ~ZSIG_IGNORED)))
	sigaddset(&set, SIGINT);
#endif /* POSIX_SIGNALS || BSD_SIGNALS */

#ifdef POSIX_SIGNALS
# ifdef BROKEN_POSIX_SIGSUSPEND
    sigprocmask(SIG_SETMASK, &set, &oset);
    ret = pause();
    sigprocmask(SIG_SETMASK, &oset, NULL);
# else /* not BROKEN_POSIX_SIGSUSPEND */
    ret = sigsuspend(&set);
# endif /* BROKEN_POSIX_SIGSUSPEND */
#else /* not POSIX_SIGNALS */
# ifdef BSD_SIGNALS
    ret = sigpause(set);
# else
#  ifdef SYSV_SIGNALS
    ret = sigpause(sig);

#  else  /* NO_SIGNAL_BLOCKING */
    /* need to use signal_longjmp to make this race-free *
     * between the child_unblock() and pause()           */
    if (signal_setjmp(suspend_jmp_buf) == 0) {
        suspend_longjmp = 1;   /* we want to signal_longjmp after catching signal */
        child_unblock();       /* do we need to do wait_cmd stuff as well?             */
        ret = pause();
    }
    suspend_longjmp = 0;       /* turn off using signal_longjmp since we are past *
                                * the pause() function.                           */
#  endif /* SYSV_SIGNALS  */
# endif  /* BSD_SIGNALS   */
#endif   /* POSIX_SIGNALS */

    return ret;
}

/* last signal we handled: race prone, or what? */
/**/
int last_signal;

/*
 * Wait for any processes that have changed state.
 *
 * The main use for this is in the SIGCHLD handler.  However,
 * we also use it to pick up status changes of jobs when
 * updating jobs.
 */
/**/
void
wait_for_processes(void)
{
    /* keep WAITING until no more child processes to reap */
    for (;;) {
	/* save the errno, since WAIT may change it */
	int old_errno = errno;
	int status;
	Job jn;
	Process pn;
	pid_t pid;
	pid_t *procsubpid = &cmdoutpid;
	int *procsubval = &cmdoutval;
	int cont = 0;
	struct execstack *es = exstack;

	/*
	 * Reap the child process.
	 * If we want usage information, we need to use wait3.
	 */
#if defined(HAVE_WAIT3) || defined(HAVE_WAITPID)
# ifdef WCONTINUED
# define WAITFLAGS (WNOHANG|WUNTRACED|WCONTINUED)
# else
# define WAITFLAGS (WNOHANG|WUNTRACED)
# endif
#endif
#ifdef HAVE_WAIT3
# ifdef HAVE_GETRUSAGE
	struct rusage ru;

	pid = wait3((void *)&status, WAITFLAGS, &ru);
# else
	pid = wait3((void *)&status, WAITFLAGS, NULL);
# endif
#else
# ifdef HAVE_WAITPID
	pid = waitpid(-1, &status, WAITFLAGS);
# else
	pid = wait(&status);
# endif
#endif

	if (!pid)  /* no more children to reap */
	    break;

	/* check if child returned was from process substitution */
	for (;;) {
	    if (pid == *procsubpid) {
		*procsubpid = 0;
		if (WIFSIGNALED(status))
		    *procsubval = (0200 | WTERMSIG(status));
		else
		    *procsubval = WEXITSTATUS(status);
		use_cmdoutval = 1;
		get_usage();
		cont = 1;
		break;
	    }
	    if (!es)
		break;
	    procsubpid = &es->cmdoutpid;
	    procsubval = &es->cmdoutval;
	    es = es->next;
	}
	if (cont)
	    continue;

	/* check for WAIT error */
	if (pid == -1) {
	    if (errno != ECHILD)
		zerr("wait failed: %e", errno);
	    /* WAIT changed errno, so restore the original */
	    errno = old_errno;
	    break;
	}

	/* This is necessary to be sure queueing_enabled > 0 when
	 * we enter printjob() from update_job(), so that we don't
	 * decrement to zero in should_report_time() and improperly
	 * run other handlers in the middle of processing this one */
	queue_signals();

	/*
	 * Find the process and job containing this pid and
	 * update it.
	 */
	if (findproc(pid, &jn, &pn, 0)) {
	    if (((jn->stat & STAT_BUILTIN) ||
		 (list_pipe &&
		  (thisjob == -1 ||
		   (jobtab[thisjob].stat & STAT_BUILTIN)))) &&
		WIFSTOPPED(status) && WSTOPSIG(status) == SIGTSTP) {
		killjb(jn, SIGCONT);
		zwarn("job can't be suspended");
	    } else {
#if defined(HAVE_WAIT3) && defined(HAVE_GETRUSAGE)
		struct timezone dummy_tz;
		gettimeofday(&pn->endtime, &dummy_tz);
#ifdef WIFCONTINUED
		if (WIFCONTINUED(status))
		    pn->status = SP_RUNNING;
		else
#endif
		pn->status = status;
		pn->ti = ru;
#else
		update_process(pn, status);
#endif
		if (WIFEXITED(status) &&
		    pn->pid == jn->gleader &&
		    killpg(pn->pid, 0) == -1  &&
		    errno == ESRCH) {
		    if (last_attached_pgrp == jn->gleader &&
			!(jn->stat & STAT_NOSTTY)) {
			/*
			 * This PID was in control of the terminal;
			 * reclaim terminal now it has exited.
			 * It's still possible some future forked
			 * process of this job will become group
			 * leader, however.
			 */
			attachtty(mypgrp);
			adjustwinsize(0);
		    }
		    jn->gleader = 0;
		}
	    }
	    update_job(jn);
	} else if (findproc(pid, &jn, &pn, 1)) {
	    pn->status = status;
	    update_job(jn);
	} else {
	    /* If not found, update the shell record of time spent by
	     * children in sub processes anyway:  otherwise, this
	     * will get added on to the next found process that
	     * terminates.
	     */
	    get_usage();
	}
	/*
	 * Accumulate a list of older jobs.  We only do this for
	 * background jobs, which is something in the job table
	 * that's not marked as in the current shell or as shell builtin
	 * and is not equal to the current foreground job.
	 */
	if (jn && !(jn->stat & (STAT_CURSH|STAT_BUILTIN)) &&
	    jn - jobtab != thisjob) {
	    int val = (WIFSIGNALED(status) ?
		   0200 | WTERMSIG(status) :
		   (WIFSTOPPED(status) ?
		    0200 | WEXITSTATUS(status) :
		    WEXITSTATUS(status)));
	    addbgstatus(pid, val);
	}

	unqueue_signals();
    }
}

/* the signal handler */
 
/**/
mod_export void
zhandler(int sig)
{
    sigset_t newmask, oldmask;

#if defined(NO_SIGNAL_BLOCKING)
    int do_jump;
    signal_jmp_buf jump_to;
#endif
 
    last_signal = sig;
    signal_process(sig);
 
    sigfillset(&newmask);
    /* Block all signals temporarily           */
    oldmask = signal_block(newmask);
 
#if defined(NO_SIGNAL_BLOCKING)
    /* do we need to longjmp to signal_suspend */
    do_jump = suspend_longjmp;
    /* In case a SIGCHLD somehow arrives       */
    suspend_longjmp = 0;

    /* Traps can cause nested signal_suspend()  */
    if (sig == SIGCHLD) {
        if (do_jump) {
	    /* Copy suspend_jmp_buf                    */
            jump_to = suspend_jmp_buf;
	}
    }
#endif

    /* Are we queueing signals now?      */
    if (queueing_enabled) {
        int temp_rear = ++queue_rear % MAX_QUEUE_SIZE;

	DPUTS(temp_rear == queue_front, "BUG: signal queue full");
	/* Make sure it's not full (extremely unlikely) */
        if (temp_rear != queue_front) {
	    /* ok, not full, so add to queue   */
            queue_rear = temp_rear;
	    /* save signal caught              */
            signal_queue[queue_rear] = sig;
	    /* save current signal mask        */
            signal_mask_queue[queue_rear] = oldmask;
        }
        signal_reset(sig);
        return;
    }
 
    /* Reset signal mask, signal traps ok now */
    signal_setmask(oldmask);
 
    switch (sig) {
    case SIGCHLD:
	wait_for_processes();
        break;
 
    case SIGPIPE:
	if (!handletrap(SIGPIPE)) {
	    if (!interact)
		_exit(SIGPIPE);
	    else if (!isatty(SHTTY)) {
		stopmsg = 1;
		zexit(SIGPIPE, ZEXIT_SIGNAL);
	    }
	}
	break;

    case SIGHUP:
        if (!handletrap(SIGHUP)) {
            stopmsg = 1;
            zexit(SIGHUP, ZEXIT_SIGNAL);
        }
        break;
 
    case SIGINT:
        if (!handletrap(SIGINT)) {
	    if ((isset(PRIVILEGED) || isset(RESTRICTED)) &&
		isset(INTERACTIVE) && (noerrexit & NOERREXIT_SIGNAL))
		zexit(SIGINT, ZEXIT_SIGNAL);
            errflag |= ERRFLAG_INT;
            if (list_pipe || chline || simple_pline) {
                breaks = loops;
		inerrflush();
		check_cursh_sig(SIGINT);
            }
	    lastval = 128 + SIGINT;
        }
        break;

#ifdef SIGWINCH
    case SIGWINCH:
        adjustwinsize(1);  /* check window size and adjust */
	(void) handletrap(SIGWINCH);
        break;
#endif

    case SIGALRM:
        if (!handletrap(SIGALRM)) {
	    int idle = ttyidlegetfn(NULL);
	    int tmout = getiparam("TMOUT");
	    if (idle >= 0 && idle < tmout)
		alarm(tmout - idle);
	    else {
		/*
		 * We want to exit now.
		 * Cancel all errors, including a user interrupt
		 * which is now redundant.
		 */
		errflag = noerrs = 0;
		zwarn("timeout");
		stopmsg = 1;
		zexit(SIGALRM, ZEXIT_SIGNAL);
	    }
        }
        break;
 
    default:
        (void) handletrap(sig);
        break;
    }   /* end of switch(sig) */
 
    signal_reset(sig);

/* This is used to make signal_suspend() race-free */
#if defined(NO_SIGNAL_BLOCKING)
    if (do_jump)
        signal_longjmp(jump_to, 1);
#endif

} /* handler */

 
/* SIGHUP any jobs left running */
 
/**/
void
killrunjobs(int from_signal)
{
    int i, killed = 0;
 
    if (unset(HUP))
        return;
    for (i = 1; i <= maxjob; i++)
        if ((from_signal || i != thisjob) && (jobtab[i].stat & STAT_LOCKED) &&
            !(jobtab[i].stat & STAT_NOPRINT) &&
            !(jobtab[i].stat & STAT_STOPPED)) {
            if (jobtab[i].gleader != getpid() &&
		killpg(jobtab[i].gleader, SIGHUP) != -1)
                killed++;
        }
    if (killed)
        zwarn("warning: %d jobs SIGHUPed", killed);
}


/* send a signal to a job (simply involves kill if monitoring is on) */
 
/**/
int
killjb(Job jn, int sig)
{
    Process pn;
    int err = 0;

    if (jobbing) {
        if (jn->stat & STAT_SUPERJOB) {
            if (sig == SIGCONT) {
                for (pn = jobtab[jn->other].procs; pn; pn = pn->next)
                    if (killpg(pn->pid, sig) == -1)
			if (kill(pn->pid, sig) == -1 && errno != ESRCH)
			    err = -1;

		/*
		 * Note this does not kill the last process,
		 * which is assumed to be the one controlling the
		 * subjob, i.e. the forked zsh that was originally
		 * list_pipe_pid...
		 */
                for (pn = jn->procs; pn->next; pn = pn->next)
                    if (kill(pn->pid, sig) == -1 && errno != ESRCH)
			err = -1;

		/*
		 * ...we only continue that once the external processes
		 * currently associated with the subjob are finished.
		 */
		if (!jobtab[jn->other].procs && pn)
		    if (kill(pn->pid, sig) == -1 && errno != ESRCH)
			err = -1;

		/*
		 * The following marks both the superjob and subjob
		 * as running, as done elsewhere.
		 *
		 * It's not entirely clear to me what the right way
		 * to flag the status of a still-pausd final process,
		 * as handled above, but we should be cnsistent about
		 * this inside makerunning() rather than doing anything
		 * special here.
		 */
		if (err != -1)
		    makerunning(jn);

		return err;
            }
            if (killpg(jobtab[jn->other].gleader, sig) == -1 && errno != ESRCH)
		err = -1;

	    if (killpg(jn->gleader, sig) == -1 && errno != ESRCH)
		err = -1;

	    return err;
        }
        else {
	    err = killpg(jn->gleader, sig);
	    if (sig == SIGCONT && err != -1)
		makerunning(jn);
	    return err;
	}
    }
    for (pn = jn->procs; pn; pn = pn->next) {
	/*
	 * Do not kill this job's process if it's already dead as its
	 * pid could have been reused by the system.
	 * As the PID doesn't exist don't return an error.
	 */
	if (pn->status == SP_RUNNING || WIFSTOPPED(pn->status)) {
	    /*
	     * kill -0 on a job is pointless. We still call kill() for each process
	     * in case the user cares about it but we ignore its outcome.
	     */
	    if ((err = kill(pn->pid, sig)) == -1 && errno != ESRCH && sig != 0)
		return -1;
	}
    }
    return err;
}

/*
 * List for saving traps.  We don't usually have that many traps
 * at once, so just use a linked list.
 */
struct savetrap {
    int sig, flags, local, posix;
    void *list;
};

static LinkList savetraps;
static int dontsavetrap;

/*
 * Save the current trap by copying it.  This does nothing to
 * the existing value of sigtrapped or siglists.
 */

static void
dosavetrap(int sig, int level)
{
    struct savetrap *st;
    st = (struct savetrap *)zalloc(sizeof(*st));
    st->sig = sig;
    st->local = level;
    st->posix = (sig == SIGEXIT) ? exit_trap_posix : 0;
    if ((st->flags = sigtrapped[sig]) & ZSIG_FUNC) {
	/*
	 * Get the old function: this assumes we haven't added
	 * the new one yet.
	 */
	Shfunc shf, newshf = NULL;
	if ((shf = (Shfunc)gettrapnode(sig, 1))) {
	    /* Copy the node for saving */
	    newshf = (Shfunc) zshcalloc(sizeof(*newshf));
	    newshf->node.nam = ztrdup(shf->node.nam);
	    newshf->node.flags = shf->node.flags;
	    newshf->funcdef = dupeprog(shf->funcdef, 0);
	    if (shf->node.flags & PM_LOADDIR) {
		dircache_set(&newshf->filename, shf->filename);
	    } else {
		newshf->filename = ztrdup(shf->filename);
	    }
	    if (shf->sticky) {
		newshf->sticky = sticky_emulation_dup(shf->sticky, 0);
	    } else
		newshf->sticky = 0;
	    if (shf->node.flags & PM_UNDEFINED)
		newshf->funcdef->shf = newshf;
	}
#ifdef DEBUG
	else dputs("BUG: no function present with function trap flag set.");
#endif
	DPUTS(siglists[sig], "BUG: function signal has eval list, too.");
	st->list = newshf;
    } else if (sigtrapped[sig]) {
	st->list = siglists[sig] ? dupeprog(siglists[sig], 0) : NULL;
    } else {
	DPUTS(siglists[sig], "BUG: siglists not null for untrapped signal");
	st->list = NULL;
    }
    if (!savetraps)
	savetraps = znewlinklist();
    /*
     * Put this at the front of the list
     */
    zinsertlinknode(savetraps, (LinkNode)savetraps, st);
}


/*
 * Set a trap:  note this does not handle manipulation of
 * the function table for TRAPNAL functions.
 *
 * sig is the signal number.
 *
 * l is the list to be eval'd for a trap defined with the "trap"
 * builtin and should be NULL for a function trap.
 *
 * flags includes any additional flags to be or'd into sigtrapped[sig],
 * in particular ZSIG_FUNC; the basic flags will be assigned within
 * settrap.
 */

/**/
mod_export int
settrap(int sig, Eprog l, int flags)
{
    if (sig == -1)
        return 1;
    if (jobbing && (sig == SIGTTOU || sig == SIGTSTP || sig == SIGTTIN)) {
        zerr("can't trap SIG%s in interactive shells", sigs[sig]);
        return 1;
    }

    /*
     * Call unsettrap() unconditionally, to make sure trap is saved
     * if necessary.
     */
    queue_signals();
    unsettrap(sig);

    DPUTS((flags & ZSIG_FUNC) && l,
	  "BUG: trap function has passed eval list, too");
    siglists[sig] = l;
    if (!(flags & ZSIG_FUNC) && empty_eprog(l)) {
	sigtrapped[sig] = ZSIG_IGNORED;
        if (sig && sig <= SIGCOUNT &&
#ifdef SIGWINCH
            sig != SIGWINCH &&
#endif
            sig != SIGCHLD)
            signal_ignore(sig);
    } else {
	nsigtrapped++;
        sigtrapped[sig] = ZSIG_TRAPPED;
        if (sig && sig <= SIGCOUNT &&
#ifdef SIGWINCH
            sig != SIGWINCH &&
#endif
            sig != SIGCHLD)
            install_handler(sig);
    }
    sigtrapped[sig] |= flags;
    /*
     * Note that introducing the locallevel does not affect whether
     * sigtrapped[sig] is zero or not, i.e. a test without a mask
     * works just the same.
     */
    if (sig == SIGEXIT) {
	/* Make POSIX behaviour of EXIT trap sticky */
	exit_trap_posix = isset(POSIXTRAPS);
	/* POSIX exit traps are not local. */
	if (!exit_trap_posix)
	    sigtrapped[sig] |= (locallevel << ZSIG_SHIFT);
    }
    else
	sigtrapped[sig] |= (locallevel << ZSIG_SHIFT);
    unqueue_signals();
    return 0;
}

/**/
void
unsettrap(int sig)
{
    HashNode hn;

    queue_signals();
    hn = removetrap(sig);
    if (hn)
	shfunctab->freenode(hn);
    unqueue_signals();
}

/**/
HashNode
removetrap(int sig)
{
    int trapped;

    if (sig == -1 ||
	(jobbing && (sig == SIGTTOU || sig == SIGTSTP || sig == SIGTTIN)))
	return NULL;

    queue_signals();
    trapped = sigtrapped[sig];
    /*
     * Note that we save the trap here even if there isn't an existing
     * one, to aid in removing this one.  However, if there's
     * already one at the current locallevel we just overwrite it.
     *
     * Note we save EXIT traps based on the *current* setting of
     * POSIXTRAPS --- so if there is POSIX EXIT trap set but
     * we are in native mode it can be saved, replaced by a function
     * trap, and then restored.
     */
    if (!dontsavetrap &&
	(sig == SIGEXIT ? !isset(POSIXTRAPS) : isset(LOCALTRAPS)) &&
	locallevel &&
	(!trapped || locallevel > (sigtrapped[sig] >> ZSIG_SHIFT)))
	dosavetrap(sig, locallevel);

    if (sigtrapped[sig] & ZSIG_TRAPPED)
	nsigtrapped--;
    sigtrapped[sig] = 0;
    if (sig == SIGINT && interact) {
	/* PWS 1995/05/16:  added test for interactive, also noholdintr() *
	 * as subshells ignoring SIGINT have it blocked from delivery     */
        intr();
	noholdintr();
    } else if (sig == SIGHUP)
        install_handler(sig);
    else if (sig == SIGPIPE && interact && !forklevel)
        install_handler(sig);
    else if (sig && sig <= SIGCOUNT &&
#ifdef SIGWINCH
             sig != SIGWINCH &&
#endif
             sig != SIGCHLD)
        signal_default(sig);
    if (sig == SIGEXIT)
	exit_trap_posix = 0;

    /*
     * At this point we free the appropriate structs.  If we don't
     * want that to happen then either the function should already have been
     * removed from shfunctab, or the entry in siglists should have been set
     * to NULL.  This is no longer necessary for saving traps as that
     * copies the structures, so here we are remove the originals.
     * That causes a little inefficiency, but a good deal more reliability.
     */
    if (trapped & ZSIG_FUNC) {
	HashNode node = gettrapnode(sig, 1);

	/*
	 * As in dosavetrap(), don't call removeshfuncnode() because
	 * that calls back into unsettrap();
	 */
	if (node)
	    removehashnode(shfunctab, node->nam);
	unqueue_signals();

	return node;
    } else if (siglists[sig]) {
	freeeprog(siglists[sig]);
	siglists[sig] = NULL;
    }
    unqueue_signals();

    return NULL;
}

/**/
void
starttrapscope(void)
{
    /* No special SIGEXIT behaviour inside another trap. */
    if (intrap)
	return;

    /*
     * SIGEXIT needs to be restored at the current locallevel,
     * so give it the next higher one. dosavetrap() is called
     * automatically where necessary.
     */
    if (sigtrapped[SIGEXIT] && !exit_trap_posix) {
	locallevel++;
	unsettrap(SIGEXIT);
	locallevel--;
    }
}

/*
 * Reset traps after the end of a function: must be called after
 * endparamscope() so that the locallevel has been decremented.
 */

/**/
void
endtrapscope(void)
{
    LinkNode ln;
    struct savetrap *st;
    int exittr = 0;
    void *exitfn = NULL;

    /*
     * Remember the exit trap, but don't run it until
     * after all the other traps have been put back.
     * Don't do this inside another trap.
     */
    if (!intrap &&
	!exit_trap_posix && (exittr = sigtrapped[SIGEXIT])) {
	if (exittr & ZSIG_FUNC) {
	    exitfn = removehashnode(shfunctab, "TRAPEXIT");
	} else {
	    exitfn = siglists[SIGEXIT];
	    siglists[SIGEXIT] = NULL;
	}
	if (sigtrapped[SIGEXIT] & ZSIG_TRAPPED)
	    nsigtrapped--;
	sigtrapped[SIGEXIT] = 0;
    }

    if (savetraps) {
	while ((ln = firstnode(savetraps)) &&
	       (st = (struct savetrap *) ln->dat) &&
	       st->local > locallevel) {
	    int sig = st->sig;

	    remnode(savetraps, ln);

	    if (st->flags && (st->list != NULL)) {
		/* prevent settrap from saving this */
		dontsavetrap++;
		if (st->flags & ZSIG_FUNC)
		    settrap(sig, NULL, ZSIG_FUNC);
		else
			settrap(sig, (Eprog) st->list, 0);
		if (sig == SIGEXIT)
		    exit_trap_posix = st->posix;
		dontsavetrap--;
		/*
		 * counting of nsigtrapped should presumably be handled
		 * in settrap...
		 */
		DPUTS((sigtrapped[sig] ^ st->flags) & ZSIG_TRAPPED,
		      "BUG: settrap didn't restore correct ZSIG_TRAPPED");
		if ((sigtrapped[sig] = st->flags) & ZSIG_FUNC)
		    shfunctab->addnode(shfunctab, ((Shfunc)st->list)->node.nam,
				       (Shfunc) st->list);
	    } else if (sigtrapped[sig]) {
		/*
		 * Don't restore the old state if someone has set a
		 * POSIX-style exit trap --- allow this to propagate.
		 */
		if (sig != SIGEXIT || !exit_trap_posix)
		    unsettrap(sig);
	    }

	    zfree(st, sizeof(*st));
	}
    }

    if (exittr) {
	/*
	 * We already made sure this wasn't set as a POSIX exit trap.
	 * We respect the user's intention when the trap in question
	 * was set.
	 */
	dotrapargs(SIGEXIT, &exittr, exitfn);
	if (exittr & ZSIG_FUNC)
	    shfunctab->freenode((HashNode)exitfn);
	else
	    freeeprog(exitfn);
    }
    DPUTS(!locallevel && savetraps && firstnode(savetraps),
	  "BUG: still saved traps outside all function scope");
}


/*
 * Decide whether a trap needs handling.
 * If so, see if the trap should be run now or queued.
 * Return 1 if the trap has been or will be handled.
 * This only needs to be called in place of dotrap() in the
 * signal handler, since it's only while waiting for children
 * to exit that we queue traps.
 */
/**/
static int
handletrap(int sig)
{
    if (!sigtrapped[sig])
	return 0;

    if (trap_queueing_enabled)
    {
	/* Code borrowed from signal queueing */
	int temp_rear = ++trap_queue_rear % MAX_QUEUE_SIZE;

	DPUTS(temp_rear == trap_queue_front, "BUG: trap queue full");
	/* If queue is not full... */
	if (temp_rear != trap_queue_front) {
	    trap_queue_rear = temp_rear;
	    trap_queue[trap_queue_rear] = sig;
	}
	return 1;
    }

    dotrap(sig);

    if (sig == SIGALRM)
    {
	int tmout;
	/*
	 * Reset the alarm.
	 * It seems slightly more natural to do this when the
	 * trap is run, rather than when it's queued, since
	 * the user doesn't see the latter.
	 */
	if ((tmout = getiparam("TMOUT")))
	    alarm(tmout);
    }

    return 1;
}


/*
 * Queue traps if they shouldn't be run asynchronously, i.e.
 * we're not in the wait builtin and TRAPSASYNC isn't set, when
 * waiting for children to exit.
 *
 * Note that unlike signal queuing this should only be called
 * in single matching pairs and can't be nested.  It is
 * only needed when waiting for a job or process to finish.
 *
 * There is presumably a race setting this up: we shouldn't be running
 * traps between forking a foreground process and this point, either.
 */
/**/
void
queue_traps(int wait_cmd)
{
    if (!isset(TRAPSASYNC) && !wait_cmd) {
	/*
	 * Traps need to be handled synchronously, so
	 * enable queueing.
	 */
	trap_queueing_enabled = 1;
    }
}


/*
 * Disable trap queuing and run the traps.
 */
/**/
void
unqueue_traps(void)
{
    trap_queueing_enabled = 0;
    while (trap_queue_front != trap_queue_rear) {
	trap_queue_front = (trap_queue_front + 1) % MAX_QUEUE_SIZE;
	(void) handletrap(trap_queue[trap_queue_front]);
    }
}


/* Execute a trap function for a given signal, possibly
 * with non-standard sigtrapped & siglists values
 */

/* Are we already executing a trap? */
/**/
volatile int intrap;

/* Is the current trap a function? */

/**/
volatile int trapisfunc;

/*
 * If the current trap is not a function, at what function depth
 * did the trap get called?
 */
/**/
volatile int traplocallevel;

/*
 * sig is the signal number.
 * *sigtr is the value to be taken as the field in sigtrapped (since
 *   that may have changed by this point if we are exiting).
 * sigfn is an Eprog with a non-function eval list, or a Shfunc
 *   with a function trap.  It may be NULL with an ignored signal.
 */

/**/
static void
dotrapargs(int sig, int *sigtr, void *sigfn)
{
    LinkList args;
    char *name, num[4];
    int obreaks = breaks;
    int oretflag = retflag;
    int olastval = lastval;
    int isfunc;
    int traperr, new_trap_state, new_trap_return;

    /* if signal is being ignored or the trap function		      *
     * is NULL, then return					      *
     *								      *
     * Also return if errflag is set.  In fact, the code in the       *
     * function will test for this, but this way we keep status flags *
     * intact without working too hard.  Special cases (e.g. calling  *
     * a trap for SIGINT after the error flag was set) are handled    *
     * by the calling code.  (PWS 1995/06/08).			      *
     *                                                                *
     * This test is now replicated in dotrap().                       */
    if ((*sigtr & ZSIG_IGNORED) || !sigfn || errflag)
        return;

    /*
     * Never execute special (synchronous) traps inside other traps.
     * This can cause unexpected code execution when more than one
     * of these is set.
     *
     * The down side is that it's harder to debug traps.  I don't think
     * that's a big issue.
     */
    if (intrap) {
	switch (sig) {
	case SIGEXIT:
	case SIGDEBUG:
	case SIGZERR:
	    return;
	}
    }

    queue_signals();	/* Any time we manage memory or global state */

    intrap++;
    *sigtr |= ZSIG_IGNORED;

    zcontext_save();
    /* execsave will save the old trap_return and trap_state */
    execsave();
    breaks = retflag = 0;
    traplocallevel = locallevel;
    runhookdef(BEFORETRAPHOOK, NULL);
    if (*sigtr & ZSIG_FUNC) {
	int osc = sfcontext, old_incompfunc = incompfunc;
	HashNode hn = gettrapnode(sig, 0);

	args = znewlinklist();
	/*
	 * In case of multiple names, try to get
	 * a hint of the name in use from the function table.
	 * In special cases, e.g. EXIT traps, the function
	 * has already been removed.  Then it's OK to
	 * use the standard name.
	 */
	if (hn) {
	    name = ztrdup(hn->nam);
	} else {
	    name = (char *) zalloc(5 + strlen(sigs[sig]));
	    sprintf(name, "TRAP%s", sigs[sig]);
	}
	zaddlinknode(args, name);
	sprintf(num, "%d", sig);
	zaddlinknode(args, num);

	trap_return = -1;	/* incremented by doshfunc */
	trap_state = TRAP_STATE_PRIMED;
	trapisfunc = isfunc = 1;

	sfcontext = SFC_SIGNAL;
	incompfunc = 0;
	doshfunc((Shfunc)sigfn, args, 1);	/* manages signal queueing */
	sfcontext = osc;
	incompfunc= old_incompfunc;
	freelinklist(args, (FreeFunc) NULL);
	zsfree(name);
    } else {
	trap_return = -2;	/* not incremented, used at current level */
	trap_state = TRAP_STATE_PRIMED;
	trapisfunc = isfunc = 0;

	execode((Eprog)sigfn, 1, 0, "trap");	/* manages signal queueing */
    }
    runhookdef(AFTERTRAPHOOK, NULL);

    traperr = errflag;

    /* Grab values before they are restored */
    new_trap_state = trap_state;
    new_trap_return = trap_return;

    execrestore();
    zcontext_restore();

    if (new_trap_state == TRAP_STATE_FORCE_RETURN &&
	/* zero return from function isn't special */
	!(isfunc && new_trap_return == 0)) {
	if (isfunc) {
	    breaks = loops;
	    /*
	     * For SIGINT we behave the same as the default behaviour
	     * i.e. we set the error bit indicating an interrupt.
	     * We do this with SIGQUIT, too, even though we don't
	     * handle SIGQUIT by default.  That's to try to make
	     * it behave a bit more like its normal behaviour when
	     * the trap handler has told us that's what it wants.
	     */
	    if (sig == SIGINT || sig == SIGQUIT)
		errflag |= ERRFLAG_INT;
	    else
		errflag |= ERRFLAG_ERROR;
	}
	lastval = new_trap_return;
	/* return triggered */
	retflag = 1;
    } else {
	if (traperr && !EMULATION(EMULATE_SH))
	    lastval = 1;
	else {
	    /*
	     * With no explicit forced return, we keep the
	     * lastval from before the trap ran.
	     */
	    lastval = olastval;
	}
	if (try_tryflag) {
	    if (traperr)
		errflag |= ERRFLAG_ERROR;
	    else
		errflag &= ~ERRFLAG_ERROR;
	}
	breaks += obreaks;
	/* return not triggered: restore old flag */
	retflag = oretflag;
	if (breaks > loops)
	    breaks = loops;
    }

    /*
     * If zle was running while the trap was executed, see if we
     * need to restore the display.
     */
    if (zleactive && resetneeded)
	zleentry(ZLE_CMD_REFRESH);

    if (*sigtr != ZSIG_IGNORED)
	*sigtr &= ~ZSIG_IGNORED;
    intrap--;

    unqueue_signals();
}

/* Standard call to execute a trap for a given signal. */

/**/
void
dotrap(int sig)
{
    void *funcprog;
    int q = queue_signal_level();

    if (sigtrapped[sig] & ZSIG_FUNC) {
	HashNode hn = gettrapnode(sig, 0);
	if (hn)
	    funcprog = hn;
	else {
#ifdef DEBUG
	    dputs("BUG: running function trap which has escaped.");
#endif
	    funcprog = NULL;
	}
    } else
	funcprog = siglists[sig];

    /*
     * Copied from dotrapargs().
     * (In fact, the gain from duplicating this appears to be virtually
     * zero.  Not sure why it's here.)
     */
    if ((sigtrapped[sig] & ZSIG_IGNORED) || !funcprog || errflag)
	return;

    dont_queue_signals();

    if (sig == SIGEXIT)
	++in_exit_trap;

    dotrapargs(sig, sigtrapped+sig, funcprog);

    if (sig == SIGEXIT)
	--in_exit_trap;

    restore_queue_signals(q);
}
