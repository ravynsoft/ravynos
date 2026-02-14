/*
 * Copyright (c) 2000-2003, 2007, 2008 Apple Inc. All rights reserved.
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
/*
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991
 *              All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * MkLinux
 */

/*
 * POSIX Pthread Library
 */

#include "resolver.h"
#include "internal.h"
#include <sys/time.h>	      /* For struct timespec and getclock(). */

#ifdef PLOCKSTAT
#include "plockstat.h"
#else /* !PLOCKSTAT */
#define PLOCKSTAT_MUTEX_RELEASE(x, y)
#endif /* PLOCKSTAT */

extern int __gettimeofday(struct timeval *, struct timezone *);

PTHREAD_NOEXPORT
int _pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex,
		const struct timespec *abstime, int isRelative, int isconforming);

PTHREAD_ALWAYS_INLINE
static inline void
COND_GETSEQ_ADDR(_pthread_cond *cond,
		volatile uint64_t **c_lsseqaddr,
		volatile uint32_t **c_lseqcnt,
		volatile uint32_t **c_useqcnt,
		volatile uint32_t **c_sseqcnt)
{
	if (cond->misalign) {
		*c_lseqcnt = &cond->c_seq[1];
		*c_sseqcnt = &cond->c_seq[2];
		*c_useqcnt = &cond->c_seq[0];
	} else {
		*c_lseqcnt = &cond->c_seq[0];
		*c_sseqcnt = &cond->c_seq[1];
		*c_useqcnt = &cond->c_seq[2];
	}
	*c_lsseqaddr = (volatile uint64_t *)*c_lseqcnt;
}

#ifndef BUILDING_VARIANT /* [ */

static void _pthread_cond_cleanup(void *arg);
static void _pthread_cond_updateval(_pthread_cond *cond, _pthread_mutex *mutex,
		int error, uint32_t updateval);


int
pthread_condattr_init(pthread_condattr_t *attr)
{
	attr->sig = _PTHREAD_COND_ATTR_SIG;
	attr->pshared = _PTHREAD_DEFAULT_PSHARED;
	return 0;
}

int
pthread_condattr_destroy(pthread_condattr_t *attr)
{
	attr->sig = _PTHREAD_NO_SIG;
	return 0;
}

int
pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_COND_ATTR_SIG) {
		*pshared = (int)attr->pshared;
		res = 0;
	}
	return res;
}

int
pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_COND_ATTR_SIG) {
#if __DARWIN_UNIX03
		if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED)
#else /* __DARWIN_UNIX03 */
		if (pshared == PTHREAD_PROCESS_PRIVATE)
#endif /* __DARWIN_UNIX03 */
		{
			attr->pshared = pshared;
			res = 0;
		}
	}
	return res;
}

int
pthread_cond_timedwait_relative_np(pthread_cond_t *cond, pthread_mutex_t *mutex,
		const struct timespec *abstime)
{
	return _pthread_cond_wait(cond, mutex, abstime, 1, 0);
}

#endif /* !BUILDING_VARIANT ] */

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_cond_init(_pthread_cond *cond, const pthread_condattr_t *attr, int conforming)
{
	volatile uint64_t *c_lsseqaddr;
	volatile uint32_t *c_lseqcnt, *c_useqcnt, *c_sseqcnt;

	cond->busy = NULL;
	cond->c_seq[0] = 0;
	cond->c_seq[1] = 0;
	cond->c_seq[2] = 0;
	cond->unused = 0;

	cond->misalign = (((uintptr_t)&cond->c_seq[0]) & 0x7) != 0;
	COND_GETSEQ_ADDR(cond, &c_lsseqaddr, &c_lseqcnt, &c_useqcnt, &c_sseqcnt);
	*c_sseqcnt = PTH_RWS_CV_CBIT; // set Sword to 0c

	if (conforming) {
		if (attr) {
			cond->pshared = attr->pshared;
		} else {
			cond->pshared = _PTHREAD_DEFAULT_PSHARED;
		}
	} else {
		cond->pshared = _PTHREAD_DEFAULT_PSHARED;
	}

	long sig = _PTHREAD_COND_SIG;

	// Ensure all contents are properly set before setting signature.
#if defined(__LP64__)
	// For binary compatibility reasons we cannot require natural alignment of
	// the 64bit 'sig' long value in the struct. rdar://problem/21610439
	uint32_t *sig32_ptr = (uint32_t*)&cond->sig;
	uint32_t *sig32_val = (uint32_t*)&sig;
	*(sig32_ptr + 1) = *(sig32_val + 1);
	os_atomic_store(sig32_ptr, *sig32_val, release);
#else
	os_atomic_store2o(cond, sig, sig, release);
#endif

	return 0;
}

#ifndef BUILDING_VARIANT /* [ */

PTHREAD_NOINLINE
static int
_pthread_cond_check_init_slow(_pthread_cond *cond, bool *inited)
{
	int res = EINVAL;
	if (cond->sig == _PTHREAD_COND_SIG_init) {
		_PTHREAD_LOCK(cond->lock);
		if (cond->sig == _PTHREAD_COND_SIG_init) {
			res = _pthread_cond_init(cond, NULL, 0);
			if (inited) {
				*inited = true;
			}
		} else if (cond->sig == _PTHREAD_COND_SIG) {
			res = 0;
		}
		_PTHREAD_UNLOCK(cond->lock);
	} else if (cond->sig == _PTHREAD_COND_SIG) {
		res = 0;
	}
	return res;
}

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_cond_check_init(_pthread_cond *cond, bool *inited)
{
	int res = 0;
	if (cond->sig != _PTHREAD_COND_SIG) {
		return _pthread_cond_check_init_slow(cond, inited);
	}
	return res;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_cond_destroy(pthread_cond_t *ocond)
{
	_pthread_cond *cond = (_pthread_cond *)ocond;
	int res = EINVAL;
	if (cond->sig == _PTHREAD_COND_SIG) {
		_PTHREAD_LOCK(cond->lock);

		uint64_t oldval64, newval64;
		uint32_t lcntval, ucntval, scntval;
		volatile uint64_t *c_lsseqaddr;
		volatile uint32_t *c_lseqcnt, *c_useqcnt, *c_sseqcnt;

		COND_GETSEQ_ADDR(cond, &c_lsseqaddr, &c_lseqcnt, &c_useqcnt, &c_sseqcnt);

		do {
			lcntval = *c_lseqcnt;
			ucntval = *c_useqcnt;
			scntval = *c_sseqcnt;

			// validate it is not busy
			if ((lcntval & PTHRW_COUNT_MASK) != (scntval & PTHRW_COUNT_MASK)) {
				//res = EBUSY;
				break;
			}
			oldval64 = (((uint64_t)scntval) << 32);
			oldval64 |= lcntval;
			newval64 = oldval64;
		} while (!os_atomic_cmpxchg(c_lsseqaddr, oldval64, newval64, seq_cst));

		// <rdar://problem/13782056> Need to clear preposts.
		uint32_t flags = 0;
		bool needclearpre = ((scntval & PTH_RWS_CV_PBIT) != 0);
		if (needclearpre && cond->pshared == PTHREAD_PROCESS_SHARED) {
			flags |= _PTHREAD_MTX_OPT_PSHARED;
		}

		cond->sig = _PTHREAD_NO_SIG;
		res = 0;

		_PTHREAD_UNLOCK(cond->lock);

		if (needclearpre) {
			(void)__psynch_cvclrprepost(cond, lcntval, ucntval, scntval, 0, lcntval, flags);
		}
	} else if (cond->sig == _PTHREAD_COND_SIG_init) {
		// Compatibility for misbehaving applications that attempt to
		// destroy a statically initialized condition variable.
		cond->sig = _PTHREAD_NO_SIG;
		res = 0;
	}
	return res;
}

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_cond_signal(pthread_cond_t *ocond, bool broadcast, mach_port_t thread)
{
	int res;
	_pthread_cond *cond = (_pthread_cond *)ocond;

	uint32_t updateval;
	uint32_t diffgen;
	uint32_t ulval;

	uint64_t oldval64, newval64;
	uint32_t lcntval, ucntval, scntval;
	volatile uint64_t *c_lsseqaddr;
	volatile uint32_t *c_lseqcnt, *c_useqcnt, *c_sseqcnt;

	int retry_count = 0, uretry_count = 0;
	int ucountreset = 0;

	bool inited = false;
	res = _pthread_cond_check_init(cond, &inited);
	if (res != 0 || inited == true) {
		return res;
	}

	COND_GETSEQ_ADDR(cond, &c_lsseqaddr, &c_lseqcnt, &c_useqcnt, &c_sseqcnt);

	bool retry;
	do {
		retry = false;

		lcntval = *c_lseqcnt;
		ucntval = *c_useqcnt;
		scntval = *c_sseqcnt;
		diffgen = 0;
		ulval = 0;

		if (((lcntval & PTHRW_COUNT_MASK) == (scntval & PTHRW_COUNT_MASK)) ||
		    (thread == MACH_PORT_NULL && ((lcntval & PTHRW_COUNT_MASK) == (ucntval & PTHRW_COUNT_MASK)))) {
			/* validate it is spurious and return */
			oldval64 = (((uint64_t)scntval) << 32);
			oldval64 |= lcntval;
			newval64 = oldval64;

			if (!os_atomic_cmpxchg(c_lsseqaddr, oldval64, newval64, seq_cst)) {
				retry = true;
				continue;
			} else {
				return 0;
			}
		}

		if (thread) {
			break;
		}

		/* validate to eliminate spurious values, race snapshots */
		if (is_seqhigher((scntval & PTHRW_COUNT_MASK), (lcntval & PTHRW_COUNT_MASK))) {
			/* since ucntval may be newer, just redo */
			retry_count++;
			if (retry_count > 8192) {
				return EAGAIN;
			} else {
				sched_yield();
				retry = true;
				continue;
			}
		} else if (is_seqhigher((ucntval & PTHRW_COUNT_MASK), (lcntval & PTHRW_COUNT_MASK))) {
			/* since ucntval may be newer, just redo */
			uretry_count++;
			if (uretry_count > 8192) {
				/*
				 * U value if not used for a while can go out of sync
				 * set this to S value and try one more time.
				 */
				if (ucountreset != 0) {
					return EAGAIN;
				} else if (os_atomic_cmpxchg(c_useqcnt, ucntval, (scntval & PTHRW_COUNT_MASK), seq_cst)) {
					/* now the U is reset to S value */
					ucountreset = 1;
					uretry_count = 0;
				}
			}
			sched_yield();
			retry = true;
			continue;
		}

		if (is_seqlower(ucntval & PTHRW_COUNT_MASK, scntval & PTHRW_COUNT_MASK) != 0) {
			/* If U < S, set U = S+diff due to intr's TO, etc */
			ulval = (scntval & PTHRW_COUNT_MASK);
		} else {
			/* If U >= S, set U = U+diff due to intr's TO, etc */
			ulval = (ucntval & PTHRW_COUNT_MASK);
		}

		if (broadcast) {
			diffgen = diff_genseq(lcntval, ulval);
			// Set U = L
			ulval = (lcntval & PTHRW_COUNT_MASK);
		} else {
			ulval += PTHRW_INC;
		}

	} while (retry || !os_atomic_cmpxchg(c_useqcnt, ucntval, ulval, seq_cst));

	uint32_t flags = 0;
	if (cond->pshared == PTHREAD_PROCESS_SHARED) {
		flags |= _PTHREAD_MTX_OPT_PSHARED;
	}

	uint64_t cvlsgen = ((uint64_t)scntval << 32) | lcntval;

	if (broadcast) {
		// pass old U val so kernel will know the diffgen
		uint64_t cvudgen = ((uint64_t)ucntval << 32) | diffgen;
		updateval = __psynch_cvbroad(ocond, cvlsgen, cvudgen, flags, NULL, 0, 0);
	} else {
		updateval = __psynch_cvsignal(ocond, cvlsgen, ucntval, thread, NULL, 0, 0, flags);
	}

	if (updateval != (uint32_t)-1 && updateval != 0) {
		_pthread_cond_updateval(cond, NULL, 0, updateval);
	}

	return 0;
}

/*
 * Signal a condition variable, waking up all threads waiting for it.
 */
PTHREAD_NOEXPORT_VARIANT
int
pthread_cond_broadcast(pthread_cond_t *ocond)
{
	return _pthread_cond_signal(ocond, true, MACH_PORT_NULL);
}

/*
 * Signal a condition variable, waking a specified thread.
 */
PTHREAD_NOEXPORT_VARIANT
int
pthread_cond_signal_thread_np(pthread_cond_t *ocond, pthread_t thread)
{
	mach_port_t mp = MACH_PORT_NULL;
	if (thread) {
		mp = pthread_mach_thread_np((_Nonnull pthread_t)thread);
	}
	return _pthread_cond_signal(ocond, false, mp);
}

/*
 * Signal a condition variable, waking only one thread.
 */
PTHREAD_NOEXPORT_VARIANT
int
pthread_cond_signal(pthread_cond_t *ocond)
{
	return _pthread_cond_signal(ocond, false, MACH_PORT_NULL);
}

/*
 * Manage a list of condition variables associated with a mutex
 */

/*
 * Suspend waiting for a condition variable.
 * Note: we have to keep a list of condition variables which are using
 * this same mutex variable so we can detect invalid 'destroy' sequences.
 * If conformance is not cancelable, we skip the _pthread_testcancel(),
 * but keep the remaining conforming behavior..
 */
PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_cond_wait(pthread_cond_t *ocond,
			pthread_mutex_t *omutex,
			const struct timespec *abstime,
			int isRelative,
			int conforming)
{
	int res;
	_pthread_cond *cond = (_pthread_cond *)ocond;
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;
	struct timespec then = { 0, 0 };
	uint32_t mtxgen, mtxugen, flags=0, updateval;
	uint32_t lcntval, ucntval, scntval;
	uint32_t nlval, ulval, savebits;
	volatile uint64_t *c_lsseqaddr;
	volatile uint32_t *c_lseqcnt, *c_useqcnt, *c_sseqcnt;
	uint64_t oldval64, newval64, mugen, cvlsgen;
	uint32_t *npmtx = NULL;
	int timeout_elapsed = 0;

	res = _pthread_cond_check_init(cond, NULL);
	if (res != 0) {
		return res;
	}

	if (conforming) {
		if (!_pthread_mutex_check_signature(mutex) &&
				!_pthread_mutex_check_signature_init(mutex)) {
			return EINVAL;
		}
		if (conforming == PTHREAD_CONFORM_UNIX03_CANCELABLE) {
			_pthread_testcancel(conforming);
		}
	}

	/* send relative time to kernel */
	if (abstime) {
		if (abstime->tv_nsec < 0 || abstime->tv_nsec >= NSEC_PER_SEC) {
			return EINVAL;
		}

		if (isRelative == 0) {
			struct timespec now;
			struct timeval tv;
			__gettimeofday(&tv, NULL);
			TIMEVAL_TO_TIMESPEC(&tv, &now);

			if ((abstime->tv_sec == now.tv_sec) ?
				(abstime->tv_nsec <= now.tv_nsec) :
				(abstime->tv_sec < now.tv_sec)) {
				timeout_elapsed = 1;
			} else {
				/* Compute relative time to sleep */
				then.tv_nsec = abstime->tv_nsec - now.tv_nsec;
				then.tv_sec = abstime->tv_sec - now.tv_sec;
				if (then.tv_nsec < 0) {
					then.tv_nsec += NSEC_PER_SEC;
					then.tv_sec--;
				}
			}
		} else {
			then.tv_sec = abstime->tv_sec;
			then.tv_nsec = abstime->tv_nsec;
			if ((then.tv_sec == 0) && (then.tv_nsec == 0)) {
				timeout_elapsed = 1;
			}
		}
	}

	if (cond->busy != NULL && cond->busy != mutex) {
		return EINVAL;
	}

	/*
	 * If timeout is known to have elapsed, we still need to unlock and
	 * relock the mutex to allow other waiters to get in line and
	 * modify the condition state.
	 */
	 if (timeout_elapsed) {
		res = pthread_mutex_unlock(omutex);
		if (res != 0) {
			return res;
		}
		res = pthread_mutex_lock(omutex);
		if (res != 0) {
			return res;
		}
		return ETIMEDOUT;
	}

	COND_GETSEQ_ADDR(cond, &c_lsseqaddr, &c_lseqcnt, &c_useqcnt, &c_sseqcnt);

	do {
		lcntval = *c_lseqcnt;
		ucntval = *c_useqcnt;
		scntval = *c_sseqcnt;

		oldval64 = (((uint64_t)scntval) << 32);
		oldval64 |= lcntval;

		/* remove c and p bits on S word */
		savebits = scntval & PTH_RWS_CV_BITSALL;
		ulval = (scntval & PTHRW_COUNT_MASK);
		nlval = lcntval + PTHRW_INC;
		newval64 = (((uint64_t)ulval) << 32);
		newval64 |= nlval;
	} while (!os_atomic_cmpxchg(c_lsseqaddr, oldval64, newval64, seq_cst));

	cond->busy = mutex;

	res = _pthread_mutex_droplock(mutex, &flags, &npmtx, &mtxgen, &mtxugen);

	/* TBD: cases are for normal (non owner for recursive mutex; error checking)*/
	if (res != 0) {
		return EINVAL;
	}
	if ((flags & _PTHREAD_MTX_OPT_NOTIFY) == 0) {
		npmtx = NULL;
		mugen = 0;
	} else {
		mugen = ((uint64_t)mtxugen << 32) | mtxgen;
	}
	flags &= ~_PTHREAD_MTX_OPT_MUTEX;	/* reset the mutex bit as this is cvar */

	cvlsgen = ((uint64_t)(ulval | savebits)<< 32) | nlval;

	// SUSv3 requires pthread_cond_wait to be a cancellation point
	if (conforming) {
		pthread_cleanup_push(_pthread_cond_cleanup, (void *)cond);
		updateval = __psynch_cvwait(ocond, cvlsgen, ucntval, (pthread_mutex_t *)npmtx, mugen, flags, (int64_t)then.tv_sec, (int32_t)then.tv_nsec);
		_pthread_testcancel(conforming);
		pthread_cleanup_pop(0);
	} else {
		updateval = __psynch_cvwait(ocond, cvlsgen, ucntval, (pthread_mutex_t *)npmtx, mugen, flags, (int64_t)then.tv_sec, (int32_t)then.tv_nsec);
	}

	if (updateval == (uint32_t)-1) {
		int err = errno;
		switch (err & 0xff) {
			case ETIMEDOUT:
				res = ETIMEDOUT;
				break;
			case EINTR:
				// spurious wakeup (unless canceled)
				res = 0;
				break;
			default:
				res = EINVAL;
				break;
		}

		// add unlock ref to show one less waiter
		_pthread_cond_updateval(cond, mutex, err, 0);
	} else if (updateval != 0) {
		// Successful wait
		// The return due to prepost and might have bit states
		// update S and return for prepo if needed
		_pthread_cond_updateval(cond, mutex, 0, updateval);
	}

	pthread_mutex_lock(omutex);

	return res;
}

static void
_pthread_cond_cleanup(void *arg)
{
	_pthread_cond *cond = (_pthread_cond *)arg;
	pthread_t thread = pthread_self();
	pthread_mutex_t *mutex;

// 4597450: begin
	if (!thread->canceled) {
		return;
	}
// 4597450: end

	mutex = (pthread_mutex_t *)cond->busy;

	// add unlock ref to show one less waiter
	_pthread_cond_updateval(cond, (_pthread_mutex *)mutex,
			thread->cancel_error, 0);

	/*
	** Can't do anything if this fails -- we're on the way out
	*/
	if (mutex != NULL) {
		(void)pthread_mutex_lock(mutex);
	}
}

static void
_pthread_cond_updateval(_pthread_cond *cond, _pthread_mutex *mutex,
		int error, uint32_t updateval)
{
	int needclearpre;

	uint32_t diffgen, nsval;
	uint64_t oldval64, newval64;
	uint32_t lcntval, ucntval, scntval;
	volatile uint64_t *c_lsseqaddr;
	volatile uint32_t *c_lseqcnt, *c_useqcnt, *c_sseqcnt;

	if (error != 0) {
		updateval = PTHRW_INC;
		if (error & ECVCLEARED) {
			updateval |= PTH_RWS_CV_CBIT;
		}
		if (error & ECVPREPOST) {
			updateval |= PTH_RWS_CV_PBIT;
		}
	}

	COND_GETSEQ_ADDR(cond, &c_lsseqaddr, &c_lseqcnt, &c_useqcnt, &c_sseqcnt);

	do {
		lcntval = *c_lseqcnt;
		ucntval = *c_useqcnt;
		scntval = *c_sseqcnt;
		nsval = 0;
		needclearpre = 0;

		diffgen = diff_genseq(lcntval, scntval); // pending waiters

		oldval64 = (((uint64_t)scntval) << 32);
		oldval64 |= lcntval;

		PTHREAD_TRACE(psynch_cvar_updateval | DBG_FUNC_START, cond, oldval64,
				updateval, 0);

		if (diffgen <= 0 && !is_rws_pbit_set(updateval)) {
			/* TBD: Assert, should not be the case */
			/* validate it is spurious and return */
			newval64 = oldval64;
		} else {
			// update S by one

			// update scntval with number of expected returns and bits
			nsval = (scntval & PTHRW_COUNT_MASK) + (updateval & PTHRW_COUNT_MASK);
			// set bits
			nsval |= ((scntval & PTH_RWS_CV_BITSALL) | (updateval & PTH_RWS_CV_BITSALL));

			// if L==S and c&p bits are set, needs clearpre
			if (((nsval & PTHRW_COUNT_MASK) == (lcntval & PTHRW_COUNT_MASK)) &&
			    ((nsval & PTH_RWS_CV_BITSALL) == PTH_RWS_CV_BITSALL)) {
				// reset p bit but retain c bit on the sword
				nsval &= PTH_RWS_CV_RESET_PBIT;
				needclearpre = 1;
			}

			newval64 = (((uint64_t)nsval) << 32);
			newval64 |= lcntval;
		}
	} while (!os_atomic_cmpxchg(c_lsseqaddr, oldval64, newval64, seq_cst));

	PTHREAD_TRACE(psynch_cvar_updateval | DBG_FUNC_END, cond, newval64,
			(uint64_t)diffgen << 32 | needclearpre, 0);

	if (diffgen > 0) {
		// if L == S, then reset associated mutex
		if ((nsval & PTHRW_COUNT_MASK) == (lcntval & PTHRW_COUNT_MASK)) {
			cond->busy = NULL;
		}
	}

	if (needclearpre) {
		uint32_t flags = 0;
		if (cond->pshared == PTHREAD_PROCESS_SHARED) {
			flags |= _PTHREAD_MTX_OPT_PSHARED;
		}
		(void)__psynch_cvclrprepost(cond, lcntval, ucntval, nsval, 0, lcntval, flags);
	}
}

#endif /* !BUILDING_VARIANT ] */

PTHREAD_NOEXPORT_VARIANT
int
pthread_cond_init(pthread_cond_t *ocond, const pthread_condattr_t *attr)
{
	int conforming;

#if __DARWIN_UNIX03
	conforming = 1;
#else /* __DARWIN_UNIX03 */
	conforming = 0;
#endif /* __DARWIN_UNIX03 */

	_pthread_cond *cond = (_pthread_cond *)ocond;
	_PTHREAD_LOCK_INIT(cond->lock);
	return _pthread_cond_init(cond, attr, conforming);
}

