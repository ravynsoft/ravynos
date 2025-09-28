# fstatat.m4 serial 4
dnl Copyright (C) 2004-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Written by Jim Meyering.

# If we have the fstatat function, and it has the bug (in AIX 7.1)
# that it does not fill in st_size correctly, use the replacement function.
AC_DEFUN([gl_FUNC_FSTATAT],
[
  AC_REQUIRE([gl_SYS_STAT_H_DEFAULTS])
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_REQUIRE([gl_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_FUNCS_ONCE([fstatat])

  if test $ac_cv_func_fstatat = no; then
    HAVE_FSTATAT=0
  else
    dnl Test for an AIX 7.1 bug; see
    dnl <https://lists.gnu.org/r/bug-tar/2011-09/msg00015.html>.
    AC_CACHE_CHECK([whether fstatat (..., 0) works],
      [gl_cv_func_fstatat_zero_flag],
      [AC_RUN_IFELSE(
         [AC_LANG_SOURCE(
            [[
              #include <fcntl.h>
              #include <sys/stat.h>
              int
              main (void)
              {
                struct stat a;
                return fstatat (AT_FDCWD, ".", &a, 0) != 0;
              }
            ]])],
         [gl_cv_func_fstatat_zero_flag=yes],
         [gl_cv_func_fstatat_zero_flag=no],
         [case "$host_os" in
            aix*) gl_cv_func_fstatat_zero_flag="guessing no";;
            *)    gl_cv_func_fstatat_zero_flag="guessing yes";;
          esac
         ])
      ])

    case $gl_cv_func_fstatat_zero_flag+$gl_cv_func_lstat_dereferences_slashed_symlink in
    *yes+*yes) ;;
    *) REPLACE_FSTATAT=1 ;;
    esac

    case $host_os in
      solaris*)
        REPLACE_FSTATAT=1 ;;
    esac

    case $REPLACE_FSTATAT,$gl_cv_func_fstatat_zero_flag in
      1,*yes)
         AC_DEFINE([HAVE_WORKING_FSTATAT_ZERO_FLAG], [1],
           [Define to 1 if fstatat (..., 0) works.
            For example, it does not work in AIX 7.1.])
         ;;
    esac
  fi
])
