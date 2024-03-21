# csharp.m4 serial 4
dnl Copyright (C) 2004-2005, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Sets CSHARP_CHOICE to the preferred C# implementation:
# 'mono' or 'any' or 'no'.
AC_DEFUN([gt_CSHARP_CHOICE],
[
  AC_MSG_CHECKING([for preferred C[#] implementation])
  AC_ARG_ENABLE([csharp],
    [  --enable-csharp[[=IMPL]]  choose preferred C[#] implementation (mono)],
    [CSHARP_CHOICE="$enableval"],
    CSHARP_CHOICE=any)
  AC_SUBST([CSHARP_CHOICE])
  AC_MSG_RESULT([$CSHARP_CHOICE])
  case "$CSHARP_CHOICE" in
    mono)
      AC_DEFINE([CSHARP_CHOICE_MONO], [1],
        [Define if mono is the preferred C# implementation.])
      ;;
  esac
])
