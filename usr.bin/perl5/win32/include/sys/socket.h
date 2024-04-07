/* sys/socket.h */

/* djl */
/* Provide UNIX compatibility */

#ifndef  _INC_SYS_SOCKET
#define  _INC_SYS_SOCKET

#define WIN32_LEAN_AND_MEAN
#ifdef __GNUC__
#  define Win32_Winsock
#endif
#include <windows.h>

/* Too late to include winsock2.h if winsock.h has already been loaded */
#ifndef _WINSOCKAPI_
#  include <winsock2.h>
#  include <ws2tcpip.h>
#endif

/* Early Platform SDKs have an incorrect definition of EAI_NODATA */
#if (EAI_NODATA == EAI_NONAME)
#  undef EAI_NODATA
#  define EAI_NODATA WSANO_DATA
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "errno2.h"

#ifndef PERL_FD_SETSIZE
#define PERL_FD_SETSIZE		64
#endif

#define PERL_BITS_PER_BYTE	8
#define PERL_NFDBITS            (sizeof(Perl_fd_mask)*PERL_BITS_PER_BYTE)

typedef int			Perl_fd_mask;

typedef struct	Perl_fd_set {
    Perl_fd_mask bits[(PERL_FD_SETSIZE+PERL_NFDBITS-1)/PERL_NFDBITS];
}				Perl_fd_set;

#define PERL_FD_CLR(n,p) \
    ((p)->bits[(n)/PERL_NFDBITS] &= ~((unsigned)1 << ((n)%PERL_NFDBITS)))

#define PERL_FD_SET(n,p) \
    ((p)->bits[(n)/PERL_NFDBITS] |=  ((unsigned)1 << ((n)%PERL_NFDBITS)))

#define PERL_FD_ZERO(p) memset((char *)(p),0,sizeof(*(p)))

#define PERL_FD_ISSET(n,p) \
    ((p)->bits[(n)/PERL_NFDBITS] &   ((unsigned)1 << ((n)%PERL_NFDBITS)))

SOCKET win32_accept (SOCKET s, struct sockaddr *addr, int *addrlen);
int win32_bind (SOCKET s, const struct sockaddr *addr, int namelen);
int win32_closesocket (SOCKET s);
int win32_connect (SOCKET s, const struct sockaddr *name, int namelen);
int win32_ioctlsocket (SOCKET s, long cmd, u_long *argp);
int win32_getpeername (SOCKET s, struct sockaddr *name, int * namelen);
int win32_getsockname (SOCKET s, struct sockaddr *name, int * namelen);
int win32_getsockopt (SOCKET s, int level, int optname, char * optval, int *optlen);
u_long win32_htonl (u_long hostlong);
u_short win32_htons (u_short hostshort);
unsigned long win32_inet_addr (const char * cp);
char * win32_inet_ntoa (struct in_addr in);
int win32_listen (SOCKET s, int backlog);
u_long win32_ntohl (u_long netlong);
u_short win32_ntohs (u_short netshort);
int win32_recv (SOCKET s, char * buf, int len, int flags);
int win32_recvfrom (SOCKET s, char * buf, int len, int flags,
                         struct sockaddr *from, int * fromlen);
int win32_select (int nfds, Perl_fd_set *rfds, Perl_fd_set *wfds, Perl_fd_set *xfds,
                  const struct timeval *timeout);
int win32_send (SOCKET s, const char * buf, int len, int flags);
int win32_sendto (SOCKET s, const char * buf, int len, int flags,
                       const struct sockaddr *to, int tolen);
int win32_setsockopt (SOCKET s, int level, int optname,
                           const char * optval, int optlen);
SOCKET win32_socket (int af, int type, int protocol);
int win32_shutdown (SOCKET s, int how);

/* Database function prototypes */

struct hostent * win32_gethostbyaddr(const char * addr, int len, int type);
struct hostent * win32_gethostbyname(const char * name);
int win32_gethostname (char * name, int namelen);
struct servent * win32_getservbyport(int port, const char * proto);
struct servent * win32_getservbyname(const char * name, const char * proto);
struct protoent * win32_getprotobynumber(int proto);
struct protoent * win32_getprotobyname(const char * name);
struct protoent *win32_getprotoent(void);
struct servent *win32_getservent(void);
void win32_sethostent(int stayopen);
void win32_setnetent(int stayopen);
struct netent * win32_getnetent(void);
struct netent * win32_getnetbyname(char *name);
struct netent * win32_getnetbyaddr(long net, int type);
void win32_setprotoent(int stayopen);
void win32_setservent(int stayopen);
void win32_endhostent(void);
void win32_endnetent(void);
void win32_endprotoent(void);
void win32_endservent(void);

#ifndef WIN32SCK_IS_STDSCK

/* direct to our version */

#define htonl		win32_htonl
#define htons		win32_htons
#define ntohl		win32_ntohl
#define ntohs		win32_ntohs
#define inet_addr	win32_inet_addr
#define inet_ntoa	win32_inet_ntoa

#define socket		win32_socket
#define bind		win32_bind
#define listen		win32_listen
#define accept		win32_accept
#define connect		win32_connect
#define send		win32_send
#define sendto		win32_sendto
#define recv		win32_recv
#define recvfrom	win32_recvfrom
#define shutdown	win32_shutdown
#define closesocket	win32_closesocket
#define ioctlsocket	win32_ioctlsocket
#define setsockopt	win32_setsockopt
#define getsockopt	win32_getsockopt
#define getpeername	win32_getpeername
#define getsockname	win32_getsockname
#define gethostname	win32_gethostname
#define gethostbyname	win32_gethostbyname
#define gethostbyaddr	win32_gethostbyaddr
#define getprotobyname	win32_getprotobyname
#define getprotobynumber win32_getprotobynumber
#define getservbyname	win32_getservbyname
#define getservbyport	win32_getservbyport
#define select		win32_select
#define endhostent	win32_endhostent
#define endnetent	win32_endnetent
#define endprotoent	win32_endprotoent
#define endservent	win32_endservent
#define getnetent	win32_getnetent
#define getnetbyname	win32_getnetbyname
#define getnetbyaddr	win32_getnetbyaddr
#define getprotoent	win32_getprotoent
#define getservent	win32_getservent
#define sethostent	win32_sethostent
#define setnetent	win32_setnetent
#define setprotoent	win32_setprotoent
#define setservent	win32_setservent

#undef fd_set
#undef FD_SET
#undef FD_CLR
#undef FD_ISSET
#undef FD_ZERO
#define fd_set		Perl_fd_set
#define FD_SET(n,p)	PERL_FD_SET(n,p)
#define FD_CLR(n,p)	PERL_FD_CLR(n,p)
#define FD_ISSET(n,p)	PERL_FD_ISSET(n,p)
#define FD_ZERO(p)	PERL_FD_ZERO(p)

#endif	/* WIN32SCK_IS_STDSCK */

#ifdef __cplusplus
}
#endif

#endif	/* _INC_SYS_SOCKET */
