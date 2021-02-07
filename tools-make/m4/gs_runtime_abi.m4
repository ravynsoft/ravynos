# SYNOPSIS
#
#   GS_RUNTIME_ABI([default_runtime_abi])
#
# DESCRIPTION
#
# This macro determine the correct runtime ABI to use.
#
AC_DEFUN([GS_RUNTIME_ABI],dnl
  [AC_REQUIRE([GS_LIBRARY_COMBO])
   AC_REQUIRE([GS_CHECK_RUNTIME_ABI20_SUPPORT])
   AC_REQUIRE([GS_CHECK_CC_IS_CLANG])
   AC_REQUIRE([GS_CHECK_GCC_VERSION])
   case "$OBJC_RUNTIME_LIB" in
        gnu) default_runtime_abi=gcc         ;;
        ng)  default_runtime_abi=gnustep-1.8 ;;
        *)   default_runtime_abi="(unknown)" ;;
    esac
    if test ! x"$1" = x""; then
        default_runtime_abi="$1"
    elif test x"$OBJC_RUNTIME_LIB" = x"ng" -a x"$libobjc_SUPPORTS_ABI20" = x"yes" -a x"$CLANG_CC" = x"yes" -a "${gs_cv_gcc_major_version}" -ge "8"; then
        default_runtime_abi=gnustep-2.0
    fi
    AC_ARG_WITH([runtime-abi],
        [AS_HELP_STRING([--with-runtime-abi], [
            When using the ng runtime library, allows control over the -fobjc-runtime= flag passed to clang.
        ])],,
        [with_runtime_abi=]${default_runtime_abi})
  AC_CACHE_CHECK([for runtime ABI],[_gs_cv_runtime_abi], [
    _gs_cv_runtime_abi=${with_runtime_abi}
  ])
  AS_VAR_IF([_gs_cv_runtime_abi], ["(unknown)"], [AS_UNSET(gs_cv_runtime_abi)], [AS_VAR_SET([gs_cv_runtime_abi], [${_gs_cv_runtime_abi}])]) 
  AC_SUBST([gs_cv_runtime_abi])
])