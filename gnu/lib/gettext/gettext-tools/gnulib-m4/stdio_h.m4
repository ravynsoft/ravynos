# stdio_h.m4 serial 63
dnl Copyright (C) 2007-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_STDIO_H_EARLY],
[
  dnl Defining __USE_MINGW_ANSI_STDIO to 1 must be done early, because
  dnl the results of several configure tests depend on it: The tests
  dnl   - checking whether snprintf returns a byte count as in C99...
  dnl   - checking whether snprintf truncates the result as in C99...
  dnl   - checking whether printf supports the 'F' directive...
  dnl   - checking whether printf supports the grouping flag...
  dnl   - checking whether printf supports the zero flag correctly...
  dnl   - checking whether printf supports infinite 'double' arguments...
  dnl   - checking whether printf supports large precisions...
  dnl report 'yes' if __USE_MINGW_ANSI_STDIO is 1 but 'no' if
  dnl __USE_MINGW_ANSI_STDIO is not set.
  AH_VERBATIM([MINGW_ANSI_STDIO],
[/* Use GNU style printf and scanf.  */
#ifndef __USE_MINGW_ANSI_STDIO
# undef __USE_MINGW_ANSI_STDIO
#endif
])
  AC_DEFINE([__USE_MINGW_ANSI_STDIO])
])

AC_DEFUN_ONCE([gl_STDIO_H],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  gl_NEXT_HEADERS([stdio.h])

  dnl Determine whether __USE_MINGW_ANSI_STDIO makes printf and
  dnl inttypes.h behave like gnu instead of system; we must give our
  dnl printf wrapper the right attribute to match.
  AC_CACHE_CHECK([which flavor of printf attribute matches inttypes macros],
    [gl_cv_func_printf_attribute_flavor],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
       #define __STDC_FORMAT_MACROS 1
       #include <stdio.h>
       #include <inttypes.h>
       /* For non-mingw systems, compilation will trivially succeed.
          For mingw, compilation will succeed for older mingw (system
          printf, "I64d") and fail for newer mingw (gnu printf, "lld"). */
       #if (defined _WIN32 && ! defined __CYGWIN__) && \
         (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4))
       extern char PRIdMAX_probe[sizeof PRIdMAX == sizeof "I64d" ? 1 : -1];
       #endif
      ]])], [gl_cv_func_printf_attribute_flavor=system],
      [gl_cv_func_printf_attribute_flavor=gnu])])
  if test "$gl_cv_func_printf_attribute_flavor" = gnu; then
    AC_DEFINE([GNULIB_PRINTF_ATTRIBUTE_FLAVOR_GNU], [1],
      [Define to 1 if printf and friends should be labeled with
       attribute "__gnu_printf__" instead of "__printf__"])
  fi

  dnl For defining _PRINTF_NAN_LEN_MAX.
  gl_MUSL_LIBC

  dnl This ifdef is an optimization, to avoid performing a configure check whose
  dnl result is not used. But it does not make the test of
  dnl GNULIB_STDIO_H_NONBLOCKING or GNULIB_NONBLOCKING redundant.
  m4_ifdef([gl_NONBLOCKING_IO], [
    gl_NONBLOCKING_IO
    if test $gl_cv_have_nonblocking != yes; then
      REPLACE_STDIO_READ_FUNCS=1
    fi
  ])

  dnl This ifdef is an optimization, to avoid performing a configure check whose
  dnl result is not used. But it does not make the test of
  dnl GNULIB_STDIO_H_SIGPIPE or GNULIB_SIGPIPE redundant.
  m4_ifdef([gl_SIGNAL_SIGPIPE], [
    gl_SIGNAL_SIGPIPE
    if test $gl_cv_header_signal_h_SIGPIPE != yes; then
      REPLACE_STDIO_WRITE_FUNCS=1
    fi
  ])
  dnl This ifdef is an optimization, to avoid performing a configure check whose
  dnl result is not used. But it does not make the test of
  dnl GNULIB_STDIO_H_NONBLOCKING or GNULIB_NONBLOCKING redundant.
  m4_ifdef([gl_NONBLOCKING_IO], [
    gl_NONBLOCKING_IO
    if test $gl_cv_have_nonblocking != yes; then
      REPLACE_STDIO_WRITE_FUNCS=1
    fi
  ])

  dnl Check for declarations of anything we want to poison if the
  dnl corresponding gnulib module is not in use, and which is not
  dnl guaranteed by both C89 and C11.
  gl_WARN_ON_USE_PREPARE([[#include <stdio.h>
    ]], [dprintf fpurge fseeko ftello getdelim getline gets pclose popen
    renameat snprintf tmpfile vdprintf vsnprintf])

  AC_REQUIRE([AC_C_RESTRICT])

  AC_CHECK_DECLS_ONCE([fcloseall])
  if test $ac_cv_have_decl_fcloseall = no; then
    HAVE_DECL_FCLOSEALL=0
  fi

  AC_CHECK_DECLS_ONCE([getw])
  if test $ac_cv_have_decl_getw = no; then
    HAVE_DECL_GETW=0
  fi

  AC_CHECK_DECLS_ONCE([putw])
  if test $ac_cv_have_decl_putw = no; then
    HAVE_DECL_PUTW=0
  fi
])

# gl_STDIO_MODULE_INDICATOR([modulename])
# sets the shell variable that indicates the presence of the given module
# to a C preprocessor expression that will evaluate to 1.
# This macro invocation must not occur in macros that are AC_REQUIREd.
AC_DEFUN([gl_STDIO_MODULE_INDICATOR],
[
  dnl Ensure to expand the default settings once only.
  gl_STDIO_H_REQUIRE_DEFAULTS
  gl_MODULE_INDICATOR_SET_VARIABLE([$1])
  dnl Define it also as a C macro, for the benefit of the unit tests.
  gl_MODULE_INDICATOR_FOR_TESTS([$1])
])

# Initializes the default values for AC_SUBSTed shell variables.
# This macro must not be AC_REQUIREd.  It must only be invoked, and only
# outside of macros or in macros that are not AC_REQUIREd.
AC_DEFUN([gl_STDIO_H_REQUIRE_DEFAULTS],
[
  m4_defun(GL_MODULE_INDICATOR_PREFIX[_STDIO_H_MODULE_INDICATOR_DEFAULTS], [
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_DPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FCLOSE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FDOPEN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FFLUSH])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FGETC])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FGETS])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FOPEN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FOPEN_GNU])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FPRINTF_POSIX])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FPURGE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FPUTC])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FPUTS])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FREAD])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FREOPEN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FSCANF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FSEEK])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FSEEKO])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FTELL])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FTELLO])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_FWRITE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_GETC])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_GETCHAR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_GETDELIM])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_GETLINE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_OBSTACK_PRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_OBSTACK_PRINTF_POSIX])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PCLOSE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PERROR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_POPEN])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PRINTF_POSIX])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PUTC])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PUTCHAR])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_PUTS])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_REMOVE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_RENAME])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_RENAMEAT])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SCANF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SNPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_SPRINTF_POSIX])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_STDIO_H_NONBLOCKING])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_STDIO_H_SIGPIPE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_TMPFILE])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VASPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VFSCANF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VSCANF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VDPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VFPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VFPRINTF_POSIX])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VPRINTF_POSIX])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VSNPRINTF])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_VSPRINTF_POSIX])
    dnl Support Microsoft deprecated alias function names by default.
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_FCLOSEALL], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_FDOPEN], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_FILENO], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_GETW], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_PUTW], [1])
    gl_MODULE_INDICATOR_INIT_VARIABLE([GNULIB_MDA_TEMPNAM], [1])
  ])
  m4_require(GL_MODULE_INDICATOR_PREFIX[_STDIO_H_MODULE_INDICATOR_DEFAULTS])
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
])

AC_DEFUN([gl_STDIO_H_DEFAULTS],
[
  dnl Assume proper GNU behavior unless another module says otherwise.
  HAVE_DECL_FCLOSEALL=1;         AC_SUBST([HAVE_DECL_FCLOSEALL])
  HAVE_DECL_FPURGE=1;            AC_SUBST([HAVE_DECL_FPURGE])
  HAVE_DECL_FSEEKO=1;            AC_SUBST([HAVE_DECL_FSEEKO])
  HAVE_DECL_FTELLO=1;            AC_SUBST([HAVE_DECL_FTELLO])
  HAVE_DECL_GETDELIM=1;          AC_SUBST([HAVE_DECL_GETDELIM])
  HAVE_DECL_GETLINE=1;           AC_SUBST([HAVE_DECL_GETLINE])
  HAVE_DECL_GETW=1;              AC_SUBST([HAVE_DECL_GETW])
  HAVE_DECL_OBSTACK_PRINTF=1;    AC_SUBST([HAVE_DECL_OBSTACK_PRINTF])
  HAVE_DECL_PUTW=1;              AC_SUBST([HAVE_DECL_PUTW])
  HAVE_DECL_SNPRINTF=1;          AC_SUBST([HAVE_DECL_SNPRINTF])
  HAVE_DECL_VSNPRINTF=1;         AC_SUBST([HAVE_DECL_VSNPRINTF])
  HAVE_DPRINTF=1;                AC_SUBST([HAVE_DPRINTF])
  HAVE_FSEEKO=1;                 AC_SUBST([HAVE_FSEEKO])
  HAVE_FTELLO=1;                 AC_SUBST([HAVE_FTELLO])
  HAVE_PCLOSE=1;                 AC_SUBST([HAVE_PCLOSE])
  HAVE_POPEN=1;                  AC_SUBST([HAVE_POPEN])
  HAVE_RENAMEAT=1;               AC_SUBST([HAVE_RENAMEAT])
  HAVE_VASPRINTF=1;              AC_SUBST([HAVE_VASPRINTF])
  HAVE_VDPRINTF=1;               AC_SUBST([HAVE_VDPRINTF])
  REPLACE_DPRINTF=0;             AC_SUBST([REPLACE_DPRINTF])
  REPLACE_FCLOSE=0;              AC_SUBST([REPLACE_FCLOSE])
  REPLACE_FDOPEN=0;              AC_SUBST([REPLACE_FDOPEN])
  REPLACE_FFLUSH=0;              AC_SUBST([REPLACE_FFLUSH])
  REPLACE_FOPEN=0;               AC_SUBST([REPLACE_FOPEN])
  REPLACE_FOPEN_FOR_FOPEN_GNU=0; AC_SUBST([REPLACE_FOPEN_FOR_FOPEN_GNU])
  REPLACE_FPRINTF=0;             AC_SUBST([REPLACE_FPRINTF])
  REPLACE_FPURGE=0;              AC_SUBST([REPLACE_FPURGE])
  REPLACE_FREOPEN=0;             AC_SUBST([REPLACE_FREOPEN])
  REPLACE_FSEEK=0;               AC_SUBST([REPLACE_FSEEK])
  REPLACE_FSEEKO=0;              AC_SUBST([REPLACE_FSEEKO])
  REPLACE_FTELL=0;               AC_SUBST([REPLACE_FTELL])
  REPLACE_FTELLO=0;              AC_SUBST([REPLACE_FTELLO])
  REPLACE_GETDELIM=0;            AC_SUBST([REPLACE_GETDELIM])
  REPLACE_GETLINE=0;             AC_SUBST([REPLACE_GETLINE])
  REPLACE_OBSTACK_PRINTF=0;      AC_SUBST([REPLACE_OBSTACK_PRINTF])
  REPLACE_PERROR=0;              AC_SUBST([REPLACE_PERROR])
  REPLACE_POPEN=0;               AC_SUBST([REPLACE_POPEN])
  REPLACE_PRINTF=0;              AC_SUBST([REPLACE_PRINTF])
  REPLACE_REMOVE=0;              AC_SUBST([REPLACE_REMOVE])
  REPLACE_RENAME=0;              AC_SUBST([REPLACE_RENAME])
  REPLACE_RENAMEAT=0;            AC_SUBST([REPLACE_RENAMEAT])
  REPLACE_SNPRINTF=0;            AC_SUBST([REPLACE_SNPRINTF])
  REPLACE_SPRINTF=0;             AC_SUBST([REPLACE_SPRINTF])
  REPLACE_STDIO_READ_FUNCS=0;    AC_SUBST([REPLACE_STDIO_READ_FUNCS])
  REPLACE_STDIO_WRITE_FUNCS=0;   AC_SUBST([REPLACE_STDIO_WRITE_FUNCS])
  REPLACE_TMPFILE=0;             AC_SUBST([REPLACE_TMPFILE])
  REPLACE_VASPRINTF=0;           AC_SUBST([REPLACE_VASPRINTF])
  REPLACE_VDPRINTF=0;            AC_SUBST([REPLACE_VDPRINTF])
  REPLACE_VFPRINTF=0;            AC_SUBST([REPLACE_VFPRINTF])
  REPLACE_VPRINTF=0;             AC_SUBST([REPLACE_VPRINTF])
  REPLACE_VSNPRINTF=0;           AC_SUBST([REPLACE_VSNPRINTF])
  REPLACE_VSPRINTF=0;            AC_SUBST([REPLACE_VSPRINTF])
])
