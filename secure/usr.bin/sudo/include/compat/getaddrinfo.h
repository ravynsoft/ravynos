/*
 * Replacement implementation of getaddrinfo.
 *
 * This is an implementation of the getaddrinfo family of functions for
 * systems that lack it, so that code can use getaddrinfo always.  It provides
 * IPv4 support only; for IPv6 support, a system getaddrinfo implementation is
 * required.
 *
 * The canonical version of this file is maintained in the rra-c-util package,
 * which can be found at <http://www.eyrie.org/~eagle/software/rra-c-util/>.
 *
 * Written by Russ Allbery <rra@stanford.edu>
 *
 * The authors hereby relinquish any claim to any copyright that they may have
 * in this work, whether granted under contract or by operation of law or
 * international treaty, and hereby commit to the public, at large, that they
 * shall not, at any time in the future, seek to enforce any copyright in this
 * work against any person or entity, or prevent any person or entity from
 * copying, publishing, distributing or creating derivative works of this
 * work.
 */

#ifndef COMPAT_GETADDRINFO_H
#define COMPAT_GETADDRINFO_H

#include <config.h>

/* Skip this entire file if a system getaddrinfo was detected. */
#ifndef HAVE_GETADDRINFO

/* OpenBSD likes to have sys/types.h included before sys/socket.h. */
#include <sys/types.h>
#include <sys/socket.h>

/* The struct returned by getaddrinfo, from RFC 3493. */
struct addrinfo {
    int ai_flags;               /* AI_PASSIVE, AI_CANONNAME, .. */
    int ai_family;              /* AF_xxx */
    int ai_socktype;            /* SOCK_xxx */
    int ai_protocol;            /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
    socklen_t ai_addrlen;       /* Length of ai_addr */
    char *ai_canonname;         /* Canonical name for nodename */
    struct sockaddr *ai_addr;   /* Binary address */
    struct addrinfo *ai_next;   /* Next structure in linked list */
};

/* Constants for ai_flags from RFC 3493, combined with binary or. */
#define AI_PASSIVE      0x0001
#define AI_CANONNAME    0x0002
#define AI_NUMERICHOST  0x0004
#define AI_NUMERICSERV  0x0008
#define AI_V4MAPPED     0x0010
#define AI_ALL          0x0020
#define AI_ADDRCONFIG   0x0040

/* Error return codes from RFC 3493. */
#define EAI_AGAIN       1       /* Temporary name resolution failure */
#define EAI_BADFLAGS    2       /* Invalid value in ai_flags parameter */
#define EAI_FAIL        3       /* Permanent name resolution failure */
#define EAI_FAMILY      4       /* Address family not recognized */
#define EAI_MEMORY      5       /* Memory allocation failure */
#define EAI_NONAME      6       /* nodename or servname unknown */
#define EAI_SERVICE     7       /* Service not recognized for socket type */
#define EAI_SOCKTYPE    8       /* Socket type not recognized */
#define EAI_SYSTEM      9       /* System error occurred, see errno */
#define EAI_OVERFLOW    10      /* An argument buffer overflowed */

/* Function prototypes. */
sudo_dso_public int sudo_getaddrinfo(const char *nodename, const char *servname,
                const struct addrinfo *hints, struct addrinfo **res);
sudo_dso_public void sudo_freeaddrinfo(struct addrinfo *ai);
sudo_dso_public const char *sudo_gai_strerror(int ecode);

/* Map sudo_* to RFC 3493 names. */
#undef getaddrinfo
#define getaddrinfo(_a, _b, _c, _d) sudo_getaddrinfo((_a), (_b), (_c), (_d))
#undef freeaddrinfo
#define freeaddrinfo(_a) sudo_freeaddrinfo((_a))
#undef gai_strerror
#define gai_strerror(_a) sudo_gai_strerror((_a))

#endif /* !HAVE_GETADDRINFO */
#endif /* COMPAT_GETADDRINFO_H */
