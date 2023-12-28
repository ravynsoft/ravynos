# DEJAGNU_CHECK_VERSION DEJAGNU_CHECK_VERSION
# --------------------------------------------------------------
# Test whether there is an incompatibility between DejaGnu and GCC versions.
# Older versions ( <= 1.5.1 ) of dejagnu.h use GNU inline semantics improperly.
# The issue presents itself as link-time errors complaining about undefined
# references to 'pass' and 'fail'.
AC_DEFUN([DEJAGNU_CHECK_VERSION],
[
  AC_MSG_CHECKING([for incompatibility between DejaGnu and GCC])
  AC_MSG_RESULT([$ac_cv_dejagnu_compat])

  AC_TRY_LINK([#include <dejagnu.h>],
	      [pass ("test foo");
	       return 0;],
	      [ac_cv_dejagnu_compat=yes],
	      [ac_cv_dejagnu_compat=no])
  AC_MSG_RESULT([$ac_cv_dejagnu_compat])

  if test "$ac_cv_dejagnu_compat}" = no ; then
    AC_MSG_RESULT([detected incompatibility between dejagnu version and gcc])
  fi
])
