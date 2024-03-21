# build-to-host.m4 serial 3
dnl Copyright (C) 2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Bruno Haible.

dnl When the build environment ($build_os) is different from the target runtime
dnl environment ($host_os), file names may need to be converted from the build
dnl environment syntax to the target runtime environment syntax. This is
dnl because the Makefiles are executed (mostly) by build environment tools and
dnl therefore expect file names in build environment syntax, whereas the runtime
dnl expects file names in target runtime environment syntax.
dnl
dnl For example, if $build_os = cygwin and $host_os = mingw32, filenames need
dnl be converted from Cygwin syntax to native Windows syntax:
dnl   /cygdrive/c/foo/bar -> C:\foo\bar
dnl   /usr/local/share    -> C:\cygwin64\usr\local\share
dnl
dnl gl_BUILD_TO_HOST([somedir])
dnl This macro takes as input an AC_SUBSTed variable 'somedir', which must
dnl already have its final value assigned, and produces two additional
dnl AC_SUBSTed variables 'somedir_c' and 'somedir_c_make', that designate the
dnl same file name value, just in different syntax:
dnl   - somedir_c       is the file name in target runtime environment syntax,
dnl                     as a C string (starting and ending with a double-quote,
dnl                     and with escaped backslashes and double-quotes in
dnl                     between).
dnl   - somedir_c_make  is the same thing, escaped for use in a Makefile.

AC_DEFUN([gl_BUILD_TO_HOST],
[
  AC_REQUIRE([AC_CANONICAL_BUILD])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([gl_BUILD_TO_HOST_INIT])

  dnl Define somedir_c.
  gl_final_[$1]="$[$1]"
  dnl Translate it from build syntax to host syntax.
  case "$build_os" in
    cygwin*)
      case "$host_os" in
        mingw* | windows*)
          gl_final_[$1]=`cygpath -w "$gl_final_[$1]"` ;;
      esac
      ;;
  esac
  dnl Convert it to C string syntax.
  [$1]_c=`printf '%s\n' "$gl_final_[$1]" | sed -e "$gl_sed_double_backslashes" -e "$gl_sed_escape_doublequotes" | tr -d "$gl_tr_cr"`
  [$1]_c='"'"$[$1]_c"'"'
  AC_SUBST([$1_c])

  dnl Define somedir_c_make.
  [$1]_c_make=`printf '%s\n' "$[$1]_c" | sed -e "$gl_sed_escape_for_make_1" -e "$gl_sed_escape_for_make_2" | tr -d "$gl_tr_cr"`
  dnl Use the substituted somedir variable, when possible, so that the user
  dnl may adjust somedir a posteriori when there are no special characters.
  if test "$[$1]_c_make" = '\"'"${gl_final_[$1]}"'\"'; then
    [$1]_c_make='\"$([$1])\"'
  fi
  AC_SUBST([$1_c_make])
])

dnl Some initializations for gl_BUILD_TO_HOST.
AC_DEFUN([gl_BUILD_TO_HOST_INIT],
[
  gl_sed_double_backslashes='s/\\/\\\\/g'
  gl_sed_escape_doublequotes='s/"/\\"/g'
changequote(,)dnl
  gl_sed_escape_for_make_1="s,\\([ \"&'();<>\\\\\`|]\\),\\\\\\1,g"
changequote([,])dnl
  gl_sed_escape_for_make_2='s,\$,\\$$,g'
  dnl Find out how to remove carriage returns from output. Solaris /usr/ucb/tr
  dnl does not understand '\r'.
  case `echo r | tr -d '\r'` in
    '') gl_tr_cr='\015' ;;
    *)  gl_tr_cr='\r' ;;
  esac
])
