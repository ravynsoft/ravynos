#serial 8
dnl Copyright (C) 2004-2006, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_INTTOSTR],
[
  gl_PREREQ_INTTOSTR
  gl_PREREQ_IMAXTOSTR
  gl_PREREQ_OFFTOSTR
  gl_PREREQ_UMAXTOSTR
  gl_PREREQ_UINTTOSTR
])

# Prerequisites of lib/inttostr.h.
AC_DEFUN([gl_PREREQ_INTTOSTR], [
  AC_REQUIRE([AC_TYPE_OFF_T])
  :
])

# Prerequisites of lib/imaxtostr.c.
AC_DEFUN([gl_PREREQ_IMAXTOSTR], [:])

# Prerequisites of lib/offtostr.c.
AC_DEFUN([gl_PREREQ_OFFTOSTR], [:])

# Prerequisites of lib/umaxtostr.c.
AC_DEFUN([gl_PREREQ_UMAXTOSTR], [:])

# Prerequisites of lib/uinttostr.c.
AC_DEFUN([gl_PREREQ_UINTTOSTR], [:])
