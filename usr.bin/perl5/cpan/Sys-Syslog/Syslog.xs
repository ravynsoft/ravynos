/*
 * Syslog.xs
 * 
 * XS wrapper for the syslog(3) facility.
 * 
 */

#if defined(_WIN32)
#  include <windows.h>
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef USE_PPPORT_H
#  include "ppport.h"
#endif

#ifndef HAVE_SYSLOG
#define HAVE_SYSLOG 1
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#  undef HAVE_SYSLOG
#  include "fallback/syslog.h"
#else
#  if defined(I_SYSLOG) || PATCHLEVEL < 6
#    include <syslog.h>
#  else
#    undef HAVE_SYSLOG
#    include "fallback/syslog.h"
#  endif
#endif

static SV *ident_svptr;


#ifndef LOG_FAC
#define LOG_FACMASK     0x03f8
#define LOG_FAC(p)      (((p) & LOG_FACMASK) >> 3)
#endif

#ifndef LOG_PRIMASK
#define LOG_PRIMASK     0x07
#endif

#ifndef	LOG_PRI
#define	LOG_PRI(p)	((p) & LOG_PRIMASK)
#endif

#ifndef	LOG_MAKEPRI
#define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))
#endif

#ifndef LOG_MASK
#define	LOG_MASK(pri)	(1 << (pri))
#endif

#ifndef LOG_UPTO
#define	LOG_UPTO(pri)	((1 << ((pri)+1)) - 1)
#endif

#include "const-c.inc"


MODULE = Sys::Syslog		PACKAGE = Sys::Syslog		

INCLUDE: const-xs.inc

int
LOG_FAC(p)
    INPUT:
	int		p

int
LOG_PRI(p)
    INPUT:
	int		p

int
LOG_MAKEPRI(fac,pri)
    INPUT:
	int		fac
	int		pri

int
LOG_MASK(pri)
    INPUT:
	int		pri

int
LOG_UPTO(pri)
    INPUT:
	int		pri

#ifdef HAVE_SYSLOG

void
openlog_xs(ident, option, facility)
    INPUT:
        SV*   ident
        int   option
        int   facility
    PREINIT:
        STRLEN len;
        char*  ident_pv;
    CODE:
        ident_svptr = newSVsv(ident);
        ident_pv    = SvPV(ident_svptr, len);
        openlog(ident_pv, option, facility);

void
syslog_xs(priority, message)
    INPUT:
        int   priority
        const char * message
    CODE:
        syslog(priority, "%s", message);

int
setlogmask_xs(mask)
    INPUT:
        int mask
    CODE:
        RETVAL = setlogmask(mask);
    OUTPUT:
        RETVAL

void
closelog_xs()
    PREINIT:
        U32 refcnt;
    CODE:
        if (!ident_svptr)
            return;
        closelog();
        refcnt = SvREFCNT(ident_svptr);
        if (refcnt) {
            SvREFCNT_dec(ident_svptr);
            if (refcnt == 1)
                ident_svptr = NULL;
        }

#else  /* HAVE_SYSLOG */

void
openlog_xs(ident, option, facility)
    INPUT:
        SV*   ident
        int   option
        int   facility
    CODE:

void
syslog_xs(priority, message)
    INPUT:
        int   priority
        const char * message
    CODE:

int
setlogmask_xs(mask)
    INPUT:
        int mask
    CODE:

void
closelog_xs()
    CODE:

#endif /* HAVE_SYSLOG */
