/* Test of create_pipe_bidi/wait_subprocess.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Depending on arguments, this test intentionally closes stderr or
   starts life with stderr closed.  So, we arrange to have fd 10
   (outside the range of interesting fd's during the test) set up to
   duplicate the original stderr.  */

#define BACKUP_STDERR_FILENO 10
#define ASSERT_STREAM myerr
#include "macros.h"

static FILE *myerr;

/* Create a bi-directional pipe to a test child, and validate that the
   child program returns the expected output.
   PROG is the program to run in the child process.
   STDERR_CLOSED is true if we have already closed fd 2.  */
static void
test_pipe (const char *prog, bool stderr_closed)
{
  int fd[2];
  const char *argv[3];
  pid_t pid;
  char buffer[2] = { 'a', 't' };

  /* Set up child.  */
  argv[0] = prog;
  argv[1] = (stderr_closed ? "1" : "0");
  argv[2] = NULL;
  pid = create_pipe_bidi (prog, prog, argv, NULL, false, true, true, fd);
  ASSERT (0 <= pid);
  ASSERT (STDERR_FILENO < fd[0]);
  ASSERT (STDERR_FILENO < fd[1]);

  /* Push child's input.  */
  ASSERT (write (fd[1], buffer, 1) == 1);
  ASSERT (close (fd[1]) == 0);

  /* Get child's output.  */
  ASSERT (read (fd[0], buffer, 2) == 1);

  /* Wait for child.  */
  ASSERT (wait_subprocess (pid, prog, true, false, true, true, NULL) == 0);
  ASSERT (close (fd[0]) == 0);

  /* Check the result.  */
  ASSERT (buffer[0] == 'b');
  ASSERT (buffer[1] == 't');
}

int
main (int argc, char *argv[])
{
  int test;
  int fd;

  if (argc != 3)
    {
      fprintf (stderr, "%s: need 2 arguments\n", argv[0]);
      return 2;
    }
  /* We might close fd 2 later, so save it in fd 10.  */
  if (dup2 (STDERR_FILENO, BACKUP_STDERR_FILENO) != BACKUP_STDERR_FILENO
      || (myerr = fdopen (BACKUP_STDERR_FILENO, "w")) == NULL)
    return 2;

  /* Selectively close various standard fds, to verify the child process is
     not impacted by this.  */
  test = atoi (argv[2]);
  switch (test)
    {
    case 0:
      break;
    case 1:
      close (0);
      break;
    case 2:
      close (1);
      break;
    case 3:
      close (0);
      close (1);
      break;
    case 4:
      close (2);
      break;
    case 5:
      close (0);
      close (2);
      break;
    case 6:
      close (1);
      close (2);
      break;
    case 7:
      close (0);
      close (1);
      close (2);
      break;
    default:
      ASSERT (false);
    }

  /* Plug any file descriptor leaks inherited from outside world before
     starting, so that child has a clean slate (at least for the fds that we
     might be manipulating).  */
  for (fd = 3; fd < 7; fd++)
    close (fd);

  test_pipe (argv[1], test >= 4);

  return 0;
}
