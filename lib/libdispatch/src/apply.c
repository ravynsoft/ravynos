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

typedef void (*dispatch_apply_function_t)(void *, size_t);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_apply_invoke2(void *ctxt, bool redirect)
{
	dispatch_apply_t da = (dispatch_apply_t)ctxt;
	size_t const iter = da->da_iterations;
	size_t idx, done = 0;

	idx = dispatch_atomic_inc_orig2o(da, da_index, acquire);
	if (!fastpath(idx < iter)) goto out;

	// da_dc is only safe to access once the 'index lock' has been acquired
	dispatch_apply_function_t const func = (void *)da->da_dc->dc_func;
	void *const da_ctxt = da->da_dc->dc_ctxt;
	dispatch_queue_t dq = da->da_dc->dc_data;

	_dispatch_perfmon_workitem_dec(); // this unit executes many items

	// Handle nested dispatch_apply rdar://problem/9294578
	size_t nested = (size_t)_dispatch_thread_getspecific(dispatch_apply_key);
	_dispatch_thread_setspecific(dispatch_apply_key, (void*)da->da_nested);

	dispatch_queue_t old_dq;
	pthread_priority_t old_dp;
	if (redirect) {
		old_dq = _dispatch_thread_getspecific(dispatch_queue_key);
		_dispatch_thread_setspecific(dispatch_queue_key, dq);
		old_dp = _dispatch_set_defaultpriority(dq->dq_priority);
	}

	// Striding is the responsibility of the caller.
	do {
		_dispatch_client_callout2(da_ctxt, idx, func);
		_dispatch_perfmon_workitem_inc();
		done++;
		idx = dispatch_atomic_inc_orig2o(da, da_index, relaxed);
	} while (fastpath(idx < iter));

	if (redirect) {
		_dispatch_reset_defaultpriority(old_dp);
		_dispatch_thread_setspecific(dispatch_queue_key, old_dq);
	}
	_dispatch_thread_setspecific(dispatch_apply_key, (void*)nested);

	// The thread that finished the last workitem wakes up the possibly waiting
	// thread that called dispatch_apply. They could be one and the same.
	if (!dispatch_atomic_sub2o(da, da_todo, done, release)) {
		_dispatch_thread_semaphore_signal(da->da_sema);
	}
out:
	if (dispatch_atomic_dec2o(da, da_thr_cnt, release) == 0) {
		_dispatch_continuation_free((dispatch_continuation_t)da);
	}
}

DISPATCH_NOINLINE
void
_dispatch_apply_invoke(void *ctxt)
{
	_dispatch_apply_invoke2(ctxt, false);
}

DISPATCH_NOINLINE
void
_dispatch_apply_redirect_invoke(void *ctxt)
{
	_dispatch_apply_invoke2(ctxt, true);
}

static void
_dispatch_apply_serial(void *ctxt)
{
	dispatch_apply_t da = (dispatch_apply_t)ctxt;
	dispatch_continuation_t dc = da->da_dc;
	size_t const iter = da->da_iterations;
	size_t idx = 0;

	_dispatch_perfmon_workitem_dec(); // this unit executes many items
	do {
		_dispatch_client_callout2(dc->dc_ctxt, idx, (void*)dc->dc_func);
		_dispatch_perfmon_workitem_inc();
	} while (++idx < iter);

	_dispatch_continuation_free((dispatch_continuation_t)da);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_apply_f2(dispatch_queue_t dq, dispatch_apply_t da,
		dispatch_function_t func)
{
	uint32_t i = 0;
	dispatch_continuation_t head = NULL, tail = NULL;

	// The current thread does not need a continuation
	uint32_t continuation_cnt = da->da_thr_cnt - 1;

	dispatch_assert(continuation_cnt);

	for (i = 0; i < continuation_cnt; i++) {
		dispatch_continuation_t next = _dispatch_continuation_alloc();
		next->do_vtable = (void *)DISPATCH_OBJ_ASYNC_BIT;
		next->dc_func = func;
		next->dc_ctxt = da;
		_dispatch_continuation_voucher_set(next, 0);
		_dispatch_continuation_priority_set(next, 0, 0);

		next->do_next = head;
		head = next;

		if (!tail) {
			tail = next;
		}
	}

	_dispatch_thread_semaphore_t sema = _dispatch_get_thread_semaphore();
	da->da_sema = sema;

	_dispatch_queue_push_list(dq, head, tail, head->dc_priority,
			continuation_cnt);
	// Call the first element directly
	_dispatch_apply_invoke(da);
	_dispatch_perfmon_workitem_inc();

	_dispatch_thread_semaphore_wait(sema);
	_dispatch_put_thread_semaphore(sema);

}

static void
_dispatch_apply_redirect(void *ctxt)
{
	dispatch_apply_t da = (dispatch_apply_t)ctxt;
	uint32_t da_width = 2 * (da->da_thr_cnt - 1);
	dispatch_queue_t dq = da->da_dc->dc_data, rq = dq, tq;

	do {
		uint32_t running, width = rq->dq_width;
		running = dispatch_atomic_add2o(rq, dq_running, da_width, relaxed);
		if (slowpath(running > width)) {
			uint32_t excess = width > 1 ? running - width : da_width;
			for (tq = dq; 1; tq = tq->do_targetq) {
				(void)dispatch_atomic_sub2o(tq, dq_running, excess, relaxed);
				if (tq == rq) {
					break;
				}
			}
			da_width -= excess;
			if (slowpath(!da_width)) {
				return _dispatch_apply_serial(da);
			}
			da->da_thr_cnt -= excess / 2;
		}
		rq = rq->do_targetq;
	} while (slowpath(rq->do_targetq));
	_dispatch_apply_f2(rq, da, _dispatch_apply_redirect_invoke);
	do {
		(void)dispatch_atomic_sub2o(dq, dq_running, da_width, relaxed);
		dq = dq->do_targetq;
	} while (slowpath(dq->do_targetq));
}

#define DISPATCH_APPLY_MAX UINT16_MAX // must be < sqrt(SIZE_MAX)

DISPATCH_NOINLINE
void
dispatch_apply_f(size_t iterations, dispatch_queue_t dq, void *ctxt,
		void (*func)(void *, size_t))
{
	if (slowpath(iterations == 0)) {
		return;
	}
	uint32_t thr_cnt = dispatch_hw_config(active_cpus);
	size_t nested = (size_t)_dispatch_thread_getspecific(dispatch_apply_key);
	if (!slowpath(nested)) {
		nested = iterations;
	} else {
		thr_cnt = nested < thr_cnt ? thr_cnt / nested : 1;
		nested = nested < DISPATCH_APPLY_MAX && iterations < DISPATCH_APPLY_MAX
				? nested * iterations : DISPATCH_APPLY_MAX;
	}
	if (iterations < thr_cnt) {
		thr_cnt = (uint32_t)iterations;
	}
	struct dispatch_continuation_s dc = {
		.dc_func = (void*)func,
		.dc_ctxt = ctxt,
	};
	dispatch_apply_t da = (typeof(da))_dispatch_continuation_alloc();
	da->da_index = 0;
	da->da_todo = iterations;
	da->da_iterations = iterations;
	da->da_nested = nested;
	da->da_thr_cnt = thr_cnt;
	da->da_dc = &dc;

	dispatch_queue_t old_dq;
	old_dq = (dispatch_queue_t)_dispatch_thread_getspecific(dispatch_queue_key);
	if (slowpath(dq == DISPATCH_APPLY_CURRENT_ROOT_QUEUE)) {
		dq = old_dq ? old_dq : _dispatch_get_root_queue(
				_DISPATCH_QOS_CLASS_DEFAULT, false);
		while (slowpath(dq->do_targetq)) {
			dq = dq->do_targetq;
		}
	}
	if (slowpath(dq->dq_width <= 2) || slowpath(thr_cnt <= 1)) {
		return dispatch_sync_f(dq, da, _dispatch_apply_serial);
	}
	if (slowpath(dq->do_targetq)) {
		if (slowpath(dq == old_dq)) {
			return dispatch_sync_f(dq, da, _dispatch_apply_serial);
		} else {
			dc.dc_data = dq;
			return dispatch_sync_f(dq, da, _dispatch_apply_redirect);
		}
	}
	_dispatch_thread_setspecific(dispatch_queue_key, dq);
	_dispatch_apply_f2(dq, da, _dispatch_apply_invoke);
	_dispatch_thread_setspecific(dispatch_queue_key, old_dq);
}

#ifdef __BLOCKS__
#if DISPATCH_COCOA_COMPAT
DISPATCH_NOINLINE
static void
_dispatch_apply_slow(size_t iterations, dispatch_queue_t dq,
		void (^work)(size_t))
{
	dispatch_block_t bb = _dispatch_Block_copy((void *)work);
	dispatch_apply_f(iterations, dq, bb,
			(dispatch_apply_function_t)_dispatch_Block_invoke(bb));
	Block_release(bb);
}
#endif

void
dispatch_apply(size_t iterations, dispatch_queue_t dq, void (^work)(size_t))
{
#if DISPATCH_COCOA_COMPAT
	// Under GC, blocks transferred to other threads must be Block_copy()ed
	// rdar://problem/7455071
	if (dispatch_begin_thread_4GC) {
		return _dispatch_apply_slow(iterations, dq, work);
	}
#endif
	dispatch_apply_f(iterations, dq, work,
			(dispatch_apply_function_t)_dispatch_Block_invoke(work));
}
#endif

#if 0
#ifdef __BLOCKS__
void
dispatch_stride(size_t offset, size_t stride, size_t iterations,
		dispatch_queue_t dq, void (^work)(size_t))
{
	dispatch_stride_f(offset, stride, iterations, dq, work,
			(dispatch_apply_function_t)_dispatch_Block_invoke(work));
}
#endif

DISPATCH_NOINLINE
void
dispatch_stride_f(size_t offset, size_t stride, size_t iterations,
		dispatch_queue_t dq, void *ctxt, void (*func)(void *, size_t))
{
	if (stride == 0) {
		stride = 1;
	}
	dispatch_apply(iterations / stride, queue, ^(size_t idx) {
		size_t i = idx * stride + offset;
		size_t stop = i + stride;
		do {
			func(ctxt, i++);
		} while (i < stop);
	});

	dispatch_sync(queue, ^{
		size_t i;
		for (i = iterations - (iterations % stride); i < iterations; i++) {
			func(ctxt, i + offset);
		}
	});
}
#endif
