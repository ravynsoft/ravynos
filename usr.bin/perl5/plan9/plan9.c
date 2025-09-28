#include "EXTERN.h"
#include "perl.h"
#include "./plan9/math.h"

#define _PLAN9_SOURCE
#include <u.h>

/** Function fpclassify(double) is required by sv.c, which was refactored in perl-5.24 era and uses other libraries to classify floating points. **/

/* See /sys/src/lib/port/frexp.c */
#define SHIFT 20

int fpclassify(double d) {
        FPdbleword x;

        /* order matters: only isNaN can operate on NaN */
        if ( isNaN(d) )
                return FP_NAN;
        else if ( isInf(d, 0) )
                return FP_INFINITE;
        else if ( d == 0 )
                return FP_ZERO;

        x.x = fabs(d);
        return (x.hi >> SHIFT) ? FP_NORMAL : FP_SUBNORMAL;
}

/* Functions mentioned in /sys/include/ape/sys/socket.h but not implemented */

int recvmsg(int a, struct msghdr *b, int c)
{
    croak("Function \"recvmsg\" not implemented in this version of perl.");
    return (int)NULL;
} 

int sendmsg(int a, struct msghdr *b, int c)
{
    croak("Function \"sendmsg\" not implemented in this version of perl.");
    return (int)NULL;
} 


/* Functions mentioned in /sys/include/ape/sys/netdb.h but not implemented */
struct netent *getnetbyname(const char *a)
{
    croak("Function \"getnetbyname\" not implemented in this version of perl.");
    return (struct netent *)NULL;
}

struct netent *getnetbyaddr(long a, int b)
{
    croak("Function \"getnetbyaddr\" not implemented in this version of perl.");
    return (struct netent *)NULL;
}

struct netent *getnetent()
{
    croak("Function \"getnetent\" not implemented in this version of perl.");
    return (struct netent *)NULL;
}

struct protoent *getprotobyname(const char *a)
{
    croak("Function \"getprotobyname\" not implemented in this version of perl.");
    return (struct protoent *)NULL;
}

struct protoent *getprotobynumber(int a)
{
    croak("Function \"getprotobynumber\" not implemented in this version of perl.");
    return (struct protoent *)NULL;
}

struct protoent *getprotoent()
{
    croak("Function \"getprotoent\" not implemented in this version of perl.");
    return (struct protoent *)NULL;
}

struct servent *getservbyport(int a, const char *b)
{
    croak("Function \"getservbyport\" not implemented in this version of perl.");
    return (struct servent *)NULL;
}

struct servent *getservent()
{
    croak("Function \"getservent\" not implemented in this version of perl.");
    return (struct servent *)NULL;
}

void sethostent(int a)
{
    croak("Function \"sethostent\" not implemented in this version of perl.");
}

void setnetent(int a)
{
    croak("Function \"setnetent\" not implemented in this version of perl.");
}

void setprotoent(int a)
{
    croak("Function \"setprotoent\" not implemented in this version of perl.");
}

void setservent(int a)
{
    croak("Function \"setservent\" not implemented in this version of perl.");
}

void endnetent()
{
    croak("Function \"endnetent\" not implemented in this version of perl.");
}

void endprotoent()
{
    croak("Function \"endprotoent\" not implemented in this version of perl.");
}

void endservent()
{
    croak("Function \"endservent\" not implemented in this version of perl.");
}
