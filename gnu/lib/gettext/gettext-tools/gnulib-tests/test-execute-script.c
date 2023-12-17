/* Test of execute.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include "execute.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "read-file.h"
#include "macros.h"

#define DATA_FILENAME "test-execute-script.tmp"

int
main ()
{
  unlink (DATA_FILENAME);

  /* Check an invocation of an executable script.
     This should only be supported if the script has a '#!' marker; otherwise
     it is unsecure: <https://sourceware.org/bugzilla/show_bug.cgi?id=13134>.
     POSIX says that the execlp() and execvp() functions support executing
     shell scripts
     <https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html>,
     but this is considered an antiquated feature.  */

  /* This test is an extension of
     "Check stdout is inherited, part 1 (regular file)"
     in test-execute-main.c.  */
  FILE *fp = freopen (DATA_FILENAME, "w", stdout);
  ASSERT (fp != NULL);

  {
    size_t i;

    for (i = 0; i < 2; i++)
      {
        const char *progname =
          (i == 0 ? "executable-script" : "executable-script.sh");
        const char *prog_path =
          (i == 0 ? SRCDIR "executable-script" : SRCDIR "executable-script.sh");
        const char *prog_argv[2] = { prog_path, NULL };

        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 127);
      }
  }

#if defined _WIN32 && !defined __CYGWIN__
  /* On native Windows, scripts - even with '#!' marker - are not executable.
     Only .bat and .cmd files are.  */
  ASSERT (fclose (fp) == 0);
  ASSERT (unlink (DATA_FILENAME) == 0);
  fprintf (stderr, "Skipping test: scripts are not executable on this platform.\n");
  return 77;
#else
  {
    const char *progname = "executable-shell-script";
    const char *prog_path = SRCDIR "executable-shell-script";
    const char *prog_argv[2] = { prog_path, NULL };

    int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                       false, false, false, false, true, false, NULL);
    ASSERT (ret == 0);

    ASSERT (fclose (fp) == 0);

    size_t length;
    char *contents = read_file (DATA_FILENAME, 0, &length);
    ASSERT (length == 11 && memcmp (contents, "Halle Potta", 11) == 0);
  }

  ASSERT (unlink (DATA_FILENAME) == 0);

  return 0;
#endif
}
