/* Test of pthread_sigmask in a multi-threaded program.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2011.  */

#include <config.h>

#include <signal.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "macros.h"

#if USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS

static pthread_t main_thread;
static pthread_t killer_thread;

static void *
killer_thread_func (_GL_UNUSED void *arg)
{
  sleep (1);
  pthread_kill (main_thread, SIGINT);
  return NULL;
}

static volatile int sigint_occurred;

static void
sigint_handler (_GL_UNUSED int sig)
{
  sigint_occurred++;
}

int
main ()
{
  sigset_t set;

  signal (SIGINT, sigint_handler);

  sigemptyset (&set);
  sigaddset (&set, SIGINT);

  /* Check error handling.  */
  /* This call returns 0 on NetBSD 8.0.  */
#if !defined __NetBSD__
  ASSERT (pthread_sigmask (1729, &set, NULL) == EINVAL);
#endif

  /* Block SIGINT.  */
  ASSERT (pthread_sigmask (SIG_BLOCK, &set, NULL) == 0);

  /* Request a SIGINT signal from another thread.  */
  main_thread = pthread_self ();
  ASSERT (pthread_create (&killer_thread, NULL, killer_thread_func, NULL) == 0);

  /* Wait.  */
  sleep (2);

  /* The signal should not have arrived yet, because it is blocked.  */
  ASSERT (sigint_occurred == 0);

  /* Unblock SIGINT.  */
  ASSERT (pthread_sigmask (SIG_UNBLOCK, &set, NULL) == 0);

  /* The signal should have arrived now, because POSIX says
       "If there are any pending unblocked signals after the call to
        pthread_sigmask(), at least one of those signals shall be delivered
        before the call to pthread_sigmask() returns."  */
  ASSERT (sigint_occurred == 1);

  /* Clean up the thread.  This avoid a "ThreadSanitizer: thread leak" warning
     from "gcc -fsanitize=thread".  */
  ASSERT (pthread_join (killer_thread, NULL) == 0);

  return 0;
}

#else

int
main ()
{
  fputs ("Skipping test: POSIX threads not enabled\n", stderr);
  return 77;
}

#endif
