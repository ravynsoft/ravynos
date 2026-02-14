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

#include "internal.h"
#if HAVE_MACH
#include "protocol.h" // _dispatch_send_wakeup_runloop_thread
#endif

static inline void _dispatch_root_queues_init(void);
static void _dispatch_lane_barrier_complete(dispatch_lane_class_t dqu,
		dispatch_qos_t qos, dispatch_wakeup_flags_t flags);
static void _dispatch_lane_non_barrier_complete(dispatch_lane_t dq,
		dispatch_wakeup_flags_t flags);
#if HAVE_PTHREAD_WORKQUEUE_QOS
static inline void _dispatch_queue_wakeup_with_override(
		dispatch_queue_class_t dq, uint64_t dq_state,
		dispatch_wakeup_flags_t flags);
#endif
static void _dispatch_workloop_drain_barrier_waiter(dispatch_workloop_t dwl,
		struct dispatch_object_s *dc, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags, uint64_t owned);
static inline bool
_dispatch_async_and_wait_should_always_async(dispatch_queue_class_t dqu,
		uint64_t dq_state);

#pragma mark -
#pragma mark dispatch_assert_queue

DISPATCH_NOINLINE DISPATCH_NORETURN
static void
_dispatch_assert_queue_fail(dispatch_queue_t dq, bool expected)
{
	_dispatch_client_assert_fail(
			"Block was %sexpected to execute on queue [%s]",
			expected ? "" : "not ", dq->dq_label ?: "");
}

DISPATCH_NOINLINE DISPATCH_NORETURN
static void
_dispatch_assert_queue_barrier_fail(dispatch_queue_t dq)
{
	_dispatch_client_assert_fail(
			"Block was expected to act as a barrier on queue [%s]",
			dq->dq_label ?: "");
}

void
dispatch_assert_queue(dispatch_queue_t dq)
{
	unsigned long metatype = dx_metatype(dq);
	if (unlikely(metatype != _DISPATCH_LANE_TYPE &&
			metatype != _DISPATCH_WORKLOOP_TYPE)) {
		DISPATCH_CLIENT_CRASH(metatype, "invalid queue passed to "
				"dispatch_assert_queue()");
	}
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
	if (likely(_dq_state_drain_locked_by_self(dq_state))) {
		return;
	}
	if (likely(_dispatch_thread_frame_find_queue(dq))) {
		return;
	}
	_dispatch_assert_queue_fail(dq, true);
}

void
dispatch_assert_queue_not(dispatch_queue_t dq)
{
	unsigned long metatype = dx_metatype(dq);
	if (unlikely(metatype != _DISPATCH_LANE_TYPE &&
			metatype != _DISPATCH_WORKLOOP_TYPE)) {
		DISPATCH_CLIENT_CRASH(metatype, "invalid queue passed to "
				"dispatch_assert_queue_not()");
	}
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
	if (unlikely(_dq_state_drain_locked_by_self(dq_state))) {
		_dispatch_assert_queue_fail(dq, false);
	}
	if (unlikely(_dispatch_thread_frame_find_queue(dq))) {
		_dispatch_assert_queue_fail(dq, false);
	}
}

void
dispatch_assert_queue_barrier(dispatch_queue_t dq)
{
	dispatch_assert_queue(dq);

	if (likely(dq->dq_width == 1)) {
		return;
	}

	if (likely(dq->do_targetq)) {
		uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
		if (likely(_dq_state_is_in_barrier(dq_state))) {
			return;
		}
	}

	_dispatch_assert_queue_barrier_fail(dq);
}

#pragma mark -
#pragma mark _dispatch_set_priority_and_mach_voucher
#if HAVE_PTHREAD_WORKQUEUE_QOS

DISPATCH_NOINLINE
void
_dispatch_set_priority_and_mach_voucher_slow(pthread_priority_t pp,
		mach_voucher_t kv)
{
	_pthread_set_flags_t pflags = (_pthread_set_flags_t)0;
	if (pp && _dispatch_set_qos_class_enabled) {
		pthread_priority_t old_pri = _dispatch_get_priority();
		if (pp != old_pri) {
			if (old_pri & _PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG) {
				pflags |= _PTHREAD_SET_SELF_WQ_KEVENT_UNBIND;
				// when we unbind, overcomitness can flip, so we need to learn
				// it from the defaultpri, see _dispatch_priority_compute_update
				pp |= (_dispatch_get_basepri() &
						DISPATCH_PRIORITY_FLAG_OVERCOMMIT);
			} else {
				// else we need to keep the one that is set in the current pri
				pp |= (old_pri & _PTHREAD_PRIORITY_OVERCOMMIT_FLAG);
			}
			if (likely(old_pri & ~_PTHREAD_PRIORITY_FLAGS_MASK)) {
				pflags |= _PTHREAD_SET_SELF_QOS_FLAG;
			}
			uint64_t mgr_dq_state =
					os_atomic_load2o(&_dispatch_mgr_q, dq_state, relaxed);
			if (unlikely(_dq_state_drain_locked_by_self(mgr_dq_state))) {
				DISPATCH_INTERNAL_CRASH(pp,
						"Changing the QoS while on the manager queue");
			}
			if (unlikely(pp & _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG)) {
				DISPATCH_INTERNAL_CRASH(pp, "Cannot raise oneself to manager");
			}
			if (old_pri & _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG) {
				DISPATCH_INTERNAL_CRASH(old_pri,
						"Cannot turn a manager thread into a normal one");
			}
		}
	}
	if (kv != VOUCHER_NO_MACH_VOUCHER) {
#if VOUCHER_USE_MACH_VOUCHER
		pflags |= _PTHREAD_SET_SELF_VOUCHER_FLAG;
#endif
	}
	if (!pflags) return;
	int r = _pthread_set_properties_self(pflags, pp, kv);
	if (r == EINVAL) {
		DISPATCH_INTERNAL_CRASH(pp, "_pthread_set_properties_self failed");
	}
	(void)dispatch_assume_zero(r);
}

DISPATCH_NOINLINE
voucher_t
_dispatch_set_priority_and_voucher_slow(pthread_priority_t priority,
		voucher_t v, dispatch_thread_set_self_t flags)
{
	voucher_t ov = DISPATCH_NO_VOUCHER;
	mach_voucher_t kv = VOUCHER_NO_MACH_VOUCHER;
	if (v != DISPATCH_NO_VOUCHER) {
		bool retained = flags & DISPATCH_VOUCHER_CONSUME;
		ov = _voucher_get();
		if (ov == v && (flags & DISPATCH_VOUCHER_REPLACE)) {
			if (retained && v) _voucher_release_no_dispose(v);
			ov = DISPATCH_NO_VOUCHER;
		} else {
			if (!retained && v) _voucher_retain(v);
			kv = _voucher_swap_and_get_mach_voucher(ov, v);
		}
	}
	if (!(flags & DISPATCH_THREAD_PARK)) {
		_dispatch_set_priority_and_mach_voucher_slow(priority, kv);
	}
	if (ov != DISPATCH_NO_VOUCHER && (flags & DISPATCH_VOUCHER_REPLACE)) {
		if (ov) _voucher_release(ov);
		ov = DISPATCH_NO_VOUCHER;
	}
	return ov;
}
#endif
#pragma mark -
#pragma mark dispatch_continuation_t

static void _dispatch_async_redirect_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
static void _dispatch_queue_override_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
static void _dispatch_workloop_stealer_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);

const struct dispatch_continuation_vtable_s _dispatch_continuation_vtables[] = {
	DC_VTABLE_ENTRY(ASYNC_REDIRECT,
		.do_invoke = _dispatch_async_redirect_invoke),
#if HAVE_MACH
	DC_VTABLE_ENTRY(MACH_SEND_BARRRIER_DRAIN,
		.do_invoke = _dispatch_mach_send_barrier_drain_invoke),
	DC_VTABLE_ENTRY(MACH_SEND_BARRIER,
		.do_invoke = _dispatch_mach_barrier_invoke),
	DC_VTABLE_ENTRY(MACH_RECV_BARRIER,
		.do_invoke = _dispatch_mach_barrier_invoke),
	DC_VTABLE_ENTRY(MACH_ASYNC_REPLY,
		.do_invoke = _dispatch_mach_msg_async_reply_invoke),
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS
	DC_VTABLE_ENTRY(WORKLOOP_STEALING,
		.do_invoke = _dispatch_workloop_stealer_invoke),
	DC_VTABLE_ENTRY(OVERRIDE_STEALING,
		.do_invoke = _dispatch_queue_override_invoke),
	DC_VTABLE_ENTRY(OVERRIDE_OWNING,
		.do_invoke = _dispatch_queue_override_invoke),
#endif
#if HAVE_MACH
	DC_VTABLE_ENTRY(MACH_IPC_HANDOFF,
		.do_invoke = _dispatch_mach_ipc_handoff_invoke),
#endif
};

DISPATCH_NOINLINE
static void DISPATCH_TSD_DTOR_CC
_dispatch_cache_cleanup(void *value)
{
	dispatch_continuation_t dc, next_dc = value;

	while ((dc = next_dc)) {
		next_dc = dc->do_next;
		_dispatch_continuation_free_to_heap(dc);
	}
}

static void
_dispatch_force_cache_cleanup(void)
{
	dispatch_continuation_t dc;
	dc = _dispatch_thread_getspecific(dispatch_cache_key);
	if (dc) {
		_dispatch_thread_setspecific(dispatch_cache_key, NULL);
		_dispatch_cache_cleanup(dc);
	}
}

#if DISPATCH_USE_MEMORYPRESSURE_SOURCE
DISPATCH_NOINLINE
void
_dispatch_continuation_free_to_cache_limit(dispatch_continuation_t dc)
{
	_dispatch_continuation_free_to_heap(dc);
	dispatch_continuation_t next_dc;
	dc = _dispatch_thread_getspecific(dispatch_cache_key);
	int cnt;
	if (!dc || (cnt = dc->dc_cache_cnt -
			_dispatch_continuation_cache_limit) <= 0) {
		return;
	}
	do {
		next_dc = dc->do_next;
		_dispatch_continuation_free_to_heap(dc);
	} while (--cnt && (dc = next_dc));
	_dispatch_thread_setspecific(dispatch_cache_key, next_dc);
}
#endif

DISPATCH_NOINLINE
void
_dispatch_continuation_pop(dispatch_object_t dou, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags, dispatch_queue_class_t dqu)
{
	_dispatch_continuation_pop_inline(dou, dic, flags, dqu._dq);
}

#pragma mark -
#pragma mark dispatch_block_create

#if __BLOCKS__

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_block_flags_valid(dispatch_block_flags_t flags)
{
	return ((flags & ~DISPATCH_BLOCK_API_MASK) == 0);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_block_flags_t
_dispatch_block_normalize_flags(dispatch_block_flags_t flags)
{
	if (flags & (DISPATCH_BLOCK_NO_QOS_CLASS|DISPATCH_BLOCK_DETACHED)) {
		flags |= DISPATCH_BLOCK_HAS_PRIORITY;
	}
	if (flags & DISPATCH_BLOCK_ENFORCE_QOS_CLASS) {
		flags &= ~(dispatch_block_flags_t)DISPATCH_BLOCK_INHERIT_QOS_CLASS;
	}
	return flags;
}

static inline dispatch_block_t
_dispatch_block_create_with_voucher_and_priority(dispatch_block_flags_t flags,
		voucher_t voucher, pthread_priority_t pri, dispatch_block_t block)
{
	dispatch_block_flags_t unmodified_flags = flags;
	pthread_priority_t unmodified_pri = pri;

	flags = _dispatch_block_normalize_flags(flags);
	bool assign = (flags & DISPATCH_BLOCK_ASSIGN_CURRENT);

	if (!(flags & DISPATCH_BLOCK_HAS_VOUCHER)) {
		if (flags & DISPATCH_BLOCK_DETACHED) {
			voucher = VOUCHER_NULL;
			flags |= DISPATCH_BLOCK_HAS_VOUCHER;
		} else if (flags & DISPATCH_BLOCK_NO_VOUCHER) {
			voucher = DISPATCH_NO_VOUCHER;
			flags |= DISPATCH_BLOCK_HAS_VOUCHER;
		} else if (assign) {
#if OS_VOUCHER_ACTIVITY_SPI
			voucher = VOUCHER_CURRENT;
#endif
			flags |= DISPATCH_BLOCK_HAS_VOUCHER;
		}
	}
#if OS_VOUCHER_ACTIVITY_SPI
	if (voucher == VOUCHER_CURRENT) {
		voucher = _voucher_get();
	}
#endif
	if (assign && !(flags & DISPATCH_BLOCK_HAS_PRIORITY)) {
		pri = _dispatch_priority_propagate();
		flags |= DISPATCH_BLOCK_HAS_PRIORITY;
	}
	dispatch_block_t db = _dispatch_block_create(flags, voucher, pri, block);

#if DISPATCH_DEBUG
	dispatch_assert(_dispatch_block_get_data(db));
#endif

	_dispatch_trace_block_create_with_voucher_and_priority(db,
			_dispatch_Block_invoke(block), unmodified_flags,
			((unmodified_flags & DISPATCH_BLOCK_HAS_PRIORITY) ? unmodified_pri :
					(unsigned long)UINT32_MAX),
			_dispatch_get_priority(), pri);
	return db;
}

dispatch_block_t
dispatch_block_create(dispatch_block_flags_t flags, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) return DISPATCH_BAD_INPUT;
	return _dispatch_block_create_with_voucher_and_priority(flags, NULL, 0,
			block);
}

dispatch_block_t
dispatch_block_create_with_qos_class(dispatch_block_flags_t flags,
		dispatch_qos_class_t qos_class, int relative_priority,
		dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags) ||
			!_dispatch_qos_class_valid(qos_class, relative_priority)) {
		return DISPATCH_BAD_INPUT;
	}
	flags |= DISPATCH_BLOCK_HAS_PRIORITY;
	pthread_priority_t pri = 0;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pri = _pthread_qos_class_encode(qos_class, relative_priority, 0);
#endif
	return _dispatch_block_create_with_voucher_and_priority(flags, NULL,
			pri, block);
}

dispatch_block_t
dispatch_block_create_with_voucher(dispatch_block_flags_t flags,
		voucher_t voucher, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) return DISPATCH_BAD_INPUT;
	flags |= DISPATCH_BLOCK_HAS_VOUCHER;
	flags &= ~DISPATCH_BLOCK_NO_VOUCHER;
	return _dispatch_block_create_with_voucher_and_priority(flags, voucher, 0,
			block);
}

dispatch_block_t
dispatch_block_create_with_voucher_and_qos_class(dispatch_block_flags_t flags,
		voucher_t voucher, dispatch_qos_class_t qos_class,
		int relative_priority, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags) ||
			!_dispatch_qos_class_valid(qos_class, relative_priority)) {
		return DISPATCH_BAD_INPUT;
	}
	flags |= (DISPATCH_BLOCK_HAS_VOUCHER|DISPATCH_BLOCK_HAS_PRIORITY);
	flags &= ~(DISPATCH_BLOCK_NO_VOUCHER|DISPATCH_BLOCK_NO_QOS_CLASS);
	pthread_priority_t pri = 0;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pri = _pthread_qos_class_encode(qos_class, relative_priority, 0);
#endif
	return _dispatch_block_create_with_voucher_and_priority(flags, voucher,
			pri, block);
}

void
dispatch_block_perform(dispatch_block_flags_t flags, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) {
		DISPATCH_CLIENT_CRASH(flags, "Invalid flags passed to "
				"dispatch_block_perform()");
	}
	flags = _dispatch_block_normalize_flags(flags);

	voucher_t voucher = DISPATCH_NO_VOUCHER;
	if (flags & DISPATCH_BLOCK_DETACHED) {
		voucher = VOUCHER_NULL;
		flags |= DISPATCH_BLOCK_HAS_VOUCHER;
	}

	struct dispatch_block_private_data_s dbpds =
		DISPATCH_BLOCK_PRIVATE_DATA_PERFORM_INITIALIZER(flags, block, voucher);
	return _dispatch_block_invoke_direct(&dbpds);
}

void
_dispatch_block_invoke_direct(const struct dispatch_block_private_data_s *dbcpd)
{
	dispatch_block_private_data_t dbpd = (dispatch_block_private_data_t)dbcpd;
	dispatch_block_flags_t flags = dbpd->dbpd_flags;
	unsigned int atomic_flags = dbpd->dbpd_atomic_flags;
	if (unlikely(atomic_flags & DBF_WAITED)) {
		DISPATCH_CLIENT_CRASH(atomic_flags, "A block object may not be both "
				"run more than once and waited for");
	}
	if (atomic_flags & DBF_CANCELED) goto out;

	pthread_priority_t op = 0, p = 0;
	op = _dispatch_block_invoke_should_set_priority(flags, dbpd->dbpd_priority);
	if (op) {
		p = dbpd->dbpd_priority;
	}
	voucher_t ov, v = DISPATCH_NO_VOUCHER;
	if (flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		v = dbpd->dbpd_voucher;
	}
	ov = _dispatch_set_priority_and_voucher(p, v, 0);
	dbpd->dbpd_thread = _dispatch_tid_self();
	_dispatch_client_callout(dbpd->dbpd_block,
			_dispatch_Block_invoke(dbpd->dbpd_block));
	_dispatch_reset_priority_and_voucher(op, ov);
out:
	if ((atomic_flags & DBF_PERFORM) == 0) {
		if (os_atomic_inc2o(dbpd, dbpd_performed, relaxed) == 1) {
			dispatch_group_leave(dbpd->dbpd_group);
		}
	}
}

void
_dispatch_block_sync_invoke(void *block)
{
	dispatch_block_t b = block;
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(b);
	dispatch_block_flags_t flags = dbpd->dbpd_flags;
	unsigned int atomic_flags = dbpd->dbpd_atomic_flags;
	if (unlikely(atomic_flags & DBF_WAITED)) {
		DISPATCH_CLIENT_CRASH(atomic_flags, "A block object may not be both "
				"run more than once and waited for");
	}
	if (atomic_flags & DBF_CANCELED) goto out;

	voucher_t ov = DISPATCH_NO_VOUCHER;
	if (flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		ov = _dispatch_adopt_priority_and_set_voucher(0, dbpd->dbpd_voucher, 0);
	}
	dbpd->dbpd_block();
	_dispatch_reset_voucher(ov, 0);
out:
	if ((atomic_flags & DBF_PERFORM) == 0) {
		if (os_atomic_inc2o(dbpd, dbpd_performed, relaxed) == 1) {
			dispatch_group_leave(dbpd->dbpd_group);
		}
	}

	dispatch_queue_t boost_dq;
	boost_dq = os_atomic_xchg2o(dbpd, dbpd_queue, NULL, relaxed);
	if (boost_dq) {
		// balances dispatch_{,barrier_,}sync
		_dispatch_release_2(boost_dq);
	}
}

#define DISPATCH_BLOCK_ASYNC_INVOKE_RELEASE           0x1

DISPATCH_NOINLINE
static void
_dispatch_block_async_invoke2(dispatch_block_t b, unsigned long invoke_flags)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(b);
	unsigned int atomic_flags = dbpd->dbpd_atomic_flags;
	if (unlikely(atomic_flags & DBF_WAITED)) {
		DISPATCH_CLIENT_CRASH(atomic_flags, "A block object may not be both "
				"run more than once and waited for");
	}

	if (likely(!(atomic_flags & DBF_CANCELED))) {
		dbpd->dbpd_block();
	}
	if ((atomic_flags & DBF_PERFORM) == 0) {
		if (os_atomic_inc2o(dbpd, dbpd_performed, relaxed) == 1) {
			dispatch_group_leave(dbpd->dbpd_group);
		}
	}

	dispatch_queue_t boost_dq;
	boost_dq = os_atomic_xchg2o(dbpd, dbpd_queue, NULL, relaxed);
	if (boost_dq) {
		// balances dispatch_{,barrier_,group_}async
		_dispatch_release_2(boost_dq);
	}

	if (invoke_flags & DISPATCH_BLOCK_ASYNC_INVOKE_RELEASE) {
		Block_release(b);
	}
}

static void
_dispatch_block_async_invoke(void *block)
{
	_dispatch_block_async_invoke2(block, 0);
}

static void
_dispatch_block_async_invoke_and_release(void *block)
{
	_dispatch_block_async_invoke2(block, DISPATCH_BLOCK_ASYNC_INVOKE_RELEASE);
}

void
dispatch_block_cancel(dispatch_block_t db)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (unlikely(!dbpd)) {
		DISPATCH_CLIENT_CRASH(0, "Invalid block object passed to "
				"dispatch_block_cancel()");
	}
	(void)os_atomic_or2o(dbpd, dbpd_atomic_flags, DBF_CANCELED, relaxed);
}

long
dispatch_block_testcancel(dispatch_block_t db)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (unlikely(!dbpd)) {
		DISPATCH_CLIENT_CRASH(0, "Invalid block object passed to "
				"dispatch_block_testcancel()");
	}
	return (bool)(dbpd->dbpd_atomic_flags & DBF_CANCELED);
}

long
dispatch_block_wait(dispatch_block_t db, dispatch_time_t timeout)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (unlikely(!dbpd)) {
		DISPATCH_CLIENT_CRASH(0, "Invalid block object passed to "
				"dispatch_block_wait()");
	}

	unsigned int flags = os_atomic_or_orig2o(dbpd, dbpd_atomic_flags,
			DBF_WAITING, relaxed);
	if (unlikely(flags & (DBF_WAITED | DBF_WAITING))) {
		DISPATCH_CLIENT_CRASH(flags, "A block object may not be waited for "
				"more than once");
	}

	// <rdar://problem/17703192> If we know the queue where this block is
	// enqueued, or the thread that's executing it, then we should boost
	// it here.

	pthread_priority_t pp = _dispatch_get_priority();

	dispatch_queue_t boost_dq;
	boost_dq = os_atomic_xchg2o(dbpd, dbpd_queue, NULL, relaxed);
	if (boost_dq) {
		// release balances dispatch_{,barrier_,group_}async.
		// Can't put the queue back in the timeout case: the block might
		// finish after we fell out of group_wait and see our NULL, so
		// neither of us would ever release. Side effect: After a _wait
		// that times out, subsequent waits will not boost the qos of the
		// still-running block.
		dx_wakeup(boost_dq, _dispatch_qos_from_pp(pp),
				DISPATCH_WAKEUP_BLOCK_WAIT | DISPATCH_WAKEUP_CONSUME_2);
	}

	mach_port_t boost_th = dbpd->dbpd_thread;
	if (boost_th) {
		_dispatch_thread_override_start(boost_th, pp, dbpd);
	}

	int performed = os_atomic_load2o(dbpd, dbpd_performed, relaxed);
	if (unlikely(performed > 1 || (boost_th && boost_dq))) {
		DISPATCH_CLIENT_CRASH(performed, "A block object may not be both "
				"run more than once and waited for");
	}

	long ret = dispatch_group_wait(dbpd->dbpd_group, timeout);

	if (boost_th) {
		_dispatch_thread_override_end(boost_th, dbpd);
	}

	if (ret) {
		// timed out: reverse our changes
		os_atomic_and2o(dbpd, dbpd_atomic_flags, ~DBF_WAITING, relaxed);
	} else {
		os_atomic_or2o(dbpd, dbpd_atomic_flags, DBF_WAITED, relaxed);
		// don't need to re-test here: the second call would see
		// the first call's WAITING
	}

	return ret;
}

void
dispatch_block_notify(dispatch_block_t db, dispatch_queue_t queue,
		dispatch_block_t notification_block)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (!dbpd) {
		DISPATCH_CLIENT_CRASH(db, "Invalid block object passed to "
				"dispatch_block_notify()");
	}
	int performed = os_atomic_load2o(dbpd, dbpd_performed, relaxed);
	if (unlikely(performed > 1)) {
		DISPATCH_CLIENT_CRASH(performed, "A block object may not be both "
				"run more than once and observed");
	}

	return dispatch_group_notify(dbpd->dbpd_group, queue, notification_block);
}

DISPATCH_NOINLINE
dispatch_qos_t
_dispatch_continuation_init_slow(dispatch_continuation_t dc,
		dispatch_queue_t dq, dispatch_block_flags_t flags)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(dc->dc_ctxt);
	dispatch_block_flags_t block_flags = dbpd->dbpd_flags;
	uintptr_t dc_flags = dc->dc_flags;
	pthread_priority_t pp = 0;

	// balanced in d_block_async_invoke_and_release or d_block_wait
	if (os_atomic_cmpxchg2o(dbpd, dbpd_queue, NULL, dq, relaxed)) {
		_dispatch_retain_2(dq);
	}

	if (dc_flags & DC_FLAG_CONSUME) {
		dc->dc_func = _dispatch_block_async_invoke_and_release;
	} else {
		dc->dc_func = _dispatch_block_async_invoke;
	}

	flags |= block_flags;
	if (block_flags & DISPATCH_BLOCK_HAS_PRIORITY) {
		pp = dbpd->dbpd_priority & ~_PTHREAD_PRIORITY_FLAGS_MASK;
	} else if (flags & DISPATCH_BLOCK_HAS_PRIORITY) {
		// _dispatch_source_handler_alloc is calling is and doesn't want us
		// to propagate priorities
		pp = 0;
	} else {
		pp = _dispatch_priority_propagate();
	}
	_dispatch_continuation_priority_set(dc, dq, pp, flags);
	if (block_flags & DISPATCH_BLOCK_BARRIER) {
		dc_flags |= DC_FLAG_BARRIER;
	}
	if (block_flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		voucher_t v = dbpd->dbpd_voucher;
		dc->dc_voucher = (v && v != DISPATCH_NO_VOUCHER) ? _voucher_retain(v)
				: v;
		_dispatch_voucher_debug("continuation[%p] set", dc->dc_voucher, dc);
		_dispatch_voucher_ktrace_dc_push(dc);
	} else {
		_dispatch_continuation_voucher_set(dc, flags);
	}
	dc_flags |= DC_FLAG_BLOCK_WITH_PRIVATE_DATA;
	dc->dc_flags = dc_flags;
	return _dispatch_qos_from_pp(dc->dc_priority);
}

#endif // __BLOCKS__
#pragma mark -
#pragma mark dispatch_barrier_async

DISPATCH_NOINLINE
static void
_dispatch_async_f_slow(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, dispatch_block_flags_t flags,
		uintptr_t dc_flags)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc_from_heap();
	dispatch_qos_t qos;

	qos = _dispatch_continuation_init_f(dc, dq, ctxt, func, flags, dc_flags);
	_dispatch_continuation_async(dq, dc, qos, dc->dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_barrier_async_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc_cacheonly();
	uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_BARRIER;
	dispatch_qos_t qos;

	if (likely(!dc)) {
		return _dispatch_async_f_slow(dq, ctxt, func, 0, dc_flags);
	}

	qos = _dispatch_continuation_init_f(dc, dq, ctxt, func, 0, dc_flags);
	_dispatch_continuation_async(dq, dc, qos, dc_flags);
}

DISPATCH_NOINLINE
void
_dispatch_barrier_async_detached_f(dispatch_queue_class_t dq, void *ctxt,
		dispatch_function_t func)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->dc_flags = DC_FLAG_CONSUME | DC_FLAG_BARRIER | DC_FLAG_ALLOCATED;
	dc->dc_func = func;
	dc->dc_ctxt = ctxt;
	dc->dc_voucher = DISPATCH_NO_VOUCHER;
	dc->dc_priority = DISPATCH_NO_PRIORITY;
	_dispatch_trace_item_push(dq, dc);
	dx_push(dq._dq, dc, 0);
}

#ifdef __BLOCKS__
void
dispatch_barrier_async(dispatch_queue_t dq, dispatch_block_t work)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_BARRIER;
	dispatch_qos_t qos;

	qos = _dispatch_continuation_init(dc, dq, work, 0, dc_flags);
	_dispatch_continuation_async(dq, dc, qos, dc_flags);
}
#endif

#pragma mark -
#pragma mark dispatch_async

void
_dispatch_async_redirect_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags)
{
	dispatch_thread_frame_s dtf;
	struct dispatch_continuation_s *other_dc = dc->dc_other;
	dispatch_invoke_flags_t ctxt_flags = (dispatch_invoke_flags_t)dc->dc_ctxt;
	// if we went through _dispatch_root_queue_push_override,
	// the "right" root queue was stuffed into dc_func
	dispatch_queue_global_t assumed_rq = (dispatch_queue_global_t)dc->dc_func;
	dispatch_lane_t dq = dc->dc_data;
	dispatch_queue_t rq, old_dq;
	dispatch_priority_t old_dbp;

	if (ctxt_flags) {
		flags &= ~_DISPATCH_INVOKE_AUTORELEASE_MASK;
		flags |= ctxt_flags;
	}
	old_dq = _dispatch_queue_get_current();
	if (assumed_rq) {
		old_dbp = _dispatch_root_queue_identity_assume(assumed_rq);
		_dispatch_set_basepri(dq->dq_priority);
	} else {
		old_dbp = _dispatch_set_basepri(dq->dq_priority);
	}

	uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_NO_INTROSPECTION;
	_dispatch_thread_frame_push(&dtf, dq);
	_dispatch_continuation_pop_forwarded(dc, dc_flags, NULL, {
		_dispatch_continuation_pop(other_dc, dic, flags, dq);
	});
	_dispatch_thread_frame_pop(&dtf);
	if (assumed_rq) _dispatch_queue_set_current(old_dq);
	_dispatch_reset_basepri(old_dbp);

	rq = dq->do_targetq;
	while (unlikely(rq->do_targetq && rq != old_dq)) {
		_dispatch_lane_non_barrier_complete(upcast(rq)._dl, 0);
		rq = rq->do_targetq;
	}

	// pairs with _dispatch_async_redirect_wrap
	_dispatch_lane_non_barrier_complete(dq, DISPATCH_WAKEUP_CONSUME_2);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_async_redirect_wrap(dispatch_lane_t dq, dispatch_object_t dou)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();

	dou._do->do_next = NULL;
	dc->do_vtable = DC_VTABLE(ASYNC_REDIRECT);
	dc->dc_func = NULL;
	dc->dc_ctxt = (void *)(uintptr_t)_dispatch_queue_autorelease_frequency(dq);
	dc->dc_data = dq;
	dc->dc_other = dou._do;
	dc->dc_voucher = DISPATCH_NO_VOUCHER;
	dc->dc_priority = DISPATCH_NO_PRIORITY;
	_dispatch_retain_2(dq); // released in _dispatch_async_redirect_invoke
	return dc;
}

DISPATCH_NOINLINE
static void
_dispatch_continuation_redirect_push(dispatch_lane_t dl,
		dispatch_object_t dou, dispatch_qos_t qos)
{
	if (likely(!_dispatch_object_is_redirection(dou))) {
		dou._dc = _dispatch_async_redirect_wrap(dl, dou);
	} else if (!dou._dc->dc_ctxt) {
		// find first queue in descending target queue order that has
		// an autorelease frequency set, and use that as the frequency for
		// this continuation.
		dou._dc->dc_ctxt = (void *)
		(uintptr_t)_dispatch_queue_autorelease_frequency(dl);
	}

	dispatch_queue_t dq = dl->do_targetq;
	if (!qos) qos = _dispatch_priority_qos(dq->dq_priority);
	dx_push(dq, dou, qos);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_async_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func,
		dispatch_block_flags_t flags)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc_cacheonly();
	uintptr_t dc_flags = DC_FLAG_CONSUME;
	dispatch_qos_t qos;

	if (unlikely(!dc)) {
		return _dispatch_async_f_slow(dq, ctxt, func, flags, dc_flags);
	}

	qos = _dispatch_continuation_init_f(dc, dq, ctxt, func, flags, dc_flags);
	_dispatch_continuation_async(dq, dc, qos, dc->dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_async_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func)
{
	_dispatch_async_f(dq, ctxt, func, 0);
}

DISPATCH_NOINLINE
void
dispatch_async_enforce_qos_class_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	_dispatch_async_f(dq, ctxt, func, DISPATCH_BLOCK_ENFORCE_QOS_CLASS);
}

#ifdef __BLOCKS__
void
dispatch_async(dispatch_queue_t dq, dispatch_block_t work)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	uintptr_t dc_flags = DC_FLAG_CONSUME;
	dispatch_qos_t qos;

	qos = _dispatch_continuation_init(dc, dq, work, 0, dc_flags);
	_dispatch_continuation_async(dq, dc, qos, dc->dc_flags);
}
#endif

#pragma mark -
#pragma mark _dispatch_sync_invoke / _dispatch_sync_complete

DISPATCH_ALWAYS_INLINE
static uint64_t
_dispatch_lane_non_barrier_complete_try_lock(dispatch_lane_t dq,
		uint64_t old_state, uint64_t new_state, uint64_t owner_self)
{
	uint64_t full_width = new_state;
	if (_dq_state_has_pending_barrier(new_state)) {
		full_width -= DISPATCH_QUEUE_PENDING_BARRIER;
		full_width += DISPATCH_QUEUE_WIDTH_INTERVAL;
		full_width += DISPATCH_QUEUE_IN_BARRIER;
	} else {
		full_width += dq->dq_width * DISPATCH_QUEUE_WIDTH_INTERVAL;
		full_width += DISPATCH_QUEUE_IN_BARRIER;
	}
	if ((full_width & DISPATCH_QUEUE_WIDTH_MASK) ==
			DISPATCH_QUEUE_WIDTH_FULL_BIT) {
		new_state = full_width;
		new_state &= ~DISPATCH_QUEUE_DIRTY;
		new_state |= owner_self;
	} else if (_dq_state_is_dirty(old_state)) {
		new_state |= DISPATCH_QUEUE_ENQUEUED;
	}
	return new_state;
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_lane_non_barrier_complete_finish(dispatch_lane_t dq,
		dispatch_wakeup_flags_t flags, uint64_t old_state, uint64_t new_state)
{
	if (_dq_state_received_override(old_state)) {
		// Ensure that the root queue sees that this thread was overridden.
		_dispatch_set_basepri_override_qos(_dq_state_max_qos(old_state));
	}

	if ((old_state ^ new_state) & DISPATCH_QUEUE_IN_BARRIER) {
		if (_dq_state_is_dirty(old_state)) {
			// <rdar://problem/14637483>
			// dependency ordering for dq state changes that were flushed
			// and not acted upon
			os_atomic_thread_fence(dependency);
			dq = os_atomic_force_dependency_on(dq, old_state);
		}
		return _dispatch_lane_barrier_complete(dq, 0, flags);
	}

	if ((old_state ^ new_state) & DISPATCH_QUEUE_ENQUEUED) {
		if (!(flags & DISPATCH_WAKEUP_CONSUME_2)) {
			_dispatch_retain_2(dq);
		}
		dispatch_assert(!_dq_state_is_base_wlh(new_state));
		_dispatch_trace_item_push(dq->do_targetq, dq);
		return dx_push(dq->do_targetq, dq, _dq_state_max_qos(new_state));
	}

	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		_dispatch_release_2_tailcall(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_lane_non_barrier_complete(dispatch_lane_t dq,
		dispatch_wakeup_flags_t flags)
{
	uint64_t old_state, new_state, owner_self = _dispatch_lock_value_for_self();

	// see _dispatch_lane_resume()
	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		new_state = old_state - DISPATCH_QUEUE_WIDTH_INTERVAL;
		if (unlikely(_dq_state_drain_locked(old_state))) {
			// make drain_try_unlock() fail and reconsider whether there's
			// enough width now for a new item
			new_state |= DISPATCH_QUEUE_DIRTY;
		} else if (likely(_dq_state_is_runnable(new_state))) {
			new_state = _dispatch_lane_non_barrier_complete_try_lock(dq,
					old_state, new_state, owner_self);
		}
	});

	_dispatch_lane_non_barrier_complete_finish(dq, flags, old_state, new_state);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sync_function_invoke_inline(dispatch_queue_class_t dq, void *ctxt,
		dispatch_function_t func)
{
	dispatch_thread_frame_s dtf;
	_dispatch_thread_frame_push(&dtf, dq);
	_dispatch_client_callout(ctxt, func);
	_dispatch_perfmon_workitem_inc();
	_dispatch_thread_frame_pop(&dtf);
}

DISPATCH_NOINLINE
static void
_dispatch_sync_function_invoke(dispatch_queue_class_t dq, void *ctxt,
		dispatch_function_t func)
{
	_dispatch_sync_function_invoke_inline(dq, ctxt, func);
}

DISPATCH_NOINLINE
static void
_dispatch_sync_complete_recurse(dispatch_queue_t dq, dispatch_queue_t stop_dq,
		uintptr_t dc_flags)
{
	bool barrier = (dc_flags & DC_FLAG_BARRIER);
	do {
		if (dq == stop_dq) return;
		if (barrier) {
			dx_wakeup(dq, 0, DISPATCH_WAKEUP_BARRIER_COMPLETE);
		} else {
			_dispatch_lane_non_barrier_complete(upcast(dq)._dl, 0);
		}
		dq = dq->do_targetq;
		barrier = (dq->dq_width == 1);
	} while (unlikely(dq->do_targetq));
}

DISPATCH_NOINLINE
static void
_dispatch_sync_invoke_and_complete_recurse(dispatch_queue_class_t dq,
		void *ctxt, dispatch_function_t func, uintptr_t dc_flags
		DISPATCH_TRACE_ARG(void *dc))
{
	_dispatch_sync_function_invoke_inline(dq, ctxt, func);
	_dispatch_trace_item_complete(dc);
	_dispatch_sync_complete_recurse(dq._dq, NULL, dc_flags);
}

DISPATCH_NOINLINE
static void
_dispatch_sync_invoke_and_complete(dispatch_lane_t dq, void *ctxt,
		dispatch_function_t func DISPATCH_TRACE_ARG(void *dc))
{
	_dispatch_sync_function_invoke_inline(dq, ctxt, func);
	_dispatch_trace_item_complete(dc);
	_dispatch_lane_non_barrier_complete(dq, 0);
}

/*
 * For queues we can cheat and inline the unlock code, which is invalid
 * for objects with a more complex state machine (sources or mach channels)
 */
DISPATCH_NOINLINE
static void
_dispatch_lane_barrier_sync_invoke_and_complete(dispatch_lane_t dq,
		void *ctxt, dispatch_function_t func DISPATCH_TRACE_ARG(void *dc))
{
	_dispatch_sync_function_invoke_inline(dq, ctxt, func);
	_dispatch_trace_item_complete(dc);
	if (unlikely(dq->dq_items_tail || dq->dq_width > 1)) {
		return _dispatch_lane_barrier_complete(dq, 0, 0);
	}

	// Presence of any of these bits requires more work that only
	// _dispatch_*_barrier_complete() handles properly
	//
	// Note: testing for RECEIVED_OVERRIDE or RECEIVED_SYNC_WAIT without
	// checking the role is sloppy, but is a super fast check, and neither of
	// these bits should be set if the lock was never contended/discovered.
	const uint64_t fail_unlock_mask = DISPATCH_QUEUE_SUSPEND_BITS_MASK |
			DISPATCH_QUEUE_ENQUEUED | DISPATCH_QUEUE_DIRTY |
			DISPATCH_QUEUE_RECEIVED_OVERRIDE | DISPATCH_QUEUE_SYNC_TRANSFER |
			DISPATCH_QUEUE_RECEIVED_SYNC_WAIT;
	uint64_t old_state, new_state;

	// similar to _dispatch_queue_drain_try_unlock
	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
		new_state  = old_state - DISPATCH_QUEUE_SERIAL_DRAIN_OWNED;
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
		if (unlikely(old_state & fail_unlock_mask)) {
			os_atomic_rmw_loop_give_up({
				return _dispatch_lane_barrier_complete(dq, 0, 0);
			});
		}
	});
	if (_dq_state_is_base_wlh(old_state)) {
		_dispatch_event_loop_assert_not_owned((dispatch_wlh_t)dq);
	}
}

#pragma mark -
#pragma mark _dispatch_sync_wait / _dispatch_sync_waiter_wake

DISPATCH_NOINLINE
static void
_dispatch_waiter_wake_wlh_anon(dispatch_sync_context_t dsc)
{
	if (dsc->dsc_override_qos > dsc->dsc_override_qos_floor) {
		_dispatch_wqthread_override_start(dsc->dsc_waiter,
				dsc->dsc_override_qos);
	}
	_dispatch_thread_event_signal(&dsc->dsc_event);
}

DISPATCH_NOINLINE
static void
_dispatch_waiter_wake(dispatch_sync_context_t dsc, dispatch_wlh_t wlh,
		uint64_t old_state, uint64_t new_state)
{
	dispatch_wlh_t waiter_wlh = dsc->dc_data;

#if DISPATCH_USE_KEVENT_WORKLOOP
	//
	// We need to interact with a workloop if any of the following 3 cases:
	// 1. the current owner of the lock has a SYNC_WAIT knote to destroy
	// 2. the next owner of the lock is a workloop, we need to make sure it has
	//    a SYNC_WAIT knote to destroy when it will later release the lock
	// 3. the waiter is waiting on a workloop (which may be different from `wlh`
	//    if the hierarchy was mutated after the next owner started waiting)
	//
	// However, note that even when (2) is true, the next owner may be waiting
	// without pushing (waiter_wlh == DISPATCH_WLH_ANON), in which case the next
	// owner is really woken up when the thread event is signaled.
	//
#endif
	if (_dq_state_in_sync_transfer(old_state) ||
			_dq_state_in_sync_transfer(new_state) ||
			(waiter_wlh != DISPATCH_WLH_ANON)) {
		_dispatch_event_loop_wake_owner(dsc, wlh, old_state, new_state);
	}
	if (unlikely(waiter_wlh == DISPATCH_WLH_ANON)) {
		_dispatch_waiter_wake_wlh_anon(dsc);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_async_waiter_update(dispatch_sync_context_t dsc,
		dispatch_queue_class_t dqu)
{
	dispatch_queue_t dq = dqu._dq;
	dispatch_priority_t p = dq->dq_priority & DISPATCH_PRIORITY_REQUESTED_MASK;
	if (p) {
		pthread_priority_t pp = _dispatch_priority_to_pp_strip_flags(p);
		if (pp > (dsc->dc_priority & ~_PTHREAD_PRIORITY_FLAGS_MASK)) {
			dsc->dc_priority = pp | _PTHREAD_PRIORITY_ENFORCE_FLAG;
		}
	}

	if (dsc->dsc_autorelease == 0) {
		dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(dqu);
		dqf &= (dispatch_queue_flags_t)_DQF_AUTORELEASE_MASK;
		dsc->dsc_autorelease = (uint8_t)(dqf / DQF_AUTORELEASE_ALWAYS);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_non_barrier_waiter_redirect_or_wake(dispatch_lane_t dq,
		dispatch_object_t dou)
{
	dispatch_sync_context_t dsc = (dispatch_sync_context_t)dou._dc;
	uint64_t old_state;

	dispatch_assert(!(dsc->dc_flags & DC_FLAG_BARRIER));

again:
	old_state = os_atomic_load2o(dq, dq_state, relaxed);

	if (dsc->dsc_override_qos < _dq_state_max_qos(old_state)) {
		dsc->dsc_override_qos = (uint8_t)_dq_state_max_qos(old_state);
	}

	if (dsc->dc_flags & DC_FLAG_ASYNC_AND_WAIT) {
		_dispatch_async_waiter_update(dsc, dq);
	}

	if (unlikely(_dq_state_is_inner_queue(old_state))) {
		dispatch_queue_t tq = dq->do_targetq;
		if (likely(tq->dq_width == 1)) {
			dsc->dc_flags |= DC_FLAG_BARRIER;
		} else {
			dsc->dc_flags &= ~DC_FLAG_BARRIER;
			if (_dispatch_queue_try_reserve_sync_width(upcast(tq)._dl)) {
				dq = upcast(tq)._dl;
				goto again;
			}
		}
		return dx_push(tq, dsc, 0);
	}

	if (dsc->dc_flags & DC_FLAG_ASYNC_AND_WAIT) {
		// We're in case (2) of _dispatch_async_and_wait_f_slow() which expects
		// dc_other to be the bottom queue of the graph
		dsc->dc_other = dq;
	}
	return _dispatch_waiter_wake_wlh_anon(dsc);
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_waiter_redirect_or_wake(dispatch_queue_class_t dqu,
		dispatch_object_t dc, dispatch_wakeup_flags_t flags,
		uint64_t old_state, uint64_t new_state)
{
	dispatch_sync_context_t dsc = (dispatch_sync_context_t)dc._dc;
	dispatch_queue_t dq = dqu._dq;
	dispatch_wlh_t wlh = DISPATCH_WLH_ANON;

	if (dsc->dc_data == DISPATCH_WLH_ANON) {
		if (dsc->dsc_override_qos < _dq_state_max_qos(old_state)) {
			dsc->dsc_override_qos = (uint8_t)_dq_state_max_qos(old_state);
		}
	}

	if (_dq_state_is_base_wlh(old_state)) {
		wlh = (dispatch_wlh_t)dq;
	} else if (_dq_state_received_override(old_state)) {
		// Ensure that the root queue sees that this thread was overridden.
		_dispatch_set_basepri_override_qos(_dq_state_max_qos(old_state));
	}

	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		if (_dq_state_is_base_wlh(old_state) &&
				_dq_state_is_enqueued_on_target(new_state)) {
			// If the thread request still exists, we need to leave it a +1
			_dispatch_release_no_dispose(dq);
		} else {
			_dispatch_release_2_no_dispose(dq);
		}
	} else if (_dq_state_is_base_wlh(old_state) &&
			_dq_state_is_enqueued_on_target(old_state) &&
			!_dq_state_is_enqueued_on_target(new_state)) {
		// If we cleared the enqueued bit, we're about to destroy the workloop
		// thread request, and we need to consume its +1.
		_dispatch_release_no_dispose(dq);
	}

	//
	// Past this point we are borrowing the reference of the sync waiter
	//
	if (unlikely(_dq_state_is_inner_queue(old_state))) {
		dispatch_queue_t tq = dq->do_targetq;
		if (dsc->dc_flags & DC_FLAG_ASYNC_AND_WAIT) {
			_dispatch_async_waiter_update(dsc, dq);
		}
		if (likely(tq->dq_width == 1)) {
			dsc->dc_flags |= DC_FLAG_BARRIER;
		} else {
			dispatch_lane_t dl = upcast(tq)._dl;
			dsc->dc_flags &= ~DC_FLAG_BARRIER;
			if (_dispatch_queue_try_reserve_sync_width(dl)) {
				return _dispatch_non_barrier_waiter_redirect_or_wake(dl, dc);
			}
		}
		// passing the QoS of `dq` helps pushing on low priority waiters with
		// legacy workloops.
#if DISPATCH_INTROSPECTION
		dsc->dsc_from_async = false;
#endif
		return dx_push(tq, dsc, _dq_state_max_qos(old_state));
	}

#if DISPATCH_INTROSPECTION
	if (dsc->dsc_from_async) {
		_dispatch_trace_runtime_event(async_sync_handoff, dq, 0);
	} else {
		_dispatch_trace_runtime_event(sync_sync_handoff, dq, 0);
	}
#endif // DISPATCH_INTROSPECTION

	if (dsc->dc_flags & DC_FLAG_ASYNC_AND_WAIT) {
		// Falling into case (2) of _dispatch_async_and_wait_f_slow, dc_other is
		// the bottom queue
		dsc->dc_other = dq;
	}
	return _dispatch_waiter_wake(dsc, wlh, old_state, new_state);
}

DISPATCH_NOINLINE
static void
_dispatch_lane_drain_barrier_waiter(dispatch_lane_t dq,
		struct dispatch_object_s *dc, dispatch_wakeup_flags_t flags,
		uint64_t enqueued_bits)
{
	dispatch_sync_context_t dsc = (dispatch_sync_context_t)dc;
	struct dispatch_object_s *next_dc;
	uint64_t next_owner = 0, old_state, new_state;

	next_owner = _dispatch_lock_value_from_tid(dsc->dsc_waiter);
	next_dc = _dispatch_queue_pop_head(dq, dc);

transfer_lock_again:
	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
		new_state  = old_state;
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		new_state &= ~DISPATCH_QUEUE_DIRTY;
		new_state |= next_owner;

		if (_dq_state_is_base_wlh(old_state)) {
			new_state |= DISPATCH_QUEUE_SYNC_TRANSFER;
			if (next_dc) {
				// we know there's a next item, keep the enqueued bit if any
			} else if (unlikely(_dq_state_is_dirty(old_state))) {
				os_atomic_rmw_loop_give_up({
					os_atomic_xor2o(dq, dq_state, DISPATCH_QUEUE_DIRTY, acquire);
					next_dc = os_atomic_load2o(dq, dq_items_head, relaxed);
					goto transfer_lock_again;
				});
			} else {
				new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
				new_state &= ~DISPATCH_QUEUE_ENQUEUED;
			}
		} else {
			new_state -= enqueued_bits;
		}
	});

	return _dispatch_barrier_waiter_redirect_or_wake(dq, dc, flags,
			old_state, new_state);
}

DISPATCH_NOINLINE
static void
_dispatch_lane_class_barrier_complete(dispatch_lane_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags, dispatch_queue_wakeup_target_t target,
		uint64_t owned)
{
	uint64_t old_state, new_state, enqueue;
	dispatch_queue_t tq;

	if (target == DISPATCH_QUEUE_WAKEUP_MGR) {
		tq = _dispatch_mgr_q._as_dq;
		enqueue = DISPATCH_QUEUE_ENQUEUED_ON_MGR;
	} else if (target) {
		tq = (target == DISPATCH_QUEUE_WAKEUP_TARGET) ? dq->do_targetq : target;
		enqueue = DISPATCH_QUEUE_ENQUEUED;
	} else {
		tq = NULL;
		enqueue = 0;
	}

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
		new_state  = _dq_state_merge_qos(old_state - owned, qos);
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		if (unlikely(_dq_state_is_suspended(old_state))) {
			if (likely(_dq_state_is_base_wlh(old_state))) {
				new_state &= ~DISPATCH_QUEUE_ENQUEUED;
			}
		} else if (enqueue) {
			if (!_dq_state_is_enqueued(old_state)) {
				new_state |= enqueue;
			}
		} else if (unlikely(_dq_state_is_dirty(old_state))) {
			os_atomic_rmw_loop_give_up({
				// just renew the drain lock with an acquire barrier, to see
				// what the enqueuer that set DIRTY has done.
				// the xor generates better assembly as DISPATCH_QUEUE_DIRTY
				// is already in a register
				os_atomic_xor2o(dq, dq_state, DISPATCH_QUEUE_DIRTY, acquire);
				flags |= DISPATCH_WAKEUP_BARRIER_COMPLETE;
				return dx_wakeup(dq, qos, flags);
			});
		} else {
			new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
		}
	});
	old_state -= owned;
	dispatch_assert(_dq_state_drain_locked_by_self(old_state));
	dispatch_assert(!_dq_state_is_enqueued_on_manager(old_state));

	if (_dq_state_is_enqueued(new_state)) {
		_dispatch_trace_runtime_event(sync_async_handoff, dq, 0);
	}

#if DISPATCH_USE_KEVENT_WORKLOOP
	if (_dq_state_is_base_wlh(old_state)) {
		// - Only non-"du_is_direct" sources & mach channels can be enqueued
		//   on the manager.
		//
		// - Only dispatch_source_cancel_and_wait() and
		//   dispatch_source_set_*_handler() use the barrier complete codepath,
		//   none of which are used by mach channels.
		//
		// Hence no source-ish object can both be a workloop and need to use the
		// manager at the same time.
		dispatch_assert(!_dq_state_is_enqueued_on_manager(new_state));
		if (_dq_state_is_enqueued_on_target(old_state) ||
				_dq_state_is_enqueued_on_target(new_state) ||
				_dq_state_received_sync_wait(old_state) ||
				_dq_state_in_sync_transfer(old_state)) {
			return _dispatch_event_loop_end_ownership((dispatch_wlh_t)dq,
					old_state, new_state, flags);
		}
		_dispatch_event_loop_assert_not_owned((dispatch_wlh_t)dq);
		if (flags & DISPATCH_WAKEUP_CONSUME_2) {
			return _dispatch_release_2_tailcall(dq);
		}
		return;
	}
#endif

	if (_dq_state_received_override(old_state)) {
		// Ensure that the root queue sees that this thread was overridden.
		_dispatch_set_basepri_override_qos(_dq_state_max_qos(old_state));
	}

	if (tq) {
		if (likely((old_state ^ new_state) & enqueue)) {
			dispatch_assert(_dq_state_is_enqueued(new_state));
			dispatch_assert(flags & DISPATCH_WAKEUP_CONSUME_2);
			return _dispatch_queue_push_queue(tq, dq, new_state);
		}
#if HAVE_PTHREAD_WORKQUEUE_QOS
		// <rdar://problem/27694093> when doing sync to async handoff
		// if the queue received an override we have to forecefully redrive
		// the same override so that a new stealer is enqueued because
		// the previous one may be gone already
		if (_dq_state_should_override(new_state)) {
			return _dispatch_queue_wakeup_with_override(dq, new_state, flags);
		}
#endif
	}
	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		return _dispatch_release_2_tailcall(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_lane_drain_non_barriers(dispatch_lane_t dq,
		struct dispatch_object_s *dc, dispatch_wakeup_flags_t flags)
{
	size_t owned_width = dq->dq_width;
	struct dispatch_object_s *next_dc;

	// see _dispatch_lane_drain, go in non barrier mode, and drain items

	os_atomic_and2o(dq, dq_state, ~DISPATCH_QUEUE_IN_BARRIER, release);

	do {
		if (likely(owned_width)) {
			owned_width--;
		} else if (_dispatch_object_is_waiter(dc)) {
			// sync "readers" don't observe the limit
			_dispatch_queue_reserve_sync_width(dq);
		} else if (!_dispatch_queue_try_acquire_async(dq)) {
			// no width left
			break;
		}
		next_dc = _dispatch_queue_pop_head(dq, dc);
		if (_dispatch_object_is_waiter(dc)) {
			_dispatch_non_barrier_waiter_redirect_or_wake(dq, dc);
		} else {
			_dispatch_continuation_redirect_push(dq, dc,
					_dispatch_queue_max_qos(dq));
		}
drain_again:
		dc = next_dc;
	} while (dc && !_dispatch_object_is_barrier(dc));

	uint64_t old_state, new_state, owner_self = _dispatch_lock_value_for_self();
	uint64_t owned = owned_width * DISPATCH_QUEUE_WIDTH_INTERVAL;

	if (dc) {
		owned = _dispatch_queue_adjust_owned(dq, owned, dc);
	}

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		new_state  = old_state - owned;
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		new_state &= ~DISPATCH_QUEUE_DIRTY;

		// similar to _dispatch_lane_non_barrier_complete():
		// if by the time we get here all redirected non barrier syncs are
		// done and returned their width to the queue, we may be the last
		// chance for the next item to run/be re-driven.
		if (unlikely(dc)) {
			new_state |= DISPATCH_QUEUE_DIRTY;
			new_state = _dispatch_lane_non_barrier_complete_try_lock(dq,
					old_state, new_state, owner_self);
		} else if (unlikely(_dq_state_is_dirty(old_state))) {
			os_atomic_rmw_loop_give_up({
				os_atomic_xor2o(dq, dq_state, DISPATCH_QUEUE_DIRTY, acquire);
				next_dc = os_atomic_load2o(dq, dq_items_head, relaxed);
				goto drain_again;
			});
		}
	});

	old_state -= owned;
	_dispatch_lane_non_barrier_complete_finish(dq, flags, old_state, new_state);
}

DISPATCH_NOINLINE
static void
_dispatch_lane_barrier_complete(dispatch_lane_class_t dqu, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	dispatch_queue_wakeup_target_t target = DISPATCH_QUEUE_WAKEUP_NONE;
	dispatch_lane_t dq = dqu._dl;

	if (dq->dq_items_tail && !DISPATCH_QUEUE_IS_SUSPENDED(dq)) {
		struct dispatch_object_s *dc = _dispatch_queue_get_head(dq);
		if (likely(dq->dq_width == 1 || _dispatch_object_is_barrier(dc))) {
			if (_dispatch_object_is_waiter(dc)) {
				return _dispatch_lane_drain_barrier_waiter(dq, dc, flags, 0);
			}
		} else if (dq->dq_width > 1 && !_dispatch_object_is_barrier(dc)) {
			return _dispatch_lane_drain_non_barriers(dq, dc, flags);
		}

		if (!(flags & DISPATCH_WAKEUP_CONSUME_2)) {
			_dispatch_retain_2(dq);
			flags |= DISPATCH_WAKEUP_CONSUME_2;
		}
		target = DISPATCH_QUEUE_WAKEUP_TARGET;
	}

	uint64_t owned = DISPATCH_QUEUE_IN_BARRIER +
			dq->dq_width * DISPATCH_QUEUE_WIDTH_INTERVAL;
	return _dispatch_lane_class_barrier_complete(dq, qos, flags, target, owned);
}

static void
_dispatch_async_and_wait_invoke(void *ctxt)
{
	dispatch_sync_context_t dsc = ctxt;
	dispatch_queue_t top_dq = dsc->dc_other;
	dispatch_invoke_flags_t iflags;

	// the block runs on the thread the queue is bound to and not
	// on the calling thread, but we want to see the calling thread
	// dispatch thread frames, so we fake the link, and then undo it
	iflags = dsc->dsc_autorelease * DISPATCH_INVOKE_AUTORELEASE_ALWAYS;
	dispatch_invoke_with_autoreleasepool(iflags, {
		dispatch_thread_frame_s dtf;
		_dispatch_introspection_sync_begin(top_dq);
		_dispatch_thread_frame_push_and_rebase(&dtf, top_dq, &dsc->dsc_dtf);
		_dispatch_client_callout(dsc->dsc_ctxt, dsc->dsc_func);
		_dispatch_thread_frame_pop(&dtf);
	});

	// communicate back to _dispatch_async_and_wait_f_slow and
	// _dispatch_sync_f_slow on which queue the work item was invoked
	// so that the *_complete_recurse() call stops unlocking when it reaches it
	dsc->dc_other = _dispatch_queue_get_current();
	dsc->dsc_func = NULL;

	if (dsc->dc_data == DISPATCH_WLH_ANON) {
		_dispatch_thread_event_signal(&dsc->dsc_event); // release
	} else {
		_dispatch_event_loop_cancel_waiter(dsc);
	}
}

DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dispatch_wait_prepare(dispatch_queue_t dq)
{
	uint64_t old_state, new_state;

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		if (_dq_state_is_suspended(old_state) ||
				!_dq_state_is_base_wlh(old_state)) {
			os_atomic_rmw_loop_give_up(return old_state);
		}
		if (!_dq_state_drain_locked(old_state) ||
				_dq_state_in_sync_transfer(old_state)) {
			os_atomic_rmw_loop_give_up(return old_state);
		}
		new_state = old_state | DISPATCH_QUEUE_RECEIVED_SYNC_WAIT;
	});
	return new_state;
}

static void
_dispatch_wait_compute_wlh(dispatch_lane_t dq, dispatch_sync_context_t dsc)
{
	bool needs_locking = _dispatch_queue_is_mutable(dq);

	if (needs_locking) {
		dsc->dsc_release_storage = true;
		_dispatch_queue_sidelock_lock(dq);
	}

	dispatch_queue_t tq = dq->do_targetq;
	uint64_t tq_state = _dispatch_wait_prepare(tq);

	if (_dq_state_is_suspended(tq_state) ||
			_dq_state_is_base_anon(tq_state)) {
		dsc->dsc_release_storage = false;
		dsc->dc_data = DISPATCH_WLH_ANON;
	} else if (_dq_state_is_base_wlh(tq_state)) {
		if (dx_metatype(tq) == _DISPATCH_WORKLOOP_TYPE) {
			dsc->dsc_wlh_is_workloop = true;
			dsc->dsc_release_storage = false;
		} else if (dsc->dsc_release_storage) {
			_dispatch_queue_retain_storage(tq);
		}
		dsc->dc_data = (dispatch_wlh_t)tq;
	} else {
		_dispatch_wait_compute_wlh(upcast(tq)._dl, dsc);
	}
	if (needs_locking) {
		if (dsc->dsc_wlh_is_workloop) {
			_dispatch_queue_atomic_flags_clear(dq, DQF_MUTABLE);
		}
		_dispatch_queue_sidelock_unlock(dq);
	}
}

DISPATCH_NOINLINE
static void
__DISPATCH_WAIT_FOR_QUEUE__(dispatch_sync_context_t dsc, dispatch_queue_t dq)
{
	uint64_t dq_state = _dispatch_wait_prepare(dq);
	if (unlikely(_dq_state_drain_locked_by(dq_state, dsc->dsc_waiter))) {
		DISPATCH_CLIENT_CRASH((uintptr_t)dq_state,
				"dispatch_sync called on queue "
				"already owned by current thread");
	}

	// Blocks submitted to the main thread MUST run on the main thread, and
	// dispatch_async_and_wait also executes on the remote context rather than
	// the current thread.
	//
	// For both these cases we need to save the frame linkage for the sake of
	// _dispatch_async_and_wait_invoke
	_dispatch_thread_frame_save_state(&dsc->dsc_dtf);

	if (_dq_state_is_suspended(dq_state) ||
			_dq_state_is_base_anon(dq_state)) {
		dsc->dc_data = DISPATCH_WLH_ANON;
	} else if (_dq_state_is_base_wlh(dq_state)) {
		dsc->dc_data = (dispatch_wlh_t)dq;
	} else {
		_dispatch_wait_compute_wlh(upcast(dq)._dl, dsc);
	}

	if (dsc->dc_data == DISPATCH_WLH_ANON) {
		dsc->dsc_override_qos_floor = dsc->dsc_override_qos =
				(uint8_t)_dispatch_get_basepri_override_qos_floor();
		_dispatch_thread_event_init(&dsc->dsc_event);
	}
	dx_push(dq, dsc, _dispatch_qos_from_pp(dsc->dc_priority));
	_dispatch_trace_runtime_event(sync_wait, dq, 0);
	if (dsc->dc_data == DISPATCH_WLH_ANON) {
		_dispatch_thread_event_wait(&dsc->dsc_event); // acquire
	} else {
		_dispatch_event_loop_wait_for_ownership(dsc);
	}
	if (dsc->dc_data == DISPATCH_WLH_ANON) {
		_dispatch_thread_event_destroy(&dsc->dsc_event);
		// If _dispatch_sync_waiter_wake() gave this thread an override,
		// ensure that the root queue sees it.
		if (dsc->dsc_override_qos > dsc->dsc_override_qos_floor) {
			_dispatch_set_basepri_override_qos(dsc->dsc_override_qos);
		}
	}
}

#pragma mark -
#pragma mark _dispatch_barrier_trysync_or_async_f

DISPATCH_NOINLINE
static void
_dispatch_barrier_trysync_or_async_f_complete(dispatch_lane_t dq,
		void *ctxt, dispatch_function_t func, uint32_t flags)
{
	dispatch_wakeup_flags_t wflags = DISPATCH_WAKEUP_BARRIER_COMPLETE;

	_dispatch_sync_function_invoke_inline(dq, ctxt, func);
	if (flags & DISPATCH_BARRIER_TRYSYNC_SUSPEND) {
		uint64_t dq_state = os_atomic_sub2o(dq, dq_state,
				DISPATCH_QUEUE_SUSPEND_INTERVAL, relaxed);
		if (!_dq_state_is_suspended(dq_state)) {
			wflags |= DISPATCH_WAKEUP_CONSUME_2;
		}
	}
	dx_wakeup(dq, 0, wflags);
}

// Use for mutation of queue-/source-internal state only
// ignores target queue hierarchy!
DISPATCH_NOINLINE
void
_dispatch_barrier_trysync_or_async_f(dispatch_lane_t dq, void *ctxt,
		dispatch_function_t func, uint32_t flags)
{
	dispatch_tid tid = _dispatch_tid_self();
	uint64_t suspend_count = (flags & DISPATCH_BARRIER_TRYSYNC_SUSPEND) ? 1 : 0;
	if (unlikely(!_dispatch_queue_try_acquire_barrier_sync_and_suspend(dq, tid,
			suspend_count))) {
		return _dispatch_barrier_async_detached_f(dq, ctxt, func);
	}
	if (flags & DISPATCH_BARRIER_TRYSYNC_SUSPEND) {
		_dispatch_retain_2(dq); // see _dispatch_lane_suspend
	}
	_dispatch_barrier_trysync_or_async_f_complete(dq, ctxt, func, flags);
}

#pragma mark -
#pragma mark dispatch_sync / dispatch_barrier_sync

DISPATCH_NOINLINE
static void
_dispatch_sync_f_slow(dispatch_queue_class_t top_dqu, void *ctxt,
		dispatch_function_t func, uintptr_t top_dc_flags,
		dispatch_queue_class_t dqu, uintptr_t dc_flags)
{
	dispatch_queue_t top_dq = top_dqu._dq;
	dispatch_queue_t dq = dqu._dq;
	if (unlikely(!dq->do_targetq)) {
		return _dispatch_sync_function_invoke(dq, ctxt, func);
	}

	pthread_priority_t pp = _dispatch_get_priority();
	struct dispatch_sync_context_s dsc = {
		.dc_flags    = DC_FLAG_SYNC_WAITER | dc_flags,
		.dc_func     = _dispatch_async_and_wait_invoke,
		.dc_ctxt     = &dsc,
		.dc_other    = top_dq,
		.dc_priority = pp | _PTHREAD_PRIORITY_ENFORCE_FLAG,
		.dc_voucher  = _voucher_get(),
		.dsc_func    = func,
		.dsc_ctxt    = ctxt,
		.dsc_waiter  = _dispatch_tid_self(),
	};

	_dispatch_trace_item_push(top_dq, &dsc);
	__DISPATCH_WAIT_FOR_QUEUE__(&dsc, dq);

	if (dsc.dsc_func == NULL) {
		// dsc_func being cleared means that the block ran on another thread ie.
		// case (2) as listed in _dispatch_async_and_wait_f_slow.
		dispatch_queue_t stop_dq = dsc.dc_other;
		return _dispatch_sync_complete_recurse(top_dq, stop_dq, top_dc_flags);
	}

	_dispatch_introspection_sync_begin(top_dq);
	_dispatch_trace_item_pop(top_dq, &dsc);
	_dispatch_sync_invoke_and_complete_recurse(top_dq, ctxt, func,top_dc_flags
			DISPATCH_TRACE_ARG(&dsc));
}

DISPATCH_NOINLINE
static void
_dispatch_sync_recurse(dispatch_lane_t dq, void *ctxt,
		dispatch_function_t func, uintptr_t dc_flags)
{
	dispatch_tid tid = _dispatch_tid_self();
	dispatch_queue_t tq = dq->do_targetq;

	do {
		if (likely(tq->dq_width == 1)) {
			if (unlikely(!_dispatch_queue_try_acquire_barrier_sync(tq, tid))) {
				return _dispatch_sync_f_slow(dq, ctxt, func, dc_flags, tq,
						DC_FLAG_BARRIER);
			}
		} else {
			dispatch_queue_concurrent_t dl = upcast(tq)._dl;
			if (unlikely(!_dispatch_queue_try_reserve_sync_width(dl))) {
				return _dispatch_sync_f_slow(dq, ctxt, func, dc_flags, tq, 0);
			}
		}
		tq = tq->do_targetq;
	} while (unlikely(tq->do_targetq));

	_dispatch_introspection_sync_begin(dq);
	_dispatch_sync_invoke_and_complete_recurse(dq, ctxt, func, dc_flags
			DISPATCH_TRACE_ARG(_dispatch_trace_item_sync_push_pop(
					dq, ctxt, func, dc_flags)));
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_barrier_sync_f_inline(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, uintptr_t dc_flags)
{
	dispatch_tid tid = _dispatch_tid_self();

	if (unlikely(dx_metatype(dq) != _DISPATCH_LANE_TYPE)) {
		DISPATCH_CLIENT_CRASH(0, "Queue type doesn't support dispatch_sync");
	}

	dispatch_lane_t dl = upcast(dq)._dl;
	// The more correct thing to do would be to merge the qos of the thread
	// that just acquired the barrier lock into the queue state.
	//
	// However this is too expensive for the fast path, so skip doing it.
	// The chosen tradeoff is that if an enqueue on a lower priority thread
	// contends with this fast path, this thread may receive a useless override.
	//
	// Global concurrent queues and queues bound to non-dispatch threads
	// always fall into the slow case, see DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE
	if (unlikely(!_dispatch_queue_try_acquire_barrier_sync(dl, tid))) {
		return _dispatch_sync_f_slow(dl, ctxt, func, DC_FLAG_BARRIER, dl,
				DC_FLAG_BARRIER | dc_flags);
	}

	if (unlikely(dl->do_targetq->do_targetq)) {
		return _dispatch_sync_recurse(dl, ctxt, func,
				DC_FLAG_BARRIER | dc_flags);
	}
	_dispatch_introspection_sync_begin(dl);
	_dispatch_lane_barrier_sync_invoke_and_complete(dl, ctxt, func
			DISPATCH_TRACE_ARG(_dispatch_trace_item_sync_push_pop(
					dq, ctxt, func, dc_flags | DC_FLAG_BARRIER)));
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, uintptr_t dc_flags)
{
	_dispatch_barrier_sync_f_inline(dq, ctxt, func, dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_barrier_sync_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	_dispatch_barrier_sync_f_inline(dq, ctxt, func, 0);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sync_f_inline(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, uintptr_t dc_flags)
{
	if (likely(dq->dq_width == 1)) {
		return _dispatch_barrier_sync_f(dq, ctxt, func, dc_flags);
	}

	if (unlikely(dx_metatype(dq) != _DISPATCH_LANE_TYPE)) {
		DISPATCH_CLIENT_CRASH(0, "Queue type doesn't support dispatch_sync");
	}

	dispatch_lane_t dl = upcast(dq)._dl;
	// Global concurrent queues and queues bound to non-dispatch threads
	// always fall into the slow case, see DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE
	if (unlikely(!_dispatch_queue_try_reserve_sync_width(dl))) {
		return _dispatch_sync_f_slow(dl, ctxt, func, 0, dl, dc_flags);
	}

	if (unlikely(dq->do_targetq->do_targetq)) {
		return _dispatch_sync_recurse(dl, ctxt, func, dc_flags);
	}
	_dispatch_introspection_sync_begin(dl);
	_dispatch_sync_invoke_and_complete(dl, ctxt, func DISPATCH_TRACE_ARG(
			_dispatch_trace_item_sync_push_pop(dq, ctxt, func, dc_flags)));
}

DISPATCH_NOINLINE
static void
_dispatch_sync_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func,
		uintptr_t dc_flags)
{
	_dispatch_sync_f_inline(dq, ctxt, func, dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_sync_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func)
{
	_dispatch_sync_f_inline(dq, ctxt, func, 0);
}

#ifdef __BLOCKS__
DISPATCH_NOINLINE
static void
_dispatch_sync_block_with_privdata(dispatch_queue_t dq, dispatch_block_t work,
		uintptr_t dc_flags)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(work);
	pthread_priority_t op = 0, p = 0;
	dispatch_block_flags_t flags = dbpd->dbpd_flags;

	if (flags & DISPATCH_BLOCK_BARRIER) {
		dc_flags |= DC_FLAG_BLOCK_WITH_PRIVATE_DATA | DC_FLAG_BARRIER;
	} else {
		dc_flags |= DC_FLAG_BLOCK_WITH_PRIVATE_DATA;
	}

	op = _dispatch_block_invoke_should_set_priority(flags, dbpd->dbpd_priority);
	if (op) {
		p = dbpd->dbpd_priority;
	}
	voucher_t ov, v = DISPATCH_NO_VOUCHER;
	if (flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		v = dbpd->dbpd_voucher;
	}
	ov = _dispatch_set_priority_and_voucher(p, v, 0);

	// balanced in d_block_sync_invoke or d_block_wait
	if (os_atomic_cmpxchg2o(dbpd, dbpd_queue, NULL, dq, relaxed)) {
		_dispatch_retain_2(dq);
	}
	if (dc_flags & DC_FLAG_BARRIER) {
		_dispatch_barrier_sync_f(dq, work, _dispatch_block_sync_invoke,
				dc_flags);
	} else {
		_dispatch_sync_f(dq, work, _dispatch_block_sync_invoke, dc_flags);
	}
	_dispatch_reset_priority_and_voucher(op, ov);
}

void
dispatch_barrier_sync(dispatch_queue_t dq, dispatch_block_t work)
{
	uintptr_t dc_flags = DC_FLAG_BARRIER | DC_FLAG_BLOCK;
	if (unlikely(_dispatch_block_has_private_data(work))) {
		return _dispatch_sync_block_with_privdata(dq, work, dc_flags);
	}
	_dispatch_barrier_sync_f(dq, work, _dispatch_Block_invoke(work), dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_sync(dispatch_queue_t dq, dispatch_block_t work)
{
	uintptr_t dc_flags = DC_FLAG_BLOCK;
	if (unlikely(_dispatch_block_has_private_data(work))) {
		return _dispatch_sync_block_with_privdata(dq, work, dc_flags);
	}
	_dispatch_sync_f(dq, work, _dispatch_Block_invoke(work), dc_flags);
}
#endif // __BLOCKS__

#pragma mark -
#pragma mark dispatch_async_and_wait

DISPATCH_ALWAYS_INLINE
static inline dispatch_wlh_t
_dispatch_fake_wlh(dispatch_queue_t dq)
{
	dispatch_wlh_t new_wlh = DISPATCH_WLH_ANON;
	if (likely(dx_metatype(dq) == _DISPATCH_WORKLOOP_TYPE) ||
			_dq_state_is_base_wlh(os_atomic_load2o(dq, dq_state, relaxed))) {
		new_wlh = (dispatch_wlh_t)dq;
	}
	dispatch_wlh_t old_wlh = _dispatch_get_wlh();
	_dispatch_thread_setspecific(dispatch_wlh_key, new_wlh);
	return old_wlh;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_restore_wlh(dispatch_wlh_t wlh)
{
	_dispatch_thread_setspecific(dispatch_wlh_key, wlh);
}

DISPATCH_NOINLINE
static void
_dispatch_async_and_wait_invoke_and_complete_recurse(dispatch_queue_t dq,
		dispatch_sync_context_t dsc, dispatch_queue_t bottom_q,
		uintptr_t top_dc_flags)
{
	dispatch_invoke_flags_t iflags;
	dispatch_wlh_t old_wlh = _dispatch_fake_wlh(bottom_q);

	iflags = dsc->dsc_autorelease * DISPATCH_INVOKE_AUTORELEASE_ALWAYS;
	dispatch_invoke_with_autoreleasepool(iflags, {
		dispatch_block_flags_t bflags = DISPATCH_BLOCK_HAS_PRIORITY;
		dispatch_thread_frame_s dtf;
		pthread_priority_t op = 0, p = dsc->dc_priority;
		voucher_t ov, v = dsc->dc_voucher;

		_dispatch_introspection_sync_begin(dq);
		_dispatch_thread_frame_push(&dtf, dq);
		op = _dispatch_block_invoke_should_set_priority(bflags, p);
		ov = _dispatch_set_priority_and_voucher(op ? p : 0, v, 0);
		_dispatch_trace_item_pop(dq, dsc);
		_dispatch_client_callout(dsc->dsc_ctxt, dsc->dsc_func);
		_dispatch_perfmon_workitem_inc();
		_dispatch_reset_priority_and_voucher(op, ov);
		_dispatch_thread_frame_pop(&dtf);
	});

	_dispatch_trace_item_complete(dsc);

	_dispatch_restore_wlh(old_wlh);
	_dispatch_sync_complete_recurse(dq, NULL, top_dc_flags);
}

DISPATCH_NOINLINE
static void
_dispatch_async_and_wait_f_slow(dispatch_queue_t dq, uintptr_t top_dc_flags,
		dispatch_sync_context_t dsc, dispatch_queue_t tq)
{
	/* dc_other is an in-out parameter.
	 *
	 * As an in-param, it specifies the top queue on which the blocking
	 * primitive is called.
	 *
	 * As an out-param, it refers to the queue up till which we have the drain
	 * lock. This is slightly different depending on how we come out of
	 * _WAIT_FOR_QUEUE.
	 *
	 * Case 1:
	 * If the continuation is to be invoked on another thread - for
	 * async_and_wait, or we ran on a thread bound main queue - then someone
	 * already called _dispatch_async_and_wait_invoke which invoked the block
	 * already. dc_other as an outparam here tells the enqueuer the queue up
	 * till which the enqueuer got the drain lock so that we know what to unlock
	 * on the way out. This is the case whereby the enqueuer owns part of the
	 * locks in the queue hierachy (but not all).
	 *
	 * Case 2:
	 * If the continuation is to be invoked on the enqueuing thread -  because
	 * we were contending with another sync or async_and_wait - then enqueuer
	 * return from _WAIT_FOR_QUEUE without having invoked the block. The
	 * enqueuer has had the locks for the rest of the queue hierachy handed off
	 * to it so dc_other specifies the queue up till which it has the locks
	 * which in this case, is up till the bottom queue in the hierachy. So it
	 * needs to unlock everything up till the bottom queue, on the way out.
	 */

	__DISPATCH_WAIT_FOR_QUEUE__(dsc, tq);

	if (unlikely(dsc->dsc_func == NULL)) {
		// see _dispatch_async_and_wait_invoke
		dispatch_queue_t stop_dq = dsc->dc_other;
		return _dispatch_sync_complete_recurse(dq, stop_dq, top_dc_flags);
	}

	// see _dispatch_*_redirect_or_wake
	dispatch_queue_t bottom_q = dsc->dc_other;
	return _dispatch_async_and_wait_invoke_and_complete_recurse(dq, dsc,
			bottom_q, top_dc_flags);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_async_and_wait_should_always_async(dispatch_queue_class_t dqu,
		uint64_t dq_state)
{
	// If the queue is anchored at a pthread root queue for which we can't
	// mirror attributes, then we need to take the async path.
	return !_dq_state_is_inner_queue(dq_state) &&
			!_dispatch_is_in_root_queues_array(dqu._dq->do_targetq);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_async_and_wait_recurse_one(dispatch_queue_t dq,
		dispatch_sync_context_t dsc, dispatch_tid tid, uintptr_t dc_flags)
{
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
	if (unlikely(_dispatch_async_and_wait_should_always_async(dq, dq_state))) {
		// Remove the async_and_wait flag but drive down the slow path so that
		// we do the synchronous wait. We are guaranteed that dq is the base
		// queue.
		//
		// We're falling down to case (1) of _dispatch_async_and_wait_f_slow so
		// set dc_other to dq
		dsc->dc_flags &= ~DC_FLAG_ASYNC_AND_WAIT;
		dsc->dc_other = dq;
		return false;
	}
	if (likely(dc_flags & DC_FLAG_BARRIER)) {
		return _dispatch_queue_try_acquire_barrier_sync(dq, tid);
	}
	return _dispatch_queue_try_reserve_sync_width(upcast(dq)._dl);
}

DISPATCH_NOINLINE
static void
_dispatch_async_and_wait_recurse(dispatch_queue_t top_dq,
		dispatch_sync_context_t dsc, dispatch_tid tid, uintptr_t top_flags)
{
	dispatch_queue_t dq = top_dq;
	uintptr_t dc_flags = top_flags;

	_dispatch_trace_item_push(top_dq, dsc);

	for (;;) {
		if (unlikely(!_dispatch_async_and_wait_recurse_one(dq, dsc, tid,
				dc_flags))) {
			return _dispatch_async_and_wait_f_slow(top_dq, top_flags, dsc, dq);
		}

		_dispatch_async_waiter_update(dsc, dq);
		if (likely(!dq->do_targetq->do_targetq)) break;
		dq = dq->do_targetq;
		if (likely(dq->dq_width == 1)) {
			dc_flags |= DC_FLAG_BARRIER;
		} else {
			dc_flags &= ~DC_FLAG_BARRIER;
		}
		dsc->dc_flags = dc_flags;
	}

	_dispatch_async_and_wait_invoke_and_complete_recurse(top_dq, dsc, dq,
			top_flags);
}

DISPATCH_NOINLINE
static void
_dispatch_async_and_wait_f(dispatch_queue_t dq,
		void *ctxt, dispatch_function_t func, uintptr_t dc_flags)
{
	pthread_priority_t pp = _dispatch_get_priority();
	dispatch_tid tid = _dispatch_tid_self();
	struct dispatch_sync_context_s dsc = {
		.dc_flags    = dc_flags,
		.dc_func     = _dispatch_async_and_wait_invoke,
		.dc_ctxt     = &dsc,
		.dc_other    = dq,
		.dc_priority = pp | _PTHREAD_PRIORITY_ENFORCE_FLAG,
		.dc_voucher  = _voucher_get(),
		.dsc_func    = func,
		.dsc_ctxt    = ctxt,
		.dsc_waiter  = tid,
	};

	return _dispatch_async_and_wait_recurse(dq, &dsc, tid, dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_async_and_wait_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	if (unlikely(!dq->do_targetq)) {
		return _dispatch_sync_function_invoke(dq, ctxt, func);
	}

	uintptr_t dc_flags = DC_FLAG_ASYNC_AND_WAIT;
	if (likely(dq->dq_width == 1)) dc_flags |= DC_FLAG_BARRIER;
	return _dispatch_async_and_wait_f(dq, ctxt, func, dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_barrier_async_and_wait_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	if (unlikely(!dq->do_targetq)) {
		return _dispatch_sync_function_invoke(dq, ctxt, func);
	}

	uintptr_t dc_flags = DC_FLAG_ASYNC_AND_WAIT | DC_FLAG_BARRIER;
	return _dispatch_async_and_wait_f(dq, ctxt, func, dc_flags);
}

#ifdef __BLOCKS__
DISPATCH_NOINLINE
static void
_dispatch_async_and_wait_block_with_privdata(dispatch_queue_t dq,
		dispatch_block_t work, uintptr_t dc_flags)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(work);
	dispatch_block_flags_t flags = dbpd->dbpd_flags;
	pthread_priority_t pp;
	voucher_t v;

	if (dbpd->dbpd_flags & DISPATCH_BLOCK_BARRIER) {
		dc_flags |= DC_FLAG_BLOCK_WITH_PRIVATE_DATA | DC_FLAG_BARRIER;
	} else {
		dc_flags |= DC_FLAG_BLOCK_WITH_PRIVATE_DATA;
	}

	if (_dispatch_block_invoke_should_set_priority(flags, dbpd->dbpd_priority)){
		pp = dbpd->dbpd_priority;
	} else {
		pp = _dispatch_get_priority();
	}
	if (dbpd->dbpd_flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		v = dbpd->dbpd_voucher;
	} else {
		v = _voucher_get();
	}

	// balanced in d_block_sync_invoke or d_block_wait
	if (os_atomic_cmpxchg2o(dbpd, dbpd_queue, NULL, dq, relaxed)) {
		_dispatch_retain_2(dq);
	}

	dispatch_tid tid = _dispatch_tid_self();
	struct dispatch_sync_context_s dsc = {
		.dc_flags    = dc_flags,
		.dc_func     = _dispatch_async_and_wait_invoke,
		.dc_ctxt     = &dsc,
		.dc_other    = dq,
		.dc_priority = pp | _PTHREAD_PRIORITY_ENFORCE_FLAG,
		.dc_voucher  = v,
		.dsc_func    = _dispatch_block_sync_invoke,
		.dsc_ctxt    = work,
		.dsc_waiter  = tid,
	};

	return _dispatch_async_and_wait_recurse(dq, &dsc, tid, dc_flags);
}

void
dispatch_barrier_async_and_wait(dispatch_queue_t dq, dispatch_block_t work)
{
	if (unlikely(!dq->do_targetq)) {
		return dispatch_barrier_sync(dq, work);
	}

	uintptr_t dc_flags = DC_FLAG_ASYNC_AND_WAIT | DC_FLAG_BLOCK|DC_FLAG_BARRIER;
	if (unlikely(_dispatch_block_has_private_data(work))) {
		return _dispatch_async_and_wait_block_with_privdata(dq, work, dc_flags);
	}

	dispatch_function_t func = _dispatch_Block_invoke(work);
	return _dispatch_async_and_wait_f(dq, work, func, dc_flags);
}

void
dispatch_async_and_wait(dispatch_queue_t dq, dispatch_block_t work)
{
	if (unlikely(!dq->do_targetq)) {
		return dispatch_sync(dq, work);
	}

	uintptr_t dc_flags = DC_FLAG_ASYNC_AND_WAIT | DC_FLAG_BLOCK;
	if (likely(dq->dq_width == 1)) dc_flags |= DC_FLAG_BARRIER;
	if (unlikely(_dispatch_block_has_private_data(work))) {
		return _dispatch_async_and_wait_block_with_privdata(dq, work, dc_flags);
	}

	dispatch_function_t func = _dispatch_Block_invoke(work);
	return _dispatch_async_and_wait_f(dq, work, func, dc_flags);
}
#endif // __BLOCKS__

#pragma mark -
#pragma mark dispatch_queue_specific

static void
_dispatch_queue_specific_head_dispose_slow(void *ctxt)
{
	dispatch_queue_specific_head_t dqsh = ctxt;
	dispatch_queue_specific_t dqs, tmp;

	TAILQ_FOREACH_SAFE(dqs, &dqsh->dqsh_entries, dqs_entry, tmp) {
		dispatch_assert(dqs->dqs_destructor);
		_dispatch_client_callout(dqs->dqs_ctxt, dqs->dqs_destructor);
		free(dqs);
	}
	free(dqsh);
}

static void
_dispatch_queue_specific_head_dispose(dispatch_queue_specific_head_t dqsh)
{
	dispatch_queue_t rq = _dispatch_get_default_queue(false);
	dispatch_queue_specific_t dqs, tmp;
	TAILQ_HEAD(, dispatch_queue_specific_s) entries =
			TAILQ_HEAD_INITIALIZER(entries);

	TAILQ_CONCAT(&entries, &dqsh->dqsh_entries, dqs_entry);
	TAILQ_FOREACH_SAFE(dqs, &entries, dqs_entry, tmp) {
		if (dqs->dqs_destructor) {
			TAILQ_INSERT_TAIL(&dqsh->dqsh_entries, dqs, dqs_entry);
		} else {
			free(dqs);
		}
	}

	if (TAILQ_EMPTY(&dqsh->dqsh_entries)) {
		free(dqsh);
	} else {
		_dispatch_barrier_async_detached_f(rq, dqsh,
				_dispatch_queue_specific_head_dispose_slow);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_queue_init_specific(dispatch_queue_t dq)
{
	dispatch_queue_specific_head_t dqsh;

	dqsh = _dispatch_calloc(1, sizeof(struct dispatch_queue_specific_head_s));
	TAILQ_INIT(&dqsh->dqsh_entries);
	if (unlikely(!os_atomic_cmpxchg2o(dq, dq_specific_head,
			NULL, dqsh, release))) {
		_dispatch_queue_specific_head_dispose(dqsh);
	}
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_specific_t
_dispatch_queue_specific_find(dispatch_queue_specific_head_t dqsh,
		const void *key)
{
	dispatch_queue_specific_t dqs;

	TAILQ_FOREACH(dqs, &dqsh->dqsh_entries, dqs_entry) {
		if (dqs->dqs_key == key) {
			return dqs;
		}
	}
	return NULL;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_admits_specific(dispatch_queue_t dq)
{
	if (dx_metatype(dq) == _DISPATCH_LANE_TYPE) {
		return (dx_type(dq) == DISPATCH_QUEUE_MAIN_TYPE ||
			!dx_hastypeflag(dq, QUEUE_BASE));
	}
	return dx_metatype(dq) == _DISPATCH_WORKLOOP_TYPE;
}

DISPATCH_NOINLINE
void
dispatch_queue_set_specific(dispatch_queue_t dq, const void *key,
	void *ctxt, dispatch_function_t destructor)
{
	if (unlikely(!key)) {
		return;
	}
	dispatch_queue_t rq = _dispatch_get_default_queue(false);
	dispatch_queue_specific_head_t dqsh = dq->dq_specific_head;
	dispatch_queue_specific_t dqs;

	if (unlikely(!_dispatch_queue_admits_specific(dq))) {
		DISPATCH_CLIENT_CRASH(0,
				"Queue doesn't support dispatch_queue_set_specific");
	}

	if (ctxt && !dqsh) {
		_dispatch_queue_init_specific(dq);
		dqsh = dq->dq_specific_head;
	} else if (!dqsh) {
		return;
	}

	_dispatch_unfair_lock_lock(&dqsh->dqsh_lock);
	dqs = _dispatch_queue_specific_find(dqsh, key);
	if (dqs) {
		if (dqs->dqs_destructor) {
			_dispatch_barrier_async_detached_f(rq, dqs->dqs_ctxt,
					dqs->dqs_destructor);
		}
		if (ctxt) {
			dqs->dqs_ctxt = ctxt;
			dqs->dqs_destructor = destructor;
		} else {
			TAILQ_REMOVE(&dqsh->dqsh_entries, dqs, dqs_entry);
			free(dqs);
		}
	} else if (ctxt) {
		dqs = _dispatch_calloc(1, sizeof(struct dispatch_queue_specific_s));
		dqs->dqs_key = key;
		dqs->dqs_ctxt = ctxt;
		dqs->dqs_destructor = destructor;
		TAILQ_INSERT_TAIL(&dqsh->dqsh_entries, dqs, dqs_entry);
	}

	_dispatch_unfair_lock_unlock(&dqsh->dqsh_lock);
}

DISPATCH_ALWAYS_INLINE
static inline void *
_dispatch_queue_get_specific_inline(dispatch_queue_t dq, const void *key)
{
	dispatch_queue_specific_head_t dqsh = dq->dq_specific_head;
	dispatch_queue_specific_t dqs;
	void *ctxt = NULL;

	if (likely(_dispatch_queue_admits_specific(dq) && dqsh)) {
		_dispatch_unfair_lock_lock(&dqsh->dqsh_lock);
		dqs = _dispatch_queue_specific_find(dqsh, key);
		if (dqs) ctxt = dqs->dqs_ctxt;
		_dispatch_unfair_lock_unlock(&dqsh->dqsh_lock);
	}
	return ctxt;
}

DISPATCH_NOINLINE
void *
dispatch_queue_get_specific(dispatch_queue_t dq, const void *key)
{
	if (unlikely(!key)) {
		return NULL;
	}
	return _dispatch_queue_get_specific_inline(dq, key);
}

DISPATCH_NOINLINE
void *
dispatch_get_specific(const void *key)
{
	dispatch_queue_t dq = _dispatch_queue_get_current();
	void *ctxt = NULL;

	if (likely(key && dq)) {
		do {
			ctxt = _dispatch_queue_get_specific_inline(dq, key);
			dq = dq->do_targetq;
		} while (unlikely(ctxt == NULL && dq));
	}
	return ctxt;
}

#pragma mark -
#pragma mark dispatch_queue_t / dispatch_lane_t

void
dispatch_queue_set_label_nocopy(dispatch_queue_t dq, const char *label)
{
	if (unlikely(_dispatch_object_is_global(dq))) {
		return;
	}
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(dq);
	if (unlikely(dqf & DQF_LABEL_NEEDS_FREE)) {
		DISPATCH_CLIENT_CRASH(dq, "Cannot change label for this queue");
	}
	dq->dq_label = label;
}

static inline bool
_dispatch_base_lane_is_wlh(dispatch_lane_t dq, dispatch_queue_t tq)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	if (unlikely(!_dispatch_kevent_workqueue_enabled)) {
		return false;
	}
	if (dx_type(dq) == DISPATCH_QUEUE_NETWORK_EVENT_TYPE) {
		return true;
	}
	if (dx_metatype(dq) == _DISPATCH_SOURCE_TYPE) {
		// Sources don't support sync waiters, so the ones that never change QoS
		// don't benefit from any of the workloop features which have overhead,
		// so just use the workqueue kqueue for these.
		if (likely(!upcast(dq)._ds->ds_refs->du_can_be_wlh)) {
			return false;
		}
		dispatch_assert(upcast(dq)._ds->ds_refs->du_is_direct);
	}
	return dq->dq_width == 1 && _dispatch_is_in_root_queues_array(tq);
#else
	(void)dq; (void)tq;
	return false;
#endif // DISPATCH_USE_KEVENT_WORKLOOP
}

static void
_dispatch_lane_inherit_wlh_from_target(dispatch_lane_t dq, dispatch_queue_t tq)
{
	uint64_t old_state, new_state, role;

	if (!dx_hastypeflag(tq, QUEUE_ROOT)) {
		role = DISPATCH_QUEUE_ROLE_INNER;
	} else if (_dispatch_base_lane_is_wlh(dq, tq)) {
		role = DISPATCH_QUEUE_ROLE_BASE_WLH;
	} else {
		role = DISPATCH_QUEUE_ROLE_BASE_ANON;
	}

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		new_state = old_state & ~DISPATCH_QUEUE_ROLE_MASK;
		new_state |= role;
		if (old_state == new_state) {
			os_atomic_rmw_loop_give_up(break);
		}
	});

	if (_dq_state_is_base_wlh(old_state) && !_dq_state_is_base_wlh(new_state)) {
		dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
		if (ddi && ddi->ddi_wlh == (dispatch_wlh_t)dq) {
			_dispatch_event_loop_leave_immediate(new_state);
		}
	}
	if (!dx_hastypeflag(tq, QUEUE_ROOT)) {
		dispatch_queue_flags_t clear = 0, set = DQF_TARGETED;
		if (dx_metatype(tq) == _DISPATCH_WORKLOOP_TYPE) {
			clear |= DQF_MUTABLE;
#if !DISPATCH_ALLOW_NON_LEAF_RETARGET
		} else {
			clear |= DQF_MUTABLE;
#endif
		}
		if (clear) {
			_dispatch_queue_atomic_flags_set_and_clear(tq, set, clear);
		} else {
			_dispatch_queue_atomic_flags_set(tq, set);
		}
	}
}

dispatch_priority_t
_dispatch_queue_compute_priority_and_wlh(dispatch_queue_t dq,
		dispatch_wlh_t *wlh_out)
{
	dispatch_priority_t dpri = dq->dq_priority;
	dispatch_priority_t p = dpri & DISPATCH_PRIORITY_REQUESTED_MASK;
	dispatch_qos_t fallback = _dispatch_priority_fallback_qos(dpri);
	dispatch_queue_t tq = dq->do_targetq;
	dispatch_wlh_t wlh = DISPATCH_WLH_ANON;

	if (_dq_state_is_base_wlh(dq->dq_state)) {
		wlh = (dispatch_wlh_t)dq;
	}

	while (unlikely(!dx_hastypeflag(tq, QUEUE_ROOT))) {
		if (unlikely(tq == _dispatch_mgr_q._as_dq)) {
			if (wlh_out) *wlh_out = DISPATCH_WLH_ANON;
			return DISPATCH_PRIORITY_FLAG_MANAGER;
		}
		if (unlikely(_dispatch_queue_is_thread_bound(tq))) {
			if (wlh_out) *wlh_out = DISPATCH_WLH_ANON;
			return tq->dq_priority;
		}
		if (unlikely(DISPATCH_QUEUE_IS_SUSPENDED(tq))) {
			// this queue may not be activated yet, so the queue graph may not
			// have stabilized yet
			_dispatch_ktrace2(DISPATCH_PERF_delayed_registration, dq,
					  dx_metatype(dq) == _DISPATCH_SOURCE_TYPE ? dq : NULL);
			if (wlh_out) *wlh_out = NULL;
			return 0;
		}

		if (_dq_state_is_base_wlh(tq->dq_state)) {
			wlh = (dispatch_wlh_t)tq;
			if (dx_metatype(tq) == _DISPATCH_WORKLOOP_TYPE) {
				_dispatch_queue_atomic_flags_clear(dq, DQF_MUTABLE);
			}
		} else if (unlikely(_dispatch_queue_is_mutable(tq))) {
			// we're not allowed to dereference tq->do_targetq
			_dispatch_ktrace2(DISPATCH_PERF_delayed_registration, dq,
					  dx_metatype(dq) == _DISPATCH_SOURCE_TYPE ? dq : NULL);
			if (wlh_out) *wlh_out = NULL;
			return 0;
		}

		dispatch_priority_t tqp = tq->dq_priority;

		tq = tq->do_targetq;
		if (tqp & DISPATCH_PRIORITY_FLAG_INHERITED) {
			// if the priority is inherited, it means we got it from our target
			// which has fallback and various magical flags that the code below
			// will handle, so do not bother here.
			break;
		}

		if (!fallback) fallback = _dispatch_priority_fallback_qos(tqp);
		tqp &= DISPATCH_PRIORITY_REQUESTED_MASK;
		if (p < tqp) p = tqp;
	}

	if (likely(_dispatch_is_in_root_queues_array(tq) ||
			tq->dq_serialnum == DISPATCH_QUEUE_SERIAL_NUMBER_WLF)) {
		dispatch_priority_t rqp = tq->dq_priority;

		if (!fallback) fallback = _dispatch_priority_fallback_qos(rqp);
		rqp &= DISPATCH_PRIORITY_REQUESTED_MASK;
		if (p < rqp) p = rqp;

		p |= (tq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT);
		if ((dpri & DISPATCH_PRIORITY_FLAG_FLOOR) ||
				!(dpri & DISPATCH_PRIORITY_REQUESTED_MASK)) {
			p |= (dpri & DISPATCH_PRIORITY_FLAG_FLOOR);
			if (fallback > _dispatch_priority_qos(p)) {
				p |= _dispatch_priority_make_fallback(fallback);
			}
		}
		if (wlh_out) *wlh_out = wlh;
		return p;
	}

	// pthread root queues opt out of QoS
	if (wlh_out) *wlh_out = DISPATCH_WLH_ANON;
	return DISPATCH_PRIORITY_FLAG_MANAGER;
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_workloop_attributes_alloc_if_needed(dispatch_workloop_t dwl)
{
	if (unlikely(!dwl->dwl_attr)) {
		dwl->dwl_attr = _dispatch_calloc(1, sizeof(dispatch_workloop_attr_s));
	}
}

void
dispatch_set_qos_class_floor(dispatch_object_t dou,
		dispatch_qos_class_t cls, int relpri)
{
	if (dx_cluster(dou._do) != _DISPATCH_QUEUE_CLUSTER) {
		DISPATCH_CLIENT_CRASH(0,
				"dispatch_set_qos_class_floor called on invalid object type");
	}
	if (dx_metatype(dou._do) == _DISPATCH_WORKLOOP_TYPE) {
		return dispatch_workloop_set_qos_class_floor(dou._dwl, cls, relpri, 0);
	}

	dispatch_qos_t qos = _dispatch_qos_from_qos_class(cls);
	dispatch_priority_t pri = _dispatch_priority_make(qos, relpri);
	dispatch_priority_t old_pri = dou._dq->dq_priority;

	if (pri) pri |= DISPATCH_PRIORITY_FLAG_FLOOR;
	old_pri &= ~DISPATCH_PRIORITY_REQUESTED_MASK;
	old_pri &= ~DISPATCH_PRIORITY_FLAG_FLOOR;
	dou._dq->dq_priority = pri | old_pri;

	_dispatch_queue_setter_assert_inactive(dou._dq);
}

void
dispatch_set_qos_class(dispatch_object_t dou, dispatch_qos_class_t cls,
		int relpri)
{
	if (dx_cluster(dou._do) != _DISPATCH_QUEUE_CLUSTER ||
			dx_metatype(dou._do) == _DISPATCH_WORKLOOP_TYPE) {
		DISPATCH_CLIENT_CRASH(0,
				"dispatch_set_qos_class called on invalid object type");
	}

	dispatch_qos_t qos = _dispatch_qos_from_qos_class(cls);
	dispatch_priority_t pri = _dispatch_priority_make(qos, relpri);
	dispatch_priority_t old_pri = dou._dq->dq_priority;

	old_pri &= ~DISPATCH_PRIORITY_REQUESTED_MASK;
	old_pri &= ~DISPATCH_PRIORITY_FLAG_FLOOR;
	dou._dq->dq_priority = pri | old_pri;

	_dispatch_queue_setter_assert_inactive(dou._dq);
}

void
dispatch_set_qos_class_fallback(dispatch_object_t dou, dispatch_qos_class_t cls)
{
	if (dx_cluster(dou._do) != _DISPATCH_QUEUE_CLUSTER) {
		DISPATCH_CLIENT_CRASH(0,
				"dispatch_set_qos_class_fallback called on invalid object type");
	}

	dispatch_qos_t qos = _dispatch_qos_from_qos_class(cls);
	dispatch_priority_t pri = _dispatch_priority_make_fallback(qos);
	dispatch_priority_t old_pri = dou._dq->dq_priority;

	old_pri &= ~DISPATCH_PRIORITY_FALLBACK_QOS_MASK;
	old_pri &= ~DISPATCH_PRIORITY_FLAG_FALLBACK;
	dou._dq->dq_priority = pri | old_pri;

	_dispatch_queue_setter_assert_inactive(dou._dq);
}

static dispatch_queue_t
_dispatch_queue_priority_inherit_from_target(dispatch_lane_class_t dq,
		dispatch_queue_t tq)
{
	const dispatch_priority_t inherited = DISPATCH_PRIORITY_FLAG_INHERITED;
	dispatch_priority_t pri = dq._dl->dq_priority;

	// This priority has been selected by the client, leave it alone
	// However, when the client picked a QoS, we should adjust the target queue
	// if it is a root queue to best match the ask
	if (_dispatch_queue_priority_manually_selected(pri)) {
		if (_dispatch_is_in_root_queues_array(tq)) {
			dispatch_qos_t qos = _dispatch_priority_qos(pri);
			if (!qos) qos = DISPATCH_QOS_DEFAULT;
			tq = _dispatch_get_root_queue(qos,
					pri & DISPATCH_PRIORITY_FLAG_OVERCOMMIT)->_as_dq;
		}
		return tq;
	}

	if (_dispatch_is_in_root_queues_array(tq)) {
		// <rdar://problem/32921639> base queues need to know they target
		// the default root queue so that _dispatch_queue_wakeup_qos()
		// in _dispatch_queue_wakeup() can fallback to QOS_DEFAULT
		// if no other priority was provided.
		pri = tq->dq_priority | inherited;
	} else if (pri & inherited) {
		// if the FALLBACK flag is set on queues due to the code above
		// we need to clear it if the queue is retargeted within a hierachy
		// and is no longer a base queue.
		pri &= ~DISPATCH_PRIORITY_FALLBACK_QOS_MASK;
		pri &= ~DISPATCH_PRIORITY_FLAG_FALLBACK;
	}

	dq._dl->dq_priority = pri;
	return tq;
}


DISPATCH_NOINLINE
static dispatch_queue_t
_dispatch_lane_create_with_target(const char *label, dispatch_queue_attr_t dqa,
		dispatch_queue_t tq, bool legacy)
{
	dispatch_queue_attr_info_t dqai = _dispatch_queue_attr_to_info(dqa);

	//
	// Step 1: Normalize arguments (qos, overcommit, tq)
	//

	dispatch_qos_t qos = dqai.dqai_qos;
#if !HAVE_PTHREAD_WORKQUEUE_QOS
	if (qos == DISPATCH_QOS_USER_INTERACTIVE) {
		dqai.dqai_qos = qos = DISPATCH_QOS_USER_INITIATED;
	}
	if (qos == DISPATCH_QOS_MAINTENANCE) {
		dqai.dqai_qos = qos = DISPATCH_QOS_BACKGROUND;
	}
#endif // !HAVE_PTHREAD_WORKQUEUE_QOS

	_dispatch_queue_attr_overcommit_t overcommit = dqai.dqai_overcommit;
	if (overcommit != _dispatch_queue_attr_overcommit_unspecified && tq) {
		if (tq->do_targetq) {
			DISPATCH_CLIENT_CRASH(tq, "Cannot specify both overcommit and "
					"a non-global target queue");
		}
	}

	if (tq && dx_type(tq) == DISPATCH_QUEUE_GLOBAL_ROOT_TYPE) {
		// Handle discrepancies between attr and target queue, attributes win
		if (overcommit == _dispatch_queue_attr_overcommit_unspecified) {
			if (tq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT) {
				overcommit = _dispatch_queue_attr_overcommit_enabled;
			} else {
				overcommit = _dispatch_queue_attr_overcommit_disabled;
			}
		}
		if (qos == DISPATCH_QOS_UNSPECIFIED) {
			qos = _dispatch_priority_qos(tq->dq_priority);
		}
		tq = NULL;
	} else if (tq && !tq->do_targetq) {
		// target is a pthread or runloop root queue, setting QoS or overcommit
		// is disallowed
		if (overcommit != _dispatch_queue_attr_overcommit_unspecified) {
			DISPATCH_CLIENT_CRASH(tq, "Cannot specify an overcommit attribute "
					"and use this kind of target queue");
		}
	} else {
		if (overcommit == _dispatch_queue_attr_overcommit_unspecified) {
			// Serial queues default to overcommit!
			overcommit = dqai.dqai_concurrent ?
					_dispatch_queue_attr_overcommit_disabled :
					_dispatch_queue_attr_overcommit_enabled;
		}
	}
	if (!tq) {
		tq = _dispatch_get_root_queue(
				qos == DISPATCH_QOS_UNSPECIFIED ? DISPATCH_QOS_DEFAULT : qos,
				overcommit == _dispatch_queue_attr_overcommit_enabled)->_as_dq;
		if (unlikely(!tq)) {
			DISPATCH_CLIENT_CRASH(qos, "Invalid queue attribute");
		}
	}

	//
	// Step 2: Initialize the queue
	//

	if (legacy) {
		// if any of these attributes is specified, use non legacy classes
		if (dqai.dqai_inactive || dqai.dqai_autorelease_frequency) {
			legacy = false;
		}
	}

	const void *vtable;
	dispatch_queue_flags_t dqf = legacy ? DQF_MUTABLE : 0;
	if (dqai.dqai_concurrent) {
		vtable = DISPATCH_VTABLE(queue_concurrent);
	} else {
		vtable = DISPATCH_VTABLE(queue_serial);
	}
	switch (dqai.dqai_autorelease_frequency) {
	case DISPATCH_AUTORELEASE_FREQUENCY_NEVER:
		dqf |= DQF_AUTORELEASE_NEVER;
		break;
	case DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM:
		dqf |= DQF_AUTORELEASE_ALWAYS;
		break;
	}
	if (label) {
		const char *tmp = _dispatch_strdup_if_mutable(label);
		if (tmp != label) {
			dqf |= DQF_LABEL_NEEDS_FREE;
			label = tmp;
		}
	}

	dispatch_lane_t dq = _dispatch_object_alloc(vtable,
			sizeof(struct dispatch_lane_s));
	_dispatch_queue_init(dq, dqf, dqai.dqai_concurrent ?
			DISPATCH_QUEUE_WIDTH_MAX : 1, DISPATCH_QUEUE_ROLE_INNER |
			(dqai.dqai_inactive ? DISPATCH_QUEUE_INACTIVE : 0));

	dq->dq_label = label;
	dq->dq_priority = _dispatch_priority_make((dispatch_qos_t)dqai.dqai_qos,
			dqai.dqai_relpri);
	if (overcommit == _dispatch_queue_attr_overcommit_enabled) {
		dq->dq_priority |= DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
	}
	if (!dqai.dqai_inactive) {
		_dispatch_queue_priority_inherit_from_target(dq, tq);
		_dispatch_lane_inherit_wlh_from_target(dq, tq);
	}
	_dispatch_retain(tq);
	dq->do_targetq = tq;
	_dispatch_object_debug(dq, "%s", __func__);
	return _dispatch_trace_queue_create(dq)._dq;
}

dispatch_queue_t
dispatch_queue_create_with_target(const char *label, dispatch_queue_attr_t dqa,
		dispatch_queue_t tq)
{
	return _dispatch_lane_create_with_target(label, dqa, tq, false);
}

dispatch_queue_t
dispatch_queue_create(const char *label, dispatch_queue_attr_t attr)
{
	return _dispatch_lane_create_with_target(label, attr,
			DISPATCH_TARGET_QUEUE_DEFAULT, true);
}

dispatch_queue_t
dispatch_queue_create_with_accounting_override_voucher(const char *label,
		dispatch_queue_attr_t attr, voucher_t voucher)
{
	(void)label; (void)attr; (void)voucher;
	DISPATCH_CLIENT_CRASH(0, "Unsupported interface");
}

DISPATCH_NOINLINE
static void
_dispatch_queue_dispose(dispatch_queue_class_t dqu, bool *allow_free)
{
	dispatch_queue_specific_head_t dqsh;
	dispatch_queue_t dq = dqu._dq;

	if (dq->dq_label && _dispatch_queue_label_needs_free(dq)) {
		free((void*)dq->dq_label);
	}
	dqsh = os_atomic_xchg2o(dq, dq_specific_head, (void *)0x200, relaxed);
	if (dqsh) _dispatch_queue_specific_head_dispose(dqsh);

	// fast path for queues that never got their storage retained
	if (likely(os_atomic_load2o(dq, dq_sref_cnt, relaxed) == 0)) {
		// poison the state with something that is suspended and is easy to spot
		dq->dq_state = 0xdead000000000000;
		return;
	}

	// Take over freeing the memory from _dispatch_object_dealloc()
	//
	// As soon as we call _dispatch_queue_release_storage(), we forfeit
	// the possibility for the caller of dx_dispose() to finalize the object
	// so that responsibility is ours.
	_dispatch_object_finalize(dq);
	*allow_free = false;
	dq->dq_label = "<released queue, pending free>";
	dq->do_targetq = NULL;
	dq->do_finalizer = NULL;
	dq->do_ctxt = NULL;
	return _dispatch_queue_release_storage(dq);
}

void
_dispatch_lane_class_dispose(dispatch_lane_class_t dqu, bool *allow_free)
{
	dispatch_lane_t dq = dqu._dl;
	if (unlikely(dq->dq_items_tail)) {
		DISPATCH_CLIENT_CRASH(dq->dq_items_tail,
				"Release of a queue while items are enqueued");
	}
	dq->dq_items_head = (void *)0x200;
	dq->dq_items_tail = (void *)0x200;

	uint64_t orig_dq_state, dq_state;
	dq_state = orig_dq_state = os_atomic_load2o(dq, dq_state, relaxed);

	uint64_t initial_state = DISPATCH_QUEUE_STATE_INIT_VALUE(dq->dq_width);
	if (dx_hastypeflag(dq, QUEUE_ROOT)) {
		initial_state = DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE;
	}
	dq_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
	dq_state &= ~DISPATCH_QUEUE_DIRTY;
	dq_state &= ~DISPATCH_QUEUE_ROLE_MASK;
	if (unlikely(dq_state != initial_state)) {
		if (_dq_state_drain_locked(dq_state)) {
			DISPATCH_CLIENT_CRASH((uintptr_t)orig_dq_state,
					"Release of a locked queue");
		}
#ifndef __LP64__
		orig_dq_state >>= 32;
#endif
		DISPATCH_CLIENT_CRASH((uintptr_t)orig_dq_state,
				"Release of a queue with corrupt state");
	}
	_dispatch_queue_dispose(dqu, allow_free);
}

void
_dispatch_lane_dispose(dispatch_lane_t dq, bool *allow_free)
{
	_dispatch_object_debug(dq, "%s", __func__);
	_dispatch_trace_queue_dispose(dq);
	_dispatch_lane_class_dispose(dq, allow_free);
}

void
_dispatch_queue_xref_dispose(dispatch_queue_t dq)
{
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
	if (unlikely(_dq_state_is_suspended(dq_state))) {
		long state = (long)dq_state;
		if (sizeof(long) < sizeof(uint64_t)) state = (long)(dq_state >> 32);
		if (unlikely(dq_state & DISPATCH_QUEUE_INACTIVE_BITS_MASK)) {
			// Arguments for and against this assert are within 6705399
			DISPATCH_CLIENT_CRASH(state, "Release of an inactive object");
		}
		DISPATCH_CLIENT_CRASH(dq_state, "Release of a suspended object");
	}
	os_atomic_or2o(dq, dq_atomic_flags, DQF_RELEASED, relaxed);
}

DISPATCH_NOINLINE
static void
_dispatch_lane_suspend_slow(dispatch_lane_t dq)
{
	uint64_t old_state, new_state, delta;

	_dispatch_queue_sidelock_lock(dq);

	// what we want to transfer (remove from dq_state)
	delta  = DISPATCH_QUEUE_SUSPEND_HALF * DISPATCH_QUEUE_SUSPEND_INTERVAL;
	// but this is a suspend so add a suspend count at the same time
	delta -= DISPATCH_QUEUE_SUSPEND_INTERVAL;
	if (dq->dq_side_suspend_cnt == 0) {
		// we substract delta from dq_state, and we want to set this bit
		delta -= DISPATCH_QUEUE_HAS_SIDE_SUSPEND_CNT;
	}

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		// unsigned underflow of the substraction can happen because other
		// threads could have touched this value while we were trying to acquire
		// the lock, or because another thread raced us to do the same operation
		// and got to the lock first.
		if (unlikely(os_sub_overflow(old_state, delta, &new_state))) {
			os_atomic_rmw_loop_give_up(goto retry);
		}
	});
	if (unlikely(os_add_overflow(dq->dq_side_suspend_cnt,
			DISPATCH_QUEUE_SUSPEND_HALF, &dq->dq_side_suspend_cnt))) {
		DISPATCH_CLIENT_CRASH(0, "Too many nested calls to dispatch_suspend()");
	}
	return _dispatch_queue_sidelock_unlock(dq);

retry:
	_dispatch_queue_sidelock_unlock(dq);
	return _dispatch_lane_suspend(dq);
}

void
_dispatch_lane_suspend(dispatch_lane_t dq)
{
	uint64_t old_state, new_state;

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		new_state = DISPATCH_QUEUE_SUSPEND_INTERVAL;
		if (unlikely(os_add_overflow(old_state, new_state, &new_state))) {
			os_atomic_rmw_loop_give_up({
				return _dispatch_lane_suspend_slow(dq);
			});
		}
	});

	if (!_dq_state_is_suspended(old_state)) {
		// rdar://8181908 we need to extend the queue life for the duration
		// of the call to wakeup at _dispatch_lane_resume() time.
		_dispatch_retain_2(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_lane_resume_slow(dispatch_lane_t dq)
{
	uint64_t old_state, new_state, delta;

	_dispatch_queue_sidelock_lock(dq);

	// what we want to transfer
	delta  = DISPATCH_QUEUE_SUSPEND_HALF * DISPATCH_QUEUE_SUSPEND_INTERVAL;
	// but this is a resume so consume a suspend count at the same time
	delta -= DISPATCH_QUEUE_SUSPEND_INTERVAL;
	switch (dq->dq_side_suspend_cnt) {
	case 0:
		goto retry;
	case DISPATCH_QUEUE_SUSPEND_HALF:
		// we will transition the side count to 0, so we want to clear this bit
		delta -= DISPATCH_QUEUE_HAS_SIDE_SUSPEND_CNT;
		break;
	}
	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		// unsigned overflow of the addition can happen because other
		// threads could have touched this value while we were trying to acquire
		// the lock, or because another thread raced us to do the same operation
		// and got to the lock first.
		if (unlikely(os_add_overflow(old_state, delta, &new_state))) {
			os_atomic_rmw_loop_give_up(goto retry);
		}
	});
	dq->dq_side_suspend_cnt -= DISPATCH_QUEUE_SUSPEND_HALF;
	return _dispatch_queue_sidelock_unlock(dq);

retry:
	_dispatch_queue_sidelock_unlock(dq);
	return _dispatch_lane_resume(dq, DISPATCH_RESUME);
}

DISPATCH_NOINLINE
static void
_dispatch_lane_resume_activate(dispatch_lane_t dq)
{
	if (dx_vtable(dq)->dq_activate) {
		dx_vtable(dq)->dq_activate(dq);
	}

	_dispatch_lane_resume(dq, DISPATCH_ACTIVATION_DONE);
}

DISPATCH_NOINLINE
void
_dispatch_lane_resume(dispatch_lane_t dq, dispatch_resume_op_t op)
{
	// covers all suspend and inactive bits, including side suspend bit
	const uint64_t suspend_bits = DISPATCH_QUEUE_SUSPEND_BITS_MASK;
	uint64_t pending_barrier_width =
			(dq->dq_width - 1) * DISPATCH_QUEUE_WIDTH_INTERVAL;
	uint64_t set_owner_and_set_full_width_and_in_barrier =
			_dispatch_lock_value_for_self() | DISPATCH_QUEUE_WIDTH_FULL_BIT |
			DISPATCH_QUEUE_IN_BARRIER;

	// backward compatibility: only dispatch sources can abuse
	// dispatch_resume() to really mean dispatch_activate()
	bool is_source = (dx_metatype(dq) == _DISPATCH_SOURCE_TYPE);
	uint64_t old_state, new_state;

	//
	// Activation is a bit tricky as it needs to finalize before the wakeup.
	//
	// The inactive bits have 4 states:
	// - 11: INACTIVE
	// - 10: ACTIVATED, but not activating yet
	// - 01: ACTIVATING right now
	// - 00: fully active
	//
	// ACTIVATED is only used when the queue is otherwise also suspended.
	// In that case the last resume will take over the activation.
	//
	// The ACTIVATING state is tricky because it may be cleared by sources
	// firing, to avoid priority inversions problems such as rdar://45419440
	// where as soon as the kevent is installed, the source may fire
	// before its activating state was cleared.
	//
	if (op == DISPATCH_ACTIVATE) {
		// relaxed atomic because this doesn't publish anything, this is only
		// about picking the thread that gets to finalize the activation
		os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
			if (!_dq_state_is_inactive(old_state)) {
				// object already active or activated
				os_atomic_rmw_loop_give_up(return);
			}
			if (unlikely(_dq_state_suspend_cnt(old_state))) {
				// { sc != 0, i = INACTIVE } -> i = ACTIVATED
				new_state = old_state - DISPATCH_QUEUE_INACTIVE +
						DISPATCH_QUEUE_ACTIVATED;
			} else {
				// { sc = 0, i = INACTIVE } -> i = ACTIVATING
				new_state = old_state - DISPATCH_QUEUE_INACTIVE +
						DISPATCH_QUEUE_ACTIVATING;
			}
		});
	} else if (op == DISPATCH_ACTIVATION_DONE) {
		// release barrier needed to publish the effect of dq_activate()
		os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
			if (unlikely(!(old_state & DISPATCH_QUEUE_INACTIVE_BITS_MASK))) {
				os_atomic_rmw_loop_give_up({
					// object activation was already concurrently done
					// due to a concurrent DISPATCH_WAKEUP_CLEAR_ACTIVATING
					// wakeup call.
					//
					// We still need to consume the internal refcounts because
					// the wakeup doesn't take care of these.
					return _dispatch_release_2_tailcall(dq);
				});
			}

			new_state = old_state - DISPATCH_QUEUE_ACTIVATING;
			if (!_dq_state_is_runnable(new_state)) {
				// Out of width or still suspended.
				// For the former, force _dispatch_lane_non_barrier_complete
				// to reconsider whether it has work to do
				new_state |= DISPATCH_QUEUE_DIRTY;
			} else if (_dq_state_drain_locked(new_state)) {
				// still locked by someone else, make drain_try_unlock() fail
				// and reconsider whether it has work to do
				new_state |= DISPATCH_QUEUE_DIRTY;
			} else {
				// clear overrides and force a wakeup
				new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
				new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
			}
		});
		if (unlikely(new_state & DISPATCH_QUEUE_INACTIVE_BITS_MASK)) {
			DISPATCH_CLIENT_CRASH(dq, "Corrupt activation state");
		}
	} else {
		// release barrier needed to publish the effect of
		// - dispatch_set_target_queue()
		// - dispatch_set_*_handler()
		os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
			new_state = old_state;
			if (is_source && (old_state & suspend_bits) ==
					DISPATCH_QUEUE_INACTIVE) {
				// { sc = 0, i = INACTIVE } -> i = ACTIVATING
				new_state -= DISPATCH_QUEUE_INACTIVE;
				new_state += DISPATCH_QUEUE_ACTIVATING;
			} else if (unlikely(os_sub_overflow(old_state,
					DISPATCH_QUEUE_SUSPEND_INTERVAL, &new_state))) {
				// underflow means over-resume or a suspend count transfer
				// to the side count is needed
				os_atomic_rmw_loop_give_up({
					if (!(old_state & DISPATCH_QUEUE_HAS_SIDE_SUSPEND_CNT)) {
						goto over_resume;
					}
					return _dispatch_lane_resume_slow(dq);
				});
		//
		// below this, new_state = old_state - DISPATCH_QUEUE_SUSPEND_INTERVAL
		//
			} else if (_dq_state_is_activated(new_state)) {
				// { sc = 1, i = ACTIVATED } -> i = ACTIVATING
				new_state -= DISPATCH_QUEUE_ACTIVATED;
				new_state += DISPATCH_QUEUE_ACTIVATING;
			} else if (!_dq_state_is_runnable(new_state)) {
				// Out of width or still suspended.
				// For the former, force _dispatch_lane_non_barrier_complete
				// to reconsider whether it has work to do
				new_state |= DISPATCH_QUEUE_DIRTY;
			} else if (_dq_state_drain_locked(new_state)) {
				// still locked by someone else, make drain_try_unlock() fail
				// and reconsider whether it has work to do
				new_state |= DISPATCH_QUEUE_DIRTY;
			} else if (!is_source && (_dq_state_has_pending_barrier(new_state) ||
					new_state + pending_barrier_width <
					DISPATCH_QUEUE_WIDTH_FULL_BIT)) {
				// if we can, acquire the full width drain lock
				// and then perform a lock transfer
				//
				// However this is never useful for a source where there are no
				// sync waiters, so never take the lock and do a plain wakeup
				new_state &= DISPATCH_QUEUE_DRAIN_PRESERVED_BITS_MASK;
				new_state |= set_owner_and_set_full_width_and_in_barrier;
			} else {
				// clear overrides and force a wakeup
				new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
				new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
			}
		});
	}

	if (_dq_state_is_activating(new_state)) {
		return _dispatch_lane_resume_activate(dq);
	}

	if (_dq_state_is_suspended(new_state)) {
		return;
	}

	if (_dq_state_is_dirty(old_state)) {
		// <rdar://problem/14637483>
		// dependency ordering for dq state changes that were flushed
		// and not acted upon
		os_atomic_thread_fence(dependency);
		dq = os_atomic_force_dependency_on(dq, old_state);
	}
	// Balancing the retain_2 done in suspend() for rdar://8181908
	dispatch_wakeup_flags_t flags = DISPATCH_WAKEUP_CONSUME_2;
	if ((old_state ^ new_state) & DISPATCH_QUEUE_IN_BARRIER) {
		flags |= DISPATCH_WAKEUP_BARRIER_COMPLETE;
	} else if (!_dq_state_is_runnable(new_state)) {
		if (_dq_state_is_base_wlh(old_state)) {
			_dispatch_event_loop_assert_not_owned((dispatch_wlh_t)dq);
		}
		return _dispatch_release_2(dq);
	}
	dispatch_assert(!_dq_state_received_sync_wait(old_state));
	dispatch_assert(!_dq_state_in_sync_transfer(old_state));
	return dx_wakeup(dq, _dq_state_max_qos(old_state), flags);

over_resume:
	if (unlikely(_dq_state_is_inactive(old_state))) {
		DISPATCH_CLIENT_CRASH(dq, "Over-resume of an inactive object");
	}
	DISPATCH_CLIENT_CRASH(dq, "Over-resume of an object");
}

const char *
dispatch_queue_get_label(dispatch_queue_t dq)
{
	if (unlikely(dq == DISPATCH_CURRENT_QUEUE_LABEL)) {
		dq = _dispatch_queue_get_current_or_default();
	}
	return dq->dq_label ? dq->dq_label : "";
}

qos_class_t
dispatch_queue_get_qos_class(dispatch_queue_t dq, int *relpri_ptr)
{
	dispatch_priority_t pri = dq->dq_priority;
	dispatch_qos_t qos = _dispatch_priority_qos(pri);
	if (relpri_ptr) {
		*relpri_ptr = qos ? _dispatch_priority_relpri(dq->dq_priority) : 0;
	}
	return _dispatch_qos_to_qos_class(qos);
}

static void
_dispatch_lane_set_width(void *ctxt)
{
	int w = (int)(intptr_t)ctxt; // intentional truncation
	uint32_t tmp;
	dispatch_lane_t dq = upcast(_dispatch_queue_get_current())._dl;

	if (w >= 0) {
		tmp = w ? (unsigned int)w : 1;
	} else {
		dispatch_qos_t qos = _dispatch_qos_from_pp(_dispatch_get_priority());
		switch (w) {
		case DISPATCH_QUEUE_WIDTH_MAX_PHYSICAL_CPUS:
			tmp = _dispatch_qos_max_parallelism(qos,
					DISPATCH_MAX_PARALLELISM_PHYSICAL);
			break;
		case DISPATCH_QUEUE_WIDTH_ACTIVE_CPUS:
			tmp = _dispatch_qos_max_parallelism(qos,
					DISPATCH_MAX_PARALLELISM_ACTIVE);
			break;
		case DISPATCH_QUEUE_WIDTH_MAX_LOGICAL_CPUS:
		default:
			tmp = _dispatch_qos_max_parallelism(qos, 0);
			break;
		}
	}
	if (tmp > DISPATCH_QUEUE_WIDTH_MAX) {
		tmp = DISPATCH_QUEUE_WIDTH_MAX;
	}

	dispatch_queue_flags_t old_dqf, new_dqf;
	os_atomic_rmw_loop2o(dq, dq_atomic_flags, old_dqf, new_dqf, relaxed, {
		new_dqf = (old_dqf & DQF_FLAGS_MASK) | DQF_WIDTH(tmp);
	});
	_dispatch_lane_inherit_wlh_from_target(dq, dq->do_targetq);
	_dispatch_object_debug(dq, "%s", __func__);
}

void
dispatch_queue_set_width(dispatch_queue_t dq, long width)
{
	unsigned long type = dx_type(dq);
	if (unlikely(dx_metatype(dq) != _DISPATCH_LANE_TYPE)) {
		DISPATCH_CLIENT_CRASH(type, "Unexpected dispatch object type");
	} else if (unlikely(type != DISPATCH_QUEUE_CONCURRENT_TYPE)) {
		DISPATCH_CLIENT_CRASH(type, "Cannot set width of a serial queue");
	}

	if (likely((int)width >= 0)) {
		dispatch_lane_t dl = upcast(dq)._dl;
		_dispatch_barrier_trysync_or_async_f(dl, (void*)(intptr_t)width,
				_dispatch_lane_set_width, DISPATCH_BARRIER_TRYSYNC_SUSPEND);
	} else {
		// The negative width constants need to execute on the queue to
		// query the queue QoS
		_dispatch_barrier_async_detached_f(dq, (void*)(intptr_t)width,
				_dispatch_lane_set_width);
	}
}

static void
_dispatch_lane_legacy_set_target_queue(void *ctxt)
{
	dispatch_lane_t dq = upcast(_dispatch_queue_get_current())._dl;
	dispatch_queue_t tq = ctxt;
	dispatch_queue_t otq = dq->do_targetq;

	if (_dispatch_queue_atomic_flags(dq) & DQF_TARGETED) {
#if DISPATCH_ALLOW_NON_LEAF_RETARGET
		_dispatch_ktrace3(DISPATCH_PERF_non_leaf_retarget, dq, otq, tq);
		_dispatch_bug_deprecated("Changing the target of a queue "
				"already targeted by other dispatch objects");
#else
		DISPATCH_CLIENT_CRASH(0, "Cannot change the target of a queue "
				"already targeted by other dispatch objects");
#endif
	}

	tq = _dispatch_queue_priority_inherit_from_target(dq, tq);
	_dispatch_lane_inherit_wlh_from_target(dq, tq);
#if HAVE_PTHREAD_WORKQUEUE_QOS
	// see _dispatch_queue_wakeup()
	_dispatch_queue_sidelock_lock(dq);
#endif
	if (unlikely(!_dispatch_queue_is_mutable(dq))) {
		/* serialize with _dispatch_mach_handoff_set_wlh */
		DISPATCH_CLIENT_CRASH(0, "Cannot change the target of this object "
				"after it has been activated");
	}
	dq->do_targetq = tq;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	// see _dispatch_queue_wakeup()
	_dispatch_queue_sidelock_unlock(dq);
#endif

	_dispatch_object_debug(dq, "%s", __func__);
	_dispatch_introspection_target_queue_changed(dq->_as_dq);
	_dispatch_release_tailcall(otq);
}

void
_dispatch_lane_set_target_queue(dispatch_lane_t dq, dispatch_queue_t tq)
{
	if (tq == DISPATCH_TARGET_QUEUE_DEFAULT) {
		bool overcommit = (dq->dq_width == 1);
		tq = _dispatch_get_default_queue(overcommit);
	}

	if (_dispatch_lane_try_inactive_suspend(dq)) {
		_dispatch_object_set_target_queue_inline(dq, tq);
		return _dispatch_lane_resume(dq, DISPATCH_RESUME);
	}

#if !DISPATCH_ALLOW_NON_LEAF_RETARGET
	if (_dispatch_queue_atomic_flags(dq) & DQF_TARGETED) {
		DISPATCH_CLIENT_CRASH(0, "Cannot change the target of a queue "
				"already targeted by other dispatch objects");
	}
#endif

	if (unlikely(!_dispatch_queue_is_mutable(dq))) {
#if DISPATCH_ALLOW_NON_LEAF_RETARGET
		if (_dispatch_queue_atomic_flags(dq) & DQF_TARGETED) {
			DISPATCH_CLIENT_CRASH(0, "Cannot change the target of a queue "
					"already targeted by other dispatch objects");
		}
#endif
		DISPATCH_CLIENT_CRASH(0, "Cannot change the target of this object "
				"after it has been activated");
	}

	unsigned long metatype = dx_metatype(dq);
	switch (metatype) {
	case _DISPATCH_LANE_TYPE:
#if DISPATCH_ALLOW_NON_LEAF_RETARGET
		if (_dispatch_queue_atomic_flags(dq) & DQF_TARGETED) {
			_dispatch_bug_deprecated("Changing the target of a queue "
					"already targeted by other dispatch objects");
		}
#endif
		break;
	case _DISPATCH_SOURCE_TYPE:
		_dispatch_ktrace1(DISPATCH_PERF_post_activate_retarget, dq);
		_dispatch_bug_deprecated("Changing the target of a source "
				"after it has been activated");
		break;
	default:
		DISPATCH_CLIENT_CRASH(metatype, "Unexpected dispatch object type");
	}

	_dispatch_retain(tq);
	return _dispatch_barrier_trysync_or_async_f(dq, tq,
			_dispatch_lane_legacy_set_target_queue,
			DISPATCH_BARRIER_TRYSYNC_SUSPEND);
}

#pragma mark -
#pragma mark _dispatch_queue_debug

size_t
_dispatch_queue_debug_attr(dispatch_queue_t dq, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	dispatch_queue_t target = dq->do_targetq;
	const char *tlabel = target && target->dq_label ? target->dq_label : "";
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);

	offset += dsnprintf(&buf[offset], bufsiz - offset, "sref = %d, "
			"target = %s[%p], width = 0x%x, state = 0x%016llx",
			dq->dq_sref_cnt + 1, tlabel, target, dq->dq_width,
			(unsigned long long)dq_state);
	if (_dq_state_is_suspended(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", suspended = %d",
			_dq_state_suspend_cnt(dq_state));
	}
	if (_dq_state_is_inactive(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", inactive");
	} else if (_dq_state_is_activated(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", activated");
	} else if (_dq_state_is_activating(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", activating");
	}
	if (_dq_state_is_enqueued(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", enqueued");
	}
	if (_dq_state_is_dirty(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", dirty");
	}
	dispatch_qos_t qos = _dq_state_max_qos(dq_state);
	if (qos) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", max qos %d", qos);
	}
	mach_port_t owner = _dq_state_drain_owner(dq_state);
	if (!_dispatch_queue_is_thread_bound(dq) && owner) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", draining on 0x%x",
				owner);
	}
	if (_dq_state_is_in_barrier(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", in-barrier");
	} else  {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", in-flight = %d",
				_dq_state_used_width(dq_state, dq->dq_width));
	}
	if (_dq_state_has_pending_barrier(dq_state)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", pending-barrier");
	}
	if (_dispatch_queue_is_thread_bound(dq)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, ", thread = 0x%x ",
				owner);
	}
	return offset;
}

size_t
_dispatch_queue_debug(dispatch_queue_t dq, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			dq->dq_label ? dq->dq_label : _dispatch_object_class_name(dq), dq);
	offset += _dispatch_object_debug_attr(dq, &buf[offset], bufsiz - offset);
	offset += _dispatch_queue_debug_attr(dq, &buf[offset], bufsiz - offset);
	offset += dsnprintf(&buf[offset], bufsiz - offset, "}");
	return offset;
}

#if DISPATCH_PERF_MON

#define DISPATCH_PERF_MON_BUCKETS 8

static struct {
	uint64_t volatile time_total;
	uint64_t volatile count_total;
	uint64_t volatile thread_total;
} _dispatch_stats[DISPATCH_PERF_MON_BUCKETS] DISPATCH_ATOMIC64_ALIGN;
DISPATCH_USED static size_t _dispatch_stat_buckets = DISPATCH_PERF_MON_BUCKETS;

void
_dispatch_queue_merge_stats(uint64_t start, bool trace, perfmon_thread_type type)
{
	uint64_t delta = _dispatch_uptime() - start;
	unsigned long count;
	int bucket = 0;
	count = (unsigned long)_dispatch_thread_getspecific(dispatch_bcounter_key);
	_dispatch_thread_setspecific(dispatch_bcounter_key, NULL);
	if (count == 0) {
		bucket = 0;
		if (trace) _dispatch_ktrace1(DISPATCH_PERF_MON_worker_useless, type);
	} else {
		bucket = MIN(DISPATCH_PERF_MON_BUCKETS - 1,
				(int)sizeof(count) * CHAR_BIT - __builtin_clzl(count));
		os_atomic_add(&_dispatch_stats[bucket].count_total, count, relaxed);
	}
	os_atomic_add(&_dispatch_stats[bucket].time_total, delta, relaxed);
	os_atomic_inc(&_dispatch_stats[bucket].thread_total, relaxed);
	if (trace) {
		_dispatch_ktrace3(DISPATCH_PERF_MON_worker_thread_end, count, delta, type);
	}
}

#endif

#pragma mark -
#pragma mark dispatch queue/lane drain & invoke

DISPATCH_NOINLINE
static void
_dispatch_return_to_kernel(void)
{
#if DISPATCH_USE_KEVENT_WORKQUEUE
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	if (likely(ddi && ddi->ddi_wlh != DISPATCH_WLH_ANON)) {
		dispatch_assert(ddi->ddi_wlh_servicing);
		_dispatch_event_loop_drain(KEVENT_FLAG_IMMEDIATE);
	} else {
		_dispatch_clear_return_to_kernel();
	}
#endif
}

void
_dispatch_poll_for_events_4launchd(void)
{
	_dispatch_return_to_kernel();
}

#if DISPATCH_USE_WORKQUEUE_NARROWING
DISPATCH_STATIC_GLOBAL(os_atomic(uint64_t)
_dispatch_narrowing_deadlines[DISPATCH_QOS_NBUCKETS]);
#if !DISPATCH_TIME_UNIT_USES_NANOSECONDS
DISPATCH_STATIC_GLOBAL(uint64_t _dispatch_narrow_check_interval_cache);
#endif

DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dispatch_narrow_check_interval(void)
{
#if DISPATCH_TIME_UNIT_USES_NANOSECONDS
	return 50 * NSEC_PER_MSEC;
#else
	if (_dispatch_narrow_check_interval_cache == 0) {
		_dispatch_narrow_check_interval_cache =
				_dispatch_time_nano2mach(50 * NSEC_PER_MSEC);
	}
	return _dispatch_narrow_check_interval_cache;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_drain_init_narrowing_check_deadline(dispatch_invoke_context_t dic,
		dispatch_priority_t pri)
{
	if (!(pri & DISPATCH_PRIORITY_FLAG_OVERCOMMIT)) {
		dic->dic_next_narrow_check = _dispatch_approximate_time() +
				_dispatch_narrow_check_interval();
	}
}

DISPATCH_NOINLINE
static bool
_dispatch_queue_drain_should_narrow_slow(uint64_t now,
		dispatch_invoke_context_t dic)
{
	if (dic->dic_next_narrow_check != DISPATCH_THREAD_IS_NARROWING) {
		pthread_priority_t pp = _dispatch_get_priority();
		dispatch_qos_t qos = _dispatch_qos_from_pp(pp);
		if (unlikely(qos < DISPATCH_QOS_MIN || qos > DISPATCH_QOS_MAX)) {
			DISPATCH_CLIENT_CRASH(pp, "Thread QoS corruption");
		}
		size_t idx = DISPATCH_QOS_BUCKET(qos);
		os_atomic(uint64_t) *deadline = &_dispatch_narrowing_deadlines[idx];
		uint64_t oldval, newval = now + _dispatch_narrow_check_interval();

		dic->dic_next_narrow_check = newval;
		os_atomic_rmw_loop(deadline, oldval, newval, relaxed, {
			if (now < oldval) {
				os_atomic_rmw_loop_give_up(return false);
			}
		});

		if (!_pthread_workqueue_should_narrow(pp)) {
			return false;
		}
		dic->dic_next_narrow_check = DISPATCH_THREAD_IS_NARROWING;
	}
	return true;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_drain_should_narrow(dispatch_invoke_context_t dic)
{
	uint64_t next_check = dic->dic_next_narrow_check;
	if (unlikely(next_check)) {
		uint64_t now = _dispatch_approximate_time();
		if (unlikely(next_check < now)) {
			return _dispatch_queue_drain_should_narrow_slow(now, dic);
		}
	}
	return false;
}
#else
#define _dispatch_queue_drain_init_narrowing_check_deadline(rq, dic) ((void)0)
#define _dispatch_queue_drain_should_narrow(dic)  false
#endif

/*
 * Drain comes in 2 flavours (serial/concurrent) and 2 modes
 * (redirecting or not).
 *
 * Serial
 * ~~~~~~
 * Serial drain is about serial queues (width == 1). It doesn't support
 * the redirecting mode, which doesn't make sense, and treats all continuations
 * as barriers. Bookkeeping is minimal in serial flavour, most of the loop
 * is optimized away.
 *
 * Serial drain stops if the width of the queue grows to larger than 1.
 * Going through a serial drain prevents any recursive drain from being
 * redirecting.
 *
 * Concurrent
 * ~~~~~~~~~~
 * When in non-redirecting mode (meaning one of the target queues is serial),
 * non-barriers and barriers alike run in the context of the drain thread.
 * Slow non-barrier items are still all signaled so that they can make progress
 * toward the dispatch_sync() that will serialize them all .
 *
 * In redirecting mode, non-barrier work items are redirected downward.
 *
 * Concurrent drain stops if the width of the queue becomes 1, so that the
 * queue drain moves to the more efficient serial mode.
 */
DISPATCH_ALWAYS_INLINE
static dispatch_queue_wakeup_target_t
_dispatch_lane_drain(dispatch_lane_t dq, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags, uint64_t *owned_ptr, bool serial_drain)
{
	dispatch_queue_t orig_tq = dq->do_targetq;
	dispatch_thread_frame_s dtf;
	struct dispatch_object_s *dc = NULL, *next_dc;
	uint64_t dq_state, owned = *owned_ptr;

	if (unlikely(!dq->dq_items_tail)) return NULL;

	_dispatch_thread_frame_push(&dtf, dq);
	if (serial_drain || _dq_state_is_in_barrier(owned)) {
		// we really own `IN_BARRIER + dq->dq_width * WIDTH_INTERVAL`
		// but width can change while draining barrier work items, so we only
		// convert to `dq->dq_width * WIDTH_INTERVAL` when we drop `IN_BARRIER`
		owned = DISPATCH_QUEUE_IN_BARRIER;
	} else {
		owned &= DISPATCH_QUEUE_WIDTH_MASK;
	}

	dc = _dispatch_queue_get_head(dq);
	goto first_iteration;

	for (;;) {
		dispatch_assert(dic->dic_barrier_waiter == NULL);
		dc = next_dc;
		if (unlikely(!dc)) {
			if (!dq->dq_items_tail) {
				break;
			}
			dc = _dispatch_queue_get_head(dq);
		}
		if (unlikely(_dispatch_needs_to_return_to_kernel())) {
			_dispatch_return_to_kernel();
		}
		if (unlikely(serial_drain != (dq->dq_width == 1))) {
			break;
		}
		if (unlikely(_dispatch_queue_drain_should_narrow(dic))) {
			break;
		}
		if (likely(flags & DISPATCH_INVOKE_WORKLOOP_DRAIN)) {
			dispatch_workloop_t dwl = (dispatch_workloop_t)_dispatch_get_wlh();
			if (unlikely(_dispatch_queue_max_qos(dwl) > dwl->dwl_drained_qos)) {
				break;
			}
		}

first_iteration:
		dq_state = os_atomic_load(&dq->dq_state, relaxed);
		if (unlikely(_dq_state_is_suspended(dq_state))) {
			break;
		}
		if (unlikely(orig_tq != dq->do_targetq)) {
			break;
		}

		if (serial_drain || _dispatch_object_is_barrier(dc)) {
			if (!serial_drain && owned != DISPATCH_QUEUE_IN_BARRIER) {
				if (!_dispatch_queue_try_upgrade_full_width(dq, owned)) {
					goto out_with_no_width;
				}
				owned = DISPATCH_QUEUE_IN_BARRIER;
			}
			if (_dispatch_object_is_sync_waiter(dc) &&
					!(flags & DISPATCH_INVOKE_THREAD_BOUND)) {
				dic->dic_barrier_waiter = dc;
				goto out_with_barrier_waiter;
			}
			next_dc = _dispatch_queue_pop_head(dq, dc);
		} else {
			if (owned == DISPATCH_QUEUE_IN_BARRIER) {
				// we just ran barrier work items, we have to make their
				// effect visible to other sync work items on other threads
				// that may start coming in after this point, hence the
				// release barrier
				os_atomic_xor2o(dq, dq_state, owned, release);
				owned = dq->dq_width * DISPATCH_QUEUE_WIDTH_INTERVAL;
			} else if (unlikely(owned == 0)) {
				if (_dispatch_object_is_waiter(dc)) {
					// sync "readers" don't observe the limit
					_dispatch_queue_reserve_sync_width(dq);
				} else if (!_dispatch_queue_try_acquire_async(dq)) {
					goto out_with_no_width;
				}
				owned = DISPATCH_QUEUE_WIDTH_INTERVAL;
			}

			next_dc = _dispatch_queue_pop_head(dq, dc);
			if (_dispatch_object_is_waiter(dc)) {
				owned -= DISPATCH_QUEUE_WIDTH_INTERVAL;
				_dispatch_non_barrier_waiter_redirect_or_wake(dq, dc);
				continue;
			}

			if (flags & DISPATCH_INVOKE_REDIRECTING_DRAIN) {
				owned -= DISPATCH_QUEUE_WIDTH_INTERVAL;
				// This is a re-redirect, overrides have already been applied by
				// _dispatch_continuation_async*
				// However we want to end up on the root queue matching `dc`
				// qos, so pick up the current override of `dq` which includes
				// dc's override (and maybe more)
				_dispatch_continuation_redirect_push(dq, dc,
						_dispatch_queue_max_qos(dq));
				continue;
			}
		}

		_dispatch_continuation_pop_inline(dc, dic, flags, dq);
	}

	if (owned == DISPATCH_QUEUE_IN_BARRIER) {
		// if we're IN_BARRIER we really own the full width too
		owned += dq->dq_width * DISPATCH_QUEUE_WIDTH_INTERVAL;
	}
	if (dc) {
		owned = _dispatch_queue_adjust_owned(dq, owned, dc);
	}
	*owned_ptr &= DISPATCH_QUEUE_ENQUEUED | DISPATCH_QUEUE_ENQUEUED_ON_MGR;
	*owned_ptr |= owned;
	_dispatch_thread_frame_pop(&dtf);
	return dc ? dq->do_targetq : NULL;

out_with_no_width:
	*owned_ptr &= DISPATCH_QUEUE_ENQUEUED | DISPATCH_QUEUE_ENQUEUED_ON_MGR;
	_dispatch_thread_frame_pop(&dtf);
	return DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT;

out_with_barrier_waiter:
	if (unlikely(flags & DISPATCH_INVOKE_DISALLOW_SYNC_WAITERS)) {
		DISPATCH_INTERNAL_CRASH(0,
				"Deferred continuation on source, mach channel or mgr");
	}
	_dispatch_thread_frame_pop(&dtf);
	return dq->do_targetq;
}

DISPATCH_NOINLINE
static dispatch_queue_wakeup_target_t
_dispatch_lane_concurrent_drain(dispatch_lane_class_t dqu,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags,
		uint64_t *owned)
{
	return _dispatch_lane_drain(dqu._dl, dic, flags, owned, false);
}

DISPATCH_NOINLINE
dispatch_queue_wakeup_target_t
_dispatch_lane_serial_drain(dispatch_lane_class_t dqu,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags,
		uint64_t *owned)
{
	flags &= ~(dispatch_invoke_flags_t)DISPATCH_INVOKE_REDIRECTING_DRAIN;
	return _dispatch_lane_drain(dqu._dl, dic, flags, owned, true);
}

void
_dispatch_queue_invoke_finish(dispatch_queue_t dq,
		dispatch_invoke_context_t dic, dispatch_queue_t tq, uint64_t owned)
{
	struct dispatch_object_s *dc = dic->dic_barrier_waiter;
	dispatch_qos_t qos = dic->dic_barrier_waiter_bucket;
	if (dc) {
		dic->dic_barrier_waiter = NULL;
		dic->dic_barrier_waiter_bucket = DISPATCH_QOS_UNSPECIFIED;
		owned &= DISPATCH_QUEUE_ENQUEUED | DISPATCH_QUEUE_ENQUEUED_ON_MGR;
#if DISPATCH_INTROSPECTION
		dispatch_sync_context_t dsc = (dispatch_sync_context_t)dc;
		dsc->dsc_from_async = true;
#endif
		if (qos) {
			return _dispatch_workloop_drain_barrier_waiter(upcast(dq)._dwl,
					dc, qos, DISPATCH_WAKEUP_CONSUME_2, owned);
		}
		return _dispatch_lane_drain_barrier_waiter(upcast(dq)._dl, dc,
				DISPATCH_WAKEUP_CONSUME_2, owned);
	}

	uint64_t old_state, new_state, enqueued = DISPATCH_QUEUE_ENQUEUED;
	if (tq == DISPATCH_QUEUE_WAKEUP_MGR) {
		enqueued = DISPATCH_QUEUE_ENQUEUED_ON_MGR;
	}
	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
		new_state  = old_state - owned;
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		new_state |= DISPATCH_QUEUE_DIRTY;
		if (_dq_state_is_runnable(new_state) &&
				!_dq_state_is_enqueued(new_state)) {
			// drain was not interupted for suspension
			// we will reenqueue right away, just put ENQUEUED back
			new_state |= enqueued;
		}
	});
	old_state -= owned;
	if (_dq_state_received_override(old_state)) {
		// Ensure that the root queue sees that this thread was overridden.
		_dispatch_set_basepri_override_qos(_dq_state_max_qos(new_state));
	}
	if ((old_state ^ new_state) & enqueued) {
		dispatch_assert(_dq_state_is_enqueued(new_state));
		return _dispatch_queue_push_queue(tq, dq, new_state);
	}
	return _dispatch_release_2_tailcall(dq);
}

void
_dispatch_lane_activate(dispatch_lane_class_t dq)
{
	dispatch_queue_t tq = dq._dl->do_targetq;
	dispatch_priority_t pri = dq._dl->dq_priority;

	// Normalize priority: keep the fallback only when higher than the floor
	if (_dispatch_priority_fallback_qos(pri) <= _dispatch_priority_qos(pri) ||
			(_dispatch_priority_qos(pri) &&
			!(pri & DISPATCH_PRIORITY_FLAG_FLOOR))) {
		pri &= ~DISPATCH_PRIORITY_FALLBACK_QOS_MASK;
		pri &= ~DISPATCH_PRIORITY_FLAG_FALLBACK;
		dq._dl->dq_priority = pri;
	}
	tq = _dispatch_queue_priority_inherit_from_target(dq, tq);
	_dispatch_lane_inherit_wlh_from_target(dq._dl, tq);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_wakeup_target_t
_dispatch_lane_invoke2(dispatch_lane_t dq, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags, uint64_t *owned)
{
	dispatch_queue_t otq = dq->do_targetq;
	dispatch_queue_t cq = _dispatch_queue_get_current();

	if (unlikely(cq != otq)) {
		return otq;
	}
	if (dq->dq_width == 1) {
		return _dispatch_lane_serial_drain(dq, dic, flags, owned);
	}
	return _dispatch_lane_concurrent_drain(dq, dic, flags, owned);
}

DISPATCH_NOINLINE
void
_dispatch_lane_invoke(dispatch_lane_t dq, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags)
{
	_dispatch_queue_class_invoke(dq, dic, flags, 0, _dispatch_lane_invoke2);
}

#pragma mark -
#pragma mark dispatch_workloop_t

#define _dispatch_wl(dwl, qos) os_mpsc(dwl, dwl, s[DISPATCH_QOS_BUCKET(qos)])
#define _dispatch_workloop_looks_empty(dwl, qos) \
		os_mpsc_looks_empty(_dispatch_wl(dwl, qos))
#define _dispatch_workloop_get_head(dwl, qos) \
		os_mpsc_get_head(_dispatch_wl(dwl, qos))
#define _dispatch_workloop_pop_head(dwl, qos, dc) \
		os_mpsc_pop_head(_dispatch_wl(dwl, qos), dc, do_next)
#define _dispatch_workloop_push_update_tail(dwl, qos, dou) \
		os_mpsc_push_update_tail(_dispatch_wl(dwl, qos), dou, do_next)
#define _dispatch_workloop_push_update_prev(dwl, qos, prev, dou) \
		os_mpsc_push_update_prev(_dispatch_wl(dwl, qos), prev, dou, do_next)

dispatch_workloop_t
dispatch_workloop_copy_current(void)
{
	dispatch_workloop_t dwl = _dispatch_wlh_to_workloop(_dispatch_get_wlh());
	if (likely(dwl)) {
		_os_object_retain_with_resurrect(dwl->_as_os_obj);
		return dwl;
	}
	return NULL;
}

bool
dispatch_workloop_is_current(dispatch_workloop_t dwl)
{
	return _dispatch_get_wlh() == (dispatch_wlh_t)dwl;
}

DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dispatch_workloop_role_bits(void)
{
#if DISPATCH_USE_KEVENT_WORKLOOP
	if (likely(_dispatch_kevent_workqueue_enabled)) {
		return DISPATCH_QUEUE_ROLE_BASE_WLH;
	}
#endif
	return DISPATCH_QUEUE_ROLE_BASE_ANON;
}

bool
_dispatch_workloop_should_yield_4NW(void)
{
	dispatch_workloop_t dwl = _dispatch_wlh_to_workloop(_dispatch_get_wlh());
	if (likely(dwl)) {
		return _dispatch_queue_max_qos(dwl) > dwl->dwl_drained_qos;
	}
	return false;
}

DISPATCH_NOINLINE
static dispatch_workloop_t
_dispatch_workloop_create(const char *label, uint64_t dq_state)
{
	dispatch_queue_flags_t dqf = DQF_AUTORELEASE_ALWAYS;
	dispatch_workloop_t dwl;

	if (label) {
		const char *tmp = _dispatch_strdup_if_mutable(label);
		if (tmp != label) {
			dqf |= DQF_LABEL_NEEDS_FREE;
			label = tmp;
		}
	}

	dq_state |= _dispatch_workloop_role_bits();

	dwl = _dispatch_queue_alloc(workloop, dqf, 1, dq_state)._dwl;
	dwl->dq_label = label;
	dwl->do_targetq = _dispatch_get_default_queue(true);
	if (!(dq_state & DISPATCH_QUEUE_INACTIVE)) {
		dwl->dq_priority = DISPATCH_PRIORITY_FLAG_OVERCOMMIT |
				_dispatch_priority_make_fallback(DISPATCH_QOS_DEFAULT);
	}
	_dispatch_object_debug(dwl, "%s", __func__);
	return _dispatch_introspection_queue_create(dwl)._dwl;
}

dispatch_workloop_t
dispatch_workloop_create(const char *label)
{
	return _dispatch_workloop_create(label, 0);
}

dispatch_workloop_t
dispatch_workloop_create_inactive(const char *label)
{
	return _dispatch_workloop_create(label, DISPATCH_QUEUE_INACTIVE);
}

void
dispatch_workloop_set_autorelease_frequency(dispatch_workloop_t dwl,
		dispatch_autorelease_frequency_t frequency)
{
	if (frequency == DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM) {
		_dispatch_queue_atomic_flags_set_and_clear(dwl,
				DQF_AUTORELEASE_ALWAYS, DQF_AUTORELEASE_NEVER);
	} else {
		_dispatch_queue_atomic_flags_set_and_clear(dwl,
				DQF_AUTORELEASE_NEVER, DQF_AUTORELEASE_ALWAYS);
	}
	_dispatch_queue_setter_assert_inactive(dwl);
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_workloop_attributes_dispose(dispatch_workloop_t dwl)
{
	if (dwl->dwl_attr) {
		free(dwl->dwl_attr);
	}
}

DISPATCH_ALWAYS_INLINE
static bool
_dispatch_workloop_has_kernel_attributes(dispatch_workloop_t dwl)
{
	return dwl->dwl_attr && (dwl->dwl_attr->dwla_flags &
			(DISPATCH_WORKLOOP_ATTR_HAS_SCHED |
			 DISPATCH_WORKLOOP_ATTR_HAS_POLICY |
			 DISPATCH_WORKLOOP_ATTR_HAS_CPUPERCENT));
}

void
dispatch_workloop_set_scheduler_priority(dispatch_workloop_t dwl, int priority,
		uint64_t flags)
{
	_dispatch_queue_setter_assert_inactive(dwl);
	_dispatch_workloop_attributes_alloc_if_needed(dwl);

	if (priority) {
		dwl->dwl_attr->dwla_sched.sched_priority = priority;
		dwl->dwl_attr->dwla_flags |= DISPATCH_WORKLOOP_ATTR_HAS_SCHED;
	} else {
		dwl->dwl_attr->dwla_sched.sched_priority = 0;
		dwl->dwl_attr->dwla_flags &= ~DISPATCH_WORKLOOP_ATTR_HAS_SCHED;
	}

	if (flags & DISPATCH_WORKLOOP_FIXED_PRIORITY) {
		dwl->dwl_attr->dwla_policy = POLICY_RR;
		dwl->dwl_attr->dwla_flags |= DISPATCH_WORKLOOP_ATTR_HAS_POLICY;
	} else {
		dwl->dwl_attr->dwla_flags &= ~DISPATCH_WORKLOOP_ATTR_HAS_POLICY;
	}
}

void
dispatch_workloop_set_qos_class_floor(dispatch_workloop_t dwl,
		qos_class_t cls, int relpri, uint64_t flags)
{
	_dispatch_queue_setter_assert_inactive(dwl);
	_dispatch_workloop_attributes_alloc_if_needed(dwl);

	dispatch_qos_t qos = _dispatch_qos_from_qos_class(cls);

	if (qos) {
		dwl->dwl_attr->dwla_pri = _dispatch_priority_make(qos, relpri);
		dwl->dwl_attr->dwla_flags |= DISPATCH_WORKLOOP_ATTR_HAS_QOS_CLASS;
	} else {
		dwl->dwl_attr->dwla_pri = 0;
		dwl->dwl_attr->dwla_flags &= ~DISPATCH_WORKLOOP_ATTR_HAS_QOS_CLASS;
	}

	if (flags & DISPATCH_WORKLOOP_FIXED_PRIORITY) {
		dwl->dwl_attr->dwla_policy = POLICY_RR;
		dwl->dwl_attr->dwla_flags |= DISPATCH_WORKLOOP_ATTR_HAS_POLICY;
	} else {
		dwl->dwl_attr->dwla_flags &= ~DISPATCH_WORKLOOP_ATTR_HAS_POLICY;
	}
}

void
dispatch_workloop_set_qos_class(dispatch_workloop_t dwl,
		qos_class_t cls, uint64_t flags)
{
	dispatch_workloop_set_qos_class_floor(dwl, cls, 0, flags);
}

void
dispatch_workloop_set_cpupercent(dispatch_workloop_t dwl, uint8_t percent,
		uint32_t refillms)
{
	_dispatch_queue_setter_assert_inactive(dwl);
	_dispatch_workloop_attributes_alloc_if_needed(dwl);

	if ((dwl->dwl_attr->dwla_flags & (DISPATCH_WORKLOOP_ATTR_HAS_SCHED |
			DISPATCH_WORKLOOP_ATTR_HAS_QOS_CLASS)) == 0) {
		DISPATCH_CLIENT_CRASH(0, "workloop qos class or priority must be "
				"set before cpupercent");
	}

	dwl->dwl_attr->dwla_cpupercent.percent = percent;
	dwl->dwl_attr->dwla_cpupercent.refillms = refillms;
	dwl->dwl_attr->dwla_flags |= DISPATCH_WORKLOOP_ATTR_HAS_CPUPERCENT;
}

#if DISPATCH_IOHID_SPI
void
_dispatch_workloop_set_observer_hooks_4IOHID(dispatch_workloop_t dwl,
		dispatch_pthread_root_queue_observer_hooks_t observer_hooks)
{
	_dispatch_queue_setter_assert_inactive(dwl);
	_dispatch_workloop_attributes_alloc_if_needed(dwl);

	dwl->dwl_attr->dwla_observers = *observer_hooks;
	dwl->dwl_attr->dwla_flags |= DISPATCH_WORKLOOP_ATTR_HAS_OBSERVERS;
}
#endif

static void
_dispatch_workloop_activate_simulator_fallback(dispatch_workloop_t dwl,
		pthread_attr_t *attr)
{
	uint64_t old_state, new_state;
	dispatch_queue_global_t dprq;

	dprq = dispatch_pthread_root_queue_create(
			"com.apple.libdispatch.workloop_fallback", 0, attr, NULL);

	dwl->do_targetq = dprq->_as_dq;
	_dispatch_retain(dprq);
	dispatch_release(dprq);

	os_atomic_rmw_loop2o(dwl, dq_state, old_state, new_state, relaxed, {
		new_state = old_state & ~DISPATCH_QUEUE_ROLE_MASK;
		new_state |= DISPATCH_QUEUE_ROLE_BASE_ANON;
	});
}

static const struct dispatch_queue_global_s _dispatch_custom_workloop_root_queue = {
	DISPATCH_GLOBAL_OBJECT_HEADER(queue_global),
	.dq_state = DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE,
	.do_ctxt = NULL,
	.dq_label = "com.apple.root.workloop-custom",
	.dq_atomic_flags = DQF_WIDTH(DISPATCH_QUEUE_WIDTH_POOL),
	.dq_priority = _dispatch_priority_make_fallback(DISPATCH_QOS_DEFAULT) |
			DISPATCH_PRIORITY_SATURATED_OVERRIDE,
	.dq_serialnum = DISPATCH_QUEUE_SERIAL_NUMBER_WLF,
	.dgq_thread_pool_size = 1,
};

static void
_dispatch_workloop_activate_attributes(dispatch_workloop_t dwl)
{
	dispatch_workloop_attr_t dwla = dwl->dwl_attr;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	if (dwla->dwla_flags & DISPATCH_WORKLOOP_ATTR_HAS_QOS_CLASS) {
		dwl->dq_priority |= dwla->dwla_pri | DISPATCH_PRIORITY_FLAG_FLOOR;
	}
	if (dwla->dwla_flags & DISPATCH_WORKLOOP_ATTR_HAS_SCHED) {
		pthread_attr_setschedparam(&attr, &dwla->dwla_sched);
		// _dispatch_async_and_wait_should_always_async detects when a queue
		// targets a root queue that is not part of the root queues array in
		// order to force async_and_wait to async. We want this path to always
		// be taken on workloops that have a scheduler priority set.
		dwl->do_targetq =
				(dispatch_queue_t)_dispatch_custom_workloop_root_queue._as_dq;
	}
	if (dwla->dwla_flags & DISPATCH_WORKLOOP_ATTR_HAS_POLICY) {
		pthread_attr_setschedpolicy(&attr, dwla->dwla_policy);
	}
	if (dwla->dwla_flags & DISPATCH_WORKLOOP_ATTR_HAS_CPUPERCENT) {
		pthread_attr_setcpupercent_np(&attr, dwla->dwla_cpupercent.percent,
				(unsigned long)dwla->dwla_cpupercent.refillms);
	}
	if (_dispatch_workloop_has_kernel_attributes(dwl)) {
		int rv = _pthread_workloop_create((uint64_t)dwl, 0, &attr);
		switch (rv) {
		case 0:
			dwla->dwla_flags |= DISPATCH_WORKLOOP_ATTR_NEEDS_DESTROY;
			break;
		case ENOTSUP:
			/* simulator fallback */
			_dispatch_workloop_activate_simulator_fallback(dwl, &attr);
			break;
		default:
			dispatch_assert_zero(rv);
		}
	}
	pthread_attr_destroy(&attr);
}

void
_dispatch_workloop_dispose(dispatch_workloop_t dwl, bool *allow_free)
{
	uint64_t dq_state = os_atomic_load2o(dwl, dq_state, relaxed);
	uint64_t initial_state = DISPATCH_QUEUE_STATE_INIT_VALUE(1);

	initial_state |= _dispatch_workloop_role_bits();

	if (unlikely(dq_state != initial_state)) {
		if (_dq_state_drain_locked(dq_state)) {
			DISPATCH_CLIENT_CRASH((uintptr_t)dq_state,
					"Release of a locked workloop");
		}
#ifndef __LP64__
		dq_state >>= 32;
#endif
		DISPATCH_CLIENT_CRASH((uintptr_t)dq_state,
				"Release of a workloop with corrupt state");
	}

	_dispatch_object_debug(dwl, "%s", __func__);
	_dispatch_introspection_queue_dispose(dwl);

	for (size_t i = 0; i < countof(dwl->dwl_tails); i++) {
		if (unlikely(dwl->dwl_tails[i])) {
			DISPATCH_CLIENT_CRASH(dwl->dwl_tails[i],
					"Release of a workloop while items are enqueued");
		}
		// trash the queue so that use after free will crash
		dwl->dwl_tails[i] = (void *)0x200;
		dwl->dwl_heads[i] = (void *)0x200;
	}

	if (dwl->dwl_timer_heap) {
		for (size_t i = 0; i < DISPATCH_TIMER_WLH_COUNT; i++) {
			dispatch_assert(dwl->dwl_timer_heap[i].dth_count == 0);
		}
		free(dwl->dwl_timer_heap);
		dwl->dwl_timer_heap = NULL;
	}

	if (dwl->dwl_attr && (dwl->dwl_attr->dwla_flags &
			DISPATCH_WORKLOOP_ATTR_NEEDS_DESTROY)) {
		(void)dispatch_assume_zero(_pthread_workloop_destroy((uint64_t)dwl));
	}
	_dispatch_workloop_attributes_dispose(dwl);
	_dispatch_queue_dispose(dwl, allow_free);
}

void
_dispatch_workloop_activate(dispatch_workloop_t dwl)
{
	// This transitions either:
	// - from INACTIVE to ACTIVATING
	// - or from ACTIVE to ACTIVE
	uint64_t old_state = os_atomic_and_orig2o(dwl, dq_state,
			~DISPATCH_QUEUE_ACTIVATED, relaxed);

	if (likely(_dq_state_is_inactive(old_state))) {
		if (dwl->dwl_attr) {
			// Activation of a workloop with attributes forces us to create
			// the workloop up front and register the attributes with the
			// kernel.
			_dispatch_workloop_activate_attributes(dwl);
		}
		if (!dwl->dq_priority) {
			dwl->dq_priority =
					_dispatch_priority_make_fallback(DISPATCH_QOS_DEFAULT);
		}
		dwl->dq_priority |= DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
		os_atomic_and2o(dwl, dq_state, ~DISPATCH_QUEUE_ACTIVATING, relaxed);
		return _dispatch_workloop_wakeup(dwl, 0, DISPATCH_WAKEUP_CONSUME_2);
	}
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_workloop_try_lower_max_qos(dispatch_workloop_t dwl,
		dispatch_qos_t qos)
{
	uint64_t old_state, new_state, qos_bits = _dq_state_from_qos(qos);

	os_atomic_rmw_loop2o(dwl, dq_state, old_state, new_state, relaxed, {
		if ((old_state & DISPATCH_QUEUE_MAX_QOS_MASK) <= qos_bits) {
			os_atomic_rmw_loop_give_up(return true);
		}

		if (unlikely(_dq_state_is_dirty(old_state))) {
			os_atomic_rmw_loop_give_up({
				os_atomic_xor2o(dwl, dq_state, DISPATCH_QUEUE_DIRTY, acquire);
				return false;
			});
		}

		new_state  = old_state;
		new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
		new_state |= qos_bits;
	});

	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	if (likely(ddi)) {
		ddi->ddi_wlh_needs_update = true;
		_dispatch_return_to_kernel();
	}
	return true;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_wakeup_target_t
_dispatch_workloop_invoke2(dispatch_workloop_t dwl,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags,
		uint64_t *owned)
{
	dispatch_workloop_attr_t dwl_attr = dwl->dwl_attr;
	dispatch_thread_frame_s dtf;
	struct dispatch_object_s *dc = NULL, *next_dc;

	if (dwl_attr &&
			(dwl_attr->dwla_flags & DISPATCH_WORKLOOP_ATTR_HAS_OBSERVERS)) {
		_dispatch_set_pthread_root_queue_observer_hooks(
				&dwl_attr->dwla_observers);
	}
	_dispatch_thread_frame_push(&dtf, dwl);

	for (;;) {
		dispatch_qos_t qos;
		for (qos = DISPATCH_QOS_MAX; qos >= DISPATCH_QOS_MIN; qos--) {
			if (!_dispatch_workloop_looks_empty(dwl, qos)) break;
		}
		if (qos < DISPATCH_QOS_MIN) {
			break;
		}
		if (unlikely(!_dispatch_workloop_try_lower_max_qos(dwl, qos))) {
			continue;
		}
		dwl->dwl_drained_qos = (uint8_t)qos;

		dc = _dispatch_workloop_get_head(dwl, qos);
		do {
			if (_dispatch_object_is_sync_waiter(dc)) {
				dic->dic_barrier_waiter_bucket = qos;
				dic->dic_barrier_waiter = dc;
				dwl->dwl_drained_qos = DISPATCH_QOS_UNSPECIFIED;
				goto out_with_barrier_waiter;
			}
			next_dc = _dispatch_workloop_pop_head(dwl, qos, dc);
			if (unlikely(_dispatch_needs_to_return_to_kernel())) {
				_dispatch_return_to_kernel();
			}

			_dispatch_continuation_pop_inline(dc, dic, flags, dwl);
			qos = dwl->dwl_drained_qos;
		} while ((dc = next_dc) && (_dispatch_queue_max_qos(dwl) <= qos));
	}

	*owned = (*owned & DISPATCH_QUEUE_ENQUEUED) +
			DISPATCH_QUEUE_IN_BARRIER + DISPATCH_QUEUE_WIDTH_INTERVAL;
	_dispatch_thread_frame_pop(&dtf);
	_dispatch_set_pthread_root_queue_observer_hooks(NULL);
	return NULL;

out_with_barrier_waiter:
	_dispatch_thread_frame_pop(&dtf);
	_dispatch_set_pthread_root_queue_observer_hooks(NULL);
	return dwl->do_targetq;
}

void
_dispatch_workloop_invoke(dispatch_workloop_t dwl,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags)
{
	flags &= ~(dispatch_invoke_flags_t)DISPATCH_INVOKE_REDIRECTING_DRAIN;
	flags |= DISPATCH_INVOKE_WORKLOOP_DRAIN;
	_dispatch_queue_class_invoke(dwl, dic, flags, 0, _dispatch_workloop_invoke2);
}

DISPATCH_ALWAYS_INLINE
static bool
_dispatch_workloop_probe(dispatch_workloop_t dwl)
{
	dispatch_qos_t qos;
	for (qos = DISPATCH_QOS_MAX; qos >= DISPATCH_QOS_MIN; qos--) {
		if (!_dispatch_workloop_looks_empty(dwl, qos)) return true;
	}
	return false;
}

DISPATCH_NOINLINE
static void
_dispatch_workloop_drain_barrier_waiter(dispatch_workloop_t dwl,
		struct dispatch_object_s *dc, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags, uint64_t enqueued_bits)
{
	dispatch_sync_context_t dsc = (dispatch_sync_context_t)dc;
	uint64_t next_owner = 0, old_state, new_state;
	bool has_more_work;

	next_owner = _dispatch_lock_value_from_tid(dsc->dsc_waiter);
	has_more_work = (_dispatch_workloop_pop_head(dwl, qos, dc) != NULL);

transfer_lock_again:
	if (!has_more_work) {
		has_more_work = _dispatch_workloop_probe(dwl);
	}

	os_atomic_rmw_loop2o(dwl, dq_state, old_state, new_state, release, {
		new_state  = old_state;
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		new_state &= ~DISPATCH_QUEUE_DIRTY;
		new_state |= next_owner;

		if (likely(_dq_state_is_base_wlh(old_state))) {
			new_state |= DISPATCH_QUEUE_SYNC_TRANSFER;
			if (has_more_work) {
				// we know there's a next item, keep the enqueued bit if any
			} else if (unlikely(_dq_state_is_dirty(old_state))) {
				os_atomic_rmw_loop_give_up({
					os_atomic_xor2o(dwl, dq_state, DISPATCH_QUEUE_DIRTY, acquire);
					goto transfer_lock_again;
				});
			} else {
				new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
				new_state &= ~DISPATCH_QUEUE_ENQUEUED;
			}
		} else {
			new_state -= enqueued_bits;
		}
	});

	return _dispatch_barrier_waiter_redirect_or_wake(dwl, dc, flags,
			old_state, new_state);
}

static void
_dispatch_workloop_barrier_complete(dispatch_workloop_t dwl, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	dispatch_queue_wakeup_target_t target = DISPATCH_QUEUE_WAKEUP_NONE;
	dispatch_qos_t wl_qos;

again:
	for (wl_qos = DISPATCH_QOS_MAX; wl_qos >= DISPATCH_QOS_MIN; wl_qos--) {
		struct dispatch_object_s *dc;

		if (_dispatch_workloop_looks_empty(dwl, wl_qos)) continue;
		dc = _dispatch_workloop_get_head(dwl, wl_qos);

		if (_dispatch_object_is_waiter(dc)) {
			return _dispatch_workloop_drain_barrier_waiter(dwl, dc, wl_qos,
					flags, 0);
		}

		// We have work to do, we need to wake up
		target = DISPATCH_QUEUE_WAKEUP_TARGET;
	}

	if (unlikely(target && !(flags & DISPATCH_WAKEUP_CONSUME_2))) {
		_dispatch_retain_2(dwl);
		flags |= DISPATCH_WAKEUP_CONSUME_2;
	}

	uint64_t old_state, new_state;

	os_atomic_rmw_loop2o(dwl, dq_state, old_state, new_state, release, {
		new_state  = _dq_state_merge_qos(old_state, qos);
		new_state -= DISPATCH_QUEUE_IN_BARRIER;
		new_state -= DISPATCH_QUEUE_WIDTH_INTERVAL;
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		if (target) {
			new_state |= DISPATCH_QUEUE_ENQUEUED;
		} else if (unlikely(_dq_state_is_dirty(old_state))) {
			os_atomic_rmw_loop_give_up({
				// just renew the drain lock with an acquire barrier, to see
				// what the enqueuer that set DIRTY has done.
				// the xor generates better assembly as DISPATCH_QUEUE_DIRTY
				// is already in a register
				os_atomic_xor2o(dwl, dq_state, DISPATCH_QUEUE_DIRTY, acquire);
				goto again;
			});
		} else if (likely(_dq_state_is_base_wlh(old_state))) {
			new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
			new_state &= ~DISPATCH_QUEUE_ENQUEUED;
		} else {
			new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
		}
	});
	dispatch_assert(_dq_state_drain_locked_by_self(old_state));
	dispatch_assert(!_dq_state_is_enqueued_on_manager(old_state));

	if (_dq_state_is_enqueued(new_state)) {
		_dispatch_trace_runtime_event(sync_async_handoff, dwl, 0);
	}

#if DISPATCH_USE_KEVENT_WORKLOOP
	if (_dq_state_is_base_wlh(old_state)) {
		// - Only non-"du_is_direct" sources & mach channels can be enqueued
		//   on the manager.
		//
		// - Only dispatch_source_cancel_and_wait() and
		//   dispatch_source_set_*_handler() use the barrier complete codepath,
		//   none of which are used by mach channels.
		//
		// Hence no source-ish object can both be a workloop and need to use the
		// manager at the same time.
		dispatch_assert(!_dq_state_is_enqueued_on_manager(new_state));
		if (_dq_state_is_enqueued_on_target(old_state) ||
				_dq_state_is_enqueued_on_target(new_state) ||
				_dq_state_received_sync_wait(old_state) ||
				_dq_state_in_sync_transfer(old_state)) {
			return _dispatch_event_loop_end_ownership((dispatch_wlh_t)dwl,
					old_state, new_state, flags);
		}
		_dispatch_event_loop_assert_not_owned((dispatch_wlh_t)dwl);
		goto done;
	}
#endif

	if (_dq_state_received_override(old_state)) {
		// Ensure that the root queue sees that this thread was overridden.
		_dispatch_set_basepri_override_qos(_dq_state_max_qos(old_state));
	}

	if (target) {
		if (likely((old_state ^ new_state) & DISPATCH_QUEUE_ENQUEUED)) {
			dispatch_assert(_dq_state_is_enqueued(new_state));
			dispatch_assert(flags & DISPATCH_WAKEUP_CONSUME_2);
			return _dispatch_queue_push_queue(dwl->do_targetq, dwl, new_state);
		}
#if HAVE_PTHREAD_WORKQUEUE_QOS
		// <rdar://problem/27694093> when doing sync to async handoff
		// if the queue received an override we have to forecefully redrive
		// the same override so that a new stealer is enqueued because
		// the previous one may be gone already
		if (_dq_state_should_override(new_state)) {
			return _dispatch_queue_wakeup_with_override(dwl, new_state, flags);
		}
#endif
	}

#if DISPATCH_USE_KEVENT_WORKLOOP
done:
#endif
	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		return _dispatch_release_2_tailcall(dwl);
	}
}

#if HAVE_PTHREAD_WORKQUEUE_QOS
static void
_dispatch_workloop_stealer_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags)
{
	uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_NO_INTROSPECTION;
	_dispatch_continuation_pop_forwarded(dc, dc_flags, NULL, {
		dispatch_queue_t dq = dc->dc_data;
		dx_invoke(dq, dic, flags | DISPATCH_INVOKE_STEALING);
	});
}

DISPATCH_NOINLINE
static void
_dispatch_workloop_push_stealer(dispatch_workloop_t dwl, dispatch_queue_t dq,
		dispatch_qos_t qos)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();

	dc->do_vtable = DC_VTABLE(WORKLOOP_STEALING);
	_dispatch_retain_2(dq);
	dc->dc_func = NULL;
	dc->dc_ctxt = dc;
	dc->dc_other = NULL;
	dc->dc_data = dq;
	dc->dc_priority = DISPATCH_NO_PRIORITY;
	dc->dc_voucher = DISPATCH_NO_VOUCHER;
	_dispatch_workloop_push(dwl, dc, qos);
}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS

void
_dispatch_workloop_wakeup(dispatch_workloop_t dwl, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	if (unlikely(flags & DISPATCH_WAKEUP_BARRIER_COMPLETE)) {
		return _dispatch_workloop_barrier_complete(dwl, qos, flags);
	}

	if (unlikely(!(flags & DISPATCH_WAKEUP_CONSUME_2))) {
		DISPATCH_INTERNAL_CRASH(flags, "Invalid way to wake up a workloop");
	}

	if (unlikely(flags & DISPATCH_WAKEUP_BLOCK_WAIT)) {
		goto done;
	}

	uint64_t old_state, new_state;

	os_atomic_rmw_loop2o(dwl, dq_state, old_state, new_state, release, {
		new_state = _dq_state_merge_qos(old_state, qos);
		if (_dq_state_max_qos(new_state)) {
			// We need to make sure we have the enqueued bit when we are making
			// the syscall to update QoS and we know that we will do it since
			// we're at the base anyways
			new_state |= DISPATCH_QUEUE_ENQUEUED;
		}
		if (flags & DISPATCH_WAKEUP_MAKE_DIRTY) {
			new_state |= DISPATCH_QUEUE_DIRTY;
		} else if (new_state == old_state) {
			os_atomic_rmw_loop_give_up(goto done);
		}
	});

	if (unlikely(_dq_state_is_suspended(old_state))) {
#ifndef __LP64__
		old_state >>= 32;
#endif
		DISPATCH_CLIENT_CRASH(old_state, "Waking up an inactive workloop");
	}
	if (likely((old_state ^ new_state) & DISPATCH_QUEUE_ENQUEUED)) {
		return _dispatch_queue_push_queue(dwl->do_targetq, dwl, new_state);
	}
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (likely((old_state ^ new_state) & DISPATCH_QUEUE_MAX_QOS_MASK)) {
		return _dispatch_queue_wakeup_with_override(dwl, new_state, flags);
	}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS
done:
	return _dispatch_release_2_tailcall(dwl);
}

DISPATCH_NOINLINE
static void
_dispatch_workloop_push_waiter(dispatch_workloop_t dwl,
		dispatch_sync_context_t dsc, dispatch_qos_t qos)
{
	struct dispatch_object_s *prev, *dc = (struct dispatch_object_s *)dsc;

	dispatch_priority_t p = _dispatch_priority_from_pp(dsc->dc_priority);
	if (qos < _dispatch_priority_qos(p)) {
		qos = _dispatch_priority_qos(p);
	}
	if (qos == DISPATCH_QOS_UNSPECIFIED) {
		qos = DISPATCH_QOS_DEFAULT;
	}

	prev = _dispatch_workloop_push_update_tail(dwl, qos, dc);
	_dispatch_workloop_push_update_prev(dwl, qos, prev, dc);
	if (likely(!os_mpsc_push_was_empty(prev))) return;

	uint64_t set_owner_and_set_full_width_and_in_barrier =
			_dispatch_lock_value_for_self() |
			DISPATCH_QUEUE_WIDTH_FULL_BIT | DISPATCH_QUEUE_IN_BARRIER;
	uint64_t old_state, new_state;

	os_atomic_rmw_loop2o(dwl, dq_state, old_state, new_state, release, {
		new_state = _dq_state_merge_qos(old_state, qos);
		new_state |= DISPATCH_QUEUE_DIRTY;
		if (unlikely(_dq_state_drain_locked(old_state))) {
			// not runnable, so we should just handle overrides
		} else if (_dq_state_is_enqueued(old_state)) {
			// 32123779 let the event thread redrive since it's out already
		} else {
			// see _dispatch_queue_drain_try_lock
			new_state &= DISPATCH_QUEUE_DRAIN_PRESERVED_BITS_MASK;
			new_state |= set_owner_and_set_full_width_and_in_barrier;
		}
	});

	if ((dsc->dc_flags & DC_FLAG_ASYNC_AND_WAIT) &&
		_dispatch_async_and_wait_should_always_async(dwl, new_state)) {
		dsc->dc_other = dwl;
		dsc->dc_flags &= ~DC_FLAG_ASYNC_AND_WAIT;
	}

	dsc->dsc_wlh_was_first = (dsc->dsc_waiter == _dispatch_tid_self());

	if ((old_state ^ new_state) & DISPATCH_QUEUE_IN_BARRIER) {
		return _dispatch_workloop_barrier_complete(dwl, qos, 0);
	}
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (unlikely((old_state ^ new_state) & DISPATCH_QUEUE_MAX_QOS_MASK)) {
		if (_dq_state_should_override(new_state)) {
			return _dispatch_queue_wakeup_with_override(dwl, new_state, 0);
		}
	}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS
}

void
_dispatch_workloop_push(dispatch_workloop_t dwl, dispatch_object_t dou,
		dispatch_qos_t qos)
{
	struct dispatch_object_s *prev;

	if (unlikely(_dispatch_object_is_waiter(dou))) {
		return _dispatch_workloop_push_waiter(dwl, dou._dsc, qos);
	}

	if (qos < _dispatch_priority_qos(dwl->dq_priority)) {
		qos = _dispatch_priority_qos(dwl->dq_priority);
	}
	if (qos == DISPATCH_QOS_UNSPECIFIED) {
		qos = _dispatch_priority_fallback_qos(dwl->dq_priority);
	}
	prev = _dispatch_workloop_push_update_tail(dwl, qos, dou._do);
	if (unlikely(os_mpsc_push_was_empty(prev))) {
		_dispatch_retain_2_unsafe(dwl);
	}
	_dispatch_workloop_push_update_prev(dwl, qos, prev, dou._do);
	if (unlikely(os_mpsc_push_was_empty(prev))) {
		return _dispatch_workloop_wakeup(dwl, qos, DISPATCH_WAKEUP_CONSUME_2 |
				DISPATCH_WAKEUP_MAKE_DIRTY);
	}
}

#pragma mark -
#pragma mark dispatch queue/lane push & wakeup

#if HAVE_PTHREAD_WORKQUEUE_QOS
static void
_dispatch_queue_override_invoke(dispatch_continuation_t dc,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags)
{
	dispatch_queue_t old_rq = _dispatch_queue_get_current();
	dispatch_queue_global_t assumed_rq = dc->dc_other;
	dispatch_priority_t old_dp;
	dispatch_object_t dou;
	uintptr_t dc_flags = DC_FLAG_CONSUME;

	dou._do = dc->dc_data;
	old_dp = _dispatch_root_queue_identity_assume(assumed_rq);
	if (dc_type(dc) == DISPATCH_CONTINUATION_TYPE(OVERRIDE_STEALING)) {
		flags |= DISPATCH_INVOKE_STEALING;
		dc_flags |= DC_FLAG_NO_INTROSPECTION;
	}
	_dispatch_continuation_pop_forwarded(dc, dc_flags, assumed_rq, {
		if (_dispatch_object_has_vtable(dou._do)) {
			dx_invoke(dou._dq, dic, flags);
		} else {
			_dispatch_continuation_invoke_inline(dou, flags, assumed_rq);
		}
	});
	_dispatch_reset_basepri(old_dp);
	_dispatch_queue_set_current(old_rq);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_root_queue_push_needs_override(dispatch_queue_global_t rq,
		dispatch_qos_t qos)
{
	dispatch_qos_t fallback = _dispatch_priority_fallback_qos(rq->dq_priority);
	if (fallback) {
		return qos && qos != fallback;
	}

	dispatch_qos_t rqos = _dispatch_priority_qos(rq->dq_priority);
	return rqos && qos > rqos;
}

DISPATCH_NOINLINE
static void
_dispatch_root_queue_push_override(dispatch_queue_global_t orig_rq,
		dispatch_object_t dou, dispatch_qos_t qos)
{
	bool overcommit = orig_rq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
	dispatch_queue_global_t rq = _dispatch_get_root_queue(qos, overcommit);
	dispatch_continuation_t dc = dou._dc;

	if (_dispatch_object_is_redirection(dc)) {
		// no double-wrap is needed, _dispatch_async_redirect_invoke will do
		// the right thing
		dc->dc_func = (void *)orig_rq;
	} else {
		dc = _dispatch_continuation_alloc();
		dc->do_vtable = DC_VTABLE(OVERRIDE_OWNING);
		dc->dc_ctxt = dc;
		dc->dc_other = orig_rq;
		dc->dc_data = dou._do;
		dc->dc_priority = DISPATCH_NO_PRIORITY;
		dc->dc_voucher = DISPATCH_NO_VOUCHER;
	}
	_dispatch_root_queue_push_inline(rq, dc, dc, 1);
}

DISPATCH_NOINLINE
static void
_dispatch_root_queue_push_override_stealer(dispatch_queue_global_t orig_rq,
		dispatch_queue_t dq, dispatch_qos_t qos)
{
	bool overcommit = orig_rq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
	dispatch_queue_global_t rq = _dispatch_get_root_queue(qos, overcommit);
	dispatch_continuation_t dc = _dispatch_continuation_alloc();

	dc->do_vtable = DC_VTABLE(OVERRIDE_STEALING);
	_dispatch_retain_2(dq);
	dc->dc_func = NULL;
	dc->dc_ctxt = dc;
	dc->dc_other = orig_rq;
	dc->dc_data = dq;
	dc->dc_priority = DISPATCH_NO_PRIORITY;
	dc->dc_voucher = DISPATCH_NO_VOUCHER;
	_dispatch_root_queue_push_inline(rq, dc, dc, 1);
}

DISPATCH_NOINLINE
static void
_dispatch_queue_wakeup_with_override_slow(dispatch_queue_t dq,
		uint64_t dq_state, dispatch_wakeup_flags_t flags)
{
	dispatch_qos_t oqos, qos = _dq_state_max_qos(dq_state);
	dispatch_queue_t tq = dq->do_targetq;
	mach_port_t owner;
	bool locked;

	if (_dq_state_is_base_anon(dq_state)) {
		if (!_dispatch_is_in_root_queues_array(tq)) {
			// <rdar://problem/40320044> Do not try to override pthread root
			// queues, it isn't supported and can cause things to run
			// on the wrong hierarchy if we enqueue a stealer by accident
			goto out;
		} else if ((owner = _dq_state_drain_owner(dq_state))) {
			(void)_dispatch_wqthread_override_start_check_owner(owner, qos,
					&dq->dq_state_lock);
			goto out;
		}

		// avoid locking when we recognize the target queue as a global root
		// queue it is gross, but is a very common case. The locking isn't
		// needed because these target queues cannot go away.
		locked = false;
	} else if (likely(!_dispatch_queue_is_mutable(dq))) {
		locked = false;
	} else if (_dispatch_queue_sidelock_trylock(upcast(dq)._dl, qos)) {
		// <rdar://problem/17735825> to traverse the tq chain safely we must
		// lock it to ensure it cannot change
		locked = true;
		tq = dq->do_targetq;
		_dispatch_ktrace1(DISPATCH_PERF_mutable_target, dq);
	} else {
		//
		// Leading to being there, the current thread has:
		// 1. enqueued an object on `dq`
		// 2. raised the max_qos value, set RECEIVED_OVERRIDE on `dq`
		//    and didn't see an owner
		// 3. tried and failed to acquire the side lock
		//
		// The side lock owner can only be one of three things:
		//
		// - The suspend/resume side count code. Besides being unlikely,
		//   it means that at this moment the queue is actually suspended,
		//   which transfers the responsibility of applying the override to
		//   the eventual dispatch_resume().
		//
		// - A dispatch_set_target_queue() call. The fact that we saw no `owner`
		//   means that the trysync it does wasn't being drained when (2)
		//   happened which can only be explained by one of these interleavings:
		//
		//    o `dq` became idle between when the object queued in (1) ran and
		//      the set_target_queue call and we were unlucky enough that our
		//      step (2) happened while this queue was idle. There is no reason
		//		to override anything anymore, the queue drained to completion
		//      while we were preempted, our job is done.
		//
		//    o `dq` is queued but not draining during (1-2), then when we try
		//      to lock at (3) the queue is now draining a set_target_queue.
		//      This drainer must have seen the effects of (2) and that guy has
		//      applied our override. Our job is done.
		//
		// - Another instance of _dispatch_queue_wakeup_with_override_slow(),
		//   which is fine because trylock leaves a hint that we failed our
		//   trylock, causing the tryunlock below to fail and reassess whether
		//   a better override needs to be applied.
		//
		_dispatch_ktrace1(DISPATCH_PERF_mutable_target, dq);
		goto out;
	}

apply_again:
	if (dx_hastypeflag(tq, QUEUE_ROOT)) {
		dispatch_queue_global_t rq = upcast(tq)._dgq;
		if (qos > _dispatch_priority_qos(rq->dq_priority)) {
			_dispatch_root_queue_push_override_stealer(rq, dq, qos);
		}
	} else if (dx_metatype(tq) == _DISPATCH_WORKLOOP_TYPE) {
		_dispatch_workloop_push_stealer(upcast(tq)._dwl, dq, qos);
	} else if (_dispatch_queue_need_override(tq, qos)) {
		dx_wakeup(tq, qos, 0);
	}
	if (likely(!locked)) {
		goto out;
	}
	while (unlikely(!_dispatch_queue_sidelock_tryunlock(upcast(dq)._dl))) {
		// rdar://problem/24081326
		//
		// Another instance of _dispatch_queue_wakeup_with_override() tried
		// to acquire the side lock while we were running, and could have
		// had a better override than ours to apply.
		//
		oqos = _dispatch_queue_max_qos(dq);
		if (oqos > qos) {
			qos = oqos;
			// The other instance had a better priority than ours, override
			// our thread, and apply the override that wasn't applied to `dq`
			// because of us.
			goto apply_again;
		}
	}

out:
	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		return _dispatch_release_2_tailcall(dq);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_wakeup_with_override(dispatch_queue_class_t dq,
		uint64_t dq_state, dispatch_wakeup_flags_t flags)
{
	dispatch_assert(_dq_state_should_override(dq_state));

#if DISPATCH_USE_KEVENT_WORKLOOP
	if (likely(_dq_state_is_base_wlh(dq_state))) {
		_dispatch_trace_runtime_event(worker_request, dq._dq, 1);
		return _dispatch_event_loop_poke((dispatch_wlh_t)dq._dq, dq_state,
				flags | DISPATCH_EVENT_LOOP_OVERRIDE);
	}
#endif // DISPATCH_USE_KEVENT_WORKLOOP
	return _dispatch_queue_wakeup_with_override_slow(dq._dq, dq_state, flags);
}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS

DISPATCH_NOINLINE
void
_dispatch_queue_wakeup(dispatch_queue_class_t dqu, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags, dispatch_queue_wakeup_target_t target)
{
	dispatch_queue_t dq = dqu._dq;
	dispatch_assert(target != DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT);

	if (target && !(flags & DISPATCH_WAKEUP_CONSUME_2)) {
		_dispatch_retain_2(dq);
		flags |= DISPATCH_WAKEUP_CONSUME_2;
	}

	if (unlikely(flags & DISPATCH_WAKEUP_BARRIER_COMPLETE)) {
		//
		// _dispatch_lane_class_barrier_complete() is about what both regular
		// queues and sources needs to evaluate, but the former can have sync
		// handoffs to perform which _dispatch_lane_class_barrier_complete()
		// doesn't handle, only _dispatch_lane_barrier_complete() does.
		//
		// _dispatch_lane_wakeup() is the one for plain queues that calls
		// _dispatch_lane_barrier_complete(), and this is only taken for non
		// queue types.
		//
		dispatch_assert(dx_metatype(dq) == _DISPATCH_SOURCE_TYPE);
		qos = _dispatch_queue_wakeup_qos(dq, qos);
		return _dispatch_lane_class_barrier_complete(upcast(dq)._dl, qos,
				flags, target, DISPATCH_QUEUE_SERIAL_DRAIN_OWNED);
	}

	if (target) {
		uint64_t old_state, new_state, enqueue = DISPATCH_QUEUE_ENQUEUED;
		if (target == DISPATCH_QUEUE_WAKEUP_MGR) {
			enqueue = DISPATCH_QUEUE_ENQUEUED_ON_MGR;
		}
		qos = _dispatch_queue_wakeup_qos(dq, qos);
		os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
			new_state = _dq_state_merge_qos(old_state, qos);
			if (flags & DISPATCH_WAKEUP_CLEAR_ACTIVATING) {
				// When an event is being delivered to a source because its
				// unote was being registered before the ACTIVATING state
				// had a chance to be cleared, we don't want to fail the wakeup
				// which could lead to a priority inversion.
				//
				// Instead, these wakeups are allowed to finish the pending
				// activation.
				if (_dq_state_is_activating(old_state)) {
					new_state &= ~DISPATCH_QUEUE_ACTIVATING;
				}
			}
			if (likely(!_dq_state_is_suspended(new_state) &&
					!_dq_state_is_enqueued(old_state) &&
					(!_dq_state_drain_locked(old_state) ||
					enqueue != DISPATCH_QUEUE_ENQUEUED_ON_MGR))) {
				// Always set the enqueued bit for async enqueues on all queues
				// in the hierachy
				new_state |= enqueue;
			}
			if (flags & DISPATCH_WAKEUP_MAKE_DIRTY) {
				new_state |= DISPATCH_QUEUE_DIRTY;
			} else if (new_state == old_state) {
				os_atomic_rmw_loop_give_up(goto done);
			}
		});

		if (likely((old_state ^ new_state) & enqueue)) {
			dispatch_queue_t tq;
			if (target == DISPATCH_QUEUE_WAKEUP_TARGET) {
				// the rmw_loop above has no acquire barrier, as the last block
				// of a queue asyncing to that queue is not an uncommon pattern
				// and in that case the acquire would be completely useless
				//
				// so instead use depdendency ordering to read
				// the targetq pointer.
				os_atomic_thread_fence(dependency);
				tq = os_atomic_load_with_dependency_on2o(dq, do_targetq,
						(long)new_state);
			} else {
				tq = target;
			}
			dispatch_assert(_dq_state_is_enqueued(new_state));
			return _dispatch_queue_push_queue(tq, dq, new_state);
		}
#if HAVE_PTHREAD_WORKQUEUE_QOS
		if (unlikely((old_state ^ new_state) & DISPATCH_QUEUE_MAX_QOS_MASK)) {
			if (_dq_state_should_override(new_state)) {
				return _dispatch_queue_wakeup_with_override(dq, new_state,
						flags);
			}
		}
	} else if (qos) {
		//
		// Someone is trying to override the last work item of the queue.
		//
		uint64_t old_state, new_state;
		os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
			// Avoid spurious override if the item was drained before we could
			// apply an override
			if (!_dq_state_drain_locked(old_state) &&
				!_dq_state_is_enqueued(old_state)) {
				os_atomic_rmw_loop_give_up(goto done);
			}
			new_state = _dq_state_merge_qos(old_state, qos);
			if (new_state == old_state) {
				os_atomic_rmw_loop_give_up(goto done);
			}
		});
		if (_dq_state_should_override(new_state)) {
			return _dispatch_queue_wakeup_with_override(dq, new_state, flags);
		}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS
	}
done:
	if (likely(flags & DISPATCH_WAKEUP_CONSUME_2)) {
		return _dispatch_release_2_tailcall(dq);
	}
}

DISPATCH_NOINLINE
void
_dispatch_lane_wakeup(dispatch_lane_class_t dqu, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	dispatch_queue_wakeup_target_t target = DISPATCH_QUEUE_WAKEUP_NONE;

	if (unlikely(flags & DISPATCH_WAKEUP_BARRIER_COMPLETE)) {
		return _dispatch_lane_barrier_complete(dqu, qos, flags);
	}
	if (_dispatch_queue_class_probe(dqu)) {
		target = DISPATCH_QUEUE_WAKEUP_TARGET;
	}
	return _dispatch_queue_wakeup(dqu, qos, flags, target);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_lane_push_waiter_should_wakeup(dispatch_lane_t dq,
		dispatch_sync_context_t dsc)
{
	if (_dispatch_queue_is_thread_bound(dq)) {
		return true;
	}
	if (dsc->dc_flags & DC_FLAG_ASYNC_AND_WAIT) {
		uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
		return _dispatch_async_and_wait_should_always_async(dq, dq_state);
	}
	return false;
}

DISPATCH_NOINLINE
static void
_dispatch_lane_push_waiter(dispatch_lane_t dq, dispatch_sync_context_t dsc,
		dispatch_qos_t qos)
{
	uint64_t old_state, new_state;

	if (dsc->dc_data != DISPATCH_WLH_ANON) {
		// The kernel will handle all the overrides / priorities on our behalf.
		qos = 0;
	}

	if (unlikely(_dispatch_queue_push_item(dq, dsc))) {
		if (unlikely(_dispatch_lane_push_waiter_should_wakeup(dq, dsc))) {
			// If this returns true, we know that we are pushing onto the base
			// queue
			dsc->dc_flags &= ~DC_FLAG_ASYNC_AND_WAIT;
			dsc->dc_other = dq;
			return dx_wakeup(dq, qos, DISPATCH_WAKEUP_MAKE_DIRTY);
		}

		uint64_t pending_barrier_width =
				(dq->dq_width - 1) * DISPATCH_QUEUE_WIDTH_INTERVAL;
		uint64_t set_owner_and_set_full_width_and_in_barrier =
				_dispatch_lock_value_for_self() |
				DISPATCH_QUEUE_WIDTH_FULL_BIT | DISPATCH_QUEUE_IN_BARRIER;
		os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
			new_state  = _dq_state_merge_qos(old_state, qos);
			new_state |= DISPATCH_QUEUE_DIRTY;
			if (unlikely(_dq_state_drain_locked(old_state) ||
					!_dq_state_is_runnable(old_state))) {
				// not runnable, so we should just handle overrides
			} else if (_dq_state_is_base_wlh(old_state) &&
					_dq_state_is_enqueued(old_state)) {
				// 32123779 let the event thread redrive since it's out already
			} else if (_dq_state_has_pending_barrier(old_state) ||
					new_state + pending_barrier_width <
					DISPATCH_QUEUE_WIDTH_FULL_BIT) {
				// see _dispatch_queue_drain_try_lock
				new_state &= DISPATCH_QUEUE_DRAIN_PRESERVED_BITS_MASK;
				new_state |= set_owner_and_set_full_width_and_in_barrier;
			}
		});

		if (_dq_state_is_base_wlh(old_state)) {
			dsc->dsc_wlh_was_first = (dsc->dsc_waiter == _dispatch_tid_self());
		}

		if ((old_state ^ new_state) & DISPATCH_QUEUE_IN_BARRIER) {
			return _dispatch_lane_barrier_complete(dq, qos, 0);
		}
#if HAVE_PTHREAD_WORKQUEUE_QOS
		if (unlikely((old_state ^ new_state) & DISPATCH_QUEUE_MAX_QOS_MASK)) {
			if (_dq_state_should_override(new_state)) {
				return _dispatch_queue_wakeup_with_override(dq, new_state, 0);
			}
		}
	} else if (unlikely(qos)) {
		os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
			new_state = _dq_state_merge_qos(old_state, qos);
			if (old_state == new_state) {
				os_atomic_rmw_loop_give_up(return);
			}
		});
		if (_dq_state_should_override(new_state)) {
			return _dispatch_queue_wakeup_with_override(dq, new_state, 0);
		}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS
	}
}

DISPATCH_NOINLINE
void
_dispatch_lane_push(dispatch_lane_t dq, dispatch_object_t dou,
		dispatch_qos_t qos)
{
	dispatch_wakeup_flags_t flags = 0;
	struct dispatch_object_s *prev;

	if (unlikely(_dispatch_object_is_waiter(dou))) {
		return _dispatch_lane_push_waiter(dq, dou._dsc, qos);
	}

	dispatch_assert(!_dispatch_object_is_global(dq));
	qos = _dispatch_queue_push_qos(dq, qos);

	// If we are going to call dx_wakeup(), the queue must be retained before
	// the item we're pushing can be dequeued, which means:
	// - before we exchange the tail if we have to override
	// - before we set the head if we made the queue non empty.
	// Otherwise, if preempted between one of these and the call to dx_wakeup()
	// the blocks submitted to the queue may release the last reference to the
	// queue when invoked by _dispatch_lane_drain. <rdar://problem/6932776>

	prev = os_mpsc_push_update_tail(os_mpsc(dq, dq_items), dou._do, do_next);
	if (unlikely(os_mpsc_push_was_empty(prev))) {
		_dispatch_retain_2_unsafe(dq);
		flags = DISPATCH_WAKEUP_CONSUME_2 | DISPATCH_WAKEUP_MAKE_DIRTY;
	} else if (unlikely(_dispatch_queue_need_override(dq, qos))) {
		// There's a race here, _dispatch_queue_need_override may read a stale
		// dq_state value.
		//
		// If it's a stale load from the same drain streak, given that
		// the max qos is monotonic, too old a read can only cause an
		// unnecessary attempt at overriding which is harmless.
		//
		// We'll assume here that a stale load from an a previous drain streak
		// never happens in practice.
		_dispatch_retain_2_unsafe(dq);
		flags = DISPATCH_WAKEUP_CONSUME_2;
	}
	os_mpsc_push_update_prev(os_mpsc(dq, dq_items), prev, dou._do, do_next);
	if (flags) {
		return dx_wakeup(dq, qos, flags);
	}
}

DISPATCH_NOINLINE
void
_dispatch_lane_concurrent_push(dispatch_lane_t dq, dispatch_object_t dou,
		dispatch_qos_t qos)
{
	// <rdar://problem/24738102&24743140> reserving non barrier width
	// doesn't fail if only the ENQUEUED bit is set (unlike its barrier
	// width equivalent), so we have to check that this thread hasn't
	// enqueued anything ahead of this call or we can break ordering
	if (dq->dq_items_tail == NULL &&
			!_dispatch_object_is_waiter(dou) &&
			!_dispatch_object_is_barrier(dou) &&
			_dispatch_queue_try_acquire_async(dq)) {
		return _dispatch_continuation_redirect_push(dq, dou, qos);
	}

	_dispatch_lane_push(dq, dou, qos);
}

#pragma mark -
#pragma mark dispatch_channel_t

void
_dispatch_channel_dispose(dispatch_channel_t dch, bool *allow_free)
{
	dch->dch_callbacks = NULL;
	_dispatch_lane_class_dispose(dch, allow_free);
}

void
_dispatch_channel_xref_dispose(dispatch_channel_t dch)
{
	dispatch_channel_callbacks_t callbacks = dch->dch_callbacks;
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(dch->_as_dq);
	if (callbacks->dcc_acknowledge_cancel && !(dqf & DSF_CANCELED)) {
		DISPATCH_CLIENT_CRASH(dch, "Release of a channel that has not been "
				"cancelled, but has a cancel acknowledgement callback");
	}
	dx_wakeup(dch, 0, DISPATCH_WAKEUP_MAKE_DIRTY);
}

typedef struct dispatch_channel_invoke_ctxt_s {
	dispatch_channel_t dcic_dch;
	dispatch_thread_frame_s dcic_dtf;
	dispatch_invoke_context_t dcic_dic;
	dispatch_invoke_flags_t dcic_flags;
	dispatch_queue_wakeup_target_t dcic_tq;
	struct dispatch_object_s *dcic_next_dc;
	bool dcic_called_drain;
} dispatch_channel_invoke_ctxt_s;

static bool
_dispatch_channel_invoke_cancel_check(dispatch_channel_t dch,
		dispatch_channel_invoke_ctxt_t ctxt,
		dispatch_channel_callbacks_t callbacks)
{
	bool rc = true;
	if (!dch->dm_cancel_handler_called) {
		if (_dispatch_queue_atomic_flags(dch) & DSF_CANCELED) {
			dispatch_invoke_with_autoreleasepool(ctxt->dcic_flags, {
				rc = callbacks->dcc_acknowledge_cancel(dch, dch->do_ctxt);
			});
			if (rc) {
				dch->dm_cancel_handler_called = true;
				_dispatch_release_no_dispose(dch);
			} else {
				ctxt->dcic_tq = DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT;
			}
		}
	}
	return rc;
}

static bool
_dispatch_channel_invoke_checks(dispatch_channel_t dch,
		dispatch_channel_invoke_ctxt_t dcic,
		dispatch_channel_callbacks_t callbacks)
{
	if (!_dispatch_channel_invoke_cancel_check(dch, dcic, callbacks)) {
		return false;
	}
	if (unlikely(_dispatch_needs_to_return_to_kernel())) {
		_dispatch_return_to_kernel();
	}
	if (likely(dcic->dcic_flags & DISPATCH_INVOKE_WORKLOOP_DRAIN)) {
		dispatch_workloop_t dwl = (dispatch_workloop_t)_dispatch_get_wlh();
		if (unlikely(_dispatch_queue_max_qos(dwl) > dwl->dwl_drained_qos)) {
			dcic->dcic_tq = dch->do_targetq;
			return false;
		}
	}
	if (unlikely(_dispatch_queue_drain_should_narrow(dcic->dcic_dic))) {
		dcic->dcic_tq = dch->do_targetq;
		return false;
	}
	uint64_t dq_state = os_atomic_load(&dch->dq_state, relaxed);
	if (unlikely(_dq_state_is_suspended(dq_state))) {
		dcic->dcic_tq = dch->do_targetq;
		return false;
	}
	return true;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_wakeup_target_t
_dispatch_channel_invoke2(dispatch_channel_t dch, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags, uint64_t *owned DISPATCH_UNUSED)
{
	dispatch_channel_callbacks_t callbacks = dch->dch_callbacks;
	dispatch_channel_invoke_ctxt_s dcic = {
		.dcic_dch = dch,
		.dcic_dic = dic,
		.dcic_flags = flags &
				~(dispatch_invoke_flags_t)DISPATCH_INVOKE_REDIRECTING_DRAIN,
		.dcic_tq = DISPATCH_QUEUE_WAKEUP_NONE,
	};

	_dispatch_thread_frame_push(&dcic.dcic_dtf, dch);

	if (!_dispatch_channel_invoke_cancel_check(dch, &dcic, callbacks)) {
		goto out;
	}

	do {
		struct dispatch_object_s *dc = dcic.dcic_next_dc;

		if (unlikely(!dc)) {
			if (!dch->dq_items_tail) {
				break;
			}
			dc = _dispatch_queue_get_head(dch);
		}

		if (unlikely(_dispatch_object_is_sync_waiter(dc))) {
			DISPATCH_CLIENT_CRASH(0, "sync waiter found on channel");
		}

		if (_dispatch_object_is_channel_item(dc)) {
			dcic.dcic_next_dc = dc;
			dcic.dcic_called_drain = false;
			dispatch_invoke_with_autoreleasepool(dcic.dcic_flags, {
				if (callbacks->dcc_invoke(dch, &dcic, dch->do_ctxt)) {
					if (unlikely(!dcic.dcic_called_drain)) {
						DISPATCH_CLIENT_CRASH(0, "Channel didn't call "
								"dispatch_channel_drain");
					}
				} else {
					dcic.dcic_tq = DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT;
				}
			});
		} else {
			dcic.dcic_next_dc = _dispatch_queue_pop_head(dch, dc);
			_dispatch_continuation_pop_inline(dc, dic, flags, dch);
			if (!_dispatch_channel_invoke_checks(dch, &dcic, callbacks)) {
				break;
			}
		}
	} while (dcic.dcic_tq == DISPATCH_QUEUE_WAKEUP_NONE);

out:
	_dispatch_thread_frame_pop(&dcic.dcic_dtf);
	return dcic.dcic_tq;
}

void
_dispatch_channel_invoke(dispatch_channel_t dch,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags)
{
	_dispatch_queue_class_invoke(dch, dic, flags,
		DISPATCH_INVOKE_DISALLOW_SYNC_WAITERS, _dispatch_channel_invoke2);
}

void
dispatch_channel_foreach_work_item_peek_f(
		dispatch_channel_invoke_ctxt_t dcic,
		void *ctxt, dispatch_channel_enumerator_handler_t f)
{
	if (dcic->dcic_called_drain) {
		DISPATCH_CLIENT_CRASH(0, "Called peek after drain");
	}

	dispatch_channel_t dch = dcic->dcic_dch;
	struct dispatch_object_s *dc = dcic->dcic_next_dc;

	for (;;) {
		dispatch_continuation_t dci = (dispatch_continuation_t)dc;
		if (!_dispatch_object_is_channel_item(dc)) {
			break;
		}
		if (!f(ctxt, dci->dc_ctxt)) {
			break;
		}
		if (dc == dch->dq_items_tail) {
			break;
		}
		dc = os_mpsc_get_next(dc, do_next);
	}
}

void
dispatch_channel_drain_f(dispatch_channel_invoke_ctxt_t dcic,
		void *_Nullable ctxt, dispatch_channel_drain_handler_t f)
{
	dispatch_channel_t dch = dcic->dcic_dch;
	dispatch_channel_callbacks_t callbacks = dch->dch_callbacks;
	struct dispatch_object_s *dc;
	uintptr_t dcf = DC_FLAG_CONSUME | DC_FLAG_CHANNEL_ITEM;
	void *unpop_item = NULL;
	bool stop_invoke = false;

	if (dcic->dcic_called_drain) {
		DISPATCH_CLIENT_CRASH(0, "Called drain twice in the same invoke");
	}
	dcic->dcic_called_drain = true;

	do {
		dc = dcic->dcic_next_dc;
		if (unlikely(!dc)) {
			if (!dch->dq_items_tail) {
				break;
			}
			dc = _dispatch_queue_get_head(dch);
		}
		if (!_dispatch_object_is_channel_item(dc)) {
			break;
		}

		dcic->dcic_next_dc = _dispatch_queue_pop_head(dch, dc);

		_dispatch_continuation_pop_forwarded(upcast(dc)._dc, dcf, dch, {
			dispatch_invoke_with_autoreleasepool(dcic->dcic_flags, {
				stop_invoke = !f(ctxt, upcast(dc)._dc->dc_ctxt, &unpop_item);
			});
		});
		if (unlikely(stop_invoke)) {
			break;
		}
	} while (_dispatch_channel_invoke_checks(dch, dcic, callbacks));

	if (unlikely(unpop_item)) {
		dispatch_continuation_t dci = _dispatch_continuation_alloc();
		_dispatch_continuation_init_f(dci, dch, unpop_item, NULL, 0, dcf);
		os_mpsc_undo_pop_head(os_mpsc(dch, dq_items), upcast(dci)._do,
				dcic->dcic_next_dc, do_next);
		dcic->dcic_next_dc = upcast(dci)._do;
	}
}

#ifdef __BLOCKS__
void
dispatch_channel_foreach_work_item_peek(
		dispatch_channel_invoke_ctxt_t dcic,
		dispatch_channel_enumerator_block_t block)
{
	dispatch_channel_enumerator_handler_t f;
	f = (dispatch_channel_enumerator_handler_t)_dispatch_Block_invoke(block);
	dispatch_channel_foreach_work_item_peek_f(dcic, block, f);
}

void
dispatch_channel_drain(dispatch_channel_invoke_ctxt_t dcic,
		dispatch_channel_drain_block_t block)
{
	dispatch_channel_drain_handler_t f;
	f = (dispatch_channel_drain_handler_t)_dispatch_Block_invoke(block);
	dispatch_channel_drain_f(dcic, block, f);
}
#endif // __BLOCKS__

DISPATCH_NOINLINE
void
_dispatch_channel_wakeup(dispatch_channel_t dch, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	dispatch_channel_callbacks_t callbacks = dch->dch_callbacks;
	dispatch_queue_wakeup_target_t target = DISPATCH_QUEUE_WAKEUP_NONE;
	dispatch_queue_t dq = dch->_as_dq;

	if (unlikely(!callbacks->dcc_probe(dch, dch->do_ctxt))) {
		target = DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT;
	} else if (_dispatch_queue_class_probe(dch)) {
		target = DISPATCH_QUEUE_WAKEUP_TARGET;
	} else if (_dispatch_queue_atomic_flags(dq) & DSF_CANCELED) {
		if (!dch->dm_cancel_handler_called) {
			target = DISPATCH_QUEUE_WAKEUP_TARGET;
		}
	}

	return _dispatch_queue_wakeup(dch, qos, flags, target);
}

size_t
_dispatch_channel_debug(dispatch_channel_t dch, char *buf, size_t bufsiz)
{
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(dch);
	size_t offset = 0;

	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			_dispatch_object_class_name(dch), dch);
	offset += _dispatch_object_debug_attr(dch, &buf[offset], bufsiz - offset);
	offset += _dispatch_queue_debug_attr(dch->_as_dq, &buf[offset], bufsiz - offset);
	offset += dsnprintf(buf, bufsiz, "%s%s%s",
			(dqf & DSF_CANCELED) ? "cancelled, " : "",
			(dqf & DSF_NEEDS_EVENT) ? "needs-event, " : "",
			(dqf & DSF_DELETED) ? "deleted, " : "");

	return offset;
}

dispatch_channel_t
dispatch_channel_create(const char *label, dispatch_queue_t tq,
		void *ctxt, dispatch_channel_callbacks_t callbacks)
{
	dispatch_channel_t dch;
	dispatch_queue_flags_t dqf = DSF_STRICT;

	if (callbacks->dcc_version < 1) {
		DISPATCH_CLIENT_CRASH(callbacks->dcc_version,
				"Unsupported callbacks version");
	}

	if (label) {
		const char *tmp = _dispatch_strdup_if_mutable(label);
		if (tmp != label) {
			dqf |= DQF_LABEL_NEEDS_FREE;
			label = tmp;
		}
	}

	if (unlikely(!tq)) {
		tq = _dispatch_get_default_queue(true);
	} else {
		_dispatch_retain((dispatch_queue_t _Nonnull)tq);
	}

	dch = _dispatch_queue_alloc(channel, dqf, 1,
			DISPATCH_QUEUE_INACTIVE | DISPATCH_QUEUE_ROLE_INNER)._dch;
	dch->dq_label = label;
	dch->do_targetq = tq;
	dch->dch_callbacks = callbacks;
	dch->do_ctxt = ctxt;
	if (!callbacks->dcc_acknowledge_cancel) {
		dch->dm_cancel_handler_called = true;
		dch->do_ref_cnt--;
	}
	return dch;
}

DISPATCH_NOINLINE
static void
_dispatch_channel_enqueue_slow(dispatch_channel_t dch, void *ctxt)
{
	uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_CHANNEL_ITEM;
	dispatch_continuation_t dc = _dispatch_continuation_alloc_from_heap();
	dispatch_qos_t qos;

	qos = _dispatch_continuation_init_f(dc, dch, ctxt, NULL, 0, dc_flags);
	_dispatch_continuation_async(dch, dc, qos, dc->dc_flags);
}

DISPATCH_NOINLINE
void
dispatch_channel_enqueue(dispatch_channel_t dch, void *ctxt)
{
	uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_CHANNEL_ITEM;
	dispatch_continuation_t dc = _dispatch_continuation_alloc_cacheonly();
	dispatch_qos_t qos;

	if (unlikely(!dc)) {
		return _dispatch_channel_enqueue_slow(dch, ctxt);
	}
	qos = _dispatch_continuation_init_f(dc, dch, ctxt, NULL, 0, dc_flags);
	_dispatch_continuation_async(dch, dc, qos, dc->dc_flags);
}

#ifndef __APPLE__
#if __BLOCKS__
void typeof(dispatch_channel_async) dispatch_channel_async
		__attribute__((__alias__("dispatch_async")));
#endif

void typeof(dispatch_channel_async_f) dispatch_channel_async_f
		__attribute__((__alias__("dispatch_async_f")));
#endif

void
dispatch_channel_wakeup(dispatch_channel_t dch, qos_class_t qos_class)
{
	dispatch_qos_t oqos = _dispatch_qos_from_qos_class(qos_class);
	dx_wakeup(dch, oqos, DISPATCH_WAKEUP_MAKE_DIRTY);
}

#pragma mark -
#pragma mark dispatch_mgr_queue

#if DISPATCH_USE_PTHREAD_ROOT_QUEUES || DISPATCH_USE_KEVENT_WORKQUEUE
struct _dispatch_mgr_sched_s {
	volatile int prio;
	volatile qos_class_t qos;
	int default_prio;
	int policy;
#if defined(_WIN32)
	HANDLE hThread;
#else
	pthread_t tid;
#endif
};

DISPATCH_STATIC_GLOBAL(struct _dispatch_mgr_sched_s _dispatch_mgr_sched);
DISPATCH_STATIC_GLOBAL(dispatch_once_t _dispatch_mgr_sched_pred);

#if HAVE_PTHREAD_WORKQUEUE_QOS
// TODO: switch to "event-reflector thread" property <rdar://problem/18126138>
// Must be kept in sync with list of qos classes in sys/qos.h
static int
_dispatch_mgr_sched_qos2prio(qos_class_t qos)
{
	if (qos == QOS_CLASS_MAINTENANCE) return 4;
	switch (qos) {
	case QOS_CLASS_BACKGROUND: return 4;
	case QOS_CLASS_UTILITY: return 20;
	case QOS_CLASS_DEFAULT: return 31;
	case QOS_CLASS_USER_INITIATED: return 37;
	case QOS_CLASS_USER_INTERACTIVE: return 47;
	default: return 0;
	}
}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS

static void
_dispatch_mgr_sched_init(void *ctxt DISPATCH_UNUSED)
{
#if !defined(_WIN32)
	struct sched_param param;
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
	dispatch_pthread_root_queue_context_t pqc = _dispatch_mgr_root_queue.do_ctxt;
	pthread_attr_t *attr = &pqc->dpq_thread_attr;
#else
	pthread_attr_t a, *attr = &a;
#endif
	(void)dispatch_assume_zero(pthread_attr_init(attr));
	(void)dispatch_assume_zero(pthread_attr_getschedpolicy(attr,
			&_dispatch_mgr_sched.policy));
	(void)dispatch_assume_zero(pthread_attr_getschedparam(attr, &param));
#if HAVE_PTHREAD_WORKQUEUE_QOS
	qos_class_t qos = qos_class_main();
	if (qos == QOS_CLASS_DEFAULT) {
		qos = QOS_CLASS_USER_INITIATED; // rdar://problem/17279292
	}
	if (qos) {
		_dispatch_mgr_sched.qos = qos;
		param.sched_priority = _dispatch_mgr_sched_qos2prio(qos);
	}
#endif
	_dispatch_mgr_sched.default_prio = param.sched_priority;
#else // defined(_WIN32)
	_dispatch_mgr_sched.policy = 0;
	_dispatch_mgr_sched.default_prio = THREAD_PRIORITY_NORMAL;
#endif // defined(_WIN32)
	_dispatch_mgr_sched.prio = _dispatch_mgr_sched.default_prio;
}
#endif // DISPATCH_USE_PTHREAD_ROOT_QUEUES || DISPATCH_USE_KEVENT_WORKQUEUE

#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
#if DISPATCH_USE_MGR_THREAD
#if !defined(_WIN32)
DISPATCH_NOINLINE
static pthread_t *
_dispatch_mgr_root_queue_init(void)
{
	dispatch_once_f(&_dispatch_mgr_sched_pred, NULL, _dispatch_mgr_sched_init);
	dispatch_pthread_root_queue_context_t pqc = _dispatch_mgr_root_queue.do_ctxt;
	pthread_attr_t *attr = &pqc->dpq_thread_attr;
	struct sched_param param;
	(void)dispatch_assume_zero(pthread_attr_setdetachstate(attr,
			PTHREAD_CREATE_DETACHED));
#if !DISPATCH_DEBUG
	(void)dispatch_assume_zero(pthread_attr_setstacksize(attr, 64 * 1024));
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS
	qos_class_t qos = _dispatch_mgr_sched.qos;
	if (qos) {
		if (_dispatch_set_qos_class_enabled) {
			(void)dispatch_assume_zero(pthread_attr_set_qos_class_np(attr,
					qos, 0));
		}
	}
#endif
	param.sched_priority = _dispatch_mgr_sched.prio;
	if (param.sched_priority > _dispatch_mgr_sched.default_prio) {
		(void)dispatch_assume_zero(pthread_attr_setschedparam(attr, &param));
	}
	return &_dispatch_mgr_sched.tid;
}
#else // defined(_WIN32)
DISPATCH_NOINLINE
static PHANDLE
_dispatch_mgr_root_queue_init(void)
{
	dispatch_once_f(&_dispatch_mgr_sched_pred, NULL, _dispatch_mgr_sched_init);
	return &_dispatch_mgr_sched.hThread;
}
#endif // defined(_WIN32)

static inline void
_dispatch_mgr_priority_apply(void)
{
#if !defined(_WIN32)
	struct sched_param param;
	do {
		param.sched_priority = _dispatch_mgr_sched.prio;
		if (param.sched_priority > _dispatch_mgr_sched.default_prio) {
			(void)dispatch_assume_zero(pthread_setschedparam(
					_dispatch_mgr_sched.tid, _dispatch_mgr_sched.policy,
					&param));
		}
	} while (_dispatch_mgr_sched.prio > param.sched_priority);
#else // defined(_WIN32)
	int nPriority = _dispatch_mgr_sched.prio;
	do {
		if (nPriority > _dispatch_mgr_sched.default_prio) {
			// TODO(compnerd) set thread scheduling policy
			dispatch_assume_zero(SetThreadPriority(_dispatch_mgr_sched.hThread, nPriority));
			nPriority = GetThreadPriority(_dispatch_mgr_sched.hThread);
		}
	} while (_dispatch_mgr_sched.prio > nPriority);
#endif // defined(_WIN32)
}

DISPATCH_NOINLINE
static void
_dispatch_mgr_priority_init(void)
{
#if !defined(_WIN32)
	dispatch_pthread_root_queue_context_t pqc = _dispatch_mgr_root_queue.do_ctxt;
	pthread_attr_t *attr = &pqc->dpq_thread_attr;
	struct sched_param param;
	(void)dispatch_assume_zero(pthread_attr_getschedparam(attr, &param));
#if HAVE_PTHREAD_WORKQUEUE_QOS
	qos_class_t qos = 0;
	(void)pthread_attr_get_qos_class_np(attr, &qos, NULL);
	if (_dispatch_mgr_sched.qos > qos && _dispatch_set_qos_class_enabled) {
		(void)pthread_set_qos_class_self_np(_dispatch_mgr_sched.qos, 0);
		int p = _dispatch_mgr_sched_qos2prio(_dispatch_mgr_sched.qos);
		if (p > param.sched_priority) {
			param.sched_priority = p;
		}
	}
#endif
	if (unlikely(_dispatch_mgr_sched.prio > param.sched_priority)) {
		return _dispatch_mgr_priority_apply();
	}
#else // defined(_WIN32)
	int nPriority = GetThreadPriority(_dispatch_mgr_sched.hThread);
	if (slowpath(_dispatch_mgr_sched.prio > nPriority)) {
		return _dispatch_mgr_priority_apply();
	}
#endif // defined(_WIN32)
}
#endif // DISPATCH_USE_MGR_THREAD

#if !defined(_WIN32)
DISPATCH_NOINLINE
static void
_dispatch_mgr_priority_raise(const pthread_attr_t *attr)
{
	dispatch_once_f(&_dispatch_mgr_sched_pred, NULL, _dispatch_mgr_sched_init);
	struct sched_param param;
	(void)dispatch_assume_zero(pthread_attr_getschedparam(attr, &param));
#if HAVE_PTHREAD_WORKQUEUE_QOS
	qos_class_t q, qos = 0;
	(void)pthread_attr_get_qos_class_np((pthread_attr_t *)attr, &qos, NULL);
	if (qos) {
		param.sched_priority = _dispatch_mgr_sched_qos2prio(qos);
		os_atomic_rmw_loop2o(&_dispatch_mgr_sched, qos, q, qos, relaxed, {
			if (q >= qos) os_atomic_rmw_loop_give_up(break);
		});
	}
#endif
	int p, prio = param.sched_priority;
	os_atomic_rmw_loop2o(&_dispatch_mgr_sched, prio, p, prio, relaxed, {
		if (p >= prio) os_atomic_rmw_loop_give_up(return);
	});
#if DISPATCH_USE_KEVENT_WORKQUEUE
	_dispatch_root_queues_init();
	if (_dispatch_kevent_workqueue_enabled) {
		pthread_priority_t pp = 0;
		if (prio > _dispatch_mgr_sched.default_prio) {
			// The values of _PTHREAD_PRIORITY_SCHED_PRI_FLAG and
			// _PTHREAD_PRIORITY_ROOTQUEUE_FLAG overlap, but that is not
			// problematic in this case, since it the second one is only ever
			// used on dq_priority fields.
			// We never pass the _PTHREAD_PRIORITY_ROOTQUEUE_FLAG to a syscall,
			// it is meaningful to libdispatch only.
			pp = (pthread_priority_t)prio | _PTHREAD_PRIORITY_SCHED_PRI_FLAG;
		} else if (qos) {
			pp = _pthread_qos_class_encode(qos, 0, 0);
		}
		if (pp) {
			int r = _pthread_workqueue_set_event_manager_priority(pp);
			(void)dispatch_assume_zero(r);
		}
		return;
	}
#endif
#if DISPATCH_USE_MGR_THREAD
	if (_dispatch_mgr_sched.tid) {
		return _dispatch_mgr_priority_apply();
	}
#endif
}
#endif // !defined(_WIN32)
#endif // DISPATCH_USE_PTHREAD_ROOT_QUEUES

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_mgr_lock(struct dispatch_queue_static_s *dq)
{
	uint64_t old_state, new_state, set_owner_and_set_full_width =
			_dispatch_lock_value_for_self() | DISPATCH_QUEUE_SERIAL_DRAIN_OWNED;

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, acquire, {
		new_state = old_state;
		if (unlikely(!_dq_state_is_runnable(old_state) ||
				_dq_state_drain_locked(old_state))) {
			DISPATCH_INTERNAL_CRASH((uintptr_t)old_state,
					"Locking the manager should not fail");
		}
		new_state &= DISPATCH_QUEUE_DRAIN_PRESERVED_BITS_MASK;
		new_state |= set_owner_and_set_full_width;
	});
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_mgr_unlock(struct dispatch_queue_static_s *dq)
{
	uint64_t old_state, new_state;
	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, release, {
		new_state = old_state - DISPATCH_QUEUE_SERIAL_DRAIN_OWNED;
		new_state &= ~DISPATCH_QUEUE_DRAIN_UNLOCK_MASK;
		new_state &= ~DISPATCH_QUEUE_MAX_QOS_MASK;
	});
	return _dq_state_is_dirty(old_state);
}

static void
_dispatch_mgr_queue_drain(void)
{
	const dispatch_invoke_flags_t flags = DISPATCH_INVOKE_MANAGER_DRAIN;
	dispatch_invoke_context_s dic = { };
	struct dispatch_queue_static_s *dq = &_dispatch_mgr_q;
	uint64_t owned = DISPATCH_QUEUE_SERIAL_DRAIN_OWNED;

	if (dq->dq_items_tail) {
		_dispatch_perfmon_start();
		_dispatch_set_basepri_override_qos(DISPATCH_QOS_SATURATED);
		if (unlikely(_dispatch_lane_serial_drain(dq, &dic, flags, &owned))) {
			DISPATCH_INTERNAL_CRASH(0, "Interrupted drain on manager queue");
		}
		_dispatch_voucher_debug("mgr queue clear", NULL);
		_voucher_clear();
		_dispatch_reset_basepri_override();
		_dispatch_perfmon_end(perfmon_thread_manager);
	}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
#if DISPATCH_USE_KEVENT_WORKQUEUE
	if (!_dispatch_kevent_workqueue_enabled)
#endif
	{
		_dispatch_force_cache_cleanup();
	}
#pragma clang diagnostic pop
}

void
_dispatch_mgr_queue_push(dispatch_lane_t dq, dispatch_object_t dou,
		DISPATCH_UNUSED dispatch_qos_t qos)
{
	uint64_t dq_state;

	if (unlikely(_dispatch_object_is_waiter(dou))) {
		DISPATCH_CLIENT_CRASH(0, "Waiter pushed onto manager");
	}

	if (unlikely(_dispatch_queue_push_item(dq, dou))) {
		dq_state = os_atomic_or2o(dq, dq_state, DISPATCH_QUEUE_DIRTY, release);
		if (!_dq_state_drain_locked_by_self(dq_state)) {
			_dispatch_trace_runtime_event(worker_request, &_dispatch_mgr_q, 1);
			_dispatch_event_loop_poke(DISPATCH_WLH_MANAGER, 0, 0);
		}
	}
}

DISPATCH_NORETURN
void
_dispatch_mgr_queue_wakeup(DISPATCH_UNUSED dispatch_lane_t dq,
		DISPATCH_UNUSED dispatch_qos_t qos,
		DISPATCH_UNUSED dispatch_wakeup_flags_t flags)
{
	DISPATCH_INTERNAL_CRASH(0, "Don't try to wake up or override the manager");
}

#if DISPATCH_USE_MGR_THREAD
DISPATCH_NOINLINE DISPATCH_NORETURN
static void
_dispatch_mgr_invoke(void)
{
#if DISPATCH_EVENT_BACKEND_KEVENT
	dispatch_kevent_s evbuf[DISPATCH_DEFERRED_ITEMS_EVENT_COUNT];
#endif
	dispatch_deferred_items_s ddi = {
		.ddi_wlh = DISPATCH_WLH_ANON,
#if DISPATCH_EVENT_BACKEND_KEVENT
		.ddi_maxevents = DISPATCH_DEFERRED_ITEMS_EVENT_COUNT,
		.ddi_eventlist = evbuf,
#endif
	};

	_dispatch_deferred_items_set(&ddi);
	for (;;) {
		bool poll = false;
		_dispatch_mgr_queue_drain();
		_dispatch_event_loop_drain_anon_timers();
		poll = _dispatch_queue_class_probe(&_dispatch_mgr_q);
		_dispatch_event_loop_drain(poll ? KEVENT_FLAG_IMMEDIATE : 0);
	}
}

DISPATCH_NORETURN
void
_dispatch_mgr_thread(dispatch_lane_t dq DISPATCH_UNUSED,
		dispatch_invoke_context_t dic DISPATCH_UNUSED,
		dispatch_invoke_flags_t flags DISPATCH_UNUSED)
{
#if DISPATCH_USE_KEVENT_WORKQUEUE
	if (_dispatch_kevent_workqueue_enabled) {
		DISPATCH_INTERNAL_CRASH(0, "Manager queue invoked with "
				"kevent workqueue enabled");
	}
#endif
	_dispatch_queue_set_current(&_dispatch_mgr_q);
#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
	_dispatch_mgr_priority_init();
#endif
	_dispatch_queue_mgr_lock(&_dispatch_mgr_q);
	// never returns, so burn bridges behind us & clear stack 2k ahead
	_dispatch_clear_stack(2048);
	_dispatch_mgr_invoke();
}
#endif // DISPATCH_USE_MGR_THREAD

#if DISPATCH_USE_KEVENT_WORKQUEUE

dispatch_static_assert(WORKQ_KEVENT_EVENT_BUFFER_LEN >=
		DISPATCH_DEFERRED_ITEMS_EVENT_COUNT,
		"our list should not be longer than the kernel's");

static void _dispatch_root_queue_drain_deferred_item(
		dispatch_deferred_items_t ddi DISPATCH_PERF_MON_ARGS_PROTO);
static void _dispatch_root_queue_drain_deferred_wlh(
		dispatch_deferred_items_t ddi DISPATCH_PERF_MON_ARGS_PROTO);

void
_dispatch_kevent_workqueue_init(void)
{
	// Initialize kevent workqueue support
	_dispatch_root_queues_init();
	if (!_dispatch_kevent_workqueue_enabled) return;
	dispatch_once_f(&_dispatch_mgr_sched_pred, NULL, _dispatch_mgr_sched_init);
	qos_class_t qos = _dispatch_mgr_sched.qos;
	int prio = _dispatch_mgr_sched.prio;
	pthread_priority_t pp = 0;
	if (qos) {
		pp = _pthread_qos_class_encode(qos, 0, 0);
	}
	if (prio > _dispatch_mgr_sched.default_prio) {
		pp = (pthread_priority_t)prio | _PTHREAD_PRIORITY_SCHED_PRI_FLAG;
	}
	if (pp) {
		int r = _pthread_workqueue_set_event_manager_priority(pp);
		(void)dispatch_assume_zero(r);
	}
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_wlh_worker_thread_init(dispatch_deferred_items_t ddi)
{
	dispatch_assert(ddi->ddi_wlh);

	pthread_priority_t pp = _dispatch_get_priority();
	if (!(pp & _PTHREAD_PRIORITY_EVENT_MANAGER_FLAG)) {
		// If this thread does not have the event manager flag set, don't setup
		// as the dispatch manager and let the caller know to only process
		// the delivered events.
		//
		// Also add the NEEDS_UNBIND flag so that
		// _dispatch_priority_compute_update knows it has to unbind
		pp &= _PTHREAD_PRIORITY_OVERCOMMIT_FLAG | ~_PTHREAD_PRIORITY_FLAGS_MASK;
		if (ddi->ddi_wlh == DISPATCH_WLH_ANON) {
			pp |= _PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG;
		} else {
			// pthread sets the flag when it is an event delivery thread
			// so we need to explicitly clear it
			pp &= ~(pthread_priority_t)_PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG;
		}
		_dispatch_thread_setspecific(dispatch_priority_key,
				(void *)(uintptr_t)pp);
		if (ddi->ddi_wlh != DISPATCH_WLH_ANON) {
			_dispatch_debug("wlh[%p]: handling events", ddi->ddi_wlh);
		} else {
			ddi->ddi_can_stash = true;
		}
		return false;
	}

	if ((pp & _PTHREAD_PRIORITY_SCHED_PRI_FLAG) ||
			!(pp & ~_PTHREAD_PRIORITY_FLAGS_MASK)) {
		// When the phtread kext is delivering kevents to us, and pthread
		// root queues are in use, then the pthread priority TSD is set
		// to a sched pri with the _PTHREAD_PRIORITY_SCHED_PRI_FLAG bit set.
		//
		// Given that this isn't a valid QoS we need to fixup the TSD,
		// and the best option is to clear the qos/priority bits which tells
		// us to not do any QoS related calls on this thread.
		//
		// However, in that case the manager thread is opted out of QoS,
		// as far as pthread is concerned, and can't be turned into
		// something else, so we can't stash.
		pp &= (pthread_priority_t)_PTHREAD_PRIORITY_FLAGS_MASK;
	}
	// Managers always park without mutating to a regular worker thread, and
	// hence never need to unbind from userland, and when draining a manager,
	// the NEEDS_UNBIND flag would cause the mutation to happen.
	// So we need to strip this flag
	pp &= ~(pthread_priority_t)_PTHREAD_PRIORITY_NEEDS_UNBIND_FLAG;
	_dispatch_thread_setspecific(dispatch_priority_key, (void *)(uintptr_t)pp);

	// ensure kevents registered from this thread are registered at manager QoS
	_dispatch_init_basepri_wlh(DISPATCH_PRIORITY_FLAG_MANAGER);
	_dispatch_queue_set_current(&_dispatch_mgr_q);
	_dispatch_queue_mgr_lock(&_dispatch_mgr_q);
	return true;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_wlh_worker_thread_reset(void)
{
	bool needs_poll = _dispatch_queue_mgr_unlock(&_dispatch_mgr_q);
	_dispatch_clear_basepri();
	_dispatch_queue_set_current(NULL);
	if (needs_poll) {
		_dispatch_trace_runtime_event(worker_request, &_dispatch_mgr_q, 1);
		_dispatch_event_loop_poke(DISPATCH_WLH_MANAGER, 0, 0);
	}
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_wlh_worker_thread(dispatch_wlh_t wlh, dispatch_kevent_t events,
		int *nevents)
{
	_dispatch_introspection_thread_add();

	DISPATCH_PERF_MON_VAR_INIT

	dispatch_deferred_items_s ddi = {
		.ddi_wlh = wlh,
		.ddi_eventlist = events,
	};
	bool is_manager;

	is_manager = _dispatch_wlh_worker_thread_init(&ddi);
	if (!is_manager) {
		_dispatch_trace_runtime_event(worker_event_delivery,
				wlh == DISPATCH_WLH_ANON ? NULL : wlh, (uint64_t)*nevents);
		_dispatch_perfmon_start_impl(true);
	} else {
		_dispatch_trace_runtime_event(worker_event_delivery,
				&_dispatch_mgr_q, (uint64_t)*nevents);
		ddi.ddi_wlh = DISPATCH_WLH_ANON;
	}
	_dispatch_deferred_items_set(&ddi);
	_dispatch_event_loop_merge(events, *nevents);

	if (is_manager) {
		_dispatch_trace_runtime_event(worker_unpark, &_dispatch_mgr_q, 0);
		_dispatch_mgr_queue_drain();
		_dispatch_event_loop_drain_anon_timers();
		_dispatch_wlh_worker_thread_reset();
	} else if (ddi.ddi_stashed_dou._do) {
		_dispatch_debug("wlh[%p]: draining deferred item %p", ddi.ddi_wlh,
				ddi.ddi_stashed_dou._do);
		if (ddi.ddi_wlh == DISPATCH_WLH_ANON) {
			dispatch_assert(ddi.ddi_nevents == 0);
			_dispatch_deferred_items_set(NULL);
			_dispatch_trace_runtime_event(worker_unpark, ddi.ddi_stashed_rq, 0);
			_dispatch_root_queue_drain_deferred_item(&ddi
					DISPATCH_PERF_MON_ARGS);
		} else {
			_dispatch_trace_runtime_event(worker_unpark, wlh, 0);
			_dispatch_root_queue_drain_deferred_wlh(&ddi
					DISPATCH_PERF_MON_ARGS);
		}
	}

	_dispatch_deferred_items_set(NULL);
	if (!is_manager && !ddi.ddi_stashed_dou._do) {
		_dispatch_perfmon_end(perfmon_thread_event_no_steal);
	}
	_dispatch_debug("returning %d deferred kevents", ddi.ddi_nevents);
	_dispatch_clear_return_to_kernel();
	*nevents = ddi.ddi_nevents;

	_dispatch_trace_runtime_event(worker_park, NULL, 0);
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_worker_thread(dispatch_kevent_t *events, int *nevents)
{
	if (!dispatch_assume(events && nevents)) {
		return;
	}
	if (*nevents == 0 || *events == NULL) {
		// events for worker thread request have already been delivered earlier
		// or got cancelled before point of no return concurrently
		return;
	}
	_dispatch_adopt_wlh_anon();
	_dispatch_wlh_worker_thread(DISPATCH_WLH_ANON, *events, nevents);
	_dispatch_reset_wlh();
}

#if DISPATCH_USE_KEVENT_WORKLOOP
DISPATCH_NOINLINE
static void
_dispatch_workloop_worker_thread(uint64_t *workloop_id,
		dispatch_kevent_t *events, int *nevents)
{
	if (!dispatch_assume(workloop_id && events && nevents)) {
		return;
	}
	if (!dispatch_assume(*workloop_id != 0)) {
		return _dispatch_kevent_worker_thread(events, nevents);
	}
	if (*nevents == 0 || *events == NULL) {
		// events for worker thread request have already been delivered earlier
		// or got cancelled before point of no return concurrently
		return;
	}
	dispatch_wlh_t wlh = (dispatch_wlh_t)*workloop_id;
	_dispatch_adopt_wlh(wlh);
	_dispatch_wlh_worker_thread(wlh, *events, nevents);
	_dispatch_preserve_wlh_storage_reference(wlh);
}
#endif // DISPATCH_USE_KEVENT_WORKLOOP
#endif // DISPATCH_USE_KEVENT_WORKQUEUE
#pragma mark -
#pragma mark dispatch_root_queue

#if DISPATCH_USE_PTHREAD_POOL
static void *_dispatch_worker_thread(void *context);
#if defined(_WIN32)
static unsigned WINAPI _dispatch_worker_thread_thunk(LPVOID lpParameter);
#endif
#endif // DISPATCH_USE_PTHREAD_POOL

#if DISPATCH_DEBUG && DISPATCH_ROOT_QUEUE_DEBUG
#define _dispatch_root_queue_debug(...) _dispatch_debug(__VA_ARGS__)
static void
_dispatch_debug_root_queue(dispatch_queue_class_t dqu, const char *str)
{
	if (likely(dqu._dq)) {
		_dispatch_object_debug(dqu._dq, "%s", str);
	} else {
		_dispatch_log("queue[NULL]: %s", str);
	}
}
#else
#define _dispatch_root_queue_debug(...)
#define _dispatch_debug_root_queue(...)
#endif // DISPATCH_DEBUG && DISPATCH_ROOT_QUEUE_DEBUG

DISPATCH_NOINLINE
static void
_dispatch_root_queue_poke_slow(dispatch_queue_global_t dq, int n, int floor)
{
	int remaining = n;
	int r = ENOSYS;

	_dispatch_root_queues_init();
	_dispatch_debug_root_queue(dq, __func__);
	_dispatch_trace_runtime_event(worker_request, dq, (uint64_t)n);

#if !DISPATCH_USE_INTERNAL_WORKQUEUE
#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
	if (dx_type(dq) == DISPATCH_QUEUE_GLOBAL_ROOT_TYPE)
#endif
	{
		_dispatch_root_queue_debug("requesting new worker thread for global "
				"queue: %p", dq);
		r = _pthread_workqueue_addthreads(remaining,
				_dispatch_priority_to_pp_prefer_fallback(dq->dq_priority));
		(void)dispatch_assume_zero(r);
		return;
	}
#endif // !DISPATCH_USE_INTERNAL_WORKQUEUE
#if DISPATCH_USE_PTHREAD_POOL
	dispatch_pthread_root_queue_context_t pqc = dq->do_ctxt;
	if (likely(pqc->dpq_thread_mediator.do_vtable)) {
		while (dispatch_semaphore_signal(&pqc->dpq_thread_mediator)) {
			_dispatch_root_queue_debug("signaled sleeping worker for "
					"global queue: %p", dq);
			if (!--remaining) {
				return;
			}
		}
	}

	bool overcommit = dq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
	if (overcommit) {
		os_atomic_add2o(dq, dgq_pending, remaining, relaxed);
	} else {
		if (!os_atomic_cmpxchg2o(dq, dgq_pending, 0, remaining, relaxed)) {
			_dispatch_root_queue_debug("worker thread request still pending for "
					"global queue: %p", dq);
			return;
		}
	}

	int can_request, t_count;
	// seq_cst with atomic store to tail <rdar://problem/16932833>
	t_count = os_atomic_load2o(dq, dgq_thread_pool_size, ordered);
	do {
		can_request = t_count < floor ? 0 : t_count - floor;
		if (remaining > can_request) {
			_dispatch_root_queue_debug("pthread pool reducing request from %d to %d",
					remaining, can_request);
			os_atomic_sub2o(dq, dgq_pending, remaining - can_request, relaxed);
			remaining = can_request;
		}
		if (remaining == 0) {
			_dispatch_root_queue_debug("pthread pool is full for root queue: "
					"%p", dq);
			return;
		}
	} while (!os_atomic_cmpxchgvw2o(dq, dgq_thread_pool_size, t_count,
			t_count - remaining, &t_count, acquire));

#if !defined(_WIN32)
	pthread_attr_t *attr = &pqc->dpq_thread_attr;
	pthread_t tid, *pthr = &tid;
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
	if (unlikely(dq == &_dispatch_mgr_root_queue)) {
		pthr = _dispatch_mgr_root_queue_init();
	}
#endif
	do {
		_dispatch_retain(dq); // released in _dispatch_worker_thread
		while ((r = pthread_create(pthr, attr, _dispatch_worker_thread, dq))) {
			if (r != EAGAIN) {
				(void)dispatch_assume_zero(r);
			}
			_dispatch_temporary_resource_shortage();
		}
	} while (--remaining);
#else // defined(_WIN32)
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
	if (unlikely(dq == &_dispatch_mgr_root_queue)) {
		_dispatch_mgr_root_queue_init();
	}
#endif
	do {
		_dispatch_retain(dq); // released in _dispatch_worker_thread
#if DISPATCH_DEBUG
		unsigned dwStackSize = 0;
#else
		unsigned dwStackSize = 64 * 1024;
#endif
		uintptr_t hThread = 0;
		while (!(hThread = _beginthreadex(NULL, dwStackSize, _dispatch_worker_thread_thunk, dq, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL))) {
			if (errno != EAGAIN) {
				(void)dispatch_assume(hThread);
			}
			_dispatch_temporary_resource_shortage();
		}
		if (_dispatch_mgr_sched.prio > _dispatch_mgr_sched.default_prio) {
			(void)dispatch_assume_zero(SetThreadPriority((HANDLE)hThread, _dispatch_mgr_sched.prio) == TRUE);
		}
		CloseHandle((HANDLE)hThread);
	} while (--remaining);
#endif // defined(_WIN32)
#else
	(void)floor;
#endif // DISPATCH_USE_PTHREAD_POOL
}

DISPATCH_NOINLINE
void
_dispatch_root_queue_poke(dispatch_queue_global_t dq, int n, int floor)
{
	if (!_dispatch_queue_class_probe(dq)) {
		return;
	}
#if !DISPATCH_USE_INTERNAL_WORKQUEUE
#if DISPATCH_USE_PTHREAD_POOL
	if (likely(dx_type(dq) == DISPATCH_QUEUE_GLOBAL_ROOT_TYPE))
#endif
	{
		if (unlikely(!os_atomic_cmpxchg2o(dq, dgq_pending, 0, n, relaxed))) {
			_dispatch_root_queue_debug("worker thread request still pending "
					"for global queue: %p", dq);
			return;
		}
	}
#endif // !DISPATCH_USE_INTERNAL_WORKQUEUE
	return _dispatch_root_queue_poke_slow(dq, n, floor);
}

#define DISPATCH_ROOT_QUEUE_MEDIATOR ((struct dispatch_object_s *)~0ul)

enum {
	DISPATCH_ROOT_QUEUE_DRAIN_WAIT,
	DISPATCH_ROOT_QUEUE_DRAIN_READY,
	DISPATCH_ROOT_QUEUE_DRAIN_ABORT,
};

static int
_dispatch_root_queue_mediator_is_gone(dispatch_queue_global_t dq)
{
	return os_atomic_load2o(dq, dq_items_head, relaxed) !=
			DISPATCH_ROOT_QUEUE_MEDIATOR;
}

static int
_dispatch_root_queue_head_tail_quiesced(dispatch_queue_global_t dq)
{
	// Wait for queue head and tail to be both non-empty or both empty
	struct dispatch_object_s *head, *tail;
	head = os_atomic_load2o(dq, dq_items_head, relaxed);
	tail = os_atomic_load2o(dq, dq_items_tail, relaxed);
	if ((head == NULL) == (tail == NULL)) {
		if (tail == NULL) { // <rdar://problem/15917893>
			return DISPATCH_ROOT_QUEUE_DRAIN_ABORT;
		}
		return DISPATCH_ROOT_QUEUE_DRAIN_READY;
	}
	return DISPATCH_ROOT_QUEUE_DRAIN_WAIT;
}

DISPATCH_NOINLINE
static bool
__DISPATCH_ROOT_QUEUE_CONTENDED_WAIT__(dispatch_queue_global_t dq,
		int (*predicate)(dispatch_queue_global_t dq))
{
	unsigned int sleep_time = DISPATCH_CONTENTION_USLEEP_START;
	int status = DISPATCH_ROOT_QUEUE_DRAIN_READY;
	bool pending = false;

	do {
		// Spin for a short while in case the contention is temporary -- e.g.
		// when starting up after dispatch_apply, or when executing a few
		// short continuations in a row.
		if (_dispatch_contention_wait_until(status = predicate(dq))) {
			goto out;
		}
		// Since we have serious contention, we need to back off.
		if (!pending) {
			// Mark this queue as pending to avoid requests for further threads
			(void)os_atomic_inc2o(dq, dgq_pending, relaxed);
			pending = true;
		}
		_dispatch_contention_usleep(sleep_time);
		if (likely(status = predicate(dq))) goto out;
		sleep_time *= 2;
	} while (sleep_time < DISPATCH_CONTENTION_USLEEP_MAX);

	// The ratio of work to libdispatch overhead must be bad. This
	// scenario implies that there are too many threads in the pool.
	// Create a new pending thread and then exit this thread.
	// The kernel will grant a new thread when the load subsides.
	_dispatch_debug("contention on global queue: %p", dq);
out:
	if (pending) {
		(void)os_atomic_dec2o(dq, dgq_pending, relaxed);
	}
	if (status == DISPATCH_ROOT_QUEUE_DRAIN_WAIT) {
		_dispatch_root_queue_poke(dq, 1, 0);
	}
	return status == DISPATCH_ROOT_QUEUE_DRAIN_READY;
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline struct dispatch_object_s *
_dispatch_root_queue_drain_one(dispatch_queue_global_t dq)
{
	struct dispatch_object_s *head, *next;

start:
	// The MEDIATOR value acts both as a "lock" and a signal
	head = os_atomic_xchg2o(dq, dq_items_head,
			DISPATCH_ROOT_QUEUE_MEDIATOR, relaxed);

	if (unlikely(head == NULL)) {
		// The first xchg on the tail will tell the enqueueing thread that it
		// is safe to blindly write out to the head pointer. A cmpxchg honors
		// the algorithm.
		if (unlikely(!os_atomic_cmpxchg2o(dq, dq_items_head,
				DISPATCH_ROOT_QUEUE_MEDIATOR, NULL, relaxed))) {
			goto start;
		}
		if (unlikely(dq->dq_items_tail)) { // <rdar://problem/14416349>
			if (__DISPATCH_ROOT_QUEUE_CONTENDED_WAIT__(dq,
					_dispatch_root_queue_head_tail_quiesced)) {
				goto start;
			}
		}
		_dispatch_root_queue_debug("no work on global queue: %p", dq);
		return NULL;
	}

	if (unlikely(head == DISPATCH_ROOT_QUEUE_MEDIATOR)) {
		// This thread lost the race for ownership of the queue.
		if (likely(__DISPATCH_ROOT_QUEUE_CONTENDED_WAIT__(dq,
				_dispatch_root_queue_mediator_is_gone))) {
			goto start;
		}
		return NULL;
	}

	// Restore the head pointer to a sane value before returning.
	// If 'next' is NULL, then this item _might_ be the last item.
	next = head->do_next;

	if (unlikely(!next)) {
		os_atomic_store2o(dq, dq_items_head, NULL, relaxed);
		// 22708742: set tail to NULL with release, so that NULL write to head
		//           above doesn't clobber head from concurrent enqueuer
		if (os_atomic_cmpxchg2o(dq, dq_items_tail, head, NULL, release)) {
			// both head and tail are NULL now
			goto out;
		}
		// There must be a next item now.
		next = os_mpsc_get_next(head, do_next);
	}

	os_atomic_store2o(dq, dq_items_head, next, relaxed);
	_dispatch_root_queue_poke(dq, 1, 0);
out:
	return head;
}

#if DISPATCH_USE_KEVENT_WORKQUEUE
static void
_dispatch_root_queue_drain_deferred_wlh(dispatch_deferred_items_t ddi
		DISPATCH_PERF_MON_ARGS_PROTO)
{
	dispatch_queue_global_t rq = ddi->ddi_stashed_rq;
	dispatch_queue_t dq = ddi->ddi_stashed_dou._dq;
	_dispatch_queue_set_current(rq);

	dispatch_invoke_context_s dic = { };
	dispatch_invoke_flags_t flags = DISPATCH_INVOKE_WORKER_DRAIN |
			DISPATCH_INVOKE_REDIRECTING_DRAIN | DISPATCH_INVOKE_WLH;
	_dispatch_queue_drain_init_narrowing_check_deadline(&dic, rq->dq_priority);
	uint64_t dq_state;

	_dispatch_init_basepri_wlh(rq->dq_priority);
	ddi->ddi_wlh_servicing = true;
retry:
	dispatch_assert(ddi->ddi_wlh_needs_delete);
	_dispatch_trace_item_pop(rq, dq);

	if (_dispatch_queue_drain_try_lock_wlh(dq, &dq_state)) {
		dx_invoke(dq, &dic, flags);
#if DISPATCH_USE_KEVENT_WORKLOOP
		//
		// dx_invoke() will always return `dq` unlocked or locked by another
		// thread, and either have consumed the +2 or transferred it to the
		// other thread.
		//
#endif
		if (!ddi->ddi_wlh_needs_delete) {
#if DISPATCH_USE_KEVENT_WORKLOOP
			//
			// The fate of the workloop thread request has already been dealt
			// with, which can happen for 4 reasons, for which we just want
			// to go park and skip trying to unregister the thread request:
			// - the workloop target has been changed
			// - the workloop has been re-enqueued because of narrowing
			// - the workloop has been re-enqueued on the manager queue
			// - the workloop ownership has been handed off to a sync owner
			//
#endif
			goto park;
		}
#if DISPATCH_USE_KEVENT_WORKLOOP
		//
		// The workloop has been drained to completion or suspended.
		// dx_invoke() has cleared the enqueued bit before it returned.
		//
		// Since a dispatch_set_target_queue() could occur between the unlock
		// and our reload of `dq_state` (rdar://32671286) we need to re-assess
		// the workloop-ness of the queue. If it's not a workloop anymore,
		// _dispatch_event_loop_leave_immediate() will have handled the kevent
		// deletion already.
		//
		// Then, we check one last time that the queue is still not enqueued,
		// in which case we attempt to quiesce it.
		//
		// If we find it enqueued again, it means someone else has been
		// enqueuing concurrently and has made a thread request that coalesced
		// with ours, but since dx_invoke() cleared the enqueued bit,
		// the other thread didn't realize that and added a +1 ref count.
		// Take over that +1, and add our own to make the +2 this loop expects,
		// and drain again.
		//
#endif // DISPATCH_USE_KEVENT_WORKLOOP
		dq_state = os_atomic_load2o(dq, dq_state, relaxed);
		if (unlikely(!_dq_state_is_base_wlh(dq_state))) { // rdar://32671286
			goto park;
		}
		if (unlikely(_dq_state_is_enqueued_on_target(dq_state))) {
			_dispatch_retain(dq);
			_dispatch_trace_item_push(dq->do_targetq, dq);
			goto retry;
		}
	} else {
#if DISPATCH_USE_KEVENT_WORKLOOP
		//
		// The workloop enters this function with a +2 refcount, however we
		// couldn't acquire the lock due to suspension or discovering that
		// the workloop was locked by a sync owner.
		//
		// We need to give up, and _dispatch_event_loop_leave_deferred()
		// will do a DISPATCH_WORKLOOP_ASYNC_DISCOVER_SYNC transition to
		// tell the kernel to stop driving this thread request. We leave
		// a +1 with the thread request, and consume the extra +1 we have.
		//
#endif
		if (_dq_state_is_suspended(dq_state)) {
			dispatch_assert(!_dq_state_is_enqueued(dq_state));
			_dispatch_release_2_no_dispose(dq);
		} else {
			dispatch_assert(_dq_state_is_enqueued(dq_state));
			dispatch_assert(_dq_state_drain_locked(dq_state));
			_dispatch_release_no_dispose(dq);
		}
	}

	_dispatch_event_loop_leave_deferred(ddi, dq_state);

park:
	// event thread that could steal
	_dispatch_perfmon_end(perfmon_thread_event_steal);
	_dispatch_clear_basepri();
	_dispatch_queue_set_current(NULL);

	_dispatch_voucher_debug("root queue clear", NULL);
	_dispatch_reset_voucher(NULL, DISPATCH_THREAD_PARK);
}

static void
_dispatch_root_queue_drain_deferred_item(dispatch_deferred_items_t ddi
		DISPATCH_PERF_MON_ARGS_PROTO)
{
	dispatch_queue_global_t rq = ddi->ddi_stashed_rq;
	_dispatch_queue_set_current(rq);
	_dispatch_trace_runtime_event(worker_unpark, NULL, 0);

	dispatch_invoke_context_s dic = { };
	dispatch_invoke_flags_t flags = DISPATCH_INVOKE_WORKER_DRAIN |
			DISPATCH_INVOKE_REDIRECTING_DRAIN;
#if DISPATCH_COCOA_COMPAT
	_dispatch_last_resort_autorelease_pool_push(&dic);
#endif // DISPATCH_COCOA_COMPAT
	_dispatch_queue_drain_init_narrowing_check_deadline(&dic, rq->dq_priority);
	_dispatch_init_basepri(rq->dq_priority);

	_dispatch_continuation_pop_inline(ddi->ddi_stashed_dou, &dic, flags, rq);

	// event thread that could steal
	_dispatch_perfmon_end(perfmon_thread_event_steal);
#if DISPATCH_COCOA_COMPAT
	_dispatch_last_resort_autorelease_pool_pop(&dic);
#endif // DISPATCH_COCOA_COMPAT
	_dispatch_clear_basepri();
	_dispatch_queue_set_current(NULL);

	_dispatch_voucher_debug("root queue clear", NULL);
	_dispatch_reset_voucher(NULL, DISPATCH_THREAD_PARK);
}
#endif

DISPATCH_NOT_TAIL_CALLED // prevent tailcall (for Instrument DTrace probe)
static void
_dispatch_root_queue_drain(dispatch_queue_global_t dq,
		dispatch_priority_t pri, dispatch_invoke_flags_t flags)
{
#if DISPATCH_DEBUG
	dispatch_queue_t cq;
	if (unlikely(cq = _dispatch_queue_get_current())) {
		DISPATCH_INTERNAL_CRASH(cq, "Premature thread recycling");
	}
#endif
	_dispatch_queue_set_current(dq);
	_dispatch_init_basepri(pri);
	_dispatch_adopt_wlh_anon();

	struct dispatch_object_s *item;
	bool reset = false;
	dispatch_invoke_context_s dic = { };
#if DISPATCH_COCOA_COMPAT
	_dispatch_last_resort_autorelease_pool_push(&dic);
#endif // DISPATCH_COCOA_COMPAT
	_dispatch_queue_drain_init_narrowing_check_deadline(&dic, pri);
	_dispatch_perfmon_start();
	while (likely(item = _dispatch_root_queue_drain_one(dq))) {
		if (reset) _dispatch_wqthread_override_reset();
		_dispatch_continuation_pop_inline(item, &dic, flags, dq);
		reset = _dispatch_reset_basepri_override();
		if (unlikely(_dispatch_queue_drain_should_narrow(&dic))) {
			break;
		}
	}

	// overcommit or not. worker thread
	if (pri & DISPATCH_PRIORITY_FLAG_OVERCOMMIT) {
		_dispatch_perfmon_end(perfmon_thread_worker_oc);
	} else {
		_dispatch_perfmon_end(perfmon_thread_worker_non_oc);
	}

#if DISPATCH_COCOA_COMPAT
	_dispatch_last_resort_autorelease_pool_pop(&dic);
#endif // DISPATCH_COCOA_COMPAT
	_dispatch_reset_wlh();
	_dispatch_clear_basepri();
	_dispatch_queue_set_current(NULL);
}

#if !DISPATCH_USE_INTERNAL_WORKQUEUE
static void
_dispatch_worker_thread2(pthread_priority_t pp)
{
	bool overcommit = pp & _PTHREAD_PRIORITY_OVERCOMMIT_FLAG;
	dispatch_queue_global_t dq;

	pp &= _PTHREAD_PRIORITY_OVERCOMMIT_FLAG | ~_PTHREAD_PRIORITY_FLAGS_MASK;
	_dispatch_thread_setspecific(dispatch_priority_key, (void *)(uintptr_t)pp);
	dq = _dispatch_get_root_queue(_dispatch_qos_from_pp(pp), overcommit);

	_dispatch_introspection_thread_add();
	_dispatch_trace_runtime_event(worker_unpark, dq, 0);

	int pending = os_atomic_dec2o(dq, dgq_pending, relaxed);
	dispatch_assert(pending >= 0);
	_dispatch_root_queue_drain(dq, dq->dq_priority,
			DISPATCH_INVOKE_WORKER_DRAIN | DISPATCH_INVOKE_REDIRECTING_DRAIN);
	_dispatch_voucher_debug("root queue clear", NULL);
	_dispatch_reset_voucher(NULL, DISPATCH_THREAD_PARK);
	_dispatch_trace_runtime_event(worker_park, NULL, 0);
}
#endif // !DISPATCH_USE_INTERNAL_WORKQUEUE

#if DISPATCH_USE_PTHREAD_POOL
static inline void
_dispatch_root_queue_init_pthread_pool(dispatch_queue_global_t dq,
		int pool_size, dispatch_priority_t pri)
{
	dispatch_pthread_root_queue_context_t pqc = dq->do_ctxt;
	int thread_pool_size = DISPATCH_WORKQ_MAX_PTHREAD_COUNT;
	if (!(pri & DISPATCH_PRIORITY_FLAG_OVERCOMMIT)) {
		thread_pool_size = (int32_t)dispatch_hw_config(active_cpus);
	}
	if (pool_size && pool_size < thread_pool_size) thread_pool_size = pool_size;
	dq->dgq_thread_pool_size = thread_pool_size;
	qos_class_t cls = _dispatch_qos_to_qos_class(_dispatch_priority_qos(pri) ?:
			_dispatch_priority_fallback_qos(pri));
	if (cls) {
#if !defined(_WIN32)
		pthread_attr_t *attr = &pqc->dpq_thread_attr;
		int r = pthread_attr_init(attr);
		dispatch_assume_zero(r);
		r = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
		dispatch_assume_zero(r);
#endif // !defined(_WIN32)
#if HAVE_PTHREAD_WORKQUEUE_QOS
		r = pthread_attr_set_qos_class_np(attr, cls, 0);
		dispatch_assume_zero(r);
#endif // HAVE_PTHREAD_WORKQUEUE_QOS
	}
	_dispatch_sema4_t *sema = &pqc->dpq_thread_mediator.dsema_sema;
	pqc->dpq_thread_mediator.do_vtable = DISPATCH_VTABLE(semaphore);
	_dispatch_sema4_init(sema, _DSEMA4_POLICY_LIFO);
	_dispatch_sema4_create(sema, _DSEMA4_POLICY_LIFO);
}

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
static void *
_dispatch_worker_thread(void *context)
{
	dispatch_queue_global_t dq = context;
	dispatch_pthread_root_queue_context_t pqc = dq->do_ctxt;

	int pending = os_atomic_dec2o(dq, dgq_pending, relaxed);
	if (unlikely(pending < 0)) {
		DISPATCH_INTERNAL_CRASH(pending, "Pending thread request underflow");
	}

	if (pqc->dpq_observer_hooks.queue_will_execute) {
		_dispatch_set_pthread_root_queue_observer_hooks(
				&pqc->dpq_observer_hooks);
	}
	if (pqc->dpq_thread_configure) {
		pqc->dpq_thread_configure();
	}

#if !defined(_WIN32)
	// workaround tweaks the kernel workqueue does for us
	_dispatch_sigmask();
#endif
	_dispatch_introspection_thread_add();

	const int64_t timeout = 5ull * NSEC_PER_SEC;
	pthread_priority_t pp = _dispatch_get_priority();
	dispatch_priority_t pri = dq->dq_priority;

	// If the queue is neither
	// - the manager
	// - with a fallback set
	// - with a requested QoS or QoS floor
	// then infer the basepri from the current priority.
	if ((pri & (DISPATCH_PRIORITY_FLAG_MANAGER |
			DISPATCH_PRIORITY_FLAG_FALLBACK |
			DISPATCH_PRIORITY_FLAG_FLOOR |
			DISPATCH_PRIORITY_REQUESTED_MASK)) == 0) {
		pri &= DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
		if (pp & _PTHREAD_PRIORITY_QOS_CLASS_MASK) {
			pri |= _dispatch_priority_from_pp(pp);
		} else {
			pri |= _dispatch_priority_make_override(DISPATCH_QOS_SATURATED);
		}
	}

#if DISPATCH_USE_INTERNAL_WORKQUEUE
	bool monitored = ((pri & (DISPATCH_PRIORITY_FLAG_OVERCOMMIT |
			DISPATCH_PRIORITY_FLAG_MANAGER)) == 0);
	if (monitored) _dispatch_workq_worker_register(dq);
#endif

	do {
		_dispatch_trace_runtime_event(worker_unpark, dq, 0);
		_dispatch_root_queue_drain(dq, pri, DISPATCH_INVOKE_REDIRECTING_DRAIN);
		_dispatch_reset_priority_and_voucher(pp, NULL);
		_dispatch_trace_runtime_event(worker_park, NULL, 0);
	} while (dispatch_semaphore_wait(&pqc->dpq_thread_mediator,
			dispatch_time(0, timeout)) == 0);

#if DISPATCH_USE_INTERNAL_WORKQUEUE
	if (monitored) _dispatch_workq_worker_unregister(dq);
#endif
	(void)os_atomic_inc2o(dq, dgq_thread_pool_size, release);
	_dispatch_root_queue_poke(dq, 1, 0);
	_dispatch_release(dq); // retained in _dispatch_root_queue_poke_slow
	return NULL;
}
#if defined(_WIN32)
static unsigned WINAPI
_dispatch_worker_thread_thunk(LPVOID lpParameter)
{
  _dispatch_worker_thread(lpParameter);
  return 0;
}
#endif // defined(_WIN32)
#endif // DISPATCH_USE_PTHREAD_POOL

DISPATCH_NOINLINE
void
_dispatch_root_queue_wakeup(dispatch_queue_global_t dq,
		DISPATCH_UNUSED dispatch_qos_t qos, dispatch_wakeup_flags_t flags)
{
	if (!(flags & DISPATCH_WAKEUP_BLOCK_WAIT)) {
		DISPATCH_INTERNAL_CRASH(dq->dq_priority,
				"Don't try to wake up or override a root queue");
	}
	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		return _dispatch_release_2_tailcall(dq);
	}
}

DISPATCH_NOINLINE
void
_dispatch_root_queue_push(dispatch_queue_global_t rq, dispatch_object_t dou,
		dispatch_qos_t qos)
{
#if DISPATCH_USE_KEVENT_WORKQUEUE
	dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
	if (unlikely(ddi && ddi->ddi_can_stash)) {
		dispatch_object_t old_dou = ddi->ddi_stashed_dou;
		dispatch_priority_t rq_overcommit;
		rq_overcommit = rq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;

		if (likely(!old_dou._do || rq_overcommit)) {
			dispatch_queue_global_t old_rq = ddi->ddi_stashed_rq;
			dispatch_qos_t old_qos = ddi->ddi_stashed_qos;
			ddi->ddi_stashed_rq = rq;
			ddi->ddi_stashed_dou = dou;
			ddi->ddi_stashed_qos = qos;
			_dispatch_debug("deferring item %p, rq %p, qos %d",
					dou._do, rq, qos);
			if (rq_overcommit) {
				ddi->ddi_can_stash = false;
			}
			if (likely(!old_dou._do)) {
				return;
			}
			// push the previously stashed item
			qos = old_qos;
			rq = old_rq;
			dou = old_dou;
		}
	}
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (_dispatch_root_queue_push_needs_override(rq, qos)) {
		return _dispatch_root_queue_push_override(rq, dou, qos);
	}
#else
	(void)qos;
#endif
	_dispatch_root_queue_push_inline(rq, dou, dou, 1);
}

#pragma mark -
#pragma mark dispatch_pthread_root_queue
#if DISPATCH_USE_PTHREAD_ROOT_QUEUES

static dispatch_queue_global_t
_dispatch_pthread_root_queue_create(const char *label, unsigned long flags,
		const pthread_attr_t *attr, dispatch_block_t configure,
		dispatch_pthread_root_queue_observer_hooks_t observer_hooks)
{
	dispatch_queue_pthread_root_t dpq;
	dispatch_queue_flags_t dqf = 0;
	int32_t pool_size = flags & _DISPATCH_PTHREAD_ROOT_QUEUE_FLAG_POOL_SIZE ?
			(int8_t)(flags & ~_DISPATCH_PTHREAD_ROOT_QUEUE_FLAG_POOL_SIZE) : 0;

	if (label) {
		const char *tmp = _dispatch_strdup_if_mutable(label);
		if (tmp != label) {
			dqf |= DQF_LABEL_NEEDS_FREE;
			label = tmp;
		}
	}

	dpq = _dispatch_queue_alloc(queue_pthread_root, dqf,
			DISPATCH_QUEUE_WIDTH_POOL, 0)._dpq;
	dpq->dq_label = label;
	dpq->dq_state = DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE;
	dpq->dq_priority = DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
	dpq->do_ctxt = &dpq->dpq_ctxt;

	dispatch_pthread_root_queue_context_t pqc = &dpq->dpq_ctxt;
	_dispatch_root_queue_init_pthread_pool(dpq->_as_dgq, pool_size,
			DISPATCH_PRIORITY_FLAG_OVERCOMMIT);

#if !defined(_WIN32)
	if (attr) {
		memcpy(&pqc->dpq_thread_attr, attr, sizeof(pthread_attr_t));
		_dispatch_mgr_priority_raise(&pqc->dpq_thread_attr);
	} else {
		(void)dispatch_assume_zero(pthread_attr_init(&pqc->dpq_thread_attr));
	}
	(void)dispatch_assume_zero(pthread_attr_setdetachstate(
			&pqc->dpq_thread_attr, PTHREAD_CREATE_DETACHED));
#else // defined(_WIN32)
	dispatch_assert(attr == NULL);
#endif // defined(_WIN32)
	if (configure) {
		pqc->dpq_thread_configure = _dispatch_Block_copy(configure);
	}
	if (observer_hooks) {
		pqc->dpq_observer_hooks = *observer_hooks;
	}
	_dispatch_object_debug(dpq, "%s", __func__);
	return _dispatch_trace_queue_create(dpq)._dgq;
}

dispatch_queue_global_t
dispatch_pthread_root_queue_create(const char *label, unsigned long flags,
		const pthread_attr_t *attr, dispatch_block_t configure)
{
	return _dispatch_pthread_root_queue_create(label, flags, attr, configure,
			NULL);
}

#if DISPATCH_IOHID_SPI
dispatch_queue_global_t
_dispatch_pthread_root_queue_create_with_observer_hooks_4IOHID(const char *label,
		unsigned long flags, const pthread_attr_t *attr,
		dispatch_pthread_root_queue_observer_hooks_t observer_hooks,
		dispatch_block_t configure)
{
	if (!observer_hooks->queue_will_execute ||
			!observer_hooks->queue_did_execute) {
		DISPATCH_CLIENT_CRASH(0, "Invalid pthread root queue observer hooks");
	}
	return _dispatch_pthread_root_queue_create(label, flags, attr, configure,
			observer_hooks);
}

bool
_dispatch_queue_is_exclusively_owned_by_current_thread_4IOHID(
		dispatch_queue_t dq) // rdar://problem/18033810
{
	if (dq->dq_width != 1) {
		DISPATCH_CLIENT_CRASH(dq->dq_width, "Invalid queue type");
	}
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
	return _dq_state_drain_locked_by_self(dq_state);
}
#endif

dispatch_queue_global_t
dispatch_pthread_root_queue_copy_current(void)
{
	dispatch_queue_t dq = _dispatch_queue_get_current();
	if (!dq) return NULL;
	while (unlikely(dq->do_targetq)) {
		dq = dq->do_targetq;
	}
	if (dx_type(dq) != DISPATCH_QUEUE_PTHREAD_ROOT_TYPE) {
		return NULL;
	}
	_os_object_retain_with_resurrect(dq->_as_os_obj);
	return upcast(dq)._dgq;
}

void
_dispatch_pthread_root_queue_dispose(dispatch_queue_global_t dq,
		bool *allow_free)
{
	dispatch_pthread_root_queue_context_t pqc = dq->do_ctxt;

	_dispatch_object_debug(dq, "%s", __func__);
	_dispatch_trace_queue_dispose(dq);

#if !defined(_WIN32)
	pthread_attr_destroy(&pqc->dpq_thread_attr);
#endif
	_dispatch_semaphore_dispose(&pqc->dpq_thread_mediator, NULL);
	if (pqc->dpq_thread_configure) {
		Block_release(pqc->dpq_thread_configure);
	}
	dq->do_targetq = _dispatch_get_default_queue(false);
	_dispatch_lane_class_dispose(dq, allow_free);
}

#endif // DISPATCH_USE_PTHREAD_ROOT_QUEUES
#pragma mark -
#pragma mark dispatch_runloop_queue

DISPATCH_STATIC_GLOBAL(bool _dispatch_program_is_probably_callback_driven);

#if DISPATCH_COCOA_COMPAT
DISPATCH_STATIC_GLOBAL(dispatch_once_t _dispatch_main_q_handle_pred);

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_runloop_handle_is_valid(dispatch_runloop_handle_t handle)
{
#if TARGET_OS_MAC
	return MACH_PORT_VALID(handle);
#elif defined(__linux__)
	return handle >= 0;
#else
#error "runloop support not implemented on this platform"
#endif
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_runloop_handle_t
_dispatch_runloop_queue_get_handle(dispatch_lane_t dq)
{
#if TARGET_OS_MAC
	return ((dispatch_runloop_handle_t)(uintptr_t)dq->do_ctxt);
#elif defined(__linux__)
	// decode: 0 is a valid fd, so offset by 1 to distinguish from NULL
	return ((dispatch_runloop_handle_t)(uintptr_t)dq->do_ctxt) - 1;
#else
#error "runloop support not implemented on this platform"
#endif
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_runloop_queue_set_handle(dispatch_lane_t dq,
		dispatch_runloop_handle_t handle)
{
#if TARGET_OS_MAC
	dq->do_ctxt = (void *)(uintptr_t)handle;
#elif defined(__linux__)
	// encode: 0 is a valid fd, so offset by 1 to distinguish from NULL
	dq->do_ctxt = (void *)(uintptr_t)(handle + 1);
#else
#error "runloop support not implemented on this platform"
#endif
}

static void
_dispatch_runloop_queue_handle_init(void *ctxt)
{
	dispatch_lane_t dq = (dispatch_lane_t)ctxt;
	dispatch_runloop_handle_t handle;

	_dispatch_fork_becomes_unsafe();

#if TARGET_OS_MAC
	mach_port_options_t opts = {
		.flags = MPO_CONTEXT_AS_GUARD | MPO_STRICT | MPO_INSERT_SEND_RIGHT,
	};
	mach_port_context_t guard = (uintptr_t)dq;
	kern_return_t kr;
	mach_port_t mp;

	if (dx_type(dq) == DISPATCH_QUEUE_MAIN_TYPE) {
		opts.flags |= MPO_QLIMIT;
		opts.mpl.mpl_qlimit = 1;
	}

	kr = mach_port_construct(mach_task_self(), &opts, guard, &mp);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);

	handle = mp;
#elif defined(__linux__)
	int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (fd == -1) {
		int err = errno;
		switch (err) {
		case EMFILE:
			DISPATCH_CLIENT_CRASH(err, "eventfd() failure: "
					"process is out of file descriptors");
			break;
		case ENFILE:
			DISPATCH_CLIENT_CRASH(err, "eventfd() failure: "
					"system is out of file descriptors");
			break;
		case ENOMEM:
			DISPATCH_CLIENT_CRASH(err, "eventfd() failure: "
					"kernel is out of memory");
			break;
		default:
			DISPATCH_INTERNAL_CRASH(err, "eventfd() failure");
			break;
		}
	}
	handle = fd;
#else
#error "runloop support not implemented on this platform"
#endif
	_dispatch_runloop_queue_set_handle(dq, handle);

	_dispatch_program_is_probably_callback_driven = true;
}

static void
_dispatch_runloop_queue_handle_dispose(dispatch_lane_t dq)
{
	dispatch_runloop_handle_t handle = _dispatch_runloop_queue_get_handle(dq);
	if (!_dispatch_runloop_handle_is_valid(handle)) {
		return;
	}
	dq->do_ctxt = NULL;
#if TARGET_OS_MAC
	mach_port_t mp = (mach_port_t)handle;
	mach_port_context_t guard = (uintptr_t)dq;
	kern_return_t kr;
	kr = mach_port_destruct(mach_task_self(), mp, -1, guard);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
#elif defined(__linux__)
	int rc = close(handle);
	(void)dispatch_assume_zero(rc);
#else
#error "runloop support not implemented on this platform"
#endif
}

static inline void
_dispatch_runloop_queue_class_poke(dispatch_lane_t dq)
{
	dispatch_runloop_handle_t handle = _dispatch_runloop_queue_get_handle(dq);
	if (!_dispatch_runloop_handle_is_valid(handle)) {
		return;
	}

	_dispatch_trace_runtime_event(worker_request, dq, 1);
#if HAVE_MACH
	mach_port_t mp = handle;
	kern_return_t kr = _dispatch_send_wakeup_runloop_thread(mp, 0);
	switch (kr) {
	case MACH_SEND_TIMEOUT:
	case MACH_SEND_TIMED_OUT:
	case MACH_SEND_INVALID_DEST:
		break;
	default:
		(void)dispatch_assume_zero(kr);
		break;
	}
#elif defined(__linux__)
	int result;
	do {
		result = eventfd_write(handle, 1);
	} while (result == -1 && errno == EINTR);
	(void)dispatch_assume_zero(result);
#else
#error "runloop support not implemented on this platform"
#endif
}

DISPATCH_NOINLINE
static void
_dispatch_runloop_queue_poke(dispatch_lane_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	// it's not useful to handle WAKEUP_MAKE_DIRTY because mach_msg() will have
	// a release barrier and that when runloop queues stop being thread-bound
	// they have a non optional wake-up to start being a "normal" queue
	// either in _dispatch_runloop_queue_xref_dispose,
	// or in _dispatch_queue_cleanup2() for the main thread.
	uint64_t old_state, new_state;

	if (dx_type(dq) == DISPATCH_QUEUE_MAIN_TYPE) {
		dispatch_once_f(&_dispatch_main_q_handle_pred, dq,
				_dispatch_runloop_queue_handle_init);
	}

	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, relaxed, {
		new_state = _dq_state_merge_qos(old_state, qos);
		if (old_state == new_state) {
			os_atomic_rmw_loop_give_up(goto no_change);
		}
	});

	dispatch_qos_t dq_qos = _dispatch_priority_qos(dq->dq_priority);
	if (qos > dq_qos) {
		mach_port_t owner = _dq_state_drain_owner(new_state);
		pthread_priority_t pp = _dispatch_qos_to_pp(qos);
		_dispatch_thread_override_start(owner, pp, dq);
		if (_dq_state_max_qos(old_state) > dq_qos) {
			_dispatch_thread_override_end(owner, dq);
		}
	}
no_change:
	_dispatch_runloop_queue_class_poke(dq);
	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		return _dispatch_release_2_tailcall(dq);
	}
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_runloop_queue_reset_max_qos(dispatch_lane_t dq)
{
	uint64_t old_state, clear_bits = DISPATCH_QUEUE_MAX_QOS_MASK |
			DISPATCH_QUEUE_RECEIVED_OVERRIDE;
	old_state = os_atomic_and_orig2o(dq, dq_state, ~clear_bits, relaxed);
	return _dq_state_max_qos(old_state);
}

void
_dispatch_runloop_queue_wakeup(dispatch_lane_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	if (unlikely(_dispatch_queue_atomic_flags(dq) & DQF_RELEASED)) {
		// <rdar://problem/14026816>
		return _dispatch_lane_wakeup(dq, qos, flags);
	}

	if (flags & DISPATCH_WAKEUP_MAKE_DIRTY) {
		os_atomic_or2o(dq, dq_state, DISPATCH_QUEUE_DIRTY, release);
	}
	if (_dispatch_queue_class_probe(dq)) {
		return _dispatch_runloop_queue_poke(dq, qos, flags);
	}

	qos = _dispatch_runloop_queue_reset_max_qos(dq);
	if (qos) {
		mach_port_t owner = DISPATCH_QUEUE_DRAIN_OWNER(dq);
		if (_dispatch_queue_class_probe(dq)) {
			_dispatch_runloop_queue_poke(dq, qos, flags);
		}
		_dispatch_thread_override_end(owner, dq);
		return;
	}
	if (flags & DISPATCH_WAKEUP_CONSUME_2) {
		return _dispatch_release_2_tailcall(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_main_queue_update_priority_from_thread(void)
{
	dispatch_queue_main_t dq = &_dispatch_main_q;
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
	mach_port_t owner = _dq_state_drain_owner(dq_state);

	dispatch_priority_t main_pri =
			_dispatch_priority_from_pp_strip_flags(_dispatch_get_priority());
	dispatch_qos_t main_qos = _dispatch_priority_qos(main_pri);
	dispatch_qos_t max_qos = _dq_state_max_qos(dq_state);
	dispatch_qos_t old_qos = _dispatch_priority_qos(dq->dq_priority);

	// the main thread QoS was adjusted by someone else, learn the new QoS
	// and reinitialize _dispatch_main_q.dq_priority
	dq->dq_priority = main_pri;

	if (old_qos < max_qos && main_qos == DISPATCH_QOS_UNSPECIFIED) {
		// main thread is opted out of QoS and we had an override
		return _dispatch_thread_override_end(owner, dq);
	}

	if (old_qos < max_qos && max_qos <= main_qos) {
		// main QoS was raised, and we had an override which is now useless
		return _dispatch_thread_override_end(owner, dq);
	}

	if (main_qos < max_qos && max_qos <= old_qos) {
		// main thread QoS was lowered, and we actually need an override
		pthread_priority_t pp = _dispatch_qos_to_pp(max_qos);
		return _dispatch_thread_override_start(owner, pp, dq);
	}
}

static void
_dispatch_main_queue_drain(dispatch_queue_main_t dq)
{
	dispatch_thread_frame_s dtf;

	if (!dq->dq_items_tail) {
		return;
	}

	_dispatch_perfmon_start_notrace();
	if (unlikely(!_dispatch_queue_is_thread_bound(dq))) {
		DISPATCH_CLIENT_CRASH(0, "_dispatch_main_queue_callback_4CF called"
				" after dispatch_main()");
	}
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);
	if (unlikely(!_dq_state_drain_locked_by_self(dq_state))) {
		DISPATCH_CLIENT_CRASH((uintptr_t)dq_state,
				"_dispatch_main_queue_callback_4CF called"
				" from the wrong thread");
	}

	dispatch_once_f(&_dispatch_main_q_handle_pred, dq,
			_dispatch_runloop_queue_handle_init);

	// <rdar://problem/23256682> hide the frame chaining when CFRunLoop
	// drains the main runloop, as this should not be observable that way
	_dispatch_adopt_wlh_anon();
	_dispatch_thread_frame_push_and_rebase(&dtf, dq, NULL);

	pthread_priority_t pp = _dispatch_get_priority();
	dispatch_priority_t pri = _dispatch_priority_from_pp(pp);
	dispatch_qos_t qos = _dispatch_priority_qos(pri);
	voucher_t voucher = _voucher_copy();

	if (unlikely(qos != _dispatch_priority_qos(dq->dq_priority))) {
		_dispatch_main_queue_update_priority_from_thread();
	}
	dispatch_priority_t old_dbp = _dispatch_set_basepri(pri);
	_dispatch_set_basepri_override_qos(DISPATCH_QOS_SATURATED);

	dispatch_invoke_context_s dic = { };
	struct dispatch_object_s *dc, *next_dc, *tail;
	dc = os_mpsc_capture_snapshot(os_mpsc(dq, dq_items), &tail);
	do {
		next_dc = os_mpsc_pop_snapshot_head(dc, tail, do_next);
		_dispatch_continuation_pop_inline(dc, &dic,
				DISPATCH_INVOKE_THREAD_BOUND, dq);
	} while ((dc = next_dc));

	dx_wakeup(dq->_as_dq, 0, 0);
	_dispatch_voucher_debug("main queue restore", voucher);
	_dispatch_reset_basepri(old_dbp);
	_dispatch_reset_basepri_override();
	_dispatch_reset_priority_and_voucher(pp, voucher);
	_dispatch_thread_frame_pop(&dtf);
	_dispatch_reset_wlh();
	_dispatch_force_cache_cleanup();
	_dispatch_perfmon_end_notrace();
}

static bool
_dispatch_runloop_queue_drain_one(dispatch_lane_t dq)
{
	if (!dq->dq_items_tail) {
		return false;
	}
	_dispatch_perfmon_start_notrace();
	dispatch_thread_frame_s dtf;
	bool should_reset_wlh = _dispatch_adopt_wlh_anon_recurse();
	_dispatch_thread_frame_push(&dtf, dq);
	pthread_priority_t pp = _dispatch_get_priority();
	dispatch_priority_t pri = _dispatch_priority_from_pp(pp);
	voucher_t voucher = _voucher_copy();
	dispatch_priority_t old_dbp = _dispatch_set_basepri(pri);
	_dispatch_set_basepri_override_qos(DISPATCH_QOS_SATURATED);

	dispatch_invoke_context_s dic = { };
	struct dispatch_object_s *dc, *next_dc;
	dc = _dispatch_queue_get_head(dq);
	next_dc = _dispatch_queue_pop_head(dq, dc);
	_dispatch_continuation_pop_inline(dc, &dic,
			DISPATCH_INVOKE_THREAD_BOUND, dq);

	if (!next_dc) {
		dx_wakeup(dq, 0, 0);
	}

	_dispatch_voucher_debug("runloop queue restore", voucher);
	_dispatch_reset_basepri(old_dbp);
	_dispatch_reset_basepri_override();
	_dispatch_reset_priority_and_voucher(pp, voucher);
	_dispatch_thread_frame_pop(&dtf);
	if (should_reset_wlh) _dispatch_reset_wlh();
	_dispatch_force_cache_cleanup();
	_dispatch_perfmon_end_notrace();
	return next_dc;
}

dispatch_queue_serial_t
_dispatch_runloop_root_queue_create_4CF(const char *label, unsigned long flags)
{
	pthread_priority_t pp = _dispatch_get_priority();
	dispatch_lane_t dq;

	if (unlikely(flags)) {
		return DISPATCH_BAD_INPUT;
	}
	dq = _dispatch_object_alloc(DISPATCH_VTABLE(queue_runloop),
			sizeof(struct dispatch_lane_s));
	_dispatch_queue_init(dq, DQF_THREAD_BOUND, 1,
			DISPATCH_QUEUE_ROLE_BASE_ANON);
	dq->do_targetq = _dispatch_get_default_queue(true);
	dq->dq_label = label ? label : "runloop-queue"; // no-copy contract
	if (pp & _PTHREAD_PRIORITY_QOS_CLASS_MASK) {
		dq->dq_priority = _dispatch_priority_from_pp_strip_flags(pp);
	}
	_dispatch_runloop_queue_handle_init(dq);
	_dispatch_queue_set_bound_thread(dq);
	_dispatch_object_debug(dq, "%s", __func__);
	return _dispatch_trace_queue_create(dq)._dl;
}

void
_dispatch_runloop_queue_xref_dispose(dispatch_lane_t dq)
{
	_dispatch_object_debug(dq, "%s", __func__);

	dispatch_qos_t qos = _dispatch_runloop_queue_reset_max_qos(dq);
	_dispatch_queue_clear_bound_thread(dq);
	dx_wakeup(dq, qos, DISPATCH_WAKEUP_MAKE_DIRTY);
	if (qos) _dispatch_thread_override_end(DISPATCH_QUEUE_DRAIN_OWNER(dq), dq);
}

void
_dispatch_runloop_queue_dispose(dispatch_lane_t dq, bool *allow_free)
{
	_dispatch_object_debug(dq, "%s", __func__);
	_dispatch_trace_queue_dispose(dq);
	_dispatch_runloop_queue_handle_dispose(dq);
	_dispatch_lane_class_dispose(dq, allow_free);
}

bool
_dispatch_runloop_root_queue_perform_4CF(dispatch_queue_t dq)
{
	if (unlikely(dx_type(dq) != DISPATCH_QUEUE_RUNLOOP_TYPE)) {
		DISPATCH_CLIENT_CRASH(dx_type(dq), "Not a runloop queue");
	}
	dispatch_retain(dq);
	bool r = _dispatch_runloop_queue_drain_one(upcast(dq)._dl);
	dispatch_release(dq);
	return r;
}

void
_dispatch_runloop_root_queue_wakeup_4CF(dispatch_queue_t dq)
{
	if (unlikely(dx_type(dq) != DISPATCH_QUEUE_RUNLOOP_TYPE)) {
		DISPATCH_CLIENT_CRASH(dx_type(dq), "Not a runloop queue");
	}
	_dispatch_runloop_queue_wakeup(upcast(dq)._dl, 0, false);
}

#if TARGET_OS_MAC
dispatch_runloop_handle_t
_dispatch_runloop_root_queue_get_port_4CF(dispatch_queue_t dq)
{
	if (unlikely(dx_type(dq) != DISPATCH_QUEUE_RUNLOOP_TYPE)) {
		DISPATCH_CLIENT_CRASH(dx_type(dq), "Not a runloop queue");
	}
	return _dispatch_runloop_queue_get_handle(upcast(dq)._dl);
}
#endif

#endif // DISPATCH_COCOA_COMPAT
#pragma mark -
#pragma mark dispatch_main_queue
#if DISPATCH_COCOA_COMPAT

dispatch_runloop_handle_t
_dispatch_get_main_queue_handle_4CF(void)
{
	dispatch_queue_main_t dq = &_dispatch_main_q;
	dispatch_once_f(&_dispatch_main_q_handle_pred, dq,
			_dispatch_runloop_queue_handle_init);
	return _dispatch_runloop_queue_get_handle(dq->_as_dl);
}

#if TARGET_OS_MAC
dispatch_runloop_handle_t
_dispatch_get_main_queue_port_4CF(void)
{
	return _dispatch_get_main_queue_handle_4CF();
}
#endif

void
_dispatch_main_queue_callback_4CF(
		void *ignored DISPATCH_UNUSED)
{
	// the main queue cannot be suspended and no-one looks at this bit
	// so abuse it to avoid dirtying more memory

	if (_dispatch_main_q.dq_side_suspend_cnt) {
		return;
	}
	_dispatch_main_q.dq_side_suspend_cnt = true;
	_dispatch_main_queue_drain(&_dispatch_main_q);
	_dispatch_main_q.dq_side_suspend_cnt = false;
}

#endif // DISPATCH_COCOA_COMPAT

DISPATCH_NOINLINE
void
_dispatch_main_queue_push(dispatch_queue_main_t dq, dispatch_object_t dou,
		dispatch_qos_t qos)
{
	// Same as _dispatch_lane_push() but without the refcounting due to being
	// a global object
	if (_dispatch_queue_push_item(dq, dou)) {
		return dx_wakeup(dq, qos, DISPATCH_WAKEUP_MAKE_DIRTY);
	}

	qos = _dispatch_queue_push_qos(dq, qos);
	if (_dispatch_queue_need_override(dq, qos)) {
		return dx_wakeup(dq, qos, 0);
	}
}

void
_dispatch_main_queue_wakeup(dispatch_queue_main_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
#if DISPATCH_COCOA_COMPAT
	if (_dispatch_queue_is_thread_bound(dq)) {
		return _dispatch_runloop_queue_wakeup(dq->_as_dl, qos, flags);
	}
#endif
	return _dispatch_lane_wakeup(dq, qos, flags);
}

#if !defined(_WIN32)
DISPATCH_NOINLINE DISPATCH_NORETURN
static void
_dispatch_sigsuspend(void)
{
	static const sigset_t mask;
	pthread_sigmask(SIG_SETMASK, &mask, NULL);
	for (;;) {
		sigsuspend(&mask);
	}
}
#endif // !defined(_WIN32)

DISPATCH_NORETURN
static void
_dispatch_sig_thread(void *ctxt DISPATCH_UNUSED)
{
	// never returns, so burn bridges behind us
	_dispatch_clear_stack(0);
#if !defined(_WIN32)
	_dispatch_sigsuspend();
#endif
}

void
dispatch_main(void)
{
	_dispatch_root_queues_init();
#if HAVE_PTHREAD_MAIN_NP
	if (pthread_main_np()) {
#endif
		_dispatch_object_debug(&_dispatch_main_q, "%s", __func__);
		_dispatch_program_is_probably_callback_driven = true;
		_dispatch_ktrace0(ARIADNE_ENTER_DISPATCH_MAIN_CODE);
#ifdef __linux__
		// On Linux, if the main thread calls pthread_exit, the process becomes a zombie.
		// To avoid that, just before calling pthread_exit we register a TSD destructor
		// that will call _dispatch_sig_thread -- thus capturing the main thread in sigsuspend.
		// This relies on an implementation detail (currently true in glibc) that TSD destructors
		// will be called in the order of creation to cause all the TSD cleanup functions to
		// run before the thread becomes trapped in sigsuspend.
		pthread_key_t dispatch_main_key;
		pthread_key_create(&dispatch_main_key, _dispatch_sig_thread);
		pthread_setspecific(dispatch_main_key, &dispatch_main_key);
		_dispatch_sigmask();
#endif
#if !defined(_WIN32)
		pthread_exit(NULL);
#else
		_endthreadex(0);
#endif // defined(_WIN32)
		DISPATCH_INTERNAL_CRASH(errno, "pthread_exit() returned");
#if HAVE_PTHREAD_MAIN_NP
	}
	DISPATCH_CLIENT_CRASH(0, "dispatch_main() must be called on the main thread");
#endif
}

DISPATCH_NOINLINE
static void
_dispatch_queue_cleanup2(void)
{
	dispatch_queue_main_t dq = &_dispatch_main_q;
	uint64_t old_state, new_state;

	// Turning the main queue from a runloop queue into an ordinary serial queue
	// is a 3 steps operation:
	// 1. finish taking the main queue lock the usual way
	// 2. clear the THREAD_BOUND flag
	// 3. do a handoff
	//
	// If an enqueuer executes concurrently, he may do the wakeup the runloop
	// way, because he still believes the queue to be thread-bound, but the
	// dirty bit will force this codepath to notice the enqueue, and the usual
	// lock transfer will do the proper wakeup.
	os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, acquire, {
		new_state = old_state & ~DISPATCH_QUEUE_DIRTY;
		new_state += DISPATCH_QUEUE_WIDTH_INTERVAL;
		new_state += DISPATCH_QUEUE_IN_BARRIER;
	});
	_dispatch_queue_atomic_flags_clear(dq, DQF_THREAD_BOUND);
	_dispatch_lane_barrier_complete(dq, 0, 0);

	// overload the "probably" variable to mean that dispatch_main() or
	// similar non-POSIX API was called
	// this has to run before the DISPATCH_COCOA_COMPAT below
	// See dispatch_main for call to _dispatch_sig_thread on linux.
#ifndef __linux__
	if (_dispatch_program_is_probably_callback_driven) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_t tid;
		int r = pthread_create(&tid, &attr, (void*)_dispatch_sig_thread, NULL);
		if (unlikely(r)) {
			DISPATCH_CLIENT_CRASH(r, "Unable to create signal thread");
		}
		pthread_attr_destroy(&attr);
		// this used to be here as a workaround for 6778970
		// but removing it had bincompat fallouts :'(
		sleep(1);
	}
#endif

#if DISPATCH_COCOA_COMPAT
	dispatch_once_f(&_dispatch_main_q_handle_pred, dq,
			_dispatch_runloop_queue_handle_init);
	_dispatch_runloop_queue_handle_dispose(dq->_as_dl);
#endif
}

static void DISPATCH_TSD_DTOR_CC
_dispatch_queue_cleanup(void *ctxt)
{
	if (ctxt == &_dispatch_main_q) {
		return _dispatch_queue_cleanup2();
	}
	// POSIX defines that destructors are only called if 'ctxt' is non-null
	DISPATCH_INTERNAL_CRASH(ctxt,
			"Premature thread exit while a dispatch queue is running");
}

static void DISPATCH_TSD_DTOR_CC
_dispatch_wlh_cleanup(void *ctxt)
{
	// POSIX defines that destructors are only called if 'ctxt' is non-null
	dispatch_queue_t wlh;
	wlh = (dispatch_queue_t)((uintptr_t)ctxt & ~DISPATCH_WLH_STORAGE_REF);
	_dispatch_queue_release_storage(wlh);
}

DISPATCH_NORETURN
static void DISPATCH_TSD_DTOR_CC
_dispatch_deferred_items_cleanup(void *ctxt)
{
	// POSIX defines that destructors are only called if 'ctxt' is non-null
	DISPATCH_INTERNAL_CRASH(ctxt,
			"Premature thread exit with unhandled deferred items");
}

DISPATCH_NORETURN
static DISPATCH_TSD_DTOR_CC void
_dispatch_frame_cleanup(void *ctxt)
{
	// POSIX defines that destructors are only called if 'ctxt' is non-null
	DISPATCH_INTERNAL_CRASH(ctxt,
			"Premature thread exit while a dispatch frame is active");
}

DISPATCH_NORETURN
static void DISPATCH_TSD_DTOR_CC
_dispatch_context_cleanup(void *ctxt)
{
	// POSIX defines that destructors are only called if 'ctxt' is non-null
	DISPATCH_INTERNAL_CRASH(ctxt,
			"Premature thread exit while a dispatch context is set");
}
#pragma mark -
#pragma mark dispatch_init

static void
_dispatch_root_queues_init_once(void *context DISPATCH_UNUSED)
{
	_dispatch_fork_becomes_unsafe();
#if DISPATCH_USE_INTERNAL_WORKQUEUE
	size_t i;
	for (i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
		_dispatch_root_queue_init_pthread_pool(&_dispatch_root_queues[i], 0,
				_dispatch_root_queues[i].dq_priority);
	}
#else
	int wq_supported = _pthread_workqueue_supported();
	int r = ENOTSUP;

	if (!(wq_supported & WORKQ_FEATURE_MAINTENANCE)) {
		DISPATCH_INTERNAL_CRASH(wq_supported,
				"QoS Maintenance support required");
	}

#if DISPATCH_USE_KEVENT_SETUP
	struct pthread_workqueue_config cfg = {
		.version = PTHREAD_WORKQUEUE_CONFIG_VERSION,
		.flags = 0,
		.workq_cb = 0,
		.kevent_cb = 0,
		.workloop_cb = 0,
		.queue_serialno_offs = dispatch_queue_offsets.dqo_serialnum,
#if PTHREAD_WORKQUEUE_CONFIG_VERSION >= 2
		.queue_label_offs = dispatch_queue_offsets.dqo_label,
#endif
	};
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
	if (unlikely(!_dispatch_kevent_workqueue_enabled)) {
#if DISPATCH_USE_KEVENT_SETUP
		cfg.workq_cb = _dispatch_worker_thread2;
		r = pthread_workqueue_setup(&cfg, sizeof(cfg));
#else
		r = _pthread_workqueue_init(_dispatch_worker_thread2,
				offsetof(struct dispatch_queue_s, dq_serialnum), 0);
#endif // DISPATCH_USE_KEVENT_SETUP
#if DISPATCH_USE_KEVENT_WORKLOOP
	} else if (wq_supported & WORKQ_FEATURE_WORKLOOP) {
#if DISPATCH_USE_KEVENT_SETUP
		cfg.workq_cb = _dispatch_worker_thread2;
		cfg.kevent_cb = (pthread_workqueue_function_kevent_t) _dispatch_kevent_worker_thread;
		cfg.workloop_cb = (pthread_workqueue_function_workloop_t) _dispatch_workloop_worker_thread;
		r = pthread_workqueue_setup(&cfg, sizeof(cfg));
#else
		r = _pthread_workqueue_init_with_workloop(_dispatch_worker_thread2,
				(pthread_workqueue_function_kevent_t)
				_dispatch_kevent_worker_thread,
				(pthread_workqueue_function_workloop_t)
				_dispatch_workloop_worker_thread,
				offsetof(struct dispatch_queue_s, dq_serialnum), 0);
#endif // DISPATCH_USE_KEVENT_SETUP
#endif // DISPATCH_USE_KEVENT_WORKLOOP
#if DISPATCH_USE_KEVENT_WORKQUEUE
	} else if (wq_supported & WORKQ_FEATURE_KEVENT) {
#if DISPATCH_USE_KEVENT_SETUP
		cfg.workq_cb = _dispatch_worker_thread2;
		cfg.kevent_cb = (pthread_workqueue_function_kevent_t) _dispatch_kevent_worker_thread;
		r = pthread_workqueue_setup(&cfg, sizeof(cfg));
#else
		r = _pthread_workqueue_init_with_kevent(_dispatch_worker_thread2,
				(pthread_workqueue_function_kevent_t)
				_dispatch_kevent_worker_thread,
				offsetof(struct dispatch_queue_s, dq_serialnum), 0);
#endif // DISPATCH_USE_KEVENT_SETUP
#endif
	} else {
		DISPATCH_INTERNAL_CRASH(wq_supported, "Missing Kevent WORKQ support");
	}
#pragma clang diagnostic pop

	if (r != 0) {
		DISPATCH_INTERNAL_CRASH((r << 16) | wq_supported,
				"Root queue initialization failed");
	}
#endif // DISPATCH_USE_INTERNAL_WORKQUEUE
}

DISPATCH_STATIC_GLOBAL(dispatch_once_t _dispatch_root_queues_pred);
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_root_queues_init(void)
{
	dispatch_once_f(&_dispatch_root_queues_pred, NULL,
			_dispatch_root_queues_init_once);
}

DISPATCH_EXPORT DISPATCH_NOTHROW
void
libdispatch_init(void)
{
	dispatch_assert(sizeof(struct dispatch_apply_s) <=
			DISPATCH_CONTINUATION_SIZE);

	if (_dispatch_getenv_bool("LIBDISPATCH_STRICT", false)) {
		_dispatch_mode |= DISPATCH_MODE_STRICT;
	}
#if HAVE_OS_FAULT_WITH_PAYLOAD && TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
	if (_dispatch_getenv_bool("LIBDISPATCH_NO_FAULTS", false)) {
		_dispatch_mode |= DISPATCH_MODE_NO_FAULTS;
	} else if (getpid() == 1 ||
			!os_variant_has_internal_diagnostics("com.apple.libdispatch")) {
		_dispatch_mode |= DISPATCH_MODE_NO_FAULTS;
	}
#endif // HAVE_OS_FAULT_WITH_PAYLOAD && TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR


#if DISPATCH_DEBUG || DISPATCH_PROFILE
#if DISPATCH_USE_KEVENT_WORKQUEUE
	if (getenv("LIBDISPATCH_DISABLE_KEVENT_WQ")) {
		_dispatch_kevent_workqueue_enabled = false;
	}
#endif
#endif

#if HAVE_PTHREAD_WORKQUEUE_QOS
	dispatch_qos_t qos = _dispatch_qos_from_qos_class(qos_class_main());
	_dispatch_main_q.dq_priority = _dispatch_priority_make(qos, 0);
#if DISPATCH_DEBUG
	if (!getenv("LIBDISPATCH_DISABLE_SET_QOS")) {
		_dispatch_set_qos_class_enabled = 1;
	}
#endif
#endif

#if DISPATCH_USE_THREAD_LOCAL_STORAGE
	_dispatch_thread_key_create(&__dispatch_tsd_key, _libdispatch_tsd_cleanup);
#else
	_dispatch_thread_key_create(&dispatch_priority_key, NULL);
	_dispatch_thread_key_create(&dispatch_r2k_key, NULL);
	_dispatch_thread_key_create(&dispatch_queue_key, _dispatch_queue_cleanup);
	_dispatch_thread_key_create(&dispatch_frame_key, _dispatch_frame_cleanup);
	_dispatch_thread_key_create(&dispatch_cache_key, _dispatch_cache_cleanup);
	_dispatch_thread_key_create(&dispatch_context_key, _dispatch_context_cleanup);
	_dispatch_thread_key_create(&dispatch_pthread_root_queue_observer_hooks_key,
			NULL);
	_dispatch_thread_key_create(&dispatch_basepri_key, NULL);
#if DISPATCH_INTROSPECTION
	_dispatch_thread_key_create(&dispatch_introspection_key , NULL);
#elif DISPATCH_PERF_MON
	_dispatch_thread_key_create(&dispatch_bcounter_key, NULL);
#endif
	_dispatch_thread_key_create(&dispatch_wlh_key, _dispatch_wlh_cleanup);
	_dispatch_thread_key_create(&dispatch_voucher_key, _voucher_thread_cleanup);
	_dispatch_thread_key_create(&dispatch_deferred_items_key,
			_dispatch_deferred_items_cleanup);
#endif

#if DISPATCH_USE_RESOLVERS // rdar://problem/8541707
	_dispatch_main_q.do_targetq = _dispatch_get_default_queue(true);
#endif

	_dispatch_queue_set_current(&_dispatch_main_q);
	_dispatch_queue_set_bound_thread(&_dispatch_main_q);

#if DISPATCH_USE_PTHREAD_ATFORK
	(void)dispatch_assume_zero(pthread_atfork(dispatch_atfork_prepare,
			dispatch_atfork_parent, dispatch_atfork_child));
#endif
	_dispatch_hw_config_init();
	_dispatch_time_init();
	_dispatch_vtable_init();
	_os_object_init();
	_voucher_init();
	_dispatch_introspection_init();
}

#if DISPATCH_USE_THREAD_LOCAL_STORAGE
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif
#if !defined(_WIN32)
#include <sys/syscall.h>
#endif

#ifndef __ANDROID__
#ifdef SYS_gettid
DISPATCH_ALWAYS_INLINE
static inline pid_t
gettid(void)
{
	return (pid_t)syscall(SYS_gettid);
}
#elif defined(__FreeBSD__)
DISPATCH_ALWAYS_INLINE
static inline pid_t
gettid(void)
{
	return (pid_t)pthread_getthreadid_np();
}
#elif defined(_WIN32)
DISPATCH_ALWAYS_INLINE
static inline DWORD
gettid(void)
{
	return GetCurrentThreadId();
}
#else
#error "SYS_gettid unavailable on this system"
#endif /* SYS_gettid */
#endif /* ! __ANDROID__ */

#define _tsd_call_cleanup(k, f)  do { \
		if ((f) && tsd->k) ((void(*)(void*))(f))(tsd->k); \
	} while (0)

#ifdef __ANDROID__
static void (*_dispatch_thread_detach_callback)(void);

void
_dispatch_install_thread_detach_callback(dispatch_function_t cb)
{
	if (os_atomic_xchg(&_dispatch_thread_detach_callback, cb, relaxed)) {
		DISPATCH_CLIENT_CRASH(0, "Installing a thread detach callback twice");
	}
}
#endif

#if defined(_WIN32)
static bool
_dispatch_process_is_exiting(void)
{
   // The goal here is to detect if the current thread is executing cleanup
   // code (e.g. FLS destructors) as a result of calling ExitProcess(). Windows
   // doesn't provide an official method of getting this information, so we
   // take advantage of how ExitProcess() works internally. The first thing
   // that it does (according to MSDN) is terminate every other thread in the
   // process. Logically, it should not be possible to create more threads
   // after this point, and Windows indeed enforces this. Try to create a
   // lightweight suspended thread, and if access is denied, assume that this
   // is because the process is exiting.
   //
   // We aren't worried about any race conditions here during process exit.
   // Cleanup code is only run on the thread that already called ExitProcess(),
   // and every other thread will have been forcibly terminated by the time
   // that happens. Additionally, while CreateThread() could conceivably fail
   // due to resource exhaustion, the process would already be in a bad state
   // if that happens. This is only intended to prevent unwanted cleanup code
   // from running, so the worst case is that a thread doesn't clean up after
   // itself when the process is about to die anyway.
   const size_t stack_size = 1;  // As small as possible
   HANDLE thread = CreateThread(NULL, stack_size, NULL, NULL,
           CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
   if (thread) {
       // Although Microsoft recommends against using TerminateThread, it's
       // safe to use it here because we know that the thread is suspended and
       // it has not executed any code due to a NULL lpStartAddress. There was
       // a bug in Windows Server 2003 and Windows XP where the initial stack
       // would not be freed, but libdispatch does not support them anyway.
       TerminateThread(thread, 0);
       CloseHandle(thread);
       return false;
   }
   return GetLastError() == ERROR_ACCESS_DENIED;
}
#endif // defined(_WIN32)


void DISPATCH_TSD_DTOR_CC
_libdispatch_tsd_cleanup(void *ctx)
{
#if defined(_WIN32)
   // On Windows, exiting a process will still call FLS destructors for the
   // thread that called ExitProcess(). pthreads-based platforms don't call key
   // destructors on exit, so be consistent.
   if (_dispatch_process_is_exiting()) {
       return;
   }
#endif // defined(_WIN32)

	struct dispatch_tsd *tsd = (struct dispatch_tsd*) ctx;

	_tsd_call_cleanup(dispatch_priority_key, NULL);
	_tsd_call_cleanup(dispatch_r2k_key, NULL);

	_tsd_call_cleanup(dispatch_queue_key, _dispatch_queue_cleanup);
	_tsd_call_cleanup(dispatch_frame_key, _dispatch_frame_cleanup);
	_tsd_call_cleanup(dispatch_cache_key, _dispatch_cache_cleanup);
	_tsd_call_cleanup(dispatch_context_key, _dispatch_context_cleanup);
	_tsd_call_cleanup(dispatch_pthread_root_queue_observer_hooks_key,
			NULL);
	_tsd_call_cleanup(dispatch_basepri_key, NULL);
#if DISPATCH_INTROSPECTION
	_tsd_call_cleanup(dispatch_introspection_key, NULL);
#elif DISPATCH_PERF_MON
	_tsd_call_cleanup(dispatch_bcounter_key, NULL);
#endif
	_tsd_call_cleanup(dispatch_wlh_key, _dispatch_wlh_cleanup);
	_tsd_call_cleanup(dispatch_voucher_key, _voucher_thread_cleanup);
	_tsd_call_cleanup(dispatch_deferred_items_key,
			_dispatch_deferred_items_cleanup);
#ifdef __ANDROID__
	if (_dispatch_thread_detach_callback) {
		_dispatch_thread_detach_callback();
	}
#endif
	tsd->tid = 0;
}

DISPATCH_NOINLINE
void
libdispatch_tsd_init(void)
{
#if !defined(_WIN32)
	pthread_setspecific(__dispatch_tsd_key, &__dispatch_tsd);
#else
	FlsSetValue(__dispatch_tsd_key, &__dispatch_tsd);
#endif // defined(_WIN32)
	__dispatch_tsd.tid = gettid();
}
#endif

DISPATCH_NOTHROW
void
_dispatch_queue_atfork_child(void)
{
	dispatch_queue_main_t main_q = &_dispatch_main_q;
	void *crash = (void *)0x100;
	size_t i;

	if (_dispatch_queue_is_thread_bound(main_q)) {
		_dispatch_queue_set_bound_thread(main_q);
	}

	if (!_dispatch_is_multithreaded_inline()) return;

	main_q->dq_items_head = crash;
	main_q->dq_items_tail = crash;

	_dispatch_mgr_q.dq_items_head = crash;
	_dispatch_mgr_q.dq_items_tail = crash;

	for (i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
		_dispatch_root_queues[i].dq_items_head = crash;
		_dispatch_root_queues[i].dq_items_tail = crash;
	}
}

DISPATCH_NOINLINE
void
_dispatch_fork_becomes_unsafe_slow(void)
{
	uint8_t value = os_atomic_or(&_dispatch_unsafe_fork,
			_DISPATCH_UNSAFE_FORK_MULTITHREADED, relaxed);
	if (value & _DISPATCH_UNSAFE_FORK_PROHIBIT) {
		DISPATCH_CLIENT_CRASH(0, "Transition to multithreaded is prohibited");
	}
}

DISPATCH_NOINLINE
void
_dispatch_prohibit_transition_to_multithreaded(bool prohibit)
{
	if (prohibit) {
		uint8_t value = os_atomic_or(&_dispatch_unsafe_fork,
				_DISPATCH_UNSAFE_FORK_PROHIBIT, relaxed);
		if (value & _DISPATCH_UNSAFE_FORK_MULTITHREADED) {
			DISPATCH_CLIENT_CRASH(0, "The executable is already multithreaded");
		}
	} else {
		os_atomic_and(&_dispatch_unsafe_fork,
				(uint8_t)~_DISPATCH_UNSAFE_FORK_PROHIBIT, relaxed);
	}
}
