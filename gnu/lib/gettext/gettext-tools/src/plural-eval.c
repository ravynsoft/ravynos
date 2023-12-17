/* Expression evaluation for plural form selection.
   Copyright (C) 2000-2003, 2005, 2019-2020 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@cygnus.com>, 2000.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "plural-eval.h"

#include <stddef.h>
#include <signal.h>

#include "plural-exp.h"


#define STATIC /*extern*/

/* Include the expression evaluation code from libintl, this time with
   'extern' linkage.  */
#include "eval-plural.h"


/* Exit point.  Must be set before calling install_sigfpe_handler().  */
sigjmp_buf sigfpe_exit;

#if USE_SIGINFO

/* Additional information that is set before sigfpe_exit is invoked.  */
int volatile sigfpe_code;

/* Signal handler called in case of arithmetic exception (e.g. division
   by zero) during plural_eval.  */
static _GL_ASYNC_SAFE void
sigfpe_handler (int sig, siginfo_t *sip, void *scp)
{
  sigfpe_code = sip->si_code;
  /* This handler is invoked on the thread that caused the SIGFPE, that is,
     the thread that is doing plural evaluation.  Therefore it's OK to use
     siglongjmp.  */
  siglongjmp (sigfpe_exit, 1);
}

#else

/* Signal handler called in case of arithmetic exception (e.g. division
   by zero) during plural_eval.  */
static _GL_ASYNC_SAFE void
sigfpe_handler (int sig)
{
  /* This handler is invoked on the thread that caused the SIGFPE, that is,
     the thread that is doing plural evaluation.  Therefore it's OK to use
     siglongjmp.  */
  siglongjmp (sigfpe_exit, 1);
}

#endif

void
install_sigfpe_handler (void)
{
#if USE_SIGINFO
  struct sigaction action;
  action.sa_sigaction = sigfpe_handler;
  action.sa_flags = SA_SIGINFO;
  sigemptyset (&action.sa_mask);
  sigaction (SIGFPE, &action, (struct sigaction *) NULL);
#else
  signal (SIGFPE, sigfpe_handler);
#endif
}

void
uninstall_sigfpe_handler (void)
{
#if USE_SIGINFO
  struct sigaction action;
  action.sa_handler = SIG_DFL;
  action.sa_flags = 0;
  sigemptyset (&action.sa_mask);
  sigaction (SIGFPE, &action, (struct sigaction *) NULL);
#else
  signal (SIGFPE, SIG_DFL);
#endif
}
