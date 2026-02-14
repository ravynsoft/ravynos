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
 * POSIX Threads - IEEE 1003.1c
 */

#ifndef _POSIX_PTHREAD_INTERNALS_H
#define _POSIX_PTHREAD_INTERNALS_H

#define _PTHREAD_BUILDING_PTHREAD_

// suppress pthread_attr_t typedef in sys/signal.h
#define _PTHREAD_ATTR_T
struct _pthread_attr_t; /* forward reference */
typedef struct _pthread_attr_t pthread_attr_t;

#include <_simple.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <TargetConditionals.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <sys/queue.h>
#include <sys/reason.h>
#include <pthread/bsdthread_private.h>
#include <pthread/workqueue_syscalls.h>

#define __OS_EXPOSE_INTERNALS__ 1
#include <os/internal/internal_shared.h>
#include <os/once_private.h>
#include <os/reason_private.h>

#if TARGET_IPHONE_SIMULATOR
#error Unsupported target
#endif


#define PTHREAD_INTERNAL_CRASH(c, x) do { \
		_os_set_crash_log_cause_and_message((c), \
				"BUG IN LIBPTHREAD: " x); \
		__builtin_trap(); \
	} while (0)

#define PTHREAD_CLIENT_CRASH(c, x) do { \
		_os_set_crash_log_cause_and_message((c), \
				"BUG IN CLIENT OF LIBPTHREAD: " x); \
		__builtin_trap(); \
	} while (0)

#ifndef __POSIX_LIB__
#define __POSIX_LIB__
#endif

#ifndef PTHREAD_LAYOUT_SPI
#define PTHREAD_LAYOUT_SPI 1
#endif

#include "posix_sched.h"
#include "tsd_private.h"
#include "spinlock_private.h"

#define PTHREAD_EXPORT extern __attribute__((visibility("default")))
#define PTHREAD_EXTERN extern
#define PTHREAD_NOEXPORT __attribute__((visibility("hidden")))
#define PTHREAD_NOEXPORT_VARIANT
#define PTHREAD_NORETURN __attribute__((__noreturn__))
#define PTHREAD_ALWAYS_INLINE __attribute__((always_inline))
#define PTHREAD_NOINLINE __attribute__((noinline))
#define PTHREAD_WEAK __attribute__((weak))
#define PTHREAD_USED __attribute__((used))
#define PTHREAD_NOT_TAIL_CALLED __attribute__((__not_tail_called__))


#define OS_UNFAIR_LOCK_INLINE 1
#include <os/lock_private.h>
typedef os_unfair_lock _pthread_lock;
#define _PTHREAD_LOCK_INITIALIZER OS_UNFAIR_LOCK_INIT
#define _PTHREAD_LOCK_INIT(lock) ((lock) = (_pthread_lock)_PTHREAD_LOCK_INITIALIZER)
#define _PTHREAD_LOCK(lock) os_unfair_lock_lock_with_options_inline(&(lock), OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION)
#define _PTHREAD_LOCK_FROM_MACH_THREAD(lock) os_unfair_lock_lock_inline_no_tsd_4libpthread(&(lock))
#define _PTHREAD_UNLOCK(lock) os_unfair_lock_unlock_inline(&(lock))
#define _PTHREAD_UNLOCK_FROM_MACH_THREAD(lock) os_unfair_lock_unlock_inline_no_tsd_4libpthread(&(lock))

#define _PTHREAD_POLICY_IS_FIXEDPRI(x) ((x) == SCHED_RR || (x) == SCHED_FIFO)

extern int __is_threaded;
extern int __unix_conforming;
PTHREAD_NOEXPORT
extern uintptr_t _pthread_ptr_munge_token;

// List of all pthreads in the process.
TAILQ_HEAD(__pthread_list, _pthread);
PTHREAD_NOEXPORT extern struct __pthread_list __pthread_head;

// Lock protects access to above list.
PTHREAD_NOEXPORT extern _pthread_lock _pthread_list_lock;

PTHREAD_NOEXPORT extern uint32_t _main_qos;

#if PTHREAD_DEBUG_LOG
#include <mach/mach_time.h>
PTHREAD_NOEXPORT extern int _pthread_debuglog;
PTHREAD_NOEXPORT extern uint64_t _pthread_debugstart;
#endif

/*
 * Compiled-in limits
 */
#if TARGET_OS_EMBEDDED
#define _EXTERNAL_POSIX_THREAD_KEYS_MAX 256
#define _INTERNAL_POSIX_THREAD_KEYS_MAX 256
#define _INTERNAL_POSIX_THREAD_KEYS_END 512
#else
#define _EXTERNAL_POSIX_THREAD_KEYS_MAX 512
#define _INTERNAL_POSIX_THREAD_KEYS_MAX 256
#define _INTERNAL_POSIX_THREAD_KEYS_END 768
#endif

#if defined(__arm64__)
/* Pull the pthread_t into the same page as the top of the stack so we dirty one less page.
 * <rdar://problem/19941744> The _pthread struct at the top of the stack shouldn't be page-aligned
 */
#define PTHREAD_T_OFFSET (12*1024)
#else
#define PTHREAD_T_OFFSET 0
#endif

#define MAXTHREADNAMESIZE	64
#define _PTHREAD_T
typedef struct _pthread {
	//
	// ABI - These fields are externally known as struct _opaque_pthread_t.
	//
	long sig;
	struct __darwin_pthread_handler_rec *__cleanup_stack;

	//
	// SPI - These fields are private.
	//

	//
	// Fields protected by _pthread_list_lock
	//

	TAILQ_ENTRY(_pthread) tl_plist;	// global thread list [aligned]
	struct pthread_join_context_s *tl_join_ctx;
	void *tl_exit_value;
	uint32_t tl_policy:8,
			tl_joinable:1,
			tl_joiner_cleans_up:1,
			tl_has_custom_stack:1,
			__tl_pad:21;
	// MACH_PORT_NULL if no joiner
	// tsd[_PTHREAD_TSD_SLOT_MACH_THREAD_SELF] when has a joiner
	// MACH_PORT_DEAD if the thread exited
	uint32_t tl_exit_gate;
	struct sched_param tl_param;
	void *__unused_padding;

	//
	// Fields protected by pthread_t::lock
	//

	_pthread_lock lock;
	uint16_t max_tsd_key;
	uint16_t inherit:8,
			kernalloc:1,
			schedset:1,
			wqthread:1,
			wqkillset:1,
			__flags_pad:4;

	char pthread_name[MAXTHREADNAMESIZE];	// includes NUL [aligned]

	void *(*fun)(void *);	// thread start routine
	void *arg;				// thread start routine argument
	int   wq_nevents;		// wqthreads (workloop / kevent)
	bool  wq_outsideqos;
	uint8_t canceled;		// 4597450 set if conformant cancelation happened
	uint16_t cancel_state;	// whether the thread can be canceled [atomic]
	errno_t cancel_error;
	errno_t err_no;			// thread-local errno

	void *stackaddr;		// base of the stack (page aligned)
	void *stackbottom;		// stackaddr - stacksize
	void *freeaddr;			// stack/thread allocation base address
	size_t freesize;		// stack/thread allocation size
	size_t guardsize;		// guard page size in bytes

	// tsd-base relative accessed elements
	__attribute__((aligned(8)))
	uint64_t thread_id;	// 64-bit unique thread id

	/* Thread Specific Data slots
	 *
	 * The offset of this field from the start of the structure is difficult to
	 * change on OS X because of a thorny bitcompat issue: mono has hard coded
	 * the value into their source.  Newer versions of mono will fall back to
	 * scanning to determine it at runtime, but there's lots of software built
	 * with older mono that won't.  We will have to break them someday...
	 */
	__attribute__ ((aligned (16)))
	void *tsd[_EXTERNAL_POSIX_THREAD_KEYS_MAX + _INTERNAL_POSIX_THREAD_KEYS_MAX];
} *pthread_t;

#define _PTHREAD_ATTR_REFILLMS_MAX ((2<<24) - 1)
struct _pthread_attr_t {
	long   sig;
	size_t guardsize; // size in bytes of stack overflow guard area
	void  *stackaddr; // stack base; vm_page_size aligned
	size_t stacksize; // stack size; multiple of vm_page_size and >= PTHREAD_STACK_MIN
	union {
		struct sched_param param; // [aligned]
		unsigned long qosclass; // pthread_priority_t
	};
	uint32_t
		detached:8,
		inherit:8,
		policy:8,
		schedset:1,
		qosset:1,
		policyset:1,
		cpupercentset:1,
		defaultguardpage:1,
		unused:3;
	uint32_t
		cpupercent:8,
		refillms:24;
#if defined(__LP64__)
	uint32_t _reserved[4];
#else
	uint32_t _reserved[2];
#endif
};

/*
 * Mutex attributes
 */

#define _PTHREAD_MUTEXATTR_T
typedef struct {
	long sig;
	int prioceiling;
	uint32_t protocol:2,
		type:2,
		pshared:2,
		opt:3,
		unused:23;
} pthread_mutexattr_t;

struct _pthread_mutex_options {
	uint32_t protocol:2,
		type:2,
		pshared:2,
		policy:3,
		hold:2,
		misalign:1,
		notify:1,
		mutex:1,
		unused:2,
		lock_count:16;
};
//
#define _PTHREAD_MUTEX_POLICY_LAST		(PTHREAD_MUTEX_POLICY_FIRSTFIT_NP + 1)
#define _PTHREAD_MTX_OPT_POLICY_FAIRSHARE 1
#define _PTHREAD_MTX_OPT_POLICY_FIRSTFIT 2
#define _PTHREAD_MTX_OPT_POLICY_DEFAULT _PTHREAD_MTX_OPT_POLICY_FIRSTFIT
// The following _pthread_mutex_options defintions exist in synch_internal.h
// such that the kernel extension can test for flags. They must be kept in
// sync with the bit values in the struct above.
// _PTHREAD_MTX_OPT_PSHARED 0x010
// _PTHREAD_MTX_OPT_NOTIFY 0x1000
// _PTHREAD_MTX_OPT_MUTEX 0x2000

// The fixed mask is used to mask out portions of the mutex options that
// change on a regular basis (notify, lock_count).
#define _PTHREAD_MTX_OPT_FIXED_MASK	0x27ff

typedef struct {
	long sig;
	_pthread_lock lock;
	union {
		uint32_t value;
		struct _pthread_mutex_options options;
	} mtxopts;
	int16_t prioceiling;
	int16_t priority;
#if defined(__LP64__)
	uint32_t _pad;
#endif
	uint32_t m_tid[2]; // thread id of thread that has mutex locked
	uint32_t m_seq[2]; // mutex sequence id
	uint32_t m_mis[2]; // for misaligned locks m_tid/m_seq will span into here
#if defined(__LP64__)
	uint32_t _reserved[4];
#else
	uint32_t _reserved[1];
#endif
} _pthread_mutex;


#define _PTHREAD_CONDATTR_T
typedef struct {
	long sig;
	uint32_t pshared:2,
		unsupported:30;
} pthread_condattr_t;


typedef struct {
	long sig;
	_pthread_lock lock;
	uint32_t unused:29,
		misalign:1,
		pshared:2;
	_pthread_mutex *busy;
	uint32_t c_seq[3];
#if defined(__LP64__)
	uint32_t _reserved[3];
#endif
} _pthread_cond;


#define _PTHREAD_ONCE_T
typedef struct {
	long sig;
	os_once_t once;
} pthread_once_t;


#define _PTHREAD_RWLOCKATTR_T
typedef struct {
	long sig;
	int pshared;
#if defined(__LP64__)
	uint32_t _reserved[3];
#else
	uint32_t _reserved[2];
#endif
} pthread_rwlockattr_t;


typedef struct {
	long sig;
	_pthread_lock lock;
	uint32_t unused:29,
			misalign:1,
			pshared:2;
	uint32_t rw_flags;
#if defined(__LP64__)
	uint32_t _pad;
#endif
	uint32_t rw_tid[2]; // thread id of thread that has exclusive (write) lock
	uint32_t rw_seq[4]; // rw sequence id (at 128-bit aligned boundary)
	uint32_t rw_mis[4]; // for misaligned locks rw_seq will span into here
#if defined(__LP64__)
	uint32_t _reserved[34];
#else
	uint32_t _reserved[18];
#endif
} _pthread_rwlock;

#include "pthread.h"
#include "pthread_spis.h"

_Static_assert(sizeof(_pthread_mutex) == sizeof(pthread_mutex_t),
		"Incorrect _pthread_mutex structure size");

_Static_assert(sizeof(_pthread_rwlock) == sizeof(pthread_rwlock_t),
		"Incorrect _pthread_rwlock structure size");

// Internal references to pthread_self() use TSD slot 0 directly.
__header_always_inline __pure2 pthread_t
_pthread_self_direct(void)
{
#if TARGET_OS_SIMULATOR || defined(__i386__) || defined(__x86_64__)
	return (pthread_t)_pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_SELF);
#elif defined(__arm__) || defined(__arm64__)
	uintptr_t tsd_base = (uintptr_t)_os_tsd_get_base();
	return (pthread_t)(tsd_base - offsetof(struct _pthread, tsd));
#else
#error unsupported architecture
#endif
}
#define pthread_self() _pthread_self_direct()

PTHREAD_ALWAYS_INLINE
inline static uint64_t __attribute__((__pure__))
_pthread_selfid_direct(void)
{
	return (_pthread_self_direct())->thread_id;
}

#define _PTHREAD_DEFAULT_INHERITSCHED	PTHREAD_INHERIT_SCHED
#define _PTHREAD_DEFAULT_PROTOCOL	PTHREAD_PRIO_NONE
#define _PTHREAD_DEFAULT_PRIOCEILING	0
#define _PTHREAD_DEFAULT_POLICY		SCHED_OTHER
#define _PTHREAD_DEFAULT_STACKSIZE	0x80000	  /* 512K */
#define _PTHREAD_DEFAULT_PSHARED	PTHREAD_PROCESS_PRIVATE

#define _PTHREAD_NO_SIG			0x00000000
#define _PTHREAD_MUTEX_ATTR_SIG		0x4D545841  /* 'MTXA' */
#define _PTHREAD_MUTEX_SIG		0x4D555458  /* 'MUTX' */
#define _PTHREAD_MUTEX_SIG_fast		0x4D55545A  /* 'MUTZ' */
#define _PTHREAD_MUTEX_SIG_MASK		0xfffffffd
#define _PTHREAD_MUTEX_SIG_CMP		0x4D555458  /* _PTHREAD_MUTEX_SIG & _PTHREAD_MUTEX_SIG_MASK */
#define _PTHREAD_MUTEX_SIG_init		0x32AAABA7  /* [almost] ~'MUTX' */
#define _PTHREAD_ERRORCHECK_MUTEX_SIG_init      0x32AAABA1
#define _PTHREAD_RECURSIVE_MUTEX_SIG_init       0x32AAABA2
#define _PTHREAD_FIRSTFIT_MUTEX_SIG_init        0x32AAABA3
#define _PTHREAD_MUTEX_SIG_init_MASK            0xfffffff0
#define _PTHREAD_MUTEX_SIG_init_CMP             0x32AAABA0
#define _PTHREAD_COND_ATTR_SIG		0x434E4441  /* 'CNDA' */
#define _PTHREAD_COND_SIG		0x434F4E44  /* 'COND' */
#define _PTHREAD_COND_SIG_init		0x3CB0B1BB  /* [almost] ~'COND' */
#define _PTHREAD_ATTR_SIG		0x54484441  /* 'THDA' */
#define _PTHREAD_ONCE_SIG		0x4F4E4345  /* 'ONCE' */
#define _PTHREAD_ONCE_SIG_init		0x30B1BCBA  /* [almost] ~'ONCE' */
#define _PTHREAD_SIG			0x54485244  /* 'THRD' */
#define _PTHREAD_RWLOCK_ATTR_SIG	0x52574C41  /* 'RWLA' */
#define _PTHREAD_RWLOCK_SIG		0x52574C4B  /* 'RWLK' */
#define _PTHREAD_RWLOCK_SIG_init	0x2DA8B3B4  /* [almost] ~'RWLK' */


#define _PTHREAD_KERN_COND_SIG		0x12345678  /*  */
#define _PTHREAD_KERN_MUTEX_SIG		0x34567812  /*  */
#define _PTHREAD_KERN_RWLOCK_SIG	0x56781234  /*  */

#if defined(DEBUG)
#define _PTHREAD_MUTEX_OWNER_SELF	pthread_self()
#else
#define _PTHREAD_MUTEX_OWNER_SELF	(pthread_t)0x12141968
#endif
#define _PTHREAD_MUTEX_OWNER_SWITCHING	(pthread_t)(~0)

#define _PTHREAD_CANCEL_STATE_MASK   0x01
#define _PTHREAD_CANCEL_TYPE_MASK    0x02
#define _PTHREAD_CANCEL_PENDING	     0x10  /* pthread_cancel() has been called for this thread */

extern boolean_t swtch_pri(int);

#include "kern/kern_internal.h"

/* Prototypes. */

/* Internal globals. */
PTHREAD_NOEXPORT void _pthread_tsd_cleanup(pthread_t self);

PTHREAD_NOEXPORT int _pthread_mutex_droplock(_pthread_mutex *mutex, uint32_t * flagp, uint32_t ** pmtxp, uint32_t * mgenp, uint32_t * ugenp);

/* internally redirected upcalls. */
PTHREAD_NOEXPORT void* malloc(size_t);
PTHREAD_NOEXPORT void free(void*);

/* syscall interfaces */
extern uint32_t __psynch_mutexwait(_pthread_mutex * mutex,  uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags);
extern uint32_t __psynch_mutexdrop(_pthread_mutex * mutex,  uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags);

extern uint32_t __psynch_cvbroad(pthread_cond_t * cv, uint64_t cvlsgen, uint64_t cvudgen, uint32_t flags, pthread_mutex_t * mutex,  uint64_t mugen, uint64_t tid);
extern uint32_t __psynch_cvsignal(pthread_cond_t * cv, uint64_t cvlsgen, uint32_t cvugen, int thread_port, pthread_mutex_t * mutex,  uint64_t mugen, uint64_t tid, uint32_t flags);
extern uint32_t __psynch_cvwait(pthread_cond_t * cv, uint64_t cvlsgen, uint32_t cvugen, pthread_mutex_t * mutex,  uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec);
extern uint32_t __psynch_cvclrprepost(void * cv, uint32_t cvgen, uint32_t cvugen, uint32_t cvsgen, uint32_t prepocnt, uint32_t preposeq, uint32_t flags);
extern uint32_t __psynch_rw_longrdlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_yieldwrlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern int __psynch_rw_downgrade(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_upgrade(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_rdlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_wrlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_unlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_unlock2(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __bsdthread_ctl(uintptr_t cmd, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);

PTHREAD_EXTERN
int
__proc_info(int callnum, int pid, int flavor, uint64_t arg, void * buffer, int buffersize);

PTHREAD_NOEXPORT
void
_pthread_deallocate(pthread_t t, bool from_mach_thread);

PTHREAD_NOEXPORT
thread_qos_t
_pthread_qos_class_to_thread_qos(qos_class_t qos);

PTHREAD_NOEXPORT
void
_pthread_set_main_qos(pthread_priority_t qos);

PTHREAD_NOEXPORT
void
_pthread_key_global_init(const char *envp[]);

PTHREAD_NOEXPORT
void
_pthread_mutex_global_init(const char *envp[], struct _pthread_registration_data *registration_data);

PTHREAD_EXPORT
void
_pthread_start(pthread_t self, mach_port_t kport, void *(*fun)(void *), void * funarg, size_t stacksize, unsigned int flags);

PTHREAD_EXPORT
void
_pthread_wqthread(pthread_t self, mach_port_t kport, void *stackaddr, void *keventlist, int flags, int nkevents);

PTHREAD_NOEXPORT
void
_pthread_main_thread_init(pthread_t p);

PTHREAD_NOEXPORT
void
_pthread_main_thread_postfork_init(pthread_t p);

PTHREAD_NOEXPORT
void
_pthread_bsdthread_init(struct _pthread_registration_data *data);

PTHREAD_NOEXPORT_VARIANT
void
_pthread_clear_qos_tsd(mach_port_t thread_port);

#define PTHREAD_CONFORM_DARWIN_LEGACY     0
#define PTHREAD_CONFORM_UNIX03_NOCANCEL   1
#define PTHREAD_CONFORM_UNIX03_CANCELABLE 2

PTHREAD_NOEXPORT_VARIANT
void
_pthread_testcancel(int conforming);

PTHREAD_EXPORT
void
_pthread_exit_if_canceled(int error);

PTHREAD_NOEXPORT
void
_pthread_markcancel_if_canceled(pthread_t thread, mach_port_t kport);

PTHREAD_NOEXPORT
void
_pthread_setcancelstate_exit(pthread_t self, void *value_ptr);

PTHREAD_NOEXPORT
semaphore_t
_pthread_joiner_prepost_wake(pthread_t thread);

PTHREAD_ALWAYS_INLINE
static inline mach_port_t
_pthread_kernel_thread(pthread_t t)
{
	return t->tsd[_PTHREAD_TSD_SLOT_MACH_THREAD_SELF];
}

PTHREAD_ALWAYS_INLINE
static inline void
_pthread_set_kernel_thread(pthread_t t, mach_port_t p)
{
	t->tsd[_PTHREAD_TSD_SLOT_MACH_THREAD_SELF] = p;
}

#ifdef DEBUG
#define PTHREAD_DEBUG_ASSERT(b) \
		do { \
			if (os_unlikely(!(b))) { \
				PTHREAD_INTERNAL_CRASH(0, "Assertion failed: " #b); \
			} \
		} while (0)
#else
#define PTHREAD_DEBUG_ASSERT(b) ((void)0)
#endif

#include <os/semaphore_private.h>
#include <os/alloc_once_private.h>

struct pthread_atfork_entry {
	void (*prepare)(void);
	void (*parent)(void);
	void (*child)(void);
};

#define PTHREAD_ATFORK_INLINE_MAX 10
#define PTHREAD_ATFORK_MAX (vm_page_size/sizeof(struct pthread_atfork_entry))

struct pthread_globals_s {
	// atfork.c
	pthread_t psaved_self;
	_pthread_lock psaved_self_global_lock;
	_pthread_lock pthread_atfork_lock;

	size_t atfork_count;
	struct pthread_atfork_entry atfork_storage[PTHREAD_ATFORK_INLINE_MAX];
	struct pthread_atfork_entry *atfork;
	uint16_t qmp_logical[THREAD_QOS_LAST];
	uint16_t qmp_physical[THREAD_QOS_LAST];

};
typedef struct pthread_globals_s *pthread_globals_t;

__attribute__((__pure__))
static inline pthread_globals_t
_pthread_globals(void)
{
	return os_alloc_once(OS_ALLOC_ONCE_KEY_LIBSYSTEM_PTHREAD,
			     sizeof(struct pthread_globals_s),
			     NULL);
}

#pragma mark _pthread_mutex_check_signature

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_mutex_check_signature_fast(_pthread_mutex *mutex)
{
	return (mutex->sig == _PTHREAD_MUTEX_SIG_fast);
}

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_mutex_check_signature(_pthread_mutex *mutex)
{
	return ((mutex->sig & _PTHREAD_MUTEX_SIG_MASK) == _PTHREAD_MUTEX_SIG_CMP);
}

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_mutex_check_signature_init(_pthread_mutex *mutex)
{
	return ((mutex->sig & _PTHREAD_MUTEX_SIG_init_MASK) ==
			_PTHREAD_MUTEX_SIG_init_CMP);
}

#pragma mark _pthread_rwlock_check_signature

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_rwlock_check_signature(_pthread_rwlock *rwlock)
{
	return (rwlock->sig == _PTHREAD_RWLOCK_SIG);
}

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_rwlock_check_signature_init(_pthread_rwlock *rwlock)
{
	return (rwlock->sig == _PTHREAD_RWLOCK_SIG_init);
}

PTHREAD_ALWAYS_INLINE
static inline void
_pthread_validate_signature(pthread_t thread)
{
	pthread_t th  = (pthread_t)(thread->sig ^ _pthread_ptr_munge_token);
#if __has_feature(ptrauth_calls)
	th = ptrauth_auth_data(th, ptrauth_key_process_dependent_data,
			ptrauth_string_discriminator("pthread.signature"));
#endif
	if (os_unlikely(th != thread)) {
		/* OS_REASON_LIBSYSTEM_CODE_PTHREAD_CORRUPTION == 4 */
		abort_with_reason(OS_REASON_LIBSYSTEM, 4, "pthread_t was corrupted", 0);
	}
}

PTHREAD_ALWAYS_INLINE
static inline void
_pthread_init_signature(pthread_t thread)
{
	pthread_t th = thread;
#if __has_feature(ptrauth_calls)
	th = ptrauth_sign_unauthenticated(th, ptrauth_key_process_dependent_data,
			ptrauth_string_discriminator("pthread.signature"));
#endif
	thread->sig = (uintptr_t)th ^ _pthread_ptr_munge_token;
}

/*
 * ALWAYS called without list lock and return with list lock held on success
 *
 * This weird calling convention exists because this function will sometimes
 * drop the lock, and it's best callers don't have to remember this.
 */
PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_validate_thread_and_list_lock(pthread_t thread)
{
	pthread_t p;
	if (thread == NULL) return false;
	_PTHREAD_LOCK(_pthread_list_lock);
	TAILQ_FOREACH(p, &__pthread_head, tl_plist) {
		if (p != thread) continue;
		_pthread_validate_signature(p);
		return true;
	}
	_PTHREAD_UNLOCK(_pthread_list_lock);

	return false;
}

PTHREAD_ALWAYS_INLINE
static inline bool
_pthread_is_valid(pthread_t thread, mach_port_t *portp)
{
	mach_port_t kport = MACH_PORT_NULL;
	bool valid;

	if (thread == pthread_self()) {
		_pthread_validate_signature(thread);
		valid = true;
		kport = _pthread_kernel_thread(thread);
	} else if (!_pthread_validate_thread_and_list_lock(thread)) {
		valid = false;
	} else {
		kport = _pthread_kernel_thread(thread);
		valid = true;
		_PTHREAD_UNLOCK(_pthread_list_lock);
	}

	if (portp != NULL) {
		*portp = kport;
	}
	return valid;
}

PTHREAD_ALWAYS_INLINE
static inline void*
_pthread_atomic_xchg_ptr_inline(void **p, void *v)
{
	return os_atomic_xchg(p, v, seq_cst);
}

PTHREAD_ALWAYS_INLINE
static inline uint32_t
_pthread_atomic_xchg_uint32_relaxed_inline(uint32_t *p,uint32_t v)
{
	return os_atomic_xchg(p, v, relaxed);
}

#define _pthread_atomic_xchg_ptr(p, v) \
		_pthread_atomic_xchg_ptr_inline(p, v)
#define _pthread_atomic_xchg_uint32_relaxed(p, v) \
		_pthread_atomic_xchg_uint32_relaxed_inline(p, v)

#endif /* _POSIX_PTHREAD_INTERNALS_H */
