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

#ifndef __DISPATCH_SHIMS_ATOMIC__
#define __DISPATCH_SHIMS_ATOMIC__

// generate error during codegen
#define _dispatch_atomic_unimplemented() \
		({ __asm__(".err unimplemented"); })

#pragma mark -
#pragma mark memory_order

typedef enum _dispatch_atomic_memory_order
{
    _dispatch_atomic_memory_order_relaxed,
	_dispatch_atomic_memory_order_consume,
	_dispatch_atomic_memory_order_acquire,
	_dispatch_atomic_memory_order_release,
	_dispatch_atomic_memory_order_acq_rel,
	_dispatch_atomic_memory_order_seq_cst,
} _dispatch_atomic_memory_order;

#if !DISPATCH_ATOMIC_UP

#define dispatch_atomic_memory_order_relaxed \
		_dispatch_atomic_memory_order_relaxed
#define dispatch_atomic_memory_order_acquire \
		_dispatch_atomic_memory_order_acquire
#define dispatch_atomic_memory_order_release \
		_dispatch_atomic_memory_order_release
#define dispatch_atomic_memory_order_acq_rel \
		_dispatch_atomic_memory_order_acq_rel
#define dispatch_atomic_memory_order_seq_cst \
		_dispatch_atomic_memory_order_seq_cst

#else // DISPATCH_ATOMIC_UP

#define dispatch_atomic_memory_order_relaxed \
		_dispatch_atomic_memory_order_relaxed
#define dispatch_atomic_memory_order_acquire \
		_dispatch_atomic_memory_order_relaxed
#define dispatch_atomic_memory_order_release \
		_dispatch_atomic_memory_order_relaxed
#define dispatch_atomic_memory_order_acq_rel \
		_dispatch_atomic_memory_order_relaxed
#define dispatch_atomic_memory_order_seq_cst \
		_dispatch_atomic_memory_order_relaxed

#endif // DISPATCH_ATOMIC_UP

#if __has_extension(c_generic_selections)
#define _dispatch_atomic_basetypeof(p) \
		typeof(*_Generic((p), \
		int*: (int*)(p), \
		volatile int*: (int*)(p), \
		unsigned int*: (unsigned int*)(p), \
		volatile unsigned int*: (unsigned int*)(p), \
		long*: (long*)(p), \
		volatile long*: (long*)(p), \
		unsigned long*: (unsigned long*)(p), \
		volatile unsigned long*: (unsigned long*)(p), \
		long long*: (long long*)(p), \
		volatile long long*: (long long*)(p), \
		unsigned long long*: (unsigned long long*)(p), \
		volatile unsigned long long*: (unsigned long long*)(p), \
		default: (void**)(p)))
#endif

/* clang lies on FreeBSD */
#if !__FreeBSD__ && __has_extension(c_atomic) && __has_extension(c_generic_selections)
#pragma mark -
#pragma mark c11

#define _dispatch_atomic_c11_atomic(p) \
		_Generic((p), \
		int*: (_Atomic(int)*)(p), \
		volatile int*: (volatile _Atomic(int)*)(p), \
		unsigned int*: (_Atomic(unsigned int)*)(p), \
		volatile unsigned int*: (volatile _Atomic(unsigned int)*)(p), \
		long*: (_Atomic(long)*)(p), \
		volatile long*: (volatile _Atomic(long)*)(p), \
		unsigned long*: (_Atomic(unsigned long)*)(p), \
		volatile unsigned long*: (volatile _Atomic(unsigned long)*)(p), \
		long long*: (_Atomic(long long)*)(p), \
		volatile long long*: (volatile _Atomic(long long)*)(p), \
		unsigned long long*: (_Atomic(unsigned long long)*)(p), \
		volatile unsigned long long*: \
				(volatile _Atomic(unsigned long long)*)(p), \
		default: (volatile _Atomic(void*)*)(p))

#define _dispatch_atomic_barrier(m) \
		({ __c11_atomic_thread_fence(dispatch_atomic_memory_order_##m); })
#define dispatch_atomic_load(p, m) \
		({ _dispatch_atomic_basetypeof(p) _r = \
		__c11_atomic_load(_dispatch_atomic_c11_atomic(p), \
		dispatch_atomic_memory_order_##m); (typeof(*(p)))_r; })
#define dispatch_atomic_store(p, v, m) \
		({ _dispatch_atomic_basetypeof(p) _v = (v); \
		__c11_atomic_store(_dispatch_atomic_c11_atomic(p), _v, \
		dispatch_atomic_memory_order_##m); })
#define dispatch_atomic_xchg(p, v, m) \
		({ _dispatch_atomic_basetypeof(p) _v = (v), _r = \
		__c11_atomic_exchange(_dispatch_atomic_c11_atomic(p), _v, \
		dispatch_atomic_memory_order_##m); (typeof(*(p)))_r; })
#define dispatch_atomic_cmpxchg(p, e, v, m) \
		({ _dispatch_atomic_basetypeof(p) _v = (v), _r = (e); \
		__c11_atomic_compare_exchange_strong(_dispatch_atomic_c11_atomic(p), \
		&_r, _v, dispatch_atomic_memory_order_##m, \
		dispatch_atomic_memory_order_relaxed); })
#define dispatch_atomic_cmpxchgv(p, e, v, g, m) \
		({ _dispatch_atomic_basetypeof(p) _v = (v), _r = (e); _Bool _b = \
		__c11_atomic_compare_exchange_strong(_dispatch_atomic_c11_atomic(p), \
		&_r, _v, dispatch_atomic_memory_order_##m, \
		dispatch_atomic_memory_order_relaxed); *(g) = (typeof(*(p)))_r; _b; })
#define dispatch_atomic_cmpxchgvw(p, e, v, g, m) \
		({ _dispatch_atomic_basetypeof(p) _v = (v), _r = (e); _Bool _b = \
		__c11_atomic_compare_exchange_weak(_dispatch_atomic_c11_atomic(p), \
		&_r, _v, dispatch_atomic_memory_order_##m, \
		dispatch_atomic_memory_order_relaxed); *(g) = (typeof(*(p)))_r;  _b; })
#define _dispatch_atomic_c11_op(p, v, m, o, op) \
		({ _dispatch_atomic_basetypeof(p) _v = (v), _r = \
		__c11_atomic_fetch_##o(_dispatch_atomic_c11_atomic(p), _v, \
		dispatch_atomic_memory_order_##m); (typeof(*(p)))(_r op _v); })
#define _dispatch_atomic_c11_op_orig(p, v, m, o, op) \
		({ _dispatch_atomic_basetypeof(p) _v = (v), _r = \
		__c11_atomic_fetch_##o(_dispatch_atomic_c11_atomic(p), _v, \
		dispatch_atomic_memory_order_##m); (typeof(*(p)))_r; })

#define dispatch_atomic_add(p, v, m) \
		_dispatch_atomic_c11_op((p), (v), m, add, +)
#define dispatch_atomic_add_orig(p, v, m) \
		_dispatch_atomic_c11_op_orig((p), (v), m, add, +)
#define dispatch_atomic_sub(p, v, m) \
		_dispatch_atomic_c11_op((p), (v), m, sub, -)
#define dispatch_atomic_sub_orig(p, v, m) \
		_dispatch_atomic_c11_op_orig((p), (v), m, sub, -)
#define dispatch_atomic_and(p, v, m) \
		_dispatch_atomic_c11_op((p), (v), m, and, &)
#define dispatch_atomic_and_orig(p, v, m) \
		_dispatch_atomic_c11_op_orig((p), (v), m, and, &)
#define dispatch_atomic_or(p, v, m) \
		_dispatch_atomic_c11_op((p), (v), m, or, |)
#define dispatch_atomic_or_orig(p, v, m) \
		_dispatch_atomic_c11_op_orig((p), (v), m, or, |)
#define dispatch_atomic_xor(p, v, m) \
		_dispatch_atomic_c11_op((p), (v), m, xor, ^)
#define dispatch_atomic_xor_orig(p, v, m) \
		_dispatch_atomic_c11_op_orig((p), (v), m, xor, ^)

#elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
#pragma mark -
#pragma mark gnu99

#define _dispatch_atomic_full_barrier()	\
		__sync_synchronize()
#define _dispatch_atomic_barrier(m)	\
		({ switch(dispatch_atomic_memory_order_##m) { \
		case _dispatch_atomic_memory_order_relaxed: \
			break; \
		default: \
			_dispatch_atomic_full_barrier(); break; \
		} })
// seq_cst: only emulate explicit store(seq_cst) -> load(seq_cst)
#define dispatch_atomic_load(p, m) \
		({ typeof(*(p)) _r = *(p); \
		switch(dispatch_atomic_memory_order_##m) { \
		case _dispatch_atomic_memory_order_seq_cst: \
			_dispatch_atomic_barrier(m); /* fallthrough */ \
		case _dispatch_atomic_memory_order_relaxed: \
			break; \
		default: \
			_dispatch_atomic_unimplemented(); break; \
		} _r; })
#define dispatch_atomic_store(p, v, m) \
		({ switch(dispatch_atomic_memory_order_##m) { \
		case _dispatch_atomic_memory_order_release: \
		case _dispatch_atomic_memory_order_seq_cst: \
			_dispatch_atomic_barrier(m); /* fallthrough */ \
		case _dispatch_atomic_memory_order_relaxed: \
			*(p) = (v); break; \
		default: \
			_dispatch_atomic_unimplemented(); break; \
		} switch(dispatch_atomic_memory_order_##m) { \
		case _dispatch_atomic_memory_order_seq_cst: \
			_dispatch_atomic_barrier(m); break; \
		default: \
			break; \
		} })
#if __has_builtin(__sync_swap)
#define dispatch_atomic_xchg(p, v, m) \
		((typeof(*(p)))__sync_swap((p), (v)))
#else
#define dispatch_atomic_xchg(p, v, m) \
		((typeof(*(p)))__sync_lock_test_and_set((p), (v)))
#endif
#define dispatch_atomic_cmpxchg(p, e, v, m) \
		__sync_bool_compare_and_swap((p), (e), (v))
#define dispatch_atomic_cmpxchgv(p, e, v, g, m) \
		({ typeof(*(g)) _e = (e), _r = \
		__sync_val_compare_and_swap((p), _e, (v)); \
		bool _b = (_e == _r); *(g) = _r; _b; })
#define dispatch_atomic_cmpxchgvw(p, e, v, g, m) \
		dispatch_atomic_cmpxchgv((p), (e), (v), (g), m)

#define dispatch_atomic_add(p, v, m) \
		__sync_add_and_fetch((p), (v))
#define dispatch_atomic_add_orig(p, v, m) \
		__sync_fetch_and_add((p), (v))
#define dispatch_atomic_sub(p, v, m) \
		__sync_sub_and_fetch((p), (v))
#define dispatch_atomic_sub_orig(p, v, m) \
		__sync_fetch_and_sub((p), (v))
#define dispatch_atomic_and(p, v, m) \
		__sync_and_and_fetch((p), (v))
#define dispatch_atomic_and_orig(p, v, m) \
		__sync_fetch_and_and((p), (v))
#define dispatch_atomic_or(p, v, m) \
		__sync_or_and_fetch((p), (v))
#define dispatch_atomic_or_orig(p, v, m) \
		__sync_fetch_and_or((p), (v))
#define dispatch_atomic_xor(p, v, m) \
		__sync_xor_and_fetch((p), (v))
#define dispatch_atomic_xor_orig(p, v, m) \
		__sync_fetch_and_xor((p), (v))

#if defined(__x86_64__) || defined(__i386__)
// GCC emits nothing for __sync_synchronize() on x86_64 & i386
#undef _dispatch_atomic_full_barrier
#define _dispatch_atomic_full_barrier() \
		({ __asm__ __volatile__( \
		"mfence" \
		: : : "memory"); })
#undef dispatch_atomic_load
#define dispatch_atomic_load(p, m) \
		({ switch(dispatch_atomic_memory_order_##m) { \
		case _dispatch_atomic_memory_order_seq_cst: \
		case _dispatch_atomic_memory_order_relaxed: \
			break; \
		default: \
			_dispatch_atomic_unimplemented(); break; \
		} *(p); })
// xchg is faster than store + mfence
#undef dispatch_atomic_store
#define dispatch_atomic_store(p, v, m) \
		({ switch(dispatch_atomic_memory_order_##m) { \
		case _dispatch_atomic_memory_order_relaxed: \
		case _dispatch_atomic_memory_order_release: \
			*(p) = (v); break; \
		case _dispatch_atomic_memory_order_seq_cst: \
			(void)dispatch_atomic_xchg((p), (v), m); break; \
		default:\
			_dispatch_atomic_unimplemented(); break; \
		} })
#endif

#else
#error "Please upgrade to GCC 4.2 or newer."
#endif

#pragma mark -
#pragma mark generic

// assume atomic builtins provide barriers
#define dispatch_atomic_barrier(m)
// see comment in dispatch_once.c
#define dispatch_atomic_maximally_synchronizing_barrier() \
		_dispatch_atomic_barrier(seq_cst)

#define dispatch_atomic_load2o(p, f, m) \
		dispatch_atomic_load(&(p)->f, m)
#define dispatch_atomic_store2o(p, f, v, m) \
		dispatch_atomic_store(&(p)->f, (v), m)
#define dispatch_atomic_xchg2o(p, f, v, m) \
		dispatch_atomic_xchg(&(p)->f, (v), m)
#define dispatch_atomic_cmpxchg2o(p, f, e, v, m) \
		dispatch_atomic_cmpxchg(&(p)->f, (e), (v), m)
#define dispatch_atomic_cmpxchgv2o(p, f, e, v, g, m) \
		dispatch_atomic_cmpxchgv(&(p)->f, (e), (v), (g), m)
#define dispatch_atomic_cmpxchgvw2o(p, f, e, v, g, m) \
		dispatch_atomic_cmpxchgvw(&(p)->f, (e), (v), (g), m)
#define dispatch_atomic_add2o(p, f, v, m) \
		dispatch_atomic_add(&(p)->f, (v), m)
#define dispatch_atomic_add_orig2o(p, f, v, m) \
		dispatch_atomic_add_orig(&(p)->f, (v), m)
#define dispatch_atomic_sub2o(p, f, v, m) \
		dispatch_atomic_sub(&(p)->f, (v), m)
#define dispatch_atomic_sub_orig2o(p, f, v, m) \
		dispatch_atomic_sub_orig(&(p)->f, (v), m)
#define dispatch_atomic_and2o(p, f, v, m) \
		dispatch_atomic_and(&(p)->f, (v), m)
#define dispatch_atomic_and_orig2o(p, f, v, m) \
		dispatch_atomic_and_orig(&(p)->f, (v), m)
#define dispatch_atomic_or2o(p, f, v, m) \
		dispatch_atomic_or(&(p)->f, (v), m)
#define dispatch_atomic_or_orig2o(p, f, v, m) \
		dispatch_atomic_or_orig(&(p)->f, (v), m)
#define dispatch_atomic_xor2o(p, f, v, m) \
		dispatch_atomic_xor(&(p)->f, (v), m)
#define dispatch_atomic_xor_orig2o(p, f, v, m) \
		dispatch_atomic_xor_orig(&(p)->f, (v), m)

#define dispatch_atomic_inc(p, m) \
		dispatch_atomic_add((p), 1, m)
#define dispatch_atomic_inc_orig(p, m) \
		dispatch_atomic_add_orig((p), 1, m)
#define dispatch_atomic_inc2o(p, f, m) \
		dispatch_atomic_add2o(p, f, 1, m)
#define dispatch_atomic_inc_orig2o(p, f, m) \
		dispatch_atomic_add_orig2o(p, f, 1, m)
#define dispatch_atomic_dec(p, m) \
		dispatch_atomic_sub((p), 1, m)
#define dispatch_atomic_dec_orig(p, m) \
		dispatch_atomic_sub_orig((p), 1, m)
#define dispatch_atomic_dec2o(p, f, m) \
		dispatch_atomic_sub2o(p, f, 1, m)
#define dispatch_atomic_dec_orig2o(p, f, m) \
		dispatch_atomic_sub_orig2o(p, f, 1, m)

#define dispatch_atomic_tsx_xacq_cmpxchgv(p, e, v, g) \
		dispatch_atomic_cmpxchgv((p), (e), (v), (g), acquire)
#define dispatch_atomic_tsx_xrel_store(p, v) \
		dispatch_atomic_store(p, v, release)
#define dispatch_atomic_tsx_xacq_cmpxchgv2o(p, f, e, v, g) \
		dispatch_atomic_tsx_xacq_cmpxchgv(&(p)->f, (e), (v), (g))
#define dispatch_atomic_tsx_xrel_store2o(p, f, v) \
		dispatch_atomic_tsx_xrel_store(&(p)->f, (v))

#if defined(__x86_64__) || defined(__i386__)
#pragma mark -
#pragma mark x86

#undef dispatch_atomic_maximally_synchronizing_barrier
#ifdef __LP64__
#define dispatch_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"cpuid" \
		: "=a" (_clbr) : "0" (0) : "rbx", "rcx", "rdx", "cc", "memory"); })
#else
#ifdef __llvm__
#define dispatch_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"cpuid" \
		: "=a" (_clbr) : "0" (0) : "ebx", "ecx", "edx", "cc", "memory"); })
#else // gcc does not allow inline i386 asm to clobber ebx
#define dispatch_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"pushl	%%ebx\n\t" \
		"cpuid\n\t" \
		"popl	%%ebx" \
		: "=a" (_clbr) : "0" (0) : "ecx", "edx", "cc", "memory"); })
#endif
#endif


#endif


#endif // __DISPATCH_SHIMS_ATOMIC__
