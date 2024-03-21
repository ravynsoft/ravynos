/* Test that gettext() does not crash by stack overflow when msgid is very long.
   Copyright (C) 2007, 2014, 2018 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <haible@clisp.cons.org>, 2007.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if HAVE_GETRLIMIT && HAVE_SETRLIMIT
# include <sys/types.h>
# include <sys/time.h>
# include <sys/resource.h>
#endif

#if USE_SYSTEM_LIBINTL
# include <libintl.h>
#else
/* Make sure we use the included libintl, not the system's one. */
# undef _LIBINTL_H
# include "libgnuintl.h"
#endif

int
main ()
{
  size_t n;
  char *msg;
  char *translated;

  n = 1000000;

#if HAVE_GETRLIMIT && HAVE_SETRLIMIT
  {
    struct rlimit limit;

# ifdef RLIMIT_STACK
    if (getrlimit (RLIMIT_STACK, &limit) < 0)
      {
        printf ("Skipping test: getrlimit does not work\n");
        return 77;
      }
    if (limit.rlim_max == RLIM_INFINITY || limit.rlim_max > n)
      limit.rlim_max = n;
    limit.rlim_cur = limit.rlim_max;
    if (setrlimit (RLIMIT_STACK, &limit) < 0)
      {
        printf ("Skipping test: setrlimit does not work\n");
        return 77;
      }
# endif
  }
#endif

  msg = (char *) malloc (n + 1);
  if (msg == NULL)
    {
      printf ("Skipping test: out of memory\n");
      return 77;
    }
  memset (msg, 'x', n);
  msg[n] = '\0';

  translated = gettext (msg);
  free (msg);
  assert (translated != NULL);

  return 0;
}
