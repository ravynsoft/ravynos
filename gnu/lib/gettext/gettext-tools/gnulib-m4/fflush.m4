# fflush.m4 serial 19

# Copyright (C) 2007-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl From Eric Blake

dnl Find out how to obey POSIX semantics of fflush(stdin) discarding
dnl unread input on seekable streams, rather than C99 undefined semantics.

AC_DEFUN([gl_FUNC_FFLUSH],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  gl_FUNC_FFLUSH_STDIN
  case "$gl_cv_func_fflush_stdin" in
    *yes) ;;
    *) REPLACE_FFLUSH=1 ;;
  esac
])

dnl Determine whether fflush works on input streams.
dnl Sets gl_cv_func_fflush_stdin.

AC_DEFUN([gl_FUNC_FFLUSH_STDIN],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_CACHE_CHECK([whether fflush works on input streams],
    [gl_cv_func_fflush_stdin],
    [echo hello world > conftest.txt
     AC_RUN_IFELSE([AC_LANG_PROGRAM(
       [[
#include <stdio.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#else /* on Windows with MSVC */
# include <io.h>
#endif
       ]GL_MDA_DEFINES],
       [[FILE *f = fopen ("conftest.txt", "r");
         char buffer[10];
         int fd;
         int c;
         if (f == NULL)
           return 1;
         fd = fileno (f);
         if (fd < 0 || fread (buffer, 1, 5, f) != 5)
           { fclose (f); return 2; }
         /* For deterministic results, ensure f read a bigger buffer.  */
         if (lseek (fd, 0, SEEK_CUR) == 5)
           { fclose (f); return 3; }
         /* POSIX requires fflush-fseek to set file offset of fd.  This fails
            on BSD systems and on mingw.  */
         if (fflush (f) != 0 || fseek (f, 0, SEEK_CUR) != 0)
           { fclose (f); return 4; }
         if (lseek (fd, 0, SEEK_CUR) != 5)
           { fclose (f); return 5; }
         /* Verify behaviour of fflush after ungetc. See
            <https://www.opengroup.org/austin/aardvark/latest/xshbug3.txt>  */
         /* Verify behaviour of fflush after a backup ungetc.  This fails on
            mingw.  */
         c = fgetc (f);
         ungetc (c, f);
         fflush (f);
         if (fgetc (f) != c)
           { fclose (f); return 6; }
         /* Verify behaviour of fflush after a non-backup ungetc.  This fails
            on glibc 2.8 and on BSD systems.  */
         c = fgetc (f);
         ungetc ('@', f);
         fflush (f);
         if (fgetc (f) != c)
           { fclose (f); return 7; }
         fclose (f);
         return 0;
       ]])],
       [gl_cv_func_fflush_stdin=yes],
       [gl_cv_func_fflush_stdin=no],
       [case "$host_os" in
                             # Guess no on native Windows.
          mingw* | windows*) gl_cv_func_fflush_stdin="guessing no" ;;
          *)                 gl_cv_func_fflush_stdin=cross ;;
        esac
       ])
     rm conftest.txt
    ])
  case "$gl_cv_func_fflush_stdin" in
    *yes) gl_func_fflush_stdin=1 ;;
    *no)  gl_func_fflush_stdin=0 ;;
    *)    gl_func_fflush_stdin='(-1)' ;;
  esac
  AC_DEFINE_UNQUOTED([FUNC_FFLUSH_STDIN], [$gl_func_fflush_stdin],
    [Define to 1 if fflush is known to work on stdin as per POSIX.1-2008,
     0 if fflush is known to not work, -1 if unknown.])
])

# Prerequisites of lib/fflush.c.
AC_DEFUN([gl_PREREQ_FFLUSH], [:])
