/* Test of create_pipe_in/wait_subprocess.
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

#include "spawn-pipe.h"
#include "wait-process.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "macros.h"

int
main ()
{
  /* Check an invocation of an executable script.
     This should only be supported if the script has a '#!' marker; otherwise
     it is unsecure: <https://sourceware.org/bugzilla/show_bug.cgi?id=13134>.
     POSIX says that the execlp() and execvp() functions support executing
     shell scripts
     <https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html>,
     but this is considered an antiquated feature.  */
  int fd[1];
  pid_t pid;

  {
    size_t i;

    for (i = 0; i < 2; i++)
      {
        const char *progname =
          (i == 0 ? "executable-script" : "executable-script.sh");
        const char *prog_path =
          (i == 0 ? SRCDIR "executable-script" : SRCDIR "executable-script.sh");
        const char *prog_argv[2] = { prog_path, NULL };

        pid = create_pipe_in (progname, prog_argv[0], prog_argv, NULL,
                              NULL, false, true, false, fd);
        if (pid >= 0)
          {
            /* Wait for child.  */
            ASSERT (wait_subprocess (pid, progname, true, true, true, false,
                                     NULL)
                    == 127);
          }
        else
          {
            ASSERT (pid == -1);
            ASSERT (errno == ENOEXEC);
          }
      }
  }

#if defined _WIN32 && !defined __CYGWIN__
  /* On native Windows, scripts - even with '#!' marker - are not executable.
     Only .bat and .cmd files are.  */
  fprintf (stderr, "Skipping test: scripts are not executable on this platform.\n");
  return 77;
#else
  {
    const char *progname = "executable-shell-script";
    const char *prog_path = SRCDIR "executable-shell-script";
    const char *prog_argv[2] = { prog_path, NULL };

    pid = create_pipe_in (progname, prog_argv[0], prog_argv, NULL,
                          NULL, false, true, false, fd);
    ASSERT (pid >= 0);
    ASSERT (fd[0] > STDERR_FILENO);

    /* Get child's output.  */
    char buffer[1024];
    FILE *fp = fdopen (fd[0], "r");
    ASSERT (fp != NULL);
    ASSERT (fread (buffer, 1, sizeof (buffer), fp) == 11);

    /* Check the result.  */
    ASSERT (memcmp (buffer, "Halle Potta", 11) == 0);

    /* Wait for child.  */
    ASSERT (wait_subprocess (pid, progname, true, false, true, true, NULL) == 0);

    ASSERT (fclose (fp) == 0);
  }

  return 0;
#endif
}
