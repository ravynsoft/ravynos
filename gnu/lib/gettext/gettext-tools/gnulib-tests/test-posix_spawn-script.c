/* Test of posix_spawn() function.
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

#include <spawn.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "macros.h"

#define DATA_FILENAME "test-posix_spawn-script.tmp"

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
  pid_t child;

  posix_spawn_file_actions_t actions;
  ASSERT (posix_spawn_file_actions_init (&actions) == 0);
  ASSERT (posix_spawn_file_actions_addopen (&actions, STDOUT_FILENO,
                                            DATA_FILENAME,
                                            O_RDWR | O_CREAT | O_TRUNC, 0600)
          == 0);

  {
    size_t i;

    for (i = 0; i < 2; i++)
      {
        const char *prog_path =
          (i == 0 ? SRCDIR "executable-script" : SRCDIR "executable-script.sh");
        const char *prog_argv[2] = { prog_path, NULL };

        int err = posix_spawn (&child, prog_path, &actions, NULL,
                               (char **) prog_argv, environ);
        if (err != ENOEXEC)
          {
            if (err != 0)
              {
                errno = err;
                perror ("posix_spawn");
                return 1;
              }

            /* Wait for child.  */
            int status = 0;
            while (waitpid (child, &status, 0) != child)
              ;
            if (!WIFEXITED (status))
              {
                fprintf (stderr, "subprocess terminated with unexpected wait status %d\n", status);
                return 1;
              }
            int exitstatus = WEXITSTATUS (status);
            if (exitstatus != 127)
              {
                fprintf (stderr, "subprocess terminated with unexpected exit status %d\n", exitstatus);
                return 1;
              }
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
    const char *prog_path = SRCDIR "executable-shell-script";
    const char *prog_argv[2] = { prog_path, NULL };

    int err = posix_spawn (&child, prog_path, &actions, NULL,
                           (char **) prog_argv, environ);
    if (err != 0)
      {
        errno = err;
        perror ("posix_spawn");
        return 1;
      }

    posix_spawn_file_actions_destroy (&actions);

    /* Wait for child.  */
    int status = 0;
    while (waitpid (child, &status, 0) != child)
      ;
    if (!WIFEXITED (status))
      {
        fprintf (stderr, "subprocess terminated with unexpected wait status %d\n", status);
        return 1;
      }
    int exitstatus = WEXITSTATUS (status);
    if (exitstatus != 0)
      {
        fprintf (stderr, "subprocess terminated with unexpected exit status %d\n", exitstatus);
        return 1;
      }

    /* Check the contents of the data file.  */
    FILE *fp = fopen (DATA_FILENAME, "rb");
    if (fp == NULL)
      {
        perror ("cannot open data file");
        return 1;
      }
    char buf[1024];
    int nread = fread (buf, 1, sizeof (buf), fp);
    if (!(nread == 11 && memcmp (buf, "Halle Potta", 11) == 0))
      {
        fprintf (stderr, "data file wrong: has %d bytes, expected %d bytes\n", nread, 11);
        return 1;
      }
    if (fclose (fp))
      {
        perror ("cannot close data file");
        return 1;
      }
  }
#endif

  /* Clean up data file.  */
  unlink (DATA_FILENAME);

  return 0;
}
