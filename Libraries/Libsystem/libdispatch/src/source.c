/*
 * Copyright (c) 2008-2016 Apple Inc. All rights reserved.
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

static void _dispatch_source_handler_free(dispatch_source_refs_t ds, long kind);

#pragma mark -
#pragma mark dispatch_source_t

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_source_get_handler(dispatch_source_refs_t dr, long kind)
{
	return os_atomic_load(&dr->ds_handler[kind], relaxed);
}
#define _dispatch_source_get_event_handler(dr) \
		_dispatch_source_get_handler(dr, DS_EVENT_HANDLER)
#define _dispatch_source_get_cancel_handler(dr) \
		_dispatch_source_get_handler(dr, DS_CANCEL_HANDLER)
#define _dispatch_source_get_registration_handler(dr) \
		_dispatch_source_get_handler(dr, DS_REGISTN_HANDLER)

dispatch_source_t
dispatch_source_create(dispatch_source_type_t dst, uintptr_t handle,
		unsigned long mask, dispatch_queue_t dq)
{
	dispatch_source_refs_t dr;
	dispatch_source_t ds;

	dr = dux_create(dst, handle, mask)._dr;
	if (unlikely(!dr)) {
		return DISPATCH_BAD_INPUT;
	}

	ds = _dispatch_queue_alloc(source,
			dux_type(dr)->dst_strict ? DSF_STRICT : DQF_MUTABLE, 1,
			DISPATCH_QUEUE_INACTIVE | DISPATCH_QUEUE_ROLE_INNER)._ds;
	ds->dq_label = "source";
	ds->ds_refs = dr;
	dr->du_owner_wref = _dispatch_ptr2wref(ds);

	if (unlikely(!dq)) {
		dq = _dispatch_get_default_queue(true);
	} else {
		_dispatch_retain((dispatch_queue_t _Nonnull)dq);
	}
	ds->do_targetq = dq;
	if (dr->du_is_timer && (dr->du_timer_flags & DISPATCH_TIMER_INTERVAL)) {
		dispatch_source_set_timer(ds, DISPATCH_TIME_NOW, handle, UINT64_MAX);
	}
	_dispatch_object_debug(ds, "%s", __func__);
	return ds;
}

void
_dispatch_source_dispose(dispatch_source_t ds, bool *allow_free)
{
	_dispatch_object_debug(ds, "%s", __func__);

	_dispatch_trace_source_dispose(ds);
	_dispatch_source_handler_free(ds->ds_refs, DS_REGISTN_HANDLER);
	_dispatch_source_handler_free(ds->ds_refs, DS_EVENT_HANDLER);
	_dispatch_source_handler_free(ds->ds_refs, DS_CANCEL_HANDLER);
	_dispatch_unote_dispose(ds->ds_refs);
	ds->ds_refs = NULL;
	_dispatch_lane_class_dispose(ds, allow_free);
}

void
_dispatch_source_xref_dispose(dispatch_source_t ds)
{
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(ds);
	if (unlikely((dqf & DSF_STRICT) && !(dqf & DSF_CANCELED) &&
			_dispatch_source_get_cancel_handler(ds->ds_refs))) {
		DISPATCH_CLIENT_CRASH(ds, "Release of a source that has not been "
				"cancelled, but has a mandatory cancel handler");
	}
	dx_wakeup(ds, 0, DISPATCH_WAKEUP_MAKE_DIRTY);
}

long
dispatch_source_testcancel(dispatch_source_t ds)
{
	return (bool)(ds->dq_atomic_flags & DSF_CANCELED);
}

unsigned long
dispatch_source_get_mask(dispatch_source_t ds)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	if (ds->dq_atomic_flags & DSF_CANCELED) {
		return 0;
	}
#if DISPATCH_USE_MEMORYSTATUS
	if (dr->du_vmpressure_override) {
		return NOTE_VM_PRESSURE;
	}
#if TARGET_OS_SIMULATOR
	if (dr->du_memorypressure_override) {
		return NOTE_MEMORYSTATUS_PRESSURE_WARN;
	}
#endif
#endif // DISPATCH_USE_MEMORYSTATUS
	if (dr->du_is_timer) {
		return dr->du_timer_flags;
	}
	return dr->du_fflags;
}

uintptr_t
dispatch_source_get_handle(dispatch_source_t ds)
{
	dispatch_source_refs_t dr = ds->ds_refs;
#if TARGET_OS_SIMULATOR
	if (dr->du_memorypressure_override) {
		return 0;
	}
#endif
	if (dr->du_filter == DISPATCH_EVFILT_TIMER_WITH_CLOCK) {
		switch (_dispatch_timer_flags_to_clock(dr->du_timer_flags)) {
		case DISPATCH_CLOCK_UPTIME: return DISPATCH_CLOCKID_UPTIME;
		case DISPATCH_CLOCK_MONOTONIC: return DISPATCH_CLOCKID_MONOTONIC;
		case DISPATCH_CLOCK_WALL: return DISPATCH_CLOCKID_WALLTIME;
		}
	}
	return dr->du_ident;
}

unsigned long
dispatch_source_get_data(dispatch_source_t ds)
{
#if DISPATCH_USE_MEMORYSTATUS
	dispatch_source_refs_t dr = ds->ds_refs;
	if (dr->du_vmpressure_override) {
		return NOTE_VM_PRESSURE;
	}
#if TARGET_OS_SIMULATOR
	if (dr->du_memorypressure_override) {
		return NOTE_MEMORYSTATUS_PRESSURE_WARN;
	}
#endif
#endif // DISPATCH_USE_MEMORYSTATUS
	uint64_t value = os_atomic_load2o(dr, ds_data, relaxed);
	return (unsigned long)(dr->du_has_extended_status ?
			DISPATCH_SOURCE_GET_DATA(value) : value);
}

size_t
dispatch_source_get_extended_data(dispatch_source_t ds,
		dispatch_source_extended_data_t edata, size_t size)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	size_t target_size = MIN(size,
		sizeof(struct dispatch_source_extended_data_s));
	if (size > 0) {
		unsigned long data, status = 0;
		if (dr->du_has_extended_status) {
			uint64_t combined = os_atomic_load(&dr->ds_data, relaxed);
			data = DISPATCH_SOURCE_GET_DATA(combined);
			status = DISPATCH_SOURCE_GET_STATUS(combined);
		} else {
			data = dispatch_source_get_data(ds);
		}
		if (size >= offsetof(struct dispatch_source_extended_data_s, data)
				+ sizeof(edata->data)) {
			edata->data = data;
		}
		if (size >= offsetof(struct dispatch_source_extended_data_s, status)
				+ sizeof(edata->status)) {
			edata->status = status;
		}
		if (size > sizeof(struct dispatch_source_extended_data_s)) {
			memset(
				(char *)edata + sizeof(struct dispatch_source_extended_data_s),
				0, size - sizeof(struct dispatch_source_extended_data_s));
		}
	}
	return target_size;
}

void
dispatch_source_merge_data(dispatch_source_t ds, unsigned long val)
{
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(ds);
	dispatch_source_refs_t dr = ds->ds_refs;

	if (unlikely(dqf & (DSF_CANCELED | DQF_RELEASED))) {
		return;
	}

	switch (dr->du_filter) {
	case DISPATCH_EVFILT_CUSTOM_ADD:
		os_atomic_add2o(dr, ds_pending_data, val, relaxed);
		break;
	case DISPATCH_EVFILT_CUSTOM_OR:
		os_atomic_or2o(dr, ds_pending_data, val, relaxed);
		break;
	case DISPATCH_EVFILT_CUSTOM_REPLACE:
		os_atomic_store2o(dr, ds_pending_data, val, relaxed);
		break;
	default:
		DISPATCH_CLIENT_CRASH(dr->du_filter, "Invalid source type");
	}

	dx_wakeup(ds, 0, DISPATCH_WAKEUP_MAKE_DIRTY);
}

#pragma mark -
#pragma mark dispatch_source_handler

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_source_handler_alloc(dispatch_source_t ds, void *func, uintptr_t kind,
		bool is_block)
{
	// sources don't propagate priority by default
	const dispatch_block_flags_t flags =
			DISPATCH_BLOCK_HAS_PRIORITY | DISPATCH_BLOCK_NO_VOUCHER;
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	if (func) {
		uintptr_t dc_flags = 0;

		if (kind != DS_EVENT_HANDLER) {
			dc_flags |= DC_FLAG_CONSUME;
		}
		if (is_block) {
#ifdef __BLOCKS__
			_dispatch_continuation_init(dc, ds, func, flags, dc_flags);
#endif /* __BLOCKS__ */
		} else {
			dc_flags |= DC_FLAG_FETCH_CONTEXT;
			_dispatch_continuation_init_f(dc, ds, ds->do_ctxt, func, flags,
					dc_flags);
		}
	} else {
		dc->dc_flags = DC_FLAG_ALLOCATED;
		dc->dc_func = NULL;
	}
	return dc;
}

DISPATCH_NOINLINE
static void
_dispatch_source_handler_dispose(dispatch_continuation_t dc)
{
#ifdef __BLOCKS__
	if (dc->dc_flags & DC_FLAG_BLOCK) {
		Block_release(dc->dc_ctxt);
	}
#endif /* __BLOCKS__ */
	if (dc->dc_voucher) {
		_voucher_release(dc->dc_voucher);
		dc->dc_voucher = VOUCHER_INVALID;
	}
	_dispatch_continuation_free(dc);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_source_handler_take(dispatch_source_refs_t dr, long kind)
{
	return os_atomic_xchg(&dr->ds_handler[kind], NULL, relaxed);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_source_handler_free(dispatch_source_refs_t dr, long kind)
{
	dispatch_continuation_t dc = _dispatch_source_handler_take(dr, kind);
	if (dc) _dispatch_source_handler_dispose(dc);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_source_handler_replace(dispatch_source_t ds, uintptr_t kind,
		dispatch_continuation_t dc)
{
	if (!dc->dc_func) {
		_dispatch_continuation_free(dc);
		dc = NULL;
	} else if (dc->dc_flags & DC_FLAG_FETCH_CONTEXT) {
		dc->dc_ctxt = ds->do_ctxt;
	}
	dc = os_atomic_xchg(&ds->ds_refs->ds_handler[kind], dc, release);
	if (dc) _dispatch_source_handler_dispose(dc);
}

DISPATCH_NOINLINE
static void
_dispatch_source_set_handler_slow(void *context)
{
	dispatch_source_t ds = upcast(_dispatch_queue_get_current())._ds;
	dispatch_assert(dx_type(ds) == DISPATCH_SOURCE_KEVENT_TYPE);

	dispatch_continuation_t dc = context;
	uintptr_t kind = (uintptr_t)dc->dc_data;
	dc->dc_data = NULL;
	_dispatch_source_handler_replace(ds, kind, dc);
}

DISPATCH_NOINLINE
static void
_dispatch_source_set_handler(dispatch_source_t ds, void *func,
		uintptr_t kind, bool is_block)
{
	dispatch_continuation_t dc;

	dc = _dispatch_source_handler_alloc(ds, func, kind, is_block);

	if (_dispatch_lane_try_inactive_suspend(ds)) {
		_dispatch_source_handler_replace(ds, kind, dc);
		return _dispatch_lane_resume(ds, DISPATCH_RESUME);
	}

	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(ds);
	if (unlikely(dqf & DSF_STRICT)) {
		DISPATCH_CLIENT_CRASH(kind, "Cannot change a handler of this source "
				"after it has been activated");
	}
	// Ignore handlers mutations past cancelation, it's harmless
	if ((dqf & DSF_CANCELED) == 0) {
		_dispatch_ktrace1(DISPATCH_PERF_post_activate_mutation, ds);
		if (kind == DS_REGISTN_HANDLER) {
			_dispatch_bug_deprecated("Setting registration handler after "
					"the source has been activated");
		} else if (func == NULL) {
			_dispatch_bug_deprecated("Clearing handler after "
					"the source has been activated");
		}
	}
	dc->dc_data = (void *)kind;
	_dispatch_barrier_trysync_or_async_f(ds, dc,
			_dispatch_source_set_handler_slow, 0);
}

#ifdef __BLOCKS__
void
dispatch_source_set_event_handler(dispatch_source_t ds,
		dispatch_block_t handler)
{
	_dispatch_source_set_handler(ds, handler, DS_EVENT_HANDLER, true);
}
#endif /* __BLOCKS__ */

void
dispatch_source_set_event_handler_f(dispatch_source_t ds,
		dispatch_function_t handler)
{
	_dispatch_source_set_handler(ds, handler, DS_EVENT_HANDLER, false);
}

#ifdef __BLOCKS__
void
dispatch_source_set_cancel_handler(dispatch_source_t ds,
		dispatch_block_t handler)
{
	_dispatch_source_set_handler(ds, handler, DS_CANCEL_HANDLER, true);
}

void
dispatch_source_set_mandatory_cancel_handler(dispatch_source_t ds,
		dispatch_block_t handler)
{
	_dispatch_queue_atomic_flags_set_and_clear(ds, DSF_STRICT, DQF_MUTABLE);
	dispatch_source_set_cancel_handler(ds, handler);
}
#endif /* __BLOCKS__ */

void
dispatch_source_set_cancel_handler_f(dispatch_source_t ds,
		dispatch_function_t handler)
{
	_dispatch_source_set_handler(ds, handler, DS_CANCEL_HANDLER, false);
}

void
dispatch_source_set_mandatory_cancel_handler_f(dispatch_source_t ds,
		dispatch_function_t handler)
{
	_dispatch_queue_atomic_flags_set_and_clear(ds, DSF_STRICT, DQF_MUTABLE);
	dispatch_source_set_cancel_handler_f(ds, handler);
}

#ifdef __BLOCKS__
void
dispatch_source_set_registration_handler(dispatch_source_t ds,
		dispatch_block_t handler)
{
	_dispatch_source_set_handler(ds, handler, DS_REGISTN_HANDLER, true);
}
#endif /* __BLOCKS__ */

void
dispatch_source_set_registration_handler_f(dispatch_source_t ds,
	dispatch_function_t handler)
{
	_dispatch_source_set_handler(ds, handler, DS_REGISTN_HANDLER, false);
}

#pragma mark -
#pragma mark dispatch_source_invoke

bool
_dispatch_source_will_reenable_kevent_4NW(dispatch_source_t ds)
{
	uint64_t dq_state = os_atomic_load2o(ds, dq_state, relaxed);

	if (unlikely(!_dq_state_drain_locked_by_self(dq_state))) {
		DISPATCH_CLIENT_CRASH(0, "_dispatch_source_will_reenable_kevent_4NW "
				"not called from within the event handler");
	}
	return _dispatch_unote_needs_rearm(ds->ds_refs);
}

static void
_dispatch_source_registration_callout(dispatch_source_t ds, dispatch_queue_t cq,
		dispatch_invoke_flags_t flags)
{
	dispatch_continuation_t dc;

	dc = _dispatch_source_handler_take(ds->ds_refs, DS_REGISTN_HANDLER);
	if (ds->dq_atomic_flags & (DSF_CANCELED | DQF_RELEASED)) {
		// no registration callout if source is canceled rdar://problem/8955246
		return _dispatch_source_handler_dispose(dc);
	}
	if (dc->dc_flags & DC_FLAG_FETCH_CONTEXT) {
		dc->dc_ctxt = ds->do_ctxt;
	}

	_dispatch_trace_source_callout_entry(ds, DS_REGISTN_HANDLER, cq, dc);
	_dispatch_continuation_pop(dc, NULL, flags, cq);
}

static void
_dispatch_source_cancel_callout(dispatch_source_t ds, dispatch_queue_t cq,
		dispatch_invoke_flags_t flags)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_continuation_t dc;

	dc = _dispatch_source_handler_take(dr, DS_CANCEL_HANDLER);
	dr->ds_pending_data = 0;
	dr->ds_data = 0;
	_dispatch_source_handler_free(dr, DS_EVENT_HANDLER);
	_dispatch_source_handler_free(dr, DS_REGISTN_HANDLER);
	if (!dc) {
		return;
	}
	if (!(ds->dq_atomic_flags & DSF_CANCELED)) {
		return _dispatch_source_handler_dispose(dc);
	}
	if (dc->dc_flags & DC_FLAG_FETCH_CONTEXT) {
		dc->dc_ctxt = ds->do_ctxt;
	}
	_dispatch_trace_source_callout_entry(ds, DS_CANCEL_HANDLER, cq, dc);
	_dispatch_continuation_pop(dc, NULL, flags, cq);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_source_refs_needs_configuration(dispatch_unote_t du)
{
	return du._du->du_is_timer &&
			os_atomic_load2o(du._dt, dt_pending_config, relaxed);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_source_refs_needs_rearm(dispatch_unote_t du)
{
	if (!du._du->du_is_timer) {
		return _dispatch_unote_needs_rearm(du);
	}
	if (os_atomic_load2o(du._dt, dt_pending_config, relaxed)) {
		return true;
	}
	if (_dispatch_unote_needs_rearm(du)) {
		return du._dt->dt_timer.target < INT64_MAX;
	}
	return false;
}

DISPATCH_ALWAYS_INLINE
static inline unsigned long
_dispatch_source_timer_data(dispatch_timer_source_refs_t dr, uint64_t prev)
{
	unsigned long data = (unsigned long)prev >> 1;

	// The timer may be in _dispatch_source_invoke2() already for other
	// reasons such as running the registration handler when ds_pending_data
	// is changed by _dispatch_timers_run2() without holding the drain lock.
	//
	// We hence need dependency ordering to pair with the release barrier
	// done by _dispatch_timers_run2() when setting the DISARMED_MARKER bit.
	os_atomic_thread_fence(dependency);
	dr = os_atomic_force_dependency_on(dr, data);

	if (dr->dt_timer.target < INT64_MAX) {
		uint64_t now = _dispatch_time_now(DISPATCH_TIMER_CLOCK(dr->du_ident));
		if (now >= dr->dt_timer.target) {
			data = _dispatch_timer_unote_compute_missed(dr, now, data);
		}
	}

	return data;
}

static void
_dispatch_source_latch_and_call(dispatch_source_t ds, dispatch_queue_t cq,
		dispatch_invoke_flags_t flags)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_continuation_t dc = _dispatch_source_get_handler(dr, DS_EVENT_HANDLER);
	uint64_t prev = os_atomic_xchg2o(dr, ds_pending_data, 0, relaxed);

	if (dr->du_is_timer && (dr->du_timer_flags & DISPATCH_TIMER_AFTER)) {
		_dispatch_trace_item_pop(cq, dc); // see _dispatch_after
	}
	switch (dux_type(dr)->dst_action) {
	case DISPATCH_UNOTE_ACTION_SOURCE_TIMER:
		if (prev & DISPATCH_TIMER_DISARMED_MARKER) {
			dr->ds_data = _dispatch_source_timer_data(ds->ds_timer_refs, prev);
		} else {
			dr->ds_data = prev >> 1;
		}
		break;
	case DISPATCH_UNOTE_ACTION_SOURCE_SET_DATA:
		dr->ds_data = ~prev;
		break;
	default:
		if (prev == 0 && dr->du_filter == DISPATCH_EVFILT_CUSTOM_REPLACE) {
			return;
		}
		dr->ds_data = prev;
		break;
	}
	if (unlikely(!dc)) {
		return _dispatch_ktrace1(DISPATCH_PERF_handlerless_source_fire, ds);
	}
	if (!dispatch_assume(prev != 0)) {
		return;
	}
	_dispatch_trace_source_callout_entry(ds, DS_EVENT_HANDLER, cq, dc);
#ifdef DBG_BSD_MEMSTAT
	if (unlikely(dr->du_filter == EVFILT_MEMORYSTATUS)) {
		_dispatch_ktrace2(KDBG_CODE(DBG_BSD, DBG_BSD_MEMSTAT, 0x100) | DBG_FUNC_START,
				prev, _dispatch_continuation_get_function_symbol(dc));
	}
#endif
	_dispatch_continuation_pop(dc, NULL, flags, cq);
#ifdef DBG_BSD_MEMSTAT
	if (unlikely(dr->du_filter == EVFILT_MEMORYSTATUS)) {
		_dispatch_ktrace0(KDBG_CODE(DBG_BSD, DBG_BSD_MEMSTAT, 0x100) | DBG_FUNC_END);
	}
#endif
	if (dr->du_is_timer) {
		if ((prev & DISPATCH_TIMER_DISARMED_MARKER) &&
				_dispatch_source_refs_needs_configuration(dr)) {
			_dispatch_timer_unote_configure(ds->ds_timer_refs);
		}
		if (dr->du_timer_flags & DISPATCH_TIMER_AFTER) {
			_dispatch_trace_item_complete(dc); // see _dispatch_after
			_dispatch_source_handler_free(dr, DS_EVENT_HANDLER);
			dispatch_release(ds); // dispatch_after sources are one-shot
		}
	}
}

DISPATCH_NOINLINE
static void
_dispatch_source_refs_finalize_unregistration(dispatch_source_t ds)
{
	dispatch_queue_flags_t dqf;
	dqf = _dispatch_queue_atomic_flags_set_and_clear_orig(ds,
			DSF_DELETED, DSF_NEEDS_EVENT | DSF_CANCEL_WAITER);
	if (dqf & DSF_DELETED) {
		DISPATCH_INTERNAL_CRASH(dqf, "Source finalized twice");
	}
	if (dqf & DSF_CANCEL_WAITER) {
		_dispatch_wake_by_address(&ds->dq_atomic_flags);
	}
	_dispatch_object_debug(ds, "%s", __func__);
	return _dispatch_release_tailcall(ds); // see _dispatch_queue_alloc()
}

static void
_dispatch_source_refs_unregister(dispatch_source_t ds, uint32_t options)
{
	_dispatch_object_debug(ds, "%s", __func__);
	dispatch_source_refs_t dr = ds->ds_refs;

	if (_dispatch_unote_unregister(dr, options)) {
		return _dispatch_source_refs_finalize_unregistration(ds);
	}

	// deferred unregistration
	dispatch_queue_flags_t oqf, nqf;
	os_atomic_rmw_loop2o(ds, dq_atomic_flags, oqf, nqf, relaxed, {
		if (oqf & (DSF_NEEDS_EVENT | DSF_DELETED)) {
			os_atomic_rmw_loop_give_up(break);
		}
		nqf = oqf | DSF_NEEDS_EVENT;
	});
}

static void
_dispatch_source_install(dispatch_source_t ds, dispatch_wlh_t wlh,
		dispatch_priority_t pri)
{
	dispatch_source_refs_t dr = ds->ds_refs;

	dispatch_assert(!ds->ds_is_installed);
	ds->ds_is_installed = true;

	_dispatch_object_debug(ds, "%s", __func__);
	if (unlikely(!_dispatch_unote_register(dr, wlh, pri))) {
		return _dispatch_source_refs_finalize_unregistration(ds);
	}
}

void
_dispatch_source_activate(dispatch_source_t ds)
{
	dispatch_continuation_t dc;
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_priority_t pri;
	dispatch_wlh_t wlh;

	if (unlikely(_dispatch_queue_atomic_flags(ds) & DSF_CANCELED)) {
		ds->ds_is_installed = true;
		return _dispatch_source_refs_finalize_unregistration(ds);
	}

	dc = _dispatch_source_get_event_handler(dr);
	if (dc) {
		if (_dispatch_object_is_barrier(dc)) {
			_dispatch_queue_atomic_flags_set(ds, DQF_BARRIER_BIT);
		}
		if ((dc->dc_priority & _PTHREAD_PRIORITY_ENFORCE_FLAG) ||
				!_dispatch_queue_priority_manually_selected(ds->dq_priority)) {
			ds->dq_priority = _dispatch_priority_from_pp_strip_flags(dc->dc_priority);
		}
		if (dc->dc_flags & DC_FLAG_FETCH_CONTEXT) {
			dc->dc_ctxt = ds->do_ctxt;
		}
	} else {
		_dispatch_bug_deprecated("dispatch source activated "
				"with no event handler set");
	}

	// call "super"
	_dispatch_lane_activate(ds);

	if ((dr->du_is_direct || dr->du_is_timer) && !ds->ds_is_installed) {
		pri = _dispatch_queue_compute_priority_and_wlh(ds, &wlh);
		if (pri) {
#if DISPATCH_USE_KEVENT_WORKLOOP
			dispatch_workloop_t dwl = _dispatch_wlh_to_workloop(wlh);
			if (dwl && dr->du_filter == DISPATCH_EVFILT_TIMER_WITH_CLOCK &&
					dr->du_ident < DISPATCH_TIMER_WLH_COUNT) {
				if (!dwl->dwl_timer_heap) {
					uint32_t count = DISPATCH_TIMER_WLH_COUNT;
					dwl->dwl_timer_heap = _dispatch_calloc(count,
							sizeof(struct dispatch_timer_heap_s));
				}
				dr->du_is_direct = true;
				_dispatch_wlh_retain(wlh);
				_dispatch_unote_state_set(dr, wlh, 0);
			}
#endif
			// rdar://45419440 this needs to be last
			_dispatch_source_install(ds, wlh, pri);
		}
	}
}

DISPATCH_NOINLINE
static void
_dispatch_source_handle_wlh_change(dispatch_source_t ds)
{
	dispatch_queue_flags_t dqf;

	dqf = _dispatch_queue_atomic_flags_set_orig(ds, DSF_WLH_CHANGED);
	if (!(dqf & DQF_MUTABLE)) {
		DISPATCH_CLIENT_CRASH(0, "Changing target queue "
				"hierarchy after source was activated");
	}
	if (!(dqf & DSF_WLH_CHANGED)) {
		_dispatch_bug_deprecated("Changing target queue "
				"hierarchy after source was activated");
	}
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_wakeup_target_t
_dispatch_source_invoke2(dispatch_source_t ds, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags, uint64_t *owned)
{
	dispatch_queue_wakeup_target_t retq = DISPATCH_QUEUE_WAKEUP_NONE;
	dispatch_queue_t dq = _dispatch_queue_get_current();
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_queue_flags_t dqf;

	if (unlikely(!(flags & DISPATCH_INVOKE_MANAGER_DRAIN) &&
			_dispatch_unote_wlh_changed(dr, _dispatch_get_event_wlh()))) {
		_dispatch_source_handle_wlh_change(ds);
	}

	if (_dispatch_queue_class_probe(ds)) {
		// Intentionally always drain even when on the manager queue
		// and not the source's regular target queue: we need to be able
		// to drain timer setting and the like there.
		dispatch_with_disabled_narrowing(dic, {
			retq = _dispatch_lane_serial_drain(ds, dic, flags, owned);
		});
	}

	// This function performs all source actions. Each action is responsible
	// for verifying that it takes place on the appropriate queue. If the
	// current queue is not the correct queue for this action, the correct queue
	// will be returned and the invoke will be re-driven on that queue.

	// The order of tests here in invoke and in wakeup should be consistent.

	dispatch_queue_t dkq = _dispatch_mgr_q._as_dq;
	bool avoid_starvation = false;

	if (dr->du_is_direct) {
		dkq = ds->do_targetq;
	}

	if (!ds->ds_is_installed) {
		// The source needs to be installed on the kevent queue.
		if (dq != dkq) {
			return dkq;
		}
		dispatch_priority_t pri = DISPATCH_PRIORITY_FLAG_MANAGER;
		if (likely(flags & DISPATCH_INVOKE_WORKER_DRAIN)) {
			pri = _dispatch_get_basepri();
		}
		_dispatch_source_install(ds, _dispatch_get_event_wlh(), pri);
	}

	if (unlikely(DISPATCH_QUEUE_IS_SUSPENDED(ds))) {
		// Source suspended by an item drained from the source queue.
		return ds->do_targetq;
	}

	if (_dispatch_source_refs_needs_configuration(dr)) {
		dqf = _dispatch_queue_atomic_flags(ds);
		if (!(dqf & (DSF_CANCELED | DQF_RELEASED))) {
			if (dq != dkq) {
				return dkq;
			}
			_dispatch_timer_unote_configure(ds->ds_timer_refs);
		}
	}

	if (_dispatch_source_get_registration_handler(dr)) {
		// The source has been registered and the registration handler needs
		// to be delivered on the target queue.
		if (dq != ds->do_targetq) {
			return ds->do_targetq;
		}
		// clears ds_registration_handler
		_dispatch_source_registration_callout(ds, dq, flags);
	}

	if (_dispatch_unote_needs_delete(dr)) {
		_dispatch_source_refs_unregister(ds, DUU_DELETE_ACK | DUU_MUST_SUCCEED);
	}

	dqf = _dispatch_queue_atomic_flags(ds);
	if (!(dqf & (DSF_CANCELED | DQF_RELEASED)) &&
			os_atomic_load2o(dr, ds_pending_data, relaxed)) {
		// The source has pending data to deliver via the event handler callback
		// on the target queue. Some sources need to be rearmed on the kevent
		// queue after event delivery.
		if (dq == ds->do_targetq) {
			_dispatch_source_latch_and_call(ds, dq, flags);
			dqf = _dispatch_queue_atomic_flags(ds);

			// starvation avoidance: if the source triggers itself then force a
			// re-queue to give other things already queued on the target queue
			// a chance to run.
			//
			// however, if the source is directly targeting an overcommit root
			// queue, this would requeue the source and ask for a new overcommit
			// thread right away.
			if (!(dqf & (DSF_CANCELED | DSF_DELETED))) {
				avoid_starvation = dq->do_targetq ||
						!(dq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT);
			}

			ds->ds_latched = true;
		} else {
			// there is no point trying to be eager, the next thing to do is
			// to deliver the event
			return ds->do_targetq;
		}
	}

	if ((dqf & (DSF_CANCELED | DQF_RELEASED)) && !(dqf & DSF_DELETED)) {
		// The source has been cancelled and needs to be uninstalled from the
		// kevent queue. After uninstallation, the cancellation handler needs
		// to be delivered to the target queue.
		if (dr->du_is_timer && !_dispatch_unote_armed(dr)) {
			// timers can cheat if not armed because there's nothing left
			// to do on the manager queue and unregistration can happen
			// on the regular target queue
		} else if (dq != dkq) {
			return dkq;
		}
		uint32_t duu_options = DUU_DELETE_ACK;
		if (!(dqf & DSF_NEEDS_EVENT)) duu_options |= DUU_PROBE;
		_dispatch_source_refs_unregister(ds, duu_options);
		dqf = _dispatch_queue_atomic_flags(ds);
		if (unlikely(!(dqf & DSF_DELETED))) {
			// we need to wait for the EV_DELETE
			return retq ? retq : DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT;
		}
	}

	if ((dqf & (DSF_CANCELED | DQF_RELEASED)) && (dqf & DSF_DELETED)) {
		if (dq != ds->do_targetq && (_dispatch_source_get_event_handler(dr) ||
				_dispatch_source_get_cancel_handler(dr) ||
				_dispatch_source_get_registration_handler(dr))) {
			retq = ds->do_targetq;
		} else {
			_dispatch_source_cancel_callout(ds, dq, flags);
			dqf = _dispatch_queue_atomic_flags(ds);
		}
		avoid_starvation = false;
	}

	if (!(dqf & (DSF_CANCELED | DQF_RELEASED)) &&
			_dispatch_source_refs_needs_rearm(dr)) {
		// The source needs to be rearmed on the kevent queue.
		if (dq != dkq) {
			return dkq;
		}
		if (unlikely(DISPATCH_QUEUE_IS_SUSPENDED(ds))) {
			// do not try to rearm the kevent if the source is suspended
			// from the source handler
			return ds->do_targetq;
		}
		if (dr->du_is_direct && _dispatch_unote_wlh(dr) == DISPATCH_WLH_ANON) {
			//
			// <rdar://problem/43622806> for legacy, direct event delivery,
			// _dispatch_source_install above could cause a worker thread to
			// deliver an event, and disarm the knote before we're through.
			//
			// This can lead to a double fire of the event handler for the same
			// event with the following ordering:
			//
			//------------------------------------------------------------------
			//  Thread1                         Thread2
			//
			//  _dispatch_source_invoke()
			//    _dispatch_source_install()
			//                                  _dispatch_kevent_worker_thread()
			//                                  _dispatch_source_merge_evt()
			//
			//    _dispatch_unote_resume()
			//                                  _dispatch_kevent_worker_thread()
			//  < re-enqueue due DIRTY >
			//
			//  _dispatch_source_invoke()
			//    ..._latch_and_call()
			//    _dispatch_unote_resume()
			//                                  _dispatch_source_merge_evt()
			//
			//  _dispatch_source_invoke()
			//    ..._latch_and_call()
			//
			//------------------------------------------------------------------
			//
			// To avoid this situation, we should never resume a direct source
			// for which we haven't fired an event.
			//
			// Note: this isn't a concern for kqworkloops as event delivery is
			//       serial with draining it by design.
			//
			if (ds->ds_latched) {
				ds->ds_latched = false;
				_dispatch_unote_resume(dr);
			}
			if (avoid_starvation) {
				// To avoid starvation of a source firing immediately when we
				// rearm it, force a round-trip through the end of the target
				// queue no matter what.
				return ds->do_targetq;
			}
		} else {
			_dispatch_unote_resume(dr);
			if (!avoid_starvation && _dispatch_wlh_should_poll_unote(dr)) {
				// try to redrive the drain from under the lock for sources
				// targeting an overcommit root queue to avoid parking
				// when the next event has already fired
				_dispatch_event_loop_drain(KEVENT_FLAG_IMMEDIATE);
			}
		}
	}

	return retq;
}

DISPATCH_NOINLINE
void
_dispatch_source_invoke(dispatch_source_t ds, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags)
{
	_dispatch_queue_class_invoke(ds, dic, flags,
			DISPATCH_INVOKE_DISALLOW_SYNC_WAITERS, _dispatch_source_invoke2);

#if DISPATCH_EVENT_BACKEND_KEVENT
	if (flags & DISPATCH_INVOKE_WORKLOOP_DRAIN) {
		dispatch_workloop_t dwl = (dispatch_workloop_t)_dispatch_get_wlh();
		dispatch_timer_heap_t dth = dwl->dwl_timer_heap;
		if (dth && dth[0].dth_dirty_bits) {
			_dispatch_event_loop_drain_timers(dwl->dwl_timer_heap,
					DISPATCH_TIMER_WLH_COUNT);
		}
	}
#endif // DISPATCH_EVENT_BACKEND_KEVENT
}

void
_dispatch_source_wakeup(dispatch_source_t ds, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags)
{
	// This function determines whether the source needs to be invoked.
	// The order of tests here in wakeup and in invoke should be consistent.

	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_queue_wakeup_target_t dkq = DISPATCH_QUEUE_WAKEUP_MGR;
	dispatch_queue_wakeup_target_t tq = DISPATCH_QUEUE_WAKEUP_NONE;
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(ds);
	dispatch_unote_state_t du_state = _dispatch_unote_state(dr);

	if (dr->du_is_direct) {
		dkq = DISPATCH_QUEUE_WAKEUP_TARGET;
	}

	if (!ds->ds_is_installed) {
		// The source needs to be installed on the kevent queue.
		tq = dkq;
	} else if (!(dqf & (DSF_CANCELED | DQF_RELEASED)) &&
			_dispatch_source_refs_needs_configuration(dr)) {
		// timer has to be configured on the kevent queue
		tq = dkq;
	} else if (_dispatch_source_get_registration_handler(dr)) {
		// The registration handler needs to be delivered to the target queue.
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
	} else if (_du_state_needs_delete(du_state)) {
		// Deferred deletion can be acknowledged which can always be done
		// from the target queue
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
	} else if (!(dqf & (DSF_CANCELED | DQF_RELEASED)) &&
			os_atomic_load2o(dr, ds_pending_data, relaxed)) {
		// The source has pending data to deliver to the target queue.
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
	} else if ((dqf & (DSF_CANCELED | DQF_RELEASED)) && !(dqf & DSF_DELETED)) {
		// The source needs to be uninstalled from the kevent queue, or the
		// cancellation handler needs to be delivered to the target queue.
		// Note: cancellation assumes installation.
		if (dr->du_is_timer && !_dispatch_unote_armed(dr)) {
			// timers can cheat if not armed because there's nothing left
			// to do on the manager queue and unregistration can happen
			// on the regular target queue
			tq = DISPATCH_QUEUE_WAKEUP_TARGET;
		} else if ((dqf & DSF_NEEDS_EVENT) && !(flags & DISPATCH_WAKEUP_EVENT)){
			// we're waiting for an event
		} else {
			// we need to initialize the deletion sequence
			tq = dkq;
		}
	} else if ((dqf & (DSF_CANCELED | DQF_RELEASED)) && (dqf & DSF_DELETED) &&
			(_dispatch_source_get_event_handler(dr) ||
			_dispatch_source_get_cancel_handler(dr) ||
			_dispatch_source_get_registration_handler(dr))) {
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
	} else if (!(dqf & (DSF_CANCELED | DQF_RELEASED)) &&
			_dispatch_source_refs_needs_rearm(dr)) {
		// The source needs to be rearmed on the kevent queue.
		tq = dkq;
	}
	if (!tq && _dispatch_queue_class_probe(ds)) {
		tq = DISPATCH_QUEUE_WAKEUP_TARGET;
	}

	if ((tq == DISPATCH_QUEUE_WAKEUP_TARGET) &&
			ds->do_targetq == _dispatch_mgr_q._as_dq) {
		tq = DISPATCH_QUEUE_WAKEUP_MGR;
	}

	return _dispatch_queue_wakeup(ds, qos, flags, tq);
}

void
dispatch_source_cancel(dispatch_source_t ds)
{
	_dispatch_object_debug(ds, "%s", __func__);
	// Right after we set the cancel flag, someone else
	// could potentially invoke the source, do the cancellation,
	// unregister the source, and deallocate it. We would
	// need to therefore retain/release before setting the bit
	_dispatch_retain_2(ds);

	if (_dispatch_queue_atomic_flags_set_orig(ds, DSF_CANCELED) & DSF_CANCELED){
		_dispatch_release_2_tailcall(ds);
	} else {
		dx_wakeup(ds, 0, DISPATCH_WAKEUP_MAKE_DIRTY | DISPATCH_WAKEUP_CONSUME_2);
	}
}

void
dispatch_source_cancel_and_wait(dispatch_source_t ds)
{
	dispatch_queue_flags_t old_dqf, new_dqf;
	dispatch_source_refs_t dr = ds->ds_refs;

	if (unlikely(_dispatch_source_get_cancel_handler(dr))) {
		DISPATCH_CLIENT_CRASH(ds, "Source has a cancel handler");
	}

	_dispatch_object_debug(ds, "%s", __func__);
	os_atomic_rmw_loop2o(ds, dq_atomic_flags, old_dqf, new_dqf, relaxed, {
		new_dqf = old_dqf | DSF_CANCELED;
		if (old_dqf & DSF_CANCEL_WAITER) {
			os_atomic_rmw_loop_give_up(break);
		}
		if (old_dqf & DSF_DELETED) {
			// just add DSF_CANCELED
		} else if ((old_dqf & DSF_NEEDS_EVENT) || dr->du_is_timer ||
				!dr->du_is_direct) {
			new_dqf |= DSF_CANCEL_WAITER;
		}
	});

	if (old_dqf & DQF_RELEASED) {
		DISPATCH_CLIENT_CRASH(ds, "Dispatch source used after last release");
	}
	if (old_dqf & DSF_DELETED) {
		return;
	}
	if (new_dqf & DSF_CANCEL_WAITER) {
		goto wakeup;
	}

	// simplified version of _dispatch_queue_drain_try_lock
	// that also sets the DIRTY bit on failure to lock
	uint64_t set_owner_and_set_full_width = _dispatch_lock_value_for_self() |
			DISPATCH_QUEUE_WIDTH_FULL_BIT | DISPATCH_QUEUE_IN_BARRIER;
	uint64_t old_state, new_state;

	os_atomic_rmw_loop2o(ds, dq_state, old_state, new_state, seq_cst, {
		new_state = old_state;
		if (likely(_dq_state_is_runnable(old_state) &&
				!_dq_state_drain_locked(old_state))) {
			new_state &= DISPATCH_QUEUE_DRAIN_PRESERVED_BITS_MASK;
			new_state |= set_owner_and_set_full_width;
		} else if (old_dqf & DSF_CANCELED) {
			os_atomic_rmw_loop_give_up(break);
		} else {
			// this case needs a release barrier, hence the seq_cst above
			new_state |= DISPATCH_QUEUE_DIRTY;
		}
	});

	if (unlikely(_dq_state_is_suspended(old_state))) {
		if (unlikely(_dq_state_suspend_cnt(old_state))) {
			DISPATCH_CLIENT_CRASH(ds, "Source is suspended");
		}
		// inactive sources have never been registered and there is no need
		// to wait here because activation will notice and mark the source
		// as deleted without ever trying to use the fd or mach port.
		return dispatch_activate(ds);
	}

	if (likely(_dq_state_is_runnable(old_state) &&
			!_dq_state_drain_locked(old_state))) {
		// deletion may have proceeded concurrently while we were
		// taking the lock, so we need to check we're not doing it twice.
		if (likely(!(_dispatch_queue_atomic_flags(ds) & DSF_DELETED))) {
			// same thing _dispatch_source_invoke2() does for cancellation
			_dispatch_source_refs_unregister(ds, DUU_DELETE_ACK | DUU_PROBE);
		}
		if (likely(_dispatch_queue_atomic_flags(ds) & DSF_DELETED)) {
			_dispatch_source_cancel_callout(ds, NULL, DISPATCH_INVOKE_NONE);
		}
		dx_wakeup(ds, 0, DISPATCH_WAKEUP_EVENT |
				DISPATCH_WAKEUP_BARRIER_COMPLETE);
	} else if (unlikely(_dq_state_drain_locked_by_self(old_state))) {
		DISPATCH_CLIENT_CRASH(ds, "dispatch_source_cancel_and_wait "
				"called from a source handler");
	} else {
		dispatch_qos_t qos;
wakeup:
		qos = _dispatch_qos_from_pp(_dispatch_get_priority());
		dx_wakeup(ds, qos, DISPATCH_WAKEUP_MAKE_DIRTY);
		dispatch_activate(ds);
	}

	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(ds);
	while (unlikely(!(dqf & DSF_DELETED))) {
		if (unlikely(!(dqf & DSF_CANCEL_WAITER))) {
			if (!os_atomic_cmpxchgv2o(ds, dq_atomic_flags,
					dqf, dqf | DSF_CANCEL_WAITER, &dqf, relaxed)) {
				continue;
			}
			dqf |= DSF_CANCEL_WAITER;
		}
		_dispatch_wait_on_address(&ds->dq_atomic_flags, dqf,
				DISPATCH_TIME_FOREVER, DLOCK_LOCK_NONE);
		dqf = _dispatch_queue_atomic_flags(ds);
	}
}

void
_dispatch_source_merge_evt(dispatch_unote_t du, uint32_t flags,
		OS_UNUSED uintptr_t data, pthread_priority_t pp)
{
	dispatch_source_t ds = _dispatch_source_from_refs(du._dr);

	dispatch_unote_state_t du_state = _dispatch_unote_state(du);
	if (!(flags & EV_UDATA_SPECIFIC) && !_du_state_registered(du_state)) {
		if (!du._du->du_is_timer) {
			// Timers must be unregistered from their target queue, else this
			// unregistration can race with the optimization in
			// _dispatch_source_invoke() to unregister fired oneshot timers.
			//
			// Because oneshot timers dominate the world, we prefer paying an
			// extra wakeup for repeating timers, and avoid the wakeup for
			// oneshot timers.
			_dispatch_source_refs_finalize_unregistration(ds);
		}
	}

	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(ds);
	if (unlikely(flags & EV_VANISHED)) {
		if (dqf & DSF_STRICT) {
			DISPATCH_CLIENT_CRASH(du._du->du_ident, "Unexpected EV_VANISHED "
					"(do not destroy random mach ports or file descriptors)");
		} else {
			_dispatch_bug_kevent_vanished(du._du);
		}
		// if the resource behind the ident vanished, the event handler can't
		// do anything useful anymore, so do not try to call it at all
		os_atomic_store2o(du._dr, ds_pending_data, 0, relaxed);
	}

	_dispatch_debug("kevent-source[%p]: merged kevent[%p]", ds, du._dr);
	_dispatch_object_debug(ds, "%s", __func__);
	dx_wakeup(ds, _dispatch_qos_from_pp(pp), DISPATCH_WAKEUP_EVENT |
			DISPATCH_WAKEUP_CLEAR_ACTIVATING |
			DISPATCH_WAKEUP_CONSUME_2 | DISPATCH_WAKEUP_MAKE_DIRTY);
}

#pragma mark -
#pragma mark dispatch_source_timer

#define _dispatch_source_timer_telemetry_enabled() false

DISPATCH_NOINLINE
static void
_dispatch_source_timer_telemetry_slow(dispatch_source_t ds,
		dispatch_clock_t clock, struct dispatch_timer_source_s *values)
{
	if (_dispatch_trace_timer_configure_enabled()) {
		_dispatch_trace_timer_configure(ds, clock, values);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_source_timer_telemetry(dispatch_source_t ds, dispatch_clock_t clock,
		struct dispatch_timer_source_s *values)
{
	if (_dispatch_trace_timer_configure_enabled() ||
			_dispatch_source_timer_telemetry_enabled()) {
		_dispatch_source_timer_telemetry_slow(ds, clock, values);
		__asm__ __volatile__ (""); // prevent tailcall
	}
}

static dispatch_timer_config_t
_dispatch_timer_config_create(dispatch_time_t start,
		uint64_t interval, uint64_t leeway, dispatch_timer_source_refs_t dt)
{
	dispatch_timer_config_t dtc;
	dtc = _dispatch_calloc(1ul, sizeof(struct dispatch_timer_config_s));
	if (unlikely(interval == 0)) {
		if (start != DISPATCH_TIME_FOREVER) {
			_dispatch_bug_deprecated("Setting timer interval to 0 requests "
					"a 1ns timer, did you mean FOREVER (a one-shot timer)?");
		}
		interval = 1;
	} else if ((int64_t)interval < 0) {
		// 6866347 - make sure nanoseconds won't overflow
		interval = INT64_MAX;
	}
	if ((int64_t)leeway < 0) {
		leeway = INT64_MAX;
	}

	dispatch_clock_t clock;
	uint64_t target;
	if (start == DISPATCH_TIME_FOREVER) {
		target = INT64_MAX;
		// Do not change the clock when postponing the time forever in the
		// future, this will default to UPTIME if no clock was set.
		clock = _dispatch_timer_flags_to_clock(dt->du_timer_flags);
	} else {
		_dispatch_time_to_clock_and_value(start, &clock, &target);
		if (target == DISPATCH_TIME_NOW) {
			if (clock == DISPATCH_CLOCK_UPTIME) {
				target = _dispatch_uptime();
			} else {
				dispatch_assert(clock == DISPATCH_CLOCK_MONOTONIC);
				target = _dispatch_monotonic_time();
			}
		}
	}

	if (clock != DISPATCH_CLOCK_WALL) {
		// uptime or monotonic clock
		interval = _dispatch_time_nano2mach(interval);
		if (interval < 1) {
			// rdar://problem/7287561 interval must be at least one in
			// in order to avoid later division by zero when calculating
			// the missed interval count. (NOTE: the wall clock's
			// interval is already "fixed" to be 1 or more)
			interval = 1;
		}
		leeway = _dispatch_time_nano2mach(leeway);
	}
	if (interval < INT64_MAX && leeway > interval / 2) {
		leeway = interval / 2;
	}

	dtc->dtc_clock = clock;
	dtc->dtc_timer.target = target;
	dtc->dtc_timer.interval = interval;
	if (target + leeway < INT64_MAX) {
		dtc->dtc_timer.deadline = target + leeway;
	} else {
		dtc->dtc_timer.deadline = INT64_MAX;
	}
	return dtc;
}

static dispatch_timer_config_t
_dispatch_interval_config_create(dispatch_time_t start,
		uint64_t interval, uint64_t leeway, dispatch_timer_source_refs_t dt)
{
#define NSEC_PER_FRAME (NSEC_PER_SEC/60)
// approx 1 year (60s * 60m * 24h * 365d)
#define FOREVER_NSEC 31536000000000000ull

	const bool animation = dt->du_timer_flags & DISPATCH_INTERVAL_UI_ANIMATION;
	dispatch_timer_config_t dtc;
	dtc = _dispatch_calloc(1ul, sizeof(struct dispatch_timer_config_s));
	dtc->dtc_clock = DISPATCH_CLOCK_UPTIME;

	if (start == DISPATCH_TIME_FOREVER) {
		dtc->dtc_timer.target = INT64_MAX;
		dtc->dtc_timer.interval = INT64_MAX;
		dtc->dtc_timer.deadline = INT64_MAX;
		return dtc;
	}

	if (start != DISPATCH_TIME_NOW) {
		DISPATCH_CLIENT_CRASH(0, "Start value is not DISPATCH_TIME_NOW or "
				"DISPATCH_TIME_FOREVER");
	} else if (unlikely(interval == 0)) {
		DISPATCH_CLIENT_CRASH(0, "Setting interval to 0");
	}

	if (likely(interval <= (animation ? FOREVER_NSEC/NSEC_PER_FRAME :
			FOREVER_NSEC/NSEC_PER_MSEC))) {
		interval *= animation ? NSEC_PER_FRAME : NSEC_PER_MSEC;
	} else {
		interval = FOREVER_NSEC;
	}

	interval = _dispatch_time_nano2mach(interval);
	start = _dispatch_uptime() + interval;
	start -= (start % interval);
	if (leeway <= 1000) {
		leeway = interval * leeway / 1000;
	} else if (leeway != UINT64_MAX) {
		DISPATCH_CLIENT_CRASH(0, "Passing an invalid leeway");
	} else if (animation) {
		leeway = _dispatch_time_nano2mach(NSEC_PER_FRAME);
	} else {
		leeway = interval / 2;
	}
	dtc->dtc_clock = DISPATCH_CLOCK_UPTIME;
	dtc->dtc_timer.target = start;
	dtc->dtc_timer.deadline = start + leeway;
	dtc->dtc_timer.interval = interval;
	return dtc;
}

DISPATCH_NOINLINE
void
dispatch_source_set_timer(dispatch_source_t ds, dispatch_time_t start,
		uint64_t interval, uint64_t leeway)
{
	dispatch_timer_source_refs_t dt = ds->ds_timer_refs;
	dispatch_timer_config_t dtc;

	if (unlikely(!dt->du_is_timer)) {
		DISPATCH_CLIENT_CRASH(ds, "Attempt to set timer on a non-timer source");
	}

	if (dt->du_timer_flags & DISPATCH_TIMER_INTERVAL) {
		dtc = _dispatch_interval_config_create(start, interval, leeway, dt);
	} else {
		dtc = _dispatch_timer_config_create(start, interval, leeway, dt);
	}
	if (_dispatch_timer_flags_to_clock(dt->du_timer_flags) != dtc->dtc_clock &&
			dt->du_filter == DISPATCH_EVFILT_TIMER_WITH_CLOCK) {
		DISPATCH_CLIENT_CRASH(0, "Attempting to modify timer clock");
	}

	_dispatch_source_timer_telemetry(ds, dtc->dtc_clock, &dtc->dtc_timer);
	dtc = os_atomic_xchg2o(dt, dt_pending_config, dtc, release);
	if (dtc) free(dtc);
	dx_wakeup(ds, 0, DISPATCH_WAKEUP_MAKE_DIRTY);
}

#pragma mark -
#pragma mark dispatch_after

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_after(dispatch_time_t when, dispatch_queue_t dq,
		void *ctxt, void *handler, bool block)
{
	dispatch_timer_source_refs_t dt;
	dispatch_source_t ds;
	uint64_t leeway, delta;

	if (when == DISPATCH_TIME_FOREVER) {
#if DISPATCH_DEBUG
		DISPATCH_CLIENT_CRASH(0, "dispatch_after called with 'when' == infinity");
#endif
		return;
	}

	delta = _dispatch_timeout(when);
	if (delta == 0) {
		if (block) {
			return dispatch_async(dq, handler);
		}
		return dispatch_async_f(dq, ctxt, handler);
	}
	leeway = delta / 10; // <rdar://problem/13447496>

	if (leeway < NSEC_PER_MSEC) leeway = NSEC_PER_MSEC;
	if (leeway > 60 * NSEC_PER_SEC) leeway = 60 * NSEC_PER_SEC;

	// this function can and should be optimized to not use a dispatch source
	ds = dispatch_source_create(&_dispatch_source_type_after, 0, 0, dq);
	dt = ds->ds_timer_refs;

	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	if (block) {
		_dispatch_continuation_init(dc, dq, handler, 0, 0);
	} else {
		_dispatch_continuation_init_f(dc, dq, ctxt, handler, 0, 0);
	}
	// reference `ds` so that it doesn't show up as a leak
	dc->dc_data = ds;
	_dispatch_trace_item_push(dq, dc);
	os_atomic_store2o(dt, ds_handler[DS_EVENT_HANDLER], dc, relaxed);

	dispatch_clock_t clock;
	uint64_t target;
	_dispatch_time_to_clock_and_value(when, &clock, &target);
	if (clock != DISPATCH_CLOCK_WALL) {
		leeway = _dispatch_time_nano2mach(leeway);
	}
	dt->du_timer_flags |= _dispatch_timer_flags_from_clock(clock);
	dt->dt_timer.target = target;
	dt->dt_timer.interval = UINT64_MAX;
	dt->dt_timer.deadline = target + leeway;
	dispatch_activate(ds);
}

DISPATCH_NOINLINE
void
dispatch_after_f(dispatch_time_t when, dispatch_queue_t queue, void *ctxt,
		dispatch_function_t func)
{
	_dispatch_after(when, queue, ctxt, func, false);
}

#ifdef __BLOCKS__
void
dispatch_after(dispatch_time_t when, dispatch_queue_t queue,
		dispatch_block_t work)
{
	_dispatch_after(when, queue, NULL, work, true);
}
#endif

#pragma mark -
#pragma mark dispatch_source_debug

DISPATCH_COLD
static size_t
_dispatch_source_debug_attr(dispatch_source_t ds, char* buf, size_t bufsiz)
{
	dispatch_queue_t target = ds->do_targetq;
	dispatch_source_refs_t dr = ds->ds_refs;
	dispatch_queue_flags_t dqf = _dispatch_queue_atomic_flags(ds);
	dispatch_unote_state_t du_state = _dispatch_unote_state(dr);
	return dsnprintf(buf, bufsiz, "target = %s[%p], ident = 0x%x, "
			"mask = 0x%x, pending_data = 0x%llx, registered = %d, "
			"armed = %d, %s%s%s",
			target && target->dq_label ? target->dq_label : "", target,
			dr->du_ident, dr->du_fflags, (unsigned long long)dr->ds_pending_data,
			_du_state_registered(du_state), _du_state_armed(du_state),
			(dqf & DSF_CANCELED) ? "cancelled, " : "",
			(dqf & DSF_NEEDS_EVENT) ? "needs-event, " : "",
			(dqf & DSF_DELETED) ? "deleted, " : "");
}

DISPATCH_COLD
static size_t
_dispatch_timer_debug_attr(dispatch_source_t ds, char* buf, size_t bufsiz)
{
	dispatch_timer_source_refs_t dr = ds->ds_timer_refs;
	return dsnprintf(buf, bufsiz, "timer = { target = 0x%llx, deadline = 0x%llx"
			", interval = 0x%llx, flags = 0x%x }, ",
			(unsigned long long)dr->dt_timer.target,
			(unsigned long long)dr->dt_timer.deadline,
			(unsigned long long)dr->dt_timer.interval, dr->du_timer_flags);
}

size_t
_dispatch_source_debug(dispatch_source_t ds, char *buf, size_t bufsiz)
{
	dispatch_source_refs_t dr = ds->ds_refs;
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			_dispatch_object_class_name(ds), ds);
	offset += _dispatch_object_debug_attr(ds, &buf[offset], bufsiz - offset);
	offset += _dispatch_source_debug_attr(ds, &buf[offset], bufsiz - offset);
	if (dr->du_is_timer) {
		offset += _dispatch_timer_debug_attr(ds, &buf[offset], bufsiz - offset);
	}
	offset += dsnprintf(&buf[offset], bufsiz - offset, "kevent = %p%s, "
			"filter = %s }", dr,  dr->du_is_direct ? " (direct)" : "",
			dux_type(dr)->dst_kind);
	return offset;
}
