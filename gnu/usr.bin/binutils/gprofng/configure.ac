dnl Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright (C) 2021-2023 Free Software Foundation, Inc.
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

m4_include([../bfd/version.m4])
m4_include([../config/zlib.m4])
AC_INIT([gprofng], BFD_VERSION)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([subdir-objects])
AM_MAINTAINER_MODE

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_AR
gl_PROG_BISON([BISON],[3.0.4])
if test x$BISON = "x:"; then
  AC_MSG_ERROR([Building gprofng requires bison 3.0.4 or later.])
fi

AC_DISABLE_SHARED
LT_INIT

GPROFNG_LIBADD="-L../../libiberty -liberty"
if test "$enable_shared" = "yes"; then
  GPROFNG_LIBADD="-L../../libiberty/pic -liberty"
fi
AC_SUBST(GPROFNG_LIBADD)

# Figure out what compiler warnings we can enable.
# See config/warnings.m4 for details.

ACX_PROG_CC_WARNINGS_ARE_ERRORS([manual])
ACX_PROG_CC_WARNING_OPTS([-Wall], [gprofng_cflags])
ACX_PROG_CC_WARNING_OPTS([-Wno-switch], [GPROFNG_NO_SWITCH_CFLAGS])
gprofng_cppflags="-U_ASM"
build_collector=
build_src=

  case "${host}" in
    x86_64-*-linux*)
      build_src=true
      build_collector=true
      ;;
    i?86-*-linux*)
      build_src=true
      build_collector=true
      ;;
    aarch64-*-linux*)
      build_src=true
      build_collector=true
      ;;
  esac
  AC_ARG_ENABLE(gprofng-tools,
    AS_HELP_STRING([--disable-gprofng-tools], [do not build gprofng/src directory]),
    build_src=$enableval)

AM_CONDITIONAL([BUILD_COLLECTOR], [test x$build_collector = xtrue])
AM_CONDITIONAL([BUILD_SRC], [test x$build_src = xtrue])

if test x$build_collector = xtrue; then
  AC_CONFIG_SUBDIRS([libcollector])
fi
AX_PTHREAD

# Specify a location for JDK
enable_gprofng_jp=
jdk_inc=
AC_ARG_WITH(jdk,
[AS_HELP_STRING([--with-jdk=PATH],
		[specify prefix directory for installed JDK.])])

if test "x$with_jdk" != x; then
  jdk_inc="-I$with_jdk/include -I$with_jdk/include/linux"
  enable_gprofng_jp=yes
else
  AC_PATH_PROG([JAVAC], [javac], [javac])
  if test -f $JAVAC; then
    x=`readlink -f $JAVAC`
    x=`dirname $x`
    x=`dirname $x`
    if ! test -f $x/include/jni.h; then
      x=`dirname $x`
    fi
    if test -f $x/include/jni.h; then
      jdk_inc="-I$x/include -I$x/include/linux"
      enable_gprofng_jp=yes
    fi
  fi
fi
if test "x$enable_gprofng_jp" = x; then
  AC_PATH_PROG([JAVA], [java], [java])
  if test -f $JAVA; then
    x=`readlink -f $JAVA`
    x=`dirname $x`
    x=`dirname $x`
    if ! test -f $x/include/jni.h; then
      x=`dirname $x`
    fi
    if test -f $x/include/jni.h; then
      jdk_inc="-I$x/include -I$x/include/linux"
      enable_gprofng_jp=yes
    fi
  fi
fi
if test "x$enable_gprofng_jp" = x; then
  AC_CHECK_HEADER([jni.h], [ enable_gprofng_jp=yes ], [], [] )
fi
GPROFNG_BROKEN_JAVAC=no
if test "x$enable_gprofng_jp" = x; then
    AC_MSG_WARN([ Cannot find the JDK include directory.
      gprofng will be build without support for profiling Java applications.
      Use --with-jdk=PATH to specify directory for the installed JDK])
else
    AC_DEFINE(GPROFNG_JAVA_PROFILING, 1, [Enable java profiling])
    if test "x$JAVAC" != x; then
      cat > configtest.java << EOF
class Simple{
  public static void main(String args[]){
    System.out.println("Hello Java");
  }
}
EOF
      if AC_TRY_COMMAND($JAVAC configtest.java >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD); then
	GPROFNG_BROKEN_JAVAC=no
      else
	GPROFNG_BROKEN_JAVAC=yes
      fi
      rm -f configtest.*
    fi
fi
AC_SUBST(GPROFNG_BROKEN_JAVAC)
AC_SUBST(jdk_inc)

DEBUG=
GCC_ENABLE([gprofng-debug], [no], [], [Enable debugging output])
if test "${enable_gprofng_debug}" = yes; then
    AC_DEFINE(DEBUG, 1, [Enable debugging output.])
fi

cat > "dummy.c" << EOF
#include <features.h>
#if defined(__UCLIBC__)
LIBC=uclibc
#elif defined(__dietlibc__)
LIBC=dietlibc
#elif defined(__GLIBC__)
LIBC=gnu
#else
#include <stdarg.h>
/* First heuristic to detect musl libc.  */
#ifdef __DEFINED_va_list
LIBC=musl
#else
LIBC=gnu
#endif
#endif
EOF
cc_set_libc=`$CC -E "dummy.c" 2>/dev/null | grep '^LIBC=' | sed 's, ,,g'`
eval "$cc_set_libc"
if test "$LIBC" = musl; then
  AC_DEFINE(__MUSL_LIBC, 1, [Build with musl-libc.])
fi


# Check if linker supports --as-needed and --no-as-needed options.
AC_CACHE_CHECK(linker --as-needed support, bfd_cv_ld_as_needed,
	[bfd_cv_ld_as_needed=no
	if $LD --help 2>/dev/null | grep as-needed > /dev/null; then
		bfd_cv_ld_as_needed=yes
	fi
	])

no_as_needed=
if test x"$bfd_cv_ld_as_needed" = xyes; then
    no_as_needed='-Wl,--no-as-needed'
fi

AC_PATH_PROG([EXPECT], [expect])
AC_CACHE_CHECK([for Tcl supporting try/catch], [ac_cv_libctf_tcl_try],
  [ac_cv_libctf_tcl_try=`if test -z $EXPECT; then echo no; else $EXPECT << EOF
if @<:@llength @<:@info commands try@:>@@:>@ then { puts yes } else { puts no }
EOF
fi`
])

# Only native testing and only on supported platforms:
AM_CONDITIONAL(TCL_TRY, [test "${ac_cv_libctf_tcl_try}" = yes \
	-a "x${build_src}" = xtrue -a "x${build_collector}" = xtrue \
	-a "x${host}" = "x${target}"])

AM_ZLIB

# Generate manpages, if possible.
build_man=false
build_doc=false
if test $cross_compiling = no; then
  AC_CHECK_PROGS([HELP2MAN], help2man, [$MISSING help2man])
  case "x$HELP2MAN" in
    x | */missing\ help2man* )
      AC_MSG_WARN([gprofng: help2man is missing. Man pages will not be built.])
      ;;
    * ) build_man=true ;;
    esac

  AC_CHECK_PROGS([MAKEINFO], makeinfo, [$MISSING makeinfo])
  case "x$MAKEINFO" in
    x | */missing\ makeinfo*)
      AC_MSG_WARN([gprofng: makeinfo is missing. Info documentation will not be built.])
      ;;
    *)
      case x"`$MAKEINFO --version | grep 'GNU texinfo'`" in
	x*\ [[1-5]].*|x*\ 6.[[0-4]].* )
	  AC_MSG_WARN([gprofng: $MAKEINFO is too old. Info documentation will not be built.])
	  MAKEINFO="@echo $MAKEINFO is too old, 6.5 or newer required; true"
	  ;;
	x* ) build_doc=true ;;
	esac
      ;;
    esac
  AC_SUBST(MAKEINFO)
fi
AM_CONDITIONAL([BUILD_MAN], [test x$build_man = xtrue])
AM_CONDITIONAL([BUILD_DOC], [test x$build_doc = xtrue])

AC_SUBST(LD_NO_AS_NEEDED, [${no_as_needed}])
AC_SUBST(GPROFNG_CFLAGS, [${gprofng_cflags}])
AC_SUBST(GPROFNG_NO_FORMAT_TRUNCATION_CFLAGS)
AC_SUBST(GPROFNG_NO_SWITCH_CFLAGS)
AC_SUBST(GPROFNG_CPPFLAGS, [${gprofng_cppflags}])

AC_CHECK_DECLS([basename])
AC_CHECK_FUNCS(clock_gettime strsignal)

clock_gettime_link=
# At least for glibc, clock_gettime is in librt.  But don't
# pull that in if it still doesn't give us the function we want.  This
# test is copied from libgomp, and modified to not link in -lrt as
# we're using this for test timing only.
if test "$ac_cv_func_clock_gettime" = no; then
  AC_CHECK_LIB(rt, clock_gettime,
    [CLOCK_GETTIME_LINK=-lrt
     AC_DEFINE(HAVE_CLOCK_GETTIME, 1,
	       [Define to 1 if you have the `clock_gettime' function.])])
fi
AC_SUBST(CLOCK_GETTIME_LINK)

AC_SUBST(BUILD_SUBDIRS)

AC_CONFIG_FILES([Makefile src/Makefile gp-display-html/Makefile doc/Makefile])
AC_CONFIG_HEADERS([config.h:common/config.h.in])

AC_OUTPUT

