/* Test of posix_spawn() function with 'open' action and O_APPEND flag.
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

/* Written by Bruno Haible <bruno@clisp.org>, 2020.  */

/* Test whether posix_spawn_file_actions_addopen supports the O_APPEND flag.  */

#include <config.h>

#include <spawn.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHILD_PROGRAM_FILENAME "test-posix_spawn-open2" EXEEXT
#define DATA_FILENAME "test-posix_spawn-open2-data.tmp"

static int
parent_main (void)
{
  FILE *fp;
  char *argv[3] = { CHILD_PROGRAM_FILENAME, "-child", NULL };
  posix_spawn_file_actions_t actions;
  bool actions_allocated;
  int err;
  pid_t child;
  int status;
  int exitstatus;

  /* Create a data file with specific contents.  */
  fp = fopen (DATA_FILENAME, "wb");
  if (fp == NULL)
    {
      perror ("cannot create data file");
      return 1;
    }
  fwrite ("Halle ", 1, 6, fp);
  if (fflush (fp) || fclose (fp))
    {
      perror ("cannot prepare data file");
      return 1;
    }

  /* Test whether posix_spawn_file_actions_addopen with O_APPEND flag causes
     the child to append to this file.  */
  actions_allocated = false;
  if ((err = posix_spawn_file_actions_init (&actions)) != 0
      || (actions_allocated = true,
          (err = posix_spawn_file_actions_addopen (&actions, STDOUT_FILENO, DATA_FILENAME, O_RDWR | O_APPEND, 0600)) != 0
          || (err = posix_spawn (&child, CHILD_PROGRAM_FILENAME, &actions, NULL, argv, environ)) != 0))
    {
      if (actions_allocated)
        posix_spawn_file_actions_destroy (&actions);
      errno = err;
      perror ("subprocess failed");
      return 1;
    }
  posix_spawn_file_actions_destroy (&actions);
  status = 0;
  while (waitpid (child, &status, 0) != child)
    ;
  if (!WIFEXITED (status))
    {
      fprintf (stderr, "subprocess terminated with unexpected wait status %d\n", status);
      return 1;
    }
  exitstatus = WEXITSTATUS (status);
  if (exitstatus != 0)
    {
      fprintf (stderr, "subprocess terminated with unexpected exit status %d\n", exitstatus);
      return 1;
    }

  /* Check the contents of the data file.  */
  fp = fopen (DATA_FILENAME, "rb");
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

  /* Clean up data file.  */
  unlink (DATA_FILENAME);

  return 0;
}

static int
child_main (void)
{
  /* Write to STDOUT_FILENO.  */
  fwrite ("Potta", 1, 5, stdout);
  /* No 'fflush (stdout);' is needed.  It is implicit when the child process
     exits.  */

  return 0;
}

static void
cleanup_then_die (int sig)
{
  /* Clean up data file.  */
  unlink (DATA_FILENAME);

  /* Re-raise the signal and die from it.  */
  signal (sig, SIG_DFL);
  raise (sig);
}

int
main (int argc, char *argv[])
{
  int exitstatus;

  if (!(argc > 1 && strcmp (argv[1], "-child") == 0))
    {
      /* This is the parent process.  */
      signal (SIGINT, cleanup_then_die);
      signal (SIGTERM, cleanup_then_die);
      #ifdef SIGHUP
      signal (SIGHUP, cleanup_then_die);
      #endif

      exitstatus = parent_main ();
    }
  else
    {
      /* This is the child process.  */
      exitstatus = child_main ();
    }
  return exitstatus;
}
