AC_DEFUN([AC_CHECK_ICU], [
  ok=no

  if test -z "$ICU_CONFIG"; then
    AC_PATH_PROG(ICU_CONFIG, icu-config, no)
  fi

  if test "$ICU_CONFIG" = "no" ; then
    echo "Could not find the icu-config script."
    echo "Please ensure that it is in your path."
    echo "See http://site.icu-project.org/ for help."
  else
    ICU_VERSION=`$ICU_CONFIG --version`
    AC_MSG_CHECKING(for ICU >= $1)
      found=`expr $ICU_VERSION \>= $1`
      if test "$found" = "1" ; then
	AC_MSG_RESULT(yes)
	ok=yes

	AC_MSG_CHECKING(ICU_LIBS)
	ICU_LIBS=`$ICU_CONFIG --ldflags-libsonly`
	AC_MSG_RESULT($ICU_LIBS)
	AC_MSG_CHECKING(ICU_LDFLAGS)
	ICU_LDFLAGS=`$ICU_CONFIG --ldflags-searchpath`
	AC_MSG_RESULT($ICU_LDFLAGS)
      else
	ICU_LIBS=""
	ICU_LDFLAGS=""
	## Either perform custom action or print error message
	ifelse([$3], ,echo "can't find ICU >= $1 (got $ICU_VERSION)",)
      fi

      AC_SUBST(ICU_LIBS)
  fi

  if test $ok = yes; then
    ifelse([$2], , :, [$2])
  else
    ifelse([$3], , AC_MSG_ERROR([Library requirements (ICU) not met.]), [$3])
  fi
])
