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

#ifndef __DISPATCH_INTROSPECTION_INTERNAL__
#define __DISPATCH_INTROSPECTION_INTERNAL__

/* keep in sync with introspection_private.h */
enum dispatch_introspection_runtime_event {
	dispatch_introspection_runtime_event_worker_event_delivery = 1,
	dispatch_introspection_runtime_event_worker_unpark = 2,
	dispatch_introspection_runtime_event_worker_request = 3,
	dispatch_introspection_runtime_event_worker_park = 4,

	dispatch_introspection_runtime_event_sync_wait = 10,
	dispatch_introspection_runtime_event_async_sync_handoff = 11,
	dispatch_introspection_runtime_event_sync_sync_handoff = 12,
	dispatch_introspection_runtime_event_sync_async_handoff = 13,
};

#if DISPATCH_INTROSPECTION

#define DC_BARRIER 0x1
#define DC_SYNC 0x2
#define DC_APPLY 0x4

typedef struct dispatch_queue_introspection_context_s {
	dispatch_queue_class_t dqic_queue;
	dispatch_function_t dqic_finalizer;
	LIST_ENTRY(dispatch_queue_introspection_context_s) dqic_list;

	char __dqic_no_queue_inversion[0];

	// used for queue inversion debugging only
	dispatch_unfair_lock_s dqic_order_top_head_lock;
	dispatch_unfair_lock_s dqic_order_bottom_head_lock;
	LIST_HEAD(, dispatch_queue_order_entry_s) dqic_order_top_head;
	LIST_HEAD(, dispatch_queue_order_entry_s) dqic_order_bottom_head;
} *dispatch_queue_introspection_context_t;

struct dispatch_introspection_state_s {
	LIST_HEAD(, dispatch_introspection_thread_s) threads;
	LIST_HEAD(, dispatch_queue_introspection_context_s) queues;
	dispatch_unfair_lock_s threads_lock;
	dispatch_unfair_lock_s queues_lock;

	ptrdiff_t thread_queue_offset;

	// dispatch introspection features
	bool debug_queue_inversions; // DISPATCH_DEBUG_QUEUE_INVERSIONS
};

extern struct dispatch_introspection_state_s _dispatch_introspection;

void _dispatch_introspection_init(void);
void _dispatch_introspection_thread_add(void);
dispatch_function_t _dispatch_object_finalizer(dispatch_object_t dou);
void _dispatch_object_set_finalizer(dispatch_object_t dou,
		dispatch_function_t finalizer);
dispatch_queue_class_t _dispatch_introspection_queue_create(
		dispatch_queue_class_t dqu);
void _dispatch_introspection_queue_dispose(dispatch_queue_class_t dqu);
void _dispatch_introspection_queue_item_enqueue(dispatch_queue_class_t dqu,
		dispatch_object_t dou);
void _dispatch_introspection_queue_item_dequeue(dispatch_queue_class_t dqu,
		dispatch_object_t dou);
void _dispatch_introspection_queue_item_complete(dispatch_object_t dou);
void _dispatch_introspection_callout_entry(void *ctxt, dispatch_function_t f);
void _dispatch_introspection_callout_return(void *ctxt, dispatch_function_t f);
struct dispatch_object_s *_dispatch_introspection_queue_fake_sync_push_pop(
		dispatch_queue_t dq, void *ctxt, dispatch_function_t func,
		uintptr_t dc_flags);
void _dispatch_introspection_runtime_event(
		enum dispatch_introspection_runtime_event event,
		void *ptr, uint64_t value);

#if DISPATCH_PURE_C

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push_list(dispatch_queue_class_t dqu,
		dispatch_object_t head, dispatch_object_t tail) {
	struct dispatch_object_s *dou = head._do;
	do {
		_dispatch_introspection_queue_item_enqueue(dqu, dou);
	} while (dou != tail._do && (dou = dou->do_next));
};

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push(dispatch_queue_class_t dqu,
		dispatch_object_t dou)
{
	_dispatch_introspection_queue_item_enqueue(dqu, dou);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_pop(dispatch_queue_class_t dqu,
		dispatch_object_t dou)
{
	_dispatch_introspection_queue_item_dequeue(dqu, dou);
}

void
_dispatch_introspection_order_record(dispatch_queue_t top_q);

void
_dispatch_introspection_target_queue_changed(dispatch_queue_t dq);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_sync_begin(dispatch_queue_class_t dq)
{
	if (!_dispatch_introspection.debug_queue_inversions) return;
	_dispatch_introspection_order_record(dq._dq);
}

#endif // DISPATCH_PURE_C

#else // DISPATCH_INTROSPECTION

#define _dispatch_introspection_init()
#define _dispatch_introspection_thread_add()

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_class_t
_dispatch_introspection_queue_create(dispatch_queue_class_t dqu)
{
	return dqu;
}

#if DISPATCH_PURE_C

DISPATCH_ALWAYS_INLINE
static inline dispatch_function_t
_dispatch_object_finalizer(dispatch_object_t dou)
{
	return dou._do->do_finalizer;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_object_set_finalizer(dispatch_object_t dou,
		dispatch_function_t finalizer)
{
	dou._do->do_finalizer = finalizer;
}

#endif // DISPATCH_PURE_C

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_dispose(
		dispatch_queue_class_t dqu DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push_list(
		dispatch_queue_class_t dqu DISPATCH_UNUSED,
		dispatch_object_t head DISPATCH_UNUSED,
		dispatch_object_t tail DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push(dispatch_queue_class_t dqu DISPATCH_UNUSED,
		dispatch_object_t dou DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_pop(dispatch_queue_class_t dqu DISPATCH_UNUSED,
		dispatch_object_t dou DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_item_complete(
		dispatch_object_t dou DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_callout_entry(void *ctxt DISPATCH_UNUSED,
		dispatch_function_t f DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_callout_return(void *ctxt DISPATCH_UNUSED,
		dispatch_function_t f DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_target_queue_changed(
		dispatch_queue_t dq DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_sync_begin(
		dispatch_queue_class_t dq DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline struct dispatch_object_s *
_dispatch_introspection_queue_fake_sync_push_pop(
		dispatch_queue_t dq DISPATCH_UNUSED,
		void *ctxt DISPATCH_UNUSED, dispatch_function_t func DISPATCH_UNUSED,
		uintptr_t dc_flags DISPATCH_UNUSED) { return NULL; }

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_runtime_event(
		enum dispatch_introspection_runtime_event event DISPATCH_UNUSED,
		void *ptr DISPATCH_UNUSED, uint64_t value DISPATCH_UNUSED) {}

#endif // DISPATCH_INTROSPECTION

#endif // __DISPATCH_INTROSPECTION_INTERNAL__
