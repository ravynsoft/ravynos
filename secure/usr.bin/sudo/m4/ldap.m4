AC_DEFUN([SUDO_CHECK_LDAP], [
    if test ${with_ldap-'no'} != "no"; then
	O_LDFLAGS="$LDFLAGS"
	if test "$with_ldap" != "yes"; then
	    SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${with_ldap}/lib])
	    LDFLAGS="$LDFLAGS -L${with_ldap}/lib"
	    if test -d "${with_ldap}/lib64"; then
		SUDO_APPEND_LIBPATH(SUDOERS_LDFLAGS, [${with_ldap}/lib64])
		LDFLAGS="$LDFLAGS -L${with_ldap}/lib64"
	    fi
	    AX_APPEND_FLAG([-I${with_ldap}/include], [CPPFLAGS])
	    with_ldap=yes
	fi
	SUDOERS_OBJS="${SUDOERS_OBJS} ldap.lo ldap_conf.lo ldap_innetgr.lo"
	case "$SUDOERS_OBJS" in
	    *ldap_util.lo*) ;;
	    *) SUDOERS_OBJS="${SUDOERS_OBJS} ldap_util.lo";;
	esac
	LDAP=""

	_LIBS="$LIBS"
	LDAP_LIBS=""
	IBMLDAP_EXTRA=""
	found=no
	# On HP-UX, libibmldap has a hidden dependency on libCsup
	case "$host_os" in
	    hpux*) AC_CHECK_LIB([Csup], [main], [IBMLDAP_EXTRA=" -lCsup"]);;
	esac
	AC_SEARCH_LIBS([ldap_init], ["ibmldap${IBMLDAP_EXTRA}" "ibmldap -lidsldif${IBMLDAP_EXTRA}" "ldap" "ldap -llber" "ldap -llber -lssl -lcrypto" "ibmldap${IBMLDAP_EXTRA}]", [
	    test "${ac_cv_search_ldap_init}" != "none required" && LDAP_LIBS="${ac_cv_search_ldap_init}"
	    found=yes
	])
	# If nothing linked, try -lldap and hope for the best
	if test "$found" = "no"; then
	    LDAP_LIBS="-lldap"
	fi
	LIBS="${_LIBS} ${LDAP_LIBS}"

	AC_CHECK_DECL([LBER_OPT_DEBUG_LEVEL], [
	    case "$LDAP_LIBS" in
	    *-llber*)
		# Already linking with -llber
		;;
	    *)	# Link with -llber for ber_set_option() if it exists
		AC_CHECK_LIB([lber], [ber_set_option], [found=yes], [found=no])
		if test X"$found" = X"yes"; then
		    LDAP_LIBS="$LDAP_LIBS -llber"
		fi
		;;
	    esac
	], [], [AC_INCLUDES_DEFAULT
#include <lber.h>])
	AC_CACHE_CHECK([whether lber.h is needed when including ldap.h], [sudo_cv_header_lber_h], [
	    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <ldap.h>]], [[return ldap_msgfree(NULL)]])], [
		# No need to explicitly include lber.h when including ldap.h.
		sudo_cv_header_lber_h=no
	    ], [
		sudo_cv_header_lber_h=yes
	    ])
	])
	if test X"$sudo_cv_header_lber_h" = X"yes"; then
	    AC_DEFINE(HAVE_LBER_H)
	fi

	if test ${enable_sasl-'yes'} = "yes"; then
	    found_sasl_h=no
	    AC_CHECK_HEADERS([sasl/sasl.h] [sasl.h], [
		found_sasl_h=yes
		AC_CHECK_FUNCS([ldap_sasl_interactive_bind_s])
		break
	    ])
	    if test X${enable_sasl} = X"yes"; then
		if test X"$found_sasl_h" != X"yes"; then
		    AC_MSG_ERROR([--enable-sasl specified but unable to locate SASL development headers.])
		fi
		if test X"$ac_cv_func_ldap_sasl_interactive_bind_s" != X"yes"; then :
		    AC_MSG_ERROR([--enable-sasl specified but SASL support is missing in your LDAP library])
		fi
	    fi
	fi
	AC_CHECK_HEADERS([ldapssl.h] [ldap_ssl.h] [mps/ldap_ssl.h], [break], [], [#include <ldap.h>])
	AC_CHECK_FUNCS([ldap_initialize ldap_start_tls_s ldapssl_init ldapssl_set_strength ldap_unbind_ext_s ldap_str2dn ldap_create ldap_sasl_bind_s ldap_ssl_init ldap_ssl_client_init ldap_start_tls_s_np])
	AC_CHECK_FUNCS([ldap_search_ext_s ldap_search_st], [break])

	if test X"$check_gss_krb5_ccache_name" = X"yes"; then
	    AC_CHECK_LIB([gssapi], [gss_krb5_ccache_name], [
		AC_DEFINE(HAVE_GSS_KRB5_CCACHE_NAME)
		LDAP_LIBS="${LDAP_LIBS} -lgssapi"
	    ], [
		AC_CHECK_LIB(gssapi_krb5, gss_krb5_ccache_name, [
		    AC_DEFINE(HAVE_GSS_KRB5_CCACHE_NAME)
		    LDAP_LIBS="${LDAP_LIBS} -lgssapi_krb5"
		])
	    ])

	    # gssapi headers may be separate or part of Kerberos V
	    found=no
	    O_CPPFLAGS="$CPPFLAGS"
	    for dir in "" "kerberosV" "krb5" "kerberos5" "kerberosv5"; do
		test X"$dir" != X"" && CPPFLAGS="$O_CPPFLAGS -I/usr/include/${dir}"
		# Use AC_PREPROC_IFELSE to check existence to avoid caching
		# since we test with multiple values of CPPFLAGS
		AC_PREPROC_IFELSE([
		    AC_LANG_PROGRAM([[#include <gssapi/gssapi.h>]])
		], [
		    AC_CHECK_HEADERS([gssapi/gssapi.h])
		    break
		], [
		    AC_PREPROC_IFELSE([
			AC_LANG_PROGRAM([[#include <gssapi.h>]])
		    ], [
			AC_CHECK_HEADERS([gssapi.h])
			break
		    ])
		])
	    done
	    if test X"$ac_cv_header_gssapi_gssapi_h" != X"no"; then
		AC_CHECK_HEADERS([gssapi/gssapi_krb5.h])
	    elif test X"$ac_cv_header_gssapi_h" = X"no"; then
		CPPFLAGS="$O_CPPFLAGS"
		AC_MSG_WARN([unable to locate gssapi.h, you will have to edit the Makefile and add -I/path/to/gssapi/includes to CPPFLAGS])
	    fi
	fi

	SUDOERS_LIBS="${SUDOERS_LIBS} ${LDAP_LIBS}"
	LIBS="$_LIBS"
	LDFLAGS="$O_LDFLAGS"
    fi
])
