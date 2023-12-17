/* Test of <signal.h> substitute.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Eric Blake <ebb9@byu.net>, 2009.  */

#include <config.h>

#include <signal.h>

/* Check for required types.  */
struct
{
  size_t a;
  uid_t b;
  volatile sig_atomic_t c;
  sigset_t d;
  pid_t e;
#if 0
  /* Not guaranteed by gnulib.  */
  pthread_t f;
  struct timespec g;
#endif
} s;

/* Check that NSIG is defined.  */
int nsig = NSIG;

int
main (void)
{
  switch (0)
    {
      /* The following are guaranteed by C.  */
    case 0:
    case SIGABRT:
    case SIGFPE:
    case SIGILL:
    case SIGINT:
    case SIGSEGV:
    case SIGTERM:
      /* The following is guaranteed by gnulib.  */
#if GNULIB_SIGPIPE || defined SIGPIPE
    case SIGPIPE:
#endif
      /* Ensure no conflict with other standardized names.  */
#ifdef SIGALRM
    case SIGALRM:
#endif
      /* On Haiku, SIGBUS is mistakenly equal to SIGSEGV.  */
#if defined SIGBUS && SIGBUS != SIGSEGV
    case SIGBUS:
#endif
#ifdef SIGCHLD
    case SIGCHLD:
#endif
#ifdef SIGCONT
    case SIGCONT:
#endif
#ifdef SIGHUP
    case SIGHUP:
#endif
#ifdef SIGKILL
    case SIGKILL:
#endif
#ifdef SIGQUIT
    case SIGQUIT:
#endif
#ifdef SIGSTOP
    case SIGSTOP:
#endif
#ifdef SIGTSTP
    case SIGTSTP:
#endif
#ifdef SIGTTIN
    case SIGTTIN:
#endif
#ifdef SIGTTOU
    case SIGTTOU:
#endif
#ifdef SIGUSR1
    case SIGUSR1:
#endif
#ifdef SIGUSR2
    case SIGUSR2:
#endif
#ifdef SIGSYS
    case SIGSYS:
#endif
#ifdef SIGTRAP
    case SIGTRAP:
#endif
#ifdef SIGURG
    case SIGURG:
#endif
#ifdef SIGVTALRM
    case SIGVTALRM:
#endif
#ifdef SIGXCPU
    case SIGXCPU:
#endif
#ifdef SIGXFSZ
    case SIGXFSZ:
#endif
      /* SIGRTMIN and SIGRTMAX need not be compile-time constants.  */
#if 0
# ifdef SIGRTMIN
    case SIGRTMIN:
# endif
# ifdef SIGRTMAX
    case SIGRTMAX:
# endif
#endif
      ;
    }
  return s.a + s.b + s.c + s.e;
}
