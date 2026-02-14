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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_EVENT_EVENT_INTERNAL__
#define __DISPATCH_EVENT_EVENT_INTERNAL__

#include "event_config.h"

/*
 * The unote state has 3 pieces of information and reflects the state
 * of the unote registration and mirrors the state of the knote if any.
 *
 * This state is peculiar in the sense that it can be read concurrently, but
 * is never written to concurrently. This is achieved by serializing through
 * kevent calls from appropriate synchronization context (referred as `dkq`
 * for dispatch kevent queue in the dispatch source code).
 *
 * DU_STATE_ARMED
 *
 *   This bit represents the fact that the registration is active and may
 *   receive events at any given time. This bit can only be set if the WLH bits
 *   are set and the DU_STATE_NEEDS_DELETE bit is not.
 *
 * DU_STATE_NEEDS_DELETE
 *
 *   The kernel has indicated that it wants the next event for this unote to be
 *   an unregistration. This bit can only be set if the DU_STATE_ARMED bit is
 *   not set.
 *
 *   DU_STATE_NEEDS_DELETE may be the only bit set in the unote state
 *
 * DU_STATE_WLH_MASK
 *
 *   The most significant bits of du_state represent which event loop this unote
 *   is registered with, and has a storage reference on it taken with
 *   _dispatch_wlh_retain().
 *
 * Registration
 *
 *   Unote registration attempt is made with _dispatch_unote_register().
 *   On succes, it will set the WLH bits and the DU_STATE_ARMED bit, on failure
 *   the state is 0.
 *
 *   _dispatch_unote_register() must be called from the appropriate
 *   synchronization context depending on the unote type.
 *
 * Event delivery
 *
 *   When an event is delivered for a unote type that requires explicit
 *   re-arming (EV_DISPATCH or EV_ONESHOT), the DU_STATE_ARMED bit is cleared.
 *   If the event is marked as EV_ONESHOT, then the DU_STATE_NEEDS_DELETE bit
 *   is also set, initiating the "deferred delete" state machine.
 *
 *   For other unote types, the state isn't touched, unless the event is
 *   EV_ONESHOT, in which case it causes an automatic unregistration.
 *
 * Unregistration
 *
 *   The unote owner can attempt unregistering the unote with
 *   _dispatch_unote_unregister() from the proper synchronization context
 *   at any given time. When successful, the state will be set to 0 and the
 *   unote is no longer active. Unregistration is always successful for events
 *   that don't require explcit re-arming.
 *
 *   When this unregistration fails, then the unote owner must wait for the
 *   next event delivery for this unote.
 */
typedef uintptr_t dispatch_unote_state_t;
#define DU_STATE_ARMED            ((dispatch_unote_state_t)0x1ul)
#define DU_STATE_NEEDS_DELETE     ((dispatch_unote_state_t)0x2ul)
#define DU_STATE_WLH_MASK         ((dispatch_unote_state_t)~0x3ul)
#define DU_STATE_UNREGISTERED     ((dispatch_unote_state_t)0)

struct dispatch_sync_context_s;
typedef struct dispatch_wlh_s *dispatch_wlh_t; // opaque handle
#define DISPATCH_WLH_ANON       ((dispatch_wlh_t)(void*)(~0x3ul))
#define DISPATCH_WLH_MANAGER    ((dispatch_wlh_t)(void*)(~0x7ul))

DISPATCH_OPTIONS(dispatch_unote_timer_flags, uint8_t,
	/* DISPATCH_TIMER_STRICT 0x1 */
	/* DISPATCH_TIMER_BACKGROUND = 0x2, */
	DISPATCH_TIMER_CLOCK_UPTIME = DISPATCH_CLOCK_UPTIME << 2,
	DISPATCH_TIMER_CLOCK_MONOTONIC = DISPATCH_CLOCK_MONOTONIC << 2,
	DISPATCH_TIMER_CLOCK_WALL = DISPATCH_CLOCK_WALL << 2,
#define _DISPATCH_TIMER_CLOCK_MASK (0x3 << 2)
	DISPATCH_TIMER_INTERVAL = 0x10,
	/* DISPATCH_INTERVAL_UI_ANIMATION = 0x20 */ // See source_private.h
	DISPATCH_TIMER_AFTER = 0x40,
);

static inline dispatch_clock_t
_dispatch_timer_flags_to_clock(dispatch_unote_timer_flags_t flags)
{
	return (dispatch_clock_t)((flags & _DISPATCH_TIMER_CLOCK_MASK) >> 2);
}

static inline dispatch_unote_timer_flags_t
_dispatch_timer_flags_from_clock(dispatch_clock_t clock)
{
	return (dispatch_unote_timer_flags_t)(clock << 2);
}

#define DISPATCH_UNOTE_CLASS_HEADER() \
	dispatch_source_type_t du_type; \
	uintptr_t du_owner_wref; /* "weak" back reference to the owner object */ \
	os_atomic(dispatch_unote_state_t) du_state; \
	uint32_t  du_ident; \
	int8_t    du_filter; \
	uint8_t   du_is_direct : 1; \
	uint8_t   du_is_timer : 1; \
	uint8_t   du_has_extended_status : 1; \
	uint8_t   du_memorypressure_override : 1; \
	uint8_t   du_vmpressure_override : 1; \
	uint8_t   du_can_be_wlh : 1; \
	uint8_t   dmrr_handler_is_block : 1; \
	uint8_t   du_unused_flag : 1; \
	union { \
		uint8_t   du_timer_flags; \
		os_atomic(bool) dmsr_notification_armed; \
		bool dmr_reply_port_owned; \
	}; \
	uint8_t   du_unused; \
	uint32_t  du_fflags; \
	dispatch_priority_t du_priority

#define _dispatch_ptr2wref(ptr) (~(uintptr_t)(ptr))
#define _dispatch_wref2ptr(ref) ((void*)~(ref))
#define _dispatch_source_from_refs(dr) \
		((dispatch_source_t)_dispatch_wref2ptr((dr)->du_owner_wref))

typedef struct dispatch_unote_class_s {
	DISPATCH_UNOTE_CLASS_HEADER();
} *dispatch_unote_class_t;

enum {
	DS_EVENT_HANDLER = 0,
	DS_CANCEL_HANDLER,
	DS_REGISTN_HANDLER,
};

#define DISPATCH_SOURCE_REFS_HEADER() \
	DISPATCH_UNOTE_CLASS_HEADER(); \
	struct dispatch_continuation_s *volatile ds_handler[3]; \
	uint64_t ds_data DISPATCH_ATOMIC64_ALIGN; \
	uint64_t ds_pending_data DISPATCH_ATOMIC64_ALIGN


// Extracts source data from the ds_data field
#define DISPATCH_SOURCE_GET_DATA(d) ((d) & 0xFFFFFFFF)

// Extracts status from the ds_data field
#define DISPATCH_SOURCE_GET_STATUS(d) ((d) >> 32)

// Combine data and status for the ds_data field
#define DISPATCH_SOURCE_COMBINE_DATA_AND_STATUS(data, status) \
		((((uint64_t)(status)) << 32) | (data))

#define DISPATCH_TIMER_DISARMED_MARKER  1ul


// Source state which may contain references to the source object
// Separately allocated so that 'leaks' can see sources <rdar://problem/9050566>
typedef struct dispatch_source_refs_s {
	DISPATCH_SOURCE_REFS_HEADER();
} *dispatch_source_refs_t;

typedef struct dispatch_timer_delay_s {
	uint64_t delay, leeway;
} dispatch_timer_delay_s;

#define DTH_INVALID_ID  (~0u)
#define DTH_TARGET_ID   0u
#define DTH_DEADLINE_ID 1u
#define DTH_ID_COUNT    2u

typedef struct dispatch_timer_source_s {
	union {
		struct {
			uint64_t target;
			uint64_t deadline;
		};
		uint64_t heap_key[DTH_ID_COUNT];
	};
	uint64_t interval;
} *dispatch_timer_source_t;

typedef struct dispatch_timer_config_s {
	struct dispatch_timer_source_s dtc_timer;
	dispatch_clock_t dtc_clock;
} *dispatch_timer_config_t;

typedef struct dispatch_timer_source_refs_s {
	DISPATCH_SOURCE_REFS_HEADER();
	struct dispatch_timer_source_s dt_timer;
	struct dispatch_timer_config_s *dt_pending_config;
	uint32_t dt_heap_entry[DTH_ID_COUNT];
} *dispatch_timer_source_refs_t;

typedef struct dispatch_timer_heap_s {
	uint32_t dth_count;
	uint8_t dth_segments;
	uint8_t dth_max_qos;
#define DTH_DIRTY_GLOBAL   0x80
#define DTH_DIRTY_QOS_MASK ((1u << DISPATCH_TIMER_QOS_COUNT) - 1)
	uint8_t dth_dirty_bits; // Only used in the first heap
	uint8_t dth_armed : 1;
	uint8_t dth_needs_program : 1;
	dispatch_timer_source_refs_t dth_min[DTH_ID_COUNT];
	void **dth_heap;
} *dispatch_timer_heap_t;

#if HAVE_MACH
#if DISPATCH_MACHPORT_DEBUG
void dispatch_debug_machport(mach_port_t name, const char *str);
#define _dispatch_debug_machport(name) \
		dispatch_debug_machport((name), __func__)
#else
#define _dispatch_debug_machport(name) ((void)(name))
#endif // DISPATCH_MACHPORT_DEBUG

// Mach channel state which may contain references to the channel object
// layout must match dispatch_source_refs_s
struct dispatch_mach_recv_refs_s {
	DISPATCH_UNOTE_CLASS_HEADER();
	dispatch_mach_handler_function_t dmrr_handler_func;
	void *dmrr_handler_ctxt;
};
typedef struct dispatch_mach_recv_refs_s *dispatch_mach_recv_refs_t;

struct dispatch_mach_reply_refs_s {
	DISPATCH_UNOTE_CLASS_HEADER();
	pthread_priority_t dmr_priority : 32;
	void *dmr_ctxt;
	voucher_t dmr_voucher;
	LIST_ENTRY(dispatch_mach_reply_refs_s) dmr_list;
};
typedef struct dispatch_mach_reply_refs_s *dispatch_mach_reply_refs_t;

struct dispatch_mach_reply_wait_refs_s {
	struct dispatch_mach_reply_refs_s dwr_refs;
	mach_port_t dwr_waiter_tid;
};
typedef struct dispatch_mach_reply_wait_refs_s *dispatch_mach_reply_wait_refs_t;

#define _DISPATCH_MACH_STATE_UNUSED_MASK        0xffffff8000000000ull
#define DISPATCH_MACH_STATE_ENQUEUED            0x0000008000000000ull
#define DISPATCH_MACH_STATE_DIRTY               0x0000002000000000ull
#define DISPATCH_MACH_STATE_PENDING_BARRIER     0x0000001000000000ull
#define DISPATCH_MACH_STATE_RECEIVED_OVERRIDE   0x0000000800000000ull
#define DISPATCH_MACH_STATE_MAX_QOS_MASK        0x0000000700000000ull
#define DISPATCH_MACH_STATE_MAX_QOS_SHIFT       32
#define DISPATCH_MACH_STATE_UNLOCK_MASK         0x00000000ffffffffull

struct dispatch_mach_send_refs_s {
	DISPATCH_UNOTE_CLASS_HEADER();
	dispatch_unfair_lock_s dmsr_replies_lock;
	dispatch_mach_msg_t dmsr_checkin;
	LIST_HEAD(, dispatch_mach_reply_refs_s) dmsr_replies;
#define DISPATCH_MACH_NEVER_CONNECTED      0x80000000
	DISPATCH_UNION_LE(uint64_t volatile dmsr_state,
		dispatch_unfair_lock_s dmsr_state_lock,
		uint32_t dmsr_state_bits
	) DISPATCH_ATOMIC64_ALIGN;
	struct dispatch_object_s *volatile dmsr_tail;
	struct dispatch_object_s *volatile dmsr_head;
	uint32_t volatile dmsr_disconnect_cnt;
	mach_port_t dmsr_send, dmsr_checkin_port;
};
typedef struct dispatch_mach_send_refs_s *dispatch_mach_send_refs_t;

bool _dispatch_mach_notification_armed(dispatch_mach_send_refs_t dmsr);
void _dispatch_mach_notification_set_armed(dispatch_mach_send_refs_t dmsr);

struct dispatch_xpc_term_refs_s {
	DISPATCH_UNOTE_CLASS_HEADER();
};
typedef struct dispatch_xpc_term_refs_s *dispatch_xpc_term_refs_t;
void _dispatch_sync_ipc_handoff_begin(dispatch_wlh_t wlh, mach_port_t port,
		uint64_t _Atomic *addr);
void _dispatch_sync_ipc_handoff_end(dispatch_wlh_t wlh, mach_port_t port);
#endif // HAVE_MACH

typedef union dispatch_unote_u {
	dispatch_unote_class_t _du;
	dispatch_source_refs_t _dr;
	dispatch_timer_source_refs_t _dt;
#if HAVE_MACH
	dispatch_mach_recv_refs_t _dmrr;
	dispatch_mach_send_refs_t _dmsr;
	dispatch_mach_reply_refs_t _dmr;
	dispatch_xpc_term_refs_t _dxtr;
#endif
} dispatch_unote_t DISPATCH_TRANSPARENT_UNION;

#define DISPATCH_UNOTE_NULL ((dispatch_unote_t){ ._du = NULL })

#if TARGET_OS_IPHONE
#define DSL_HASH_SIZE  64u // must be a power of two
#else
#define DSL_HASH_SIZE 256u // must be a power of two
#endif
#define DSL_HASH(x) ((x) & (DSL_HASH_SIZE - 1))

typedef struct dispatch_unote_linkage_s {
	LIST_ENTRY(dispatch_unote_linkage_s) du_link;
	struct dispatch_muxnote_s *du_muxnote;
} DISPATCH_ATOMIC64_ALIGN *dispatch_unote_linkage_t;

DISPATCH_ENUM(dispatch_unote_action, uint8_t,
	DISPATCH_UNOTE_ACTION_PASS_DATA,        // pass ke->data
	DISPATCH_UNOTE_ACTION_PASS_FFLAGS,      // pass ke->fflags
	DISPATCH_UNOTE_ACTION_SOURCE_OR_FFLAGS, // ds_pending_data |= ke->fflags
	DISPATCH_UNOTE_ACTION_SOURCE_SET_DATA,  // ds_pending_data = ~ke->data
	DISPATCH_UNOTE_ACTION_SOURCE_ADD_DATA,  // ds_pending_data += ke->data
	DISPATCH_UNOTE_ACTION_SOURCE_TIMER,     // timer
);

typedef struct dispatch_source_type_s {
	const char *dst_kind;
	int8_t     dst_filter;
	dispatch_unote_action_t dst_action;
	uint8_t    dst_per_trigger_qos : 1;
	uint8_t    dst_strict : 1;
	uint8_t    dst_allow_empty_mask : 1;
	uint8_t    dst_timer_flags;
	uint16_t   dst_flags;
#if DISPATCH_EVENT_BACKEND_KEVENT
	uint16_t   dst_data;
#endif
	uint32_t   dst_fflags;
	uint32_t   dst_mask;
	uint32_t   dst_size;

	dispatch_unote_t (*dst_create)(dispatch_source_type_t dst,
			uintptr_t handle, unsigned long mask);
#if DISPATCH_EVENT_BACKEND_KEVENT
	bool (*dst_update_mux)(struct dispatch_muxnote_s *dmn);
#endif
	void (*dst_merge_evt)(dispatch_unote_t du, uint32_t flags, uintptr_t data,
			pthread_priority_t pp);
#if HAVE_MACH
	void (*dst_merge_msg)(dispatch_unote_t du, uint32_t flags,
			mach_msg_header_t *msg, mach_msg_size_t sz,
			pthread_priority_t msg_pp, pthread_priority_t override_pp);
#endif
} dispatch_source_type_s;

#define dux_create(dst, handle, mask)	(dst)->dst_create(dst, handle, mask)
#define dux_type(du)           (du)->du_type
#define dux_needs_rearm(du)    (dux_type(du)->dst_flags & (EV_ONESHOT | EV_DISPATCH))
#define dux_merge_evt(du, ...) dux_type(du)->dst_merge_evt(du, __VA_ARGS__)
#define dux_merge_msg(du, ...) dux_type(du)->dst_merge_msg(du, __VA_ARGS__)

extern const dispatch_source_type_s _dispatch_source_type_after;

#if HAVE_MACH
extern const dispatch_source_type_s _dispatch_mach_type_notification;
extern const dispatch_source_type_s _dispatch_mach_type_send;
extern const dispatch_source_type_s _dispatch_mach_type_recv;
extern const dispatch_source_type_s _dispatch_mach_type_reply;
extern const dispatch_source_type_s _dispatch_xpc_type_sigterm;
extern const dispatch_source_type_s _dispatch_source_type_timer_with_clock;
#define DISPATCH_MACH_TYPE_WAITER ((const dispatch_source_type_s *)-2)
#endif

#pragma mark -
#pragma mark deferred items

#if DISPATCH_EVENT_BACKEND_KEVENT
#if DISPATCH_USE_KEVENT_QOS
typedef struct kevent_qos_s dispatch_kevent_s;
#else
typedef struct kevent dispatch_kevent_s;
#endif
typedef dispatch_kevent_s *dispatch_kevent_t;
#endif // DISPATCH_EVENT_BACKEND_KEVENT

#define DISPATCH_DEFERRED_ITEMS_EVENT_COUNT 16

typedef struct dispatch_deferred_items_s {
	dispatch_queue_global_t ddi_stashed_rq;
	dispatch_object_t ddi_stashed_dou;
	dispatch_qos_t ddi_stashed_qos;
	dispatch_wlh_t ddi_wlh;
#if DISPATCH_EVENT_BACKEND_KEVENT
	dispatch_kevent_t ddi_eventlist;
	uint16_t ddi_nevents;
	uint16_t ddi_maxevents;
	bool     ddi_can_stash;
	uint16_t ddi_wlh_needs_delete : 1;
	uint16_t ddi_wlh_needs_update : 1;
	uint16_t ddi_wlh_servicing : 1;
#endif
} dispatch_deferred_items_s, *dispatch_deferred_items_t;

#pragma mark -
#pragma mark inlines

#if DISPATCH_PURE_C

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_deferred_items_set(dispatch_deferred_items_t ddi)
{
	_dispatch_thread_setspecific(dispatch_deferred_items_key, (void *)ddi);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_deferred_items_t
_dispatch_deferred_items_get(void)
{
	return (dispatch_deferred_items_t)
			_dispatch_thread_getspecific(dispatch_deferred_items_key);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_needs_to_return_to_kernel(void)
{
	return (uintptr_t)_dispatch_thread_getspecific(dispatch_r2k_key) != 0;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_set_return_to_kernel(void)
{
	_dispatch_thread_setspecific(dispatch_r2k_key, (void *)1);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_clear_return_to_kernel(void)
{
	_dispatch_thread_setspecific(dispatch_r2k_key, (void *)0);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_wlh_t
_du_state_wlh(dispatch_unote_state_t du_state)
{
	return (dispatch_wlh_t)(du_state & DU_STATE_WLH_MASK);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_du_state_registered(dispatch_unote_state_t du_state)
{
	return du_state != DU_STATE_UNREGISTERED;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_du_state_armed(dispatch_unote_state_t du_state)
{
	return du_state & DU_STATE_ARMED;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_du_state_needs_delete(dispatch_unote_state_t du_state)
{
	return du_state & DU_STATE_NEEDS_DELETE;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_du_state_needs_rearm(dispatch_unote_state_t du_state)
{
	return _du_state_registered(du_state) && !_du_state_armed(du_state) &&
			!_du_state_needs_delete(du_state);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_unote_state_t
_dispatch_unote_state(dispatch_unote_t du)
{
	return os_atomic_load(&du._du->du_state, relaxed);
}
#define _dispatch_unote_wlh(du) \
		_du_state_wlh(_dispatch_unote_state(du))
#define _dispatch_unote_registered(du) \
		_du_state_registered(_dispatch_unote_state(du))
#define _dispatch_unote_armed(du) \
		_du_state_armed(_dispatch_unote_state(du))
#define _dispatch_unote_needs_delete(du) \
		_du_state_needs_delete(_dispatch_unote_state(du))
#define _dispatch_unote_needs_rearm(du) \
		_du_state_needs_rearm(_dispatch_unote_state(du))

DISPATCH_ALWAYS_INLINE DISPATCH_OVERLOADABLE
static inline void
_dispatch_unote_state_set(dispatch_unote_t du, dispatch_unote_state_t value)
{
	os_atomic_store(&du._du->du_state, value, relaxed);
}

DISPATCH_ALWAYS_INLINE DISPATCH_OVERLOADABLE
static inline void
_dispatch_unote_state_set(dispatch_unote_t du, dispatch_wlh_t wlh,
		dispatch_unote_state_t bits)
{
	_dispatch_unote_state_set(du, (dispatch_unote_state_t)wlh | bits);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_unote_state_set_bit(dispatch_unote_t du, dispatch_unote_state_t bit)
{
	_dispatch_unote_state_set(du, _dispatch_unote_state(du) | bit);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_unote_state_clear_bit(dispatch_unote_t du, dispatch_unote_state_t bit)
{
	_dispatch_unote_state_set(du, _dispatch_unote_state(du) & ~bit);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_unote_wlh_changed(dispatch_unote_t du, dispatch_wlh_t expected_wlh)
{
	dispatch_wlh_t wlh = _dispatch_unote_wlh(du);
	return wlh && wlh != DISPATCH_WLH_ANON && wlh != expected_wlh;
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_unote_linkage_t
_dispatch_unote_get_linkage(dispatch_unote_t du)
{
	dispatch_assert(!du._du->du_is_direct);
	return (dispatch_unote_linkage_t)((char *)du._du
			- sizeof(struct dispatch_unote_linkage_s));
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_unote_t
_dispatch_unote_linkage_get_unote(dispatch_unote_linkage_t dul)
{
	return (dispatch_unote_t){ ._du = (dispatch_unote_class_t)(dul + 1) };
}

#endif // DISPATCH_PURE_C

DISPATCH_ALWAYS_INLINE
static inline unsigned long
_dispatch_timer_unote_compute_missed(dispatch_timer_source_refs_t dt,
		uint64_t now, unsigned long prev)
{
	uint64_t missed = (now - dt->dt_timer.target) / dt->dt_timer.interval;
	if (++missed + prev > LONG_MAX) {
		missed = LONG_MAX - prev;
	}
	if (dt->dt_timer.interval < INT64_MAX) {
		uint64_t push_by = missed * dt->dt_timer.interval;
		dt->dt_timer.target += push_by;
		dt->dt_timer.deadline += push_by;
	} else {
		dt->dt_timer.target = UINT64_MAX;
		dt->dt_timer.deadline = UINT64_MAX;
	}
	prev += missed;
	return prev;
}

#pragma mark -
#pragma mark prototypes

#if DISPATCH_HAVE_TIMER_QOS
#define DISPATCH_TIMER_QOS_NORMAL       0u
#define DISPATCH_TIMER_QOS_CRITICAL     1u
#define DISPATCH_TIMER_QOS_BACKGROUND   2u
#define DISPATCH_TIMER_QOS_COUNT        3u
#else
#define DISPATCH_TIMER_QOS_NORMAL       0u
#define DISPATCH_TIMER_QOS_COUNT        1u
#endif

#define DISPATCH_TIMER_QOS(tidx)   ((uint32_t)(tidx) % DISPATCH_TIMER_QOS_COUNT)
#define DISPATCH_TIMER_CLOCK(tidx) (dispatch_clock_t)((tidx) / DISPATCH_TIMER_QOS_COUNT)

#define DISPATCH_TIMER_INDEX(clock, qos) (((clock) * DISPATCH_TIMER_QOS_COUNT) + (qos))
#define DISPATCH_TIMER_COUNT \
		DISPATCH_TIMER_INDEX(DISPATCH_CLOCK_COUNT, 0)
// Workloops do not support optimizing WALL timers
#define DISPATCH_TIMER_WLH_COUNT \
		DISPATCH_TIMER_INDEX(DISPATCH_CLOCK_WALL, 0)

#define DISPATCH_TIMER_IDENT_CANCELED    (~0u)

extern struct dispatch_timer_heap_s _dispatch_timers_heap[DISPATCH_TIMER_COUNT];

dispatch_unote_t _dispatch_unote_create_with_handle(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask);
dispatch_unote_t _dispatch_unote_create_with_fd(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask);
dispatch_unote_t _dispatch_unote_create_without_handle(
		dispatch_source_type_t dst, uintptr_t handle, unsigned long mask);
void _dispatch_unote_dispose(dispatch_unote_t du);

/*
 * @const DUU_DELETE_ACK
 * Unregistration can acknowledge the "needs-delete" state of a unote.
 * There must be some sort of synchronization between callers passing this flag
 * for a given unote.
 *
 * @const DUU_PROBE
 * This flag is passed for the first unregistration attempt of a unote.
 * When passed, it allows the unregistration to speculatively try to do the
 * unregistration syscalls and maybe get lucky. If the flag isn't passed,
 * unregistration will preflight the attempt, and will not perform any syscall
 * if it cannot guarantee their success.
 *
 * @const DUU_MUST_SUCCEED
 * The caller expects the unregistration to always succeeed.
 * _dispatch_unote_unregister will either crash or return true.
 */
#define DUU_DELETE_ACK   0x1
#define DUU_PROBE        0x2
#define DUU_MUST_SUCCEED 0x4
bool _dispatch_unote_unregister(dispatch_unote_t du, uint32_t flags);
bool _dispatch_unote_register(dispatch_unote_t du, dispatch_wlh_t wlh,
		dispatch_priority_t pri);
void _dispatch_unote_resume(dispatch_unote_t du);

bool _dispatch_unote_unregister_muxed(dispatch_unote_t du);
bool _dispatch_unote_register_muxed(dispatch_unote_t du);
void _dispatch_unote_resume_muxed(dispatch_unote_t du);

#if DISPATCH_HAVE_DIRECT_KNOTES
bool _dispatch_unote_unregister_direct(dispatch_unote_t du, uint32_t flags);
bool _dispatch_unote_register_direct(dispatch_unote_t du, dispatch_wlh_t wlh);
void _dispatch_unote_resume_direct(dispatch_unote_t du);
#endif

void _dispatch_timer_unote_configure(dispatch_timer_source_refs_t dt);

#if !DISPATCH_EVENT_BACKEND_WINDOWS
void _dispatch_event_loop_atfork_child(void);
#endif
#define DISPATCH_EVENT_LOOP_CONSUME_2 DISPATCH_WAKEUP_CONSUME_2
#define DISPATCH_EVENT_LOOP_OVERRIDE  0x80000000
void _dispatch_event_loop_poke(dispatch_wlh_t wlh, uint64_t dq_state,
		uint32_t flags);
void _dispatch_event_loop_cancel_waiter(struct dispatch_sync_context_s *dsc);
void _dispatch_event_loop_wake_owner(struct dispatch_sync_context_s *dsc,
		dispatch_wlh_t wlh, uint64_t old_state, uint64_t new_state);
void _dispatch_event_loop_wait_for_ownership(
		struct dispatch_sync_context_s *dsc);
void _dispatch_event_loop_end_ownership(dispatch_wlh_t wlh,
		uint64_t old_state, uint64_t new_state, uint32_t flags);
#if DISPATCH_WLH_DEBUG
void _dispatch_event_loop_assert_not_owned(dispatch_wlh_t wlh);
#else
#undef _dispatch_event_loop_assert_not_owned
#define _dispatch_event_loop_assert_not_owned(wlh) ((void)wlh)
#endif
void _dispatch_event_loop_leave_immediate(uint64_t dq_state);
#if DISPATCH_EVENT_BACKEND_KEVENT
void _dispatch_event_loop_leave_deferred(dispatch_deferred_items_t ddi,
		uint64_t dq_state);
void _dispatch_event_loop_merge(dispatch_kevent_t events, int nevents);
#endif
void _dispatch_event_loop_drain(uint32_t flags);

void _dispatch_event_loop_timer_arm(dispatch_timer_heap_t dth, uint32_t tidx,
		dispatch_timer_delay_s range, dispatch_clock_now_cache_t nows);
void _dispatch_event_loop_timer_delete(dispatch_timer_heap_t dth, uint32_t tidx);

void _dispatch_event_loop_drain_timers(dispatch_timer_heap_t dth, uint32_t count);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_timers_heap_dirty(dispatch_timer_heap_t dth, uint32_t tidx)
{
	// Note: the dirty bits are only maintained in the first heap for any tidx
	dth[0].dth_dirty_bits |= (1 << DISPATCH_TIMER_QOS(tidx)) | DTH_DIRTY_GLOBAL;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_event_loop_drain_anon_timers(void)
{
	if (_dispatch_timers_heap[0].dth_dirty_bits) {
		_dispatch_event_loop_drain_timers(_dispatch_timers_heap,
				DISPATCH_TIMER_COUNT);
	}
}

#endif /* __DISPATCH_EVENT_EVENT_INTERNAL__ */
