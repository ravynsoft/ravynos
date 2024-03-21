dnl @synopsis CHECK_VISIBILITY([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl @summary check for the gcc -fvisibility flag
dnl

AC_DEFUN([CHECK_VISIBILITY_HIDDEN], [
  save_CFLAGS="$CFLAGS"
  VISIBILITY_HIDDEN_CFLAGS=""
  OPTION=-fvisibility=hidden

  AC_MSG_CHECKING(for gcc $OPTION support)

  CFLAGS="$CFLAGS $OPTION"

  AC_TRY_COMPILE([
      int default_vis __attribute__ ((visibility("default")));
      int hidden_vis __attribute__ ((visibility("hidden")));
    ],
    [],
    ac_visibility_supported=yes,
    ac_visibility_supported=no)
  AC_MSG_RESULT($ac_visibility_supported)

  if test x"$ac_visibility_supported" = xyes; then
    ifelse([$1],,AC_DEFINE(HAVE_GCC_VISIBILITY,1,[Define if you have gcc -fvisibility=hidden support ]),[$1])
    VISIBILITY_HIDDEN_CFLAGS="$OPTION -DHAVE_VISIBILITY_HIDDEN"
    AC_DEFINE(HAVE_VISIBILITY_HIDDEN,[],[Support for visibility hidden])
  else
    $2
    :
  fi

  AC_SUBST(VISIBILITY_HIDDEN_CFLAGS)

  CFLAGS="$save_CFLAGS"
])
