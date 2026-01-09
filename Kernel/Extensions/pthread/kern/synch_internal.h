/*
 * Copyright (c) 2000-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __SYNCH_INTERNAL_H__
#define __SYNCH_INTERNAL_H__

// kwe_state
enum {
	KWE_THREAD_INWAIT = 1,
	KWE_THREAD_PREPOST,
	KWE_THREAD_BROADCAST,
};

#define _PTHREAD_MTX_OPT_PSHARED 0x010
#define _PTHREAD_MTX_OPT_NOTIFY 0x1000	/* notify to drop mutex handling in cvwait */
#define _PTHREAD_MTX_OPT_MUTEX	0x2000 /* this is a mutex type  */


#define PTHRW_COUNT_SHIFT	8
#define PTHRW_INC		(1 << PTHRW_COUNT_SHIFT)
#define PTHRW_BIT_MASK		((1 << PTHRW_COUNT_SHIFT) - 1)
#define PTHRW_COUNT_MASK 	((uint32_t)~PTHRW_BIT_MASK)
#define PTHRW_MAX_READERS 	PTHRW_COUNT_MASK

// L word
#define PTH_RWL_KBIT		0x01 	// cannot acquire in user mode
#define PTH_RWL_EBIT		0x02	// exclusive lock in progress
#define PTH_RWL_WBIT		0x04	// write waiters pending in kernel
#define PTH_RWL_PBIT		0x04	// prepost (cv) pending in kernel

#define PTH_RWL_MTX_WAIT	0x20 	// in cvar in mutex wait
#define PTH_RWL_UBIT		0x40	// lock is unlocked (no readers or writers)
#define PTH_RWL_MBIT		0x40	// overlapping grants from kernel (only in updateval)
#define PTH_RWL_IBIT		0x80	// lock reset, held until first successful unlock

#define PTHRW_RWL_INIT		PTH_RWL_IBIT	// reset on the lock bits (U)
#define PTHRW_RWLOCK_INIT	(PTH_RWL_IBIT | PTH_RWL_UBIT)   // reset on the lock bits (U)

// S word
#define PTH_RWS_SBIT		0x01	// kernel transition seq not set yet
#define PTH_RWS_IBIT		0x02	// Sequence is not set on return from kernel

#define PTH_RWS_CV_CBIT		PTH_RWS_SBIT	// kernel has cleared all info w.r.s.t CV
#define PTH_RWS_CV_PBIT		PTH_RWS_IBIT	// kernel has prepost/fake structs only,no waiters
#define PTH_RWS_CV_BITSALL	(PTH_RWS_CV_CBIT | PTH_RWS_CV_PBIT)
#define PTH_RWS_CV_MBIT		PTH_RWL_MBIT    // to indicate prepost return from kernel
#define PTH_RWS_CV_RESET_PBIT	((uint32_t)~PTH_RWS_CV_PBIT)

#define PTH_RWS_WSVBIT		0x04	// save W bit

#define PTHRW_RWS_SAVEMASK	(PTH_RWS_WSVBIT)	// save bits mask

#define PTHRW_RWS_INIT		PTH_RWS_SBIT	// reset on the lock bits (U)

// rw_flags
#define PTHRW_KERN_PROCESS_SHARED	0x10
#define PTHRW_KERN_PROCESS_PRIVATE	0x20

#define PTHREAD_MTX_TID_SWITCHING (uint64_t)-1

// L word tests
#define is_rwl_ebit_set(x) (((x) & PTH_RWL_EBIT) != 0)
#define is_rwl_wbit_set(x) (((x) & PTH_RWL_WBIT) != 0)
#define is_rwl_ebit_clear(x) (((x) & PTH_RWL_EBIT) == 0)
#define is_rwl_readoverlap(x) (((x) & PTH_RWL_MBIT) != 0)

// S word tests
#define is_rws_sbit_set(x) (((x) & PTH_RWS_SBIT) != 0)
#define is_rws_unlockinit_set(x) (((x) & PTH_RWS_IBIT) != 0)
#define is_rws_savemask_set(x) (((x) & PTHRW_RWS_SAVEMASK) != 0)
#define is_rws_pbit_set(x) (((x) & PTH_RWS_CV_PBIT) != 0)

// kwe_flags
#define KWE_FLAG_LOCKPREPOST	0x1 // cvwait caused a lock prepost

static inline int
is_seqlower(uint32_t x, uint32_t y)
{
	x &= PTHRW_COUNT_MASK;
	y &= PTHRW_COUNT_MASK;
	if (x < y) {
		return ((y - x) < (PTHRW_MAX_READERS / 2));
	} else {
		return ((x - y) > (PTHRW_MAX_READERS / 2));
	}
}

static inline int
is_seqlower_eq(uint32_t x, uint32_t y)
{
	if ((x & PTHRW_COUNT_MASK) == (y & PTHRW_COUNT_MASK)) {
		return 1;
	} else {
		return is_seqlower(x, y);
	}
}

static inline int
is_seqhigher(uint32_t x, uint32_t y)
{
	x &= PTHRW_COUNT_MASK;
	y &= PTHRW_COUNT_MASK;
	if (x > y) {
		return ((x - y) < (PTHRW_MAX_READERS / 2));
	} else {
		return ((y - x) > (PTHRW_MAX_READERS / 2));
	}
}

static inline int
is_seqhigher_eq(uint32_t x, uint32_t y)
{
	if ((x & PTHRW_COUNT_MASK) == (y & PTHRW_COUNT_MASK)) {
		return 1;
	} else {
		return is_seqhigher(x,y);
	}
}

static inline int
diff_genseq(uint32_t x, uint32_t y)
{
	x &= PTHRW_COUNT_MASK;
	y &= PTHRW_COUNT_MASK;
	if (x == y) {
		return 0;
	} else if (x > y)  {
		return x - y;
	} else {
		return ((PTHRW_MAX_READERS - y) + x + PTHRW_INC);
	}
}

static inline int
find_diff(uint32_t upto, uint32_t lowest)
{
	uint32_t diff;
	
	if (upto == lowest)
		return(0);
#if 0
	diff = diff_genseq(upto, lowest);
#else
        if (is_seqlower(upto, lowest) != 0)
                diff = diff_genseq(lowest, upto);
        else
                diff = diff_genseq(upto, lowest);
#endif
	diff = (diff >> PTHRW_COUNT_SHIFT);
	return(diff);
}

#endif /* __SYNCH_INTERNAL_H__ */
