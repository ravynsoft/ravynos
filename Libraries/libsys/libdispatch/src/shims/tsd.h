/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SHIMS_TSD__
#define __DISPATCH_SHIMS_TSD__

#if HAVE_PTHREAD_MACHDEP_H
#include <pthread_machdep.h>
#endif

#define DISPATCH_TSD_INLINE DISPATCH_ALWAYS_INLINE_NDEBUG

#if USE_APPLE_TSD_OPTIMIZATIONS && HAVE_PTHREAD_KEY_INIT_NP && \
	!defined(DISPATCH_USE_DIRECT_TSD)
#define DISPATCH_USE_DIRECT_TSD 1
#if __has_include(<os/tsd.h>)
#include <os/tsd.h>
#endif

#if !defined(OS_GS_RELATIVE) && (defined(__i386__) || defined(__x86_64__))
#define OS_GS_RELATIVE __attribute__((address_space(256)))
#endif

#ifdef _os_tsd_get_base
#ifdef OS_GS_RELATIVE
typedef long dispatch_tsd_pair_t \
		__attribute__((vector_size(sizeof(long) * 2), aligned(sizeof(long))));
#define _os_tsd_get_pair_address(k) \
	(dispatch_tsd_pair_t OS_GS_RELATIVE *)((k) * sizeof(long))
#else
typedef struct { void *a; void *b; } dispatch_tsd_pair_t;
#define _os_tsd_get_pair_address(k) \
	(dispatch_tsd_pair_t *)(_os_tsd_get_base() + (k))
#endif
#endif // _os_tsd_get_base
#endif

#if defined(_WIN32)
#define DISPATCH_TSD_DTOR_CC __stdcall
#else
#define DISPATCH_TSD_DTOR_CC
#endif

#if DISPATCH_USE_DIRECT_TSD
#ifndef __TSD_THREAD_QOS_CLASS
#define __TSD_THREAD_QOS_CLASS 4
#endif
#ifndef __TSD_RETURN_TO_KERNEL
#define __TSD_RETURN_TO_KERNEL 5
#endif
#ifndef __TSD_MACH_SPECIAL_REPLY
#define __TSD_MACH_SPECIAL_REPLY 8
#endif


static const unsigned long dispatch_priority_key	= __TSD_THREAD_QOS_CLASS;
static const unsigned long dispatch_r2k_key			= __TSD_RETURN_TO_KERNEL;

// dispatch_queue_key & dispatch_frame_key need to be contiguous
// in that order, and queue_key to be an even number
static const unsigned long dispatch_queue_key		= __PTK_LIBDISPATCH_KEY0;
static const unsigned long dispatch_frame_key		= __PTK_LIBDISPATCH_KEY1;
static const unsigned long dispatch_cache_key		= __PTK_LIBDISPATCH_KEY2;
static const unsigned long dispatch_context_key		= __PTK_LIBDISPATCH_KEY3;
static const unsigned long dispatch_pthread_root_queue_observer_hooks_key =
		__PTK_LIBDISPATCH_KEY4;
static const unsigned long dispatch_basepri_key     = __PTK_LIBDISPATCH_KEY5;
#if DISPATCH_INTROSPECTION
static const unsigned long dispatch_introspection_key = __PTK_LIBDISPATCH_KEY6;
#elif DISPATCH_PERF_MON
static const unsigned long dispatch_bcounter_key	= __PTK_LIBDISPATCH_KEY6;
#endif
static const unsigned long dispatch_wlh_key			= __PTK_LIBDISPATCH_KEY7;
static const unsigned long dispatch_voucher_key		= __PTK_LIBDISPATCH_KEY8;
static const unsigned long dispatch_deferred_items_key = __PTK_LIBDISPATCH_KEY9;

DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_key_create(const unsigned long *k, void (*d)(void *))
{
	if (!*k || !d) return;
	dispatch_assert_zero(pthread_key_init_np((int)*k, d));
}
#elif DISPATCH_USE_THREAD_LOCAL_STORAGE

#if defined(_WIN32)

DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_key_create(DWORD *k, void (DISPATCH_TSD_DTOR_CC *d)(void *))
{
	dispatch_assert_zero((*k = FlsAlloc(d)));
}

extern DWORD __dispatch_tsd_key;

#else

DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_key_create(pthread_key_t *k, void (DISPATCH_TSD_DTOR_CC *d)(void *))
{
	dispatch_assert_zero(pthread_key_create(k, d));
}

extern pthread_key_t __dispatch_tsd_key;

#endif

struct dispatch_tsd {
#if defined(_WIN32)
	DWORD tid;
#else
	pid_t tid;
#endif
	void *dispatch_queue_key;
	void *dispatch_frame_key;
	void *dispatch_cache_key;
	void *dispatch_context_key;
	void *dispatch_pthread_root_queue_observer_hooks_key;
	void *dispatch_basepri_key;
#if DISPATCH_INTROSPECTION
	void *dispatch_introspection_key;
#elif DISPATCH_PERF_MON
	void *dispatch_bcounter_key;
#endif
	void *dispatch_priority_key;
	void *dispatch_r2k_key;
	void *dispatch_wlh_key;
	void *dispatch_voucher_key;
	void *dispatch_deferred_items_key;
};

extern _Thread_local struct dispatch_tsd __dispatch_tsd;

extern void libdispatch_tsd_init(void);
extern void _libdispatch_tsd_cleanup(void *ctx);

DISPATCH_ALWAYS_INLINE
static inline struct dispatch_tsd *
_dispatch_get_tsd_base(void)
{
	if (unlikely(__dispatch_tsd.tid == 0)) {
		libdispatch_tsd_init();
	}
	OS_COMPILER_CAN_ASSUME(__dispatch_tsd.tid != 0);
	return &__dispatch_tsd;
}

#define _dispatch_thread_getspecific(key) \
	(_dispatch_get_tsd_base()->key)
#define _dispatch_thread_setspecific(key, value) \
	(void)(_dispatch_get_tsd_base()->key = (value))

#define _dispatch_thread_getspecific_pair(k1, p1, k2, p2) \
	( *(p1) = _dispatch_thread_getspecific(k1), \
	  *(p2) = _dispatch_thread_getspecific(k2) )

#define _dispatch_thread_getspecific_packed_pair(k1, k2, p) \
	( (p)[0] = _dispatch_thread_getspecific(k1), \
	  (p)[1] = _dispatch_thread_getspecific(k2) )

#define _dispatch_thread_setspecific_pair(k1, p1, k2, p2) \
	( _dispatch_thread_setspecific(k1,p1), \
	  _dispatch_thread_setspecific(k2,p2) )

#define _dispatch_thread_setspecific_packed_pair(k1, k2, p) \
	( _dispatch_thread_setspecific(k1,(p)[0]), \
	  _dispatch_thread_setspecific(k2,(p)[1]) )

#else
extern pthread_key_t dispatch_priority_key;
extern pthread_key_t dispatch_r2k_key;
extern pthread_key_t dispatch_queue_key;
extern pthread_key_t dispatch_frame_key;
extern pthread_key_t dispatch_cache_key;
extern pthread_key_t dispatch_context_key;
extern pthread_key_t dispatch_pthread_root_queue_observer_hooks_key;
extern pthread_key_t dispatch_basepri_key;
#if DISPATCH_INTROSPECTION
extern pthread_key_t dispatch_introspection_key;
#elif DISPATCH_PERF_MON
extern pthread_key_t dispatch_bcounter_key;
#endif
extern pthread_key_t dispatch_wlh_key;
extern pthread_key_t dispatch_voucher_key;
extern pthread_key_t dispatch_deferred_items_key;

DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_key_create(pthread_key_t *k, void (*d)(void *))
{
	dispatch_assert_zero(pthread_key_create(k, d));
}
#endif

#ifndef DISPATCH_USE_THREAD_LOCAL_STORAGE
DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_setspecific(pthread_key_t k, void *v)
{
#if DISPATCH_USE_DIRECT_TSD
	if (_pthread_has_direct_tsd()) {
		(void)_pthread_setspecific_direct(k, v);
	} else {
#if TARGET_OS_SIMULATOR
		(void)_pthread_setspecific_static(k, v); // rdar://26058142
#else
		__builtin_trap(); // unreachable
#endif
	}
	return;
#endif
	dispatch_assert_zero(pthread_setspecific(k, v));
}

DISPATCH_TSD_INLINE
static inline void *
_dispatch_thread_getspecific(pthread_key_t k)
{
#if DISPATCH_USE_DIRECT_TSD
	if (_pthread_has_direct_tsd()) {
		return _pthread_getspecific_direct(k);
	}
#endif
	return pthread_getspecific(k);
}

// this is used when loading a pair at once and the caller will want to
// look at each component individually.
// some platforms can load a pair of pointers efficiently that way (like arm)
// intel doesn't, hence this degrades to two loads on intel
DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_getspecific_pair(pthread_key_t k1, void **p1,
		pthread_key_t k2, void **p2)
{
	*p1 = _dispatch_thread_getspecific(k1);
	*p2 = _dispatch_thread_getspecific(k2);
}

// this is used for save/restore purposes
// and the caller doesn't need to look at a specific component
// this does SSE on intel, and SSE is bad at breaking/assembling components
DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_getspecific_packed_pair(pthread_key_t k1, pthread_key_t k2,
		void **p)
{
#if DISPATCH_USE_DIRECT_TSD && defined(_os_tsd_get_pair_address)
	dispatch_assert(k2 == k1 + 1);
	if (_pthread_has_direct_tsd()) {
		*(dispatch_tsd_pair_t *)p = *_os_tsd_get_pair_address(k1);
		return;
	}
#endif
	p[0] = _dispatch_thread_getspecific(k1);
	p[1] = _dispatch_thread_getspecific(k2);
}

// this is used when storing a pair at once from separated components
// some platforms can store a pair of pointers efficiently that way (like arm)
// intel doesn't, hence this degrades to two stores on intel
DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_setspecific_pair(pthread_key_t k1, void *p1,
		pthread_key_t k2, void *p2)
{
	_dispatch_thread_setspecific(k1, p1);
	_dispatch_thread_setspecific(k2, p2);
}

// this is used for save/restore purposes
// and the caller doesn't need to look at a specific component
// this does SSE on intel, and SSE is bad at breaking/assembling components
DISPATCH_TSD_INLINE
static inline void
_dispatch_thread_setspecific_packed_pair(pthread_key_t k1, pthread_key_t k2,
		void **p)
{
#if DISPATCH_USE_DIRECT_TSD && defined(_os_tsd_get_pair_address)
	dispatch_assert(k2 == k1 + 1);
	if (_pthread_has_direct_tsd()) {
		*_os_tsd_get_pair_address(k1) = *(dispatch_tsd_pair_t *)p;
		return;
	}
#endif
	_dispatch_thread_setspecific(k1, p[0]);
	_dispatch_thread_setspecific(k2, p[1]);
}
#endif

#if defined(_WIN32)
#define _dispatch_thread_self() ((uintptr_t)GetCurrentThreadId())
#else
#if DISPATCH_USE_DIRECT_TSD
#define _dispatch_thread_self() ((uintptr_t)_dispatch_thread_getspecific( \
		_PTHREAD_TSD_SLOT_PTHREAD_SELF))
#else
#define _dispatch_thread_self() ((uintptr_t)pthread_self())
#endif
#endif

#if defined(_WIN32)
#define _dispatch_thread_port() ((mach_port_t)0)
#elif !DISPATCH_USE_THREAD_LOCAL_STORAGE
#if DISPATCH_USE_DIRECT_TSD
#define _dispatch_thread_port() ((mach_port_t)(uintptr_t)\
		_dispatch_thread_getspecific(_PTHREAD_TSD_SLOT_MACH_THREAD_SELF))
#else
#define _dispatch_thread_port() pthread_mach_thread_np(_dispatch_thread_self())
#endif
#endif

#if HAVE_MACH
#define _dispatch_get_thread_mig_reply_port() ((mach_port_t)(uintptr_t) \
		_dispatch_thread_getspecific(_PTHREAD_TSD_SLOT_MIG_REPLY))
#define _dispatch_set_thread_mig_reply_port(p) ( \
		_dispatch_thread_setspecific(_PTHREAD_TSD_SLOT_MIG_REPLY, \
		(void*)(uintptr_t)(p)))
#define _dispatch_get_thread_special_reply_port() ((mach_port_t)(uintptr_t) \
		_dispatch_thread_getspecific(__TSD_MACH_SPECIAL_REPLY))
#define _dispatch_set_thread_special_reply_port(p) ( \
		_dispatch_thread_setspecific(__TSD_MACH_SPECIAL_REPLY, \
		(void*)(uintptr_t)(p)))
#endif

DISPATCH_TSD_INLINE DISPATCH_CONST
static inline unsigned int
_dispatch_cpu_number(void)
{
#if __has_include(<os/tsd.h>)
	return _os_cpu_number();
#elif defined(__x86_64__) || defined(__i386__)
	struct { uintptr_t p1, p2; } p;
	__asm__("sidt %[p]" : [p] "=&m" (p));
	return (unsigned int)(p.p1 & 0xfff);
#else
	// Not yet implemented.
	return 0;
#endif
}

#undef DISPATCH_TSD_INLINE

#endif
