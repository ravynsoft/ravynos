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

#include <execinfo.h>
#include "internal.h"
#include "dispatch/introspection.h"
#include "introspection_private.h"

typedef struct dispatch_introspection_thread_s {
#if !OS_OBJECT_HAVE_OBJC1
	void *dit_isa;
#endif
	LIST_ENTRY(dispatch_introspection_thread_s) dit_list;
	pthread_t thread;
#if OS_OBJECT_HAVE_OBJC1
	void *dit_isa;
#endif
	dispatch_queue_t *queue;
} dispatch_introspection_thread_s;
dispatch_static_assert(offsetof(struct dispatch_continuation_s, dc_flags) ==
		offsetof(struct dispatch_introspection_thread_s, dit_isa),
		"These fields must alias so that leaks instruments work");
typedef struct dispatch_introspection_thread_s *dispatch_introspection_thread_t;

struct dispatch_introspection_state_s _dispatch_introspection = {
	.threads = LIST_HEAD_INITIALIZER(_dispatch_introspection.threads),
	.queues = LIST_HEAD_INITIALIZER(_dispatch_introspection.queues),
};

static void _dispatch_introspection_thread_remove(void *ctxt);

static void _dispatch_introspection_queue_order_dispose(
		dispatch_queue_introspection_context_t dqic);

#pragma mark -
#pragma mark dispatch_introspection_init

void
_dispatch_introspection_init(void)
{
	_dispatch_introspection.debug_queue_inversions =
			_dispatch_getenv_bool("LIBDISPATCH_DEBUG_QUEUE_INVERSIONS", false);

	// Hack to determine queue TSD offset from start of pthread structure
	uintptr_t thread = _dispatch_thread_self();
	thread_identifier_info_data_t tiid;
	mach_msg_type_number_t cnt = THREAD_IDENTIFIER_INFO_COUNT;
	kern_return_t kr = thread_info(pthread_mach_thread_np((void*)thread),
			THREAD_IDENTIFIER_INFO, (thread_info_t)&tiid, &cnt);
	if (!dispatch_assume_zero(kr)) {
		_dispatch_introspection.thread_queue_offset =
				(void*)(uintptr_t)tiid.dispatch_qaddr - (void*)thread;
	}
	_dispatch_thread_key_create(&dispatch_introspection_key,
			_dispatch_introspection_thread_remove);
	_dispatch_introspection_thread_add(); // add main thread

	for (size_t i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
		_dispatch_trace_queue_create(&_dispatch_root_queues[i]);
	}
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
	_dispatch_trace_queue_create(_dispatch_mgr_q.do_targetq);
#endif
	_dispatch_trace_queue_create(&_dispatch_main_q);
	_dispatch_trace_queue_create(&_dispatch_mgr_q);
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
	dit->queue = !_dispatch_introspection.thread_queue_offset ? NULL :
			(void*)thread + _dispatch_introspection.thread_queue_offset;
	_dispatch_thread_setspecific(dispatch_introspection_key, dit);
	_dispatch_unfair_lock_lock(&_dispatch_introspection.threads_lock);
	LIST_INSERT_HEAD(&_dispatch_introspection.threads, dit, dit_list);
	_dispatch_unfair_lock_unlock(&_dispatch_introspection.threads_lock);
}

static DISPATCH_TSD_DTOR_CC void
_dispatch_introspection_thread_remove(void *ctxt)
{
	dispatch_introspection_thread_t dit = ctxt;
	_dispatch_unfair_lock_lock(&_dispatch_introspection.threads_lock);
	LIST_REMOVE(dit, dit_list);
	_dispatch_unfair_lock_unlock(&_dispatch_introspection.threads_lock);
	_dispatch_continuation_free((void*)dit);
	_dispatch_thread_setspecific(dispatch_introspection_key, NULL);
}

#pragma mark -
#pragma mark dispatch_introspection_info

DISPATCH_ALWAYS_INLINE
static inline dispatch_introspection_queue_s
_dispatch_introspection_lane_get_info(dispatch_lane_class_t dqu)
{
	dispatch_lane_t dq = dqu._dl;
	bool global = _dispatch_object_is_global(dq);
	uint64_t dq_state = os_atomic_load2o(dq, dq_state, relaxed);

	dispatch_introspection_queue_s diq = {
		.queue = dq->_as_dq,
		.target_queue = dq->do_targetq,
		.label = dq->dq_label,
		.serialnum = dq->dq_serialnum,
		.width = dq->dq_width,
		.suspend_count = _dq_state_suspend_cnt(dq_state) + dq->dq_side_suspend_cnt,
		.enqueued = _dq_state_is_enqueued(dq_state) && !global,
		.barrier = _dq_state_is_in_barrier(dq_state) && !global,
		.draining = (dq->dq_items_head == (void*)~0ul) ||
				(!dq->dq_items_head && dq->dq_items_tail),
		.global = global,
		.main = dx_type(dq) == DISPATCH_QUEUE_MAIN_TYPE,
	};
	return diq;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_introspection_queue_s
_dispatch_introspection_workloop_get_info(dispatch_workloop_t dwl)
{
	uint64_t dq_state = os_atomic_load2o(dwl, dq_state, relaxed);

	dispatch_introspection_queue_s diq = {
		.queue = dwl->_as_dq,
		.target_queue = dwl->do_targetq,
		.label = dwl->dq_label,
		.serialnum = dwl->dq_serialnum,
		.width = 1,
		.suspend_count = 0,
		.enqueued = _dq_state_is_enqueued(dq_state),
		.barrier = _dq_state_is_in_barrier(dq_state),
		.draining = 0,
		.global = 0,
		.main = 0,
	};
	return diq;
}

DISPATCH_USED inline
dispatch_introspection_queue_s
dispatch_introspection_queue_get_info(dispatch_queue_t dq)
{
	if (dx_metatype(dq) == _DISPATCH_WORKLOOP_TYPE) {
		return _dispatch_introspection_workloop_get_info(upcast(dq)._dwl);
	}
	return _dispatch_introspection_lane_get_info(upcast(dq)._dl);
}

static inline void
_dispatch_introspection_continuation_get_info(dispatch_queue_t dq,
		dispatch_continuation_t dc, dispatch_introspection_queue_item_t diqi)
{
	void *ctxt = dc->dc_ctxt;
	dispatch_function_t func = dc->dc_func;
	pthread_t waiter = NULL;
	bool apply = false;
	uintptr_t flags = dc->dc_flags;

	if (_dispatch_object_has_vtable(dc)) {
		flags = 0;
		switch (dc_type(dc)) {
#if HAVE_PTHREAD_WORKQUEUE_QOS
		case DISPATCH_CONTINUATION_TYPE(WORKLOOP_STEALING):
		case DISPATCH_CONTINUATION_TYPE(OVERRIDE_STEALING):
		case DISPATCH_CONTINUATION_TYPE(OVERRIDE_OWNING):
			dc = dc->dc_data;
			if (!_dispatch_object_is_continuation(dc)) {
				// these really wrap queues so we should hide the continuation type
				dq = (dispatch_queue_t)dc;
				diqi->type = dispatch_introspection_queue_item_type_queue;
				diqi->queue = dispatch_introspection_queue_get_info(dq);
				return;
			}
			return _dispatch_introspection_continuation_get_info(dq, dc, diqi);
#endif
		case DISPATCH_CONTINUATION_TYPE(ASYNC_REDIRECT):
			DISPATCH_INTERNAL_CRASH(0, "Handled by the caller");
		case DISPATCH_CONTINUATION_TYPE(MACH_ASYNC_REPLY):
			break;
		case DISPATCH_CONTINUATION_TYPE(MACH_SEND_BARRRIER_DRAIN):
			break;
		case DISPATCH_CONTINUATION_TYPE(MACH_SEND_BARRIER):
		case DISPATCH_CONTINUATION_TYPE(MACH_RECV_BARRIER):
			flags = (uintptr_t)dc->dc_data;
			dq = dq->do_targetq;
			break;
		case DISPATCH_CONTINUATION_TYPE(MACH_IPC_HANDOFF):
			flags = (uintptr_t)dc->dc_data;
			break;
		default:
			DISPATCH_INTERNAL_CRASH(dc->do_vtable, "Unknown dc vtable type");
		}
	} else if (flags & (DC_FLAG_SYNC_WAITER | DC_FLAG_ASYNC_AND_WAIT)) {
		dispatch_sync_context_t dsc = (dispatch_sync_context_t)dc;
		waiter = pthread_from_mach_thread_np(dsc->dsc_waiter);
		ctxt = dsc->dsc_ctxt;
		func = dsc->dsc_func;
	} else if (_dispatch_object_is_channel_item(dc)) {
		dispatch_channel_callbacks_t callbacks = upcast(dq)._dch->dch_callbacks;
		ctxt = dc->dc_ctxt;
		func = (dispatch_function_t)callbacks->dcc_invoke;
	} else if (func == _dispatch_apply_invoke ||
			func == _dispatch_apply_redirect_invoke) {
		dispatch_apply_t da = ctxt;
		if (da->da_todo) {
			dc = da->da_dc;
			dq = dc->dc_data;
			ctxt = dc->dc_ctxt;
			func = dc->dc_func;
			apply = true;
		}
	}

	if (flags & DC_FLAG_BLOCK_WITH_PRIVATE_DATA) {
		dispatch_block_private_data_t dbpd = _dispatch_block_get_data(ctxt);
		diqi->type = dispatch_introspection_queue_item_type_block;
		func = _dispatch_Block_invoke(dbpd->dbpd_block);
	} else if (flags & DC_FLAG_BLOCK) {
		diqi->type = dispatch_introspection_queue_item_type_block;
		func = _dispatch_Block_invoke(ctxt);
	} else {
		diqi->type = dispatch_introspection_queue_item_type_function;
	}
	diqi->function = (dispatch_introspection_queue_function_s){
		.continuation = dc,
		.target_queue = dq,
		.context = ctxt,
		.function = func,
		.waiter = waiter,
		.barrier = (flags & DC_FLAG_BARRIER) || dq->dq_width == 1,
		.sync = (bool)(flags & (DC_FLAG_SYNC_WAITER | DC_FLAG_ASYNC_AND_WAIT)),
		.apply = apply,
	};
	if (flags & DC_FLAG_GROUP_ASYNC) {
		dispatch_group_t group = dc->dc_data;
		if (dx_type(group) == DISPATCH_GROUP_TYPE) {
			diqi->function.group = group;
		}
	}
}

static inline
dispatch_introspection_object_s
_dispatch_introspection_object_get_info(dispatch_object_t dou)
{
	dispatch_introspection_object_s dio = {
		.object = dou._dc,
		.target_queue = dou._do->do_targetq,
		.type = (void*)dou._do->do_vtable,
		.kind = _dispatch_object_class_name(dou._do),
	};
	return dio;
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
		hdlr_is_block = (dc->dc_flags & DC_FLAG_BLOCK);
	}

	uint64_t dq_state = os_atomic_load2o(ds, dq_state, relaxed);
	dispatch_introspection_source_s dis = {
		.source = ds,
		.target_queue = ds->do_targetq,
		.context = ctxt,
		.handler = handler,
		.suspend_count = _dq_state_suspend_cnt(dq_state) + ds->dq_side_suspend_cnt,
		.enqueued = _dq_state_is_enqueued(dq_state),
		.handler_is_block = hdlr_is_block,
		.timer = dr->du_is_timer,
		.after = dr->du_is_timer && (dr->du_timer_flags & DISPATCH_TIMER_AFTER),
		.type = (unsigned long)dr->du_filter,
		.handle = (unsigned long)dr->du_ident,
	};
	return dis;
}

static inline
dispatch_introspection_source_s
_dispatch_introspection_mach_get_info(dispatch_mach_t dm)
{
	dispatch_mach_recv_refs_t dmrr = dm->dm_recv_refs;
	uint64_t dq_state = os_atomic_load2o(dm, dq_state, relaxed);

	dispatch_introspection_source_s dis = {
		.source = upcast(dm)._ds,
		.target_queue = dm->do_targetq,
		.context = dmrr->dmrr_handler_ctxt,
		.handler = (void *)dmrr->dmrr_handler_func,
		.suspend_count = _dq_state_suspend_cnt(dq_state) + dm->dq_side_suspend_cnt,
		.enqueued = _dq_state_is_enqueued(dq_state),
		.handler_is_block = dmrr->dmrr_handler_is_block,
		.type = (unsigned long)dmrr->du_filter,
		.handle = (unsigned long)dmrr->du_ident,
		.is_xpc = dm->dm_is_xpc,
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
	dispatch_object_t dou;

again:
	dou._dc = dc;
	if (_dispatch_object_has_vtable(dou._do)) {
		unsigned long type = dx_type(dou._do);
		unsigned long metatype = type & _DISPATCH_META_TYPE_MASK;
		if (type == DISPATCH_CONTINUATION_TYPE(ASYNC_REDIRECT)) {
			dq = dc->dc_data;
			dc = dc->dc_other;
			goto again;
		}
		if (metatype == _DISPATCH_CONTINUATION_TYPE) {
			_dispatch_introspection_continuation_get_info(dq, dc, &diqi);
		} else if (metatype == _DISPATCH_LANE_TYPE ||
				type == DISPATCH_CHANNEL_TYPE) {
			diqi.type = dispatch_introspection_queue_item_type_queue;
			diqi.queue = _dispatch_introspection_lane_get_info(dou._dl);
		} else if (metatype == _DISPATCH_WORKLOOP_TYPE) {
			diqi.type = dispatch_introspection_queue_item_type_queue;
			diqi.queue = _dispatch_introspection_workloop_get_info(dou._dwl);
		} else if (type == DISPATCH_SOURCE_KEVENT_TYPE) {
			diqi.type = dispatch_introspection_queue_item_type_source;
			diqi.source = _dispatch_introspection_source_get_info(dou._ds);
		} else if (type == DISPATCH_MACH_CHANNEL_TYPE) {
			diqi.type = dispatch_introspection_queue_item_type_source;
			diqi.source = _dispatch_introspection_mach_get_info(dou._dm);
		} else {
			diqi.type = dispatch_introspection_queue_item_type_object;
			diqi.object = _dispatch_introspection_object_get_info(dou._do);
		}
	} else {
		_dispatch_introspection_continuation_get_info(dq, dc, &diqi);
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
	dispatch_queue_introspection_context_t next;

	if (start) {
		next = start->do_finalizer;
	} else {
		next = LIST_FIRST(&_dispatch_introspection.queues);
	}
	while (count--) {
		if (!next) {
			queues->queue = NULL;
			return NULL;
		}
		*queues++ = dispatch_introspection_queue_get_info(next->dqic_queue._dq);
		next = LIST_NEXT(next, dqic_list);
	}
	return next->dqic_queue._dq;
}

DISPATCH_USED
dispatch_continuation_t
dispatch_introspection_get_queue_threads(dispatch_continuation_t start,
		size_t count, dispatch_introspection_queue_thread_t threads)
{
	dispatch_introspection_thread_t next = start ? (void*)start :
			LIST_FIRST(&_dispatch_introspection.threads);
	while (count--) {
		if (!next) {
			threads->object = NULL;
			break;
		}
		*threads++ = _dispatch_introspection_thread_get_info(next);
		next = LIST_NEXT(next, dit_list);
	}
	return (void*)next;
}

DISPATCH_USED
dispatch_continuation_t
dispatch_introspection_queue_get_items(dispatch_queue_t _dq,
		dispatch_continuation_t start, size_t count,
		dispatch_introspection_queue_item_t items)
{
	if (dx_metatype(_dq) != _DISPATCH_LANE_TYPE) return NULL;
	dispatch_lane_t dq = upcast(_dq)._dl;
	dispatch_continuation_t next = start ? start :
			dq->dq_items_head == (void*)~0ul ? NULL : (void*)dq->dq_items_head;
	while (count--) {
		if (!next) {
			items->type = dispatch_introspection_queue_item_type_none;
			break;
		}
		*items++ = dispatch_introspection_queue_item_get_info(_dq, next);
		next = next->do_next;
	}
	return next;
}

#pragma mark -
#pragma mark tracing & introspection helpers

struct dispatch_object_s *
_dispatch_introspection_queue_fake_sync_push_pop(dispatch_queue_t dq,
		void *ctxt, dispatch_function_t func, uintptr_t dc_flags)
{
	// fake just what introspection really needs here: flags, func, ctxt, queue,
	// dc_priority, and of course waiter
	struct dispatch_sync_context_s dsc = {
		.dc_priority = _dispatch_get_priority(),
		.dc_flags    = DC_FLAG_SYNC_WAITER | dc_flags,
		.dc_other    = dq,
		.dsc_func    = func,
		.dsc_ctxt    = ctxt,
		.dsc_waiter  = _dispatch_tid_self(),
	};

	_dispatch_trace_item_push(dq, &dsc);
	_dispatch_trace_item_pop(dq, &dsc);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
	return (struct dispatch_object_s *)(uintptr_t)&dsc;
#pragma clang diagnostic pop
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
	.runtime_event = DISPATCH_INTROSPECTION_NO_HOOK,
};

#define DISPATCH_INTROSPECTION_HOOKS_COUNT (( \
		sizeof(_dispatch_introspection_hook_callouts_enabled) - \
		sizeof(_dispatch_introspection_hook_callouts_enabled._reserved)) / \
		sizeof(dispatch_function_t))

#define DISPATCH_INTROSPECTION_HOOK_ENABLED(h) \
		unlikely(_dispatch_introspection_hooks.h)

#define DISPATCH_INTROSPECTION_HOOK_CALLOUT(h, ...) ({ \
		__typeof__(_dispatch_introspection_hooks.h) _h; \
		_h = _dispatch_introspection_hooks.h; \
		if (unlikely((void*)(_h) != DISPATCH_INTROSPECTION_NO_HOOK)) { \
			_h(__VA_ARGS__); \
		} })

#define DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK(h) \
		DISPATCH_EXPORT void _dispatch_introspection_hook_##h(void) \
		__asm__("_dispatch_introspection_hook_" #h); \
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

dispatch_function_t
_dispatch_object_finalizer(dispatch_object_t dou)
{
	dispatch_queue_introspection_context_t dqic;
	switch (dx_metatype(dou._do)) {
	case _DISPATCH_LANE_TYPE:
	case _DISPATCH_WORKLOOP_TYPE:
		dqic = dou._dq->do_finalizer;
		return dqic->dqic_finalizer;
	default:
		return dou._do->do_finalizer;
	}
}

void
_dispatch_object_set_finalizer(dispatch_object_t dou,
		dispatch_function_t finalizer)
{
	dispatch_queue_introspection_context_t dqic;
	switch (dx_metatype(dou._do)) {
	case _DISPATCH_LANE_TYPE:
	case _DISPATCH_WORKLOOP_TYPE:
		dqic = dou._dq->do_finalizer;
		dqic->dqic_finalizer = finalizer;
		break;
	default:
		dou._do->do_finalizer = finalizer;
		break;
	}
}

dispatch_queue_class_t
_dispatch_introspection_queue_create(dispatch_queue_t dq)
{
	dispatch_queue_introspection_context_t dqic;
	size_t sz = sizeof(struct dispatch_queue_introspection_context_s);

	if (!_dispatch_introspection.debug_queue_inversions) {
		sz = offsetof(struct dispatch_queue_introspection_context_s,
				__dqic_no_queue_inversion);
	}
	dqic = _dispatch_calloc(1, sz);
	dqic->dqic_queue._dq = dq;
	if (_dispatch_introspection.debug_queue_inversions) {
		LIST_INIT(&dqic->dqic_order_top_head);
		LIST_INIT(&dqic->dqic_order_bottom_head);
	}
	dq->do_finalizer = dqic;

	_dispatch_unfair_lock_lock(&_dispatch_introspection.queues_lock);
	LIST_INSERT_HEAD(&_dispatch_introspection.queues, dqic, dqic_list);
	_dispatch_unfair_lock_unlock(&_dispatch_introspection.queues_lock);

	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(queue_create, dq);
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(queue_create)) {
		_dispatch_introspection_queue_create_hook(dq);
	}
	return upcast(dq)._dqu;
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
	dispatch_queue_introspection_context_t dqic = dq->do_finalizer;

	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(queue_destroy, dq);
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(queue_dispose)) {
		_dispatch_introspection_queue_dispose_hook(dq);
	}

	_dispatch_unfair_lock_lock(&_dispatch_introspection.queues_lock);
	LIST_REMOVE(dqic, dqic_list);
	if (_dispatch_introspection.debug_queue_inversions) {
		_dispatch_introspection_queue_order_dispose(dqic);
	}
	_dispatch_unfair_lock_unlock(&_dispatch_introspection.queues_lock);

	dq->do_finalizer = dqic->dqic_finalizer; // restore the real finalizer
	free(dqic);
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

void
_dispatch_trace_item_push_internal(dispatch_queue_t dq,
		dispatch_object_t dou)
{
	if (dx_metatype(dq) != _DISPATCH_LANE_TYPE) {
		return;
	}

	dispatch_continuation_t dc = dou._dc;

	/* Only track user continuations */
	if (_dispatch_object_is_continuation(dou) &&
		_dispatch_object_has_vtable(dou) && dc_type(dc) > 0){
		return;
	}

	struct dispatch_introspection_queue_item_s idc;
	idc = dispatch_introspection_queue_item_get_info(dq, dc);

	switch (idc.type) {
	case dispatch_introspection_queue_item_type_none:
		break;
	case dispatch_introspection_queue_item_type_block:
	{
		uintptr_t dc_flags = 0;
		dc_flags |= (idc.block.barrier ? DC_BARRIER : 0);
		dc_flags |= (idc.block.sync ? DC_SYNC : 0);
		dc_flags |= (idc.block.apply ? DC_APPLY : 0);

		if (dc->dc_flags & DC_FLAG_BLOCK_WITH_PRIVATE_DATA) {
			_dispatch_ktrace4(DISPATCH_QOS_TRACE_continuation_push_eb,
					dou._do_value,
					(uintptr_t)idc.block.block, /* Heap allocated block ptr */
					BITPACK_UINT32_PAIR(dq->dq_serialnum, dc_flags),
					BITPACK_UINT32_PAIR(_dispatch_get_priority(),
							dc->dc_priority));
		} else {
			_dispatch_ktrace4(DISPATCH_QOS_TRACE_continuation_push_ab,
					dou._do_value,
					(uintptr_t)idc.block.block_invoke, /* Function pointer */
					BITPACK_UINT32_PAIR(dq->dq_serialnum, dc_flags),
					BITPACK_UINT32_PAIR(_dispatch_get_priority(),
							dc->dc_priority));
		}

		break;
	}
	case dispatch_introspection_queue_item_type_function:
	{
		uintptr_t dc_flags = 0;
		dc_flags |= (idc.function.barrier ? DC_BARRIER : 0);
		dc_flags |= (idc.function.sync ? DC_SYNC : 0);
		dc_flags |= (idc.function.apply ? DC_APPLY : 0);

		_dispatch_ktrace4(DISPATCH_QOS_TRACE_continuation_push_f,
				dou._do_value,
				(uintptr_t)idc.function.function, /* Function pointer */
				BITPACK_UINT32_PAIR(dq->dq_serialnum, dc_flags),
				BITPACK_UINT32_PAIR(_dispatch_get_priority(), dc->dc_priority));
		break;
	}
	case dispatch_introspection_queue_item_type_object:
		/* Generic dispatch object - we don't know how to handle this yet */
		break;
	case dispatch_introspection_queue_item_type_queue:
		/* Dispatch queue - we don't know how to handle this yet */
		break;
	case dispatch_introspection_queue_item_type_source:
		/* Dispatch sources */
		_dispatch_ktrace4(DISPATCH_QOS_TRACE_source_push,
				dou._do_value,
				idc.source.type,
				(uintptr_t)idc.source.handler,
				dq->dq_serialnum);
		break;
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

void
_dispatch_trace_item_pop_internal(dispatch_queue_t dq,
		dispatch_object_t dou)
{
	if (dx_metatype(dq) != _DISPATCH_LANE_TYPE) {
		return;
	}

	dispatch_continuation_t dc = dou._dc;

	/* Only track user continuations */
	if (_dispatch_object_is_continuation(dou) &&
		_dispatch_object_has_vtable(dou) && dc_type(dc) > 0){
		return;
	}

	struct dispatch_introspection_queue_item_s idc;
	idc = dispatch_introspection_queue_item_get_info(dq, dc);

	switch (idc.type) {
	case dispatch_introspection_queue_item_type_none:
		break;
	case dispatch_introspection_queue_item_type_block:
	case dispatch_introspection_queue_item_type_function:
		_dispatch_ktrace3(DISPATCH_QOS_TRACE_continuation_pop,
				dou._do_value, _dispatch_get_priority(), dq->dq_serialnum);
		break;
	case dispatch_introspection_queue_item_type_object:
		/* Generic dispatch object - we don't know how to handle this yet */
		break;
	case dispatch_introspection_queue_item_type_queue:
		/* Dispatch queue - we don't know how to handle this yet */
		break;
	case dispatch_introspection_queue_item_type_source:
		/* Dispatch sources */
		_dispatch_ktrace2(DISPATCH_QOS_TRACE_source_pop,
				dou._do_value, dq->dq_serialnum);
		break;
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
_dispatch_introspection_callout_entry(void *ctxt, dispatch_function_t f)
{
	dispatch_queue_t dq = _dispatch_queue_get_current();
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(
			queue_callout_begin, dq, ctxt, f);
}

void
_dispatch_trace_source_callout_entry_internal(dispatch_source_t ds, long kind,
		dispatch_queue_t dq, dispatch_continuation_t dc)
{
	if (dx_metatype(dq) != _DISPATCH_LANE_TYPE) {
		return;
	}

	_dispatch_ktrace3(DISPATCH_QOS_TRACE_src_callout,
					(uintptr_t)ds, (uintptr_t)dc, kind);

	_dispatch_trace_item_push_internal(dq, (dispatch_object_t) dc);
}

void
_dispatch_introspection_callout_return(void *ctxt, dispatch_function_t f)
{
	dispatch_queue_t dq = _dispatch_queue_get_current();
	DISPATCH_INTROSPECTION_INTERPOSABLE_HOOK_CALLOUT(
			queue_callout_end, dq, ctxt, f);
}

void
_dispatch_introspection_runtime_event(
		enum dispatch_introspection_runtime_event event,
		void *ptr, uint64_t value)
{
	if (DISPATCH_INTROSPECTION_HOOK_ENABLED(runtime_event)) {
		DISPATCH_INTROSPECTION_HOOK_CALLOUT(runtime_event, event, ptr, value);
	}
}

#pragma mark -
#pragma mark dispatch introspection deadlock detection

typedef struct dispatch_queue_order_entry_s *dispatch_queue_order_entry_t;
struct dispatch_queue_order_entry_s {
	LIST_ENTRY(dispatch_queue_order_entry_s) dqoe_order_top_list;
	LIST_ENTRY(dispatch_queue_order_entry_s) dqoe_order_bottom_list;
	const char *dqoe_top_label;
	const char *dqoe_bottom_label;
	dispatch_queue_t dqoe_top_tq;
	dispatch_queue_t dqoe_bottom_tq;
	int   dqoe_pcs_n;
	void *dqoe_pcs[];
};

static void
_dispatch_introspection_queue_order_dispose(
		dispatch_queue_introspection_context_t dqic)
{
	dispatch_queue_introspection_context_t o_dqic;
	dispatch_queue_order_entry_t e, te;
	dispatch_queue_t otherq;
	LIST_HEAD(, dispatch_queue_order_entry_s) head;

	// this whole thing happens with _dispatch_introspection.queues_lock locked

	_dispatch_unfair_lock_lock(&dqic->dqic_order_top_head_lock);
	LIST_INIT(&head);
	LIST_SWAP(&head, &dqic->dqic_order_top_head,
			dispatch_queue_order_entry_s, dqoe_order_top_list);
	_dispatch_unfair_lock_unlock(&dqic->dqic_order_top_head_lock);

	LIST_FOREACH_SAFE(e, &head, dqoe_order_top_list, te) {
		otherq = e->dqoe_bottom_tq;
		o_dqic = otherq->do_finalizer;
		_dispatch_unfair_lock_lock(&o_dqic->dqic_order_bottom_head_lock);
		LIST_REMOVE(e, dqoe_order_bottom_list);
		_dispatch_unfair_lock_unlock(&o_dqic->dqic_order_bottom_head_lock);
		free(e);
	}

	_dispatch_unfair_lock_lock(&dqic->dqic_order_bottom_head_lock);
	LIST_INIT(&head);
	LIST_SWAP(&head, &dqic->dqic_order_bottom_head,
			dispatch_queue_order_entry_s, dqoe_order_top_list);
	_dispatch_unfair_lock_unlock(&dqic->dqic_order_bottom_head_lock);

	LIST_FOREACH_SAFE(e, &head, dqoe_order_bottom_list, te) {
		otherq = e->dqoe_top_tq;
		o_dqic = otherq->do_finalizer;
		_dispatch_unfair_lock_lock(&o_dqic->dqic_order_top_head_lock);
		LIST_REMOVE(e, dqoe_order_top_list);
		_dispatch_unfair_lock_unlock(&o_dqic->dqic_order_top_head_lock);
		free(e);
	}
}

// caller must make sure dq is not a root quueue
DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
_dispatch_queue_bottom_target_queue(dispatch_queue_t dq)
{
	while (dq->do_targetq->do_targetq) {
		dq = dq->do_targetq;
	}
	return dq;
}

typedef struct dispatch_order_frame_s *dispatch_order_frame_t;
struct dispatch_order_frame_s {
	dispatch_order_frame_t dof_prev;
	dispatch_queue_order_entry_t dof_e;
};

DISPATCH_NOINLINE DISPATCH_NORETURN
static void
_dispatch_introspection_lock_inversion_fail(dispatch_order_frame_t dof,
		dispatch_queue_t top_q, dispatch_queue_t bottom_q)
{
	_SIMPLE_STRING buf = _simple_salloc();
	const char *leading_word = "with";

	_simple_sprintf(buf, "%s Lock inversion detected\n"
			"queue [%s] trying to sync onto queue [%s] conflicts\n",
			DISPATCH_ASSERTION_FAILED_MESSAGE,
			bottom_q->dq_label ?: "", top_q->dq_label ?: "");

	while (dof) {
		dispatch_queue_order_entry_t e = dof->dof_e;
		char **symbols;

		_simple_sprintf(buf,
			"%s queue [%s] syncing onto queue [%s] at:\n", leading_word,
			dof->dof_e->dqoe_bottom_label, dof->dof_e->dqoe_top_label);

		symbols = backtrace_symbols(e->dqoe_pcs, e->dqoe_pcs_n);
		if (symbols) {
			for (int i = 0; i < e->dqoe_pcs_n; i++) {
				_simple_sprintf(buf, "%s\n", symbols[i]);
			}
			free(symbols);
		} else {
			_simple_sappend(buf, "<missing backtrace>\n");
		}

		leading_word = "and";
		dof = dof->dof_prev;
	}

	// <rdar://problem/25053293> turn off the feature for crash handlers
	_dispatch_introspection.debug_queue_inversions = false;
	_dispatch_assert_crash(_simple_string(buf));
	_simple_sfree(buf);
}

static void
_dispatch_introspection_order_check(dispatch_order_frame_t dof_prev,
		dispatch_queue_t top_q, dispatch_queue_t top_tq,
		dispatch_queue_t bottom_q, dispatch_queue_t bottom_tq)
{
	struct dispatch_order_frame_s dof = { .dof_prev = dof_prev };
	dispatch_queue_introspection_context_t btqic = bottom_tq->do_finalizer;

	// has anyone above bottom_tq ever sync()ed onto top_tq ?
	_dispatch_unfair_lock_lock(&btqic->dqic_order_top_head_lock);
	LIST_FOREACH(dof.dof_e, &btqic->dqic_order_top_head, dqoe_order_top_list) {
		if (unlikely(dof.dof_e->dqoe_bottom_tq == top_tq)) {
			_dispatch_introspection_lock_inversion_fail(&dof, top_q, bottom_q);
		}
		_dispatch_introspection_order_check(&dof, top_q, top_tq,
				bottom_q, dof.dof_e->dqoe_bottom_tq);
	}
	_dispatch_unfair_lock_unlock(&btqic->dqic_order_top_head_lock);
}

void
_dispatch_introspection_order_record(dispatch_queue_t top_q)
{
	dispatch_queue_t bottom_q = _dispatch_queue_get_current();
	dispatch_queue_order_entry_t e, it;
	const int pcs_skip = 1, pcs_n_max = 128;
	void *pcs[pcs_n_max];
	int pcs_n;

	if (!bottom_q || !bottom_q->do_targetq || !top_q->do_targetq) {
		return;
	}

	dispatch_queue_t top_tq = _dispatch_queue_bottom_target_queue(top_q);
	dispatch_queue_t bottom_tq = _dispatch_queue_bottom_target_queue(bottom_q);
	dispatch_queue_introspection_context_t ttqic = top_tq->do_finalizer;
	dispatch_queue_introspection_context_t btqic = bottom_tq->do_finalizer;

	_dispatch_unfair_lock_lock(&ttqic->dqic_order_top_head_lock);
	LIST_FOREACH(it, &ttqic->dqic_order_top_head, dqoe_order_top_list) {
		if (it->dqoe_bottom_tq == bottom_tq) {
			// that dispatch_sync() is known and validated
			// move on
			_dispatch_unfair_lock_unlock(&ttqic->dqic_order_top_head_lock);
			return;
		}
	}
	_dispatch_unfair_lock_unlock(&ttqic->dqic_order_top_head_lock);

	_dispatch_introspection_order_check(NULL, top_q, top_tq, bottom_q, bottom_tq);
	pcs_n = MAX(backtrace(pcs, pcs_n_max) - pcs_skip, 0);

	bool copy_top_label = false, copy_bottom_label = false;
	size_t size = sizeof(struct dispatch_queue_order_entry_s)
			+ (size_t)pcs_n * sizeof(void *);

	if (_dispatch_queue_label_needs_free(top_q)) {
		size += strlen(top_q->dq_label) + 1;
		copy_top_label = true;
	}
	if (_dispatch_queue_label_needs_free(bottom_q)) {
		size += strlen(bottom_q->dq_label) + 1;
		copy_bottom_label = true;
	}

	e = _dispatch_calloc(1, size);
	e->dqoe_top_tq = top_tq;
	e->dqoe_bottom_tq = bottom_tq;
	e->dqoe_pcs_n = pcs_n;
	memcpy(e->dqoe_pcs, pcs + pcs_skip, (size_t)pcs_n * sizeof(void *));
	// and then lay out the names of the queues at the end
	char *p = (char *)(e->dqoe_pcs + pcs_n);
	if (copy_top_label) {
		e->dqoe_top_label = strcpy(p, top_q->dq_label);
		p += strlen(p) + 1;
	} else {
		e->dqoe_top_label = top_q->dq_label ?: "";
	}
	if (copy_bottom_label) {
		e->dqoe_bottom_label = strcpy(p, bottom_q->dq_label);
	} else {
		e->dqoe_bottom_label = bottom_q->dq_label ?: "";
	}

	_dispatch_unfair_lock_lock(&ttqic->dqic_order_top_head_lock);
	LIST_FOREACH(it, &ttqic->dqic_order_top_head, dqoe_order_top_list) {
		if (unlikely(it->dqoe_bottom_tq == bottom_tq)) {
			// someone else validated it at the same time
			// go away quickly
			_dispatch_unfair_lock_unlock(&ttqic->dqic_order_top_head_lock);
			free(e);
			return;
		}
	}
	LIST_INSERT_HEAD(&ttqic->dqic_order_top_head, e, dqoe_order_top_list);
	_dispatch_unfair_lock_unlock(&ttqic->dqic_order_top_head_lock);

	_dispatch_unfair_lock_lock(&btqic->dqic_order_bottom_head_lock);
	LIST_INSERT_HEAD(&btqic->dqic_order_bottom_head, e, dqoe_order_bottom_list);
	_dispatch_unfair_lock_unlock(&btqic->dqic_order_bottom_head_lock);
}

void
_dispatch_introspection_target_queue_changed(dispatch_queue_t dq)
{
	if (!_dispatch_introspection.debug_queue_inversions) return;

	if (_dispatch_queue_atomic_flags(dq) & DQF_TARGETED) {
		_dispatch_log(
				"BUG IN CLIENT OF LIBDISPATCH: queue inversion debugging "
				"cannot be used with code that changes the target "
				"of a queue already targeted by other dispatch objects\n"
				"queue %p[%s] was already targeted by other dispatch objects",
				dq, dq->dq_label ?: "");
		_dispatch_introspection.debug_queue_inversions = false;
		return;
	}

	static char const * const reasons[] = {
		[1] = "an initiator",
		[2] = "a recipient",
		[3] = "both an initiator and a recipient"
	};
	dispatch_queue_introspection_context_t dqic = dq->do_finalizer;
	bool as_top = !LIST_EMPTY(&dqic->dqic_order_top_head);
	bool as_bottom = !LIST_EMPTY(&dqic->dqic_order_top_head);

	if (as_top || as_bottom) {
		_dispatch_log(
				"BUG IN CLIENT OF LIBDISPATCH: queue inversion debugging "
				"expects queues to not participate in dispatch_sync() "
				"before their setup is complete\n"
				"forgetting that queue 0x%p[%s] participated as %s of "
				"a dispatch_sync", dq, dq->dq_label ?: "",
				reasons[(int)as_top + 2 * (int)as_bottom]);
		_dispatch_unfair_lock_lock(&_dispatch_introspection.queues_lock);
		_dispatch_introspection_queue_order_dispose(dq->do_finalizer);
		_dispatch_unfair_lock_unlock(&_dispatch_introspection.queues_lock);
	}
}

#endif // DISPATCH_INTROSPECTION
