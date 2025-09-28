/* Test program, used by the intl-thread-2 test.
   Copyright (C) 2005-2007, 2009-2010, 2013, 2018-2019 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <haible@clisp.cons.org>, 2005.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if USE_POSIX_THREADS && HAVE_WORKING_USELOCALE

#include <pthread.h>

#if USE_SYSTEM_LIBINTL
# include <libintl.h>
#else
/* Make sure we use the included libintl, not the system's one. */
# undef _LIBINTL_H
# include "libgnuintl.h"
#endif

/* Name of locale to use in thread1.  */
const char *locale_name_1;
/* Name of locale to use in thread2.  */
const char *locale_name_2;

/* Set to 1 if the program is not behaving correctly.  */
int result;

/* Denotes which thread should run next.  */
int flipflop;
/* Lock and wait queue used to switch between the threads.  */
pthread_mutex_t lock;
pthread_cond_t waitqueue;

/* Waits until the flipflop has a given value.
   Before the call, the lock is unlocked.  After the call, it is locked.  */
static void
waitfor (int value)
{
  if (pthread_mutex_lock (&lock))
    exit (10);
  while (flipflop != value)
    if (pthread_cond_wait (&waitqueue, &lock))
      exit (11);
}

/* Sets the flipflop to a given value.
   Before the call, the lock is locked.  After the call, it is unlocked.  */
static void
setto (int value)
{
  flipflop = value;
  if (pthread_cond_signal (&waitqueue))
    exit (20);
  if (pthread_mutex_unlock (&lock))
    exit (21);
}

void *
thread1_execution (void *arg)
{
  char *s;

  waitfor (1);
  uselocale (newlocale (LC_ALL_MASK, locale_name_1, NULL));
  setto (2);

  waitfor (1);
  s = gettext ("beauty");
  puts (s);
  if (strcmp (s, "beaut\303\251"))
    {
      fprintf (stderr, "thread 1 call 1 returned: %s\n", s);
      result = 1;
    }
  setto (2);

  waitfor (1);
  s = gettext ("beauty");
  puts (s);
  if (strcmp (s, "beaut\303\251"))
    {
      fprintf (stderr, "thread 1 call 2 returned: %s\n", s);
      result = 1;
    }
  setto (2);

  return NULL;
}

void *
thread2_execution (void *arg)
{
  char *s;

  waitfor (2);
  uselocale (newlocale (LC_ALL_MASK, locale_name_2, NULL));
  setto (1);

  waitfor (2);
  s = gettext ("beauty");
  puts (s);
  if (strcmp (s, "Sch\303\266nheit"))
    {
      fprintf (stderr, "thread 2 call 1 returned: %s\n", s);
      result = 1;
    }
  setto (1);

  waitfor (2);
  s = gettext ("beauty");
  puts (s);
  if (strcmp (s, "Sch\303\266nheit"))
    {
      fprintf (stderr, "thread 2 call 2 returned: %s\n", s);
      result = 1;
    }
  setto (1);

  return NULL;
}

int
main (int argc, char *argv[])
{
  pthread_t thread1;
  pthread_t thread2;

  locale_name_1 = argv[1];
  locale_name_2 = argv[2];

  unsetenv ("LANGUAGE");
  unsetenv ("OUTPUT_CHARSET");
  textdomain ("tstthread");
  bindtextdomain ("tstthread", "in-th-2");
  result = 0;

  flipflop = 1;
  if (pthread_mutex_init (&lock, NULL))
    exit (2);
  if (pthread_cond_init (&waitqueue, NULL))
    exit (2);
  if (pthread_create (&thread1, NULL, &thread1_execution, NULL))
    exit (2);
  if (pthread_create (&thread2, NULL, &thread2_execution, NULL))
    exit (2);
  if (pthread_join (thread2, NULL))
    exit (3);

  return result;
}

#else

/* This test is not executed.  */

int
main (void)
{
  return 77;
}

#endif
