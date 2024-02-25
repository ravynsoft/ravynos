# SPDX-License-Identifier: MIT
#
# Copyright © 2013 Red Hat, Inc.
#

AC_PREREQ([2.62])

# change meson version too
AC_INIT([libevdev],
        [1.11.0],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=libevdev],
        [libevdev],
        [http://freedesktop.org/wiki/Software/libevdev/])

AC_CONFIG_SRCDIR([libevdev/libevdev.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_USE_SYSTEM_EXTENSIONS

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz subdir-objects])

# DO NOT MODIFY THIS
# Use symbol versioning instead.
LIBEVDEV_LT_VERSION=5:0:3
AC_SUBST(LIBEVDEV_LT_VERSION)


AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CC_C99

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT
LT_PATH_LD

with_ldflags=""
if test "x$lt_cv_prog_gnu_ld" = "xyes"; then
	CC_CHECK_FLAGS_APPEND([with_ldflags], [LDFLAGS], [\
				-Wl,--as-needed \
				-Wl,--gc-sections \
				-Wl,-z,relro \
				-Wl,-z,now])
fi
AC_SUBST([GNU_LD_FLAGS], $with_ldflags)

case "${host_os}" in
    freebsd*)
AC_SUBST([OS], [freebsd])
        ;;
    *)
AC_SUBST([OS], [linux])
        ;;
esac

AC_CHECK_LIB([m], [round])

PKG_PROG_PKG_CONFIG()
PKG_CHECK_MODULES(CHECK, [check >= 0.9.9], [HAVE_CHECK="yes"], [HAVE_CHECK="no"])
if test "x$HAVE_CHECK" = "xyes"; then
	AC_PATH_PROG(VALGRIND, [valgrind])
else
	AC_MSG_WARN([check not found - skipping building unit tests])
fi
AM_CONDITIONAL(HAVE_VALGRIND, [test "x$VALGRIND" != "x"])
AM_CONDITIONAL(ENABLE_RUNTIME_TESTS, [test "x$HAVE_CHECK" = "xyes"])
AM_CONDITIONAL(ENABLE_STATIC_LINK_TEST, [test "x$enable_static" = "xyes"])

with_cflags=""
if test "x$GCC" = "xyes"; then
	CC_CHECK_FLAGS_APPEND([with_cflags], [CFLAGS], [\
				-Wall \
				-Wextra \
				-Wno-unused-parameter \
				-Wstrict-prototypes \
				-Wmissing-prototypes \
				-fvisibility=hidden \
				-pipe \
				-fno-strict-aliasing \
				-ffunction-sections \
				-fdata-sections \
				-fno-strict-aliasing \
				-fdiagnostics-show-option \
				-fno-common])
fi
AC_SUBST([GCC_CFLAGS], $with_cflags)

AC_PATH_PROG(DOXYGEN, [doxygen])
if test "x$DOXYGEN" = "x"; then
	AC_MSG_WARN([doxygen not found - required for documentation])
	have_doxygen="no"
else
	have_doxygen="yes"
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test "x$have_doxygen" = "xyes"])

AC_MSG_CHECKING([whether to build with gcov])
AC_ARG_ENABLE([gcov],
	      [AS_HELP_STRING([--enable-gcov],
			      [Whether to enable coverage testing (default:disabled)])],
	      [],
	      [enable_gcov=no],
	      )
AS_IF([test "x$enable_gcov" != "xno"],
      [
       GCOV_CFLAGS="-fprofile-arcs -ftest-coverage"
       GCOV_LDFLAGS="-lgcov"
       enable_gcov=yes
       ],
)
AM_CONDITIONAL([GCOV_ENABLED], [test "x$enable_gcov" != "xno"])
AC_SUBST([GCOV_CFLAGS])
AC_SUBST([GCOV_LDFLAGS])
AC_MSG_RESULT([$enable_gcov])

AC_MSG_CHECKING([whether to build with coverity support])
AC_ARG_ENABLE([coverity],
	      [AS_HELP_STRING([--enable-coverity],
			      [Whether to build with coverity support (default: disabled)])],
	      [],
	      [enable_coverity=no],
	      )
AS_IF([test "x$enable_coverity" != "xno"],
      [
       AC_DEFINE([_Float128], [__uint128_t], [Override for coverity])
       AC_DEFINE([_Float32], [int], [Override for coverity])
       AC_DEFINE([_Float32x], [int], [Override for coverity])
       AC_DEFINE([_Float64], [long], [Override for coverity])
       AC_DEFINE([_Float64x], [long], [Override for coverity])
       enable_coverity=yes
       ],
)
AC_MSG_RESULT([$enable_coverity])


AM_PATH_PYTHON([2.6])

# nm to check for leaking symbols in the static library
AC_PATH_PROG(NM, [nm])
AM_CONDITIONAL(HAVE_NM, [test "x$NM" != "x"])
if test "x$enable_static" = "xno"; then
	static_symbol_leaks_test="no - static build disabled"
else
	if test "x$NM" = "x"; then
		AC_MSG_WARN([nm not found - skipping symbol leak test])
		have_nm="no"
		static_symbol_leaks_test="no - nm not found"
	else
		have_nm="yes"
		static_symbol_leaks_test="yes"
	fi
fi

AM_CONDITIONAL(ENABLE_STATIC_SYMBOL_LEAKS_TEST, [test "x$static_symbol_leaks_test" = "xyes"])

AC_CONFIG_FILES([Makefile
		 libevdev/Makefile
		 doc/Makefile
		 doc/libevdev.doxygen
		 doc/libevdev.man
		 tools/Makefile
		 test/Makefile
		 libevdev.pc])
AC_OUTPUT

AC_MSG_RESULT([
	       Prefix				${prefix}
	       Libdir				${libdir}

	       Build documentation		${have_doxygen}
	       Enable unit-tests		${HAVE_CHECK}
	       Enable profiling			${enable_gcov}
	       Enable coverity support		${enable_coverity}
	       Static library symbol check	${static_symbol_leaks_test}
	       ])
