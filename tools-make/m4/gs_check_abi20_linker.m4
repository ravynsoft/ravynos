# SYNOPSIS
#
#   GS_CHECK_ABI20_LINKER([run-if-true],[run-if-false])
#
# DESCRIPTION
#
#   This macro checks whether we are using a linker that has known problems with the gnustep-2.0 ABI.
#   If so, we currently just print a warning because we don't have a 100% accurate way of checking yet.
#
AC_DEFUN([GS_CHECK_ABI20_LINKER], [dnl
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_GREP])
    AC_CACHE_CHECK([for an gnustep-2.0 ABI compatible linker],[gs_cv_abi20_linker], [dnl
        gs_cv_abi20_linker="unkown"
        AS_VAR_PUSHDEF([LD], [gs_cv_abi20_linker_prog])
        LD=$($CC --print-prog-name=ld)
        if $LD --version | $GREP -q 'GNU ld'; then
            gs_cv_abi20_linker="unlikely (GNU ld)"
        elif $LD --version | $GREP -q 'GNU gold'; then
            gs_cv_abi20_linker="yes (GNU gold)"
        elif $LD --version | $GREP -q 'LLD'; then
            gs_cv_abi20_linker="yes (LLD)"
        fi
        AS_VAR_POPDEF([LD])
    ])
    if echo "$gs_cv_abi20_linker" | $GREP -q '^yes'; then
        _gs_abi20_linker=yes
    else
        _gs_abi20_linker=no
        AC_MSG_WARN([The detected linker might not produce working Objective-C binaries using the gnustep-2.0 ABI. Consider using gold or LLD.])
    fi
    AS_VAR_IF([_gs_abi20_linker], ["yes"], [$1], [$2])
])