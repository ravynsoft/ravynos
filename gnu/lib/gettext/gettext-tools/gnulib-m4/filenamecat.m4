# filenamecat.m4 serial 12
dnl Copyright (C) 2002-2006, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FILE_NAME_CONCAT],
[
  AC_REQUIRE([gl_FILE_NAME_CONCAT_LGPL])
])

AC_DEFUN([gl_FILE_NAME_CONCAT_LGPL],
[
  dnl Prerequisites of lib/filenamecat-lgpl.c.
  gl_CHECK_FUNCS_ANDROID([mempcpy], [[#include <string.h>]])
])
