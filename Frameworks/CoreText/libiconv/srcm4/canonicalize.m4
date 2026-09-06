# canonicalize.m4
# serial 40

dnl Copyright (C) 2003-2007, 2009-2026 Free Software Foundation, Inc.

dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# Provides canonicalize_file_name and canonicalize_filename_mode, but does
# not provide or fix realpath.
AC_DEFUN([gl_FUNC_CANONICALIZE_FILENAME_MODE],
[
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_REQUIRE([gl_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK])
  AC_CHECK_FUNCS_ONCE([canonicalize_file_name])
  gl_CHECK_FUNCS_ANDROID([faccessat], [[#include <unistd.h>]])
  AC_REQUIRE([gl_DOUBLE_SLASH_ROOT])
  AC_REQUIRE([gl_FUNC_REALPATH_WORKS])
  if test $ac_cv_func_canonicalize_file_name = no; then
    HAVE_CANONICALIZE_FILE_NAME=0
  else
    case "$gl_cv_func_realpath_works" in
      *yes) ;;
      *)    REPLACE_CANONICALIZE_FILE_NAME=1 ;;
    esac
  fi
])

# Provides canonicalize_file_name and realpath.
AC_DEFUN([gl_CANONICALIZE_LGPL],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_CANONICALIZE_LGPL_SEPARATE])
  if test $ac_cv_func_canonicalize_file_name = no; then
    HAVE_CANONICALIZE_FILE_NAME=0
    if test $ac_cv_func_realpath = no; then
      HAVE_REALPATH=0
    else
      case "$gl_cv_func_realpath_works" in
        *yes) ;;
        *)    REPLACE_REALPATH=1 ;;
      esac
    fi
  else
    case "$gl_cv_func_realpath_works" in
      *yes)
        ;;
      *)
        REPLACE_CANONICALIZE_FILE_NAME=1
        REPLACE_REALPATH=1
        ;;
    esac
  fi
])

# Like gl_CANONICALIZE_LGPL, except prepare for separate compilation
# (no REPLACE_CANONICALIZE_FILE_NAME, no REPLACE_REALPATH, no AC_LIBOBJ).
AC_DEFUN([gl_CANONICALIZE_LGPL_SEPARATE],
[
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_REQUIRE([gl_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK])
  AC_CHECK_FUNCS_ONCE([canonicalize_file_name])
  gl_CHECK_FUNCS_ANDROID([faccessat], [[#include <unistd.h>]])

  dnl On native Windows, we use _getcwd(), regardless whether getcwd() is
  dnl available through the linker option '-loldnames'.
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | windows*) ;;
    *) AC_CHECK_FUNCS([getcwd]) ;;
  esac

  AC_REQUIRE([gl_DOUBLE_SLASH_ROOT])
  AC_REQUIRE([gl_FUNC_REALPATH_WORKS])
  AC_CHECK_HEADERS_ONCE([sys/param.h])
])

# Check whether realpath works.  Assume that if a platform has both
# realpath and canonicalize_file_name, but the former is broken, then
# so is the latter.
AC_DEFUN([gl_FUNC_REALPATH_WORKS],
[
  AC_CHECK_FUNCS_ONCE([realpath lstat])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether realpath works], [gl_cv_func_realpath_works], [
    rm -rf conftest.a conftest.d
    touch conftest.a
    # Assume that if we have lstat, we can also check symlinks.
    if test $ac_cv_func_lstat = yes; then
      ln -s conftest.a conftest.l
    fi
    mkdir conftest.d
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([[
        ]GL_NOCRASH[
        #include <errno.h>
        #include <stdlib.h>
        #include <string.h>
      ]], [[
        int result = 0;
        /* This test fails on Solaris 10.  */
        {
          char *name = realpath ("conftest.a", NULL);
          if (!(name && *name == '/'))
            result |= 1;
          free (name);
        }
        /* This test fails on older versions of Cygwin.  */
        {
          char *name = realpath ("conftest.b/../conftest.a", NULL);
          if (name != NULL)
            result |= 2;
          free (name);
        }
        /* This test fails on macOS 14, Cygwin 2.9.  */
        #if HAVE_LSTAT
        {
          char *name = realpath ("conftest.l/../conftest.a", NULL);
          if (name != NULL || errno != ENOTDIR)
            result |= 4;
          free (name);
        }
        #endif
        /* This test fails on macOS 14, OpenBSD 6.0.  */
        {
          char *name = realpath ("conftest.a/", NULL);
          if (name != NULL)
            result |= 8;
          free (name);
        }
        /* This test fails on AIX 7, Solaris 10.  */
        {
          char *name1 = realpath (".", NULL);
          char *name2 = realpath ("conftest.d//./..", NULL);
          if (! name1 || ! name2 || strcmp (name1, name2))
            result |= 16;
          free (name1);
          free (name2);
        }
        #ifdef __linux__
        /* On Linux, // is the same as /. See also double-slash-root.m4.
           realpath() should respect this.
           This test fails on musl libc 1.2.2.  */
        {
          char *name = realpath ("//", NULL);
          if (! name || strcmp (name, "/"))
            result |= 32;
          free (name);
        }
        #endif
        return result;
      ]])
     ],
     [gl_cv_func_realpath_works=yes],
     [case $? in
        32) gl_cv_func_realpath_works=nearly ;;
        *)  gl_cv_func_realpath_works=no ;;
      esac
     ],
     [case "$host_os" in
                           # Guess yes on glibc systems.
        *-gnu* | gnu*)     gl_cv_func_realpath_works="guessing yes" ;;
                           # Guess 'nearly' on musl systems.
        *-musl*)           gl_cv_func_realpath_works="guessing nearly" ;;
                           # Guess no on macOS.
        darwin*)           gl_cv_func_realpath_works="guessing no" ;;
                           # Guess no on Cygwin.
        cygwin*)           gl_cv_func_realpath_works="guessing no" ;;
                           # Guess no on native Windows.
        mingw* | windows*) gl_cv_func_realpath_works="guessing no" ;;
                           # If we don't know, obey --enable-cross-guesses.
        *)                 gl_cv_func_realpath_works="$gl_cross_guess_normal" ;;
      esac
     ])
    rm -rf conftest.a conftest.l conftest.d
  ])
  case "$gl_cv_func_realpath_works" in
    *yes)
      AC_DEFINE([FUNC_REALPATH_WORKS], [1],
        [Define to 1 if realpath() can malloc memory, always gives an absolute path, and handles leading slashes and a trailing slash correctly.])
      ;;
    *nearly)
      AC_DEFINE([FUNC_REALPATH_NEARLY_WORKS], [1],
        [Define to 1 if realpath() can malloc memory, always gives an absolute path, and handles a trailing slash correctly.])
      ;;
  esac
])
