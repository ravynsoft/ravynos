# exported.m4 serial 3 (gettext-0.21.1)
dnl Copyright (C) 2006, 2009, 2019-2020 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Prerequisites of the exported.sh script:
dnl Check for nm output filter that yields the exported symbols.
AC_DEFUN([gt_GLOBAL_SYMBOL_PIPE], [
  AC_REQUIRE([LT_PATH_NM]) dnl provided by libtool.m4
  AC_SUBST([NM])
  AC_REQUIRE([_LT_CMD_GLOBAL_SYMBOLS]) dnl provided by libtool.m4
  GLOBAL_SYMBOL_PIPE=$lt_cv_sys_global_symbol_pipe
  AC_SUBST([GLOBAL_SYMBOL_PIPE])
  if test -n "$GLOBAL_SYMBOL_PIPE"; then
    HAVE_GLOBAL_SYMBOL_PIPE=1
  else
    HAVE_GLOBAL_SYMBOL_PIPE=
  fi
  AC_SUBST([HAVE_GLOBAL_SYMBOL_PIPE])
])
