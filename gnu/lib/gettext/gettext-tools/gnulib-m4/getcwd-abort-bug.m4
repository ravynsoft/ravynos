# serial 16
# Determine whether getcwd aborts when the length of the working directory
# name is unusually large.  Any length between 4k and 16k trigger the bug
# when using glibc-2.4.90-9 or older.

# Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# From Jim Meyering

# gl_FUNC_GETCWD_ABORT_BUG([ACTION-IF-BUGGY[, ACTION-IF-WORKS]])
AC_DEFUN([gl_FUNC_GETCWD_ABORT_BUG],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CHECK_DECLS_ONCE([getcwd])
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_REQUIRE([gl_PATHMAX_SNIPPET_PREREQ])

  gl_CHECK_FUNC_GETPAGESIZE
  if test $gl_cv_func_getpagesize = yes; then
    AC_DEFINE_UNQUOTED([HAVE_GETPAGESIZE], [1],
      [Define to 1 if the system has the 'getpagesize' function.])
  fi

  AC_CACHE_CHECK([whether getcwd succeeds when 4k < cwd_length < 16k],
    [gl_cv_func_getcwd_succeeds_beyond_4k],
    [# Remove any remnants of a previous test.
     rm -rf confdir-14B---
     # Arrange for deletion of the temporary directory this test creates.
     ac_clean_files="$ac_clean_files confdir-14B---"
     dnl Please keep this in sync with tests/test-getcwd.c.
     AC_RUN_IFELSE(
       [AC_LANG_SOURCE(
          [[
#include <errno.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#else /* on Windows with MSVC */
# include <direct.h>
#endif
#include <string.h>
#include <sys/stat.h>

]gl_PATHMAX_SNIPPET[
]GL_MDA_DEFINES[

#ifndef S_IRWXU
# define S_IRWXU 0700
#endif

/* FIXME: skip the run-test altogether on systems without getpagesize.  */
#if ! HAVE_GETPAGESIZE
# define getpagesize() 0
#endif

/* This size is chosen to be larger than PATH_MAX (4k), yet smaller than
   the 16kB pagesize on ia64 linux.  Those conditions make the code below
   trigger a bug in glibc's getcwd implementation before 2.4.90-10.  */
#define TARGET_LEN (5 * 1024)

int
main ()
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

  if (1)
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
          ]])],
       [gl_cv_func_getcwd_succeeds_beyond_4k=yes],
       [dnl An abort will provoke an exit code of something like 134 (128 + 6).
        dnl An exit code of 4 can also occur (for example in
        dnl musl libc 1.2.2/powerpc64le, NetBSD 9.0, OpenBSD 6.7:
        dnl getcwd (NULL, 0) fails rather than returning a string longer than
        dnl PATH_MAX.  This may be POSIX compliant (in some interpretations of
        dnl POSIX).  But gnulib's getcwd module wants to provide a non-NULL
        dnl value in this case.
        ret=$?
        if test $ret -ge 128 || test $ret = 4; then
          gl_cv_func_getcwd_succeeds_beyond_4k=no
        else
          gl_cv_func_getcwd_succeeds_beyond_4k=yes
        fi
       ],
       [case "$host_os" in
             # Guess no otherwise, even on glibc systems and musl systems.
          *) gl_cv_func_getcwd_succeeds_beyond_4k="guessing no"
        esac
       ])
    ])
  case "$gl_cv_func_getcwd_succeeds_beyond_4k" in
    *no)
      $1
      ;;
    *)
      $2
      ;;
  esac
])
