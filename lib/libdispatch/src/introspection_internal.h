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

#if DISPATCH_INTROSPECTION

#define DISPATCH_INTROSPECTION_QUEUE_LIST \
		TAILQ_ENTRY(dispatch_queue_s) diq_list
#define DISPATCH_INTROSPECTION_QUEUE_LIST_SIZE \
		sizeof(TAILQ_ENTRY(dispatch_queue_s))

void _dispatch_introspection_init(void);
void _dispatch_introspection_thread_add(void);
dispatch_queue_t _dispatch_introspection_queue_create(dispatch_queue_t dq);
void _dispatch_introspection_queue_dispose(dispatch_queue_t dq);
void _dispatch_introspection_queue_item_enqueue(dispatch_queue_t dq,
		dispatch_object_t dou);
void _dispatch_introspection_queue_item_dequeue(dispatch_queue_t dq,
		dispatch_object_t dou);
void _dispatch_introspection_queue_item_complete(dispatch_object_t dou);
void _dispatch_introspection_callout_entry(void *ctxt, dispatch_function_t f);
void _dispatch_introspection_callout_return(void *ctxt, dispatch_function_t f);

#if !__OBJC2__

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push_list(dispatch_queue_t dq,
		dispatch_object_t head, dispatch_object_t tail) {
	struct dispatch_object_s *dou = head._do;
	do {
		_dispatch_introspection_queue_item_enqueue(dq, dou);
	} while (dou != tail._do && (dou = dou->do_next));
};

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push(dispatch_queue_t dq, dispatch_object_t dou) {
	_dispatch_introspection_queue_item_enqueue(dq, dou);
};

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_pop(dispatch_queue_t dq, dispatch_object_t dou) {
	_dispatch_introspection_queue_item_dequeue(dq, dou);
};

#endif

#else

#define DISPATCH_INTROSPECTION_QUEUE_LIST
#define DISPATCH_INTROSPECTION_QUEUE_LIST_SIZE 0

#define _dispatch_introspection_init()
#define _dispatch_introspection_thread_add()
#define _dispatch_introspection_thread_remove()

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
_dispatch_introspection_queue_create(dispatch_queue_t dq) { return dq; }

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_dispose(dispatch_queue_t dq) { (void)dq; }

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push_list(dispatch_queue_t dq DISPATCH_UNUSED,
		dispatch_object_t head DISPATCH_UNUSED,
		dispatch_object_t tail DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_push(dispatch_queue_t dq DISPATCH_UNUSED,
		dispatch_object_t dou DISPATCH_UNUSED) {}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_introspection_queue_pop(dispatch_queue_t dq DISPATCH_UNUSED,
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

#endif // DISPATCH_INTROSPECTION

#endif // __DISPATCH_INTROSPECTION_INTERNAL__
