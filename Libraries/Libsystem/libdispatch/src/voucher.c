/*
 * Copyright (c) 2013-2016 Apple Inc. All rights reserved.
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

#ifndef PERSONA_ID_NONE
#define PERSONA_ID_NONE ((uid_t)-1)
#endif

#if !DISPATCH_VARIANT_DYLD_STUB

#if VOUCHER_USE_MACH_VOUCHER
#if !HAVE_PTHREAD_WORKQUEUE_QOS
#error Unsupported configuration, workqueue QoS support is required
#endif
#include <mach/mach_voucher.h>
#include <sys/proc_info.h>

#define MACH_ACTIVITY_ID_RANGE_SIZE 16
#define MACH_ACTIVITY_ID_MASK ((1ULL << FIREHOSE_ACTIVITY_ID_FLAGS_SHIFT) - 1)
#define FIREHOSE_ACTIVITY_ID_MAKE(aid, flags) \
		FIREHOSE_ACTIVITY_ID_MERGE_FLAGS((aid) & MACH_ACTIVITY_ID_MASK, flags)

DISPATCH_STATIC_GLOBAL(volatile uint64_t _voucher_aid_next);

#pragma mark -
#pragma mark voucher_t

OS_OBJECT_CLASS_DECL(voucher);
#if !USE_OBJC
OS_OBJECT_VTABLE_INSTANCE(voucher,
		(void (*)(_os_object_t))_voucher_xref_dispose,
		(void (*)(_os_object_t))_voucher_dispose);
#endif // USE_OBJC
#define VOUCHER_CLASS OS_OBJECT_VTABLE(voucher)

static inline voucher_t
_voucher_alloc(mach_voucher_attr_recipe_size_t extra)
{
	voucher_t voucher;
	size_t voucher_size = sizeof(voucher_s) + extra;
	voucher = (voucher_t)_os_object_alloc_realized(VOUCHER_CLASS, voucher_size);
#if VOUCHER_ENABLE_RECIPE_OBJECTS
	voucher->v_recipe_extra_size = extra;
	voucher->v_recipe_extra_offset = voucher_size - extra;
#else
	dispatch_assert(!extra);
#endif
	_dispatch_voucher_debug("alloc", voucher);
	return voucher;
}

#if VOUCHER_ENABLE_RECIPE_OBJECTS
voucher_t
voucher_create(voucher_recipe_t recipe)
{
	// TODO: capture current activities or current kvoucher ?
	mach_voucher_attr_recipe_size_t extra = recipe ? recipe->vr_size : 0;
	voucher_t voucher = _voucher_alloc(extra);
	if (extra) {
		memcpy(_voucher_extra_recipes(voucher), recipe->vr_data, extra);
	}
	_voucher_trace(CREATE, voucher, MACH_PORT_NULL, 0);
	return voucher;
}
#endif

DISPATCH_ALWAYS_INLINE
static inline voucher_t
_voucher_clone(const voucher_t ov, voucher_fields_t ignore_fields)
{
	mach_voucher_attr_recipe_size_t extra = 0;
	voucher_t v;

	if (ov && !(ignore_fields & VOUCHER_FIELD_EXTRA)) {
		extra = _voucher_extra_size(ov);
	}
	v = _voucher_alloc(extra);
	if (ov) {
		voucher_fields_t fields = ~ignore_fields;
		if ((fields & VOUCHER_FIELD_KVOUCHER) && ov->v_kvoucher) {
			voucher_t kvb = ov->v_kvbase ? ov->v_kvbase : ov;
			v->v_kvbase = _voucher_retain(kvb);
			v->v_kvoucher = kvb->v_kvoucher;
			v->v_kv_has_importance = kvb->v_kv_has_importance;
		}
		if (fields & VOUCHER_FIELD_PRIORITY) {
			v->v_priority = ov->v_priority;
		}
		if (fields & VOUCHER_FIELD_ACTIVITY) {
			v->v_activity = ov->v_activity;
			v->v_activity_creator = ov->v_activity_creator;
			v->v_parent_activity = ov->v_parent_activity;
		}
		if ((fields & VOUCHER_FIELD_EXTRA) && extra) {
			memcpy(_voucher_extra_recipes(v), _voucher_extra_recipes(ov),extra);
		}
	}
	return v;
}

voucher_t
voucher_adopt(voucher_t voucher)
{
	if (voucher == VOUCHER_CURRENT) {
		return _voucher_copy();
	}
	return _voucher_adopt(voucher);
}

voucher_t
voucher_copy(void)
{
	return _voucher_copy();
}

voucher_t
voucher_copy_without_importance(void)
{
	return _voucher_copy_without_importance();
}

voucher_t
voucher_retain(voucher_t voucher)
{
	return _voucher_retain(voucher);
}

void
voucher_release(voucher_t voucher)
{
	return _voucher_release(voucher);
}

void DISPATCH_TSD_DTOR_CC
_voucher_thread_cleanup(void *voucher)
{
	// when a thread exits and has a voucher left, the kernel
	// will get rid of the voucher kernel object that is set on the thread,
	// we only need to release the voucher_t object.
	_voucher_release(voucher);
}

#pragma mark -
#pragma mark voucher_hash

extern voucher_hash_head_s _voucher_hash[VL_HASH_SIZE];
DISPATCH_GLOBAL_INIT(voucher_hash_head_s _voucher_hash[VL_HASH_SIZE], {
	[0 ... VL_HASH_SIZE - 1] = { ~(uintptr_t)VOUCHER_NULL },
});
DISPATCH_STATIC_GLOBAL(dispatch_unfair_lock_s _voucher_hash_lock);

#define _voucher_hash_head(kv)   (&_voucher_hash[VL_HASH((kv))])
#define _voucher_hash_lock_lock() \
		_dispatch_unfair_lock_lock(&_voucher_hash_lock)
#define _voucher_hash_lock_unlock() \
		_dispatch_unfair_lock_unlock(&_voucher_hash_lock)

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_hash_enqueue(mach_voucher_t kv, voucher_t v)
{
	// same as LIST_INSERT_HEAD
	voucher_hash_head_s *head = _voucher_hash_head(kv);
	voucher_t next = _voucher_hash_get_next(head->vhh_first);
	v->v_list.vhe_next = head->vhh_first;
	if (next) {
		_voucher_hash_set_prev_ptr(&next->v_list.vhe_prev_ptr,
				&v->v_list.vhe_next);
	}
	_voucher_hash_set_next(&head->vhh_first, v);
	_voucher_hash_set_prev_ptr(&v->v_list.vhe_prev_ptr, &head->vhh_first);
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_hash_remove(voucher_t v)
{
	// same as LIST_REMOVE
	voucher_t next = _voucher_hash_get_next(v->v_list.vhe_next);
	uintptr_t prev_ptr = v->v_list.vhe_prev_ptr;
	if (next) {
		next->v_list.vhe_prev_ptr = prev_ptr;
	}
	_voucher_hash_store_to_prev_ptr(prev_ptr, next);
	_voucher_hash_mark_not_enqueued(v);
}

static voucher_t
_voucher_find_and_retain(mach_voucher_t kv)
{
	if (!kv) return NULL;
	_voucher_hash_lock_lock();
	voucher_hash_head_s *head = _voucher_hash_head(kv);
	voucher_t v = _voucher_hash_get_next(head->vhh_first);
	while (v) {
		if (v->v_ipc_kvoucher == kv) {
			int xref_cnt = os_atomic_inc2o(v, os_obj_xref_cnt, relaxed);
			_dispatch_voucher_debug("retain  -> %d", v, xref_cnt + 1);
			if (unlikely(xref_cnt < 0)) {
				_dispatch_voucher_debug("over-release", v);
				_OS_OBJECT_CLIENT_CRASH("Voucher over-release");
			}
			if (xref_cnt == 0) {
				// resurrection: raced with _voucher_remove
				(void)os_atomic_inc2o(v, os_obj_ref_cnt, relaxed);
			}
			break;
		}
		v = _voucher_hash_get_next(v->v_list.vhe_next);
	}
	_voucher_hash_lock_unlock();
	return v;
}

static void
_voucher_insert(voucher_t v)
{
	mach_voucher_t kv = v->v_ipc_kvoucher;
	if (!kv) return;
	_voucher_hash_lock_lock();
	if (unlikely(_voucher_hash_is_enqueued(v))) {
		_dispatch_voucher_debug("corruption", v);
		DISPATCH_CLIENT_CRASH(0, "Voucher corruption");
	}
	_voucher_hash_enqueue(kv, v);
	_voucher_hash_lock_unlock();
}

static void
_voucher_remove(voucher_t v)
{
	mach_voucher_t kv = v->v_ipc_kvoucher;
	if (!_voucher_hash_is_enqueued(v)) return;
	_voucher_hash_lock_lock();
	if (unlikely(!kv)) {
		_dispatch_voucher_debug("corruption", v);
		DISPATCH_CLIENT_CRASH(0, "Voucher corruption");
	}
	// check for resurrection race with _voucher_find_and_retain
	if (os_atomic_load2o(v, os_obj_xref_cnt, ordered) < 0) {
		if (_voucher_hash_is_enqueued(v)) _voucher_hash_remove(v);
	}
	_voucher_hash_lock_unlock();
}

#pragma mark -
#pragma mark mach_voucher_t

void
_voucher_dealloc_mach_voucher(mach_voucher_t kv)
{
	_dispatch_kvoucher_debug("dealloc", kv);
	_dispatch_voucher_debug_machport(kv);
	kern_return_t kr = mach_voucher_deallocate(kv);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
}

static inline kern_return_t
_voucher_create_mach_voucher(const mach_voucher_attr_recipe_data_t *recipes,
		size_t recipes_size, mach_voucher_t *kvp)
{
	kern_return_t kr;
	mach_port_t mhp = _dispatch_get_mach_host_port();
	mach_voucher_t kv = MACH_VOUCHER_NULL;
	mach_voucher_attr_raw_recipe_array_t kvr;
	mach_voucher_attr_recipe_size_t kvr_size;
	kvr = (mach_voucher_attr_raw_recipe_array_t)recipes;
	kvr_size = (mach_voucher_attr_recipe_size_t)recipes_size;
	kr = host_create_mach_voucher(mhp, kvr, kvr_size, &kv);
	DISPATCH_VERIFY_MIG(kr);
	if (!kr) {
		_dispatch_kvoucher_debug("create", kv);
		_dispatch_voucher_debug_machport(kv);
	}
	*kvp = kv;
	return kr;
}

void
_voucher_task_mach_voucher_init(void* ctxt DISPATCH_UNUSED)
{
	kern_return_t kr;
	mach_voucher_t kv = MACH_VOUCHER_NULL;
#if !VOUCHER_USE_EMPTY_MACH_BASE_VOUCHER
	static const mach_voucher_attr_recipe_data_t task_create_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_BANK,
		.command = MACH_VOUCHER_ATTR_BANK_CREATE,
	};
	kr = _voucher_create_mach_voucher(&task_create_recipe,
			sizeof(task_create_recipe), &kv);
	if (unlikely(kr)) {
		DISPATCH_CLIENT_CRASH(kr, "Could not create task mach voucher");
	}
	_voucher_default_task_mach_voucher = kv;
#endif
	_voucher_task_mach_voucher = kv;
}

void
voucher_replace_default_voucher(void)
{
	(void)_voucher_get_task_mach_voucher(); // initalize task mach voucher
	mach_voucher_t kv, tkv = MACH_VOUCHER_NULL;
	voucher_t v = _voucher_get();
	if (v && v->v_kvoucher) {
		kern_return_t kr;
		kv = v->v_ipc_kvoucher ? v->v_ipc_kvoucher : v->v_kvoucher;
		const mach_voucher_attr_recipe_data_t task_copy_recipe = {
			.key = MACH_VOUCHER_ATTR_KEY_BANK,
			.command = MACH_VOUCHER_ATTR_COPY,
			.previous_voucher = kv,
		};
		kr = _voucher_create_mach_voucher(&task_copy_recipe,
				sizeof(task_copy_recipe), &tkv);
		if (dispatch_assume_zero(kr)) {
			tkv = MACH_VOUCHER_NULL;
		}
	}
	if (!tkv) tkv = _voucher_default_task_mach_voucher;
	kv = os_atomic_xchg(&_voucher_task_mach_voucher, tkv, relaxed);
	if (kv && kv != _voucher_default_task_mach_voucher) {
		_voucher_dealloc_mach_voucher(kv);
	}
	_dispatch_voucher_debug("kvoucher[0x%08x] replace default voucher", v, tkv);
}

#define _voucher_mach_recipe_size(payload_size) \
	(sizeof(mach_voucher_attr_recipe_data_t) + (payload_size))

#define _voucher_mach_recipe_alloca(v) ((mach_voucher_attr_recipe_t)alloca(\
		_voucher_mach_recipe_size(0) + \
		_voucher_mach_recipe_size(sizeof(ipc_pthread_priority_value_t)) + \
		_voucher_mach_recipe_size(sizeof(_voucher_mach_udata_s)) + \
		_voucher_extra_size(v)))

DISPATCH_ALWAYS_INLINE
static inline mach_voucher_attr_recipe_size_t
_voucher_mach_recipe_init(mach_voucher_attr_recipe_t mvar_buf, voucher_s *v,
		mach_voucher_t kvb, pthread_priority_t pp)
{
	mach_voucher_attr_recipe_size_t extra = _voucher_extra_size(v);
	mach_voucher_attr_recipe_size_t size = 0;

	// normalize to just the QoS class and 0 relative priority
	pp &= _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	if (pp) pp |= _PTHREAD_PRIORITY_PRIORITY_MASK;

	*mvar_buf++ = (mach_voucher_attr_recipe_data_t){
		.key = MACH_VOUCHER_ATTR_KEY_ALL,
		.command = MACH_VOUCHER_ATTR_COPY,
		.previous_voucher = kvb,
	};
	size += _voucher_mach_recipe_size(0);

	if (pp) {
		ipc_pthread_priority_value_t value = (ipc_pthread_priority_value_t)pp;
		*mvar_buf++ = (mach_voucher_attr_recipe_data_t){
			.key = MACH_VOUCHER_ATTR_KEY_PTHPRIORITY,
			.command = MACH_VOUCHER_ATTR_PTHPRIORITY_CREATE,
			.content_size = sizeof(value),
		};
		mvar_buf = _dispatch_memappend(mvar_buf, &value);
		size += _voucher_mach_recipe_size(sizeof(value));
	}

	if ((v && v->v_activity) || pp) {
		_voucher_mach_udata_s *udata_buf;
		unsigned udata_size = 0;

		if (v && v->v_activity) {
			udata_size = offsetof(_voucher_mach_udata_s, _vmu_after_activity);
		} else {
			udata_size = offsetof(_voucher_mach_udata_s, _vmu_after_priority);
		}
		*mvar_buf = (mach_voucher_attr_recipe_data_t){
			.key = MACH_VOUCHER_ATTR_KEY_USER_DATA,
			.command = MACH_VOUCHER_ATTR_USER_DATA_STORE,
			.content_size = udata_size,
		};
		udata_buf = (_voucher_mach_udata_s *)(mvar_buf->content);

		if (v && v->v_activity) {
			*udata_buf = (_voucher_mach_udata_s){
				.vmu_magic = VOUCHER_MAGIC_V3,
				.vmu_priority = (_voucher_priority_t)pp,
				.vmu_activity = v->v_activity,
				.vmu_activity_pid = v->v_activity_creator,
				.vmu_parent_activity = v->v_parent_activity,
			};
		} else {
			*udata_buf = (_voucher_mach_udata_s){
				.vmu_magic = VOUCHER_MAGIC_V3,
				.vmu_priority = (_voucher_priority_t)pp,
			};
		}

		mvar_buf = (mach_voucher_attr_recipe_t)(mvar_buf->content + udata_size);
		size += _voucher_mach_recipe_size(udata_size);
	}

	if (extra) {
		memcpy(mvar_buf, _voucher_extra_recipes(v), extra);
		size += extra;
	}
	return size;
}

mach_voucher_t
_voucher_get_mach_voucher(voucher_t voucher)
{
	if (!voucher) return MACH_VOUCHER_NULL;
	if (voucher->v_ipc_kvoucher) return voucher->v_ipc_kvoucher;
	mach_voucher_t kvb = voucher->v_kvoucher;
	if (!kvb) kvb = _voucher_get_task_mach_voucher();
	if (!voucher->v_activity && !voucher->v_priority &&
			!_voucher_extra_size(voucher)) {
		return kvb;
	}

	mach_voucher_attr_recipe_t mvar = _voucher_mach_recipe_alloca(voucher);
	mach_voucher_attr_recipe_size_t size;
	mach_voucher_t kv, kvo;
	kern_return_t kr;

	size = _voucher_mach_recipe_init(mvar, voucher, kvb, voucher->v_priority);
	kr = _voucher_create_mach_voucher(mvar, size, &kv);
	if (dispatch_assume_zero(kr) || !kv) {
		return MACH_VOUCHER_NULL;
	}
	if (!os_atomic_cmpxchgv2o(voucher, v_ipc_kvoucher, MACH_VOUCHER_NULL,
			kv, &kvo, relaxed)) {
		_voucher_dealloc_mach_voucher(kv);
		kv = kvo;
	} else {
		if (kv == voucher->v_kvoucher) {
			// if v_kvoucher == v_ipc_kvoucher we keep only one reference
			_voucher_dealloc_mach_voucher(kv);
		}
		_voucher_insert(voucher);
		_dispatch_voucher_debug("kvoucher[0x%08x] create", voucher, kv);
	}
	return kv;
}

mach_voucher_t
_voucher_create_mach_voucher_with_priority(voucher_t voucher,
		pthread_priority_t priority)
{
	if (priority == _voucher_get_priority(voucher)) {
		return MACH_VOUCHER_NULL; // caller will use _voucher_get_mach_voucher
	}
	kern_return_t kr;
	mach_voucher_t kv, kvb = voucher ? voucher->v_kvoucher : MACH_VOUCHER_NULL;
	if (!kvb) kvb = _voucher_get_task_mach_voucher();

	mach_voucher_attr_recipe_t mvar = _voucher_mach_recipe_alloca(voucher);
	mach_voucher_attr_recipe_size_t size;

	size = _voucher_mach_recipe_init(mvar, voucher, kvb, priority);
	kr = _voucher_create_mach_voucher(mvar, size, &kv);
	if (dispatch_assume_zero(kr) || !kv) {
		return MACH_VOUCHER_NULL;
	}
	_dispatch_kvoucher_debug("create with priority from voucher[%p]", kv,
			voucher);
	return kv;
}

static voucher_t
_voucher_create_with_mach_voucher(mach_voucher_t kv, mach_msg_bits_t msgh_bits)
{
	if (!kv) return NULL;
	kern_return_t kr;
	mach_voucher_attr_recipe_t vr;
	size_t vr_size;
	mach_voucher_attr_recipe_size_t kvr_size = 0;
	mach_voucher_attr_content_size_t udata_sz = 0;
	_voucher_mach_udata_s *udata = NULL;
	voucher_t v = _voucher_find_and_retain(kv);
	if (v) {
		_dispatch_voucher_debug("kvoucher[0x%08x] found", v, kv);
		_voucher_dealloc_mach_voucher(kv);
		return v;
	}
	vr_size = sizeof(*vr) + sizeof(_voucher_mach_udata_s);
	vr = alloca(vr_size);
	if (kv) {
		kvr_size = (mach_voucher_attr_recipe_size_t)vr_size;
		kr = mach_voucher_extract_attr_recipe(kv,
				MACH_VOUCHER_ATTR_KEY_USER_DATA, (void*)vr, &kvr_size);
		DISPATCH_VERIFY_MIG(kr);
		if (!dispatch_assume_zero(kr) && kvr_size >= sizeof(*vr)) {
			udata_sz = vr->content_size;
			udata = (_voucher_mach_udata_s*)vr->content;
			dispatch_assume(udata_sz >= sizeof(_voucher_magic_t));
		}
	}
	vr = NULL;

	v = _voucher_alloc(0);
	v->v_ipc_kvoucher = v->v_kvoucher = kv;
	v->v_kv_has_importance = !!(msgh_bits & MACH_MSGH_BITS_RAISEIMP);

	if (udata_sz >= offsetof(_voucher_mach_udata_s,_vmu_after_priority)){
		if (udata->vmu_magic == VOUCHER_MAGIC_V3) {
			v->v_priority = udata->vmu_priority;
		}
	}
	bool remove_kv_userdata = false;
	if (udata_sz >= offsetof(_voucher_mach_udata_s, _vmu_after_activity)) {
#if !RDAR_25050791
		remove_kv_userdata = true;
#endif
		if (udata->vmu_magic == VOUCHER_MAGIC_V3 && udata->vmu_activity) {
			v->v_activity = udata->vmu_activity;
			v->v_activity_creator = udata->vmu_activity_pid;
			v->v_parent_activity = udata->vmu_parent_activity;
		}
	}

	if (remove_kv_userdata) {
		mach_voucher_t nkv = MACH_VOUCHER_NULL;
		const mach_voucher_attr_recipe_data_t remove_userdata_recipe[] = {
			[0] = {
				.key = MACH_VOUCHER_ATTR_KEY_ALL,
				.command = MACH_VOUCHER_ATTR_COPY,
				.previous_voucher = kv,
			},
			[1] = {
				.key = MACH_VOUCHER_ATTR_KEY_USER_DATA,
				.command = MACH_VOUCHER_ATTR_REMOVE,
			},
			[2] = {
				.key = MACH_VOUCHER_ATTR_KEY_PTHPRIORITY,
				.command = MACH_VOUCHER_ATTR_REMOVE,
			},
		};
		mach_voucher_attr_recipe_size_t size = sizeof(remove_userdata_recipe);
		kr = _voucher_create_mach_voucher(remove_userdata_recipe, size, &nkv);
		if (!dispatch_assume_zero(kr)) {
			_dispatch_voucher_debug("kvoucher[0x%08x] udata removal "
					"(created 0x%08x)", v, kv, nkv);
			v->v_ipc_kvoucher = MACH_VOUCHER_NULL;
			v->v_kvoucher = nkv;
			v->v_kvbase = _voucher_find_and_retain(nkv);
			if (v->v_kvbase) {
				_voucher_dealloc_mach_voucher(nkv); // borrow base reference
			}
			_voucher_dealloc_mach_voucher(kv);
			kv = nkv;
		} else {
			_dispatch_voucher_debug_machport(kv);
		}
	}

	_voucher_trace(CREATE, v, v->v_kvoucher, v->v_activity);
	_voucher_insert(v);
	_dispatch_voucher_debug("kvoucher[0x%08x] create", v, kv);
	return v;
}

voucher_t
_voucher_create_with_priority_and_mach_voucher(voucher_t ov,
		pthread_priority_t priority, mach_voucher_t kv)
{
	if (priority == _voucher_get_priority(ov)) {
		if (kv) _voucher_dealloc_mach_voucher(kv);
		return ov ? _voucher_retain(ov) : NULL;
	}
	voucher_t v = _voucher_find_and_retain(kv);
	voucher_fields_t ignore_fields = VOUCHER_FIELD_PRIORITY;

	if (v) {
		_dispatch_voucher_debug("kvoucher[0x%08x] find", v, kv);
		_voucher_dealloc_mach_voucher(kv);
		return v;
	}

	if (kv) ignore_fields |= VOUCHER_FIELD_KVOUCHER;
	v = _voucher_clone(ov, ignore_fields);
	if (priority) {
		v->v_priority = (_voucher_priority_t)priority;
	}
	if (kv) {
		v->v_ipc_kvoucher = v->v_kvoucher = kv;
		_voucher_insert(v);
		_dispatch_voucher_debug("kvoucher[0x%08x] create with priority from "
				"voucher[%p]", v, kv, ov);
		_dispatch_voucher_debug_machport(kv);
	}
	_voucher_trace(CREATE, v, v->v_kvoucher, v->v_activity);
	return v;
}

voucher_t
_voucher_create_without_importance(voucher_t ov)
{
	// Nothing to do unless the old voucher has a kernel voucher. If it
	// doesn't, it can't have any importance, now or in the future.
	if (!ov) return NULL;
	if (!ov->v_kvoucher || !ov->v_kv_has_importance) return _voucher_retain(ov);
	kern_return_t kr;
	mach_voucher_t kv, okv;
	// Copy kernel voucher, removing importance.
	okv = ov->v_ipc_kvoucher ? ov->v_ipc_kvoucher : ov->v_kvoucher;
	const mach_voucher_attr_recipe_data_t importance_remove_recipe[] = {
		[0] = {
			.key = MACH_VOUCHER_ATTR_KEY_ALL,
			.command = MACH_VOUCHER_ATTR_COPY,
			.previous_voucher = okv,
		},
		[1] = {
			.key = MACH_VOUCHER_ATTR_KEY_IMPORTANCE,
			.command = MACH_VOUCHER_ATTR_REMOVE,
		},
	};
	kr = _voucher_create_mach_voucher(importance_remove_recipe,
			sizeof(importance_remove_recipe), &kv);
	if (dispatch_assume_zero(kr) || !kv) {
		if (ov->v_ipc_kvoucher) return NULL;
		kv = MACH_VOUCHER_NULL;
	}
	if (kv == okv) {
		_voucher_dealloc_mach_voucher(kv);
		return _voucher_retain(ov);
	}
	voucher_t v = _voucher_find_and_retain(kv);
	if (v && ov->v_ipc_kvoucher) {
		_dispatch_voucher_debug("kvoucher[0x%08x] find without importance "
				"from voucher[%p]", v, kv, ov);
		_voucher_dealloc_mach_voucher(kv);
		return v;
	}
	voucher_t kvbase = v;
	voucher_fields_t ignore_fields = VOUCHER_FIELD_KVOUCHER;
	v = _voucher_clone(ov, ignore_fields);
	v->v_kvoucher = kv;
	if (ov->v_ipc_kvoucher) {
		v->v_ipc_kvoucher = kv;
		_voucher_insert(v);
	} else if (kvbase) {
		v->v_kvbase = kvbase;
		_voucher_dealloc_mach_voucher(kv); // borrow base reference
	}
	if (!kvbase) {
		_dispatch_voucher_debug("kvoucher[0x%08x] create without importance "
				"from voucher[%p]", v, kv, ov);
	}
	_voucher_trace(CREATE, v, v->v_kvoucher, v->v_activity);
	return v;
}

voucher_t
_voucher_create_accounting_voucher(voucher_t ov)
{
	// Nothing to do unless the old voucher has a kernel voucher. If it does
	// doesn't, it can't have any accounting attributes.
	if (!ov || !ov->v_kvoucher) return NULL;
	kern_return_t kr = KERN_SUCCESS;
	mach_voucher_t okv, kv = MACH_VOUCHER_NULL;
	okv = ov->v_ipc_kvoucher ? ov->v_ipc_kvoucher : ov->v_kvoucher;
	const mach_voucher_attr_recipe_data_t accounting_copy_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_BANK,
		.command = MACH_VOUCHER_ATTR_COPY,
		.previous_voucher = okv,
	};
	kr = _voucher_create_mach_voucher(&accounting_copy_recipe,
			sizeof(accounting_copy_recipe), &kv);
	if (dispatch_assume_zero(kr) || !kv) {
		return NULL;
	}
	voucher_t v = _voucher_find_and_retain(kv);
	if (v) {
		_dispatch_voucher_debug("kvoucher[0x%08x] find accounting voucher "
				"from voucher[%p]", v, kv, ov);
		_voucher_dealloc_mach_voucher(kv);
		return v;
	}
	v = _voucher_alloc(0);
	v->v_ipc_kvoucher = v->v_kvoucher = kv;
	if (kv == okv) {
		v->v_kvbase = _voucher_retain(ov);
		_voucher_dealloc_mach_voucher(kv); // borrow base reference
	}
	_voucher_trace(CREATE, v, kv, v->v_activity);
	_voucher_insert(v);
	_dispatch_voucher_debug("kvoucher[0x%08x] create accounting voucher "
			"from voucher[%p]", v, kv, ov);
	return v;
}

voucher_t
voucher_create_with_mach_msg(mach_msg_header_t *msg)
{
	mach_msg_bits_t msgh_bits;
	mach_voucher_t kv = _voucher_mach_msg_get(msg, &msgh_bits);
	return _voucher_create_with_mach_voucher(kv, msgh_bits);
}

void
voucher_decrement_importance_count4CF(voucher_t v)
{
	if (!v || !v->v_kvoucher || !v->v_kv_has_importance) return;
	kern_return_t kr;
	mach_voucher_t kv = v->v_ipc_kvoucher ? v->v_ipc_kvoucher : v->v_kvoucher;
	uint32_t dec = 1;
	mach_voucher_attr_content_t kvc_in = (mach_voucher_attr_content_t)&dec;
	mach_voucher_attr_content_size_t kvc_in_size = sizeof(dec);
	mach_voucher_attr_content_t kvc_out = NULL;
	mach_voucher_attr_content_size_t kvc_out_size = 0;
#if DISPATCH_DEBUG
	uint32_t count = UINT32_MAX;
	kvc_out = (mach_voucher_attr_content_t)&count;
	kvc_out_size = sizeof(count);
#endif
	kr = mach_voucher_attr_command(kv, MACH_VOUCHER_ATTR_KEY_IMPORTANCE,
			MACH_VOUCHER_IMPORTANCE_ATTR_DROP_EXTERNAL, kvc_in, kvc_in_size,
			kvc_out, &kvc_out_size);
	DISPATCH_VERIFY_MIG(kr);
	if (kr == KERN_INVALID_TASK) return; // non-denap receiver rdar://25643185
#if DISPATCH_DEBUG
	_dispatch_voucher_debug("kvoucher[0x%08x] decrement importance count to %u:"
			" %s - 0x%x", v, kv, count, mach_error_string(kr), kr);
#endif
	if (unlikely(dispatch_assume_zero(kr) == KERN_FAILURE)) {
		DISPATCH_CLIENT_CRASH(kr, "Voucher importance count underflow");
	}
}

#if VOUCHER_ENABLE_GET_MACH_VOUCHER
mach_voucher_t
voucher_get_mach_voucher(voucher_t voucher)
{
	return _voucher_get_mach_voucher(voucher);
}
#endif

void
_voucher_xref_dispose(voucher_t voucher)
{
	_dispatch_voucher_debug("xref_dispose", voucher);
	_voucher_remove(voucher);
	return _os_object_release_internal_n_inline((_os_object_t)voucher, 1);
}

void
_voucher_dispose(voucher_t voucher)
{
	_voucher_trace(DISPOSE, voucher);
	_dispatch_voucher_debug("dispose", voucher);
	if (unlikely(_voucher_hash_is_enqueued(voucher))) {
		_dispatch_voucher_debug("corruption", voucher);
		DISPATCH_CLIENT_CRASH(0, "Voucher corruption");
	}
	_voucher_hash_mark_not_enqueued(voucher);
	if (voucher->v_ipc_kvoucher) {
		if (voucher->v_ipc_kvoucher != voucher->v_kvoucher) {
			_voucher_dealloc_mach_voucher(voucher->v_ipc_kvoucher);
		}
		voucher->v_ipc_kvoucher = MACH_VOUCHER_NULL;
	}
	if (voucher->v_kvoucher) {
		if (!voucher->v_kvbase) {
			_voucher_dealloc_mach_voucher(voucher->v_kvoucher);
		}
		voucher->v_kvoucher = MACH_VOUCHER_NULL;
	}
	if (voucher->v_kvbase) {
		_voucher_release(voucher->v_kvbase);
		voucher->v_kvbase = NULL;
	}
	voucher->v_activity = 0;
	voucher->v_activity_creator = 0;
	voucher->v_parent_activity = 0;
	voucher->v_priority = 0;
#if VOUCHER_ENABLE_RECIPE_OBJECTS
	voucher->v_recipe_extra_size = 0;
	voucher->v_recipe_extra_offset = 0;
#endif
#if !USE_OBJC
	return _os_object_dealloc((_os_object_t)voucher);
#endif // !USE_OBJC
}

void
_voucher_activity_debug_channel_init(void)
{
	dispatch_mach_handler_function_t handler = NULL;

	if (_voucher_libtrace_hooks) {
		handler = _voucher_libtrace_hooks->vah_debug_channel_handler;
	}
	if (!handler) return;

	dispatch_mach_t dm;
	mach_port_t dbgp;
	kern_return_t kr;

	kr = task_get_debug_control_port(mach_task_self(), &dbgp);
	DISPATCH_VERIFY_MIG(kr);
	if (kr) {
		DISPATCH_CLIENT_CRASH(kr, "Couldn't get debug control port");
	}
	if (dbgp) {
		dm = dispatch_mach_create_f("com.apple.debug-channel",
				DISPATCH_TARGET_QUEUE_DEFAULT, NULL, handler);
		dm->dm_recv_refs->du_can_be_wlh = false; // 29906118
		dispatch_mach_connect(dm, dbgp, MACH_PORT_NULL, NULL);
		_voucher_activity_debug_channel = dm;
	}
}

void
_voucher_atfork_child(void)
{
	_dispatch_thread_setspecific(dispatch_voucher_key, NULL);
	_voucher_task_mach_voucher_pred = 0;
	_voucher_task_mach_voucher = MACH_VOUCHER_NULL;
#if !VOUCHER_USE_EMPTY_MACH_BASE_VOUCHER
	_voucher_default_task_mach_voucher = MACH_PORT_NULL;
#endif
	_voucher_aid_next = 0;
	_firehose_task_buffer_pred = 0;
	_firehose_task_buffer = NULL; // firehose buffer is VM_INHERIT_NONE
}

voucher_t
voucher_copy_with_persona_mach_voucher(mach_voucher_t persona_mach_voucher)
{
#if !VOUCHER_USE_PERSONA
	(void)persona_mach_voucher;
	return voucher_copy();
#else // !VOUCHER_USE_PERSONA
	if (!persona_mach_voucher) return voucher_copy();
	kern_return_t kr;
	mach_voucher_t okv = MACH_VOUCHER_NULL, kv;
	voucher_t ov = _voucher_get();
	if (ov) {
		okv = ov->v_ipc_kvoucher ? ov->v_ipc_kvoucher : ov->v_kvoucher;
	}
	const mach_voucher_attr_recipe_data_t bank_redeem_recipe[] = {
		[0] = {
			.key = MACH_VOUCHER_ATTR_KEY_ALL,
			.command = MACH_VOUCHER_ATTR_COPY,
			.previous_voucher = okv,
		},
		[1] = {
			.key = MACH_VOUCHER_ATTR_KEY_BANK,
			.command = MACH_VOUCHER_ATTR_REDEEM,
			.previous_voucher = persona_mach_voucher,
		},
	};
	kr = _voucher_create_mach_voucher(bank_redeem_recipe,
			sizeof(bank_redeem_recipe), &kv);
	if (dispatch_assume_zero(kr)) {
		if (kr == KERN_INVALID_CAPABILITY) {
			// bank attribute redeem failed
			return VOUCHER_INVALID;
		}
		kv = MACH_VOUCHER_NULL;
	}
	if (kv == okv) {
		if (kv) _voucher_dealloc_mach_voucher(kv);
		return _voucher_retain(ov);
	}
	voucher_t v = _voucher_find_and_retain(kv);
	if (v && (!ov || ov->v_ipc_kvoucher)) {
		_dispatch_voucher_debug("kvoucher[0x%08x] find with persona "
				"from voucher[%p]", v, kv, ov);
		_voucher_dealloc_mach_voucher(kv);
		return v;
	}
	voucher_t kvbase = v;
	voucher_fields_t ignore_fields = VOUCHER_FIELD_KVOUCHER;
	v = _voucher_clone(ov, ignore_fields);
	v->v_kvoucher = kv;
	if (!ov || ov->v_ipc_kvoucher) {
		v->v_ipc_kvoucher = kv;
		_voucher_insert(v);
	} else if (kvbase) {
		v->v_kvbase = kvbase;
		_voucher_dealloc_mach_voucher(kv); // borrow base reference
	}
	if (!kvbase) {
		_dispatch_voucher_debug("kvoucher[0x%08x] create with persona "
				"from voucher[%p]", v, kv, ov);
	}
	_voucher_trace(CREATE, v, v->v_kvoucher, v->v_activity);
	return v;
#endif // VOUCHER_USE_PERSONA
}

kern_return_t
mach_voucher_persona_self(mach_voucher_t *persona_mach_voucher)
{
	mach_voucher_t bkv = MACH_VOUCHER_NULL;
	kern_return_t kr = KERN_NOT_SUPPORTED;
#if VOUCHER_USE_PERSONA
	const mach_voucher_attr_recipe_data_t bank_send_recipe[] = {
		[0] = {
			.key = MACH_VOUCHER_ATTR_KEY_BANK,
			.command = MACH_VOUCHER_ATTR_BANK_CREATE,
		},
		[1] = {
			.key = MACH_VOUCHER_ATTR_KEY_BANK,
			.command = MACH_VOUCHER_ATTR_SEND_PREPROCESS,
		},
	};
	kr = _voucher_create_mach_voucher(bank_send_recipe,
			sizeof(bank_send_recipe), &bkv);
	if (dispatch_assume_zero(kr)) {
		bkv = MACH_VOUCHER_NULL;
	}
#endif // VOUCHER_USE_PERSONA
	*persona_mach_voucher = bkv;
	return kr;
}

kern_return_t
mach_voucher_persona_for_originator(uid_t persona_id,
	mach_voucher_t originator_persona_mach_voucher,
	uint64_t originator_unique_pid, mach_voucher_t *persona_mach_voucher)
{
	mach_voucher_t bkv = MACH_VOUCHER_NULL;
	kern_return_t kr = KERN_NOT_SUPPORTED;
#if VOUCHER_USE_PERSONA
	struct persona_modify_info modify_info = {
		.persona_id = persona_id,
		.unique_pid = originator_unique_pid,
	};
	size_t bank_modify_recipe_size = _voucher_mach_recipe_size(0) +
			_voucher_mach_recipe_size(sizeof(modify_info));
	mach_voucher_attr_recipe_t bank_modify_recipe =
			(mach_voucher_attr_recipe_t)alloca(bank_modify_recipe_size);

	bzero((void *)bank_modify_recipe, bank_modify_recipe_size);

	bank_modify_recipe[0] = (mach_voucher_attr_recipe_data_t){
		.key = MACH_VOUCHER_ATTR_KEY_BANK,
		.command = MACH_VOUCHER_ATTR_COPY,
		.previous_voucher = originator_persona_mach_voucher,
	};
	bank_modify_recipe[1] = (mach_voucher_attr_recipe_data_t){
		.key = MACH_VOUCHER_ATTR_KEY_BANK,
		.command = MACH_VOUCHER_ATTR_BANK_MODIFY_PERSONA,
		.content_size = sizeof(modify_info),
	};
	_dispatch_memappend(bank_modify_recipe[1].content, &modify_info);
	kr = _voucher_create_mach_voucher(bank_modify_recipe,
			bank_modify_recipe_size, &bkv);
	if (dispatch_assume_zero(kr)) {
		bkv = MACH_VOUCHER_NULL;
	}
#else // VOUCHER_USE_PERSONA
	(void)persona_id;
	(void)originator_persona_mach_voucher;
	(void)originator_unique_pid;
#endif // VOUCHER_USE_PERSONA
	*persona_mach_voucher = bkv;
	return kr;
}

#if VOUCHER_USE_PERSONA
static kern_return_t
_voucher_get_current_persona_token(struct persona_token *token)
{
	kern_return_t kr = KERN_FAILURE;
	voucher_t v = _voucher_get();

	if (v && v->v_kvoucher) {
		mach_voucher_t kv = v->v_ipc_kvoucher ?: v->v_kvoucher;
		mach_voucher_attr_content_t kvc_in = NULL;
		mach_voucher_attr_content_size_t kvc_in_size = 0;
		mach_voucher_attr_content_t kvc_out =
			(mach_voucher_attr_content_t)token;
		mach_voucher_attr_content_size_t kvc_out_size = sizeof(*token);

		kr = mach_voucher_attr_command(kv, MACH_VOUCHER_ATTR_KEY_BANK,
				BANK_PERSONA_TOKEN, kvc_in, kvc_in_size,
				kvc_out, &kvc_out_size);
		if (kr != KERN_NOT_SUPPORTED
				// Voucher doesn't have a PERSONA_TOKEN
				&& kr != KERN_INVALID_VALUE
				// Kernel doesn't understand BANK_PERSONA_TOKEN
				&& kr != KERN_INVALID_ARGUMENT) {
			(void)dispatch_assume_zero(kr);
		}
	}
	return kr;
}

static kern_return_t
_voucher_get_current_persona_id(uid_t *persona_id)
{
	kern_return_t kr = KERN_FAILURE;
	voucher_t v = _voucher_get();

	if (v && v->v_kvoucher) {
		mach_voucher_t kv = v->v_ipc_kvoucher ?: v->v_kvoucher;
		mach_voucher_attr_content_t kvc_in = NULL;
		mach_voucher_attr_content_size_t kvc_in_size = 0;
		mach_voucher_attr_content_t kvc_out =
			(mach_voucher_attr_content_t)persona_id;
		mach_voucher_attr_content_size_t kvc_out_size = sizeof(*persona_id);

		kr = mach_voucher_attr_command(kv, MACH_VOUCHER_ATTR_KEY_BANK,
				BANK_PERSONA_ID, kvc_in, kvc_in_size,
				kvc_out, &kvc_out_size);
		if (kr != KERN_NOT_SUPPORTED
				// Voucher doesn't have a persona id
				&& kr != KERN_INVALID_VALUE
				// Kernel doesn't understand BANK_PERSONA_ID
				&& kr != KERN_INVALID_ARGUMENT) {
			(void)dispatch_assume_zero(kr);
		}
	}
	return kr;
}
#endif // VOUCHER_USE_PERSONA

uid_t
voucher_get_current_persona(void)
{
	uid_t persona_id = PERSONA_ID_NONE;

#if VOUCHER_USE_PERSONA
	int err;

	if (_voucher_get_current_persona_id(&persona_id) == KERN_SUCCESS) {
		return persona_id;
	}

	// falling back to the process persona if there is no adopted voucher
	if (kpersona_get(&persona_id) < 0) {
		err = errno;
		if (err != ESRCH) {
			(void)dispatch_assume_zero(err);
		}
	}
#endif // VOUCHER_USE_PERSONA
	return persona_id;
}

int
voucher_get_current_persona_originator_info(struct proc_persona_info *persona_info)
{
#if VOUCHER_USE_PERSONA
	struct persona_token token;
	if (_voucher_get_current_persona_token(&token) == KERN_SUCCESS) {
		*persona_info = token.originator;
		return 0;
	}
#else // VOUCHER_USE_PERSONA
	(void)persona_info;
#endif // VOUCHER_USE_PERSONA
	return -1;
}

int
voucher_get_current_persona_proximate_info(struct proc_persona_info *persona_info)
{
#if VOUCHER_USE_PERSONA
	struct persona_token token;
	if (_voucher_get_current_persona_token(&token) == KERN_SUCCESS) {
		*persona_info = token.proximate;
		return 0;
	}
#else // VOUCHER_USE_PERSONA
	(void)persona_info;
#endif // VOUCHER_USE_PERSONA
	return -1;
}

#pragma mark -
#pragma mark _voucher_init

boolean_t
voucher_mach_msg_set(mach_msg_header_t *msg)
{
	return _voucher_mach_msg_set(msg, _voucher_get());
}

void
voucher_mach_msg_clear(mach_msg_header_t *msg)
{
	(void)_voucher_mach_msg_clear(msg, false);
}

voucher_mach_msg_state_t
voucher_mach_msg_adopt(mach_msg_header_t *msg)
{
	mach_msg_bits_t msgh_bits;
	mach_voucher_t kv = _voucher_mach_msg_get(msg, &msgh_bits);
	if (!kv) return VOUCHER_MACH_MSG_STATE_UNCHANGED;
	voucher_t v = _voucher_create_with_mach_voucher(kv, msgh_bits);
	return (voucher_mach_msg_state_t)_voucher_adopt(v);
}

void
voucher_mach_msg_revert(voucher_mach_msg_state_t state)
{
	if (state == VOUCHER_MACH_MSG_STATE_UNCHANGED) return;
	_voucher_replace((voucher_t)state);
}

#if DISPATCH_USE_LIBKERNEL_VOUCHER_INIT
#include <_libkernel_init.h>

static const struct _libkernel_voucher_functions _voucher_libkernel_functions =
{
	.version = 1,
	.voucher_mach_msg_set = voucher_mach_msg_set,
	.voucher_mach_msg_clear = voucher_mach_msg_clear,
	.voucher_mach_msg_adopt = voucher_mach_msg_adopt,
	.voucher_mach_msg_revert = voucher_mach_msg_revert,
};

static void
_voucher_libkernel_init(void)
{
	kern_return_t kr = __libkernel_voucher_init(&_voucher_libkernel_functions);
	dispatch_assert(!kr);
}
#else
#define _voucher_libkernel_init()
#endif

void
voucher_activity_initialize_4libtrace(voucher_activity_hooks_t hooks)
{
	if (hooks->vah_version < 3) {
		DISPATCH_CLIENT_CRASH(hooks->vah_version, "unsupported vah_version");
	}
	if (!os_atomic_cmpxchg(&_voucher_libtrace_hooks, NULL,
			hooks, relaxed)) {
		DISPATCH_CLIENT_CRASH(_voucher_libtrace_hooks,
				"voucher_activity_initialize_4libtrace called twice");
	}
}

void
_voucher_init(void)
{
	_voucher_libkernel_init();
}

#pragma mark -
#pragma mark voucher_activity_t

DISPATCH_NOINLINE
static uint64_t
_voucher_activity_id_allocate_slow(uint64_t aid)
{
	kern_return_t kr;
	uint64_t next;

	kr = mach_generate_activity_id(mach_task_self(), 1, &next);
	if (unlikely(kr)) {
		DISPATCH_CLIENT_CRASH(kr, "Could not generate an activity ID");
	}
	next *= MACH_ACTIVITY_ID_RANGE_SIZE;
	next &= MACH_ACTIVITY_ID_MASK;
	if (unlikely(next == 0)) {
		next++;
	}

	if (unlikely(aid == 0)) {
		if (os_atomic_cmpxchg(&_voucher_aid_next, 0, next + 1, relaxed)) {
			return next;
		}
	}
	return os_atomic_xchg(&_voucher_aid_next, next, relaxed);
}

DISPATCH_ALWAYS_INLINE
static firehose_activity_id_t
_voucher_activity_id_allocate(firehose_activity_flags_t flags)
{
	uint64_t aid, next;
	os_atomic_rmw_loop(&_voucher_aid_next, aid, next, relaxed, {
		next = aid + 1;
		if (aid == 0 || next % MACH_ACTIVITY_ID_RANGE_SIZE == 0) {
			os_atomic_rmw_loop_give_up({
				aid = _voucher_activity_id_allocate_slow(aid);
				break;
			});
		}
	});
	return FIREHOSE_ACTIVITY_ID_MAKE(aid, flags);
}

firehose_activity_id_t
voucher_activity_id_allocate(firehose_activity_flags_t flags)
{
	return _voucher_activity_id_allocate(flags);
}

#define _voucher_activity_tracepoint_reserve(stamp, stream, pub, priv, \
		privbuf, reliable) \
		firehose_buffer_tracepoint_reserve(_firehose_task_buffer, stamp, \
				stream, pub, priv, privbuf, reliable)

#define _voucher_activity_tracepoint_flush(ft, ftid) \
		firehose_buffer_tracepoint_flush(_firehose_task_buffer, ft, ftid)

DISPATCH_NOINLINE
static void
_firehose_task_buffer_init(void *ctx OS_UNUSED)
{
	mach_port_t logd_port;

	/* Query the uniquepid of the current process */
	struct proc_uniqidentifierinfo p_uniqinfo = { };
	int info_size = 0;

	info_size = proc_pidinfo(getpid(), PROC_PIDUNIQIDENTIFIERINFO, 1,
			&p_uniqinfo, PROC_PIDUNIQIDENTIFIERINFO_SIZE);
	if (unlikely(info_size != PROC_PIDUNIQIDENTIFIERINFO_SIZE)) {
		if (info_size == 0) {
			DISPATCH_INTERNAL_CRASH(errno,
				"Unable to get the unique pid (error)");
		} else {
			DISPATCH_INTERNAL_CRASH(info_size,
				"Unable to get the unique pid (size)");
		}
	}
	_voucher_unique_pid = p_uniqinfo.p_uniqueid;


	if (unlikely(!_voucher_libtrace_hooks)) {
		return;
	}
	logd_port = _voucher_libtrace_hooks->vah_get_logd_port();
	if (logd_port) {
		unsigned long flags = 0;
#if DISPATCH_USE_MEMORYPRESSURE_SOURCE
		if (_dispatch_memory_warn) {
			flags |= FIREHOSE_BUFFER_BANK_FLAG_LOW_MEMORY;
		}
#endif
		// firehose_buffer_create always consumes the send-right
		_firehose_task_buffer = firehose_buffer_create(logd_port,
				_voucher_unique_pid, flags);
		if (_voucher_libtrace_hooks->vah_version >= 4 &&
				_voucher_libtrace_hooks->vah_metadata_init) {
			firehose_buffer_t fb = _firehose_task_buffer;
			size_t meta_sz = FIREHOSE_BUFFER_LIBTRACE_HEADER_SIZE;
			void *meta = (void *)((uintptr_t)(&fb->fb_header + 1) - meta_sz);
			_voucher_libtrace_hooks->vah_metadata_init(meta, meta_sz);
		}
	}
}

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_activity_disabled(void)
{

	dispatch_once_f(&_firehose_task_buffer_pred,
			NULL, _firehose_task_buffer_init);

	firehose_buffer_t fb = _firehose_task_buffer;
	return !fb || fb->fb_header.fbh_sendp[false] == MACH_PORT_DEAD;
}

void *
voucher_activity_get_logging_preferences(size_t *length)
{
	if (unlikely(_voucher_activity_disabled())) {
		*length = 0;
		return NULL;
	}

	return firehose_buffer_get_logging_prefs(_firehose_task_buffer, length);
}

bool
voucher_activity_should_send_strings(void)
{
	if (unlikely(_voucher_activity_disabled())) {
		return false;
	}

	return firehose_buffer_should_send_strings(_firehose_task_buffer);
}

void *
voucher_activity_get_metadata_buffer(size_t *length)
{
	if (_voucher_activity_disabled()) {
		*length = 0;
		return NULL;
	}

	firehose_buffer_header_t fbh = &_firehose_task_buffer->fb_header;

	*length = FIREHOSE_BUFFER_LIBTRACE_HEADER_SIZE;
	return (void *)((uintptr_t)(fbh + 1) - *length);
}

voucher_t
voucher_activity_create_with_data(firehose_tracepoint_id_t *trace_id,
		voucher_t base, firehose_activity_flags_t flags,
		const void *pubdata, size_t publen)
{
	firehose_activity_id_t va_id = 0, current_id = 0, parent_id = 0;
	firehose_tracepoint_id_u ftid = { .ftid_value = *trace_id };
	uint64_t creator_id = 0;
	uint16_t pubsize;
	voucher_t ov = _voucher_get();
	voucher_t v;

	if (os_add_overflow(sizeof(va_id), publen, &pubsize) || pubsize > 128) {
		DISPATCH_CLIENT_CRASH(pubsize, "Absurd publen");
	}
	if (base == VOUCHER_CURRENT) {
		base = ov;
	}

	FIREHOSE_TRACE_ID_CLEAR_FLAG(ftid, base, has_unique_pid);
	if (ov && (current_id = ov->v_activity)) {
		FIREHOSE_TRACE_ID_SET_FLAG(ftid, base, has_current_aid);
		pubsize += sizeof(firehose_activity_id_t);
		if ((creator_id = ov->v_activity_creator)) {
			FIREHOSE_TRACE_ID_SET_FLAG(ftid, base, has_unique_pid);
			pubsize += sizeof(uint64_t);
		}
	}
	if (base != VOUCHER_NULL) {
		parent_id = base->v_activity;
	}

	if (parent_id) {
		FIREHOSE_TRACE_ID_SET_FLAG(ftid, activity, has_other_aid);
		pubsize += sizeof(firehose_activity_id_t);
		flags |= FIREHOSE_ACTIVITY_ID_FLAGS(parent_id);
	}

	if (firehose_precise_timestamps_enabled()) {
		flags |= firehose_activity_flags_precise_timestamp;
	}
	voucher_fields_t ignore_fields = VOUCHER_FIELD_ACTIVITY;
	v = _voucher_clone(base, ignore_fields);
	v->v_activity = va_id = _voucher_activity_id_allocate(flags);
	v->v_activity_creator = _voucher_unique_pid;
	v->v_parent_activity = parent_id;

	if (_voucher_activity_disabled()) {
		goto done;
	}

	static const firehose_stream_t streams[2] = {
		firehose_stream_metadata,
		firehose_stream_persist,
	};
	firehose_tracepoint_t ft;
	uint64_t stamp = firehose_tracepoint_time(flags);

	for (size_t i = 0; i < countof(streams); i++) {
		ft = _voucher_activity_tracepoint_reserve(stamp, streams[i], pubsize,
				0, NULL, true);
		if (unlikely(!ft)) continue;

		uint8_t *pubptr = ft->ft_data;
		if (current_id) {
			pubptr = _dispatch_memappend(pubptr, &current_id);
		}
		if (creator_id) {
			pubptr = _dispatch_memappend(pubptr, &creator_id);
		}
		if (parent_id) {
			pubptr = _dispatch_memappend(pubptr, &parent_id);
		}
		pubptr = _dispatch_memappend(pubptr, &va_id);
		pubptr = _dispatch_mempcpy(pubptr, pubdata, publen);
		_voucher_activity_tracepoint_flush(ft, ftid);
	}
done:
	*trace_id = ftid.ftid_value;
	_voucher_trace(CREATE, v, v->v_kvoucher, va_id);
	return v;
}

voucher_t
voucher_activity_create_with_location(firehose_tracepoint_id_t *trace_id,
		voucher_t base, firehose_activity_flags_t flags, uint64_t loc)
{
	return voucher_activity_create_with_data(trace_id, base, flags,
			&loc, sizeof(loc));
}

#if OS_VOUCHER_ACTIVITY_GENERATE_SWAPS
void
_voucher_activity_swap(firehose_activity_id_t old_id,
		firehose_activity_id_t new_id)
{
	if (_voucher_activity_disabled()) return;

	firehose_tracepoint_id_u ftid = { .ftid = {
		._namespace = firehose_tracepoint_namespace_activity,
		._type = _firehose_tracepoint_type_activity_swap,
	} };
	uint16_t pubsize = 0;

	if (old_id) {
		FIREHOSE_TRACE_ID_SET_FLAG(ftid, base, has_current_aid);
		pubsize += sizeof(firehose_activity_id_t);
	}
	if (new_id) {
		FIREHOSE_TRACE_ID_SET_FLAG(ftid, activity, has_other_aid);
		pubsize += sizeof(firehose_activity_id_t);
	}

	firehose_stream_t stream = firehose_stream_metadata;
	firehose_tracepoint_t ft;
	firehose_activity_flags_t flags = FIREHOSE_ACTIVITY_ID_FLAGS(old_id) |
			FIREHOSE_ACTIVITY_ID_FLAGS(new_id);
	uint64_t stamp = firehose_tracepoint_time(flags);

	_dispatch_voucher_ktrace_activity_adopt(new_id);

	ft = _voucher_activity_tracepoint_reserve(stamp, stream, pubsize, 0, NULL,
			true);
	if (unlikely(!ft)) return;
	uint8_t *pubptr = ft->ft_data;
	if (old_id) pubptr = _dispatch_memappend(pubptr, &old_id);
	if (new_id) pubptr = _dispatch_memappend(pubptr, &new_id);
	_voucher_activity_tracepoint_flush(ft, ftid);
}
#endif

firehose_activity_id_t
voucher_get_activity_id_and_creator(voucher_t v, uint64_t *creator_pid,
		firehose_activity_id_t *parent_id)
{
	if (v == VOUCHER_CURRENT) {
		v = _voucher_get();
	}
	if (v == VOUCHER_NULL) {
		if (creator_pid) *creator_pid = 0;
		if (parent_id) *parent_id = FIREHOSE_ACTIVITY_ID_NULL;
		return FIREHOSE_ACTIVITY_ID_NULL;
	}
	if (creator_pid) *creator_pid = v->v_activity_creator;
	if (parent_id) *parent_id = v->v_parent_activity;
	return v->v_activity;
}

firehose_activity_id_t
voucher_get_activity_id(voucher_t v, firehose_activity_id_t *parent_id)
{
	return voucher_get_activity_id_and_creator(v, NULL, parent_id);
}

void
voucher_activity_flush(firehose_stream_t stream)
{
	if (_voucher_activity_disabled()) return;
	firehose_buffer_stream_flush(_firehose_task_buffer, stream);
}

DISPATCH_NOINLINE
firehose_tracepoint_id_t
voucher_activity_trace_v_2(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t stamp,
		const struct iovec *iov, size_t publen, size_t privlen, uint32_t flags)
{
	firehose_tracepoint_id_u ftid = { .ftid_value = trace_id };
	const uint16_t ft_size = offsetof(struct firehose_tracepoint_s, ft_data);
	const size_t _firehose_chunk_payload_size =
			sizeof(((struct firehose_chunk_s *)0)->fc_data);
	bool reliable = !(flags & VOUCHER_ACTIVITY_TRACE_FLAG_UNRELIABLE);

	if (_voucher_activity_disabled()) return 0;

	firehose_tracepoint_t ft;
	firehose_activity_id_t va_id = 0;
	firehose_chunk_t fc;
	uint8_t *privptr, *pubptr;
	size_t pubsize = publen;
	voucher_t ov = _voucher_get();
	uint64_t creator_pid;

	if ((va_id = _voucher_get_activity_id(ov, &creator_pid))) {
		FIREHOSE_TRACE_ID_SET_FLAG(ftid, base, has_current_aid);
		pubsize += sizeof(va_id);
	}
	if (FIREHOSE_TRACE_ID_HAS_FLAG(ftid, base, has_unique_pid)) {
		if (creator_pid) {
			pubsize += sizeof(creator_pid);
		} else {
			FIREHOSE_TRACE_ID_CLEAR_FLAG(ftid, base, has_unique_pid);
		}
	} else {
		creator_pid = 0;
	}

	if (privlen) {
		FIREHOSE_TRACE_ID_SET_FLAG(ftid, log, has_private_data);
		pubsize += sizeof(struct firehose_buffer_range_s);
	}

	if (unlikely(ft_size + pubsize + privlen > _firehose_chunk_payload_size)) {
		DISPATCH_CLIENT_CRASH(ft_size + pubsize + privlen, "Log is too large");
	}

	ft = _voucher_activity_tracepoint_reserve(stamp, stream, (uint16_t)pubsize,
				(uint16_t)privlen, &privptr, reliable);
	if (unlikely(!ft)) return 0;
	pubptr = ft->ft_data;
	if (va_id) {
		pubptr = _dispatch_memappend(pubptr, &va_id);
	}
	if (creator_pid) {
		pubptr = _dispatch_memappend(pubptr, &creator_pid);
	}
	if (privlen) {
		fc = firehose_buffer_chunk_for_address(ft);
		struct firehose_buffer_range_s range = {
			.fbr_offset = (uint16_t)(privptr - fc->fc_start),
			.fbr_length = (uint16_t)privlen,
		};
		pubptr = _dispatch_memappend(pubptr, &range);
	}
	while (publen > 0) {
		pubptr = _dispatch_mempcpy(pubptr, iov->iov_base, iov->iov_len);
		if (unlikely(os_sub_overflow(publen, iov->iov_len, &publen))) {
			DISPATCH_CLIENT_CRASH(0, "Invalid arguments");
		}
		iov++;
	}
	while (privlen > 0) {
		privptr = _dispatch_mempcpy(privptr, iov->iov_base, iov->iov_len);
		if (unlikely(os_sub_overflow(privlen, iov->iov_len, &privlen))) {
			DISPATCH_CLIENT_CRASH(0, "Invalid arguments");
		}
		iov++;
	}
	_voucher_activity_tracepoint_flush(ft, ftid);
	return ftid.ftid_value;
}

DISPATCH_NOINLINE
firehose_tracepoint_id_t
voucher_activity_trace_v(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t stamp,
		const struct iovec *iov, size_t publen, size_t privlen)
{
	return voucher_activity_trace_v_2(stream, trace_id, stamp, iov, publen,
			privlen, 0);
}

firehose_tracepoint_id_t
voucher_activity_trace(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t stamp,
		const void *pubdata, size_t publen)
{
	struct iovec iov = { (void *)pubdata, publen };
	return voucher_activity_trace_v(stream, trace_id, stamp, &iov, publen, 0);
}

#pragma mark -
#pragma mark _voucher_debug

#define bufprintf(...) \
		offset += dsnprintf(&buf[offset], bufsiz > offset ? bufsiz - offset : 0, ##__VA_ARGS__)
#define bufprintprefix() \
		if (prefix) bufprintf("%s", prefix)
#define VOUCHER_DETAIL_PREFIX "        "
#define IKOT_VOUCHER	37U
#define VOUCHER_CONTENTS_SIZE 8192
#define MAX_HEX_DATA_SIZE 1024

size_t
_voucher_debug(voucher_t v, char *buf, size_t bufsiz)
{
	size_t offset = 0;
	bufprintf("voucher[%p] = { xref = %d, ref = %d", v,
			v->os_obj_xref_cnt + 1, v->os_obj_ref_cnt + 1);

	if (v->v_kvbase) {
		bufprintf(", base voucher %p", v->v_kvbase);
	}
	if (v->v_kvoucher) {
		bufprintf(", kvoucher%s 0x%x [\n", v->v_kvoucher == v->v_ipc_kvoucher ?
				" & ipc kvoucher" : "", v->v_kvoucher);
		offset = voucher_kvoucher_debug(mach_task_self(), v->v_kvoucher, buf,
				bufsiz, offset, VOUCHER_DETAIL_PREFIX, MAX_HEX_DATA_SIZE);
		bufprintf("]");
	}
	if (v->v_ipc_kvoucher && v->v_ipc_kvoucher != v->v_kvoucher) {
		bufprintf(", ipc kvoucher 0x%x [\n", v->v_ipc_kvoucher);
		offset = voucher_kvoucher_debug(mach_task_self(), v->v_ipc_kvoucher,
				buf, bufsiz, offset, VOUCHER_DETAIL_PREFIX, MAX_HEX_DATA_SIZE);
		bufprintf("]");
	}
	if (v->v_priority) {
		bufprintf(", QOS 0x%x", v->v_priority);
	}
	if (v->v_activity) {
		bufprintf(", activity 0x%llx (pid: 0x%16llx, parent 0x%llx)",
				v->v_activity, v->v_activity_creator, v->v_parent_activity);
	}
	bufprintf(" }");
	
	return offset;
}

static size_t
format_hex_data(char *prefix, char *desc, uint8_t *data, size_t data_len,
	   char *buf, size_t bufsiz, size_t offset)
{
	size_t i;
	uint8_t chars[17];
	uint8_t *pc = data;

	if (desc) {
 		bufprintf("%s%s:\n", prefix, desc);
	}

	ssize_t offset_in_row = -1;
	for (i = 0; i < data_len; i++) {
		offset_in_row = i % 16;
		if (offset_in_row == 0) {
			if (i != 0) {
				bufprintf("  %s\n", chars);
			}
			bufprintf("%s  %04lx ", prefix, i);
		}
		bufprintf(" %02x", pc[i]);
		chars[offset_in_row] = (pc[i] < 0x20) || (pc[i] > 0x7e) ? '.' : pc[i];
	}
	chars[offset_in_row + 1] = '\0';

	if ((i % 16) != 0) {
		while ((i % 16) != 0) {
			bufprintf("   ");
			i++;
		}
		bufprintf("  %s\n", chars);
	}
	return offset;
}

static size_t
format_recipe_detail(mach_voucher_attr_recipe_t recipe, char *buf,
		size_t bufsiz, size_t offset, char *prefix, size_t max_hex_data)
{
	bufprintprefix();
	bufprintf("Key: %u, ", recipe->key);
	bufprintf("Command: %u, ", recipe->command);
	bufprintf("Previous voucher: 0x%x, ", recipe->previous_voucher);
	bufprintf("Content size: %u\n", recipe->content_size);

	switch (recipe->key) {
	case MACH_VOUCHER_ATTR_KEY_ATM:
		bufprintprefix();
		bufprintf("ATM ID: %llu", *(uint64_t *)(uintptr_t)recipe->content);
		break;
	case MACH_VOUCHER_ATTR_KEY_IMPORTANCE:
		bufprintprefix();
		bufprintf("IMPORTANCE INFO: %s", (char *)recipe->content);
		break;
	case MACH_VOUCHER_ATTR_KEY_BANK:
		bufprintprefix();
		bufprintf("RESOURCE ACCOUNTING INFO: %s", (char *)recipe->content);
		break;
	default:
		offset = format_hex_data(prefix, "Recipe Contents", recipe->content,
				MIN(recipe->content_size, max_hex_data), buf, bufsiz, offset);
		break;
	}
	if (buf[offset - 1] != '\n') {
		bufprintf("\n");
	}
	return offset;
}

size_t
voucher_kvoucher_debug(mach_port_t task, mach_port_name_t voucher, char *buf,
		size_t bufsiz, size_t offset, char *prefix, size_t max_hex_data)
{
	uint8_t voucher_contents[VOUCHER_CONTENTS_SIZE];
	bzero(voucher_contents, VOUCHER_CONTENTS_SIZE);
	size_t recipe_size = VOUCHER_CONTENTS_SIZE;
	unsigned v_kobject = 0;
	unsigned v_kotype = 0;

	kern_return_t kr = mach_port_kernel_object(task, voucher, &v_kotype,
			&v_kobject);
	if (kr == KERN_SUCCESS && v_kotype == IKOT_VOUCHER) {
		kr = mach_voucher_debug_info(task, voucher,
				(mach_voucher_attr_raw_recipe_array_t)voucher_contents,
				(mach_msg_type_number_t *)&recipe_size);
		if (kr != KERN_SUCCESS && kr != KERN_NOT_SUPPORTED) {
			bufprintprefix();
			bufprintf("Voucher: 0x%x Failed to get contents %s\n", v_kobject,
					mach_error_string(kr));
			goto done;
		}

		if (recipe_size == 0) {
			bufprintprefix();
			bufprintf("Voucher: 0x%x has no contents\n", v_kobject);
			goto done;
		}

		bufprintprefix();
		bufprintf("Voucher: 0x%x\n", v_kobject);
		unsigned int used_size = 0;
		mach_voucher_attr_recipe_t recipe = NULL;
		while (recipe_size > used_size) {
			recipe = (mach_voucher_attr_recipe_t)&voucher_contents[used_size];
			if (recipe->key) {
				offset = format_recipe_detail(recipe, buf, bufsiz, offset,
						prefix, max_hex_data);
			}
			used_size += sizeof(mach_voucher_attr_recipe_data_t)
					+ recipe->content_size;
		}
	} else {
		bufprintprefix();
		bufprintf("Invalid voucher: 0x%x\n", voucher);
   	}

done:
	return offset;
}

#else // VOUCHER_USE_MACH_VOUCHER

#pragma mark -
#pragma mark Simulator / vouchers disabled

#if VOUCHER_ENABLE_RECIPE_OBJECTS
voucher_t
voucher_create(voucher_recipe_t recipe)
{
	(void)recipe;
	return NULL;
}
#endif // VOUCHER_ENABLE_RECIPE_OBJECTS

voucher_t
voucher_adopt(voucher_t voucher)
{
	return voucher;
}

voucher_t
voucher_copy(void)
{
	return NULL;
}

voucher_t
voucher_copy_without_importance(void)
{
	return NULL;
}

voucher_t
voucher_retain(voucher_t voucher)
{
	return voucher;
}

void
voucher_release(voucher_t voucher)
{
	(void)voucher;
}

void
voucher_replace_default_voucher(void)
{
}

void
voucher_decrement_importance_count4CF(voucher_t v)
{
	(void)v;
}

void DISPATCH_TSD_DTOR_CC
_voucher_thread_cleanup(void *voucher)
{
	(void)voucher;
}

void
_voucher_dealloc_mach_voucher(mach_voucher_t kv)
{
	(void)kv;
}

mach_voucher_t
_voucher_get_mach_voucher(voucher_t voucher)
{
	(void)voucher;
	return MACH_VOUCHER_NULL;
}

mach_voucher_t
_voucher_create_mach_voucher_with_priority(voucher_t voucher,
		pthread_priority_t priority)
{
	(void)voucher; (void)priority;
	return MACH_VOUCHER_NULL;
}

voucher_t
_voucher_create_with_priority_and_mach_voucher(voucher_t voucher,
		pthread_priority_t priority, mach_voucher_t kv)
{
	(void)voucher; (void)priority; (void)kv;
	return NULL;
}

voucher_t
_voucher_create_accounting_voucher(voucher_t voucher)
{
	(void)voucher;
	return NULL;
}

#if HAVE_MACH
voucher_t
voucher_create_with_mach_msg(mach_msg_header_t *msg)
{
	(void)msg;
	return NULL;
}
#endif

#if VOUCHER_ENABLE_GET_MACH_VOUCHER
mach_voucher_t
voucher_get_mach_voucher(voucher_t voucher)
{
	(void)voucher;
	return 0;
}
#endif // VOUCHER_ENABLE_GET_MACH_VOUCHER

void
_voucher_xref_dispose(voucher_t voucher)
{
	(void)voucher;
}

void
_voucher_dispose(voucher_t voucher)
{
	(void)voucher;
}

voucher_t
voucher_copy_with_persona_mach_voucher(mach_voucher_t persona_mach_voucher)
{
	(void)persona_mach_voucher;
	return NULL;
}

kern_return_t
mach_voucher_persona_self(mach_voucher_t *persona_mach_voucher)
{
	(void)persona_mach_voucher;
	return KERN_NOT_SUPPORTED;
}

kern_return_t
mach_voucher_persona_for_originator(uid_t persona_id,
	mach_voucher_t originator_persona_mach_voucher,
	uint64_t originator_unique_pid, mach_voucher_t *persona_mach_voucher)
{
	(void)persona_id; (void)originator_persona_mach_voucher;
	(void)originator_unique_pid; (void)persona_mach_voucher;
	return KERN_NOT_SUPPORTED;
}

uid_t
voucher_get_current_persona(void)
{
	return PERSONA_ID_NONE;
}

int
voucher_get_current_persona_originator_info(struct proc_persona_info *persona_info)
{
	(void)persona_info;
	return -1;
}

int
voucher_get_current_persona_proximate_info(struct proc_persona_info *persona_info)
{
	(void)persona_info;
	return -1;
}

void
_voucher_activity_debug_channel_init(void)
{
}

void
_voucher_atfork_child(void)
{
}

void
_voucher_init(void)
{
}

#if OS_VOUCHER_ACTIVITY_SPI
void*
voucher_activity_get_metadata_buffer(size_t *length)
{
    *length = 0;
    return NULL;
}

voucher_t
voucher_activity_create(firehose_tracepoint_id_t trace_id,
		voucher_t base, firehose_activity_flags_t flags, uint64_t location)
{
	(void)trace_id; (void)base; (void)flags; (void)location;
	return NULL;
}

voucher_t
voucher_activity_create_with_location(firehose_tracepoint_id_t *trace_id,
		voucher_t base, firehose_activity_flags_t flags, uint64_t location)
{
	(void)trace_id; (void)base; (void)flags; (void)location;
	return NULL;
}

firehose_activity_id_t
voucher_get_activity_id(voucher_t voucher, firehose_activity_id_t *parent_id)
{
	(void)voucher; (void)parent_id;
	return 0;
}

firehose_activity_id_t
voucher_get_activity_id_and_creator(voucher_t voucher, uint64_t *creator_pid,
		firehose_activity_id_t *parent_id)
{
	if (creator_pid) *creator_pid = 0;
	(void)voucher; (void)parent_id;
	return 0;
}

firehose_tracepoint_id_t
voucher_activity_trace(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t timestamp,
		const void *pubdata, size_t publen)
{
	(void)stream; (void)trace_id; (void)timestamp;
	(void)pubdata; (void)publen;
	return 0;
}

firehose_tracepoint_id_t
voucher_activity_trace_with_private_strings(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t timestamp,
		const void *pubdata, size_t publen,
		const void *privdata, size_t privlen)
{
	(void)stream; (void)trace_id; (void)timestamp;
	(void)pubdata; (void)publen; (void)privdata; (void)privlen;
	return 0;
}

firehose_tracepoint_id_t
voucher_activity_trace_v(firehose_stream_t stream,
		firehose_tracepoint_id_t trace_id, uint64_t timestamp,
		const struct iovec *iov, size_t publen, size_t privlen)
{
	(void)stream; (void)trace_id; (void)timestamp;
	(void)iov; (void)publen; (void)privlen;
	return 0;
}

void
voucher_activity_flush(firehose_stream_t stream)
{
	(void)stream;
}

void
voucher_activity_initialize_4libtrace(voucher_activity_hooks_t hooks)
{
	(void)hooks;
}
#endif // OS_VOUCHER_ACTIVITY_SPI

size_t
_voucher_debug(voucher_t v, char* buf, size_t bufsiz)
{
	(void)v; (void)buf; (void)bufsiz;
	return 0;
}

#endif // VOUCHER_USE_MACH_VOUCHER

#else // DISPATCH_VARIANT_DYLD_STUB

firehose_activity_id_t
voucher_get_activity_id_4dyld(void)
{
#if VOUCHER_USE_MACH_VOUCHER
	return _voucher_get_activity_id(_voucher_get(), NULL);
#else
	return 0;
#endif
}

#endif // DISPATCH_VARIANT_DYLD_STUB
