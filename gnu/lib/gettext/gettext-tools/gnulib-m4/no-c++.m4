# no-c++.m4 serial 2
dnl Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Support for C source files that cannot be compiled by a C++ compiler.
# Set NO_CXX to the C++ compiler flags needed to request C mode instead of
# C++ mode.
# So far only g++ is supported. This includes clang++, as it is g++ compatible.

AC_DEFUN([gt_NO_CXX],
[
  NO_CXX=
  AC_EGREP_CPP([Is_g_plus_plus], [
#if defined __GNUC__ && defined __cplusplus
  Is_g_plus_plus
#endif
    ],
    [NO_CXX="-x c"])
  AC_SUBST([NO_CXX])
])
