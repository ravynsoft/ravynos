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

#pragma mark -
#pragma mark _os_object_t

unsigned long
_os_object_retain_count(_os_object_t obj)
{
	int xref_cnt = obj->os_obj_xref_cnt;
	if (unlikely(xref_cnt == _OS_OBJECT_GLOBAL_REFCNT)) {
		return ULONG_MAX; // global object
	}
	return (unsigned long)(xref_cnt + 1);
}

DISPATCH_NOINLINE
_os_object_t
_os_object_retain_internal(_os_object_t obj)
{
	return _os_object_retain_internal_n_inline(obj, 1);
}

DISPATCH_NOINLINE
_os_object_t
_os_object_retain_internal_n(_os_object_t obj, uint16_t n)
{
	return _os_object_retain_internal_n_inline(obj, n);
}

DISPATCH_NOINLINE
void
_os_object_release_internal(_os_object_t obj)
{
	return _os_object_release_internal_n_inline(obj, 1);
}

DISPATCH_NOINLINE
void
_os_object_release_internal_n(_os_object_t obj, uint16_t n)
{
	return _os_object_release_internal_n_inline(obj, n);
}

DISPATCH_NOINLINE
_os_object_t
_os_object_retain(_os_object_t obj)
{
	int xref_cnt = _os_object_xrefcnt_inc_orig(obj);
	if (unlikely(xref_cnt < 0)) {
		_OS_OBJECT_CLIENT_CRASH("Resurrection of an object");
	}
	return obj;
}

DISPATCH_NOINLINE
_os_object_t
_os_object_retain_with_resurrect(_os_object_t obj)
{
	int xref_cnt = _os_object_xrefcnt_inc_orig(obj) + 1;
	if (unlikely(xref_cnt < 0)) {
		_OS_OBJECT_CLIENT_CRASH("Resurrection of an over-released object");
	}
	if (unlikely(xref_cnt == 0)) {
		_os_object_retain_internal(obj);
	}
	return obj;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_os_object_release_inline(_os_object_t obj)
{
	int xref_cnt = _os_object_xrefcnt_dec(obj);
	if (likely(xref_cnt >= 0)) {
		return false;
	}
	if (unlikely(xref_cnt < -1)) {
		_OS_OBJECT_CLIENT_CRASH("Over-release of an object");
	}
	return true;
}


DISPATCH_NOINLINE
void
_os_object_release(_os_object_t obj)
{
	if (_os_object_release_inline(obj)) {
		return _os_object_xref_dispose(obj);
	}
}

DISPATCH_NOINLINE
void
_os_object_release_without_xref_dispose(_os_object_t obj)
{
	if (_os_object_release_inline(obj)) {
		return _os_object_release_internal(obj);
	}
}

bool
_os_object_retain_weak(_os_object_t obj)
{
	int xref_cnt, nxref_cnt;
	os_atomic_rmw_loop2o(obj, os_obj_xref_cnt, xref_cnt, nxref_cnt, relaxed, {
		if (unlikely(xref_cnt == _OS_OBJECT_GLOBAL_REFCNT)) {
			os_atomic_rmw_loop_give_up(return true); // global object
		}
		if (unlikely(xref_cnt == -1)) {
			os_atomic_rmw_loop_give_up(return false);
		}
		if (unlikely(xref_cnt < -1)) {
			os_atomic_rmw_loop_give_up(goto overrelease);
		}
		nxref_cnt = xref_cnt + 1;
	});
	return true;
overrelease:
	_OS_OBJECT_CLIENT_CRASH("Over-release of an object");
}

bool
_os_object_allows_weak_reference(_os_object_t obj)
{
	int xref_cnt = obj->os_obj_xref_cnt;
	if (unlikely(xref_cnt == -1)) {
		return false;
	}
	if (unlikely(xref_cnt < -1)) {
		_OS_OBJECT_CLIENT_CRASH("Over-release of an object");
	}
	return true;
}

#pragma mark -
#pragma mark dispatch_object_t

void *
_dispatch_object_alloc(const void *vtable, size_t size)
{
#if OS_OBJECT_HAVE_OBJC1
	const struct dispatch_object_vtable_s *_vtable = vtable;
	dispatch_object_t dou;
	dou._os_obj = _os_object_alloc_realized(_vtable->_os_obj_objc_isa, size);
	dou._do->do_vtable = vtable;
	return dou._do;
#else
	return _os_object_alloc_realized(vtable, size);
#endif
}

void
_dispatch_object_finalize(dispatch_object_t dou)
{
#if USE_OBJC
	objc_destructInstance((id)dou._do);
#else
	(void)dou;
#endif
}

void
_dispatch_object_dealloc(dispatch_object_t dou)
{
	// so that ddt doesn't pick up bad objects when malloc reuses this memory
	dou._os_obj->os_obj_isa = NULL;
#if OS_OBJECT_HAVE_OBJC1
	dou._do->do_vtable = NULL;
#endif
	free(dou._os_obj);
}

void
dispatch_retain(dispatch_object_t dou)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_retain, dou);
	(void)_os_object_retain(dou._os_obj);
}

void
dispatch_release(dispatch_object_t dou)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_release, dou);
	if (_os_object_release_inline(dou._os_obj)) {
		// bypass -_xref_dispose to avoid the dynamic dispatch
		_os_object_xrefcnt_dispose_barrier(dou._os_obj);
		_dispatch_xref_dispose(dou);
	}
}

void
_dispatch_xref_dispose(dispatch_object_t dou)
{
	if (dx_cluster(dou._do) == _DISPATCH_QUEUE_CLUSTER) {
		_dispatch_queue_xref_dispose(dou._dq);
		switch (dx_type(dou._do)) {
		case DISPATCH_SOURCE_KEVENT_TYPE:
			_dispatch_source_xref_dispose(dou._ds);
			break;
		case DISPATCH_CHANNEL_TYPE:
			_dispatch_channel_xref_dispose(dou._dch);
			break;
#if HAVE_MACH
		case DISPATCH_MACH_CHANNEL_TYPE:
			_dispatch_mach_xref_dispose(dou._dm);
			break;
#endif
		case DISPATCH_QUEUE_RUNLOOP_TYPE:
			_dispatch_runloop_queue_xref_dispose(dou._dl);
			break;
		}
	}
	return _dispatch_release_tailcall(dou._os_obj);
}

void
_dispatch_dispose(dispatch_object_t dou)
{
	dispatch_queue_t tq = dou._do->do_targetq;
	dispatch_function_t func = _dispatch_object_finalizer(dou);
	void *ctxt = dou._do->do_ctxt;
	bool allow_free = true;

	if (unlikely(dou._do->do_next != DISPATCH_OBJECT_LISTLESS)) {
		DISPATCH_INTERNAL_CRASH(dou._do->do_next, "Release while enqueued");
	}

	if (unlikely(tq && tq->dq_serialnum == DISPATCH_QUEUE_SERIAL_NUMBER_WLF)) {
		// the workloop fallback global queue is never serviced, so redirect
		// the finalizer onto a global queue
		tq = _dispatch_get_root_queue(DISPATCH_QOS_DEFAULT, false)->_as_dq;
	}

	dx_dispose(dou._do, &allow_free);

	// Past this point, the only thing left of the object is its memory
	if (likely(allow_free)) {
		_dispatch_object_finalize(dou);
		_dispatch_object_dealloc(dou);
	}
	if (func && ctxt) {
		dispatch_async_f(tq, ctxt, func);
	}
	if (tq) _dispatch_release_tailcall(tq);
}

void *
dispatch_get_context(dispatch_object_t dou)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_get_context, dou);
	if (unlikely(dx_hastypeflag(dou._do, NO_CONTEXT))) {
		return NULL;
	}
	return dou._do->do_ctxt;
}

void
dispatch_set_context(dispatch_object_t dou, void *context)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_set_context, dou, context);
	if (unlikely(dx_hastypeflag(dou._do, NO_CONTEXT))) {
		return;
	}
	dou._do->do_ctxt = context;
}

void
dispatch_set_finalizer_f(dispatch_object_t dou, dispatch_function_t finalizer)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_set_finalizer_f, dou, finalizer);
	if (unlikely(dx_hastypeflag(dou._do, NO_CONTEXT))) {
		return;
	}
	_dispatch_object_set_finalizer(dou, finalizer);
}

void
dispatch_set_target_queue(dispatch_object_t dou, dispatch_queue_t tq)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_set_target_queue, dou, tq);
	if (unlikely(_dispatch_object_is_global(dou) ||
			_dispatch_object_is_root_or_base_queue(dou))) {
		return;
	}
	if (dx_cluster(dou._do) == _DISPATCH_QUEUE_CLUSTER) {
		return _dispatch_lane_set_target_queue(dou._dl, tq);
	}
	if (dx_type(dou._do) == DISPATCH_IO_TYPE) {
		// <rdar://problem/34417216> FIXME: dispatch IO should be a "source"
		return _dispatch_io_set_target_queue(dou._dchannel, tq);
	}
	if (tq == DISPATCH_TARGET_QUEUE_DEFAULT) {
		tq = _dispatch_get_default_queue(false);
	}
	_dispatch_object_set_target_queue_inline(dou._do, tq);
}

void
dispatch_activate(dispatch_object_t dou)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_activate, dou);
	if (unlikely(_dispatch_object_is_global(dou))) {
		return;
	}
	if (dx_metatype(dou._do) == _DISPATCH_WORKLOOP_TYPE) {
		return _dispatch_workloop_activate(dou._dwl);
	}
	if (dx_cluster(dou._do) == _DISPATCH_QUEUE_CLUSTER) {
		return _dispatch_lane_resume(dou._dl, DISPATCH_ACTIVATE);
	}
}

void
dispatch_suspend(dispatch_object_t dou)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_suspend, dou);
	if (unlikely(_dispatch_object_is_global(dou) ||
			_dispatch_object_is_root_or_base_queue(dou))) {
		return;
	}
	if (dx_cluster(dou._do) == _DISPATCH_QUEUE_CLUSTER) {
		return _dispatch_lane_suspend(dou._dl);
	}
}

void
dispatch_resume(dispatch_object_t dou)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_resume, dou);
	if (unlikely(_dispatch_object_is_global(dou) ||
			_dispatch_object_is_root_or_base_queue(dou))) {
		return;
	}
	if (dx_cluster(dou._do) == _DISPATCH_QUEUE_CLUSTER) {
		_dispatch_lane_resume(dou._dl, DISPATCH_RESUME);
	}
}

size_t
_dispatch_object_debug_attr(dispatch_object_t dou, char* buf, size_t bufsiz)
{
	return dsnprintf(buf, bufsiz, "xref = %d, ref = %d, ",
			dou._do->do_xref_cnt + 1, dou._do->do_ref_cnt + 1);
}
