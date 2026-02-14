/*
 * Copyright (c) 2010-2013 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_TRACE__
#define __DISPATCH_TRACE__

#if DISPATCH_PURE_C

#if DISPATCH_USE_DTRACE_INTROSPECTION
#define _dispatch_trace_callout(_c, _f, _dcc) do { \
		if (unlikely(DISPATCH_CALLOUT_ENTRY_ENABLED() || \
				DISPATCH_CALLOUT_RETURN_ENABLED())) { \
			dispatch_queue_t _dq = _dispatch_queue_get_current(); \
			const char *_label = _dq && _dq->dq_label ? _dq->dq_label : ""; \
			dispatch_function_t _func = (dispatch_function_t)(_f); \
			void *_ctxt = (_c); \
			DISPATCH_CALLOUT_ENTRY(_dq, _label, _func, _ctxt); \
			_dcc; \
			DISPATCH_CALLOUT_RETURN(_dq, _label, _func, _ctxt); \
		} else { \
			_dcc; \
		} \
	} while (0)
#elif DISPATCH_INTROSPECTION
#define _dispatch_trace_callout(_c, _f, _dcc) \
		do { (void)(_c); (void)(_f); _dcc; } while (0)
#endif // DISPATCH_USE_DTRACE_INTROSPECTION || DISPATCH_INTROSPECTION

#if DISPATCH_USE_DTRACE_INTROSPECTION || DISPATCH_INTROSPECTION
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_client_callout(void *ctxt, dispatch_function_t f)
{
	dispatch_function_t func = (f == _dispatch_call_block_and_release &&
			ctxt ? _dispatch_Block_invoke(ctxt) : f);
	_dispatch_introspection_callout_entry(ctxt, func);
	_dispatch_trace_callout(ctxt, func, _dispatch_client_callout(ctxt, f));
	_dispatch_introspection_callout_return(ctxt, func);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_client_callout2(void *ctxt, size_t i, void (*f)(void *, size_t))
{
	dispatch_function_t func = (dispatch_function_t)f;
	_dispatch_introspection_callout_entry(ctxt, func);
	_dispatch_trace_callout(ctxt, func, _dispatch_client_callout2(ctxt, i, f));
	_dispatch_introspection_callout_return(ctxt, func);
}

#define _dispatch_client_callout		_dispatch_trace_client_callout
#define _dispatch_client_callout2		_dispatch_trace_client_callout2
#endif // DISPATCH_USE_DTRACE_INTROSPECTION || DISPATCH_INTROSPECTION

#ifdef _COMM_PAGE_KDEBUG_ENABLE
#define DISPATCH_KTRACE_ENABLED \
		(*(volatile uint32_t *)_COMM_PAGE_KDEBUG_ENABLE != 0)

#if DISPATCH_INTROSPECTION
#define _dispatch_only_if_ktrace_enabled(...) \
		if (unlikely(DISPATCH_KTRACE_ENABLED)) ({ __VA_ARGS__; })
#else
#define _dispatch_only_if_ktrace_enabled(...) (void)0
#endif /* DISPATCH_INTROSPECTION */

#else /* _COMM_PAGE_KDEBUG_ENABLE */

#define DISPATCH_KTRACE_ENABLED 0
#define _dispatch_only_if_ktrace_enabled(...) (void)0
#endif /* _COMM_PAGE_KDEBUG_ENABLE */


#if DISPATCH_USE_DTRACE_INTROSPECTION
#define _dispatch_trace_continuation(_q, _o, _t) do { \
		dispatch_queue_t _dq = (_q); \
		const char *_label = _dq && _dq->dq_label ? _dq->dq_label : ""; \
		struct dispatch_object_s *_do = (_o); \
		dispatch_continuation_t _dc; \
		char *_kind; \
		dispatch_function_t _func; \
		void *_ctxt; \
		if (_dispatch_object_has_vtable(_do)) { \
			_kind = (char*)_dispatch_object_class_name(_do); \
			if ((dx_metatype(_do) == _DISPATCH_SOURCE_TYPE) && \
					_dq != _dispatch_mgr_q._as_dq) { \
				dispatch_source_t _ds = (dispatch_source_t)_do; \
				_dc = os_atomic_load(&_ds->ds_refs->ds_handler[ \
						DS_EVENT_HANDLER], relaxed); \
				_func = _dc ? _dc->dc_func : NULL; \
				_ctxt = _dc ? _dc->dc_ctxt : NULL; \
			} else { \
				_func = (dispatch_function_t)_dispatch_lane_invoke; \
				_ctxt = _do->do_ctxt; \
			} \
		} else { \
			_dc = (void*)_do; \
			_ctxt = _dc->dc_ctxt; \
			if (_dc->dc_flags & DC_FLAG_SYNC_WAITER) { \
				_kind = "semaphore"; \
				_func = (dispatch_function_t)dispatch_semaphore_signal; \
			} else if (_dc->dc_flags & DC_FLAG_BLOCK) { \
				_kind = "block"; \
				_func = _dispatch_Block_invoke(_dc->dc_ctxt); \
			} else { \
				_kind = "function"; \
				_func = _dc->dc_func; \
			} \
		} \
		_t(_dq, _label, _do, _kind, _func, _ctxt); \
	} while (0)
#elif DISPATCH_INTROSPECTION
#define _dispatch_trace_continuation(_q, _o, _t) \
		do { (void)(_q); (void)(_o); } while(0)
#define DISPATCH_QUEUE_PUSH_ENABLED() 0
#define DISPATCH_QUEUE_POP_ENABLED() 0
#endif // DISPATCH_USE_DTRACE_INTROSPECTION || DISPATCH_INTROSPECTION

#if DISPATCH_USE_DTRACE_INTROSPECTION || DISPATCH_INTROSPECTION

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_class_t
_dispatch_trace_queue_create(dispatch_queue_class_t dqu)
{
	_dispatch_only_if_ktrace_enabled({
		uint64_t dq_label[4] = {0}; // So that we get the right null termination
		dispatch_queue_t dq = dqu._dq;
		strncpy((char *)dq_label, (char *)dq->dq_label ?: "", sizeof(dq_label));

		_dispatch_ktrace2(DISPATCH_QOS_TRACE_queue_creation_start,
				dq->dq_serialnum,
				_dispatch_priority_to_pp_prefer_fallback(dq->dq_priority));

		_dispatch_ktrace4(DISPATCH_QOS_TRACE_queue_creation_end,
						dq_label[0], dq_label[1], dq_label[2], dq_label[3]);
	});

	return _dispatch_introspection_queue_create(dqu);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_queue_dispose(dispatch_queue_class_t dqu)
{
	_dispatch_ktrace1(DISPATCH_QOS_TRACE_queue_dispose, (dqu._dq)->dq_serialnum);
	_dispatch_introspection_queue_dispose(dqu);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_source_dispose(dispatch_source_t ds)
{
	_dispatch_ktrace1(DISPATCH_QOS_TRACE_src_dispose, (uintptr_t)ds);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_block_create_with_voucher_and_priority(dispatch_block_t db,
		void *func, dispatch_block_flags_t original_flags,
		pthread_priority_t original_priority,
		pthread_priority_t thread_prio, pthread_priority_t final_block_prio)
{
	_dispatch_ktrace4(DISPATCH_QOS_TRACE_private_block_creation,
			(uintptr_t)db,
			(uintptr_t)func,
			BITPACK_UINT32_PAIR(original_flags, original_priority),
			BITPACK_UINT32_PAIR(thread_prio, final_block_prio));
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_firehose_reserver_gave_up(uint8_t stream, uint8_t ref,
		bool waited, uint64_t old_state, uint64_t new_state)
{
	uint64_t first = ((uint64_t)ref << 8) | (uint64_t)stream;
	uint64_t second = waited;
	_dispatch_ktrace4(DISPATCH_FIREHOSE_TRACE_reserver_gave_up, first, second,
			old_state, new_state);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_firehose_reserver_wait(uint8_t stream, uint8_t ref,
		bool waited, uint64_t old_state, uint64_t new_state, bool reliable)
{
	uint64_t first = ((uint64_t)ref << 8) | (uint64_t)stream;
	uint64_t second = ((uint64_t)reliable << 1) | waited;
	_dispatch_ktrace4(DISPATCH_FIREHOSE_TRACE_reserver_wait, first, second,
			old_state, new_state);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_firehose_allocator(uint64_t ask0, uint64_t ask1,
		uint64_t old_state, uint64_t new_state)
{
	_dispatch_ktrace4(DISPATCH_FIREHOSE_TRACE_allocator, ask0, ask1, old_state,
			new_state);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_firehose_wait_for_logd(uint8_t stream, uint64_t timestamp,
		uint64_t old_state, uint64_t new_state)
{
	_dispatch_ktrace4(DISPATCH_FIREHOSE_TRACE_wait_for_logd, stream, timestamp,
			old_state, new_state);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_firehose_chunk_install(uint64_t ask0, uint64_t ask1,
		uint64_t old_state, uint64_t new_state)
{
	_dispatch_ktrace4(DISPATCH_FIREHOSE_TRACE_chunk_install, ask0, ask1,
			old_state, new_state);
}

/* Implemented in introspection.c */
void
_dispatch_trace_item_push_internal(dispatch_queue_t dq, dispatch_object_t dou);

#define _dispatch_trace_item_push_inline(...) \
		_dispatch_only_if_ktrace_enabled({ \
			_dispatch_trace_item_push_internal(__VA_ARGS__); \
		})

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_item_push_list(dispatch_queue_global_t dq,
		dispatch_object_t _head, dispatch_object_t _tail)
{
	if (unlikely(DISPATCH_QUEUE_PUSH_ENABLED() || DISPATCH_KTRACE_ENABLED)) {
		struct dispatch_object_s *dou = _head._do;
		do {
			if (unlikely(DISPATCH_QUEUE_PUSH_ENABLED())) {
				_dispatch_trace_continuation(dq->_as_dq, dou, DISPATCH_QUEUE_PUSH);
			}

			_dispatch_trace_item_push_inline(dq->_as_dq, dou);
		} while (dou != _tail._do && (dou = dou->do_next));
	}
	_dispatch_introspection_queue_push_list(dq, _head, _tail);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_item_push(dispatch_queue_class_t dqu, dispatch_object_t _tail)
{
	if (unlikely(DISPATCH_QUEUE_PUSH_ENABLED())) {
		_dispatch_trace_continuation(dqu._dq, _tail._do, DISPATCH_QUEUE_PUSH);
	}

	_dispatch_trace_item_push_inline(dqu._dq, _tail._do);
	_dispatch_introspection_queue_push(dqu, _tail);
}

/* Implemented in introspection.c */
void
_dispatch_trace_item_pop_internal(dispatch_queue_t dq, dispatch_object_t dou);

#define _dispatch_trace_item_pop_inline(...) \
		_dispatch_only_if_ktrace_enabled({ \
			_dispatch_trace_item_pop_internal(__VA_ARGS__); \
		})

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_item_pop(dispatch_queue_class_t dqu, dispatch_object_t dou)
{
	if (unlikely(DISPATCH_QUEUE_POP_ENABLED())) {
		_dispatch_trace_continuation(dqu._dq, dou._do, DISPATCH_QUEUE_POP);
	}

	_dispatch_trace_item_pop_inline(dqu._dq, dou);
	_dispatch_introspection_queue_pop(dqu, dou);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_item_complete_inline(dispatch_object_t dou)
{
	_dispatch_ktrace1(DISPATCH_QOS_TRACE_queue_item_complete, dou._do_value);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_item_complete(dispatch_object_t dou)
{
	_dispatch_trace_item_complete_inline(dou);
	_dispatch_introspection_queue_item_complete(dou);
}

DISPATCH_ALWAYS_INLINE
static inline struct dispatch_object_s *
_dispatch_trace_item_sync_push_pop(dispatch_queue_class_t dqu,
		void *ctx, dispatch_function_t f, uintptr_t dc_flags)
{
	// No need to add tracing here since the introspection calls out to
	// _trace_item_push and _trace_item_pop
	return _dispatch_introspection_queue_fake_sync_push_pop(dqu._dq, ctx,
			f, dc_flags);
}

/* Implemented in introspection.c */
void
_dispatch_trace_source_callout_entry_internal(dispatch_source_t ds, long kind,
		dispatch_queue_t dq, dispatch_continuation_t dc);

#define _dispatch_trace_source_callout_entry(...) \
		_dispatch_only_if_ktrace_enabled({ \
			_dispatch_trace_source_callout_entry_internal(__VA_ARGS__); \
		})

#define _dispatch_trace_runtime_event(evt, ptr, value) \
		_dispatch_introspection_runtime_event(\
				dispatch_introspection_runtime_event_##evt, ptr, value)

#define DISPATCH_TRACE_ARG(arg) , arg
#else
#define _dispatch_trace_queue_create _dispatch_introspection_queue_create
#define _dispatch_trace_queue_dispose _dispatch_introspection_queue_dispose
#define _dispatch_trace_source_dispose(ds) ((void)0)
#define _dispatch_trace_block_create_with_voucher_and_priority(_db, _func, \
		_flags, _pri, _tpri, _bpri) \
		do { (void)_db; (void)_func; (void) _flags; (void) _pri; (void) _tpri; \
			(void) _bpri; } while (0)
#define _dispatch_trace_firehose_reserver_gave_up(stream, ref, waited, \
		old_state, new_state) \
		do { (void)(stream); (void)(ref); (void)(waited); (void)(old_state); \
			(void)(new_state); } while (0)
#define _dispatch_trace_firehose_reserver_wait(stream, ref, waited, \
		old_state, new_state, reliable) \
		do { (void)(stream); (void)(ref); (void)(waited); (void)(old_state); \
			(void)(new_state); (void)(reliable); } while (0)
#define _dispatch_trace_firehose_allocator(ask0, ask1, old_state, new_state) \
		do { (void)(ask0); (void)(ask1); (void)(old_state); \
			(void)(new_state); } while (0)
#define _dispatch_trace_firehose_wait_for_logd(stream, timestamp, old_state, \
		new_state) \
		do { (void)(stream); (void)(timestamp); (void)(old_state); \
			(void)(new_state); } while (0)
#define _dispatch_trace_firehose_chunk_install(ask0, ask1, old_state, \
		new_state) \
		do { (void)(ask0); (void)(ask1); (void)(old_state); \
			(void)(new_state); } while (0)
#define _dispatch_trace_item_push(dq, dou) \
		do { (void)(dq); (void)(dou); } while(0)
#define _dispatch_trace_item_push_list(dq, head, tail) \
		do { (void)(dq); (void)(head); (void)tail; } while(0)
#define _dispatch_trace_item_pop(dq, dou) \
		do { (void)(dq); (void)(dou); } while(0)
#define _dispatch_trace_item_complete(dou) ((void)0)
#define _dispatch_trace_item_sync_push_pop(dq, ctxt, func, flags) \
		do { (void)(dq); (void)(ctxt); (void)(func); (void)(flags); } while(0)
#define _dispatch_trace_source_callout_entry(ds, k, dq, dc) ((void)0)
#define _dispatch_trace_runtime_event(evt, ptr, value) \
		do { (void)(ptr); (void)(value); } while(0)
#define DISPATCH_TRACE_ARG(arg)
#endif // DISPATCH_USE_DTRACE_INTROSPECTION || DISPATCH_INTROSPECTION

#if DISPATCH_USE_DTRACE
static inline dispatch_function_t
_dispatch_trace_timer_function(dispatch_timer_source_refs_t dr)
{
	dispatch_continuation_t dc;
	dc = os_atomic_load(&dr->ds_handler[DS_EVENT_HANDLER], relaxed);
	return dc ? dc->dc_func : NULL;
}

DISPATCH_ALWAYS_INLINE
static inline uint64_t
_dispatch_time_clock_to_nsecs(dispatch_clock_t clock, uint64_t t)
{
#if !DISPATCH_TIME_UNIT_USES_NANOSECONDS
	switch (clock) {
	case DISPATCH_CLOCK_MONOTONIC:
	case DISPATCH_CLOCK_UPTIME:
		return _dispatch_time_mach2nano(t);
	case DISPATCH_CLOCK_WALL:
		return t;
	}
#else
	(void)clock;
	return t;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_trace_timer_params_t
_dispatch_trace_timer_params(dispatch_clock_t clock,
		struct dispatch_timer_source_s *values, uint64_t deadline,
		dispatch_trace_timer_params_t params)
{
	#define _dispatch_trace_time2nano3(t) \
			(_dispatch_time_clock_to_nsecs(clock, t))
	#define _dispatch_trace_time2nano2(v, t) ({ uint64_t _t = (t); \
			(v) >= INT64_MAX ? -1ll : (int64_t)_dispatch_trace_time2nano3(_t);})
	#define _dispatch_trace_time2nano(v) ({ uint64_t _t; \
			_t = _dispatch_trace_time2nano3(v); _t >= INT64_MAX ? -1ll : \
			(int64_t)_t; })
	if (deadline) {
		params->deadline = (int64_t)deadline;
	} else {
		uint64_t now = _dispatch_time_now(clock);
		params->deadline = _dispatch_trace_time2nano2(values->target,
				values->target < now ? 0 : values->target - now);
	}
	uint64_t leeway = values->deadline - values->target;
	params->interval = _dispatch_trace_time2nano(values->interval);
	params->leeway = _dispatch_trace_time2nano(leeway);
	return params;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_trace_timer_configure_enabled(void)
{
	return DISPATCH_TIMER_CONFIGURE_ENABLED();
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_configure(dispatch_source_t ds, dispatch_clock_t clock,
		struct dispatch_timer_source_s *values)
{
	dispatch_timer_source_refs_t dr = ds->ds_timer_refs;
	struct dispatch_trace_timer_params_s params;
	DISPATCH_TIMER_CONFIGURE(ds, _dispatch_trace_timer_function(dr),
			_dispatch_trace_timer_params(clock, values, 0, &params));
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_program(dispatch_timer_source_refs_t dr, uint64_t deadline)
{
	if (unlikely(DISPATCH_TIMER_PROGRAM_ENABLED())) {
		if (deadline && dr) {
			dispatch_source_t ds = _dispatch_source_from_refs(dr);
			dispatch_clock_t clock = DISPATCH_TIMER_CLOCK(dr->du_ident);
			struct dispatch_trace_timer_params_s params;
			DISPATCH_TIMER_PROGRAM(ds, _dispatch_trace_timer_function(dr),
					_dispatch_trace_timer_params(clock, &dr->dt_timer,
					deadline, &params));
		}
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_wake(dispatch_timer_source_refs_t dr)
{
	if (unlikely(DISPATCH_TIMER_WAKE_ENABLED())) {
		if (dr) {
			dispatch_source_t ds = _dispatch_source_from_refs(dr);
			DISPATCH_TIMER_WAKE(ds, _dispatch_trace_timer_function(dr));
		}
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_fire(dispatch_timer_source_refs_t dr, uint64_t data,
		uint64_t missed)
{
	if (unlikely(DISPATCH_TIMER_FIRE_ENABLED())) {
		if (!(data - missed) && dr) {
			dispatch_source_t ds = _dispatch_source_from_refs(dr);
			DISPATCH_TIMER_FIRE(ds, _dispatch_trace_timer_function(dr));
		}
	}
}

#else

#define _dispatch_trace_timer_configure_enabled() false
#define _dispatch_trace_timer_configure(ds, clock, values) \
		do { (void)(ds); (void)(clock); (void)(values); } while(0)
#define _dispatch_trace_timer_program(dr, deadline) \
		do { (void)(dr); (void)(deadline); } while(0)
#define _dispatch_trace_timer_wake(dr) \
		do { (void)(dr); } while(0)
#define _dispatch_trace_timer_fire(dr, data, missed) \
		do { (void)(dr); (void)(data); (void)(missed); } while(0)

#endif // DISPATCH_USE_DTRACE

#endif // DISPATCH_PURE_C

#endif // __DISPATCH_TRACE__
