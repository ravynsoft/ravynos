# ftello.m4 serial 16
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FTELLO],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([gl_STDIN_LARGE_OFFSET])
  AC_REQUIRE([gl_SYS_TYPES_H])

  dnl Persuade glibc <stdio.h> to declare ftello().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_CHECK_DECLS_ONCE([ftello])
  if test $ac_cv_have_decl_ftello = no; then
    HAVE_DECL_FTELLO=0
  fi

  AC_CACHE_CHECK([for ftello], [gl_cv_func_ftello],
    [
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
           [[#include <stdio.h>]],
           [[ftello (stdin);]])],
        [gl_cv_func_ftello=yes],
        [gl_cv_func_ftello=no])
    ])
  if test $gl_cv_func_ftello = no; then
    HAVE_FTELLO=0
  else
    if test $WINDOWS_64_BIT_OFF_T = 1; then
      REPLACE_FTELLO=1
    fi
    if test $gl_cv_var_stdin_large_offset = no; then
      REPLACE_FTELLO=1
    fi
    AC_REQUIRE([AC_CANONICAL_HOST])
    if test $REPLACE_FTELLO = 0; then
      dnl On native Windows, in some circumstances, ftell(), ftello(),
      dnl fgetpos(), lseek(), _lseeki64() all succeed on devices of type
      dnl FILE_TYPE_PIPE. However, to match POSIX behaviour, we want
      dnl ftell(), ftello(), fgetpos(), lseek() to fail when the argument fd
      dnl designates a pipe. See also
      dnl https://github.com/python/cpython/issues/78961#issuecomment-1093800325
      case "$host_os" in
        mingw* | windows*) REPLACE_FTELLO=1 ;;
      esac
    fi
    if test $REPLACE_FTELLO = 0; then
      dnl Detect bug on Solaris.
      dnl ftell and ftello produce incorrect results after putc that followed a
      dnl getc call that reached EOF on Solaris. This is because the _IOREAD
      dnl flag does not get cleared in this case, even though _IOWRT gets set,
      dnl and ftell and ftello look whether the _IOREAD flag is set.
      AC_CACHE_CHECK([whether ftello works],
        [gl_cv_func_ftello_works],
        [
          dnl Initial guess, used when cross-compiling or when /dev/tty cannot
          dnl be opened.
changequote(,)dnl
          case "$host_os" in
                               # Guess no on Solaris.
            solaris*)          gl_cv_func_ftello_works="guessing no" ;;
                               # Guess yes on native Windows.
            mingw* | windows*) gl_cv_func_ftello_works="guessing yes" ;;
                               # Guess yes otherwise.
            *)                 gl_cv_func_ftello_works="guessing yes" ;;
          esac
changequote([,])dnl
          AC_RUN_IFELSE(
            [AC_LANG_SOURCE([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TESTFILE "conftest.tmp"
int
main (void)
{
  FILE *fp;

  /* Create a file with some contents.  */
  fp = fopen (TESTFILE, "w");
  if (fp == NULL)
    return 70;
  if (fwrite ("foogarsh", 1, 8, fp) < 8)
    { fclose (fp); return 71; }
  if (fclose (fp))
    return 72;

  /* The file's contents is now "foogarsh".  */

  /* Try writing after reading to EOF.  */
  fp = fopen (TESTFILE, "r+");
  if (fp == NULL)
    return 73;
  if (fseek (fp, -1, SEEK_END))
    { fclose (fp); return 74; }
  if (!(getc (fp) == 'h'))
    { fclose (fp); return 1; }
  if (!(getc (fp) == EOF))
    { fclose (fp); return 2; }
  if (!(ftell (fp) == 8))
    { fclose (fp); return 3; }
  if (!(ftell (fp) == 8))
    { fclose (fp); return 4; }
  if (!(putc ('!', fp) == '!'))
    { fclose (fp); return 5; }
  if (!(ftell (fp) == 9))
    { fclose (fp); return 6; }
  if (!(fclose (fp) == 0))
    return 7;
  fp = fopen (TESTFILE, "r");
  if (fp == NULL)
    return 75;
  {
    char buf[10];
    if (!(fread (buf, 1, 10, fp) == 9))
      { fclose (fp); return 10; }
    if (!(memcmp (buf, "foogarsh!", 9) == 0))
      { fclose (fp); return 11; }
  }
  if (!(fclose (fp) == 0))
    return 12;

  /* The file's contents is now "foogarsh!".  */

  return 0;
}]])],
            [gl_cv_func_ftello_works=yes],
            [gl_cv_func_ftello_works=no], [:])
        ])
      case "$gl_cv_func_ftello_works" in
        *yes) ;;
        *)
          REPLACE_FTELLO=1
          AC_DEFINE([FTELLO_BROKEN_AFTER_SWITCHING_FROM_READ_TO_WRITE], [1],
            [Define to 1 if the system's ftello function has the Solaris bug.])
          ;;
      esac
    fi
    if test $REPLACE_FTELLO = 0; then
      dnl Detect bug on macOS >= 10.15.
      gl_FUNC_UNGETC_WORKS
      if test $gl_ftello_broken_after_ungetc = yes; then
        REPLACE_FTELLO=1
        AC_DEFINE([FTELLO_BROKEN_AFTER_UNGETC], [1],
          [Define to 1 if the system's ftello function has the macOS bug.])
      fi
    fi
  fi
])

# Prerequisites of lib/ftello.c.
AC_DEFUN([gl_PREREQ_FTELLO],
[
  dnl Native Windows has the function _ftelli64. mingw hides it, but mingw64
  dnl makes it usable again.
  AC_CHECK_FUNCS([_ftelli64])
])
