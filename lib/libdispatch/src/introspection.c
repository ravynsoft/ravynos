/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

// Contains introspection routines that only exist in the version of the
// library with introspection support

#if DISPATCH_INTROSPECTION

#include "internal.h"
#include "dispatch/introspection.h"
#include "introspection_private.h"

typedef struct dispatch_introspection_thread_s {
	void *dit_isa;
	TAILQ_ENTRY(dispatch_introspection_thread_s) dit_list;
	pthread_t thread;
	dispatch_queue_t *queue;
} dispatch_introspection_thread_s;
typedef struct dispatch_introspection_thread_s *dispatch_introspection_thread_t;

static TAILQ_HEAD(, dispatch_introspection_thread_s)
		_dispatch_introspection_threads =
		TAILQ_HEAD_INITIALIZER(_dispatch_introspection_threads);
static volatile OSSpinLock _dispatch_introspection_threads_lock;

static void _dispatch_introspection_thread_remove(void *ctxt);

static TAILQ_HEAD(, dispatch_queue_s) _dispatch_introspection_queues =
		TAILQ_HEAD_INITIALIZER(_dispatch_introspection_queues);
static volatile OSSpinLock _dispatch_introspection_queues_lock;

static ptrdiff_t _dispatch_introspection_thread_queue_offset;

#pragma mark -
#pragma mark dispatch_introspection_init

void
_dispatch_introspection_init(void)
{
	TAILQ_INSERT_TAIL(&_dispatch_introspection_queues,
			&_dispatch_main_q, diq_list);
	TAILQ_INSERT_TAIL(&_dispatch_introspection_queues,
			&_dispatch_mgr_q, diq_list);
#if DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES
	TAILQ_INSERT_TAIL(&_dispatch_introspection_queues,
			_dispatch_mgr_q.do_targetq, diq_list);
#endif
	for (size_t i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
		TAILQ_INSERT_TAIL(&_dispatch_introspection_queues,
				&_dispatch_root_queues[i], diq_list);
	}

	// Hack to determine queue TSD offset from start of pthread structure
	uintptr_t thread = _dispatch_thread_self();
	thread_identifier_info_data_t tiid;
	mach_msg_type_number_t cnt = THREAD_IDENTIFIER_INFO_COUNT;
	kern_return_t kr = thread_info(pthread_mach_thread_np((void*)thread),
			THREAD_IDENTIFIER_INFO, (thread_info_t)&tiid, &cnt);
	if (!dispatch_assume_zero(kr)) {
		_dispatch_introspection_thread_queue_offset =
				(void*)(uintptr_t)tiid.dispatch_qaddr - (void*)thread;
	}
	_dispatch_thread_key_create(&dispatch_introspection_key,
			_dispatch_introspection_thread_remove);
	_dispatch_introspection_thread_add(); // add main thread
}

const struct dispatch_introspection_versions_s
dispatch_introspection_versions = {
	.introspection_version = 1,
	.hooks_version = 2,
	.hooks_size = sizeof(dispatch_introspection_hooks_s),
	.queue_item_version = 1,
	.queue_item_size = sizeof(dispatch_introspection_queue_item_s),
	.queue_block_version = 1,
	.queue_block_size = sizeof(dispatch_introspection_queue_block_s),
	.queue_function_version = 1,
	.queue_function_size = sizeof(dispatch_introspection_queue_function_s),
	.queue_thread_version = 1,
	.queue_thread_size = sizeof(dispatch_introspection_queue_thread_s),
	.object_version = 1,
	.object_size = sizeof(dispatch_introspection_object_s),
	.queue_version = 1,
	.queue_size = sizeof(dispatch_introspection_queue_s),
	.source_version = 1,
	.source_size = sizeof(dispatch_introspection_source_s),
};

#pragma mark -
#pragma mark dispatch_introspection_threads

void
_dispatch_introspection_thread_add(void)
{
	if (_dispatch_thread_getspecific(dispatch_introspection_key)) {
		return;
	}
	uintptr_t thread = _dispatch_thread_self();
	dispatch_introspection_thread_t dit = (void*)_dispatch_continuation_alloc();
	dit->dit_isa = (void*)0x41;
	dit->thread = (void*)thread;
	dit->queue = !_dispatch_introspection_thread_queue_offset ? NULL :
			(void*)thread + _dispatch_introspection_thread_queue_offset;
	_dispatch_thread_setspecific(dispatch_introspection_key, dit);
	OSSpinLockLock(&_dispatch_introspection_threads_lock);
	TAILQ_INSERT_TAIL(&_dispatch_introspection_threads, dit, dit_list);
	OSSpinLockUnlock(&_dispatch_introspection_threads_lock);
}

static void
_dispatch_introspection_thread_remove(void *ctxt)
{
	dispatch_introspection_thread_t dit = ctxt;
	OSSpinLockLock(&_dispatch_introspection_threads_lock);
	TAILQ_REMOVE(&_dispatch_introspection_threads, dit, dit_list);
	OSSpinLockUnlock(&_dispatch_introspection_threads_lock);
	_dispatch_continuation_free((void*)dit);
	_dispatch_thread_setspecific(dispatch_introspection_key, NULL);
}

#pragma mark -
#pragma mark dispatch_introspection_info

static inline
dispatch_introspection_queue_function_s
_dispatch_introspection_continuation_get_info(dispatch_queue_t dq,
		dispatch_continuation_t dc, unsigned long *type)
{
	void *ctxt = dc->dc_ctxt;
	dispatch_function_t func = dc->dc_func;
	pthread_t waiter = NULL;
	bool apply = false;
	long flags = (long)dc->do_vtable;
	if (flags & DISPATCH_OBJ_SYNC_SLOW_BIT) {
		waiter = pthread_from_mach_thread_np((mach_port_t)dc->dc_data);
		if (flags & DISPATCH_OBJ_BARRIER_BIT) {
			dc = dc->dc_ctxt;
			dq = dc->dc_data;
		}
		ctxt = dc->dc_ctxt;
		func = dc->dc_func;
	}
	if (func == _dispatch_sync_recurse_invoke) {
		dc = dc->dc_ctxt;
		dq = dc->dc_data;
		ctxt = dc->dc_ctxt;
		func = dc->dc_func;
	} else if (func == _dispatch_async_redirect_invoke) {
		dq = dc->dc_data;
		dc = dc->dc_other;
		ctxt = dc->dc_ctxt;
		func = dc->dc_func;
		flags = (long)dc->do_vtable;
	} else if (func == _dispatch_mach_barrier_invoke) {
		dq = dq->do_targetq;
		ctxt = dc->dc_data;
		func = dc->dc_other;
	} else if (func == _dispatch_apply_invoke ||
			func == _dispatch_apply_redirect_invoke) {
		dispatch_apply_t da = ctxt;
		if (da->da_todo) {
			dc = da->da_dc;
			if (func == _dispatch_apply_redirect_invoke) {
				dq = dc->dc_data;
			}
			ctxt = dc->dc_ctxt;
			func = dc->dc_func;
			apply = true;
		}
	}
	if (func == _dispatch_call_block_and_release) {
		*type = dispatch_introspection_queue_item_type_block;
		func = _dispatch_Block_invoke(ctxt);
	} else {
		*type = dispatch_introspection_queue_item_type_function;
	}
	dispatch_introspection_queue_function_s diqf= {
		.continuation = dc,
		.target_queue = dq,
		.context = ctxt,
		.function = func,
		.group = flags & DISPATCH_OBJ_GROUP_BIT ? dc->dc_data : NULL,
		.waiter = waiter,
		.barrier = flags & DISPATCH_OBJ_BARRIER_BIT,
		.sync = flags & DISPATCH_OBJ_SYNC_SLOW_BIT,
		.apply = apply,
	};
	return diqf;
}

static inline
dispatch_introspection_object_s
_dispatch_introspection_object_get_info(dispatch_object_t dou)
{
	dispatch_introspection_object_s dio = {
		.object = dou._dc,
		.target_queue = dou._do->do_targetq,
		.type = (void*)dou._do->do_vtable,
		.kind = dx_kind(dou._do),
	};
	return dio;
}

DISPATCH_USED inline
dispatch_introspection_queue_s
dispatch_introspection_queue_get_info(dispatch_queue_t dq)
{
	bool global = (dq->do_xref_cnt == DISPATCH_OBJECT_GLOBAL_REFCNT) ||
			(dq->do_ref_cnt == DISPATCH_OBJECT_GLOBAL_REFCNT);
	uint16_t width = dq->dq_width;
	if (width > 1 && width != DISPATCH_QUEUE_WIDTH_MAX) width /= 2;
	dispatch_introspection_queue_s diq = {
		.queue = dq,
		.target_queue = dq->do_targetq,
		.label = dq->dq_label,
		.serialnum = dq->dq_serialnum,
		.width = width,
		.suspend_count = dq->do_suspend_cnt / 2,
		.enqueued = (dq->do_suspend_cnt & 1) && !global,
		.barrier = (dq->dq_running & 1) && !global,
		.draining = (dq->dq_items_head == (void*)~0ul) ||
				(!dq->dq_items_head && dq->dq_items_tail),
		.global = global,
		.main = (dq == &_dispatch_main_q),
	};
	return diq;
}

static inline
dispatch_introspection_source_s
_dispatch_introspection_source_get_info(dispatch_source_t ds)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_continuation_t dc = dr->ds_handler[DS_EVENT_HANDLER];
	void *ctxt = NULL;
	dispatch_function_t handler = NULL;
	bool hdlr_is_block = false;
	if (dc) {
		ctxt = dc->dc_ctxt;
		handler = dc->dc_func;
		hdlr_is_block = ((long)dc->do_vtable & DISPATCH_OBJ_BLOCK_RELEASE_BIT);
	}
	bool after = (handler == _dispatch_after_timer_callback);
	if (after && !(ds->ds_atomic_flags & DSF_CANCELED)) {
		dc = ctxt;
		ctxt = dc->dc_ctxt;
		handler = dc->dc_func;
		hdlr_is_block = (handler == _dispatch_call_block_and_release);
		if (hdlr_is_block) {
			handler = _dispatch_Block_invoke(ctxt);
		}
	}
	dispatch_introspection_source_s dis = {
		.source = ds,
		.target_queue = ds->do_targetq,
		.type = ds->ds_dkev ? (unsigned long)ds->ds_dkev->dk_kevent.filter : 0,
		.handle = ds->ds_dkev ? (unsigned long)ds->ds_dkev->dk_kevent.ident : 0,
		.context = ctxt,
		.handler = handler,
		.suspend_count = ds->do_suspend_cnt / 2,
		.enqueued = (ds->do_suspend_cnt & 1),
		.handler_is_block = hdlr_is_block,
		.timer = ds->ds_is_timer,
		.after = after,
	};
	return dis;
}

static inline
dispatch_introspection_queue_thread_s
_dispatch_introspection_thread_get_info(dispatch_introspection_thread_t dit)
{
	dispatch_introspection_queue_thread_s diqt = {
		.object = (void*)dit,
		.thread = dit->thread,
	};
	if (dit->queue && *dit->queue) {
		diqt.queue = dispatch_introspection_queue_get_info(*dit->queue);
	}
	return diqt;
}

DISPATCH_USED inline
dispatch_introspection_queue_item_s
dispatch_introspection_queue_item_get_info(dispatch_queue_t dq,
		dispatch_continuation_t dc)
{
	dispatch_introspection_queue_item_s diqi;
	if (DISPATCH_OBJ_IS_VTABLE(dc)) {
		dispatch_object_t dou = (dispatch_object_t)dc;
		unsigned long type = dx_type(dou._do);
		unsigned long metatype = type & _DISPATCH_META_TYPE_MASK;
		if (metatype == _DISPATCH_QUEUE_TYPE &&
				type != DISPATCH_QUEUE_SPECIFIC_TYPE) {
			diqi.type = dispatch_introspection_queue_item_type_queue;
			diqi.queue = dispatch_introspection_queue_get_info(dou._dq);
		} else if (metatype == _DISPATCH_SOURCE_TYPE) {
			diqi.type = dispatch_introspection_queue_item_type_source;
			diqi.source = _dispatch_introspection_source_get_info(dou._ds);
		} else {
			diqi.type = dispatch_introspection_queue_item_type_object;
			diqi.object = _dispatch_introspection_object_get_info(dou._do);
		}
	} else {
		diqi.function = _dispatch_introspection_continuation_get_info(dq, dc,
				&diqi.type);
	}
	return diqi;
}

#pragma mark -
#pragma mark dispatch_introspection_iterators

DISPATCH_USED
dispatch_queue_t
dispatch_introspection_get_queues(dispatch_queue_t start, size_t count,
		dispatch_introspection_queue_t queues)
{
	dispatch_queue_t next;
	next = start ? start : TAILQ_FIRST(&_dispatch_introspection_queues);
	while (count--) {
		if (!next) {
			queues->queue = NULL;
			break;
		}
		*queues++ = dispatch_introspection_queue_get_info(next);
		next = TAILQ_NEXT(next, diq_list);
	}
	return next;
}

DISPATCH_USED
dispatch_continuation_t
dispatch_introspection_get_queue_threads(dispatch_continuation_t start,
		size_t count, dispatch_introspection_queue_thread_t threads)
{
	dispatch_introspection_thread_t next = start ? (void*)start :
			TAILQ_FIRST(&_dispatch_introspection_threads);
	while (count--) {
		if (!next) {
			threads->object = NULL;
			break;
		}
		*threads++ = _dispatch_introspection_thread_get_info(next);
		next = TAILQ_NEXT(next, dit_list);
	}
	return (void*)next;
}

DISPATCH_USED
dispatch_continuation_t
dispatch_introspection_queue_get_items(dispatch_queue_t dq,
		dispatch_continuation_t start, size_t count,
		dispatch_introspection_queue_item_t items)
{
	dispatch_continuation_t next = start ? start :
			dq->dq_items_head == (void*)~0ul ? NULL : (void*)dq->dq_items_head;
	while (count--) {
		if (!next) {
			items->type = dispatch_introspection_queue_item_type_none;
			break;
		}
		*items++ = dispatch_introspection_queue_item_get_info(dq, next);
		next = next->do_next;
	}
	return next;
}

#pragma mark -
#pragma mark dispatch_introspection_hooks

#define DISPATCH_INTROSPECTION_NO_HOOK ((void*)~0ul)

dispatch_introspection_hooks_s _dispatch_introspection_hooks;
dispatch_introspection_hooks_s _dispatch_introspection_hook_callouts;
static const
dispatch_introspection_hooks_s _dispatch_introspection_hook_callouts_enabled = {
	.queue_create = DISPATCH_INTROSPECTION_NO_HOOK,
	.queue_dispose = DISPATCH_INTROSPECTION_NO_HOOK,
	.queue_item_enqueue = DISPATCH_INTROSPECTION_NO_HOOK,
	.queue_item_dequeue = DISPATCH_INTROSPECTION_NO_HOOK,
	.queue_item_complete = DISPATCH_INTROSPECTION_NO_HOOK,
};

#define DISPATCH_INTROSPECTION_HOOKS_COUNT (( \
		sizeof(_dispatch_introspection_hook_callouts_enabled) - \
		sizeof(_dispatch_introspection_hook_callouts_enabled._reserved)) / \
		sizeof(dispatch_function_t))

#define DISPATCH_INTROSPECTION_HOOK_ENABLED(h) \
		(slowpath(_dispatch_introspection_hooks.h))

#define DISPATCH_INTROSPECTION_HOOK_CALLOUT(h, ...) ({ \
		typeof(_dispatch_introspection_hooks.h) _h; \
		_h = _dispatch_introspection_hooks.h; \
		if (slowpath((void*)(_h) != DISPATCH_INTROSPECTION_NO_HOOK)) { \
			_h(__VA_ARGS__); \
		} })

#define DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(h) \
		DISPATCH_EXPORT void _dispatch_introspection_hook_##h(void) \
		asm("_dispatch_introspection_hook_" #h); \
		void _dispatch_introspection_hook_##h(void) {}

#define DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(h, ...)\
		dispatch_introspection_hook_##h(__VA_ARGS__)

DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(queue_create);
DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(queue_destroy);
DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(queue_item_enqueue);
DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(queue_item_dequeue);
DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(queue_item_complete);
DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(queue_callout_begin);
DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(queue_callout_end);

DISPATCH_USED
void
dispatch_introspection_hooks_install(dispatch_introspection_hooks_t hooks)
{
	dispatch_introspection_hooks_s old_hooks = _dispatch_introspection_hooks;
	_dispatch_introspection_hooks = *hooks;
	dispatch_function_t *e = (void*)&_dispatch_introspection_hook_callouts,
			*h = (void*)&_dispatch_introspection_hooks, *oh = (void*)&old_hooks;
	for (size_t i = 0; i < DISPATCH_INTROSPECTION_HOOKS_COUNT; i++) {
		if (!h[i] && e[i]) {
			h[i] = DISPATCH_INTROSPECTION_NO_HOOK;
		}
		if (oh[i] == DISPATCH_INTROSPECTION_NO_HOOK) {
			oh[i] = NULL;
		}
	}
	*hooks = old_hooks;
}

DISPATCH_USED
void
dispatch_introspection_hook_callouts_enable(
		dispatch_introspection_hooks_t enable)
{
	_dispatch_introspection_hook_callouts = enable ? *enable :
			_dispatch_introspection_hook_callouts_enabled;
	dispatch_function_t *e = (void*)&_dispatch_introspection_hook_callouts,
			*h = (void*)&_dispatch_introspection_hooks;
	for (size_t i = 0; i < DISPATCH_INTROSPECTION_HOOKS_COUNT; i++) {
		if (e[i] && !h[i]) {
			h[i] = DISPATCH_INTROSPECTION_NO_HOOK;
		} else if (!e[i] && h[i] == DISPATCH_INTROSPECTION_NO_HOOK) {
			h[i] = NULL;
		}
	}
}

DISPATCH_NOINLINE
void
dispatch_introspection_hook_callout_queue_create(
		dispatch_introspection_queue_t queue_info)
{
	DISPATCH_INTROSPECTION_HOOK_CALLOUT(queue_create, queue_info);
}

DISPATCH_NOINLINE
static void
_dispatch_introspection_queue_create_hook(dispatch_queue_t dq)
{
	dispatch_introspection_queue_s diq;
	diq = dispatch_introspection_queue_get_info(dq);
	dispatch_introspection_hook_callout_queue_create(&diq);
}

dispatch_queue_t
_dispatch_introspection_queue_create(dispatch_queue_t dq)
{
	OSSpinLockLock(&_dispatch_introspection_queues_lock);
	TAILQ_INSERT_TAIL(&_dispatch_introspection_queues, dq, diq_list);
	OSSpinLockUnlock(&_dispatch_introspection_queues_lock);

	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(queue_create, dq);
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(queue_create)) {
		_dispatch_introspection_queue_create_hook(dq);
	}
	return dq;
}

DISPATCH_NOINLINE
void
dispatch_introspection_hook_callout_queue_dispose(
		dispatch_introspection_queue_t queue_info)
{
	DISPATCH_INTROSPECTION_HOOK_CALLOUT(queue_dispose, queue_info);
}

DISPATCH_NOINLINE
static void
_dispatch_introspection_queue_dispose_hook(dispatch_queue_t dq)
{
	dispatch_introspection_queue_s diq;
	diq = dispatch_introspection_queue_get_info(dq);
	dispatch_introspection_hook_callout_queue_dispose(&diq);
}

void
_dispatch_introspection_queue_dispose(dispatch_queue_t dq)
{
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(queue_destroy, dq);
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(queue_dispose)) {
		_dispatch_introspection_queue_dispose_hook(dq);
	}

	OSSpinLockLock(&_dispatch_introspection_queues_lock);
	TAILQ_REMOVE(&_dispatch_introspection_queues, dq, diq_list);
	OSSpinLockUnlock(&_dispatch_introspection_queues_lock);
}

DISPATCH_NOINLINE
void
dispatch_introspection_hook_callout_queue_item_enqueue(dispatch_queue_t queue,
		dispatch_introspection_queue_item_t item)
{
	DISPATCH_INTROSPECTION_HOOK_CALLOUT(queue_item_enqueue, queue, item);
}

DISPATCH_NOINLINE
static void
_dispatch_introspection_queue_item_enqueue_hook(dispatch_queue_t dq,
		dispatch_object_t dou)
{
	dispatch_introspection_queue_item_s diqi;
	diqi = dispatch_introspection_queue_item_get_info(dq, dou._dc);
	dispatch_introspection_hook_callout_queue_item_enqueue(dq, &diqi);
}

void
_dispatch_introspection_queue_item_enqueue(dispatch_queue_t dq,
		dispatch_object_t dou)
{
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(
			queue_item_enqueue, dq, dou);
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(queue_item_enqueue)) {
		_dispatch_introspection_queue_item_enqueue_hook(dq, dou);
	}
}

DISPATCH_NOINLINE
void
dispatch_introspection_hook_callout_queue_item_dequeue(dispatch_queue_t queue,
		dispatch_introspection_queue_item_t item)
{
	DISPATCH_INTROSPECTION_HOOK_CALLOUT(queue_item_dequeue, queue, item);
}

DISPATCH_NOINLINE
static void
_dispatch_introspection_queue_item_dequeue_hook(dispatch_queue_t dq,
		dispatch_object_t dou)
{
	dispatch_introspection_queue_item_s diqi;
	diqi = dispatch_introspection_queue_item_get_info(dq, dou._dc);
	dispatch_introspection_hook_callout_queue_item_dequeue(dq, &diqi);
}

void
_dispatch_introspection_queue_item_dequeue(dispatch_queue_t dq,
		dispatch_object_t dou)
{
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(
			queue_item_dequeue, dq, dou);
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(queue_item_dequeue)) {
		_dispatch_introspection_queue_item_dequeue_hook(dq, dou);
	}
}

DISPATCH_NOINLINE
void
dispatch_introspection_hook_callout_queue_item_complete(
		dispatch_continuation_t object)
{
	DISPATCH_INTROSPECTION_HOOK_CALLOUT(queue_item_complete, object);
}

DISPATCH_NOINLINE
static void
_dispatch_introspection_queue_item_complete_hook(dispatch_object_t dou)
{
	dispatch_introspection_hook_callout_queue_item_complete(dou._dc);
}

void
_dispatch_introspection_queue_item_complete(dispatch_object_t dou)
{
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(queue_item_complete, dou);
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(queue_item_complete)) {
		_dispatch_introspection_queue_item_complete_hook(dou);
	}
}

void
_dispatch_introspection_callout_entry(void *ctxt, dispatch_function_t f) {
	dispatch_queue_t dq = _dispatch_queue_get_current();
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(
			queue_callout_begin, dq, ctxt, f);
}

void
_dispatch_introspection_callout_return(void *ctxt, dispatch_function_t f) {
	dispatch_queue_t dq = _dispatch_queue_get_current();
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(
			queue_callout_end, dq, ctxt, f);
}

#endif // DISPATCH_INTROSPECTION
