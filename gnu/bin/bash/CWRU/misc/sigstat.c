/*
 * sigstat - print out useful information about signal arguments
 *
 * Chet Ramey
 * chet@po.cwru.edu
 */

/* Copyright (C) 1991-2009 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

extern char	*strrchr();
static char	*signames[NSIG];

char	*progname;

void	sigstat();

main(argc, argv)
int	argc;
char	**argv;
{
	register int	i;
	char	*t;

	if (t = strrchr(argv[0], '/'))
		progname = ++t;
	else
		progname = argv[0];
	init_signames();
	if (argc == 1) {
		for (i = 1; i < NSIG; i++)
			sigstat(i);
		exit(0);
	}
	for (i = 1; i < argc; i++)
		sigstat(atoi(argv[i]));
	exit(0);
}

void
sigstat(sig)
int	sig;
{
	struct sigaction oact;
	char *signame;
	sigset_t set, oset;
	int blocked;

	if (sig < 0 || sig >= NSIG) {
		fprintf(stderr, "%s: %d: signal out of range\n", progname, sig);
		return;
	}
	signame = signames[sig];
	sigemptyset(&oset);
	sigprocmask(SIG_BLOCK, (sigset_t *)NULL, &oset);
	if (sigismember(&oset, sig))
		printf("%s: signal is blocked\n", signame);
	sigaction(sig, (struct sigaction *)NULL, &oact);
	if (oact.sa_handler == SIG_IGN)
		printf("%s: signal is ignored\n", signame);
	else if (oact.sa_handler == SIG_DFL)
		printf("%s: signal is defaulted\n", signame);
	else
		printf("%s: signal is trapped (?)\n", signame);
}

init_signames()
{
	register int i;
	bzero(signames, sizeof(signames));

#if defined (SIGHUP)		/* hangup */
  	signames[SIGHUP] = "SIGHUP";
#endif
#if defined (SIGINT)		/* interrupt */
  	signames[SIGINT] = "SIGINT";
#endif
#if defined (SIGQUIT)		/* quit */
  	signames[SIGQUIT] = "SIGQUIT";
#endif
#if defined (SIGILL)		/* illegal instruction (not reset when caught) */
  	signames[SIGILL] = "SIGILL";
#endif
#if defined (SIGTRAP)		/* trace trap (not reset when caught) */
  	signames[SIGTRAP] = "SIGTRAP";
#endif
#if defined (SIGABRT)		/*  */
  	signames[SIGABRT] = "SIGABRT";
#endif
#if defined (SIGIOT)		/* IOT instruction */
  	signames[SIGIOT] = "SIGIOT";
#endif
#if defined (SIGEMT)		/* EMT instruction */
  	signames[SIGEMT] = "SIGEMT";
#endif
#if defined (SIGFPE)		/* floating point exception */
  	signames[SIGFPE] = "SIGFPE";
#endif
#if defined (SIGKILL)		/* kill (cannot be caught or ignored) */
  	signames[SIGKILL] = "SIGKILL";
#endif
#if defined (SIGBUS)		/* bus error */
  	signames[SIGBUS] = "SIGBUS";
#endif
#if defined (SIGSEGV)		/* segmentation violation */
  	signames[SIGSEGV] = "SIGSEGV";
#endif
#if defined (SIGSYS)		/* bad argument to system call */
  	signames[SIGSYS] = "SIGSYS";
#endif
#if defined (SIGPIPE)		/* write on a pipe with no one to read it */
  	signames[SIGPIPE] = "SIGPIPE";
#endif
#if defined (SIGALRM)		/* alarm clock */
  	signames[SIGALRM] = "SIGALRM";
#endif
#if defined (SIGTERM)		/* software termination signal from kill */
  	signames[SIGTERM] = "SIGTERM";
#endif
#if defined (SIGCLD)		/* Like SIGCHLD.  */
  	signames[SIGCLD] = "SIGCLD";
#endif
#if defined (SIGPWR)		/* Magic thing for some machines. */
  	signames[SIGPWR] = "SIGPWR";
#endif
#if defined (SIGPOLL)		/* For keyboard input?  */
  	signames[SIGPOLL] = "SIGPOLL";
#endif
#if defined (SIGURG)		/* urgent condition on IO channel */
  	signames[SIGURG] = "SIGURG";
#endif
#if defined (SIGSTOP)		/* sendable stop signal not from tty */
  	signames[SIGSTOP] = "SIGSTOP";
#endif
#if defined (SIGTSTP)		/* stop signal from tty */
  	signames[SIGTSTP] = "SIGTSTP";
#endif
#if defined (SIGCONT)		/* continue a stopped process */
  	signames[SIGCONT] = "SIGCONT";
#endif
#if defined (SIGCHLD)		/* to parent on child stop or exit */
  	signames[SIGCHLD] = "SIGCHLD";
#endif
#if defined (SIGTTIN)		/* to readers pgrp upon background tty read */
  	signames[SIGTTIN] = "SIGTTIN";
#endif
#if defined (SIGTTOU)		/* like TTIN for output if (tp->t_local&LTOSTOP) */
  	signames[SIGTTOU] = "SIGTTOU";
#endif
#if defined (SIGIO)		/* input/output possible signal */
  	signames[SIGIO] = "SIGIO";
#endif
#if defined (SIGXCPU)		/* exceeded CPU time limit */
  	signames[SIGXCPU] = "SIGXCPU";
#endif
#if defined (SIGXFSZ)		/* exceeded file size limit */
  	signames[SIGXFSZ] = "SIGXFSZ";
#endif
#if defined (SIGVTALRM)		/* virtual time alarm */
  	signames[SIGVTALRM] = "SIGVTALRM";
#endif
#if defined (SIGPROF)		/* profiling time alarm */
  	signames[SIGPROF] = "SIGPROF";
#endif
#if defined (SIGWINCH)		/* window changed */
  	signames[SIGWINCH] = "SIGWINCH";
#endif
#if defined (SIGLOST)		/* resource lost (eg, record-lock lost) */
  	signames[SIGLOST] = "SIGLOST";
#endif
#if defined (SIGUSR1)		/* user defined signal 1 */
  	signames[SIGUSR1] = "SIGUSR1";
#endif
#if defined (SIGUSR2)		/* user defined signal 2 */
  	signames[SIGUSR2] = "SIGUSR2";
#endif
#if defined (SIGMSG)	/* HFT input data pending */
  	signames[SIGMSG] = "SIGMSG";
#endif
#if defined (SIGPWR)	/* power failure imminent (save your data) */
  	signames[SIGPWR] = "SIGPWR";
#endif
#if defined (SIGDANGER)	/* system crash imminent */
  	signames[SIGDANGER] = "SIGDANGER";
#endif
#if defined (SIGMIGRATE)	/* migrate process to another CPU */
  	signames[SIGMIGRATE] = "SIGMIGRATE";
#endif
#if defined (SIGPRE)	/* programming error */
  	signames[SIGPRE] = "SIGPRE";
#endif
#if defined (SIGGRANT)	/* HFT monitor mode granted */
  	signames[SIGGRANT] = "SIGGRANT";
#endif
#if defined (SIGRETRACT)	/* HFT monitor mode retracted */
  	signames[SIGRETRACT] = "SIGRETRACT";
#endif
#if defined (SIGSOUND)	/* HFT sound sequence has completed */
  	signames[SIGSOUND] = "SIGSOUND";
#endif

	for (i = 0; i < NSIG; i++)
		if (signames[i] == (char *)NULL) {
			signames[i] = (char *)malloc (16);;
			sprintf (signames[i], "signal %d", i);
	  	}
}
