# backupfile.m4 serial 3 (gettext-0.15)
dnl Copyright (C) 2001-2003, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Prerequisites of lib/backupfile.h

AC_DEFUN([gt_PREREQ_BACKUPFILE],
[
  dnl For backupfile.c.
  AC_CHECK_HEADERS(dirent.h string.h)
  dnl For addext.c.
  AC_SYS_LONG_FILE_NAMES
  AC_CHECK_FUNCS(pathconf)
  AC_CHECK_HEADERS(string.h unistd.h)
])
