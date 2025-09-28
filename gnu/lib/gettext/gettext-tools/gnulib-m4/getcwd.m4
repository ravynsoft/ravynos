# getcwd.m4 - check for working getcwd that is compatible with glibc

# Copyright (C) 2001, 2003-2007, 2009-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# Written by Paul Eggert.
# serial 21

AC_DEFUN([gl_FUNC_GETCWD_NULL],
  [
   AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
   AC_CHECK_HEADERS_ONCE([unistd.h])
   AC_CACHE_CHECK([whether getcwd (NULL, 0) allocates memory for result],
     [gl_cv_func_getcwd_null],
     [AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#	 include <stdlib.h>
#        if HAVE_UNISTD_H
#         include <unistd.h>
#        else /* on Windows with MSVC */
#         include <direct.h>
#        endif
         ]GL_MDA_DEFINES[
#        ifndef getcwd
         char *getcwd ();
#        endif
]], [[
#if defined _WIN32 && ! defined __CYGWIN__
/* mingw cwd does not start with '/', but _getcwd does allocate.
   However, mingw fails to honor non-zero size.  */
#else
           if (chdir ("/") != 0)
             return 1;
           else
             {
               char *f = getcwd (NULL, 0);
               if (! f)
                 return 2;
               if (f[0] != '/')
                 { free (f); return 3; }
               if (f[1] != '\0')
                 { free (f); return 4; }
               free (f);
               return 0;
             }
#endif
         ]])],
        [gl_cv_func_getcwd_null=yes],
        [gl_cv_func_getcwd_null=no],
        [[case "$host_os" in
                           # Guess yes on glibc systems.
            *-gnu* | gnu*) gl_cv_func_getcwd_null="guessing yes";;
                           # Guess yes on musl systems.
            *-musl*)       gl_cv_func_getcwd_null="guessing yes";;
                           # Guess yes on systems that emulate the Linux system calls.
            midipix*)      gl_cv_func_getcwd_null="guessing yes";;
                           # Guess yes on Cygwin.
            cygwin*)       gl_cv_func_getcwd_null="guessing yes";;
                           # If we don't know, obey --enable-cross-guesses.
            *)             gl_cv_func_getcwd_null="$gl_cross_guess_normal";;
          esac
        ]])])
])

AC_DEFUN([gl_FUNC_GETCWD_SIGNATURE],
[
  AC_CACHE_CHECK([for getcwd with POSIX signature],
    [gl_cv_func_getcwd_posix_signature],
    [AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM(
         [[#include <unistd.h>
         ]GL_MDA_DEFINES],
         [[extern
           #ifdef __cplusplus
           "C"
           #endif
           char *getcwd (char *, size_t);
         ]])
      ],
      [gl_cv_func_getcwd_posix_signature=yes],
      [gl_cv_func_getcwd_posix_signature=no])
   ])
])

dnl Guarantee that getcwd will malloc with a NULL first argument.  Assumes
dnl that either the system getcwd is robust, or that calling code is okay
dnl with spurious failures when run from a directory with an absolute name
dnl larger than 4k bytes.
dnl
dnl Assumes that getcwd exists; if you are worried about obsolete
dnl platforms that lacked getcwd(), then you need to use the GPL module.
AC_DEFUN([gl_FUNC_GETCWD_LGPL],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_GETCWD_NULL])
  AC_REQUIRE([gl_FUNC_GETCWD_SIGNATURE])

  case $gl_cv_func_getcwd_null,$gl_cv_func_getcwd_posix_signature in
  *yes,yes) ;;
  *)
    dnl Minimal replacement lib/getcwd-lgpl.c.
    REPLACE_GETCWD=1
    ;;
  esac
])

dnl Check for all known getcwd bugs; useful for a program likely to be
dnl executed from an arbitrary location.
AC_DEFUN([gl_FUNC_GETCWD],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_GETCWD_NULL])
  AC_REQUIRE([gl_FUNC_GETCWD_SIGNATURE])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles

  gl_abort_bug=no
  case "$host_os" in
    mingw* | windows*)
      gl_cv_func_getcwd_path_max=yes
      ;;
    *)
      gl_FUNC_GETCWD_PATH_MAX
      case "$gl_cv_func_getcwd_null" in
        *yes)
          gl_FUNC_GETCWD_ABORT_BUG([gl_abort_bug=yes])
          ;;
      esac
      ;;
  esac
  dnl Define HAVE_MINIMALLY_WORKING_GETCWD and HAVE_PARTLY_WORKING_GETCWD
  dnl if appropriate.
  case "$gl_cv_func_getcwd_path_max" in
    *"no" | *"no, it has the AIX bug") ;;
    *)
      AC_DEFINE([HAVE_MINIMALLY_WORKING_GETCWD], [1],
        [Define to 1 if getcwd minimally works, that is, its result can be
         trusted when it succeeds.])
      ;;
  esac
  case "$gl_cv_func_getcwd_path_max" in
    *"no, but it is partly working")
      AC_DEFINE([HAVE_PARTLY_WORKING_GETCWD], [1],
        [Define to 1 if getcwd works, except it sometimes fails when it
         shouldn't, setting errno to ERANGE, ENAMETOOLONG, or ENOENT.])
      ;;
    *"yes, but with shorter paths")
      AC_DEFINE([HAVE_GETCWD_SHORTER], [1],
        [Define to 1 if getcwd works, but with shorter paths
         than is generally tested with the replacement.])
      ;;
  esac

  if { case "$gl_cv_func_getcwd_null" in *yes) false;; *) true;; esac; } \
     || test $gl_cv_func_getcwd_posix_signature != yes \
     || { case "$gl_cv_func_getcwd_path_max" in *yes*) false;; *) true;; esac; } \
     || test $gl_abort_bug = yes; then
    REPLACE_GETCWD=1
  fi
])

# Prerequisites of lib/getcwd.c, when full replacement is in effect.
AC_DEFUN([gl_PREREQ_GETCWD],
[
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_REQUIRE([gl_CHECK_TYPE_STRUCT_DIRENT_D_INO])
  :
])
