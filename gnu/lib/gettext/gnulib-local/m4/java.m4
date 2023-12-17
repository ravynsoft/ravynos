# java.m4 serial 2
dnl Copyright (C) 2005, 2017 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Sets JAVA_CHOICE to 'yes' or 'no', depending on the preferred use of Java.
AC_DEFUN([gt_JAVA_CHOICE],
[
  AC_MSG_CHECKING([whether to use Java])
  AC_ARG_ENABLE(java,
    [  --disable-java          do not build Java sources],
    [JAVA_CHOICE="$enableval"],
    [JAVA_CHOICE=yes])
  AC_MSG_RESULT([$JAVA_CHOICE])
  AC_SUBST([JAVA_CHOICE])
])
