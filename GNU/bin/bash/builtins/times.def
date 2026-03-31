This file is times.def, from which is created times.c.
It implements the builtin "times" in Bash.

Copyright (C) 1987-2009 Free Software Foundation, Inc.

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

$PRODUCES times.c

$BUILTIN times
$FUNCTION times_builtin
$SHORT_DOC times
Display process times.

Prints the accumulated user and system times for the shell and all of its
child processes.

Exit Status:
Always succeeds.
$END

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include <stdio.h>
#include "../bashtypes.h"
#include "../shell.h"

#include <posixtime.h>

#if defined (HAVE_SYS_TIMES_H)
#  include <sys/times.h>
#endif /* HAVE_SYS_TIMES_H */

#if defined (HAVE_SYS_RESOURCE_H) && !defined (RLIMTYPE)
#  include <sys/resource.h>
#endif

#include "common.h"

/* Print the totals for system and user time used. */
int
times_builtin (list)
     WORD_LIST *list;
{
#if defined (HAVE_GETRUSAGE) && defined (HAVE_TIMEVAL) && defined (RUSAGE_SELF)
  struct rusage self, kids;

  USE_VAR(list);

  if (no_options (list))
    return (EX_USAGE);

  getrusage (RUSAGE_SELF, &self);
  getrusage (RUSAGE_CHILDREN, &kids);	/* terminated child processes */

  print_timeval (stdout, &self.ru_utime);
  putchar (' ');
  print_timeval (stdout, &self.ru_stime);
  putchar ('\n');
  print_timeval (stdout, &kids.ru_utime);
  putchar (' ');
  print_timeval (stdout, &kids.ru_stime);
  putchar ('\n');

#else
#  if defined (HAVE_TIMES)
  /* This uses the POSIX.1/XPG5 times(2) interface, which fills in a 
     `struct tms' with values of type clock_t. */
  struct tms t;

  USE_VAR(list);

  if (no_options (list))
    return (EX_USAGE);

  times (&t);

  print_clock_t (stdout, t.tms_utime);
  putchar (' ');
  print_clock_t (stdout, t.tms_stime);
  putchar ('\n');
  print_clock_t (stdout, t.tms_cutime);
  putchar (' ');
  print_clock_t (stdout, t.tms_cstime);
  putchar ('\n');

#  else /* !HAVE_TIMES */

  USE_VAR(list);

  if (no_options (list))
    return (EX_USAGE);
  printf ("0.00 0.00\n0.00 0.00\n");

#  endif /* HAVE_TIMES */
#endif /* !HAVE_TIMES */

  return (sh_chkwrite (EXECUTION_SUCCESS));
}
