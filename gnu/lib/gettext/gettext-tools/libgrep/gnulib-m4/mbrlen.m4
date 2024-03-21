# mbrlen.m4 serial 12  -*- coding: utf-8 -*-
dnl Copyright (C) 2008, 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_MBRLEN],
[
  AC_REQUIRE([gl_WCHAR_H_DEFAULTS])

  AC_REQUIRE([AC_TYPE_MBSTATE_T])
  AC_REQUIRE([gl_FUNC_MBRTOWC])
  AC_CHECK_FUNCS_ONCE([mbrlen])
  if test $ac_cv_func_mbrlen = no; then
    HAVE_MBRLEN=0
    AC_CHECK_DECLS([mbrlen],,, [[
      #include <wchar.h>
    ]])
    if test $ac_cv_have_decl_mbrlen = yes; then
      dnl On Minix 3.1.8, the system's <wchar.h> declares mbrlen() although
      dnl it does not have the function. Avoid a collision with gnulib's
      dnl replacement.
      REPLACE_MBRLEN=1
    fi
  else
    dnl Most bugs affecting the system's mbrtowc function also affect the
    dnl mbrlen function. So override mbrlen whenever mbrtowc is overridden.
    dnl We could also run the individual tests below; the results would be
    dnl the same.
    if test $REPLACE_MBRTOWC = 1; then
      REPLACE_MBRLEN=1
    fi
  fi
])

dnl Test whether mbrlen puts the state into non-initial state when parsing an
dnl incomplete multibyte character.
dnl Result is gl_cv_func_mbrlen_incomplete_state.

AC_DEFUN([gl_MBRLEN_INCOMPLETE_STATE],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([gt_LOCALE_JA])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether mbrlen handles incomplete characters],
    [gl_cv_func_mbrlen_incomplete_state],
    [
      dnl Initial guess, used when cross-compiling or when no suitable locale
      dnl is present.
changequote(,)dnl
      case "$host_os" in
                     # Guess no on AIX and OSF/1.
        aix* | osf*) gl_cv_func_mbrlen_incomplete_state="guessing no" ;;
                     # Guess yes otherwise.
        *)           gl_cv_func_mbrlen_incomplete_state="guessing yes" ;;
      esac
changequote([,])dnl
      if test $LOCALE_JA != none; then
        AC_RUN_IFELSE(
          [AC_LANG_SOURCE([[
#include <locale.h>
#include <string.h>
#include <wchar.h>
int main ()
{
  if (setlocale (LC_ALL, "$LOCALE_JA") != NULL)
    {
      const char input[] = "B\217\253\344\217\251\316er"; /* "Büßer" */
      mbstate_t state;

      memset (&state, '\0', sizeof (mbstate_t));
      if (mbrlen (input + 1, 1, &state) == (size_t)(-2))
        if (mbsinit (&state))
          return 1;
    }
  return 0;
}]])],
          [gl_cv_func_mbrlen_incomplete_state=yes],
          [gl_cv_func_mbrlen_incomplete_state=no],
          [])
      fi
    ])
])

dnl Test whether mbrlen, when parsing the end of a multibyte character,
dnl correctly returns the number of bytes that were needed to complete the
dnl character (not the total number of bytes of the multibyte character).
dnl Result is gl_cv_func_mbrlen_retval.

AC_DEFUN([gl_MBRLEN_RETVAL],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([gt_LOCALE_FR_UTF8])
  AC_REQUIRE([gt_LOCALE_JA])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether mbrlen has a correct return value],
    [gl_cv_func_mbrlen_retval],
    [
      dnl Initial guess, used when cross-compiling or when no suitable locale
      dnl is present.
changequote(,)dnl
      case "$host_os" in
                          # Guess no on HP-UX and Solaris.
        hpux* | solaris*) gl_cv_func_mbrlen_retval="guessing no" ;;
                          # Guess yes otherwise.
        *)                gl_cv_func_mbrlen_retval="guessing yes" ;;
      esac
changequote([,])dnl
      if test $LOCALE_FR_UTF8 != none || test $LOCALE_JA != none; then
        AC_RUN_IFELSE(
          [AC_LANG_SOURCE([[
#include <locale.h>
#include <string.h>
#include <wchar.h>
int main ()
{
  int result = 0;
  /* This fails on Solaris.  */
  if (strcmp ("$LOCALE_FR_UTF8", "none") != 0
      && setlocale (LC_ALL, "$LOCALE_FR_UTF8") != NULL)
    {
      char input[] = "B\303\274\303\237er"; /* "Büßer" */
      mbstate_t state;

      memset (&state, '\0', sizeof (mbstate_t));
      if (mbrlen (input + 1, 1, &state) == (size_t)(-2))
        {
          input[1] = '\0';
          if (mbrlen (input + 2, 5, &state) != 1)
            result |= 1;
        }
    }
  /* This fails on HP-UX 11.11.  */
  if (strcmp ("$LOCALE_JA", "none") != 0
      && setlocale (LC_ALL, "$LOCALE_JA") != NULL)
    {
      char input[] = "B\217\253\344\217\251\316er"; /* "Büßer" */
      mbstate_t state;

      memset (&state, '\0', sizeof (mbstate_t));
      if (mbrlen (input + 1, 1, &state) == (size_t)(-2))
        {
          input[1] = '\0';
          if (mbrlen (input + 2, 5, &state) != 2)
            result |= 2;
        }
    }
  return result;
}]])],
          [gl_cv_func_mbrlen_retval=yes],
          [gl_cv_func_mbrlen_retval=no],
          [])
      fi
    ])
])

dnl Test whether mbrlen, when parsing a NUL character, correctly returns 0.
dnl Result is gl_cv_func_mbrlen_nul_retval.

AC_DEFUN([gl_MBRLEN_NUL_RETVAL],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([gt_LOCALE_ZH_CN])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether mbrlen returns 0 when parsing a NUL character],
    [gl_cv_func_mbrlen_nul_retval],
    [
      dnl Initial guess, used when cross-compiling or when no suitable locale
      dnl is present.
changequote(,)dnl
      case "$host_os" in
                    # Guess no on Solaris 9.
        solaris2.9) gl_cv_func_mbrlen_nul_retval="guessing no" ;;
                    # Guess yes otherwise.
        *)          gl_cv_func_mbrlen_nul_retval="guessing yes" ;;
      esac
changequote([,])dnl
      if test $LOCALE_ZH_CN != none; then
        AC_RUN_IFELSE(
          [AC_LANG_SOURCE([[
#include <locale.h>
#include <string.h>
#include <wchar.h>
int main ()
{
  /* This crashes on Solaris 9 inside __mbrtowc_dense_gb18030.  */
  if (setlocale (LC_ALL, "$LOCALE_ZH_CN") != NULL)
    {
      mbstate_t state;

      memset (&state, '\0', sizeof (mbstate_t));
      if (mbrlen ("", 1, &state) != 0)
        return 1;
    }
  return 0;
}]])],
          [gl_cv_func_mbrlen_nul_retval=yes],
          [gl_cv_func_mbrlen_nul_retval=no],
          [])
      fi
    ])
])

dnl Test whether mbrlen returns the correct value on empty input.

AC_DEFUN([gl_MBRLEN_EMPTY_INPUT],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether mbrlen works on empty input],
    [gl_cv_func_mbrlen_empty_input],
    [
      dnl Initial guess, used when cross-compiling or when no suitable locale
      dnl is present.
changequote(,)dnl
      case "$host_os" in
                              # Guess no on AIX and glibc systems.
        aix* | *-gnu* | gnu*) gl_cv_func_mbrlen_empty_input="guessing no" ;;
        *)                    gl_cv_func_mbrlen_empty_input="guessing yes" ;;
      esac
changequote([,])dnl
      AC_RUN_IFELSE(
        [AC_LANG_SOURCE([[
           #include <wchar.h>
           static mbstate_t mbs;
           int
           main (void)
           {
             return mbrlen ("", 0, &mbs) != (size_t) -2;
           }]])],
        [gl_cv_func_mbrlen_empty_input=yes],
        [gl_cv_func_mbrlen_empty_input=no],
        [:])
    ])
])

# Prerequisites of lib/mbrlen.c.
AC_DEFUN([gl_PREREQ_MBRLEN], [
  :
])
