# serial 25
# Check for several getcwd bugs with long file names.
# If so, arrange to compile the wrapper function.

# This is necessary for at least GNU libc on linux-2.4.19 and 2.4.20.
# I've heard that this is due to a Linux kernel bug, and that it has
# been fixed between 2.4.21-pre3 and 2.4.21-pre4.

# Copyright (C) 2003-2007, 2009-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# From Jim Meyering

AC_DEFUN([gl_FUNC_GETCWD_PATH_MAX],
[
  AC_CHECK_DECLS_ONCE([getcwd])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_REQUIRE([gl_PATHMAX_SNIPPET_PREREQ])
  AC_CACHE_CHECK([whether getcwd handles long file names properly],
    [gl_cv_func_getcwd_path_max],
    [# Arrange for deletion of the temporary directory this test creates.
     ac_clean_files="$ac_clean_files confdir3"
     dnl Please keep this in sync with tests/test-getcwd.c.
     AC_RUN_IFELSE(
       [AC_LANG_SOURCE(
          [[
#include <errno.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#else
# include <direct.h>
#endif
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

]gl_PATHMAX_SNIPPET[

#ifndef AT_FDCWD
# define AT_FDCWD 0
#endif
#ifdef ENAMETOOLONG
# define is_ENAMETOOLONG(x) ((x) == ENAMETOOLONG)
#else
# define is_ENAMETOOLONG(x) 0
#endif

/* Use the getcwd function, not any macro.  */
#undef getcwd

]GL_MDA_DEFINES[

#ifndef S_IRWXU
# define S_IRWXU 0700
#endif

/* The length of this name must be 8.  */
#define DIR_NAME "confdir3"
#define DIR_NAME_LEN 8
#define DIR_NAME_SIZE (DIR_NAME_LEN + 1)

/* The length of "../".  */
#define DOTDOTSLASH_LEN 3

/* Leftover bytes in the buffer, to work around library or OS bugs.  */
#define BUF_SLOP 20

int
main ()
{
#ifndef PATH_MAX
  /* The Hurd doesn't define this, so getcwd can't exhibit the bug --
     at least not on a local file system.  And if we were to start worrying
     about remote file systems, we'd have to enable the wrapper function
     all of the time, just to be safe.  That's not worth the cost.  */
  exit (0);
#elif ((INT_MAX / (DIR_NAME_SIZE / DOTDOTSLASH_LEN + 1) \
        - DIR_NAME_SIZE - BUF_SLOP) \
       <= PATH_MAX)
  /* FIXME: Assuming there's a system for which this is true,
     this should be done in a compile test.  */
  exit (0);
#else
  char buf[PATH_MAX * (DIR_NAME_SIZE / DOTDOTSLASH_LEN + 1)
           + DIR_NAME_SIZE + BUF_SLOP];
  char *cwd = getcwd (buf, PATH_MAX);
  size_t initial_cwd_len;
  size_t cwd_len;
  int fail = 0;
  size_t n_chdirs = 0;

  if (cwd == NULL)
    exit (10);

  cwd_len = initial_cwd_len = strlen (cwd);

  while (1)
    {
      size_t dotdot_max = PATH_MAX * (DIR_NAME_SIZE / DOTDOTSLASH_LEN);
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
          if (! (errno == ERANGE || is_ENAMETOOLONG (errno)))
            #ifdef __linux__
            if (! (errno == EINVAL))
            #endif
              fail = 20;
          break;
        }

      if (PATH_MAX <= cwd_len && cwd_len < PATH_MAX + DIR_NAME_SIZE)
        {
          struct stat sb;

          c = getcwd (buf, PATH_MAX);
          if (!c && errno == ENOENT)
            {
              fail = 11;
              break;
            }
          if (c)
            {
              fail = 31;
              break;
            }
          if (! (errno == ERANGE || is_ENAMETOOLONG (errno)))
            {
              fail = 21;
              break;
            }

          /* Our replacement needs to be able to stat() long ../../paths,
             so generate a path larger than PATH_MAX to check,
             avoiding the replacement if we can't stat().  */
          c = getcwd (buf, cwd_len + 1);
          if (c && !AT_FDCWD && stat (c, &sb) != 0 && is_ENAMETOOLONG (errno))
            {
              fail = 32;
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
                     || is_ENAMETOOLONG (errno)))
                {
                  fail = 22;
                  break;
                }
              if (AT_FDCWD || errno == ERANGE || errno == ENOENT)
                {
                  fail = 12;
                  break;
                }
            }
        }

      if (c && strlen (c) != cwd_len)
        {
          fail = 23;
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

  exit (fail);
#endif
}
          ]])],
       [gl_cv_func_getcwd_path_max=yes],
       [case $? in
        10|11|12) gl_cv_func_getcwd_path_max='no, but it is partly working';;
        31) gl_cv_func_getcwd_path_max='no, it has the AIX bug';;
        32) gl_cv_func_getcwd_path_max='yes, but with shorter paths';;
        *) gl_cv_func_getcwd_path_max=no;;
        esac],
       [# Cross-compilation guesses:
        case "$host_os" in
          aix*) # On AIX, it has the AIX bug.
            gl_cv_func_getcwd_path_max='guessing no, it has the AIX bug' ;;
          gnu*) # On Hurd, it is 'yes'.
            gl_cv_func_getcwd_path_max='guessing yes' ;;
          linux* | kfreebsd*)
            # On older Linux+glibc it's 'no, but it is partly working',
            # on newer Linux+glibc it's 'yes'.
            # On Linux+musl libc, it's 'no, but it is partly working'.
            # On kFreeBSD+glibc, it's 'no, but it is partly working'.
            gl_cv_func_getcwd_path_max='guessing no, but it is partly working' ;;
          *) # If we don't know, obey --enable-cross-guesses.
            gl_cv_func_getcwd_path_max="$gl_cross_guess_normal" ;;
        esac
       ])
    ])
])
