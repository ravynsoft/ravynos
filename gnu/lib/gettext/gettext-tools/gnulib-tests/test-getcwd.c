/* Test of getcwd() function.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

#include <config.h>

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "pathmax.h"
#include "qemu.h"
#include "macros.h"

#if !(HAVE_GETPAGESIZE || defined getpagesize)
# define getpagesize() 0
#endif

/* This size is chosen to be larger than PATH_MAX (4k), yet smaller than
   the 16kB pagesize on ia64 linux.  Those conditions make the code below
   trigger a bug in glibc's getcwd implementation before 2.4.90-10.  */
#define TARGET_LEN (5 * 1024)

#if defined HAVE_OPENAT || (defined GNULIB_OPENAT && defined HAVE_FDOPENDIR)
# define HAVE_OPENAT_SUPPORT 1
#else
# define HAVE_OPENAT_SUPPORT 0
#endif

/* Keep this test in sync with m4/getcwd-abort-bug.m4.  */
static int
test_abort_bug (void)
{
  char *cwd;
  size_t initial_cwd_len;
  int fail = 0;

  /* The bug is triggered when PATH_MAX < getpagesize (), so skip
     this relatively expensive and invasive test if that's not true.  */
#ifdef PATH_MAX
  int bug_possible = PATH_MAX < getpagesize ();
#else
  int bug_possible = 0;
#endif
  if (! bug_possible)
    return 0;

  cwd = getcwd (NULL, 0);
  if (cwd == NULL)
    return 2;

  initial_cwd_len = strlen (cwd);
  free (cwd);

  if (HAVE_OPENAT_SUPPORT)
    {
      static char const dir_name[] = "confdir-14B---";
      size_t desired_depth = ((TARGET_LEN - 1 - initial_cwd_len)
                              / sizeof dir_name);
      size_t d;
      for (d = 0; d < desired_depth; d++)
        {
          if (mkdir (dir_name, S_IRWXU) < 0 || chdir (dir_name) < 0)
            {
              if (! (errno == ERANGE || errno == ENAMETOOLONG
                     || errno == ENOENT))
                fail = 3; /* Unable to construct deep hierarchy.  */
              break;
            }
        }

      /* If libc has the bug in question, this invocation of getcwd
         results in a failed assertion.  */
      cwd = getcwd (NULL, 0);
      if (cwd == NULL)
        fail = 4; /* getcwd didn't assert, but it failed for a long name
                     where the answer could have been learned.  */
      free (cwd);

      /* Call rmdir first, in case the above chdir failed.  */
      rmdir (dir_name);
      while (0 < d--)
        {
          if (chdir ("..") < 0)
            {
              fail = 5;
              break;
            }
          rmdir (dir_name);
        }
    }

  return fail;
}

/* The length of this name must be 8.  */
#define DIR_NAME "confdir3"
#define DIR_NAME_LEN 8
#define DIR_NAME_SIZE (DIR_NAME_LEN + 1)

/* The length of "../".  */
#define DOTDOTSLASH_LEN 3

/* Leftover bytes in the buffer, to work around library or OS bugs.  */
#define BUF_SLOP 20

/* Keep this test in sync with m4/getcwd-path-max.m4.  */
static int
test_long_name (void)
{
#ifndef PATH_MAX
  /* The Hurd doesn't define this, so getcwd can't exhibit the bug --
     at least not on a local file system.  And if we were to start worrying
     about remote file systems, we'd have to enable the wrapper function
     all of the time, just to be safe.  That's not worth the cost.  */
  return 0;
#elif ((INT_MAX / (DIR_NAME_SIZE / DOTDOTSLASH_LEN + 1) \
        - DIR_NAME_SIZE - BUF_SLOP) \
       <= PATH_MAX)
  /* FIXME: Assuming there's a system for which this is true,
     this should be done in a compile test.  */
  return 0;
#else
  /* For a process running under QEMU user-mode, the "/" directory is not
     really the root directory, but the value of the QEMU_LD_PREFIX environment
     variable or of the -L command-line option.  This causes the logic from
     glibc/sysdeps/posix/getcwd.c to fail.  In this case, skip the test.  */
  if (is_running_under_qemu_user ())
    return 77;

  char buf[PATH_MAX * (DIR_NAME_SIZE / DOTDOTSLASH_LEN + 1)
           + DIR_NAME_SIZE + BUF_SLOP];
  char *cwd = getcwd (buf, PATH_MAX);
  size_t initial_cwd_len;
  size_t cwd_len;
  int fail = 0;
  size_t n_chdirs = 0;

  if (cwd == NULL)
    return 1;

  cwd_len = initial_cwd_len = strlen (cwd);

  while (1)
    {
# ifdef HAVE_GETCWD_SHORTER
      /* On OS/X <= 10.9 for example, we're restricted to shorter paths
         as lstat() doesn't support more than PATH_MAX.  */
      size_t dotdot_max = PATH_MAX * 2;
# else
      size_t dotdot_max = PATH_MAX * (DIR_NAME_SIZE / DOTDOTSLASH_LEN);
# endif
      char *c = NULL;

      cwd_len += DIR_NAME_SIZE;
      /* If mkdir or chdir fails, it could be that this system cannot create
         any file with an absolute name longer than PATH_MAX, such as cygwin.
         If so, leave fail as 0, because the current working directory can't
         be too long for getcwd if it can't even be created.  On Linux with
         the 9p file system, mkdir fails with error EINVAL when cwd_len gets
         too long; ignore this failure because the getcwd() system call
         produces good results whereas the gnulib substitute calls getdents64
         which fails with error EPROTO.
         For other errors, be pessimistic and consider that as a failure,
         too.  */
      if (mkdir (DIR_NAME, S_IRWXU) < 0 || chdir (DIR_NAME) < 0)
        {
          if (! (errno == ERANGE || errno == ENAMETOOLONG || errno == ENOENT))
            #ifdef __linux__
            if (! (errno == EINVAL))
            #endif
              fail = 2;
          break;
        }

      if (PATH_MAX <= cwd_len && cwd_len < PATH_MAX + DIR_NAME_SIZE)
        {
          c = getcwd (buf, PATH_MAX);
          if (!c && errno == ENOENT)
            {
              fail = 3;
              break;
            }
          if (c)
            {
              fail = 4;
              break;
            }
          if (! (errno == ERANGE || errno == ENAMETOOLONG))
            {
              fail = 5;
              break;
            }
        }

      if (dotdot_max <= cwd_len - initial_cwd_len)
        {
          if (dotdot_max + DIR_NAME_SIZE < cwd_len - initial_cwd_len)
            break;
          c = getcwd (buf, cwd_len + 1);
          if (!c)
            {
              if (! (errno == ERANGE || errno == ENOENT
                     || errno == ENAMETOOLONG))
                {
                  fail = 6;
                  break;
                }
              if (HAVE_OPENAT_SUPPORT || errno == ERANGE || errno == ENOENT)
                {
                  fail = 7;
                  break;
                }
            }
        }

      if (c && strlen (c) != cwd_len)
        {
          fail = 8;
          break;
        }
      ++n_chdirs;
    }

  /* Leaving behind such a deep directory is not polite.
     So clean up here, right away, even though the driving
     shell script would also clean up.  */
  {
    size_t i;

    /* Try rmdir first, in case the chdir failed.  */
    rmdir (DIR_NAME);
    for (i = 0; i <= n_chdirs; i++)
      {
        if (chdir ("..") < 0)
          break;
        if (rmdir (DIR_NAME) != 0)
          break;
      }
  }

  return fail;
#endif
}

int
main ()
{
  int err1 = test_abort_bug ();
  int err2 = test_long_name ();
  return err1 * 10 + (err1 != 0 && err2 == 77 ? 0 : err2);
}
