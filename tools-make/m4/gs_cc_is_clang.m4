# SYNOPSIS
#
#   GS_CHECK_CC_IS_CLANG([run-if-true],[run-if-false])
#
# DESCRIPTION
#
#   This macro checks whether the active C compiler is a variant of clang. Upon return
#
#     * The makefile variable `CLANG_CC' is set to `yes' or `no'.
#     * The variables `CLANG_CC' and `gs_cv_cc_is_clang' are set to the same values.
#     * Additionally if clang, run shell code run-if-true
#       else run shell code run-if-false.
AC_DEFUN([GS_CHECK_CC_IS_CLANG],dnl
  [AC_REQUIRE([AC_PROG_CC])
    AC_CACHE_CHECK([whether the compiler is clang],[_gs_cv_cc_is_clang], [dnl
    _gs_cv_cc_is_clang="no"
    if "${CC}" -v 2>&1 | grep -q 'clang version'; then
        _gs_cv_cc_is_clang="yes";
    fi
    ])
    AS_VAR_SET([gs_cv_cc_is_clang], [${_gs_cv_cc_is_clang}])
    AS_VAR_SET([CLANG_CC], [${_gs_cv_cc_is_clang}])
    AC_SUBST([CLANG_CC])
    AS_VAR_IF([_gs_cv_cc_is_clang], ["yes"], [$1], [$2])
])