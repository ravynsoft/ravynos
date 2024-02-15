AC_DEFUN([SUDO_CHECK_GETTEXT], [
    # gettext() and friends may be located in libc (Linux and Solaris)
    # or in libintl.  However, it is possible to have libintl installed
    # even when gettext() is present in libc.  In the case of GNU libintl,
    # gettext() will be defined to gettext_libintl in libintl.h.
    # Since gcc prefers /usr/local/include to /usr/include, we need to
    # make sure we use the gettext() that matches the include file.
    if test "$enable_nls" != "no"; then
	if test "$enable_nls" != "yes"; then
	    AX_APPEND_FLAG([-I${enable_nls}/include], [CPPFLAGS])
	    SUDO_APPEND_LIBPATH(LDFLAGS, [$enable_nls/lib])
	fi
	OLIBS="$LIBS"
	for l in "libc" "-lintl" "-lintl -liconv"; do
	    if test "$l" = "libc"; then
		# If user specified a dir for libintl ignore libc
		if test "$enable_nls" != "yes"; then
		    continue
		fi
		gettext_name=sudo_cv_gettext
		AC_MSG_CHECKING([for gettext])
	    else
		LIBS="$OLIBS $l"
		gettext_name=sudo_cv_gettext"`echo $l|sed -e 's/ //g' -e 's/-/_/g'`"
		AC_MSG_CHECKING([for gettext in $l])
	    fi
	    AC_CACHE_VAL($gettext_name, [
		    AC_LINK_IFELSE(
			[
			    AC_LANG_PROGRAM([[#include <libintl.h>]], [(void)gettext((char *)0);])
			], [eval $gettext_name=yes], [eval $gettext_name=no]
		    )
	    ])
	    eval gettext_result="\$$gettext_name"
	    AC_MSG_RESULT($gettext_result)
	    if test "$gettext_result" = "yes"; then
		AC_CHECK_FUNCS([ngettext])
		break
	    fi
	done
	LIBS="$OLIBS"

	if test "$sudo_cv_gettext" = "yes"; then
	    SUDO_NLS=enabled
	    # For Solaris we need links from lang to lang.UTF-8 in localedir
	    case "$host_os" in
		solaris2*) LOCALEDIR_SUFFIX=".UTF-8";;
	    esac
	elif test "$sudo_cv_gettext_lintl" = "yes"; then
	    SUDO_NLS=enabled
	    LIBINTL="-lintl"
	elif test "$sudo_cv_gettext_lintl_liconv" = "yes"; then
	    SUDO_NLS=enabled
	    LIBINTL="-lintl -liconv"
	fi
	if test X"$SUDO_NLS" = X"enabled"; then
	    AC_DEFINE(HAVE_LIBINTL_H)
	    SUDO_APPEND_COMPAT_EXP(sudo_warn_gettext_v1)
	fi
    fi
])
