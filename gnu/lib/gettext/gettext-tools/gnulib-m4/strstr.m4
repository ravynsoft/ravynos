# strstr.m4 serial 24
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Check that strstr works.
AC_DEFUN([gl_FUNC_STRSTR_SIMPLE],
[
  AC_REQUIRE([gl_STRING_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_MEMCHR])
  if test $REPLACE_MEMCHR = 1; then
    REPLACE_STRSTR=1
  else
    dnl Detect https://sourceware.org/bugzilla/show_bug.cgi?id=12092
    dnl and https://sourceware.org/bugzilla/show_bug.cgi?id=23637.
    AC_CACHE_CHECK([whether strstr works],
      [gl_cv_func_strstr_works_always],
      [AC_RUN_IFELSE(
         [AC_LANG_PROGRAM([[
#include <string.h> /* for __GNU_LIBRARY__, strstr */
#ifdef __GNU_LIBRARY__
 #include <features.h>
 #if __GLIBC__ == 2 && __GLIBC_MINOR__ == 28
  Unlucky user
 #endif
#endif
#define P "_EF_BF_BD"
#define HAYSTACK "F_BD_CE_BD" P P P P "_C3_88_20" P P P "_C3_A7_20" P
#define NEEDLE P P P P P
]],
            [[return !!strstr (HAYSTACK, NEEDLE);
            ]])],
         [gl_cv_func_strstr_works_always=yes],
         [gl_cv_func_strstr_works_always=no],
         [dnl glibc 2.12 and cygwin 1.7.7 have a known bug.  uClibc is not
          dnl affected, since it uses different source code for strstr than
          dnl glibc.
          dnl Assume that it works on all other platforms, even if it is not
          dnl linear.
          AC_EGREP_CPP([Lucky user],
            [
#include <string.h> /* for __GNU_LIBRARY__ */
#ifdef __GNU_LIBRARY__
 #include <features.h>
 #if ((__GLIBC__ == 2 && __GLIBC_MINOR__ > 12) || (__GLIBC__ > 2)) \
     || defined __UCLIBC__
  Lucky user
 #endif
#elif defined __CYGWIN__
 #include <cygwin/version.h>
 #if CYGWIN_VERSION_DLL_COMBINED > CYGWIN_VERSION_DLL_MAKE_COMBINED (1007, 7)
  Lucky user
 #endif
#else
  Lucky user
#endif
            ],
            [gl_cv_func_strstr_works_always="guessing yes"],
            [gl_cv_func_strstr_works_always="$gl_cross_guess_normal"])
         ])
      ])
    case "$gl_cv_func_strstr_works_always" in
      *yes) ;;
      *)
        REPLACE_STRSTR=1
        ;;
    esac
  fi
]) # gl_FUNC_STRSTR_SIMPLE

dnl Additionally, check that strstr is efficient.
AC_DEFUN([gl_FUNC_STRSTR],
[
  AC_REQUIRE([gl_FUNC_STRSTR_SIMPLE])
  if test $REPLACE_STRSTR = 0; then
    AC_CACHE_CHECK([whether strstr works in linear time],
      [gl_cv_func_strstr_linear],
      [AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef __MVS__
/* z/OS does not deliver signals while strstr() is running (thanks to
   restrictions on its LE runtime), which prevents us from limiting the
   running time of this test.  */
# error "This test does not work properly on z/OS"
#endif
#include <signal.h> /* for signal */
#include <string.h> /* for strstr */
#include <stdlib.h> /* for malloc */
#include <unistd.h> /* for alarm */
static void quit (int sig) { _exit (sig + 128); }
]], [[
    int result = 0;
    size_t m = 1000000;
    char *haystack = (char *) malloc (2 * m + 2);
    char *needle = (char *) malloc (m + 2);
    /* Failure to compile this test due to missing alarm is okay,
       since all such platforms (mingw) also have quadratic strstr.  */
    signal (SIGALRM, quit);
    alarm (5);
    /* Check for quadratic performance.  */
    if (haystack && needle)
      {
        memset (haystack, 'A', 2 * m);
        haystack[2 * m] = 'B';
        haystack[2 * m + 1] = 0;
        memset (needle, 'A', m);
        needle[m] = 'B';
        needle[m + 1] = 0;
        if (!strstr (haystack, needle))
          result |= 1;
      }
    /* Free allocated memory, in case some sanitizer is watching.  */
    free (haystack);
    free (needle);
    return result;
    ]])],
        [gl_cv_func_strstr_linear=yes], [gl_cv_func_strstr_linear=no],
        [dnl Only glibc > 2.12 on processors without SSE 4.2 instructions and
         dnl cygwin > 1.7.7 are known to have a bug-free strstr that works in
         dnl linear time.
         AC_EGREP_CPP([Lucky user],
           [
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if ((__GLIBC__ == 2 && __GLIBC_MINOR__ > 12) || (__GLIBC__ > 2)) \
     && !(defined __i386__ || defined __x86_64__) \
     && !defined __UCLIBC__
  Lucky user
 #endif
#endif
#ifdef __CYGWIN__
 #include <cygwin/version.h>
 #if CYGWIN_VERSION_DLL_COMBINED > CYGWIN_VERSION_DLL_MAKE_COMBINED (1007, 7)
  Lucky user
 #endif
#endif
           ],
           [gl_cv_func_strstr_linear="guessing yes"],
           [gl_cv_func_strstr_linear="$gl_cross_guess_normal"])
        ])
      ])
    case "$gl_cv_func_strstr_linear" in
      *yes) ;;
      *)
        REPLACE_STRSTR=1
        ;;
    esac
  fi
]) # gl_FUNC_STRSTR
