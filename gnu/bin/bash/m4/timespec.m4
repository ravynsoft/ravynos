# Configure checks for struct timespec

# Copyright (C) 2000-2001, 2003-2007, 2009-2011, 2012 Free Software Foundation, Inc.

# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# Original written by Paul Eggert and Jim Meyering.
# Modified by Chet Ramey for bash

dnl Define HAVE_STRUCT_TIMESPEC if `struct timespec' is declared
dnl in time.h, sys/time.h, or pthread.h.

AC_DEFUN([BASH_CHECK_TYPE_STRUCT_TIMESPEC],
[
  AC_CHECK_HEADERS_ONCE([sys/time.h])
  AC_CACHE_CHECK([for struct timespec in <time.h>],
    [bash_cv_sys_struct_timespec_in_time_h],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include <time.h>
          ]],
          [[static struct timespec x; x.tv_sec = x.tv_nsec;]])],
       [bash_cv_sys_struct_timespec_in_time_h=yes],
       [bash_cv_sys_struct_timespec_in_time_h=no])])

  HAVE_STRUCT_TIMESPEC=0
  TIME_H_DEFINES_STRUCT_TIMESPEC=0
  SYS_TIME_H_DEFINES_STRUCT_TIMESPEC=0
  PTHREAD_H_DEFINES_STRUCT_TIMESPEC=0
  if test $bash_cv_sys_struct_timespec_in_time_h = yes; then
    AC_DEFINE([HAVE_STRUCT_TIMESPEC])
    AC_DEFINE([TIME_H_DEFINES_STRUCT_TIMESPEC])
    TIME_H_DEFINES_STRUCT_TIMESPEC=1
  else
    AC_CACHE_CHECK([for struct timespec in <sys/time.h>],
      [bash_cv_sys_struct_timespec_in_sys_time_h],
      [AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM(
            [[#include <sys/time.h>
            ]],
            [[static struct timespec x; x.tv_sec = x.tv_nsec;]])],
         [bash_cv_sys_struct_timespec_in_sys_time_h=yes],
         [bash_cv_sys_struct_timespec_in_sys_time_h=no])])
    if test $bash_cv_sys_struct_timespec_in_sys_time_h = yes; then
      SYS_TIME_H_DEFINES_STRUCT_TIMESPEC=1
      AC_DEFINE([HAVE_STRUCT_TIMESPEC])
      AC_DEFINE([SYS_TIME_H_DEFINES_STRUCT_TIMESPEC])
    else
      AC_CACHE_CHECK([for struct timespec in <pthread.h>],
        [bash_cv_sys_struct_timespec_in_pthread_h],
        [AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM(
              [[#include <pthread.h>
              ]],
              [[static struct timespec x; x.tv_sec = x.tv_nsec;]])],
           [bash_cv_sys_struct_timespec_in_pthread_h=yes],
           [bash_cv_sys_struct_timespec_in_pthread_h=no])])
      if test $bash_cv_sys_struct_timespec_in_pthread_h = yes; then
        PTHREAD_H_DEFINES_STRUCT_TIMESPEC=1
	AC_DEFINE([HAVE_STRUCT_TIMESPEC])
	AC_DEFINE([PTHREAD_H_DEFINES_STRUCT_TIMESPEC])
      fi
    fi
  fi
  AC_SUBST([TIME_H_DEFINES_STRUCT_TIMESPEC])
  AC_SUBST([SYS_TIME_H_DEFINES_STRUCT_TIMESPEC])
  AC_SUBST([PTHREAD_H_DEFINES_STRUCT_TIMESPEC])

])
