/*
 * jobs.c - job control
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
#include "jobs.pro"

/*
 * Job control in zsh
 * ==================
 *
 * A 'job' represents a pipeline; see the section JOBS in zshmisc(1)) for an
 * introduction.  The 'struct job's are allocated in the array 'jobtab' which
 * has 'jobtabsize' elements.  The job whose processes we are currently
 * preparing to execute is identified by the global variable 'thisjob'.
 *
 * A 'superjob' is a job that represents a complex shell construct that has been
 * backgrounded.  For example, if one runs '() { vi; echo }', a job is created
 * for the pipeline 'vi'.  If one then backgrounds vi (with ^Z / SIGTSTP), 
 * the shell forks; the parent shell returns to the interactive prompt and
 * the child shell becomes a new job in the parent shell.  The job representing
 * the child shell to the parent shell is a superjob (STAT_SUPERJOB); the 'vi'
 * job is marked as a subjob (STAT_SUBJOB) in the parent shell.  When the child
 * shell is resumed (with fg / SIGCONT), it forwards the signal to vi and,
 * after vi exits, continues executing the remainder of the function.
 * (See workers/43565.)
 */

/* the process group of the shell at startup (equal to mypgprp, except
   when we started without being process group leader */

/**/
mod_export pid_t origpgrp;

/* the process group of the shell */

/**/
mod_export pid_t mypgrp;

/* the last process group to attach to the terminal */

/**/
pid_t last_attached_pgrp;
 
/* the job we are working on, or -1 if none */
 
/**/
mod_export int thisjob;

/* the current job (%+) */
 
/**/
mod_export int curjob;
 
/* the previous job (%-) */
 
/**/
mod_export int prevjob;
 
/* the job table */
 
/**/
mod_export struct job *jobtab;

/* Size of the job table. */

/**/
mod_export int jobtabsize;

/* The highest numbered job in the jobtable */

/**/
mod_export int maxjob;

/* If we have entered a subshell, the original shell's job table. */
/**/
mod_export struct job *oldjobtab;

/* The size of that. */
/**/
mod_export int oldmaxjob;

/* shell timings */
 
/**/
#ifdef HAVE_GETRUSAGE
/**/
static struct rusage child_usage;
/**/
#else
/**/
static struct tms shtms;
/**/
#endif
 
/* 1 if ttyctl -f has been executed */
 
/**/
mod_export int ttyfrozen;

/* Previous values of errflag and breaks if the signal handler had to
 * change them. And a flag saying if it did that. */

/**/
int prev_errflag, prev_breaks, errbrk_saved;

/**/
int numpipestats, pipestats[MAX_PIPESTATS];

/* Diff two timevals for elapsed-time computations */

/**/
static struct timeval *
dtime(struct timeval *dt, struct timeval *t1, struct timeval *t2)
{
    dt->tv_sec = t2->tv_sec - t1->tv_sec;
    dt->tv_usec = t2->tv_usec - t1->tv_usec;
    if (dt->tv_usec < 0) {
	dt->tv_usec += 1000000.0;
	dt->tv_sec -= 1.0;
    }
    return dt;
}

/* change job table entry from stopped to running */

/**/
void
makerunning(Job jn)
{
    Process pn;

    jn->stat &= ~STAT_STOPPED;
    for (pn = jn->procs; pn; pn = pn->next) {
#if 0
	if (WIFSTOPPED(pn->status) && 
	    (!(jn->stat & STAT_SUPERJOB) || pn->next))
	    pn->status = SP_RUNNING;
#endif
        if (WIFSTOPPED(pn->status))
	    pn->status = SP_RUNNING;
    }

    if (jn->stat & STAT_SUPERJOB)
	makerunning(jobtab + jn->other);
}

/* Find process and job associated with pid.         *
 * Return 1 if search was successful, else return 0. */

/**/
int
findproc(pid_t pid, Job *jptr, Process *pptr, int aux)
{
    Process pn;
    int i;

    *jptr = NULL;
    *pptr = NULL;
    for (i = 1; i <= maxjob; i++)
    {
	/*
	 * We are only interested in jobs with processes still
	 * marked as live.  Careful in case there's an identical
	 * process number in a job we haven't quite got around
	 * to deleting.
	 */
	if (jobtab[i].stat & STAT_DONE)
	    continue;

	for (pn = aux ? jobtab[i].auxprocs : jobtab[i].procs;
	     pn; pn = pn->next)
	{
	    /*
	     * Make sure we match a process that's still running.
	     *
	     * When a job contains two pids, one terminated pid and one
	     * running pid, then the condition (jobtab[i].stat &
	     * STAT_DONE) will not stop these pids from being candidates
	     * for the findproc result (which is supposed to be a
	     * RUNNING pid), and if the terminated pid is an identical
	     * process number for the pid identifying the running
	     * process we are trying to find (after pid number
	     * wrapping), then we need to avoid returning the terminated
	     * pid, otherwise the shell would block and wait forever for
	     * the termination of the process which pid we were supposed
	     * to return in a different job.
	     */
	    if (pn->pid == pid) {
		*pptr = pn;
		*jptr = jobtab + i;
		if (pn->status == SP_RUNNING) 
		    return 1;
	    }
	}
    }

    return (*pptr && *jptr);
}

/* Does the given job number have any processes? */

/**/
int
hasprocs(int job)
{
    Job jn;

    if (job < 0) {
	DPUTS(1, "job number invalid in hasprocs");
	return 0;
    }
    jn = jobtab + job;

    return jn->procs || jn->auxprocs;
}

/* Find the super-job of a sub-job. */

/**/
static int
super_job(int sub)
{
    int i;

    for (i = 1; i <= maxjob; i++)
	if ((jobtab[i].stat & STAT_SUPERJOB) &&
	    jobtab[i].other == sub &&
	    jobtab[i].gleader)
	    return i;
    return 0;
}

/**/
static int
handle_sub(int job, int fg)
{
    /* job: superjob; sj: subjob. */
    Job jn = jobtab + job, sj = jobtab + jn->other;

    if ((sj->stat & STAT_DONE) || (!sj->procs && !sj->auxprocs)) {
	struct process *p;

	for (p = sj->procs; p; p = p->next) {
	    if (WIFSIGNALED(p->status)) {
		if (jn->gleader != mypgrp && jn->procs->next)
		    killpg(jn->gleader, WTERMSIG(p->status));
		else
		    kill(jn->procs->pid, WTERMSIG(p->status));
		kill(sj->other, SIGCONT);
		kill(sj->other, WTERMSIG(p->status));
		break;
	    }
	}
	if (!p) {
	    int cp;

	    jn->stat &= ~STAT_SUPERJOB;
	    jn->stat |= STAT_WASSUPER;

	    if ((cp = ((WIFEXITED(jn->procs->status) ||
			WIFSIGNALED(jn->procs->status)) &&
		       (killpg(jn->gleader, 0) == -1 &&
			errno == ESRCH)))) {
		Process p;
		for (p = jn->procs; p->next; p = p->next);
		jn->gleader = p->pid;
	    }
	    /* This deleted the job too early if the parent
	       shell waited for a command in a list that will
	       be executed by the sub-shell (e.g.: if we have
	       `ls|if true;then sleep 20;cat;fi' and ^Z the
	       sleep, the rest will be executed by a sub-shell,
	       but the parent shell gets notified for the
	       sleep.
	       deletejob(sj, 0); */
	    /* If this super-job contains only the sub-shell,
	       we have to attach the tty to its process group
	       now. */
	    if ((fg || thisjob == job) &&
		(!jn->procs->next || cp || jn->procs->pid != jn->gleader))
		attachtty(jn->gleader);
	    kill(sj->other, SIGCONT);
	    if (jn->stat & STAT_DISOWN)
	    {
		deletejob(jn, 1);
	    }
	}
	curjob = jn - jobtab;
    } else if (sj->stat & STAT_STOPPED) {
	struct process *p;

	jn->stat |= STAT_STOPPED;
	for (p = jn->procs; p; p = p->next)
	    if (p->status == SP_RUNNING ||
		(!WIFEXITED(p->status) && !WIFSIGNALED(p->status)))
		p->status = sj->procs->status;
	curjob = jn - jobtab;
	printjob(jn, !!isset(LONGLISTJOBS), 1);
	return 1;
    }
    return 0;
}


/* Get the latest usage information */

/**/
void 
get_usage(void)
{
#ifdef HAVE_GETRUSAGE
    getrusage(RUSAGE_CHILDREN, &child_usage);
#else
    times(&shtms);
#endif
}


#if !defined HAVE_WAIT3 || !defined HAVE_GETRUSAGE
/* Update status of process that we have just WAIT'ed for */

/**/
void
update_process(Process pn, int status)
{
    struct timezone dummy_tz;
#ifdef HAVE_GETRUSAGE
    struct timeval childs = child_usage.ru_stime;
    struct timeval childu = child_usage.ru_utime;
#else
    long childs = shtms.tms_cstime;
    long childu = shtms.tms_cutime;
#endif

    /* get time-accounting info          */
    get_usage();
    gettimeofday(&pn->endtime, &dummy_tz);  /* record time process exited        */

    pn->status = status;                    /* save the status returned by WAIT  */
#ifdef HAVE_GETRUSAGE
    dtime(&pn->ti.ru_stime, &childs, &child_usage.ru_stime);
    dtime(&pn->ti.ru_utime, &childu, &child_usage.ru_utime);
#else
    pn->ti.st  = shtms.tms_cstime - childs; /* compute process system space time */
    pn->ti.ut  = shtms.tms_cutime - childu; /* compute process user space time   */
#endif
}
#endif

/*
 * Called when the current shell is behaving as if it received
 * a interactively generated signal (sig).
 * 
 * As we got the signal or are pretending we did, we need to pretend
 * anything attached to a CURSH process got it, too.
 */
/**/
void
check_cursh_sig(int sig)
{
    int i, j;

    if (!errflag)
	return;
    for (i = 1; i <= maxjob; i++) {
	if ((jobtab[i].stat & (STAT_CURSH|STAT_DONE)) ==
	    STAT_CURSH) {
	    for (j = 0; j < 2; j++) {
		Process pn = j ? jobtab[i].auxprocs : jobtab[i].procs;
		for (; pn; pn = pn->next) {
		    if (pn->status == SP_RUNNING) {
			kill(pn->pid, sig);
		    }
		}
	    }
	}
    }
}

/**/
void
storepipestats(Job jn, int inforeground, int fixlastval)
{
    int i, pipefail = 0, jpipestats[MAX_PIPESTATS];
    Process p;

    for (p = jn->procs, i = 0; p && i < MAX_PIPESTATS; p = p->next, i++) {
	jpipestats[i] = (WIFSIGNALED(p->status) ?
			 0200 | WTERMSIG(p->status) :
			 (WIFSTOPPED(p->status) ?
			  0200 | WEXITSTATUS(p->status) :
			  WEXITSTATUS(p->status)));
	if (jpipestats[i])
	    pipefail = jpipestats[i];
    }
    if (inforeground) {
	memcpy(pipestats, jpipestats, sizeof(int)*i);
	if ((jn->stat & STAT_CURSH) && i < MAX_PIPESTATS)
	    pipestats[i++] = lastval;
	numpipestats = i;
    }

    if (fixlastval) {
      if (jn->stat & STAT_CURSH) {
	if (!lastval && isset(PIPEFAIL))
	  lastval = pipefail;
      } else if (isset(PIPEFAIL))
	lastval = pipefail;
    }
}

/* Update status of job, possibly printing it */

/**/
void
update_job(Job jn)
{
    Process pn;
    int job;
    int val = 0, status = 0;
    int somestopped = 0, inforeground = 0, signalled = 0;

    for (pn = jn->auxprocs; pn; pn = pn->next) {
#ifdef WIFCONTINUED
	if (WIFCONTINUED(pn->status))
	    pn->status = SP_RUNNING;
#endif
	if (pn->status == SP_RUNNING)
	    return;
    }

    for (pn = jn->procs; pn; pn = pn->next) {
#ifdef WIFCONTINUED
	if (WIFCONTINUED(pn->status)) {
	    jn->stat &= ~STAT_STOPPED;
	    pn->status = SP_RUNNING;
	}
#endif
	if (pn->status == SP_RUNNING)      /* some processes in this job are running       */
	    return;                        /* so no need to update job table entry         */
	if (WIFSTOPPED(pn->status))        /* some processes are stopped                   */
	    somestopped = 1;               /* so job is not done, but entry needs updating */
	if (!pn->next) {
	    /* last job in pipeline determines exit status  */
	    val = (WIFSIGNALED(pn->status) ?
		   0200 | WTERMSIG(pn->status) :
		   (WIFSTOPPED(pn->status) ?
		    0200 | WEXITSTATUS(pn->status) :
		    WEXITSTATUS(pn->status)));
	    signalled = WIFSIGNALED(pn->status);
	}
	if (pn->pid == jn->gleader)        /* if this process is process group leader      */
	    status = pn->status;
    }

    job = jn - jobtab;   /* compute job number */

    if (somestopped) {
	if (jn->stty_in_env && !jn->ty) {
	    jn->ty = (struct ttyinfo *) zalloc(sizeof(struct ttyinfo));
	    gettyinfo(jn->ty);
	}
	if (jn->stat & STAT_SUBJOB) {
	    /* If we have `cat foo|while read a; grep $a bar;done'
	     * and have hit ^Z, the sub-job is stopped, but the
	     * super-job may still be running, waiting to be stopped
	     * or to exit. So we have to send it a SIGTSTP. */
	    int i;

	    jn->stat |= STAT_CHANGED | STAT_STOPPED;
	    if ((i = super_job(job))) {
		Job sjn = &jobtab[i];
		killpg(sjn->gleader, SIGTSTP);
		/*
		 * Job may already be stopped if it consists of only the
		 * forked shell waiting for the subjob -- so mark as
		 * stopped immediately.  This ensures we send it (and,
		 * crucially, the subjob, as the visible job used with
		 * fg/bg is the superjob) a SIGCONT if we need it.
		 */
		sjn->stat |= STAT_CHANGED | STAT_STOPPED;
		if (isset(NOTIFY) && (sjn->stat & STAT_LOCKED) &&
		    !(sjn->stat & STAT_NOPRINT)) {
		    /*
		     * Print the subjob state, which we don't usually
		     * do, so the user knows something has stopped.
		     * So as not to be confusing, we actually output
		     * the user-visible superjob.
		     */
		    if (printjob(sjn, !!isset(LONGLISTJOBS), 0) &&
			zleactive)
			zleentry(ZLE_CMD_REFRESH);
		}
	    }
	    return;
	}
	if (jn->stat & STAT_STOPPED)
	    return;
    }
    {                   /* job is done or stopped, remember return value */
	lastval2 = val;
	/* If last process was run in the current shell, keep old status
	 * and let it handle its own traps, but always allow the test
	 * for the pgrp.
	 */
	if (jn->stat & STAT_CURSH)
	    inforeground = 1;
	else if (job == thisjob) {
	    lastval = val;
	    inforeground = 2;
	}
    }

    if (shout && shout != stderr && !ttyfrozen && !jn->stty_in_env &&
	!zleactive && job == thisjob && !somestopped &&
	!(jn->stat & STAT_NOSTTY)) 
	gettyinfo(&shttyinfo);

    if (isset(MONITOR)) {
	pid_t pgrp = gettygrp();           /* get process group of tty      */

	/* is this job in the foreground of an interactive shell? */
	if (mypgrp != pgrp && inforeground &&
	    (jn->gleader == pgrp ||
	     (pgrp > 1 &&
	      (kill(-pgrp, 0) == -1 && errno == ESRCH)))) {
	    if (list_pipe) {
		if (somestopped || (pgrp > 1 &&
				    kill(-pgrp, 0) == -1 &&
				    errno == ESRCH)) {
		    attachtty(mypgrp);
		    /* check window size and adjust if necessary */
		    adjustwinsize(0);
		} else {
		    /*
		     * Oh, dear, we're right in the middle of some confusion
		     * of shell jobs on the righthand side of a pipeline, so
		     * it's death to call attachtty() just yet.  Mark the
		     * fact in the job, so that the attachtty() will be called
		     * when the job is finally deleted.
		     */
		    jn->stat |= STAT_ATTACH;
		}
		/* If we have `foo|while true; (( x++ )); done', and hit
		 * ^C, we have to stop the loop, too. */
		if (signalled && inforeground == 1 &&
		    ((val & ~0200) == SIGINT || (val & ~0200) == SIGQUIT)) {
		    if (!errbrk_saved) {
			errbrk_saved = 1;
			prev_breaks = breaks;
			prev_errflag = errflag;
		    }
		    breaks = loops;
		    errflag |= ERRFLAG_INT;
		    inerrflush();
		}
	    } else {
		attachtty(mypgrp);
		/* check window size and adjust if necessary */
		adjustwinsize(0);
	    }
	}
    } else if (list_pipe && signalled && inforeground == 1 &&
	       ((val & ~0200) == SIGINT || (val & ~0200) == SIGQUIT)) {
	if (!errbrk_saved) {
	    errbrk_saved = 1;
	    prev_breaks = breaks;
	    prev_errflag = errflag;
	}
	breaks = loops;
	errflag |= ERRFLAG_INT;
	inerrflush();
    }
    if (somestopped && jn->stat & STAT_SUPERJOB)
	return;
    jn->stat |= (somestopped) ? STAT_CHANGED | STAT_STOPPED :
	STAT_CHANGED | STAT_DONE;
    if (jn->stat & (STAT_DONE|STAT_STOPPED)) {
	/* This may be redundant with printjob() but note that inforeground
	 * is true here for STAT_CURSH jobs even when job != thisjob, most
	 * likely because thisjob = -1 from exec.c:execsimple() trickery.
	 * However, if we reset lastval here we break it for printjob().
	 */
	storepipestats(jn, inforeground, 0);
    }
    if (!inforeground &&
	(jn->stat & (STAT_SUBJOB | STAT_DONE)) == (STAT_SUBJOB | STAT_DONE)) {
	int su;

	if ((su = super_job(jn - jobtab)))
	    handle_sub(su, 0);
    }
    if ((jn->stat & (STAT_DONE | STAT_STOPPED)) == STAT_STOPPED) {
	prevjob = curjob;
	curjob = job;
    }
    if ((isset(NOTIFY) || job == thisjob) && (jn->stat & STAT_LOCKED)) {
	if (printjob(jn, !!isset(LONGLISTJOBS), 0) &&
	    zleactive)
	    zleentry(ZLE_CMD_REFRESH);
    }
    if (sigtrapped[SIGCHLD] && job != thisjob)
	dotrap(SIGCHLD);

    /* When MONITOR is set, the foreground process runs in a different *
     * process group from the shell, so the shell will not receive     *
     * terminal signals, therefore we pretend that the shell got       *
     * the signal too.                                                 */
    if (inforeground == 2 && isset(MONITOR) && WIFSIGNALED(status)) {
	int sig = WTERMSIG(status);

	if (sig == SIGINT || sig == SIGQUIT) {
	    if (sigtrapped[sig]) {
		dotrap(sig);
		/* We keep the errflag as set or not by dotrap.
		 * This is to fulfil the promise to carry on
		 * with the jobs if trap returns zero.
		 * Setting breaks = loops ensures a consistent return
		 * status if inside a loop.  Maybe the code in loops
		 * should be changed.
		 */
		if (errflag)
		    breaks = loops;
	    } else {
		breaks = loops;
		errflag |= ERRFLAG_INT;
	    }
	    check_cursh_sig(sig);
	}
    }
}

/* set the previous job to something reasonable */

/**/
static void
setprevjob(void)
{
    int i;

    for (i = maxjob; i; i--)
	if ((jobtab[i].stat & STAT_INUSE) && (jobtab[i].stat & STAT_STOPPED) &&
	    !(jobtab[i].stat & STAT_SUBJOB) && i != curjob && i != thisjob) {
	    prevjob = i;
	    return;
	}

    for (i = maxjob; i; i--)
	if ((jobtab[i].stat & STAT_INUSE) && !(jobtab[i].stat & STAT_SUBJOB) &&
	    i != curjob && i != thisjob) {
	    prevjob = i;
	    return;
	}

    prevjob = -1;
}

/**/
long
get_clktck(void)
{
    static long clktck;

#ifdef _SC_CLK_TCK
    if (!clktck)
	/* fetch clock ticks per second from *
	 * sysconf only the first time       */
	clktck = sysconf(_SC_CLK_TCK);
#else
# ifdef __NeXT__
    /* NeXTStep 3.3 defines CLK_TCK wrongly */
    clktck = 60;
# else
#  ifdef CLK_TCK
    clktck = CLK_TCK;
#  else
#   ifdef HZ
     clktck = HZ;
#   else
     clktck = 60;
#   endif
#  endif
# endif
#endif

     return clktck;
}

/**/
static void
printhhmmss(double secs)
{
    int mins = (int) secs / 60;
    int hours = mins / 60;

    secs -= 60 * mins;
    mins -= 60 * hours;
    if (hours)
	fprintf(stderr, "%d:%02d:%05.2f", hours, mins, secs);
    else if (mins)
	fprintf(stderr,      "%d:%05.2f",        mins, secs);
    else
	fprintf(stderr,           "%.3f",              secs);
}

static void
printtime(struct timeval *real, child_times_t *ti, char *desc)
{
    char *s;
    double elapsed_time, user_time, system_time;
#ifdef HAVE_GETRUSAGE
    double total_time;
#endif
    int percent, desclen;

    if (!desc)
    {
	desc = "";
	desclen = 0;
    }
    else
    {
	desc = dupstring(desc);
	unmetafy(desc, &desclen);
    }

    /* go ahead and compute these, since almost every TIMEFMT will have them */
    elapsed_time = real->tv_sec + real->tv_usec / 1000000.0;

#ifdef HAVE_GETRUSAGE
    user_time = ti->ru_utime.tv_sec + ti->ru_utime.tv_usec / 1000000.0;
    system_time = ti->ru_stime.tv_sec + ti->ru_stime.tv_usec / 1000000.0;
    total_time = user_time + system_time;
    percent = 100.0 * total_time
	/ (real->tv_sec + real->tv_usec / 1000000.0);
#else
    {
	long clktck = get_clktck();
	user_time    = ti->ut / (double) clktck;
	system_time  = ti->st / (double) clktck;
	percent      =  100.0 * (ti->ut + ti->st)
	    / (clktck * real->tv_sec + clktck * real->tv_usec / 1000000.0);
    }
#endif

    queue_signals();
    if (!(s = getsparam("TIMEFMT")))
	s = DEFAULT_TIMEFMT;
    else
	s = unmetafy(s, NULL);

    for (; *s; s++)
	if (*s == '%')
	    switch (*++s) {
	    case 'E':
		fprintf(stderr, "%4.2fs", elapsed_time);
		break;
	    case 'U':
		fprintf(stderr, "%4.2fs", user_time);
		break;
	    case 'S':
		fprintf(stderr, "%4.2fs", system_time);
		break;
	    case 'm':
		switch (*++s) {
		case 'E':
		    fprintf(stderr, "%0.fms", elapsed_time * 1000.0);
		    break;
		case 'U':
		    fprintf(stderr, "%0.fms", user_time * 1000.0);
		    break;
		case 'S':
		    fprintf(stderr, "%0.fms", system_time * 1000.0);
		    break;
		default:
		    fprintf(stderr, "%%m");
		    s--;
		    break;
		}
		break;
	    case 'u':
		switch (*++s) {
		case 'E':
		    fprintf(stderr, "%0.fus", elapsed_time * 1000000.0);
		    break;
		case 'U':
		    fprintf(stderr, "%0.fus", user_time * 1000000.0);
		    break;
		case 'S':
		    fprintf(stderr, "%0.fus", system_time * 1000000.0);
		    break;
		default:
		    fprintf(stderr, "%%u");
		    s--;
		    break;
		}
		break;
	    case '*':
		switch (*++s) {
		case 'E':
		    printhhmmss(elapsed_time);
		    break;
		case 'U':
		    printhhmmss(user_time);
		    break;
		case 'S':
		    printhhmmss(system_time);
		    break;
		default:
		    fprintf(stderr, "%%*");
		    s--;
		    break;
		}
		break;
	    case 'P':
		fprintf(stderr, "%d%%", percent);
		break;
#ifdef HAVE_STRUCT_RUSAGE_RU_NSWAP
	    case 'W':
		fprintf(stderr, "%ld", ti->ru_nswap);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_IXRSS
	    case 'X':
		fprintf(stderr, "%ld", 
			total_time ?
			(long)(ti->ru_ixrss / total_time) :
			(long)0);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_IDRSS
	    case 'D':
		fprintf(stderr, "%ld",
			total_time ? 
			(long) ((ti->ru_idrss
#ifdef HAVE_STRUCT_RUSAGE_RU_ISRSS
				 + ti->ru_isrss
#endif
				    ) / total_time) :
			(long)0);
		break;
#endif
#if defined(HAVE_STRUCT_RUSAGE_RU_IDRSS) || \
    defined(HAVE_STRUCT_RUSAGE_RU_ISRSS) || \
    defined(HAVE_STRUCT_RUSAGE_RU_IXRSS)
	    case 'K':
		/* treat as D if X not available */
		fprintf(stderr, "%ld",
			total_time ?
			(long) ((
#ifdef HAVE_STRUCT_RUSAGE_RU_IXRSS
				    ti->ru_ixrss
#else
				    0
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_IDRSS
				    + ti->ru_idrss
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_ISRSS
				    + ti->ru_isrss
#endif
				    ) / total_time) :
			(long)0);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_MAXRSS
	    case 'M':
		fprintf(stderr, "%ld", ti->ru_maxrss / 1024);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_MAJFLT
	    case 'F':
		fprintf(stderr, "%ld", ti->ru_majflt);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_MINFLT
	    case 'R':
		fprintf(stderr, "%ld", ti->ru_minflt);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_INBLOCK
	    case 'I':
		fprintf(stderr, "%ld", ti->ru_inblock);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_OUBLOCK
	    case 'O':
		fprintf(stderr, "%ld", ti->ru_oublock);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_MSGRCV
	    case 'r':
		fprintf(stderr, "%ld", ti->ru_msgrcv);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_MSGSND
	    case 's':
		fprintf(stderr, "%ld", ti->ru_msgsnd);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_NSIGNALS
	    case 'k':
		fprintf(stderr, "%ld", ti->ru_nsignals);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_NVCSW
	    case 'w':
		fprintf(stderr, "%ld", ti->ru_nvcsw);
		break;
#endif
#ifdef HAVE_STRUCT_RUSAGE_RU_NIVCSW
	    case 'c':
		fprintf(stderr, "%ld", ti->ru_nivcsw);
		break;
#endif
	    case 'J':
		fwrite(desc, sizeof(char), desclen, stderr);
		break;
	    case '%':
		putc('%', stderr);
		break;
	    case '\0':
		s--;
		break;
	    default:
		fprintf(stderr, "%%%c", *s);
		break;
	} else
	    putc(*s, stderr);
    unqueue_signals();
    putc('\n', stderr);
    fflush(stderr);
}

/**/
static void
dumptime(Job jn)
{
    Process pn;
    struct timeval dtimeval;

    if (!jn->procs)
	return;
    for (pn = jn->procs; pn; pn = pn->next)
	printtime(dtime(&dtimeval, &pn->bgtime, &pn->endtime), &pn->ti,
		  pn->text);
}

/* Check whether shell should report the amount of time consumed   *
 * by job.  This will be the case if we have preceded the command  *
 * with the keyword time, or if REPORTTIME is non-negative and the *
 * amount of time consumed by the job is greater than REPORTTIME   */

/**/
static int
should_report_time(Job j)
{
    struct value vbuf;
    Value v;
    char *s = "REPORTTIME";
    int save_errflag = errflag;
    zlong reporttime = -1;
#ifdef HAVE_GETRUSAGE
    char *sm = "REPORTMEMORY";
    zlong reportmemory = -1;
#endif

    /* if the time keyword was used */
    if (j->stat & STAT_TIMED)
	return 1;

    queue_signals();
    errflag = 0;
    if ((v = getvalue(&vbuf, &s, 0)))
	reporttime = getintvalue(v);
#ifdef HAVE_GETRUSAGE
    if ((v = getvalue(&vbuf, &sm, 0)))
	reportmemory = getintvalue(v);
#endif
    errflag = save_errflag;
    unqueue_signals();
    if (reporttime < 0
#ifdef HAVE_GETRUSAGE
	&& reportmemory < 0
#endif
	)
	return 0;
    /* can this ever happen? */
    if (!j->procs)
	return 0;
    if (zleactive)
	return 0;

    if (reporttime >= 0)
    {
#ifdef HAVE_GETRUSAGE
	reporttime -= j->procs->ti.ru_utime.tv_sec +
	    j->procs->ti.ru_stime.tv_sec;
	if (j->procs->ti.ru_utime.tv_usec +
	    j->procs->ti.ru_stime.tv_usec >= 1000000)
	    reporttime--;
	if (reporttime <= 0)
	    return 1;
#else
	{
	    clktck = get_clktck();
	    if ((j->procs->ti.ut + j->procs->ti.st) / clktck >= reporttime)
		return 1;
	}
#endif
    }

#ifdef HAVE_GETRUSAGE
    if (reportmemory >= 0 &&
	j->procs->ti.ru_maxrss / 1024 > reportmemory)
	return 1;
#endif

    return 0;
}

/* !(lng & 3) means jobs    *
 *  (lng & 1) means jobs -l *
 *  (lng & 2) means jobs -p
 *  (lng & 4) means jobs -d
 *
 * synch = 0 means asynchronous
 * synch = 1 means synchronous
 * synch = 2 means called synchronously from jobs
 * synch = 3 means called synchronously from bg or fg
 *
 * Returns 1 if some output was done.
 *
 * The function also deletes the job if it was done, even it
 * is not printed.
 */

/**/
int
printjob(Job jn, int lng, int synch)
{
    Process pn;
    int job, len = 9, sig, sflag = 0, llen;
    int conted = 0, lineleng = zterm_columns, skip = 0, doputnl = 0;
    int doneprint = 0, skip_print = 0;
    FILE *fout = (synch == 2 || !shout) ? stdout : shout;

    if (synch > 1 && oldjobtab != NULL)
	job = jn - oldjobtab;
    else
	job = jn - jobtab;
    DPUTS3(job < 0 || job > (oldjobtab && synch > 1 ? oldmaxjob : maxjob),
	   "bogus job number, jn = %L, jobtab = %L, oldjobtab = %L",
	   (long)jn, (long)jobtab, (long)oldjobtab);

    if (jn->stat & STAT_NOPRINT)
	skip_print = 1;

    if (lng < 0) {
	conted = 1;
	lng = !!isset(LONGLISTJOBS);
    }

    if (jn->stat & STAT_SUPERJOB &&
	jn->other)
    {
	Job sjn = &jobtab[jn->other];
	if (sjn->procs || sjn->auxprocs)
	{
	    /*
	     * A subjob still has process, which must finish before
	     * further execution of the superjob, which the user wants to
	     * know about.  So report the status of the subjob as if it
	     * were the user-visible superjob.
	     */
	    jn = sjn;
	}
    }

/* find length of longest signame, check to see */
/* if we really need to print this job          */

    for (pn = jn->procs; pn; pn = pn->next) {
	if (jn->stat & STAT_SUPERJOB &&
	    jn->procs->status == SP_RUNNING && !pn->next)
	    pn->status = SP_RUNNING;
	if (pn->status != SP_RUNNING) {
	    if (WIFSIGNALED(pn->status)) {
		sig = WTERMSIG(pn->status);
		llen = strlen(sigmsg(sig));
		if (WCOREDUMP(pn->status))
		    llen += 14;
		if (llen > len)
		    len = llen;
		if (sig != SIGINT && sig != SIGPIPE)
		    sflag = 1;
		if (job == thisjob && sig == SIGINT)
		    doputnl = 1;
		if (isset(PRINTEXITVALUE) && isset(SHINSTDIN)) {
		    sflag = 1;
		    skip_print = 0;
		}
	    } else if (WIFSTOPPED(pn->status)) {
		sig = WSTOPSIG(pn->status);
		if ((int)strlen(sigmsg(sig)) > len)
		    len = strlen(sigmsg(sig));
		if (job == thisjob && sig == SIGTSTP)
		    doputnl = 1;
	    } else if (isset(PRINTEXITVALUE) && isset(SHINSTDIN) &&
		       WEXITSTATUS(pn->status)) {
		sflag = 1;
		skip_print = 0;
	    }
	}
    }

    if (skip_print) {
	if (jn->stat & STAT_DONE) {
	    /* This looks silly, but see update_job() */
	    if (synch <= 1)
		storepipestats(jn, job == thisjob, job == thisjob);
	    if (should_report_time(jn))
		dumptime(jn);
	    deletejob(jn, 0);
	    if (job == curjob) {
		curjob = prevjob;
		prevjob = job;
	    }
	    if (job == prevjob)
		setprevjob();
	}
	return 0;
    }

    /*
     * - Always print if called from jobs
     * - Otherwise, require MONITOR option ("jobbing") and some
     *   change of state
     * - also either the shell is interactive or this is synchronous.
     */
    if (synch == 2 ||
	((interact || synch) && jobbing &&
	 ((jn->stat & STAT_STOPPED) || sflag || job != thisjob))) {
	int len2, fline = 1;
	/* POSIX requires just the job text for bg and fg */
	int plainfmt = (synch == 3) && isset(POSIXJOBS);
	/* use special format for current job, except in `jobs' */
	int thisfmt = job == thisjob && synch != 2;
	Process qn;

	if (!synch)
	    zleentry(ZLE_CMD_TRASH);
	if (doputnl && !synch) {
	    doneprint = 1;
	    putc('\n', fout);
	}
	for (pn = jn->procs; pn;) {
	    len2 = (thisfmt ? 5 : 10) + len;	/* 2 spaces */
	    if (lng & 3)
		qn = pn->next;
	    else
		for (qn = pn->next; qn; qn = qn->next) {
		    if (qn->status != pn->status)
			break;
		    if ((int)strlen(qn->text) + len2 + ((qn->next) ? 3 : 0)
			> lineleng)
			break;
		    len2 += strlen(qn->text) + 2;
		}
	    doneprint = 1;
	    if (!plainfmt) {
		if (!thisfmt || lng) {
		    if (fline)
			fprintf(fout, "[%ld]  %c ",
				(long)job,
				(job == curjob) ? '+'
				: (job == prevjob) ? '-' : ' ');
		    else
			fprintf(fout, (job > 9) ? "        " : "       ");
		} else
		    fprintf(fout, "zsh: ");
		if (lng & 1)
		    fprintf(fout, "%ld ", (long) pn->pid);
		else if (lng & 2) {
		    pid_t x = jn->gleader;

		    fprintf(fout, "%ld ", (long) x);
		    do
			skip++;
		    while ((x /= 10));
		    skip++;
		    lng &= ~3;
		} else
		    fprintf(fout, "%*s", skip, "");
		if (pn->status == SP_RUNNING) {
		    if (!conted)
			fprintf(fout, "running%*s", len - 7 + 2, "");
		    else
			fprintf(fout, "continued%*s", len - 9 + 2, "");
		}
		else if (WIFEXITED(pn->status)) {
		    if (WEXITSTATUS(pn->status))
			fprintf(fout, "exit %-4d%*s", WEXITSTATUS(pn->status),
				len - 9 + 2, "");
		    else
			fprintf(fout, "done%*s", len - 4 + 2, "");
		} else if (WIFSTOPPED(pn->status))
		    fprintf(fout, "%-*s", len + 2,
			    sigmsg(WSTOPSIG(pn->status)));
		else if (WCOREDUMP(pn->status))
		    fprintf(fout, "%s (core dumped)%*s",
			    sigmsg(WTERMSIG(pn->status)),
			    (int)(len - 14 + 2 -
				  strlen(sigmsg(WTERMSIG(pn->status)))), "");
		else
		    fprintf(fout, "%-*s", len + 2,
			    sigmsg(WTERMSIG(pn->status)));
	    }
	    for (; pn != qn; pn = pn->next) {
		char *txt = dupstring(pn->text);
		int txtlen;
		unmetafy(txt, &txtlen);
		fwrite(txt, sizeof(char), txtlen, fout);
		if (pn->next)
		    fputs(" | ", fout);
	    }
	    putc('\n', fout);
	    fline = 0;
	}
	fflush(fout);
    } else if (doputnl && interact && !synch) {
	doneprint = 1;
	putc('\n', fout);
	fflush(fout);
    }

    /* print "(pwd now: foo)" messages: with (lng & 4) we are printing
     * the directory where the job is running, otherwise the current directory
     */

    if ((lng & 4) || (interact && job == thisjob &&
		      jn->pwd && strcmp(jn->pwd, pwd))) {
	doneprint = 1;
	fprintf(fout, "(pwd %s: ", (lng & 4) ? "" : "now");
	fprintdir(((lng & 4) && jn->pwd) ? jn->pwd : pwd, fout);
	fprintf(fout, ")\n");
	fflush(fout);
    }

    /* delete job if done */

    if (jn->stat & STAT_DONE) {
	/* This looks silly, but see update_job() */
	if (synch <= 1)
	    storepipestats(jn, job == thisjob, job == thisjob);
	if (should_report_time(jn))
	    dumptime(jn);
	deletejob(jn, 0);
	if (job == curjob) {
	    curjob = prevjob;
	    prevjob = job;
	}
	if (job == prevjob)
	    setprevjob();
    } else
	jn->stat &= ~STAT_CHANGED;

    return doneprint;
}

/* Add a file to be deleted or fd to be closed to the current job */

/**/
void
addfilelist(const char *name, int fd)
{
    Jobfile jf = (Jobfile)zalloc(sizeof(struct jobfile));
    LinkList ll = jobtab[thisjob].filelist;

    if (!ll)
	ll = jobtab[thisjob].filelist = znewlinklist();
    if (name)
    {
	jf->u.name = ztrdup(name);
	jf->is_fd = 0;
    }
    else
    {
	jf->u.fd = fd;
	jf->is_fd = 1;
    }
    zaddlinknode(ll, jf);
}

/* Clean up pipes no longer needed associated with a job */

/**/
void
pipecleanfilelist(LinkList filelist, int proc_subst_only)
{
    LinkNode node;

    if (!filelist)
	return;
    node = firstnode(filelist);
    while (node) {
	Jobfile jf = (Jobfile)getdata(node);
	if (jf->is_fd &&
	    (!proc_subst_only || fdtable[jf->u.fd] == FDT_PROC_SUBST)) {
	    LinkNode next = nextnode(node);
	    zclose(jf->u.fd);
	    (void)remnode(filelist, node);
	    zfree(jf, sizeof(*jf));
	    node = next;
	} else
	    incnode(node);
    }
}

/* Finished with list of files for a job */

/**/
void
deletefilelist(LinkList file_list, int disowning)
{
    Jobfile jf;
    if (file_list) {
	while ((jf = (Jobfile)getlinknode(file_list))) {
	    if (jf->is_fd) {
		if (!disowning)
		    zclose(jf->u.fd);
	    } else {
		if (!disowning)
		    unlink(jf->u.name);
		zsfree(jf->u.name);
	    }
	    zfree(jf, sizeof(*jf));
	}
	zfree(file_list, sizeof(struct linklist));
    }
}

/**/
void
cleanfilelists(void)
{
    int i;

    DPUTS(shell_exiting >= 0, "BUG: cleanfilelists() before exit");
 
    for (i = 1; i <= maxjob; i++)
	deletefilelist(jobtab[i].filelist, 0);
}

/**/
void
freejob(Job jn, int deleting)
{
    struct process *pn, *nx;

    pn = jn->procs;
    jn->procs = NULL;
    for (; pn; pn = nx) {
	nx = pn->next;
	zfree(pn, sizeof(struct process));
    }

    pn = jn->auxprocs;
    jn->auxprocs = NULL;
    for (; pn; pn = nx) {
	nx = pn->next;
	zfree(pn, sizeof(struct process));
    }

    if (jn->ty)
	zfree(jn->ty, sizeof(struct ttyinfo));
    if (jn->pwd)
	zsfree(jn->pwd);
    jn->pwd = NULL;
    if (jn->stat & STAT_WASSUPER) {
	/* careful in case we shrink and move the job table */
	int job = jn - jobtab;
	if (deleting)
	    deletejob(jobtab + jn->other, 0);
	else
	    freejob(jobtab + jn->other, 0);
	jn = jobtab + job;
    }
    jn->gleader = jn->other = 0;
    jn->stat = jn->stty_in_env = 0;
    jn->filelist = NULL;
    jn->ty = NULL;

    /* Find the new highest job number. */
    if (maxjob == jn - jobtab) {
	while (maxjob && !(jobtab[maxjob].stat & STAT_INUSE))
	    maxjob--;
    }
}

/*
 * We are actually finished with this job, rather
 * than freeing it to make space.
 *
 * If "disowning" is set, files associated with the job are not
 * actually deleted --- and won't be as there is nothing left
 * to clear up.
 */

/**/
void
deletejob(Job jn, int disowning)
{
    deletefilelist(jn->filelist, disowning);
    if (jn->stat & STAT_ATTACH) {
	attachtty(mypgrp);
	adjustwinsize(0);
    }
    if (jn->stat & STAT_SUPERJOB) {
	Job jno = jobtab + jn->other;
	if (jno->stat & STAT_SUBJOB)
	    jno->stat |= STAT_SUBJOB_ORPHANED;
    }

    freejob(jn, 1);
}

/*
 * Add a process to the current job.
 * The third argument is 1 if we are adding a process which is not
 * part of the main pipeline but an auxiliary process used for
 * handling MULTIOS or process substitution.  We will wait for it
 * but not display job information about it.
 */

/**/
void
addproc(pid_t pid, char *text, int aux, struct timeval *bgtime,
	int gleader, int list_pipe_job_used)
{
    Process pn, *pnlist;

    DPUTS(thisjob == -1, "No valid job in addproc.");
    pn = (Process) zshcalloc(sizeof *pn);
    pn->pid = pid;
    if (text)
	strcpy(pn->text, text);
    else
	*pn->text = '\0';
    pn->status = SP_RUNNING;
    pn->next = NULL;

    if (!aux)
    {
	pn->bgtime = *bgtime;
	/*
	 * if this is the first process we are adding to
	 * the job, then it's the group leader.
	 *
	 * Exception: if the forked subshell reported its own group
	 * leader, set that.  If it reported the use of list_pipe_job,
	 * set it for that, too.
	 */
	if (gleader != -1) {
	    if (jobtab[thisjob].stat & STAT_CURSH)
		jobtab[thisjob].gleader = gleader;
	    else
		jobtab[thisjob].gleader = pid;
	    if (list_pipe_job_used != -1)
		jobtab[list_pipe_job_used].gleader = gleader;
	    /*
	     * Record here this is the latest process group to grab the
	     * terminal as attachtty() was run in the subshell.
	     */
	    last_attached_pgrp = gleader;
	} else if (!jobtab[thisjob].gleader)
		jobtab[thisjob].gleader = pid;
	/* attach this process to end of process list of current job */
	pnlist = &jobtab[thisjob].procs;
    }
    else
	pnlist = &jobtab[thisjob].auxprocs;

    if (*pnlist) {
	Process n;

	for (n = *pnlist; n->next; n = n->next);
	n->next = pn;
    } else {
	/* first process for this job */
	*pnlist = pn;
    }
    /* If the first process in the job finished before any others were *
     * added, maybe STAT_DONE got set incorrectly.  This can happen if *
     * a $(...) was waited for and the last existing job in the        *
     * pipeline was already finished.  We need to be very careful that *
     * there was no call to printjob() between then and now, else      *
     * the job will already have been deleted from the table.          */
    jobtab[thisjob].stat &= ~STAT_DONE;
}

/* Check if we have files to delete.  We need to check this to see *
 * if it's all right to exec a command without forking in the last *
 * component of subshells or after the `-c' option.                */

/**/
int
havefiles(void)
{
    int i;

    for (i = 1; i <= maxjob; i++)
	if (jobtab[i].stat && jobtab[i].filelist)
	    return 1;
    return 0;

}

/*
 * Wait for a particular process.
 * wait_cmd indicates this is from the interactive wait command,
 * in which case the behaviour is a little different:  the command
 * itself can be interrupted by a trapped signal.
 */

/**/
int
waitforpid(pid_t pid, int wait_cmd)
{
    int first = 1, q = queue_signal_level();

    /* child_block() around this loop in case #ifndef WNOHANG */
    dont_queue_signals();
    child_block();		/* unblocked in signal_suspend() */
    queue_traps(wait_cmd);

    /* This function should never be called with a pid that is not a
     * child of the current shell.  Consequently, if kill(0, pid)
     * fails here with ESRCH, the child has already been reaped.  In
     * the loop body, we expect this to happen in signal_suspend()
     * via zhandler(), after which this test terminates the loop.
     */
    while (!errflag && (kill(pid, 0) >= 0 || errno != ESRCH)) {
	if (first)
	    first = 0;
	else if (!wait_cmd)
	    kill(pid, SIGCONT);

	last_signal = -1;
	signal_suspend(SIGCHLD, wait_cmd);
	if (last_signal != SIGCHLD && wait_cmd && last_signal >= 0 &&
	    (sigtrapped[last_signal] & ZSIG_TRAPPED)) {
	    /* wait command interrupted, but no error: return */
	    restore_queue_signals(q);
	    return 128 + last_signal;
	}
	child_block();
    }
    unqueue_traps();
    child_unblock();
    restore_queue_signals(q);

    return 0;
}

/*
 * Wait for a job to finish.
 * wait_cmd indicates this is from the wait builtin; see
 * wait_cmd in waitforpid().
 */

/**/
static int
zwaitjob(int job, int wait_cmd)
{
    int q = queue_signal_level();
    Job jn = jobtab + job;

    child_block();		 /* unblocked during signal_suspend() */
    queue_traps(wait_cmd);
    dont_queue_signals();
    if (jn->procs || jn->auxprocs) { /* if any forks were done         */
	jn->stat |= STAT_LOCKED;
	if (jn->stat & STAT_CHANGED)
	    printjob(jn, !!isset(LONGLISTJOBS), 1);
	if (jn->filelist) {
	    /*
	     * The main shell is finished with any file descriptors used
	     * for process substitution associated with this job: close
	     * them to indicate to listeners there's no more input.
	     *
	     * Note we can't safely delete temporary files yet as these
	     * are directly visible to other processes.  However,
	     * we can't deadlock on the fact that those still exist, so
	     * that's not a problem.
	     */
	    pipecleanfilelist(jn->filelist, 0);
	}
	while (!(errflag & ERRFLAG_ERROR) && jn->stat &&
	       !(jn->stat & STAT_DONE) &&
	       !(interact && (jn->stat & STAT_STOPPED))) {
	    signal_suspend(SIGCHLD, wait_cmd);
	    if (last_signal != SIGCHLD && wait_cmd && last_signal >= 0 &&
		(sigtrapped[last_signal] & ZSIG_TRAPPED))
	    {
		/* builtin wait interrupted by trapped signal */
		restore_queue_signals(q);
		return 128 + last_signal;
	    }
           /* Commenting this out makes ^C-ing a job started by a function
              stop the whole function again.  But I guess it will stop
              something else from working properly, we have to find out
              what this might be.  --oberon

	      When attempting to separate errors and interrupts, we
	      assumed because of the previous comment it would be OK
	      to remove ERRFLAG_ERROR and leave ERRFLAG_INT set, since
	      that's the one related to ^C.  But that doesn't work.
	      There's something more here we don't understand.  --pws

	      The change above to ignore ERRFLAG_INT in the loop test
	      solves a problem wherein child processes that ignore the
	      INT signal were never waited-for.  Clearing the flag here
	      still seems the wrong thing, but perhaps ERRFLAG_INT
	      should be saved and restored around signal_suspend() to
	      prevent it being lost within a signal trap?  --Bart

           errflag = 0; */

	    if (subsh)
		killjb(jn, SIGCONT);
	    if (jn->stat & STAT_SUPERJOB)
		if (handle_sub(jn - jobtab, 1))
		    break;
	    child_block();
	}
    } else {
	deletejob(jn, 0);
	pipestats[0] = lastval;
	numpipestats = 1;
    }
    restore_queue_signals(q);
    unqueue_traps();
    child_unblock();

    return 0;
}

static void waitonejob(Job jn)
{
    if (jn->procs || jn->auxprocs)
	zwaitjob(jn - jobtab, 0);
    else {
	deletejob(jn, 0);
	pipestats[0] = lastval;
	numpipestats = 1;
    }
}

/* wait for running job to finish */

/**/
void
waitjobs(void)
{
    Job jn = jobtab + thisjob;
    DPUTS(thisjob == -1, "No valid job in waitjobs.");

    /* If there's a subjob, it should finish first. */
    if (jn->stat & STAT_SUPERJOB)
	waitonejob(jobtab + jn->other);
    waitonejob(jn);

    thisjob = -1;
}

/* clear job table when entering subshells */

/**/
mod_export void
clearjobtab(int monitor)
{
    int i;

    if (isset(POSIXJOBS))
	oldmaxjob = 0;
    for (i = 1; i <= maxjob; i++) {
	/*
	 * See if there is a jobtable worth saving.
	 * We never free the saved version; it only happens
	 * once for each subshell of a shell with job control,
	 * so doesn't create a leak.
	 */
	if (monitor && !isset(POSIXJOBS) && jobtab[i].stat)
	    oldmaxjob = i+1;
	else if (jobtab[i].stat & STAT_INUSE)
	    freejob(jobtab + i, 0);
    }

    if (monitor && oldmaxjob) {
	int sz = oldmaxjob * sizeof(struct job);
	if (oldjobtab)
	    free(oldjobtab);
	oldjobtab = (struct job *)zalloc(sz);
	memcpy(oldjobtab, jobtab, sz);

	/* Don't report any job we're part of */
	if (thisjob != -1 && thisjob < oldmaxjob)
	    memset(oldjobtab+thisjob, 0, sizeof(struct job));

	/* oldmaxjob is now the size of the table, but outside
	 * this function, it's used as a job number, which must
	 * be the largest index available in the table.
	 */
	--oldmaxjob;
    }


    memset(jobtab, 0, jobtabsize * sizeof(struct job)); /* zero out table */
    maxjob = 0;

    /*
     * Although we don't have job control in subshells, we
     * sometimes needs control structures for other purposes such
     * as multios.  Grab a job for this purpose; any will do
     * since we've freed them all up (so there's no question
     * of problems with the job table size here).
     */
    thisjob = initjob();
}

/* In a subshell, decide we want our own job table after all. */

/**/
mod_export void
clearoldjobtab(void)
{
    if (oldjobtab)
	free(oldjobtab);
    oldjobtab = NULL;
    oldmaxjob = 0;
}

static int initnewjob(int i)
{
    jobtab[i].stat = STAT_INUSE;
    if (jobtab[i].pwd) {
	zsfree(jobtab[i].pwd);
	jobtab[i].pwd = NULL;
    }
    jobtab[i].gleader = 0;

    if (i > maxjob)
	maxjob = i;

    return i;
}

/* Get a free entry in the job table and initialize it. */

/**/
int
initjob(void)
{
    int i;

    for (i = 1; i <= maxjob; i++)
	if (!jobtab[i].stat)
	    return initnewjob(i);
    if (maxjob + 1 < jobtabsize)
	return initnewjob(maxjob+1);

    if (expandjobtab())
	return initnewjob(i);

    zerr("job table full or recursion limit exceeded");
    return -1;
}

/**/
void
setjobpwd(void)
{
    int i;

    for (i = 1; i <= maxjob; i++)
	if (jobtab[i].stat && !jobtab[i].pwd)
	    jobtab[i].pwd = ztrdup(pwd);
}

/* print pids for & */

/**/
void
spawnjob(void)
{
    Process pn;

    DPUTS(thisjob == -1, "No valid job in spawnjob.");
    /* if we are not in a subshell */
    if (!subsh) {
	if (curjob == -1 || !(jobtab[curjob].stat & STAT_STOPPED)) {
	    curjob = thisjob;
	    setprevjob();
	} else if (prevjob == -1 || !(jobtab[prevjob].stat & STAT_STOPPED))
	    prevjob = thisjob;
	if (jobbing && jobtab[thisjob].procs) {
	    FILE *fout = shout ? shout : stdout;
	    fprintf(fout, "[%d]", thisjob);
	    for (pn = jobtab[thisjob].procs; pn; pn = pn->next)
		fprintf(fout, " %ld", (long) pn->pid);
	    fprintf(fout, "\n");
	    fflush(fout);
	}
    }
    if (!hasprocs(thisjob))
	deletejob(jobtab + thisjob, 0);
    else {
	jobtab[thisjob].stat |= STAT_LOCKED;
	pipecleanfilelist(jobtab[thisjob].filelist, 0);
    }
    thisjob = -1;
}

/**/
void
shelltime(void)
{
    struct timezone dummy_tz;
    struct timeval dtimeval, now;
    child_times_t ti;
#ifndef HAVE_GETRUSAGE
    struct tms buf;
#endif

    gettimeofday(&now, &dummy_tz);

#ifdef HAVE_GETRUSAGE
    getrusage(RUSAGE_SELF, &ti);
#else
    times(&buf);

    ti.ut = buf.tms_utime;
    ti.st = buf.tms_stime;
#endif
    printtime(dtime(&dtimeval, &shtimer, &now), &ti, "shell");

#ifdef HAVE_GETRUSAGE
    getrusage(RUSAGE_CHILDREN, &ti);
#else
    ti.ut = buf.tms_cutime;
    ti.st = buf.tms_cstime;
#endif
    printtime(&dtimeval, &ti, "children");

}

/* see if jobs need printing */
 
/**/
void
scanjobs(void)
{
    int i;
 
    for (i = 1; i <= maxjob; i++)
        if (jobtab[i].stat & STAT_CHANGED)
            printjob(jobtab + i, !!isset(LONGLISTJOBS), 1);
}

/**** job control builtins ****/

/* This simple function indicates whether or not s may represent      *
 * a number.  It returns true iff s consists purely of digits and     *
 * minuses.  Note that minus may appear more than once.               */

/**/
static int
isanum(char *s)
{
    if (*s == '\0')
	return 0;
    while (*s == '-' || idigit(*s))
	s++;
    return *s == '\0';
}

/* Make sure we have a suitable current and previous job set. */

/**/
static void
setcurjob(void)
{
    if (curjob == thisjob ||
	(curjob != -1 && !(jobtab[curjob].stat & STAT_INUSE))) {
	curjob = prevjob;
	setprevjob();
	if (curjob == thisjob ||
	    (curjob != -1 && !((jobtab[curjob].stat & STAT_INUSE) &&
			       curjob != thisjob))) {
	    curjob = prevjob;
	    setprevjob();
	}
    }
}

/* Find the job table for reporting jobs */

/**/
mod_export void
selectjobtab(Job *jtabp, int *jmaxp)
{
    if (oldjobtab)
    {
	/* In subshell --- use saved job table to report */
	*jtabp = oldjobtab;
	*jmaxp = oldmaxjob;
    }
    else
    {
	/* Use main job table */
	*jtabp = jobtab;
	*jmaxp = maxjob;
    }
}

/* Convert a job specifier ("%%", "%1", "%foo", "%?bar?", etc.) *
 * to a job number.                                             */

/**/
mod_export int
getjob(const char *s, const char *prog)
{
    int jobnum, returnval, mymaxjob;
    Job myjobtab;

    selectjobtab(&myjobtab, &mymaxjob);

    /* if there is no %, treat as a name */
    if (*s != '%')
	goto jump;
    s++;
    /* "%%", "%+" and "%" all represent the current job */
    if (*s == '%' || *s == '+' || !*s) {
	if (curjob == -1) {
	    if (prog && !isset(POSIXBUILTINS))
		zwarnnam(prog, "no current job");
	    returnval = -1;
	    goto done;
	}
	returnval = curjob;
	goto done;
    }
    /* "%-" represents the previous job */
    if (*s == '-') {
	if (prevjob == -1) {
	    if (prog && !isset(POSIXBUILTINS))
		zwarnnam(prog, "no previous job");
	    returnval = -1;
	    goto done;
	}
	returnval = prevjob;
	goto done;
    }
    /* a digit here means we have a job number */
    if (idigit(*s)) {
	jobnum = atoi(s);
	if (jobnum > 0 && jobnum <= mymaxjob && myjobtab[jobnum].stat &&
	    !(myjobtab[jobnum].stat & STAT_SUBJOB) &&
	    /*
	     * If running jobs in a subshell, we are allowed to
	     * refer to the "current" job (it's not really the
	     * current job in the subshell).  It's possible we
	     * should reset thisjob to -1 on entering the subshell.
	     */
	    (myjobtab == oldjobtab || jobnum != thisjob)) {
	    returnval = jobnum;
	    goto done;
	}
	if (prog && !isset(POSIXBUILTINS))
	    zwarnnam(prog, "%%%s: no such job", s);
	returnval = -1;
	goto done;
    }
    /* "%?" introduces a search string */
    if (*s == '?') {
	struct process *pn;

	for (jobnum = mymaxjob; jobnum >= 0; jobnum--)
	    if (myjobtab[jobnum].stat &&
		!(myjobtab[jobnum].stat & STAT_SUBJOB) &&
		jobnum != thisjob)
		for (pn = myjobtab[jobnum].procs; pn; pn = pn->next)
		    if (strstr(pn->text, s + 1)) {
			returnval = jobnum;
			goto done;
		    }
	if (prog && !isset(POSIXBUILTINS))
	    zwarnnam(prog, "job not found: %s", s);
	returnval = -1;
	goto done;
    }
  jump:
    /* anything else is a job name, specified as a string that begins the
    job's command */
    if ((jobnum = findjobnam(s)) != -1) {
	returnval = jobnum;
	goto done;
    }
    /* if we get here, it is because none of the above succeeded and went
    to done */
    if (!isset(POSIXBUILTINS))
	zwarnnam(prog, "job not found: %s", s);
    returnval = -1;
  done:
    return returnval;
}

#ifndef HAVE_SETPROCTITLE
/* For jobs -Z (which modifies the shell's name as seen in ps listings).  *
 * hackzero is the start of the safely writable space, and hackspace is   *
 * its length, excluding a final NUL terminator that will always be left. */

static char *hackzero;
static int hackspace;
#endif


/* Initialise job handling. */

/**/
void
init_jobs(char **argv, char **envp)
{
#ifndef HAVE_SETPROCTITLE
    char *p, *q;
#endif
    size_t init_bytes = MAXJOBS_ALLOC*sizeof(struct job);

    /*
     * Initialise the job table.  If this fails, we're in trouble.
     */
    jobtab = (struct job *)zalloc(init_bytes);
    if (!jobtab) {
	zerr("failed to allocate job table, aborting.");
	exit(1);
    }
    jobtabsize = MAXJOBS_ALLOC;
    memset(jobtab, 0, init_bytes);

#ifndef HAVE_SETPROCTITLE
    /*
     * Initialise the jobs -Z system.  The technique is borrowed from
     * perl: check through the argument and environment space, to see
     * how many of the strings are in contiguous space.  This determines
     * the value of hackspace.
     */
    hackzero = *argv;
    p = strchr(hackzero, 0);
    while(*++argv) {
	q = *argv;
	if(q != p+1)
	    goto done;
	p = strchr(q, 0);
    }
#if !defined(HAVE_PUTENV) && !defined(USE_SET_UNSET_ENV)
    for(; *envp; envp++) {
	q = *envp;
	if(q != p+1)
	    goto done;
	p = strchr(q, 0);
    }
#endif
    done:
    hackspace = p - hackzero;
#endif
}


/*
 * We have run out of space in the job table.
 * Expand it by an additional MAXJOBS_ALLOC slots.
 */

/*
 * An arbitrary limit on the absolute maximum size of the job table.
 * This prevents us taking over the entire universe.
 * Ought to be a multiple of MAXJOBS_ALLOC, but doesn't need to be.
 */
#define MAX_MAXJOBS	1000

/**/
int
expandjobtab(void)
{
    int newsize = jobtabsize + MAXJOBS_ALLOC;
    struct job *newjobtab;

    if (newsize > MAX_MAXJOBS)
	return 0;

    newjobtab = (struct job *)zrealloc(jobtab, newsize * sizeof(struct job));
    if (!newjobtab)
	return 0;

    /*
     * Clear the new section of the table; this is necessary for
     * the jobs to appear unused.
     */
    memset(newjobtab + jobtabsize, 0, MAXJOBS_ALLOC * sizeof(struct job));

    jobtab = newjobtab;
    jobtabsize = newsize;

    return 1;
}


/*
 * See if we can reduce the job table.  We can if we go over
 * a MAXJOBS_ALLOC boundary.  However, we leave a boundary,
 * currently 20 jobs, so that we have a place for immediate
 * expansion and don't play ping pong with the job table size.
 */

/**/
void
maybeshrinkjobtab(void)
{
    int jobbound;

    queue_signals();
    jobbound = maxjob + MAXJOBS_ALLOC - (maxjob % MAXJOBS_ALLOC);
    if (jobbound < jobtabsize && jobbound > maxjob + 20) {
	struct job *newjobtab;

	/* Hope this can't fail, but anyway... */
	newjobtab = (struct job *)zrealloc(jobtab,
					   jobbound*sizeof(struct job));

	if (newjobtab) {
	    jobtab = newjobtab;
	    jobtabsize = jobbound;
	}
    }
    unqueue_signals();
}

/*
 * Definitions for the background process stuff recorded below.
 * This would be more efficient as a hash, but
 * - that's quite heavyweight for something not needed very often
 * - we need some kind of ordering as POSIX allows us to limit
 *   the size of the list to the value of _SC_CHILD_MAX and clearly
 *   we want to clear the oldest first
 * - cases with a long list of background jobs where the user doesn't
 *   wait for a large number, and then does wait for one (the only
 *   inefficient case) are rare
 * - in the context of waiting for an external process, looping
 *   over a list isn't so very inefficient.
 * Enough excuses already.
 */

/* Data in the link list, a key (process ID) / value (exit status) pair. */
struct bgstatus {
    pid_t pid;
    int status;
};
typedef struct bgstatus *Bgstatus;
/* The list of those entries */
static LinkList bgstatus_list;
/* Count of entries.  Reaches value of _SC_CHILD_MAX and stops. */
static long bgstatus_count;

/*
 * Remove and free a bgstatus entry.
 */
static void rembgstatus(LinkNode node)
{
    zfree(remnode(bgstatus_list, node), sizeof(struct bgstatus));
    bgstatus_count--;
}

/*
 * Record the status of a background process that exited so we
 * can execute the builtin wait for it.
 *
 * We can't execute the wait builtin for something that exited in the
 * foreground as it's not visible to the user, so don't bother recording.
 */

/**/
void
addbgstatus(pid_t pid, int status)
{
    static long child_max;
    Bgstatus bgstatus_entry;

    if (!child_max) {
#ifdef _SC_CHILD_MAX
	child_max = sysconf(_SC_CHILD_MAX);
	if (!child_max) /* paranoia */
#endif
	{
	    /* Be inventive */
	    child_max = 1024L;
	}
    }

    if (!bgstatus_list) {
	bgstatus_list = znewlinklist();
	/*
	 * We're not always robust about memory failures, but
	 * this is pretty deep in the shell basics to be failing owing
	 * to memory, and a failure to wait is reported loudly, so test
	 * and fail silently here.
	 */
	if (!bgstatus_list)
	    return;
    }
    if (bgstatus_count == child_max) {
	/* Overflow.  List is in order, remove first */
	rembgstatus(firstnode(bgstatus_list));
    }
    bgstatus_entry = (Bgstatus)zalloc(sizeof(*bgstatus_entry));
    if (!bgstatus_entry) {
	/* See note above */
	return;
    }
    bgstatus_entry->pid = pid;
    bgstatus_entry->status = status;
    if (!zaddlinknode(bgstatus_list, bgstatus_entry)) {
	zfree(bgstatus_entry, sizeof(*bgstatus_entry));
	return;
    }
    bgstatus_count++;
}

/*
 * See if pid has a recorded exit status.
 * Note we make no guarantee that the PIDs haven't wrapped, so this
 * may not be the right process.
 *
 * This is only used by wait, which must only work on each
 * pid once, so we need to remove the entry if we find it.
 */

static int getbgstatus(pid_t pid)
{
    LinkNode node;
    Bgstatus bgstatus_entry;

    if (!bgstatus_list)
	return -1;
    for (node = firstnode(bgstatus_list); node; incnode(node)) {
	bgstatus_entry = (Bgstatus)getdata(node);
	if (bgstatus_entry->pid == pid) {
	    int status = bgstatus_entry->status;
	    rembgstatus(node);
	    return status;
	}
    }
    return -1;
}

/* bg, disown, fg, jobs, wait: most of the job control commands are     *
 * here.  They all take the same type of argument.  Exception: wait can *
 * take a pid or a job specifier, whereas the others only work on jobs. */

/**/
int
bin_fg(char *name, char **argv, Options ops, int func)
{
    int job, lng, firstjob = -1, retval = 0, ofunc = func;

    if (OPT_ISSET(ops,'Z')) {
	int len;

	if(isset(RESTRICTED)) {
	    zwarnnam(name, "-Z is restricted");
	    return 1;
	}
	if(!argv[0] || argv[1]) {
	    zwarnnam(name, "-Z requires one argument");
	    return 1;
	}
	queue_signals();
	unmetafy(*argv, &len);
#ifdef HAVE_SETPROCTITLE
	setproctitle("%s", *argv);
#else
	if(len > hackspace)
	    len = hackspace;
	memcpy(hackzero, *argv, len);
	memset(hackzero + len, 0, hackspace - len);
#endif

#ifdef HAVE_PRCTL
	/* try to change /proc/$$/comm which will *
	 * be used when checking with "ps -e"  */
#include <sys/prctl.h>
	prctl(PR_SET_NAME, *argv);
#endif
	unqueue_signals();
	return 0;
    }

    if (func == BIN_JOBS) {
	lng = (OPT_ISSET(ops,'l')) ? 1 : (OPT_ISSET(ops,'p')) ? 2 : 0;
	if (OPT_ISSET(ops,'d'))
	    lng |= 4;
    } else {
	lng = !!isset(LONGLISTJOBS);
    }

    if ((func == BIN_FG || func == BIN_BG) && !jobbing) {
	/* oops... maybe bg and fg should have been disabled? */
	zwarnnam(name, "no job control in this shell.");
	return 1;
    }

    queue_signals();
    /*
     * In case any processes changed state recently, wait for them.
     * This updates stopped processes (but we should have been
     * signalled about those, up to inevitable races), and also
     * continued processes if that feature is available.
     */
    wait_for_processes();

    /* If necessary, update job table. */
    if (unset(NOTIFY))
	scanjobs();

    if (func != BIN_JOBS || isset(MONITOR) || !oldmaxjob)
	setcurjob();

    if (func == BIN_JOBS)
        /* If you immediately type "exit" after "jobs", this      *
         * will prevent zexit from complaining about stopped jobs */
	stopmsg = 2;
    if (!*argv) {
	/* This block handles all of the default cases (no arguments).  bg,
	fg and disown act on the current job, and jobs and wait act on all the
	jobs. */
 	if (func == BIN_FG || func == BIN_BG || func == BIN_DISOWN) {
	    /* W.r.t. the above comment, we'd better have a current job at this
	    point or else. */
	    if (curjob == -1 || (jobtab[curjob].stat & STAT_NOPRINT)) {
		zwarnnam(name, "no current job");
		unqueue_signals();
		return 1;
	    }
	    firstjob = curjob;
	} else if (func == BIN_JOBS) {
	    /* List jobs. */
	    struct job *jobptr;
	    int curmaxjob, ignorejob;
	    if (unset(MONITOR) && oldmaxjob) {
		jobptr = oldjobtab;
		curmaxjob = oldmaxjob ? oldmaxjob - 1 : 0;
		ignorejob = 0;
	    } else {
		jobptr = jobtab;
		curmaxjob = maxjob;
		ignorejob = thisjob;
	    }
	    for (job = 0; job <= curmaxjob; job++, jobptr++)
		if (job != ignorejob && jobptr->stat) {
		    if ((!OPT_ISSET(ops,'r') && !OPT_ISSET(ops,'s')) ||
			(OPT_ISSET(ops,'r') && OPT_ISSET(ops,'s')) ||
			(OPT_ISSET(ops,'r') && 
			 !(jobptr->stat & STAT_STOPPED)) ||
			(OPT_ISSET(ops,'s') && jobptr->stat & STAT_STOPPED))
			printjob(jobptr, lng, 2);
		}
	    unqueue_signals();
	    return 0;
	} else {   /* Must be BIN_WAIT, so wait for all jobs */
	    for (job = 0; job <= maxjob; job++)
		if (job != thisjob && jobtab[job].stat &&
		    !(jobtab[job].stat & STAT_NOPRINT))
		    retval = zwaitjob(job, 1);
	    unqueue_signals();
	    return retval;
	}
    }

    /* Defaults have been handled.  We now have an argument or two, or three...
    In the default case for bg, fg and disown, the argument will be provided by
    the above routine.  We now loop over the arguments. */
    for (; (firstjob != -1) || *argv; (void)(*argv && argv++)) {
	int stopped, ocj = thisjob, jstat;

        func = ofunc;

	if (func == BIN_WAIT && isanum(*argv)) {
	    /* wait can take a pid; the others can't. */
	    pid_t pid = (long)atoi(*argv);
	    Job j;
	    Process p;

	    if (findproc(pid, &j, &p, 0)) {
		if (j->stat & STAT_STOPPED)
		    retval = (killjb(j, SIGCONT) != 0);
		if (retval == 0) {
		    /*
		     * returns 0 for normal exit, else signal+128
		     * in which case we should return that status.
		     */
		    retval = waitforpid(pid, 1);
		}
		if (retval == 0) {
		    if ((retval = getbgstatus(pid)) < 0) {
			retval = lastval2;
		    }
		}
	    } else if ((retval = getbgstatus(pid)) < 0) {
		if (!isset(POSIXBUILTINS))
		    zwarnnam(name, "pid %d is not a child of this shell", pid);
		/* presumably lastval2 doesn't tell us a heck of a lot? */
		retval = 127;
	    }
	    thisjob = ocj;
	    continue;
	}
	if (func != BIN_JOBS && oldjobtab != NULL) {
	    zwarnnam(name, "can't manipulate jobs in subshell");
	    unqueue_signals();
	    return 1;
	}
	/* The only type of argument allowed now is a job spec.  Check it. */
	job = (*argv) ? getjob(*argv, name) : firstjob;
	firstjob = -1;
	if (job == -1) {
	    retval = 127;
	    break;
	}
	jstat = oldjobtab ? oldjobtab[job].stat : jobtab[job].stat;
	if (!(jstat & STAT_INUSE) ||
	    (jstat & STAT_NOPRINT)) {
	    if (!isset(POSIXBUILTINS))
		zwarnnam(name, "%s: no such job", *argv);
	    unqueue_signals();
	    return 127;
	}
        /* If AUTO_CONTINUE is set (automatically make stopped jobs running
         * on disown), we actually do a bg and then delete the job table entry. */

        if (isset(AUTOCONTINUE) && func == BIN_DISOWN &&
            jstat & STAT_STOPPED)
            func = BIN_BG;

	/* We have a job number.  Now decide what to do with it. */
	switch (func) {
	case BIN_FG:
	case BIN_BG:
	case BIN_WAIT:
	    if (func == BIN_BG) {
		clearoldjobtab();
		jobtab[job].stat |= STAT_NOSTTY;
		jobtab[job].stat &= ~STAT_CURSH;
	    }
	    if ((stopped = (jobtab[job].stat & STAT_STOPPED))) {
		makerunning(jobtab + job);
		if (func == BIN_BG) {
		    /* Set $! to indicate this was backgrounded */
		    Process pn = jobtab[job].procs;
		    for (;;) {
			Process next = pn->next;
			if (!next) {
			    lastpid = (zlong) pn->pid;
			    break;
			}
			pn = next;
		    }
		}
	    } else if (func == BIN_BG) {
		/* Silly to bg a job already running. */
		zwarnnam(name, "job already in background");
		thisjob = ocj;
		unqueue_signals();
		return 1;
	    }
	    /* It's time to shuffle the jobs around!  Reset the current job,
	    and pick a sensible secondary job. */
	    if (curjob == job) {
		curjob = prevjob;
		prevjob = (func == BIN_BG) ? -1 : job;
	    }
	    if (prevjob == job || prevjob == -1)
		setprevjob();
	    if (curjob == -1) {
		curjob = prevjob;
		setprevjob();
	    }
	    if (func != BIN_WAIT)
		/* for bg and fg -- show the job we are operating on */
		printjob(jobtab + job, (stopped) ? -1 : lng, 3);
	    if (func != BIN_BG) {		/* fg or wait */
		if (jobtab[job].pwd && strcmp(jobtab[job].pwd, pwd)) {
		    FILE *fout = (func == BIN_JOBS || !shout) ? stdout : shout;
		    fprintf(fout, "(pwd : ");
		    fprintdir(jobtab[job].pwd, fout);
		    fprintf(fout, ")\n");
		    fflush(fout);
		}
		if (func != BIN_WAIT) {		/* fg */
		    thisjob = job;
		    if ((jobtab[job].stat & STAT_SUPERJOB) &&
			((!jobtab[job].procs->next ||
			  (jobtab[job].stat & STAT_SUBLEADER) ||
			  (killpg(jobtab[job].gleader, 0) == -1  &&
			  errno == ESRCH))) &&
			jobtab[jobtab[job].other].gleader)
			attachtty(jobtab[jobtab[job].other].gleader);
		    else
			attachtty(jobtab[job].gleader);
		}
	    }
	    if (stopped) {
		if (func != BIN_BG && jobtab[job].ty)
		    settyinfo(jobtab[job].ty);
		killjb(jobtab + job, SIGCONT);
	    }
	    if (func == BIN_WAIT)
	    {
		retval = zwaitjob(job, 1);
		if (!retval)
		    retval = lastval2;
	    }
	    else if (func != BIN_BG) {
		/*
		 * HERE: there used not to be an "else" above.  How
		 * could it be right to wait for the foreground job
		 * when we've just been told to wait for another
		 * job (and done it)?
		 */
		waitjobs();
		retval = lastval2;
	    } else if (ofunc == BIN_DISOWN)
	        deletejob(jobtab + job, 1);
	    break;
	case BIN_JOBS:
	    printjob(job + (oldjobtab ? oldjobtab : jobtab), lng, 2);
	    break;
	case BIN_DISOWN:
	    if (jobtab[job].stat & STAT_SUPERJOB) {
		jobtab[job].stat |= STAT_DISOWN;
		continue;
	    }
	    if (jobtab[job].stat & STAT_STOPPED) {
		char buf[20], *pids = "";

		if (jobtab[job].stat & STAT_SUPERJOB) {
		    Process pn;

		    for (pn = jobtab[jobtab[job].other].procs; pn; pn = pn->next) {
			sprintf(buf, " -%d", pn->pid);
			pids = dyncat(pids, buf);
		    }
		    for (pn = jobtab[job].procs; pn->next; pn = pn->next) {
			sprintf(buf, " %d", pn->pid);
			pids = dyncat(pids, buf);
		    }
		    if (!jobtab[jobtab[job].other].procs && pn) {
			sprintf(buf, " %d", pn->pid);
			pids = dyncat(pids, buf);
		    }
		} else {
		    sprintf(buf, " -%d", jobtab[job].gleader);
		    pids = buf;
		}
                zwarnnam(name,
#ifdef USE_SUSPENDED
                         "warning: job is suspended, use `kill -CONT%s' to resume",
#else
                         "warning: job is stopped, use `kill -CONT%s' to resume",
#endif
                         pids);
	    }
	    deletejob(jobtab + job, 1);
	    break;
	}
	thisjob = ocj;
    }
    unqueue_signals();
    return retval;
}

static const struct {
    const char *name;
    int num;
} alt_sigs[] = {
#if defined(SIGCHLD) && defined(SIGCLD)
#if SIGCHLD == SIGCLD
    { "CLD", SIGCLD },
#endif
#endif
#if defined(SIGPOLL) && defined(SIGIO)
#if SIGPOLL == SIGIO
    { "IO", SIGIO },
#endif
#endif
#if !defined(SIGERR)
    /*
     * If SIGERR is not defined by the operating system, use it
     * as an alias for SIGZERR.
     */
    { "ERR", SIGZERR },
#endif
    { NULL, 0 }
};

/* kill: send a signal to a process.  The process(es) may be specified *
 * by job specifier (see above) or pid.  A signal, defaulting to       *
 * SIGTERM, may be specified by name or number, preceded by a dash.    */

/**/
int
bin_kill(char *nam, char **argv, UNUSED(Options ops), UNUSED(int func))
{
    int sig = SIGTERM;
    int returnval = 0;

    /* check for, and interpret, a signal specifier */
    if (*argv && **argv == '-') {
	if (idigit((*argv)[1])) {
	    char *endp;
	    /* signal specified by number */
	    sig = zstrtol(*argv + 1, &endp, 10);
	    if (*endp) {
		zwarnnam(nam, "invalid signal number: %s", *argv);
		return 1;
	    }
	} else if ((*argv)[1] != '-' || (*argv)[2]) {
	    char *signame;

	    /* with argument "-l" display the list of signal names */
	    if ((*argv)[1] == 'l' && (*argv)[2] == '\0') {
		if (argv[1]) {
		    while (*++argv) {
			sig = zstrtol(*argv, &signame, 10);
			if (signame == *argv) {
			    if (!strncmp(signame, "SIG", 3))
				signame += 3;
			    for (sig = 1; sig <= SIGCOUNT; sig++)
				if (!strcasecmp(sigs[sig], signame))
				    break;
			    if (sig > SIGCOUNT) {
				int i;

				for (i = 0; alt_sigs[i].name; i++)
				    if (!strcasecmp(alt_sigs[i].name, signame))
				    {
					sig = alt_sigs[i].num;
					break;
				    }
			    }
			    if (sig > SIGCOUNT) {
				zwarnnam(nam, "unknown signal: SIG%s",
					 signame);
				returnval++;
			    } else
				printf("%d\n", sig);
			} else {
			    if (*signame) {
				zwarnnam(nam, "unknown signal: SIG%s",
					 signame);
				returnval++;
			    } else {
				if (WIFSIGNALED(sig))
				    sig = WTERMSIG(sig);
				else if (WIFSTOPPED(sig))
				    sig = WSTOPSIG(sig);
				if (1 <= sig && sig <= SIGCOUNT)
				    printf("%s\n", sigs[sig]);
				else
				    printf("%d\n", sig);
			    }
			}
		    }
		    return returnval;
		}
		printf("%s", sigs[1]);
		for (sig = 2; sig <= SIGCOUNT; sig++)
		    printf(" %s", sigs[sig]);
		putchar('\n');
		return 0;
	    }

    	    if ((*argv)[1] == 'n' && (*argv)[2] == '\0') {
	    	char *endp;

	    	if (!*++argv) {
		    zwarnnam(nam, "-n: argument expected");
		    return 1;
		}
		sig = zstrtol(*argv, &endp, 10);
		if (*endp) {
		    zwarnnam(nam, "invalid signal number: %s", *argv);
		    return 1;
		}
	    } else {
		if (!((*argv)[1] == 's' && (*argv)[2] == '\0'))
		    signame = *argv + 1;
		else if (!(*++argv)) {
		    zwarnnam(nam, "-s: argument expected");
		    return 1;
		} else
		    signame = *argv;
		if (!*signame) {
		    zwarnnam(nam, "-: signal name expected");
		    return 1;
		}
		signame = casemodify(signame, CASMOD_UPPER);
		if (!strncmp(signame, "SIG", 3))
		    signame+=3;

		/* check for signal matching specified name */
		for (sig = 1; sig <= SIGCOUNT; sig++)
		    if (!strcmp(*(sigs + sig), signame))
			break;
		if (*signame == '0' && !signame[1])
		    sig = 0;
		if (sig > SIGCOUNT) {
		    int i;

		    for (i = 0; alt_sigs[i].name; i++)
			if (!strcmp(alt_sigs[i].name, signame))
			{
			    sig = alt_sigs[i].num;
			    break;
			}
		}
		if (sig > SIGCOUNT) {
		    zwarnnam(nam, "unknown signal: SIG%s", signame);
		    zwarnnam(nam, "type kill -l for a list of signals");
		    return 1;
		}
	    }
	}
	argv++;
    }

    /* Discard the standard "-" and "--" option breaks */
    if (*argv && (*argv)[0] == '-' && (!(*argv)[1] || (*argv)[1] == '-'))
	argv++;

    if (!*argv) {
    	zwarnnam(nam, "not enough arguments");
	return 1;
    }

    queue_signals();
    setcurjob();

    /* Remaining arguments specify processes.  Loop over them, and send the
    signal (number sig) to each process. */
    for (; *argv; argv++) {
	if (**argv == '%') {
	    /* job specifier introduced by '%' */
	    int p;

	    if ((p = getjob(*argv, nam)) == -1) {
		returnval++;
		continue;
	    }
	    if (killjb(jobtab + p, sig) == -1) {
		zwarnnam("kill", "kill %s failed: %e", *argv, errno);
		returnval++;
		continue;
	    }
	    /* automatically update the job table if sending a SIGCONT to a
	    job, and send the job a SIGCONT if sending it a non-stopping
	    signal. */
	    if (jobtab[p].stat & STAT_STOPPED) {
#ifndef WIFCONTINUED
		/* With WIFCONTINUED we find this out properly */
		if (sig == SIGCONT)
		    makerunning(jobtab + p);
#endif
		if (sig != SIGKILL && sig != SIGCONT && sig != SIGTSTP
		    && sig != SIGTTOU && sig != SIGTTIN && sig != SIGSTOP)
		    killjb(jobtab + p, SIGCONT);
	    }
	} else if (!isanum(*argv)) {
	    zwarnnam("kill", "illegal pid: %s", *argv);
	    returnval++;
	} else {
	    int pid = atoi(*argv);
	    if (kill(pid, sig) == -1) {
		zwarnnam("kill", "kill %s failed: %e", *argv, errno);
		returnval++;
	    } 
#ifndef WIFCONTINUED
	    else if (sig == SIGCONT) {
		Job jn;
		Process pn;
		/* With WIFCONTINUED we find this out properly */
		if (findproc(pid, &jn, &pn, 0)) {
		    if (WIFSTOPPED(pn->status))
			pn->status = SP_RUNNING;
		}
	    }
#endif
	}
    }
    unqueue_signals();

    return returnval < 126 ? returnval : 1;
}
/* Get a signal number from a string */

/**/
mod_export int
getsignum(const char *s)
{
    int x, i;

    /* check for a signal specified by number */
    x = atoi(s);
    if (idigit(*s) && x >= 0 && x < VSIGCOUNT)
	return x;

    /* search for signal by name */
    if (!strncmp(s, "SIG", 3))
	s += 3;

    for (i = 0; i < VSIGCOUNT; i++)
	if (!strcmp(s, sigs[i]))
	    return i;

    for (i = 0; alt_sigs[i].name; i++)
    {
	if (!strcmp(s, alt_sigs[i].name))
	    return alt_sigs[i].num;
    }

    /* no matching signal */
    return -1;
}

/* Get the name for a signal. */

/**/
mod_export const char *
getsigname(int sig)
{
    if (sigtrapped[sig] & ZSIG_ALIAS)
    {
	int i;
	for (i = 0; alt_sigs[i].name; i++)
	    if (sig == alt_sigs[i].num)
		return alt_sigs[i].name;
    }
    else
	return sigs[sig];

    /* shouldn't reach here */
#ifdef DEBUG
    dputs("Bad alias flag for signal");
#endif
    return "";
}


/* Get the function node for a trap, taking care about alternative names */
/**/
HashNode
gettrapnode(int sig, int ignoredisable)
{
    char fname[20];
    HashNode hn;
    HashNode (*getptr)(HashTable ht, const char *name);
    int i;
    if (ignoredisable)
	getptr = shfunctab->getnode2;
    else
	getptr = shfunctab->getnode;

    sprintf(fname, "TRAP%s", sigs[sig]);
    if ((hn = getptr(shfunctab, fname)))
	return hn;

    for (i = 0; alt_sigs[i].name; i++) {
	if (alt_sigs[i].num == sig) {
	    sprintf(fname, "TRAP%s", alt_sigs[i].name);
	    if ((hn = getptr(shfunctab, fname)))
		return hn;
	}
    }

    return NULL;
}

/* Remove a TRAP function under any name for the signal */

/**/
void
removetrapnode(int sig)
{
    HashNode hn = gettrapnode(sig, 1);
    if (hn) {
	shfunctab->removenode(shfunctab, hn->nam);
	shfunctab->freenode(hn);
    }
}

/* Suspend this shell */

/**/
int
bin_suspend(char *name, UNUSED(char **argv), Options ops, UNUSED(int func))
{
    /* won't suspend a login shell, unless forced */
    if (islogin && !OPT_ISSET(ops,'f')) {
	zwarnnam(name, "can't suspend login shell");
	return 1;
    }
    if (jobbing) {
	/* stop ignoring signals */
	signal_default(SIGTTIN);
	signal_default(SIGTSTP);
	signal_default(SIGTTOU);

	/* Move ourselves back to the process group we came from */
	release_pgrp();
    }

    /* suspend ourselves with a SIGTSTP */
    killpg(origpgrp, SIGTSTP);

    if (jobbing) {
	acquire_pgrp();
	/* restore signal handling */
	signal_ignore(SIGTTOU);
	signal_ignore(SIGTSTP);
	signal_ignore(SIGTTIN);
    }
    return 0;
}

/* find a job named s */

/**/
int
findjobnam(const char *s)
{
    int jobnum;

    for (jobnum = maxjob; jobnum >= 0; jobnum--)
	if (!(jobtab[jobnum].stat & (STAT_SUBJOB | STAT_NOPRINT)) &&
	    jobtab[jobnum].stat && jobtab[jobnum].procs && jobnum != thisjob &&
	    jobtab[jobnum].procs->text[0] && strpfx(s, jobtab[jobnum].procs->text))
	    return jobnum;
    return -1;
}


/* make sure we are a process group leader by creating a new process
   group if necessary */

/**/
void
acquire_pgrp(void)
{
    long ttpgrp;
    sigset_t blockset, oldset;

    if ((mypgrp = GETPGRP()) >= 0) {
	long lastpgrp = mypgrp;
	sigemptyset(&blockset);
	sigaddset(&blockset, SIGTTIN);
	sigaddset(&blockset, SIGTTOU);
	sigaddset(&blockset, SIGTSTP);
	oldset = signal_block(blockset);
	int loop_count = 0;
	while ((ttpgrp = gettygrp()) != -1 && ttpgrp != mypgrp) {
	    mypgrp = GETPGRP();
	    if (mypgrp == mypid) {
		if (!interact)
		    break; /* attachtty() will be a no-op, give up */
		signal_setmask(oldset);
		attachtty(mypgrp); /* Might generate SIGT* */
		signal_block(blockset);
	    }
	    if (mypgrp == gettygrp())
		break;
	    signal_setmask(oldset);
	    if (read(0, NULL, 0) != 0) {} /* Might generate SIGT* */
	    signal_block(blockset);
	    mypgrp = GETPGRP();
	    if (mypgrp == lastpgrp) {
		if (!interact)
		    break; /* Unlikely that pgrp will ever change */
		if (++loop_count == 100)
		{
		    /*
		     * It's time to give up.  The count is arbitrary;
		     * this is just to fix up unusual cases, so it's
		     * left large in an attempt not to break normal
		     * cases where there's some delay in the system
		     * setting up the terminal.
		     */
		    break;
		}
	    }
	    lastpgrp = mypgrp;
	}
	if (mypgrp != mypid) {
	    if (setpgrp(0, 0) == 0) {
		mypgrp = mypid;
		attachtty(mypgrp);
	    } else
		opts[MONITOR] = 0;
	}
	signal_setmask(oldset);
    } else
	opts[MONITOR] = 0;
}

/* revert back to the process group we came from (before acquire_pgrp) */

/**/
void
release_pgrp(void)
{
    if (origpgrp != mypgrp) {
	/* in linux pid namespaces, origpgrp may never have been set */
	if (origpgrp) {
	    attachtty(origpgrp);
	    setpgrp(0, origpgrp);
	}
	mypgrp = origpgrp;
    }
}
