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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SOURCE_INTERNAL__
#define __DISPATCH_SOURCE_INTERNAL__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

#define DISPATCH_EVFILT_TIMER		(-EVFILT_SYSCOUNT - 1)
#define DISPATCH_EVFILT_CUSTOM_ADD	(-EVFILT_SYSCOUNT - 2)
#define DISPATCH_EVFILT_CUSTOM_OR	(-EVFILT_SYSCOUNT - 3)
#define DISPATCH_EVFILT_MACH_NOTIFICATION	(-EVFILT_SYSCOUNT - 4)
#define DISPATCH_EVFILT_SYSCOUNT	( EVFILT_SYSCOUNT + 4)

// NOTE: dispatch_source_mach_send_flags_t and dispatch_source_mach_recv_flags_t
//       bit values must not overlap as they share the same kevent fflags !

/*!
 * @enum dispatch_source_mach_send_flags_t
 *
 * @constant DISPATCH_MACH_SEND_DELETED
 * Port-deleted notification. Disabled for source registration.
 */
enum {
	DISPATCH_MACH_SEND_DELETED = 0x4,
};
/*!
 * @enum dispatch_source_mach_recv_flags_t
 *
 * @constant DISPATCH_MACH_RECV_MESSAGE
 * Receive right has pending messages
 *
 * @constant DISPATCH_MACH_RECV_MESSAGE_DIRECT
 * Receive messages from receive right directly via kevent64()
 *
 * @constant DISPATCH_MACH_RECV_NO_SENDERS
 * Receive right has no more senders. TODO <rdar://problem/8132399>
 */
enum {
	DISPATCH_MACH_RECV_MESSAGE = 0x2,
	DISPATCH_MACH_RECV_MESSAGE_DIRECT = 0x10,
	DISPATCH_MACH_RECV_MESSAGE_DIRECT_ONCE = 0x20,
	DISPATCH_MACH_RECV_NO_SENDERS = 0x40,
};

enum {
	DISPATCH_TIMER_WALL_CLOCK = 0x4,
	DISPATCH_TIMER_INTERVAL = 0x8,
	DISPATCH_TIMER_WITH_AGGREGATE = 0x10,
};

// low bits are timer QoS class
#define DISPATCH_TIMER_QOS_NORMAL 0u
#define DISPATCH_TIMER_QOS_CRITICAL 1u
#define DISPATCH_TIMER_QOS_BACKGROUND 2u
#define DISPATCH_TIMER_QOS_COUNT (DISPATCH_TIMER_QOS_BACKGROUND + 1)
#define DISPATCH_TIMER_QOS(tidx) ((uintptr_t)(tidx) & 0x3ul)

#define DISPATCH_TIMER_KIND_WALL 0u
#define DISPATCH_TIMER_KIND_MACH 1u
#define DISPATCH_TIMER_KIND_COUNT (DISPATCH_TIMER_KIND_MACH + 1)
#define DISPATCH_TIMER_KIND(tidx) (((uintptr_t)(tidx) >> 2) & 0x1ul)

#define DISPATCH_TIMER_INDEX(kind, qos) (((kind) << 2) | (qos))
#define DISPATCH_TIMER_INDEX_DISARM \
		DISPATCH_TIMER_INDEX(DISPATCH_TIMER_KIND_COUNT, 0)
#define DISPATCH_TIMER_INDEX_COUNT (DISPATCH_TIMER_INDEX_DISARM + 1)
#define DISPATCH_TIMER_IDENT(flags) ({ unsigned long f = (flags); \
		DISPATCH_TIMER_INDEX(f & DISPATCH_TIMER_WALL_CLOCK ? \
		DISPATCH_TIMER_KIND_WALL : DISPATCH_TIMER_KIND_MACH, \
		f & DISPATCH_TIMER_STRICT ? DISPATCH_TIMER_QOS_CRITICAL : \
		f & DISPATCH_TIMER_BACKGROUND ? DISPATCH_TIMER_QOS_BACKGROUND : \
		DISPATCH_TIMER_QOS_NORMAL); })


struct dispatch_kevent_s {
	TAILQ_ENTRY(dispatch_kevent_s) dk_list;
	TAILQ_HEAD(, dispatch_source_refs_s) dk_sources;
	struct kevent64_s dk_kevent;
};

typedef struct dispatch_kevent_s *dispatch_kevent_t;

struct dispatch_source_type_s {
	struct kevent64_s ke;
	uint64_t mask;
	void (*init)(dispatch_source_t ds, dispatch_source_type_t type,
			uintptr_t handle, unsigned long mask, dispatch_queue_t q);
};

struct dispatch_timer_source_s {
	uint64_t target;
	uint64_t deadline;
	uint64_t last_fire;
	uint64_t interval;
	uint64_t leeway;
	unsigned long flags; // dispatch_timer_flags_t
	unsigned long missed;
};

enum {
	DS_EVENT_HANDLER = 0,
	DS_CANCEL_HANDLER,
	DS_REGISTN_HANDLER,
};

// Source state which may contain references to the source object
// Separately allocated so that 'leaks' can see sources <rdar://problem/9050566>
typedef struct dispatch_source_refs_s {
	TAILQ_ENTRY(dispatch_source_refs_s) dr_list;
	uintptr_t dr_source_wref; // "weak" backref to dispatch_source_t
	dispatch_continuation_t ds_handler[3];
} *dispatch_source_refs_t;

typedef struct dispatch_timer_source_refs_s {
	struct dispatch_source_refs_s _ds_refs;
	struct dispatch_timer_source_s _ds_timer;
	TAILQ_ENTRY(dispatch_timer_source_refs_s) dt_list;
} *dispatch_timer_source_refs_t;

typedef struct dispatch_timer_source_aggregate_refs_s {
	struct dispatch_timer_source_refs_s _dsa_refs;
	TAILQ_ENTRY(dispatch_timer_source_aggregate_refs_s) dra_list;
	TAILQ_ENTRY(dispatch_timer_source_aggregate_refs_s) dta_list;
} *dispatch_timer_source_aggregate_refs_t;

#define _dispatch_ptr2wref(ptr) (~(uintptr_t)(ptr))
#define _dispatch_wref2ptr(ref) ((void*)~(ref))
#define _dispatch_source_from_refs(dr) \
		((dispatch_source_t)_dispatch_wref2ptr((dr)->dr_source_wref))
#define ds_timer(dr) \
		(((dispatch_timer_source_refs_t)(dr))->_ds_timer)
#define ds_timer_aggregate(ds) \
		((dispatch_timer_aggregate_t)((ds)->dq_specific_q))

DISPATCH_ALWAYS_INLINE
static inline unsigned int
_dispatch_source_timer_idx(dispatch_source_refs_t dr)
{
	return DISPATCH_TIMER_IDENT(ds_timer(dr).flags);
}

// ds_atomic_flags bits
#define DSF_CANCELED 1u // cancellation has been requested
#define DSF_ARMED 2u // source is armed

#define DISPATCH_SOURCE_HEADER(refs) \
	dispatch_kevent_t ds_dkev; \
	dispatch_##refs##_refs_t ds_refs; \
	unsigned int ds_atomic_flags; \
	unsigned int \
		ds_is_level:1, \
		ds_is_adder:1, \
		ds_is_installed:1, \
		ds_needs_rearm:1, \
		ds_is_timer:1, \
		ds_vmpressure_override:1, \
		ds_memorystatus_override:1, \
		dm_handler_is_block:1, \
		dm_connect_handler_called:1, \
		dm_cancel_handler_called:1; \
	unsigned long ds_pending_data_mask;

DISPATCH_CLASS_DECL(source);
struct dispatch_source_s {
	DISPATCH_STRUCT_HEADER(source);
	DISPATCH_QUEUE_HEADER;
	DISPATCH_SOURCE_HEADER(source);
	unsigned long ds_ident_hack;
	unsigned long ds_data;
	unsigned long ds_pending_data;
};

// Mach channel state which may contain references to the channel object
// layout must match dispatch_source_refs_s
struct dispatch_mach_refs_s {
	TAILQ_ENTRY(dispatch_mach_refs_s) dr_list;
	uintptr_t dr_source_wref; // "weak" backref to dispatch_mach_t
	dispatch_mach_handler_function_t dm_handler_func;
	void *dm_handler_ctxt;
};
typedef struct dispatch_mach_refs_s *dispatch_mach_refs_t;

struct dispatch_mach_reply_refs_s {
	TAILQ_ENTRY(dispatch_mach_reply_refs_s) dr_list;
	uintptr_t dr_source_wref; // "weak" backref to dispatch_mach_t
	dispatch_kevent_t dmr_dkev;
	void *dmr_ctxt;
	pthread_priority_t dmr_priority;
	voucher_t dmr_voucher;
	TAILQ_ENTRY(dispatch_mach_reply_refs_s) dmr_list;
};
typedef struct dispatch_mach_reply_refs_s *dispatch_mach_reply_refs_t;

struct dispatch_mach_send_refs_s {
	TAILQ_ENTRY(dispatch_mach_send_refs_s) dr_list;
	uintptr_t dr_source_wref; // "weak" backref to dispatch_mach_t
	dispatch_mach_msg_t dm_checkin;
	TAILQ_HEAD(, dispatch_mach_reply_refs_s) dm_replies;
	uint32_t volatile dm_disconnect_cnt;
	uint32_t volatile dm_sending;
	unsigned int dm_needs_mgr:1;
	struct dispatch_object_s *volatile dm_tail;
	struct dispatch_object_s *volatile dm_head;
	mach_port_t dm_send, dm_checkin_port;
};
typedef struct dispatch_mach_send_refs_s *dispatch_mach_send_refs_t;

DISPATCH_CLASS_DECL(mach);
struct dispatch_mach_s {
	DISPATCH_STRUCT_HEADER(mach);
	DISPATCH_QUEUE_HEADER;
	DISPATCH_SOURCE_HEADER(mach);
	dispatch_kevent_t dm_dkev;
	dispatch_mach_send_refs_t dm_refs;
};

DISPATCH_CLASS_DECL(mach_msg);
struct dispatch_mach_msg_s {
	DISPATCH_STRUCT_HEADER(mach_msg);
	mach_port_t dmsg_reply;
	pthread_priority_t dmsg_priority;
	voucher_t dmsg_voucher;
	dispatch_mach_msg_destructor_t dmsg_destructor;
	size_t dmsg_size;
	union {
		mach_msg_header_t *dmsg_msg;
		char dmsg_buf[0];
	};
};

#if TARGET_OS_EMBEDDED
#define DSL_HASH_SIZE  64u // must be a power of two
#else
#define DSL_HASH_SIZE 256u // must be a power of two
#endif

void _dispatch_source_xref_dispose(dispatch_source_t ds);
void _dispatch_source_dispose(dispatch_source_t ds);
void _dispatch_source_invoke(dispatch_source_t ds);
unsigned long _dispatch_source_probe(dispatch_source_t ds);
size_t _dispatch_source_debug(dispatch_source_t ds, char* buf, size_t bufsiz);
void _dispatch_source_set_interval(dispatch_source_t ds, uint64_t interval);
void _dispatch_source_set_event_handler_with_context_f(dispatch_source_t ds,
		void *ctxt, dispatch_function_t handler);

void _dispatch_mach_dispose(dispatch_mach_t dm);
void _dispatch_mach_invoke(dispatch_mach_t dm);
unsigned long _dispatch_mach_probe(dispatch_mach_t dm);
size_t _dispatch_mach_debug(dispatch_mach_t dm, char* buf, size_t bufsiz);

void _dispatch_mach_msg_dispose(dispatch_mach_msg_t dmsg);
void _dispatch_mach_msg_invoke(dispatch_mach_msg_t dmsg);
size_t _dispatch_mach_msg_debug(dispatch_mach_msg_t dmsg, char* buf, size_t bufsiz);

void _dispatch_mach_barrier_invoke(void *ctxt);

unsigned long _dispatch_mgr_wakeup(dispatch_queue_t dq);
void _dispatch_mgr_thread(dispatch_queue_t dq);

#endif /* __DISPATCH_SOURCE_INTERNAL__ */
