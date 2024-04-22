dnl @synopsis AC_CHECK_SYMBOL(SYMBOL, HEADER... [,ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]])
dnl
dnl a wrapper around AC_EGREP_HEADER the shellvar $ac_found will hold
dnl the HEADER-name that had been containing the symbol. This value is
dnl shown to the user.
dnl
dnl @category C
dnl @author Guido U. Draheim <guidod@gmx.de>
dnl @version 2006-10-13
dnl @license GPLWithACException

AC_DEFUN([AC_CHECK_SYMBOL],
[AC_MSG_CHECKING([for $1 in $2])
AC_CACHE_VAL(ac_cv_func_$1,
[AC_REQUIRE_CPP()dnl
changequote(, )dnl
symbol="[^a-zA-Z_0-9]$1[^a-zA-Z_0-9]"
changequote([, ])dnl
ac_found=no
for ac_header in $2 ; do
  ac_safe=`echo "$ac_header" | sed 'y%./+-%__p_%' `
  if test $ac_found != "yes" ; then
      if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
            AC_EGREP_HEADER( $symbol, $ac_header, [ac_found="$ac_header"] )
      fi
  fi
done
if test "$ac_found" != "no" ; then
  AC_MSG_RESULT($ac_found)
  ifelse([$3], , :, [$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4], , , [$4
])dnl
fi
])])

dnl AC_CHECK_SYMBOLS( symbol..., header... [, action-if-found [, action-if-not-found]])
AC_DEFUN([AC_CHECK_SYMBOLS],
[for ac_func in $1
do
P4_CHECK_SYMBOL($ac_func, $2,
[changequote(, )dnl
  ac_tr_func=HAVE_`echo $ac_func | sed -e 'y:abcdefghijklmnopqrstuvwxyz:ABCDEFGHIJKLMNOPQRSTUVWXYZ:' -e 's:[[^A-Z0-9]]:_:'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_func) $2], $3)dnl
done
])
