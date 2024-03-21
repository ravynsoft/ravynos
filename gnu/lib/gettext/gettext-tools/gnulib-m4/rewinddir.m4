# rewinddir.m4 serial 3
dnl Copyright (C) 2011-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_REWINDDIR],
[
  AC_REQUIRE([gl_DIRENT_H_DEFAULTS])

  AC_CHECK_FUNCS([rewinddir])
  if test $ac_cv_func_rewinddir = no; then
    HAVE_REWINDDIR=0
  else
    dnl Replace rewinddir() on native Windows and OS/2 kLIBC,
    dnl to support fdopendir().
    AC_REQUIRE([gl_DIRENT_DIR])
    if test $DIR_HAS_FD_MEMBER = 0; then
      REPLACE_REWINDDIR=1
    fi
  fi
])
