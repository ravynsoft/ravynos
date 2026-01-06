/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1995-2005 Apple Computer, Inc. All Rights Reserved */
/*
 *	pthread_support.c
 */

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/resourcevar.h>
//#include <sys/proc_internal.h>
#include <sys/kauth.h>
#include <sys/systm.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/acct.h>
#include <sys/kernel.h>
#include <sys/wait.h>
#include <sys/signalvar.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/lock.h>
#include <sys/kdebug.h>
//#include <sys/sysproto.h>
//#include <sys/pthread_internal.h>
#include <sys/vm.h>
#include <sys/user.h>

#include <mach/mach_types.h>
#include <mach/vm_prot.h>
#include <mach/semaphore.h>
#include <mach/sync_policy.h>
#include <mach/task.h>
#include <kern/kern_types.h>
#include <kern/task.h>
#include <kern/clock.h>
#include <mach/kern_return.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/thread_call.h>
#include <kern/kalloc.h>
#include <kern/zalloc.h>
#include <kern/sched_prim.h>
#include <kern/processor.h>
#include <kern/block_hint.h>
#include <kern/turnstile.h>
//#include <kern/mach_param.h>
#include <mach/mach_vm.h>
#include <mach/mach_param.h>
#include <mach/thread_policy.h>
#include <mach/message.h>
#include <mach/port.h>
//#include <vm/vm_protos.h>
#include <vm/vm_map.h>
#include <mach/vm_region.h>

#include <libkern/OSAtomic.h>

#include <pexpert/pexpert.h>

#include "kern_internal.h"
#include "synch_internal.h"
#include "kern_trace.h"

typedef struct uthread *uthread_t;

//#define __FAILEDUSERTEST__(s) do { panic(s); } while (0)
#define __FAILEDUSERTEST__(s) do { printf("PSYNCH: pid[%d]: %s\n", proc_pid(current_proc()), s); } while (0)
#define __FAILEDUSERTEST2__(s, x...) do { printf("PSYNCH: pid[%d]: " s "\n", proc_pid(current_proc()), x); } while (0)

lck_mtx_t *pthread_list_mlock;

#define PTH_HASHSIZE 100

static LIST_HEAD(pthhashhead, ksyn_wait_queue) *pth_glob_hashtbl;
static unsigned long pthhash;

static LIST_HEAD(, ksyn_wait_queue) pth_free_list;

static zone_t kwq_zone; /* zone for allocation of ksyn_queue */
static zone_t kwe_zone;	/* zone for allocation of ksyn_waitq_element */

#define SEQFIT 0
#define FIRSTFIT 1

struct ksyn_queue {
	TAILQ_HEAD(ksynq_kwelist_head, ksyn_waitq_element) ksynq_kwelist;
	uint32_t	ksynq_count;		/* number of entries in queue */
	uint32_t	ksynq_firstnum;		/* lowest seq in queue */
	uint32_t	ksynq_lastnum;		/* highest seq in queue */
};
typedef struct ksyn_queue *ksyn_queue_t;

typedef enum {
	KSYN_QUEUE_READ = 0,
	KSYN_QUEUE_WRITE,
	KSYN_QUEUE_MAX,
} kwq_queue_type_t;

typedef enum {
	KWQ_INTR_NONE = 0,
	KWQ_INTR_READ = 0x1,
	KWQ_INTR_WRITE = 0x2,
} kwq_intr_type_t;

struct ksyn_wait_queue {
	LIST_ENTRY(ksyn_wait_queue) kw_hash;
	LIST_ENTRY(ksyn_wait_queue) kw_list;
	user_addr_t kw_addr;
	thread_t kw_owner;		/* current owner or THREAD_NULL, has a +1 */
	uint64_t kw_object;		/* object backing in shared mode */
	uint64_t kw_offset;		/* offset inside the object in shared mode */
	int	kw_pflags;		/* flags under listlock protection */
	struct timeval kw_ts;		/* timeval need for upkeep before free */
	int	kw_iocount;		/* inuse reference */
	int 	kw_dropcount;		/* current users unlocking... */
	
	int	kw_type;		/* queue type like mutex, cvar, etc */
	uint32_t kw_inqueue;		/* num of waiters held */
	uint32_t kw_fakecount;		/* number of error/prepost fakes */
	uint32_t kw_highseq;		/* highest seq in the queue */
	uint32_t kw_lowseq;		/* lowest seq in the queue */
	uint32_t kw_lword;		/* L value from userland */
	uint32_t kw_uword;		/* U world value from userland */
	uint32_t kw_sword;		/* S word value from userland */
	uint32_t kw_lastunlockseq;	/* the last seq that unlocked */
	/* for CV to be used as the seq kernel has seen so far */
#define kw_cvkernelseq kw_lastunlockseq
	uint32_t kw_lastseqword;		/* the last seq that unlocked */
	/* for mutex and cvar we need to track I bit values */
	uint32_t kw_nextseqword;	/* the last seq that unlocked; with num of waiters */
	struct {
		uint32_t count; /* prepost count */
		uint32_t lseq; /* prepost target seq */
		uint32_t sseq; /* prepost target sword, in cvar used for mutexowned */
	} kw_prepost;
	struct {
		kwq_intr_type_t type; /* type of failed wakueps */
		uint32_t count; /* prepost of missed wakeup due to intrs */
		uint32_t seq; /* prepost of missed wakeup limit seq */
		uint32_t returnbits; /* return bits value for missed wakeup threads */
	} kw_intr;
	
	int 	kw_kflags;
	int		kw_qos_override;	/* QoS of max waiter during contention period */
	struct turnstile *kw_turnstile;
	struct ksyn_queue kw_ksynqueues[KSYN_QUEUE_MAX];	/* queues to hold threads */
	lck_spin_t kw_lock;		/* spinlock protecting this structure */
};
typedef struct ksyn_wait_queue * ksyn_wait_queue_t;

#define TID_ZERO (uint64_t)0

/* bits needed in handling the rwlock unlock */
#define PTH_RW_TYPE_READ	0x01
#define PTH_RW_TYPE_WRITE	0x04
#define PTH_RW_TYPE_MASK	0xff
#define PTH_RW_TYPE_SHIFT	8

#define PTH_RWSHFT_TYPE_READ	0x0100
#define PTH_RWSHFT_TYPE_WRITE	0x0400
#define PTH_RWSHFT_TYPE_MASK	0xff00

/*
 * Mutex pshared attributes
 */
#define PTHREAD_PROCESS_SHARED		_PTHREAD_MTX_OPT_PSHARED
#define PTHREAD_PROCESS_PRIVATE		0x20
#define PTHREAD_PSHARED_FLAGS_MASK	0x30

/*
 * Mutex policy attributes
 */
#define _PTHREAD_MTX_OPT_POLICY_FAIRSHARE	0x040	/* 1 */
#define _PTHREAD_MTX_OPT_POLICY_FIRSTFIT	0x080	/* 2 */
#define _PTHREAD_MTX_OPT_POLICY_MASK		0x1c0

/* pflags */
#define KSYN_WQ_INHASH	2
#define KSYN_WQ_SHARED	4
#define KSYN_WQ_WAITING 8	/* threads waiting for this wq to be available */
#define KSYN_WQ_FLIST 	0X10	/* in free list to be freed after a short delay */

/* kflags */
#define KSYN_KWF_INITCLEARED	0x1	/* the init status found and preposts cleared */
#define KSYN_KWF_ZEROEDOUT	0x2	/* the lword, etc are inited to 0 */
#define KSYN_KWF_QOS_APPLIED	0x4	/* QoS override applied to owner */
#define KSYN_KWF_OVERLAP_GUARD	0x8	/* overlap guard */

#define KSYN_CLEANUP_DEADLINE 10
static int psynch_cleanupset;
thread_call_t psynch_thcall;

#define KSYN_WQTYPE_INWAIT	0x1000
#define KSYN_WQTYPE_INDROP	0x2000
#define KSYN_WQTYPE_MTX		0x01
#define KSYN_WQTYPE_CVAR	0x02
#define KSYN_WQTYPE_RWLOCK	0x04
#define KSYN_WQTYPE_SEMA	0x08
#define KSYN_WQTYPE_MASK	0xff

#define KSYN_WQTYPE_MUTEXDROP	(KSYN_WQTYPE_INDROP | KSYN_WQTYPE_MTX)

static inline int
_kwq_type(ksyn_wait_queue_t kwq)
{
	return (kwq->kw_type & KSYN_WQTYPE_MASK);
}

static inline bool
_kwq_use_turnstile(ksyn_wait_queue_t kwq)
{
	// <rdar://problem/15926625> If we had writer-owner information from the
	// rwlock then we could use the turnstile to push on it. For now, only
	// plain mutexes use it.
	return (_kwq_type(kwq) == KSYN_WQTYPE_MTX);
}

#define KW_UNLOCK_PREPOST 		0x01
#define KW_UNLOCK_PREPOST_READLOCK 	0x08
#define KW_UNLOCK_PREPOST_WRLOCK 	0x20

static int ksyn_wq_hash_lookup(user_addr_t uaddr, proc_t p, int flags, ksyn_wait_queue_t *kwq, struct pthhashhead **hashptr, uint64_t object, uint64_t offset);
static int ksyn_wqfind(user_addr_t mutex, uint32_t mgen, uint32_t ugen, uint32_t rw_wc, int flags, int wqtype , ksyn_wait_queue_t *wq);
static void ksyn_wqrelease(ksyn_wait_queue_t mkwq, int qfreenow, int wqtype);
static int ksyn_findobj(user_addr_t uaddr, uint64_t *objectp, uint64_t *offsetp);

static int _wait_result_to_errno(wait_result_t result);

static int ksyn_wait(ksyn_wait_queue_t, kwq_queue_type_t, uint32_t, int, uint64_t, uint16_t, thread_continue_t, block_hint_t);
static kern_return_t ksyn_signal(ksyn_wait_queue_t, kwq_queue_type_t, ksyn_waitq_element_t, uint32_t);
static void ksyn_freeallkwe(ksyn_queue_t kq);

static kern_return_t ksyn_mtxsignal(ksyn_wait_queue_t, ksyn_waitq_element_t kwe, uint32_t, thread_t *);

static int kwq_handle_unlock(ksyn_wait_queue_t, uint32_t mgen, uint32_t rw_wc, uint32_t *updatep, int flags, int *blockp, uint32_t premgen);

static void ksyn_queue_init(ksyn_queue_t kq);
static int ksyn_queue_insert(ksyn_wait_queue_t kwq, int kqi, ksyn_waitq_element_t kwe, uint32_t mgen, int firstfit);
static void ksyn_queue_remove_item(ksyn_wait_queue_t kwq, ksyn_queue_t kq, ksyn_waitq_element_t kwe);
static void ksyn_queue_free_items(ksyn_wait_queue_t kwq, int kqi, uint32_t upto, int all);

static void update_low_high(ksyn_wait_queue_t kwq, uint32_t lockseq);
static uint32_t find_nextlowseq(ksyn_wait_queue_t kwq);
static uint32_t find_nexthighseq(ksyn_wait_queue_t kwq);
static int find_seq_till(ksyn_wait_queue_t kwq, uint32_t upto, uint32_t nwaiters, uint32_t *countp);

static uint32_t ksyn_queue_count_tolowest(ksyn_queue_t kq, uint32_t upto);

static ksyn_waitq_element_t ksyn_queue_find_cvpreposeq(ksyn_queue_t kq, uint32_t cgen);
static void ksyn_handle_cvbroad(ksyn_wait_queue_t ckwq, uint32_t upto, uint32_t *updatep);
static void ksyn_cvupdate_fixup(ksyn_wait_queue_t ckwq, uint32_t *updatep);
static ksyn_waitq_element_t ksyn_queue_find_signalseq(ksyn_wait_queue_t kwq, ksyn_queue_t kq, uint32_t toseq, uint32_t lockseq);

static void __dead2 psynch_cvcontinue(void *, wait_result_t);
static void __dead2 psynch_mtxcontinue(void *, wait_result_t);
static void __dead2 psynch_rw_rdcontinue(void *, wait_result_t);
static void __dead2 psynch_rw_wrcontinue(void *, wait_result_t);

static int ksyn_wakeupreaders(ksyn_wait_queue_t kwq, uint32_t limitread, int allreaders, uint32_t updatebits, int *wokenp);
static int kwq_find_rw_lowest(ksyn_wait_queue_t kwq, int flags, uint32_t premgen, int *type, uint32_t lowest[]);
static ksyn_waitq_element_t ksyn_queue_find_seq(ksyn_wait_queue_t kwq, ksyn_queue_t kq, uint32_t seq);

static void
UPDATE_CVKWQ(ksyn_wait_queue_t kwq, uint32_t mgen, uint32_t ugen, uint32_t rw_wc)
{
	int sinit = ((rw_wc & PTH_RWS_CV_CBIT) != 0);
	
	// assert((kwq->kw_type & KSYN_WQTYPE_MASK) == KSYN_WQTYPE_CVAR);
	
	if ((kwq->kw_kflags & KSYN_KWF_ZEROEDOUT) != 0) {
		/* the values of L,U and S are cleared out due to L==S in previous transition */
		kwq->kw_lword = mgen;
		kwq->kw_uword = ugen;
		kwq->kw_sword = rw_wc;
		kwq->kw_kflags &= ~KSYN_KWF_ZEROEDOUT;
	} else {
		if (is_seqhigher(mgen, kwq->kw_lword)) {
			kwq->kw_lword = mgen;
		}
		if (is_seqhigher(ugen, kwq->kw_uword)) {
			kwq->kw_uword = ugen;
		}
		if (sinit && is_seqhigher(rw_wc, kwq->kw_sword)) {
			kwq->kw_sword = rw_wc;
		}
	}
	if (sinit && is_seqlower(kwq->kw_cvkernelseq, rw_wc)) {
		kwq->kw_cvkernelseq = (rw_wc & PTHRW_COUNT_MASK);
	}
}

static inline void
_kwq_clear_preposted_wakeup(ksyn_wait_queue_t kwq)
{
	kwq->kw_prepost.lseq = 0;
	kwq->kw_prepost.sseq = PTHRW_RWS_INIT;
	kwq->kw_prepost.count = 0;
}

static inline void
_kwq_mark_preposted_wakeup(ksyn_wait_queue_t kwq, uint32_t count,
		uint32_t lseq, uint32_t sseq)
{
	kwq->kw_prepost.count = count;
	kwq->kw_prepost.lseq = lseq;
	kwq->kw_prepost.sseq = sseq;
}

static inline void
_kwq_clear_interrupted_wakeup(ksyn_wait_queue_t kwq)
{
	kwq->kw_intr.type = KWQ_INTR_NONE;
	kwq->kw_intr.count = 0;
	kwq->kw_intr.seq = 0;
	kwq->kw_intr.returnbits = 0;
}

static inline void
_kwq_mark_interruped_wakeup(ksyn_wait_queue_t kwq, kwq_intr_type_t type,
		uint32_t count, uint32_t lseq, uint32_t returnbits)
{
	kwq->kw_intr.count = count;
	kwq->kw_intr.seq = lseq;
	kwq->kw_intr.returnbits = returnbits;
	kwq->kw_intr.type = type;
}

static void
_kwq_destroy(ksyn_wait_queue_t kwq)
{
	if (kwq->kw_owner) {
		thread_deallocate(kwq->kw_owner);
	}
	lck_spin_destroy(&kwq->kw_lock, pthread_lck_grp);
	zfree(kwq_zone, kwq);
}

#define KWQ_SET_OWNER_TRANSFER_REF  0x1

static inline thread_t
_kwq_set_owner(ksyn_wait_queue_t kwq, thread_t new_owner, int flags)
{
	thread_t old_owner = kwq->kw_owner;
	if (old_owner == new_owner) {
		if (flags & KWQ_SET_OWNER_TRANSFER_REF) return new_owner;
		return THREAD_NULL;
	}
	if ((flags & KWQ_SET_OWNER_TRANSFER_REF) == 0) {
		thread_reference(new_owner);
	}
	kwq->kw_owner = new_owner;
	return old_owner;
}

static inline thread_t
_kwq_clear_owner(ksyn_wait_queue_t kwq)
{
	return _kwq_set_owner(kwq, THREAD_NULL, KWQ_SET_OWNER_TRANSFER_REF);
}

static inline void
_kwq_cleanup_old_owner(thread_t *thread)
{
	if (*thread) {
		thread_deallocate(*thread);
		*thread = THREAD_NULL;
	}
}

static void
CLEAR_REINIT_BITS(ksyn_wait_queue_t kwq)
{
	if ((kwq->kw_type & KSYN_WQTYPE_MASK) == KSYN_WQTYPE_CVAR) {
		if (kwq->kw_inqueue != 0 && kwq->kw_inqueue != kwq->kw_fakecount) {
			panic("CV:entries in queue durinmg reinit %d:%d\n",kwq->kw_inqueue, kwq->kw_fakecount);
		}
	};
	if ((kwq->kw_type & KSYN_WQTYPE_MASK) == KSYN_WQTYPE_RWLOCK) {
		kwq->kw_nextseqword = PTHRW_RWS_INIT;
		kwq->kw_kflags &= ~KSYN_KWF_OVERLAP_GUARD;
	};
	_kwq_clear_preposted_wakeup(kwq);
	kwq->kw_lastunlockseq = PTHRW_RWL_INIT;
	kwq->kw_lastseqword = PTHRW_RWS_INIT;
	_kwq_clear_interrupted_wakeup(kwq);
	kwq->kw_lword = 0;
	kwq->kw_uword = 0;
	kwq->kw_sword = PTHRW_RWS_INIT;
}

static bool
_kwq_handle_preposted_wakeup(ksyn_wait_queue_t kwq, uint32_t type,
		uint32_t lseq, uint32_t *retval)
{
	if (kwq->kw_prepost.count == 0 ||
			!is_seqlower_eq(lseq, kwq->kw_prepost.lseq)) {
		return false;
	}

	kwq->kw_prepost.count--;
	if (kwq->kw_prepost.count > 0) {
		return false;
	}

	int error, should_block = 0;
	uint32_t updatebits = 0;
	uint32_t pp_lseq = kwq->kw_prepost.lseq;
	uint32_t pp_sseq = kwq->kw_prepost.sseq;
	_kwq_clear_preposted_wakeup(kwq);

	kwq->kw_kflags &= ~KSYN_KWF_INITCLEARED;

	error = kwq_handle_unlock(kwq, pp_lseq, pp_sseq, &updatebits,
			(type | KW_UNLOCK_PREPOST), &should_block, lseq);
	if (error) {
		panic("_kwq_handle_preposted_wakeup: kwq_handle_unlock failed %d",
				error);
	}

	if (should_block) {
		return false;
	}
	*retval = updatebits;
	return true;
}

static bool
_kwq_handle_overlap(ksyn_wait_queue_t kwq, uint32_t type, uint32_t lgenval, 
		uint32_t rw_wc, uint32_t *retval)
{
	int res = 0;

	// overlaps only occur on read lockers
	if (type != PTH_RW_TYPE_READ) {
		return false;
	}

	// check for overlap and no pending W bit (indicates writers)
	if ((kwq->kw_kflags & KSYN_KWF_OVERLAP_GUARD) &&
			!is_rws_savemask_set(rw_wc) && !is_rwl_wbit_set(lgenval)) {
		/* overlap is set, so no need to check for valid state for overlap */

		if (is_seqlower_eq(rw_wc, kwq->kw_nextseqword) || is_seqhigher_eq(kwq->kw_lastseqword, rw_wc)) {
			/* increase the next expected seq by one */
			kwq->kw_nextseqword += PTHRW_INC;
			/* set count by one & bits from the nextseq and add M bit */
			*retval = PTHRW_INC | ((kwq->kw_nextseqword & PTHRW_BIT_MASK) | PTH_RWL_MBIT);
			res = 1;
		}
	}
	return res;
}

static inline bool
_kwq_is_used(ksyn_wait_queue_t kwq)
{
	return (kwq->kw_inqueue != 0 || kwq->kw_prepost.count != 0 ||
			kwq->kw_intr.count != 0);
}

/*
 * consumes a pending interrupted waiter, returns true if the current
 * thread should return back to userspace because it was previously
 * interrupted.
 */
static inline bool
_kwq_handle_interrupted_wakeup(ksyn_wait_queue_t kwq, kwq_intr_type_t type,
		uint32_t lseq, uint32_t *retval)
{
	if (kwq->kw_intr.count != 0 && kwq->kw_intr.type == type &&
			(!kwq->kw_intr.seq || is_seqlower_eq(lseq, kwq->kw_intr.seq))) {
		kwq->kw_intr.count--;
		*retval = kwq->kw_intr.returnbits;
		if (kwq->kw_intr.returnbits == 0) {
			_kwq_clear_interrupted_wakeup(kwq);
		}
		return true;
	}
	return false;
}

static void
pthread_list_lock(void)
{
	lck_mtx_lock_spin(pthread_list_mlock);
}

static void
pthread_list_unlock(void)
{
	lck_mtx_unlock(pthread_list_mlock);
}

static void
ksyn_wqlock(ksyn_wait_queue_t kwq)
{
	lck_spin_lock(&kwq->kw_lock);
}

static void
ksyn_wqunlock(ksyn_wait_queue_t kwq)
{
	lck_spin_unlock(&kwq->kw_lock);
}

/* routine to drop the mutex unlocks , used both for mutexunlock system call and drop during cond wait */
static uint32_t
_psynch_mutexdrop_internal(ksyn_wait_queue_t kwq, uint32_t mgen, uint32_t ugen,
		int flags)
{
	kern_return_t ret;
	uint32_t returnbits = 0;
	uint32_t updatebits = 0;
	int firstfit = (flags & _PTHREAD_MTX_OPT_POLICY_MASK) ==
			_PTHREAD_MTX_OPT_POLICY_FIRSTFIT;
	uint32_t nextgen = (ugen + PTHRW_INC);
	thread_t old_owner = THREAD_NULL;

	ksyn_wqlock(kwq);
	kwq->kw_lastunlockseq = (ugen & PTHRW_COUNT_MASK);

redrive:
	updatebits = (kwq->kw_highseq & PTHRW_COUNT_MASK) |
			(PTH_RWL_EBIT | PTH_RWL_KBIT);

	if (firstfit) {
		if (kwq->kw_inqueue == 0) {
			uint32_t count = kwq->kw_prepost.count + 1;
			// Increment the number of preposters we have waiting
			_kwq_mark_preposted_wakeup(kwq, count, mgen & PTHRW_COUNT_MASK, 0);
			// We don't know the current owner as we've determined this mutex
			// drop should have a preposted locker inbound into the kernel but
			// we have no way of knowing who it is. When it arrives, the lock
			// path will update the turnstile owner and return it to userspace.
			old_owner = _kwq_clear_owner(kwq);
			pthread_kern->psynch_wait_update_owner(kwq, THREAD_NULL,
					&kwq->kw_turnstile);
			PTHREAD_TRACE(psynch_mutex_kwqprepost, kwq->kw_addr,
					kwq->kw_prepost.lseq, count, 0);
		} else {
			// signal first waiter
			ret = ksyn_mtxsignal(kwq, NULL, updatebits, &old_owner);
			if (ret == KERN_NOT_WAITING) {
				// <rdar://problem/39093536> ksyn_mtxsignal attempts to signal
				// the thread but it sets up the turnstile inheritor first.
				// That means we can't redrive the mutex in a loop without
				// dropping the wq lock and cleaning up the turnstile state.
				ksyn_wqunlock(kwq);
				pthread_kern->psynch_wait_cleanup();
				_kwq_cleanup_old_owner(&old_owner);
				ksyn_wqlock(kwq);
				goto redrive;
			}
		}
	} else {	
		bool prepost = false;
		if (kwq->kw_inqueue == 0) {
			// No waiters in the queue.
			prepost = true;
		} else {
			uint32_t low_writer = (kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_firstnum & PTHRW_COUNT_MASK);
			if (low_writer == nextgen) {
				/* next seq to be granted found */
				/* since the grant could be cv, make sure mutex wait is set incase the thread interrupted out */
				ret = ksyn_mtxsignal(kwq, NULL,
						updatebits | PTH_RWL_MTX_WAIT, &old_owner);
				if (ret == KERN_NOT_WAITING) {
					/* interrupt post */
					_kwq_mark_interruped_wakeup(kwq, KWQ_INTR_WRITE, 1,
							nextgen, updatebits);
				}
			} else if (is_seqhigher(low_writer, nextgen)) {
				prepost = true;
			} else {
				//__FAILEDUSERTEST__("psynch_mutexdrop_internal: FS mutex unlock sequence higher than the lowest one is queue\n");
				ksyn_waitq_element_t kwe;
				kwe = ksyn_queue_find_seq(kwq,
						&kwq->kw_ksynqueues[KSYN_QUEUE_WRITE], nextgen);
				if (kwe != NULL) {
					/* next seq to be granted found */
					/* since the grant could be cv, make sure mutex wait is set incase the thread interrupted out */
					ret = ksyn_mtxsignal(kwq, kwe,
							updatebits | PTH_RWL_MTX_WAIT, &old_owner);
					if (ret == KERN_NOT_WAITING) {
						goto redrive;
					}
				} else {
					prepost = true;
				}
			}
		}
		if (prepost) {
			if (kwq->kw_prepost.count != 0) {
				__FAILEDUSERTEST__("_psynch_mutexdrop_internal: multiple preposts\n");
			} else {
				_kwq_mark_preposted_wakeup(kwq, 1, nextgen & PTHRW_COUNT_MASK,
						0);
			}
			old_owner = _kwq_clear_owner(kwq);
			pthread_kern->psynch_wait_update_owner(kwq, THREAD_NULL,
					&kwq->kw_turnstile);
		}
	}

	ksyn_wqunlock(kwq);
	pthread_kern->psynch_wait_cleanup();
	_kwq_cleanup_old_owner(&old_owner);
	ksyn_wqrelease(kwq, 1, KSYN_WQTYPE_MUTEXDROP);
	return returnbits;
}

static int
_ksyn_check_init(ksyn_wait_queue_t kwq, uint32_t lgenval)
{
	int res = (lgenval & PTHRW_RWL_INIT) != 0;
	if (res) {
		if ((kwq->kw_kflags & KSYN_KWF_INITCLEARED) == 0) {
			/* first to notice the reset of the lock, clear preposts */
			CLEAR_REINIT_BITS(kwq);
			kwq->kw_kflags |= KSYN_KWF_INITCLEARED;
		}
	}
	return res;
}

/*
 * psynch_mutexwait: This system call is used for contended psynch mutexes to
 * block.
 */
int
_psynch_mutexwait(__unused proc_t p, user_addr_t mutex, uint32_t mgen,
		uint32_t ugen, uint64_t tid, uint32_t flags, uint32_t *retval)
{
	ksyn_wait_queue_t kwq;
	int error = 0;
	int firstfit = (flags & _PTHREAD_MTX_OPT_POLICY_MASK)
			== _PTHREAD_MTX_OPT_POLICY_FIRSTFIT;
	int ins_flags = SEQFIT;
	uint32_t lseq = (mgen & PTHRW_COUNT_MASK);
	uint32_t updatebits = 0;
	thread_t tid_th = THREAD_NULL, old_owner = THREAD_NULL;

	if (firstfit) {
		/* first fit */
		ins_flags = FIRSTFIT;
	}

	error = ksyn_wqfind(mutex, mgen, ugen, 0, flags,
			(KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_MTX), &kwq);
	if (error != 0) {
		return error;
	}

again:
	ksyn_wqlock(kwq);

	if (_kwq_handle_interrupted_wakeup(kwq, KWQ_INTR_WRITE, lseq, retval)) {
		old_owner = _kwq_set_owner(kwq, current_thread(), 0);
		pthread_kern->psynch_wait_update_owner(kwq, kwq->kw_owner,
				&kwq->kw_turnstile);
		ksyn_wqunlock(kwq);
		goto out;
	}

	if (kwq->kw_prepost.count && (firstfit || (lseq == kwq->kw_prepost.lseq))) {
		/* got preposted lock */
		kwq->kw_prepost.count--;

		if (!firstfit) {
			if (kwq->kw_prepost.count > 0) {
				__FAILEDUSERTEST__("psynch_mutexwait: more than one prepost\n");
				kwq->kw_prepost.lseq += PTHRW_INC; /* look for next one */
				ksyn_wqunlock(kwq);
				error = EINVAL;
				goto out;
			}
			_kwq_clear_preposted_wakeup(kwq);
		}

		if (kwq->kw_inqueue == 0) {
			updatebits = lseq | (PTH_RWL_KBIT | PTH_RWL_EBIT);
		} else {
			updatebits = (kwq->kw_highseq & PTHRW_COUNT_MASK) |
					(PTH_RWL_KBIT | PTH_RWL_EBIT);
		}
		updatebits &= ~PTH_RWL_MTX_WAIT;

		if (updatebits == 0) {
			__FAILEDUSERTEST__("psynch_mutexwait(prepost): returning 0 lseq in mutexwait with no EBIT \n");
		}

		PTHREAD_TRACE(psynch_mutex_kwqprepost, kwq->kw_addr,
				kwq->kw_prepost.lseq, kwq->kw_prepost.count, 1);

		old_owner = _kwq_set_owner(kwq, current_thread(), 0);
		pthread_kern->psynch_wait_update_owner(kwq, kwq->kw_owner,
				&kwq->kw_turnstile);

		ksyn_wqunlock(kwq);
		*retval = updatebits;
		goto out;
	}

	// mutexwait passes in an owner hint at the time userspace contended for
	// the mutex, however, the owner tid in the userspace data structure may be
	// unset or SWITCHING (-1), or it may correspond to a stale snapshot after
	// the lock has subsequently been unlocked by another thread.
	if (tid == thread_tid(kwq->kw_owner)) {
		// userspace and kernel agree
	} else if (tid == 0) {
		// contender came in before owner could write TID
		// let's assume that what the kernel knows is accurate
		// for all we know this waiter came in late in the kernel
	} else if (kwq->kw_lastunlockseq != PTHRW_RWL_INIT &&
			   is_seqlower(ugen, kwq->kw_lastunlockseq)) {
		// owner is stale, someone has come in and unlocked since this
		// contended read the TID, so assume what is known in the kernel is
		// accurate
	} else if (tid == PTHREAD_MTX_TID_SWITCHING) {
		// userspace didn't know the owner because it was being unlocked, but
		// that unlocker hasn't reached the kernel yet. So assume what is known
		// in the kernel is accurate
	} else {
		// hint is being passed in for a specific thread, and we have no reason
		// not to trust it (like the kernel unlock sequence being higher)
		//
		// So resolve the hint to a thread_t if we haven't done so yet
		// and redrive as we dropped the lock
		if (tid_th == THREAD_NULL) {
			ksyn_wqunlock(kwq);
			tid_th = pthread_kern->task_findtid(current_task(), tid);
			if (tid_th == THREAD_NULL) tid = 0;
			goto again;
		}
		tid_th = _kwq_set_owner(kwq, tid_th, KWQ_SET_OWNER_TRANSFER_REF);
	}

	if (tid_th) {
		// We are on our way to block, and can't drop the spinlock anymore
		pthread_kern->thread_deallocate_safe(tid_th);
		tid_th = THREAD_NULL;
	}
	assert(old_owner == THREAD_NULL);
	error = ksyn_wait(kwq, KSYN_QUEUE_WRITE, mgen, ins_flags, 0, 0,
			psynch_mtxcontinue, kThreadWaitPThreadMutex);
	// ksyn_wait drops wait queue lock
out:
	pthread_kern->psynch_wait_cleanup();
	ksyn_wqrelease(kwq, 1, (KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_MTX));
	if (tid_th) {
		thread_deallocate(tid_th);
	}
	if (old_owner) {
		thread_deallocate(old_owner);
	}
	return error;
}

void __dead2
psynch_mtxcontinue(void *parameter, wait_result_t result)
{
	uthread_t uth = current_uthread();
	ksyn_wait_queue_t kwq = parameter;
	ksyn_waitq_element_t kwe = pthread_kern->uthread_get_uukwe(uth);

	ksyn_wqlock(kwq);

	int error = _wait_result_to_errno(result);
	if (error != 0) {
		if (kwe->kwe_kwqqueue) {
			ksyn_queue_remove_item(kwq, &kwq->kw_ksynqueues[KSYN_QUEUE_WRITE], kwe);
		}
	} else {
		uint32_t updatebits = kwe->kwe_psynchretval & ~PTH_RWL_MTX_WAIT;
		pthread_kern->uthread_set_returnval(uth, updatebits);

		if (updatebits == 0) {
			__FAILEDUSERTEST__("psynch_mutexwait: returning 0 lseq in mutexwait with no EBIT \n");
		}
	}

	pthread_kern->psynch_wait_complete(kwq, &kwq->kw_turnstile);

	ksyn_wqunlock(kwq);
	pthread_kern->psynch_wait_cleanup();
	ksyn_wqrelease(kwq, 1, (KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_MTX));
	pthread_kern->unix_syscall_return(error);
	__builtin_unreachable();
}

static void __dead2
_psynch_rw_continue(ksyn_wait_queue_t kwq, kwq_queue_type_t kqi,
		wait_result_t result)
{
	uthread_t uth = current_uthread();
	ksyn_waitq_element_t kwe = pthread_kern->uthread_get_uukwe(uth);

	ksyn_wqlock(kwq);

	int error = _wait_result_to_errno(result);
	if (error != 0) {
		if (kwe->kwe_kwqqueue) {
			ksyn_queue_remove_item(kwq, &kwq->kw_ksynqueues[kqi], kwe);
		}
	} else {
		pthread_kern->uthread_set_returnval(uth, kwe->kwe_psynchretval);
	}

	ksyn_wqunlock(kwq);
	ksyn_wqrelease(kwq, 0, (KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_RWLOCK));

	pthread_kern->unix_syscall_return(error);
	__builtin_unreachable();
}

void __dead2
psynch_rw_rdcontinue(void *parameter, wait_result_t result)
{
	_psynch_rw_continue(parameter, KSYN_QUEUE_READ, result);
}

void __dead2
psynch_rw_wrcontinue(void *parameter, wait_result_t result)
{
	_psynch_rw_continue(parameter, KSYN_QUEUE_WRITE, result);
}

/*
 * psynch_mutexdrop: This system call is used for unlock postings on contended psynch mutexes.
 */
int
_psynch_mutexdrop(__unused proc_t p, user_addr_t mutex, uint32_t mgen,
		uint32_t ugen, uint64_t tid __unused, uint32_t flags, uint32_t *retval)
{
	int res;
	ksyn_wait_queue_t kwq;

	res = ksyn_wqfind(mutex, mgen, ugen, 0, flags, KSYN_WQTYPE_MUTEXDROP, &kwq);
	if (res == 0) {
		uint32_t updateval = _psynch_mutexdrop_internal(kwq, mgen, ugen, flags);
		/* drops the kwq reference */
		if (retval) {
			*retval = updateval;
		}
	}

	return res;
}

static kern_return_t
ksyn_mtxsignal(ksyn_wait_queue_t kwq, ksyn_waitq_element_t kwe,
		uint32_t updateval, thread_t *old_owner)
{
	kern_return_t ret;

	if (!kwe) {
		kwe = TAILQ_FIRST(&kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_kwelist);
		if (!kwe) {
			panic("ksyn_mtxsignal: panic signaling empty queue");
		}
	}

	PTHREAD_TRACE(psynch_mutex_kwqsignal | DBG_FUNC_START, kwq->kw_addr, kwe,
			thread_tid(kwe->kwe_thread), kwq->kw_inqueue);

	ret = ksyn_signal(kwq, KSYN_QUEUE_WRITE, kwe, updateval);
	if (ret == KERN_SUCCESS) {
		*old_owner = _kwq_set_owner(kwq, kwe->kwe_thread, 0);
	} else {
		*old_owner = _kwq_clear_owner(kwq);
	}
	PTHREAD_TRACE(psynch_mutex_kwqsignal | DBG_FUNC_END, kwq->kw_addr, kwe,
			ret, 0);
	return ret;
}


static void
ksyn_prepost(ksyn_wait_queue_t kwq, ksyn_waitq_element_t kwe, uint32_t state,
	     uint32_t lockseq)
{
	bzero(kwe, sizeof(*kwe));
	kwe->kwe_state = state;
	kwe->kwe_lockseq = lockseq;
	kwe->kwe_count = 1;

	(void)ksyn_queue_insert(kwq, KSYN_QUEUE_WRITE, kwe, lockseq, SEQFIT);
	kwq->kw_fakecount++;
}

static void
ksyn_cvsignal(ksyn_wait_queue_t ckwq, thread_t th, uint32_t uptoseq,
		uint32_t signalseq, uint32_t *updatebits, int *broadcast,
		ksyn_waitq_element_t *nkwep)
{
	ksyn_waitq_element_t kwe = NULL;
	ksyn_waitq_element_t nkwe = NULL;
	ksyn_queue_t kq = &ckwq->kw_ksynqueues[KSYN_QUEUE_WRITE];

	uptoseq &= PTHRW_COUNT_MASK;

	// Find the specified thread to wake.
	if (th != THREAD_NULL) {
		uthread_t uth = pthread_kern->get_bsdthread_info(th);
		kwe = pthread_kern->uthread_get_uukwe(uth);
		if (kwe->kwe_kwqqueue != ckwq ||
		    is_seqhigher(kwe->kwe_lockseq, uptoseq)) {
			// Unless it's no longer waiting on this CV...
			kwe = NULL;
			// ...in which case we post a broadcast instead.
			*broadcast = 1;
			return;
		}
	}

	// If no thread was specified, find any thread to wake (with the right
	// sequence number).
	while (th == THREAD_NULL) {
		if (kwe == NULL) {
			kwe = ksyn_queue_find_signalseq(ckwq, kq, uptoseq, signalseq);
		}
		if (kwe == NULL && nkwe == NULL) {
			// No eligible entries; need to allocate a new
			// entry to prepost. Loop to rescan after
			// reacquiring the lock after allocation in
			// case anything new shows up.
			ksyn_wqunlock(ckwq);
			nkwe = (ksyn_waitq_element_t)zalloc(kwe_zone);
			ksyn_wqlock(ckwq);
		} else {
			break;
		}
	}

	if (kwe != NULL) {
		// If we found a thread to wake...
		if (kwe->kwe_state == KWE_THREAD_INWAIT) {
			if (is_seqlower(kwe->kwe_lockseq, signalseq)) {
				/*
				 * A valid thread in our range, but lower than our signal.
				 * Matching it may leave our match with nobody to wake it if/when
				 * it arrives (the signal originally meant for this thread might
				 * not successfully wake it).
				 *
				 * Convert to broadcast - may cause some spurious wakeups
				 * (allowed by spec), but avoids starvation (better choice).
				 */
				*broadcast = 1;
			} else {
				(void)ksyn_signal(ckwq, KSYN_QUEUE_WRITE, kwe, PTH_RWL_MTX_WAIT);
				*updatebits += PTHRW_INC;
			}
		} else if (kwe->kwe_state == KWE_THREAD_PREPOST) {
			// Merge with existing prepost at same uptoseq.
			kwe->kwe_count += 1;
		} else if (kwe->kwe_state == KWE_THREAD_BROADCAST) {
			// Existing broadcasts subsume this signal.
		} else {
			panic("unknown kwe state\n");
		}
		if (nkwe) {
			/*
			 * If we allocated a new kwe above but then found a different kwe to
			 * use then we need to deallocate the spare one.
			 */
			zfree(kwe_zone, nkwe);
			nkwe = NULL;
		}
	} else if (nkwe != NULL) {
		// ... otherwise, insert the newly allocated prepost.
		ksyn_prepost(ckwq, nkwe, KWE_THREAD_PREPOST, uptoseq);
		nkwe = NULL;
	} else {
		panic("failed to allocate kwe\n");
	}

	*nkwep = nkwe;
}

static int
__psynch_cvsignal(user_addr_t cv, uint32_t cgen, uint32_t cugen,
		uint32_t csgen, uint32_t flags, int broadcast,
		mach_port_name_t threadport, uint32_t *retval)
{
	int error = 0;
	thread_t th = THREAD_NULL;
	ksyn_wait_queue_t kwq;
	
	uint32_t uptoseq = cgen & PTHRW_COUNT_MASK;
	uint32_t fromseq = (cugen & PTHRW_COUNT_MASK) + PTHRW_INC;
	
	// validate sane L, U, and S values
	if ((threadport == 0 && is_seqhigher(fromseq, uptoseq)) || is_seqhigher(csgen, uptoseq)) {
		__FAILEDUSERTEST__("cvbroad: invalid L, U and S values\n");
		return EINVAL;
	}
	
	if (threadport != 0) {
		th = port_name_to_thread((mach_port_name_t)threadport);
		if (th == THREAD_NULL) {
			return ESRCH;
		}
	}
	
	error = ksyn_wqfind(cv, cgen, cugen, csgen, flags, (KSYN_WQTYPE_CVAR | KSYN_WQTYPE_INDROP), &kwq);
	if (error == 0) {
		uint32_t updatebits = 0;
		ksyn_waitq_element_t nkwe = NULL;
		
		ksyn_wqlock(kwq);
		
		// update L, U and S...
		UPDATE_CVKWQ(kwq, cgen, cugen, csgen);

		PTHREAD_TRACE(psynch_cvar_signal | DBG_FUNC_START, kwq->kw_addr,
				fromseq, uptoseq, broadcast);

		if (!broadcast) {
			// No need to signal if the CV is already balanced.
			if (diff_genseq(kwq->kw_lword, kwq->kw_sword)) {
				ksyn_cvsignal(kwq, th, uptoseq, fromseq, &updatebits,
						&broadcast, &nkwe);
				PTHREAD_TRACE(psynch_cvar_signal, kwq->kw_addr, broadcast, 0,0);
			}
		}
		
		if (broadcast) {
			ksyn_handle_cvbroad(kwq, uptoseq, &updatebits);
		}
		
		kwq->kw_sword += (updatebits & PTHRW_COUNT_MASK);
		// set C or P bits and free if needed
		ksyn_cvupdate_fixup(kwq, &updatebits);
		*retval = updatebits;

		PTHREAD_TRACE(psynch_cvar_signal | DBG_FUNC_END, kwq->kw_addr,
				updatebits, 0, 0);
		
		ksyn_wqunlock(kwq);

		pthread_kern->psynch_wait_cleanup();
		
		if (nkwe != NULL) {
			zfree(kwe_zone, nkwe);
		}
		
		ksyn_wqrelease(kwq, 1, (KSYN_WQTYPE_INDROP | KSYN_WQTYPE_CVAR));
	}
	
	if (th != NULL) {
		thread_deallocate(th);
	}
	
	return error;
}

/*
 * psynch_cvbroad: This system call is used for broadcast posting on blocked waiters of psynch cvars.
 */
int
_psynch_cvbroad(__unused proc_t p, user_addr_t cv, uint64_t cvlsgen,
		uint64_t cvudgen, uint32_t flags, __unused user_addr_t mutex,
		__unused uint64_t mugen, __unused uint64_t tid, uint32_t *retval)
{
	uint32_t diffgen = cvudgen & 0xffffffff;
	uint32_t count = diffgen >> PTHRW_COUNT_SHIFT;
	if (count > pthread_kern->get_task_threadmax()) {
		__FAILEDUSERTEST__("cvbroad: difference greater than maximum possible thread count\n");
		return EBUSY;
	}
	
	uint32_t csgen = (cvlsgen >> 32) & 0xffffffff;
	uint32_t cgen = cvlsgen & 0xffffffff;
	uint32_t cugen = (cvudgen >> 32) & 0xffffffff;
	
	return __psynch_cvsignal(cv, cgen, cugen, csgen, flags, 1, 0, retval);
}

/*
 * psynch_cvsignal: This system call is used for signalling the blocked waiters of psynch cvars.
 */
int
_psynch_cvsignal(__unused proc_t p, user_addr_t cv, uint64_t cvlsgen,
		 uint32_t cvugen, int threadport, __unused user_addr_t mutex,
		 __unused uint64_t mugen, __unused uint64_t tid, uint32_t flags,
		 uint32_t *retval)
{
	uint32_t csgen = (cvlsgen >> 32) & 0xffffffff;
	uint32_t cgen = cvlsgen & 0xffffffff;
	
	return __psynch_cvsignal(cv, cgen, cvugen, csgen, flags, 0, threadport, retval);
}

/*
 * psynch_cvwait: This system call is used for psynch cvar waiters to block in kernel.
 */
int
_psynch_cvwait(__unused proc_t p, user_addr_t cv, uint64_t cvlsgen,
		uint32_t cvugen, user_addr_t mutex, uint64_t mugen, uint32_t flags,
		int64_t sec, uint32_t nsec, uint32_t *retval)
{
	int error = 0;
	uint32_t updatebits = 0;
	ksyn_wait_queue_t ckwq = NULL;
	ksyn_waitq_element_t kwe, nkwe = NULL;
	
	/* for conformance reasons */
	pthread_kern->__pthread_testcancel(0);
	
	uint32_t csgen = (cvlsgen >> 32) & 0xffffffff;
	uint32_t cgen = cvlsgen & 0xffffffff;
	uint32_t ugen = (mugen >> 32) & 0xffffffff;
	uint32_t mgen = mugen & 0xffffffff;
	
	uint32_t lockseq = (cgen & PTHRW_COUNT_MASK);
	
	/*
	 * In cvwait U word can be out of range as cv could be used only for
	 * timeouts. However S word needs to be within bounds and validated at
	 * user level as well.
	 */
	if (is_seqhigher_eq(csgen, lockseq) != 0) {
		__FAILEDUSERTEST__("psync_cvwait; invalid sequence numbers\n");
		return EINVAL;
	}

	PTHREAD_TRACE(psynch_cvar_kwait | DBG_FUNC_START, cv, mutex, cgen, 0);
	
	error = ksyn_wqfind(cv, cgen, cvugen, csgen, flags, KSYN_WQTYPE_CVAR | KSYN_WQTYPE_INWAIT, &ckwq);
	if (error != 0) {
		return error;
	}
	
	if (mutex != 0) {
		uint32_t mutexrv = 0;
		error = _psynch_mutexdrop(NULL, mutex, mgen, ugen, 0, flags, &mutexrv);
		if (error != 0) {
			goto out;
		}
	}
	
	ksyn_wqlock(ckwq);
	
	// update L, U and S...
	UPDATE_CVKWQ(ckwq, cgen, cvugen, csgen);
	
	/* Look for the sequence for prepost (or conflicting thread */
	ksyn_queue_t kq = &ckwq->kw_ksynqueues[KSYN_QUEUE_WRITE];
	kwe = ksyn_queue_find_cvpreposeq(kq, lockseq);
	if (kwe != NULL) {
		if (kwe->kwe_state == KWE_THREAD_PREPOST) {
			if ((kwe->kwe_lockseq & PTHRW_COUNT_MASK) == lockseq) {
				/* we can safely consume a reference, so do so */
				if (--kwe->kwe_count == 0) {
					ksyn_queue_remove_item(ckwq, kq, kwe);
					ckwq->kw_fakecount--;
					nkwe = kwe;
				}
			} else {
				/*
				 * consuming a prepost higher than our lock sequence is valid, but
				 * can leave the higher thread without a match. Convert the entry
				 * to a broadcast to compensate for this.
				 */
				ksyn_handle_cvbroad(ckwq, kwe->kwe_lockseq, &updatebits);
#if __TESTPANICS__
				if (updatebits != 0)
					panic("psync_cvwait: convert pre-post to broadcast: woke up %d threads that shouldn't be there\n", updatebits);
#endif /* __TESTPANICS__ */
			}
		} else if (kwe->kwe_state == KWE_THREAD_BROADCAST) {
			// XXX
			// Nothing to do.
		} else if (kwe->kwe_state == KWE_THREAD_INWAIT) {
			__FAILEDUSERTEST__("cvwait: thread entry with same sequence already present\n");
			error = EBUSY;
		} else {
			panic("psync_cvwait: unexpected wait queue element type\n");
		}
		
		if (error == 0) {
			updatebits |= PTHRW_INC;
			ckwq->kw_sword += PTHRW_INC;
			
			/* set C or P bits and free if needed */
			ksyn_cvupdate_fixup(ckwq, &updatebits);
			*retval = updatebits;
		}
	} else {
		uint64_t abstime = 0;
		uint16_t kwe_flags = 0;

		if (sec != 0 || (nsec & 0x3fffffff) != 0) {
			struct timespec ts;
			ts.tv_sec = (__darwin_time_t)sec;
			ts.tv_nsec = (nsec & 0x3fffffff);
			nanoseconds_to_absolutetime(
					(uint64_t)ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec, &abstime);
			clock_absolutetime_interval_to_deadline(abstime, &abstime);
		}

		PTHREAD_TRACE(psynch_cvar_kwait, cv, mutex, kwe_flags, 1);
		
		error = ksyn_wait(ckwq, KSYN_QUEUE_WRITE, cgen, SEQFIT, abstime,
				kwe_flags, psynch_cvcontinue, kThreadWaitPThreadCondVar);
		// ksyn_wait drops wait queue lock
	}
	
	ksyn_wqunlock(ckwq);

	if (nkwe != NULL) {
		zfree(kwe_zone, nkwe);
	}
out:

	PTHREAD_TRACE(psynch_cvar_kwait | DBG_FUNC_END, cv, error, updatebits, 2);

	ksyn_wqrelease(ckwq, 1, (KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_CVAR));
	return error;
}


void __dead2
psynch_cvcontinue(void *parameter, wait_result_t result)
{
	uthread_t uth = current_uthread();
	ksyn_wait_queue_t ckwq = parameter;
	ksyn_waitq_element_t kwe = pthread_kern->uthread_get_uukwe(uth);

	int error = _wait_result_to_errno(result);
	if (error != 0) {
		ksyn_wqlock(ckwq);
		/* just in case it got woken up as we were granting */
		int retval = kwe->kwe_psynchretval;
		pthread_kern->uthread_set_returnval(uth, retval);

		if (kwe->kwe_kwqqueue) {
			ksyn_queue_remove_item(ckwq, &ckwq->kw_ksynqueues[KSYN_QUEUE_WRITE], kwe);
		}
		if ((kwe->kwe_psynchretval & PTH_RWL_MTX_WAIT) != 0) {
			/* the condition var granted.
			 * reset the error so that the thread returns back.
			 */
			error = 0;
			/* no need to set any bits just return as cvsig/broad covers this */
		} else {
			ckwq->kw_sword += PTHRW_INC;
			
			/* set C and P bits, in the local error */
			if ((ckwq->kw_lword & PTHRW_COUNT_MASK) == (ckwq->kw_sword & PTHRW_COUNT_MASK)) {
				PTHREAD_TRACE(psynch_cvar_zeroed, ckwq->kw_addr,
						ckwq->kw_lword, ckwq->kw_sword, ckwq->kw_inqueue);
				error |= ECVCLEARED;
				if (ckwq->kw_inqueue != 0) {
					ksyn_queue_free_items(ckwq, KSYN_QUEUE_WRITE, ckwq->kw_lword, 1);
				}
				ckwq->kw_lword = ckwq->kw_uword = ckwq->kw_sword = 0;
				ckwq->kw_kflags |= KSYN_KWF_ZEROEDOUT;
			} else {
				/* everythig in the queue is a fake entry ? */
				if (ckwq->kw_inqueue != 0 && ckwq->kw_fakecount == ckwq->kw_inqueue) {
					error |= ECVPREPOST;
				}
			}
		}
		ksyn_wqunlock(ckwq);

		PTHREAD_TRACE(psynch_cvar_kwait | DBG_FUNC_END, ckwq->kw_addr,
				error, 0, 3);
	} else {
		int val = 0;
		// PTH_RWL_MTX_WAIT is removed
		if ((kwe->kwe_psynchretval & PTH_RWS_CV_MBIT) != 0) {
			val = PTHRW_INC | PTH_RWS_CV_CBIT;
		}
		PTHREAD_TRACE(psynch_cvar_kwait | DBG_FUNC_END, ckwq->kw_addr,
				val, 0, 4);
		pthread_kern->uthread_set_returnval(uth, val);
	}
	
	ksyn_wqrelease(ckwq, 1, (KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_CVAR));
	pthread_kern->unix_syscall_return(error);
	__builtin_unreachable();
}

/*
 * psynch_cvclrprepost: This system call clears pending prepost if present.
 */
int
_psynch_cvclrprepost(__unused proc_t p, user_addr_t cv, uint32_t cvgen,
		uint32_t cvugen, uint32_t cvsgen, __unused uint32_t prepocnt,
		uint32_t preposeq, uint32_t flags, int *retval)
{
	int error = 0;
	int mutex = (flags & _PTHREAD_MTX_OPT_MUTEX);
	int wqtype = (mutex ? KSYN_WQTYPE_MTX : KSYN_WQTYPE_CVAR) | KSYN_WQTYPE_INDROP;
	ksyn_wait_queue_t kwq = NULL;
	
	*retval = 0;
	
	error = ksyn_wqfind(cv, cvgen, cvugen, mutex ? 0 : cvsgen, flags, wqtype,
			&kwq);
	if (error != 0) {
		return error;
	}
	
	ksyn_wqlock(kwq);
	
	if (mutex) {
		int firstfit = (flags & _PTHREAD_MTX_OPT_POLICY_MASK)
				== _PTHREAD_MTX_OPT_POLICY_FIRSTFIT;
		if (firstfit && kwq->kw_prepost.count) {
			if (is_seqlower_eq(kwq->kw_prepost.lseq, cvgen)) {
				PTHREAD_TRACE(psynch_mutex_kwqprepost, kwq->kw_addr,
						kwq->kw_prepost.lseq, 0, 2);
				_kwq_clear_preposted_wakeup(kwq);
			}
		}
	} else {
		PTHREAD_TRACE(psynch_cvar_clrprepost, kwq->kw_addr, wqtype,
				preposeq, 0);
		ksyn_queue_free_items(kwq, KSYN_QUEUE_WRITE, preposeq, 0);
	}
	
	ksyn_wqunlock(kwq);
	ksyn_wqrelease(kwq, 1, wqtype);
	return error;
}

/* ***************** pthread_rwlock ************************ */

static int
__psynch_rw_lock(int type, user_addr_t rwlock, uint32_t lgenval,
		 uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval)
{
	uint32_t lockseq = lgenval & PTHRW_COUNT_MASK;
	ksyn_wait_queue_t kwq;
	int error, prepost_type, kqi;
	thread_continue_t tc;

	if (type == PTH_RW_TYPE_READ) {
		prepost_type = KW_UNLOCK_PREPOST_READLOCK;
		kqi = KSYN_QUEUE_READ;
		tc = psynch_rw_rdcontinue;
	} else {
		prepost_type = KW_UNLOCK_PREPOST_WRLOCK;
		kqi = KSYN_QUEUE_WRITE;
		tc = psynch_rw_wrcontinue;
	}

	error = ksyn_wqfind(rwlock, lgenval, ugenval, rw_wc, flags,
			(KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_RWLOCK), &kwq);
	if (error != 0) {
		return error;
	}

	ksyn_wqlock(kwq);
	_ksyn_check_init(kwq, lgenval);
	if (_kwq_handle_interrupted_wakeup(kwq, type, lockseq, retval) ||
			// handle overlap first as they are not counted against pre_rwwc
			// handle_overlap uses the flags in lgenval (vs. lockseq)
			_kwq_handle_overlap(kwq, type, lgenval, rw_wc, retval) ||
			_kwq_handle_preposted_wakeup(kwq, prepost_type, lockseq, retval)) {
		ksyn_wqunlock(kwq);
		goto out;
	}

	block_hint_t block_hint = type == PTH_RW_TYPE_READ ?
		kThreadWaitPThreadRWLockRead : kThreadWaitPThreadRWLockWrite;
	error = ksyn_wait(kwq, kqi, lgenval, SEQFIT, 0, 0, tc, block_hint);
	// ksyn_wait drops wait queue lock
out:
	ksyn_wqrelease(kwq, 0, (KSYN_WQTYPE_INWAIT | KSYN_WQTYPE_RWLOCK));
	return error;
}

/*
 * psynch_rw_rdlock: This system call is used for psync rwlock readers to block.
 */
int
_psynch_rw_rdlock(__unused proc_t p, user_addr_t rwlock, uint32_t lgenval,
		uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval)
{
	return __psynch_rw_lock(PTH_RW_TYPE_READ, rwlock, lgenval, ugenval, rw_wc,
			flags, retval);
}

/*
 * psynch_rw_longrdlock: This system call is used for psync rwlock long readers to block.
 */
int
_psynch_rw_longrdlock(__unused proc_t p, __unused user_addr_t rwlock,
		__unused uint32_t lgenval, __unused uint32_t ugenval,
		__unused uint32_t rw_wc, __unused int flags, __unused uint32_t *retval)
{
	return ESRCH;
}


/*
 * psynch_rw_wrlock: This system call is used for psync rwlock writers to block.
 */
int
_psynch_rw_wrlock(__unused proc_t p, user_addr_t rwlock, uint32_t lgenval,
		uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval)
{
	return __psynch_rw_lock(PTH_RW_TYPE_WRITE, rwlock, lgenval, ugenval,
			rw_wc, flags, retval);
}

/*
 * psynch_rw_yieldwrlock: This system call is used for psync rwlock yielding writers to block.
 */
int
_psynch_rw_yieldwrlock(__unused proc_t p, __unused user_addr_t rwlock,
		__unused uint32_t lgenval, __unused uint32_t ugenval,
		__unused uint32_t rw_wc, __unused int flags, __unused uint32_t *retval)
{
	return ESRCH;
}

/*
 * psynch_rw_unlock: This system call is used for unlock state postings. This will grant appropriate
 *			reader/writer variety lock.
 */
int
_psynch_rw_unlock(__unused proc_t p, user_addr_t rwlock, uint32_t lgenval,
		uint32_t ugenval, uint32_t rw_wc, int flags, uint32_t *retval)
{
	int error = 0;
	ksyn_wait_queue_t kwq;
	uint32_t updatebits = 0;
	int diff;
	uint32_t count = 0;
	uint32_t curgen = lgenval & PTHRW_COUNT_MASK;
	int clearedkflags = 0;

	error = ksyn_wqfind(rwlock, lgenval, ugenval, rw_wc, flags,
			(KSYN_WQTYPE_INDROP | KSYN_WQTYPE_RWLOCK), &kwq);
	if (error != 0) {
		return(error);
	}
	
	ksyn_wqlock(kwq);
	int isinit = _ksyn_check_init(kwq, lgenval);

	/* if lastunlock seq is set, ensure the current one is not lower than that, as it would be spurious */
	if ((kwq->kw_lastunlockseq != PTHRW_RWL_INIT) &&
			(is_seqlower(ugenval, kwq->kw_lastunlockseq)!= 0)) {
		error = 0;
		goto out;
	}
	
	/* If L-U != num of waiters, then it needs to be preposted or spr */
	diff = find_diff(lgenval, ugenval);
	
	if (find_seq_till(kwq, curgen, diff, &count) == 0) {
		if ((count == 0) || (count < (uint32_t)diff))
			goto prepost;
	}
	
	/* no prepost and all threads are in place, reset the bit */
	if ((isinit != 0) && ((kwq->kw_kflags & KSYN_KWF_INITCLEARED) != 0)){
		kwq->kw_kflags &= ~KSYN_KWF_INITCLEARED;
		clearedkflags = 1;
	}
	
	/* can handle unlock now */
	
	_kwq_clear_preposted_wakeup(kwq);
	
	error = kwq_handle_unlock(kwq, lgenval, rw_wc, &updatebits, 0, NULL, 0);
#if __TESTPANICS__
	if (error != 0)
		panic("psynch_rw_unlock: kwq_handle_unlock failed %d\n",error);
#endif /* __TESTPANICS__ */
out:
	if (error == 0) {
		/* update bits?? */
		*retval = updatebits;
	}

	// <rdar://problem/22244050> If any of the wakeups failed because they
	// already returned to userspace because of a signal then we need to ensure
	// that the reset state is not cleared when that thread returns. Otherwise,
	// _pthread_rwlock_lock will clear the interrupted state before it is read.
	if (clearedkflags != 0 && kwq->kw_intr.count > 0) {
		kwq->kw_kflags |= KSYN_KWF_INITCLEARED;
	}
	
	ksyn_wqunlock(kwq);
	pthread_kern->psynch_wait_cleanup();
	ksyn_wqrelease(kwq, 0, (KSYN_WQTYPE_INDROP | KSYN_WQTYPE_RWLOCK));
	
	return(error);
	
prepost:
	/* update if the new seq is higher than prev prepost, or first set */
	if (is_rws_sbit_set(kwq->kw_prepost.sseq) ||
			is_seqhigher_eq(rw_wc, kwq->kw_prepost.sseq)) {
		_kwq_mark_preposted_wakeup(kwq, diff - count, curgen, rw_wc);
		updatebits = lgenval;	/* let this not do unlock handling */
	}
	error = 0;
	goto out;
}


/* ************************************************************************** */
void
pth_global_hashinit(void)
{
	pth_glob_hashtbl = hashinit(PTH_HASHSIZE * 4, M_PROC, &pthhash);
}

void
_pth_proc_hashinit(proc_t p)
{
	void *ptr = hashinit(PTH_HASHSIZE, M_PCB, &pthhash);
	if (ptr == NULL) {
		panic("pth_proc_hashinit: hash init returned 0\n");
	}
	
	pthread_kern->proc_set_pthhash(p, ptr);
}


static int
ksyn_wq_hash_lookup(user_addr_t uaddr, proc_t p, int flags,
		ksyn_wait_queue_t *out_kwq, struct pthhashhead **out_hashptr,
		uint64_t object, uint64_t offset)
{
	int res = 0;
	ksyn_wait_queue_t kwq;
	struct pthhashhead *hashptr;
	if ((flags & PTHREAD_PSHARED_FLAGS_MASK) == PTHREAD_PROCESS_SHARED) {
		hashptr = pth_glob_hashtbl;
		LIST_FOREACH(kwq, &hashptr[object & pthhash], kw_hash) {
			if (kwq->kw_object == object && kwq->kw_offset == offset) {
				break;
			}
		}
	} else {
		hashptr = pthread_kern->proc_get_pthhash(p);
		LIST_FOREACH(kwq, &hashptr[uaddr & pthhash], kw_hash) {
			if (kwq->kw_addr == uaddr) {
				break;
			}
		}
	}
	*out_kwq = kwq;
	*out_hashptr = hashptr;
	return res;
}

void
_pth_proc_hashdelete(proc_t p)
{
	struct pthhashhead * hashptr;
	ksyn_wait_queue_t kwq;
	unsigned long hashsize = pthhash + 1;
	unsigned long i;
	
	hashptr = pthread_kern->proc_get_pthhash(p);
	pthread_kern->proc_set_pthhash(p, NULL);
	if (hashptr == NULL) {
		return;
	}
	
	pthread_list_lock();
	for(i= 0; i < hashsize; i++) {
		while ((kwq = LIST_FIRST(&hashptr[i])) != NULL) {
			if ((kwq->kw_pflags & KSYN_WQ_INHASH) != 0) {
				kwq->kw_pflags &= ~KSYN_WQ_INHASH;
				LIST_REMOVE(kwq, kw_hash);
			}
			if ((kwq->kw_pflags & KSYN_WQ_FLIST) != 0) {
				kwq->kw_pflags &= ~KSYN_WQ_FLIST;
				LIST_REMOVE(kwq, kw_list);
			}
			pthread_list_unlock();
			/* release fake entries if present for cvars */
			if (((kwq->kw_type & KSYN_WQTYPE_MASK) == KSYN_WQTYPE_CVAR) && (kwq->kw_inqueue != 0))
				ksyn_freeallkwe(&kwq->kw_ksynqueues[KSYN_QUEUE_WRITE]);
			_kwq_destroy(kwq);
			pthread_list_lock();
		}
	}
	pthread_list_unlock();
	FREE(hashptr, M_PROC);
}

/* no lock held for this as the waitqueue is getting freed */
void
ksyn_freeallkwe(ksyn_queue_t kq)
{
	ksyn_waitq_element_t kwe;
	while ((kwe = TAILQ_FIRST(&kq->ksynq_kwelist)) != NULL) {
		TAILQ_REMOVE(&kq->ksynq_kwelist, kwe, kwe_list);
		if (kwe->kwe_state != KWE_THREAD_INWAIT) {
			zfree(kwe_zone, kwe);
		}
	}
}

static inline void
_kwq_report_inuse(ksyn_wait_queue_t kwq)
{
	if (kwq->kw_prepost.count != 0) {
		__FAILEDUSERTEST2__("uaddr 0x%llx busy for synch type 0x%x [pre %d:0x%x:0x%x]",
				(uint64_t)kwq->kw_addr, kwq->kw_type, kwq->kw_prepost.count,
				kwq->kw_prepost.lseq, kwq->kw_prepost.sseq);
		PTHREAD_TRACE(psynch_mutex_kwqcollision, kwq->kw_addr,
				kwq->kw_type, 1, 0);
	}
	if (kwq->kw_intr.count != 0) {
		__FAILEDUSERTEST2__("uaddr 0x%llx busy for synch type 0x%x [intr %d:0x%x:0x%x:0x%x]",
				(uint64_t)kwq->kw_addr, kwq->kw_type, kwq->kw_intr.count,
				kwq->kw_intr.type, kwq->kw_intr.seq,
				kwq->kw_intr.returnbits);
		PTHREAD_TRACE(psynch_mutex_kwqcollision, kwq->kw_addr,
				kwq->kw_type, 2, 0);
	}
	if (kwq->kw_iocount) {
		__FAILEDUSERTEST2__("uaddr 0x%llx busy for synch type 0x%x [ioc %d:%d]",
				(uint64_t)kwq->kw_addr, kwq->kw_type, kwq->kw_iocount,
				kwq->kw_dropcount);
		PTHREAD_TRACE(psynch_mutex_kwqcollision, kwq->kw_addr,
				kwq->kw_type, 3, 0);
	}
	if (kwq->kw_inqueue) {
		__FAILEDUSERTEST2__("uaddr 0x%llx busy for synch type 0x%x [inq %d:%d]",
				(uint64_t)kwq->kw_addr, kwq->kw_type, kwq->kw_inqueue,
				kwq->kw_fakecount);
		PTHREAD_TRACE(psynch_mutex_kwqcollision, kwq->kw_addr, kwq->kw_type,
				4, 0);
	}
}

/* find kernel waitqueue, if not present create one. Grants a reference  */
int
ksyn_wqfind(user_addr_t uaddr, uint32_t mgen, uint32_t ugen, uint32_t sgen,
		int flags, int wqtype, ksyn_wait_queue_t *kwqp)
{
	int res = 0;
	ksyn_wait_queue_t kwq = NULL;
	ksyn_wait_queue_t nkwq = NULL;
	struct pthhashhead *hashptr;
	proc_t p = current_proc();
	
	uint64_t object = 0, offset = 0;
	if ((flags & PTHREAD_PSHARED_FLAGS_MASK) == PTHREAD_PROCESS_SHARED) {
		res = ksyn_findobj(uaddr, &object, &offset);
		hashptr = pth_glob_hashtbl;
	} else {
		hashptr = pthread_kern->proc_get_pthhash(p);
	}

	while (res == 0) {
		pthread_list_lock();
		res = ksyn_wq_hash_lookup(uaddr, current_proc(), flags, &kwq, &hashptr,
				object, offset);
		if (res != 0) {
			pthread_list_unlock();
			break;
		}
		if (kwq == NULL && nkwq == NULL) {
			// Drop the lock to allocate a new kwq and retry.
			pthread_list_unlock();

			nkwq = (ksyn_wait_queue_t)zalloc(kwq_zone);
			bzero(nkwq, sizeof(struct ksyn_wait_queue));
			int i;
			for (i = 0; i < KSYN_QUEUE_MAX; i++) {
				ksyn_queue_init(&nkwq->kw_ksynqueues[i]);
			}
			lck_spin_init(&nkwq->kw_lock, pthread_lck_grp, pthread_lck_attr);
			continue;
		} else if (kwq == NULL && nkwq != NULL) {
			// Still not found, add the new kwq to the hash.
			kwq = nkwq;
			nkwq = NULL; // Don't free.
			if ((flags & PTHREAD_PSHARED_FLAGS_MASK) == PTHREAD_PROCESS_SHARED) {
				kwq->kw_pflags |= KSYN_WQ_SHARED;
				LIST_INSERT_HEAD(&hashptr[object & pthhash], kwq, kw_hash);
			} else {
				LIST_INSERT_HEAD(&hashptr[uaddr & pthhash], kwq, kw_hash);
			}
			kwq->kw_pflags |= KSYN_WQ_INHASH;
		} else if (kwq != NULL) {
			// Found an existing kwq, use it.
			if ((kwq->kw_pflags & KSYN_WQ_FLIST) != 0) {
				LIST_REMOVE(kwq, kw_list);
				kwq->kw_pflags &= ~KSYN_WQ_FLIST;
			}
			if ((kwq->kw_type & KSYN_WQTYPE_MASK) != (wqtype & KSYN_WQTYPE_MASK)) {
				if (!_kwq_is_used(kwq)) {
					if (kwq->kw_iocount == 0) {
						kwq->kw_type = 0; // mark for reinitialization
					} else if (kwq->kw_iocount == 1 &&
							kwq->kw_dropcount == kwq->kw_iocount) {
						/* if all users are unlockers then wait for it to finish */
						kwq->kw_pflags |= KSYN_WQ_WAITING;
						// Drop the lock and wait for the kwq to be free.
						(void)msleep(&kwq->kw_pflags, pthread_list_mlock,
								PDROP, "ksyn_wqfind", 0);
						continue;
					} else {
						_kwq_report_inuse(kwq);
						res = EINVAL;
					}
				} else {
					_kwq_report_inuse(kwq);
					res = EINVAL;
				}
			}
		}
		if (res == 0) {
			if (kwq->kw_type == 0) {
				kwq->kw_addr = uaddr;
				kwq->kw_object = object;
				kwq->kw_offset = offset;
				kwq->kw_type = (wqtype & KSYN_WQTYPE_MASK);
				CLEAR_REINIT_BITS(kwq);
				kwq->kw_lword = mgen;
				kwq->kw_uword = ugen;
				kwq->kw_sword = sgen;
				kwq->kw_owner = THREAD_NULL;
				kwq->kw_kflags = 0;
				kwq->kw_qos_override = THREAD_QOS_UNSPECIFIED;
				PTHREAD_TRACE(psynch_mutex_kwqallocate | DBG_FUNC_START, uaddr,
						kwq->kw_type, kwq, 0);
				PTHREAD_TRACE(psynch_mutex_kwqallocate | DBG_FUNC_END, uaddr,
						mgen, ugen, sgen);
			}
			kwq->kw_iocount++;
			if (wqtype == KSYN_WQTYPE_MUTEXDROP) {
				kwq->kw_dropcount++;
			}
		}
		pthread_list_unlock();
		break;
	}
	if (kwqp != NULL) {
		*kwqp = kwq;
	}
	if (nkwq) {
		_kwq_destroy(nkwq);
	}
	return res;
}

/* Reference from find is dropped here. Starts the free process if needed */
void
ksyn_wqrelease(ksyn_wait_queue_t kwq, int qfreenow, int wqtype)
{
	uint64_t deadline;
	ksyn_wait_queue_t free_elem = NULL;
	
	pthread_list_lock();
	if (wqtype == KSYN_WQTYPE_MUTEXDROP) {
		kwq->kw_dropcount--;
	}
	if (--kwq->kw_iocount == 0) {
		if ((kwq->kw_pflags & KSYN_WQ_WAITING) != 0) {
			/* some one is waiting for the waitqueue, wake them up */
			kwq->kw_pflags &= ~KSYN_WQ_WAITING;
			wakeup(&kwq->kw_pflags);
		}
		
		if (!_kwq_is_used(kwq)) {
			if (kwq->kw_turnstile) {
				panic("kw_turnstile still non-null upon release");
			}

			PTHREAD_TRACE(psynch_mutex_kwqdeallocate | DBG_FUNC_START,
					kwq->kw_addr, kwq->kw_type, qfreenow, 0);
			PTHREAD_TRACE(psynch_mutex_kwqdeallocate | DBG_FUNC_END,
					kwq->kw_addr, kwq->kw_lword, kwq->kw_uword, kwq->kw_sword);

			if (qfreenow == 0) {
				microuptime(&kwq->kw_ts);
				LIST_INSERT_HEAD(&pth_free_list, kwq, kw_list);
				kwq->kw_pflags |= KSYN_WQ_FLIST;
				if (psynch_cleanupset == 0) {
					struct timeval t;
					microuptime(&t);
					t.tv_sec += KSYN_CLEANUP_DEADLINE;
					deadline = tvtoabstime(&t);
					thread_call_enter_delayed(psynch_thcall, deadline);
					psynch_cleanupset = 1;
				}
			} else {
				kwq->kw_pflags &= ~KSYN_WQ_INHASH;
				LIST_REMOVE(kwq, kw_hash);
				free_elem = kwq;
			}
		}
	}
	pthread_list_unlock();
	if (free_elem != NULL) {
		_kwq_destroy(free_elem);
	}
}

/* responsible to free the waitqueues */
void
psynch_wq_cleanup(__unused void *param, __unused void * param1)
{
	ksyn_wait_queue_t kwq, tmp;
	struct timeval t;
	int reschedule = 0;
	uint64_t deadline = 0;
	LIST_HEAD(, ksyn_wait_queue) freelist;
	LIST_INIT(&freelist);

	pthread_list_lock();
	
	microuptime(&t);
	
	LIST_FOREACH(kwq, &pth_free_list, kw_list) {
		if (_kwq_is_used(kwq) || kwq->kw_iocount != 0) {
			// still in use
			continue;
		}
		__darwin_time_t diff = t.tv_sec - kwq->kw_ts.tv_sec;
		if (diff < 0)
			diff *= -1;
		if (diff >= KSYN_CLEANUP_DEADLINE) {
			kwq->kw_pflags &= ~(KSYN_WQ_FLIST | KSYN_WQ_INHASH);
			LIST_REMOVE(kwq, kw_hash);
			LIST_REMOVE(kwq, kw_list);
			LIST_INSERT_HEAD(&freelist, kwq, kw_list);
		} else {
			reschedule = 1;
		}
		
	}
	if (reschedule != 0) {
		t.tv_sec += KSYN_CLEANUP_DEADLINE;
		deadline = tvtoabstime(&t);
		thread_call_enter_delayed(psynch_thcall, deadline);
		psynch_cleanupset = 1;
	} else {
		psynch_cleanupset = 0;
	}
	pthread_list_unlock();

	LIST_FOREACH_SAFE(kwq, &freelist, kw_list, tmp) {
		_kwq_destroy(kwq);
	}
}

static int
_wait_result_to_errno(wait_result_t result)
{
	int res = 0;
	switch (result) {
		case THREAD_TIMED_OUT:
			res = ETIMEDOUT;
			break;
		case THREAD_INTERRUPTED:
			res = EINTR;
			break;
	}
	return res;
}

int
ksyn_wait(ksyn_wait_queue_t kwq, kwq_queue_type_t kqi, uint32_t lockseq,
		int fit, uint64_t abstime, uint16_t kwe_flags,
		thread_continue_t continuation, block_hint_t block_hint)
{
	thread_t th = current_thread();
	uthread_t uth = pthread_kern->get_bsdthread_info(th);
	struct turnstile **tstore = NULL;
	int res;

	assert(continuation != THREAD_CONTINUE_NULL);

	ksyn_waitq_element_t kwe = pthread_kern->uthread_get_uukwe(uth);
	bzero(kwe, sizeof(*kwe));
	kwe->kwe_count = 1;
	kwe->kwe_lockseq = lockseq & PTHRW_COUNT_MASK;
	kwe->kwe_state = KWE_THREAD_INWAIT;
	kwe->kwe_uth = uth;
	kwe->kwe_thread = th;
	kwe->kwe_flags = kwe_flags;

	res = ksyn_queue_insert(kwq, kqi, kwe, lockseq, fit);
	if (res != 0) {
		//panic("psynch_rw_wrlock: failed to enqueue\n"); // XXX
		ksyn_wqunlock(kwq);
		return res;
	}

	PTHREAD_TRACE(psynch_mutex_kwqwait, kwq->kw_addr, kwq->kw_inqueue,
			kwq->kw_prepost.count, kwq->kw_intr.count);

	if (_kwq_use_turnstile(kwq)) {
		// pthread mutexes and rwlocks both (at least sometimes) know their
		// owner and can use turnstiles. Otherwise, we pass NULL as the
		// tstore to the shims so they wait on the global waitq.
		tstore = &kwq->kw_turnstile;
	}

	pthread_kern->psynch_wait_prepare((uintptr_t)kwq, tstore, kwq->kw_owner,
			block_hint, abstime);

	ksyn_wqunlock(kwq);

	if (tstore) {
		pthread_kern->psynch_wait_update_complete(kwq->kw_turnstile);
	}
	
	thread_block_parameter(continuation, kwq);

	// NOT REACHED
	panic("ksyn_wait continuation returned");
	__builtin_unreachable();
}

kern_return_t
ksyn_signal(ksyn_wait_queue_t kwq, kwq_queue_type_t kqi,
		ksyn_waitq_element_t kwe, uint32_t updateval)
{
	kern_return_t ret;
	struct turnstile **tstore = NULL;

	// If no wait element was specified, wake the first.
	if (!kwe) {
		kwe = TAILQ_FIRST(&kwq->kw_ksynqueues[kqi].ksynq_kwelist);
		if (!kwe) {
			panic("ksyn_signal: panic signaling empty queue");
		}
	}

	if (kwe->kwe_state != KWE_THREAD_INWAIT) {
		panic("ksyn_signal: panic signaling non-waiting element");
	}

	ksyn_queue_remove_item(kwq, &kwq->kw_ksynqueues[kqi], kwe);
	kwe->kwe_psynchretval = updateval;

	if (_kwq_use_turnstile(kwq)) {
		tstore = &kwq->kw_turnstile;
	}

	ret = pthread_kern->psynch_wait_wakeup(kwq, kwe, tstore);

	if (ret != KERN_SUCCESS && ret != KERN_NOT_WAITING) {
		panic("ksyn_signal: panic waking up thread %x\n", ret);
	}
	return ret;
}

int
ksyn_findobj(user_addr_t uaddr, uint64_t *objectp, uint64_t *offsetp)
{
	kern_return_t ret;
	vm_page_info_basic_data_t info;
	mach_msg_type_number_t count = VM_PAGE_INFO_BASIC_COUNT;
	ret = pthread_kern->vm_map_page_info(pthread_kern->current_map(), uaddr,
			VM_PAGE_INFO_BASIC, (vm_page_info_t)&info, &count);
	if (ret != KERN_SUCCESS) {
		return EINVAL;
	}
	
	if (objectp != NULL) {
		*objectp = (uint64_t)info.object_id;
	}
	if (offsetp != NULL) {
		*offsetp = (uint64_t)info.offset;
	}
	
	return(0);
}


/* lowest of kw_fr, kw_flr, kw_fwr, kw_fywr */
int
kwq_find_rw_lowest(ksyn_wait_queue_t kwq, int flags, uint32_t premgen,
		int *typep, uint32_t lowest[])
{
	uint32_t kw_fr, kw_fwr, low;
	int type = 0, lowtype, typenum[2] = { 0 };
	uint32_t numbers[2] = { 0 };
	int count = 0, i;
	
	if ((kwq->kw_ksynqueues[KSYN_QUEUE_READ].ksynq_count != 0) ||
			((flags & KW_UNLOCK_PREPOST_READLOCK) != 0)) {
		type |= PTH_RWSHFT_TYPE_READ;
		/* read entries are present */
		if (kwq->kw_ksynqueues[KSYN_QUEUE_READ].ksynq_count != 0) {
			kw_fr = kwq->kw_ksynqueues[KSYN_QUEUE_READ].ksynq_firstnum;
			if (((flags & KW_UNLOCK_PREPOST_READLOCK) != 0) &&
					(is_seqlower(premgen, kw_fr) != 0))
				kw_fr = premgen;
		} else
			kw_fr = premgen;
		
		lowest[KSYN_QUEUE_READ] = kw_fr;
		numbers[count]= kw_fr;
		typenum[count] = PTH_RW_TYPE_READ;
		count++;
	} else
		lowest[KSYN_QUEUE_READ] = 0;
	
	if ((kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_count != 0) ||
			((flags & KW_UNLOCK_PREPOST_WRLOCK) != 0)) {
		type |= PTH_RWSHFT_TYPE_WRITE;
		/* read entries are present */
		if (kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_count != 0) {
			kw_fwr = kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_firstnum;
			if (((flags & KW_UNLOCK_PREPOST_WRLOCK) != 0) &&
					(is_seqlower(premgen, kw_fwr) != 0))
				kw_fwr = premgen;
		} else
			kw_fwr = premgen;
		
		lowest[KSYN_QUEUE_WRITE] = kw_fwr;
		numbers[count]= kw_fwr;
		typenum[count] = PTH_RW_TYPE_WRITE;
		count++;
	} else
		lowest[KSYN_QUEUE_WRITE] = 0;
	
#if __TESTPANICS__
	if (count == 0)
		panic("nothing in the queue???\n");
#endif /* __TESTPANICS__ */
	
	low = numbers[0];
	lowtype = typenum[0];
	if (count > 1) {
		for (i = 1; i< count; i++) {
			if (is_seqlower(numbers[i] , low) != 0) {
				low = numbers[i];
				lowtype = typenum[i];
			}
		}
	}
	type |= lowtype;
	
	if (typep != 0)
		*typep = type;
	return(0);
}

/* wakeup readers to upto the writer limits */
int
ksyn_wakeupreaders(ksyn_wait_queue_t kwq, uint32_t limitread, int allreaders,
		uint32_t updatebits, int *wokenp)
{
	ksyn_queue_t kq;
	int failedwakeup = 0;
	int numwoken = 0;
	kern_return_t kret = KERN_SUCCESS;
	uint32_t lbits = 0;
	
	lbits = updatebits;
	
	kq = &kwq->kw_ksynqueues[KSYN_QUEUE_READ];
	while ((kq->ksynq_count != 0) &&
			(allreaders || (is_seqlower(kq->ksynq_firstnum, limitread) != 0))) {
		kret = ksyn_signal(kwq, KSYN_QUEUE_READ, NULL, lbits);
		if (kret == KERN_NOT_WAITING) {
			failedwakeup++;
		}
		numwoken++;
	}
	
	if (wokenp != NULL)
		*wokenp = numwoken;
	return(failedwakeup);
}


/*
 * This handles the unlock grants for next set on rw_unlock() or on arrival
 * of all preposted waiters.
 */
int
kwq_handle_unlock(ksyn_wait_queue_t kwq, __unused uint32_t mgen, uint32_t rw_wc,
		uint32_t *updatep, int flags, int *blockp, uint32_t premgen)
{
	uint32_t low_writer, limitrdnum;
	int rwtype, error=0;
	int allreaders, nfailed;
	uint32_t updatebits=0, numneeded = 0;;
	int prepost = flags & KW_UNLOCK_PREPOST;
	thread_t preth = THREAD_NULL;
	ksyn_waitq_element_t kwe;
	uthread_t uth;
	thread_t th;
	int woken = 0;
	int block = 1;
	uint32_t lowest[KSYN_QUEUE_MAX]; /* np need for upgrade as it is handled separately */
	kern_return_t kret = KERN_SUCCESS;
	ksyn_queue_t kq;
	int curthreturns = 0;
	
	if (prepost != 0) {
		preth = current_thread();
	}
	
	kq = &kwq->kw_ksynqueues[KSYN_QUEUE_READ];
	kwq->kw_lastseqword = rw_wc;
	kwq->kw_lastunlockseq = (rw_wc & PTHRW_COUNT_MASK);
	kwq->kw_kflags &= ~KSYN_KWF_OVERLAP_GUARD;
	
	error = kwq_find_rw_lowest(kwq, flags, premgen, &rwtype, lowest);
#if __TESTPANICS__
	if (error != 0)
		panic("rwunlock: cannot fails to slot next round of threads");
#endif /* __TESTPANICS__ */
	
	low_writer = lowest[KSYN_QUEUE_WRITE];
	
	allreaders = 0;
	updatebits = 0;
	
	switch (rwtype & PTH_RW_TYPE_MASK) {
		case PTH_RW_TYPE_READ: {
			// XXX
			/* what about the preflight which is LREAD or READ ?? */
			if ((rwtype & PTH_RWSHFT_TYPE_MASK) != 0) {
				if (rwtype & PTH_RWSHFT_TYPE_WRITE) {
					updatebits |= (PTH_RWL_WBIT | PTH_RWL_KBIT);
				}
			}
			limitrdnum = 0;
			if ((rwtype & PTH_RWSHFT_TYPE_WRITE) != 0) {
				limitrdnum = low_writer;
			} else {
				allreaders = 1;
			}
			
			numneeded = 0;
			
			if ((rwtype & PTH_RWSHFT_TYPE_WRITE) != 0) {
				limitrdnum = low_writer;
				numneeded = ksyn_queue_count_tolowest(kq, limitrdnum);
				if (((flags & KW_UNLOCK_PREPOST_READLOCK) != 0) && (is_seqlower(premgen, limitrdnum) != 0)) {
					curthreturns = 1;
					numneeded += 1;
				}
			} else {
				// no writers at all
				// no other waiters only readers
				kwq->kw_kflags |= KSYN_KWF_OVERLAP_GUARD;
				numneeded += kwq->kw_ksynqueues[KSYN_QUEUE_READ].ksynq_count;
				if ((flags & KW_UNLOCK_PREPOST_READLOCK) != 0) {
					curthreturns = 1;
					numneeded += 1;
				}
			}
			
			updatebits += (numneeded << PTHRW_COUNT_SHIFT);
			
			kwq->kw_nextseqword = (rw_wc & PTHRW_COUNT_MASK) + updatebits;
			
			if (curthreturns != 0) {
				block = 0;
				uth = current_uthread();
				kwe = pthread_kern->uthread_get_uukwe(uth);
				kwe->kwe_psynchretval = updatebits;
			}
			
			
			nfailed = ksyn_wakeupreaders(kwq, limitrdnum, allreaders,
					updatebits, &woken);
			if (nfailed != 0) {
				_kwq_mark_interruped_wakeup(kwq, KWQ_INTR_READ, nfailed,
						limitrdnum, updatebits);
			}
			
			error = 0;
			
			if ((kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_count != 0) && 
					((updatebits & PTH_RWL_WBIT) == 0)) {
				panic("kwq_handle_unlock: writer pending but no writebit set %x\n", updatebits);
			}
		}
			break;
			
		case PTH_RW_TYPE_WRITE: {
			
			/* only one thread is goin to be granted */
			updatebits |= (PTHRW_INC);
			updatebits |= PTH_RWL_KBIT| PTH_RWL_EBIT;
			
			if (((flags & KW_UNLOCK_PREPOST_WRLOCK) != 0) && (low_writer == premgen)) {
				block = 0;
				if (kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_count != 0) {
					updatebits |= PTH_RWL_WBIT;
				}
				th = preth;
				uth = pthread_kern->get_bsdthread_info(th);
				kwe = pthread_kern->uthread_get_uukwe(uth);
				kwe->kwe_psynchretval = updatebits;
			} else {
				/* we are not granting writelock to the preposting thread */
				/* if there are writers present or the preposting write thread then W bit is to be set */
				if (kwq->kw_ksynqueues[KSYN_QUEUE_WRITE].ksynq_count > 1 ||
				    (flags & KW_UNLOCK_PREPOST_WRLOCK) != 0) {
					updatebits |= PTH_RWL_WBIT;
				}
				/* setup next in the queue */
				kret = ksyn_signal(kwq, KSYN_QUEUE_WRITE, NULL, updatebits);
				if (kret == KERN_NOT_WAITING) {
					_kwq_mark_interruped_wakeup(kwq, KWQ_INTR_WRITE, 1,
							low_writer, updatebits);
				}
				error = 0;
			}
			kwq->kw_nextseqword = (rw_wc & PTHRW_COUNT_MASK) + updatebits;
			if ((updatebits & (PTH_RWL_KBIT | PTH_RWL_EBIT)) != 
					(PTH_RWL_KBIT | PTH_RWL_EBIT)) {
				panic("kwq_handle_unlock: writer lock granted but no ke set %x\n", updatebits);
			}
		}
			break;
			
		default:
			panic("rwunlock: invalid type for lock grants");
			
	};
	
	if (updatep != NULL)
		*updatep = updatebits;
	if (blockp != NULL)
		*blockp = block;
	return(error);
}

/************* Indiv queue support routines ************************/
void
ksyn_queue_init(ksyn_queue_t kq)
{
	TAILQ_INIT(&kq->ksynq_kwelist);
	kq->ksynq_count = 0;
	kq->ksynq_firstnum = 0;
	kq->ksynq_lastnum = 0;
}

int
ksyn_queue_insert(ksyn_wait_queue_t kwq, int kqi, ksyn_waitq_element_t kwe,
		uint32_t mgen, int fit)
{
	ksyn_queue_t kq = &kwq->kw_ksynqueues[kqi];
	uint32_t lockseq = mgen & PTHRW_COUNT_MASK;
	int res = 0;

	if (kwe->kwe_kwqqueue != NULL) {
		panic("adding enqueued item to another queue");
	}

	if (kq->ksynq_count == 0) {
		TAILQ_INSERT_HEAD(&kq->ksynq_kwelist, kwe, kwe_list);
		kq->ksynq_firstnum = lockseq;
		kq->ksynq_lastnum = lockseq;
	} else if (fit == FIRSTFIT) {
		/* TBD: if retry bit is set for mutex, add it to the head */
		/* firstfit, arriving order */
		TAILQ_INSERT_TAIL(&kq->ksynq_kwelist, kwe, kwe_list);
		if (is_seqlower(lockseq, kq->ksynq_firstnum)) {
			kq->ksynq_firstnum = lockseq;
		}
		if (is_seqhigher(lockseq, kq->ksynq_lastnum)) {
			kq->ksynq_lastnum = lockseq;
		}
	} else if (lockseq == kq->ksynq_firstnum || lockseq == kq->ksynq_lastnum) {
		/* During prepost when a thread is getting cancelled, we could have
		 * two with same seq */
		res = EBUSY;
		if (kwe->kwe_state == KWE_THREAD_PREPOST) {
			ksyn_waitq_element_t tmp = ksyn_queue_find_seq(kwq, kq, lockseq);
			if (tmp != NULL && tmp->kwe_uth != NULL &&
					pthread_kern->uthread_is_cancelled(tmp->kwe_uth)) {
				TAILQ_INSERT_TAIL(&kq->ksynq_kwelist, kwe, kwe_list);
				res = 0;
			}
		}
	} else if (is_seqlower(kq->ksynq_lastnum, lockseq)) { // XXX is_seqhigher
		TAILQ_INSERT_TAIL(&kq->ksynq_kwelist, kwe, kwe_list);
		kq->ksynq_lastnum = lockseq;
	} else if (is_seqlower(lockseq, kq->ksynq_firstnum)) {
		TAILQ_INSERT_HEAD(&kq->ksynq_kwelist, kwe, kwe_list);
		kq->ksynq_firstnum = lockseq;
	} else {
		ksyn_waitq_element_t q_kwe, r_kwe;
		
		res = ESRCH;
		TAILQ_FOREACH_SAFE(q_kwe, &kq->ksynq_kwelist, kwe_list, r_kwe) {
			if (is_seqhigher(q_kwe->kwe_lockseq, lockseq)) {
				TAILQ_INSERT_BEFORE(q_kwe, kwe, kwe_list);
				res = 0;
				break;
			}
		}
	}
	
	if (res == 0) {
		kwe->kwe_kwqqueue = kwq;
		kq->ksynq_count++;
		kwq->kw_inqueue++;
		update_low_high(kwq, lockseq);
	}
	return res;
}

void
ksyn_queue_remove_item(ksyn_wait_queue_t kwq, ksyn_queue_t kq,
		ksyn_waitq_element_t kwe)
{
	if (kq->ksynq_count == 0) {
		panic("removing item from empty queue");
	}

	if (kwe->kwe_kwqqueue != kwq) {
		panic("removing item from wrong queue");
	}

	TAILQ_REMOVE(&kq->ksynq_kwelist, kwe, kwe_list);
	kwe->kwe_list.tqe_next = NULL;
	kwe->kwe_list.tqe_prev = NULL;
	kwe->kwe_kwqqueue = NULL;
	
	if (--kq->ksynq_count > 0) {
		ksyn_waitq_element_t tmp;
		tmp = TAILQ_FIRST(&kq->ksynq_kwelist);
		kq->ksynq_firstnum = tmp->kwe_lockseq & PTHRW_COUNT_MASK;
		tmp = TAILQ_LAST(&kq->ksynq_kwelist, ksynq_kwelist_head);
		kq->ksynq_lastnum = tmp->kwe_lockseq & PTHRW_COUNT_MASK;
	} else {
		kq->ksynq_firstnum = 0;
		kq->ksynq_lastnum = 0;
	}
	
	if (--kwq->kw_inqueue > 0) {
		uint32_t curseq = kwe->kwe_lockseq & PTHRW_COUNT_MASK;
		if (kwq->kw_lowseq == curseq) {
			kwq->kw_lowseq = find_nextlowseq(kwq);
		}
		if (kwq->kw_highseq == curseq) {
			kwq->kw_highseq = find_nexthighseq(kwq);
		}
	} else {
		kwq->kw_lowseq = 0;
		kwq->kw_highseq = 0;
	}
}

ksyn_waitq_element_t
ksyn_queue_find_seq(__unused ksyn_wait_queue_t kwq, ksyn_queue_t kq,
		uint32_t seq)
{
	ksyn_waitq_element_t kwe;
	
	// XXX: should stop searching when higher sequence number is seen
	TAILQ_FOREACH(kwe, &kq->ksynq_kwelist, kwe_list) {
		if ((kwe->kwe_lockseq & PTHRW_COUNT_MASK) == seq) {
			return kwe;
		}
	}
	return NULL;
}

/* find the thread at the target sequence (or a broadcast/prepost at or above) */
ksyn_waitq_element_t
ksyn_queue_find_cvpreposeq(ksyn_queue_t kq, uint32_t cgen)
{
	ksyn_waitq_element_t result = NULL;
	ksyn_waitq_element_t kwe;
	uint32_t lgen = (cgen & PTHRW_COUNT_MASK);
	
	TAILQ_FOREACH(kwe, &kq->ksynq_kwelist, kwe_list) {
		if (is_seqhigher_eq(kwe->kwe_lockseq, cgen)) {
			result = kwe;
			
			// KWE_THREAD_INWAIT must be strictly equal
			if (kwe->kwe_state == KWE_THREAD_INWAIT &&
					(kwe->kwe_lockseq & PTHRW_COUNT_MASK) != lgen) {
				result = NULL;
			}
			break;
		}
	}
	return result;
}

/* look for a thread at lockseq, a */
ksyn_waitq_element_t
ksyn_queue_find_signalseq(__unused ksyn_wait_queue_t kwq, ksyn_queue_t kq,
		uint32_t uptoseq, uint32_t signalseq)
{
	ksyn_waitq_element_t result = NULL;
	ksyn_waitq_element_t q_kwe, r_kwe;
	
	// XXX
	/* case where wrap in the tail of the queue exists */
	TAILQ_FOREACH_SAFE(q_kwe, &kq->ksynq_kwelist, kwe_list, r_kwe) {
		if (q_kwe->kwe_state == KWE_THREAD_PREPOST) {
			if (is_seqhigher(q_kwe->kwe_lockseq, uptoseq)) {
				return result;
			}
		}
		if (q_kwe->kwe_state == KWE_THREAD_PREPOST |
				q_kwe->kwe_state == KWE_THREAD_BROADCAST) {
			/* match any prepost at our same uptoseq or any broadcast above */
			if (is_seqlower(q_kwe->kwe_lockseq, uptoseq)) {
				continue;
			}
			return q_kwe;
		} else if (q_kwe->kwe_state == KWE_THREAD_INWAIT) {
			/*
			 * Match any (non-cancelled) thread at or below our upto sequence -
			 * but prefer an exact match to our signal sequence (if present) to
			 * keep exact matches happening.
			 */
			if (is_seqhigher(q_kwe->kwe_lockseq, uptoseq)) {
				return result;
			}
			if (q_kwe->kwe_kwqqueue == kwq) {
				if (!pthread_kern->uthread_is_cancelled(q_kwe->kwe_uth)) {
					/* if equal or higher than our signal sequence, return this one */
					if (is_seqhigher_eq(q_kwe->kwe_lockseq, signalseq)) {
						return q_kwe;
					}
					
					/* otherwise, just remember this eligible thread and move on */
					if (result == NULL) {
						result = q_kwe;
					}
				}
			}
		} else {
			panic("ksyn_queue_find_signalseq(): unknown wait queue element type (%d)\n", q_kwe->kwe_state);
		}
	}
	return result;
}

void
ksyn_queue_free_items(ksyn_wait_queue_t kwq, int kqi, uint32_t upto, int all)
{
	ksyn_waitq_element_t kwe;
	uint32_t tseq = upto & PTHRW_COUNT_MASK;
	ksyn_queue_t kq = &kwq->kw_ksynqueues[kqi];
	uint32_t freed = 0, signaled = 0;

	PTHREAD_TRACE(psynch_cvar_freeitems | DBG_FUNC_START, kwq->kw_addr,
			kqi, upto, all);
	
	while ((kwe = TAILQ_FIRST(&kq->ksynq_kwelist)) != NULL) {
		if (all == 0 && is_seqhigher(kwe->kwe_lockseq, tseq)) {
			break;
		}
		if (kwe->kwe_state == KWE_THREAD_INWAIT) {
			/*
			 * This scenario is typically noticed when the cvar is
			 * reinited and the new waiters are waiting. We can
			 * return them as spurious wait so the cvar state gets
			 * reset correctly.
			 */

			PTHREAD_TRACE(psynch_cvar_freeitems, kwq->kw_addr, kwe,
					kwq->kw_inqueue, 1);
			
			/* skip canceled ones */
			/* wake the rest */
			/* set M bit to indicate to waking CV to retun Inc val */
			(void)ksyn_signal(kwq, kqi, kwe,
					PTHRW_INC | PTH_RWS_CV_MBIT | PTH_RWL_MTX_WAIT);
			signaled++;
		} else {
			PTHREAD_TRACE(psynch_cvar_freeitems, kwq->kw_addr, kwe,
					kwq->kw_inqueue, 2);
			ksyn_queue_remove_item(kwq, kq, kwe);
			zfree(kwe_zone, kwe);
			kwq->kw_fakecount--;
			freed++;
		}
	}

	PTHREAD_TRACE(psynch_cvar_freeitems | DBG_FUNC_END, kwq->kw_addr, freed,
			signaled, kwq->kw_inqueue);
}

/*************************************************************************/

void
update_low_high(ksyn_wait_queue_t kwq, uint32_t lockseq)
{
	if (kwq->kw_inqueue == 1) {
		kwq->kw_lowseq = lockseq;
		kwq->kw_highseq = lockseq;
	} else {
		if (is_seqlower(lockseq, kwq->kw_lowseq)) {
			kwq->kw_lowseq = lockseq;
		}
		if (is_seqhigher(lockseq, kwq->kw_highseq)) {
			kwq->kw_highseq = lockseq;
		}
	}
}

uint32_t
find_nextlowseq(ksyn_wait_queue_t kwq)
{
	uint32_t lowest = 0;
	int first = 1;
	int i;
	
	for (i = 0; i < KSYN_QUEUE_MAX; i++) {
		if (kwq->kw_ksynqueues[i].ksynq_count > 0) {
			uint32_t current = kwq->kw_ksynqueues[i].ksynq_firstnum;
			if (first || is_seqlower(current, lowest)) {
				lowest = current;
				first = 0;
			}
		}
	}
	
	return lowest;
}

uint32_t
find_nexthighseq(ksyn_wait_queue_t kwq)
{
	uint32_t highest = 0;
	int first = 1;
	int i;
	
	for (i = 0; i < KSYN_QUEUE_MAX; i++) {
		if (kwq->kw_ksynqueues[i].ksynq_count > 0) {
			uint32_t current = kwq->kw_ksynqueues[i].ksynq_lastnum;
			if (first || is_seqhigher(current, highest)) {
				highest = current;
				first = 0;
			}
		}
	}
	
	return highest;
}

int
find_seq_till(ksyn_wait_queue_t kwq, uint32_t upto, uint32_t nwaiters,
		uint32_t *countp)
{
	int i;
	uint32_t count = 0;
	
	for (i = 0; i< KSYN_QUEUE_MAX; i++) {
		count += ksyn_queue_count_tolowest(&kwq->kw_ksynqueues[i], upto);
		if (count >= nwaiters) {
			break;
		}
	}
	
	if (countp != NULL) {
		*countp = count;
	}
	
	if (count == 0) {
		return 0;
	} else if (count >= nwaiters) {
		return 1;
	} else {
		return 0;
	}
}


uint32_t
ksyn_queue_count_tolowest(ksyn_queue_t kq, uint32_t upto)
{
	uint32_t i = 0;
	ksyn_waitq_element_t kwe, newkwe;
	
	if (kq->ksynq_count == 0 || is_seqhigher(kq->ksynq_firstnum, upto)) {
		return 0;
	}
	if (upto == kq->ksynq_firstnum) {
		return 1;
	}
	TAILQ_FOREACH_SAFE(kwe, &kq->ksynq_kwelist, kwe_list, newkwe) {
		uint32_t curval = (kwe->kwe_lockseq & PTHRW_COUNT_MASK);
		if (is_seqhigher(curval, upto)) {
			break;
		}
		++i;
		if (upto == curval) {
			break;
		}
	}
	return i;
}

/* handles the cond broadcast of cvar and returns number of woken threads and bits for syscall return */
void
ksyn_handle_cvbroad(ksyn_wait_queue_t ckwq, uint32_t upto, uint32_t *updatep)
{
	ksyn_waitq_element_t kwe, newkwe;
	uint32_t updatebits = 0;
	ksyn_queue_t kq = &ckwq->kw_ksynqueues[KSYN_QUEUE_WRITE];
	
	struct ksyn_queue kfreeq;
	ksyn_queue_init(&kfreeq);

	PTHREAD_TRACE(psynch_cvar_broadcast | DBG_FUNC_START, ckwq->kw_addr, upto,
			ckwq->kw_inqueue, 0);
	
retry:
	TAILQ_FOREACH_SAFE(kwe, &kq->ksynq_kwelist, kwe_list, newkwe) {
		if (is_seqhigher(kwe->kwe_lockseq, upto)) {
			// outside our range
			break;
		}

		if (kwe->kwe_state == KWE_THREAD_INWAIT) {
			// Wake only non-canceled threads waiting on this CV.
			if (!pthread_kern->uthread_is_cancelled(kwe->kwe_uth)) {
				PTHREAD_TRACE(psynch_cvar_broadcast, ckwq->kw_addr, kwe, 0, 1);
				(void)ksyn_signal(ckwq, KSYN_QUEUE_WRITE, kwe, PTH_RWL_MTX_WAIT);
				updatebits += PTHRW_INC;
			}
		} else if (kwe->kwe_state == KWE_THREAD_BROADCAST ||
			   kwe->kwe_state == KWE_THREAD_PREPOST) {
			PTHREAD_TRACE(psynch_cvar_broadcast, ckwq->kw_addr, kwe,
					kwe->kwe_state, 2);
			ksyn_queue_remove_item(ckwq, kq, kwe);
			TAILQ_INSERT_TAIL(&kfreeq.ksynq_kwelist, kwe, kwe_list);
			ckwq->kw_fakecount--;
		} else {
			panic("unknown kwe state\n");
		}
	}
	
	/* Need to enter a broadcast in the queue (if not already at L == S) */
	
	if (diff_genseq(ckwq->kw_lword, ckwq->kw_sword)) {
		PTHREAD_TRACE(psynch_cvar_broadcast, ckwq->kw_addr, ckwq->kw_lword,
				ckwq->kw_sword, 3);

		newkwe = TAILQ_FIRST(&kfreeq.ksynq_kwelist);
		if (newkwe == NULL) {
			ksyn_wqunlock(ckwq);
			newkwe = (ksyn_waitq_element_t)zalloc(kwe_zone);
			TAILQ_INSERT_TAIL(&kfreeq.ksynq_kwelist, newkwe, kwe_list);
			ksyn_wqlock(ckwq);
			goto retry;
		} else {
			TAILQ_REMOVE(&kfreeq.ksynq_kwelist, newkwe, kwe_list);
			ksyn_prepost(ckwq, newkwe, KWE_THREAD_BROADCAST, upto);
			PTHREAD_TRACE(psynch_cvar_broadcast, ckwq->kw_addr, newkwe, 0, 4);
		}
	}
	
	// free up any remaining things stumbled across above
	while ((kwe = TAILQ_FIRST(&kfreeq.ksynq_kwelist)) != NULL) {
		TAILQ_REMOVE(&kfreeq.ksynq_kwelist, kwe, kwe_list);
		zfree(kwe_zone, kwe);
	}

	PTHREAD_TRACE(psynch_cvar_broadcast | DBG_FUNC_END, ckwq->kw_addr,
			updatebits, 0, 0);
	
	if (updatep != NULL) {
		*updatep |= updatebits;
	}
}

void
ksyn_cvupdate_fixup(ksyn_wait_queue_t ckwq, uint32_t *updatebits)
{
	if ((ckwq->kw_lword & PTHRW_COUNT_MASK) == (ckwq->kw_sword & PTHRW_COUNT_MASK)) {
		if (ckwq->kw_inqueue != 0) {
			/* FREE THE QUEUE */
			ksyn_queue_free_items(ckwq, KSYN_QUEUE_WRITE, ckwq->kw_lword, 0);
#if __TESTPANICS__
			if (ckwq->kw_inqueue != 0)
				panic("ksyn_cvupdate_fixup: L == S, but entries in queue beyond S");
#endif /* __TESTPANICS__ */
		}
		ckwq->kw_lword = ckwq->kw_uword = ckwq->kw_sword = 0;
		ckwq->kw_kflags |= KSYN_KWF_ZEROEDOUT;
		*updatebits |= PTH_RWS_CV_CBIT;
	} else if (ckwq->kw_inqueue != 0 && ckwq->kw_fakecount == ckwq->kw_inqueue) {
		// only fake entries are present in the queue
		*updatebits |= PTH_RWS_CV_PBIT;
	}
}

void
psynch_zoneinit(void)
{
	kwq_zone = zinit(sizeof(struct ksyn_wait_queue),
			8192 * sizeof(struct ksyn_wait_queue), 4096, "ksyn_wait_queue");
	kwe_zone = zinit(sizeof(struct ksyn_waitq_element),
			8192 * sizeof(struct ksyn_waitq_element), 4096, "ksyn_waitq_element");
}

void *
_pthread_get_thread_kwq(thread_t thread)
{
	assert(thread);
	struct uthread * uthread = pthread_kern->get_bsdthread_info(thread);
	assert(uthread);
	ksyn_waitq_element_t kwe = pthread_kern->uthread_get_uukwe(uthread);
	assert(kwe);
	ksyn_wait_queue_t kwq = kwe->kwe_kwqqueue;
	return kwq;
}

/* This function is used by stackshot to determine why a thread is blocked, and report
 * who owns the object that the thread is blocked on. It should *only* be called if the
 * `block_hint' field in the relevant thread's struct is populated with something related
 * to pthread sync objects.
 */
void
_pthread_find_owner(thread_t thread,
		struct stackshot_thread_waitinfo * waitinfo)
{
	ksyn_wait_queue_t kwq = _pthread_get_thread_kwq(thread);
	switch (waitinfo->wait_type) {
		case kThreadWaitPThreadMutex:
			assert((kwq->kw_type & KSYN_WQTYPE_MASK) == KSYN_WQTYPE_MTX);
			waitinfo->owner   = thread_tid(kwq->kw_owner);
			waitinfo->context = kwq->kw_addr;
			break;
		/* Owner of rwlock not stored in kernel space due to races. Punt
		 * and hope that the userspace address is helpful enough. */
		case kThreadWaitPThreadRWLockRead:
		case kThreadWaitPThreadRWLockWrite:
			assert((kwq->kw_type & KSYN_WQTYPE_MASK) == KSYN_WQTYPE_RWLOCK);
			waitinfo->owner   = 0;
			waitinfo->context = kwq->kw_addr;
			break;
		/* Condvars don't have owners, so just give the userspace address. */
		case kThreadWaitPThreadCondVar:
			assert((kwq->kw_type & KSYN_WQTYPE_MASK) == KSYN_WQTYPE_CVAR);
			waitinfo->owner   = 0;
			waitinfo->context = kwq->kw_addr;
			break;
		case kThreadWaitNone:
		default:
			waitinfo->owner = 0;
			waitinfo->context = 0;
			break;
	}
}
