# SYNOPSIS
#
#   GS_CHECK_GCC_VERSION()
#
# DESCRIPTION
#
#   Extracts the major and minor version numbers from a GCC-like compiler (GCC or clang)
#   into the following variables:
#   * gs_cv_gcc_major_version
#   * gs_cv_gcc_minor_version
#   * gs_cv_gcc_parsed_version (the combined version string)
AC_DEFUN([GS_CHECK_GCC_VERSION],dnl
  [AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_AWK])
  AC_REQUIRE([GS_CHECK_CC_IS_CLANG])
  if test x"${gs_cv_cc_is_clang}" = x"yes"; then
    compiler_identification="clang"
  else
    compiler_identification="GCC"
  fi
  AC_CACHE_CHECK([for the ${compiler_identification} version],[_gs_cv_gcc_parsed_version], [dnl
    _gs_cv_gcc_major_version=""
    _gs_cv_gcc_minor_version=""
    _gs_cv_gcc_parsed_version="no: it's not gcc"
    if test x"${GCC}" = x"yes" ; then
      dnl Running gcc -dumpversion we get something like 2.95.4 or
      dnl  egcs-2.91.66 or 3.0.2 or 3.1 20011211
      dnl We want to discard anything but the major number.
      dnl Explanation of the regexp -
      dnl \([^0-9]*\) matches non numeric chars at the beginning
      dnl \([0-9][0-9]*\) matches 1 or more numeric chars (this is the 2^nd
      dnl  subpattern)
      dnl \([^0-9]*\) matches one or more non numeric chars
      dnl \([0-9][0-9]*\) matches 1 or more numeric chars (this is the 4^nd
      dnl  subpattern)
      dnl \([^0-9].*\) matches a non numeric char followed by anything
      dnl /\2/ replace the whole lot with the 2^nd subpattern
      dnl /\4/ replace the whole lot with the 4^nd subpattern
      dnl All square brackets are doubled because this file is processed by m4 first.
      dnl Finally, any error messages are redirected to &5, so that they are logged
      dnl into config.log but don't clutter the normal user output.
      _gs_cv_gcc_major_version=`(${CC} -dumpfullversion -dumpversion | sed "s/\([[^0-9]]*\)\([[0-9]][[0-9]]*\)\([[^0-9]]*\)\([[0-9]][[0-9]]*\)\([[^0-9]]*.*\)/\2/") 2>&5`;
      _gs_cv_gcc_minor_version=`(${CC} -dumpfullversion -dumpversion | sed "s/\([[^0-9]]*\)\([[0-9]][[0-9]]*\)\([[^0-9]]*\)\([[0-9]][[0-9]]*\)\([[^0-9]]*.*\)/\4/") 2>&5`;
      _gs_cv_gcc_parsed_version="${_gs_cv_gcc_major_version}.${_gs_cv_gcc_minor_version}";
    fi
  ])
  if test ! x"${_gs_cv_gcc_parsed_version}" = x"no: it's not gcc"; then
     _gs_cv_gcc_major_version=$(echo $_gs_cv_gcc_parsed_version | $AWK -F. '{ print $[1] }')
     _gs_cv_gcc_minor_version=$(echo $_gs_cv_gcc_parsed_version | $AWK -F. '{ print $[2] }')
  fi
  AS_VAR_IF([_gs_cv_gcc_major_version], [""], [AS_UNSET([gs_cv_gcc_major_version])], [AS_VAR_SET([gs_cv_gcc_major_version], [${_gs_cv_gcc_major_version}])])
  AS_VAR_IF([_gs_cv_gcc_minor_version], [""], [AS_UNSET([gs_cv_gcc_minor_version])], [AS_VAR_SET([gs_cv_gcc_minor_version], [${_gs_cv_gcc_minor_version}])])
  AS_VAR_IF([_gs_cv_gcc_major_version], [""], [AS_UNSET([gs_cv_gcc_parsed_version])], [AS_VAR_SET([gs_cv_gcc_parsed_version], ["${_gs_cv_gcc_major_version}.${_gs_cv_gcc_minor_version}"])])
])
