/* Test program, used by the intl-thread-1 test.
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

/* Written by Bruno Haible <haible@clisp.cons.org>, 2005, 2018.  */

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

/* Set to 1 if the program is not behaving correctly.  */
int result;

void *
thread1_execution (void *arg)
{
  char *s;

  uselocale (newlocale (LC_ALL_MASK, locale_name_1, NULL));

  /* Here we expect translated output.  */

  s = gettext ("cheese");
  puts (s);
  if (strcmp (s, "fromage"))
    {
      fprintf (stderr, "thread 1 call 1 returned: %s\n", s);
      result = 1;
    }

  return NULL;
}

int
main (int argc, char *argv[])
{
  pthread_t thread1;

  locale_name_1 = argv[1];

  unsetenv ("LANGUAGE");
  unsetenv ("OUTPUT_CHARSET");
  textdomain ("tstthread");
  bindtextdomain ("tstthread", "in-th-1");
  result = 0;

  if (pthread_create (&thread1, NULL, &thread1_execution, NULL))
    exit (2);
  if (pthread_join (thread1, NULL))
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
