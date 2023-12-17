# moo.m4 serial 1 (gettext-0.17)
dnl Copyright (C) 2006-2007 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Support for Minimal Object-Oriented style programming.

AC_DEFUN([gl_MOO],
[
  AC_REQUIRE([AC_C_INLINE])
  dnl Test for a C++ compiler at configure time, rather than at compile time,
  dnl because when building Woe32 DLLs we need to build some compilation units
  dnl in C++ mode and not others, and in this case we don't want to use C++
  dnl classes with constructors, member functions, and operators.
  AC_CACHE_CHECK([whether the C compiler is actually a C++ compiler],
    [gl_cv_c_cplusplus],
    [AC_EGREP_CPP([Is c++], [
#ifdef __cplusplus
  Is c++
#endif
       ],
       [gl_cv_c_cplusplus=yes],
       [gl_cv_c_cplusplus=no])])
  if test $gl_cv_c_cplusplus = yes; then
    AC_DEFINE([IS_CPLUSPLUS], 1,
      [Define to 1 if the C compiler is actually a C++ compiler.])
  fi
])
