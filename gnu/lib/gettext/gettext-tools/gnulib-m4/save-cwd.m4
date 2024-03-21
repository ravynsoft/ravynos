# serial 10
dnl Copyright (C) 2002-2006, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Prerequisites for lib/save-cwd.c.
AC_DEFUN([gl_SAVE_CWD],
[
  AC_CHECK_FUNCS_ONCE([fchdir])
])
