dnl
dnl   Copyright (C) 2012-2022 Free Software Foundation, Inc.
dnl
dnl This file is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; see the file COPYING3.  If not see
dnl <http://www.gnu.org/licenses/>.
dnl

dnl See whether 64-bit bfd lib has been enabled.
AC_DEFUN([BFD_64_BIT], [dnl
AC_ARG_ENABLE(64-bit-bfd,
  AS_HELP_STRING([--enable-64-bit-bfd],
		 [64-bit support (on hosts with narrower word sizes)]),
  [AS_CASE([$enableval],
	   [yes|no], [],
	   [*], [AC_MSG_ERROR(bad value ${enableval} for 64-bit-bfd option)])],
  [enable_64_bit_bfd=no])

dnl If the host is 64-bit, then 64-bit bfd is enabled automatically.
AS_IF([test "x$enable_64_bit_bfd" = "xno"], [dnl
  AC_CHECK_SIZEOF(void *)
  AS_IF([test "x$ac_cv_sizeof_void_p" = "x8"], [enable_64_bit_bfd=yes])
])

AM_CONDITIONAL([ENABLE_BFD_64_BIT], [test "x$enable_64_bit_bfd" = "xyes"])
])
