# warnings.m4 serial 19
dnl Copyright (C) 2008-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Simon Josefsson

AC_PREREQ([2.64])

# gl_COMPILER_OPTION_IF(OPTION, [IF-SUPPORTED], [IF-NOT-SUPPORTED],
#                       [PROGRAM = AC_LANG_PROGRAM()])
# -----------------------------------------------------------------
# Check if the compiler supports OPTION when compiling PROGRAM.
#
# The effects of this macro depend on the current language (_AC_LANG).
AC_DEFUN([gl_COMPILER_OPTION_IF],
[
AS_VAR_PUSHDEF([gl_Warn], [gl_cv_warn_[]_AC_LANG_ABBREV[]_$1])dnl
AS_VAR_PUSHDEF([gl_Flags], [_AC_LANG_PREFIX[]FLAGS])dnl
AS_LITERAL_IF([$1],
  [m4_pushdef([gl_Positive], m4_bpatsubst([$1], [^-Wno-], [-W]))],
  [gl_positive="$1"
case $gl_positive in
  -Wno-*) gl_positive=-W`expr "X$gl_positive" : 'X-Wno-\(.*\)'` ;;
esac
m4_pushdef([gl_Positive], [$gl_positive])])dnl
AC_CACHE_CHECK([whether _AC_LANG compiler handles $1], [gl_Warn], [
  gl_save_compiler_FLAGS="$gl_Flags"
  AS_VAR_APPEND(m4_defn([gl_Flags]),
    [" $gl_unknown_warnings_are_errors ]m4_defn([gl_Positive])["])
  AC_LINK_IFELSE([m4_default([$4], [AC_LANG_PROGRAM([[]])])],
                 [AS_VAR_SET([gl_Warn], [yes])],
                 [AS_VAR_SET([gl_Warn], [no])])
  gl_Flags="$gl_save_compiler_FLAGS"
])
AS_VAR_IF(gl_Warn, [yes], [$2], [$3])
m4_popdef([gl_Positive])dnl
AS_VAR_POPDEF([gl_Flags])dnl
AS_VAR_POPDEF([gl_Warn])dnl
])

# gl_UNKNOWN_WARNINGS_ARE_ERRORS
# ------------------------------
# Clang doesn't complain about unknown warning options unless one also
# specifies -Wunknown-warning-option -Werror.  Detect this.
#
# The effects of this macro depend on the current language (_AC_LANG).
AC_DEFUN([gl_UNKNOWN_WARNINGS_ARE_ERRORS],
[_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])

# Specialization for _AC_LANG = C. This macro can be AC_REQUIREd.
AC_DEFUN([gl_UNKNOWN_WARNINGS_ARE_ERRORS(C)],
[
  AC_LANG_PUSH([C])
  gl_UNKNOWN_WARNINGS_ARE_ERRORS_IMPL
  AC_LANG_POP([C])
])

# Specialization for _AC_LANG = C++. This macro can be AC_REQUIREd.
AC_DEFUN([gl_UNKNOWN_WARNINGS_ARE_ERRORS(C++)],
[
  AC_LANG_PUSH([C++])
  gl_UNKNOWN_WARNINGS_ARE_ERRORS_IMPL
  AC_LANG_POP([C++])
])

# Specialization for _AC_LANG = Objective C. This macro can be AC_REQUIREd.
AC_DEFUN([gl_UNKNOWN_WARNINGS_ARE_ERRORS(Objective C)],
[
  AC_LANG_PUSH([Objective C])
  gl_UNKNOWN_WARNINGS_ARE_ERRORS_IMPL
  AC_LANG_POP([Objective C])
])

AC_DEFUN([gl_UNKNOWN_WARNINGS_ARE_ERRORS_IMPL],
[gl_COMPILER_OPTION_IF([-Werror -Wunknown-warning-option],
   [gl_unknown_warnings_are_errors='-Wunknown-warning-option -Werror'],
   [gl_unknown_warnings_are_errors=])])

# gl_WARN_ADD(OPTION, [VARIABLE = WARN_CFLAGS/WARN_CXXFLAGS],
#             [PROGRAM = AC_LANG_PROGRAM()])
# -----------------------------------------------------------
# Adds OPTION to VARIABLE (which defaults to WARN_CFLAGS or WARN_CXXFLAGS)
# if the compiler supports it when compiling PROGRAM.
#
# If VARIABLE is a variable name, AC_SUBST it.
#
# The effects of this macro depend on the current language (_AC_LANG).
#
# Example: gl_WARN_ADD([-Wparentheses]).
AC_DEFUN([gl_WARN_ADD],
[AC_REQUIRE([gl_UNKNOWN_WARNINGS_ARE_ERRORS(]_AC_LANG[)])
gl_COMPILER_OPTION_IF([$1],
  [AS_VAR_APPEND(m4_if([$2], [], [[WARN_]_AC_LANG_PREFIX[FLAGS]], [[$2]]), [" $1"])],
  [],
  [$3])
m4_ifval([$2],
         [AS_LITERAL_IF([$2], [AC_SUBST([$2])])],
         [AC_SUBST([WARN_]_AC_LANG_PREFIX[FLAGS])])dnl
])


# gl_CC_INHIBIT_WARNINGS
# sets and substitutes a variable GL_CFLAG_INHIBIT_WARNINGS, to a $(CC) option
# that reverts all preceding -W* options, if available.
# This is expected to be '-w' at least on gcc, clang, AIX xlc, xlclang, Sun cc,
# "compile cl" (MSVC), "compile clang-cl" (MSVC-compatible clang). Or it can be
# empty.
AC_DEFUN([gl_CC_INHIBIT_WARNINGS],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_CACHE_CHECK([for C compiler option to inhibit all warnings],
    [gl_cv_cc_winhibit],
    [rm -f conftest*
     echo 'int dummy;' > conftest.c
     AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS -c conftest.c 2>conftest1.err]) >/dev/null
     AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS -w -c conftest.c 2>conftest2.err]) >/dev/null
     if test $? = 0 && test `wc -l < conftest1.err` = `wc -l < conftest2.err`; then
       gl_cv_cc_winhibit='-w'
     else
       gl_cv_cc_winhibit=none
     fi
     rm -f conftest*
    ])
  case "$gl_cv_cc_winhibit" in
    none) GL_CFLAG_INHIBIT_WARNINGS='' ;;
    *)
      GL_CFLAG_INHIBIT_WARNINGS="$gl_cv_cc_winhibit"
      dnl If all warnings are inhibited, there's no point in having the GCC
      dnl analyzer enabled. This saves RAM requirements and CPU consumption.
      gl_WARN_ADD([-fno-analyzer], [GL_CFLAG_INHIBIT_WARNINGS])
      ;;
  esac
  AC_SUBST([GL_CFLAG_INHIBIT_WARNINGS])
])

# gl_CXX_INHIBIT_WARNINGS
# sets and substitutes a variable GL_CXXFLAG_INHIBIT_WARNINGS, to a $(CC) option
# that reverts all preceding -W* options, if available.
AC_DEFUN([gl_CXX_INHIBIT_WARNINGS],
[
  dnl Requires AC_PROG_CXX or gl_PROG_ANSI_CXX.
  if test -n "$CXX" && test "$CXX" != no; then
    AC_CACHE_CHECK([for C++ compiler option to inhibit all warnings],
      [gl_cv_cxx_winhibit],
      [rm -f conftest*
       echo 'int dummy;' > conftest.cc
       AC_TRY_COMMAND([${CXX-c++} $CXXFLAGS $CPPFLAGS -c conftest.cc 2>conftest1.err]) >/dev/null
       AC_TRY_COMMAND([${CXX-c++} $CXXFLAGS $CPPFLAGS -w -c conftest.cc 2>conftest2.err]) >/dev/null
       if test $? = 0 && test `wc -l < conftest1.err` = `wc -l < conftest2.err`; then
         gl_cv_cxx_winhibit='-w'
       else
         gl_cv_cxx_winhibit=none
       fi
       rm -f conftest*
      ])
    case "$gl_cv_cxx_winhibit" in
      none) GL_CXXFLAG_INHIBIT_WARNINGS='' ;;
      *)
        GL_CXXFLAG_INHIBIT_WARNINGS="$gl_cv_cxx_winhibit"
        dnl If all warnings are inhibited, there's no point in having the GCC
        dnl analyzer enabled. This saves RAM requirements and CPU consumption.
        gl_WARN_ADD([-fno-analyzer], [GL_CXXFLAG_INHIBIT_WARNINGS])
        ;;
    esac
  else
    GL_CXXFLAG_INHIBIT_WARNINGS=''
  fi
  AC_SUBST([GL_CXXFLAG_INHIBIT_WARNINGS])
])


# Local Variables:
# mode: autoconf
# End:
