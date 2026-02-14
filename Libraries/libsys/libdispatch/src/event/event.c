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

#pragma mark unote generic functions

static void _dispatch_timer_unote_register(dispatch_timer_source_refs_t dt,
		dispatch_wlh_t wlh, dispatch_priority_t pri);
static void _dispatch_timer_unote_resume(dispatch_timer_source_refs_t dt);
static void _dispatch_timer_unote_unregister(dispatch_timer_source_refs_t dt);

DISPATCH_NOINLINE
static dispatch_unote_t
_dispatch_unote_create(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask)
{
	dispatch_unote_linkage_t dul;
	dispatch_unote_class_t du;

	if (mask & ~dst->dst_mask) {
		return DISPATCH_UNOTE_NULL;
	}

	if (dst->dst_mask && !dst->dst_allow_empty_mask && !mask) {
		return DISPATCH_UNOTE_NULL;
	}

	if (dst->dst_flags & EV_UDATA_SPECIFIC) {
		du = _dispatch_calloc(1u, dst->dst_size);
	} else {
		dul = _dispatch_calloc(1u, sizeof(*dul) + dst->dst_size);
		du = _dispatch_unote_linkage_get_unote(dul)._du;
	}
	du->du_type = dst;
	du->du_can_be_wlh = dst->dst_per_trigger_qos;
	du->du_ident = (uint32_t)handle;
	du->du_filter = dst->dst_filter;
	du->du_fflags = (__typeof__(du->du_fflags))mask;
	if (dst->dst_flags & EV_UDATA_SPECIFIC) {
		du->du_is_direct = true;
	}
	return (dispatch_unote_t){ ._du = du };
}

DISPATCH_NOINLINE
dispatch_unote_t
_dispatch_unote_create_with_handle(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask)
{
	if (!handle) {
		return DISPATCH_UNOTE_NULL;
	}
	return _dispatch_unote_create(dst, handle, mask);
}

DISPATCH_NOINLINE
dispatch_unote_t
_dispatch_unote_create_with_fd(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask)
{
#if !TARGET_OS_MAC // <rdar://problem/27756657>
	if (handle > INT_MAX) {
		return DISPATCH_UNOTE_NULL;
	}
#endif
	return _dispatch_unote_create(dst, handle, mask);
}

DISPATCH_NOINLINE
dispatch_unote_t
_dispatch_unote_create_without_handle(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask)
{
	if (handle) {
		return DISPATCH_UNOTE_NULL;
	}
	return _dispatch_unote_create(dst, handle, mask);
}

DISPATCH_NOINLINE
void
_dispatch_unote_dispose(dispatch_unote_t du)
{
	void *ptr = du._du;
#if HAVE_MACH
	if (du._du->dmrr_handler_is_block) {
		Block_release(du._dmrr->dmrr_handler_ctxt);
	}
#endif
	if (du._du->du_is_timer) {
		if (unlikely(du._dt->dt_heap_entry[DTH_TARGET_ID] != DTH_INVALID_ID ||
				du._dt->dt_heap_entry[DTH_DEADLINE_ID] != DTH_INVALID_ID)) {
			DISPATCH_INTERNAL_CRASH(0, "Disposing of timer still in its heap");
		}
		if (unlikely(du._dt->dt_pending_config)) {
			free(du._dt->dt_pending_config);
			du._dt->dt_pending_config = NULL;
		}
	} else if (!du._du->du_is_direct) {
		ptr = _dispatch_unote_get_linkage(du);
	}
	free(ptr);
}

bool
_dispatch_unote_register(dispatch_unote_t du, dispatch_wlh_t wlh,
		dispatch_priority_t pri)
{
	dispatch_assert(du._du->du_is_timer || !_dispatch_unote_registered(du));
	dispatch_priority_t masked_pri;

	masked_pri = pri & (DISPATCH_PRIORITY_FLAG_MANAGER |
			DISPATCH_PRIORITY_FLAG_FALLBACK |
			DISPATCH_PRIORITY_FLAG_FLOOR |
			DISPATCH_PRIORITY_FALLBACK_QOS_MASK |
			DISPATCH_PRIORITY_REQUESTED_MASK);

	dispatch_assert(wlh == DISPATCH_WLH_ANON || masked_pri);
	if (masked_pri == _dispatch_priority_make_fallback(DISPATCH_QOS_DEFAULT)) {
		_dispatch_ktrace1(DISPATCH_PERF_source_registration_without_qos,
				_dispatch_wref2ptr(du._du->du_owner_wref));
	}

	du._du->du_priority = pri;

	switch (du._du->du_filter) {
	case DISPATCH_EVFILT_CUSTOM_ADD:
	case DISPATCH_EVFILT_CUSTOM_OR:
	case DISPATCH_EVFILT_CUSTOM_REPLACE:
		_dispatch_unote_state_set(du, DISPATCH_WLH_ANON, DU_STATE_ARMED);
		return true;
	}
	if (du._du->du_is_timer) {
		_dispatch_timer_unote_register(du._dt, wlh, pri);
		return true;
	}
#if DISPATCH_HAVE_DIRECT_KNOTES
	if (du._du->du_is_direct) {
		return _dispatch_unote_register_direct(du, wlh);
	}
#endif
	return _dispatch_unote_register_muxed(du);
}

void
_dispatch_unote_resume(dispatch_unote_t du)
{
	dispatch_assert(du._du->du_is_timer || _dispatch_unote_needs_rearm(du));
	if (du._du->du_is_timer) {
		_dispatch_timer_unote_resume(du._dt);
#if DISPATCH_HAVE_DIRECT_KNOTES
	} else if (du._du->du_is_direct) {
		_dispatch_unote_resume_direct(du);
#endif
	} else {
		_dispatch_unote_resume_muxed(du);
	}
}

bool
_dispatch_unote_unregister(dispatch_unote_t du, uint32_t flags)
{
	if (!_dispatch_unote_registered(du)) {
		return true;
	}
	switch (du._du->du_filter) {
	case DISPATCH_EVFILT_CUSTOM_ADD:
	case DISPATCH_EVFILT_CUSTOM_OR:
	case DISPATCH_EVFILT_CUSTOM_REPLACE:
		_dispatch_unote_state_set(du, DU_STATE_UNREGISTERED);
		return true;
	}
	if (du._du->du_is_timer) {
		_dispatch_timer_unote_unregister(du._dt);
		return true;
	}
#if DISPATCH_HAVE_DIRECT_KNOTES
	if (du._du->du_is_direct) {
		return _dispatch_unote_unregister_direct(du, flags);
	}
#endif

	dispatch_assert(flags & DUU_DELETE_ACK);
	return _dispatch_unote_unregister_muxed(du);
}

#pragma mark data or / add

static dispatch_unote_t
_dispatch_source_data_create(dispatch_source_type_t dst, uintptr_t handle,
		unsigned long mask)
{
	if (handle || mask) {
		return DISPATCH_UNOTE_NULL;
	}

	// bypass _dispatch_unote_create() because this is always "direct"
	// even when EV_UDATA_SPECIFIC is 0
	dispatch_unote_class_t du = _dispatch_calloc(1u, dst->dst_size);
	du->du_type = dst;
	du->du_filter = dst->dst_filter;
	du->du_is_direct = true;
	return (dispatch_unote_t){ ._du = du };
}

const dispatch_source_type_s _dispatch_source_type_data_add = {
	.dst_kind       = "data-add",
	.dst_filter     = DISPATCH_EVFILT_CUSTOM_ADD,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_CLEAR,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_DATA,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_source_data_create,
	.dst_merge_evt  = NULL,
};

const dispatch_source_type_s _dispatch_source_type_data_or = {
	.dst_kind       = "data-or",
	.dst_filter     = DISPATCH_EVFILT_CUSTOM_OR,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_CLEAR,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_DATA,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_source_data_create,
	.dst_merge_evt  = NULL,
};

const dispatch_source_type_s _dispatch_source_type_data_replace = {
	.dst_kind       = "data-replace",
	.dst_filter     = DISPATCH_EVFILT_CUSTOM_REPLACE,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_CLEAR,
	.dst_action     = DISPATCH_UNOTE_ACTION_PASS_DATA,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_source_data_create,
	.dst_merge_evt  = NULL,
};

#pragma mark file descriptors

const dispatch_source_type_s _dispatch_source_type_read = {
	.dst_kind       = "read",
	.dst_filter     = EVFILT_READ,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH|EV_VANISHED,
#if DISPATCH_EVENT_BACKEND_KEVENT
#if HAVE_DECL_NOTE_LOWAT
	.dst_fflags     = NOTE_LOWAT,
#endif
	.dst_data       = 1,
#endif // DISPATCH_EVENT_BACKEND_KEVENT
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_SET_DATA,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_fd,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};

const dispatch_source_type_s _dispatch_source_type_write = {
	.dst_kind       = "write",
	.dst_filter     = EVFILT_WRITE,
	.dst_flags      = EV_UDATA_SPECIFIC|EV_DISPATCH|EV_VANISHED,
#if DISPATCH_EVENT_BACKEND_KEVENT
#if HAVE_DECL_NOTE_LOWAT
	.dst_fflags     = NOTE_LOWAT,
#endif
	.dst_data       = 1,
#endif // DISPATCH_EVENT_BACKEND_KEVENT
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_SET_DATA,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_unote_create_with_fd,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};

#pragma mark signals

static dispatch_unote_t
_dispatch_source_signal_create(dispatch_source_type_t dst, uintptr_t handle,
		unsigned long mask)
{
	if (handle >= NSIG) {
		return DISPATCH_UNOTE_NULL;
	}
	return _dispatch_unote_create_with_handle(dst, handle, mask);
}

const dispatch_source_type_s _dispatch_source_type_signal = {
	.dst_kind       = "signal",
	.dst_filter     = EVFILT_SIGNAL,
	.dst_flags      = DISPATCH_EV_DIRECT|EV_CLEAR,
	.dst_action     = DISPATCH_UNOTE_ACTION_SOURCE_ADD_DATA,
	.dst_size       = sizeof(struct dispatch_source_refs_s),
	.dst_strict     = false,

	.dst_create     = _dispatch_source_signal_create,
	.dst_merge_evt  = _dispatch_source_merge_evt,
};

#pragma mark -
#pragma mark timer globals

DISPATCH_GLOBAL(struct dispatch_timer_heap_s
_dispatch_timers_heap[DISPATCH_TIMER_COUNT]);

#if DISPATCH_USE_DTRACE
DISPATCH_STATIC_GLOBAL(dispatch_timer_source_refs_t
_dispatch_trace_next_timer[DISPATCH_TIMER_QOS_COUNT]);
#define _dispatch_trace_next_timer_set(x, q) \
		_dispatch_trace_next_timer[(q)] = (x)
#define _dispatch_trace_next_timer_program(d, q) \
		_dispatch_trace_timer_program(_dispatch_trace_next_timer[(q)], (d))
#else
#define _dispatch_trace_next_timer_set(x, q)
#define _dispatch_trace_next_timer_program(d, q)
#endif

#pragma mark timer heap
/*
 * The dispatch_timer_heap_t structure is a double min-heap of timers,
 * interleaving the by-target min-heap in the even slots, and the by-deadline
 * in the odd ones.
 *
 * The min element of these is held inline in the dispatch_timer_heap_t
 * structure, and further entries are held in segments.
 *
 * dth_segments is the number of allocated segments.
 *
 * Segment 0 has a size of `DISPATCH_HEAP_INIT_SEGMENT_CAPACITY` pointers
 * Segment k has a size of (DISPATCH_HEAP_INIT_SEGMENT_CAPACITY << (k - 1))
 *
 * Segment n (dth_segments - 1) is the last segment and points its final n
 * entries to previous segments. Its address is held in the `dth_heap` field.
 *
 * segment n   [ regular timer pointers | n-1 | k | 0 ]
 *                                         |    |   |
 * segment n-1 <---------------------------'    |   |
 * segment k   <--------------------------------'   |
 * segment 0   <------------------------------------'
 */
#define DISPATCH_HEAP_INIT_SEGMENT_CAPACITY 8u

/*
 * There are two min-heaps stored interleaved in a single array,
 * even indices are for the by-target min-heap, and odd indices for
 * the by-deadline one.
 */
#define DTH_HEAP_ID_MASK (DTH_ID_COUNT - 1)
#define DTH_HEAP_ID(idx) ((idx) & DTH_HEAP_ID_MASK)
#define DTH_IDX_FOR_HEAP_ID(idx, heap_id) \
		(((idx) & ~DTH_HEAP_ID_MASK) | (heap_id))

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_dispatch_timer_heap_capacity(uint32_t segments)
{
	if (segments == 0) return 2;
	uint32_t seg_no = segments - 1;
	// for C = DISPATCH_HEAP_INIT_SEGMENT_CAPACITY,
	// 2 + C + SUM(C << (i-1), i = 1..seg_no) - seg_no
	return 2 + (DISPATCH_HEAP_INIT_SEGMENT_CAPACITY << seg_no) - seg_no;
}

static void
_dispatch_timer_heap_grow(dispatch_timer_heap_t dth)
{
	uint32_t seg_capacity = DISPATCH_HEAP_INIT_SEGMENT_CAPACITY;
	uint32_t seg_no = dth->dth_segments++;
	void **heap, **heap_prev = dth->dth_heap;

	if (seg_no > 0) {
		seg_capacity <<= (seg_no - 1);
	}
	heap = _dispatch_calloc(seg_capacity, sizeof(void *));
	if (seg_no > 1) {
		uint32_t prev_seg_no = seg_no - 1;
		uint32_t prev_seg_capacity = seg_capacity >> 1;
		memcpy(&heap[seg_capacity - prev_seg_no],
				&heap_prev[prev_seg_capacity - prev_seg_no],
				prev_seg_no * sizeof(void *));
	}
	if (seg_no > 0) {
		heap[seg_capacity - seg_no] = heap_prev;
	}
	dth->dth_heap = heap;
}

static void
_dispatch_timer_heap_shrink(dispatch_timer_heap_t dth)
{
	uint32_t seg_capacity = DISPATCH_HEAP_INIT_SEGMENT_CAPACITY;
	uint32_t seg_no = --dth->dth_segments;
	void **heap = dth->dth_heap, **heap_prev = NULL;

	if (seg_no > 0) {
		seg_capacity <<= (seg_no - 1);
		heap_prev = heap[seg_capacity - seg_no];
	}
	if (seg_no > 1) {
		uint32_t prev_seg_no = seg_no - 1;
		uint32_t prev_seg_capacity = seg_capacity >> 1;
		memcpy(&heap_prev[prev_seg_capacity - prev_seg_no],
				&heap[seg_capacity - prev_seg_no],
				prev_seg_no * sizeof(void *));
	}
	dth->dth_heap = heap_prev;
	free(heap);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_timer_source_refs_t *
_dispatch_timer_heap_get_slot(dispatch_timer_heap_t dth, uint32_t idx)
{
	uint32_t seg_no, segments = dth->dth_segments;
	void **segment;

	if (idx < DTH_ID_COUNT) {
		return &dth->dth_min[idx];
	}
	idx -= DTH_ID_COUNT;

	// Derive the segment number from the index. Naming
	// DISPATCH_HEAP_INIT_SEGMENT_CAPACITY `C`, the segments index ranges are:
	// 0: 0 .. (C - 1)
	// 1: C .. 2 * C - 1
	// k: 2^(k-1) * C .. 2^k * C - 1
	// so `k` can be derived from the first bit set in `idx`
	seg_no = (uint32_t)(__builtin_clz(DISPATCH_HEAP_INIT_SEGMENT_CAPACITY - 1) -
			__builtin_clz(idx | (DISPATCH_HEAP_INIT_SEGMENT_CAPACITY - 1)));
	if (seg_no + 1 == segments) {
		segment = dth->dth_heap;
	} else {
		uint32_t seg_capacity = DISPATCH_HEAP_INIT_SEGMENT_CAPACITY;
		seg_capacity <<= (segments - 2);
		segment = dth->dth_heap[seg_capacity - seg_no - 1];
	}
	if (seg_no) {
		idx -= DISPATCH_HEAP_INIT_SEGMENT_CAPACITY << (seg_no - 1);
	}
	return (dispatch_timer_source_refs_t *)(segment + idx);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_timer_heap_set(dispatch_timer_heap_t dth,
		dispatch_timer_source_refs_t *slot,
		dispatch_timer_source_refs_t dt, uint32_t idx)
{
	if (idx < DTH_ID_COUNT) {
		dth->dth_needs_program = true;
	}
	*slot = dt;
	dt->dt_heap_entry[DTH_HEAP_ID(idx)] = idx;
}

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_dispatch_timer_heap_parent(uint32_t idx)
{
	uint32_t heap_id = DTH_HEAP_ID(idx);
	idx = (idx - DTH_ID_COUNT) / 2; // go to the parent
	return DTH_IDX_FOR_HEAP_ID(idx, heap_id);
}

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_dispatch_timer_heap_left_child(uint32_t idx)
{
	uint32_t heap_id = DTH_HEAP_ID(idx);
	// 2 * (idx - heap_id) + DTH_ID_COUNT + heap_id
	return 2 * idx + DTH_ID_COUNT - heap_id;
}

#if DISPATCH_HAVE_TIMER_COALESCING
DISPATCH_ALWAYS_INLINE
static inline uint32_t
_dispatch_timer_heap_walk_skip(uint32_t idx, uint32_t count)
{
	uint32_t heap_id = DTH_HEAP_ID(idx);

	idx -= heap_id;
	if (unlikely(idx + DTH_ID_COUNT == count)) {
		// reaching `count` doesn't mean we're done, but there is a weird
		// corner case if the last item of the heap is a left child:
		//
		//     /\
		//    /  \
		//   /  __\
		//  /__/
		//     ^
		//
		// The formula below would return the sibling of `idx` which is
		// out of bounds. Fortunately, the correct answer is the same
		// as for idx's parent
		idx = _dispatch_timer_heap_parent(idx);
	}

	//
	// When considering the index in a non interleaved, 1-based array
	// representation of a heap, hence looking at (idx / DTH_ID_COUNT + 1)
	// for a given idx in our dual-heaps, that index is in one of two forms:
	//
	//     (a) 1xxxx011111    or    (b) 111111111
	//         d    i    0              d       0
	//
	// The first bit set is the row of the binary tree node (0-based).
	// The following digits from most to least significant represent the path
	// to that node, where `0` is a left turn and `1` a right turn.
	//
	// For example 0b0101 (5) is a node on row 2 accessed going left then right:
	//
	// row 0          1
	//              /   .
	// row 1      2       3
	//           . \     . .
	// row 2    4   5   6   7
	//         : : : : : : : :
	//
	// Skipping a sub-tree in walk order means going to the sibling of the last
	// node reached after we turned left. If the node was of the form (a),
	// this node is 1xxxx1, which for the above example is 0b0011 (3).
	// If the node was of the form (b) then we never took a left, meaning
	// we reached the last element in traversal order.
	//

	//
	// we want to find
	// - the least significant bit set to 0 in (idx / DTH_ID_COUNT + 1)
	// - which is offset by log_2(DTH_ID_COUNT) from the position of the least
	//   significant 0 in (idx + DTH_ID_COUNT + DTH_ID_COUNT - 1)
	//   since idx is a multiple of DTH_ID_COUNT and DTH_ID_COUNT a power of 2.
	// - which in turn is the same as the position of the least significant 1 in
	//   ~(idx + DTH_ID_COUNT + DTH_ID_COUNT - 1)
	//
	dispatch_static_assert(powerof2(DTH_ID_COUNT));
	idx += DTH_ID_COUNT + DTH_ID_COUNT - 1;
	idx >>= __builtin_ctz(~idx);

	//
	// `idx` is now either:
	// - 0 if it was the (b) case above, in which case the walk is done
	// - 1xxxx0 as the position in a 0 based array representation of a non
	//   interleaved heap, so we just have to compute the interleaved index.
	//
	return likely(idx) ? DTH_ID_COUNT * idx + heap_id : UINT32_MAX;
}

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_dispatch_timer_heap_walk_next(uint32_t idx, uint32_t count)
{
	//
	// Goes to the next element in heap walk order, which is the prefix ordered
	// walk of the tree.
	//
	// From a given node, the next item to return is the left child if it
	// exists, else the first right sibling we find by walking our parent chain,
	// which is exactly what _dispatch_timer_heap_walk_skip() returns.
	//
	uint32_t lchild = _dispatch_timer_heap_left_child(idx);
	if (lchild < count) {
		return lchild;
	}
	return _dispatch_timer_heap_walk_skip(idx, count);
}

static uint64_t
_dispatch_timer_heap_max_target_before(dispatch_timer_heap_t dth, uint64_t limit)
{
	dispatch_timer_source_refs_t dri;
	uint32_t idx = _dispatch_timer_heap_left_child(DTH_TARGET_ID);
	uint32_t count = dth->dth_count;
	uint64_t tmp, target = dth->dth_min[DTH_TARGET_ID]->dt_timer.target;

	while (idx < count) {
		dri = *_dispatch_timer_heap_get_slot(dth, idx);
		tmp = dri->dt_timer.target;
		if (tmp > limit) {
			// skip subtree since none of the targets below can be before limit
			idx = _dispatch_timer_heap_walk_skip(idx, count);
		} else {
			target = tmp;
			idx = _dispatch_timer_heap_walk_next(idx, count);
		}
	}
	return target;
}
#endif // DISPATCH_HAVE_TIMER_COALESCING

static void
_dispatch_timer_heap_resift(dispatch_timer_heap_t dth,
		dispatch_timer_source_refs_t dt, uint32_t idx)
{
	dispatch_static_assert(offsetof(struct dispatch_timer_source_s, target) ==
			offsetof(struct dispatch_timer_source_s, heap_key[DTH_TARGET_ID]));
	dispatch_static_assert(offsetof(struct dispatch_timer_source_s, deadline) ==
			offsetof(struct dispatch_timer_source_s, heap_key[DTH_DEADLINE_ID]));
#define dth_cmp(hid, dt1, op, dt2) \
		(((dt1)->dt_timer.heap_key)[hid] op ((dt2)->dt_timer.heap_key)[hid])

	dispatch_timer_source_refs_t *pslot, pdt;
	dispatch_timer_source_refs_t *cslot, cdt;
	dispatch_timer_source_refs_t *rslot, rdt;
	uint32_t cidx, dth_count = dth->dth_count;
	dispatch_timer_source_refs_t *slot;
	int heap_id = DTH_HEAP_ID(idx);
	bool sifted_up = false;

	// try to sift up

	slot = _dispatch_timer_heap_get_slot(dth, idx);
	while (idx >= DTH_ID_COUNT) {
		uint32_t pidx = _dispatch_timer_heap_parent(idx);
		pslot = _dispatch_timer_heap_get_slot(dth, pidx);
		pdt = *pslot;
		if (dth_cmp(heap_id, pdt, <=, dt)) {
			break;
		}
		_dispatch_timer_heap_set(dth, slot, pdt, idx);
		slot = pslot;
		idx = pidx;
		sifted_up = true;
	}
	if (sifted_up) {
		goto done;
	}

	// try to sift down

	while ((cidx = _dispatch_timer_heap_left_child(idx)) < dth_count) {
		uint32_t ridx = cidx + DTH_ID_COUNT;
		cslot = _dispatch_timer_heap_get_slot(dth, cidx);
		cdt = *cslot;
		if (ridx < dth_count) {
			rslot = _dispatch_timer_heap_get_slot(dth, ridx);
			rdt = *rslot;
			if (dth_cmp(heap_id, cdt, >, rdt)) {
				cidx = ridx;
				cdt = rdt;
				cslot = rslot;
			}
		}
		if (dth_cmp(heap_id, dt, <=, cdt)) {
			break;
		}
		_dispatch_timer_heap_set(dth, slot, cdt, idx);
		slot = cslot;
		idx = cidx;
	}

done:
	_dispatch_timer_heap_set(dth, slot, dt, idx);
#undef dth_cmp
}

DISPATCH_ALWAYS_INLINE
static void
_dispatch_timer_heap_insert(dispatch_timer_heap_t dth,
		dispatch_timer_source_refs_t dt)
{
	uint32_t idx = (dth->dth_count += DTH_ID_COUNT) - DTH_ID_COUNT;

	DISPATCH_TIMER_ASSERT(dt->dt_heap_entry[DTH_TARGET_ID], ==,
			DTH_INVALID_ID, "target idx");
	DISPATCH_TIMER_ASSERT(dt->dt_heap_entry[DTH_DEADLINE_ID], ==,
			DTH_INVALID_ID, "deadline idx");

	dispatch_qos_t qos = MAX(_dispatch_priority_qos(dt->du_priority),
			_dispatch_priority_fallback_qos(dt->du_priority));
	if (dth->dth_max_qos < qos) {
		dth->dth_max_qos = (uint8_t)qos;
		dth->dth_needs_program = true;
	}

	if (idx == 0) {
		dth->dth_needs_program = true;
		dt->dt_heap_entry[DTH_TARGET_ID] = DTH_TARGET_ID;
		dt->dt_heap_entry[DTH_DEADLINE_ID] = DTH_DEADLINE_ID;
		dth->dth_min[DTH_TARGET_ID] = dth->dth_min[DTH_DEADLINE_ID] = dt;
		return;
	}

	if (unlikely(idx + DTH_ID_COUNT >
			_dispatch_timer_heap_capacity(dth->dth_segments))) {
		_dispatch_timer_heap_grow(dth);
	}
	_dispatch_timer_heap_resift(dth, dt, idx + DTH_TARGET_ID);
	_dispatch_timer_heap_resift(dth, dt, idx + DTH_DEADLINE_ID);
}

static void
_dispatch_timer_heap_remove(dispatch_timer_heap_t dth,
		dispatch_timer_source_refs_t dt)
{
	uint32_t idx = (dth->dth_count -= DTH_ID_COUNT);

	DISPATCH_TIMER_ASSERT(dt->dt_heap_entry[DTH_TARGET_ID], !=,
			DTH_INVALID_ID, "target idx");
	DISPATCH_TIMER_ASSERT(dt->dt_heap_entry[DTH_DEADLINE_ID], !=,
			DTH_INVALID_ID, "deadline idx");

	if (idx == 0) {
		DISPATCH_TIMER_ASSERT(dth->dth_min[DTH_TARGET_ID], ==, dt,
				"target slot");
		DISPATCH_TIMER_ASSERT(dth->dth_min[DTH_DEADLINE_ID], ==, dt,
				"deadline slot");
		dth->dth_needs_program = true;
		dth->dth_min[DTH_TARGET_ID] = dth->dth_min[DTH_DEADLINE_ID] = NULL;
		goto clear_heap_entry;
	}

	for (uint32_t heap_id = 0; heap_id < DTH_ID_COUNT; heap_id++) {
		dispatch_timer_source_refs_t *slot, last_dt;
		slot = _dispatch_timer_heap_get_slot(dth, idx + heap_id);
		last_dt = *slot; *slot = NULL;
		if (last_dt != dt) {
			uint32_t removed_idx = dt->dt_heap_entry[heap_id];
			_dispatch_timer_heap_resift(dth, last_dt, removed_idx);
		}
	}
	if (unlikely(idx <= _dispatch_timer_heap_capacity(dth->dth_segments - 1))) {
		_dispatch_timer_heap_shrink(dth);
	}

clear_heap_entry:
	dt->dt_heap_entry[DTH_TARGET_ID] = DTH_INVALID_ID;
	dt->dt_heap_entry[DTH_DEADLINE_ID] = DTH_INVALID_ID;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_timer_heap_update(dispatch_timer_heap_t dth,
		dispatch_timer_source_refs_t dt)
{
	DISPATCH_TIMER_ASSERT(dt->dt_heap_entry[DTH_TARGET_ID], !=,
			DTH_INVALID_ID, "target idx");
	DISPATCH_TIMER_ASSERT(dt->dt_heap_entry[DTH_DEADLINE_ID], !=,
			DTH_INVALID_ID, "deadline idx");

	_dispatch_timer_heap_resift(dth, dt, dt->dt_heap_entry[DTH_TARGET_ID]);
	_dispatch_timer_heap_resift(dth, dt, dt->dt_heap_entry[DTH_DEADLINE_ID]);
}

#pragma mark timer unote

#define _dispatch_timer_du_debug(what, du) \
		_dispatch_debug("kevent-source[%p]: %s kevent[%p] { ident = 0x%x }", \
				_dispatch_wref2ptr((du)->du_owner_wref), what, \
				(du), (du)->du_ident)

DISPATCH_ALWAYS_INLINE
static inline unsigned int
_dispatch_timer_unote_idx(dispatch_timer_source_refs_t dt)
{
	dispatch_clock_t clock = _dispatch_timer_flags_to_clock(dt->du_timer_flags);
	uint32_t qos = 0;

#if DISPATCH_HAVE_TIMER_QOS
	dispatch_assert(DISPATCH_TIMER_STRICT == DISPATCH_TIMER_QOS_CRITICAL);
	dispatch_assert(DISPATCH_TIMER_BACKGROUND == DISPATCH_TIMER_QOS_BACKGROUND);
	qos = dt->du_timer_flags & (DISPATCH_TIMER_STRICT|DISPATCH_TIMER_BACKGROUND);
	// flags are normalized so this should never happen
	dispatch_assert(qos < DISPATCH_TIMER_QOS_COUNT);
#endif

	return DISPATCH_TIMER_INDEX(clock, qos);
}

static void
_dispatch_timer_unote_disarm(dispatch_timer_source_refs_t dt,
		dispatch_timer_heap_t dth)
{
	uint32_t tidx = dt->du_ident;

	dispatch_assert(_dispatch_unote_armed(dt));
	_dispatch_timer_heap_remove(&dth[tidx], dt);
	_dispatch_timers_heap_dirty(dth, tidx);
	_dispatch_unote_state_clear_bit(dt, DU_STATE_ARMED);
	_dispatch_timer_du_debug("disarmed", dt);
}

static void
_dispatch_timer_unote_arm(dispatch_timer_source_refs_t dt,
		dispatch_timer_heap_t dth, uint32_t tidx)
{
	if (_dispatch_unote_armed(dt)) {
		DISPATCH_TIMER_ASSERT(dt->du_ident, ==, tidx, "tidx");
		_dispatch_timer_heap_update(&dth[tidx], dt);
		_dispatch_timer_du_debug("updated", dt);
	} else {
		dt->du_ident = tidx;
		_dispatch_timer_heap_insert(&dth[tidx], dt);
		_dispatch_unote_state_set_bit(dt, DU_STATE_ARMED);
		_dispatch_timer_du_debug("armed", dt);
	}
	_dispatch_timers_heap_dirty(dth, tidx);
}

#define DISPATCH_TIMER_UNOTE_TRACE_SUSPENSION 0x1

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_timer_unote_needs_rearm(dispatch_timer_source_refs_t dr, int flags)
{
	dispatch_source_t ds = _dispatch_source_from_refs(dr);
	if (unlikely(DISPATCH_QUEUE_IS_SUSPENDED(ds))) {
		if (flags & DISPATCH_TIMER_UNOTE_TRACE_SUSPENSION) {
			_dispatch_ktrace1(DISPATCH_PERF_suspended_timer_fire, ds);
		}
		return false;
	}
	return dr->du_ident != DISPATCH_TIMER_IDENT_CANCELED &&
			dr->dt_timer.target < INT64_MAX;
}

DISPATCH_NOINLINE
static void
_dispatch_timer_unote_register(dispatch_timer_source_refs_t dt,
		dispatch_wlh_t wlh, dispatch_priority_t pri)
{
	// aggressively coalesce background/maintenance QoS timers
	// <rdar://problem/12200216&27342536>
	if (_dispatch_qos_is_background(_dispatch_priority_qos(pri))) {
		if (dt->du_timer_flags & DISPATCH_TIMER_STRICT) {
			_dispatch_ktrace1(DISPATCH_PERF_strict_bg_timer,
					_dispatch_source_from_refs(dt));
		} else {
			dt->du_timer_flags |= DISPATCH_TIMER_BACKGROUND;
			dt->du_ident = _dispatch_timer_unote_idx(dt);
		}
	}
	// _dispatch_source_activate() can pre-set a wlh for timers directly
	// attached to their workloops.
	if (_dispatch_unote_wlh(dt) != wlh) {
		dispatch_assert(_dispatch_unote_wlh(dt) == NULL);
		_dispatch_unote_state_set(dt, DISPATCH_WLH_ANON, 0);
	}
	if (os_atomic_load2o(dt, dt_pending_config, relaxed)) {
		_dispatch_timer_unote_configure(dt);
	}
}

void
_dispatch_timer_unote_configure(dispatch_timer_source_refs_t dt)
{
	dispatch_timer_config_t dtc;

	dtc = os_atomic_xchg2o(dt, dt_pending_config, NULL, dependency);
	if (dtc->dtc_clock != _dispatch_timer_flags_to_clock(dt->du_timer_flags)) {
		dt->du_timer_flags &= ~_DISPATCH_TIMER_CLOCK_MASK;
		dt->du_timer_flags |= _dispatch_timer_flags_from_clock(dtc->dtc_clock);
	}
	dt->dt_timer = dtc->dtc_timer;
	free(dtc);
	// Clear any pending data that might have accumulated on
	// older timer params <rdar://problem/8574886>
	os_atomic_store2o(dt, ds_pending_data, 0, relaxed);

	if (_dispatch_unote_armed(dt)) {
		return _dispatch_timer_unote_resume(dt);
	}
}

static inline dispatch_timer_heap_t
_dispatch_timer_unote_heap(dispatch_timer_source_refs_t dt)
{
	dispatch_wlh_t wlh = _dispatch_unote_wlh(dt);
	if (wlh == DISPATCH_WLH_ANON) {
		return _dispatch_timers_heap;
	}
	return ((dispatch_workloop_t)wlh)->dwl_timer_heap;
}

DISPATCH_NOINLINE
static void
_dispatch_timer_unote_resume(dispatch_timer_source_refs_t dt)
{
	// ... and now reflect any impact the reconfiguration has to the heap.
	// The heap also owns a +2 on dispatch sources it references, so maintain
	// this invariant as we tweak the registration.

	bool will_arm = _dispatch_timer_unote_needs_rearm(dt, 0);
	bool was_armed = _dispatch_unote_armed(dt);
	uint32_t tidx = _dispatch_timer_unote_idx(dt);
	dispatch_timer_heap_t dth = _dispatch_timer_unote_heap(dt);

	if (unlikely(was_armed && (!will_arm || dt->du_ident != tidx))) {
		_dispatch_timer_unote_disarm(dt, dth);
	}
	if (will_arm) {
		if (!was_armed) _dispatch_retain_unote_owner(dt);
		_dispatch_timer_unote_arm(dt, dth, tidx);
	} else if (was_armed) {
		_dispatch_release_unote_owner_tailcall(dt);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_timer_unote_unregister(dispatch_timer_source_refs_t dt)
{
	dispatch_timer_heap_t dth = _dispatch_timer_unote_heap(dt);
	if (_dispatch_unote_armed(dt)) {
		_dispatch_timer_unote_disarm(dt, dth);
		_dispatch_release_2_no_dispose(_dispatch_source_from_refs(dt));
	}
	_dispatch_wlh_release(_dispatch_unote_wlh(dt));
	_dispatch_unote_state_set(dt, DU_STATE_UNREGISTERED);
	dt->du_ident = DISPATCH_TIMER_IDENT_CANCELED;
}

static dispatch_unote_t
_dispatch_source_timer_create(dispatch_source_type_t dst,
		uintptr_t handle, unsigned long mask)
{
	dispatch_timer_source_refs_t dt;

	// normalize flags
	if (mask & DISPATCH_TIMER_STRICT) {
		mask &= ~(unsigned long)DISPATCH_TIMER_BACKGROUND;
	}
	if (mask & ~dst->dst_mask) {
		return DISPATCH_UNOTE_NULL;
	}

	if (dst->dst_timer_flags & DISPATCH_TIMER_INTERVAL) {
		if (!handle) return DISPATCH_UNOTE_NULL;
	} else if (dst->dst_filter == DISPATCH_EVFILT_TIMER_WITH_CLOCK) {
		if (handle) return DISPATCH_UNOTE_NULL;
	} else switch (handle) {
	case 0:
		break;
	case DISPATCH_CLOCKID_UPTIME:
		dst = &_dispatch_source_type_timer_with_clock;
		mask |= DISPATCH_TIMER_CLOCK_UPTIME;
		break;
	case DISPATCH_CLOCKID_MONOTONIC:
		dst = &_dispatch_source_type_timer_with_clock;
		mask |= DISPATCH_TIMER_CLOCK_MONOTONIC;
		break;
	case DISPATCH_CLOCKID_WALLTIME:
		dst = &_dispatch_source_type_timer_with_clock;
		mask |= DISPATCH_TIMER_CLOCK_WALL;
		break;
	default:
		return DISPATCH_UNOTE_NULL;
	}

	dt = _dispatch_calloc(1u, dst->dst_size);
	dt->du_type = dst;
	dt->du_filter = dst->dst_filter;
	dt->du_is_timer = true;
	dt->du_timer_flags |= (uint8_t)(mask | dst->dst_timer_flags);
	dt->du_ident = _dispatch_timer_unote_idx(dt);
	dt->dt_timer.target = UINT64_MAX;
	dt->dt_timer.deadline = UINT64_MAX;
	dt->dt_timer.interval = UINT64_MAX;
	dt->dt_heap_entry[DTH_TARGET_ID] = DTH_INVALID_ID;
	dt->dt_heap_entry[DTH_DEADLINE_ID] = DTH_INVALID_ID;
	return (dispatch_unote_t){ ._dt = dt };
}

const dispatch_source_type_s _dispatch_source_type_timer = {
	.dst_kind           = "timer",
	.dst_filter         = DISPATCH_EVFILT_TIMER,
	.dst_flags          = EV_DISPATCH,
	.dst_mask           = DISPATCH_TIMER_STRICT|DISPATCH_TIMER_BACKGROUND,
	.dst_timer_flags    = 0,
	.dst_action         = DISPATCH_UNOTE_ACTION_SOURCE_TIMER,
	.dst_size           = sizeof(struct dispatch_timer_source_refs_s),
	.dst_strict         = false,

	.dst_create         = _dispatch_source_timer_create,
	.dst_merge_evt      = _dispatch_source_merge_evt,
};

const dispatch_source_type_s _dispatch_source_type_timer_with_clock = {
	.dst_kind           = "timer (fixed-clock)",
	.dst_filter         = DISPATCH_EVFILT_TIMER_WITH_CLOCK,
	.dst_flags          = EV_DISPATCH,
	.dst_mask           = DISPATCH_TIMER_STRICT|DISPATCH_TIMER_BACKGROUND,
	.dst_timer_flags    = 0,
	.dst_action         = DISPATCH_UNOTE_ACTION_SOURCE_TIMER,
	.dst_size           = sizeof(struct dispatch_timer_source_refs_s),

	.dst_create         = _dispatch_source_timer_create,
	.dst_merge_evt      = _dispatch_source_merge_evt,
};

const dispatch_source_type_s _dispatch_source_type_after = {
	.dst_kind           = "timer (after)",
	.dst_filter         = DISPATCH_EVFILT_TIMER_WITH_CLOCK,
	.dst_flags          = EV_DISPATCH,
	.dst_mask           = 0,
	.dst_timer_flags    = DISPATCH_TIMER_AFTER,
	.dst_action         = DISPATCH_UNOTE_ACTION_SOURCE_TIMER,
	.dst_size           = sizeof(struct dispatch_timer_source_refs_s),

	.dst_create         = _dispatch_source_timer_create,
	.dst_merge_evt      = _dispatch_source_merge_evt,
};

const dispatch_source_type_s _dispatch_source_type_interval = {
	.dst_kind           = "timer (interval)",
	.dst_filter         = DISPATCH_EVFILT_TIMER_WITH_CLOCK,
	.dst_flags          = EV_DISPATCH,
	.dst_mask           = DISPATCH_TIMER_STRICT|DISPATCH_TIMER_BACKGROUND|
			DISPATCH_INTERVAL_UI_ANIMATION,
	.dst_timer_flags    = DISPATCH_TIMER_INTERVAL|DISPATCH_TIMER_CLOCK_UPTIME,
	.dst_action         = DISPATCH_UNOTE_ACTION_SOURCE_TIMER,
	.dst_size           = sizeof(struct dispatch_timer_source_refs_s),

	.dst_create         = _dispatch_source_timer_create,
	.dst_merge_evt      = _dispatch_source_merge_evt,
};

#pragma mark timer draining

static void
_dispatch_timers_run(dispatch_timer_heap_t dth, uint32_t tidx,
		dispatch_clock_now_cache_t nows)
{
	dispatch_timer_source_refs_t dr;
	uint64_t pending, now;

	while ((dr = dth[tidx].dth_min[DTH_TARGET_ID])) {
		DISPATCH_TIMER_ASSERT(dr->du_ident, ==, tidx, "tidx");
		DISPATCH_TIMER_ASSERT(dr->dt_timer.target, !=, 0, "missing target");

		now = _dispatch_time_now_cached(DISPATCH_TIMER_CLOCK(tidx), nows);
		if (dr->dt_timer.target > now) {
			// Done running timers for now.
			break;
		}

		if (dr->du_timer_flags & DISPATCH_TIMER_AFTER) {
			_dispatch_timer_unote_disarm(dr, dth); // +2 is consumed by _merge_evt()
			_dispatch_wlh_release(_dispatch_unote_wlh(dr));
			_dispatch_unote_state_set(dr, DU_STATE_UNREGISTERED);
			os_atomic_store2o(dr, ds_pending_data, 2, relaxed);
			_dispatch_trace_timer_fire(dr, 1, 1);
			dux_merge_evt(dr, EV_ONESHOT, 0, 0);
			continue;
		}

		if (os_atomic_load2o(dr, dt_pending_config, relaxed)) {
			_dispatch_timer_unote_configure(dr);
			continue;
		}

		// We want to try to keep repeating timers in the heap if their handler
		// is keeping up to avoid useless hops through the manager thread.
		//
		// However, if we can observe a non consumed ds_pending_data, we have to
		// remove the timer from the heap until the handler keeps up (disarm).
		// Such an operation is a one-way street, as _dispatch_source_invoke2()
		// can decide to dispose of a timer without going back to the manager if
		// it can observe that it is disarmed.
		//
		// To solve this race, we use a the MISSED marker in ds_pending_data
		// with a release barrier to make the changes accumulated on `ds_timer`
		// visible to _dispatch_source_timer_data(). Doing this also transfers
		// the responsibility to call _dispatch_timer_unote_compute_missed()
		// to _dispatch_source_invoke2() without the manager involvement.
		//
		// Suspension also causes the timer to be removed from the heap. We need
		// to make sure _dispatch_source_timer_data() will recompute the proper
		// number of fired events when the source is resumed, and also use the
		// MISSED marker for this similar purpose.
		if (unlikely(os_atomic_load2o(dr, ds_pending_data, relaxed))) {
			_dispatch_timer_unote_disarm(dr, dth);
			pending = os_atomic_or_orig2o(dr, ds_pending_data,
					DISPATCH_TIMER_DISARMED_MARKER, relaxed);
		} else {
			pending = _dispatch_timer_unote_compute_missed(dr, now, 0) << 1;
			if (_dispatch_timer_unote_needs_rearm(dr,
					DISPATCH_TIMER_UNOTE_TRACE_SUSPENSION)) {
				// _dispatch_source_merge_evt() consumes a +2 which we transfer
				// from the heap ownership when we disarm the timer. If it stays
				// armed, we need to take new retain counts
				_dispatch_retain_unote_owner(dr);
				_dispatch_timer_unote_arm(dr, dth, tidx);
				os_atomic_store2o(dr, ds_pending_data, pending, relaxed);
			} else {
				_dispatch_timer_unote_disarm(dr, dth);
				pending |= DISPATCH_TIMER_DISARMED_MARKER;
				os_atomic_store2o(dr, ds_pending_data, pending, release);
			}
		}
		_dispatch_trace_timer_fire(dr, pending >> 1, pending >> 1);
		dux_merge_evt(dr, EV_ONESHOT, 0, 0);
	}
}

#if DISPATCH_HAVE_TIMER_COALESCING
#define DISPATCH_KEVENT_COALESCING_WINDOW_INIT(qos, ms) \
		[DISPATCH_TIMER_QOS_##qos] = 2ull * (ms) * NSEC_PER_MSEC

static const uint64_t _dispatch_kevent_coalescing_window[] = {
	DISPATCH_KEVENT_COALESCING_WINDOW_INIT(NORMAL, 75),
#if DISPATCH_HAVE_TIMER_QOS
	DISPATCH_KEVENT_COALESCING_WINDOW_INIT(CRITICAL, 1),
	DISPATCH_KEVENT_COALESCING_WINDOW_INIT(BACKGROUND, 100),
#endif
};
#endif // DISPATCH_HAVE_TIMER_COALESCING

DISPATCH_ALWAYS_INLINE
static inline dispatch_timer_delay_s
_dispatch_timers_get_delay(dispatch_timer_heap_t dth, uint32_t tidx,
		uint32_t qos, dispatch_clock_now_cache_t nows)
{
	uint64_t target, deadline;
	dispatch_timer_delay_s rc;

	if (!dth[tidx].dth_min[DTH_TARGET_ID]) {
		rc.delay = rc.leeway = INT64_MAX;
		return rc;
	}

	target = dth[tidx].dth_min[DTH_TARGET_ID]->dt_timer.target;
	deadline = dth[tidx].dth_min[DTH_DEADLINE_ID]->dt_timer.deadline;
	dispatch_assert(target <= deadline && target < INT64_MAX);

	uint64_t now = _dispatch_time_now_cached(DISPATCH_TIMER_CLOCK(tidx), nows);
	if (target <= now) {
		rc.delay = rc.leeway = 0;
		return rc;
	}

	if (qos < DISPATCH_TIMER_QOS_COUNT && dth[tidx].dth_count > 2) {
#if DISPATCH_HAVE_TIMER_COALESCING
		// Timer pre-coalescing <rdar://problem/13222034>
		// When we have several timers with this target/deadline bracket:
		//
		//      Target        window  Deadline
		//        V           <-------V
		// t1:    [...........|.................]
		// t2:         [......|.......]
		// t3:             [..|..........]
		// t4:                | [.............]
		//                 ^
		//          Optimal Target
		//
		// Coalescing works better if the Target is delayed to "Optimal", by
		// picking the latest target that isn't too close to the deadline.
		uint64_t window = _dispatch_kevent_coalescing_window[qos];
		if (target + window < deadline) {
			uint64_t latest = deadline - window;
			target = _dispatch_timer_heap_max_target_before(&dth[tidx], latest);
		}
#endif
	}

	rc.delay = MIN(target - now, INT64_MAX);
	rc.leeway = MIN(deadline - target, INT64_MAX);
	return rc;
}

static void
_dispatch_timers_program(dispatch_timer_heap_t dth, uint32_t tidx,
		dispatch_clock_now_cache_t nows)
{
	uint32_t qos = DISPATCH_TIMER_QOS(tidx);
	dispatch_timer_delay_s range;

	range = _dispatch_timers_get_delay(dth, tidx, qos, nows);
	if (range.delay == 0) {
		_dispatch_timers_heap_dirty(dth, tidx);
	}
	if (range.delay == 0 || range.delay >= INT64_MAX) {
		_dispatch_trace_next_timer_set(NULL, qos);
		if (dth[tidx].dth_armed) {
			_dispatch_event_loop_timer_delete(dth, tidx);
		}
		dth[tidx].dth_armed = false;
		dth[tidx].dth_needs_program = false;
	} else {
		_dispatch_trace_next_timer_set(dth[tidx].dth_min[DTH_TARGET_ID], qos);
		_dispatch_trace_next_timer_program(range.delay, qos);
		_dispatch_event_loop_timer_arm(dth, tidx, range, nows);
		dth[tidx].dth_armed = true;
		dth[tidx].dth_needs_program = false;
	}
}

void
_dispatch_event_loop_drain_timers(dispatch_timer_heap_t dth, uint32_t count)
{
	dispatch_clock_now_cache_s nows = { };
	uint32_t tidx;

	do {
		for (tidx = 0; tidx < count; tidx++) {
			_dispatch_timers_run(dth, tidx, &nows);
		}

#if DISPATCH_USE_DTRACE
		uint32_t mask = dth[0].dth_dirty_bits & DTH_DIRTY_QOS_MASK;
		while (mask && DISPATCH_TIMER_WAKE_ENABLED()) {
			int qos = __builtin_ctz(mask);
			mask -= 1 << qos;
			_dispatch_trace_timer_wake(_dispatch_trace_next_timer[qos]);
		}
#endif // DISPATCH_USE_DTRACE

		dth[0].dth_dirty_bits = 0;

		for (tidx = 0; tidx < count; tidx++) {
			if (dth[tidx].dth_needs_program) {
				_dispatch_timers_program(dth, tidx, &nows);
			}
		}

		/*
		 * Note: dth_dirty_bits being set again can happen if we notice
		 * a new configuration during _dispatch_timers_run() that causes
		 * the timer to change clocks for a bucket we already drained.
		 *
		 * This is however extremely unlikely, and because we drain relatively
		 * to a constant cached "now", this will converge quickly.
		 */
	} while (unlikely(dth[0].dth_dirty_bits));
}
