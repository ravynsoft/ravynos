/* siglist.c -- signal list for those machines that don't have one. */

/* Copyright (C) 1989-2021 Free Software Foundation, Inc.

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

#include "config.h"

#if !defined (HAVE_SYS_SIGLIST) && !defined (HAVE_UNDER_SYS_SIGLIST) && !defined (HAVE_STRSIGNAL)

#include <stdio.h>
#include "command.h"
#include "general.h"
#include <signal.h>

#include "siglist.h"

#if !defined (NSIG)
#  include "trap.h"
#endif

#include "bashintl.h"

char *sys_siglist[NSIG];

void
initialize_siglist ()
{
  register int i;

  for (i = 0; i < NSIG; i++)
    sys_siglist[i] = (char *)0x0;

  sys_siglist[0] = _("Bogus signal");

#if defined (SIGHUP)
  sys_siglist[SIGHUP] = _("Hangup");
#endif

#if defined (SIGINT)
  sys_siglist[SIGINT] = _("Interrupt");
#endif

#if defined (SIGQUIT)
  sys_siglist[SIGQUIT] = _("Quit");
#endif

#if defined (SIGILL)
  sys_siglist[SIGILL] = _("Illegal instruction");
#endif

#if defined (SIGTRAP)
  sys_siglist[SIGTRAP] = _("BPT trace/trap");
#endif

#if defined (SIGIOT) && !defined (SIGABRT)
#define SIGABRT SIGIOT
#endif

#if defined (SIGABRT)
  sys_siglist[SIGABRT] = _("ABORT instruction");
#endif

#if defined (SIGEMT)
  sys_siglist[SIGEMT] = _("EMT instruction");
#endif

#if defined (SIGFPE)
  sys_siglist[SIGFPE] = _("Floating point exception");
#endif

#if defined (SIGKILL)
  sys_siglist[SIGKILL] = _("Killed");
#endif

#if defined (SIGBUS)
  sys_siglist[SIGBUS] = _("Bus error");
#endif

#if defined (SIGSEGV)
  sys_siglist[SIGSEGV] = _("Segmentation fault");
#endif

#if defined (SIGSYS)
  sys_siglist[SIGSYS] = _("Bad system call");
#endif

#if defined (SIGPIPE)
  sys_siglist[SIGPIPE] = _("Broken pipe");
#endif

#if defined (SIGALRM)
  sys_siglist[SIGALRM] = _("Alarm clock");
#endif

#if defined (SIGTERM)
  sys_siglist[SIGTERM] = _("Terminated");
#endif

#if defined (SIGURG)
  sys_siglist[SIGURG] = _("Urgent IO condition");
#endif

#if defined (SIGSTOP)
  sys_siglist[SIGSTOP] = _("Stopped (signal)");
#endif

#if defined (SIGTSTP)
  sys_siglist[SIGTSTP] = _("Stopped");
#endif

#if defined (SIGCONT)
  sys_siglist[SIGCONT] = _("Continue");
#endif

#if !defined (SIGCHLD) && defined (SIGCLD)
#define SIGCHLD SIGCLD
#endif

#if defined (SIGCHLD)
  sys_siglist[SIGCHLD] = _("Child death or stop");
#endif

#if defined (SIGTTIN)
  sys_siglist[SIGTTIN] = _("Stopped (tty input)");
#endif

#if defined (SIGTTOU)
  sys_siglist[SIGTTOU] = _("Stopped (tty output)");
#endif

#if defined (SIGIO)
  sys_siglist[SIGIO] = _("I/O ready");
#endif

#if defined (SIGXCPU)
  sys_siglist[SIGXCPU] = _("CPU limit");
#endif

#if defined (SIGXFSZ)
  sys_siglist[SIGXFSZ] = _("File limit");
#endif

#if defined (SIGVTALRM)
  sys_siglist[SIGVTALRM] = _("Alarm (virtual)");
#endif

#if defined (SIGPROF)
  sys_siglist[SIGPROF] = _("Alarm (profile)");
#endif

#if defined (SIGWINCH)
  sys_siglist[SIGWINCH] = _("Window changed");
#endif

#if defined (SIGLOST)
  sys_siglist[SIGLOST] = _("Record lock");
#endif

#if defined (SIGUSR1)
  sys_siglist[SIGUSR1] = _("User signal 1");
#endif

#if defined (SIGUSR2)
  sys_siglist[SIGUSR2] = _("User signal 2");
#endif

#if defined (SIGMSG)
  sys_siglist[SIGMSG] = _("HFT input data pending");
#endif

#if defined (SIGPWR)
  sys_siglist[SIGPWR] = _("power failure imminent");
#endif

#if defined (SIGDANGER)
  sys_siglist[SIGDANGER] = _("system crash imminent");
#endif

#if defined (SIGMIGRATE)
  sys_siglist[SIGMIGRATE] = _("migrate process to another CPU");
#endif

#if defined (SIGPRE)
  sys_siglist[SIGPRE] = _("programming error");
#endif

#if defined (SIGGRANT)
  sys_siglist[SIGGRANT] = _("HFT monitor mode granted");
#endif

#if defined (SIGRETRACT)
  sys_siglist[SIGRETRACT] = _("HFT monitor mode retracted");
#endif

#if defined (SIGSOUND)
  sys_siglist[SIGSOUND] = _("HFT sound sequence has completed");
#endif

#if defined (SIGINFO)
  sys_siglist[SIGINFO] = _("Information request");
#endif

  for (i = 0; i < NSIG; i++)
    {
      if (!sys_siglist[i])
	{
	  sys_siglist[i] =
	    (char *)xmalloc (INT_STRLEN_BOUND (int) + 1 + strlen (_("Unknown Signal #%d")));

	  sprintf (sys_siglist[i], _("Unknown Signal #%d"), i);
	}
    }
}
#endif /* !HAVE_SYS_SIGLIST && !HAVE_UNDER_SYS_SIGLIST && !HAVE_STRSIGNAL */
