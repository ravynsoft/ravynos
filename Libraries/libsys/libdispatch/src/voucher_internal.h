/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_VOUCHER_INTERNAL__
#define __DISPATCH_VOUCHER_INTERNAL__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

#pragma mark -
#pragma mark voucher_recipe_t (disabled)

#if VOUCHER_ENABLE_RECIPE_OBJECTS
/*!
 * @group Voucher Creation SPI
 * SPI intended for clients that need to create vouchers.
 */
OS_OBJECT_DECL_CLASS(voucher_recipe);

/*!
 * @function voucher_create
 *
 * @abstract
 * Creates a new voucher object from a recipe.
 *
 * @discussion
 * Error handling TBD
 *
 * @result
 * The newly created voucher object.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_EXPORT OS_OBJECT_RETURNS_RETAINED OS_WARN_RESULT OS_NOTHROW
voucher_t
voucher_create(voucher_recipe_t recipe);
#endif // VOUCHER_ENABLE_RECIPE_OBJECTS

#if VOUCHER_ENABLE_GET_MACH_VOUCHER
/*!
 * @function voucher_get_mach_voucher
 *
 * @abstract
 * Returns the mach voucher port underlying the specified voucher object.
 *
 * @discussion
 * The caller must either maintain a reference on the voucher object while the
 * returned mach voucher port is in use to ensure it stays valid for the
 * duration, or it must retain the mach voucher port with mach_port_mod_refs().
 *
 * @param voucher
 * The voucher object to query.
 *
 * @result
 * A mach voucher port.
 */
API_AVAILABLE(macos(10.10), ios(8.0))
OS_VOUCHER_EXPORT OS_WARN_RESULT OS_NOTHROW
mach_voucher_t
voucher_get_mach_voucher(voucher_t voucher);
#endif // VOUCHER_ENABLE_GET_MACH_VOUCHER

#pragma mark -
#pragma mark voucher_t

void _voucher_init(void);
void _voucher_atfork_child(void);
void _voucher_activity_debug_channel_init(void);
#if OS_VOUCHER_ACTIVITY_SPI && OS_VOUCHER_ACTIVITY_GENERATE_SWAPS
void _voucher_activity_swap(firehose_activity_id_t old_id,
		firehose_activity_id_t new_id);
#endif
void _voucher_xref_dispose(voucher_t voucher);
void _voucher_dispose(voucher_t voucher);
size_t _voucher_debug(voucher_t v, char* buf, size_t bufsiz);
void DISPATCH_TSD_DTOR_CC _voucher_thread_cleanup(void *voucher);
mach_voucher_t _voucher_get_mach_voucher(voucher_t voucher);
voucher_t _voucher_create_without_importance(voucher_t voucher);
voucher_t _voucher_create_accounting_voucher(voucher_t voucher);
mach_voucher_t _voucher_create_mach_voucher_with_priority(voucher_t voucher,
		pthread_priority_t priority);
voucher_t _voucher_create_with_priority_and_mach_voucher(voucher_t voucher,
		pthread_priority_t priority, mach_voucher_t kv);
void _voucher_dealloc_mach_voucher(mach_voucher_t kv);

#if VOUCHER_ENABLE_RECIPE_OBJECTS
_OS_OBJECT_DECL_SUBCLASS_INTERFACE(voucher_recipe, object)
#endif

voucher_t voucher_retain(voucher_t voucher);
void voucher_release(voucher_t voucher);

#define VOUCHER_NO_MACH_VOUCHER MACH_PORT_DEAD

#if VOUCHER_USE_MACH_VOUCHER

#if DISPATCH_DEBUG
#define DISPATCH_VOUCHER_DEBUG 1
#define DISPATCH_VOUCHER_ACTIVITY_DEBUG 1
#endif

#include <voucher/ipc_pthread_priority_types.h>

typedef uint32_t _voucher_magic_t;
typedef uint32_t _voucher_priority_t;

#define VOUCHER_MAGIC_V3  ((_voucher_magic_t)0x0390cefa) // FACE9003

typedef struct _voucher_mach_udata_s {
	_voucher_magic_t vmu_magic;
	_voucher_priority_t vmu_priority;
	uint8_t _vmu_after_priority[0];
	firehose_activity_id_t vmu_activity;
	uint64_t vmu_activity_pid;
	firehose_activity_id_t vmu_parent_activity;
	uint8_t _vmu_after_activity[0];
} _voucher_mach_udata_s;

OS_ENUM(voucher_fields, uint16_t,
	VOUCHER_FIELD_NONE		= 0,
	VOUCHER_FIELD_KVOUCHER	= 1u << 0,
	VOUCHER_FIELD_PRIORITY	= 1u << 1,
	VOUCHER_FIELD_ACTIVITY	= 1u << 2,

#if VOUCHER_ENABLE_RECIPE_OBJECTS
	VOUCHER_FIELD_EXTRA		= 1u << 15,
#else
	VOUCHER_FIELD_EXTRA		= 0,
#endif
);

typedef struct voucher_s {
	_OS_OBJECT_HEADER(
	struct voucher_vtable_s *os_obj_isa,
	os_obj_ref_cnt,
	os_obj_xref_cnt);
	struct voucher_hash_entry_s {
		uintptr_t vhe_next;
		uintptr_t vhe_prev_ptr;
	} v_list;
	mach_voucher_t v_kvoucher, v_ipc_kvoucher; // if equal, only one reference
	voucher_t v_kvbase; // if non-NULL, v_kvoucher is a borrowed reference
	firehose_activity_id_t v_activity;
	uint64_t v_activity_creator;
	firehose_activity_id_t v_parent_activity;
	_voucher_priority_t v_priority;
	unsigned int v_kv_has_importance:1;
#if VOUCHER_ENABLE_RECIPE_OBJECTS
	size_t v_recipe_extra_offset;
	mach_voucher_attr_recipe_size_t v_recipe_extra_size;
#endif
} voucher_s;

typedef struct voucher_hash_head_s {
	uintptr_t vhh_first;
} voucher_hash_head_s;

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_hash_is_enqueued(const struct voucher_s *v)
{
	return v->v_list.vhe_prev_ptr != 0;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_hash_mark_not_enqueued(struct voucher_s *v)
{
	v->v_list.vhe_prev_ptr = 0;
	v->v_list.vhe_next = (uintptr_t)DISPATCH_OBJECT_LISTLESS;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_hash_set_next(uintptr_t *next, struct voucher_s *v)
{
	*next = ~(uintptr_t)v;
}

DISPATCH_ALWAYS_INLINE
static inline voucher_t
_voucher_hash_get_next(uintptr_t next)
{
	return (voucher_t)~next;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_hash_set_prev_ptr(uintptr_t *prev_ptr, uintptr_t *addr)
{
	*prev_ptr = ~(uintptr_t)addr;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_hash_store_to_prev_ptr(uintptr_t prev_ptr, struct voucher_s *v)
{
	*(uintptr_t *)~prev_ptr = ~(uintptr_t)v;
}

#if VOUCHER_ENABLE_RECIPE_OBJECTS
#define _voucher_extra_size(v) ((v)->v_recipe_extra_size)
#define _voucher_extra_recipes(v) ((char*)(v) + (v)->v_recipe_extra_offset)
#else
#define _voucher_extra_size(v) 0
#define _voucher_extra_recipes(v) NULL
#endif

#if VOUCHER_ENABLE_RECIPE_OBJECTS
typedef struct voucher_recipe_s {
	_OS_OBJECT_HEADER(
	const _os_object_vtable_s *os_obj_isa,
	os_obj_ref_cnt,
	os_obj_xref_cnt);
	size_t vr_allocation_size;
	mach_voucher_attr_recipe_size_t volatile vr_size;
	mach_voucher_attr_recipe_t vr_data;
} voucher_recipe_s;
#endif

#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#define VL_HASH_SIZE  64u // must be a power of two
#else
#define VL_HASH_SIZE 256u // must be a power of two
#endif
#define VL_HASH(kv) (MACH_PORT_INDEX(kv) & (VL_HASH_SIZE - 1))

#if DISPATCH_DEBUG && DISPATCH_VOUCHER_DEBUG
#define _dispatch_voucher_debug(msg, v, ...) \
		_dispatch_debug("voucher[%p]: " msg, v, ##__VA_ARGS__)
#define _dispatch_kvoucher_debug(msg, kv, ...) \
		_dispatch_debug("kvoucher[0x%08x]: " msg, kv, ##__VA_ARGS__)
#define _dispatch_voucher_debug_machport(name) _dispatch_debug_machport(name)
#else
#define _dispatch_voucher_debug(msg, v, ...)
#define _dispatch_kvoucher_debug(msg, kv, ...)
#define _dispatch_voucher_debug_machport(name) ((void)(name))
#endif

#if DISPATCH_USE_DTRACE_INTROSPECTION && defined(__APPLE__) // rdar://33642820
#define _voucher_trace(how, ...)  ({ \
		if (unlikely(VOUCHER_##how##_ENABLED())) { \
			VOUCHER_##how(__VA_ARGS__); \
		} \
	})
#else
#define _voucher_trace(how, ...) ((void)0)
#endif

#ifndef DISPATCH_VOUCHER_OBJC_DEBUG
#if DISPATCH_INTROSPECTION || DISPATCH_DEBUG
#define DISPATCH_VOUCHER_OBJC_DEBUG 1
#else
#define DISPATCH_VOUCHER_OBJC_DEBUG 0
#endif
#endif // DISPATCH_VOUCHER_OBJC_DEBUG

DISPATCH_ALWAYS_INLINE
static inline struct voucher_s *
_voucher_retain_inline(struct voucher_s *voucher)
{
	// not using _os_object_refcnt* because we don't need barriers:
	// vouchers are immutable and are in a hash table with a lock
	int xref_cnt = os_atomic_inc2o(voucher, os_obj_xref_cnt, relaxed);
	_voucher_trace(RETAIN, (voucher_t)voucher, xref_cnt + 1);
	_dispatch_voucher_debug("retain  -> %d", voucher, xref_cnt + 1);
	if (unlikely(xref_cnt <= 0)) {
		_OS_OBJECT_CLIENT_CRASH("Voucher resurrection");
	}
	return voucher;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_release_inline(struct voucher_s *voucher)
{
	// not using _os_object_refcnt* because we don't need barriers:
	// vouchers are immutable and are in a hash table with a lock
	int xref_cnt = os_atomic_dec2o(voucher, os_obj_xref_cnt, relaxed);
	_voucher_trace(RELEASE, (voucher_t)voucher, xref_cnt + 1);
	_dispatch_voucher_debug("release -> %d", voucher, xref_cnt + 1);
	if (likely(xref_cnt >= 0)) {
		return;
	}
	if (unlikely(xref_cnt < -1)) {
		_OS_OBJECT_CLIENT_CRASH("Voucher over-release");
	}
	return _voucher_xref_dispose((voucher_t)voucher);
}

#if DISPATCH_PURE_C

DISPATCH_ALWAYS_INLINE
static inline voucher_t
_voucher_retain(voucher_t voucher)
{
#if DISPATCH_VOUCHER_OBJC_DEBUG
	os_retain(voucher);
#else
	_voucher_retain_inline(voucher);
#endif // DISPATCH_VOUCHER_OBJC_DEBUG
	return voucher;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_release(voucher_t voucher)
{
#if DISPATCH_VOUCHER_OBJC_DEBUG
	os_release(voucher);
#else
	_voucher_release_inline(voucher);
#endif // DISPATCH_VOUCHER_OBJC_DEBUG
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_release_no_dispose(voucher_t voucher)
{
#if !DISPATCH_VOUCHER_OBJC_DEBUG
	// not using _os_object_refcnt* because we don't need barriers:
	// vouchers are immutable and are in a hash table with a lock
	int xref_cnt = os_atomic_dec2o(voucher, os_obj_xref_cnt, relaxed);
	_voucher_trace(RELEASE, voucher, xref_cnt + 1);
	_dispatch_voucher_debug("release -> %d", voucher, xref_cnt + 1);
	if (likely(xref_cnt >= 0)) {
		return;
	}
	_OS_OBJECT_CLIENT_CRASH("Voucher over-release");
#else
	return os_release(voucher);
#endif // DISPATCH_DEBUG
}

DISPATCH_ALWAYS_INLINE
static inline voucher_t
_voucher_get(void)
{
	return _dispatch_thread_getspecific(dispatch_voucher_key);
}

DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline voucher_t
_voucher_copy(void)
{
	voucher_t voucher = _voucher_get();
	if (voucher) _voucher_retain(voucher);
	return voucher;
}

DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline voucher_t
_voucher_copy_without_importance(void)
{
	voucher_t voucher = _voucher_get();
	if (voucher) voucher = _voucher_create_without_importance(voucher);
	return voucher;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_mach_voucher_set(mach_voucher_t kv)
{
	if (kv == VOUCHER_NO_MACH_VOUCHER) return;
	_dispatch_set_priority_and_mach_voucher_slow(0, kv);
}

DISPATCH_ALWAYS_INLINE
static inline mach_voucher_t
_voucher_swap_and_get_mach_voucher(voucher_t ov, voucher_t voucher)
{
	if (ov == voucher) return VOUCHER_NO_MACH_VOUCHER;
	if (ov) _voucher_trace(ORPHAN, ov);
	_dispatch_thread_setspecific(dispatch_voucher_key, voucher);
	if (voucher) _voucher_trace(ADOPT, voucher);
	_dispatch_voucher_debug("swap from voucher[%p]", voucher, ov);
	mach_voucher_t kv = voucher ? voucher->v_kvoucher : MACH_VOUCHER_NULL;
	mach_voucher_t okv = ov ? ov->v_kvoucher : MACH_VOUCHER_NULL;
#if OS_VOUCHER_ACTIVITY_GENERATE_SWAPS
	firehose_activity_id_t aid = voucher ? voucher->v_activity : 0;
	firehose_activity_id_t oaid = ov ? ov->v_activity : 0;
	if (aid != oaid) _voucher_activity_swap(aid, oaid);
#endif
	return (kv != okv) ? kv : VOUCHER_NO_MACH_VOUCHER;
}

DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline voucher_t
_voucher_adopt(voucher_t voucher)
{
	voucher_t ov = _voucher_get();
	_voucher_mach_voucher_set(_voucher_swap_and_get_mach_voucher(ov, voucher));
	return ov;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_replace(voucher_t voucher)
{
	voucher_t ov = _voucher_adopt(voucher);
	if (ov) _voucher_release(ov);
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_clear(void)
{
	_voucher_replace(NULL);
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_voucher_get_priority(voucher_t v)
{
	return v ? (pthread_priority_t)v->v_priority : 0;
}

DISPATCH_ALWAYS_INLINE
static inline firehose_activity_id_t
_voucher_get_activity_id(voucher_t v, uint64_t *creator_pid)
{
	if (creator_pid) *creator_pid = v ? v->v_activity_creator : 0;
	return v ? v->v_activity : 0;
}

void _voucher_task_mach_voucher_init(void* ctxt);
extern dispatch_once_t _voucher_task_mach_voucher_pred;
extern mach_voucher_t _voucher_task_mach_voucher;
#if VOUCHER_USE_EMPTY_MACH_BASE_VOUCHER
#define _voucher_default_task_mach_voucher MACH_VOUCHER_NULL
#else
extern mach_voucher_t _voucher_default_task_mach_voucher;
#endif
DISPATCH_ALWAYS_INLINE
static inline mach_voucher_t
_voucher_get_task_mach_voucher(void)
{
	dispatch_once_f(&_voucher_task_mach_voucher_pred, NULL,
			_voucher_task_mach_voucher_init);
	return _voucher_task_mach_voucher;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_mach_msg_set_mach_voucher(mach_msg_header_t *msg, mach_voucher_t kv,
		bool move_send)
{
	if (MACH_MSGH_BITS_HAS_VOUCHER(msg->msgh_bits)) return false;
	if (!kv) return false;
	msg->msgh_voucher_port = kv;
	msg->msgh_bits |= MACH_MSGH_BITS_SET_PORTS(0, 0, move_send ?
			MACH_MSG_TYPE_MOVE_SEND : MACH_MSG_TYPE_COPY_SEND);
	_dispatch_kvoucher_debug("msg[%p] set %s", kv, msg, move_send ?
			"move-send" : "copy-send");
	_dispatch_voucher_debug_machport(kv);
	return true;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_mach_msg_set(mach_msg_header_t *msg, voucher_t voucher)
{
	if (MACH_MSGH_BITS_HAS_VOUCHER(msg->msgh_bits)) return false;
	mach_voucher_t kv;
	if (voucher) {
		kv = _voucher_get_mach_voucher(voucher);
	} else {
		kv = _voucher_get_task_mach_voucher();
	}
	return _voucher_mach_msg_set_mach_voucher(msg, kv, false);
}

DISPATCH_ALWAYS_INLINE
static inline mach_voucher_t
_voucher_mach_msg_get(mach_msg_header_t *msg, mach_msg_bits_t *msgh_bits)
{
	if (!MACH_MSGH_BITS_HAS_VOUCHER(msg->msgh_bits)) {
		*msgh_bits = 0;
		return MACH_VOUCHER_NULL;
	}
	mach_voucher_t kv = msg->msgh_voucher_port;
	msg->msgh_voucher_port = MACH_VOUCHER_NULL;
	mach_msg_bits_t mask = MACH_MSGH_BITS_VOUCHER_MASK|MACH_MSGH_BITS_RAISEIMP;
	*msgh_bits = msg->msgh_bits & mask;
	msg->msgh_bits &= ~mask;
	return kv;
}

DISPATCH_ALWAYS_INLINE
static inline mach_voucher_t
_voucher_mach_msg_clear(mach_msg_header_t *msg, bool move_send)
{
	mach_msg_bits_t kvbits = MACH_MSGH_BITS_VOUCHER(msg->msgh_bits);
	mach_voucher_t kv = msg->msgh_voucher_port, kvm = MACH_VOUCHER_NULL;
	if ((kvbits == MACH_MSG_TYPE_COPY_SEND ||
			kvbits == MACH_MSG_TYPE_MOVE_SEND) && kv) {
		_dispatch_kvoucher_debug("msg[%p] clear %s", kv, msg, move_send ?
				"move-send" : "copy-send");
		_dispatch_voucher_debug_machport(kv);
		if (kvbits == MACH_MSG_TYPE_MOVE_SEND) {
			// <rdar://problem/15694142> return/drop received or pseudo-received
			// voucher reference (e.g. due to send failure).
			if (move_send) {
				kvm = kv;
			} else {
				_voucher_dealloc_mach_voucher(kv);
			}
		}
		msg->msgh_voucher_port = MACH_VOUCHER_NULL;
		msg->msgh_bits &= (mach_msg_bits_t)~MACH_MSGH_BITS_VOUCHER_MASK;
	}
	return kvm;
}

#pragma mark -
#pragma mark dispatch_continuation_t + voucher_t

#if DISPATCH_USE_VOUCHER_KDEBUG_TRACE
#define DISPATCH_VOUCHER_CODE(code) DISPATCH_CODE(VOUCHER, code)
#else
#define DISPATCH_VOUCHER_CODE(code) 0
#endif // DISPATCH_USE_VOUCHER_KDEBUG_TRACE

#define DISPATCH_TRACE_VOUCHER_DC_PUSH          DISPATCH_VOUCHER_CODE(0x1)
#define DISPATCH_TRACE_VOUCHER_DC_POP           DISPATCH_VOUCHER_CODE(0x2)
#define DISPATCH_TRACE_VOUCHER_DMSG_PUSH        DISPATCH_VOUCHER_CODE(0x3)
#define DISPATCH_TRACE_VOUCHER_DMSG_POP         DISPATCH_VOUCHER_CODE(0x4)
#define DISPATCH_TRACE_VOUCHER_ACTIVITY_ADOPT   DISPATCH_VOUCHER_CODE(0x5)

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_voucher_ktrace(uint32_t code, voucher_t v, const void *container)
{
	if (v == DISPATCH_NO_VOUCHER) return;
	natural_t voucher = v ? v->v_kvoucher : MACH_VOUCHER_NULL;
	_dispatch_ktrace2(code, voucher, (uintptr_t)container);
}
#define _dispatch_voucher_ktrace(code, v, container) \
		_dispatch_voucher_ktrace(DISPATCH_TRACE_VOUCHER_##code, v, container)
#define _dispatch_voucher_ktrace_dc_push(dc) \
		_dispatch_voucher_ktrace(DC_PUSH, (dc)->dc_voucher, (dc))
#define _dispatch_voucher_ktrace_dc_pop(dc, v) \
		_dispatch_voucher_ktrace(DC_POP, v, (dc))
#define _dispatch_voucher_ktrace_dmsg_push(dmsg) \
		_dispatch_voucher_ktrace(DMSG_PUSH, (dmsg)->dmsg_voucher, (dmsg))
#define _dispatch_voucher_ktrace_dmsg_pop(dmsg) \
		_dispatch_voucher_ktrace(DMSG_POP, (dmsg)->dmsg_voucher, (dmsg))
#define _dispatch_voucher_ktrace_activity_adopt(aid) \
		_dispatch_ktrace1(DISPATCH_TRACE_VOUCHER_ACTIVITY_ADOPT, aid);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_voucher_set(dispatch_continuation_t dc,
		dispatch_block_flags_t flags)
{
	voucher_t v = NULL;

	// _dispatch_continuation_voucher_set is never called for blocks with
	// private data or with the DISPATCH_BLOCK_HAS_VOUCHER flag set.
	// only _dispatch_continuation_init_slow handles this bit.
	dispatch_assert(!(flags & DISPATCH_BLOCK_HAS_VOUCHER));

	if (!(flags & DISPATCH_BLOCK_NO_VOUCHER)) {
		v = _voucher_copy();
	}
	dc->dc_voucher = v;
	_dispatch_voucher_debug("continuation[%p] set", dc->dc_voucher, dc);
	_dispatch_voucher_ktrace_dc_push(dc);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_voucher_adopt(dispatch_continuation_t dc,
		uintptr_t dc_flags)
{
	voucher_t v = dc->dc_voucher;
	dispatch_thread_set_self_t consume = (dc_flags & DC_FLAG_CONSUME);
	dispatch_assert(DC_FLAG_CONSUME == DISPATCH_VOUCHER_CONSUME);

	if (consume) {
		dc->dc_voucher = VOUCHER_INVALID;
	}
	if (likely(v != DISPATCH_NO_VOUCHER)) {
		_dispatch_voucher_ktrace_dc_pop(dc, v);
		_dispatch_voucher_debug("continuation[%p] adopt", v, dc);
	}
	(void)_dispatch_adopt_priority_and_set_voucher(dc->dc_priority, v,
			consume | DISPATCH_VOUCHER_REPLACE);
}

#pragma mark -
#pragma mark _voucher activity subsystem

extern dispatch_once_t _firehose_task_buffer_pred;
extern union firehose_buffer_u *_firehose_task_buffer;
extern uint64_t _voucher_unique_pid;
extern dispatch_mach_t _voucher_activity_debug_channel;
extern voucher_activity_hooks_t _voucher_libtrace_hooks;

#endif // DISPATCH_PURE_C

#else // VOUCHER_USE_MACH_VOUCHER

#pragma mark -
#pragma mark Simulator / vouchers disabled

#define _dispatch_voucher_debug(msg, v, ...)
#define _dispatch_kvoucher_debug(msg, kv, ...)

DISPATCH_ALWAYS_INLINE
static inline voucher_t
_voucher_retain(voucher_t voucher)
{
	return voucher;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_release(voucher_t voucher)
{
	(void)voucher;
}

DISPATCH_ALWAYS_INLINE
static inline voucher_t
_voucher_get(void)
{
	return NULL;
}

DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline voucher_t
_voucher_copy(void)
{
	return NULL;
}

DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline voucher_t
_voucher_copy_without_importance(void)
{
	return NULL;
}

DISPATCH_ALWAYS_INLINE
static inline mach_voucher_t
_voucher_swap_and_get_mach_voucher(voucher_t ov, voucher_t voucher)
{
	(void)ov; (void)voucher;
	return MACH_VOUCHER_NULL;
}

DISPATCH_ALWAYS_INLINE
static inline voucher_t
_voucher_adopt(voucher_t voucher)
{
	return voucher;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_replace(voucher_t voucher)
{
	(void)voucher;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_clear(void)
{
}

DISPATCH_ALWAYS_INLINE
static inline pthread_priority_t
_voucher_get_priority(voucher_t voucher)
{
	(void)voucher;
	return 0;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_mach_msg_set_mach_voucher(mach_msg_header_t *msg, mach_voucher_t kv,
		bool move_send)
{
	(void)msg; (void)kv; (void)move_send;
	return false;

}

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_mach_msg_set(mach_msg_header_t *msg, voucher_t voucher)
{
	(void)msg; (void)voucher;
	return false;
}

DISPATCH_ALWAYS_INLINE
static inline mach_voucher_t
_voucher_mach_msg_get(mach_msg_header_t *msg, mach_msg_bits_t *msgh_bits)
{
	(void)msg;(void)msgh_bits;
	return 0;
}

DISPATCH_ALWAYS_INLINE
static inline mach_voucher_t
_voucher_mach_msg_clear(mach_msg_header_t *msg, bool move_send)
{
	(void)msg; (void)move_send;
	return MACH_VOUCHER_NULL;
}

#define _dispatch_voucher_ktrace_dc_push(dc)
#define _dispatch_voucher_ktrace_dc_pop(dc, v)
#define _dispatch_voucher_ktrace_dmsg_push(dmsg)
#define _dispatch_voucher_ktrace_dmsg_pop(dmsg)

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_voucher_set(dispatch_continuation_t dc,
		dispatch_block_flags_t flags)
{
	(void)dc; (void)flags;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_voucher_adopt(dispatch_continuation_t dc,
		uintptr_t dc_flags)
{
	(void)dc; (void)dc_flags;
}

#endif // VOUCHER_USE_MACH_VOUCHER

#endif /* __DISPATCH_VOUCHER_INTERNAL__ */
