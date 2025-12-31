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
/*-
 * Copyright (c) 1998 Alex Nash
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libc_r/uthread/uthread_rwlock.c,v 1.6 2001/04/10 04:19:20 deischen Exp $
 */

/*
 * POSIX Pthread Library
 * -- Read Write Lock support
 * 4/24/02: A. Ramesh
 *	   Ported from FreeBSD
 */

#include "resolver.h"
#include "internal.h"
#if DEBUG
#include <platform/compat.h> // for bzero
#endif

#ifndef errno_t
#include <errno.h>
#endif

#ifdef PLOCKSTAT
#include "plockstat.h"
#else /* !PLOCKSTAT */
#define PLOCKSTAT_RW_ERROR(x, y, z)
#define PLOCKSTAT_RW_BLOCK(x, y)
#define PLOCKSTAT_RW_BLOCKED(x, y, z)
#define PLOCKSTAT_RW_ACQUIRE(x, y)
#define PLOCKSTAT_RW_RELEASE(x, y)
#endif /* PLOCKSTAT */

#define READ_LOCK_PLOCKSTAT  0
#define WRITE_LOCK_PLOCKSTAT 1

#define BLOCK_FAIL_PLOCKSTAT    0
#define BLOCK_SUCCESS_PLOCKSTAT 1

#define PTHREAD_RWLOCK_INIT_UNUSED 1

// maximum number of times a read lock may be obtained
#define	MAX_READ_LOCKS		(INT_MAX - 1)

union rwlock_seq; // forward declaration
enum rwlock_seqfields; // forward declaration

PTHREAD_NOEXPORT PTHREAD_WEAK // prevent inlining of return value into callers
int _pthread_rwlock_lock_slow(pthread_rwlock_t *orwlock, bool readlock,
		bool trylock);

PTHREAD_NOEXPORT PTHREAD_WEAK // prevent inlining of return value into callers
int _pthread_rwlock_unlock_slow(pthread_rwlock_t *orwlock,
		enum rwlock_seqfields updated_seqfields);


#if defined(__LP64__)
#define RWLOCK_USE_INT128 1
#endif

typedef union rwlock_seq {
	uint32_t seq[4];
	struct { uint32_t lcntval; uint32_t rw_seq; uint32_t ucntval; };
	struct { uint32_t lgen; uint32_t rw_wc; uint32_t ugen; };
#if RWLOCK_USE_INT128
	unsigned __int128 seq_LSU;
	unsigned __int128 _Atomic atomic_seq_LSU;
#endif
	struct {
		uint64_t seq_LS;
		uint32_t seq_U;
		uint32_t _pad;
	};
	struct {
		uint64_t _Atomic atomic_seq_LS;
		uint32_t _Atomic atomic_seq_U;
		uint32_t _Atomic _atomic_pad;
	};
} rwlock_seq;

_Static_assert(sizeof(rwlock_seq) == 4 * sizeof(uint32_t),
		"Incorrect rwlock_seq size");

typedef enum rwlock_seqfields {
	RWLOCK_SEQ_NONE = 0,
	RWLOCK_SEQ_LS = 1,
	RWLOCK_SEQ_U = 2,
	RWLOCK_SEQ_LSU = RWLOCK_SEQ_LS | RWLOCK_SEQ_U,
} rwlock_seqfields;

#if PTHREAD_DEBUG_LOG
#define RWLOCK_DEBUG_SEQ(op, rwlock, oldseq, newseq, updateval, f) \
		if (_pthread_debuglog >= 0) { \
		_simple_dprintf(_pthread_debuglog, "rw_" #op " %p tck %7llu thr %llx " \
		"L %x -> %x S %x -> %x U %x -> %x updt %x\n", rwlock, \
		mach_absolute_time() - _pthread_debugstart, _pthread_selfid_direct(), \
		(f) & RWLOCK_SEQ_LS ? (oldseq).lcntval : 0, \
		(f) & RWLOCK_SEQ_LS ? (newseq).lcntval : 0, \
		(f) & RWLOCK_SEQ_LS ? (oldseq).rw_seq  : 0, \
		(f) & RWLOCK_SEQ_LS ? (newseq).rw_seq  : 0, \
		(f) & RWLOCK_SEQ_U  ? (oldseq).ucntval : 0, \
		(f) & RWLOCK_SEQ_U  ? (newseq).ucntval : 0, updateval); }
#else
#define RWLOCK_DEBUG_SEQ(m, rwlock, oldseq, newseq, updateval, f)
#endif

#if !__LITTLE_ENDIAN__
#error RWLOCK_GETSEQ_ADDR assumes little endian layout of sequence words
#endif

PTHREAD_ALWAYS_INLINE
static inline void
RWLOCK_GETSEQ_ADDR(_pthread_rwlock *rwlock, rwlock_seq **seqaddr)
{
	// 128-bit aligned address inside rw_seq & rw_mis arrays
	*seqaddr = (void*)(((uintptr_t)rwlock->rw_seq + 0xful) & ~0xful);
}

PTHREAD_ALWAYS_INLINE
static inline void
RWLOCK_GETTID_ADDR(_pthread_rwlock *rwlock, uint64_t **tidaddr)
{
	// 64-bit aligned address inside rw_tid array (&rw_tid[0] for aligned lock)
	*tidaddr = (void*)(((uintptr_t)rwlock->rw_tid + 0x7ul) & ~0x7ul);
}

PTHREAD_ALWAYS_INLINE
static inline void
rwlock_seq_load(rwlock_seq *seqaddr, rwlock_seq *oldseqval,
		const rwlock_seqfields seqfields)
{
	switch (seqfields) {
	case RWLOCK_SEQ_LSU:
#if RWLOCK_USE_INT128
		oldseqval->seq_LSU = seqaddr->seq_LSU;
#else
		oldseqval->seq_LS = seqaddr->seq_LS;
		oldseqval->seq_U = seqaddr->seq_U;
#endif
		break;
	case RWLOCK_SEQ_LS:
		oldseqval->seq_LS = seqaddr->seq_LS;
		break;
#if DEBUG // unused
	case RWLOCK_SEQ_U:
		oldseqval->seq_U = seqaddr->seq_U;
		break;
#endif // unused
	default:
		__builtin_trap();
	}
}

PTHREAD_ALWAYS_INLINE
static inline void
rwlock_seq_atomic_load_relaxed(rwlock_seq *seqaddr, rwlock_seq *oldseqval,
		const rwlock_seqfields seqfields)
{
	switch (seqfields) {
	case RWLOCK_SEQ_LSU:
#if RWLOCK_USE_INT128
#if defined(__arm64__) && defined(__ARM_ARCH_8_2__)
		// Workaround clang armv81 codegen bug for 128bit os_atomic_load
		// rdar://problem/31213932
		oldseqval->seq_LSU = seqaddr->seq_LSU;
		while (!os_atomic_cmpxchgvw(&seqaddr->atomic_seq_LSU,
				oldseqval->seq_LSU, oldseqval->seq_LSU, &oldseqval->seq_LSU,
				relaxed));
#else
		oldseqval->seq_LSU = os_atomic_load(&seqaddr->atomic_seq_LSU, relaxed);
#endif
#else
		oldseqval->seq_LS = os_atomic_load(&seqaddr->atomic_seq_LS, relaxed);
		oldseqval->seq_U = os_atomic_load(&seqaddr->atomic_seq_U, relaxed);
#endif
		break;
	case RWLOCK_SEQ_LS:
		oldseqval->seq_LS = os_atomic_load(&seqaddr->atomic_seq_LS, relaxed);
		break;
#if DEBUG // unused
	case RWLOCK_SEQ_U:
		oldseqval->seq_U = os_atomic_load(&seqaddr->atomic_seq_U, relaxed);
		break;
#endif // unused
	default:
		__builtin_trap();
	}
}

#define rwlock_seq_atomic_load(seqaddr, oldseqval, seqfields, m) \
		rwlock_seq_atomic_load_##m(seqaddr, oldseqval, seqfields)

PTHREAD_ALWAYS_INLINE
static inline rwlock_seqfields
rwlock_seq_atomic_cmpxchgv_relaxed(rwlock_seq *seqaddr, rwlock_seq *oldseqval,
		rwlock_seq *newseqval, const rwlock_seqfields seqfields)
{
	bool r;
	rwlock_seqfields updated_seqfields = RWLOCK_SEQ_NONE;
	switch (seqfields) {
#if DEBUG // unused
	case RWLOCK_SEQ_LSU:
#if RWLOCK_USE_INT128
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LSU, oldseqval->seq_LSU,
				newseqval->seq_LSU, &oldseqval->seq_LSU, relaxed);
		if (r) updated_seqfields = RWLOCK_SEQ_LSU;
#else
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LS, oldseqval->seq_LS,
				newseqval->seq_LS, &oldseqval->seq_LS, relaxed);
		if (r) {
			r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_U, oldseqval->seq_U,
					newseqval->seq_U, &oldseqval->seq_U, relaxed);
			if (!r) oldseqval->seq_LS = newseqval->seq_LS;
			updated_seqfields = r ? RWLOCK_SEQ_LSU : RWLOCK_SEQ_LS;
		} else {
			oldseqval->seq_U = os_atomic_load(&seqaddr->atomic_seq_U, relaxed);
		}
#endif
		break;
	case RWLOCK_SEQ_U:
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_U, oldseqval->seq_U,
				newseqval->seq_U, &oldseqval->seq_U, relaxed);
		if (r) updated_seqfields = RWLOCK_SEQ_U;
		break;
#endif // unused
	case RWLOCK_SEQ_LS:
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LS, oldseqval->seq_LS,
				newseqval->seq_LS, &oldseqval->seq_LS, relaxed);
		if (r) updated_seqfields = RWLOCK_SEQ_LS;
		break;
	default:
		__builtin_trap();
	}
	return updated_seqfields;
}

PTHREAD_ALWAYS_INLINE
static inline rwlock_seqfields
rwlock_seq_atomic_cmpxchgv_acquire(rwlock_seq *seqaddr, rwlock_seq *oldseqval,
		rwlock_seq *newseqval, const rwlock_seqfields seqfields)
{
	bool r;
	rwlock_seqfields updated_seqfields = RWLOCK_SEQ_NONE;
	switch (seqfields) {
#if DEBUG // unused
	case RWLOCK_SEQ_LSU:
#if RWLOCK_USE_INT128
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LSU, oldseqval->seq_LSU,
				newseqval->seq_LSU, &oldseqval->seq_LSU, acquire);
		if (r) updated_seqfields = RWLOCK_SEQ_LSU;
#else
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LS, oldseqval->seq_LS,
				newseqval->seq_LS, &oldseqval->seq_LS, acquire);
		if (r) {
			r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_U, oldseqval->seq_U,
					newseqval->seq_U, &oldseqval->seq_U, relaxed);
			if (!r) oldseqval->seq_LS = newseqval->seq_LS;
			updated_seqfields = r ? RWLOCK_SEQ_LSU : RWLOCK_SEQ_LS;
		} else {
			oldseqval->seq_U = os_atomic_load(&seqaddr->atomic_seq_U, relaxed);
		}
#endif
		break;
	case RWLOCK_SEQ_U:
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_U, oldseqval->seq_U,
				newseqval->seq_U, &oldseqval->seq_U, acquire);
		if (r) updated_seqfields = RWLOCK_SEQ_U;
		break;
#endif // unused
	case RWLOCK_SEQ_LS:
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LS, oldseqval->seq_LS,
				newseqval->seq_LS, &oldseqval->seq_LS, acquire);
		if (r) updated_seqfields = RWLOCK_SEQ_LS;
		break;
	default:
		__builtin_trap();
	}
	return updated_seqfields;
}

PTHREAD_ALWAYS_INLINE
static inline rwlock_seqfields
rwlock_seq_atomic_cmpxchgv_release(rwlock_seq *seqaddr, rwlock_seq *oldseqval,
		rwlock_seq *newseqval, const rwlock_seqfields seqfields)
{
	bool r;
	rwlock_seqfields updated_seqfields = RWLOCK_SEQ_NONE;
	switch (seqfields) {
	case RWLOCK_SEQ_LSU:
#if RWLOCK_USE_INT128
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LSU, oldseqval->seq_LSU,
				newseqval->seq_LSU, &oldseqval->seq_LSU, release);
		if (r) updated_seqfields = RWLOCK_SEQ_LSU;
#else
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_U, oldseqval->seq_U,
				newseqval->seq_U, &oldseqval->seq_U, release);
		if (r) {
			r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LS, oldseqval->seq_LS,
					newseqval->seq_LS, &oldseqval->seq_LS, relaxed);
			if (!r) oldseqval->seq_U = newseqval->seq_U;
			updated_seqfields = r ? RWLOCK_SEQ_LSU : RWLOCK_SEQ_U;
		} else {
			oldseqval->seq_LS = os_atomic_load(&seqaddr->atomic_seq_LS,relaxed);
		}
#endif
		break;
	case RWLOCK_SEQ_LS:
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_LS, oldseqval->seq_LS,
				newseqval->seq_LS, &oldseqval->seq_LS, release);
		if (r) updated_seqfields = RWLOCK_SEQ_LS;
		break;
#if DEBUG // unused
	case RWLOCK_SEQ_U:
		r = os_atomic_cmpxchgv(&seqaddr->atomic_seq_U, oldseqval->seq_U,
				newseqval->seq_U, &oldseqval->seq_U, release);
		if (r) updated_seqfields = RWLOCK_SEQ_U;
		break;
#endif // unused
	default:
		__builtin_trap();
	}
	return updated_seqfields;
}

#define rwlock_seq_atomic_cmpxchgv(seqaddr, oldseqval, newseqval, seqfields, m)\
		rwlock_seq_atomic_cmpxchgv_##m(seqaddr, oldseqval, newseqval, seqfields)

#ifndef BUILDING_VARIANT /* [ */

int
pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
	attr->sig = _PTHREAD_RWLOCK_ATTR_SIG;
	attr->pshared = _PTHREAD_DEFAULT_PSHARED;
	return 0;
}

int
pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
	attr->sig = _PTHREAD_NO_SIG;
	attr->pshared = 0;
	return 0;
}

int
pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_RWLOCK_ATTR_SIG) {
		*pshared = (int)attr->pshared;
		res = 0;
	}
	return res;
}

int
pthread_rwlockattr_setpshared(pthread_rwlockattr_t * attr, int pshared)
{
	int res = EINVAL;
	if (attr->sig == _PTHREAD_RWLOCK_ATTR_SIG) {
#if __DARWIN_UNIX03
		if (( pshared == PTHREAD_PROCESS_PRIVATE) ||
				(pshared == PTHREAD_PROCESS_SHARED))
#else /* __DARWIN_UNIX03 */
		if ( pshared == PTHREAD_PROCESS_PRIVATE)
#endif /* __DARWIN_UNIX03 */
		{
			attr->pshared = pshared ;
			res = 0;
		}
	}
	return res;
}

#endif /* !BUILDING_VARIANT ] */

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_rwlock_init(_pthread_rwlock *rwlock, const pthread_rwlockattr_t *attr)
{
	uint64_t *tidaddr;
	RWLOCK_GETTID_ADDR(rwlock, &tidaddr);

	rwlock_seq *seqaddr;
	RWLOCK_GETSEQ_ADDR(rwlock, &seqaddr);

#if PTHREAD_RWLOCK_INIT_UNUSED
	if ((uint32_t*)tidaddr != rwlock->rw_tid) {
		rwlock->misalign = 1;
		__builtin_memset(rwlock->rw_tid, 0xff, sizeof(rwlock->rw_tid));
	}
	if ((uint32_t*)seqaddr != rwlock->rw_seq) {
		__builtin_memset(rwlock->rw_seq, 0xff, sizeof(rwlock->rw_seq));
	}
	__builtin_memset(rwlock->rw_mis, 0xff, sizeof(rwlock->rw_mis));
#endif // PTHREAD_MUTEX_INIT_UNUSED
	*tidaddr = 0;
	*seqaddr = (rwlock_seq){
		.lcntval = PTHRW_RWLOCK_INIT,
		.rw_seq = PTHRW_RWS_INIT,
		.ucntval = 0,
	};

	if (attr != NULL && attr->pshared == PTHREAD_PROCESS_SHARED) {
		rwlock->pshared = PTHREAD_PROCESS_SHARED;
		rwlock->rw_flags = PTHRW_KERN_PROCESS_SHARED;
	} else {
		rwlock->pshared = _PTHREAD_DEFAULT_PSHARED;
		rwlock->rw_flags = PTHRW_KERN_PROCESS_PRIVATE;
	}

	long sig = _PTHREAD_RWLOCK_SIG;

#if DEBUG
	bzero(rwlock->_reserved, sizeof(rwlock->_reserved));
#endif
#if PTHREAD_RWLOCK_INIT_UNUSED
	// For detecting copied rwlocks and smashes during debugging
	uint32_t sig32 = (uint32_t)sig;
	uintptr_t guard = ~(uintptr_t)rwlock; // use ~ to hide from leaks
	__builtin_memcpy(rwlock->_reserved, &guard, sizeof(guard));
#define countof(x) (sizeof(x) / sizeof(x[0]))
	rwlock->_reserved[countof(rwlock->_reserved) - 1] = sig32;
#if defined(__LP64__)
	rwlock->_pad = sig32;
#endif
#endif // PTHREAD_RWLOCK_INIT_UNUSED

	// Ensure all contents are properly set before setting signature.
#if defined(__LP64__)
	// For binary compatibility reasons we cannot require natural alignment of
	// the 64bit 'sig' long value in the struct. rdar://problem/21610439
	uint32_t *sig32_ptr = (uint32_t*)&rwlock->sig;
	uint32_t *sig32_val = (uint32_t*)&sig;
	*(sig32_ptr + 1) = *(sig32_val + 1);
	os_atomic_store(sig32_ptr, *sig32_val, release);
#else
	os_atomic_store2o(rwlock, sig, sig, release);
#endif

	return 0;
}

static uint32_t
_pthread_rwlock_modbits(uint32_t lgenval, uint32_t updateval, uint32_t savebits)
{
	uint32_t lval = lgenval & PTHRW_BIT_MASK;
	uint32_t uval = updateval & PTHRW_BIT_MASK;
	uint32_t rval, nlval;

	nlval = (lval | uval) & ~(PTH_RWL_MBIT);

	// reconcile bits on the lock with what kernel needs to set
	if ((uval & PTH_RWL_KBIT) == 0 && (lval & PTH_RWL_WBIT) == 0) {
		nlval &= ~PTH_RWL_KBIT;
	}

	if (savebits != 0) {
		if ((savebits & PTH_RWS_WSVBIT) != 0 && (nlval & PTH_RWL_WBIT) == 0 &&
				(nlval & PTH_RWL_EBIT) == 0) {
			nlval |= (PTH_RWL_WBIT | PTH_RWL_KBIT);
		}
	}
	rval = (lgenval & PTHRW_COUNT_MASK) | nlval;
	return(rval);
}

PTHREAD_ALWAYS_INLINE
static inline void
_pthread_rwlock_updateval(_pthread_rwlock *rwlock, uint32_t updateval)
{
	bool isoverlap = (updateval & PTH_RWL_MBIT) != 0;

	// TBD: restore U bit
	rwlock_seq *seqaddr;
	RWLOCK_GETSEQ_ADDR(rwlock, &seqaddr);

	rwlock_seq oldseq, newseq;
	rwlock_seq_load(seqaddr, &oldseq, RWLOCK_SEQ_LS);
	do {
		newseq = oldseq;
		if (isoverlap || is_rws_unlockinit_set(oldseq.rw_seq)) {
			// Set S word to the specified value
			uint32_t savebits = (oldseq.rw_seq & PTHRW_RWS_SAVEMASK);
			newseq.lcntval = _pthread_rwlock_modbits(oldseq.lcntval, updateval,
					savebits);
			newseq.rw_seq += (updateval & PTHRW_COUNT_MASK);
			if (!isoverlap) {
				newseq.rw_seq &= PTHRW_COUNT_MASK;
			}
			newseq.rw_seq &= ~PTHRW_RWS_SAVEMASK;
		}
	} while (!rwlock_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			RWLOCK_SEQ_LS, relaxed));
	RWLOCK_DEBUG_SEQ(update, rwlock, oldseq, newseq, updateval, RWLOCK_SEQ_LS);
}

#if __DARWIN_UNIX03
PTHREAD_ALWAYS_INLINE
static inline int
_pthread_rwlock_check_busy(_pthread_rwlock *rwlock)
{
	int res = 0;

	rwlock_seq *seqaddr;
	RWLOCK_GETSEQ_ADDR(rwlock, &seqaddr);

	rwlock_seq seq;
	rwlock_seq_atomic_load(seqaddr, &seq, RWLOCK_SEQ_LSU, relaxed);
	if ((seq.lcntval & PTHRW_COUNT_MASK) != seq.ucntval) {
		res = EBUSY;
	}

	return res;
}
#endif /* __DARWIN_UNIX03 */

PTHREAD_NOEXPORT_VARIANT
int
pthread_rwlock_destroy(pthread_rwlock_t *orwlock)
{
	int res = 0;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;

	_PTHREAD_LOCK(rwlock->lock);
	if (_pthread_rwlock_check_signature(rwlock)) {
#if __DARWIN_UNIX03
		res = _pthread_rwlock_check_busy(rwlock);
#endif /* __DARWIN_UNIX03 */
	} else if (!_pthread_rwlock_check_signature_init(rwlock)) {
		res = EINVAL;
	}
	if (res == 0) {
		rwlock->sig = _PTHREAD_NO_SIG;
	}
	_PTHREAD_UNLOCK(rwlock->lock);
	return res;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_rwlock_init(pthread_rwlock_t *orwlock, const pthread_rwlockattr_t *attr)
{
	int res = 0;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;

#if __DARWIN_UNIX03
	if (attr && attr->sig != _PTHREAD_RWLOCK_ATTR_SIG) {
		res = EINVAL;
	}

	if (res == 0 && _pthread_rwlock_check_signature(rwlock)) {
		res = _pthread_rwlock_check_busy(rwlock);
	}
#endif
	if (res == 0) {
		_PTHREAD_LOCK_INIT(rwlock->lock);
		res = _pthread_rwlock_init(rwlock, attr);
	}
	return res;
}

PTHREAD_NOINLINE
static int
_pthread_rwlock_check_init_slow(pthread_rwlock_t *orwlock)
{
	int res = EINVAL;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;

	if (_pthread_rwlock_check_signature_init(rwlock)) {
		_PTHREAD_LOCK(rwlock->lock);
		if (_pthread_rwlock_check_signature_init(rwlock)) {
			res = _pthread_rwlock_init(rwlock, NULL);
		} else if (_pthread_rwlock_check_signature(rwlock)){
			res = 0;
		}
		_PTHREAD_UNLOCK(rwlock->lock);
	} else if (_pthread_rwlock_check_signature(rwlock)){
		res = 0;
	}
	if (res != 0) {
		PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, res);
	}
	return res;
}

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_rwlock_check_init(pthread_rwlock_t *orwlock)
{
	int res = 0;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;

	if (!_pthread_rwlock_check_signature(rwlock)) {
		return _pthread_rwlock_check_init_slow(orwlock);
	}
	return res;
}

PTHREAD_NOINLINE
static int
_pthread_rwlock_lock_wait(pthread_rwlock_t *orwlock, bool readlock,
		rwlock_seq newseq)
{
	int res;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;

#ifdef PLOCKSTAT
	int plockstat = readlock ? READ_LOCK_PLOCKSTAT : WRITE_LOCK_PLOCKSTAT;
#endif

	if (readlock) {
		RWLOCK_DEBUG_SEQ(rdlock, rwlock, oldseq, newseq, gotlock,
				RWLOCK_SEQ_LSU);
	} else {
		RWLOCK_DEBUG_SEQ(wrlock, rwlock, oldseq, newseq, gotlock,
				RWLOCK_SEQ_LSU);
	}

	uint32_t updateval;

	PLOCKSTAT_RW_BLOCK(orwlock, plockstat);

	do {
		if (readlock) {
			updateval = __psynch_rw_rdlock(orwlock, newseq.lcntval,
					newseq.ucntval, newseq.rw_seq, rwlock->rw_flags);
		} else {
			updateval = __psynch_rw_wrlock(orwlock, newseq.lcntval,
					newseq.ucntval, newseq.rw_seq, rwlock->rw_flags);
		}
		if (updateval == (uint32_t)-1) {
			res = errno;
		} else {
			res = 0;
		}
	} while (res == EINTR);

	if (res == 0) {
		_pthread_rwlock_updateval(rwlock, updateval);
		PLOCKSTAT_RW_BLOCKED(orwlock, plockstat, BLOCK_SUCCESS_PLOCKSTAT);
	} else {
		PLOCKSTAT_RW_BLOCKED(orwlock, plockstat, BLOCK_FAIL_PLOCKSTAT);
		PTHREAD_INTERNAL_CRASH(res, "kernel rwlock returned unknown error");
	}

	return res;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_rwlock_lock_slow(pthread_rwlock_t *orwlock, bool readlock,
		bool trylock)
{
	int res;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;

#ifdef PLOCKSTAT
	int plockstat = readlock ? READ_LOCK_PLOCKSTAT : WRITE_LOCK_PLOCKSTAT;
#endif

	res = _pthread_rwlock_check_init(orwlock);
	if (res != 0) return res;

	rwlock_seq *seqaddr;
	RWLOCK_GETSEQ_ADDR(rwlock, &seqaddr);

	rwlock_seq oldseq, newseq;
	rwlock_seq_atomic_load(seqaddr, &oldseq, RWLOCK_SEQ_LSU, relaxed);

#if __DARWIN_UNIX03
	uint64_t *tidaddr;
	RWLOCK_GETTID_ADDR(rwlock, &tidaddr);
	uint64_t selfid = _pthread_selfid_direct();
	if (is_rwl_ebit_set(oldseq.lcntval)) {
		if (os_atomic_load(tidaddr, relaxed) == selfid) return EDEADLK;
	}
#endif /* __DARWIN_UNIX03 */

	int retry_count;
	bool gotlock;
	do {
		retry_count = 0;
retry:
		newseq = oldseq;

		// if W and K bit are clear or U bit is on, acquire lock in userland
		if (readlock) {
			gotlock = (oldseq.lcntval & (PTH_RWL_WBIT | PTH_RWL_KBIT)) == 0;
		} else {
			gotlock = (oldseq.lcntval & PTH_RWL_UBIT) != 0;
		}

		if (trylock && !gotlock) {
			// A trylock on a held lock will fail immediately. But since
			// we did not load the sequence words atomically, perform a
			// no-op CAS to ensure that nobody has unlocked concurrently.
		} else if (gotlock) {
			if (readlock) {
				if (diff_genseq(oldseq.lcntval, oldseq.ucntval) >=
						PTHRW_MAX_READERS) {
					// since ucntval may be newer, just redo
					retry_count++;
					if (retry_count > 1024) {
						gotlock = false;
						res = EAGAIN;
						goto out;
					} else {
						sched_yield();
						rwlock_seq_atomic_load(seqaddr, &oldseq,
								RWLOCK_SEQ_LSU, relaxed);
						goto retry;
					}
				}
				// Need to update L (remove U bit) and S word
				newseq.lcntval &= ~PTH_RWL_UBIT;
			} else {
				newseq.lcntval &= PTHRW_COUNT_MASK;
				newseq.lcntval |= PTH_RWL_IBIT | PTH_RWL_KBIT | PTH_RWL_EBIT;
			}
			newseq.lcntval += PTHRW_INC;
			newseq.rw_seq  += PTHRW_INC;
		} else {
			if (readlock) {
				// Need to block in kernel. Remove U bit.
				newseq.lcntval &= ~PTH_RWL_UBIT;
			} else {
				newseq.lcntval |= PTH_RWL_KBIT | PTH_RWL_WBIT;
			}
			newseq.lcntval += PTHRW_INC;
			if (is_rws_sbit_set(oldseq.rw_seq)) {
				// Clear the S bit and set S to L
				newseq.rw_seq &= (PTHRW_BIT_MASK & ~PTH_RWS_SBIT);
				newseq.rw_seq |= (oldseq.lcntval & PTHRW_COUNT_MASK);
			}
		}
	} while (!rwlock_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			RWLOCK_SEQ_LS, acquire));

	if (gotlock) {
#if __DARWIN_UNIX03
		if (!readlock) os_atomic_store(tidaddr, selfid, relaxed);
#endif /* __DARWIN_UNIX03 */
		res = 0;
	} else if (trylock) {
		res = EBUSY;
	} else {
		res = _pthread_rwlock_lock_wait(orwlock, readlock, newseq);
	}

out:
#ifdef PLOCKSTAT
	if (res == 0) {
		PLOCKSTAT_RW_ACQUIRE(orwlock, plockstat);
	} else {
		PLOCKSTAT_RW_ERROR(orwlock, plockstat, res);
	}
#endif

	return res;
}

PTHREAD_ALWAYS_INLINE
static inline int
_pthread_rwlock_lock(pthread_rwlock_t *orwlock, bool readlock, bool trylock)
{
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;
#if PLOCKSTAT
	if (PLOCKSTAT_RW_ACQUIRE_ENABLED() || PLOCKSTAT_RW_ERROR_ENABLED()) {
		return _pthread_rwlock_lock_slow(orwlock, readlock, trylock);
	}
#endif

	if (os_unlikely(!_pthread_rwlock_check_signature(rwlock))) {
		return _pthread_rwlock_lock_slow(orwlock, readlock, trylock);
	}

	rwlock_seq *seqaddr;
	RWLOCK_GETSEQ_ADDR(rwlock, &seqaddr);

	rwlock_seq oldseq, newseq;
	// no need to perform a single-copy-atomic 128-bit load in the fastpath,
	// if stores to L and U are seen out of order, we will fallback to the
	// slowpath below (which has rwlock_seq_atomic_load)
	rwlock_seq_load(seqaddr, &oldseq, RWLOCK_SEQ_LSU);

#if __DARWIN_UNIX03
	if (os_unlikely(is_rwl_ebit_set(oldseq.lcntval))) {
		return _pthread_rwlock_lock_slow(orwlock, readlock, trylock);
	}
#endif /* __DARWIN_UNIX03 */

	bool gotlock;
	do {
		newseq = oldseq;

		// if W and K bit are clear or U bit is on, acquire lock in userland
		if (readlock) {
			gotlock = (oldseq.lcntval & (PTH_RWL_WBIT | PTH_RWL_KBIT)) == 0;
		} else {
			gotlock = (oldseq.lcntval & PTH_RWL_UBIT) != 0;
		}

		if (trylock && !gotlock) {
			// A trylock on a held lock will fail immediately. But since
			// we did not load the sequence words atomically, perform a
			// no-op CAS to ensure that nobody has unlocked concurrently.
		} else if (os_likely(gotlock)) {
			if (readlock) {
				if (os_unlikely(diff_genseq(oldseq.lcntval, oldseq.ucntval) >=
						PTHRW_MAX_READERS)) {
					return _pthread_rwlock_lock_slow(orwlock, readlock,trylock);
				}
				// Need to update L (remove U bit) and S word
				newseq.lcntval &= ~PTH_RWL_UBIT;
			} else {
				newseq.lcntval &= PTHRW_COUNT_MASK;
				newseq.lcntval |= PTH_RWL_IBIT | PTH_RWL_KBIT | PTH_RWL_EBIT;
			}
			newseq.lcntval += PTHRW_INC;
			newseq.rw_seq  += PTHRW_INC;
		} else {
			return _pthread_rwlock_lock_slow(orwlock, readlock, trylock);
		}
	} while (os_unlikely(!rwlock_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq,
			RWLOCK_SEQ_LS, acquire)));

	if (os_likely(gotlock)) {
#if __DARWIN_UNIX03
		if (!readlock) {
			uint64_t *tidaddr;
			RWLOCK_GETTID_ADDR(rwlock, &tidaddr);
			uint64_t selfid = _pthread_selfid_direct();
			os_atomic_store(tidaddr, selfid, relaxed);
		}
#endif /* __DARWIN_UNIX03 */
		return 0;
	} else if (trylock) {
		return EBUSY;
	} else {
		__builtin_trap();
	}
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_rwlock_rdlock(pthread_rwlock_t *orwlock)
{
	// read lock, no try
	return _pthread_rwlock_lock(orwlock, true, false);
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_rwlock_tryrdlock(pthread_rwlock_t *orwlock)
{
	// read lock, try lock
	return _pthread_rwlock_lock(orwlock, true, true);
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_rwlock_wrlock(pthread_rwlock_t *orwlock)
{
	// write lock, no try
	return _pthread_rwlock_lock(orwlock, false, false);
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_rwlock_trywrlock(pthread_rwlock_t *orwlock)
{
	// write lock, try lock
	return _pthread_rwlock_lock(orwlock, false, true);
}

PTHREAD_NOINLINE
static int
_pthread_rwlock_unlock_drop(pthread_rwlock_t *orwlock, rwlock_seq oldseq,
		rwlock_seq newseq)
{
	int res;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;

	RWLOCK_DEBUG_SEQ(unlock, rwlock, oldseq, newseq, !droplock, RWLOCK_SEQ_LSU);
	uint32_t updateval;
	do {
		updateval = __psynch_rw_unlock(orwlock, oldseq.lcntval,
				newseq.ucntval, newseq.rw_seq, rwlock->rw_flags);
		if (updateval == (uint32_t)-1) {
			res = errno;
		} else {
			res = 0;
			RWLOCK_DEBUG_SEQ(wakeup, rwlock, oldseq, newseq, updateval,
					RWLOCK_SEQ_LSU);
		}
	} while (res == EINTR);

	if (res != 0) {
		PTHREAD_INTERNAL_CRASH(res, "kernel rwunlock returned unknown error");
	}

	return res;
}

PTHREAD_NOEXPORT PTHREAD_NOINLINE
int
_pthread_rwlock_unlock_slow(pthread_rwlock_t *orwlock,
		rwlock_seqfields updated_seqfields)
{
	int res;
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;
	rwlock_seqfields seqfields = RWLOCK_SEQ_LSU;
#ifdef PLOCKSTAT
	int wrlock = 0;
#endif

	res = _pthread_rwlock_check_init(orwlock);
	if (res != 0) return res;

	rwlock_seq *seqaddr;
	RWLOCK_GETSEQ_ADDR(rwlock, &seqaddr);

	rwlock_seq oldseq, newseq;
	rwlock_seq_load(seqaddr, &oldseq, seqfields);

	if ((oldseq.lcntval & PTH_RWL_UBIT) != 0) {
		// spurious unlock (unlock of unlocked lock)
		return 0;
	}

	if (is_rwl_ebit_set(oldseq.lcntval)) {
#ifdef PLOCKSTAT
		wrlock = 1;
#endif
#if __DARWIN_UNIX03
		uint64_t *tidaddr;
		RWLOCK_GETTID_ADDR(rwlock, &tidaddr);
		os_atomic_store(tidaddr, 0, relaxed);
#endif /* __DARWIN_UNIX03 */
	}

	bool droplock;
	do {
		// stop loading & updating fields that have successfully been stored
		seqfields &= ~updated_seqfields;

		newseq = oldseq;
		if (seqfields & RWLOCK_SEQ_U) {
			newseq.ucntval += PTHRW_INC;
		}

		droplock = false;
		uint32_t oldlcnt = (oldseq.lcntval & PTHRW_COUNT_MASK);
		if (newseq.ucntval == oldlcnt) {
			// last unlock, set L with U and init bits and set S to L with S bit
			newseq.lcntval = oldlcnt | PTHRW_RWLOCK_INIT;
			newseq.rw_seq =  oldlcnt | PTHRW_RWS_INIT;
		} else {
			// no L/S update if lock is not exclusive or no writer pending
			if ((oldseq.lcntval &
					(PTH_RWL_EBIT | PTH_RWL_WBIT | PTH_RWL_KBIT)) == 0) {
				continue;
			}

			// kernel transition only needed if U == S
			if (newseq.ucntval != (oldseq.rw_seq & PTHRW_COUNT_MASK)) {
				continue;
			}

			droplock = true;
			// reset all bits and set K
			newseq.lcntval = oldlcnt | PTH_RWL_KBIT;
			// set I bit on S word
			newseq.rw_seq |= PTH_RWS_IBIT;
			if ((oldseq.lcntval & PTH_RWL_WBIT) != 0) {
				newseq.rw_seq |= PTH_RWS_WSVBIT;
			}
		}
	} while (seqfields != (updated_seqfields = rwlock_seq_atomic_cmpxchgv(
			seqaddr, &oldseq, &newseq, seqfields, release)));

	if (droplock) {
		res = _pthread_rwlock_unlock_drop(orwlock, oldseq, newseq);
	}

	PLOCKSTAT_RW_RELEASE(orwlock, wrlock);

	return res;
}

PTHREAD_NOEXPORT_VARIANT
int
pthread_rwlock_unlock(pthread_rwlock_t *orwlock)
{
	_pthread_rwlock *rwlock = (_pthread_rwlock *)orwlock;
	rwlock_seqfields seqfields = RWLOCK_SEQ_LSU;
	rwlock_seqfields updated_seqfields = RWLOCK_SEQ_NONE;

#if PLOCKSTAT
	if (PLOCKSTAT_RW_RELEASE_ENABLED() || PLOCKSTAT_RW_ERROR_ENABLED()) {
		return _pthread_rwlock_unlock_slow(orwlock, updated_seqfields);
	}
#endif

	if (os_unlikely(!_pthread_rwlock_check_signature(rwlock))) {
		return _pthread_rwlock_unlock_slow(orwlock, updated_seqfields);
	}

	rwlock_seq *seqaddr;
	RWLOCK_GETSEQ_ADDR(rwlock, &seqaddr);

	rwlock_seq oldseq, newseq;
	rwlock_seq_load(seqaddr, &oldseq, seqfields);

	if (os_unlikely(oldseq.lcntval & PTH_RWL_UBIT)) {
		// spurious unlock (unlock of unlocked lock)
		return 0;
	}

	if (is_rwl_ebit_set(oldseq.lcntval)) {
#if __DARWIN_UNIX03
		uint64_t *tidaddr;
		RWLOCK_GETTID_ADDR(rwlock, &tidaddr);
		os_atomic_store(tidaddr, 0, relaxed);
#endif /* __DARWIN_UNIX03 */
	}

	do {
		if (updated_seqfields) {
			return _pthread_rwlock_unlock_slow(orwlock, updated_seqfields);
		}

		newseq = oldseq;
		if (seqfields & RWLOCK_SEQ_U) {
			newseq.ucntval += PTHRW_INC;
		}

		uint32_t oldlcnt = (oldseq.lcntval & PTHRW_COUNT_MASK);
		if (os_likely(newseq.ucntval == oldlcnt)) {
			// last unlock, set L with U and init bits and set S to L with S bit
			newseq.lcntval = oldlcnt | PTHRW_RWLOCK_INIT;
			newseq.rw_seq =  oldlcnt | PTHRW_RWS_INIT;
		} else {
			if (os_likely((oldseq.lcntval &
					(PTH_RWL_EBIT | PTH_RWL_WBIT | PTH_RWL_KBIT)) == 0 ||
					newseq.ucntval != (oldseq.rw_seq & PTHRW_COUNT_MASK))) {
				// no L/S update if lock is not exclusive or no writer pending
				// kernel transition only needed if U == S
			} else {
				return _pthread_rwlock_unlock_slow(orwlock, updated_seqfields);
			}
		}
	} while (os_unlikely(seqfields != (updated_seqfields =
			rwlock_seq_atomic_cmpxchgv(seqaddr, &oldseq, &newseq, seqfields,
			release))));

	return 0;
}

