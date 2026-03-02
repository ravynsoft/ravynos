AC_DEFUN([SUDO_CHECK_OPENSSL], [
    openssl_missing=no
    if test "${enable_openssl-no}" != no; then
	# Use pkg-config to find the openssl cflags and libs if possible.
	if test "$enable_openssl" != "yes" -a "$enable_openssl" != "maybe"; then
	    PKG_CONFIG_LIBDIR=
	    for d in ${enable_openssl}/*/pkgconfig; do
		if test -d "$d"; then
		    PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR:$d"
		fi
	    done
	    if test -n "$PKG_CONFIG_LIBDIR"; then
		PKG_CONFIG_LIBDIR=${PKG_CONFIG_LIBDIR#:}
		export PKG_CONFIG_LIBDIR
	    fi
	elif test "$cross_compiling" = "yes" -a -z "$PKG_CONFIG"; then
	    # Cannot use pkg-config when cross-compiling
	    PKG_CONFIG=false
	fi
	: ${PKG_CONFIG='pkg-config'}
	pkg_openssl=`printf $enable_openssl_pkgconfig_template "openssl"`
	pkg_libcrypto=lib`printf $enable_openssl_pkgconfig_template "crypto"`
	if $PKG_CONFIG --exists "$pkg_openssl >= 1.0.1" >/dev/null 2>&1; then
	    AC_DEFINE(HAVE_OPENSSL)
	    if test "$enable_openssl" = "maybe"; then
		enable_openssl=yes
	    fi

	    # Check whether --static is needed (don't assume name of ssl lib)
	    # There may be dependent libraries or -pthread.
	    O_LDFLAGS="$LDFLAGS"
	    LDFLAGS="$LDFLAGS `$PKG_CONFIG --libs-only-L $pkg_openssl`"
	    libssl="`$PKG_CONFIG --libs-only-l $pkg_openssl | sed 's/^ *-l//'`"
	    libssl_extra="`echo $libssl | sed 's/^[[^ ]]* *//'`"
	    libssl="`echo $libssl | sed 's/ .*//'`"
	    AC_CHECK_LIB([$libssl], [SSL_new], [STATIC=""], [STATIC="--static"], [$libssl_extra])
	    LDFLAGS="$O_LDFLAGS"

	    # Use pkg-config to determine OpenSSL libs and cflags
	    for f in `$PKG_CONFIG $STATIC --libs $pkg_openssl`; do
		case "$f" in
		    -L*)
			f="${f#-L}"
			SUDO_APPEND_LIBPATH([LIBTLS], [$f])
			;;
		    *)
			# Do not use AX_APPEND_FLAG as it will break static builds by removing
			# duplicates such as -lz or -latomic which are needed by -lssl and -lcrypto
		        LIBTLS="$LIBTLS $f"
			;;
		esac
	    done
	    if $PKG_CONFIG --exists $pkg_libcrypto >/dev/null 2>&1; then
		# Use OpenSSL's sha2 functions if possible (don't assume name of crypto)
		O_LDFLAGS="$LDFLAGS"
		libcrypto=
		libcrypto_extra=
		for f in `$PKG_CONFIG $STATIC --libs $pkg_libcrypto`; do
		    case "$f" in
			-l*)
			    if test -z "$libcrypto"; then
				libcrypto="${f#-l}"
			    else
				libcrypto_extra="$libcrypto_extra $f"
			    fi
			    ;;
			*)
			    AX_APPEND_FLAG([$f], [LDFLAGS])
			    ;;
			esac
		done
		AC_CHECK_LIB([$libcrypto], [EVP_MD_CTX_new], [DIGEST=digest_openssl.lo], [], [$libcrypto_extra])
		LDFLAGS="$O_LDFLAGS"

		# Use pkg-config to determine libcrypto libs and cflags
		for f in `$PKG_CONFIG $STATIC --libs $pkg_libcrypto`; do
		    case "$f" in
			-L*)
			    f="${f#-L}"
			    SUDO_APPEND_LIBPATH([LIBCRYPTO], [$f])
			    ;;
			*)
			    AX_APPEND_FLAG([$f], [LIBCRYPTO])
			    ;;
		    esac
		done
	    else
		# No separate pkg config for libcrypto
		LIBCRYPTO="$LIBTLS"
		LIBCRYPTO_R="$LIBTLS_R"
	    fi
	    for f in `$PKG_CONFIG --cflags-only-I $pkg_openssl`; do
		AX_APPEND_FLAG([$f], [CPPFLAGS])
	    done
	else
	    # No pkg-config file present, try to do it manually
	    O_LDFLAGS="$LDFLAGS"
	    if test "$enable_openssl" != "yes" -a "$enable_openssl" != "maybe"; then
		SUDO_APPEND_LIBPATH(LDFLAGS, [${enable_openssl}/lib])
	    fi
	    AC_CHECK_LIB([ssl], [SSL_new], [
		# Check OPENSSL_VERSION_NUMBER in headers
		O_CPPFLAGS="$CPPFLAGS"
		if test "$enable_openssl" != "yes" -a "$enable_openssl" != "maybe"; then
		    # Note: we only reset CPPFLAGS on failure
		    AX_APPEND_FLAG([-I${enable_openssl}/include], [CPPFLAGS])
		fi
		AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[#include <openssl/opensslv.h>
#if !defined(OPENSSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER < 0x1000100fL
#error "OpenSSL too old"
#endif
    ]])], [
		# OpenSSL >= 1.0.1 detected, use it.
		AC_DEFINE(HAVE_OPENSSL)
		if test "$enable_openssl" != "yes" -a "$enable_openssl" != "maybe"; then
		    SUDO_APPEND_LIBPATH(LIBCRYPTO, [${enable_openssl}/lib])
		    SUDO_APPEND_LIBPATH(LIBTLS, [${enable_openssl}/lib])
		else
		    enable_openssl=yes
		fi
		LIBCRYPTO="${LIBCRYPTO} -lcrypto"
		LIBTLS="${LIBTLS} -lssl -lcrypto"

		# Use OpenSSL's sha2 functions if possible
		AC_CHECK_LIB([crypto], [EVP_MD_CTX_new], [
		    DIGEST=digest_openssl.lo
		])
    ], [
		# OpenSSL < 1.0.1 detected, ignore it.
		if test "$enable_openssl" = "maybe"; then
		    AC_MSG_WARN([OpenSSL too old (1.0.1 or higher required), Sudo logsrv connections will not be encrypted.])
		    openssl_missing=yes
		    enable_openssl=no
		else
		    AC_MSG_ERROR([OpenSSL too old (1.0.1 or higher required).])
		fi
		CPPFLAGS="$O_CPPFLAGS"
    ])
	    ], [
		if test "$enable_openssl" = "maybe"; then
		    openssl_missing=yes
		    enable_openssl=no
		else
		    AC_MSG_ERROR([OpenSSL development libraries not found.])
		fi
	    ], [-lcrypto])
	    LDFLAGS="$O_LDFLAGS"
	fi
	if test "$enable_openssl" != "yes" -a "$enable_openssl" != "maybe"; then
	    unset PKG_CONFIG_LIBDIR
	fi
    fi
    #
    # Note that enable_openssl may be reset above.
    #
    if test "${enable_openssl-no}" != no; then
	OLIBS="$LIBS"
	LIBS="$LIBS $LIBTLS"
	AC_CHECK_FUNCS([X509_STORE_CTX_get0_cert ASN1_STRING_get0_data SSL_CTX_get0_certificate SSL_CTX_set0_tmp_dh_pkey TLS_method])
	# SSL_CTX_set_min_proto_version may be a macro
	AC_CHECK_DECL([SSL_CTX_set_min_proto_version], [AC_DEFINE(HAVE_SSL_CTX_SET_MIN_PROTO_VERSION)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <openssl/ssl.h>
	])
	AC_CHECK_FUNCS([SSL_read_ex], [], [
	    SSL_COMPAT_SRC=lib/ssl_compat
	])
	# LibreSSL TLS 1.3 support may not be enabled, check for declaration too.
	AC_CHECK_FUNC([SSL_CTX_set_ciphersuites], [
	    AC_CHECK_DECL([SSL_CTX_set_ciphersuites], [AC_DEFINE(HAVE_SSL_CTX_SET_CIPHERSUITES)], [], [
		AC_INCLUDES_DEFAULT
		#include <openssl/ssl.h>
	    ])
	])
	LIBS="$OLIBS"
    elif test "${enable_wolfssl-no}" != no; then
    	# Check for OpenSSL compatibility functions in wolfSSL.
	# Use pkg-config to find the wolfssl cflags and libs if possible.
	if test "$enable_wolfssl" != "yes"; then
	    PKG_CONFIG_LIBDIR="${enable_wolfssl}/lib/pkgconfig:${enable_wolfssl}/lib64/pkgconfig:${enable_wolfssl}/share/pkgconfig"
	    export PKG_CONFIG_LIBDIR
	elif test "$cross_compiling" = "yes" -a -z "$PKG_CONFIG"; then
	    # Cannot use pkg-config when cross-compiling
	    PKG_CONFIG=false
	fi
	: ${PKG_CONFIG='pkg-config'}
	if $PKG_CONFIG --exists wolfssl >/dev/null 2>&1; then
	    AC_DEFINE(HAVE_OPENSSL)
	    AC_DEFINE(HAVE_WOLFSSL)

	    O_CPPFLAGS="$CPPFLAGS"
	    CPPFLAGS="$CPPFLAGS `$PKG_CONFIG --cflags-only-I wolfssl`"
	    O_LDFLAGS="$LDFLAGS"
	    LDFLAGS="$LDFLAGS `$PKG_CONFIG --libs-only-L wolfssl`"

	    # Check whether --static is needed
	    libssl="`$PKG_CONFIG --libs-only-l wolfssl | sed 's/^ *-l//'`"
	    libssl_extra=`echo $libssl | sed 's/^[[^ ]]* *//'`
	    libssl=`echo $libssl | sed 's/ .*//'`
	    AC_CHECK_LIB([$libssl], [wolfSSL_new], [STATIC=""], [STATIC="--static"], [$libssl_extra])

	    # Use wolfSSL's sha2 functions if possible
	    AC_CHECK_DECL([EVP_MD_CTX_new], [DIGEST=digest_openssl.lo], [], [
		AC_INCLUDES_DEFAULT
		#include <wolfssl/options.h>
		#include <wolfssl/openssl/evp.h>
	    ])
	    CPPFLAGS="$O_CPPFLAGS"
	    LDFLAGS="$O_LDFLAGS"

	    # Use pkg-config to determine wolfSSL libs and cflags
	    for f in `$PKG_CONFIG $STATIC --libs wolfssl`; do
		case "$f" in
		    -L*)
			f="${f#-L}"
			SUDO_APPEND_LIBPATH([LIBTLS], [$f])
			;;
		    *)
			AX_APPEND_FLAG([$f], [LIBTLS])
			;;
		esac
	    done
	    # No separate pkg config for libcrypto
	    LIBCRYPTO="$LIBTLS"
	    LIBCRYPTO_R="$LIBTLS_R"
	    for f in `$PKG_CONFIG --cflags-only-I wolfssl`; do
		AX_APPEND_FLAG([$f], [CPPFLAGS])
		# So we find the openssl compat headers under wolfssl
		AX_APPEND_FLAG([$f/wolfssl], [CPPFLAGS])
	    done
	    if test "$CPPFLAGS" = "$O_CPPFLAGS"; then
		# So we find the openssl compat headers under wolfssl (XXX)
		AX_APPEND_FLAG([-I/usr/include/wolfssl], [CPPFLAGS])
	    fi
	else
	    AC_DEFINE(HAVE_OPENSSL)
	    AC_DEFINE(HAVE_WOLFSSL)

	    # No pkg-config file present, try to do it manually
	    if test "$enable_wolfssl" != "yes"; then
		SUDO_APPEND_LIBPATH(LIBCRYPTO, [${enable_wolfssl}/lib])
		SUDO_APPEND_LIBPATH(LIBTLS, [${enable_wolfssl}/lib])
		AX_APPEND_FLAG([-I${enable_wolfssl}/include], [CPPFLAGS])
		# So we find the openssl compat headers under wolfssl
		AX_APPEND_FLAG([-I${enable_wolfssl}/include/wolfssl], [CPPFLAGS])
	    else
		# So we find the openssl compat headers under wolfssl (XXX)
		AX_APPEND_FLAG([-I/usr/include/wolfssl], [CPPFLAGS])
	    fi
	    LIBTLS="${LIBTLS} -lwolfssl"
	    LIBCRYPTO="${LIBCRYPTO} -lwolfssl"

	    # Use wolfSSL's sha2 functions if possible
	    AC_CHECK_DECL([EVP_MD_CTX_new], [DIGEST=digest_openssl.lo], [], [
		AC_INCLUDES_DEFAULT
		#include <wolfssl/options.h>
		#include <wolfssl/openssl/evp.h>
	    ])
	fi
	dnl
	dnl Check for specific OpenSSL API compatibility macros
	dnl
	AC_CHECK_DECL([X509_STORE_CTX_get0_cert], [AC_DEFINE(HAVE_X509_STORE_CTX_GET0_CERT)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/x509.h>
	])
	AC_CHECK_DECL([ASN1_STRING_get0_data], [AC_DEFINE(HAVE_ASN1_STRING_GET0_DATA)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/asn1.h>
	])
	AC_CHECK_DECL([SSL_CTX_get0_certificate], [AC_DEFINE(HAVE_SSL_CTX_GET0_CERTIFICATE)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/ssl.h>
	])
	AC_CHECK_DECL([SSL_CTX_set0_tmp_dh_pkey], [AC_DEFINE(HAVE_SSL_CTX_SET0_TMP_DH_PKEY)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/ssl.h>
	])
	AC_CHECK_DECL([TLS_method], [AC_DEFINE(HAVE_TLS_METHOD)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/ssl.h>
	])
	AC_CHECK_DECL([SSL_CTX_set_min_proto_version], [AC_DEFINE(HAVE_SSL_CTX_SET_MIN_PROTO_VERSION)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/ssl.h>
	])
	AC_CHECK_DECL([SSL_CTX_set_ciphersuites], [AC_DEFINE(HAVE_SSL_CTX_SET_CIPHERSUITES)], [], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/ssl.h>
	])
	AC_CHECK_DECL([SSL_read_ex], [AC_DEFINE(HAVE_SSL_READ_EX)], [
	    SSL_COMPAT_SRC=lib/ssl_compat
	], [
	    AC_INCLUDES_DEFAULT
	    #include <wolfssl/options.h>
	    #include <wolfssl/openssl/ssl.h>
	])
    fi
    if test -n "$SSL_COMPAT_SRC"; then
	LIBTLS='$(top_builddir)/lib/ssl_compat/libssl_compat.la '"${LIBTLS}"
    fi
])
