# curses.m4 serial 1
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Sets gl_curses_allowed to yes or no.
AC_DEFUN([gl_CURSES],
[
  AC_MSG_CHECKING([whether curses libraries may be used])
  AC_ARG_ENABLE(curses,
    [  --disable-curses        do not use libncurses, libtermcap even if they exist],
    [gl_curses_allowed="$enableval"],
    [gl_curses_allowed=yes])
  AC_MSG_RESULT([$gl_curses_allowed])
])
