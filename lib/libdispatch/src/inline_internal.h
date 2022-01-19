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

#ifndef __DISPATCH_INLINE_INTERNAL__
#define __DISPATCH_INLINE_INTERNAL__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

#if DISPATCH_USE_CLIENT_CALLOUT

DISPATCH_NOTHROW void
_dispatch_client_callout(void *ctxt, dispatch_function_t f);
DISPATCH_NOTHROW void
_dispatch_client_callout2(void *ctxt, size_t i, void (*f)(void *, size_t));
DISPATCH_NOTHROW bool
_dispatch_client_callout3(void *ctxt, dispatch_data_t region, size_t offset,
		const void *buffer, size_t size, dispatch_data_applier_function_t f);
DISPATCH_NOTHROW void
_dispatch_client_callout4(void *ctxt, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t dmsg, mach_error_t error,
		dispatch_mach_handler_function_t f);

#else // !DISPATCH_USE_CLIENT_CALLOUT

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_client_callout(void *ctxt, dispatch_function_t f)
{
	return f(ctxt);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_client_callout2(void *ctxt, size_t i, void (*f)(void *, size_t))
{
	return f(ctxt, i);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_client_callout3(void *ctxt, dispatch_data_t region, size_t offset,
		const void *buffer, size_t size, dispatch_data_applier_function_t f)
{
	return f(ctxt, region, offset, buffer, size);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_client_callout4(void *ctxt, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t dmsg, mach_error_t error,
		dispatch_mach_handler_function_t f)
{
	return f(ctxt, reason, dmsg, error);
}

#endif // !DISPATCH_USE_CLIENT_CALLOUT

#if !(USE_OBJC && __OBJC2__)

#pragma mark -
#pragma mark _os_object_t & dispatch_object_t

DISPATCH_ALWAYS_INLINE
static inline _os_object_t
_os_object_retain_internal_inline(_os_object_t obj)
{
	int ref_cnt = obj->os_obj_ref_cnt;
	if (slowpath(ref_cnt == _OS_OBJECT_GLOBAL_REFCNT)) {
		return obj; // global object
	}
	ref_cnt = dispatch_atomic_inc2o(obj, os_obj_ref_cnt, relaxed);
	if (slowpath(ref_cnt <= 0)) {
		DISPATCH_CRASH("Resurrection of an object");
	}
	return obj;
}

DISPATCH_ALWAYS_INLINE
static inline void
_os_object_release_internal_inline(_os_object_t obj)
{
	int ref_cnt = obj->os_obj_ref_cnt;
	if (slowpath(ref_cnt == _OS_OBJECT_GLOBAL_REFCNT)) {
		return; // global object
	}
	ref_cnt = dispatch_atomic_dec2o(obj, os_obj_ref_cnt, relaxed);
	if (fastpath(ref_cnt >= 0)) {
		return;
	}
	if (slowpath(ref_cnt < -1)) {
		DISPATCH_CRASH("Over-release of an object");
	}
#if DISPATCH_DEBUG
	if (slowpath(obj->os_obj_xref_cnt >= 0)) {
		DISPATCH_CRASH("Release while external references exist");
	}
#endif
	return _os_object_dispose(obj);
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline void
_dispatch_retain(dispatch_object_t dou)
{
	(void)_os_object_retain_internal_inline(dou._os_obj);
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline void
_dispatch_release(dispatch_object_t dou)
{
	_os_object_release_internal_inline(dou._os_obj);
}

#pragma mark -
#pragma mark dispatch_thread

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_wqthread_override_start(mach_port_t thread,
		pthread_priority_t priority)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (!_dispatch_set_qos_class_enabled) return;
	(void)_pthread_workqueue_override_start_direct(thread, priority);
#else
	(void)thread; (void)priority;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_wqthread_override_reset(void)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (!_dispatch_set_qos_class_enabled) return;
	(void)_pthread_workqueue_override_reset();
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_override_start(mach_port_t thread, pthread_priority_t priority)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (!_dispatch_set_qos_class_enabled) return;
	(void)_pthread_override_qos_class_start_direct(thread, priority);
#else
	(void)thread; (void)priority;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_thread_override_end(mach_port_t thread)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (!_dispatch_set_qos_class_enabled) return;
	(void)_pthread_override_qos_class_end_direct(thread);
#else
	(void)thread;
#endif
}

#pragma mark -
#pragma mark dispatch_queue_t

static inline bool _dispatch_queue_need_override(dispatch_queue_t dq,
		pthread_priority_t pp);
static inline bool _dispatch_queue_need_override_retain(dispatch_queue_t dq,
		pthread_priority_t pp);
static inline bool _dispatch_queue_retain_if_override(dispatch_queue_t dq,
		pthread_priority_t pp);
static inline pthread_priority_t _dispatch_queue_get_override_priority(
		dispatch_queue_t dq);
static inline pthread_priority_t _dispatch_queue_reset_override_priority(
		dispatch_queue_t dq);
static inline pthread_priority_t _dispatch_get_defaultpriority(void);
static inline void _dispatch_set_defaultpriority_override(void);
static inline void _dispatch_reset_defaultpriority(pthread_priority_t priority);
static inline void _dispatch_set_priority(pthread_priority_t priority);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_set_thread(dispatch_queue_t dq)
{
	// The manager queue uses dispatch_queue_drain but is thread bound
	if (!dq->dq_is_thread_bound) {
		dq->dq_thread = _dispatch_thread_port();
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_clear_thread(dispatch_queue_t dq)
{
	if (!dq->dq_is_thread_bound) {
		dq->dq_thread = MACH_PORT_NULL;
	}
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_push_list2(dispatch_queue_t dq, struct dispatch_object_s *head,
		struct dispatch_object_s *tail)
{
	struct dispatch_object_s *prev;
	tail->do_next = NULL;
	prev = dispatch_atomic_xchg2o(dq, dq_items_tail, tail, release);
	if (fastpath(prev)) {
		// if we crash here with a value less than 0x1000, then we are at a
		// known bug in client code for example, see _dispatch_queue_dispose
		// or _dispatch_atfork_child
		prev->do_next = head;
	}
	return (prev != NULL);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_push_list(dispatch_queue_t dq, dispatch_object_t _head,
		dispatch_object_t _tail, pthread_priority_t pp, unsigned int n)
{
	struct dispatch_object_s *head = _head._do, *tail = _tail._do;
	bool override = _dispatch_queue_need_override_retain(dq, pp);
	if (!fastpath(_dispatch_queue_push_list2(dq, head, tail))) {
		_dispatch_queue_push_list_slow(dq, pp, head, n, override);
	} else if (override) {
		_dispatch_queue_wakeup_with_qos_and_release(dq, pp);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_push(dispatch_queue_t dq, dispatch_object_t _tail,
		pthread_priority_t pp)
{
	struct dispatch_object_s *tail = _tail._do;
	bool override = _dispatch_queue_need_override_retain(dq, pp);
	if (!fastpath(_dispatch_queue_push_list2(dq, tail, tail))) {
		_dispatch_queue_push_slow(dq, pp, tail, override);
	} else if (override) {
		_dispatch_queue_wakeup_with_qos_and_release(dq, pp);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_push_wakeup(dispatch_queue_t dq, dispatch_object_t _tail,
		pthread_priority_t pp, bool wakeup)
{
	// caller assumed to have a reference on dq
	struct dispatch_object_s *tail = _tail._do;
	if (!fastpath(_dispatch_queue_push_list2(dq, tail, tail))) {
		_dispatch_queue_push_slow(dq, pp, tail, false);
	} else if (_dispatch_queue_need_override(dq, pp)) {
		_dispatch_queue_wakeup_with_qos(dq, pp);
	} else if (slowpath(wakeup)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_class_invoke(dispatch_object_t dou,
		dispatch_queue_t (*invoke)(dispatch_object_t,
		_dispatch_thread_semaphore_t*))
{
	pthread_priority_t p = 0;
	dispatch_queue_t dq = dou._dq;
	if (!slowpath(DISPATCH_OBJECT_SUSPENDED(dq)) &&
			fastpath(dispatch_atomic_cmpxchg2o(dq, dq_running, 0, 1, acquire))){
		_dispatch_queue_set_thread(dq);
		dispatch_queue_t tq = NULL;
		_dispatch_thread_semaphore_t sema = 0;
		tq = invoke(dq, &sema);
		_dispatch_queue_clear_thread(dq);
		p = _dispatch_queue_reset_override_priority(dq);
		if (p > (dq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK)) {
			// Ensure that the root queue sees that this thread was overridden.
			_dispatch_set_defaultpriority_override();
		}
		// We do not need to check the result.
		// When the suspend-count lock is dropped, then the check will happen.
		(void)dispatch_atomic_dec2o(dq, dq_running, release);
		if (sema) {
			_dispatch_thread_semaphore_signal(sema);
		} else if (tq) {
			_dispatch_introspection_queue_item_complete(dq);
			return _dispatch_queue_push(tq, dq, p);
		}
	}
	dq->do_next = DISPATCH_OBJECT_LISTLESS;
	_dispatch_introspection_queue_item_complete(dq);
	if (!dispatch_atomic_sub2o(dq, do_suspend_cnt,
			DISPATCH_OBJECT_SUSPEND_LOCK, seq_cst)) {
		// seq_cst with atomic store to suspend_cnt <rdar://problem/11915417>
		if (dispatch_atomic_load2o(dq, dq_running, seq_cst) == 0) {
			// verify that the queue is idle
			return _dispatch_queue_wakeup_with_qos_and_release(dq, p);
		}
	}
	_dispatch_release(dq); // added when the queue is put on the list
}

DISPATCH_ALWAYS_INLINE
static inline unsigned long
_dispatch_queue_class_probe(dispatch_object_t dou)
{
	dispatch_queue_t dq = dou._dq;
	struct dispatch_object_s *tail;
	// seq_cst with atomic store to suspend_cnt <rdar://problem/14637483>
	tail = dispatch_atomic_load2o(dq, dq_items_tail, seq_cst);
	return (unsigned long)slowpath(tail != NULL);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_object_suspended(dispatch_object_t dou)
{
	struct dispatch_object_s *obj = dou._do;
	unsigned int suspend_cnt;
	// seq_cst with atomic store to tail <rdar://problem/14637483>
	suspend_cnt = dispatch_atomic_load2o(obj, do_suspend_cnt, seq_cst);
	return slowpath(suspend_cnt >= DISPATCH_OBJECT_SUSPEND_INTERVAL);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
_dispatch_queue_get_current(void)
{
	return (dispatch_queue_t)_dispatch_thread_getspecific(dispatch_queue_key);
}

DISPATCH_ALWAYS_INLINE DISPATCH_CONST
static inline dispatch_queue_t
_dispatch_get_root_queue(qos_class_t priority, bool overcommit)
{
	if (overcommit) switch (priority) {
	case _DISPATCH_QOS_CLASS_MAINTENANCE:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS_OVERCOMMIT];
	case _DISPATCH_QOS_CLASS_BACKGROUND:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT];
	case _DISPATCH_QOS_CLASS_UTILITY:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT];
	case _DISPATCH_QOS_CLASS_DEFAULT:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT];
	case _DISPATCH_QOS_CLASS_USER_INITIATED:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT];
	case _DISPATCH_QOS_CLASS_USER_INTERACTIVE:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT];
	} else switch (priority) {
	case _DISPATCH_QOS_CLASS_MAINTENANCE:
		return &_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS];
	case _DISPATCH_QOS_CLASS_BACKGROUND:
		return &_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS];
	case _DISPATCH_QOS_CLASS_UTILITY:
		return &_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS];
	case _DISPATCH_QOS_CLASS_DEFAULT:
		return &_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS];
	case _DISPATCH_QOS_CLASS_USER_INITIATED:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS];
	case _DISPATCH_QOS_CLASS_USER_INTERACTIVE:
		return &_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS];
	}
	return NULL;
}

// Note to later developers: ensure that any initialization changes are
// made for statically allocated queues (i.e. _dispatch_main_q).
static inline void
_dispatch_queue_init(dispatch_queue_t dq)
{
	dq->do_next = (struct dispatch_queue_s *)DISPATCH_OBJECT_LISTLESS;

	dq->dq_running = 0;
	dq->dq_width = 1;
	dq->dq_serialnum = dispatch_atomic_inc_orig(&_dispatch_queue_serial_numbers,
			relaxed);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_set_bound_thread(dispatch_queue_t dq)
{
	//Tag thread-bound queues with the owning thread
	dispatch_assert(dq->dq_is_thread_bound);
	dq->dq_thread = _dispatch_thread_port();
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_clear_bound_thread(dispatch_queue_t dq)
{
	dispatch_assert(dq->dq_is_thread_bound);
	dq->dq_thread = MACH_PORT_NULL;
}

DISPATCH_ALWAYS_INLINE
static inline mach_port_t
_dispatch_queue_get_bound_thread(dispatch_queue_t dq)
{
	dispatch_assert(dq->dq_is_thread_bound);
	return dq->dq_thread;
}

#pragma mark -
#pragma mark dispatch_priority

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_get_defaultpriority(void)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t priority = (uintptr_t)_dispatch_thread_getspecific(
			dispatch_defaultpriority_key);
	return priority;
#else
	return 0;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_reset_defaultpriority(pthread_priority_t priority)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t old_priority = _dispatch_get_defaultpriority();
	// if an inner-loop or'd in the override flag to the per-thread priority,
	// it needs to be propogated up the chain
	priority |= old_priority & _PTHREAD_PRIORITY_OVERRIDE_FLAG;

	if (slowpath(priority != old_priority)) {
		_dispatch_thread_setspecific(dispatch_defaultpriority_key,
				(void*)priority);
	}
#else
	(void)priority;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_set_defaultpriority_override(void)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t old_priority = _dispatch_get_defaultpriority();
	pthread_priority_t priority = old_priority |
			_PTHREAD_PRIORITY_OVERRIDE_FLAG;

	if (slowpath(priority != old_priority)) {
		_dispatch_thread_setspecific(dispatch_defaultpriority_key,
				(void*)priority);
	}
#endif
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_reset_defaultpriority_override(void)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t old_priority = _dispatch_get_defaultpriority();
	pthread_priority_t priority = old_priority &
			~((pthread_priority_t)_PTHREAD_PRIORITY_OVERRIDE_FLAG);

	if (slowpath(priority != old_priority)) {
		_dispatch_thread_setspecific(dispatch_defaultpriority_key,
				(void*)priority);
		return true;
	}
#endif
	return false;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_priority_inherit_from_target(dispatch_queue_t dq,
		dispatch_queue_t tq)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	const pthread_priority_t rootqueue_flag = _PTHREAD_PRIORITY_ROOTQUEUE_FLAG;
	const pthread_priority_t inherited_flag = _PTHREAD_PRIORITY_INHERIT_FLAG;
	pthread_priority_t dqp = dq->dq_priority, tqp = tq->dq_priority;
	if ((!dqp || (dqp & inherited_flag)) && (tqp & rootqueue_flag)) {
		dq->dq_priority = (tqp & ~rootqueue_flag) | inherited_flag;
	}
#else
	(void)dq; (void)tq;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_set_defaultpriority(pthread_priority_t priority)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t old_priority = _dispatch_get_defaultpriority();
	if (old_priority) {
		pthread_priority_t flags, defaultqueue, basepri;
		flags = (priority & _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG);
		defaultqueue = (old_priority & _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG);
		basepri = (old_priority & ~_PTHREAD_PRIORITY_FLAGS_MASK);
		priority &= ~_PTHREAD_PRIORITY_FLAGS_MASK;
		if (!priority) {
			flags = _PTHREAD_PRIORITY_INHERIT_FLAG | defaultqueue;
			priority = basepri;
		} else if (priority < basepri && !defaultqueue) { // rdar://16349734
			priority = basepri;
		}
		priority |= flags | (old_priority & _PTHREAD_PRIORITY_OVERRIDE_FLAG);
	}
	if (slowpath(priority != old_priority)) {
		_dispatch_thread_setspecific(dispatch_defaultpriority_key,
				(void*)priority);
	}
	return old_priority;
#else
	(void)priority;
	return 0;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_priority_adopt(pthread_priority_t priority, unsigned long flags)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t defaultpri = _dispatch_get_defaultpriority();
	bool enforce, inherited, defaultqueue;
	enforce = (flags & DISPATCH_PRIORITY_ENFORCE) ||
			(priority & _PTHREAD_PRIORITY_ENFORCE_FLAG);
	inherited = (defaultpri & _PTHREAD_PRIORITY_INHERIT_FLAG);
	defaultqueue = (defaultpri & _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG);
	defaultpri &= ~_PTHREAD_PRIORITY_FLAGS_MASK;
	priority &= ~_PTHREAD_PRIORITY_FLAGS_MASK;
	if (!priority) {
		enforce = false;
	} else if (!enforce) {
		if (priority < defaultpri) {
			if (defaultqueue) enforce = true; // rdar://16349734
		} else if (inherited || defaultqueue) {
			enforce = true;
		}
	} else if (priority < defaultpri && !defaultqueue) { // rdar://16349734
		enforce = false;
	}
	return enforce ? priority : defaultpri;
#else
	(void)priority; (void)flags;
	return 0;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_get_priority(void)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t priority = (uintptr_t)_dispatch_thread_getspecific(
			dispatch_priority_key);
	return (priority & ~_PTHREAD_PRIORITY_FLAGS_MASK);
#else
	return 0;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_set_priority_and_mach_voucher(pthread_priority_t priority,
		mach_voucher_t kv)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	_pthread_set_flags_t flags = 0;
	if (priority && _dispatch_set_qos_class_enabled) {
		pthread_priority_t old_priority = _dispatch_get_priority();
		if (priority != old_priority && old_priority) {
			flags |= _PTHREAD_SET_SELF_QOS_FLAG;
		}
	}
	if (kv != VOUCHER_NO_MACH_VOUCHER) {
#if VOUCHER_USE_MACH_VOUCHER
		flags |= _PTHREAD_SET_SELF_VOUCHER_FLAG;
#endif
	}
	if (!flags) return;
	int r = _pthread_set_properties_self(flags, priority, kv);
	(void)dispatch_assume_zero(r);
#elif VOUCHER_USE_MACH_VOUCHER
#error Invalid build configuration
#else
	(void)priority; (void)kv;
#endif
}

DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline voucher_t
_dispatch_set_priority_and_adopt_voucher(pthread_priority_t priority,
		voucher_t voucher)
{
	pthread_priority_t p = (priority != DISPATCH_NO_PRIORITY) ? priority : 0;
	voucher_t ov = DISPATCH_NO_VOUCHER;
	mach_voucher_t kv = VOUCHER_NO_MACH_VOUCHER;
	if (voucher != DISPATCH_NO_VOUCHER) {
		ov = _voucher_get();
		kv = _voucher_swap_and_get_mach_voucher(ov, voucher);
	}
	_dispatch_set_priority_and_mach_voucher(p, kv);
	return ov;
}

DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline voucher_t
_dispatch_adopt_priority_and_voucher(pthread_priority_t priority,
		voucher_t voucher, unsigned long flags)
{
	pthread_priority_t p = 0;
	if (priority != DISPATCH_NO_PRIORITY) {
		p = _dispatch_priority_adopt(priority, flags);
	}
	return _dispatch_set_priority_and_adopt_voucher(p, voucher);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_adopt_priority_and_replace_voucher(pthread_priority_t priority,
		voucher_t voucher, unsigned long flags)
{
	voucher_t ov;
	ov = _dispatch_adopt_priority_and_voucher(priority, voucher, flags);
	if (voucher != DISPATCH_NO_VOUCHER && ov) _voucher_release(ov);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_set_priority_and_replace_voucher(pthread_priority_t priority,
		voucher_t voucher)
{
	voucher_t ov;
	ov = _dispatch_set_priority_and_adopt_voucher(priority, voucher);
	if (voucher != DISPATCH_NO_VOUCHER && ov) _voucher_release(ov);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_set_priority(pthread_priority_t priority)
{
	_dispatch_set_priority_and_mach_voucher(priority, VOUCHER_NO_MACH_VOUCHER);
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_priority_normalize(pthread_priority_t pp)
{
	dispatch_assert_zero(pp & ~(pthread_priority_t)
			_PTHREAD_PRIORITY_QOS_CLASS_MASK);
	unsigned int qosbits = (unsigned int)pp, idx;
	if (!qosbits) return 0;
	idx = (unsigned int)(sizeof(qosbits)*8) -
			(unsigned int)__builtin_clz(qosbits) - 1;
	return (1 << idx);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_need_override(dispatch_queue_t dq, pthread_priority_t pp)
{
	if (!pp || dx_type(dq) == DISPATCH_QUEUE_ROOT_TYPE) return false;
	uint32_t p = (pp & _PTHREAD_PRIORITY_QOS_CLASS_MASK);
	uint32_t o = dq->dq_override;
	return (o < p);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_need_override_retain(dispatch_queue_t dq, pthread_priority_t pp)
{
	bool override = _dispatch_queue_need_override(dq, pp);
	if (override) _dispatch_retain(dq);
	return override;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_override_priority(dispatch_queue_t dq, pthread_priority_t pp)
{
	uint32_t p = (pp & _PTHREAD_PRIORITY_QOS_CLASS_MASK);
	uint32_t o = dq->dq_override;
	if (o < p) o = dispatch_atomic_or_orig2o(dq, dq_override, p, relaxed);
	return (o < p);
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_queue_get_override_priority(dispatch_queue_t dq)
{
	uint32_t p = (dq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK);
	uint32_t o = dq->dq_override;
	if (o == p) return o;
	return _dispatch_priority_normalize(o);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_set_override_priority(dispatch_queue_t dq)
{
	uint32_t p = 0;
	if (!(dq->dq_priority & _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG)) {
		p = dq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	}
	dispatch_atomic_store2o(dq, dq_override, p, relaxed);
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_queue_reset_override_priority(dispatch_queue_t dq)
{
	uint32_t p = 0;
	if (!(dq->dq_priority & _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG)) {
		p = dq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	}
	uint32_t o = dispatch_atomic_xchg2o(dq, dq_override, p, relaxed);
	if (o == p) return o;
	return _dispatch_priority_normalize(o);
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_priority_propagate(void)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t priority = _dispatch_get_priority();
	if (priority > _dispatch_user_initiated_priority) {
		// Cap QOS for propagation at user-initiated <rdar://16681262&16998036>
		priority = _dispatch_user_initiated_priority;
	}
	return priority;
#else
	return 0;
#endif
}

// including maintenance
DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_is_background_thread(void)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t priority;
	priority = _dispatch_get_priority();
	return priority && (priority <= _dispatch_background_priority);
#else
	return false;
#endif
}

#pragma mark -
#pragma mark dispatch_block_t

#ifdef __BLOCKS__

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_block_has_private_data(const dispatch_block_t block __unused)
{
#ifdef notyet	
	extern void (*_dispatch_block_special_invoke)(void*);
	return (_dispatch_Block_invoke(block) == _dispatch_block_special_invoke);
#else
	return (0);
#endif
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_block_private_data_t
_dispatch_block_get_data(const dispatch_block_t db)
{
	if (!_dispatch_block_has_private_data(db)) {
		return NULL;
	}
	// Keep in sync with _dispatch_block_create implementation
	uint8_t *x = (uint8_t *)db;
	// x points to base of struct Block_layout
	x += sizeof(struct Block_layout);
	// x points to addresss of captured block
	x += sizeof(dispatch_block_t);
#if USE_OBJC
	// x points to addresss of captured voucher
	x += sizeof(voucher_t);
#endif
	// x points to base of captured dispatch_block_private_data_s structure
	dispatch_block_private_data_t dbpd = (dispatch_block_private_data_t)(uintptr_t)x;
	if (dbpd->dbpd_magic != DISPATCH_BLOCK_PRIVATE_DATA_MAGIC) {
		DISPATCH_CRASH("Corruption of dispatch block object");
	}
	return dbpd;
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_block_get_priority(const dispatch_block_t db)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	return dbpd ? dbpd->dbpd_priority : 0;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_block_flags_t
_dispatch_block_get_flags(const dispatch_block_t db)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	return dbpd ? dbpd->dbpd_flags : 0;
}

#define DISPATCH_BLOCK_HAS(flag, db) \
		((_dispatch_block_get_flags((db)) & DISPATCH_BLOCK_HAS_ ## flag) != 0)
#define DISPATCH_BLOCK_IS(flag, db) \
		((_dispatch_block_get_flags((db)) & DISPATCH_BLOCK_ ## flag) != 0)

#endif

#pragma mark -
#pragma mark dispatch_continuation_t

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_continuation_alloc_cacheonly(void)
{
	dispatch_continuation_t dc = (dispatch_continuation_t)
			fastpath(_dispatch_thread_getspecific(dispatch_cache_key));
	if (dc) {
		_dispatch_thread_setspecific(dispatch_cache_key, dc->do_next);
	}
	return dc;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_continuation_alloc(void)
{
	dispatch_continuation_t dc =
			fastpath(_dispatch_continuation_alloc_cacheonly());
	if(!dc) {
		return _dispatch_continuation_alloc_from_heap();
	}
	return dc;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_continuation_free_cacheonly(dispatch_continuation_t dc)
{
	dispatch_continuation_t prev_dc = (dispatch_continuation_t)
			fastpath(_dispatch_thread_getspecific(dispatch_cache_key));
	int cnt = prev_dc ? prev_dc->dc_cache_cnt + 1 : 1;
	// Cap continuation cache
	if (slowpath(cnt > _dispatch_continuation_cache_limit)) {
		return dc;
	}
	dc->do_next = prev_dc;
	dc->dc_cache_cnt = cnt;
	_dispatch_thread_setspecific(dispatch_cache_key, dc);
	return NULL;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_free(dispatch_continuation_t dc)
{
	dc = _dispatch_continuation_free_cacheonly(dc);
	if (slowpath(dc)) {
		_dispatch_continuation_free_to_cache_limit(dc);
	}
}

#include "trace.h"

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline void
_dispatch_continuation_pop(dispatch_object_t dou)
{
	dispatch_continuation_t dc = dou._dc, dc1;
	dispatch_group_t dg;

	_dispatch_trace_continuation_pop(_dispatch_queue_get_current(), dou);
	if (DISPATCH_OBJ_IS_VTABLE(dou._do)) {
		return dx_invoke(dou._do);
	}

	// Add the item back to the cache before calling the function. This
	// allows the 'hot' continuation to be used for a quick callback.
	//
	// The ccache version is per-thread.
	// Therefore, the object has not been reused yet.
	// This generates better assembly.
	if ((long)dc->do_vtable & DISPATCH_OBJ_ASYNC_BIT) {
		_dispatch_continuation_voucher_adopt(dc);
		dc1 = _dispatch_continuation_free_cacheonly(dc);
	} else {
		dc1 = NULL;
	}
	if ((long)dc->do_vtable & DISPATCH_OBJ_GROUP_BIT) {
		dg = dc->dc_data;
	} else {
		dg = NULL;
	}
	_dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
	if (dg) {
		dispatch_group_leave(dg);
		_dispatch_release(dg);
	}
	_dispatch_introspection_queue_item_complete(dou);
	if (slowpath(dc1)) {
		_dispatch_continuation_free_to_cache_limit(dc1);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_priority_set(dispatch_continuation_t dc,
		pthread_priority_t pp, dispatch_block_flags_t flags)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t prio = 0;
	if (flags & DISPATCH_BLOCK_HAS_PRIORITY) {
		prio = pp;
	} else if (!(flags & DISPATCH_BLOCK_NO_QOS_CLASS)) {
		prio = _dispatch_priority_propagate();
	}
	if (flags & DISPATCH_BLOCK_ENFORCE_QOS_CLASS) {
		prio |= _PTHREAD_PRIORITY_ENFORCE_FLAG;
	}
	dc->dc_priority = prio;
#else
	(void)dc; (void)pp; (void)flags;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_dispatch_continuation_get_override_priority(dispatch_queue_t dq,
		dispatch_continuation_t dc)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t p = dc->dc_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	bool enforce = dc->dc_priority & _PTHREAD_PRIORITY_ENFORCE_FLAG;
	pthread_priority_t dqp = dq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	bool defaultqueue = dq->dq_priority & _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG;
	if (!p) {
		enforce = false;
	} else if (!enforce && (!dqp || defaultqueue)) {
		enforce = true;
	}
	if (!enforce) {
		p = dqp;
	}
	return p;
#else
	(void)dq; (void)dc;
	return 0;
#endif
}

#endif // !(USE_OBJC && __OBJC2__)

#endif /* __DISPATCH_INLINE_INTERNAL__ */
