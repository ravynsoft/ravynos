# fclose.m4 serial 12
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_FUNC_FCLOSE],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])

  gl_FUNC_FFLUSH_STDIN
  case "$gl_cv_func_fflush_stdin" in
    *yes) ;;
    *) REPLACE_FCLOSE=1 ;;
  esac

  AC_REQUIRE([gl_FUNC_CLOSE])
  if test $REPLACE_CLOSE = 1; then
    REPLACE_FCLOSE=1
  fi

  case "$host_os" in
    openedition) REPLACE_FCLOSE=1 ;;
  esac

  if test $REPLACE_FCLOSE = 0; then
    gl_FUNC_FCLOSE_STDIN
    case "$gl_cv_func_fclose_stdin" in
      *yes) ;;
      *) REPLACE_FCLOSE=1 ;;
    esac
  fi
])

dnl Determine whether fclose works on input streams.
dnl Sets gl_cv_func_fclose_stdin.

AC_DEFUN([gl_FUNC_FCLOSE_STDIN],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CHECK_HEADERS_ONCE([unistd.h])
  AC_CACHE_CHECK([whether fclose works on input streams],
    [gl_cv_func_fclose_stdin],
    [echo hello world > conftest.txt
     AC_RUN_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <fcntl.h>
            #include <stdio.h>
            #if HAVE_UNISTD_H
            # include <unistd.h>
            #else /* on Windows with MSVC */
            # include <io.h>
            #endif
           ]GL_MDA_DEFINES],
          [[int fd;
            int fd2;
            FILE *fp;
            fd = open ("conftest.txt", O_RDONLY);
            if (fd < 0)
              return 1;
            if (lseek (fd, 1, SEEK_SET) != 1)
              return 2;
            fd2 = dup (fd);
            if (fd2 < 0)
              return 3;
            fp = fdopen (fd2, "r");
            if (fp == NULL)
              return 4;
            if (fgetc (fp) != 'e')
              { fclose (fp); return 5; }
            /* This fclose() call should reposition the underlying file
               descriptor.  */
            if (fclose (fp) != 0)
              return 6;
            if (lseek (fd2, 0, SEEK_CUR) != -1) /* should fail with EBADF */
              return 7;
            /* Verify the file position.  */
            if (lseek (fd, 0, SEEK_CUR) != 2)
              return 8;
            return 0;
          ]])],
       [gl_cv_func_fclose_stdin=yes],
       [gl_cv_func_fclose_stdin=no],
       [case "$host_os" in
                              # Guess no on glibc systems.
          *-gnu* | gnu*)      gl_cv_func_fclose_stdin="guessing no" ;;
                              # Guess yes on musl systems.
          *-musl* | midipix*) gl_cv_func_fclose_stdin="guessing yes" ;;
                              # Guess no on native Windows.
          mingw* | windows*)  gl_cv_func_fclose_stdin="guessing no" ;;
                              # If we don't know, obey --enable-cross-guesses.
          *)                  gl_cv_func_fclose_stdin="$gl_cross_guess_normal" ;;
        esac
       ])
     rm conftest.txt
    ])
])
