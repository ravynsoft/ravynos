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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#if defined _WIN32 && ! defined __CYGWIN__
/* Get _isatty, _getcwd.  */
# include <io.h>
#endif

#include "read-file.h"
#include "macros.h"

/* The name of the "always silent" device.  */
#if defined _WIN32 && ! defined __CYGWIN__
/* Native Windows API.  */
# define DEV_NULL "NUL"
#else
/* Unix API.  */
# define DEV_NULL "/dev/null"
#endif

#define BASE "test-execute"

int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      fprintf (stderr, "%s: need 2 arguments\n", argv[0]);
      return 2;
    }
  char *prog_path = argv[1];
  const char *progname = "test-execute-child";
  int test = atoi (argv[2]);

  /* When this test is executed through 'make' (GNU make 4.4) and
     build-aux/test-driver, i.e. through
       make check TESTS=test-execute.sh
     or
       rm -f test-execute.sh.log; make test-execute.sh.log
     the signal handler for SIGPIPE is set to SIG_IGN.  This is a bug in
     GNU make 4.4: <https://savannah.gnu.org/bugs/index.php?63307>.
     It causes the tests 3 and 4 to fail.  Work around it by resetting
     the signal handler for SIGPIPE to the default.  */
  #ifdef SIGPIPE
  signal (SIGPIPE, SIG_DFL);
  #endif

  switch (test)
    {
    case 14:
    case 15:
    case 16:
      /* Close file descriptors that have been inherited from the parent
         process and that would cause failures in test-execute-child.c.
         Such file descriptors have been seen:
           - with GNU make, when invoked as 'make -j N' with j > 1,
           - in some versions of the KDE desktop environment,
           - on NetBSD,
           - in MacPorts with the "trace mode" enabled.
       */
      #if HAVE_CLOSE_RANGE
      if (close_range (3, 20 - 1, 0) < 0)
      #endif
        {
          int fd;
          for (fd = 3; fd < 20; fd++)
            close (fd);
        }
      break;
    default:
      break;
    }

  switch (test)
    {
    case 0:
      {
        /* Check an invocation without arguments.  Check the exit code.  */
        const char *prog_argv[2] = { prog_path, NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 40);
      }
      break;
    case 1:
      {
        /* Check an invocation of a nonexistent program.  */
        const char *prog_argv[3] = { "./nonexistent", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 127);
      }
      break;
    case 2:
      {
        /* Check argument passing.  */
        const char *prog_argv[13] =
          {
            prog_path,
            "2",
            "abc def",
            "abc\"def\"ghi",
            "xyz\"",
            "abc\\def\\ghi",
            "xyz\\",
            "???",
            "***",
            "",
            "foo",
            "",
            NULL
          };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);
      }
      break;
    case 3:
      #if !(defined _WIN32 && !defined __CYGWIN__)
      {
        /* Check SIGPIPE handling with ignore_sigpipe = false.  */
        const char *prog_argv[3] = { prog_path, "3", NULL };
        int termsig = 0x7DEADBEE;
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, &termsig);
        ASSERT (ret == 127);
        ASSERT (termsig == SIGPIPE);
      }
      #endif
      break;
    case 4:
      #if !(defined _WIN32 && !defined __CYGWIN__)
      {
        /* Check SIGPIPE handling with ignore_sigpipe = true.  */
        const char *prog_argv[3] = { prog_path, "4", NULL };
        int termsig = 0x7DEADBEE;
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           true, false, false, false, true, false, &termsig);
        ASSERT (ret == 0);
        ASSERT (termsig == SIGPIPE);
      }
      #endif
      break;
    case 5:
      {
        /* Check other signal.  */
        const char *prog_argv[3] = { prog_path, "5", NULL };
        int termsig = 0x7DEADBEE;
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, &termsig);
        ASSERT (ret == 127);
        #if defined _WIN32 && !defined __CYGWIN__
        ASSERT (termsig == SIGTERM); /* dummy, from WTERMSIG in <sys/wait.h> */
        #else
        ASSERT (termsig == SIGINT);
        #endif
      }
      break;
    case 6:
      {
        /* Check stdin is inherited.  */
        FILE *fp = fopen (BASE ".tmp", "w");
        fputs ("Foo", fp);
        ASSERT (fclose (fp) == 0);

        fp = freopen (BASE ".tmp", "r", stdin);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "6", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);

        ASSERT (fclose (stdin) == 0);
        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 7:
      {
        /* Check null_stdin = true.  */
        FILE *fp = fopen (BASE ".tmp", "w");
        fputs ("Foo", fp);
        ASSERT (fclose (fp) == 0);

        fp = freopen (BASE ".tmp", "r", stdin);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "7", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, true, false, false, true, false, NULL);
        ASSERT (ret == 0);

        ASSERT (fclose (stdin) == 0);
        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 8:
      {
        /* Check stdout is inherited, part 1 (regular file).  */
        FILE *fp = freopen (BASE ".tmp", "w", stdout);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "8", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);

        ASSERT (fclose (stdout) == 0);

        size_t length;
        char *contents = read_file (BASE ".tmp", 0, &length);
        ASSERT (length == 3 && memcmp (contents, "bar", 3) == 0);

        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 9:
      {
        /* Check stdout is inherited, part 2 (device).  */
        FILE *fp = freopen (DEV_NULL, "w", stdout);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "9", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);
      }
      break;
    case 10:
      {
        /* Check null_stdout = true.  */
        FILE *fp = freopen (BASE ".tmp", "w", stdout);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "10", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, true, false, true, false, NULL);
        ASSERT (ret == 0);

        ASSERT (fclose (stdout) == 0);

        size_t length;
        (void) read_file (BASE ".tmp", 0, &length);
        ASSERT (length == 0);

        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 11:
      {
        /* Check stderr is inherited, part 1 (regular file).  */
        FILE *fp = freopen (BASE ".tmp", "w", stderr);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "11", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);

        ASSERT (fclose (stderr) == 0);

        size_t length;
        char *contents = read_file (BASE ".tmp", 0, &length);
        ASSERT (length == 3 && memcmp (contents, "bar", 3) == 0);

        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 12:
      {
        /* Check stderr is inherited, part 2 (device).  */
        FILE *fp = freopen (DEV_NULL, "w", stderr);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "12", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);
      }
      break;
    case 13:
      {
        /* Check null_stderr = true.  */
        FILE *fp = freopen (BASE ".tmp", "w", stderr);
        ASSERT (fp != NULL);

        const char *prog_argv[3] = { prog_path, "13", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, true, true, false, NULL);
        ASSERT (ret == 0);

        ASSERT (fclose (stderr) == 0);

        size_t length;
        (void) read_file (BASE ".tmp", 0, &length);
        ASSERT (length == 0);

        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 14:
      {
        /* Check file descriptors >= 3 can be inherited.  */
        ASSERT (dup2 (STDOUT_FILENO, 10) >= 0);
        const char *prog_argv[3] = { prog_path, "14", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           true, false, false, false, true, false, NULL);
        ASSERT (ret == 0);
      }
      break;
    case 15:
      {
        /* Check file descriptors >= 3 can be inherited.  */
        ASSERT (fcntl (STDOUT_FILENO, F_DUPFD, 10) >= 0);
        const char *prog_argv[3] = { prog_path, "15", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           true, false, false, false, true, false, NULL);
        ASSERT (ret == 0);
      }
      break;
    case 16:
      {
        /* Check file descriptors >= 3 with O_CLOEXEC bit are not inherited.  */
        ASSERT (fcntl (STDOUT_FILENO, F_DUPFD_CLOEXEC, 10) >= 0);
        const char *prog_argv[3] = { prog_path, "16", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           true, false, false, false, true, false, NULL);
        ASSERT (ret == 0);
      }
      break;
    case 17:
      {
        /* Check that file descriptors >= 3, open for reading, can be inherited,
           including the file position.  */
        FILE *fp = fopen (BASE ".tmp", "w");
        fputs ("Foobar", fp);
        ASSERT (fclose (fp) == 0);

        int fd = open (BASE ".tmp", O_RDONLY);
        ASSERT (fd >= 0 && fd < 10);

        ASSERT (dup2 (fd, 10) >= 0);
        close (fd);
        fd = 10;

        char buf[2];
        ASSERT (read (fd, buf, sizeof (buf)) == sizeof (buf));
        /* The file position is now 2.  */

        const char *prog_argv[3] = { prog_path, "17", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);

        close (fd);
        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 18:
      {
        /* Check that file descriptors >= 3, open for writing, can be inherited,
           including the file position.  */
        remove (BASE ".tmp");
        int fd = open (BASE ".tmp", O_RDWR | O_CREAT | O_TRUNC, 0600);
        ASSERT (fd >= 0 && fd < 10);

        ASSERT (dup2 (fd, 10) >= 0);
        close (fd);
        fd = 10;

        ASSERT (write (fd, "Foo", 3) == 3);
        /* The file position is now 3.  */

        const char *prog_argv[3] = { prog_path, "18", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);

        close (fd);

        size_t length;
        char *contents = read_file (BASE ".tmp", 0, &length);
        ASSERT (length == 6 && memcmp (contents, "Foobar", 6) == 0);

        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 19:
      {
        /* Check that file descriptors >= 3, when inherited, preserve their
           isatty() property, part 1 (regular file).  */
        FILE *fp = fopen (BASE ".tmp", "w");
        fputs ("Foo", fp);
        ASSERT (fclose (fp) == 0);

        int fd_in = open (BASE ".tmp", O_RDONLY);
        ASSERT (fd_in >= 0 && fd_in < 10);

        int fd_out = open (BASE ".tmp", O_WRONLY | O_APPEND);
        ASSERT (fd_out >= 0 && fd_out < 10);

        ASSERT (dup2 (fd_in, 10) >= 0);
        close (fd_in);
        fd_in = 10;

        ASSERT (dup2 (fd_out, 11) >= 0);
        close (fd_out);
        fd_out = 11;

        const char *prog_argv[3] = { prog_path, "19", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        #if defined _WIN32 && ! defined __CYGWIN__
        ASSERT (ret == 4 + 2 * (_isatty (10) != 0) + (_isatty (11) != 0));
        #else
        ASSERT (ret == 4 + 2 * (isatty (10) != 0) + (isatty (11) != 0));
        #endif

        close (fd_in);
        close (fd_out);
        ASSERT (remove (BASE ".tmp") == 0);
      }
      break;
    case 20:
      {
        /* Check that file descriptors >= 3, when inherited, preserve their
           isatty() property, part 2 (character devices).  */
        ASSERT (dup2 (STDIN_FILENO, 10) >= 0);
        int fd_in = 10;

        ASSERT (dup2 (STDOUT_FILENO, 11) >= 0);
        int fd_out = 11;

        const char *prog_argv[3] = { prog_path, "20", NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, NULL,
                           false, false, false, false, true, false, NULL);
        #if defined _WIN32 && ! defined __CYGWIN__
        ASSERT (ret == 4 + 2 * (_isatty (10) != 0) + (_isatty (11) != 0));
        #else
        ASSERT (ret == 4 + 2 * (isatty (10) != 0) + (isatty (11) != 0));
        #endif

        close (fd_in);
        close (fd_out);
      }
      break;
    case 21:
      {
        /* Check execution in a different directory.  */
        rmdir (BASE ".sub");
        ASSERT (mkdir (BASE ".sub", 0700) == 0);

        char cwd[1024];
        #if defined _WIN32 && ! defined __CYGWIN__
        ASSERT (_getcwd (cwd, sizeof (cwd)) != NULL);
        #else
        ASSERT (getcwd (cwd, sizeof (cwd)) != NULL);
        #endif

        const char *prog_argv[4] = { prog_path, "21", cwd, NULL };
        int ret = execute (progname, prog_argv[0], prog_argv, BASE ".sub",
                           false, false, false, false, true, false, NULL);
        ASSERT (ret == 0);

        ASSERT (rmdir (BASE ".sub") == 0);
      }
      break;
    default:
      ASSERT (false);
    }
  return 0;
}
