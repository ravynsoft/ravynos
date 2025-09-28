# vasnprintf.m4 serial 52
dnl Copyright (C) 2002-2004, 2006-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_VASNPRINTF],
[
  AC_CHECK_FUNCS_ONCE([vasnprintf])
  if test $ac_cv_func_vasnprintf = no; then
    gl_REPLACE_VASNPRINTF
  fi
])

AC_DEFUN([gl_REPLACE_VASNPRINTF],
[
  AC_CHECK_FUNCS_ONCE([vasnprintf])
  AC_LIBOBJ([vasnprintf])
  AC_LIBOBJ([printf-args])
  AC_LIBOBJ([printf-parse])
  AC_LIBOBJ([asnprintf])
  if test $ac_cv_func_vasnprintf = yes; then
    AC_DEFINE([REPLACE_VASNPRINTF], [1],
      [Define if vasnprintf exists but is overridden by gnulib.])
  fi
  gl_PREREQ_PRINTF_ARGS
  gl_PREREQ_PRINTF_PARSE
  gl_PREREQ_VASNPRINTF
  gl_PREREQ_ASNPRINTF
])

AC_DEFUN([gl_FUNC_VASNWPRINTF],
[
  AC_LIBOBJ([printf-args])
  gl_PREREQ_PRINTF_ARGS
  gl_PREREQ_PRINTF_PARSE
  gl_PREREQ_VASNWPRINTF
  gl_PREREQ_ASNPRINTF
])

# Prerequisites of lib/printf-args.h, lib/printf-args.c.
AC_DEFUN([gl_PREREQ_PRINTF_ARGS],
[
  AC_REQUIRE([gt_TYPE_WCHAR_T])
  AC_REQUIRE([gt_TYPE_WINT_T])
])

# Prerequisites of lib/printf-parse.h, lib/printf-parse.c.
# Prerequisites of lib/wprintf-parse.h, lib/wprintf-parse.c.
AC_DEFUN([gl_PREREQ_PRINTF_PARSE],
[
  AC_REQUIRE([gl_FEATURES_H])
  AC_REQUIRE([gt_TYPE_WCHAR_T])
  AC_REQUIRE([gt_TYPE_WINT_T])
  AC_REQUIRE([AC_TYPE_SIZE_T])
  AC_CHECK_TYPE([ptrdiff_t], ,
    [AC_DEFINE([ptrdiff_t], [long],
       [Define as the type of the result of subtracting two pointers, if the system doesn't define it.])
    ])
  AC_REQUIRE([gt_AC_TYPE_INTMAX_T])
])

# Prerequisites of lib/vasnprintf.c if !WIDE_CHAR_VERSION.
AC_DEFUN_ONCE([gl_PREREQ_VASNPRINTF],
[
  AC_CHECK_FUNCS([snprintf strnlen wcrtomb])
  dnl Use the _snprintf function only if it is declared (because on NetBSD it
  dnl is defined as a weak alias of snprintf; we prefer to use the latter).
  AC_CHECK_DECLS([_snprintf], , , [[#include <stdio.h>]])
  dnl We can avoid a lot of code by assuming that snprintf's return value
  dnl conforms to ISO C99. So check that.
  AC_REQUIRE([gl_SNPRINTF_RETVAL_C99])
  case "$gl_cv_func_snprintf_retval_c99" in
    *yes)
      AC_DEFINE([HAVE_SNPRINTF_RETVAL_C99], [1],
        [Define if the return value of the snprintf function is the number of
         of bytes (excluding the terminating NUL) that would have been produced
         if the buffer had been large enough.])
      ;;
  esac
  dnl Additionally, the use of %n can be eliminated by assuming that snprintf
  dnl always produces NUL-terminated strings (no truncation).
  AC_REQUIRE([gl_SNPRINTF_TRUNCATION_C99])
  case "$gl_cv_func_snprintf_truncation_c99" in
    *yes)
      AC_DEFINE([HAVE_SNPRINTF_TRUNCATION_C99], [1],
        [Define if the string produced by the snprintf function is always NUL
         terminated.])
      ;;
  esac
  gl_PREREQ_VASNXPRINTF
])

# Prerequisites of lib/vasnwprintf.c.
AC_DEFUN_ONCE([gl_PREREQ_VASNWPRINTF],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CHECK_FUNCS_ONCE([swprintf wcsnlen mbrtowc])
  AC_CHECK_DECLS([_snwprintf], , , [[#include <stdio.h>]])
  AC_CHECK_DECLS([wcsnlen], , , [[#include <wchar.h>]])
  gl_SWPRINTF_WORKS
  case "$gl_cv_func_swprintf_works" in
    *yes)
      AC_DEFINE([HAVE_WORKING_SWPRINTF], [1],
        [Define if the swprintf function works correctly when it produces output
         that contains null wide characters.])
      ;;
  esac
  gl_MBRTOWC_C_LOCALE
  case "$gl_cv_func_mbrtowc_C_locale_sans_EILSEQ" in
    *yes)
      AC_CACHE_CHECK([whether swprintf in the C locale is free of encoding errors],
        [gl_cv_func_swprintf_C_locale_sans_EILSEQ],
        [
          AC_RUN_IFELSE(
            [AC_LANG_SOURCE([[
#ifndef __USE_MINGW_ANSI_STDIO
# define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <wchar.h>
int main()
{
  int result = 0;
  { /* This test fails on glibc 2.35, musl libc 1.2.4, FreeBSD 13.2, NetBSD 9.3,
       OpenBSD 7.2, Cygwin 2.9.0.
       Reported at <https://www.openwall.com/lists/musl/2023/06/12/2>.  */
    wchar_t buf[12];
    int ret = swprintf (buf, 12, L"%c", '\377');
    if (ret < 0)
      result |= 1;
  }
  return result;
}]])],
            [gl_cv_func_swprintf_C_locale_sans_EILSEQ=yes],
            [gl_cv_func_swprintf_C_locale_sans_EILSEQ=no],
            [case "$host_os" in
                                   # Guess no on glibc systems.
               *-gnu* | gnu*)      gl_cv_func_swprintf_C_locale_sans_EILSEQ="guessing yes";;
                                   # Guess no on musl systems.
               *-musl* | midipix*) gl_cv_func_swprintf_C_locale_sans_EILSEQ="guessing no";;
                                   # If we don't know, obey --enable-cross-guesses.
               *)                  gl_cv_func_swprintf_C_locale_sans_EILSEQ="$gl_cross_guess_normal";;
             esac
            ])
        ])
      ;;
  esac
  if case "$gl_cv_func_mbrtowc_C_locale_sans_EILSEQ" in
       *yes) false ;;
       *) true ;;
     esac \
     || case "$gl_cv_func_swprintf_C_locale_sans_EILSEQ" in
          *yes) false ;;
          *) true ;;
        esac; then
    AC_DEFINE([NEED_WPRINTF_DIRECTIVE_C], [1],
      [Define if the vasnwprintf implementation needs special code for
       the 'c' directive.])
  fi
  gl_SWPRINTF_DIRECTIVE_LA
  case "$gl_cv_func_swprintf_directive_la" in
    *yes) ;;
    *)
      AC_DEFINE([NEED_WPRINTF_DIRECTIVE_LA], [1],
        [Define if the vasnwprintf implementation needs special code for
         the 'a' directive with 'long double' arguments.])
      ;;
  esac
  gl_SWPRINTF_DIRECTIVE_LC
  case "$gl_cv_func_swprintf_directive_lc" in
    *yes) ;;
    *)
      AC_DEFINE([NEED_WPRINTF_DIRECTIVE_LC], [1],
        [Define if the vasnwprintf implementation needs special code for
         the 'lc' directive.])
      ;;
  esac
  gl_MUSL_LIBC
  gl_PREREQ_VASNXPRINTF
])

# Common prerequisites of lib/vasnprintf.c and lib/vasnwprintf.c.
AC_DEFUN_ONCE([gl_PREREQ_VASNXPRINTF],
[
  AC_REQUIRE([AC_FUNC_ALLOCA])
  AC_REQUIRE([gt_TYPE_WCHAR_T])
  AC_REQUIRE([gt_TYPE_WINT_T])
  AC_CHECK_FUNCS([wcslen])
  dnl Knowing DBL_EXPBIT0_WORD and DBL_EXPBIT0_BIT enables an optimization
  dnl in the code for NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_DOUBLE.
  AC_REQUIRE([gl_DOUBLE_EXPONENT_LOCATION])
])

# Extra prerequisites of lib/vasnprintf.c for supporting 'long double'
# arguments.
AC_DEFUN_ONCE([gl_PREREQ_VASNPRINTF_LONG_DOUBLE],
[
  AC_REQUIRE([gl_PRINTF_LONG_DOUBLE])
  case "$gl_cv_func_printf_long_double" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_LONG_DOUBLE], [1],
        [Define if the vasnprintf implementation needs special code for
         'long double' arguments.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting infinite 'double'
# arguments.
AC_DEFUN([gl_PREREQ_VASNPRINTF_INFINITE_DOUBLE],
[
  AC_REQUIRE([gl_PRINTF_INFINITE])
  case "$gl_cv_func_printf_infinite" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_INFINITE_DOUBLE], [1],
        [Define if the vasnprintf implementation needs special code for
         infinite 'double' arguments.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting infinite 'long double'
# arguments.
AC_DEFUN([gl_PREREQ_VASNPRINTF_INFINITE_LONG_DOUBLE],
[
  AC_REQUIRE([gl_PRINTF_INFINITE_LONG_DOUBLE])
  dnl There is no need to set NEED_PRINTF_INFINITE_LONG_DOUBLE if
  dnl NEED_PRINTF_LONG_DOUBLE is already set.
  AC_REQUIRE([gl_PREREQ_VASNPRINTF_LONG_DOUBLE])
  case "$gl_cv_func_printf_long_double" in
    *yes)
      case "$gl_cv_func_printf_infinite_long_double" in
        *yes)
          ;;
        *)
          AC_DEFINE([NEED_PRINTF_INFINITE_LONG_DOUBLE], [1],
            [Define if the vasnprintf implementation needs special code for
             infinite 'long double' arguments.])
          ;;
      esac
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the 'a' directive.
AC_DEFUN([gl_PREREQ_VASNPRINTF_DIRECTIVE_A],
[
  AC_REQUIRE([gl_PRINTF_DIRECTIVE_A])
  case "$gl_cv_func_printf_directive_a" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_DIRECTIVE_A], [1],
        [Define if the vasnprintf implementation needs special code for
         the 'a' and 'A' directives.])
      gl_CHECK_FUNCS_ANDROID([nl_langinfo], [[#include <langinfo.h>]])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the 'b' directive.
AC_DEFUN([gl_PREREQ_VASNPRINTF_DIRECTIVE_B],
[
  AC_REQUIRE([gl_PRINTF_DIRECTIVE_B])
  case "$gl_cv_func_printf_directive_b" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_DIRECTIVE_B], [1],
        [Define if the vasnprintf implementation needs special code for
         the 'b' directive.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the 'F' directive.
AC_DEFUN([gl_PREREQ_VASNPRINTF_DIRECTIVE_F],
[
  AC_REQUIRE([gl_PRINTF_DIRECTIVE_F])
  case "$gl_cv_func_printf_directive_f" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_DIRECTIVE_F], [1],
        [Define if the vasnprintf implementation needs special code for
         the 'F' directive.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the 'ls' directive.
AC_DEFUN([gl_PREREQ_VASNPRINTF_DIRECTIVE_LS],
[
  AC_REQUIRE([gl_PRINTF_DIRECTIVE_LS])
  case "$gl_cv_func_printf_directive_ls" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_DIRECTIVE_LS], [1],
        [Define if the vasnprintf implementation needs special code for
         the 'ls' directive.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the 'lc' directive.
AC_DEFUN([gl_PREREQ_VASNPRINTF_DIRECTIVE_LC],
[
  AC_REQUIRE([gl_PRINTF_DIRECTIVE_LC])
  case "$gl_cv_func_printf_directive_lc" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_DIRECTIVE_LC], [1],
        [Define if the vasnprintf implementation needs special code for
         the 'lc' directive.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the ' flag.
AC_DEFUN([gl_PREREQ_VASNPRINTF_FLAG_GROUPING],
[
  AC_REQUIRE([gl_PRINTF_FLAG_GROUPING])
  case "$gl_cv_func_printf_flag_grouping" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_FLAG_GROUPING], [1],
        [Define if the vasnprintf implementation needs special code for the
         ' flag.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the '-' flag.
AC_DEFUN([gl_PREREQ_VASNPRINTF_FLAG_LEFTADJUST],
[
  AC_REQUIRE([gl_PRINTF_FLAG_LEFTADJUST])
  case "$gl_cv_func_printf_flag_leftadjust" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_FLAG_LEFTADJUST], [1],
        [Define if the vasnprintf implementation needs special code for the
         '-' flag.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the 0 flag.
AC_DEFUN([gl_PREREQ_VASNPRINTF_FLAG_ZERO],
[
  AC_REQUIRE([gl_PRINTF_FLAG_ZERO])
  case "$gl_cv_func_printf_flag_zero" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_FLAG_ZERO], [1],
        [Define if the vasnprintf implementation needs special code for the
         0 flag.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting the # flag with a
# zero precision and a zero value in the 'x' and 'X' directives.
AC_DEFUN([gl_PREREQ_VASNPRINTF_FLAG_ALT_PRECISION_ZERO],
[
  AC_REQUIRE([gl_PRINTF_FLAG_ALT_PRECISION_ZERO])
  case "$gl_cv_func_printf_flag_alt_precision_zero" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_FLAG_ALT_PRECISION_ZERO], [1],
        [Define if the vasnprintf implementation needs special code for the
         # flag with a zero precision and a zero value in the 'x' and 'X' directives.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for supporting large precisions.
AC_DEFUN([gl_PREREQ_VASNPRINTF_PRECISION],
[
  AC_REQUIRE([gl_PRINTF_PRECISION])
  case "$gl_cv_func_printf_precision" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_UNBOUNDED_PRECISION], [1],
        [Define if the vasnprintf implementation needs special code for
         supporting large precisions without arbitrary bounds.])
      AC_DEFINE([NEED_PRINTF_DOUBLE], [1],
        [Define if the vasnprintf implementation needs special code for
         'double' arguments.])
      AC_DEFINE([NEED_PRINTF_LONG_DOUBLE], [1],
        [Define if the vasnprintf implementation needs special code for
         'long double' arguments.])
      ;;
  esac
])

# Extra prerequisites of lib/vasnprintf.c for surviving out-of-memory
# conditions.
AC_DEFUN([gl_PREREQ_VASNPRINTF_ENOMEM],
[
  AC_REQUIRE([gl_PRINTF_ENOMEM])
  case "$gl_cv_func_printf_enomem" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_ENOMEM], [1],
        [Define if the vasnprintf implementation needs special code for
         surviving out-of-memory conditions.])
      AC_DEFINE([NEED_PRINTF_DOUBLE], [1],
        [Define if the vasnprintf implementation needs special code for
         'double' arguments.])
      AC_DEFINE([NEED_PRINTF_LONG_DOUBLE], [1],
        [Define if the vasnprintf implementation needs special code for
         'long double' arguments.])
      ;;
  esac
])

# Prerequisites of lib/vasnprintf.c including all extras for POSIX compliance.
AC_DEFUN([gl_PREREQ_VASNPRINTF_WITH_POSIX_EXTRAS],
[
  AC_REQUIRE([gl_PREREQ_VASNPRINTF])
  gl_PREREQ_VASNPRINTF_LONG_DOUBLE
  gl_PREREQ_VASNPRINTF_INFINITE_DOUBLE
  gl_PREREQ_VASNPRINTF_INFINITE_LONG_DOUBLE
  gl_PREREQ_VASNPRINTF_DIRECTIVE_A
  gl_PREREQ_VASNPRINTF_DIRECTIVE_B
  gl_PREREQ_VASNPRINTF_DIRECTIVE_F
  gl_PREREQ_VASNPRINTF_DIRECTIVE_LS
  gl_PREREQ_VASNPRINTF_DIRECTIVE_LC
  gl_PREREQ_VASNPRINTF_FLAG_GROUPING
  gl_PREREQ_VASNPRINTF_FLAG_LEFTADJUST
  gl_PREREQ_VASNPRINTF_FLAG_ZERO
  gl_PREREQ_VASNPRINTF_FLAG_ALT_PRECISION_ZERO
  gl_PREREQ_VASNPRINTF_PRECISION
  gl_PREREQ_VASNPRINTF_ENOMEM
])

# Extra prerequisites of lib/vasnprintf.c for supporting the 'B' directive.
AC_DEFUN([gl_PREREQ_VASNPRINTF_DIRECTIVE_UPPERCASE_B],
[
  AC_REQUIRE([gl_PRINTF_DIRECTIVE_UPPERCASE_B])
  case "$gl_cv_func_printf_directive_uppercase_b" in
    *yes)
      ;;
    *)
      AC_DEFINE([NEED_PRINTF_DIRECTIVE_UPPERCASE_B], [1],
        [Define if the vasnprintf implementation needs special code for
         the 'B' directive.])
      ;;
  esac
])

# Prerequisites of lib/vasnprintf.c including all extras for POSIX compliance
# and GNU compatibility.
AC_DEFUN([gl_PREREQ_VASNPRINTF_WITH_GNU_EXTRAS],
[
  gl_PREREQ_VASNPRINTF_WITH_POSIX_EXTRAS
  AC_DEFINE([SUPPORT_GNU_PRINTF_DIRECTIVES], [1],
    [Define if the vasnprintf implementation should support GNU compatible
     printf directives.])
  gl_PREREQ_VASNPRINTF_DIRECTIVE_UPPERCASE_B
])

# Prerequisites of lib/asnprintf.c.
# Prerequisites of lib/asnwprintf.c.
AC_DEFUN([gl_PREREQ_ASNPRINTF],
[
])
