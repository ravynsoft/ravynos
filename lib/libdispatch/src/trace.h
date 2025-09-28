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

#if !__OBJC2__

#if DISPATCH_USE_DTRACE || DISPATCH_USE_DTRACE_INTROSPECTION
typedef struct dispatch_trace_timer_params_s {
	int64_t deadline, interval, leeway;
} *dispatch_trace_timer_params_t;

#include "provider.h"
#endif // DISPATCH_USE_DTRACE || DISPATCH_USE_DTRACE_INTROSPECTION

#if DISPATCH_USE_DTRACE_INTROSPECTION
#define _dispatch_trace_callout(_c, _f, _dcc) do { \
		if (slowpath(DISPATCH_CALLOUT_ENTRY_ENABLED()) || \
				slowpath(DISPATCH_CALLOUT_RETURN_ENABLED())) { \
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

#if DISPATCH_USE_DTRACE_INTROSPECTION
#define _dispatch_trace_continuation(_q, _o, _t) do { \
		dispatch_queue_t _dq = (_q); \
		const char *_label = _dq && _dq->dq_label ? _dq->dq_label : ""; \
		struct dispatch_object_s *_do = (_o); \
		dispatch_continuation_t _dc; \
		char *_kind; \
		dispatch_function_t _func; \
		void *_ctxt; \
		if (DISPATCH_OBJ_IS_VTABLE(_do)) { \
			_kind = (char*)dx_kind(_do); \
			if ((dx_type(_do) & _DISPATCH_META_TYPE_MASK) == \
					_DISPATCH_SOURCE_TYPE && (_dq) != &_dispatch_mgr_q) { \
				dispatch_source_t _ds = (dispatch_source_t)_do; \
				_dc = _ds->ds_refs->ds_handler[DS_EVENT_HANDLER]; \
				_func = _dc->dc_func; \
				_ctxt = _dc->dc_ctxt; \
			} else { \
				_func = (dispatch_function_t)_dispatch_queue_invoke; \
				_ctxt = _do->do_ctxt; \
			} \
		} else { \
			_dc = (void*)_do; \
			_ctxt = _dc->dc_ctxt; \
			if ((long)_dc->do_vtable & DISPATCH_OBJ_SYNC_SLOW_BIT) { \
				_kind = "semaphore"; \
				_func = (dispatch_function_t)dispatch_semaphore_signal; \
			} else if (_dc->dc_func == _dispatch_call_block_and_release) { \
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
static inline void
_dispatch_trace_queue_push_list(dispatch_queue_t dq, dispatch_object_t _head,
		dispatch_object_t _tail, pthread_priority_t pp, unsigned int n)
{
	if (slowpath(DISPATCH_QUEUE_PUSH_ENABLED())) {
		struct dispatch_object_s *dou = _head._do;
		do {
			_dispatch_trace_continuation(dq, dou, DISPATCH_QUEUE_PUSH);
		} while (dou != _tail._do && (dou = dou->do_next));
	}
	_dispatch_introspection_queue_push_list(dq, _head, _tail);
	_dispatch_queue_push_list(dq, _head, _tail, pp, n);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_queue_push(dispatch_queue_t dq, dispatch_object_t _tail, pthread_priority_t pp)
{
	if (slowpath(DISPATCH_QUEUE_PUSH_ENABLED())) {
		struct dispatch_object_s *dou = _tail._do;
		_dispatch_trace_continuation(dq, dou, DISPATCH_QUEUE_PUSH);
	}
	_dispatch_introspection_queue_push(dq, _tail);
	_dispatch_queue_push(dq, _tail, pp);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_queue_push_wakeup(dispatch_queue_t dq, dispatch_object_t _tail,
		pthread_priority_t pp, bool wakeup)
{
	if (slowpath(DISPATCH_QUEUE_PUSH_ENABLED())) {
		struct dispatch_object_s *dou = _tail._do;
		_dispatch_trace_continuation(dq, dou, DISPATCH_QUEUE_PUSH);
	}
	_dispatch_introspection_queue_push(dq, _tail);
	_dispatch_queue_push_wakeup(dq, _tail, pp, wakeup);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_continuation_push(dispatch_queue_t dq, dispatch_object_t _tail)
{
	if (slowpath(DISPATCH_QUEUE_PUSH_ENABLED())) {
		struct dispatch_object_s *dou = _tail._do;
		_dispatch_trace_continuation(dq, dou, DISPATCH_QUEUE_PUSH);
	}
	_dispatch_introspection_queue_push(dq, _tail);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_push_notrace(dispatch_queue_t dq, dispatch_object_t dou, pthread_priority_t pp)
{
	_dispatch_queue_push(dq, dou, pp);
}

#define _dispatch_queue_push_list _dispatch_trace_queue_push_list
#define _dispatch_queue_push _dispatch_trace_queue_push
#define _dispatch_queue_push_wakeup _dispatch_trace_queue_push_wakeup

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_continuation_pop(dispatch_queue_t dq, dispatch_object_t dou)
{
	if (slowpath(DISPATCH_QUEUE_POP_ENABLED())) {
		_dispatch_trace_continuation(dq, dou._do, DISPATCH_QUEUE_POP);
	}
	_dispatch_introspection_queue_pop(dq, dou);
}
#else
#define _dispatch_queue_push_notrace _dispatch_queue_push
#define _dispatch_trace_continuation_push(dq, dou) \
		do { (void)(dq); (void)(dou); } while(0)
#define _dispatch_trace_continuation_pop(dq, dou) \
		do { (void)(dq); (void)(dou); } while(0)
#endif // DISPATCH_USE_DTRACE_INTROSPECTION || DISPATCH_INTROSPECTION

#if DISPATCH_USE_DTRACE
static inline dispatch_function_t
_dispatch_trace_timer_function(dispatch_source_t ds, dispatch_source_refs_t dr)
{
	dispatch_continuation_t dc = dr->ds_handler[DS_EVENT_HANDLER];
	dispatch_function_t func = dc ? dc->dc_func : NULL;
	if (func == _dispatch_after_timer_callback &&
			!(ds->ds_atomic_flags & DSF_CANCELED)) {
		dc = ds->do_ctxt;
		func = dc->dc_func != _dispatch_call_block_and_release ? dc->dc_func :
				dc->dc_ctxt ? _dispatch_Block_invoke(dc->dc_ctxt) : NULL;
	}
	return func;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_trace_timer_params_t
_dispatch_trace_timer_params(uintptr_t ident,
		struct dispatch_timer_source_s *values, uint64_t deadline,
		dispatch_trace_timer_params_t params)
{
	#define _dispatch_trace_time2nano3(t) (DISPATCH_TIMER_KIND(ident) \
			== DISPATCH_TIMER_KIND_MACH ? _dispatch_time_mach2nano(t) : (t))
	#define _dispatch_trace_time2nano2(v, t) ({ uint64_t _t = (t); \
			(v) >= INT64_MAX ? -1ll : (int64_t)_dispatch_trace_time2nano3(_t);})
	#define _dispatch_trace_time2nano(v) ({ uint64_t _t; \
			_t = _dispatch_trace_time2nano3(v); _t >= INT64_MAX ? -1ll : \
			(int64_t)_t; })
	if (deadline) {
		params->deadline = (int64_t)deadline;
	} else {
		uint64_t now = (DISPATCH_TIMER_KIND(ident) ==
				DISPATCH_TIMER_KIND_MACH ? _dispatch_absolute_time() :
				 _dispatch_get_nanoseconds());
		params->deadline = _dispatch_trace_time2nano2(values->target,
				values->target < now ? 0 : values->target - now);
	}
	params->interval = _dispatch_trace_time2nano(values->interval);
	params->leeway = _dispatch_trace_time2nano(values->leeway);
	return params;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_trace_timer_configure_enabled(void)
{
	return slowpath(DISPATCH_TIMER_CONFIGURE_ENABLED());
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_configure(dispatch_source_t ds, uintptr_t ident,
		struct dispatch_timer_source_s *values)
{
	struct dispatch_trace_timer_params_s params;
	DISPATCH_TIMER_CONFIGURE(ds, _dispatch_trace_timer_function(ds,
			ds->ds_refs), _dispatch_trace_timer_params(ident, values, 0,
			&params));
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_program(dispatch_source_refs_t dr, uint64_t deadline)
{
	if (slowpath(DISPATCH_TIMER_PROGRAM_ENABLED())) {
		if (deadline && dr) {
			dispatch_source_t ds = _dispatch_source_from_refs(dr);
			struct dispatch_trace_timer_params_s params;
			DISPATCH_TIMER_PROGRAM(ds, _dispatch_trace_timer_function(ds, dr),
					_dispatch_trace_timer_params(ds->ds_ident_hack,
					&ds_timer(dr), deadline, &params));
		}
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_wake(dispatch_source_refs_t dr)
{
	if (slowpath(DISPATCH_TIMER_WAKE_ENABLED())) {
		if (dr) {
			dispatch_source_t ds = _dispatch_source_from_refs(dr);
			DISPATCH_TIMER_WAKE(ds, _dispatch_trace_timer_function(ds, dr));
		}
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_trace_timer_fire(dispatch_source_refs_t dr, unsigned long data,
		unsigned long missed)
{
	if (slowpath(DISPATCH_TIMER_FIRE_ENABLED())) {
		if (!(data - missed) && dr) {
			dispatch_source_t ds = _dispatch_source_from_refs(dr);
			DISPATCH_TIMER_FIRE(ds, _dispatch_trace_timer_function(ds, dr));
		}
	}
}

#else

#define _dispatch_trace_timer_configure_enabled() false
#define _dispatch_trace_timer_configure(ds, ident, values) \
		do { (void)(ds); (void)(ident); (void)(values); } while(0)
#define _dispatch_trace_timer_program(dr, deadline) \
		do { (void)(dr); (void)(deadline); } while(0)
#define _dispatch_trace_timer_wake(dr) \
		do { (void)(dr); } while(0)
#define _dispatch_trace_timer_fire(dr, data, missed) \
		do { (void)(dr); (void)(data); (void)(missed); } while(0)

#endif // DISPATCH_USE_DTRACE

#endif // !__OBJC2__

#endif // __DISPATCH_TRACE__
