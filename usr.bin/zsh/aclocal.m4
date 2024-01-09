# Local additions to Autoconf macros.
# Copyright (C) 1992, 1994 Free Software Foundation, Inc.
# Francois Pinard <pinard@iro.umontreal.ca>, 1992.

# @defmac fp_PROG_CC_STDC
# @maindex PROG_CC_STDC
# @ovindex CC
# If the C compiler in not in ANSI C mode by default, try to add an option
# to output variable @code{CC} to make it so.  This macro tries various
# options that select ANSI C on some system or another.  It considers the
# compiler to be in ANSI C mode if it defines @code{__STDC__} to 1 and
# handles function prototypes correctly.
# 
# If you use this macro, you should check after calling it whether the C
# compiler has been set to accept ANSI C; if not, the shell variable
# @code{fp_cv_prog_cc_stdc} is set to @samp{no}.  If you wrote your source
# code in ANSI C, you can make an un-ANSIfied copy of it by using the
# program @code{ansi2knr}, which comes with Ghostscript.
# @end defmac

define(fp_PROG_CC_STDC,
[AC_CACHE_CHECK(for ${CC-cc} option to accept ANSI C,
fp_cv_prog_cc_stdc,
[fp_cv_prog_cc_stdc=no
ac_save_CFLAGS="$CFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Ae  or  -Aa -D_HPUX_SOURCE
# SVR4			-Xc
#  For HP-UX, we try -Ae first; this turns on ANSI but also extensions,
#  as well as defining _HPUX_SOURCE, and we can then use long long.
#  We keep the old version for backward compatibility.
for ac_arg in "" -qlanglvl=ansi -std1 -Ae "-Aa -D_HPUX_SOURCE" -Xc
do
  CFLAGS="$ac_save_CFLAGS $ac_arg"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
[#ifndef __STDC__
choke me
#endif	
]], [[int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};]])],
[fp_cv_prog_cc_stdc="$ac_arg"; break],[])
done
CFLAGS="$ac_save_CFLAGS"
])
case "x$fp_cv_prog_cc_stdc" in
  x|xno) ;;
  *) CC="$CC $fp_cv_prog_cc_stdc" ;;
esac
])

AC_DEFUN(AC_PROG_LN,
[AC_MSG_CHECKING(whether ln works)
AC_CACHE_VAL(ac_cv_prog_LN,
[rm -f conftestdata conftestlink
echo > conftestdata
if ln conftestdata conftestlink 2>/dev/null
then
  rm -f conftestdata conftestlink
  ac_cv_prog_LN="ln"
else
  rm -f conftestdata
  ac_cv_prog_LN="cp"
fi])dnl
LN="$ac_cv_prog_LN"
if test "$ac_cv_prog_LN" = "ln"; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST(LN)dnl
])

builtin(include, aczsh.m4)
