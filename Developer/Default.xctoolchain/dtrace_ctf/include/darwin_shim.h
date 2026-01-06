/*
 * Copyright (c) 2005-2008 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _DARWIN_SHIM_H
#define _DARWIN_SHIM_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h> /* In lieu of Solaris <sys/synch.h> */

#undef NULL
#define NULL (0) /* quiets many warnings */

typedef uint8_t		uchar_t;
typedef uint16_t	ushort_t;
typedef uint32_t	uint_t;
typedef unsigned long	ulong_t;
typedef uint64_t	u_longlong_t;
typedef int64_t		longlong_t;
typedef int64_t		off64_t;

typedef int64_t hrtime_t;
hrtime_t gethrtime(void);

typedef int processorid_t;

#define pthread_cond_reltimedwait_np pthread_cond_timedwait_relative_np

/*
 * return x rounded up to an align boundary
 * eg, P2ROUNDUP(0x1234, 0x100) == 0x1300 (0x13*align)
 * eg, P2ROUNDUP(0x5600, 0x100) == 0x5600 (0x56*align)
 */
#define P2ROUNDUP(x, align)             (-(-(x) & -(align)))

#define SEC			1
#define MILLISEC	1000
#define MICROSEC	1000000
#define NANOSEC		1000000000

#define    P_ONLINE        0x0002  /* processor is online */
#define    P_STATUS        0x0003  /* value passed to p_online to request status */

// Solaris sysconf(3) selectors. Absent from Darwin unistd.h. dt_sysconf() can deal.
#define _SC_CPUID_MAX -1
#define _SC_NPROCESSORS_MAX -2

// Solaris sysinfo selectors. Mapped to Darwin sysctl's in sysinfo().
#define SI_SYSNAME 2
#define SI_RELEASE 3

extern int gmatch(const char *, const char *);
extern int p_online(processorid_t, int);
extern long sysinfo(int, char *, long);

// The following are used only for "assert()"
struct _rwlock;
struct _lwp_mutex;
extern int _rw_read_held(struct _rwlock *);
extern int _rw_write_held(struct _rwlock *);
extern int _mutex_held(struct _lwp_mutex *);

#endif /* _DARWIN_SHIM_H */
