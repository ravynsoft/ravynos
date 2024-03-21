#
# SYNOPSIS
#
#   AX_FUNC_GETADDRINFO
#
# DESCRIPTION
#
#   Checks for the getaddrinfo function in the standard C library,
#   as well as the socket and inet libraries, if they are present.
#   If extra libraries are required, they are added to LIBS.
#   If no getaddrinfo function is found, it is added to LIBOBJS.
#   Note: Tru64 UNIX contains two versions of getaddrinfo and we must
#   include netdb.h to get the proper definition.
#
# LICENSE
#
#   Placed in the public domain by Todd C. Miller on November 20, 2013.
#

AC_DEFUN([AX_FUNC_GETADDRINFO],
[AC_MSG_CHECKING(for getaddrinfo)
AC_CACHE_VAL(ax_cv_func_getaddrinfo,
[AC_LINK_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
int main() { return getaddrinfo(0, 0, 0, 0); }]])], [ax_cv_func_getaddrinfo=yes], [ax_cv_func_getaddrinfo=no])])
AC_MSG_RESULT([$ax_cv_func_getaddrinfo])
if test X"$ax_cv_func_getaddrinfo" = X"yes"; then
    AC_DEFINE(HAVE_GETADDRINFO, 1, [Define to 1 if you have the 'getaddrinfo' function.])
else
    # Not found in libc, check libsocket and libinet
    _found=no
    for _libs in "-lsocket" "-linet" "-lsocket -lnsl"; do
	_cv="ax_cv_lib_getaddrinfo`echo \"$_libs\"|sed -e 's/-l/_/g' -e 's/ *//g'`"
	AC_MSG_CHECKING([for getaddrinfo in $_libs])
	AC_CACHE_VAL([$_cv], [
	    _nlibs=
	    for _l in $_libs; do
		case "$LIBS" in
		    *"$_l"*)	;;
		    *)		_nlibs="$_nlibs $_l";;
		esac
	    done
	    _libs="${_nlibs# }"
	    if test -z "$_libs"; then
		# No new libs to check
		eval $_cv=no
	    else
		AX_FUNC_GETADDRINFO_OLIBS="$LIBS"
		LIBS="$LIBS $_libs"
		AC_LINK_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    int main() { return getaddrinfo(0, 0, 0, 0); }]])], [eval $_cv=yes], [eval $_cv=no])
		LIBS="$AX_FUNC_GETADDRINFO_OLIBS"
	    fi
	])
	if eval test \$$_cv = "yes"; then
	    AC_MSG_RESULT([yes])
	    AC_DEFINE(HAVE_GETADDRINFO)
	    test -n "$_libs" && LIBS="$LIBS $_libs"
	    break
	fi
	AC_MSG_RESULT([no])
    done
    if eval test \$$_cv != "yes"; then
	AC_LIBOBJ(getaddrinfo)
    fi
fi
])
