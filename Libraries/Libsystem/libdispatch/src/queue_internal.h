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

#ifndef __DISPATCH_QUEUE_INTERNAL__
#define __DISPATCH_QUEUE_INTERNAL__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

#pragma mark -
#pragma mark dispatch_queue_flags, dq_state

DISPATCH_OPTIONS(dispatch_queue_flags, uint32_t,
	DQF_NONE                = 0x00000000,
	DQF_AUTORELEASE_ALWAYS  = 0x00010000,
	DQF_AUTORELEASE_NEVER   = 0x00020000,
#define _DQF_AUTORELEASE_MASK 0x00030000
	DQF_THREAD_BOUND        = 0x00040000, // queue is bound to a thread
	DQF_BARRIER_BIT         = 0x00080000, // queue is a barrier on its target
	DQF_TARGETED            = 0x00100000, // queue is targeted by another object
	DQF_LABEL_NEEDS_FREE    = 0x00200000, // queue label was strdup()ed
	DQF_MUTABLE             = 0x00400000,
	DQF_RELEASED            = 0x00800000, // xref_cnt == -1

	//
	// Only applies to sources
	//
	// @const DSF_STRICT
	// Semantics of the source are strict (implies DQF_MUTABLE being unset):
	// - handlers can't be changed past activation
	// - EV_VANISHED causes a hard failure
	// - source can't change WLH
	//
	// @const DSF_WLH_CHANGED
	// The wlh for the source changed (due to retarget past activation).
	// Only used for debugging and diagnostics purposes.
	//
	// @const DSF_CANCELED
	// Explicit cancelation has been requested.
	//
	// @const DSF_CANCEL_WAITER
	// At least one caller of dispatch_source_cancel_and_wait() is waiting on
	// the cancelation to finish. DSF_CANCELED must be set if this bit is set.
	//
	// @const DSF_NEEDS_EVENT
	// The source has started to delete its unotes due to cancelation, but
	// couldn't finish its unregistration and is waiting for some asynchronous
	// events to fire to be able to.
	//
	// This flag prevents spurious wakeups when the source state machine
	// requires specific events to make progress. Events that are likely
	// to unblock a source state machine pass DISPATCH_WAKEUP_EVENT
	// which neuters the effect of DSF_NEEDS_EVENT.
	//
	// @const DSF_DELETED
	// The source can now only be used as a queue and is not allowed to register
	// any new unote anymore. All the previously registered unotes are inactive
	// and their knote is gone. However, these previously registered unotes may
	// still be in the process of delivering their last event.
	//
	// Sources have an internal refcount taken always while they use eventing
	// subsystems which is consumed when this bit is set.
	//
	DSF_STRICT              = 0x04000000,
	DSF_WLH_CHANGED         = 0x08000000,
	DSF_CANCELED            = 0x10000000,
	DSF_CANCEL_WAITER       = 0x20000000,
	DSF_NEEDS_EVENT         = 0x40000000,
	DSF_DELETED             = 0x80000000,

#define DQF_FLAGS_MASK        ((dispatch_queue_flags_t)0xffff0000)
#define DQF_WIDTH_MASK        ((dispatch_queue_flags_t)0x0000ffff)
#define DQF_WIDTH(n)          ((dispatch_queue_flags_t)(uint16_t)(n))
);

/*
 * dispatch queues `dq_state` demystified
 *
 *******************************************************************************
 *
 * Most Significant 32 bit Word
 * ----------------------------
 *
 * sc: suspend count (bits 63 - 58)
 *    The suspend count unsurprisingly holds the suspend count of the queue
 *    Only 7 bits are stored inline. Extra counts are transfered in a side
 *    suspend count and when that has happened, the ssc: bit is set.
 */
#define DISPATCH_QUEUE_SUSPEND_INTERVAL		0x0400000000000000ull
#define DISPATCH_QUEUE_SUSPEND_HALF			0x20u
/*
 * ssc: side suspend count (bit 57)
 *    This bit means that the total suspend count didn't fit in the inline
 *    suspend count, and that there are additional suspend counts stored in the
 *    `dq_side_suspend_cnt` field.
 */
#define DISPATCH_QUEUE_HAS_SIDE_SUSPEND_CNT	0x0200000000000000ull
/*
 * i: inactive state (bit 56-55)
 *    This bit means that the object is inactive (see dispatch_activate)
 */
#define DISPATCH_QUEUE_INACTIVE				0x0180000000000000ull
#define DISPATCH_QUEUE_ACTIVATED			0x0100000000000000ull
#define DISPATCH_QUEUE_ACTIVATING			0x0080000000000000ull
/*
 * This mask covers the inactive bits state
 */
#define DISPATCH_QUEUE_INACTIVE_BITS_MASK	0x0180000000000000ull
/*
 * This mask covers the suspend count (sc), side suspend count bit (ssc),
 * inactive (i) and needs activation (na) bits
 */
#define DISPATCH_QUEUE_SUSPEND_BITS_MASK	0xff80000000000000ull
/*
 * ib: in barrier (bit 54)
 *    This bit is set when the queue is currently executing a barrier
 */
#define DISPATCH_QUEUE_IN_BARRIER			0x0040000000000000ull
/*
 * qf: queue full (bit 53)
 *    This bit is a subtle hack that allows to check for any queue width whether
 *    the full width of the queue is used or reserved (depending on the context)
 *    In other words that the queue has reached or overflown its capacity.
 */
#define DISPATCH_QUEUE_WIDTH_FULL_BIT		0x0020000000000000ull
#define DISPATCH_QUEUE_WIDTH_FULL			0x1000ull
#define DISPATCH_QUEUE_WIDTH_POOL (DISPATCH_QUEUE_WIDTH_FULL - 1)
#define DISPATCH_QUEUE_WIDTH_MAX  (DISPATCH_QUEUE_WIDTH_FULL - 2)
#define DISPATCH_QUEUE_USES_REDIRECTION(width) \
		({ uint16_t _width = (width); \
		_width > 1 && _width < DISPATCH_QUEUE_WIDTH_POOL; })
/*
 * w:  width (bits 52 - 41)
 *    This encodes how many work items are in flight. Barriers hold `dq_width`
 *    of them while they run. This is encoded as a signed offset with respect,
 *    to full use, where the negative values represent how many available slots
 *    are left, and the positive values how many work items are exceeding our
 *    capacity.
 *
 *    When this value is positive, then `wo` is always set to 1.
 */
#define DISPATCH_QUEUE_WIDTH_INTERVAL		0x0000020000000000ull
#define DISPATCH_QUEUE_WIDTH_MASK			0x003ffe0000000000ull
#define DISPATCH_QUEUE_WIDTH_SHIFT			41
/*
 * pb: pending barrier (bit 40)
 *    Drainers set this bit when they couldn't run the next work item and it is
 *    a barrier. When this bit is set, `dq_width - 1` work item slots are
 *    reserved so that no wakeup happens until the last work item in flight
 *    completes.
 */
#define DISPATCH_QUEUE_PENDING_BARRIER		0x0000010000000000ull
/*
 * d: dirty bit (bit 39)
 *    This bit is set when a queue transitions from empty to not empty.
 *    This bit is set before dq_items_head is set, with appropriate barriers.
 *    Any thread looking at a queue head is responsible for unblocking any
 *    dispatch_*_sync that could be enqueued at the beginning.
 *
 *    Drainer perspective
 *    ===================
 *
 *    When done, any "Drainer", in particular for dispatch_*_sync() handoff
 *    paths, exits in 3 steps, and the point of the DIRTY bit is to make
 *    the Drainers take the slow path at step 2 to take into account enqueuers
 *    that could have made the queue non idle concurrently.
 *
 *    <code>
 *        // drainer-exit step 1
 *        if (unlikely(dq->dq_items_tail)) { // speculative test
 *            return handle_non_empty_queue_or_wakeup(dq);
 *        }
 *        // drainer-exit step 2
 *        if (!_dispatch_queue_drain_try_unlock(dq, ${owned}, ...)) {
 *            return handle_non_empty_queue_or_wakeup(dq);
 *        }
 *        // drainer-exit step 3
 *        // no need to wake up the queue, it's really empty for sure
 *        return;
 *    </code>
 *
 *    The crux is _dispatch_queue_drain_try_unlock(), it is a function whose
 *    contract is to release everything the current thread owns from the queue
 *    state, so that when it's successful, any other thread can acquire
 *    width from that queue.
 *
 *    But, that function must fail if it sees the DIRTY bit set, leaving
 *    the state untouched. Leaving the state untouched is vital as it ensures
 *    that no other Slayer^WDrainer can rise at the same time, because the
 *    resource stays locked.
 *
 *
 *    Note that releasing the DRAIN_LOCK or ENQUEUE_LOCK (see below) currently
 *    doesn't use that pattern, and always tries to requeue. It isn't a problem
 *    because while holding either of these locks prevents *some* sync (the
 *    barrier one) codepaths to acquire the resource, the retry they perform
 *    at their step D (see just below) isn't affected by the state of these bits
 *    at all.
 *
 *
 *    Sync items perspective
 *    ======================
 *
 *    On the dispatch_*_sync() acquire side, the code must look like this:
 *
 *    <code>
 *        // step A
 *        if (try_acquire_sync(dq)) {
 *            return sync_operation_fastpath(dq, item);
 *        }
 *
 *        // step B
 *        if (queue_push_and_inline(dq, item)) {
 *            atomic_store(dq->dq_items_head, item, relaxed);
 *            // step C
 *            atomic_or(dq->dq_state, DIRTY, release);
 *
 *            // step D
 *            if (try_acquire_sync(dq)) {
 *                try_lock_transfer_or_wakeup(dq);
 *            }
 *        }
 *
 *        // step E
 *        wait_for_lock_transfer(dq);
 *    </code>
 *
 *    A. If this code can acquire the resource it needs at step A, we're good.
 *
 *    B. If the item isn't the first at enqueue time, then there is no issue
 *       At least another thread went through C, this thread isn't interesting
 *       for the possible races, responsibility to make progress is transfered
 *       to the thread which went through C-D.
 *
 *    C. The DIRTY bit is set with a release barrier, after the head/tail
 *       has been set, so that seeing the DIRTY bit means that head/tail
 *       will be visible to any drainer that has the matching acquire barrier.
 *
 *       Drainers may see the head/tail and fail to see DIRTY, in which
 *       case, their _dispatch_queue_drain_try_unlock() will clear the DIRTY
 *       bit, and fail, causing the caller to retry exactly once.
 *
 *    D. At this stage, there's two possible outcomes:
 *
 *       - either the acquire works this time, in which case this thread
 *         successfuly becomes a drainer. That's obviously the happy path.
 *         It means all drainers are after Step 2 (or there is no Drainer)
 *
 *       - or the acquire fails, which means that another drainer is before
 *         its Step 2. Since we set the DIRTY bit on the dq_state by now,
 *         and that drainers manipulate the state atomically, at least one
 *         drainer that is still before its step 2 will fail its step 2, and
 *         be responsible for making progress.
 *
 *
 *    Async items perspective
 *    ======================
 *
 *    On the async codepath, when the queue becomes non empty, the queue
 *    is always woken up. There is no point in trying to avoid that wake up
 *    for the async case, because it's required for the async()ed item to make
 *    progress: a drain of the queue must happen.
 *
 *    So on the async "acquire" side, there is no subtlety at all.
 */
#define DISPATCH_QUEUE_DIRTY				0x0000008000000000ull
/*
 * md: enqueued/draining on manager (bit 38)
 *    Set when enqueued and draining on the manager hierarchy.
 *
 *    Unlike the ENQUEUED bit, it is kept until the queue is unlocked from its
 *    invoke call on the manager. This is used to prevent stealing, and
 *    overrides to be applied down the target queue chain.
 */
#define DISPATCH_QUEUE_ENQUEUED_ON_MGR		0x0000004000000000ull
/*
 * r: queue graph role (bits 37 - 36)
 *    Queue role in the target queue graph
 *
 *    11: unused
 *    10: WLH base
 *    01: non wlh base
 *    00: inner queue
 */
#define DISPATCH_QUEUE_ROLE_MASK			0x0000003000000000ull
#define DISPATCH_QUEUE_ROLE_BASE_WLH		0x0000002000000000ull
#define DISPATCH_QUEUE_ROLE_BASE_ANON		0x0000001000000000ull
#define DISPATCH_QUEUE_ROLE_INNER			0x0000000000000000ull
/*
 * o: has override (bit 35, if role is DISPATCH_QUEUE_ROLE_BASE_ANON)
 *    Set when a queue has received a QOS override and needs to reset it.
 *    This bit is only cleared when the final drain_try_unlock() succeeds.
 *
 * sw: has received sync wait (bit 35, if role DISPATCH_QUEUE_ROLE_BASE_WLH)
 *    Set when a queue owner has been exposed to the kernel because of
 *    dispatch_sync() contention.
 */
#define DISPATCH_QUEUE_RECEIVED_OVERRIDE	0x0000000800000000ull
#define DISPATCH_QUEUE_RECEIVED_SYNC_WAIT	0x0000000800000000ull
/*
 * max_qos: max qos (bits 34 - 32)
 *   This is the maximum qos that has been enqueued on the queue
 */
#define DISPATCH_QUEUE_MAX_QOS_MASK			0x0000000700000000ull
#define DISPATCH_QUEUE_MAX_QOS_SHIFT		32
/*
 * dl: drain lock (bits 31-0)
 *    This is used by the normal drain to drain exlusively relative to other
 *    drain stealers (like the QoS Override codepath). It holds the identity
 *    (thread port) of the current drainer.
 *
 * st: sync transfer (bit 1 or 30)
 *    Set when a dispatch_sync() is transferred to
 *
 * e: enqueued bit (bit 0 or 31)
 *    Set when a queue is enqueued on its target queue
 */
#define DISPATCH_QUEUE_DRAIN_OWNER_MASK		((uint64_t)DLOCK_OWNER_MASK)
#define DISPATCH_QUEUE_SYNC_TRANSFER		((uint64_t)DLOCK_FAILED_TRYLOCK_BIT)
#define DISPATCH_QUEUE_ENQUEUED				((uint64_t)DLOCK_WAITERS_BIT)

#define DISPATCH_QUEUE_DRAIN_PRESERVED_BITS_MASK \
		(DISPATCH_QUEUE_ENQUEUED_ON_MGR | DISPATCH_QUEUE_ENQUEUED | \
		DISPATCH_QUEUE_ROLE_MASK | DISPATCH_QUEUE_MAX_QOS_MASK)

#define DISPATCH_QUEUE_DRAIN_UNLOCK_MASK \
		(DISPATCH_QUEUE_DRAIN_OWNER_MASK | DISPATCH_QUEUE_RECEIVED_OVERRIDE | \
		DISPATCH_QUEUE_RECEIVED_SYNC_WAIT | DISPATCH_QUEUE_SYNC_TRANSFER)

/*
 *******************************************************************************
 *
 * `Drainers`
 *
 * Drainers are parts of the code that hold the drain lock by setting its value
 * to their thread port. There are two kinds:
 * 1. async drainers,
 * 2. lock transfer handlers.
 *
 * Drainers from the first category are _dispatch_queue_class_invoke and its
 * stealers. Those drainers always try to reserve width at the same time they
 * acquire the drain lock, to make sure they can make progress, and else exit
 * quickly.
 *
 * Drainers from the second category are `slow` work items. Those run on the
 * calling thread, and when done, try to transfer the width they own to the
 * possible next `slow` work item, and if there is no such item, they reliquish
 * that right. To do so, prior to taking any decision, they also try to own
 * the full "barrier" width on the given queue.
 *
 *******************************************************************************
 *
 * Enqueuing and wakeup rules
 *
 * Nobody should enqueue any dispatch object if it has no chance to make any
 * progress. That means that queues that:
 * - are suspended
 * - have reached or overflown their capacity
 * - are currently draining
 * - are already enqueued
 *
 * should not try to be enqueued.
 *
 *******************************************************************************
 *
 * Lock transfer
 *
 * The point of the lock transfer code is to allow pure dispatch_*_sync()
 * callers to make progress without requiring the bring up of a drainer.
 * There are two reason for that:
 *
 * - performance, as draining has to give up for dispatch_*_sync() work items,
 *   so waking up a queue for this is wasteful.
 *
 * - liveness, as with dispatch_*_sync() you burn threads waiting, you're more
 *   likely to hit various thread limits and may not have any drain being
 *   brought up if the process hits a limit.
 *
 *
 * Lock transfer happens at the end on the dispatch_*_sync() codepaths:
 *
 * - obviously once a dispatch_*_sync() work item finishes, it owns queue
 *   width and it should try to transfer that ownership to the possible next
 *   queued item if it is a dispatch_*_sync() item
 *
 * - just before such a work item blocks to make sure that that work item
 *   itself isn't its own last chance to be woken up. That can happen when
 *   a Drainer pops up everything from the queue, and that a dispatch_*_sync()
 *   work item has taken the slow path then was preempted for a long time.
 *
 *   That's why such work items, if first in the queue, must try a lock
 *   transfer procedure.
 *
 *
 * For transfers where a partial width is owned, we give back that width.
 * If the queue state is "idle" again, we attempt to acquire the full width.
 * If that succeeds, this falls back to the full barrier lock
 * transfer, else it wakes up the queue according to its state.
 *
 * For full barrier transfers, if items eligible for lock transfer are found,
 * then they are woken up and the lock transfer is successful.
 *
 * If none are found, the full barrier width is released. If by doing so the
 * DIRTY bit is found, releasing the full barrier width fails and transferring
 * the lock is retried from scratch.
 */

#define DISPATCH_QUEUE_STATE_INIT_VALUE(width) \
		((DISPATCH_QUEUE_WIDTH_FULL - (width)) << DISPATCH_QUEUE_WIDTH_SHIFT)

/* Magic dq_state values for global queues: they have QUEUE_FULL and IN_BARRIER
 * set to force the slow path in dispatch_barrier_sync() and dispatch_sync()
 */
#define DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE \
		(DISPATCH_QUEUE_WIDTH_FULL_BIT | DISPATCH_QUEUE_IN_BARRIER)

#define DISPATCH_QUEUE_SERIAL_DRAIN_OWNED \
		(DISPATCH_QUEUE_IN_BARRIER | DISPATCH_QUEUE_WIDTH_INTERVAL)

#pragma mark -
#pragma mark dispatch_queue_t

typedef struct dispatch_queue_specific_s {
	const void *dqs_key;
	void *dqs_ctxt;
	dispatch_function_t dqs_destructor;
	TAILQ_ENTRY(dispatch_queue_specific_s) dqs_entry;
} *dispatch_queue_specific_t;

typedef struct dispatch_queue_specific_head_s {
	dispatch_unfair_lock_s dqsh_lock;
	TAILQ_HEAD(, dispatch_queue_specific_s) dqsh_entries;
} *dispatch_queue_specific_head_t;

#define DISPATCH_WORKLOOP_ATTR_HAS_SCHED      0x0001u
#define DISPATCH_WORKLOOP_ATTR_HAS_POLICY     0x0002u
#define DISPATCH_WORKLOOP_ATTR_HAS_CPUPERCENT 0x0004u
#define DISPATCH_WORKLOOP_ATTR_HAS_QOS_CLASS  0x0008u
#define DISPATCH_WORKLOOP_ATTR_NEEDS_DESTROY  0x0010u
#define DISPATCH_WORKLOOP_ATTR_HAS_OBSERVERS  0x0020u
typedef struct dispatch_workloop_attr_s *dispatch_workloop_attr_t;
typedef struct dispatch_workloop_attr_s {
	uint32_t dwla_flags;
	dispatch_priority_t dwla_pri;
	struct sched_param dwla_sched;
	int dwla_policy;
	struct {
		uint8_t percent;
		uint32_t refillms;
	} dwla_cpupercent;
	dispatch_pthread_root_queue_observer_hooks_s dwla_observers;
} dispatch_workloop_attr_s;

/*
 * Dispatch Queue cluster related types
 *
 * The dispatch queue cluster uses aliasing structs, and loosely follows the
 * external types exposed in <dispatch/queue.h>
 *
 * The API types pretend to have this hierarchy:
 *
 * dispatch_queue_t
 *  +--> dispatch_workloop_t
 *  +--> dispatch_queue_serial_t --> dispatch_queue_main_t
 *  +--> dispatch_queue_concurrent_t
 *  '--> dispatch_queue_global_t
 *
 *
 * However, in the library itself, there are more types and a finer grained
 * hierarchy when it comes to the struct members.
 *
 * dispatch_queue_class_t / struct dispatch_queue_s
 *  +--> struct dispatch_workloop_s
 *  '--> dispatch_lane_class_t
 *        +--> struct dispatch_lane_s
 *        |     +--> struct dispatch_source_s
 *        |     +--> struct dispatch_channel_s
 *        |     '--> struct dispatch_mach_s
 *        +--> struct dispatch_queue_static_s
 *        '--> struct dispatch_queue_global_s
 *              +--> struct dispatch_queue_pthread_root_s
 *
 *
 * dispatch_queue_class_t && struct dispatch_queue_s
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * The queue class type is a transparent union of all queue types, which allows
 * cutting down the explicit downcasts to `dispatch_queue_t` when calling
 * a function working on any dispatch_queue_t type.
 *
 * The concrete struct layout is struct dispatch_queue_s
 * it provides:
 * - dispatch object fields
 * - dq_state
 * - dq_serialnum
 * - dq_label
 * - dq_atomic_flags
 * - dq_sref_cnt
 * - an auxiliary pointer used by sub-classes (dq_specific_head, ds_refs, ...)
 * - dq_priority (XXX: we should push it down to lanes)
 *
 * It also provides storage for one opaque pointer sized field.
 *
 * dispatch_lane_class_t
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * The lane class type is a transparent union of all "lane" types, which have
 * a single head/tail pair.
 *
 * There's no proper concrete struct layout associated, `struct dispatch_lane_s`
 * is used most of the time instead. The lane class adds:
 * - dq_items_head
 * - dq_items_tail (allocated in the hole the queue class carves out)
 *
 *
 * struct dispatch_lane_s and variants
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This is the concrete type used for:
 * - API serial/concurrent/runloop queues
 * - sources and mach channels
 * - the main and manager queues, as struct dispatch_queue_static_s which is
 *   a cacheline aligned variant of struct dispatch_lane_s.
 *
 * It also provides:
 * - dq_sidelock, used for suspension & target queue handling,
 * - dq_side_suspend_cnt.
 *
 * Sources (struct dispatch_source_s) and mach channels (struct dispatch_mach_s)
 * use the last 32bit word for flags private to their use.
 *
 * struct dispatch_queue_global_s is used for all dispatch root queues:
 * - global concurent queues
 * - pthread root queues
 * - the network event thread
 *
 * These pretend to derive from dispatch_lane_s but use the dq_sidelock,
 * dq_side_suspend_cnt differently, which is possible because root queues cannot
 * be targetted or suspended and hence have no use for these.
 */

#if OS_OBJECT_HAVE_OBJC1
#define _DISPATCH_QUEUE_CLASS_HEADER(x, __pointer_sized_field__) \
	DISPATCH_OBJECT_HEADER(x); \
	DISPATCH_UNION_LE(uint64_t volatile dq_state, \
			dispatch_lock dq_state_lock, \
			uint32_t dq_state_bits \
	); \
	__pointer_sized_field__
#else
#define _DISPATCH_QUEUE_CLASS_HEADER(x, __pointer_sized_field__) \
	DISPATCH_OBJECT_HEADER(x); \
	__pointer_sized_field__; \
	DISPATCH_UNION_LE(uint64_t volatile dq_state, \
			dispatch_lock dq_state_lock, \
			uint32_t dq_state_bits \
	)
#endif

#define DISPATCH_QUEUE_CLASS_HEADER(x, __pointer_sized_field__) \
	_DISPATCH_QUEUE_CLASS_HEADER(x, __pointer_sized_field__); \
	/* LP64 global queue cacheline boundary */ \
	unsigned long dq_serialnum; \
	const char *dq_label; \
	DISPATCH_UNION_LE(uint32_t volatile dq_atomic_flags, \
		const uint16_t dq_width, \
		const uint16_t __dq_opaque2 \
	); \
	dispatch_priority_t dq_priority; \
	union { \
		struct dispatch_queue_specific_head_s *dq_specific_head; \
		struct dispatch_source_refs_s *ds_refs; \
		struct dispatch_timer_source_refs_s *ds_timer_refs; \
		struct dispatch_mach_recv_refs_s *dm_recv_refs; \
		struct dispatch_channel_callbacks_s const *dch_callbacks; \
	}; \
	int volatile dq_sref_cnt

struct dispatch_queue_s {
	DISPATCH_QUEUE_CLASS_HEADER(queue, void *__dq_opaque1);
	/* 32bit hole on LP64 */
} DISPATCH_ATOMIC64_ALIGN;

struct dispatch_workloop_s {
	struct dispatch_queue_s _as_dq[0];
	DISPATCH_QUEUE_CLASS_HEADER(workloop, dispatch_timer_heap_t dwl_timer_heap);
	uint8_t dwl_drained_qos;
	/* 24 bits hole */
	struct dispatch_object_s *dwl_heads[DISPATCH_QOS_NBUCKETS];
	struct dispatch_object_s *dwl_tails[DISPATCH_QOS_NBUCKETS];
	dispatch_workloop_attr_t dwl_attr;
} DISPATCH_ATOMIC64_ALIGN;

#define DISPATCH_LANE_CLASS_HEADER(x) \
	struct dispatch_queue_s _as_dq[0]; \
	DISPATCH_QUEUE_CLASS_HEADER(x, \
			struct dispatch_object_s *volatile dq_items_tail); \
	dispatch_unfair_lock_s dq_sidelock; \
	struct dispatch_object_s *volatile dq_items_head; \
	uint32_t dq_side_suspend_cnt

typedef struct dispatch_lane_s {
	DISPATCH_LANE_CLASS_HEADER(lane);
	/* 32bit hole on LP64 */
} DISPATCH_ATOMIC64_ALIGN *dispatch_lane_t;

// Cache aligned type for static queues (main queue, manager)
struct dispatch_queue_static_s {
	struct dispatch_lane_s _as_dl[0]; \
	DISPATCH_LANE_CLASS_HEADER(lane);
} DISPATCH_CACHELINE_ALIGN;

#define DISPATCH_QUEUE_ROOT_CLASS_HEADER(x) \
	struct dispatch_queue_s _as_dq[0]; \
	DISPATCH_QUEUE_CLASS_HEADER(x, \
			struct dispatch_object_s *volatile dq_items_tail); \
	int volatile dgq_thread_pool_size; \
	struct dispatch_object_s *volatile dq_items_head; \
	int volatile dgq_pending

struct dispatch_queue_global_s {
	DISPATCH_QUEUE_ROOT_CLASS_HEADER(lane);
} DISPATCH_CACHELINE_ALIGN;


typedef struct dispatch_pthread_root_queue_observer_hooks_s {
	void (*queue_will_execute)(dispatch_queue_t queue);
	void (*queue_did_execute)(dispatch_queue_t queue);
} dispatch_pthread_root_queue_observer_hooks_s;
typedef dispatch_pthread_root_queue_observer_hooks_s
		*dispatch_pthread_root_queue_observer_hooks_t;

#ifdef __APPLE__
#define DISPATCH_IOHID_SPI 1

DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW DISPATCH_NONNULL4
dispatch_queue_global_t
_dispatch_pthread_root_queue_create_with_observer_hooks_4IOHID(
	const char *label, unsigned long flags, const pthread_attr_t *attr,
	dispatch_pthread_root_queue_observer_hooks_t observer_hooks,
	dispatch_block_t configure);

DISPATCH_EXPORT DISPATCH_PURE DISPATCH_WARN_RESULT DISPATCH_NOTHROW
bool
_dispatch_queue_is_exclusively_owned_by_current_thread_4IOHID(
		dispatch_queue_t queue);

DISPATCH_EXPORT DISPATCH_NOTHROW
void
_dispatch_workloop_set_observer_hooks_4IOHID(dispatch_workloop_t workloop,
		dispatch_pthread_root_queue_observer_hooks_t observer_hooks);
#endif // __APPLE__

#if DISPATCH_USE_PTHREAD_POOL
typedef struct dispatch_pthread_root_queue_context_s {
#if !defined(_WIN32)
	pthread_attr_t dpq_thread_attr;
#endif
	dispatch_block_t dpq_thread_configure;
	struct dispatch_semaphore_s dpq_thread_mediator;
	dispatch_pthread_root_queue_observer_hooks_s dpq_observer_hooks;
} *dispatch_pthread_root_queue_context_t;
#endif // DISPATCH_USE_PTHREAD_POOL

#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
typedef struct dispatch_queue_pthread_root_s {
	struct dispatch_queue_global_s _as_dgq[0];
	DISPATCH_QUEUE_ROOT_CLASS_HEADER(lane);
	struct dispatch_pthread_root_queue_context_s dpq_ctxt;
} *dispatch_queue_pthread_root_t;
#endif // DISPATCH_USE_PTHREAD_ROOT_QUEUES

dispatch_static_assert(sizeof(struct dispatch_queue_s) <= 128);
dispatch_static_assert(sizeof(struct dispatch_lane_s) <= 128);
dispatch_static_assert(sizeof(struct dispatch_queue_global_s) <= 128);
dispatch_static_assert(offsetof(struct dispatch_queue_s, dq_state) %
		sizeof(uint64_t) == 0, "dq_state must be 8-byte aligned");

#define dispatch_assert_valid_queue_type(type) \
		dispatch_static_assert(sizeof(struct dispatch_queue_s) <= \
				sizeof(struct type), #type " smaller than dispatch_queue_s"); \
		dispatch_static_assert(_Alignof(struct type) >= sizeof(uint64_t), \
				#type " is not 8-byte aligned"); \
		dispatch_assert_aliases(dispatch_queue_s, type, dq_state); \
		dispatch_assert_aliases(dispatch_queue_s, type, dq_serialnum); \
		dispatch_assert_aliases(dispatch_queue_s, type, dq_label); \
		dispatch_assert_aliases(dispatch_queue_s, type, dq_atomic_flags); \
		dispatch_assert_aliases(dispatch_queue_s, type, dq_sref_cnt); \
		dispatch_assert_aliases(dispatch_queue_s, type, dq_specific_head); \
		dispatch_assert_aliases(dispatch_queue_s, type, dq_priority)

#define dispatch_assert_valid_lane_type(type) \
		dispatch_assert_valid_queue_type(type); \
		dispatch_assert_aliases(dispatch_lane_s, type, dq_items_head); \
		dispatch_assert_aliases(dispatch_lane_s, type, dq_items_tail)

dispatch_assert_valid_queue_type(dispatch_lane_s);
dispatch_assert_valid_lane_type(dispatch_queue_static_s);
dispatch_assert_valid_lane_type(dispatch_queue_global_s);
#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
dispatch_assert_valid_lane_type(dispatch_queue_pthread_root_s);
#endif

DISPATCH_CLASS_DECL(queue, QUEUE);
DISPATCH_CLASS_DECL_BARE(lane, QUEUE);
DISPATCH_CLASS_DECL(workloop, QUEUE);
DISPATCH_SUBCLASS_DECL(queue_serial, queue, lane);
DISPATCH_SUBCLASS_DECL(queue_main, queue_serial, lane);
DISPATCH_SUBCLASS_DECL(queue_concurrent, queue, lane);
DISPATCH_SUBCLASS_DECL(queue_global, queue, lane);
#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
DISPATCH_INTERNAL_SUBCLASS_DECL(queue_pthread_root, queue, lane);
#endif
DISPATCH_INTERNAL_SUBCLASS_DECL(queue_runloop, queue_serial, lane);
DISPATCH_INTERNAL_SUBCLASS_DECL(queue_mgr, queue_serial, lane);

struct firehose_client_s;

typedef struct dispatch_thread_context_s *dispatch_thread_context_t;
typedef struct dispatch_thread_context_s {
	dispatch_thread_context_t dtc_prev;
	const void *dtc_key;
	union {
		size_t dtc_apply_nesting;
		dispatch_io_t dtc_io_in_barrier;
		union firehose_buffer_u *dtc_fb;
		void *dtc_mig_demux_ctx;
		dispatch_mach_msg_t dtc_dmsg;
		struct dispatch_ipc_handoff_s *dtc_dih;
	};
} dispatch_thread_context_s;

typedef union dispatch_thread_frame_s *dispatch_thread_frame_t;
typedef union dispatch_thread_frame_s {
	struct {
		// must be in the same order as our TSD keys!
		dispatch_queue_t dtf_queue;
		dispatch_thread_frame_t dtf_prev;
	};
	void *dtf_pair[2];
} dispatch_thread_frame_s;

typedef dispatch_queue_t dispatch_queue_wakeup_target_t;
#define DISPATCH_QUEUE_WAKEUP_NONE           ((dispatch_queue_wakeup_target_t)0)
#define DISPATCH_QUEUE_WAKEUP_TARGET         ((dispatch_queue_wakeup_target_t)1)
#define DISPATCH_QUEUE_WAKEUP_MGR            (_dispatch_mgr_q._as_dq)
#define DISPATCH_QUEUE_WAKEUP_WAIT_FOR_EVENT ((dispatch_queue_wakeup_target_t)-1)

void _dispatch_queue_xref_dispose(dispatch_queue_class_t dq);
void _dispatch_queue_wakeup(dispatch_queue_class_t dqu, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags, dispatch_queue_wakeup_target_t target);
void _dispatch_queue_invoke_finish(dispatch_queue_t dq,
		dispatch_invoke_context_t dic, dispatch_queue_t tq, uint64_t owned);

dispatch_priority_t _dispatch_queue_compute_priority_and_wlh(
		dispatch_queue_class_t dq, dispatch_wlh_t *wlh_out);

DISPATCH_ENUM(dispatch_resume_op, int,
	DISPATCH_RESUME,
	DISPATCH_ACTIVATE,
	DISPATCH_ACTIVATION_DONE,
);
void _dispatch_lane_resume(dispatch_lane_class_t dq, dispatch_resume_op_t how);

void _dispatch_lane_set_target_queue(dispatch_lane_t dq, dispatch_queue_t tq);
void _dispatch_lane_class_dispose(dispatch_queue_class_t dq, bool *allow_free);
void _dispatch_lane_dispose(dispatch_lane_class_t dq, bool *allow_free);
void _dispatch_lane_suspend(dispatch_lane_class_t dq);
void _dispatch_lane_activate(dispatch_lane_class_t dq);
void _dispatch_lane_invoke(dispatch_lane_class_t dq,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
void _dispatch_lane_push(dispatch_lane_class_t dq, dispatch_object_t dou,
		dispatch_qos_t qos);
void _dispatch_lane_concurrent_push(dispatch_lane_class_t dq,
		dispatch_object_t dou, dispatch_qos_t qos);
void _dispatch_lane_wakeup(dispatch_lane_class_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);
dispatch_queue_wakeup_target_t _dispatch_lane_serial_drain(
		dispatch_lane_class_t dq, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags, uint64_t *owned);

void _dispatch_workloop_dispose(dispatch_workloop_t dwl, bool *allow_free);
void _dispatch_workloop_activate(dispatch_workloop_t dwl);
void _dispatch_workloop_invoke(dispatch_workloop_t dwl,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags);
void _dispatch_workloop_push(dispatch_workloop_t dwl, dispatch_object_t dou,
		dispatch_qos_t qos);
void _dispatch_workloop_wakeup(dispatch_workloop_t dwl, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);

void _dispatch_root_queue_poke(dispatch_queue_global_t dq, int n, int floor);
void _dispatch_root_queue_wakeup(dispatch_queue_global_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);
void _dispatch_root_queue_push(dispatch_queue_global_t dq,
		dispatch_object_t dou, dispatch_qos_t qos);
#if DISPATCH_USE_KEVENT_WORKQUEUE
void _dispatch_kevent_workqueue_init(void);
#endif
#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
void _dispatch_pthread_root_queue_dispose(dispatch_lane_class_t dq,
		bool *allow_free);
#endif // DISPATCH_USE_PTHREAD_ROOT_QUEUES
void _dispatch_main_queue_push(dispatch_queue_main_t dq, dispatch_object_t dou,
		dispatch_qos_t qos);
void _dispatch_main_queue_wakeup(dispatch_queue_main_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);
#if DISPATCH_COCOA_COMPAT
void _dispatch_runloop_queue_wakeup(dispatch_lane_t dq,
		dispatch_qos_t qos, dispatch_wakeup_flags_t flags);
void _dispatch_runloop_queue_xref_dispose(dispatch_lane_t dq);
void _dispatch_runloop_queue_dispose(dispatch_lane_t dq, bool *allow_free);
#endif // DISPATCH_COCOA_COMPAT
void _dispatch_mgr_queue_push(dispatch_lane_t dq, dispatch_object_t dou,
		dispatch_qos_t qos);
void _dispatch_mgr_queue_wakeup(dispatch_lane_t dq, dispatch_qos_t qos,
		dispatch_wakeup_flags_t flags);
#if DISPATCH_USE_MGR_THREAD
void _dispatch_mgr_thread(dispatch_lane_t dq, dispatch_invoke_context_t dic,
		dispatch_invoke_flags_t flags);
#endif

void _dispatch_apply_invoke(void *ctxt);
void _dispatch_apply_redirect_invoke(void *ctxt);
void _dispatch_barrier_async_detached_f(dispatch_queue_class_t dq, void *ctxt,
		dispatch_function_t func);
#define DISPATCH_BARRIER_TRYSYNC_SUSPEND 0x1
void _dispatch_barrier_trysync_or_async_f(dispatch_lane_class_t dq, void *ctxt,
		dispatch_function_t func, uint32_t flags);
void _dispatch_queue_atfork_child(void);

DISPATCH_COLD
size_t _dispatch_queue_debug(dispatch_queue_class_t dq,
		char *buf, size_t bufsiz);
DISPATCH_COLD
size_t _dispatch_queue_debug_attr(dispatch_queue_t dq,
		char *buf, size_t bufsiz);

#define DISPATCH_ROOT_QUEUE_COUNT (DISPATCH_QOS_NBUCKETS * 2)

// must be in lowest to highest qos order (as encoded in dispatch_qos_t)
// overcommit qos index values need bit 1 set
enum {
	DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS = 0,
	DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS_OVERCOMMIT,
	DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS,
	DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT,
	DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS,
	DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT,
	DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS,
	DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT,
	DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS,
	DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT,
	DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS,
	DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT,
	_DISPATCH_ROOT_QUEUE_IDX_COUNT,
};

// skip zero
// 1 - main_q
// 2 - mgr_q
// 3 - mgr_root_q
// 4,5,6,7,8,9,10,11,12,13,14,15 - global queues
// 17 - workloop_fallback_q
// we use 'xadd' on Intel, so the initial value == next assigned
#define DISPATCH_QUEUE_SERIAL_NUMBER_INIT 17
extern unsigned long volatile _dispatch_queue_serial_numbers;

// mark the workloop fallback queue to avoid finalizing objects on the base
// queue of custom outside-of-qos workloops
#define DISPATCH_QUEUE_SERIAL_NUMBER_WLF 16

extern struct dispatch_queue_static_s _dispatch_mgr_q; // serial 2
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
extern struct dispatch_queue_global_s _dispatch_mgr_root_queue; // serial 3
#endif
extern struct dispatch_queue_global_s _dispatch_root_queues[]; // serials 4 - 15

#if DISPATCH_DEBUG
#define DISPATCH_ASSERT_ON_MANAGER_QUEUE() \
		dispatch_assert_queue(_dispatch_mgr_q._as_dq)
#else
#define DISPATCH_ASSERT_ON_MANAGER_QUEUE()
#endif

#pragma mark -
#pragma mark dispatch_queue_attr_t

DISPATCH_CLASS_DECL(queue_attr, OBJECT);
struct dispatch_queue_attr_s {
	OS_OBJECT_STRUCT_HEADER(dispatch_queue_attr);
};

typedef struct dispatch_queue_attr_info_s {
	dispatch_qos_t dqai_qos : 8;
	int      dqai_relpri : 8;
	uint16_t dqai_overcommit:2;
	uint16_t dqai_autorelease_frequency:2;
	uint16_t dqai_concurrent:1;
	uint16_t dqai_inactive:1;
} dispatch_queue_attr_info_t;

typedef enum {
	_dispatch_queue_attr_overcommit_unspecified = 0,
	_dispatch_queue_attr_overcommit_enabled,
	_dispatch_queue_attr_overcommit_disabled,
} _dispatch_queue_attr_overcommit_t;

#define DISPATCH_QUEUE_ATTR_OVERCOMMIT_COUNT 3

#define DISPATCH_QUEUE_ATTR_AUTORELEASE_FREQUENCY_COUNT 3

#define DISPATCH_QUEUE_ATTR_QOS_COUNT (DISPATCH_QOS_MAX + 1)

#define DISPATCH_QUEUE_ATTR_PRIO_COUNT (1 - QOS_MIN_RELATIVE_PRIORITY)

#define DISPATCH_QUEUE_ATTR_CONCURRENCY_COUNT 2

#define DISPATCH_QUEUE_ATTR_INACTIVE_COUNT 2

#define DISPATCH_QUEUE_ATTR_COUNT  ( \
		DISPATCH_QUEUE_ATTR_OVERCOMMIT_COUNT * \
		DISPATCH_QUEUE_ATTR_AUTORELEASE_FREQUENCY_COUNT * \
		DISPATCH_QUEUE_ATTR_QOS_COUNT * \
		DISPATCH_QUEUE_ATTR_PRIO_COUNT * \
		DISPATCH_QUEUE_ATTR_CONCURRENCY_COUNT * \
		DISPATCH_QUEUE_ATTR_INACTIVE_COUNT )

extern const struct dispatch_queue_attr_s
_dispatch_queue_attrs[DISPATCH_QUEUE_ATTR_COUNT];

dispatch_queue_attr_info_t _dispatch_queue_attr_to_info(dispatch_queue_attr_t);

#pragma mark -
#pragma mark dispatch_continuation_t

// If dc_flags is less than 0x1000, then the object is a continuation.
// Otherwise, the object has a private layout and memory management rules. The
// layout until after 'do_next' must align with normal objects.
#if __LP64__
#define DISPATCH_CONTINUATION_HEADER(x) \
	union { \
		const void *do_vtable; \
		uintptr_t dc_flags; \
	}; \
	union { \
		pthread_priority_t dc_priority; \
		int dc_cache_cnt; \
		uintptr_t dc_pad; \
	}; \
	struct dispatch_##x##_s *volatile do_next; \
	struct voucher_s *dc_voucher; \
	dispatch_function_t dc_func; \
	void *dc_ctxt; \
	void *dc_data; \
	void *dc_other
#elif OS_OBJECT_HAVE_OBJC1
#define DISPATCH_CONTINUATION_HEADER(x) \
	dispatch_function_t dc_func; \
	union { \
		pthread_priority_t dc_priority; \
		int dc_cache_cnt; \
		uintptr_t dc_pad; \
	}; \
	struct voucher_s *dc_voucher; \
	union { \
		const void *do_vtable; \
		uintptr_t dc_flags; \
	}; \
	struct dispatch_##x##_s *volatile do_next; \
	void *dc_ctxt; \
	void *dc_data; \
	void *dc_other
#else
#define DISPATCH_CONTINUATION_HEADER(x) \
	union { \
		const void *do_vtable; \
		uintptr_t dc_flags; \
	}; \
	union { \
		pthread_priority_t dc_priority; \
		int dc_cache_cnt; \
		uintptr_t dc_pad; \
	}; \
	struct voucher_s *dc_voucher; \
	struct dispatch_##x##_s *volatile do_next; \
	dispatch_function_t dc_func; \
	void *dc_ctxt; \
	void *dc_data; \
	void *dc_other
#endif
#define _DISPATCH_CONTINUATION_PTRS 8
#if DISPATCH_HW_CONFIG_UP
// UP devices don't contend on continuations so we don't need to force them to
// occupy a whole cacheline (which is intended to avoid contention)
#define DISPATCH_CONTINUATION_SIZE \
		(_DISPATCH_CONTINUATION_PTRS * DISPATCH_SIZEOF_PTR)
#else
#define DISPATCH_CONTINUATION_SIZE  ROUND_UP_TO_CACHELINE_SIZE( \
		(_DISPATCH_CONTINUATION_PTRS * DISPATCH_SIZEOF_PTR))
#endif
#define ROUND_UP_TO_CONTINUATION_SIZE(x) \
		(((x) + (DISPATCH_CONTINUATION_SIZE - 1u)) & \
		~(DISPATCH_CONTINUATION_SIZE - 1u))

// continuation is a dispatch_sync or dispatch_barrier_sync
#define DC_FLAG_SYNC_WAITER				0x001ul
// continuation acts as a barrier
#define DC_FLAG_BARRIER					0x002ul
// continuation resources are freed on run
// this is set on async or for non event_handler source handlers
#define DC_FLAG_CONSUME					0x004ul
// continuation has a group in dc_data
#define DC_FLAG_GROUP_ASYNC				0x008ul
// continuation function is a block (copied in dc_ctxt)
#define DC_FLAG_BLOCK					0x010ul
// continuation function is a block with private data, implies BLOCK_BIT
#define DC_FLAG_BLOCK_WITH_PRIVATE_DATA	0x020ul
// source handler requires fetching context from source
#define DC_FLAG_FETCH_CONTEXT			0x040ul
// continuation is a dispatch_async_and_wait
#define DC_FLAG_ASYNC_AND_WAIT			0x080ul
// bit used to make sure dc_flags is never 0 for allocated continuations
#define DC_FLAG_ALLOCATED				0x100ul
// continuation is an internal implementation detail that should not be
// introspected
#define DC_FLAG_NO_INTROSPECTION		0x200ul
// The item is a channel item, not a continuation
#define DC_FLAG_CHANNEL_ITEM			0x400ul

typedef struct dispatch_continuation_s {
	DISPATCH_CONTINUATION_HEADER(continuation);
} *dispatch_continuation_t;

dispatch_assert_aliases(dispatch_continuation_s, dispatch_object_s, do_next);
dispatch_assert_aliases(dispatch_continuation_s, dispatch_object_s, do_vtable);

typedef struct dispatch_sync_context_s {
	struct dispatch_continuation_s _as_dc[0];
	DISPATCH_CONTINUATION_HEADER(continuation);
	dispatch_function_t dsc_func;
	void *dsc_ctxt;
	dispatch_thread_frame_s dsc_dtf;
	dispatch_thread_event_s dsc_event;
	dispatch_tid dsc_waiter;
	uint8_t dsc_override_qos_floor;
	uint8_t dsc_override_qos;
	uint16_t dsc_autorelease : 2;
	uint16_t dsc_wlh_was_first : 1;
	uint16_t dsc_wlh_is_workloop : 1;
	uint16_t dsc_waiter_needs_cancel : 1;
	uint16_t dsc_release_storage : 1;
#if DISPATCH_INTROSPECTION
	uint16_t dsc_from_async : 1;
#endif
} *dispatch_sync_context_t;

typedef struct dispatch_continuation_vtable_s {
	_OS_OBJECT_CLASS_HEADER();
	DISPATCH_OBJECT_VTABLE_HEADER(dispatch_continuation);
} const *dispatch_continuation_vtable_t;

#ifndef DISPATCH_CONTINUATION_CACHE_LIMIT
#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#define DISPATCH_CONTINUATION_CACHE_LIMIT 112 // one 256k heap for 64 threads
#define DISPATCH_CONTINUATION_CACHE_LIMIT_MEMORYPRESSURE_PRESSURE_WARN 16
#else
#define DISPATCH_CONTINUATION_CACHE_LIMIT 1024
#define DISPATCH_CONTINUATION_CACHE_LIMIT_MEMORYPRESSURE_PRESSURE_WARN 128
#endif
#endif

dispatch_continuation_t _dispatch_continuation_alloc_from_heap(void);
void _dispatch_continuation_free_to_heap(dispatch_continuation_t c);
void _dispatch_continuation_pop(dispatch_object_t dou,
		dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags,
		dispatch_queue_class_t dqu);

#if DISPATCH_USE_MEMORYPRESSURE_SOURCE
extern int _dispatch_continuation_cache_limit;
void _dispatch_continuation_free_to_cache_limit(dispatch_continuation_t c);
#else
#define _dispatch_continuation_cache_limit DISPATCH_CONTINUATION_CACHE_LIMIT
#define _dispatch_continuation_free_to_cache_limit(c) \
		_dispatch_continuation_free_to_heap(c)
#endif

#pragma mark -
#pragma mark dispatch_continuation vtables

enum {
	_DC_USER_TYPE = 0,
	DC_ASYNC_REDIRECT_TYPE,
	DC_MACH_SEND_BARRRIER_DRAIN_TYPE,
	DC_MACH_SEND_BARRIER_TYPE,
	DC_MACH_RECV_BARRIER_TYPE,
	DC_MACH_ASYNC_REPLY_TYPE,
#if HAVE_PTHREAD_WORKQUEUE_QOS
	DC_WORKLOOP_STEALING_TYPE,
	DC_OVERRIDE_STEALING_TYPE,
	DC_OVERRIDE_OWNING_TYPE,
#endif
#if HAVE_MACH
	DC_MACH_IPC_HANDOFF_TYPE,
#endif
	_DC_MAX_TYPE,
};

DISPATCH_ALWAYS_INLINE
static inline unsigned long
dc_type(dispatch_continuation_t dc)
{
	return dx_type((struct dispatch_object_s *)dc);
}

extern const struct dispatch_continuation_vtable_s
		_dispatch_continuation_vtables[_DC_MAX_TYPE];

#define DC_VTABLE(name)  (&_dispatch_continuation_vtables[DC_##name##_TYPE])

#define DC_VTABLE_ENTRY(name, ...)  \
	[DC_##name##_TYPE] = { \
		.do_type = DISPATCH_CONTINUATION_TYPE(name), \
		__VA_ARGS__ \
	}

#pragma mark -
#pragma mark _dispatch_set_priority_and_voucher
#if HAVE_PTHREAD_WORKQUEUE_QOS

void _dispatch_set_priority_and_mach_voucher_slow(pthread_priority_t pri,
		mach_voucher_t kv);
voucher_t _dispatch_set_priority_and_voucher_slow(pthread_priority_t pri,
		voucher_t voucher, dispatch_thread_set_self_t flags);
#else
static inline void
_dispatch_set_priority_and_mach_voucher_slow(pthread_priority_t pri,
		mach_voucher_t kv)
{
	(void)pri; (void)kv;
}
#endif
#pragma mark -
#pragma mark dispatch_apply_t

struct dispatch_apply_s {
#if !OS_OBJECT_HAVE_OBJC1
	dispatch_continuation_t da_dc;
#endif
	size_t volatile da_index, da_todo;
	size_t da_iterations;
#if OS_OBJECT_HAVE_OBJC1
	dispatch_continuation_t da_dc;
#endif
	size_t da_nested;
	dispatch_thread_event_s da_event;
	dispatch_invoke_flags_t da_flags;
	int32_t da_thr_cnt;
};
dispatch_static_assert(offsetof(struct dispatch_continuation_s, dc_flags) ==
		offsetof(struct dispatch_apply_s, da_dc),
		"These fields must alias so that leaks instruments work");
typedef struct dispatch_apply_s *dispatch_apply_t;

#pragma mark -
#pragma mark dispatch_block_t

#ifdef __BLOCKS__

#define DISPATCH_BLOCK_API_MASK (0x100u - 1)
#define DISPATCH_BLOCK_HAS_VOUCHER (1u << 31)
#define DISPATCH_BLOCK_HAS_PRIORITY (1u << 30)

#define DISPATCH_BLOCK_PRIVATE_DATA_HEADER() \
	unsigned long dbpd_magic; \
	dispatch_block_flags_t dbpd_flags; \
	unsigned int volatile dbpd_atomic_flags; \
	int volatile dbpd_performed; \
	pthread_priority_t dbpd_priority; \
	voucher_t dbpd_voucher; \
	dispatch_block_t dbpd_block; \
	dispatch_group_t dbpd_group; \
	dispatch_queue_t dbpd_queue; \
	mach_port_t dbpd_thread;

#if !defined(__cplusplus)
struct dispatch_block_private_data_s {
	DISPATCH_BLOCK_PRIVATE_DATA_HEADER();
};
#endif
typedef struct dispatch_block_private_data_s *dispatch_block_private_data_t;

// dbpd_atomic_flags bits
#define DBF_CANCELED 1u // block has been cancelled
#define DBF_WAITING 2u // dispatch_block_wait has begun
#define DBF_WAITED 4u // dispatch_block_wait has finished without timeout
#define DBF_PERFORM 8u // dispatch_block_perform: don't group_leave

#define DISPATCH_BLOCK_PRIVATE_DATA_MAGIC 0xD159B10C // 0xDISPatch_BLOCk

// struct for synchronous perform: no group_leave at end of invoke
#define DISPATCH_BLOCK_PRIVATE_DATA_PERFORM_INITIALIZER(flags, block, voucher) \
		{ \
			.dbpd_magic = DISPATCH_BLOCK_PRIVATE_DATA_MAGIC, \
			.dbpd_flags = (flags), \
			.dbpd_atomic_flags = DBF_PERFORM, \
			.dbpd_block = (block), \
			.dbpd_voucher = (voucher), \
		}

extern void (*const _dispatch_block_special_invoke)(void*);

dispatch_block_t _dispatch_block_create(dispatch_block_flags_t flags,
		voucher_t voucher, pthread_priority_t priority, dispatch_block_t block);
void _dispatch_block_invoke_direct(const struct dispatch_block_private_data_s *dbcpd);
void _dispatch_block_sync_invoke(void *block);

void *_dispatch_continuation_get_function_symbol(dispatch_continuation_t dc);
dispatch_qos_t _dispatch_continuation_init_slow(dispatch_continuation_t dc,
		dispatch_queue_class_t dqu, dispatch_block_flags_t flags);

#endif /* __BLOCKS__ */

#endif
