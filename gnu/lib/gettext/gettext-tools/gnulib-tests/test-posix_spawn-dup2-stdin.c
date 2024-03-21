/* Test of posix_spawn() function: writing to a subprocess.
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

#include <spawn.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHILD_PROGRAM_FILENAME "test-posix_spawn-dup2-stdin.sh"

static int
fd_safer (int fd)
{
  if (0 <= fd && fd <= 2)
    {
      int f = fd_safer (dup (fd));
      int e = errno;
      close (fd);
      errno = e;
      fd = f;
    }

  return fd;
}

int
main ()
{
  char *argv[3] = { (char *) BOURNE_SHELL, (char *) CHILD_PROGRAM_FILENAME, NULL };
  int ofd[2];
  sigset_t blocked_signals;
  sigset_t fatal_signal_set;
  posix_spawn_file_actions_t actions;
  bool actions_allocated;
  posix_spawnattr_t attrs;
  bool attrs_allocated;
  int err;
  pid_t child;
  int fd;
  FILE *fp;
  int written;
  int status;
  int exitstatus;

  if (pipe (ofd) < 0 || (ofd[1] = fd_safer (ofd[1])) < 0)
    {
      perror ("cannot create pipe");
      exit (1);
    }
  sigprocmask (SIG_SETMASK, NULL, &blocked_signals);
  sigemptyset (&fatal_signal_set);
  sigaddset (&fatal_signal_set, SIGINT);
  sigaddset (&fatal_signal_set, SIGTERM);
  #ifdef SIGHUP
  sigaddset (&fatal_signal_set, SIGHUP);
  #endif
  #ifdef SIGPIPE
  sigaddset (&fatal_signal_set, SIGPIPE);
  #endif
  sigprocmask (SIG_BLOCK, &fatal_signal_set, NULL);
  actions_allocated = false;
  attrs_allocated = false;
  if ((err = posix_spawn_file_actions_init (&actions)) != 0
      || (actions_allocated = true,
          (err = posix_spawn_file_actions_adddup2 (&actions, ofd[0], STDIN_FILENO)) != 0
          || (err = posix_spawn_file_actions_addclose (&actions, ofd[0])) != 0
          || (err = posix_spawn_file_actions_addclose (&actions, ofd[1])) != 0
          || (err = posix_spawnattr_init (&attrs)) != 0
          || (attrs_allocated = true,
              #if defined _WIN32 && !defined __CYGWIN__
              0
              #else
              (err = posix_spawnattr_setsigmask (&attrs, &blocked_signals)) != 0
              || (err = posix_spawnattr_setflags (&attrs, POSIX_SPAWN_SETSIGMASK)) != 0
              #endif
             )
          || (err = posix_spawnp (&child, BOURNE_SHELL, &actions, &attrs, argv, environ)) != 0))
    {
      if (actions_allocated)
        posix_spawn_file_actions_destroy (&actions);
      if (attrs_allocated)
        posix_spawnattr_destroy (&attrs);
      sigprocmask (SIG_UNBLOCK, &fatal_signal_set, NULL);
      errno = err;
      perror ("subprocess failed");
      exit (1);
    }
  posix_spawn_file_actions_destroy (&actions);
  posix_spawnattr_destroy (&attrs);
  sigprocmask (SIG_UNBLOCK, &fatal_signal_set, NULL);
  close (ofd[0]);
  fd = ofd[1];
  fp = fdopen (fd, "w");
  if (fp == NULL)
    {
      fprintf (stderr, "fdopen() failed\n");
      exit (1);
    }
  written = fwrite ("Halle Potta\n", 1, 12, fp);
  if (written < 12)
    {
      fprintf (stderr, "could not write input\n");
      exit (1);
    }
  fclose (fp);
  status = 0;
  while (waitpid (child, &status, 0) != child)
    ;
  if (!WIFEXITED (status))
    {
      fprintf (stderr, "subprocess terminated with unexpected wait status %d\n", status);
      exit (1);
    }
  exitstatus = WEXITSTATUS (status);
  if (exitstatus != 0)
    {
      fprintf (stderr, "subprocess terminated with unexpected exit status %d\n", exitstatus);
      exit (1);
    }
  return 0;
}
