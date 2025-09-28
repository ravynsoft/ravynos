dnl Common configure.ac fragment
dnl
dnl   Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

# AC_EGREP_CPP_FOR_BUILD(PATTERN, PROGRAM,
#              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ------------------------------------------------------
AC_DEFUN([AC_EGREP_CPP_FOR_BUILD],
[AC_LANG_PREPROC_REQUIRE()dnl
AC_REQUIRE([AC_PROG_EGREP])dnl
AC_LANG_CONFTEST([AC_LANG_SOURCE([[$2]])])
AS_IF([dnl eval is necessary to expand ac_cpp.
dnl Ultrix and Pyramid sh refuse to redirect output of eval, so use subshell.
(eval "$ac_cpp_for_build conftest.$ac_ext") 2>&AS_MESSAGE_LOG_FD |
dnl Quote $1 to prevent m4 from eating character classes
  $EGREP "[$1]" >/dev/null 2>&1],
  [$3],
  [$4])
rm -f conftest*
])# AC_EGREP_CPP_FOR_BUILD


AC_DEFUN([AM_BINUTILS_WARNINGS],[
# Set the 'development' global.
. $srcdir/../bfd/development.sh

# Set acp_cpp_for_build variable
ac_cpp_for_build="$CC_FOR_BUILD -E $CPPFLAGS_FOR_BUILD"

# Default set of GCC warnings to enable.
GCC_WARN_CFLAGS="-W -Wall -Wstrict-prototypes -Wmissing-prototypes"
GCC_WARN_CFLAGS_FOR_BUILD="-W -Wall -Wstrict-prototypes -Wmissing-prototypes"

# Add -Wshadow if the compiler is a sufficiently recent version of GCC.
AC_EGREP_CPP([(^[0-3]$|^__GNUC__$)],[__GNUC__],,GCC_WARN_CFLAGS="$GCC_WARN_CFLAGS -Wshadow")

# Add -Wstack-usage if the compiler is a sufficiently recent version of GCC.
AC_EGREP_CPP([(^[0-4]$|^__GNUC__$)],[__GNUC__],,dnl
[AC_EGREP_CPP([^__clang__$],[__clang__],[GCC_WARN_CFLAGS="$GCC_WARN_CFLAGS -Wstack-usage=262144"],)])

# Set WARN_WRITE_STRINGS if the compiler supports -Wwrite-strings.
WARN_WRITE_STRINGS=""
AC_EGREP_CPP([(^[0-3]$|^__GNUC__$)],[__GNUC__],,WARN_WRITE_STRINGS="-Wwrite-strings")

# Verify CC_FOR_BUILD to be compatible with warning flags

# Add -Wshadow if the compiler is a sufficiently recent version of GCC.
AC_EGREP_CPP_FOR_BUILD([(^[0-3]$|^__GNUC__$)],[__GNUC__],,GCC_WARN_CFLAGS_FOR_BUILD="$GCC_WARN_CFLAGS_FOR_BUILD -Wshadow")

# Add -Wstack-usage if the compiler is a sufficiently recent version of GCC.
AC_EGREP_CPP_FOR_BUILD([(^[0-4]$|^__GNUC__$)],[__GNUC__],,dnl
[AC_EGREP_CPP_FOR_BUILD([^__clang__$],[__clang__],[GCC_WARN_CFLAGS_FOR_BUILD="$GCC_WARN_CFLAGS_FOR_BUILD -Wstack-usage=262144"],)])

AC_ARG_ENABLE(werror,
  [  --enable-werror         treat compile warnings as errors],
  [case "${enableval}" in
     yes | y) ERROR_ON_WARNING="yes" ;;
     no | n)  ERROR_ON_WARNING="no" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-werror) ;;
   esac])

# Disable -Wformat by default when using gcc on mingw
case "${host}" in
  *-*-mingw32*)
    if test "${GCC}" = yes -a -z "${ERROR_ON_WARNING}" ; then
      GCC_WARN_CFLAGS="$GCC_WARN_CFLAGS -Wno-format"
      GCC_WARN_CFLAGS_FOR_BUILD="$GCC_WARN_CFLAGS_FOR_BUILD -Wno-format"
    fi
    ;;
  *) ;;
esac

# Enable -Werror by default when using gcc.  Turn it off for releases.
if test "${GCC}" = yes -a -z "${ERROR_ON_WARNING}" -a "$development" = true ; then
    ERROR_ON_WARNING=yes
fi

NO_WERROR=
if test "${ERROR_ON_WARNING}" = yes ; then
    GCC_WARN_CFLAGS="$GCC_WARN_CFLAGS -Werror"
    GCC_WARN_CFLAGS_FOR_BUILD="$GCC_WARN_CFLAGS_FOR_BUILD -Werror"
    NO_WERROR="-Wno-error"
fi

if test "${GCC}" = yes ; then
  WARN_CFLAGS="${GCC_WARN_CFLAGS}"
  WARN_CFLAGS_FOR_BUILD="${GCC_WARN_CFLAGS_FOR_BUILD}"
fi

AC_ARG_ENABLE(build-warnings,
[  --enable-build-warnings enable build-time compiler warnings],
[case "${enableval}" in
  yes)	WARN_CFLAGS="${GCC_WARN_CFLAGS}" 
        WARN_CFLAGS_FOR_BUILD="${GCC_WARN_CFLAGS_FOR_BUILD}";;
  no)	if test "${GCC}" = yes ; then
	  WARN_CFLAGS="-w"
      WARN_CFLAGS_FOR_BUILD="-w" 
	fi;;
  ,*)   t=`echo "${enableval}" | sed -e "s/,/ /g"`
        WARN_CFLAGS="${GCC_WARN_CFLAGS} ${t}"
        WARN_CFLAGS_FOR_BUILD="${GCC_WARN_CFLAGS_FOR_BUILD} ${t}";;
  *,)   t=`echo "${enableval}" | sed -e "s/,/ /g"`
        WARN_CFLAGS="${t} ${GCC_WARN_CFLAGS}"
        WARN_CFLAGS_FOR_BUILD="${t} ${GCC_WARN_CFLAGS_FOR_BUILD}";;
  *)    WARN_CFLAGS=`echo "${enableval}" | sed -e "s/,/ /g"`
        WARN_CFLAGS_FOR_BUILD=`echo "${enableval}" | sed -e "s/,/ /g"`;;
esac])

if test x"$silent" != x"yes" && test x"$WARN_CFLAGS" != x""; then
  echo "Setting warning flags = $WARN_CFLAGS" 6>&1
fi

AC_SUBST(WARN_CFLAGS)
AC_SUBST(WARN_CFLAGS_FOR_BUILD)
AC_SUBST(NO_WERROR)
     AC_SUBST(WARN_WRITE_STRINGS)
])
