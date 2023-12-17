# posix_spawn_faction_addchdir.m4 serial 1
dnl Copyright (C) 2018-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR],
[
  AC_REQUIRE([gl_SPAWN_H_DEFAULTS])
  AC_REQUIRE([AC_PROG_CC])
  gl_POSIX_SPAWN
  AC_CHECK_FUNCS_ONCE([posix_spawn_file_actions_addchdir posix_spawn_file_actions_addchdir_np])
  if test $ac_cv_func_posix_spawn_file_actions_addchdir = yes; then
    dnl This function is not yet standardized. Therefore override the
    dnl system's implementation always.
    REPLACE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR=1
  else
    HAVE_POSIX_SPAWN_FILE_ACTIONS_ADDCHDIR=0
  fi
])
