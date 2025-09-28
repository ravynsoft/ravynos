dnl Copyright (C) 2022 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Make sure we replace strtoimax if we don't have a declaration
dnl We can use this as a template for future function checks

AC_DEFUN([BASH_FUNC_STRTOIMAX], [
AC_MSG_CHECKING([for usable strtoimax])
AC_CACHE_VAL(bash_cv_func_strtoimax,
[
  HAVE_STRTOIMAX=0 HAVE_DECL_STRTOIMAX=0

  AC_CHECK_FUNCS([strtoimax])
  AC_CHECK_DECLS([strtoimax])

  if test "$ac_cv_func_strtoimax" = "yes" ; then
    HAVE_STRTOIMAX=1
  fi
  if test "$ac_cv_have_decl_strtoimax" = "yes" ; then
    HAVE_DECL_STRTOIMAX=1
  fi

  if test "$HAVE_STRTOIMAX" = 0 || test "$HAVE_DECL_STRTOIMAX" = 0 ; then
    bash_cv_func_strtoimax=no REPLACE_STRTOIMAX=1
  else
    bash_cv_func_strtoimax=yes
  fi
])
AC_MSG_RESULT($bash_cv_func_strtoimax)
if test $bash_cv_func_strtoimax = yes; then
AC_LIBOBJ(strtoimax)
fi
])
