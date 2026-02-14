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

#ifndef __OS_INTERNAL_ATOMIC__
#define __OS_INTERNAL_ATOMIC__

#ifndef __OS_EXPOSE_INTERNALS_INDIRECT__
/*
 * Use c11 <stdatomic.h> or c++11 std::atomic from <atomic> instead
 *
 * XXX                           /!\ WARNING /!\                           XXX
 *
 * This header file describes INTERNAL interfaces to libplatform used by other
 * libsystem targets, which are subject to change in future releases of OS X
 * and iOS. Any applications relying on these interfaces WILL break.
 *
 * If you are not a libsystem target, you should NOT EVER use these headers.
 * Not even a little.
 *
 * XXX                           /!\ WARNING /!\                           XXX
 */
#error "Please #include <os/internal/internal_shared.h> instead of this file directly."
#else

// generate error during codegen
#define _os_atomic_unimplemented() \
		({ __asm__(".err unimplemented"); })

#pragma mark -
#pragma mark memory_order

typedef enum _os_atomic_memory_order {
	_os_atomic_memory_order_relaxed,
	_os_atomic_memory_order_consume,
	_os_atomic_memory_order_acquire,
	_os_atomic_memory_order_release,
	_os_atomic_memory_order_acq_rel,
	_os_atomic_memory_order_seq_cst,
	_os_atomic_memory_order_ordered,
	_os_atomic_memory_order_dependency,
} _os_atomic_memory_order;

#if !OS_ATOMIC_UP

#define os_atomic_memory_order_relaxed    _os_atomic_memory_order_relaxed
#define os_atomic_memory_order_acquire    _os_atomic_memory_order_acquire
#define os_atomic_memory_order_release    _os_atomic_memory_order_release
#define os_atomic_memory_order_acq_rel    _os_atomic_memory_order_acq_rel
#define os_atomic_memory_order_seq_cst    _os_atomic_memory_order_seq_cst
#define os_atomic_memory_order_ordered    _os_atomic_memory_order_seq_cst
#define os_atomic_memory_order_dependency _os_atomic_memory_order_acquire

#else // OS_ATOMIC_UP

#define os_atomic_memory_order_relaxed    _os_atomic_memory_order_relaxed
#define os_atomic_memory_order_acquire    _os_atomic_memory_order_relaxed
#define os_atomic_memory_order_release    _os_atomic_memory_order_relaxed
#define os_atomic_memory_order_acq_rel    _os_atomic_memory_order_relaxed
#define os_atomic_memory_order_seq_cst    _os_atomic_memory_order_relaxed
#define os_atomic_memory_order_ordered    _os_atomic_memory_order_relaxed
#define os_atomic_memory_order_dependency _os_atomic_memory_order_relaxed

#endif // OS_ATOMIC_UP

#pragma mark -
#pragma mark c11

#if !__has_extension(c_atomic)
#error "Please use a C11 compiler"
#endif

#define os_atomic(type) type _Atomic

#define _os_atomic_c11_atomic(p) \
		((typeof(*(p)) _Atomic *)(p))

// This removes the _Atomic and volatile qualifiers on the type of *p
#define _os_atomic_basetypeof(p) \
		typeof(__c11_atomic_load(_os_atomic_c11_atomic(p), \
		_os_atomic_memory_order_relaxed))

#define _os_atomic_baseptr(p) \
		((_os_atomic_basetypeof(p) *)(p))

#define _os_atomic_barrier(m) \
		__c11_atomic_thread_fence(os_atomic_memory_order_##m)
#define os_atomic_load(p, m) \
		__c11_atomic_load(_os_atomic_c11_atomic(p), os_atomic_memory_order_##m)
#define os_atomic_store(p, v, m) \
		__c11_atomic_store(_os_atomic_c11_atomic(p), v, \
		os_atomic_memory_order_##m)
#define os_atomic_xchg(p, v, m) \
		__c11_atomic_exchange(_os_atomic_c11_atomic(p), v, \
		os_atomic_memory_order_##m)
#define os_atomic_cmpxchg(p, e, v, m) \
		({ _os_atomic_basetypeof(p) _r = (e); \
		__c11_atomic_compare_exchange_strong(_os_atomic_c11_atomic(p), \
		&_r, v, os_atomic_memory_order_##m, os_atomic_memory_order_relaxed); })
#define os_atomic_cmpxchgv(p, e, v, g, m) \
		({ _os_atomic_basetypeof(p) _r = (e); _Bool _b = \
		__c11_atomic_compare_exchange_strong(_os_atomic_c11_atomic(p), \
		&_r, v, os_atomic_memory_order_##m, os_atomic_memory_order_relaxed); \
		*(g) = _r; _b; })
#define os_atomic_cmpxchgvw(p, e, v, g, m) \
		({ _os_atomic_basetypeof(p) _r = (e); _Bool _b = \
		__c11_atomic_compare_exchange_weak(_os_atomic_c11_atomic(p), \
		&_r, v, os_atomic_memory_order_##m, os_atomic_memory_order_relaxed); \
		*(g) = _r; _b; })
#define _os_atomic_c11_op(p, v, m, o, op) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = \
		__c11_atomic_fetch_##o(_os_atomic_c11_atomic(p), _v, \
		os_atomic_memory_order_##m); (typeof(_r))(_r op _v); })
#define _os_atomic_c11_op_orig(p, v, m, o, op) \
		__c11_atomic_fetch_##o(_os_atomic_c11_atomic(p), v, \
		os_atomic_memory_order_##m)

#define os_atomic_add(p, v, m) \
		_os_atomic_c11_op((p), (v), m, add, +)
#define os_atomic_add_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, add, +)
#define os_atomic_sub(p, v, m) \
		_os_atomic_c11_op((p), (v), m, sub, -)
#define os_atomic_sub_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, sub, -)
#define os_atomic_and(p, v, m) \
		_os_atomic_c11_op((p), (v), m, and, &)
#define os_atomic_and_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, and, &)
#define os_atomic_or(p, v, m) \
		_os_atomic_c11_op((p), (v), m, or, |)
#define os_atomic_or_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, or, |)
#define os_atomic_xor(p, v, m) \
		_os_atomic_c11_op((p), (v), m, xor, ^)
#define os_atomic_xor_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, xor, ^)

#define os_atomic_force_dependency_on(p, e) (p)
#define os_atomic_load_with_dependency_on(p, e) \
		os_atomic_load(os_atomic_force_dependency_on(p, e), relaxed)
#define os_atomic_load_with_dependency_on2o(p, f, e) \
		os_atomic_load_with_dependency_on(&(p)->f, e)

#pragma mark -
#pragma mark generic

#define os_atomic_thread_fence(m) _os_atomic_barrier(m)

#define os_atomic_load2o(p, f, m) \
		os_atomic_load(&(p)->f, m)
#define os_atomic_store2o(p, f, v, m) \
		os_atomic_store(&(p)->f, (v), m)
#define os_atomic_xchg2o(p, f, v, m) \
		os_atomic_xchg(&(p)->f, (v), m)
#define os_atomic_cmpxchg2o(p, f, e, v, m) \
		os_atomic_cmpxchg(&(p)->f, (e), (v), m)
#define os_atomic_cmpxchgv2o(p, f, e, v, g, m) \
		os_atomic_cmpxchgv(&(p)->f, (e), (v), (g), m)
#define os_atomic_cmpxchgvw2o(p, f, e, v, g, m) \
		os_atomic_cmpxchgvw(&(p)->f, (e), (v), (g), m)
#define os_atomic_add2o(p, f, v, m) \
		os_atomic_add(&(p)->f, (v), m)
#define os_atomic_add_orig2o(p, f, v, m) \
		os_atomic_add_orig(&(p)->f, (v), m)
#define os_atomic_sub2o(p, f, v, m) \
		os_atomic_sub(&(p)->f, (v), m)
#define os_atomic_sub_orig2o(p, f, v, m) \
		os_atomic_sub_orig(&(p)->f, (v), m)
#define os_atomic_and2o(p, f, v, m) \
		os_atomic_and(&(p)->f, (v), m)
#define os_atomic_and_orig2o(p, f, v, m) \
		os_atomic_and_orig(&(p)->f, (v), m)
#define os_atomic_or2o(p, f, v, m) \
		os_atomic_or(&(p)->f, (v), m)
#define os_atomic_or_orig2o(p, f, v, m) \
		os_atomic_or_orig(&(p)->f, (v), m)
#define os_atomic_xor2o(p, f, v, m) \
		os_atomic_xor(&(p)->f, (v), m)
#define os_atomic_xor_orig2o(p, f, v, m) \
		os_atomic_xor_orig(&(p)->f, (v), m)

#define os_atomic_inc(p, m) \
		os_atomic_add((p), 1, m)
#define os_atomic_inc_orig(p, m) \
		os_atomic_add_orig((p), 1, m)
#define os_atomic_inc2o(p, f, m) \
		os_atomic_add2o(p, f, 1, m)
#define os_atomic_inc_orig2o(p, f, m) \
		os_atomic_add_orig2o(p, f, 1, m)
#define os_atomic_dec(p, m) \
		os_atomic_sub((p), 1, m)
#define os_atomic_dec_orig(p, m) \
		os_atomic_sub_orig((p), 1, m)
#define os_atomic_dec2o(p, f, m) \
		os_atomic_sub2o(p, f, 1, m)
#define os_atomic_dec_orig2o(p, f, m) \
		os_atomic_sub_orig2o(p, f, 1, m)

#define os_atomic_rmw_loop(p, ov, nv, m, ...)  ({ \
		bool _result = false; \
		typeof(p) _p = (p); \
		ov = os_atomic_load(_p, relaxed); \
		do { \
			__VA_ARGS__; \
			_result = os_atomic_cmpxchgvw(_p, ov, nv, &ov, m); \
		} while (os_unlikely(!_result)); \
		_result; \
	})
#define os_atomic_rmw_loop2o(p, f, ov, nv, m, ...) \
		os_atomic_rmw_loop(&(p)->f, ov, nv, m, __VA_ARGS__)
#define os_atomic_rmw_loop_give_up_with_fence(m, expr) \
		({ os_atomic_thread_fence(m); expr; __builtin_unreachable(); })
#define os_atomic_rmw_loop_give_up(expr) \
		os_atomic_rmw_loop_give_up_with_fence(relaxed, expr)


#endif // __OS_EXPOSE_INTERNALS_INDIRECT__

#endif // __OS_ATOMIC__
