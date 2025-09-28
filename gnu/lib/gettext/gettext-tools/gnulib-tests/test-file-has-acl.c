/* Test for presence of ACL.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include "acl.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  const char *file;
  struct stat statbuf;

  ASSERT (argc == 2);

  file = argv[1];

  if (stat (file, &statbuf) < 0)
    {
      fprintf (stderr, "could not access file \"%s\"\n", file);
      exit (EXIT_FAILURE);
    }

  /* Check against possible infinite loop in file_has_acl.  */
#if HAVE_DECL_ALARM
  /* Declare failure if test takes too long, by using default abort
     caused by SIGALRM.  */
  {
    int alarm_value = 5;
    signal (SIGALRM, SIG_DFL);
    alarm (alarm_value);
  }
#endif

#if USE_ACL
  {
    int ret = file_has_acl (file, &statbuf);
    if (ret < 0)
      {
        fprintf (stderr, "could not access the ACL of file \"%s\"\n", file);
        exit (EXIT_FAILURE);
      }
    printf ("%s\n", ret ? "yes" : "no");
  }
#else
  printf ("no\n");
#endif

  return 0;
}
