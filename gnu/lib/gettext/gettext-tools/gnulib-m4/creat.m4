# creat.m4 serial 2
dnl Copyright (C) 2019-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_CREAT],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw* | windows*)
      REPLACE_CREAT=1
      ;;
    *)
      gl_OPEN_TRAILING_SLASH_BUG
      case "$gl_cv_func_open_slash" in
        *no)
          REPLACE_CREAT=1
          ;;
      esac
      ;;
  esac
])
