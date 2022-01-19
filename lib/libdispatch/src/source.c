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
#include "protocol.h"
#include "protocolServer.h"
#endif
#include <sys/mount.h>

static void _dispatch_source_merge_kevent(dispatch_source_t ds,
		const struct kevent64_s *ke);
static bool _dispatch_kevent_register(dispatch_kevent_t *dkp, uint32_t *flgp);
static void _dispatch_kevent_unregister(dispatch_kevent_t dk, uint32_t flg);
static bool _dispatch_kevent_resume(dispatch_kevent_t dk, uint32_t new_flags,
		uint32_t del_flags);
static void _dispatch_kevent_drain(struct kevent64_s *ke);
static void _dispatch_kevent_merge(struct kevent64_s *ke);
static void _dispatch_timers_kevent(struct kevent64_s *ke);
static void _dispatch_timers_unregister(dispatch_source_t ds,
		dispatch_kevent_t dk);
static void _dispatch_timers_update(dispatch_source_t ds);
static void _dispatch_timer_aggregates_check(void);
static void _dispatch_timer_aggregates_register(dispatch_source_t ds);
static void _dispatch_timer_aggregates_update(dispatch_source_t ds,
		unsigned int tidx);
static void _dispatch_timer_aggregates_unregister(dispatch_source_t ds,
		unsigned int tidx);
static inline unsigned long _dispatch_source_timer_data(
		dispatch_source_refs_t dr, unsigned long prev);
static long _dispatch_kq_update(const struct kevent64_s *);
static void _dispatch_memorystatus_init(void);
#if HAVE_MACH
static void _dispatch_mach_host_calendar_change_register(void);
static void _dispatch_mach_recv_msg_buf_init(void);
static kern_return_t _dispatch_kevent_machport_resume(dispatch_kevent_t dk,
		uint32_t new_flags, uint32_t del_flags);
static kern_return_t _dispatch_kevent_mach_notify_resume(dispatch_kevent_t dk,
		uint32_t new_flags, uint32_t del_flags);
static inline void _dispatch_kevent_mach_portset(struct kevent64_s *ke);
#else
static inline void _dispatch_mach_host_calendar_change_register(void) {}
static inline void _dispatch_mach_recv_msg_buf_init(void) {}
#endif
static const char * _evfiltstr(short filt);
#if DISPATCH_DEBUG
static void _dispatch_kevent_debug(struct kevent64_s* kev, const char* str);
static void _dispatch_kevent_debugger(void *context);
#define DISPATCH_ASSERT_ON_MANAGER_QUEUE() \
	dispatch_assert(_dispatch_queue_get_current() == &_dispatch_mgr_q)
#else
static inline void
_dispatch_kevent_debug(struct kevent64_s* kev DISPATCH_UNUSED,
		const char* str DISPATCH_UNUSED) {}
#define DISPATCH_ASSERT_ON_MANAGER_QUEUE()
#endif

#pragma mark -
#pragma mark dispatch_source_t

dispatch_source_t
dispatch_source_create(dispatch_source_type_t type,
	uintptr_t handle,
	unsigned long mask,
	dispatch_queue_t q)
{
	const struct kevent64_s *proto_kev = &type->ke;
	dispatch_source_t ds;
	dispatch_kevent_t dk;

	// input validation
	if (type == NULL || (mask & ~type->mask)) {
		return NULL;
	}

	switch (type->ke.filter) {
	case EVFILT_SIGNAL:
		if (handle >= NSIG) {
			return NULL;
		}
		break;
	case EVFILT_FS:
#if DISPATCH_USE_VM_PRESSURE
	case EVFILT_VM:
#endif
#if DISPATCH_USE_MEMORYSTATUS
	case EVFILT_MEMORYSTATUS:
#endif
	case DISPATCH_EVFILT_CUSTOM_ADD:
	case DISPATCH_EVFILT_CUSTOM_OR:
		if (handle) {
			return NULL;
		}
		break;
	case DISPATCH_EVFILT_TIMER:
		if (!!handle ^ !!type->ke.ident) {
			return NULL;
		}
		break;
	default:
		break;
	}

	ds = _dispatch_alloc(DISPATCH_VTABLE(source),
			sizeof(struct dispatch_source_s));
	// Initialize as a queue first, then override some settings below.
	_dispatch_queue_init((dispatch_queue_t)ds);
	ds->dq_label = "source";

	ds->do_ref_cnt++; // the reference the manager queue holds
	ds->do_ref_cnt++; // since source is created suspended
	ds->do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_INTERVAL;
	// The initial target queue is the manager queue, in order to get
	// the source installed. <rdar://problem/8928171>
	ds->do_targetq = &_dispatch_mgr_q;

	dk = _dispatch_calloc(1ul, sizeof(struct dispatch_kevent_s));
	dk->dk_kevent = *proto_kev;
	dk->dk_kevent.ident = handle;
	dk->dk_kevent.flags |= EV_ADD|EV_ENABLE;
	dk->dk_kevent.fflags |= (uint32_t)mask;
	dk->dk_kevent.udata = (uintptr_t)dk;
	TAILQ_INIT(&dk->dk_sources);

	ds->ds_dkev = dk;
	ds->ds_pending_data_mask = dk->dk_kevent.fflags;
	ds->ds_ident_hack = (uintptr_t)dk->dk_kevent.ident;
	if ((EV_DISPATCH|EV_ONESHOT) & proto_kev->flags) {
		ds->ds_is_level = true;
		ds->ds_needs_rearm = true;
	} else if (!(EV_CLEAR & proto_kev->flags)) {
		// we cheat and use EV_CLEAR to mean a "flag thingy"
		ds->ds_is_adder = true;
	}
	// Some sources require special processing
	if (type->init != NULL) {
		type->init(ds, type, handle, mask, q);
	}
	dispatch_assert(!(ds->ds_is_level && ds->ds_is_adder));

	if (fastpath(!ds->ds_refs)) {
		ds->ds_refs = _dispatch_calloc(1ul,
				sizeof(struct dispatch_source_refs_s));
	}
	ds->ds_refs->dr_source_wref = _dispatch_ptr2wref(ds);

	// First item on the queue sets the user-specified target queue
	dispatch_set_target_queue(ds, q);
	_dispatch_object_debug(ds, "%s", __func__);
	return ds;
}

void
_dispatch_source_dispose(dispatch_source_t ds)
{
	_dispatch_object_debug(ds, "%s", __func__);
	free(ds->ds_refs);
	_dispatch_queue_destroy(ds);
}

void
_dispatch_source_xref_dispose(dispatch_source_t ds)
{
	_dispatch_wakeup(ds);
}

void
dispatch_source_cancel(dispatch_source_t ds)
{
	_dispatch_object_debug(ds, "%s", __func__);
	// Right after we set the cancel flag, someone else
	// could potentially invoke the source, do the cancelation,
	// unregister the source, and deallocate it. We would
	// need to therefore retain/release before setting the bit

	_dispatch_retain(ds);
	(void)dispatch_atomic_or2o(ds, ds_atomic_flags, DSF_CANCELED, relaxed);
	_dispatch_wakeup(ds);
	_dispatch_release(ds);
}

long
dispatch_source_testcancel(dispatch_source_t ds)
{
	return (bool)(ds->ds_atomic_flags & DSF_CANCELED);
}

unsigned long
dispatch_source_get_mask(dispatch_source_t ds)
{
	unsigned long mask = ds->ds_pending_data_mask;
	if (ds->ds_vmpressure_override) {
		mask = NOTE_VM_PRESSURE;
	}
#if TARGET_IPHONE_SIMULATOR
	else if (ds->ds_memorystatus_override) {
		mask = NOTE_MEMORYSTATUS_PRESSURE_WARN;
	}
#endif
	return mask;
}

uintptr_t
dispatch_source_get_handle(dispatch_source_t ds)
{
	unsigned int handle = (unsigned int)ds->ds_ident_hack;
#if TARGET_IPHONE_SIMULATOR
	if (ds->ds_memorystatus_override) {
		handle = 0;
	}
#endif
	return handle;
}

unsigned long
dispatch_source_get_data(dispatch_source_t ds)
{
	unsigned long data = ds->ds_data;
	if (ds->ds_vmpressure_override) {
		data = NOTE_VM_PRESSURE;
	}
#if TARGET_IPHONE_SIMULATOR
	else if (ds->ds_memorystatus_override) {
		data = NOTE_MEMORYSTATUS_PRESSURE_WARN;
	}
#endif
	return data;
}

void
dispatch_source_merge_data(dispatch_source_t ds, unsigned long val)
{
	struct kevent64_s kev = {
		.fflags = (typeof(kev.fflags))val,
		.data = (typeof(kev.data))val,
	};

	dispatch_assert(
			ds->ds_dkev->dk_kevent.filter == DISPATCH_EVFILT_CUSTOM_ADD ||
			ds->ds_dkev->dk_kevent.filter == DISPATCH_EVFILT_CUSTOM_OR);

	_dispatch_source_merge_kevent(ds, &kev);
}

#pragma mark -
#pragma mark dispatch_source_handler

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_source_handler_alloc(dispatch_source_t ds, void *handler, long kind,
		bool block)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	if (handler) {
		dc->do_vtable = (void *)((block ? DISPATCH_OBJ_BLOCK_RELEASE_BIT :
				DISPATCH_OBJ_CTXT_FETCH_BIT) | (kind != DS_EVENT_HANDLER ?
				DISPATCH_OBJ_ASYNC_BIT : 0l));
		dc->dc_priority = 0;
		dc->dc_voucher = NULL;
		if (block) {
#ifdef __BLOCKS__
			if (slowpath(_dispatch_block_has_private_data(handler))) {
				// sources don't propagate priority by default
				dispatch_block_flags_t flags = DISPATCH_BLOCK_NO_QOS_CLASS;
				flags |= _dispatch_block_get_flags(handler);
				_dispatch_continuation_priority_set(dc,
						_dispatch_block_get_priority(handler), flags);
			}
			if (kind != DS_EVENT_HANDLER) {
				dc->dc_func = _dispatch_call_block_and_release;
			} else {
				dc->dc_func = _dispatch_Block_invoke(handler);
			}
			dc->dc_ctxt = _dispatch_Block_copy(handler);
#endif /* __BLOCKS__ */
		} else {
			dc->dc_func = handler;
			dc->dc_ctxt = ds->do_ctxt;
		}
		_dispatch_trace_continuation_push((dispatch_queue_t)ds, dc);
	} else {
		dc->dc_func = NULL;
	}
	dc->dc_data = (void*)kind;
	return dc;
}

static inline void
_dispatch_source_handler_replace(dispatch_source_refs_t dr, long kind,
		dispatch_continuation_t dc_new)
{
	dispatch_continuation_t dc = dr->ds_handler[kind];
	if (dc) {
#ifdef __BLOCKS__
		if ((long)dc->do_vtable & DISPATCH_OBJ_BLOCK_RELEASE_BIT) {
			Block_release(dc->dc_ctxt);
		}
#endif /* __BLOCKS__ */
		if (dc->dc_voucher) {
			_voucher_release(dc->dc_voucher);
			dc->dc_voucher = NULL;
		}
		_dispatch_continuation_free(dc);
	}
	dr->ds_handler[kind] = dc_new;
}

static inline void
_dispatch_source_handler_free(dispatch_source_refs_t dr, long kind)
{
	_dispatch_source_handler_replace(dr, kind, NULL);
}

static void
_dispatch_source_set_handler(void *context)
{
	dispatch_source_t ds = (dispatch_source_t)_dispatch_queue_get_current();
	dispatch_assert(dx_type(ds) == DISPATCH_SOURCE_KEVENT_TYPE);
	dispatch_continuation_t dc = context;
	long kind = (long)dc->dc_data;
	dc->dc_data = 0;
	if (!dc->dc_func) {
		_dispatch_continuation_free(dc);
		dc = NULL;
	} else if ((long)dc->do_vtable & DISPATCH_OBJ_CTXT_FETCH_BIT) {
		dc->dc_ctxt = ds->do_ctxt;
	}
	_dispatch_source_handler_replace(ds->ds_refs, kind, dc);
	if (kind == DS_EVENT_HANDLER && dc && dc->dc_priority) {
#if HAVE_PTHREAD_WORKQUEUE_QOS
		ds->dq_priority = dc->dc_priority & ~_PTHREAD_PRIORITY_FLAGS_MASK;
		_dispatch_queue_set_override_priority((dispatch_queue_t)ds);
#endif
	}
}

#ifdef __BLOCKS__
void
dispatch_source_set_event_handler(dispatch_source_t ds,
		dispatch_block_t handler)
{
	dispatch_continuation_t dc;
	dc = _dispatch_source_handler_alloc(ds, handler, DS_EVENT_HANDLER, true);
	_dispatch_barrier_trysync_f((dispatch_queue_t)ds, dc,
			_dispatch_source_set_handler);
}
#endif /* __BLOCKS__ */

void
dispatch_source_set_event_handler_f(dispatch_source_t ds,
		dispatch_function_t handler)
{
	dispatch_continuation_t dc;
	dc = _dispatch_source_handler_alloc(ds, handler, DS_EVENT_HANDLER, false);
	_dispatch_barrier_trysync_f((dispatch_queue_t)ds, dc,
			_dispatch_source_set_handler);
}

void
_dispatch_source_set_event_handler_with_context_f(dispatch_source_t ds,
		void *ctxt, dispatch_function_t handler)
{
	dispatch_continuation_t dc;
	dc = _dispatch_source_handler_alloc(ds, handler, DS_EVENT_HANDLER, false);
	dc->do_vtable = (void *)((long)dc->do_vtable &~DISPATCH_OBJ_CTXT_FETCH_BIT);
	dc->dc_other = dc->dc_ctxt;
	dc->dc_ctxt = ctxt;
	_dispatch_barrier_trysync_f((dispatch_queue_t)ds, dc,
			_dispatch_source_set_handler);
}

#ifdef __BLOCKS__
void
dispatch_source_set_cancel_handler(dispatch_source_t ds,
		dispatch_block_t handler)
{
	dispatch_continuation_t dc;
	dc = _dispatch_source_handler_alloc(ds, handler, DS_CANCEL_HANDLER, true);
	_dispatch_barrier_trysync_f((dispatch_queue_t)ds, dc,
			_dispatch_source_set_handler);
}
#endif /* __BLOCKS__ */

void
dispatch_source_set_cancel_handler_f(dispatch_source_t ds,
		dispatch_function_t handler)
{
	dispatch_continuation_t dc;
	dc = _dispatch_source_handler_alloc(ds, handler, DS_CANCEL_HANDLER, false);
	_dispatch_barrier_trysync_f((dispatch_queue_t)ds, dc,
			_dispatch_source_set_handler);
}

#ifdef __BLOCKS__
void
dispatch_source_set_registration_handler(dispatch_source_t ds,
		dispatch_block_t handler)
{
	dispatch_continuation_t dc;
	dc = _dispatch_source_handler_alloc(ds, handler, DS_REGISTN_HANDLER, true);
	_dispatch_barrier_trysync_f((dispatch_queue_t)ds, dc,
			_dispatch_source_set_handler);
}
#endif /* __BLOCKS__ */

void
dispatch_source_set_registration_handler_f(dispatch_source_t ds,
	dispatch_function_t handler)
{
	dispatch_continuation_t dc;
	dc = _dispatch_source_handler_alloc(ds, handler, DS_REGISTN_HANDLER, false);
	_dispatch_barrier_trysync_f((dispatch_queue_t)ds, dc,
			_dispatch_source_set_handler);
}

#pragma mark -
#pragma mark dispatch_source_invoke

static void
_dispatch_source_registration_callout(dispatch_source_t ds)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_continuation_t dc = dr->ds_handler[DS_REGISTN_HANDLER];
	if ((ds->ds_atomic_flags & DSF_CANCELED) || (ds->do_xref_cnt == -1)) {
		// no registration callout if source is canceled rdar://problem/8955246
		return _dispatch_source_handler_free(dr, DS_REGISTN_HANDLER);
	}
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(ds->dq_priority);
	if ((long)dc->do_vtable & DISPATCH_OBJ_CTXT_FETCH_BIT) {
		dc->dc_ctxt = ds->do_ctxt;
	}
	_dispatch_continuation_pop(dc);
	dr->ds_handler[DS_REGISTN_HANDLER] = NULL;
	_dispatch_reset_defaultpriority(old_dp);
}

static void
_dispatch_source_cancel_callout(dispatch_source_t ds)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_continuation_t dc = dr->ds_handler[DS_CANCEL_HANDLER];
	ds->ds_pending_data_mask = 0;
	ds->ds_pending_data = 0;
	ds->ds_data = 0;
	_dispatch_source_handler_free(dr, DS_EVENT_HANDLER);
	_dispatch_source_handler_free(dr, DS_REGISTN_HANDLER);
	if (!dc) {
		return;
	}
	if (!(ds->ds_atomic_flags & DSF_CANCELED)) {
		return _dispatch_source_handler_free(dr, DS_CANCEL_HANDLER);
	}
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(ds->dq_priority);
	if ((long)dc->do_vtable & DISPATCH_OBJ_CTXT_FETCH_BIT) {
		dc->dc_ctxt = ds->do_ctxt;
	}
	_dispatch_continuation_pop(dc);
	dr->ds_handler[DS_CANCEL_HANDLER] = NULL;
	_dispatch_reset_defaultpriority(old_dp);
}

static void
_dispatch_source_latch_and_call(dispatch_source_t ds)
{
	unsigned long prev;

	if ((ds->ds_atomic_flags & DSF_CANCELED) || (ds->do_xref_cnt == -1)) {
		return;
	}
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_continuation_t dc = dr->ds_handler[DS_EVENT_HANDLER];
	prev = dispatch_atomic_xchg2o(ds, ds_pending_data, 0, relaxed);
	if (ds->ds_is_level) {
		ds->ds_data = ~prev;
	} else if (ds->ds_is_timer && ds_timer(dr).target && prev) {
		ds->ds_data = _dispatch_source_timer_data(dr, prev);
	} else {
		ds->ds_data = prev;
	}
	if (!dispatch_assume(prev) || !dc) {
		return;
	}
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(ds->dq_priority);
	_dispatch_trace_continuation_pop(_dispatch_queue_get_current(), dc);
	voucher_t voucher = dc->dc_voucher ? _voucher_retain(dc->dc_voucher) : NULL;
	_dispatch_continuation_voucher_adopt(dc); // consumes voucher reference
	_dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
	_dispatch_introspection_queue_item_complete(dc);
	if (voucher) dc->dc_voucher = voucher;
	_dispatch_reset_defaultpriority(old_dp);
}

static void
_dispatch_source_kevent_unregister(dispatch_source_t ds)
{
	_dispatch_object_debug(ds, "%s", __func__);
	dispatch_kevent_t dk = ds->ds_dkev;
	ds->ds_dkev = NULL;
	switch (dk->dk_kevent.filter) {
	case DISPATCH_EVFILT_TIMER:
		_dispatch_timers_unregister(ds, dk);
		break;
	default:
		TAILQ_REMOVE(&dk->dk_sources, ds->ds_refs, dr_list);
		_dispatch_kevent_unregister(dk, (uint32_t)ds->ds_pending_data_mask);
		break;
	}

	(void)dispatch_atomic_and2o(ds, ds_atomic_flags, ~DSF_ARMED, relaxed);
	ds->ds_needs_rearm = false; // re-arm is pointless and bad now
	_dispatch_release(ds); // the retain is done at creation time
}

static void
_dispatch_source_kevent_resume(dispatch_source_t ds, uint32_t new_flags)
{
	switch (ds->ds_dkev->dk_kevent.filter) {
	case DISPATCH_EVFILT_TIMER:
		return _dispatch_timers_update(ds);
	case EVFILT_MACHPORT:
		if (ds->ds_pending_data_mask & DISPATCH_MACH_RECV_MESSAGE) {
			new_flags |= DISPATCH_MACH_RECV_MESSAGE; // emulate EV_DISPATCH
		}
		break;
	}
	if (_dispatch_kevent_resume(ds->ds_dkev, new_flags, 0)) {
		_dispatch_source_kevent_unregister(ds);
	}
}

static void
_dispatch_source_kevent_register(dispatch_source_t ds)
{
	dispatch_assert_zero(ds->ds_is_installed);
	switch (ds->ds_dkev->dk_kevent.filter) {
	case DISPATCH_EVFILT_TIMER:
		return _dispatch_timers_update(ds);
	}
	uint32_t flags;
	bool do_resume = _dispatch_kevent_register(&ds->ds_dkev, &flags);
	TAILQ_INSERT_TAIL(&ds->ds_dkev->dk_sources, ds->ds_refs, dr_list);
	if (do_resume || ds->ds_needs_rearm) {
		_dispatch_source_kevent_resume(ds, flags);
	}
	(void)dispatch_atomic_or2o(ds, ds_atomic_flags, DSF_ARMED, relaxed);
	_dispatch_object_debug(ds, "%s", __func__);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
_dispatch_source_invoke2(dispatch_object_t dou,
		_dispatch_thread_semaphore_t *sema_ptr DISPATCH_UNUSED)
{
	dispatch_source_t ds = dou._ds;
	if (slowpath(_dispatch_queue_drain(ds))) {
		DISPATCH_CLIENT_CRASH("Sync onto source");
	}

	// This function performs all source actions. Each action is responsible
	// for verifying that it takes place on the appropriate queue. If the
	// current queue is not the correct queue for this action, the correct queue
	// will be returned and the invoke will be re-driven on that queue.

	// The order of tests here in invoke and in probe should be consistent.

	dispatch_queue_t dq = _dispatch_queue_get_current();
	dispatch_source_refs_t dr = ds->ds_refs;

	if (!ds->ds_is_installed) {
		// The source needs to be installed on the manager queue.
		if (dq != &_dispatch_mgr_q) {
			return &_dispatch_mgr_q;
		}
		_dispatch_source_kevent_register(ds);
		ds->ds_is_installed = true;
		if (dr->ds_handler[DS_REGISTN_HANDLER]) {
			return ds->do_targetq;
		}
		if (slowpath(ds->do_xref_cnt == -1)) {
			return &_dispatch_mgr_q; // rdar://problem/9558246
		}
	} else if (slowpath(DISPATCH_OBJECT_SUSPENDED(ds))) {
		// Source suspended by an item drained from the source queue.
		return NULL;
	} else if (dr->ds_handler[DS_REGISTN_HANDLER]) {
		// The source has been registered and the registration handler needs
		// to be delivered on the target queue.
		if (dq != ds->do_targetq) {
			return ds->do_targetq;
		}
		// clears ds_registration_handler
		_dispatch_source_registration_callout(ds);
		if (slowpath(ds->do_xref_cnt == -1)) {
			return &_dispatch_mgr_q; // rdar://problem/9558246
		}
	} else if ((ds->ds_atomic_flags & DSF_CANCELED) || (ds->do_xref_cnt == -1)){
		// The source has been cancelled and needs to be uninstalled from the
		// manager queue. After uninstallation, the cancellation handler needs
		// to be delivered to the target queue.
		if (ds->ds_dkev) {
			if (dq != &_dispatch_mgr_q) {
				return &_dispatch_mgr_q;
			}
			_dispatch_source_kevent_unregister(ds);
		}
		if (dr->ds_handler[DS_EVENT_HANDLER] ||
				dr->ds_handler[DS_CANCEL_HANDLER] ||
				dr->ds_handler[DS_REGISTN_HANDLER]) {
			if (dq != ds->do_targetq) {
				return ds->do_targetq;
			}
		}
		_dispatch_source_cancel_callout(ds);
	} else if (ds->ds_pending_data) {
		// The source has pending data to deliver via the event handler callback
		// on the target queue. Some sources need to be rearmed on the manager
		// queue after event delivery.
		if (dq != ds->do_targetq) {
			return ds->do_targetq;
		}
		_dispatch_source_latch_and_call(ds);
		if (ds->ds_needs_rearm) {
			return &_dispatch_mgr_q;
		}
	} else if (ds->ds_needs_rearm && !(ds->ds_atomic_flags & DSF_ARMED)) {
		// The source needs to be rearmed on the manager queue.
		if (dq != &_dispatch_mgr_q) {
			return &_dispatch_mgr_q;
		}
		_dispatch_source_kevent_resume(ds, 0);
		(void)dispatch_atomic_or2o(ds, ds_atomic_flags, DSF_ARMED, relaxed);
	}

	return NULL;
}

DISPATCH_NOINLINE
void
_dispatch_source_invoke(dispatch_source_t ds)
{
	_dispatch_queue_class_invoke(ds, _dispatch_source_invoke2);
}

unsigned long
_dispatch_source_probe(dispatch_source_t ds)
{
	// This function determines whether the source needs to be invoked.
	// The order of tests here in probe and in invoke should be consistent.

	dispatch_source_refs_t dr = ds->ds_refs;
	if (!ds->ds_is_installed) {
		// The source needs to be installed on the manager queue.
		return true;
	} else if (dr->ds_handler[DS_REGISTN_HANDLER]) {
		// The registration handler needs to be delivered to the target queue.
		return true;
	} else if ((ds->ds_atomic_flags & DSF_CANCELED) || (ds->do_xref_cnt == -1)){
		// The source needs to be uninstalled from the manager queue, or the
		// cancellation handler needs to be delivered to the target queue.
		// Note: cancellation assumes installation.
		if (ds->ds_dkev || dr->ds_handler[DS_EVENT_HANDLER] ||
				dr->ds_handler[DS_CANCEL_HANDLER] ||
				dr->ds_handler[DS_REGISTN_HANDLER]) {
			return true;
		}
	} else if (ds->ds_pending_data) {
		// The source has pending data to deliver to the target queue.
		return true;
	} else if (ds->ds_needs_rearm && !(ds->ds_atomic_flags & DSF_ARMED)) {
		// The source needs to be rearmed on the manager queue.
		return true;
	}
	return _dispatch_queue_class_probe(ds);
}

static void
_dispatch_source_merge_kevent(dispatch_source_t ds, const struct kevent64_s *ke)
{
	if ((ds->ds_atomic_flags & DSF_CANCELED) || (ds->do_xref_cnt == -1)) {
		return;
	}
	if (ds->ds_is_level) {
		// ke->data is signed and "negative available data" makes no sense
		// zero bytes happens when EV_EOF is set
		// 10A268 does not fail this assert with EVFILT_READ and a 10 GB file
		dispatch_assert(ke->data >= 0l);
		dispatch_atomic_store2o(ds, ds_pending_data, ~(unsigned long)ke->data,
				relaxed);
	} else if (ds->ds_is_adder) {
		(void)dispatch_atomic_add2o(ds, ds_pending_data,
				(unsigned long)ke->data, relaxed);
	} else if (ke->fflags & ds->ds_pending_data_mask) {
		(void)dispatch_atomic_or2o(ds, ds_pending_data,
				ke->fflags & ds->ds_pending_data_mask, relaxed);
	}
	// EV_DISPATCH and EV_ONESHOT sources are no longer armed after delivery
	if (ds->ds_needs_rearm) {
		(void)dispatch_atomic_and2o(ds, ds_atomic_flags, ~DSF_ARMED, relaxed);
	}

	_dispatch_wakeup(ds);
}

#pragma mark -
#pragma mark dispatch_kevent_t

#if DISPATCH_USE_GUARDED_FD_CHANGE_FDGUARD
static void _dispatch_kevent_guard(dispatch_kevent_t dk);
static void _dispatch_kevent_unguard(dispatch_kevent_t dk);
#else
static inline void _dispatch_kevent_guard(dispatch_kevent_t dk) { (void)dk; }
static inline void _dispatch_kevent_unguard(dispatch_kevent_t dk) { (void)dk; }
#endif

static struct dispatch_kevent_s _dispatch_kevent_data_or = {
	.dk_kevent = {
		.filter = DISPATCH_EVFILT_CUSTOM_OR,
		.flags = EV_CLEAR,
	},
	.dk_sources = TAILQ_HEAD_INITIALIZER(_dispatch_kevent_data_or.dk_sources),
};
static struct dispatch_kevent_s _dispatch_kevent_data_add = {
	.dk_kevent = {
		.filter = DISPATCH_EVFILT_CUSTOM_ADD,
	},
	.dk_sources = TAILQ_HEAD_INITIALIZER(_dispatch_kevent_data_add.dk_sources),
};

#define DSL_HASH(x) ((x) & (DSL_HASH_SIZE - 1))

DISPATCH_CACHELINE_ALIGN
static TAILQ_HEAD(, dispatch_kevent_s) _dispatch_sources[DSL_HASH_SIZE];

static void
_dispatch_kevent_init()
{
	unsigned int i;
	for (i = 0; i < DSL_HASH_SIZE; i++) {
		TAILQ_INIT(&_dispatch_sources[i]);
	}

	TAILQ_INSERT_TAIL(&_dispatch_sources[0],
			&_dispatch_kevent_data_or, dk_list);
	TAILQ_INSERT_TAIL(&_dispatch_sources[0],
			&_dispatch_kevent_data_add, dk_list);
	_dispatch_kevent_data_or.dk_kevent.udata =
			(uintptr_t)&_dispatch_kevent_data_or;
	_dispatch_kevent_data_add.dk_kevent.udata =
			(uintptr_t)&_dispatch_kevent_data_add;
}

static inline uintptr_t
_dispatch_kevent_hash(uint64_t ident, short filter)
{
	uint64_t value;
#if HAVE_MACH
	value = (filter == EVFILT_MACHPORT ||
			filter == DISPATCH_EVFILT_MACH_NOTIFICATION ?
			MACH_PORT_INDEX(ident) : ident);
#else
	value = ident;
#endif
	return DSL_HASH((uintptr_t)value);
}

static dispatch_kevent_t
_dispatch_kevent_find(uint64_t ident, short filter)
{
	uintptr_t hash = _dispatch_kevent_hash(ident, filter);
	dispatch_kevent_t dki;

	TAILQ_FOREACH(dki, &_dispatch_sources[hash], dk_list) {
		if (dki->dk_kevent.ident == ident && dki->dk_kevent.filter == filter) {
			break;
		}
	}
	return dki;
}

static void
_dispatch_kevent_insert(dispatch_kevent_t dk)
{
	_dispatch_kevent_guard(dk);
	uintptr_t hash = _dispatch_kevent_hash(dk->dk_kevent.ident,
			dk->dk_kevent.filter);
	TAILQ_INSERT_TAIL(&_dispatch_sources[hash], dk, dk_list);
}

// Find existing kevents, and merge any new flags if necessary
static bool
_dispatch_kevent_register(dispatch_kevent_t *dkp, uint32_t *flgp)
{
	dispatch_kevent_t dk, ds_dkev = *dkp;
	uint32_t new_flags;
	bool do_resume = false;

	dk = _dispatch_kevent_find(ds_dkev->dk_kevent.ident,
			ds_dkev->dk_kevent.filter);
	if (dk) {
		// If an existing dispatch kevent is found, check to see if new flags
		// need to be added to the existing kevent
		new_flags = ~dk->dk_kevent.fflags & ds_dkev->dk_kevent.fflags;
		dk->dk_kevent.fflags |= ds_dkev->dk_kevent.fflags;
		free(ds_dkev);
		*dkp = dk;
		do_resume = new_flags;
	} else {
		dk = ds_dkev;
		_dispatch_kevent_insert(dk);
		new_flags = dk->dk_kevent.fflags;
		do_resume = true;
	}
	// Re-register the kevent with the kernel if new flags were added
	// by the dispatch kevent
	if (do_resume) {
		dk->dk_kevent.flags |= EV_ADD;
	}
	*flgp = new_flags;
	return do_resume;
}

static bool
_dispatch_kevent_resume(dispatch_kevent_t dk, uint32_t new_flags,
		uint32_t del_flags)
{
	long r;
	switch (dk->dk_kevent.filter) {
	case DISPATCH_EVFILT_TIMER:
	case DISPATCH_EVFILT_CUSTOM_ADD:
	case DISPATCH_EVFILT_CUSTOM_OR:
		// these types not registered with kevent
		return 0;
#if HAVE_MACH
	case EVFILT_MACHPORT:
		return _dispatch_kevent_machport_resume(dk, new_flags, del_flags);
	case DISPATCH_EVFILT_MACH_NOTIFICATION:
		return _dispatch_kevent_mach_notify_resume(dk, new_flags, del_flags);
#endif
	case EVFILT_PROC:
		if (dk->dk_kevent.flags & EV_ONESHOT) {
			return 0;
		}
		// fall through
	default:
		r = _dispatch_kq_update(&dk->dk_kevent);
		if (dk->dk_kevent.flags & EV_DISPATCH) {
			dk->dk_kevent.flags &= ~EV_ADD;
		}
		return r;
	}
}

static void
_dispatch_kevent_dispose(dispatch_kevent_t dk)
{
	uintptr_t hash;

	switch (dk->dk_kevent.filter) {
	case DISPATCH_EVFILT_TIMER:
	case DISPATCH_EVFILT_CUSTOM_ADD:
	case DISPATCH_EVFILT_CUSTOM_OR:
		// these sources live on statically allocated lists
		return;
#if HAVE_MACH
	case EVFILT_MACHPORT:
		_dispatch_kevent_machport_resume(dk, 0, dk->dk_kevent.fflags);
		break;
	case DISPATCH_EVFILT_MACH_NOTIFICATION:
		_dispatch_kevent_mach_notify_resume(dk, 0, dk->dk_kevent.fflags);
		break;
#endif
	case EVFILT_PROC:
		if (dk->dk_kevent.flags & EV_ONESHOT) {
			break; // implicitly deleted
		}
		// fall through
	default:
		if (~dk->dk_kevent.flags & EV_DELETE) {
			dk->dk_kevent.flags |= EV_DELETE;
			dk->dk_kevent.flags &= ~(EV_ADD|EV_ENABLE);
			_dispatch_kq_update(&dk->dk_kevent);
		}
		break;
	}

	hash = _dispatch_kevent_hash(dk->dk_kevent.ident,
			dk->dk_kevent.filter);
	TAILQ_REMOVE(&_dispatch_sources[hash], dk, dk_list);
	_dispatch_kevent_unguard(dk);
	free(dk);
}

static void
_dispatch_kevent_unregister(dispatch_kevent_t dk, uint32_t flg)
{
	dispatch_source_refs_t dri;
	uint32_t del_flags, fflags = 0;

	if (TAILQ_EMPTY(&dk->dk_sources)) {
		_dispatch_kevent_dispose(dk);
	} else {
		TAILQ_FOREACH(dri, &dk->dk_sources, dr_list) {
			dispatch_source_t dsi = _dispatch_source_from_refs(dri);
			uint32_t mask = (uint32_t)dsi->ds_pending_data_mask;
			fflags |= mask;
		}
		del_flags = flg & ~fflags;
		if (del_flags) {
			dk->dk_kevent.flags |= EV_ADD;
			dk->dk_kevent.fflags = fflags;
			_dispatch_kevent_resume(dk, 0, del_flags);
		}
	}
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_proc_exit(struct kevent64_s *ke)
{
	// EVFILT_PROC may fail with ESRCH when the process exists but is a zombie
	// <rdar://problem/5067725>. As a workaround, we simulate an exit event for
	// any EVFILT_PROC with an invalid pid <rdar://problem/6626350>.
	struct kevent64_s fake;
	fake = *ke;
	fake.flags &= ~EV_ERROR;
	fake.fflags = NOTE_EXIT;
	fake.data = 0;
	_dispatch_kevent_drain(&fake);
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_error(struct kevent64_s *ke)
{
	_dispatch_kevent_debug(ke, __func__);
	if (ke->data) {
		// log the unexpected error
		_dispatch_bug_kevent_client("kevent", _evfiltstr(ke->filter),
				ke->flags & EV_DELETE ? "delete" :
				ke->flags & EV_ADD ? "add" :
				ke->flags & EV_ENABLE ? "enable" : "monitor",
				(int)ke->data);
	}
}

static void
_dispatch_kevent_drain(struct kevent64_s *ke)
{
#if DISPATCH_DEBUG
	static dispatch_once_t pred;
	dispatch_once_f(&pred, NULL, _dispatch_kevent_debugger);
#endif
	if (ke->filter == EVFILT_USER) {
		return;
	}
	if (slowpath(ke->flags & EV_ERROR)) {
		if (ke->filter == EVFILT_PROC) {
			if (ke->flags & EV_DELETE) {
				// Process exited while monitored
				return;
			} else if (ke->data == ESRCH) {
				return _dispatch_kevent_proc_exit(ke);
			}
		}
		return _dispatch_kevent_error(ke);
	}
	_dispatch_kevent_debug(ke, __func__);
	if (ke->filter == EVFILT_TIMER) {
		return _dispatch_timers_kevent(ke);
	}
#if HAVE_MACH
	if (ke->filter == EVFILT_MACHPORT) {
		return _dispatch_kevent_mach_portset(ke);
	}
#endif
	return _dispatch_kevent_merge(ke);
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_merge(struct kevent64_s *ke)
{
	dispatch_kevent_t dk;
	dispatch_source_refs_t dri;

	dk = (void*)ke->udata;
	dispatch_assert(dk);

	if (ke->flags & EV_ONESHOT) {
		dk->dk_kevent.flags |= EV_ONESHOT;
	}
	TAILQ_FOREACH(dri, &dk->dk_sources, dr_list) {
		_dispatch_source_merge_kevent(_dispatch_source_from_refs(dri), ke);
	}
}

#if DISPATCH_USE_GUARDED_FD_CHANGE_FDGUARD
static void
_dispatch_kevent_guard(dispatch_kevent_t dk)
{
	guardid_t guard;
	const unsigned int guard_flags = GUARD_CLOSE;
	int r, fd_flags = 0;
	switch (dk->dk_kevent.filter) {
	case EVFILT_READ:
	case EVFILT_WRITE:
	case EVFILT_VNODE:
		guard = &dk->dk_kevent;
		r = change_fdguard_np((int)dk->dk_kevent.ident, NULL, 0,
				&guard, guard_flags, &fd_flags);
		if (slowpath(r == -1)) {
			int err = errno;
			if (err != EPERM) {
				(void)dispatch_assume_zero(err);
			}
			return;
		}
		dk->dk_kevent.ext[0] = guard_flags;
		dk->dk_kevent.ext[1] = fd_flags;
		break;
	}
}

static void
_dispatch_kevent_unguard(dispatch_kevent_t dk)
{
	guardid_t guard;
	unsigned int guard_flags;
	int r, fd_flags;
	switch (dk->dk_kevent.filter) {
	case EVFILT_READ:
	case EVFILT_WRITE:
	case EVFILT_VNODE:
		guard_flags = (unsigned int)dk->dk_kevent.ext[0];
		if (!guard_flags) {
			return;
		}
		guard = &dk->dk_kevent;
		fd_flags = (int)dk->dk_kevent.ext[1];
		r = change_fdguard_np((int)dk->dk_kevent.ident, &guard,
				guard_flags, NULL, 0, &fd_flags);
		if (slowpath(r == -1)) {
			(void)dispatch_assume_zero(errno);
			return;
		}
		dk->dk_kevent.ext[0] = 0;
		break;
	}
}
#endif // DISPATCH_USE_GUARDED_FD_CHANGE_FDGUARD

#pragma mark -
#pragma mark dispatch_source_timer

#if DISPATCH_USE_DTRACE
static dispatch_source_refs_t
		_dispatch_trace_next_timer[DISPATCH_TIMER_QOS_COUNT];
#define _dispatch_trace_next_timer_set(x, q) \
		_dispatch_trace_next_timer[(q)] = (x)
#define _dispatch_trace_next_timer_program(d, q) \
		_dispatch_trace_timer_program(_dispatch_trace_next_timer[(q)], (d))
#define _dispatch_trace_next_timer_wake(q) \
		_dispatch_trace_timer_wake(_dispatch_trace_next_timer[(q)])
#else
#define _dispatch_trace_next_timer_set(x, q)
#define _dispatch_trace_next_timer_program(d, q)
#define _dispatch_trace_next_timer_wake(q)
#endif

#define _dispatch_source_timer_telemetry_enabled() false

DISPATCH_NOINLINE
static void
_dispatch_source_timer_telemetry_slow(dispatch_source_t ds,
		uintptr_t ident, struct dispatch_timer_source_s *values)
{
	if (_dispatch_trace_timer_configure_enabled()) {
		_dispatch_trace_timer_configure(ds, ident, values);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_source_timer_telemetry(dispatch_source_t ds, uintptr_t ident,
		struct dispatch_timer_source_s *values)
{
	if (_dispatch_trace_timer_configure_enabled() ||
			_dispatch_source_timer_telemetry_enabled()) {
		_dispatch_source_timer_telemetry_slow(ds, ident, values);
		asm(""); // prevent tailcall
	}
}

// approx 1 year (60s * 60m * 24h * 365d)
#define FOREVER_NSEC 31536000000000000ull

DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dispatch_source_timer_now(uint64_t nows[], unsigned int tidx)
{
	unsigned int tk = DISPATCH_TIMER_KIND(tidx);
	if (nows && fastpath(nows[tk])) {
		return nows[tk];
	}
	uint64_t now;
	switch (tk) {
	case DISPATCH_TIMER_KIND_MACH:
		now = _dispatch_absolute_time();
		break;
	case DISPATCH_TIMER_KIND_WALL:
		now = _dispatch_get_nanoseconds();
		break;
	}
	if (nows) {
		nows[tk] = now;
	}
	return now;
}

static inline unsigned long
_dispatch_source_timer_data(dispatch_source_refs_t dr, unsigned long prev)
{
	// calculate the number of intervals since last fire
	unsigned long data, missed;
	uint64_t now;
	now = _dispatch_source_timer_now(NULL, _dispatch_source_timer_idx(dr));
	missed = (unsigned long)((now - ds_timer(dr).last_fire) /
			ds_timer(dr).interval);
	// correct for missed intervals already delivered last time
	data = prev - ds_timer(dr).missed + missed;
	ds_timer(dr).missed = missed;
	return data;
}

struct dispatch_set_timer_params {
	dispatch_source_t ds;
	uintptr_t ident;
	struct dispatch_timer_source_s values;
};

static void
_dispatch_source_set_timer3(void *context)
{
	// Called on the _dispatch_mgr_q
	struct dispatch_set_timer_params *params = context;
	dispatch_source_t ds = params->ds;
	ds->ds_ident_hack = params->ident;
	ds_timer(ds->ds_refs) = params->values;
	// Clear any pending data that might have accumulated on
	// older timer params <rdar://problem/8574886>
	ds->ds_pending_data = 0;
	// Re-arm in case we got disarmed because of pending set_timer suspension
	(void)dispatch_atomic_or2o(ds, ds_atomic_flags, DSF_ARMED, release);
	dispatch_resume(ds);
	// Must happen after resume to avoid getting disarmed due to suspension
	_dispatch_timers_update(ds);
	dispatch_release(ds);
	if (params->values.flags & DISPATCH_TIMER_WALL_CLOCK) {
		_dispatch_mach_host_calendar_change_register();
	}
	free(params);
}

static void
_dispatch_source_set_timer2(void *context)
{
	// Called on the source queue
	struct dispatch_set_timer_params *params = context;
	dispatch_suspend(params->ds);
	_dispatch_barrier_async_detached_f(&_dispatch_mgr_q, params,
			_dispatch_source_set_timer3);
}

DISPATCH_NOINLINE
static struct dispatch_set_timer_params *
_dispatch_source_timer_params(dispatch_source_t ds, dispatch_time_t start,
		uint64_t interval, uint64_t leeway)
{
	struct dispatch_set_timer_params *params;
	params = _dispatch_calloc(1ul, sizeof(struct dispatch_set_timer_params));
	params->ds = ds;
	params->values.flags = ds_timer(ds->ds_refs).flags;

	if (interval == 0) {
		// we use zero internally to mean disabled
		interval = 1;
	} else if ((int64_t)interval < 0) {
		// 6866347 - make sure nanoseconds won't overflow
		interval = INT64_MAX;
	}
	if ((int64_t)leeway < 0) {
		leeway = INT64_MAX;
	}
	if (start == DISPATCH_TIME_NOW) {
		start = _dispatch_absolute_time();
	} else if (start == DISPATCH_TIME_FOREVER) {
		start = INT64_MAX;
	}

	if ((int64_t)start < 0) {
		// wall clock
		start = (dispatch_time_t)-((int64_t)start);
		params->values.flags |= DISPATCH_TIMER_WALL_CLOCK;
	} else {
		// absolute clock
		interval = _dispatch_time_nano2mach(interval);
		if (interval < 1) {
			// rdar://problem/7287561 interval must be at least one in
			// in order to avoid later division by zero when calculating
			// the missed interval count. (NOTE: the wall clock's
			// interval is already "fixed" to be 1 or more)
			interval = 1;
		}
		leeway = _dispatch_time_nano2mach(leeway);
		params->values.flags &= ~(unsigned long)DISPATCH_TIMER_WALL_CLOCK;
	}
	params->ident = DISPATCH_TIMER_IDENT(params->values.flags);
	params->values.target = start;
	params->values.deadline = (start < UINT64_MAX - leeway) ?
			start + leeway : UINT64_MAX;
	params->values.interval = interval;
	params->values.leeway = (interval == INT64_MAX || leeway < interval / 2) ?
			leeway : interval / 2;
	return params;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_source_set_timer(dispatch_source_t ds, dispatch_time_t start,
		uint64_t interval, uint64_t leeway, bool source_sync)
{
	if (slowpath(!ds->ds_is_timer) ||
			slowpath(ds_timer(ds->ds_refs).flags & DISPATCH_TIMER_INTERVAL)) {
		DISPATCH_CLIENT_CRASH("Attempt to set timer on a non-timer source");
	}

	struct dispatch_set_timer_params *params;
	params = _dispatch_source_timer_params(ds, start, interval, leeway);

	_dispatch_source_timer_telemetry(ds, params->ident, &params->values);
	// Suspend the source so that it doesn't fire with pending changes
	// The use of suspend/resume requires the external retain/release
	dispatch_retain(ds);
	if (source_sync) {
		return _dispatch_barrier_trysync_f((dispatch_queue_t)ds, params,
				_dispatch_source_set_timer2);
	} else {
		return _dispatch_source_set_timer2(params);
	}
}

void
dispatch_source_set_timer(dispatch_source_t ds, dispatch_time_t start,
		uint64_t interval, uint64_t leeway)
{
	_dispatch_source_set_timer(ds, start, interval, leeway, true);
}

void
_dispatch_source_set_runloop_timer_4CF(dispatch_source_t ds,
		dispatch_time_t start, uint64_t interval, uint64_t leeway)
{
	// Don't serialize through the source queue for CF timers <rdar://13833190>
	_dispatch_source_set_timer(ds, start, interval, leeway, false);
}

void
_dispatch_source_set_interval(dispatch_source_t ds, uint64_t interval)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	#define NSEC_PER_FRAME (NSEC_PER_SEC/60)
	const bool animation = ds_timer(dr).flags & DISPATCH_INTERVAL_UI_ANIMATION;
	if (fastpath(interval <= (animation ? FOREVER_NSEC/NSEC_PER_FRAME :
			FOREVER_NSEC/NSEC_PER_MSEC))) {
		interval *= animation ? NSEC_PER_FRAME : NSEC_PER_MSEC;
	} else {
		interval = FOREVER_NSEC;
	}
	interval = _dispatch_time_nano2mach(interval);
	uint64_t target = _dispatch_absolute_time() + interval;
	target = (target / interval) * interval;
	const uint64_t leeway = animation ?
			_dispatch_time_nano2mach(NSEC_PER_FRAME) : interval / 2;
	ds_timer(dr).target = target;
	ds_timer(dr).deadline = target + leeway;
	ds_timer(dr).interval = interval;
	ds_timer(dr).leeway = leeway;
	_dispatch_source_timer_telemetry(ds, ds->ds_ident_hack, &ds_timer(dr));
}

#pragma mark -
#pragma mark dispatch_timers

#define DISPATCH_TIMER_STRUCT(refs) \
	uint64_t target, deadline; \
	TAILQ_HEAD(, refs) dt_sources

typedef struct dispatch_timer_s {
	DISPATCH_TIMER_STRUCT(dispatch_timer_source_refs_s);
} *dispatch_timer_t;

#define DISPATCH_TIMER_INITIALIZER(tidx) \
	[tidx] = { \
		.target = UINT64_MAX, \
		.deadline = UINT64_MAX, \
		.dt_sources = TAILQ_HEAD_INITIALIZER( \
				_dispatch_timer[tidx].dt_sources), \
	}
#define DISPATCH_TIMER_INIT(kind, qos) \
		DISPATCH_TIMER_INITIALIZER(DISPATCH_TIMER_INDEX( \
		DISPATCH_TIMER_KIND_##kind, DISPATCH_TIMER_QOS_##qos))

static struct dispatch_timer_s _dispatch_timer[] =  {
	DISPATCH_TIMER_INIT(WALL, NORMAL),
	DISPATCH_TIMER_INIT(WALL, CRITICAL),
	DISPATCH_TIMER_INIT(WALL, BACKGROUND),
	DISPATCH_TIMER_INIT(MACH, NORMAL),
	DISPATCH_TIMER_INIT(MACH, CRITICAL),
	DISPATCH_TIMER_INIT(MACH, BACKGROUND),
};
#define DISPATCH_TIMER_COUNT \
		((sizeof(_dispatch_timer) / sizeof(_dispatch_timer[0])))

#define DISPATCH_KEVENT_TIMER_UDATA(tidx) \
	(uintptr_t)&_dispatch_kevent_timer[tidx]
#ifdef __LP64__
#define DISPATCH_KEVENT_TIMER_UDATA_INITIALIZER(tidx) \
		.udata = DISPATCH_KEVENT_TIMER_UDATA(tidx)
#else // __LP64__
// dynamic initialization in _dispatch_timers_init()
#define DISPATCH_KEVENT_TIMER_UDATA_INITIALIZER(tidx) \
		.udata = 0
#endif // __LP64__
#define DISPATCH_KEVENT_TIMER_INITIALIZER(tidx) \
	[tidx] = { \
		.dk_kevent = { \
			.ident = tidx, \
			.filter = DISPATCH_EVFILT_TIMER, \
			DISPATCH_KEVENT_TIMER_UDATA_INITIALIZER(tidx), \
		}, \
		.dk_sources = TAILQ_HEAD_INITIALIZER( \
				_dispatch_kevent_timer[tidx].dk_sources), \
	}
#define DISPATCH_KEVENT_TIMER_INIT(kind, qos) \
		DISPATCH_KEVENT_TIMER_INITIALIZER(DISPATCH_TIMER_INDEX( \
		DISPATCH_TIMER_KIND_##kind, DISPATCH_TIMER_QOS_##qos))

static struct dispatch_kevent_s _dispatch_kevent_timer[] = {
	DISPATCH_KEVENT_TIMER_INIT(WALL, NORMAL),
	DISPATCH_KEVENT_TIMER_INIT(WALL, CRITICAL),
	DISPATCH_KEVENT_TIMER_INIT(WALL, BACKGROUND),
	DISPATCH_KEVENT_TIMER_INIT(MACH, NORMAL),
	DISPATCH_KEVENT_TIMER_INIT(MACH, CRITICAL),
	DISPATCH_KEVENT_TIMER_INIT(MACH, BACKGROUND),
	DISPATCH_KEVENT_TIMER_INITIALIZER(DISPATCH_TIMER_INDEX_DISARM),
};
#define DISPATCH_KEVENT_TIMER_COUNT \
		((sizeof(_dispatch_kevent_timer) / sizeof(_dispatch_kevent_timer[0])))

#define DISPATCH_KEVENT_TIMEOUT_IDENT_MASK (~0ull << 8)
#define DISPATCH_KEVENT_TIMEOUT_INITIALIZER(qos, note) \
	[qos] = { \
		.ident = DISPATCH_KEVENT_TIMEOUT_IDENT_MASK|(qos), \
		.filter = EVFILT_TIMER, \
		.flags = EV_ONESHOT, \
		.fflags = NOTE_ABSOLUTE|NOTE_NSECONDS|NOTE_LEEWAY|(note), \
	}
#define DISPATCH_KEVENT_TIMEOUT_INIT(qos, note) \
		DISPATCH_KEVENT_TIMEOUT_INITIALIZER(DISPATCH_TIMER_QOS_##qos, note)

static struct kevent64_s _dispatch_kevent_timeout[] = {
	DISPATCH_KEVENT_TIMEOUT_INIT(NORMAL, 0),
	DISPATCH_KEVENT_TIMEOUT_INIT(CRITICAL, NOTE_CRITICAL),
	DISPATCH_KEVENT_TIMEOUT_INIT(BACKGROUND, NOTE_BACKGROUND),
};

#define DISPATCH_KEVENT_COALESCING_WINDOW_INIT(qos, ms) \
		[DISPATCH_TIMER_QOS_##qos] = 2ull * (ms) * NSEC_PER_MSEC

static const uint64_t _dispatch_kevent_coalescing_window[] = {
	DISPATCH_KEVENT_COALESCING_WINDOW_INIT(NORMAL, 75),
	DISPATCH_KEVENT_COALESCING_WINDOW_INIT(CRITICAL, 1),
	DISPATCH_KEVENT_COALESCING_WINDOW_INIT(BACKGROUND, 100),
};

#define _dispatch_timers_insert(tidx, dra, dr, dr_list, dta, dt, dt_list) ({ \
	typeof(dr) dri = NULL; typeof(dt) dti; \
	if (tidx != DISPATCH_TIMER_INDEX_DISARM) { \
		TAILQ_FOREACH(dri, &dra[tidx].dk_sources, dr_list) { \
			if (ds_timer(dr).target < ds_timer(dri).target) { \
				break; \
			} \
		} \
		TAILQ_FOREACH(dti, &dta[tidx].dt_sources, dt_list) { \
			if (ds_timer(dt).deadline < ds_timer(dti).deadline) { \
				break; \
			} \
		} \
		if (dti) { \
			TAILQ_INSERT_BEFORE(dti, dt, dt_list); \
		} else { \
			TAILQ_INSERT_TAIL(&dta[tidx].dt_sources, dt, dt_list); \
		} \
	} \
	if (dri) { \
		TAILQ_INSERT_BEFORE(dri, dr, dr_list); \
	} else { \
		TAILQ_INSERT_TAIL(&dra[tidx].dk_sources, dr, dr_list); \
	} \
	})

#define _dispatch_timers_remove(tidx, dk, dra, dr, dr_list, dta, dt, dt_list) \
	({ \
	if (tidx != DISPATCH_TIMER_INDEX_DISARM) { \
		TAILQ_REMOVE(&dta[tidx].dt_sources, dt, dt_list); \
	} \
	TAILQ_REMOVE(dk ? &(*(dk)).dk_sources : &dra[tidx].dk_sources, dr, \
			dr_list); })

#define _dispatch_timers_check(dra, dta) ({ \
	unsigned int qosm = _dispatch_timers_qos_mask; \
	bool update = false; \
	unsigned int tidx; \
	for (tidx = 0; tidx < DISPATCH_TIMER_COUNT; tidx++) { \
		if (!(qosm & 1 << DISPATCH_TIMER_QOS(tidx))){ \
			continue; \
		} \
		dispatch_timer_source_refs_t dr = (dispatch_timer_source_refs_t) \
				TAILQ_FIRST(&dra[tidx].dk_sources); \
		dispatch_timer_source_refs_t dt = (dispatch_timer_source_refs_t) \
				TAILQ_FIRST(&dta[tidx].dt_sources); \
		uint64_t target = dr ? ds_timer(dr).target : UINT64_MAX; \
		uint64_t deadline = dr ? ds_timer(dt).deadline : UINT64_MAX; \
		if (target != dta[tidx].target) { \
			dta[tidx].target = target; \
			update = true; \
		} \
		if (deadline != dta[tidx].deadline) { \
			dta[tidx].deadline = deadline; \
			update = true; \
		} \
	} \
	update; })

static bool _dispatch_timers_reconfigure, _dispatch_timer_expired;
static unsigned int _dispatch_timers_qos_mask;
static bool _dispatch_timers_force_max_leeway;

static void
_dispatch_timers_init(void)
{
#ifndef __LP64__
	unsigned int tidx;
	for (tidx = 0; tidx < DISPATCH_TIMER_COUNT; tidx++) {
		_dispatch_kevent_timer[tidx].dk_kevent.udata = \
				DISPATCH_KEVENT_TIMER_UDATA(tidx);
	}
#endif // __LP64__
	if (slowpath(getenv("LIBDISPATCH_TIMERS_FORCE_MAX_LEEWAY"))) {
		_dispatch_timers_force_max_leeway = true;
	}
}

static inline void
_dispatch_timers_unregister(dispatch_source_t ds, dispatch_kevent_t dk)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	unsigned int tidx = (unsigned int)dk->dk_kevent.ident;

	if (slowpath(ds_timer_aggregate(ds))) {
		_dispatch_timer_aggregates_unregister(ds, tidx);
	}
	_dispatch_timers_remove(tidx, dk, _dispatch_kevent_timer, dr, dr_list,
			_dispatch_timer, (dispatch_timer_source_refs_t)dr, dt_list);
	if (tidx != DISPATCH_TIMER_INDEX_DISARM) {
		_dispatch_timers_reconfigure = true;
		_dispatch_timers_qos_mask |= 1 << DISPATCH_TIMER_QOS(tidx);
	}
}

// Updates the ordered list of timers based on next fire date for changes to ds.
// Should only be called from the context of _dispatch_mgr_q.
static void
_dispatch_timers_update(dispatch_source_t ds)
{
	dispatch_kevent_t dk = ds->ds_dkev;
	dispatch_source_refs_t dr = ds->ds_refs;
	unsigned int tidx;

	DISPATCH_ASSERT_ON_MANAGER_QUEUE();

	// Do not reschedule timers unregistered with _dispatch_kevent_unregister()
	if (slowpath(!dk)) {
		return;
	}
	// Move timers that are disabled, suspended or have missed intervals to the
	// disarmed list, rearm after resume resp. source invoke will reenable them
	if (!ds_timer(dr).target || DISPATCH_OBJECT_SUSPENDED(ds) ||
			ds->ds_pending_data) {
		tidx = DISPATCH_TIMER_INDEX_DISARM;
		(void)dispatch_atomic_and2o(ds, ds_atomic_flags, ~DSF_ARMED, relaxed);
	} else {
		tidx = _dispatch_source_timer_idx(dr);
	}
	if (slowpath(ds_timer_aggregate(ds))) {
		_dispatch_timer_aggregates_register(ds);
	}
	if (slowpath(!ds->ds_is_installed)) {
		ds->ds_is_installed = true;
		if (tidx != DISPATCH_TIMER_INDEX_DISARM) {
			(void)dispatch_atomic_or2o(ds, ds_atomic_flags, DSF_ARMED, relaxed);
		}
		_dispatch_object_debug(ds, "%s", __func__);
		ds->ds_dkev = NULL;
		free(dk);
	} else {
		_dispatch_timers_unregister(ds, dk);
	}
	if (tidx != DISPATCH_TIMER_INDEX_DISARM) {
		_dispatch_timers_reconfigure = true;
		_dispatch_timers_qos_mask |= 1 << DISPATCH_TIMER_QOS(tidx);
	}
	if (dk != &_dispatch_kevent_timer[tidx]){
		ds->ds_dkev = &_dispatch_kevent_timer[tidx];
	}
	_dispatch_timers_insert(tidx, _dispatch_kevent_timer, dr, dr_list,
			_dispatch_timer, (dispatch_timer_source_refs_t)dr, dt_list);
	if (slowpath(ds_timer_aggregate(ds))) {
		_dispatch_timer_aggregates_update(ds, tidx);
	}
}

static inline void
_dispatch_timers_run2(uint64_t nows[], unsigned int tidx)
{
	dispatch_source_refs_t dr;
	dispatch_source_t ds;
	uint64_t now, missed;

	now = _dispatch_source_timer_now(nows, tidx);
	while ((dr = TAILQ_FIRST(&_dispatch_kevent_timer[tidx].dk_sources))) {
		ds = _dispatch_source_from_refs(dr);
		// We may find timers on the wrong list due to a pending update from
		// dispatch_source_set_timer. Force an update of the list in that case.
		if (tidx != ds->ds_ident_hack) {
			_dispatch_timers_update(ds);
			continue;
		}
		if (!ds_timer(dr).target) {
			// No configured timers on the list
			break;
		}
		if (ds_timer(dr).target > now) {
			// Done running timers for now.
			break;
		}
		// Remove timers that are suspended or have missed intervals from the
		// list, rearm after resume resp. source invoke will reenable them
		if (DISPATCH_OBJECT_SUSPENDED(ds) || ds->ds_pending_data) {
			_dispatch_timers_update(ds);
			continue;
		}
		// Calculate number of missed intervals.
		missed = (now - ds_timer(dr).target) / ds_timer(dr).interval;
		if (++missed > INT_MAX) {
			missed = INT_MAX;
		}
		if (ds_timer(dr).interval < INT64_MAX) {
			ds_timer(dr).target += missed * ds_timer(dr).interval;
			ds_timer(dr).deadline = ds_timer(dr).target + ds_timer(dr).leeway;
		} else {
			ds_timer(dr).target = UINT64_MAX;
			ds_timer(dr).deadline = UINT64_MAX;
		}
		_dispatch_timers_update(ds);
		ds_timer(dr).last_fire = now;

		unsigned long data;
		data = dispatch_atomic_add2o(ds, ds_pending_data,
				(unsigned long)missed, relaxed);
		_dispatch_trace_timer_fire(dr, data, (unsigned long)missed);
		_dispatch_wakeup(ds);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_timers_run(uint64_t nows[])
{
	unsigned int tidx;
	for (tidx = 0; tidx < DISPATCH_TIMER_COUNT; tidx++) {
		if (!TAILQ_EMPTY(&_dispatch_kevent_timer[tidx].dk_sources)) {
			_dispatch_timers_run2(nows, tidx);
		}
	}
}

static inline unsigned int
_dispatch_timers_get_delay(uint64_t nows[], struct dispatch_timer_s timer[],
		uint64_t *delay, uint64_t *leeway, int qos)
{
	unsigned int tidx, ridx = DISPATCH_TIMER_COUNT;
	uint64_t tmp, delta = UINT64_MAX, dldelta = UINT64_MAX;

	for (tidx = 0; tidx < DISPATCH_TIMER_COUNT; tidx++) {
		if (qos >= 0 && qos != DISPATCH_TIMER_QOS(tidx)){
			continue;
		}
		uint64_t target = timer[tidx].target;
		if (target == UINT64_MAX) {
			continue;
		}
		uint64_t deadline = timer[tidx].deadline;
		if (qos >= 0) {
			// Timer pre-coalescing <rdar://problem/13222034>
			uint64_t window = _dispatch_kevent_coalescing_window[qos];
			uint64_t latest = deadline > window ? deadline - window : 0;
			dispatch_source_refs_t dri;
			TAILQ_FOREACH(dri, &_dispatch_kevent_timer[tidx].dk_sources,
					dr_list) {
				tmp = ds_timer(dri).target;
				if (tmp > latest) break;
				target = tmp;
			}
		}
		uint64_t now = _dispatch_source_timer_now(nows, tidx);
		if (target <= now) {
			delta = 0;
			break;
		}
		tmp = target - now;
		if (DISPATCH_TIMER_KIND(tidx) != DISPATCH_TIMER_KIND_WALL) {
			tmp = _dispatch_time_mach2nano(tmp);
		}
		if (tmp < INT64_MAX && tmp < delta) {
			ridx = tidx;
			delta = tmp;
		}
		dispatch_assert(target <= deadline);
		tmp = deadline - now;
		if (DISPATCH_TIMER_KIND(tidx) != DISPATCH_TIMER_KIND_WALL) {
			tmp = _dispatch_time_mach2nano(tmp);
		}
		if (tmp < INT64_MAX && tmp < dldelta) {
			dldelta = tmp;
		}
	}
	*delay = delta;
	*leeway = delta && delta < UINT64_MAX ? dldelta - delta : UINT64_MAX;
	return ridx;
}

static bool
_dispatch_timers_program2(uint64_t nows[], struct kevent64_s *ke,
		unsigned int qos)
{
	unsigned int tidx;
	bool poll;
	uint64_t delay, leeway;

	tidx = _dispatch_timers_get_delay(nows, _dispatch_timer, &delay, &leeway,
			(int)qos);
	poll = (delay == 0);
	if (poll || delay == UINT64_MAX) {
		_dispatch_trace_next_timer_set(NULL, qos);
		if (!ke->data) {
			return poll;
		}
		ke->data = 0;
		ke->flags |= EV_DELETE;
		ke->flags &= ~(EV_ADD|EV_ENABLE);
	} else {
		_dispatch_trace_next_timer_set(
				TAILQ_FIRST(&_dispatch_kevent_timer[tidx].dk_sources), qos);
		_dispatch_trace_next_timer_program(delay, qos);
		delay += _dispatch_source_timer_now(nows, DISPATCH_TIMER_KIND_WALL);
		if (slowpath(_dispatch_timers_force_max_leeway)) {
			ke->data = (int64_t)(delay + leeway);
			ke->ext[1] = 0;
		} else {
			ke->data = (int64_t)delay;
			ke->ext[1] = leeway;
		}
		ke->flags |= EV_ADD|EV_ENABLE;
		ke->flags &= ~EV_DELETE;
	}
	_dispatch_kq_update(ke);
	return poll;
}

DISPATCH_NOINLINE
static bool
_dispatch_timers_program(uint64_t nows[])
{
	bool poll = false;
	unsigned int qos, qosm = _dispatch_timers_qos_mask;
	for (qos = 0; qos < DISPATCH_TIMER_QOS_COUNT; qos++) {
		if (!(qosm & 1 << qos)){
			continue;
		}
		poll |= _dispatch_timers_program2(nows, &_dispatch_kevent_timeout[qos],
				qos);
	}
	return poll;
}

DISPATCH_NOINLINE
static bool
_dispatch_timers_configure(void)
{
	_dispatch_timer_aggregates_check();
	// Find out if there is a new target/deadline on the timer lists
	return _dispatch_timers_check(_dispatch_kevent_timer, _dispatch_timer);
}

static void
_dispatch_timers_calendar_change(void)
{
	// calendar change may have gone past the wallclock deadline
	_dispatch_timer_expired = true;
	_dispatch_timers_qos_mask = ~0u;
}

static void
_dispatch_timers_kevent(struct kevent64_s *ke)
{
	dispatch_assert(ke->data > 0);
	dispatch_assert((ke->ident & DISPATCH_KEVENT_TIMEOUT_IDENT_MASK) ==
			DISPATCH_KEVENT_TIMEOUT_IDENT_MASK);
	unsigned int qos = ke->ident & ~DISPATCH_KEVENT_TIMEOUT_IDENT_MASK;
	dispatch_assert(qos < DISPATCH_TIMER_QOS_COUNT);
	dispatch_assert(_dispatch_kevent_timeout[qos].data);
	_dispatch_kevent_timeout[qos].data = 0; // kevent deleted via EV_ONESHOT
	_dispatch_timer_expired = true;
	_dispatch_timers_qos_mask |= 1 << qos;
	_dispatch_trace_next_timer_wake(qos);
}

static inline bool
_dispatch_mgr_timers(void)
{
	uint64_t nows[DISPATCH_TIMER_KIND_COUNT] = {};
	bool expired = slowpath(_dispatch_timer_expired);
	if (expired) {
		_dispatch_timers_run(nows);
	}
	bool reconfigure = slowpath(_dispatch_timers_reconfigure);
	if (reconfigure || expired) {
		if (reconfigure) {
			reconfigure = _dispatch_timers_configure();
			_dispatch_timers_reconfigure = false;
		}
		if (reconfigure || expired) {
			expired = _dispatch_timer_expired = _dispatch_timers_program(nows);
			expired = expired || _dispatch_mgr_q.dq_items_tail;
		}
		_dispatch_timers_qos_mask = 0;
	}
	return expired;
}

#pragma mark -
#pragma mark dispatch_timer_aggregate

typedef struct {
	TAILQ_HEAD(, dispatch_timer_source_aggregate_refs_s) dk_sources;
} dispatch_timer_aggregate_refs_s;

typedef struct dispatch_timer_aggregate_s {
	DISPATCH_STRUCT_HEADER(queue);
	DISPATCH_QUEUE_HEADER;
	TAILQ_ENTRY(dispatch_timer_aggregate_s) dta_list;
	dispatch_timer_aggregate_refs_s
			dta_kevent_timer[DISPATCH_KEVENT_TIMER_COUNT];
	struct {
		DISPATCH_TIMER_STRUCT(dispatch_timer_source_aggregate_refs_s);
	} dta_timer[DISPATCH_TIMER_COUNT];
	struct dispatch_timer_s dta_timer_data[DISPATCH_TIMER_COUNT];
	unsigned int dta_refcount;
} dispatch_timer_aggregate_s;

typedef TAILQ_HEAD(, dispatch_timer_aggregate_s) dispatch_timer_aggregates_s;
static dispatch_timer_aggregates_s _dispatch_timer_aggregates =
		TAILQ_HEAD_INITIALIZER(_dispatch_timer_aggregates);

dispatch_timer_aggregate_t
dispatch_timer_aggregate_create(void)
{
	unsigned int tidx;
	dispatch_timer_aggregate_t dta = _dispatch_alloc(DISPATCH_VTABLE(queue),
			sizeof(struct dispatch_timer_aggregate_s));
	_dispatch_queue_init((dispatch_queue_t)dta);
	dta->do_targetq = _dispatch_get_root_queue(
			_DISPATCH_QOS_CLASS_USER_INITIATED, true);
	dta->dq_width = DISPATCH_QUEUE_WIDTH_MAX;
	//FIXME: aggregates need custom vtable
	//dta->dq_label = "timer-aggregate";
	for (tidx = 0; tidx < DISPATCH_KEVENT_TIMER_COUNT; tidx++) {
		TAILQ_INIT(&dta->dta_kevent_timer[tidx].dk_sources);
	}
	for (tidx = 0; tidx < DISPATCH_TIMER_COUNT; tidx++) {
		TAILQ_INIT(&dta->dta_timer[tidx].dt_sources);
		dta->dta_timer[tidx].target = UINT64_MAX;
		dta->dta_timer[tidx].deadline = UINT64_MAX;
		dta->dta_timer_data[tidx].target = UINT64_MAX;
		dta->dta_timer_data[tidx].deadline = UINT64_MAX;
	}
	return (dispatch_timer_aggregate_t)_dispatch_introspection_queue_create(
			(dispatch_queue_t)dta);
}

typedef struct dispatch_timer_delay_s {
	dispatch_timer_t timer;
	uint64_t delay, leeway;
} *dispatch_timer_delay_t;

static void
_dispatch_timer_aggregate_get_delay(void *ctxt)
{
	dispatch_timer_delay_t dtd = ctxt;
	struct { uint64_t nows[DISPATCH_TIMER_KIND_COUNT]; } dtn = {};
	_dispatch_timers_get_delay(dtn.nows, dtd->timer, &dtd->delay, &dtd->leeway,
			-1);
}

uint64_t
dispatch_timer_aggregate_get_delay(dispatch_timer_aggregate_t dta,
		uint64_t *leeway_ptr)
{
	struct dispatch_timer_delay_s dtd = {
		.timer = dta->dta_timer_data,
	};
	dispatch_sync_f((dispatch_queue_t)dta, &dtd,
			_dispatch_timer_aggregate_get_delay);
	if (leeway_ptr) {
		*leeway_ptr = dtd.leeway;
	}
	return dtd.delay;
}

static void
_dispatch_timer_aggregate_update(void *ctxt)
{
	dispatch_timer_aggregate_t dta = (void*)_dispatch_queue_get_current();
	dispatch_timer_t dtau = ctxt;
	unsigned int tidx;
	for (tidx = 0; tidx < DISPATCH_TIMER_COUNT; tidx++) {
		dta->dta_timer_data[tidx].target = dtau[tidx].target;
		dta->dta_timer_data[tidx].deadline = dtau[tidx].deadline;
	}
	free(dtau);
}

DISPATCH_NOINLINE
static void
_dispatch_timer_aggregates_configure(void)
{
	dispatch_timer_aggregate_t dta;
	dispatch_timer_t dtau;
	TAILQ_FOREACH(dta, &_dispatch_timer_aggregates, dta_list) {
		if (!_dispatch_timers_check(dta->dta_kevent_timer, dta->dta_timer)) {
			continue;
		}
		dtau = _dispatch_calloc(DISPATCH_TIMER_COUNT, sizeof(*dtau));
		memcpy(dtau, dta->dta_timer, sizeof(dta->dta_timer));
		_dispatch_barrier_async_detached_f((dispatch_queue_t)dta, dtau,
				_dispatch_timer_aggregate_update);
	}
}

static inline void
_dispatch_timer_aggregates_check(void)
{
	if (fastpath(TAILQ_EMPTY(&_dispatch_timer_aggregates))) {
		return;
	}
	_dispatch_timer_aggregates_configure();
}

static void
_dispatch_timer_aggregates_register(dispatch_source_t ds)
{
	dispatch_timer_aggregate_t dta = ds_timer_aggregate(ds);
	if (!dta->dta_refcount++) {
		TAILQ_INSERT_TAIL(&_dispatch_timer_aggregates, dta, dta_list);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_timer_aggregates_update(dispatch_source_t ds, unsigned int tidx)
{
	dispatch_timer_aggregate_t dta = ds_timer_aggregate(ds);
	dispatch_timer_source_aggregate_refs_t dr;
	dr = (dispatch_timer_source_aggregate_refs_t)ds->ds_refs;
	_dispatch_timers_insert(tidx, dta->dta_kevent_timer, dr, dra_list,
			dta->dta_timer, dr, dta_list);
}

DISPATCH_NOINLINE
static void
_dispatch_timer_aggregates_unregister(dispatch_source_t ds, unsigned int tidx)
{
	dispatch_timer_aggregate_t dta = ds_timer_aggregate(ds);
	dispatch_timer_source_aggregate_refs_t dr;
	dr = (dispatch_timer_source_aggregate_refs_t)ds->ds_refs;
	_dispatch_timers_remove(tidx, (dispatch_timer_aggregate_refs_s*)NULL,
			dta->dta_kevent_timer, dr, dra_list, dta->dta_timer, dr, dta_list);
	if (!--dta->dta_refcount) {
		TAILQ_REMOVE(&_dispatch_timer_aggregates, dta, dta_list);
	}
}

#pragma mark -
#pragma mark dispatch_select

static int _dispatch_kq;

static unsigned int _dispatch_select_workaround;
static fd_set _dispatch_rfds;
static fd_set _dispatch_wfds;
static uint64_t*_dispatch_rfd_ptrs;
static uint64_t*_dispatch_wfd_ptrs;

DISPATCH_NOINLINE
static bool
_dispatch_select_register(struct kevent64_s *kev)
{

	// Must execute on manager queue
	DISPATCH_ASSERT_ON_MANAGER_QUEUE();

	// If an EINVAL or ENOENT error occurred while adding/enabling a read or
	// write kevent, assume it was due to a type of filedescriptor not
	// supported by kqueue and fall back to select
	switch (kev->filter) {
	case EVFILT_READ:
		if ((kev->data == EINVAL || kev->data == ENOENT) &&
				dispatch_assume(kev->ident < FD_SETSIZE)) {
			FD_SET((int)kev->ident, &_dispatch_rfds);
			if (slowpath(!_dispatch_rfd_ptrs)) {
				_dispatch_rfd_ptrs = _dispatch_calloc(FD_SETSIZE,
						sizeof(*_dispatch_rfd_ptrs));
			}
			if (!_dispatch_rfd_ptrs[kev->ident]) {
				_dispatch_rfd_ptrs[kev->ident] = kev->udata;
				_dispatch_select_workaround++;
				_dispatch_debug("select workaround used to read fd %d: 0x%lx",
						(int)kev->ident, (long)kev->data);
			}
		}
		return true;
	case EVFILT_WRITE:
		if ((kev->data == EINVAL || kev->data == ENOENT) &&
				dispatch_assume(kev->ident < FD_SETSIZE)) {
			FD_SET((int)kev->ident, &_dispatch_wfds);
			if (slowpath(!_dispatch_wfd_ptrs)) {
				_dispatch_wfd_ptrs = _dispatch_calloc(FD_SETSIZE,
						sizeof(*_dispatch_wfd_ptrs));
			}
			if (!_dispatch_wfd_ptrs[kev->ident]) {
				_dispatch_wfd_ptrs[kev->ident] = kev->udata;
				_dispatch_select_workaround++;
				_dispatch_debug("select workaround used to write fd %d: 0x%lx",
						(int)kev->ident, (long)kev->data);
			}
		}
		return true;
	}
	return false;
}

DISPATCH_NOINLINE
static bool
_dispatch_select_unregister(const struct kevent64_s *kev)
{
	// Must execute on manager queue
	DISPATCH_ASSERT_ON_MANAGER_QUEUE();

	switch (kev->filter) {
	case EVFILT_READ:
		if (_dispatch_rfd_ptrs && kev->ident < FD_SETSIZE &&
				_dispatch_rfd_ptrs[kev->ident]) {
			FD_CLR((int)kev->ident, &_dispatch_rfds);
			_dispatch_rfd_ptrs[kev->ident] = 0;
			_dispatch_select_workaround--;
			return true;
		}
		break;
	case EVFILT_WRITE:
		if (_dispatch_wfd_ptrs && kev->ident < FD_SETSIZE &&
				_dispatch_wfd_ptrs[kev->ident]) {
			FD_CLR((int)kev->ident, &_dispatch_wfds);
			_dispatch_wfd_ptrs[kev->ident] = 0;
			_dispatch_select_workaround--;
			return true;
		}
		break;
	}
	return false;
}

DISPATCH_NOINLINE
static bool
_dispatch_mgr_select(bool poll)
{
	static const struct timeval timeout_immediately = { 0, 0 };
	fd_set tmp_rfds, tmp_wfds;
	struct kevent64_s kev;
	int err, i, r;
	bool kevent_avail = false;

	FD_COPY(&_dispatch_rfds, &tmp_rfds);
	FD_COPY(&_dispatch_wfds, &tmp_wfds);

	r = select(FD_SETSIZE, &tmp_rfds, &tmp_wfds, NULL,
			poll ? (struct timeval*)&timeout_immediately : NULL);
	if (slowpath(r == -1)) {
		err = errno;
		if (err != EBADF) {
			if (err != EINTR) {
				(void)dispatch_assume_zero(err);
			}
			return false;
		}
		for (i = 0; i < FD_SETSIZE; i++) {
			if (i == _dispatch_kq) {
				continue;
			}
			if (!FD_ISSET(i, &_dispatch_rfds) && !FD_ISSET(i, &_dispatch_wfds)){
				continue;
			}
			r = dup(i);
			if (dispatch_assume(r != -1)) {
				close(r);
			} else {
				if (_dispatch_rfd_ptrs && _dispatch_rfd_ptrs[i]) {
					FD_CLR(i, &_dispatch_rfds);
					_dispatch_rfd_ptrs[i] = 0;
					_dispatch_select_workaround--;
				}
				if (_dispatch_wfd_ptrs && _dispatch_wfd_ptrs[i]) {
					FD_CLR(i, &_dispatch_wfds);
					_dispatch_wfd_ptrs[i] = 0;
					_dispatch_select_workaround--;
				}
			}
		}
		return false;
	}
	if (r > 0) {
		for (i = 0; i < FD_SETSIZE; i++) {
			if (FD_ISSET(i, &tmp_rfds)) {
				if (i == _dispatch_kq) {
					kevent_avail = true;
					continue;
				}
				FD_CLR(i, &_dispatch_rfds); // emulate EV_DISPATCH
				EV_SET64(&kev, i, EVFILT_READ,
						EV_ADD|EV_ENABLE|EV_DISPATCH, 0, 1,
						_dispatch_rfd_ptrs[i], 0, 0);
				_dispatch_kevent_drain(&kev);
			}
			if (FD_ISSET(i, &tmp_wfds)) {
				FD_CLR(i, &_dispatch_wfds); // emulate EV_DISPATCH
				EV_SET64(&kev, i, EVFILT_WRITE,
						EV_ADD|EV_ENABLE|EV_DISPATCH, 0, 1,
						_dispatch_wfd_ptrs[i], 0, 0);
				_dispatch_kevent_drain(&kev);
			}
		}
	}
	return kevent_avail;
}

#pragma mark -
#pragma mark dispatch_kqueue

static void
_dispatch_kq_init(void *context DISPATCH_UNUSED)
{
	static const struct kevent64_s kev = {
		.ident = 1,
		.filter = EVFILT_USER,
		.flags = EV_ADD|EV_CLEAR,
	};

	_dispatch_safe_fork = false;
#if DISPATCH_USE_GUARDED_FD
	guardid_t guard = (uintptr_t)&kev;
	_dispatch_kq = guarded_kqueue_np(&guard, GUARD_CLOSE | GUARD_DUP);
#else
	_dispatch_kq = kqueue();
#endif
	if (_dispatch_kq == -1) {
		int err = errno;
		switch (err) {
		case EMFILE:
			DISPATCH_CLIENT_CRASH("kqueue() failure: "
					"process is out of file descriptors");
			break;
		case ENFILE:
			DISPATCH_CLIENT_CRASH("kqueue() failure: "
					"system is out of file descriptors");
			break;
		case ENOMEM:
			DISPATCH_CLIENT_CRASH("kqueue() failure: "
					"kernel is out of memory");
			break;
		default:
			(void)dispatch_assume_zero(err);
			DISPATCH_CRASH("kqueue() failure");
			break;
		}
	} else if (dispatch_assume(_dispatch_kq < FD_SETSIZE)) {
		// in case we fall back to select()
		FD_SET(_dispatch_kq, &_dispatch_rfds);
	}

	(void)dispatch_assume_zero(kevent64(_dispatch_kq, &kev, 1, NULL, 0, 0,
			NULL));
	_dispatch_queue_push(_dispatch_mgr_q.do_targetq, &_dispatch_mgr_q, 0);
}

static int
_dispatch_get_kq(void)
{
	static dispatch_once_t pred;

	dispatch_once_f(&pred, NULL, _dispatch_kq_init);

	return _dispatch_kq;
}

DISPATCH_NOINLINE
static long
_dispatch_kq_update(const struct kevent64_s *kev)
{
	int r;
	struct kevent64_s kev_copy;

	if (slowpath(_dispatch_select_workaround) && (kev->flags & EV_DELETE)) {
		if (_dispatch_select_unregister(kev)) {
			return 0;
		}
	}
	kev_copy = *kev;
	// This ensures we don't get a pending kevent back while registering
	// a new kevent
	kev_copy.flags |= EV_RECEIPT;
retry:
	r = dispatch_assume(kevent64(_dispatch_get_kq(), &kev_copy, 1,
			&kev_copy, 1, 0, NULL));
	if (slowpath(r == -1)) {
		int err = errno;
		switch (err) {
		case EINTR:
			goto retry;
		case EBADF:
			DISPATCH_CLIENT_CRASH("Do not close random Unix descriptors");
			break;
		default:
			(void)dispatch_assume_zero(err);
			break;
		}
		return err;
	}
	switch (kev_copy.data) {
	case 0:
		return 0;
	case EBADF:
	case EPERM:
	case EINVAL:
	case ENOENT:
		if ((kev->flags & (EV_ADD|EV_ENABLE)) && !(kev->flags & EV_DELETE)) {
			if (_dispatch_select_register(&kev_copy)) {
				return 0;
			}
		}
		// fall through
	default:
		kev_copy.flags |= kev->flags;
		_dispatch_kevent_drain(&kev_copy);
		break;
	}
	return (long)kev_copy.data;
}

#pragma mark -
#pragma mark dispatch_mgr

static struct kevent64_s *_dispatch_kevent_enable;

static void inline
_dispatch_mgr_kevent_reenable(struct kevent64_s *ke)
{
	dispatch_assert(!_dispatch_kevent_enable || _dispatch_kevent_enable == ke);
	_dispatch_kevent_enable = ke;
}

unsigned long
_dispatch_mgr_wakeup(dispatch_queue_t dq DISPATCH_UNUSED)
{
	if (_dispatch_queue_get_current() == &_dispatch_mgr_q) {
		return false;
	}

	static const struct kevent64_s kev = {
		.ident = 1,
		.filter = EVFILT_USER,
		.fflags = NOTE_TRIGGER,
	};

#if DISPATCH_DEBUG && DISPATCH_MGR_QUEUE_DEBUG
	_dispatch_debug("waking up the dispatch manager queue: %p", dq);
#endif

	_dispatch_kq_update(&kev);

	return false;
}

DISPATCH_NOINLINE
static void
_dispatch_mgr_init(void)
{
	(void)dispatch_atomic_inc2o(&_dispatch_mgr_q, dq_running, relaxed);
	_dispatch_thread_setspecific(dispatch_queue_key, &_dispatch_mgr_q);
	_dispatch_queue_set_bound_thread(&_dispatch_mgr_q);
	_dispatch_mgr_priority_init();
	_dispatch_kevent_init();
	_dispatch_timers_init();
	_dispatch_mach_recv_msg_buf_init();
	_dispatch_memorystatus_init();
}

DISPATCH_NOINLINE DISPATCH_NORETURN
static void
_dispatch_mgr_invoke(void)
{
	static const struct timespec timeout_immediately = { 0, 0 };
	struct kevent64_s kev;
	bool poll;
	int r;

	for (;;) {
		_dispatch_mgr_queue_drain();
		poll = _dispatch_mgr_timers();
		if (slowpath(_dispatch_select_workaround)) {
			poll = _dispatch_mgr_select(poll);
			if (!poll) continue;
		}
		poll = poll || _dispatch_queue_class_probe(&_dispatch_mgr_q);
		r = kevent64(_dispatch_kq, _dispatch_kevent_enable,
				_dispatch_kevent_enable ? 1 : 0, &kev, 1, 0,
				poll ? &timeout_immediately : NULL);
		_dispatch_kevent_enable = NULL;
		if (slowpath(r == -1)) {
			int err = errno;
			switch (err) {
			case EINTR:
				break;
			case EBADF:
				DISPATCH_CLIENT_CRASH("Do not close random Unix descriptors");
				break;
			default:
				(void)dispatch_assume_zero(err);
				break;
			}
		} else if (r) {
			_dispatch_kevent_drain(&kev);
		}
	}
}

DISPATCH_NORETURN
void
_dispatch_mgr_thread(dispatch_queue_t dq DISPATCH_UNUSED)
{
	_dispatch_mgr_init();
	// never returns, so burn bridges behind us & clear stack 2k ahead
	_dispatch_clear_stack(2048);
	_dispatch_mgr_invoke();
}

#pragma mark -
#pragma mark dispatch_memorystatus

#if DISPATCH_USE_MEMORYSTATUS_SOURCE
#define DISPATCH_MEMORYSTATUS_SOURCE_TYPE DISPATCH_SOURCE_TYPE_MEMORYSTATUS
#define DISPATCH_MEMORYSTATUS_SOURCE_MASK ( \
		DISPATCH_MEMORYSTATUS_PRESSURE_NORMAL | \
		DISPATCH_MEMORYSTATUS_PRESSURE_WARN)
#elif DISPATCH_USE_VM_PRESSURE_SOURCE
#define DISPATCH_MEMORYSTATUS_SOURCE_TYPE DISPATCH_SOURCE_TYPE_VM
#define DISPATCH_MEMORYSTATUS_SOURCE_MASK DISPATCH_VM_PRESSURE
#endif

#if DISPATCH_USE_MEMORYSTATUS_SOURCE || DISPATCH_USE_VM_PRESSURE_SOURCE
static dispatch_source_t _dispatch_memorystatus_source;

static void
_dispatch_memorystatus_handler(void *context DISPATCH_UNUSED)
{
#if DISPATCH_USE_MEMORYSTATUS_SOURCE
	unsigned long memorystatus;
	memorystatus = dispatch_source_get_data(_dispatch_memorystatus_source);
	if (memorystatus & DISPATCH_MEMORYSTATUS_PRESSURE_NORMAL) {
		_dispatch_continuation_cache_limit = DISPATCH_CONTINUATION_CACHE_LIMIT;
		_voucher_activity_heap_pressure_normal();
		return;
	}
	_dispatch_continuation_cache_limit =
			DISPATCH_CONTINUATION_CACHE_LIMIT_MEMORYSTATUS_PRESSURE_WARN;
	_voucher_activity_heap_pressure_warn();
#endif
	malloc_zone_pressure_relief(0,0);
}

static void
_dispatch_memorystatus_init(void)
{
	_dispatch_memorystatus_source = dispatch_source_create(
			DISPATCH_MEMORYSTATUS_SOURCE_TYPE, 0,
			DISPATCH_MEMORYSTATUS_SOURCE_MASK,
			_dispatch_get_root_queue(_DISPATCH_QOS_CLASS_DEFAULT, true));
	dispatch_source_set_event_handler_f(_dispatch_memorystatus_source,
			_dispatch_memorystatus_handler);
	dispatch_resume(_dispatch_memorystatus_source);
}
#else
static inline void _dispatch_memorystatus_init(void) {}
#endif // DISPATCH_USE_MEMORYSTATUS_SOURCE || DISPATCH_USE_VM_PRESSURE_SOURCE

#pragma mark -
#pragma mark dispatch_mach

#if HAVE_MACH

#if DISPATCH_DEBUG && DISPATCH_MACHPORT_DEBUG
#define _dispatch_debug_machport(name) \
		dispatch_debug_machport((name), __func__)
#else
#define _dispatch_debug_machport(name) ((void)(name))
#endif

// Flags for all notifications that are registered/unregistered when a
// send-possible notification is requested/delivered
#define _DISPATCH_MACH_SP_FLAGS (DISPATCH_MACH_SEND_POSSIBLE| \
		DISPATCH_MACH_SEND_DEAD|DISPATCH_MACH_SEND_DELETED)
#define _DISPATCH_MACH_RECV_FLAGS (DISPATCH_MACH_RECV_MESSAGE| \
		DISPATCH_MACH_RECV_MESSAGE_DIRECT| \
		DISPATCH_MACH_RECV_MESSAGE_DIRECT_ONCE)
#define _DISPATCH_MACH_RECV_DIRECT_FLAGS ( \
		DISPATCH_MACH_RECV_MESSAGE_DIRECT| \
		DISPATCH_MACH_RECV_MESSAGE_DIRECT_ONCE)

#define _DISPATCH_IS_POWER_OF_TWO(v) (!(v & (v - 1)) && v)
#define _DISPATCH_HASH(x, y) (_DISPATCH_IS_POWER_OF_TWO(y) ? \
		(MACH_PORT_INDEX(x) & ((y) - 1)) : (MACH_PORT_INDEX(x) % (y)))

#define _DISPATCH_MACHPORT_HASH_SIZE 32
#define _DISPATCH_MACHPORT_HASH(x) \
		_DISPATCH_HASH((x), _DISPATCH_MACHPORT_HASH_SIZE)

#ifndef MACH_RCV_LARGE_IDENTITY
#define MACH_RCV_LARGE_IDENTITY 0x00000008
#endif
#ifndef MACH_RCV_VOUCHER
#define MACH_RCV_VOUCHER 0x00000800
#endif
#define DISPATCH_MACH_RCV_TRAILER MACH_RCV_TRAILER_CTX
#define DISPATCH_MACH_RCV_OPTIONS ( \
		MACH_RCV_MSG | MACH_RCV_LARGE | MACH_RCV_LARGE_IDENTITY | \
		MACH_RCV_TRAILER_ELEMENTS(DISPATCH_MACH_RCV_TRAILER) | \
		MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)) | \
		MACH_RCV_VOUCHER

#define DISPATCH_MACH_KEVENT_ARMED(dk) ((dk)->dk_kevent.ext[0])

static void _dispatch_kevent_machport_drain(struct kevent64_s *ke);
static void _dispatch_kevent_mach_msg_drain(struct kevent64_s *ke);
static void _dispatch_kevent_mach_msg_recv(mach_msg_header_t *hdr);
static void _dispatch_kevent_mach_msg_destroy(mach_msg_header_t *hdr);
static void _dispatch_source_merge_mach_msg(dispatch_source_t ds,
		dispatch_source_refs_t dr, dispatch_kevent_t dk,
		mach_msg_header_t *hdr, mach_msg_size_t siz);
static kern_return_t _dispatch_mach_notify_update(dispatch_kevent_t dk,
		uint32_t new_flags, uint32_t del_flags, uint32_t mask,
		mach_msg_id_t notify_msgid, mach_port_mscount_t notify_sync);
static void _dispatch_mach_notify_source_invoke(mach_msg_header_t *hdr);
static void _dispatch_mach_reply_kevent_unregister(dispatch_mach_t dm,
		dispatch_mach_reply_refs_t dmr, bool disconnected);
static void _dispatch_mach_kevent_unregister(dispatch_mach_t dm);
static inline void _dispatch_mach_msg_set_options(dispatch_object_t dou,
		mach_msg_option_t options);
static void _dispatch_mach_msg_recv(dispatch_mach_t dm,
		dispatch_mach_reply_refs_t dmr, mach_msg_header_t *hdr,
		mach_msg_size_t siz);
static void _dispatch_mach_merge_kevent(dispatch_mach_t dm,
		const struct kevent64_s *ke);
static inline mach_msg_option_t _dispatch_mach_checkin_options(void);

static const size_t _dispatch_mach_recv_msg_size =
		DISPATCH_MACH_RECEIVE_MAX_INLINE_MESSAGE_SIZE;
static const size_t dispatch_mach_trailer_size =
		sizeof(dispatch_mach_trailer_t);
static mach_msg_size_t _dispatch_mach_recv_msg_buf_size;
static mach_port_t _dispatch_mach_portset, _dispatch_mach_recv_portset;
static mach_port_t _dispatch_mach_notify_port;
static struct kevent64_s _dispatch_mach_recv_kevent = {
	.filter = EVFILT_MACHPORT,
	.flags = EV_ADD|EV_ENABLE|EV_DISPATCH,
	.fflags = DISPATCH_MACH_RCV_OPTIONS,
};
static dispatch_source_t _dispatch_mach_notify_source;
static const
struct dispatch_source_type_s _dispatch_source_type_mach_recv_direct = {
	.ke = {
		.filter = EVFILT_MACHPORT,
		.flags = EV_CLEAR,
		.fflags = DISPATCH_MACH_RECV_MESSAGE_DIRECT,
	},
};

static void
_dispatch_mach_recv_msg_buf_init(void)
{
	mach_vm_size_t vm_size = mach_vm_round_page(
			_dispatch_mach_recv_msg_size + dispatch_mach_trailer_size);
	_dispatch_mach_recv_msg_buf_size = (mach_msg_size_t)vm_size;
	mach_vm_address_t vm_addr = vm_page_size;
	kern_return_t kr;

	while (slowpath(kr = mach_vm_allocate(mach_task_self(), &vm_addr, vm_size,
			VM_FLAGS_ANYWHERE))) {
		if (kr != KERN_NO_SPACE) {
			(void)dispatch_assume_zero(kr);
			DISPATCH_CLIENT_CRASH("Could not allocate mach msg receive buffer");
		}
		_dispatch_temporary_resource_shortage();
		vm_addr = vm_page_size;
	}
	_dispatch_mach_recv_kevent.ext[0] = (uintptr_t)vm_addr;
	_dispatch_mach_recv_kevent.ext[1] = vm_size;
}

static inline void*
_dispatch_get_mach_recv_msg_buf(void)
{
	return (void*)_dispatch_mach_recv_kevent.ext[0];
}

static void
_dispatch_mach_recv_portset_init(void *context DISPATCH_UNUSED)
{
	kern_return_t kr;

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
			&_dispatch_mach_recv_portset);
	DISPATCH_VERIFY_MIG(kr);
	if (dispatch_assume_zero(kr)) {
		DISPATCH_CLIENT_CRASH(
				"mach_port_allocate() failed: cannot create port set");
	}
	dispatch_assert(_dispatch_get_mach_recv_msg_buf());
	dispatch_assert(dispatch_mach_trailer_size ==
			REQUESTED_TRAILER_SIZE_NATIVE(MACH_RCV_TRAILER_ELEMENTS(
			DISPATCH_MACH_RCV_TRAILER)));
	_dispatch_mach_recv_kevent.ident = _dispatch_mach_recv_portset;
	_dispatch_kq_update(&_dispatch_mach_recv_kevent);

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
			&_dispatch_mach_notify_port);
	DISPATCH_VERIFY_MIG(kr);
	if (dispatch_assume_zero(kr)) {
		DISPATCH_CLIENT_CRASH(
				"mach_port_allocate() failed: cannot create receive right");
	}
	_dispatch_mach_notify_source = dispatch_source_create(
			&_dispatch_source_type_mach_recv_direct,
			_dispatch_mach_notify_port, 0, &_dispatch_mgr_q);
	static const struct dispatch_continuation_s dc = {
		.dc_func = (void*)_dispatch_mach_notify_source_invoke,
	};
	_dispatch_mach_notify_source->ds_refs->ds_handler[DS_EVENT_HANDLER] =
			(dispatch_continuation_t)&dc;
	dispatch_assert(_dispatch_mach_notify_source);
	dispatch_resume(_dispatch_mach_notify_source);
}

static mach_port_t
_dispatch_get_mach_recv_portset(void)
{
	static dispatch_once_t pred;
	dispatch_once_f(&pred, NULL, _dispatch_mach_recv_portset_init);
	return _dispatch_mach_recv_portset;
}

static void
_dispatch_mach_portset_init(void *context DISPATCH_UNUSED)
{
	struct kevent64_s kev = {
		.filter = EVFILT_MACHPORT,
		.flags = EV_ADD,
	};
	kern_return_t kr;

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
			&_dispatch_mach_portset);
	DISPATCH_VERIFY_MIG(kr);
	if (dispatch_assume_zero(kr)) {
		DISPATCH_CLIENT_CRASH(
				"mach_port_allocate() failed: cannot create port set");
	}
	kev.ident = _dispatch_mach_portset;
	_dispatch_kq_update(&kev);
}

static mach_port_t
_dispatch_get_mach_portset(void)
{
	static dispatch_once_t pred;
	dispatch_once_f(&pred, NULL, _dispatch_mach_portset_init);
	return _dispatch_mach_portset;
}

static kern_return_t
_dispatch_mach_portset_update(dispatch_kevent_t dk, mach_port_t mps)
{
	mach_port_t mp = (mach_port_t)dk->dk_kevent.ident;
	kern_return_t kr;

	_dispatch_debug_machport(mp);
	kr = mach_port_move_member(mach_task_self(), mp, mps);
	if (slowpath(kr)) {
		DISPATCH_VERIFY_MIG(kr);
		switch (kr) {
		case KERN_INVALID_RIGHT:
			if (mps) {
				_dispatch_bug_mach_client("_dispatch_kevent_machport_enable: "
						"mach_port_move_member() failed ", kr);
				break;
			}
			//fall through
		case KERN_INVALID_NAME:
#if DISPATCH_DEBUG
			_dispatch_log("Corruption: Mach receive right 0x%x destroyed "
					"prematurely", mp);
#endif
			break;
		default:
			(void)dispatch_assume_zero(kr);
			break;
		}
	}
	return mps ? kr : 0;
}

static void
_dispatch_kevent_mach_recv_reenable(struct kevent64_s *ke DISPATCH_UNUSED)
{
#if (TARGET_IPHONE_SIMULATOR && \
		IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED < 1090) || \
		(!TARGET_OS_IPHONE && __MAC_OS_X_VERSION_MIN_REQUIRED < 1090)
	// delete and re-add kevent to workaround <rdar://problem/13924256>
	if (ke->ext[1] != _dispatch_mach_recv_kevent.ext[1]) {
		struct kevent64_s kev = _dispatch_mach_recv_kevent;
		kev.flags = EV_DELETE;
		_dispatch_kq_update(&kev);
	}
#endif
	_dispatch_mgr_kevent_reenable(&_dispatch_mach_recv_kevent);
}

static kern_return_t
_dispatch_kevent_machport_resume(dispatch_kevent_t dk, uint32_t new_flags,
		uint32_t del_flags)
{
	kern_return_t kr = 0;
	dispatch_assert_zero(new_flags & del_flags);
	if ((new_flags & _DISPATCH_MACH_RECV_FLAGS) ||
			(del_flags & _DISPATCH_MACH_RECV_FLAGS)) {
		mach_port_t mps;
		if (new_flags & _DISPATCH_MACH_RECV_DIRECT_FLAGS) {
			mps = _dispatch_get_mach_recv_portset();
		} else if ((new_flags & DISPATCH_MACH_RECV_MESSAGE) ||
				((del_flags & _DISPATCH_MACH_RECV_DIRECT_FLAGS) &&
				(dk->dk_kevent.fflags & DISPATCH_MACH_RECV_MESSAGE))) {
			mps = _dispatch_get_mach_portset();
		} else {
			mps = MACH_PORT_NULL;
		}
		kr = _dispatch_mach_portset_update(dk, mps);
	}
	return kr;
}

static kern_return_t
_dispatch_kevent_mach_notify_resume(dispatch_kevent_t dk, uint32_t new_flags,
		uint32_t del_flags)
{
	kern_return_t kr = 0;
	dispatch_assert_zero(new_flags & del_flags);
	if ((new_flags & _DISPATCH_MACH_SP_FLAGS) ||
			(del_flags & _DISPATCH_MACH_SP_FLAGS)) {
		// Requesting a (delayed) non-sync send-possible notification
		// registers for both immediate dead-name notification and delayed-arm
		// send-possible notification for the port.
		// The send-possible notification is armed when a mach_msg() with the
		// the MACH_SEND_NOTIFY to the port times out.
		// If send-possible is unavailable, fall back to immediate dead-name
		// registration rdar://problem/2527840&9008724
		kr = _dispatch_mach_notify_update(dk, new_flags, del_flags,
				_DISPATCH_MACH_SP_FLAGS, MACH_NOTIFY_SEND_POSSIBLE,
				MACH_NOTIFY_SEND_POSSIBLE == MACH_NOTIFY_DEAD_NAME ? 1 : 0);
	}
	return kr;
}

static inline void
_dispatch_kevent_mach_portset(struct kevent64_s *ke)
{
	if (ke->ident == _dispatch_mach_recv_portset) {
		return _dispatch_kevent_mach_msg_drain(ke);
	} else if (ke->ident == _dispatch_mach_portset) {
		return _dispatch_kevent_machport_drain(ke);
	} else {
		return _dispatch_kevent_error(ke);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_machport_drain(struct kevent64_s *ke)
{
	mach_port_t name = (mach_port_name_t)ke->data;
	dispatch_kevent_t dk;
	struct kevent64_s kev;

	_dispatch_debug_machport(name);
	dk = _dispatch_kevent_find(name, EVFILT_MACHPORT);
	if (!dispatch_assume(dk)) {
		return;
	}
	_dispatch_mach_portset_update(dk, MACH_PORT_NULL); // emulate EV_DISPATCH

	EV_SET64(&kev, name, EVFILT_MACHPORT, EV_ADD|EV_ENABLE|EV_DISPATCH,
			DISPATCH_MACH_RECV_MESSAGE, 0, (uintptr_t)dk, 0, 0);
	_dispatch_kevent_debug(&kev, __func__);
	_dispatch_kevent_merge(&kev);
}

DISPATCH_NOINLINE
static void
_dispatch_kevent_mach_msg_drain(struct kevent64_s *ke)
{
	mach_msg_header_t *hdr = (mach_msg_header_t*)ke->ext[0];
	mach_msg_size_t siz, msgsiz;
	mach_msg_return_t kr = (mach_msg_return_t)ke->fflags;

	_dispatch_kevent_mach_recv_reenable(ke);
	if (!dispatch_assume(hdr)) {
		DISPATCH_CRASH("EVFILT_MACHPORT with no message");
	}
	if (fastpath(!kr)) {
		return _dispatch_kevent_mach_msg_recv(hdr);
	} else if (kr != MACH_RCV_TOO_LARGE) {
		goto out;
	}
	if (!dispatch_assume(ke->ext[1] <= UINT_MAX -
			dispatch_mach_trailer_size)) {
		DISPATCH_CRASH("EVFILT_MACHPORT with overlarge message");
	}
	siz = (mach_msg_size_t)ke->ext[1] + dispatch_mach_trailer_size;
	hdr = malloc(siz);
	if (ke->data) {
		if (!dispatch_assume(hdr)) {
			// Kernel will discard message too large to fit
			hdr = _dispatch_get_mach_recv_msg_buf();
			siz = _dispatch_mach_recv_msg_buf_size;
		}
		mach_port_t name = (mach_port_name_t)ke->data;
		const mach_msg_option_t options = ((DISPATCH_MACH_RCV_OPTIONS |
				MACH_RCV_TIMEOUT) & ~MACH_RCV_LARGE);
		kr = mach_msg(hdr, options, 0, siz, name, MACH_MSG_TIMEOUT_NONE,
				MACH_PORT_NULL);
		if (fastpath(!kr)) {
			return _dispatch_kevent_mach_msg_recv(hdr);
		} else if (kr == MACH_RCV_TOO_LARGE) {
			_dispatch_log("BUG in libdispatch client: "
					"_dispatch_kevent_mach_msg_drain: dropped message too "
					"large to fit in memory: id = 0x%x, size = %zd",
					hdr->msgh_id, ke->ext[1]);
			kr = MACH_MSG_SUCCESS;
		}
	} else {
		// We don't know which port in the portset contains the large message,
		// so need to receive all messages pending on the portset to ensure the
		// large message is drained. <rdar://problem/13950432>
		bool received = false;
		for (;;) {
			if (!dispatch_assume(hdr)) {
				DISPATCH_CLIENT_CRASH("Message too large to fit in memory");
			}
			const mach_msg_option_t options = (DISPATCH_MACH_RCV_OPTIONS |
					MACH_RCV_TIMEOUT);
			kr = mach_msg(hdr, options, 0, siz, _dispatch_mach_recv_portset,
					MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
			if ((!kr || kr == MACH_RCV_TOO_LARGE) && !dispatch_assume(
					hdr->msgh_size <= UINT_MAX - dispatch_mach_trailer_size)) {
				DISPATCH_CRASH("Overlarge message");
			}
			if (fastpath(!kr)) {
				msgsiz = hdr->msgh_size + dispatch_mach_trailer_size;
				if (msgsiz < siz) {
					void *shrink = realloc(hdr, msgsiz);
					if (shrink) hdr = shrink;
				}
				_dispatch_kevent_mach_msg_recv(hdr);
				hdr = NULL;
				received = true;
			} else if (kr == MACH_RCV_TOO_LARGE) {
				siz = hdr->msgh_size + dispatch_mach_trailer_size;
			} else {
				if (kr == MACH_RCV_TIMED_OUT && received) {
					kr = MACH_MSG_SUCCESS;
				}
				break;
			}
			hdr = reallocf(hdr, siz);
		}
	}
	if (hdr != _dispatch_get_mach_recv_msg_buf()) {
		free(hdr);
	}
out:
	if (slowpath(kr)) {
		_dispatch_bug_mach_client("_dispatch_kevent_mach_msg_drain: "
				"message reception failed", kr);
	}
}

static void
_dispatch_kevent_mach_msg_recv(mach_msg_header_t *hdr)
{
	dispatch_source_refs_t dri;
	dispatch_kevent_t dk;
	mach_port_t name = hdr->msgh_local_port;
	mach_msg_size_t siz = hdr->msgh_size + dispatch_mach_trailer_size;

	if (!dispatch_assume(hdr->msgh_size <= UINT_MAX -
			dispatch_mach_trailer_size)) {
		_dispatch_bug_client("_dispatch_kevent_mach_msg_recv: "
				"received overlarge message");
		return _dispatch_kevent_mach_msg_destroy(hdr);
	}
	if (!dispatch_assume(name)) {
		_dispatch_bug_client("_dispatch_kevent_mach_msg_recv: "
				"received message with MACH_PORT_NULL port");
		return _dispatch_kevent_mach_msg_destroy(hdr);
	}
	_dispatch_debug_machport(name);
	dk = _dispatch_kevent_find(name, EVFILT_MACHPORT);
	if (!dispatch_assume(dk)) {
		_dispatch_bug_client("_dispatch_kevent_mach_msg_recv: "
				"received message with unknown kevent");
		return _dispatch_kevent_mach_msg_destroy(hdr);
	}
	_dispatch_kevent_debug(&dk->dk_kevent, __func__);
	TAILQ_FOREACH(dri, &dk->dk_sources, dr_list) {
		dispatch_source_t dsi = _dispatch_source_from_refs(dri);
		if (dsi->ds_pending_data_mask & _DISPATCH_MACH_RECV_DIRECT_FLAGS) {
			return _dispatch_source_merge_mach_msg(dsi, dri, dk, hdr, siz);
		}
	}
	_dispatch_bug_client("_dispatch_kevent_mach_msg_recv: "
			"received message with no listeners");
	return _dispatch_kevent_mach_msg_destroy(hdr);
}

static void
_dispatch_kevent_mach_msg_destroy(mach_msg_header_t *hdr)
{
	if (hdr) {
		mach_msg_destroy(hdr);
		if (hdr != _dispatch_get_mach_recv_msg_buf()) {
			free(hdr);
		}
	}
}

static void
_dispatch_source_merge_mach_msg(dispatch_source_t ds, dispatch_source_refs_t dr,
		dispatch_kevent_t dk, mach_msg_header_t *hdr, mach_msg_size_t siz)
{
	if (ds == _dispatch_mach_notify_source) {
		_dispatch_mach_notify_source_invoke(hdr);
		return _dispatch_kevent_mach_msg_destroy(hdr);
	}
	dispatch_mach_reply_refs_t dmr = NULL;
	if (dk->dk_kevent.fflags & DISPATCH_MACH_RECV_MESSAGE_DIRECT_ONCE) {
		dmr = (dispatch_mach_reply_refs_t)dr;
	}
	return _dispatch_mach_msg_recv((dispatch_mach_t)ds, dmr, hdr, siz);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_notify_merge(mach_port_t name, uint32_t flag, bool final)
{
	dispatch_source_refs_t dri, dr_next;
	dispatch_kevent_t dk;
	struct kevent64_s kev;
	bool unreg;

	dk = _dispatch_kevent_find(name, DISPATCH_EVFILT_MACH_NOTIFICATION);
	if (!dk) {
		return;
	}

	// Update notification registration state.
	dk->dk_kevent.data &= ~_DISPATCH_MACH_SP_FLAGS;
	EV_SET64(&kev, name, DISPATCH_EVFILT_MACH_NOTIFICATION, EV_ADD|EV_ENABLE,
			flag, 0, (uintptr_t)dk, 0, 0);
	if (final) {
		// This can never happen again
		unreg = true;
	} else {
		// Re-register for notification before delivery
		unreg = _dispatch_kevent_resume(dk, flag, 0);
	}
	DISPATCH_MACH_KEVENT_ARMED(dk) = 0;
	TAILQ_FOREACH_SAFE(dri, &dk->dk_sources, dr_list, dr_next) {
		dispatch_source_t dsi = _dispatch_source_from_refs(dri);
		if (dx_type(dsi) == DISPATCH_MACH_CHANNEL_TYPE) {
			dispatch_mach_t dm = (dispatch_mach_t)dsi;
			_dispatch_mach_merge_kevent(dm, &kev);
			if (unreg && dm->dm_dkev) {
				_dispatch_mach_kevent_unregister(dm);
			}
		} else {
			_dispatch_source_merge_kevent(dsi, &kev);
			if (unreg) {
				_dispatch_source_kevent_unregister(dsi);
			}
		}
		if (!dr_next || DISPATCH_MACH_KEVENT_ARMED(dk)) {
			// current merge is last in list (dk might have been freed)
			// or it re-armed the notification
			return;
		}
	}
}

static kern_return_t
_dispatch_mach_notify_update(dispatch_kevent_t dk, uint32_t new_flags,
		uint32_t del_flags, uint32_t mask, mach_msg_id_t notify_msgid,
		mach_port_mscount_t notify_sync)
{
	mach_port_t previous, port = (mach_port_t)dk->dk_kevent.ident;
	typeof(dk->dk_kevent.data) prev = dk->dk_kevent.data;
	kern_return_t kr, krr = 0;

	// Update notification registration state.
	dk->dk_kevent.data |= (new_flags | dk->dk_kevent.fflags) & mask;
	dk->dk_kevent.data &= ~(del_flags & mask);

	_dispatch_debug_machport(port);
	if ((dk->dk_kevent.data & mask) && !(prev & mask)) {
		// initialize _dispatch_mach_notify_port:
		(void)_dispatch_get_mach_recv_portset();
		_dispatch_debug("machport[0x%08x]: registering for send-possible "
				"notification", port);
		previous = MACH_PORT_NULL;
		krr = mach_port_request_notification(mach_task_self(), port,
				notify_msgid, notify_sync, _dispatch_mach_notify_port,
				MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous);
		DISPATCH_VERIFY_MIG(krr);

		switch(krr) {
		case KERN_INVALID_NAME:
		case KERN_INVALID_RIGHT:
			// Supress errors & clear registration state
			dk->dk_kevent.data &= ~mask;
			break;
		default:
			// Else, we dont expect any errors from mach. Log any errors
			if (dispatch_assume_zero(krr)) {
				// log the error & clear registration state
				dk->dk_kevent.data &= ~mask;
			} else if (dispatch_assume_zero(previous)) {
				// Another subsystem has beat libdispatch to requesting the
				// specified Mach notification on this port. We should
				// technically cache the previous port and message it when the
				// kernel messages our port. Or we can just say screw those
				// subsystems and deallocate the previous port.
				// They should adopt libdispatch :-P
				kr = mach_port_deallocate(mach_task_self(), previous);
				DISPATCH_VERIFY_MIG(kr);
				(void)dispatch_assume_zero(kr);
				previous = MACH_PORT_NULL;
			}
		}
	} else if (!(dk->dk_kevent.data & mask) && (prev & mask)) {
		_dispatch_debug("machport[0x%08x]: unregistering for send-possible "
				"notification", port);
		previous = MACH_PORT_NULL;
		kr = mach_port_request_notification(mach_task_self(), port,
				notify_msgid, notify_sync, MACH_PORT_NULL,
				MACH_MSG_TYPE_MOVE_SEND_ONCE, &previous);
		DISPATCH_VERIFY_MIG(kr);

		switch (kr) {
		case KERN_INVALID_NAME:
		case KERN_INVALID_RIGHT:
		case KERN_INVALID_ARGUMENT:
			break;
		default:
			if (dispatch_assume_zero(kr)) {
				// log the error
			}
		}
	} else {
		return 0;
	}
	if (slowpath(previous)) {
		// the kernel has not consumed the send-once right yet
		(void)dispatch_assume_zero(
				_dispatch_send_consume_send_once_right(previous));
	}
	return krr;
}

static void
_dispatch_mach_host_notify_update(void *context DISPATCH_UNUSED)
{
	(void)_dispatch_get_mach_recv_portset();
	_dispatch_debug("registering for calendar-change notification");
	kern_return_t kr = host_request_notification(_dispatch_get_mach_host_port(),
			HOST_NOTIFY_CALENDAR_CHANGE, _dispatch_mach_notify_port);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
}

static void
_dispatch_mach_host_calendar_change_register(void)
{
	static dispatch_once_t pred;
	dispatch_once_f(&pred, NULL, _dispatch_mach_host_notify_update);
}

static void
_dispatch_mach_notify_source_invoke(mach_msg_header_t *hdr)
{
	mig_reply_error_t reply;
	dispatch_assert(sizeof(mig_reply_error_t) == sizeof(union
		__ReplyUnion___dispatch_libdispatch_internal_protocol_subsystem));
	dispatch_assert(sizeof(mig_reply_error_t) < _dispatch_mach_recv_msg_size);
	boolean_t success = libdispatch_internal_protocol_server(hdr, &reply.Head);
	if (!success && reply.RetCode == MIG_BAD_ID && hdr->msgh_id == 950) {
		// host_notify_reply.defs: host_calendar_changed
		_dispatch_debug("calendar-change notification");
		_dispatch_timers_calendar_change();
		_dispatch_mach_host_notify_update(NULL);
		success = TRUE;
		reply.RetCode = KERN_SUCCESS;
	}
	if (dispatch_assume(success) && reply.RetCode != MIG_NO_REPLY) {
		(void)dispatch_assume_zero(reply.RetCode);
	}
}

kern_return_t
_dispatch_mach_notify_port_deleted(mach_port_t notify DISPATCH_UNUSED,
		mach_port_name_t name)
{
#if DISPATCH_DEBUG
	_dispatch_log("Corruption: Mach send/send-once/dead-name right 0x%x "
			"deleted prematurely", name);
#endif

	_dispatch_debug_machport(name);
	_dispatch_mach_notify_merge(name, DISPATCH_MACH_SEND_DELETED, true);

	return KERN_SUCCESS;
}

kern_return_t
_dispatch_mach_notify_dead_name(mach_port_t notify DISPATCH_UNUSED,
		mach_port_name_t name)
{
	kern_return_t kr;

	_dispatch_debug("machport[0x%08x]: dead-name notification", name);
	_dispatch_debug_machport(name);
	_dispatch_mach_notify_merge(name, DISPATCH_MACH_SEND_DEAD, true);

	// the act of receiving a dead name notification allocates a dead-name
	// right that must be deallocated
	kr = mach_port_deallocate(mach_task_self(), name);
	DISPATCH_VERIFY_MIG(kr);
	//(void)dispatch_assume_zero(kr);

	return KERN_SUCCESS;
}

kern_return_t
_dispatch_mach_notify_send_possible(mach_port_t notify DISPATCH_UNUSED,
		mach_port_name_t name)
{
	_dispatch_debug("machport[0x%08x]: send-possible notification", name);
	_dispatch_debug_machport(name);
	_dispatch_mach_notify_merge(name, DISPATCH_MACH_SEND_POSSIBLE, false);

	return KERN_SUCCESS;
}

#pragma mark -
#pragma mark dispatch_mach_t

#define DISPATCH_MACH_NEVER_CONNECTED (UINT32_MAX/2)
#define DISPATCH_MACH_REGISTER_FOR_REPLY 0x2
#define DISPATCH_MACH_OPTIONS_MASK 0xffff

static mach_port_t _dispatch_mach_msg_get_remote_port(dispatch_object_t dou);
static void _dispatch_mach_msg_disconnected(dispatch_mach_t dm,
		mach_port_t local_port, mach_port_t remote_port);
static dispatch_mach_msg_t _dispatch_mach_msg_create_reply_disconnected(
		dispatch_object_t dou, dispatch_mach_reply_refs_t dmr);
static bool _dispatch_mach_reconnect_invoke(dispatch_mach_t dm,
		dispatch_object_t dou);
static inline mach_msg_header_t* _dispatch_mach_msg_get_msg(
		dispatch_mach_msg_t dmsg);
static void _dispatch_mach_push(dispatch_object_t dm, dispatch_object_t dou,
		pthread_priority_t pp);

static dispatch_mach_t
_dispatch_mach_create(const char *label, dispatch_queue_t q, void *context,
		dispatch_mach_handler_function_t handler, bool handler_is_block)
{
	dispatch_mach_t dm;
	dispatch_mach_refs_t dr;

	dm = _dispatch_alloc(DISPATCH_VTABLE(mach),
			sizeof(struct dispatch_mach_s));
	_dispatch_queue_init((dispatch_queue_t)dm);
	dm->dq_label = label;

	dm->do_ref_cnt++; // the reference _dispatch_mach_cancel_invoke holds
	dm->do_ref_cnt++; // since channel is created suspended
	dm->do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_INTERVAL;
	dm->do_targetq = &_dispatch_mgr_q;

	dr = _dispatch_calloc(1ul, sizeof(struct dispatch_mach_refs_s));
	dr->dr_source_wref = _dispatch_ptr2wref(dm);
	dr->dm_handler_func = handler;
	dr->dm_handler_ctxt = context;
	dm->ds_refs = dr;
	dm->dm_handler_is_block = handler_is_block;

	dm->dm_refs = _dispatch_calloc(1ul,
			sizeof(struct dispatch_mach_send_refs_s));
	dm->dm_refs->dr_source_wref = _dispatch_ptr2wref(dm);
	dm->dm_refs->dm_disconnect_cnt = DISPATCH_MACH_NEVER_CONNECTED;
	TAILQ_INIT(&dm->dm_refs->dm_replies);

	// First item on the channel sets the user-specified target queue
	dispatch_set_target_queue(dm, q);
	_dispatch_object_debug(dm, "%s", __func__);
	return dm;
}

dispatch_mach_t
dispatch_mach_create(const char *label, dispatch_queue_t q,
		dispatch_mach_handler_t handler)
{
	dispatch_block_t bb = _dispatch_Block_copy((void*)handler);
	return _dispatch_mach_create(label, q, bb,
			(dispatch_mach_handler_function_t)_dispatch_Block_invoke(bb), true);
}

dispatch_mach_t
dispatch_mach_create_f(const char *label, dispatch_queue_t q, void *context,
		dispatch_mach_handler_function_t handler)
{
	return _dispatch_mach_create(label, q, context, handler, false);
}

void
_dispatch_mach_dispose(dispatch_mach_t dm)
{
	_dispatch_object_debug(dm, "%s", __func__);
	dispatch_mach_refs_t dr = dm->ds_refs;
	if (dm->dm_handler_is_block && dr->dm_handler_ctxt) {
		Block_release(dr->dm_handler_ctxt);
	}
	free(dr);
	free(dm->dm_refs);
	_dispatch_queue_destroy(dm);
}

void
dispatch_mach_connect(dispatch_mach_t dm, mach_port_t receive,
		mach_port_t send, dispatch_mach_msg_t checkin)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	dispatch_kevent_t dk;

	if (MACH_PORT_VALID(receive)) {
		dk = _dispatch_calloc(1ul, sizeof(struct dispatch_kevent_s));
		dk->dk_kevent = _dispatch_source_type_mach_recv_direct.ke;
		dk->dk_kevent.ident = receive;
		dk->dk_kevent.flags |= EV_ADD|EV_ENABLE;
		dk->dk_kevent.udata = (uintptr_t)dk;
		TAILQ_INIT(&dk->dk_sources);
		dm->ds_dkev = dk;
		dm->ds_pending_data_mask = dk->dk_kevent.fflags;
		_dispatch_retain(dm); // the reference the manager queue holds
	}
	dr->dm_send = send;
	if (MACH_PORT_VALID(send)) {
		if (checkin) {
			dispatch_retain(checkin);
			mach_msg_option_t options = _dispatch_mach_checkin_options();
			_dispatch_mach_msg_set_options(checkin, options);
			dr->dm_checkin_port = _dispatch_mach_msg_get_remote_port(checkin);
		}
		dr->dm_checkin = checkin;
	}
	// monitor message reply ports
	dm->ds_pending_data_mask |= DISPATCH_MACH_RECV_MESSAGE_DIRECT_ONCE;
	if (slowpath(!dispatch_atomic_cmpxchg2o(dr, dm_disconnect_cnt,
			DISPATCH_MACH_NEVER_CONNECTED, 0, release))) {
		DISPATCH_CLIENT_CRASH("Channel already connected");
	}
	_dispatch_object_debug(dm, "%s", __func__);
	return dispatch_resume(dm);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_reply_kevent_unregister(dispatch_mach_t dm,
		dispatch_mach_reply_refs_t dmr, bool disconnected)
{
	dispatch_mach_msg_t dmsgr = NULL;
	if (disconnected) {
		dmsgr = _dispatch_mach_msg_create_reply_disconnected(NULL, dmr);
	}
	dispatch_kevent_t dk = dmr->dmr_dkev;
	TAILQ_REMOVE(&dk->dk_sources, (dispatch_source_refs_t)dmr, dr_list);
	_dispatch_kevent_unregister(dk, DISPATCH_MACH_RECV_MESSAGE_DIRECT_ONCE);
	TAILQ_REMOVE(&dm->dm_refs->dm_replies, dmr, dmr_list);
	if (dmr->dmr_voucher) _voucher_release(dmr->dmr_voucher);
	free(dmr);
	if (dmsgr) _dispatch_mach_push(dm, dmsgr, dmsgr->dmsg_priority);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_reply_kevent_register(dispatch_mach_t dm, mach_port_t reply,
		dispatch_mach_msg_t dmsg)
{
	dispatch_kevent_t dk;
	dispatch_mach_reply_refs_t dmr;

	dk = _dispatch_calloc(1ul, sizeof(struct dispatch_kevent_s));
	dk->dk_kevent = _dispatch_source_type_mach_recv_direct.ke;
	dk->dk_kevent.ident = reply;
	dk->dk_kevent.flags |= EV_ADD|EV_ENABLE;
	dk->dk_kevent.fflags = DISPATCH_MACH_RECV_MESSAGE_DIRECT_ONCE;
	dk->dk_kevent.udata = (uintptr_t)dk;
	TAILQ_INIT(&dk->dk_sources);

	dmr = _dispatch_calloc(1ul, sizeof(struct dispatch_mach_reply_refs_s));
	dmr->dr_source_wref = _dispatch_ptr2wref(dm);
	dmr->dmr_dkev = dk;
	if (dmsg->dmsg_voucher) {
		dmr->dmr_voucher =_voucher_retain(dmsg->dmsg_voucher);
	}
	dmr->dmr_priority = dmsg->dmsg_priority;
	// make reply context visible to leaks rdar://11777199
	dmr->dmr_ctxt = dmsg->do_ctxt;

	_dispatch_debug("machport[0x%08x]: registering for reply, ctxt %p", reply,
			dmsg->do_ctxt);
	uint32_t flags;
	bool do_resume = _dispatch_kevent_register(&dmr->dmr_dkev, &flags);
	TAILQ_INSERT_TAIL(&dmr->dmr_dkev->dk_sources, (dispatch_source_refs_t)dmr,
			dr_list);
	TAILQ_INSERT_TAIL(&dm->dm_refs->dm_replies, dmr, dmr_list);
	if (do_resume && _dispatch_kevent_resume(dmr->dmr_dkev, flags, 0)) {
		_dispatch_mach_reply_kevent_unregister(dm, dmr, true);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_mach_kevent_unregister(dispatch_mach_t dm)
{
	dispatch_kevent_t dk = dm->dm_dkev;
	dm->dm_dkev = NULL;
	TAILQ_REMOVE(&dk->dk_sources, (dispatch_source_refs_t)dm->dm_refs,
			dr_list);
	dm->ds_pending_data_mask &= ~(unsigned long)
			(DISPATCH_MACH_SEND_POSSIBLE|DISPATCH_MACH_SEND_DEAD);
	_dispatch_kevent_unregister(dk,
			DISPATCH_MACH_SEND_POSSIBLE|DISPATCH_MACH_SEND_DEAD);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_kevent_register(dispatch_mach_t dm, mach_port_t send)
{
	dispatch_kevent_t dk;

	dk = _dispatch_calloc(1ul, sizeof(struct dispatch_kevent_s));
	dk->dk_kevent = _dispatch_source_type_mach_send.ke;
	dk->dk_kevent.ident = send;
	dk->dk_kevent.flags |= EV_ADD|EV_ENABLE;
	dk->dk_kevent.fflags = DISPATCH_MACH_SEND_POSSIBLE|DISPATCH_MACH_SEND_DEAD;
	dk->dk_kevent.udata = (uintptr_t)dk;
	TAILQ_INIT(&dk->dk_sources);

	dm->ds_pending_data_mask |= dk->dk_kevent.fflags;

	uint32_t flags;
	bool do_resume = _dispatch_kevent_register(&dk, &flags);
	TAILQ_INSERT_TAIL(&dk->dk_sources,
			(dispatch_source_refs_t)dm->dm_refs, dr_list);
	dm->dm_dkev = dk;
	if (do_resume && _dispatch_kevent_resume(dm->dm_dkev, flags, 0)) {
		_dispatch_mach_kevent_unregister(dm);
	}
}

static inline void
_dispatch_mach_push(dispatch_object_t dm, dispatch_object_t dou,
		pthread_priority_t pp)
{
	return _dispatch_queue_push(dm._dq, dou, pp);
}

static inline void
_dispatch_mach_msg_set_options(dispatch_object_t dou, mach_msg_option_t options)
{
	dou._do->do_suspend_cnt = (unsigned int)options;
}

static inline mach_msg_option_t
_dispatch_mach_msg_get_options(dispatch_object_t dou)
{
	mach_msg_option_t options = (mach_msg_option_t)dou._do->do_suspend_cnt;
	return options;
}

static inline void
_dispatch_mach_msg_set_reason(dispatch_object_t dou, mach_error_t err,
		unsigned long reason)
{
	dispatch_assert_zero(reason & ~(unsigned long)code_emask);
	dou._do->do_suspend_cnt =  (unsigned int)((err || !reason) ? err :
			 err_local|err_sub(0x3e0)|(mach_error_t)reason);
}

static inline unsigned long
_dispatch_mach_msg_get_reason(dispatch_object_t dou, mach_error_t *err_ptr)
{
	mach_error_t err = (mach_error_t)dou._do->do_suspend_cnt;
	dou._do->do_suspend_cnt = 0;
	if ((err & system_emask) == err_local && err_get_sub(err) == 0x3e0) {
		*err_ptr = 0;
		return err_get_code(err);
	}
	*err_ptr = err;
	return err ? DISPATCH_MACH_MESSAGE_SEND_FAILED : DISPATCH_MACH_MESSAGE_SENT;
}

static void
_dispatch_mach_msg_recv(dispatch_mach_t dm, dispatch_mach_reply_refs_t dmr,
		mach_msg_header_t *hdr, mach_msg_size_t siz)
{
	_dispatch_debug_machport(hdr->msgh_remote_port);
	_dispatch_debug("machport[0x%08x]: received msg id 0x%x, reply on 0x%08x",
			hdr->msgh_local_port, hdr->msgh_id, hdr->msgh_remote_port);
	if (slowpath(dm->ds_atomic_flags & DSF_CANCELED)) {
		return _dispatch_kevent_mach_msg_destroy(hdr);
	}
	dispatch_mach_msg_t dmsg;
	voucher_t voucher;
	pthread_priority_t priority;
	void *ctxt = NULL;
	if (dmr) {
		_voucher_mach_msg_clear(hdr, false); // deallocate reply message voucher
		voucher = dmr->dmr_voucher;
		dmr->dmr_voucher = NULL; // transfer reference
		priority = dmr->dmr_priority;
		ctxt = dmr->dmr_ctxt;
		_dispatch_mach_reply_kevent_unregister(dm, dmr, false);
	} else {
		voucher = voucher_create_with_mach_msg(hdr);
		priority = _voucher_get_priority(voucher);
	}
	dispatch_mach_msg_destructor_t destructor;
	destructor = (hdr == _dispatch_get_mach_recv_msg_buf()) ?
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT :
			DISPATCH_MACH_MSG_DESTRUCTOR_FREE;
	dmsg = dispatch_mach_msg_create(hdr, siz, destructor, NULL);
	dmsg->dmsg_voucher = voucher;
	dmsg->dmsg_priority = priority;
	dmsg->do_ctxt = ctxt;
	_dispatch_mach_msg_set_reason(dmsg, 0, DISPATCH_MACH_MESSAGE_RECEIVED);
	_dispatch_voucher_debug("mach-msg[%p] create", voucher, dmsg);
	_dispatch_voucher_ktrace_dmsg_push(dmsg);
	return _dispatch_mach_push(dm, dmsg, dmsg->dmsg_priority);
}

static inline mach_port_t
_dispatch_mach_msg_get_remote_port(dispatch_object_t dou)
{
	mach_msg_header_t *hdr = _dispatch_mach_msg_get_msg(dou._dmsg);
	mach_port_t remote = hdr->msgh_remote_port;
	return remote;
}

static inline void
_dispatch_mach_msg_disconnected(dispatch_mach_t dm, mach_port_t local_port,
		mach_port_t remote_port)
{
	mach_msg_header_t *hdr;
	dispatch_mach_msg_t dmsg;
	dmsg = dispatch_mach_msg_create(NULL, sizeof(mach_msg_header_t),
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT, &hdr);
	if (local_port) hdr->msgh_local_port = local_port;
	if (remote_port) hdr->msgh_remote_port = remote_port;
	_dispatch_mach_msg_set_reason(dmsg, 0, DISPATCH_MACH_DISCONNECTED);
	return _dispatch_mach_push(dm, dmsg, dmsg->dmsg_priority);
}

static inline dispatch_mach_msg_t
_dispatch_mach_msg_create_reply_disconnected(dispatch_object_t dou,
		dispatch_mach_reply_refs_t dmr)
{
	dispatch_mach_msg_t dmsg = dou._dmsg, dmsgr;
	if (dmsg && !dmsg->dmsg_reply) return NULL;
	mach_msg_header_t *hdr;
	dmsgr = dispatch_mach_msg_create(NULL, sizeof(mach_msg_header_t),
			DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT, &hdr);
	if (dmsg) {
		hdr->msgh_local_port = dmsg->dmsg_reply;
		if (dmsg->dmsg_voucher) {
			dmsgr->dmsg_voucher = _voucher_retain(dmsg->dmsg_voucher);
		}
		dmsgr->dmsg_priority = dmsg->dmsg_priority;
		dmsgr->do_ctxt = dmsg->do_ctxt;
	} else {
		hdr->msgh_local_port = (mach_port_t)dmr->dmr_dkev->dk_kevent.ident;
		dmsgr->dmsg_voucher = dmr->dmr_voucher;
		dmr->dmr_voucher = NULL;  // transfer reference
		dmsgr->dmsg_priority = dmr->dmr_priority;
		dmsgr->do_ctxt = dmr->dmr_ctxt;
	}
	_dispatch_mach_msg_set_reason(dmsgr, 0, DISPATCH_MACH_DISCONNECTED);
	return dmsgr;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_msg_not_sent(dispatch_mach_t dm, dispatch_object_t dou)
{
	dispatch_mach_msg_t dmsg = dou._dmsg, dmsgr;
	dmsgr = _dispatch_mach_msg_create_reply_disconnected(dmsg, NULL);
	_dispatch_mach_msg_set_reason(dmsg, 0, DISPATCH_MACH_MESSAGE_NOT_SENT);
	_dispatch_mach_push(dm, dmsg, dmsg->dmsg_priority);
	if (dmsgr) _dispatch_mach_push(dm, dmsgr, dmsgr->dmsg_priority);
}

DISPATCH_NOINLINE
static dispatch_object_t
_dispatch_mach_msg_send(dispatch_mach_t dm, dispatch_object_t dou)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	dispatch_mach_msg_t dmsg = dou._dmsg, dmsgr = NULL;
	voucher_t voucher = dmsg->dmsg_voucher;
	mach_voucher_t ipc_kvoucher = MACH_VOUCHER_NULL;
	bool clear_voucher = false, kvoucher_move_send = false;
	dr->dm_needs_mgr = 0;
	if (slowpath(dr->dm_checkin) && dmsg != dr->dm_checkin) {
		// send initial checkin message
		if (dm->dm_dkev && slowpath(_dispatch_queue_get_current() !=
				&_dispatch_mgr_q)) {
			// send kevent must be uninstalled on the manager queue
			dr->dm_needs_mgr = 1;
			goto out;
		}
		dr->dm_checkin = _dispatch_mach_msg_send(dm, dr->dm_checkin)._dmsg;
		if (slowpath(dr->dm_checkin)) {
			goto out;
		}
	}
	mach_msg_header_t *msg = _dispatch_mach_msg_get_msg(dmsg);
	mach_msg_return_t kr = 0;
	mach_port_t reply = dmsg->dmsg_reply;
	mach_msg_option_t opts = 0, msg_opts = _dispatch_mach_msg_get_options(dmsg);
	if (!slowpath(msg_opts & DISPATCH_MACH_REGISTER_FOR_REPLY)) {
		opts = MACH_SEND_MSG | (msg_opts & ~DISPATCH_MACH_OPTIONS_MASK);
		if (MACH_MSGH_BITS_REMOTE(msg->msgh_bits) !=
				MACH_MSG_TYPE_MOVE_SEND_ONCE) {
			if (dmsg != dr->dm_checkin) {
				msg->msgh_remote_port = dr->dm_send;
			}
			if (_dispatch_queue_get_current() == &_dispatch_mgr_q) {
				if (slowpath(!dm->dm_dkev)) {
					_dispatch_mach_kevent_register(dm, msg->msgh_remote_port);
				}
				if (fastpath(dm->dm_dkev)) {
					if (DISPATCH_MACH_KEVENT_ARMED(dm->dm_dkev)) {
						goto out;
					}
					opts |= MACH_SEND_NOTIFY;
				}
			}
			opts |= MACH_SEND_TIMEOUT;
			if (dmsg->dmsg_priority != _voucher_get_priority(voucher)) {
				ipc_kvoucher = _voucher_create_mach_voucher_with_priority(
						voucher, dmsg->dmsg_priority);
			}
			_dispatch_voucher_debug("mach-msg[%p] msg_set", voucher, dmsg);
			if (ipc_kvoucher) {
				kvoucher_move_send = true;
				clear_voucher = _voucher_mach_msg_set_mach_voucher(msg,
						ipc_kvoucher, kvoucher_move_send);
			} else {
				clear_voucher = _voucher_mach_msg_set(msg, voucher);
			}
		}
		_voucher_activity_trace_msg(voucher, msg, send);
		_dispatch_debug_machport(msg->msgh_remote_port);
		if (reply) _dispatch_debug_machport(reply);
		kr = mach_msg(msg, opts, msg->msgh_size, 0, MACH_PORT_NULL, 0,
				MACH_PORT_NULL);
		_dispatch_debug("machport[0x%08x]: sent msg id 0x%x, ctxt %p, "
				"opts 0x%x, msg_opts 0x%x, kvoucher 0x%08x, reply on 0x%08x: "
				"%s - 0x%x", msg->msgh_remote_port, msg->msgh_id, dmsg->do_ctxt,
				opts, msg_opts, msg->msgh_voucher_port, reply,
				mach_error_string(kr), kr);
		if (clear_voucher) {
			if (kr == MACH_SEND_INVALID_VOUCHER && msg->msgh_voucher_port) {
				DISPATCH_CRASH("Voucher port corruption");
			}
			mach_voucher_t kv;
			kv = _voucher_mach_msg_clear(msg, kvoucher_move_send);
			if (kvoucher_move_send) ipc_kvoucher = kv;
		}
	}
	if (kr == MACH_SEND_TIMED_OUT && (opts & MACH_SEND_TIMEOUT)) {
		if (opts & MACH_SEND_NOTIFY) {
			_dispatch_debug("machport[0x%08x]: send-possible notification "
					"armed", (mach_port_t)dm->dm_dkev->dk_kevent.ident);
			DISPATCH_MACH_KEVENT_ARMED(dm->dm_dkev) = 1;
		} else {
			// send kevent must be installed on the manager queue
			dr->dm_needs_mgr = 1;
		}
		if (ipc_kvoucher) {
			_dispatch_kvoucher_debug("reuse on re-send", ipc_kvoucher);
			voucher_t ipc_voucher;
			ipc_voucher = _voucher_create_with_priority_and_mach_voucher(
					voucher, dmsg->dmsg_priority, ipc_kvoucher);
			_dispatch_voucher_debug("mach-msg[%p] replace voucher[%p]",
					ipc_voucher, dmsg, voucher);
			if (dmsg->dmsg_voucher) _voucher_release(dmsg->dmsg_voucher);
			dmsg->dmsg_voucher = ipc_voucher;
		}
		goto out;
	} else if (ipc_kvoucher && (kr || !kvoucher_move_send)) {
		_voucher_dealloc_mach_voucher(ipc_kvoucher);
	}
	if (fastpath(!kr) && reply &&
			!(dm->ds_dkev && dm->ds_dkev->dk_kevent.ident == reply)) {
		if (_dispatch_queue_get_current() != &_dispatch_mgr_q) {
			// reply receive kevent must be installed on the manager queue
			dr->dm_needs_mgr = 1;
			_dispatch_mach_msg_set_options(dmsg, msg_opts |
					DISPATCH_MACH_REGISTER_FOR_REPLY);
			goto out;
		}
		_dispatch_mach_reply_kevent_register(dm, reply, dmsg);
	}
	if (slowpath(dmsg == dr->dm_checkin) && dm->dm_dkev) {
		_dispatch_mach_kevent_unregister(dm);
	}
	if (slowpath(kr)) {
		// Send failed, so reply was never connected <rdar://problem/14309159>
		dmsgr = _dispatch_mach_msg_create_reply_disconnected(dmsg, NULL);
	}
	_dispatch_mach_msg_set_reason(dmsg, kr, 0);
	_dispatch_mach_push(dm, dmsg, dmsg->dmsg_priority);
	if (dmsgr) _dispatch_mach_push(dm, dmsgr, dmsgr->dmsg_priority);
	dmsg = NULL;
out:
	return (dispatch_object_t)dmsg;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_send_push_wakeup(dispatch_mach_t dm, dispatch_object_t dou,
		bool wakeup)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	struct dispatch_object_s *prev, *dc = dou._do;
	dc->do_next = NULL;

	prev = dispatch_atomic_xchg2o(dr, dm_tail, dc, release);
	if (fastpath(prev)) {
		prev->do_next = dc;
	} else {
		dr->dm_head = dc;
	}
	if (wakeup || !prev) {
		_dispatch_wakeup(dm);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_mach_send_push(dispatch_mach_t dm, dispatch_object_t dou)
{
	return _dispatch_mach_send_push_wakeup(dm, dou, false);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_send_drain(dispatch_mach_t dm)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	struct dispatch_object_s *dc = NULL, *next_dc = NULL;
	while (dr->dm_tail) {
		_dispatch_wait_until(dc = fastpath(dr->dm_head));
		do {
			next_dc = fastpath(dc->do_next);
			dr->dm_head = next_dc;
			if (!next_dc && !dispatch_atomic_cmpxchg2o(dr, dm_tail, dc, NULL,
					relaxed)) {
				_dispatch_wait_until(next_dc = fastpath(dc->do_next));
				dr->dm_head = next_dc;
			}
			if (!DISPATCH_OBJ_IS_VTABLE(dc)) {
				if ((long)dc->do_vtable & DISPATCH_OBJ_BARRIER_BIT) {
					// send barrier
					// leave send queue locked until barrier has completed
					return _dispatch_mach_push(dm, dc,
							((dispatch_continuation_t)dc)->dc_priority);
				}
#if DISPATCH_MACH_SEND_SYNC
				if (slowpath((long)dc->do_vtable & DISPATCH_OBJ_SYNC_SLOW_BIT)){
					_dispatch_thread_semaphore_signal(
							(_dispatch_thread_semaphore_t)dc->do_ctxt);
					continue;
				}
#endif // DISPATCH_MACH_SEND_SYNC
				if (slowpath(!_dispatch_mach_reconnect_invoke(dm, dc))) {
					goto out;
				}
				continue;
			}
			_dispatch_voucher_ktrace_dmsg_pop((dispatch_mach_msg_t)dc);
			if (slowpath(dr->dm_disconnect_cnt) ||
					slowpath(dm->ds_atomic_flags & DSF_CANCELED)) {
				_dispatch_mach_msg_not_sent(dm, dc);
				continue;
			}
			if (slowpath(dc = _dispatch_mach_msg_send(dm, dc)._do)) {
				goto out;
			}
		} while ((dc = next_dc));
	}
out:
	// if this is not a complete drain, we must undo some things
	if (slowpath(dc)) {
		if (!next_dc &&
				!dispatch_atomic_cmpxchg2o(dr, dm_tail, NULL, dc, relaxed)) {
			// wait for enqueue slow path to finish
			_dispatch_wait_until(next_dc = fastpath(dr->dm_head));
			dc->do_next = next_dc;
		}
		dr->dm_head = dc;
	}
	(void)dispatch_atomic_dec2o(dr, dm_sending, release);
	_dispatch_wakeup(dm);
}

static inline void
_dispatch_mach_send(dispatch_mach_t dm)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	if (!fastpath(dr->dm_tail) || !fastpath(dispatch_atomic_cmpxchg2o(dr,
			dm_sending, 0, 1, acquire))) {
		return;
	}
	_dispatch_object_debug(dm, "%s", __func__);
	_dispatch_mach_send_drain(dm);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_merge_kevent(dispatch_mach_t dm, const struct kevent64_s *ke)
{
	if (!(ke->fflags & dm->ds_pending_data_mask)) {
		return;
	}
	_dispatch_mach_send(dm);
}

static inline mach_msg_option_t
_dispatch_mach_checkin_options(void)
{
	mach_msg_option_t options = 0;
#if DISPATCH_USE_CHECKIN_NOIMPORTANCE
	options = MACH_SEND_NOIMPORTANCE; // <rdar://problem/16996737>
#endif
	return options;
}


static inline mach_msg_option_t
_dispatch_mach_send_options(void)
{
	mach_msg_option_t options = 0;
	return options;
}

DISPATCH_NOINLINE
void
dispatch_mach_send(dispatch_mach_t dm, dispatch_mach_msg_t dmsg,
		mach_msg_option_t options)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	if (slowpath(dmsg->do_next != DISPATCH_OBJECT_LISTLESS)) {
		DISPATCH_CLIENT_CRASH("Message already enqueued");
	}
	dispatch_retain(dmsg);
	dispatch_assert_zero(options & DISPATCH_MACH_OPTIONS_MASK);
	options |= _dispatch_mach_send_options();
	_dispatch_mach_msg_set_options(dmsg, options & ~DISPATCH_MACH_OPTIONS_MASK);
	mach_msg_header_t *msg = _dispatch_mach_msg_get_msg(dmsg);
	dmsg->dmsg_reply = (MACH_MSGH_BITS_LOCAL(msg->msgh_bits) ==
			MACH_MSG_TYPE_MAKE_SEND_ONCE &&
			MACH_PORT_VALID(msg->msgh_local_port) ? msg->msgh_local_port :
			MACH_PORT_NULL);
	bool is_reply = (MACH_MSGH_BITS_REMOTE(msg->msgh_bits) ==
			MACH_MSG_TYPE_MOVE_SEND_ONCE);
	dmsg->dmsg_priority = _dispatch_priority_propagate();
	dmsg->dmsg_voucher = _voucher_copy();
	_dispatch_voucher_debug("mach-msg[%p] set", dmsg->dmsg_voucher, dmsg);
	if ((!is_reply && slowpath(dr->dm_tail)) ||
			slowpath(dr->dm_disconnect_cnt) ||
			slowpath(dm->ds_atomic_flags & DSF_CANCELED) ||
			slowpath(!dispatch_atomic_cmpxchg2o(dr, dm_sending, 0, 1,
					acquire))) {
		_dispatch_voucher_ktrace_dmsg_push(dmsg);
		return _dispatch_mach_send_push(dm, dmsg);
	}
	if (slowpath(dmsg = _dispatch_mach_msg_send(dm, dmsg)._dmsg)) {
		(void)dispatch_atomic_dec2o(dr, dm_sending, release);
		_dispatch_voucher_ktrace_dmsg_push(dmsg);
		return _dispatch_mach_send_push_wakeup(dm, dmsg, true);
	}
	if (!is_reply && slowpath(dr->dm_tail)) {
		return _dispatch_mach_send_drain(dm);
	}
	(void)dispatch_atomic_dec2o(dr, dm_sending, release);
	_dispatch_wakeup(dm);
}

static void
_dispatch_mach_disconnect(dispatch_mach_t dm)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	if (dm->dm_dkev) {
		_dispatch_mach_kevent_unregister(dm);
	}
	if (MACH_PORT_VALID(dr->dm_send)) {
		_dispatch_mach_msg_disconnected(dm, MACH_PORT_NULL, dr->dm_send);
	}
	dr->dm_send = MACH_PORT_NULL;
	if (dr->dm_checkin) {
		_dispatch_mach_msg_not_sent(dm, dr->dm_checkin);
		dr->dm_checkin = NULL;
	}
	if (!TAILQ_EMPTY(&dm->dm_refs->dm_replies)) {
		dispatch_mach_reply_refs_t dmr, tmp;
		TAILQ_FOREACH_SAFE(dmr, &dm->dm_refs->dm_replies, dmr_list, tmp){
			_dispatch_mach_reply_kevent_unregister(dm, dmr, true);
		}
	}
}

DISPATCH_NOINLINE
static bool
_dispatch_mach_cancel(dispatch_mach_t dm)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	if (!fastpath(dispatch_atomic_cmpxchg2o(dr, dm_sending, 0, 1, acquire))) {
		return false;
	}
	_dispatch_object_debug(dm, "%s", __func__);
	_dispatch_mach_disconnect(dm);
	if (dm->ds_dkev) {
		mach_port_t local_port = (mach_port_t)dm->ds_dkev->dk_kevent.ident;
		_dispatch_source_kevent_unregister((dispatch_source_t)dm);
		_dispatch_mach_msg_disconnected(dm, local_port, MACH_PORT_NULL);
	}
	(void)dispatch_atomic_dec2o(dr, dm_sending, release);
	return true;
}

DISPATCH_NOINLINE
static bool
_dispatch_mach_reconnect_invoke(dispatch_mach_t dm, dispatch_object_t dou)
{
	if (dm->dm_dkev || !TAILQ_EMPTY(&dm->dm_refs->dm_replies)) {
		if (slowpath(_dispatch_queue_get_current() != &_dispatch_mgr_q)) {
			// send/reply kevents must be uninstalled on the manager queue
			return false;
		}
	}
	_dispatch_mach_disconnect(dm);
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	dr->dm_checkin = dou._dc->dc_data;
	dr->dm_send = (mach_port_t)dou._dc->dc_other;
	_dispatch_continuation_free(dou._dc);
	(void)dispatch_atomic_dec2o(dr, dm_disconnect_cnt, relaxed);
	_dispatch_object_debug(dm, "%s", __func__);
	return true;
}

DISPATCH_NOINLINE
void
dispatch_mach_reconnect(dispatch_mach_t dm, mach_port_t send,
		dispatch_mach_msg_t checkin)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	(void)dispatch_atomic_inc2o(dr, dm_disconnect_cnt, relaxed);
	if (MACH_PORT_VALID(send) && checkin) {
		dispatch_retain(checkin);
		mach_msg_option_t options = _dispatch_mach_checkin_options();
		_dispatch_mach_msg_set_options(checkin, options);
		dr->dm_checkin_port = _dispatch_mach_msg_get_remote_port(checkin);
	} else {
		checkin = NULL;
		dr->dm_checkin_port = MACH_PORT_NULL;
	}
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT);
	dc->dc_func = (void*)_dispatch_mach_reconnect_invoke;
	dc->dc_ctxt = dc;
	dc->dc_data = checkin;
	dc->dc_other = (void*)(uintptr_t)send;
	return _dispatch_mach_send_push(dm, dc);
}

#if DISPATCH_MACH_SEND_SYNC
DISPATCH_NOINLINE
static void
_dispatch_mach_send_sync_slow(dispatch_mach_t dm)
{
	_dispatch_thread_semaphore_t sema = _dispatch_get_thread_semaphore();
	struct dispatch_object_s dc = {
		.do_vtable = (void *)(DISPATCH_OBJ_SYNC_SLOW_BIT),
		.do_ctxt = (void*)sema,
	};
	_dispatch_mach_send_push(dm, &dc);
	_dispatch_thread_semaphore_wait(sema);
	_dispatch_put_thread_semaphore(sema);
}
#endif // DISPATCH_MACH_SEND_SYNC

DISPATCH_NOINLINE
mach_port_t
dispatch_mach_get_checkin_port(dispatch_mach_t dm)
{
	dispatch_mach_send_refs_t dr = dm->dm_refs;
	if (slowpath(dm->ds_atomic_flags & DSF_CANCELED)) {
		return MACH_PORT_DEAD;
	}
	return dr->dm_checkin_port;
}

DISPATCH_NOINLINE
static void
_dispatch_mach_connect_invoke(dispatch_mach_t dm)
{
	dispatch_mach_refs_t dr = dm->ds_refs;
	_dispatch_client_callout4(dr->dm_handler_ctxt,
			DISPATCH_MACH_CONNECTED, NULL, 0, dr->dm_handler_func);
	dm->dm_connect_handler_called = 1;
}

DISPATCH_NOINLINE
void
_dispatch_mach_msg_invoke(dispatch_mach_msg_t dmsg)
{
	dispatch_mach_t dm = (dispatch_mach_t)_dispatch_queue_get_current();
	dispatch_mach_refs_t dr = dm->ds_refs;
	mach_error_t err;
	unsigned long reason = _dispatch_mach_msg_get_reason(dmsg, &err);

	dmsg->do_next = DISPATCH_OBJECT_LISTLESS;
	_dispatch_thread_setspecific(dispatch_queue_key, dm->do_targetq);
	_dispatch_voucher_ktrace_dmsg_pop(dmsg);
	_dispatch_voucher_debug("mach-msg[%p] adopt", dmsg->dmsg_voucher, dmsg);
	_dispatch_adopt_priority_and_replace_voucher(dmsg->dmsg_priority,
			dmsg->dmsg_voucher, DISPATCH_PRIORITY_ENFORCE);
	dmsg->dmsg_voucher = NULL;
	if (slowpath(!dm->dm_connect_handler_called)) {
		_dispatch_mach_connect_invoke(dm);
	}
	_dispatch_client_callout4(dr->dm_handler_ctxt, reason, dmsg, err,
			dr->dm_handler_func);
	_dispatch_thread_setspecific(dispatch_queue_key, (dispatch_queue_t)dm);
	_dispatch_introspection_queue_item_complete(dmsg);
	dispatch_release(dmsg);
}

DISPATCH_NOINLINE
void
_dispatch_mach_barrier_invoke(void *ctxt)
{
	dispatch_mach_t dm = (dispatch_mach_t)_dispatch_queue_get_current();
	dispatch_mach_refs_t dr = dm->ds_refs;
	struct dispatch_continuation_s *dc = ctxt;
	void *context = dc->dc_data;
	dispatch_function_t barrier = dc->dc_other;
	bool send_barrier = ((long)dc->do_vtable & DISPATCH_OBJ_BARRIER_BIT);

	_dispatch_thread_setspecific(dispatch_queue_key, dm->do_targetq);
	if (slowpath(!dm->dm_connect_handler_called)) {
		_dispatch_mach_connect_invoke(dm);
	}
	_dispatch_client_callout(context, barrier);
	_dispatch_client_callout4(dr->dm_handler_ctxt,
			DISPATCH_MACH_BARRIER_COMPLETED, NULL, 0, dr->dm_handler_func);
	_dispatch_thread_setspecific(dispatch_queue_key, (dispatch_queue_t)dm);
	if (send_barrier) {
		(void)dispatch_atomic_dec2o(dm->dm_refs, dm_sending, release);
	}
}

DISPATCH_NOINLINE
void
dispatch_mach_send_barrier_f(dispatch_mach_t dm, void *context,
		dispatch_function_t barrier)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT | DISPATCH_OBJ_BARRIER_BIT);
	dc->dc_func = _dispatch_mach_barrier_invoke;
	dc->dc_ctxt = dc;
	dc->dc_data = context;
	dc->dc_other = barrier;
	_dispatch_continuation_voucher_set(dc, 0);
	_dispatch_continuation_priority_set(dc, 0, 0);

	dispatch_mach_send_refs_t dr = dm->dm_refs;
	if (slowpath(dr->dm_tail) || slowpath(!dispatch_atomic_cmpxchg2o(dr,
			dm_sending, 0, 1, acquire))) {
		return _dispatch_mach_send_push(dm, dc);
	}
	// leave send queue locked until barrier has completed
	return _dispatch_mach_push(dm, dc, dc->dc_priority);
}

DISPATCH_NOINLINE
void
dispatch_mach_receive_barrier_f(dispatch_mach_t dm, void *context,
		dispatch_function_t barrier)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT);
	dc->dc_func = _dispatch_mach_barrier_invoke;
	dc->dc_ctxt = dc;
	dc->dc_data = context;
	dc->dc_other = barrier;
	_dispatch_continuation_voucher_set(dc, 0);
	_dispatch_continuation_priority_set(dc, 0, 0);

	return _dispatch_mach_push(dm, dc, dc->dc_priority);
}

DISPATCH_NOINLINE
void
dispatch_mach_send_barrier(dispatch_mach_t dm, dispatch_block_t barrier)
{
	dispatch_mach_send_barrier_f(dm, _dispatch_Block_copy(barrier),
			_dispatch_call_block_and_release);
}

DISPATCH_NOINLINE
void
dispatch_mach_receive_barrier(dispatch_mach_t dm, dispatch_block_t barrier)
{
	dispatch_mach_receive_barrier_f(dm, _dispatch_Block_copy(barrier),
			_dispatch_call_block_and_release);
}

DISPATCH_NOINLINE
static void
_dispatch_mach_cancel_invoke(dispatch_mach_t dm)
{
	dispatch_mach_refs_t dr = dm->ds_refs;
	if (slowpath(!dm->dm_connect_handler_called)) {
		_dispatch_mach_connect_invoke(dm);
	}
	_dispatch_client_callout4(dr->dm_handler_ctxt,
			DISPATCH_MACH_CANCELED, NULL, 0, dr->dm_handler_func);
	dm->dm_cancel_handler_called = 1;
	_dispatch_release(dm); // the retain is done at creation time
}

DISPATCH_NOINLINE
void
dispatch_mach_cancel(dispatch_mach_t dm)
{
	dispatch_source_cancel((dispatch_source_t)dm);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
_dispatch_mach_invoke2(dispatch_object_t dou,
		_dispatch_thread_semaphore_t *sema_ptr DISPATCH_UNUSED)
{
	dispatch_mach_t dm = dou._dm;

	// This function performs all mach channel actions. Each action is
	// responsible for verifying that it takes place on the appropriate queue.
	// If the current queue is not the correct queue for this action, the
	// correct queue will be returned and the invoke will be re-driven on that
	// queue.

	// The order of tests here in invoke and in probe should be consistent.

	dispatch_queue_t dq = _dispatch_queue_get_current();
	dispatch_mach_send_refs_t dr = dm->dm_refs;

	if (slowpath(!dm->ds_is_installed)) {
		// The channel needs to be installed on the manager queue.
		if (dq != &_dispatch_mgr_q) {
			return &_dispatch_mgr_q;
		}
		if (dm->ds_dkev) {
			_dispatch_source_kevent_register((dispatch_source_t)dm);
		}
		dm->ds_is_installed = true;
		_dispatch_mach_send(dm);
		// Apply initial target queue change
		_dispatch_queue_drain(dou);
		if (dm->dq_items_tail) {
			return dm->do_targetq;
		}
	} else if (dm->dq_items_tail) {
		// The channel has pending messages to deliver to the target queue.
		if (dq != dm->do_targetq) {
			return dm->do_targetq;
		}
		dispatch_queue_t tq = dm->do_targetq;
		if (slowpath(_dispatch_queue_drain(dou))) {
			DISPATCH_CLIENT_CRASH("Sync onto mach channel");
		}
		if (slowpath(tq != dm->do_targetq)) {
			// An item on the channel changed the target queue
			return dm->do_targetq;
		}
	} else if (dr->dm_sending) {
		// Sending and uninstallation below require the send lock, the channel
		// will be woken up when the lock is dropped <rdar://15132939&15203957>
		return NULL;
	} else if (dr->dm_tail) {
		if (slowpath(dr->dm_needs_mgr) || (slowpath(dr->dm_disconnect_cnt) &&
				(dm->dm_dkev || !TAILQ_EMPTY(&dm->dm_refs->dm_replies)))) {
			// Send/reply kevents need to be installed or uninstalled
			if (dq != &_dispatch_mgr_q) {
				return &_dispatch_mgr_q;
			}
		}
		if (!(dm->dm_dkev && DISPATCH_MACH_KEVENT_ARMED(dm->dm_dkev)) ||
				(dm->ds_atomic_flags & DSF_CANCELED) || dr->dm_disconnect_cnt) {
			// The channel has pending messages to send.
			_dispatch_mach_send(dm);
		}
	} else if (dm->ds_atomic_flags & DSF_CANCELED){
		// The channel has been cancelled and needs to be uninstalled from the
		// manager queue. After uninstallation, the cancellation handler needs
		// to be delivered to the target queue.
		if (dm->ds_dkev || dm->dm_dkev || dr->dm_send ||
				!TAILQ_EMPTY(&dm->dm_refs->dm_replies)) {
			if (dq != &_dispatch_mgr_q) {
				return &_dispatch_mgr_q;
			}
			if (!_dispatch_mach_cancel(dm)) {
				return NULL;
			}
		}
		if (!dm->dm_cancel_handler_called) {
			if (dq != dm->do_targetq) {
				return dm->do_targetq;
			}
			_dispatch_mach_cancel_invoke(dm);
		}
	}
	return NULL;
}

DISPATCH_NOINLINE
void
_dispatch_mach_invoke(dispatch_mach_t dm)
{
	_dispatch_queue_class_invoke(dm, _dispatch_mach_invoke2);
}

unsigned long
_dispatch_mach_probe(dispatch_mach_t dm)
{
	// This function determines whether the mach channel needs to be invoked.
	// The order of tests here in probe and in invoke should be consistent.

	dispatch_mach_send_refs_t dr = dm->dm_refs;

	if (slowpath(!dm->ds_is_installed)) {
		// The channel needs to be installed on the manager queue.
		return true;
	} else if (_dispatch_queue_class_probe(dm)) {
		// The source has pending messages to deliver to the target queue.
		return true;
	} else if (dr->dm_sending) {
		// Sending and uninstallation below require the send lock, the channel
		// will be woken up when the lock is dropped <rdar://15132939&15203957>
		return false;
	} else if (dr->dm_tail &&
			(!(dm->dm_dkev && DISPATCH_MACH_KEVENT_ARMED(dm->dm_dkev)) ||
			(dm->ds_atomic_flags & DSF_CANCELED) || dr->dm_disconnect_cnt)) {
		// The channel has pending messages to send.
		return true;
	} else if (dm->ds_atomic_flags & DSF_CANCELED) {
		if (dm->ds_dkev || dm->dm_dkev || dr->dm_send ||
				!TAILQ_EMPTY(&dm->dm_refs->dm_replies) ||
				!dm->dm_cancel_handler_called) {
			// The channel needs to be uninstalled from the manager queue, or
			// the cancellation handler needs to be delivered to the target
			// queue.
			return true;
		}
	}
	// Nothing to do.
	return false;
}

#pragma mark -
#pragma mark dispatch_mach_msg_t

dispatch_mach_msg_t
dispatch_mach_msg_create(mach_msg_header_t *msg, size_t size,
		dispatch_mach_msg_destructor_t destructor, mach_msg_header_t **msg_ptr)
{
	if (slowpath(size < sizeof(mach_msg_header_t)) ||
			slowpath(destructor && !msg)) {
		DISPATCH_CLIENT_CRASH("Empty message");
	}
	dispatch_mach_msg_t dmsg = _dispatch_alloc(DISPATCH_VTABLE(mach_msg),
			sizeof(struct dispatch_mach_msg_s) +
			(destructor ? 0 : size - sizeof(dmsg->dmsg_msg)));
	if (destructor) {
		dmsg->dmsg_msg = msg;
	} else if (msg) {
		memcpy(dmsg->dmsg_buf, msg, size);
	}
	dmsg->do_next = DISPATCH_OBJECT_LISTLESS;
	dmsg->do_targetq = _dispatch_get_root_queue(_DISPATCH_QOS_CLASS_DEFAULT,
			false);
	dmsg->dmsg_destructor = destructor;
	dmsg->dmsg_size = size;
	if (msg_ptr) {
		*msg_ptr = _dispatch_mach_msg_get_msg(dmsg);
	}
	return dmsg;
}

void
_dispatch_mach_msg_dispose(dispatch_mach_msg_t dmsg)
{
	if (dmsg->dmsg_voucher) {
		_voucher_release(dmsg->dmsg_voucher);
		dmsg->dmsg_voucher = NULL;
	}
	switch (dmsg->dmsg_destructor) {
	case DISPATCH_MACH_MSG_DESTRUCTOR_DEFAULT:
		break;
	case DISPATCH_MACH_MSG_DESTRUCTOR_FREE:
		free(dmsg->dmsg_msg);
		break;
	case DISPATCH_MACH_MSG_DESTRUCTOR_VM_DEALLOCATE: {
		mach_vm_size_t vm_size = dmsg->dmsg_size;
		mach_vm_address_t vm_addr = (uintptr_t)dmsg->dmsg_msg;
		(void)dispatch_assume_zero(mach_vm_deallocate(mach_task_self(),
				vm_addr, vm_size));
		break;
	}}
}

static inline mach_msg_header_t*
_dispatch_mach_msg_get_msg(dispatch_mach_msg_t dmsg)
{
	return dmsg->dmsg_destructor ? dmsg->dmsg_msg :
		(mach_msg_header_t*)(uintptr_t)dmsg->dmsg_buf;
}

mach_msg_header_t*
dispatch_mach_msg_get_msg(dispatch_mach_msg_t dmsg, size_t *size_ptr)
{
	if (size_ptr) {
		*size_ptr = dmsg->dmsg_size;
	}
	return _dispatch_mach_msg_get_msg(dmsg);
}

size_t
_dispatch_mach_msg_debug(dispatch_mach_msg_t dmsg, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			dx_kind(dmsg), dmsg);
	offset += dsnprintf(&buf[offset], bufsiz - offset, "xrefcnt = 0x%x, "
			"refcnt = 0x%x, ", dmsg->do_xref_cnt + 1, dmsg->do_ref_cnt + 1);
	offset += dsnprintf(&buf[offset], bufsiz - offset, "opts/err = 0x%x, "
			"msgh[%p] = { ", dmsg->do_suspend_cnt, dmsg->dmsg_buf);
	mach_msg_header_t *hdr = _dispatch_mach_msg_get_msg(dmsg);
	if (hdr->msgh_id) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "id 0x%x, ",
				hdr->msgh_id);
	}
	if (hdr->msgh_size) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "size %u, ",
				hdr->msgh_size);
	}
	if (hdr->msgh_bits) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "bits <l %u, r %u",
				MACH_MSGH_BITS_LOCAL(hdr->msgh_bits),
				MACH_MSGH_BITS_REMOTE(hdr->msgh_bits));
		if (MACH_MSGH_BITS_OTHER(hdr->msgh_bits)) {
			offset += dsnprintf(&buf[offset], bufsiz - offset, ", o 0x%x",
					MACH_MSGH_BITS_OTHER(hdr->msgh_bits));
		}
		offset += dsnprintf(&buf[offset], bufsiz - offset, ">, ");
	}
	if (hdr->msgh_local_port && hdr->msgh_remote_port) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "local 0x%x, "
				"remote 0x%x", hdr->msgh_local_port, hdr->msgh_remote_port);
	} else if (hdr->msgh_local_port) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "local 0x%x",
				hdr->msgh_local_port);
	} else if (hdr->msgh_remote_port) {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "remote 0x%x",
				hdr->msgh_remote_port);
	} else {
		offset += dsnprintf(&buf[offset], bufsiz - offset, "no ports");
	}
	offset += dsnprintf(&buf[offset], bufsiz - offset, " } }");
	return offset;
}

#pragma mark -
#pragma mark dispatch_mig_server

mach_msg_return_t
dispatch_mig_server(dispatch_source_t ds, size_t maxmsgsz,
		dispatch_mig_callback_t callback)
{
	mach_msg_options_t options = MACH_RCV_MSG | MACH_RCV_TIMEOUT
		| MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_CTX)
		| MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) | MACH_RCV_VOUCHER;
	mach_msg_options_t tmp_options;
	mig_reply_error_t *bufTemp, *bufRequest, *bufReply;
	mach_msg_return_t kr = 0;
	uint64_t assertion_token = 0;
	unsigned int cnt = 1000; // do not stall out serial queues
	boolean_t demux_success;
	bool received = false;
	size_t rcv_size = maxmsgsz + MAX_TRAILER_SIZE;

	// XXX FIXME -- allocate these elsewhere
	bufRequest = alloca(rcv_size);
	bufReply = alloca(rcv_size);
	bufReply->Head.msgh_size = 0;
	bufRequest->RetCode = 0;

#if DISPATCH_DEBUG
	options |= MACH_RCV_LARGE; // rdar://problem/8422992
#endif
	tmp_options = options;
	// XXX FIXME -- change this to not starve out the target queue
	for (;;) {
		if (DISPATCH_OBJECT_SUSPENDED(ds) || (--cnt == 0)) {
			options &= ~MACH_RCV_MSG;
			tmp_options &= ~MACH_RCV_MSG;

			if (!(tmp_options & MACH_SEND_MSG)) {
				goto out;
			}
		}
		kr = mach_msg(&bufReply->Head, tmp_options, bufReply->Head.msgh_size,
				(mach_msg_size_t)rcv_size, (mach_port_t)ds->ds_ident_hack, 0,0);

		tmp_options = options;

		if (slowpath(kr)) {
			switch (kr) {
			case MACH_SEND_INVALID_DEST:
			case MACH_SEND_TIMED_OUT:
				if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) {
					mach_msg_destroy(&bufReply->Head);
				}
				break;
			case MACH_RCV_TIMED_OUT:
				// Don't return an error if a message was sent this time or
				// a message was successfully received previously
				// rdar://problems/7363620&7791738
				if(bufReply->Head.msgh_remote_port || received) {
					kr = MACH_MSG_SUCCESS;
				}
				break;
			case MACH_RCV_INVALID_NAME:
				break;
#if DISPATCH_DEBUG
			case MACH_RCV_TOO_LARGE:
				// receive messages that are too large and log their id and size
				// rdar://problem/8422992
				tmp_options &= ~MACH_RCV_LARGE;
				size_t large_size = bufReply->Head.msgh_size + MAX_TRAILER_SIZE;
				void *large_buf = malloc(large_size);
				if (large_buf) {
					rcv_size = large_size;
					bufReply = large_buf;
				}
				if (!mach_msg(&bufReply->Head, tmp_options, 0,
						(mach_msg_size_t)rcv_size,
						(mach_port_t)ds->ds_ident_hack, 0, 0)) {
					_dispatch_log("BUG in libdispatch client: "
							"dispatch_mig_server received message larger than "
							"requested size %zd: id = 0x%x, size = %d",
							maxmsgsz, bufReply->Head.msgh_id,
							bufReply->Head.msgh_size);
				}
				if (large_buf) {
					free(large_buf);
				}
				// fall through
#endif
			default:
				_dispatch_bug_mach_client(
						"dispatch_mig_server: mach_msg() failed", kr);
				break;
			}
			goto out;
		}

		if (!(tmp_options & MACH_RCV_MSG)) {
			goto out;
		}

		if (assertion_token) {
#if DISPATCH_USE_IMPORTANCE_ASSERTION
			int r = proc_importance_assertion_complete(assertion_token);
			(void)dispatch_assume_zero(r);
#endif
			assertion_token = 0;
		}
		received = true;

		bufTemp = bufRequest;
		bufRequest = bufReply;
		bufReply = bufTemp;

#if DISPATCH_USE_IMPORTANCE_ASSERTION
		int r = proc_importance_assertion_begin_with_msg(&bufRequest->Head,
				NULL, &assertion_token);
		if (r && slowpath(r != EIO)) {
			(void)dispatch_assume_zero(r);
		}
#endif
		_voucher_replace(voucher_create_with_mach_msg(&bufRequest->Head));
		demux_success = callback(&bufRequest->Head, &bufReply->Head);

		if (!demux_success) {
			// destroy the request - but not the reply port
			bufRequest->Head.msgh_remote_port = 0;
			mach_msg_destroy(&bufRequest->Head);
		} else if (!(bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
			// if MACH_MSGH_BITS_COMPLEX is _not_ set, then bufReply->RetCode
			// is present
			if (slowpath(bufReply->RetCode)) {
				if (bufReply->RetCode == MIG_NO_REPLY) {
					continue;
				}

				// destroy the request - but not the reply port
				bufRequest->Head.msgh_remote_port = 0;
				mach_msg_destroy(&bufRequest->Head);
			}
		}

		if (bufReply->Head.msgh_remote_port) {
			tmp_options |= MACH_SEND_MSG;
			if (MACH_MSGH_BITS_REMOTE(bufReply->Head.msgh_bits) !=
					MACH_MSG_TYPE_MOVE_SEND_ONCE) {
				tmp_options |= MACH_SEND_TIMEOUT;
			}
		}
	}

out:
	if (assertion_token) {
#if DISPATCH_USE_IMPORTANCE_ASSERTION
		int r = proc_importance_assertion_complete(assertion_token);
		(void)dispatch_assume_zero(r);
#endif
	}

	return kr;
}

#endif /* HAVE_MACH */

#pragma mark -
#pragma mark dispatch_source_debug

DISPATCH_NOINLINE
static const char *
_evfiltstr(short filt)
{
	switch (filt) {
#define _evfilt2(f) case (f): return #f
	_evfilt2(EVFILT_READ);
	_evfilt2(EVFILT_WRITE);
	_evfilt2(EVFILT_AIO);
	_evfilt2(EVFILT_VNODE);
	_evfilt2(EVFILT_PROC);
	_evfilt2(EVFILT_SIGNAL);
	_evfilt2(EVFILT_TIMER);
#ifdef EVFILT_VM
	_evfilt2(EVFILT_VM);
#endif
#ifdef EVFILT_MEMORYSTATUS
	_evfilt2(EVFILT_MEMORYSTATUS);
#endif
#if HAVE_MACH
	_evfilt2(EVFILT_MACHPORT);
	_evfilt2(DISPATCH_EVFILT_MACH_NOTIFICATION);
#endif
	_evfilt2(EVFILT_FS);
	_evfilt2(EVFILT_USER);

	_evfilt2(DISPATCH_EVFILT_TIMER);
	_evfilt2(DISPATCH_EVFILT_CUSTOM_ADD);
	_evfilt2(DISPATCH_EVFILT_CUSTOM_OR);
	default:
		return "EVFILT_missing";
	}
}

static size_t
_dispatch_source_debug_attr(dispatch_source_t ds, char* buf, size_t bufsiz)
{
	dispatch_queue_t target = ds->do_targetq;
	return dsnprintf(buf, bufsiz, "target = %s[%p], ident = 0x%lx, "
			"pending_data = 0x%lx, pending_data_mask = 0x%lx, ",
			target && target->dq_label ? target->dq_label : "", target,
			ds->ds_ident_hack, ds->ds_pending_data, ds->ds_pending_data_mask);
}

static size_t
_dispatch_timer_debug_attr(dispatch_source_t ds, char* buf, size_t bufsiz)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	return dsnprintf(buf, bufsiz, "timer = { target = 0x%zx, deadline = 0x%zx,"
			" last_fire = 0x%zx, interval = 0x%zx, flags = 0x%lx }, ",
			ds_timer(dr).target, ds_timer(dr).deadline, ds_timer(dr).last_fire,
			ds_timer(dr).interval, ds_timer(dr).flags);
}

size_t
_dispatch_source_debug(dispatch_source_t ds, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			dx_kind(ds), ds);
	offset += _dispatch_object_debug_attr(ds, &buf[offset], bufsiz - offset);
	offset += _dispatch_source_debug_attr(ds, &buf[offset], bufsiz - offset);
	if (ds->ds_is_timer) {
		offset += _dispatch_timer_debug_attr(ds, &buf[offset], bufsiz - offset);
	}
	offset += dsnprintf(&buf[offset], bufsiz - offset, "filter = %s }",
			ds->ds_dkev ? _evfiltstr(ds->ds_dkev->dk_kevent.filter) : "????");
	return offset;
}

static size_t
_dispatch_mach_debug_attr(dispatch_mach_t dm, char* buf, size_t bufsiz)
{
	dispatch_queue_t target = dm->do_targetq;
	return dsnprintf(buf, bufsiz, "target = %s[%p], receive = 0x%x, "
			"send = 0x%x, send-possible = 0x%x%s, checkin = 0x%x%s, "
			"sending = %d, disconnected = %d, canceled = %d ",
			target && target->dq_label ? target->dq_label : "", target,
			dm->ds_dkev ?(mach_port_t)dm->ds_dkev->dk_kevent.ident:0,
			dm->dm_refs->dm_send,
			dm->dm_dkev ?(mach_port_t)dm->dm_dkev->dk_kevent.ident:0,
			dm->dm_dkev && DISPATCH_MACH_KEVENT_ARMED(dm->dm_dkev) ?
			" (armed)" : "", dm->dm_refs->dm_checkin_port,
			dm->dm_refs->dm_checkin ? " (pending)" : "",
			dm->dm_refs->dm_sending, dm->dm_refs->dm_disconnect_cnt,
			(bool)(dm->ds_atomic_flags & DSF_CANCELED));
}
size_t
_dispatch_mach_debug(dispatch_mach_t dm, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			dm->dq_label && !dm->dm_cancel_handler_called ? dm->dq_label :
			dx_kind(dm), dm);
	offset += _dispatch_object_debug_attr(dm, &buf[offset], bufsiz - offset);
	offset += _dispatch_mach_debug_attr(dm, &buf[offset], bufsiz - offset);
	offset += dsnprintf(&buf[offset], bufsiz - offset, "}");
	return offset;
}

#if DISPATCH_DEBUG
static void
_dispatch_kevent_debug(struct kevent64_s* kev, const char* str)
{
	_dispatch_log("kevent[%p] = { ident = 0x%llx, filter = %s, flags = 0x%x, "
			"fflags = 0x%x, data = 0x%llx, udata = 0x%llx, ext[0] = 0x%llx, "
			"ext[1] = 0x%llx }: %s", kev, kev->ident, _evfiltstr(kev->filter),
			kev->flags, kev->fflags, kev->data, kev->udata, kev->ext[0],
			kev->ext[1], str);
}

static void
_dispatch_kevent_debugger2(void *context)
{
	struct sockaddr sa;
	socklen_t sa_len = sizeof(sa);
	int c, fd = (int)(long)context;
	unsigned int i;
	dispatch_kevent_t dk;
	dispatch_source_t ds;
	dispatch_source_refs_t dr;
	FILE *debug_stream;

	c = accept(fd, &sa, &sa_len);
	if (c == -1) {
		if (errno != EAGAIN) {
			(void)dispatch_assume_zero(errno);
		}
		return;
	}
#if 0
	int r = fcntl(c, F_SETFL, 0); // disable non-blocking IO
	if (r == -1) {
		(void)dispatch_assume_zero(errno);
	}
#endif
	debug_stream = fdopen(c, "a");
	if (!dispatch_assume(debug_stream)) {
		close(c);
		return;
	}

	fprintf(debug_stream, "HTTP/1.0 200 OK\r\n");
	fprintf(debug_stream, "Content-type: text/html\r\n");
	fprintf(debug_stream, "Pragma: nocache\r\n");
	fprintf(debug_stream, "\r\n");
	fprintf(debug_stream, "<html>\n");
	fprintf(debug_stream, "<head><title>PID %u</title></head>\n", getpid());
	fprintf(debug_stream, "<body>\n<ul>\n");

	//fprintf(debug_stream, "<tr><td>DK</td><td>DK</td><td>DK</td><td>DK</td>"
	//		"<td>DK</td><td>DK</td><td>DK</td></tr>\n");

	for (i = 0; i < DSL_HASH_SIZE; i++) {
		if (TAILQ_EMPTY(&_dispatch_sources[i])) {
			continue;
		}
		TAILQ_FOREACH(dk, &_dispatch_sources[i], dk_list) {
			fprintf(debug_stream, "\t<br><li>DK %p ident %lu filter %s flags "
					"0x%hx fflags 0x%x data 0x%lx udata %p\n",
					dk, (unsigned long)dk->dk_kevent.ident,
					_evfiltstr(dk->dk_kevent.filter), dk->dk_kevent.flags,
					dk->dk_kevent.fflags, (unsigned long)dk->dk_kevent.data,
					(void*)dk->dk_kevent.udata);
			fprintf(debug_stream, "\t\t<ul>\n");
			TAILQ_FOREACH(dr, &dk->dk_sources, dr_list) {
				ds = _dispatch_source_from_refs(dr);
				fprintf(debug_stream, "\t\t\t<li>DS %p refcnt 0x%x suspend "
						"0x%x data 0x%lx mask 0x%lx flags 0x%x</li>\n",
						ds, ds->do_ref_cnt + 1, ds->do_suspend_cnt,
						ds->ds_pending_data, ds->ds_pending_data_mask,
						ds->ds_atomic_flags);
				if (ds->do_suspend_cnt == DISPATCH_OBJECT_SUSPEND_LOCK) {
					dispatch_queue_t dq = ds->do_targetq;
					fprintf(debug_stream, "\t\t<br>DQ: %p refcnt 0x%x suspend "
							"0x%x label: %s\n", dq, dq->do_ref_cnt + 1,
							dq->do_suspend_cnt, dq->dq_label ? dq->dq_label:"");
				}
			}
			fprintf(debug_stream, "\t\t</ul>\n");
			fprintf(debug_stream, "\t</li>\n");
		}
	}
	fprintf(debug_stream, "</ul>\n</body>\n</html>\n");
	fflush(debug_stream);
	fclose(debug_stream);
}

static void
_dispatch_kevent_debugger2_cancel(void *context)
{
	int ret, fd = (int)(long)context;

	ret = close(fd);
	if (ret != -1) {
		(void)dispatch_assume_zero(errno);
	}
}

static void
_dispatch_kevent_debugger(void *context DISPATCH_UNUSED)
{
	union {
		struct sockaddr_in sa_in;
		struct sockaddr sa;
	} sa_u = {
		.sa_in = {
			.sin_family = AF_INET,
			.sin_addr = { htonl(INADDR_LOOPBACK), },
		},
	};
	dispatch_source_t ds;
	const char *valstr;
	int val, r, fd, sock_opt = 1;
	socklen_t slen = sizeof(sa_u);

	if (issetugid()) {
		return;
	}
	valstr = getenv("LIBDISPATCH_DEBUGGER");
	if (!valstr) {
		return;
	}
	val = atoi(valstr);
	if (val == 2) {
		sa_u.sa_in.sin_addr.s_addr = 0;
	}
	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		(void)dispatch_assume_zero(errno);
		return;
	}
	r = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&sock_opt,
			(socklen_t) sizeof sock_opt);
	if (r == -1) {
		(void)dispatch_assume_zero(errno);
		goto out_bad;
	}
#if 0
	r = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (r == -1) {
		(void)dispatch_assume_zero(errno);
		goto out_bad;
	}
#endif
	r = bind(fd, &sa_u.sa, sizeof(sa_u));
	if (r == -1) {
		(void)dispatch_assume_zero(errno);
		goto out_bad;
	}
	r = listen(fd, SOMAXCONN);
	if (r == -1) {
		(void)dispatch_assume_zero(errno);
		goto out_bad;
	}
	r = getsockname(fd, &sa_u.sa, &slen);
	if (r == -1) {
		(void)dispatch_assume_zero(errno);
		goto out_bad;
	}

	ds = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)fd, 0,
			&_dispatch_mgr_q);
	if (dispatch_assume(ds)) {
		_dispatch_log("LIBDISPATCH: debug port: %hu",
				(in_port_t)ntohs(sa_u.sa_in.sin_port));

		/* ownership of fd transfers to ds */
		dispatch_set_context(ds, (void *)(long)fd);
		dispatch_source_set_event_handler_f(ds, _dispatch_kevent_debugger2);
		dispatch_source_set_cancel_handler_f(ds,
				_dispatch_kevent_debugger2_cancel);
		dispatch_resume(ds);

		return;
	}
out_bad:
	close(fd);
}

#if HAVE_MACH

#ifndef MACH_PORT_TYPE_SPREQUEST
#define MACH_PORT_TYPE_SPREQUEST 0x40000000
#endif

DISPATCH_NOINLINE
void
dispatch_debug_machport(mach_port_t name, const char* str)
{
	mach_port_type_t type;
	mach_msg_bits_t ns = 0, nr = 0, nso = 0, nd = 0;
	unsigned int dnreqs = 0, dnrsiz;
	kern_return_t kr = mach_port_type(mach_task_self(), name, &type);
	if (kr) {
		_dispatch_log("machport[0x%08x] = { error(0x%x) \"%s\" }: %s", name,
				kr, mach_error_string(kr), str);
		return;
	}
	if (type & MACH_PORT_TYPE_SEND) {
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_SEND, &ns));
	}
	if (type & MACH_PORT_TYPE_SEND_ONCE) {
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_SEND_ONCE, &nso));
	}
	if (type & MACH_PORT_TYPE_DEAD_NAME) {
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_DEAD_NAME, &nd));
	}
	if (type & (MACH_PORT_TYPE_RECEIVE|MACH_PORT_TYPE_SEND)) {
		kr = mach_port_dnrequest_info(mach_task_self(), name, &dnrsiz, &dnreqs);
		if (kr != KERN_INVALID_RIGHT) (void)dispatch_assume_zero(kr);
	}
	if (type & MACH_PORT_TYPE_RECEIVE) {
		mach_port_status_t status = { .mps_pset = 0, };
		mach_msg_type_number_t cnt = MACH_PORT_RECEIVE_STATUS_COUNT;
		(void)dispatch_assume_zero(mach_port_get_refs(mach_task_self(), name,
				MACH_PORT_RIGHT_RECEIVE, &nr));
		(void)dispatch_assume_zero(mach_port_get_attributes(mach_task_self(),
				name, MACH_PORT_RECEIVE_STATUS, (void*)&status, &cnt));
		_dispatch_log("machport[0x%08x] = { R(%03u) S(%03u) SO(%03u) D(%03u) "
				"dnreqs(%03u) spreq(%s) nsreq(%s) pdreq(%s) srights(%s) "
				"sorights(%03u) qlim(%03u) msgcount(%03u) mkscount(%03u) "
				"seqno(%03u) }: %s", name, nr, ns, nso, nd, dnreqs,
				type & MACH_PORT_TYPE_SPREQUEST ? "Y":"N",
				status.mps_nsrequest ? "Y":"N", status.mps_pdrequest ? "Y":"N",
				status.mps_srights ? "Y":"N", status.mps_sorights,
				status.mps_qlimit, status.mps_msgcount, status.mps_mscount,
				status.mps_seqno, str);
	} else if (type & (MACH_PORT_TYPE_SEND|MACH_PORT_TYPE_SEND_ONCE|
			MACH_PORT_TYPE_DEAD_NAME)) {
		_dispatch_log("machport[0x%08x] = { R(%03u) S(%03u) SO(%03u) D(%03u) "
				"dnreqs(%03u) spreq(%s) }: %s", name, nr, ns, nso, nd, dnreqs,
				type & MACH_PORT_TYPE_SPREQUEST ? "Y":"N", str);
	} else {
		_dispatch_log("machport[0x%08x] = { type(0x%08x) }: %s", name, type,
				str);
	}
}

#endif // HAVE_MACH

#endif // DISPATCH_DEBUG
