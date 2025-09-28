# calloc.m4 serial 31

# Copyright (C) 2004-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# Written by Jim Meyering.

# Determine whether calloc (N, S) returns non-NULL when N*S is zero,
# and returns NULL when N*S overflows.
# If so, define HAVE_CALLOC.  Otherwise, define calloc to rpl_calloc
# and arrange to use a calloc wrapper function that does work in that case.

# _AC_FUNC_CALLOC_IF([IF-WORKS], [IF-NOT])
# -------------------------------------
# If calloc is compatible with GNU calloc, run IF-WORKS, otherwise, IF-NOT.
AC_DEFUN([_AC_FUNC_CALLOC_IF],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether calloc (0, n) and calloc (n, 0) return nonnull],
    [ac_cv_func_calloc_0_nonnull],
    [if test $cross_compiling != yes; then
       ac_cv_func_calloc_0_nonnull=yes
       AC_RUN_IFELSE(
         [AC_LANG_PROGRAM(
            [AC_INCLUDES_DEFAULT],
            [[int result = 0;
              char * volatile p = calloc (0, 0);
              if (!p)
                result |= 1;
              free (p);
              return result;
            ]])],
         [],
         [ac_cv_func_calloc_0_nonnull=no])
     else
       case "$host_os" in
                             # Guess yes on glibc systems.
         *-gnu* | gnu*)      ac_cv_func_calloc_0_nonnull="guessing yes" ;;
                             # Guess yes on musl systems.
         *-musl* | midipix*) ac_cv_func_calloc_0_nonnull="guessing yes" ;;
                             # Guess yes on native Windows.
         mingw* | windows*)  ac_cv_func_calloc_0_nonnull="guessing yes" ;;
                             # If we don't know, obey --enable-cross-guesses.
         *)                  ac_cv_func_calloc_0_nonnull="$gl_cross_guess_normal" ;;
       esac
     fi
    ])
  AS_CASE([$ac_cv_func_calloc_0_nonnull], [*yes], [$1], [$2])
])


# gl_FUNC_CALLOC_GNU
# ------------------
# Replace calloc if it is not compatible with GNU libc.
AC_DEFUN([gl_FUNC_CALLOC_GNU],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_CALLOC_POSIX])
  REPLACE_CALLOC_FOR_CALLOC_GNU="$REPLACE_CALLOC_FOR_CALLOC_POSIX"
  if test $REPLACE_CALLOC_FOR_CALLOC_GNU = 0; then
    _AC_FUNC_CALLOC_IF([], [REPLACE_CALLOC_FOR_CALLOC_GNU=1])
  fi
])# gl_FUNC_CALLOC_GNU

# gl_FUNC_CALLOC_POSIX
# --------------------
# Test whether 'calloc' is POSIX compliant (sets errno to ENOMEM when it
# fails, and doesn't mess up with ptrdiff_t or size_t overflow),
# and replace calloc if it is not.
AC_DEFUN([gl_FUNC_CALLOC_POSIX],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_FUNC_MALLOC_POSIX])
  if test $REPLACE_MALLOC_FOR_MALLOC_POSIX = 1; then
    REPLACE_CALLOC_FOR_CALLOC_POSIX=1
  fi
  dnl Although in theory we should also test for size_t overflow,
  dnl in practice testing for ptrdiff_t overflow suffices
  dnl since PTRDIFF_MAX <= SIZE_MAX on all known Gnulib porting targets.
  dnl A separate size_t test would slow down 'configure'.
])
