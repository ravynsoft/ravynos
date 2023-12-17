/* Test of gl_thread_create () macro.
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

#include "glthread/thread.h"

#include <stdio.h>
#include <string.h>

#include "macros.h"

static gl_thread_t main_thread_before;
static gl_thread_t main_thread_after;
static gl_thread_t worker_thread;

static int dummy;
static volatile int work_done;

static void *
worker_thread_func (_GL_UNUSED void *arg)
{
  work_done = 1;
  return &dummy;
}

int
main ()
{
  main_thread_before = gl_thread_self ();

  if (glthread_create (&worker_thread, worker_thread_func, NULL) == 0)
    {
      void *ret;

      /* Check that gl_thread_self () has the same value before than after the
         first call to gl_thread_create ().  */
      main_thread_after = gl_thread_self ();
      ASSERT (memcmp (&main_thread_before, &main_thread_after,
                      sizeof (gl_thread_t))
              == 0);

      gl_thread_join (worker_thread, &ret);

      /* Check the return value of the thread.  */
      ASSERT (ret == &dummy);

      /* Check that worker_thread_func () has finished executing.  */
      ASSERT (work_done);

      return 0;
    }
  else
    {
#if USE_ISOC_THREADS || USE_POSIX_THREADS || USE_ISOC_AND_POSIX_THREADS || USE_WINDOWS_THREADS
      fputs ("glthread_create failed\n", stderr);
      return 1;
#else
      fputs ("Skipping test: multithreading not enabled\n", stderr);
      return 77;
#endif
    }
}
