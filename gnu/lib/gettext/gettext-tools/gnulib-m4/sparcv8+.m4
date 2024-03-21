# sparcv8+.m4 serial 2
dnl Copyright (C) 2020-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl When compiling for SPARC in 32-bit mode, make sure that instructions for
dnl SPARC v8+ are accepted.  This is necessary for multiprocessing (for
dnl instructions like 'membar' or 'cas').  All SPARC CPUs made since 1993
dnl support this instruction set.  But GCC in its default configuration, in
dnl 32-bit mode (64-bit mode assumes SPARC v9 or newer), still defaults to
dnl SPARC v7 instruction set: "By default (unless configured otherwise), GCC
dnl generates code for the V7 variant of the SPARC architecture."  See
dnl <https://gcc.gnu.org/onlinedocs/gcc-4.6.4/gcc/SPARC-Options.html>
dnl <https://gcc.gnu.org/onlinedocs/gcc-10.2.0/gcc/SPARC-Options.html>

AC_DEFUN([gl_SPARC_V8PLUS],
[
  AC_REQUIRE([AC_CANONICAL_HOST])

  case "$host" in
    sparc*-*-solaris*)
      if test -n "$GCC"; then
        AC_CACHE_CHECK([whether SPARC v8+ instructions are supported],
          [gl_cv_sparc_v8plus],
          [AC_COMPILE_IFELSE(
             [AC_LANG_PROGRAM(
                [[]],
                [[asm volatile ("membar 2");]])],
             [gl_cv_sparc_v8plus=yes],
             [gl_cv_sparc_v8plus=no])
          ])
        if test $gl_cv_sparc_v8plus = no; then
          dnl Strangely enough, '-mv8plus' does not have the desired effect.
          dnl But '-mcpu=v9' does.
          CC="$CC -mcpu=v9"
          CXX="$CXX -mcpu=v9"
        fi
      fi
      ;;
  esac
])
