# serial 10

# Copyright (C) 2002-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# There are two types of parser skeletons:
#
# * Those that can be used with any Yacc implementation, including bison.
#   For these, in the configure.ac, up to Autoconf 2.69, you could use
#     AC_PROG_YACC
#   In newer Autoconf versions, however, this macro is broken. See
#     https://lists.gnu.org/archive/html/autoconf-patches/2013-03/msg00000.html
#     https://lists.gnu.org/archive/html/bug-autoconf/2018-12/msg00001.html
#   In the Makefile.am you could use
#     $(SHELL) $(YLWRAP) $(srcdir)/foo.y \
#                        y.tab.c foo.c \
#                        y.tab.h foo.h \
#                        y.output foo.output \
#                        -- $(YACC) $(YFLAGS) $(AM_YFLAGS)
#   or similar.
#
# * Those that make use of Bison extensions. For example,
#     - %define api.pure   requires bison 2.7 or newer,
#     - %precedence        requires bison 3.0 or newer.
#   For these, in the configure.ac you will need an invocation of
#     gl_PROG_BISON([VARIABLE], [MIN_BISON_VERSION])
#   Example:
#     gl_PROG_BISON([PARSE_DATETIME_BISON], [2.4])
#   With this preparation, in the Makefile.am there are two ways to formulate
#   the invocation. Both are direct, without use of 'ylwrap'.
#   (a) You can invoke
#         $(VARIABLE) -d $(SOME_BISON_OPTIONS) --output foo.c $(srcdir)/foo.y
#       or similar.
#   (b) If you want the invocation to honor an YFLAGS=... parameter passed to
#       'configure' or an YFLAGS environment variable present at 'configure'
#       time, add an invocation of gl_BISON to the configure.ac, and write
#         $(VARIABLE) -d $(YFLAGS) $(AM_YFLAGS) $(srcdir)/foo.y
#       or similar.

# This macro defines the autoconf variable VARIABLE to 'bison' if the specified
# minimum version of bison is found in $PATH, or to ':' otherwise.
AC_DEFUN([gl_PROG_BISON],
[
  AC_CHECK_PROGS([$1], [bison])
  if test -z "$[$1]"; then
    ac_verc_fail=yes
  else
    cat >conftest.y <<_ACEOF
%require "$2"
%%
exp:
_ACEOF
    AC_MSG_CHECKING([for bison $2 or newer])
    ac_prog_version=`$$1 --version 2>&1 | sed -n 's/^.*GNU Bison.* \([[0-9]]*\.[[0-9.]]*\).*$/\1/p'`
    : ${ac_prog_version:='v. ?.??'}
    if $$1 conftest.y -o conftest.c 2>/dev/null; then
      ac_prog_version="$ac_prog_version, ok"
      ac_verc_fail=no
    else
      ac_prog_version="$ac_prog_version, bad"
      ac_verc_fail=yes
    fi
    rm -f conftest.y conftest.c
    AC_MSG_RESULT([$ac_prog_version])
  fi
  if test $ac_verc_fail = yes; then
    [$1]=:
  fi
  AC_SUBST([$1])
])

# This macro sets the autoconf variables YACC (for old-style yacc Makefile
# rules) and YFLAGS (to allow options to be passed as 'configure' time).
AC_DEFUN([gl_BISON],
[
  : ${YACC='bison -o y.tab.c'}
dnl
dnl Declaring YACC & YFLAGS precious will not be necessary after GNULIB
dnl requires an Autoconf greater than 2.59c, but it will probably still be
dnl useful to override the description of YACC in the --help output, re
dnl parse-datetime.y assuming 'bison -o y.tab.c'.
  AC_ARG_VAR([YACC],
[The "Yet Another C Compiler" implementation to use.  Defaults to
'bison -o y.tab.c'.  Values other than 'bison -o y.tab.c' will most likely
break on most systems.])dnl
  AC_ARG_VAR([YFLAGS],
[YFLAGS contains the list arguments that will be passed by default to Bison.
This script will default YFLAGS to the empty string to avoid a default value of
'-d' given by some make applications.])dnl
])
