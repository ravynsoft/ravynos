/*
 * Copyright (c) 2005-2006 Apple Computer, Inc. All rights reserved.
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

#include "darwin_shim.h"
#ifndef __linux__

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fnmatch.h>

hrtime_t
gethrtime(void)
{
	uint64_t elapsed;
	static uint64_t start;
	static mach_timebase_info_data_t sTimebaseInfo = { 0, 0 };

	/*
	 * If this is the first time we've run, get the timebase.
	 * We can use denom == 0 to indicate that sTimebaseInfo is
	 * uninitialised because it makes no sense to have a zero
	 * denominator in a fraction.
	 */

	if ( sTimebaseInfo.denom == 0 ) {
		(void)mach_timebase_info(&sTimebaseInfo);
		start = mach_absolute_time();
	}

	elapsed = mach_absolute_time();

	// Convert to nanoseconds.
	// return (elapsed * (uint64_t)sTimebaseInfo.numer)/(uint64_t)sTimebaseInfo.denom;

	// Provided the final result is representable in 64 bits the following maneuver will
	// deliver that result without intermediate overflow.
	if (sTimebaseInfo.denom == sTimebaseInfo.numer)
		return elapsed;
	else if (sTimebaseInfo.denom == 1)
		return elapsed * (uint64_t)sTimebaseInfo.numer;
	else {
		// Decompose elapsed = eta32 * 2^32 + eps32:
		uint64_t eta32 = elapsed >> 32;
		uint64_t eps32 = elapsed & 0x00000000ffffffffLL;

		uint32_t numer = sTimebaseInfo.numer, denom = sTimebaseInfo.denom;

		// Form product of elapsed64 (decomposed) and numer:
		uint64_t mu64 = numer * eta32;
		uint64_t lambda64 = numer * eps32;

		// Divide the constituents by denom:
		uint64_t q32 = mu64/denom;
		uint64_t r32 = mu64 - (q32 * denom); // mu64 % denom

		return (q32 << 32) + ((r32 << 32) + lambda64)/denom;
	}
}

int 
gmatch(const char *s, const char *p)
{
	// OS X's fnmatch return value is inverted relative to Solaris's gmatch
	return fnmatch( p, s, 0 ) == 0;
}

long 
sysinfo(int command, char *buf, long count)
{
	switch (command)
	{
	int mib[2];
	size_t len;
	
	case SI_RELEASE:
		mib[0] = CTL_KERN;
		mib[1] = KERN_OSRELEASE;
		len = count;
		return sysctl(mib, 2, (void *)buf, &len, NULL, 0);

	case SI_SYSNAME:
		mib[0] = CTL_KERN;
		mib[1] = KERN_OSTYPE;
		len = count;
		return sysctl(mib, 2, (void *)buf, &len, NULL, 0);

	default:
		return -1;
	}
	
	/* NOTREACHED */
	return 0;
}

// The following are used only for "assert()"
int
_rw_read_held(struct _rwlock *l)
{
#pragma unused(l)
	return 1;

}

int
_rw_write_held(struct _rwlock *l)
{
#pragma unused(l)
	return 1;

}

int _mutex_held(struct _lwp_mutex *m)
{
#pragma unused(m)
	return 1;
}

/*
 * p_online() is only used to identify valid processorid's and only by testing the
 * return value against -1. On Mac OS X processors are given consecutive id numbers
 * in [0..hw.physicalcpu_max).
 */
int 
p_online(processorid_t processorid, int flag)
{
	static int ncpu = -1;
	
	if (ncpu == -1) {
		size_t len = sizeof(ncpu);
		int mib[2] = { CTL_HW, HW_NCPU };
		
		(void)sysctl(mib, 2, (void *)&ncpu, &len, NULL, 0);
	}

	switch(flag) {
	case P_STATUS:
		if (processorid < ncpu)
			return P_ONLINE;
		/* FALLTHROUGH */
	default:
		errno = EINVAL;
		return -1;
	}
	/* NOTREACHED */
}
#endif

