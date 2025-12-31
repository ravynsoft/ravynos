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
 *
 */
/*
 * MkLinux
 */

/*
 * POSIX Pthread Library
 * -- Mutex variable support
 */

#include "resolver.h"
#include "internal.h"
#include "kern/kern_trace.h"

#ifndef errno_t
#include <errno.h>
#endif

#ifndef BUILDING_VARIANT /* [ */

#ifdef PLOCKSTAT
#include "plockstat.h"
/* This function is never called and exists to provide never-fired dtrace
 * probes so that user d scripts don't get errors.
 */
PTHREAD_NOEXPORT PTHREAD_USED
void
_plockstat_never_fired(void)
{
	PLOCKSTAT_MUTEX_SPIN(NULL);
	PLOCKSTAT_MUTEX_SPUN(NULL, 0, 0);
}
#else /* !PLOCKSTAT */
#define	PLOCKSTAT_MUTEX_SPIN(x)
#define	PLOCKSTAT_MUTEX_SPUN(x, y, z)
#define	PLOCKSTAT_MUTEX_ERROR(x, y)
#define	PLOCKSTAT_MUTEX_BLOCK(x)
#define	PLOCKSTAT_MUTEX_BLOCKED(x, y)
#define	PLOCKSTAT_MUTEX_ACQUIRE(x, y, z)
#define	PLOCKSTAT_MUTEX_RELEASE(x, y)
#endif /* PLOCKSTAT */

#define BLOCK_FAIL_PLOCKSTAT    0
#define BLOCK_SUCCESS_PLOCKSTAT 1

#define PTHREAD_MUTEX_INIT_UNUSED 1

PTHREAD_NOEXPORT PTHREAD_WEAK
int _pthread_mutex_lock_init_slow(_pthread_mutex *mutex, bool trylock);

PTHREAD_NOEXPORT PTHREAD_WEAK // prevent inlining of return value into callers
int _pthread_mutex_fairshare_lock_slow(_pthread_mutex *mutex, bool trylock);

PTHREAD_NOEXPORT PTHREAD_WEAK // prevent inlining of return value into callers
int _pthread_mutex_firstfit_lock_slow(_pthread_mutex *mutex, bool trylock);

PTHREAD_NOEXPORT PTHREAD_WEAK // prevent inlining of return value into callers
int _pthread_mutex_fairshare_unlock_slow(_pthread_mutex *mutex);

PTHREAD_NOEXPORT PTHREAD_WEAK // prevent inlining of return value into callers
int _pthread_mutex_firstfit_unlock_slow(_pthread_mutex *mutex);

PTHREAD_NOEXPORT PTHREAD_WEAK // prevent inlining of return value into callers
int _pthread_mutex_corruption_abort(_pthread_mutex *mutex);

extern int __pthread_mutex_default_opt_policy PTHREAD_NOEXPORT;


int __pthread_mutex_default_opt_policy PTHREAD_NOEXPORT =
		_PTHREAD_MTX_OPT_POLICY_DEFAULT;

static inline bool
_pthread_mutex_policy_validate(int policy)
{
	return (policy >= 0 && policy < _PTHREAD_MUTEX_POLICY_LAST);
}

static inline int
_pthread_mutex_policy_to_opt(int policy)
{
	switch (policy) {
	case PTHREAD_MUTEX_POLICY_FAIRSHARE_NP:
		return _PTHREAD_MTX_OPT_POLICY_FAIRSHARE;
	case PTHREAD_MUTEX_POLICY_FIRSTFIT_NP:
		return _PTHREAD_MTX_OPT_POLICY_FIRSTFIT;
	default:
		__builtin_unreachable();
	}
}

PTHREAD_NOEXPORT
void
_pthread_mutex_global_init(const char *envp[],
		struct _pthread_registration_data *registration_data)
{
	int opt = _PTHREAD_MTX_OPT_POLICY_DEFAULT;
	if (registration_data->mutex_default_policy) {
		int policy = registration_data->mutex_default_policy;
		if (_pthread_mutex_policy_validate(policy)) {
			opt = _pthread_mutex_policy_to_opt(policy);
		}
	}

	const char *envvar = _simple_getenv(envp, "PTHREAD_MUTEX_DEFAULT_POLICY");
	if (envvar) {
		int policy = envvar[0] - '0';
		if (_pthread_mutex_policy_validate(policy)) {
			opt = _pthread_mutex_policy_to_opt(policy);
		}
	}

	if (opt != __pthread_mutex_default_opt_policy) {
		__pthread_mutex_default_opt_policy = opt;
	}
}



PTHREAD_ALWAYS_INLINE
static inline int _pthread_mutex_init(_pthread_mutex *mutex,
		const pthread_mutexattr_t *attr, uint32_t static_type);

typedef union mutex_seq {
	uint32_t seq[2];
	struct { uint32_t lgenval; uint32_t ugenval; };
	struct { uint32_t mgen; uint32_t ugen; };
	uint64_t seq_LU;
	uint64_t _Atomic atomic_seq_LU;
} mutex_seq;

_Static_assert(sizeof(mutex_seq) == 2 * sizeof(uint32_t),
		"Incorrect mutex_seq size");

#if !__LITTLE_ENDIAN__
#error MUTEX_GETSEQ_ADDR assumes little endian layout of 2 32-bit sequence words
#endif

PTHREAD_ALWAYS_INLINE
static inline void
MUTEX_GETSEQ_ADDR(_pthread_mutex *mutex, mutex_seq **seqaddr)
{
	// 64-bit aligned address inside m_seq array (&m_seq[0] for aligned mutex)
	// We don't require more than byte alignment on OS X. rdar://22278325
	*seqaddr = (void *)(((uintptr_t)mutex->m_seq + 0x7ul) & ~0x7ul);
}

PTHREAD_ALWAYS_INLINE
static inline void
MUTEX_GETTID_ADDR(_pthread_mutex *mutex, uint64_t **tidaddr)
{
	// 64-bit aligned address inside m_tid array (&m_tid[0] for aligned mutex)
	// We don't require more than byte alignment on OS X. rdar://22278325
	*tidaddr = (void*)(((uintptr_t)mutex->m_tid + 0x7ul) & ~0x7ul);
}

PTHREAD_ALWAYS_INLINE
static inline void
mutex_seq_load(mutex_seq *seqaddr, mutex_seq *oldseqval)
{
	oldseqval->seq_LU = seqaddr->seq_LU;
}

#define mutex_seq_atomic_load(seqaddr, oldseqval, m) \
		mutex_seq_atomic_load_##m(seqaddr, oldseqval)

PTHREAD_ALWAYS_INLINE PTHREAD_USED
static inline bool
mutex_seq_atomic_cmpxchgv_relaxed(mutex_seq *seqaddr, mutex_seq *oldseqval,
		mutex_seq *newseqval)
{
	return os_atomic_cmpxchgv(&seqaddr->atomic_seq_LU, oldseqval->seq_LU,
			newseqval->seq_LU, &oldseqval->seq_LU, relaxed);
}

PTHREAD_ALWAYS_INLINE PTHREAD_USED
static inline bool
mutex_seq_atomic_cmpxchgv_acquire(mutex_seq *seqaddr, mutex_seq *oldseqval,
		mutex_seq *newseqval)
{
	return os_atomic_cmpxchgv(&seqaddr->atomic_seq_LU, oldseqval->seq_LU,
			newseqval->seq_LU, &oldseqval->seq_LU, acquire);
}

PTHREAD_ALWAYS_INLINE PTHREAD_USED
static inline bool
mutex_seq_atomic_cmpxchgv_release(mutex_seq *seqaddr, mutex_seq *oldseqval,
		mutex_seq *newseqval)
{
	return os_atomic_cmpxchgv(&seqaddr->atomic_seq_LU, oldseqval->seq_LU,
			newseqval->seq_LU, &oldseqval->seq_LU, release);
}

#define mutex_seq_atomic_cmpxchgv(seqaddr, oldseqval, newseqval, m)\
		mutex_seq_atomic_cmpxchgv_##m(seqaddr, oldseqval, newseqval)

/*
 * Initialize a mutex variable, possibly with additional attributes.
 * Public interface - so don't trust the lock - initialize it first.
 */
PTHREAD_NOEXPORT_VARIANT
int
pthread_mutex_init(pthread_mutex_t *omutex, const pthread_mutexattr_t *attr)
{
#if 0
	/* conformance tests depend on not having this behavior */
	/* The test for this behavior is optional */
	if (_pthread_mutex_check_signature(mutex))
		return EBUSY;
#endif
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;
	_PTHREAD_LOCK_INIT(mutex->lock);
	return (_pthread_mutex_init(mutex, attr, 0x7));
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_mutex_getprioceiling(const pthread_mutex_t *omutex, int *prioceiling)
{
	int res = EINVAL;
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;
	if (_pthread_mutex_check_signature(mutex)) {
		_PTHREAD_LOCK(mutex->lock);
		*prioceiling = mutex->prioceiling;
		res = 0;
		_PTHREAD_UNLOCK(mutex->lock);
	}
	return res;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_mutex_setprioceiling(pthread_mutex_t *omutex, int prioceiling,
		int *old_prioceiling)
{
	int res = EINVAL;
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;
	if (_pthread_mutex_check_signature(mutex)) {
		_PTHREAD_LOCK(mutex->lock);
		if (prioceiling >= -999 && prioceiling <= 999) {
			*old_prioceiling = mutex->prioceiling;
			mutex->prioceiling = (int16_t)prioceiling;
			res = 0;
		}
		_PTHREAD_UNLOCK(mutex->lock);
	}
	return res;
}


int
pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr,
		int *prioceiling)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		*prioceiling = attr->prioceiling;
		res = 0;
	}
	return res;
}

int
pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		*protocol = attr->protocol;
		res = 0;
	}
	return res;
}

int
pthread_mutexattr_getpolicy_np(const pthread_mutexattr_t *attr, int *policy)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		switch (attr->opt) {
		case _PTHREAD_MTX_OPT_POLICY_FAIRSHARE:
			*policy = PTHREAD_MUTEX_POLICY_FAIRSHARE_NP;
			res = 0;
			break;
		case _PTHREAD_MTX_OPT_POLICY_FIRSTFIT:
			*policy = PTHREAD_MUTEX_POLICY_FIRSTFIT_NP;
			res = 0;
			break;
		}
	}
	return res;
}

int
pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		*type = attr->type;
		res = 0;
	}
	return res;
}

int
pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		*pshared = (int)attr->pshared;
		res = 0;
	}
	return res;
}

int
pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	attr->prioceiling = _PTHREAD_DEFAULT_PRIOCEILING;
	attr->protocol = _PTHREAD_DEFAULT_PROTOCOL;
	attr->opt = __pthread_mutex_default_opt_policy;
	attr->type = PTHREAD_MUTEX_DEFAULT;
	attr->sig = _PTHREAD_MUTEX_ATTR_SIG;
	attr->pshared = _PTHREAD_DEFAULT_PSHARED;
	return 0;
}

int
pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		if (prioceiling >= -999 && prioceiling <= 999) {
			attr->prioceiling = prioceiling;
			res = 0;
		}
	}
	return res;
}

int
pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		switch (protocol) {
			case PTHREAD_PRIO_NONE:
			case PTHREAD_PRIO_INHERIT:
			case PTHREAD_PRIO_PROTECT:
				attr->protocol = protocol;
				res = 0;
				break;
		}
	}
	return res;
}

int
pthread_mutexattr_setpolicy_np(pthread_mutexattr_t *attr, int policy)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		// <rdar://problem/35844519> the first-fit implementation was broken
		// pre-Liberty so this mapping exists to ensure that the old first-fit
		// define (2) is no longer valid when used on older systems.
		switch (policy) {
		case PTHREAD_MUTEX_POLICY_FAIRSHARE_NP:
			attr->opt = _PTHREAD_MTX_OPT_POLICY_FAIRSHARE;
			res = 0;
			break;
		case PTHREAD_MUTEX_POLICY_FIRSTFIT_NP:
			attr->opt = _PTHREAD_MTX_OPT_POLICY_FIRSTFIT;
			res = 0;
			break;
		}
	}
	return res;
}

int
pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
		switch (type) {
			case PTHREAD_MUTEX_NORMAL:
			case PTHREAD_MUTEX_ERRORCHECK:
			case PTHREAD_MUTEX_RECURSIVE:
			//case PTHREAD_MUTEX_DEFAULT:
				attr->type = type;
				res = 0;
				break;
		}
	}
	return res;
}

int
pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
	int res = EINVAL;
#if __DARWIN_UNIX03
	if (__unix_conforming == 0) {
		__unix_conforming = 1;
	}
#endif /* __DARWIN_UNIX03 */

	if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG) {
#if __DARWIN_UNIX03
		if (( pshared == PTHREAD_PROCESS_PRIVATE) ||
				(pshared == PTHREAD_PROCESS_SHARED))
#else /* __DARWIN_UNIX03 */
		if ( pshared == PTHREAD_PROCESS_PRIVATE)
#endif /* __DARWIN_UNIX03 */
		{
			attr->pshared = pshared;
			res = 0;
		}
	}
	return res;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE PTHREAD_NORETURN
int
_pthread_mutex_corruption_abort(_pthread_mutex *mutex)
{
	PTHREAD_CLIENT_CRASH(0, "pthread_mutex corruption: mutex owner changed "
			"in the middle of lock/unlock");
}


PTHREAD_NOINLINE
static int
_pthread_mutex_check_init_slow(_pthread_mutex *mutex)
{
	int res = EINVAL;

	if (_pthread_mutex_check_signature_init(mutex)) {
		_PTHREAD_LOCK(mutex->lock);
		if (_pthread_mutex_check_signature_init(mutex)) {
			// initialize a statically initialized mutex to provide
			// compatibility for misbehaving applications.
			// (unlock should not be the first operation on a mutex)
			res = _pthread_mutex_init(mutex, NULL, (mutex->sig & 0xf));
		} else if (_pthread_mutex_check_signature(mutex)) {
			res = 0;
		}
		_PTHREAD_UNLOCK(mutex->lock);
	} else if (_pthread_mutex_check_signature(mutex)) {
		res = 0;
	}
	if (res != 0) {
		PLOCKSTAT_MUTEX_ERROR((pthread_mutex_t *)mutex, res);
	}
	return res;
}

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_mutex_check_init(_pthread_mutex *mutex)
{
	int res = 0;
	if (!_pthread_mutex_check_signature(mutex)) {
		return _pthread_mutex_check_init_slow(mutex);
	}
	return res;
}

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_mutex_is_fairshare(_pthread_mutex *mutex)
{
	return (mutex->mtxopts.options.policy == _PTHREAD_MTX_OPT_POLICY_FAIRSHARE);
}

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_mutex_is_firstfit(_pthread_mutex *mutex)
{
	return (mutex->mtxopts.options.policy == _PTHREAD_MTX_OPT_POLICY_FIRSTFIT);
}

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_mutex_is_recursive(_pthread_mutex *mutex)
{
	return (mutex->mtxopts.options.type == PTHREAD_MUTEX_RECURSIVE);
}

PTHREAD_ALWAYS_INLINE
static int
_pthread_mutex_lock_handle_options(_pthread_mutex *mutex, bool trylock,
		uint64_t *tidaddr)
{
	if (mutex->mtxopts.options.type == PTHREAD_MUTEX_NORMAL) {
		// NORMAL does not do EDEADLK checking
		return 0;
	}

	uint64_t selfid = _pthread_selfid_direct();
	if (os_atomic_load(tidaddr, relaxed) == selfid) {
		if (_pthread_mutex_is_recursive(mutex)) {
			if (mutex->mtxopts.options.lock_count < USHRT_MAX) {
				mutex->mtxopts.options.lock_count += 1;
				return mutex->mtxopts.options.lock_count;
			} else {
				return -EAGAIN;
			}
		} else if (trylock) { /* PTHREAD_MUTEX_ERRORCHECK */
			// <rdar://problem/16261552> as per OpenGroup, trylock cannot
			// return EDEADLK on a deadlock, it should return EBUSY.
			return -EBUSY;
		} else { /* PTHREAD_MUTEX_ERRORCHECK */
			return -EDEADLK;
		}
	}

	// Not recursive, or recursive but first lock.
	return 0;
}

PTHREAD_ALWAYS_INLINE
static int
_pthread_mutex_unlock_handle_options(_pthread_mutex *mutex, uint64_t *tidaddr)
{
	if (mutex->mtxopts.options.type == PTHREAD_MUTEX_NORMAL) {
		// NORMAL does not do EDEADLK checking
		return 0;
	}

	uint64_t selfid = _pthread_selfid_direct();
	if (os_atomic_load(tidaddr, relaxed) != selfid) {
		return -EPERM;
	} else if (_pthread_mutex_is_recursive(mutex) &&
			--mutex->mtxopts.options.lock_count) {
		return 1;
	}
	return 0;
}

/*
 * Sequence numbers and TID:
 *
 * In steady (and uncontended) state, an unlocked mutex will
 * look like A=[L4 U4 TID0]. When it is being locked, it transitions
 * to B=[L5+KE U4 TID0] and then C=[L5+KE U4 TID940]. For an uncontended mutex,
 * the unlock path will then transition to D=[L5 U4 TID0] and then finally
 * E=[L5 U5 TID0].
 *
 * If a contender comes in after B, the mutex will instead transition to
 * E=[L6+KE U4 TID0] and then F=[L6+KE U4 TID940]. If a contender comes in after
 * C, it will transition to F=[L6+KE U4 TID940] directly. In both cases, the
 * contender will enter the kernel with either mutexwait(U4, TID0) or
 * mutexwait(U4, TID940). The first owner will unlock the mutex by first
 * updating the owner to G=[L6+KE U4 TID-1] and then doing the actual unlock to
 * H=[L6+KE U5 TID=-1] before entering the kernel with mutexdrop(U5, -1) to
 * signal the next waiter (potentially as a prepost). When the waiter comes out
 * of the kernel, it will update the owner to I=[L6+KE U5 TID941]. An unlock at
 * this point is simply J=[L6 U5 TID0] and then K=[L6 U6 TID0].
 *
 * At various points along these timelines, since the sequence words and TID are
 * written independently, a thread may get preempted and another thread might
 * see inconsistent data. In the worst case, another thread may see the TID in
 * the SWITCHING (-1) state or unlocked (0) state for longer because the owning
 * thread was preempted.
 */

/*
 * Drop the mutex unlock references from cond_wait or mutex_unlock.
 */
PTHREAD_ALWAYS_INLINE
static inline int
_pthread_mutex_fairshare_unlock_updatebits(_pthread_mutex *mutex,
		uint32_t *flagsp, uint32_t **pmtxp, uint32_t *mgenp, uint32_t *ugenp)
{
	uint32_t flags = mutex->mtxopts.value;
	flags &= ~_PTHREAD_MTX_OPT_NOTIFY; // no notification by default

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t oldtid, newtid;

	int res = _pthread_mutex_unlock_handle_options(mutex, tidaddr);
	if (res > 0) {
		// Valid recursive unlock
		if (flagsp) {
			*flagsp = flags;
		}
		PLOCKSTAT_MUTEX_RELEASE((pthread_mutex_t *)mutex, 1);
		return 0;
	} else if (res < 0) {
		PLOCKSTAT_MUTEX_ERROR((pthread_mutex_t *)mutex, -res);
		return -res;
	}

	bool clearnotify, spurious;
	do {
		newseq = oldseq;
		oldtid = os_atomic_load(tidaddr, relaxed);

		clearnotify = false;
		spurious = false;

		// pending waiters
		int numwaiters = diff_genseq(oldseq.lgenval, oldseq.ugenval);
		if (numwaiters == 0) {
			// spurious unlock (unlock of unlocked lock)
			spurious = true;
		} else {
			newseq.ugenval += PTHRW_INC;

			if ((oldseq.lgenval & PTHRW_COUNT_MASK) ==
					(newseq.ugenval & PTHRW_COUNT_MASK)) {
				// our unlock sequence matches to lock sequence, so if the
				// CAS is successful, the mutex is unlocked

				/* do not reset Ibit, just K&E */
				newseq.lgenval &= ~(PTH_RWL_KBIT | PTH_RWL_EBIT);
				clearnotify = true;
				newtid = 0; // clear owner
			} else {
				newtid = PTHREAD_MTX_TID_SWITCHING;
				// need to signal others waiting for mutex
				flags |= _PTHREAD_MTX_OPT_NOTIFY;
			}

			if (newtid != oldtid) {
				// We're giving up the mutex one way or the other, so go ahead
				// and update the owner to 0 so that once the CAS below
				// succeeds, there is no stale ownership information. If the
				// CAS of the seqaddr fails, we may loop, but it's still valid
				// for the owner to be SWITCHING/0
				if (!os_atomic_cmpxchg(tidaddr, oldtid, newtid, relaxed)) {
					// we own this mutex, nobody should be updating it except us
					return _pthread_mutex_corruption_abort(mutex);
				}
			}
		}

		if (clearnotify || spurious) {
			flags &= ~_PTHREAD_MTX_OPT_NOTIFY;
		}
	} while (!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq, release));

	PTHREAD_TRACE(psynch_mutex_unlock_updatebits, mutex, oldseq.lgenval,
			newseq.lgenval, oldtid);

	if (mgenp != NULL) {
		*mgenp = newseq.lgenval;
	}
	if (ugenp != NULL) {
		*ugenp = newseq.ugenval;
	}
	if (pmtxp != NULL) {
		*pmtxp = (uint32_t *)mutex;
	}
	if (flagsp != NULL) {
		*flagsp = flags;
	}

	return 0;
}

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_mutex_fairshare_lock_updatebits(_pthread_mutex *mutex, uint64_t selfid)
{
	bool firstfit = _pthread_mutex_is_firstfit(mutex);
	bool gotlock = true;

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);

	do {
		newseq = oldseq;

		if (firstfit) {
			// firstfit locks can have the lock stolen out from under a locker
			// between the unlock from the kernel and this lock path. When this
			// happens, we still want to set the K bit before leaving the loop
			// (or notice if the lock unlocks while we try to update).
			gotlock = !is_rwl_ebit_set(oldseq.lgenval);
		} else if ((oldseq.lgenval & (PTH_RWL_KBIT | PTH_RWL_EBIT)) == 
				(PTH_RWL_KBIT | PTH_RWL_EBIT)) {
			// bit are already set, just update the owner tidaddr
			break;
		}

		newseq.lgenval |= PTH_RWL_KBIT | PTH_RWL_EBIT;
	} while (!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			acquire));

	if (gotlock) {
		os_atomic_store(tidaddr, selfid, relaxed);
	}

	PTHREAD_TRACE(psynch_mutex_lock_updatebits, mutex, oldseq.lgenval,
			newseq.lgenval, 0);

	// failing to take the lock in firstfit returns 1 to force the caller
	// to wait in the kernel
	return gotlock ? 0 : 1;
}

PTHREAD_NOINLINE
static int
_pthread_mutex_fairshare_lock_wait(_pthread_mutex *mutex, mutex_seq newseq,
		uint64_t oldtid)
{
	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t selfid = _pthread_selfid_direct();

	PLOCKSTAT_MUTEX_BLOCK((pthread_mutex_t *)mutex);
	do {
		uint32_t updateval;
		do {
			updateval = __psynch_mutexwait(mutex, newseq.lgenval,
					newseq.ugenval, oldtid, mutex->mtxopts.value);
			oldtid = os_atomic_load(tidaddr, relaxed);
		} while (updateval == (uint32_t)-1);

		// returns 0 on succesful update; in firstfit it may fail with 1
	} while (_pthread_mutex_fairshare_lock_updatebits(mutex, selfid) == 1);
	PLOCKSTAT_MUTEX_BLOCKED((pthread_mutex_t *)mutex, BLOCK_SUCCESS_PLOCKSTAT);

	return 0;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_mutex_fairshare_lock_slow(_pthread_mutex *omutex, bool trylock)
{
	int res, recursive = 0;
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t oldtid, selfid = _pthread_selfid_direct();

	res = _pthread_mutex_lock_handle_options(mutex, trylock, tidaddr);
	if (res > 0) {
		recursive = 1;
		res = 0;
		goto out;
	} else if (res < 0) {
		res = -res;
		goto out;
	}

	bool gotlock;
	do {
		newseq = oldseq;
		oldtid = os_atomic_load(tidaddr, relaxed);

		gotlock = ((oldseq.lgenval & PTH_RWL_EBIT) == 0);

		if (trylock && !gotlock) {
			// A trylock on a held lock will fail immediately. But since
			// we did not load the sequence words atomically, perform a
			// no-op CAS64 to ensure that nobody has unlocked concurrently.
		} else {
			// Increment the lock sequence number and force the lock into E+K
			// mode, whether "gotlock" is true or not.
			newseq.lgenval += PTHRW_INC;
			newseq.lgenval |= PTH_RWL_EBIT | PTH_RWL_KBIT;
		}
	} while (!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq, acquire));

	PTHREAD_TRACE(psynch_mutex_lock_updatebits, omutex, oldseq.lgenval,
			newseq.lgenval, 0);

	if (gotlock) {
		os_atomic_store(tidaddr, selfid, relaxed);
		res = 0;
		PTHREAD_TRACE(psynch_mutex_ulock, omutex, newseq.lgenval,
				newseq.ugenval, selfid);
	} else if (trylock) {
		res = EBUSY;
		PTHREAD_TRACE(psynch_mutex_utrylock_failed, omutex, newseq.lgenval,
				newseq.ugenval, oldtid);
	} else {
		PTHREAD_TRACE(psynch_mutex_ulock | DBG_FUNC_START, omutex,
				newseq.lgenval, newseq.ugenval, oldtid);
		res = _pthread_mutex_fairshare_lock_wait(mutex, newseq, oldtid);
		PTHREAD_TRACE(psynch_mutex_ulock | DBG_FUNC_END, omutex,
				newseq.lgenval, newseq.ugenval, oldtid);
	}

	if (res == 0 && _pthread_mutex_is_recursive(mutex)) {
		mutex->mtxopts.options.lock_count = 1;
	}

out:
#if PLOCKSTAT
	if (res == 0) {
		PLOCKSTAT_MUTEX_ACQUIRE((pthread_mutex_t *)mutex, recursive, 0);
	} else {
		PLOCKSTAT_MUTEX_ERROR((pthread_mutex_t *)mutex, res);
	}
#endif

	return res;
}

PTHREAD_NOINLINE
static inline int
_pthread_mutex_fairshare_lock(_pthread_mutex *mutex, bool trylock)
{
#if ENABLE_USERSPACE_TRACE
	return _pthread_mutex_fairshare_lock_slow(mutex, trylock);
#elif PLOCKSTAT
	if (PLOCKSTAT_MUTEX_ACQUIRE_ENABLED() || PLOCKSTAT_MUTEX_ERROR_ENABLED()) {
		return _pthread_mutex_fairshare_lock_slow(mutex, trylock);
	}
#endif

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t selfid = _pthread_selfid_direct();

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	if (os_unlikely(oldseq.lgenval & PTH_RWL_EBIT)) {
		return _pthread_mutex_fairshare_lock_slow(mutex, trylock);
	}

	bool gotlock;
	do {
		newseq = oldseq;

		gotlock = ((oldseq.lgenval & PTH_RWL_EBIT) == 0);

		if (trylock && !gotlock) {
			// A trylock on a held lock will fail immediately. But since
			// we did not load the sequence words atomically, perform a
			// no-op CAS64 to ensure that nobody has unlocked concurrently.
		} else if (os_likely(gotlock)) {
			// Increment the lock sequence number and force the lock into E+K
			// mode, whether "gotlock" is true or not.
			newseq.lgenval += PTHRW_INC;
			newseq.lgenval |= PTH_RWL_EBIT | PTH_RWL_KBIT;
		} else {
			return _pthread_mutex_fairshare_lock_slow(mutex, trylock);
		}
	} while (os_unlikely(!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			acquire)));

	if (os_likely(gotlock)) {
		os_atomic_store(tidaddr, selfid, relaxed);
		return 0;
	} else if (trylock) {
		return EBUSY;
	} else {
		__builtin_trap();
	}
}

PTHREAD_NOINLINE
static int
_pthread_mutex_fairshare_unlock_drop(_pthread_mutex *mutex, mutex_seq newseq,
		uint32_t flags)
{
	int res;
	uint32_t updateval;

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);

	PTHREAD_TRACE(psynch_mutex_uunlock | DBG_FUNC_START, mutex, newseq.lgenval,
			newseq.ugenval, os_atomic_load(tidaddr, relaxed));

	updateval = __psynch_mutexdrop(mutex, newseq.lgenval, newseq.ugenval,
			os_atomic_load(tidaddr, relaxed), flags);

	PTHREAD_TRACE(psynch_mutex_uunlock | DBG_FUNC_END, mutex, updateval, 0, 0);

	if (updateval == (uint32_t)-1) {
		res = errno;

		if (res == EINTR) {
			res = 0;
		}
		if (res != 0) {
			PTHREAD_INTERNAL_CRASH(res, "__psynch_mutexdrop failed");
		}
		return res;
	}

	return 0;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_mutex_fairshare_unlock_slow(_pthread_mutex *mutex)
{
	int res;
	mutex_seq newseq;
	uint32_t flags;

	res = _pthread_mutex_fairshare_unlock_updatebits(mutex, &flags, NULL,
			&newseq.lgenval, &newseq.ugenval);
	if (res != 0) return res;

	if ((flags & _PTHREAD_MTX_OPT_NOTIFY) != 0) {
		return _pthread_mutex_fairshare_unlock_drop(mutex, newseq, flags);
	} else {
		uint64_t *tidaddr;
		MUTEX_GETTID_ADDR(mutex, &tidaddr);
		PTHREAD_TRACE(psynch_mutex_uunlock, mutex, newseq.lgenval,
				newseq.ugenval, os_atomic_load(tidaddr, relaxed));
	}

	return 0;
}

PTHREAD_NOINLINE
static int
_pthread_mutex_fairshare_unlock(_pthread_mutex *mutex)
{
#if ENABLE_USERSPACE_TRACE
	return _pthread_mutex_fairshare_unlock_slow(mutex);
#elif PLOCKSTAT
	if (PLOCKSTAT_MUTEX_RELEASE_ENABLED() || PLOCKSTAT_MUTEX_ERROR_ENABLED()) {
		return _pthread_mutex_fairshare_unlock_slow(mutex);
	}
#endif

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	int numwaiters = diff_genseq(oldseq.lgenval, oldseq.ugenval);
	if (os_unlikely(numwaiters == 0)) {
		// spurious unlock (unlock of unlocked lock)
		return 0;
	}

	// We're giving up the mutex one way or the other, so go ahead and
	// update the owner to 0 so that once the CAS below succeeds, there
	// is no stale ownership information. If the CAS of the seqaddr
	// fails, we may loop, but it's still valid for the owner to be
	// SWITCHING/0
	os_atomic_store(tidaddr, 0, relaxed);

	do {
		newseq = oldseq;
		newseq.ugenval += PTHRW_INC;

		if (os_likely((oldseq.lgenval & PTHRW_COUNT_MASK) ==
				(newseq.ugenval & PTHRW_COUNT_MASK))) {
			// if we succeed in performing the CAS we can be sure of a fast
			// path (only needing the CAS) unlock, if:
			//   a. our lock and unlock sequence are equal
			//   b. we don't need to clear an unlock prepost from the kernel

			// do not reset Ibit, just K&E
			newseq.lgenval &= ~(PTH_RWL_KBIT | PTH_RWL_EBIT);
		} else {
			return _pthread_mutex_fairshare_unlock_slow(mutex);
		}
	} while (os_unlikely(!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			release)));

	return 0;
}

#pragma mark firstfit

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_mutex_firstfit_unlock_updatebits(_pthread_mutex *mutex,
		uint32_t *flagsp, uint32_t **mutexp, uint32_t *lvalp, uint32_t *uvalp)
{
	uint32_t flags = mutex->mtxopts.value & ~_PTHREAD_MTX_OPT_NOTIFY;
	bool kernel_wake;

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t oldtid;

	int res = _pthread_mutex_unlock_handle_options(mutex, tidaddr);
	if (res > 0) {
		// Valid recursive unlock
		if (flagsp) {
			*flagsp = flags;
		}
		PLOCKSTAT_MUTEX_RELEASE((pthread_mutex_t *)mutex, 1);
		return 0;
	} else if (res < 0) {
		PLOCKSTAT_MUTEX_ERROR((pthread_mutex_t *)mutex, -res);
		return -res;
	}

	do {
		newseq = oldseq;
		oldtid = os_atomic_load(tidaddr, relaxed);
		// More than one kernel waiter means we need to do a wake.
		kernel_wake = diff_genseq(oldseq.lgenval, oldseq.ugenval) > 0;
		newseq.lgenval &= ~PTH_RWL_EBIT;

		if (kernel_wake) {
			// Going to the kernel post-unlock removes a single waiter unlock
			// from the mutex counts.
			newseq.ugenval += PTHRW_INC;
		}

		if (oldtid != 0) {
			if (!os_atomic_cmpxchg(tidaddr, oldtid, 0, relaxed)) {
				return _pthread_mutex_corruption_abort(mutex);
			}
		}
	} while (!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq, release));

	PTHREAD_TRACE(psynch_ffmutex_unlock_updatebits, mutex, oldseq.lgenval,
			newseq.lgenval, newseq.ugenval);

	if (kernel_wake) {
		// We choose to return this out via flags because the condition
		// variable also uses this to determine whether to do a kernel wake
		// when beginning a cvwait.
		flags |= _PTHREAD_MTX_OPT_NOTIFY;
	}
	if (lvalp) {
		*lvalp = newseq.lgenval;
	}
	if (uvalp) {
		*uvalp = newseq.ugenval;
	}
	if (mutexp) {
		*mutexp = (uint32_t *)mutex;
	}
	if (flagsp) {
		*flagsp = flags;
	}
	return 0;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
static int
_pthread_mutex_firstfit_wake(_pthread_mutex *mutex, mutex_seq newseq,
		uint32_t flags)
{
	PTHREAD_TRACE(psynch_ffmutex_wake, mutex, newseq.lgenval, newseq.ugenval,
			0);
	int res = __psynch_mutexdrop(mutex, newseq.lgenval, newseq.ugenval, 0,
			flags);

	if (res == -1) {
		res = errno;
		if (res == EINTR) {
			res = 0;
		}
		if (res != 0) {
			PTHREAD_INTERNAL_CRASH(res, "__psynch_mutexdrop failed");
		}
		return res;
	}
	return 0;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_mutex_firstfit_unlock_slow(_pthread_mutex *mutex)
{
	mutex_seq newseq;
	uint32_t flags;
	int res;

	res = _pthread_mutex_firstfit_unlock_updatebits(mutex, &flags, NULL,
			&newseq.lgenval, &newseq.ugenval);
	if (res != 0) return res;

	if (flags & _PTHREAD_MTX_OPT_NOTIFY) {
		return _pthread_mutex_firstfit_wake(mutex, newseq, flags);
	}
	return 0;
}

PTHREAD_ALWAYS_INLINE
static bool
_pthread_mutex_firstfit_lock_updatebits(_pthread_mutex *mutex, uint64_t selfid,
		mutex_seq *newseqp)
{
	bool gotlock;

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);

	PTHREAD_TRACE(psynch_ffmutex_lock_updatebits | DBG_FUNC_START, mutex,
			oldseq.lgenval, oldseq.ugenval, 0);

	do {
		newseq = oldseq;
		gotlock = is_rwl_ebit_clear(oldseq.lgenval);

		if (gotlock) {
			// If we see the E-bit cleared, we should just attempt to take it.
			newseq.lgenval |= PTH_RWL_EBIT;
		} else {
			// If we failed to get the lock then we need to put ourselves back
			// in the queue of waiters. The previous unlocker that woke us out
			// of the kernel consumed the S-count for our previous wake. So
			// take another ticket on L and go back in the kernel to sleep.
			newseq.lgenval += PTHRW_INC;
		}
	} while (!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq, acquire));

	if (gotlock) {
		os_atomic_store(tidaddr, selfid, relaxed);
	}

	PTHREAD_TRACE(psynch_ffmutex_lock_updatebits | DBG_FUNC_END, mutex,
			newseq.lgenval, newseq.ugenval, 0);

	if (newseqp) {
		*newseqp = newseq;
	}
	return gotlock;
}

PTHREAD_NOINLINE
static int
_pthread_mutex_firstfit_lock_wait(_pthread_mutex *mutex, mutex_seq newseq,
		uint64_t oldtid)
{
	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t selfid = _pthread_selfid_direct();

	PLOCKSTAT_MUTEX_BLOCK((pthread_mutex_t *)mutex);
	do {
		uint32_t uval;
		do {
			PTHREAD_TRACE(psynch_ffmutex_wait | DBG_FUNC_START, mutex,
					newseq.lgenval, newseq.ugenval, mutex->mtxopts.value);
			uval = __psynch_mutexwait(mutex, newseq.lgenval, newseq.ugenval,
					oldtid, mutex->mtxopts.value);
			PTHREAD_TRACE(psynch_ffmutex_wait | DBG_FUNC_END, mutex,
					uval, 0, 0);
			oldtid = os_atomic_load(tidaddr, relaxed);
		} while (uval == (uint32_t)-1);
	} while (!_pthread_mutex_firstfit_lock_updatebits(mutex, selfid, &newseq));
	PLOCKSTAT_MUTEX_BLOCKED((pthread_mutex_t *)mutex, BLOCK_SUCCESS_PLOCKSTAT);

	return 0;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_mutex_firstfit_lock_slow(_pthread_mutex *mutex, bool trylock)
{
	int res, recursive = 0;

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t oldtid, selfid = _pthread_selfid_direct();

	res = _pthread_mutex_lock_handle_options(mutex, trylock, tidaddr);
	if (res > 0) {
		recursive = 1;
		res = 0;
		goto out;
	} else if (res < 0) {
		res = -res;
		goto out;
	}

	PTHREAD_TRACE(psynch_ffmutex_lock_updatebits | DBG_FUNC_START, mutex,
			oldseq.lgenval, oldseq.ugenval, 0);

	bool gotlock;
	do {
		newseq = oldseq;
		oldtid = os_atomic_load(tidaddr, relaxed);

		gotlock = is_rwl_ebit_clear(oldseq.lgenval);
		if (trylock && !gotlock) {
			// We still want to perform the CAS here, even though it won't
			// do anything so that it fails if someone unlocked while we were
			// in the loop
		} else if (gotlock) {
			// In first-fit, getting the lock simply adds the E-bit
			newseq.lgenval |= PTH_RWL_EBIT;
		} else {
			// Failed to get the lock, increment the L-val and go to
			// the kernel to sleep
			newseq.lgenval += PTHRW_INC;
		}
	} while (!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq, acquire));

	PTHREAD_TRACE(psynch_ffmutex_lock_updatebits | DBG_FUNC_END, mutex,
			newseq.lgenval, newseq.ugenval, 0);

	if (gotlock) {
		os_atomic_store(tidaddr, selfid, relaxed);
		res = 0;
		PTHREAD_TRACE(psynch_mutex_ulock, mutex, newseq.lgenval,
				newseq.ugenval, selfid);
	} else if (trylock) {
		res = EBUSY;
		PTHREAD_TRACE(psynch_mutex_utrylock_failed, mutex, newseq.lgenval,
				newseq.ugenval, oldtid);
	} else {
		PTHREAD_TRACE(psynch_mutex_ulock | DBG_FUNC_START, mutex,
				newseq.lgenval, newseq.ugenval, oldtid);
		res = _pthread_mutex_firstfit_lock_wait(mutex, newseq, oldtid);
		PTHREAD_TRACE(psynch_mutex_ulock | DBG_FUNC_END, mutex,
				newseq.lgenval, newseq.ugenval, oldtid);
	}

	if (res == 0 && _pthread_mutex_is_recursive(mutex)) {
		mutex->mtxopts.options.lock_count = 1;
	}

out:
#if PLOCKSTAT
	if (res == 0) {
		PLOCKSTAT_MUTEX_ACQUIRE((pthread_mutex_t *)mutex, recursive, 0);
	} else {
		PLOCKSTAT_MUTEX_ERROR((pthread_mutex_t *)mutex, res);
	}
#endif
	return res;
}

#pragma mark fast path

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_mutex_droplock(_pthread_mutex *mutex, uint32_t *flagsp,
		uint32_t **pmtxp, uint32_t *mgenp, uint32_t *ugenp)
{
	if (_pthread_mutex_is_fairshare(mutex)) {
		return _pthread_mutex_fairshare_unlock_updatebits(mutex, flagsp,
				pmtxp, mgenp, ugenp);
	}
	return _pthread_mutex_firstfit_unlock_updatebits(mutex, flagsp, pmtxp,
			mgenp, ugenp);
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_mutex_lock_init_slow(_pthread_mutex *mutex, bool trylock)
{
	int res;

	res = _pthread_mutex_check_init(mutex);
	if (res != 0) return res;

	if (os_unlikely(_pthread_mutex_is_fairshare(mutex))) {
		return _pthread_mutex_fairshare_lock_slow(mutex, trylock);
	}
	return _pthread_mutex_firstfit_lock_slow(mutex, trylock);
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
static int
_pthread_mutex_unlock_init_slow(_pthread_mutex *mutex)
{
	int res;

	// Initialize static mutexes for compatibility with misbehaving
	// applications (unlock should not be the first operation on a mutex).
	res = _pthread_mutex_check_init(mutex);
	if (res != 0) return res;

	if (os_unlikely(_pthread_mutex_is_fairshare(mutex))) {
		return _pthread_mutex_fairshare_unlock_slow(mutex);
	}
	return _pthread_mutex_firstfit_unlock_slow(mutex);
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_mutex_unlock(pthread_mutex_t *omutex)
{
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;
	if (os_unlikely(!_pthread_mutex_check_signature_fast(mutex))) {
		return _pthread_mutex_unlock_init_slow(mutex);
	}

	if (os_unlikely(_pthread_mutex_is_fairshare(mutex))) {
		return _pthread_mutex_fairshare_unlock(mutex);
	}

#if ENABLE_USERSPACE_TRACE
	return _pthread_mutex_firstfit_unlock_slow(mutex);
#elif PLOCKSTAT
	if (PLOCKSTAT_MUTEX_RELEASE_ENABLED() || PLOCKSTAT_MUTEX_ERROR_ENABLED()) {
		return _pthread_mutex_firstfit_unlock_slow(mutex);
	}
#endif

	/*
	 * This is the first-fit fast path. The fairshare fast-ish path is in
	 * _pthread_mutex_firstfit_unlock()
	 */
	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	// We're giving up the mutex one way or the other, so go ahead and
	// update the owner to 0 so that once the CAS below succeeds, there
	// is no stale ownership information. If the CAS of the seqaddr
	// fails, we may loop, but it's still valid for the owner to be
	// SWITCHING/0
	os_atomic_store(tidaddr, 0, relaxed);

	do {
		newseq = oldseq;

		if (diff_genseq(oldseq.lgenval, oldseq.ugenval) == 0) {
			// No outstanding waiters in kernel, we can simply drop the E-bit
			// and return.
			newseq.lgenval &= ~PTH_RWL_EBIT;
		} else {
			return _pthread_mutex_firstfit_unlock_slow(mutex);
		}
	} while (os_unlikely(!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			release)));

	return 0;
}

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_mutex_firstfit_lock(pthread_mutex_t *omutex, bool trylock)
{
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;
	if (os_unlikely(!_pthread_mutex_check_signature_fast(mutex))) {
		return _pthread_mutex_lock_init_slow(mutex, trylock);
	}

	if (os_unlikely(_pthread_mutex_is_fairshare(mutex))) {
		return _pthread_mutex_fairshare_lock(mutex, trylock);
	}

#if ENABLE_USERSPACE_TRACE
	return _pthread_mutex_firstfit_lock_slow(mutex, trylock);
#elif PLOCKSTAT
	if (PLOCKSTAT_MUTEX_ACQUIRE_ENABLED() || PLOCKSTAT_MUTEX_ERROR_ENABLED()) {
		return _pthread_mutex_firstfit_lock_slow(mutex, trylock);
	}
#endif

	/*
	 * This is the first-fit fast path. The fairshare fast-ish path is in
	 * _pthread_mutex_firstfit_lock()
	 */
	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);
	uint64_t selfid = _pthread_selfid_direct();

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	mutex_seq oldseq, newseq;
	mutex_seq_load(seqaddr, &oldseq);

	if (os_unlikely(!trylock && (oldseq.lgenval & PTH_RWL_EBIT))) {
		return _pthread_mutex_firstfit_lock_slow(mutex, trylock);
	}

	bool gotlock;
	do {
		newseq = oldseq;
		gotlock = is_rwl_ebit_clear(oldseq.lgenval);

		if (trylock && !gotlock) {
#if __LP64__
			// The sequence load is atomic, so we can bail here without writing
			// it and avoid some unnecessary coherence traffic - rdar://57259033
			os_atomic_thread_fence(acquire);
			return EBUSY;
#else
			// A trylock on a held lock will fail immediately. But since
			// we did not load the sequence words atomically, perform a
			// no-op CAS64 to ensure that nobody has unlocked concurrently.
#endif
		} else if (os_likely(gotlock)) {
			// In first-fit, getting the lock simply adds the E-bit
			newseq.lgenval |= PTH_RWL_EBIT;
		} else {
			return _pthread_mutex_firstfit_lock_slow(mutex, trylock);
		}
	} while (os_unlikely(!mutex_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			acquire)));

	if (os_likely(gotlock)) {
		os_atomic_store(tidaddr, selfid, relaxed);
		return 0;
	} else if (trylock) {
		return EBUSY;
	} else {
		__builtin_trap();
	}
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
	return _pthread_mutex_firstfit_lock(mutex, false);
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	return _pthread_mutex_firstfit_lock(mutex, true);
}


PTHREAD_ALWAYS_INLINE
static inline int
_pthread_mutex_init(_pthread_mutex *mutex, const pthread_mutexattr_t *attr,
		uint32_t static_type)
{
	mutex->mtxopts.value = 0;
	mutex->mtxopts.options.mutex = 1;
	if (attr) {
		if (attr->sig != _PTHREAD_MUTEX_ATTR_SIG) {
			return EINVAL;
		}
		mutex->prioceiling = (int16_t)attr->prioceiling;
		mutex->mtxopts.options.protocol = attr->protocol;
		mutex->mtxopts.options.policy = attr->opt;
		mutex->mtxopts.options.type = attr->type;
		mutex->mtxopts.options.pshared = attr->pshared;
	} else {
		switch (static_type) {
			case 1:
				mutex->mtxopts.options.type = PTHREAD_MUTEX_ERRORCHECK;
				break;
			case 2:
				mutex->mtxopts.options.type = PTHREAD_MUTEX_RECURSIVE;
				break;
			case 3:
				/* firstfit fall thru */
			case 7:
				mutex->mtxopts.options.type = PTHREAD_MUTEX_DEFAULT;
				break;
			default:
				return EINVAL;
		}

		mutex->prioceiling = _PTHREAD_DEFAULT_PRIOCEILING;
		mutex->mtxopts.options.protocol = _PTHREAD_DEFAULT_PROTOCOL;
		if (static_type != 3) {
			mutex->mtxopts.options.policy = __pthread_mutex_default_opt_policy;
		} else {
			mutex->mtxopts.options.policy = _PTHREAD_MTX_OPT_POLICY_FIRSTFIT;
		}
		mutex->mtxopts.options.pshared = _PTHREAD_DEFAULT_PSHARED;
	}
	mutex->priority = 0;

	mutex_seq *seqaddr;
	MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

	uint64_t *tidaddr;
	MUTEX_GETTID_ADDR(mutex, &tidaddr);

#if PTHREAD_MUTEX_INIT_UNUSED
	if ((uint32_t*)tidaddr != mutex->m_tid) {
		mutex->mtxopts.options.misalign = 1;
		__builtin_memset(mutex->m_tid, 0xff, sizeof(mutex->m_tid));
	}
	__builtin_memset(mutex->m_mis, 0xff, sizeof(mutex->m_mis));
#endif // PTHREAD_MUTEX_INIT_UNUSED
	*tidaddr = 0;
	*seqaddr = (mutex_seq){ };

	long sig = _PTHREAD_MUTEX_SIG;
	if (mutex->mtxopts.options.type == PTHREAD_MUTEX_NORMAL &&
			(_pthread_mutex_is_fairshare(mutex) ||
			 _pthread_mutex_is_firstfit(mutex))) {
		// rdar://18148854 _pthread_mutex_lock & pthread_mutex_unlock fastpath
		sig = _PTHREAD_MUTEX_SIG_fast;
	}

#if PTHREAD_MUTEX_INIT_UNUSED
	// For detecting copied mutexes and smashes during debugging
	uint32_t sig32 = (uint32_t)sig;
#if defined(__LP64__)
	uintptr_t guard = ~(uintptr_t)mutex; // use ~ to hide from leaks
	__builtin_memcpy(mutex->_reserved, &guard, sizeof(guard));
	mutex->_reserved[2] = sig32;
	mutex->_reserved[3] = sig32;
	mutex->_pad = sig32;
#else
	mutex->_reserved[0] = sig32;
#endif
#endif // PTHREAD_MUTEX_INIT_UNUSED

	// Ensure all contents are properly set before setting signature.
#if defined(__LP64__)
	// For binary compatibility reasons we cannot require natural alignment of
	// the 64bit 'sig' long value in the struct. rdar://problem/21610439
	uint32_t *sig32_ptr = (uint32_t*)&mutex->sig;
	uint32_t *sig32_val = (uint32_t*)&sig;
	*(sig32_ptr + 1) = *(sig32_val + 1);
	os_atomic_store(sig32_ptr, *sig32_val, release);
#else
	os_atomic_store2o(mutex, sig, sig, release);
#endif

	return 0;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_mutex_destroy(pthread_mutex_t *omutex)
{
	_pthread_mutex *mutex = (_pthread_mutex *)omutex;

	int res = EINVAL;

	_PTHREAD_LOCK(mutex->lock);
	if (_pthread_mutex_check_signature(mutex)) {
		mutex_seq *seqaddr;
		MUTEX_GETSEQ_ADDR(mutex, &seqaddr);

		mutex_seq seq;
		mutex_seq_load(seqaddr, &seq);

		uint64_t *tidaddr;
		MUTEX_GETTID_ADDR(mutex, &tidaddr);

		if ((os_atomic_load(tidaddr, relaxed) == 0) &&
				(seq.lgenval & PTHRW_COUNT_MASK) ==
				(seq.ugenval & PTHRW_COUNT_MASK)) {
			mutex->sig = _PTHREAD_NO_SIG;
			res = 0;
		} else {
			res = EBUSY;
		}
	} else if (_pthread_mutex_check_signature_init(mutex)) {
		mutex->sig = _PTHREAD_NO_SIG;
		res = 0;
	}
	_PTHREAD_UNLOCK(mutex->lock);

	return res;
}

#endif /* !BUILDING_VARIANT ] */

/*
 * Destroy a mutex attribute structure.
 */
int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
#if __DARWIN_UNIX03
	if (__unix_conforming == 0) {
		__unix_conforming = 1;
	}
	if (attr->sig != _PTHREAD_MUTEX_ATTR_SIG) {
		return EINVAL;
	}
#endif /* __DARWIN_UNIX03 */

	attr->sig = _PTHREAD_NO_SIG;
	return 0;
}

