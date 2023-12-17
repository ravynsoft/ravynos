# serial 8

# Copyright (C) 2005-2007, 2009-2023 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# Written by Paul Eggert.

AC_DEFUN([gl_UNLINKDIR],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_CHECK_HEADERS_ONCE([priv.h])

  # The Hurd, the Linux kernel, the FreeBSD kernel version 2.2 and later,
  # Cygwin, and mingw never let anyone (even root) unlink directories.
  # If anyone knows of another system for which unlink can never
  # remove a directory, please report it to <bug-coreutils@gnu.org>.
  # Unfortunately this is difficult to test for, since it requires root access
  # and might create garbage in the file system,
  # so the code below simply relies on the kernel name and version number.
  case $host_os in
  gnu[[0-9]]* | \
  linux-* | linux | \
  freebsd2.2* | freebsd[[3-9]]* | freebsd[[1-9]][[0-9]]* | \
  cygwin | \
  mingw* | windows*)
    AC_DEFINE([UNLINK_CANNOT_UNLINK_DIR], [1],
      [Define to 1 if unlink (dir) cannot possibly succeed.]);;
  esac
])
