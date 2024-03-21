/* Test of pthread_create () function.
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

#include <pthread.h>

#include <stdio.h>
#include <string.h>

#include "macros.h"

static pthread_t main_thread_before;
static pthread_t main_thread_after;
static pthread_t worker_thread;

#define MAGIC ((void *) 1266074729)
static volatile int work_done;

static void *
worker_thread_func (_GL_UNUSED void *arg)
{
  work_done = 1;
  return MAGIC;
}

int
main ()
{
  main_thread_before = pthread_self ();

  if (pthread_create (&worker_thread, NULL, worker_thread_func, NULL) == 0)
    {
      void *ret;

      /* Check that pthread_self () has the same value before than after the
         first call to pthread_create ().  */
      main_thread_after = pthread_self ();
      ASSERT (memcmp (&main_thread_before, &main_thread_after,
                      sizeof (pthread_t))
              == 0);

      ASSERT (pthread_join (worker_thread, &ret) == 0);

      /* Check the return value of the thread.  */
      ASSERT (ret == MAGIC);

      /* Check that worker_thread_func () has finished executing.  */
      ASSERT (work_done);

      return 0;
    }
  else
    {
      fputs ("pthread_create failed\n", stderr);
      return 1;
    }
}
