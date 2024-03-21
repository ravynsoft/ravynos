# Checks for stat-related time functions.

# Copyright (C) 1998-1999, 2001, 2003, 2005-2007, 2009-2012 Free Software
# Foundation, Inc.

# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl From Paul Eggert.
dnl Modified by Chet Ramey for bash.

# st_atim.tv_nsec - Linux, Solaris, Cygwin
# st_atimespec.tv_nsec - FreeBSD, NetBSD, if ! defined _POSIX_SOURCE
# st_atimensec - FreeBSD, NetBSD, if defined _POSIX_SOURCE
# st_atim.st__tim.tv_nsec - UnixWare (at least 2.1.2 through 7.1)

# st_birthtimespec - FreeBSD, NetBSD (hidden on OpenBSD 3.9, anyway)
# st_birthtim - Cygwin 1.7.0+

AC_DEFUN([BASH_STAT_TIME],
[
  AC_REQUIRE([AC_C_INLINE])
  AC_CHECK_HEADERS_ONCE([sys/time.h])

  AC_CHECK_MEMBERS([struct stat.st_atim.tv_nsec],
    [AC_CACHE_CHECK([whether struct stat.st_atim is of type struct timespec],
       [ac_cv_typeof_struct_stat_st_atim_is_struct_timespec],
       [AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
          [[
            #include <sys/types.h>
            #include <sys/stat.h>
            #if HAVE_SYS_TIME_H
            # include <sys/time.h>
            #endif
            #include <time.h>
            struct timespec ts;
            struct stat st;
          ]],
          [[
            st.st_atim = ts;
          ]])],
          [ac_cv_typeof_struct_stat_st_atim_is_struct_timespec=yes],
          [ac_cv_typeof_struct_stat_st_atim_is_struct_timespec=no])])
     if test $ac_cv_typeof_struct_stat_st_atim_is_struct_timespec = yes; then
       AC_DEFINE([TYPEOF_STRUCT_STAT_ST_ATIM_IS_STRUCT_TIMESPEC], [1],
         [Define to 1 if the type of the st_atim member of a struct stat is
          struct timespec.])
     fi],
    [AC_CHECK_MEMBERS([struct stat.st_atimespec.tv_nsec], [],
       [AC_CHECK_MEMBERS([struct stat.st_atimensec], [],
          [AC_CHECK_MEMBERS([struct stat.st_atim.st__tim.tv_nsec], [], [],
             [#include <sys/types.h>
              #include <sys/stat.h>])],
          [#include <sys/types.h>
           #include <sys/stat.h>])],
       [#include <sys/types.h>
        #include <sys/stat.h>])],
    [#include <sys/types.h>
     #include <sys/stat.h>])
])
