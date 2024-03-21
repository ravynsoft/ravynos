AC_DEFUN([zsh_OOT],
[
AC_CHECK_HEADERS(stdarg.h varargs.h termios.h termio.h)

AC_TYPE_SIGNAL

AC_DEFINE([ZSH_OOT_MODULE], [], [Out-of-tree module])
])
