dnl Code shamelessly stolen from glib-config by Sebastian Rittau
dnl  Copyright (C) 2005 Free Software Foundation
dnl  Copying and distribution of this file, with or without modification,
dnl  are permitted in any medium without royalty provided the copyright
dnl  notice and this notice are preserved.
dnl AM_PATH_TLS([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AC_DEFUN(AM_PATH_TLS,[
AC_ARG_WITH(tls-prefix,
            [  --with-tls-prefix=PFX    Prefix where libgnutls is installed (optional)],
            tls_config_prefix="$withval", tls_config_prefix="")
AC_ARG_ENABLE(tlstest,
              [  --disable-tlstest		Do not try to compile and run a test TLS program],,
              enable_tlstest=yes)

  if test x$tls_config_prefix != x ; then
    tls_config_args="$tls_config_args --prefix=$tls_config_prefix"
    if test x${TLS_CONFIG+set} != xset ; then
      TLS_CONFIG=$tls_config_prefix/bin/libgnutls-config
    fi
  fi
  if test ! -x "$TLS_CONFIG" ; then
    unset TLS_CONFIG
  fi

  AC_PATH_PROG(TLS_CONFIG, libgnutls-config, no)
  min_tls_version=ifelse([$1], ,2.0.0, [$1])
  AC_MSG_CHECKING(for libgnutls - version >= $min_tls_version)
  no_tls=""
  if test "$TLS_CONFIG" = "no" ; then
    if test x$tls_config_prefix != x ; then
      TLS_CFLAGS="-I$tls_config_prefix/include"
      TLS_LIBS="-L$tls_config_prefix/lib -lgnutls -lgcrypt"
    else
      TLS_CFLAGS="-I/usr/include"
      TLS_LIBS="-L/usr/lib -lgnutls -lgcrypt"
    fi

    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $TLS_CFLAGS"
    LIBS="$TLS_LIBS $LIBS"
dnl
dnl Now check if the installed libgnutls is sufficiently new.
dnl
    rm -f conf.tlstest
    AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>
#include <gnutls/gnutls.h>

int
main()
{
  system("touch conf.tlstest");

  if (gnutls_check_version("$min_tls_version") == 0)
    {
      printf("\n*** An old version of libgnutls (%s) was found.\n",
	gnutls_check_version(0));
      printf("*** You need a version of libtgnuls newer than $min_tls_version.\n");
      printf("*** If you have already installed a sufficiently new version, this error\n");
      printf("*** probably means that the wrong copy of the libgnutls-config shell script is\n");
      printf("*** being found. You can fix this is by removing the old version\n");
      printf("*** of libgnutls.\n");
      return 1;
    }
  return 0;
}
],, no_tls=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])

    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"
  else
    TLS_CFLAGS=`$TLS_CONFIG $tls_config_args --cflags`
    TLS_LIBS=`$TLS_CONFIG $tls_config_args --libs`
    tls_config_major_version=`$TLS_CONFIG $tls_config_args --version | \
      sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    tls_config_minor_version=`$TLS_CONFIG $tls_config_args --version | \
      sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    tls_config_micro_version=`$TLS_CONFIG $tls_config_args --version | \
      sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    if test "x$enable_tlstest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $TLS_CFLAGS"
      LIBS="$TLS_LIBS $LIBS"
dnl
dnl Now check if the installed libtgnuls is sufficiently new.
dnl
      rm -f conf.tlstest
      AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>
#include <gnutls/gnutls.h>

int
main()
{
  system("touch conf.tlstest");

  if (gnutls_check_version("$min_tls_version") == 0)
    {
      printf("\n*** An old version of libgnutls (%s) was found.\n",
	gnutls_check_version(0));
      printf("*** You need a version of libtgnuls newer than $min_tls_version.\n");
      printf("*** If you have already installed a sufficiently new version, this error\n");
      printf("*** probably means that the wrong copy of the libgnutls-config shell script is\n");
      printf("*** being found. You can fix this is by removing the old version\n");
      printf("*** of libgnutls.\n");
      return 1;
    }
  return 0;
}
],, no_tls=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])

      CFLAGS="$ac_save_CFLAGS"
      LIBS="$ac_save_LIBS"
    fi
  fi

  if test "x$no_tls" = x ; then
    AC_MSG_RESULT(yes)
    ifelse([$2], , :, [$2])
  else
    AC_MSG_RESULT(no)
    if test "$TLS_CONFIG" = "no" ; then
      echo "*** The libgnutls-config script installed by libgnutls could not be found"
      echo "*** If libtgnuls-config was installed in PREFIX, make sure PREFIX/bin is in"
      echo "*** your path."
    else
      if test -f conf.tlstest ; then
        :
      else
        echo "*** Could not run libtgnuls test program, checking why..."
        CFLAGS="$CFLAGS $TLS_CFLAGS"
        LIBS="$LIBS $TLS_LIBS"
        dnl FIXME: AC_TRY_LINK
      fi
    fi

    TLS_CFLAGS=""
    TLS_LIBS=""
    ifelse([$3], , :, [$3])
  fi
  AC_SUBST(TLS_CFLAGS)
  AC_SUBST(TLS_LIBS)
  rm -f conf.tlstest
])

