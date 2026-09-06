# realloc.m4
# serial 40
dnl Copyright (C) 2007, 2009-2026 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

# An an experimental option, the user can request a sanitized realloc()
# implementation, i.e. one that aborts upon undefined behaviour,
# by setting
#   gl_cv_func_realloc_sanitize=yes
# at configure time.
AC_DEFUN([gl_FUNC_REALLOC_SANITIZED],
[
  AC_CACHE_CHECK([whether realloc should abort upon undefined behaviour],
    [gl_cv_func_realloc_sanitize],
    [test -n "$gl_cv_func_realloc_sanitize" || gl_cv_func_realloc_sanitize=no])
])

# gl_FUNC_REALLOC_POSIX
# ---------------------
# Test whether 'realloc' is POSIX compliant (sets errno to ENOMEM when it
# fails, and doesn't mess up with ptrdiff_t overflow),
# and replace realloc if it is not.
AC_DEFUN([gl_FUNC_REALLOC_POSIX],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_MALLOC_POSIX])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CACHE_CHECK([whether realloc sets errno on failure],
    [gl_cv_func_realloc_posix],
    [
      dnl FreeBSD 15.0 realloc() does not set errno when asked for more than
      dnl 0x7000000000000000 bytes.
      case "$host_os" in
        darwin* | freebsd* | dragonfly* | midnightbsd* | netbsd* | openbsd*)
          AC_RUN_IFELSE(
            [AC_LANG_SOURCE(
               [[#include <errno.h>
                 #include <stdlib.h>
                 int main (int argc, char **argv)
                 {
                   void *p;
                   errno = 1729;
                   p = realloc (malloc (1), (size_t)(-1) / 100 * 49);
                   return (!p && errno == 1729);
                 }
               ]])
            ],
            [gl_cv_func_realloc_posix=yes],
            [gl_cv_func_realloc_posix=no],
            [case "$host_os" in
               freebsd*) gl_cv_func_realloc_posix="guessing no" ;;
               *)        gl_cv_func_realloc_posix="guessing yes" ;;
             esac
            ])
          ;;
        *)
          gl_cv_func_realloc_posix="$gl_cv_func_malloc_posix"
          ;;
      esac
    ])
  case "$gl_cv_func_realloc_posix" in
    *yes)
      AC_DEFINE([HAVE_REALLOC_POSIX], [1],
        [Define if realloc sets errno on allocation failure.])
      ;;
    *)
      REPLACE_REALLOC_FOR_REALLOC_POSIX=1
      ;;
  esac
  AC_REQUIRE([gl_FUNC_REALLOC_SANITIZED])
  if test "$gl_cv_func_realloc_sanitize" != no; then
    REPLACE_REALLOC_FOR_REALLOC_POSIX=1
    AC_DEFINE([NEED_SANITIZED_REALLOC], [1],
      [Define to 1 if realloc should abort upon undefined behaviour.])
  fi
])

# gl_FUNC_REALLOC_0_NONNULL
# -------------------------
# Replace realloc if realloc (..., 0) returns null.
# Modules that use this macro directly or indirectly should depend
# on extensions-aix, so that _LINUX_SOURCE_COMPAT gets defined
# before this macro gets invoked.  This helps if !(__VEC__ || __AIXVEC),
# and doesn't hurt otherwise.
AC_DEFUN([gl_FUNC_REALLOC_0_NONNULL],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])dnl for cross-compiles
  AC_REQUIRE([gl_FUNC_REALLOC_POSIX])
  AC_CACHE_CHECK([whether realloc (..., 0) returns nonnull],
    [gl_cv_func_realloc_0_nonnull],
    [AC_RUN_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <stdlib.h>
            /* Use prealloc to test; "volatile" prevents the compiler
               from optimizing the realloc call away.  */
            void *(*volatile prealloc) (void *, size_t) = realloc;]],
          [[void *p = prealloc (0, 0);
            int result = !p;
            p = prealloc (p, 0);
            result |= !p;
            free (p);
            return result;]])],
       [gl_cv_func_realloc_0_nonnull=yes],
       [gl_cv_func_realloc_0_nonnull=no],
       [AS_CASE([$host_os],
          [# Guess yes on platforms where we know the result.
           freebsd* | netbsd* | openbsd* | darwin* | bitrig* \
           | *-musl* | midipix* | midnightbsd* \
           | hpux* | solaris* | cygwin*],
            [gl_cv_func_realloc_0_nonnull="guessing yes"],
          [# Guess as follows if we don't know.
           gl_cv_func_realloc_0_nonnull=$gl_cross_guess_normal])])])
  AS_CASE([$gl_cv_func_realloc_0_nonnull],
    [*yes],
      [AC_DEFINE([HAVE_REALLOC_0_NONNULL], [1],
         [Define to 1 if realloc (..., 0) returns nonnull.])],
    [AS_CASE([$gl_cv_func_realloc_sanitize,$gl_cv_malloc_ptrdiff,$gl_cv_func_malloc_posix,$host],
       [yes,*,*,* | *,no,*,* | *,*,*no,* | *,*,*,aarch64c-*-freebsd*],
         [REPLACE_REALLOC_FOR_REALLOC_POSIX=1],
       [# Optimize for common case of glibc 2.1.1+ and compatibles.
        REPLACE_REALLOC_FOR_REALLOC_POSIX=2])])
])
