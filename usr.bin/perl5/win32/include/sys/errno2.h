#ifndef _INC_SYS_ERRNO2
#define _INC_SYS_ERRNO2

/* Too late to include winsock2.h if winsock.h has already been loaded */
#ifndef _WINSOCKAPI_
#  include <winsock2.h>
#endif

/* Ensure all the Exxx constants required by convert_wsa_error_to_errno() in
 * win32/win32sck.c are defined. Many are defined in <errno.h> already (more so
 * in VC++ 2010 and above and some MinGW/gcc-4.8 and above, which have an extra
 * "POSIX supplement") so, for the sake of compatibility with third-party code
 * linked into XS modules, we must be careful not to redefine them; for the
 * remainder we define our own values, namely the corresponding WSAExxx values.
 *
 * These definitions are also used as a supplement to the use of <errno.h> in
 * the Errno and POSIX modules, both of which may be used to test the value of
 * $!, which may have these values assigned to it (via code in win32/win32sck.c
 * and the $! case in Perl_magic_set()). It also provides numerous otherwise
 * missing values in the (hard-coded) list of Exxx constants exported by POSIX.
 * Finally, three of the non-standard errno.h values (actually all now in the
 * POSIX supplement in VC10+ and some MinGW/gcc-4.8+) are used in the perl core.
 *
 * This list is in the same order as that in convert_wsa_error_to_errno(). A
 * handful of WSAExxx constants used by that function have no corresponding Exxx
 * constant in any errno.h so there is no point in making up values for them;
 * they are just returned unchanged by that function so we do not need to worry
 * about them here.
 */

/* EINTR is a standard errno.h value */
/* EBADF is a standard errno.h value */
/* EACCES is a standard errno.h value */
/* EFAULT is a standard errno.h value */
/* EINVAL is a standard errno.h value */
/* EMFILE is a standard errno.h value */

#ifndef EWOULDBLOCK		/* New in VC10 */
#  define EWOULDBLOCK		WSAEWOULDBLOCK
#endif
#ifndef EINPROGRESS		/* New in VC10 */
#  define EINPROGRESS		WSAEINPROGRESS
#endif
#ifndef EALREADY		/* New in VC10 */
#  define EALREADY		WSAEALREADY
#endif
#ifndef ENOTSOCK		/* New in VC10 and needed in doio.c */
#  define ENOTSOCK		WSAENOTSOCK
#endif
#ifndef EDESTADDRREQ		/* New in VC10 */
#  define EDESTADDRREQ		WSAEDESTADDRREQ
#endif
#ifndef EMSGSIZE		/* New in VC10 */
#  define EMSGSIZE		WSAEMSGSIZE
#endif
#ifndef EPROTOTYPE		/* New in VC10 */
#  define EPROTOTYPE		WSAEPROTOTYPE
#endif
#ifndef ENOPROTOOPT		/* New in VC10 */
#  define ENOPROTOOPT		WSAENOPROTOOPT
#endif
#ifndef EPROTONOSUPPORT		/* New in VC10 */
#  define EPROTONOSUPPORT	WSAEPROTONOSUPPORT
#endif
#ifndef ESOCKTNOSUPPORT		/* Not in errno.h but wanted by POSIX.pm */
#  define ESOCKTNOSUPPORT	WSAESOCKTNOSUPPORT
#endif
#ifndef EOPNOTSUPP		/* New in VC10 */
#  define EOPNOTSUPP		WSAEOPNOTSUPP
#endif
#ifndef EPFNOSUPPORT		/* Not in errno.h but wanted by POSIX.pm */
#  define EPFNOSUPPORT		WSAEPFNOSUPPORT
#endif
#ifndef EAFNOSUPPORT		/* New in VC10 and needed in util.c */
#  define EAFNOSUPPORT		WSAEAFNOSUPPORT
#endif
#ifndef EADDRINUSE		/* New in VC10 */
#  define EADDRINUSE		WSAEADDRINUSE
#endif
#ifndef EADDRNOTAVAIL		/* New in VC10 */
#  define EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#endif
#ifndef ENETDOWN		/* New in VC10 */
#  define ENETDOWN		WSAENETDOWN
#endif
#ifndef ENETUNREACH		/* New in VC10 */
#  define ENETUNREACH		WSAENETUNREACH
#endif
#ifndef ENETRESET		/* New in VC10 */
#  define ENETRESET		WSAENETRESET
#endif
#ifndef ECONNABORTED		/* New in VC10 and needed in util.c */
#  define ECONNABORTED		WSAECONNABORTED
#endif
#ifndef ECONNRESET		/* New in VC10 */
#  define ECONNRESET		WSAECONNRESET
#endif
#ifndef ENOBUFS			/* New in VC10 */
#  define ENOBUFS		WSAENOBUFS
#endif
#ifndef EISCONN			/* New in VC10 */
#  define EISCONN		WSAEISCONN
#endif
#ifndef ENOTCONN		/* New in VC10 */
#  define ENOTCONN		WSAENOTCONN
#endif
#ifndef ESHUTDOWN		/* Not in errno.h but wanted by POSIX.pm */
#  define ESHUTDOWN		WSAESHUTDOWN
#endif
#ifndef ETOOMANYREFS		/* Not in errno.h but wanted by POSIX.pm */
#  define ETOOMANYREFS		WSAETOOMANYREFS
#endif
#ifndef ETIMEDOUT		/* New in VC10 */
#  define ETIMEDOUT		WSAETIMEDOUT
#endif
#ifndef ECONNREFUSED		/* New in VC10 */
#  define ECONNREFUSED		WSAECONNREFUSED
#endif
#ifndef ELOOP			/* New in VC10 */
#  define ELOOP			WSAELOOP
#endif

/* ENAMETOOLONG is a standard errno.h value */

/* EHOSTDOWN is not in errno.h and despite being wanted by POSIX.pm we cannot
 * provide any sane value since there is no WSAEHOSTDOWN */

#ifndef EHOSTUNREACH		/* New in VC10 */
#  define EHOSTUNREACH		WSAEHOSTUNREACH
#endif

/* ENOTEMPTY is a standard errno.h value */

#ifndef EPROCLIM		/* Not in errno.h but wanted by POSIX.pm */
#  define EPROCLIM		WSAEPROCLIM
#endif
#ifndef EUSERS			/* Not in errno.h but wanted by POSIX.pm */
#  define EUSERS		WSAEUSERS
#endif
#ifndef EDQUOT			/* Not in errno.h but wanted by POSIX.pm */
#  define EDQUOT		WSAEDQUOT
#endif
#ifndef ESTALE			/* Not in errno.h but wanted by POSIX.pm */
#  define ESTALE		WSAESTALE
#endif
#ifndef EREMOTE			/* Not in errno.h but wanted by POSIX.pm */
#  define EREMOTE		WSAEREMOTE
#endif

/* EDISCON is not an errno.h value at all */
/* ENOMORE is not an errno.h value at all */

#ifndef ECANCELED		/* New in VC10 */
#  ifdef WSAECANCELLED		/* New in WinSock2 */
#    define ECANCELED		WSAECANCELLED
#  endif
#endif

/* EINVALIDPROCTABLE is not an errno.h value at all */
/* EINVALIDPROVIDER is not an errno.h value at all */
/* EPROVIDERFAILEDINIT is not an errno.h value at all */
/* EREFUSED is not an errno.h value at all */

/* Set a flag indicating whether <errno.h> has the POSIX supplement (the first
 * constant in which is EADDRINUSE). If so then we won't have just defined it as
 * WSAEADDRINUSE above.
 */
#undef ERRNO_HAS_POSIX_SUPPLEMENT
#if EADDRINUSE != WSAEADDRINUSE
#  define ERRNO_HAS_POSIX_SUPPLEMENT
#endif

#endif /* _INC_SYS_ERRNO2 */
