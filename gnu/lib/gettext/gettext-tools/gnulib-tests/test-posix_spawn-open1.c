/* Test of posix_spawn() function with 'open' action.
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

/* Test whether posix_spawn_file_actions_addopen supports filename arguments
   that contain special characters such as '*'.  */

#include <config.h>

#include <spawn.h>

#include "signature.h"
SIGNATURE_CHECK (posix_spawn, int, (pid_t *, char const *,
                                    posix_spawn_file_actions_t const *,
                                    posix_spawnattr_t const *,
                                    char *const[], char *const[]));
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHILD_PROGRAM_FILENAME "test-posix_spawn-open1" EXEEXT
#define DATA_FILENAME "t!#$%&'()*+,-;=?@[\\]^_`{|}~.tmp"
/* On Windows (including Cygwin), '*' '?' '\\' '|' cannot be used in file
   names.  */
#if defined _WIN32 || defined __CYGWIN__
# undef DATA_FILENAME
# define DATA_FILENAME "t!#$%&'()+,-;=@[]^_`{}~.tmp"
#endif

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
  fwrite ("Halle Potta", 1, 11, fp);
  if (fflush (fp) || fclose (fp))
    {
      perror ("cannot prepare data file");
      return 1;
    }

  /* Avoid reading from our stdin, as it could block.  */
  if (freopen ("/dev/null", "rb", stdin) == NULL)
    {
      perror ("cannot redirect stdin");
      return 1;
    }

  /* Test whether posix_spawn_file_actions_addopen with this file name
     actually works, by spawning a child that reads from this file.  */
  actions_allocated = false;
  if ((err = posix_spawn_file_actions_init (&actions)) != 0
      || (actions_allocated = true,
          (err = posix_spawn_file_actions_addopen (&actions, STDIN_FILENO, DATA_FILENAME, O_RDONLY, 0600)) != 0
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
  return 0;
}

static int
child_main (void)
{
  char buf[1024];

  /* See if reading from STDIN_FILENO yields the expected contents.  */
  if (fread (buf, 1, sizeof (buf), stdin) == 11
      && memcmp (buf, "Halle Potta", 11) == 0)
    return 0;
  else
    return 2;
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
  unlink (DATA_FILENAME);
  return exitstatus;
}
