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

#include "internal.h"

#if VOUCHER_USE_MACH_VOUCHER

#include <mach/mach_voucher.h>

// <rdar://16363550>
#ifndef VM_MEMORY_GENEALOGY
#define VM_MEMORY_GENEALOGY 78
#endif

#ifndef VOUCHER_ATM_COLLECT_THRESHOLD
#define VOUCHER_ATM_COLLECT_THRESHOLD 1
#endif
#define VATM_COLLECT_THRESHOLD_VALUE(t) (((t) - 1) * 2)
static volatile long _voucher_atm_collect_level;
static long _voucher_atm_collect_threshold =
		VATM_COLLECT_THRESHOLD_VALUE(VOUCHER_ATM_COLLECT_THRESHOLD);
static unsigned long _voucher_atm_subid_bits;

typedef struct _voucher_atm_s *_voucher_atm_t;

static void _voucher_activity_atfork_child(void);
static inline mach_voucher_t _voucher_get_atm_mach_voucher(voucher_t voucher);
static inline mach_voucher_t _voucher_activity_get_atm_mach_voucher(
		_voucher_activity_t act);
static inline _voucher_activity_t _voucher_activity_get(voucher_t voucher);
static _voucher_activity_t _voucher_activity_copy_from_mach_voucher(
		mach_voucher_t kv, voucher_activity_id_t va_id);
static inline _voucher_activity_t _voucher_activity_retain(
		_voucher_activity_t act);
static inline void _voucher_activity_release(_voucher_activity_t act);

#pragma mark -
#pragma mark voucher_t

#if USE_OBJC
OS_OBJECT_OBJC_CLASS_DECL(voucher);
#define VOUCHER_CLASS OS_OBJECT_OBJC_CLASS(voucher)
#else
const _os_object_class_s _voucher_class = {
	._os_obj_xref_dispose = (void(*)(_os_object_t))_voucher_xref_dispose,
	._os_obj_dispose = (void(*)(_os_object_t))_voucher_dispose,
};
#define VOUCHER_CLASS &_voucher_class
#endif // USE_OBJC

static const voucher_activity_trace_id_t _voucher_activity_trace_id_release =
		(voucher_activity_trace_id_t)voucher_activity_tracepoint_type_release <<
		_voucher_activity_trace_id_type_shift;
static const unsigned int _voucher_max_activities = 16;

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_recipes_init(mach_voucher_attr_recipe_data_t *recipes,
		mach_voucher_attr_content_size_t bits_size)
{
	static const mach_voucher_attr_recipe_data_t base_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_ALL,
		.command = MACH_VOUCHER_ATTR_COPY,
	};
	_voucher_recipes_base(recipes) = base_recipe;
	static const mach_voucher_attr_recipe_data_t atm_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_ATM,
		.command = MACH_VOUCHER_ATTR_COPY,
	};
	_voucher_recipes_atm(recipes) = atm_recipe;
	static const mach_voucher_attr_recipe_data_t bits_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_USER_DATA,
		.command = MACH_VOUCHER_ATTR_USER_DATA_STORE,
	};
	_voucher_recipes_bits(recipes) = bits_recipe;
	if (!bits_size) return;
	_voucher_recipes_bits(recipes).content_size = bits_size;
	*_voucher_recipes_magic(recipes) = _voucher_magic_v1;
}

static inline voucher_t
_voucher_alloc(unsigned int activities, pthread_priority_t priority,
		mach_voucher_attr_recipe_size_t extra)
{
	if (activities > _voucher_max_activities) {
		activities = _voucher_max_activities;
	}
	voucher_t voucher;
	size_t voucher_size, recipes_size;
	mach_voucher_attr_content_size_t bits_size;
	recipes_size = (priority||activities||extra) ? _voucher_recipes_size() : 0;
	bits_size = recipes_size ? _voucher_bits_size(activities) : 0;
	voucher_size = sizeof(voucher_s) + recipes_size + bits_size + extra;
	voucher = (voucher_t)_os_object_alloc_realized(VOUCHER_CLASS, voucher_size);
#if VOUCHER_ENABLE_RECIPE_OBJECTS
	voucher->v_recipe_extra_size = extra;
	voucher->v_recipe_extra_offset = voucher_size - extra;
#else
	dispatch_assert(!extra);
#endif
	voucher->v_has_priority = priority ? 1 : 0;
	voucher->v_activities = activities;
	if (!recipes_size) return voucher;
	_voucher_recipes_init(voucher->v_recipes, bits_size);
	*_voucher_priority(voucher) = (_voucher_priority_t)priority;
	_dispatch_voucher_debug("alloc", voucher);
	return voucher;
}

#if VOUCHER_ENABLE_RECIPE_OBJECTS
voucher_t
voucher_create(voucher_recipe_t recipe)
{
	// TODO: capture current activities or current kvoucher ?
	mach_voucher_attr_recipe_size_t extra = recipe ? recipe->vr_size : 0;
	voucher_t voucher = _voucher_alloc(0, 0, extra);
	if (extra) {
		memcpy(_voucher_extra_recipes(voucher), recipe->vr_data, extra);
	}
	return voucher;
}
#endif

voucher_t
voucher_adopt(voucher_t voucher)
{
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

void
_voucher_thread_cleanup(void *voucher)
{
	_voucher_swap(voucher, NULL);
}

DISPATCH_CACHELINE_ALIGN
static TAILQ_HEAD(, voucher_s) _vouchers[VL_HASH_SIZE];
#define _vouchers(kv) (&_vouchers[VL_HASH((kv))])
static os_lock_handoff_s _vouchers_lock = OS_LOCK_HANDOFF_INIT;
#define _vouchers_lock_lock() os_lock_lock(&_vouchers_lock)
#define _vouchers_lock_unlock() os_lock_unlock(&_vouchers_lock)

static voucher_t
_voucher_find_and_retain(mach_voucher_t kv)
{
	voucher_t v;
	if (!kv) return NULL;
	_vouchers_lock_lock();
	TAILQ_FOREACH(v, _vouchers(kv), v_list) {
		if (v->v_ipc_kvoucher == kv) {
			int xref_cnt = dispatch_atomic_inc2o(v, os_obj_xref_cnt, relaxed);
			_dispatch_voucher_debug("retain  -> %d", v, xref_cnt + 1);
			if (slowpath(xref_cnt < 0)) {
				_dispatch_voucher_debug("overrelease", v);
				DISPATCH_CRASH("Voucher overrelease");
			}
			if (xref_cnt == 0) {
				// resurrection: raced with _voucher_remove
				(void)dispatch_atomic_inc2o(v, os_obj_ref_cnt, relaxed);
			}
			break;
		}
	}
	_vouchers_lock_unlock();
	return v;
}

static void
_voucher_insert(voucher_t v)
{
	mach_voucher_t kv = v->v_ipc_kvoucher;
	if (!kv) return;
	_vouchers_lock_lock();
	if (slowpath(_TAILQ_IS_ENQUEUED(v, v_list))) {
		_dispatch_voucher_debug("corruption", v);
		DISPATCH_CRASH("Voucher corruption");
	}
	TAILQ_INSERT_TAIL(_vouchers(kv), v, v_list);
	_vouchers_lock_unlock();
}

static void
_voucher_remove(voucher_t v)
{
	mach_voucher_t kv = v->v_ipc_kvoucher;
	if (!_TAILQ_IS_ENQUEUED(v, v_list)) return;
	_vouchers_lock_lock();
	if (slowpath(!kv)) {
		_dispatch_voucher_debug("corruption", v);
		DISPATCH_CRASH("Voucher corruption");
	}
	// check for resurrection race with _voucher_find_and_retain
	if (dispatch_atomic_load2o(v, os_obj_xref_cnt, seq_cst) < 0 &&
			_TAILQ_IS_ENQUEUED(v, v_list)) {
		TAILQ_REMOVE(_vouchers(kv), v, v_list);
		_TAILQ_MARK_NOT_ENQUEUED(v, v_list);
		v->v_list.tqe_next = (void*)~0ull;
	}
	_vouchers_lock_unlock();
}

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

#if __has_include(<bank/bank_types.h>) && !defined(VOUCHER_USE_ATTR_BANK)
#include <bank/bank_types.h>
#define VOUCHER_USE_ATTR_BANK 1
mach_voucher_t _voucher_default_task_mach_voucher;
#endif

void
_voucher_task_mach_voucher_init(void* ctxt DISPATCH_UNUSED)
{
#if VOUCHER_USE_ATTR_BANK
	kern_return_t kr;
	mach_voucher_t kv;
	static const mach_voucher_attr_recipe_data_t task_create_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_BANK,
		.command = MACH_VOUCHER_ATTR_BANK_CREATE,
	};
	kr = _voucher_create_mach_voucher(&task_create_recipe,
			sizeof(task_create_recipe), &kv);
	if (dispatch_assume_zero(kr)) {
		DISPATCH_CLIENT_CRASH("Could not create task mach voucher");
	}
	_voucher_default_task_mach_voucher = kv;
	_voucher_task_mach_voucher = kv;
#endif
}

void
voucher_replace_default_voucher(void)
{
#if VOUCHER_USE_ATTR_BANK
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
	kv = dispatch_atomic_xchg(&_voucher_task_mach_voucher, tkv, relaxed);
	if (kv && kv != _voucher_default_task_mach_voucher) {
		_voucher_dealloc_mach_voucher(kv);
	}
	_dispatch_voucher_debug("kvoucher[0x%08x] replace default voucher", v, tkv);
#endif
}

static inline mach_voucher_t
_voucher_get_atm_mach_voucher(voucher_t voucher)
{
	_voucher_activity_t act = _voucher_activity_get(voucher);
	return _voucher_activity_get_atm_mach_voucher(act);
}

mach_voucher_t
_voucher_get_mach_voucher(voucher_t voucher)
{
	if (!voucher) return MACH_VOUCHER_NULL;
	if (voucher->v_ipc_kvoucher) return voucher->v_ipc_kvoucher;
	mach_voucher_t kvb = voucher->v_kvoucher;
	if (!kvb) kvb = _voucher_get_task_mach_voucher();
	if (!voucher->v_has_priority && !voucher->v_activities &&
			!_voucher_extra_size(voucher)) {
		return kvb;
	}
	kern_return_t kr;
	mach_voucher_t kv, kvo;
	_voucher_base_recipe(voucher).previous_voucher = kvb;
	_voucher_atm_recipe(voucher).previous_voucher =
			_voucher_get_atm_mach_voucher(voucher);
	kr = _voucher_create_mach_voucher(voucher->v_recipes,
			_voucher_recipes_size() + _voucher_extra_size(voucher) +
			_voucher_bits_recipe(voucher).content_size, &kv);
	if (dispatch_assume_zero(kr) || !kv){
		return MACH_VOUCHER_NULL;
	}
	if (!dispatch_atomic_cmpxchgv2o(voucher, v_ipc_kvoucher, MACH_VOUCHER_NULL,
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
	mach_voucher_attr_recipe_data_t *recipes;
	size_t recipes_size = _voucher_recipes_size();
	if (voucher && (voucher->v_has_priority || voucher->v_activities ||
			_voucher_extra_size(voucher))) {
		recipes_size += _voucher_bits_recipe(voucher).content_size +
				_voucher_extra_size(voucher);
		recipes = alloca(recipes_size);
		memcpy(recipes, voucher->v_recipes, recipes_size);
		_voucher_recipes_atm(recipes).previous_voucher =
				_voucher_get_atm_mach_voucher(voucher);
	} else {
		mach_voucher_attr_content_size_t bits_size = _voucher_bits_size(0);
		recipes_size += bits_size;
		recipes = alloca(recipes_size);
		_voucher_recipes_init(recipes, bits_size);
	}
	_voucher_recipes_base(recipes).previous_voucher = kvb;
	*_voucher_recipes_priority(recipes) = (_voucher_priority_t)priority;
	kr = _voucher_create_mach_voucher(recipes, recipes_size, &kv);
	if (dispatch_assume_zero(kr) || !kv){
		return MACH_VOUCHER_NULL;
	}
	_dispatch_kvoucher_debug("create with priority from voucher[%p]", kv,
			voucher);
	return kv;
}

static voucher_t
_voucher_create_with_mach_voucher(mach_voucher_t kv)
{
	if (!kv) return NULL;
	kern_return_t kr;
	mach_voucher_t rkv;
	mach_voucher_attr_recipe_t vr;
	size_t vr_size;
	mach_voucher_attr_recipe_size_t kvr_size = 0;
	const mach_voucher_attr_recipe_data_t redeem_recipe[] = {
		[0] = {
			.key = MACH_VOUCHER_ATTR_KEY_ALL,
			.command = MACH_VOUCHER_ATTR_COPY,
			.previous_voucher = kv,
		},
#if VOUCHER_USE_ATTR_BANK
		[1] = {
			.key = MACH_VOUCHER_ATTR_KEY_BANK,
			.command = MACH_VOUCHER_ATTR_REDEEM,
		},
#endif
	};
	kr = _voucher_create_mach_voucher(redeem_recipe, sizeof(redeem_recipe),
			&rkv);
	if (!dispatch_assume_zero(kr)) {
		_voucher_dealloc_mach_voucher(kv);
	} else {
		_dispatch_voucher_debug_machport(kv);
		rkv = kv;
	}
	voucher_t v = _voucher_find_and_retain(rkv);
	if (v) {
		_dispatch_voucher_debug("kvoucher[0x%08x] find with 0x%08x", v, rkv,kv);
		_voucher_dealloc_mach_voucher(rkv);
		return v;
	}
	vr_size = sizeof(*vr) + _voucher_bits_size(_voucher_max_activities);
	vr = alloca(vr_size);
	if (rkv) {
		kvr_size = (mach_voucher_attr_recipe_size_t)vr_size;
		kr = mach_voucher_extract_attr_recipe(rkv,
				MACH_VOUCHER_ATTR_KEY_USER_DATA, (void*)vr, &kvr_size);
		DISPATCH_VERIFY_MIG(kr);
		if (dispatch_assume_zero(kr)) kvr_size = 0;
	}
	mach_voucher_attr_content_size_t content_size = vr->content_size;
	uint8_t *content = vr->content;
	bool valid = false, has_priority = false;
	unsigned int activities = 0;
	if (kvr_size >= sizeof(*vr) + sizeof(_voucher_magic_t)) {
		valid = (*(_voucher_magic_t*)content == _voucher_magic_v1);
		content += sizeof(_voucher_magic_t);
		content_size -= sizeof(_voucher_magic_t);
	}
	if (valid) {
		has_priority = (content_size >= sizeof(_voucher_priority_t));
		activities = has_priority ? (content_size - sizeof(_voucher_priority_t))
				/ sizeof(voucher_activity_id_t) : 0;
	}
	pthread_priority_t priority = 0;
	if (has_priority) {
		priority = (pthread_priority_t)*(_voucher_priority_t*)content;
		content += sizeof(_voucher_priority_t);
		content_size -= sizeof(_voucher_priority_t);
	}
	voucher_activity_id_t va_id = 0, va_base_id = 0;
	_voucher_activity_t act = NULL;
	if (activities) {
		va_id = *(voucher_activity_id_t*)content;
		act = _voucher_activity_copy_from_mach_voucher(rkv, va_id);
		if (!act && _voucher_activity_default) {
			activities++;
			// default to _voucher_activity_default base activity
			va_base_id = _voucher_activity_default->va_id;
		} else if (act && act->va_id != va_id) {
			activities++;
			va_base_id = act->va_id;
		}
	}
	v = _voucher_alloc(activities, priority, 0);
	v->v_activity = act;
	voucher_activity_id_t *activity_ids = _voucher_activity_ids(v);
	if (activities && va_base_id) {
		*activity_ids++ = va_base_id;
		activities--;
	}
	if (activities) {
		memcpy(activity_ids, content, content_size);
	}
	v->v_ipc_kvoucher = v->v_kvoucher = rkv;
	_voucher_insert(v);
	_dispatch_voucher_debug("kvoucher[0x%08x] create with 0x%08x", v, rkv, kv);
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
	if (v) {
		_dispatch_voucher_debug("kvoucher[0x%08x] find", v, kv);
		_voucher_dealloc_mach_voucher(kv);
		return v;
	}
	unsigned int activities = ov ? ov->v_activities : 0;
	mach_voucher_attr_recipe_size_t extra = ov ? _voucher_extra_size(ov) : 0;
	v = _voucher_alloc(activities, priority, extra);
	if (extra) {
		memcpy(_voucher_extra_recipes(v), _voucher_extra_recipes(ov), extra);
	}
	if (activities) {
		if (ov->v_activity) {
			v->v_activity = _voucher_activity_retain(ov->v_activity);
		}
		memcpy(_voucher_activity_ids(v), _voucher_activity_ids(ov),
				activities * sizeof(voucher_activity_id_t));
	}
	if (kv) {
		v->v_ipc_kvoucher = v->v_kvoucher = kv;
		_voucher_insert(v);
		_dispatch_voucher_debug("kvoucher[0x%08x] create with priority from "
				"voucher[%p]", v, kv, ov);
		_dispatch_voucher_debug_machport(kv);
	} else if (ov && ov->v_kvoucher) {
		voucher_t kvb = ov->v_kvbase ? ov->v_kvbase : ov;
		v->v_kvbase = _voucher_retain(kvb);
		v->v_kvoucher = kvb->v_kvoucher;
	}
	return v;
}

voucher_t
_voucher_create_without_importance(voucher_t ov)
{
	// Nothing to do unless the old voucher has a kernel voucher. If it
	// doesn't, it can't have any importance, now or in the future.
	if (!ov) return NULL;
	// TODO: 17487167: track presence of importance attribute
	if (!ov->v_kvoucher) return _voucher_retain(ov);
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
	if (dispatch_assume_zero(kr) || !kv){
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
	// Copy userspace contents
	unsigned int activities = ov->v_activities;
	pthread_priority_t priority = _voucher_get_priority(ov);
	mach_voucher_attr_recipe_size_t extra = _voucher_extra_size(ov);
	v = _voucher_alloc(activities, priority, extra);
	if (extra) {
		memcpy(_voucher_extra_recipes(v), _voucher_extra_recipes(ov), extra);
	}
	if (activities) {
		if (ov->v_activity) {
			v->v_activity = _voucher_activity_retain(ov->v_activity);
		}
		memcpy(_voucher_activity_ids(v), _voucher_activity_ids(ov),
				activities * sizeof(voucher_activity_id_t));
	}
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
	return v;
}

voucher_t
voucher_create_with_mach_msg(mach_msg_header_t *msg)
{
	voucher_t v = _voucher_create_with_mach_voucher(_voucher_mach_msg_get(msg));
	_voucher_activity_trace_msg(v, msg, receive);
	return v;
}

#ifndef MACH_VOUCHER_IMPORTANCE_ATTR_DROP_EXTERNAL
#define MACH_VOUCHER_IMPORTANCE_ATTR_DROP_EXTERNAL 2
#endif

void
voucher_decrement_importance_count4CF(voucher_t v)
{
	if (!v || !v->v_kvoucher) return;
	// TODO: 17487167: track presence of importance attribute
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
#if DISPATCH_DEBUG
	_dispatch_voucher_debug("kvoucher[0x%08x] decrement importance count to %u:"
			" %s - 0x%x", v, kv, count, mach_error_string(kr), kr);
#endif
	if (kr != KERN_INVALID_ARGUMENT &&
			dispatch_assume_zero(kr) == KERN_FAILURE) {
		// TODO: 17487167: skip KERN_INVALID_ARGUMENT check
		DISPATCH_CLIENT_CRASH("Voucher importance count underflow");
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
	return _os_object_release_internal_inline((_os_object_t)voucher);
}

void
_voucher_dispose(voucher_t voucher)
{
	_dispatch_voucher_debug("dispose", voucher);
	if (slowpath(_TAILQ_IS_ENQUEUED(voucher, v_list))) {
		_dispatch_voucher_debug("corruption", voucher);
		DISPATCH_CRASH("Voucher corruption");
	}
	voucher->v_list.tqe_next = DISPATCH_OBJECT_LISTLESS;
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
	if (voucher->v_activity) {
		_voucher_activity_release(voucher->v_activity);
		voucher->v_activity = NULL;
	}
	voucher->v_has_priority= 0;
	voucher->v_activities = 0;
#if VOUCHER_ENABLE_RECIPE_OBJECTS
	voucher->v_recipe_extra_size = 0;
	voucher->v_recipe_extra_offset = 0;
#endif
	return _os_object_dealloc((_os_object_t)voucher);
}

void
_voucher_atfork_child(void)
{
	_voucher_activity_atfork_child();
	_dispatch_thread_setspecific(dispatch_voucher_key, NULL);
	_voucher_task_mach_voucher_pred = 0;
	_voucher_task_mach_voucher = MACH_VOUCHER_NULL;

	// TODO: voucher/activity inheritance on fork ?
}

#pragma mark -
#pragma mark _voucher_init

boolean_t
voucher_mach_msg_set(mach_msg_header_t *msg)
{
	voucher_t v = _voucher_get();
	bool clear_voucher = _voucher_mach_msg_set(msg, v);
	if (clear_voucher) _voucher_activity_trace_msg(v, msg, send);
	return clear_voucher;
}

void
voucher_mach_msg_clear(mach_msg_header_t *msg)
{
	(void)_voucher_mach_msg_clear(msg, false);
}

voucher_mach_msg_state_t
voucher_mach_msg_adopt(mach_msg_header_t *msg)
{
	mach_voucher_t kv = _voucher_mach_msg_get(msg);
	if (!kv) return VOUCHER_MACH_MSG_STATE_UNCHANGED;
	voucher_t v = _voucher_create_with_mach_voucher(kv);
	_voucher_activity_trace_msg(v, msg, receive);
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
_voucher_init(void)
{
	_voucher_libkernel_init();
	char *e, *end;
	unsigned int i;
	for (i = 0; i < VL_HASH_SIZE; i++) {
		TAILQ_INIT(&_vouchers[i]);
	}
	voucher_activity_mode_t mode;
	mode = DISPATCH_DEBUG ? voucher_activity_mode_debug
			: voucher_activity_mode_release;
	e = getenv("OS_ACTIVITY_MODE");
	if (e) {
		if (strcmp(e, "release") == 0) {
			mode = voucher_activity_mode_release;
		} else if (strcmp(e, "debug") == 0) {
			mode = voucher_activity_mode_debug;
		} else if (strcmp(e, "stream") == 0) {
			mode = voucher_activity_mode_stream;
		} else if (strcmp(e, "disable") == 0) {
			mode = voucher_activity_mode_disable;
		}
	}
	_voucher_activity_mode = mode;
	if (_voucher_activity_disabled()) return;

	e = getenv("LIBDISPATCH_ACTIVITY_ATM_SUBID_BITS");
	if (e) {
		unsigned long v = strtoul(e, &end, 0);
		if (v && !*end) {
			_voucher_atm_subid_bits = v;
		}
	}
	e = getenv("LIBDISPATCH_ACTIVITY_ATM_COLLECT_THRESHOLD");
	if (e) {
		unsigned long v = strtoul(e, &end, 0);
		if (v && v < LONG_MAX/2 && !*end) {
			_voucher_atm_collect_threshold =
					VATM_COLLECT_THRESHOLD_VALUE((long)v);
		}
	}
	// default task activity
	bool default_task_activity = DISPATCH_DEBUG;
	e = getenv("LIBDISPATCH_DEFAULT_TASK_ACTIVITY");
	if (e) default_task_activity = atoi(e);
	if (default_task_activity) {
		(void)voucher_activity_start(_voucher_activity_trace_id_release, 0);
	}
}

#pragma mark -
#pragma mark _voucher_activity_lock_s

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_lock_init(_voucher_activity_lock_s *lock) {
    static const os_lock_handoff_s _os_lock_handoff_init = OS_LOCK_HANDOFF_INIT;
    *lock = _os_lock_handoff_init;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_lock_lock(_voucher_activity_lock_s *lock) {
    return os_lock_lock(lock);
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_lock_unlock(_voucher_activity_lock_s *lock) {
    return os_lock_unlock(lock);
}

#pragma mark -
#pragma mark _voucher_activity_heap

#if __has_extension(c_static_assert)
_Static_assert(sizeof(struct _voucher_activity_tracepoint_s) == 64,
		"Tracepoint too large");
_Static_assert(sizeof(struct _voucher_activity_buffer_header_s) <=
		sizeof(struct _voucher_activity_tracepoint_s),
		"Buffer header too large");
_Static_assert(offsetof(struct _voucher_activity_s, va_flags2) ==
		sizeof(struct _voucher_activity_tracepoint_s),
		"Extended activity object misaligned");
#if __LP64__
_Static_assert(sizeof(struct _voucher_activity_s) ==
		3 * sizeof(struct _voucher_activity_tracepoint_s),
		"Activity object too large");
_Static_assert(offsetof(struct _voucher_activity_s, va_flags3) ==
		2 * sizeof(struct _voucher_activity_tracepoint_s),
		"Extended activity object misaligned");
_Static_assert(offsetof(struct _voucher_atm_s, vatm_activities_lock) % 64 == 0,
		"Bad ATM padding");
_Static_assert(sizeof(struct _voucher_atm_s) <= 128,
		"ATM too large");
#else
_Static_assert(sizeof(struct _voucher_activity_s) ==
		2 * sizeof(struct _voucher_activity_tracepoint_s),
		"Activity object too large");
_Static_assert(sizeof(struct _voucher_atm_s) <= 64,
		"ATM too large");
#endif
_Static_assert(sizeof(_voucher_activity_buffer_t) ==
		sizeof(struct {char x[_voucher_activity_buffer_size];}),
		"Buffer too large");
_Static_assert(sizeof(struct _voucher_activity_metadata_s) <=
		sizeof(struct _voucher_activity_metadata_opaque_s),
		"Metadata too large");
_Static_assert(sizeof(_voucher_activity_bitmap_t) % 64 == 0,
		"Bad metadata bitmap size");
_Static_assert(offsetof(struct _voucher_activity_metadata_s,
		vam_atm_mbox_bitmap) % 64 == 0,
		"Bad metadata padding");
_Static_assert(offsetof(struct _voucher_activity_metadata_s,
		vam_base_atm_subid) % 64 == 0,
		"Bad metadata padding");
_Static_assert(offsetof(struct _voucher_activity_metadata_s, vam_base_atm_lock)
		% 32 == 0,
		"Bad metadata padding");
_Static_assert(offsetof(struct _voucher_activity_metadata_s, vam_atms) % 64 ==0,
		"Bad metadata padding");
_Static_assert(sizeof(_voucher_activity_bitmap_t) * 8 *
		sizeof(atm_mailbox_offset_t) <=
		sizeof(((_voucher_activity_metadata_t)NULL)->vam_kernel_metadata),
		"Bad kernel metadata bitmap");
_Static_assert(sizeof(atm_mailbox_offset_t) == 2 * sizeof(atm_subaid32_t),
		"Bad kernel ATM mailbox sizes");
#endif

static const size_t _voucher_atm_mailboxes =
		sizeof(((_voucher_activity_metadata_t)NULL)->vam_kernel_metadata) /
		sizeof(atm_mailbox_offset_t);

#define va_buffers_lock(va) (&(va)->va_buffers_lock)
#define vatm_activities_lock(vatm) (&(vatm)->vatm_activities_lock)
#define vatm_activities(vatm) (&(vatm)->vatm_activities)
#define vatm_used_activities(vatm) (&(vatm)->vatm_used_activities)
#define vam_base_atm_lock() (&_voucher_activity_heap->vam_base_atm_lock)
#define vam_nested_atm_lock() (&_voucher_activity_heap->vam_nested_atm_lock)
#define vam_atms_lock() (&_voucher_activity_heap->vam_atms_lock)
#define vam_activities_lock() (&_voucher_activity_heap->vam_activities_lock)
#define vam_atms(hash) (&_voucher_activity_heap->vam_atms[hash])
#define vam_activities(hash) (&_voucher_activity_heap->vam_activities[hash])
#define vam_buffer_bitmap() (_voucher_activity_heap->vam_buffer_bitmap)
#define vam_atm_mbox_bitmap() (_voucher_activity_heap->vam_atm_mbox_bitmap)
#define vam_pressure_locked_bitmap() \
		(_voucher_activity_heap->vam_pressure_locked_bitmap)
#define vam_buffer(i) ((void*)((char*)_voucher_activity_heap + \
		(i) * _voucher_activity_buffer_size))

static _voucher_activity_t _voucher_activity_create_with_atm(
		_voucher_atm_t vatm, voucher_activity_id_t va_id,
		voucher_activity_trace_id_t trace_id, uint64_t location,
		_voucher_activity_buffer_header_t buffer);
static _voucher_atm_t _voucher_atm_create(mach_voucher_t kv, atm_aid_t atm_id);
static voucher_activity_id_t _voucher_atm_nested_atm_id_make(void);

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_voucher_default_activity_buffer_limit()
{
	switch (_voucher_activity_mode) {
	case voucher_activity_mode_debug:
	case voucher_activity_mode_stream:
		// High-profile modes: Default activity can use 1/32nd of the heap
		// (twice as much as non-default activities)
		return MAX(_voucher_activity_buffers_per_heap / 32, 3) - 1;
	}
#if TARGET_OS_EMBEDDED
	// Low-profile modes: Default activity can use a total of 3 buffers.
	return 2;
#else
	// Low-profile modes: Default activity can use a total of 8 buffers.
	return 7;
#endif
}

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_voucher_activity_buffer_limit()
{
	switch (_voucher_activity_mode) {
	case voucher_activity_mode_debug:
	case voucher_activity_mode_stream:
		// High-profile modes: 64 activities, each of which can use 1/64th
		// of the entire heap.
		return MAX(_voucher_activity_buffers_per_heap / 64, 2) - 1;
	}
#if TARGET_OS_EMBEDDED
	// Low-profile modes: Each activity can use a total of 2 buffers.
	return 1;
#else
	// Low-profile modes: Each activity can use a total of 4 buffers.
	return 3;
#endif
}

// The two functions above return the number of *additional* buffers activities
// may allocate, hence the gymnastics with - 1.

DISPATCH_ALWAYS_INLINE
static inline uint32_t
_voucher_heap_buffer_limit()
{
	switch (_voucher_activity_mode) {
	case voucher_activity_mode_debug:
	case voucher_activity_mode_stream:
		// High-profile modes: Use it all.
		return _voucher_activity_buffers_per_heap;
	}
#if TARGET_OS_EMBEDDED
	// Low-profile modes: 3 activities, each of which can use 2 buffers;
	// plus the default activity, which can use 3; plus 3 buffers of overhead.
	return 12;
#else
	// Low-profile modes: 13 activities, each of which can use 4 buffers;
	// plus the default activity, which can use 8; plus 3 buffers of overhead.
	return 64;
#endif
}

#define NO_BITS_WERE_UNSET (UINT_MAX)

DISPATCH_ALWAYS_INLINE
static inline size_t
_voucher_activity_bitmap_set_first_unset_bit_upto(
		_voucher_activity_bitmap_t volatile bitmap,
		unsigned int max_index)
{
	dispatch_assert(max_index != 0);
	unsigned int index = NO_BITS_WERE_UNSET, max_map, max_bit, i;
	max_map = max_index / _voucher_activity_bits_per_bitmap_base_t;
	max_map = MIN(max_map, _voucher_activity_bitmaps_per_heap - 1);
	max_bit = max_index % _voucher_activity_bits_per_bitmap_base_t;
	for (i = 0; i < max_map; i++) {
		index = dispatch_atomic_set_first_bit(&bitmap[i], UINT_MAX);
		if (fastpath(index < NO_BITS_WERE_UNSET)) {
			return index + i * _voucher_activity_bits_per_bitmap_base_t;
		}
	}
	index = dispatch_atomic_set_first_bit(&bitmap[i], max_bit);
	if (fastpath(index < NO_BITS_WERE_UNSET)) {
		return index + i * _voucher_activity_bits_per_bitmap_base_t;
	}
	return index;
}

DISPATCH_ALWAYS_INLINE
static inline size_t
_voucher_activity_bitmap_set_first_unset_bit(
		_voucher_activity_bitmap_t volatile bitmap)
{
	return _voucher_activity_bitmap_set_first_unset_bit_upto(bitmap, UINT_MAX);
}


DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_bitmap_clear_bit(
		_voucher_activity_bitmap_t volatile bitmap, size_t index)
{
	size_t i = index / _voucher_activity_bits_per_bitmap_base_t;
	_voucher_activity_bitmap_base_t mask = ((typeof(mask))1) <<
			(index % _voucher_activity_bits_per_bitmap_base_t);
	if (slowpath((bitmap[i] & mask) == 0)) {
		DISPATCH_CRASH("Corruption: failed to clear bit exclusively");
	}
	(void)dispatch_atomic_and(&bitmap[i], ~mask, release);
}

_voucher_activity_metadata_t _voucher_activity_heap;
static dispatch_once_t _voucher_activity_heap_pred;

static void
_voucher_activity_heap_init(void *ctxt DISPATCH_UNUSED)
{
	if (_voucher_activity_disabled()) return;
	kern_return_t kr;
	mach_vm_size_t vm_size = _voucher_activity_buffer_size *
			_voucher_activity_buffers_per_heap;
	mach_vm_address_t vm_addr = vm_page_size;
	while (slowpath(kr = mach_vm_map(mach_task_self(), &vm_addr, vm_size,
			0, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_GENEALOGY),
			MEMORY_OBJECT_NULL, 0, FALSE, VM_PROT_DEFAULT, VM_PROT_ALL,
			VM_INHERIT_NONE))) {
		if (kr != KERN_NO_SPACE) {
			(void)dispatch_assume_zero(kr);
			_voucher_activity_mode = voucher_activity_mode_disable;
			return;
		}
		_dispatch_temporary_resource_shortage();
		vm_addr = vm_page_size;
	}
	_voucher_activity_metadata_t heap;
	task_trace_memory_info_data_t trace_memory_info = {
		.user_memory_address = vm_addr,
		.buffer_size = vm_size,
		.mailbox_array_size = sizeof(heap->vam_kernel_metadata),
	};
	kr = task_set_info(mach_task_self(), TASK_TRACE_MEMORY_INFO,
			(task_info_t)&trace_memory_info, TASK_TRACE_MEMORY_INFO_COUNT);
	DISPATCH_VERIFY_MIG(kr);
	if (kr) {
		if (kr != KERN_NOT_SUPPORTED) (void)dispatch_assume_zero(kr);
		kr = mach_vm_deallocate(mach_task_self(), vm_addr, vm_size);
		(void)dispatch_assume_zero(kr);
		_voucher_activity_mode = voucher_activity_mode_disable;
		return;
	}
	heap = (void*)vm_addr;
	heap->vam_self_metadata.vasm_baseaddr = (void*)vm_addr;
	heap->vam_buffer_bitmap[0] = 0xf; // first four buffers are reserved
	uint32_t i;
	for (i = 0; i < _voucher_activity_hash_size; i++) {
		TAILQ_INIT(&heap->vam_activities[i]);
		TAILQ_INIT(&heap->vam_atms[i]);
	}
	uint32_t subid_max = VATM_SUBID_MAX;
	if (_voucher_atm_subid_bits &&
			_voucher_atm_subid_bits < VATM_SUBID_MAXBITS) {
		subid_max = MIN(VATM_SUBID_BITS2MAX(_voucher_atm_subid_bits),
				VATM_SUBID_MAX);
	}
	heap->vam_base_atm_subid_max = subid_max;
	_voucher_activity_lock_init(&heap->vam_base_atm_lock);
	_voucher_activity_lock_init(&heap->vam_nested_atm_lock);
	_voucher_activity_lock_init(&heap->vam_atms_lock);
	_voucher_activity_lock_init(&heap->vam_activities_lock);
	_voucher_activity_heap = heap;

	_voucher_atm_t vatm = _voucher_atm_create(0, 0);
	dispatch_assert(vatm->vatm_kvoucher);
	heap->vam_default_activity_atm = vatm;
	_voucher_activity_buffer_header_t buffer = vam_buffer(3); // reserved index
	// consumes vatm reference:
	_voucher_activity_t va = _voucher_activity_create_with_atm(vatm,
			VATM_ACTID(vatm, _voucher_default_activity_subid), 0, 0, buffer);
	dispatch_assert(va);
	va->va_buffer_limit = _voucher_default_activity_buffer_limit();
	_voucher_activity_default = va;
	heap->vam_base_atm = _voucher_atm_create(0, 0);
	heap->vam_nested_atm_id = _voucher_atm_nested_atm_id_make();
}

static void
_voucher_activity_atfork_child(void)
{
	_voucher_activity_heap_pred = 0;
	_voucher_activity_heap = NULL; // activity heap is VM_INHERIT_NONE
	_voucher_activity_default = NULL;
}

void*
voucher_activity_get_metadata_buffer(size_t *length)
{
	dispatch_once_f(&_voucher_activity_heap_pred, NULL,
			_voucher_activity_heap_init);
	if (_voucher_activity_disabled()) {
		*length = 0;
		return NULL;
	}
	*length = sizeof(_voucher_activity_heap->vam_client_metadata);
	return _voucher_activity_heap->vam_client_metadata;
}

DISPATCH_ALWAYS_INLINE
static inline _voucher_activity_buffer_header_t
_voucher_activity_heap_buffer_alloc(void)
{
	_voucher_activity_buffer_header_t buffer = NULL;
	size_t index;
	index = _voucher_activity_bitmap_set_first_unset_bit_upto(
			vam_buffer_bitmap(), _voucher_heap_buffer_limit() - 1);
	if (index < NO_BITS_WERE_UNSET) {
		buffer = vam_buffer(index);
	}
#if DISPATCH_DEBUG && DISPATCH_VOUCHER_ACTIVITY_DEBUG
	_dispatch_debug("activity heap alloc %zd (%p)", index, buffer);
#endif
	return buffer;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_heap_buffer_free(_voucher_activity_buffer_header_t buffer)
{
	buffer->vabh_flags = _voucher_activity_trace_flag_buffer_empty;
	size_t index = (size_t)((char*)buffer - (char*)_voucher_activity_heap) /
			_voucher_activity_buffer_size;
#if DISPATCH_DEBUG && DISPATCH_VOUCHER_ACTIVITY_DEBUG
	_dispatch_debug("activity heap free %zd (%p)", index, buffer);
#endif
	_voucher_activity_bitmap_clear_bit(vam_buffer_bitmap(), index);
}

#define _voucher_activity_heap_can_madvise() \
		(PAGE_SIZE == _voucher_activity_buffer_size) // <rdar://17445544>

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_heap_madvise(size_t bitmap_num, unsigned int start,
		unsigned int len)
{
	size_t base = bitmap_num * _voucher_activity_bits_per_bitmap_base_t;
#if DISPATCH_DEBUG
#if DISPATCH_VOUCHER_ACTIVITY_DEBUG
	_dispatch_debug("activity heap madvise %zd (%p) -> %zd (%p)", base + start,
			vam_buffer(base + start), base + start + len,
			vam_buffer(base + start + len));
#endif
	dispatch_assert(!(len * _voucher_activity_buffer_size % vm_page_size));
	const uint64_t pattern = 0xFACEFACEFACEFACE;
	_voucher_activity_buffer_header_t buffer = vam_buffer(base + start);
	for (unsigned int i = 0; i < len; i++, buffer++) {
		memset_pattern8((char*)buffer + sizeof(buffer->vabh_flags), &pattern,
				_voucher_activity_buffer_size - sizeof(buffer->vabh_flags));
	}
#endif
	(void)dispatch_assume_zero(madvise(vam_buffer(base + start),
			len * _voucher_activity_buffer_size, MADV_FREE));
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_heap_madvise_contiguous(size_t bitmap_num,
		_voucher_activity_bitmap_base_t bits)
{
	// TODO: x86 has fast ctz; arm has fast clz; haswell has fast ctz
	dispatch_assert(_voucher_activity_heap_can_madvise());
	if (bits == 0) {
		return;
	} else if (~bits == 0) {
		_voucher_activity_heap_madvise(bitmap_num, 0,
				_voucher_activity_bits_per_bitmap_base_t);
	} else while (bits != 0) {
		unsigned int start = (typeof(start))__builtin_ctzl(bits), len;
		typeof(bits) inverse = ~bits >> start;
		if (inverse) {
			len = (typeof(len))__builtin_ctzl(inverse);
		} else {
			len = _voucher_activity_bits_per_bitmap_base_t - start;
		}
		typeof(bits) mask = ((((typeof(bits))1) << len) - 1) << start;
		bits &= ~mask;
		_voucher_activity_heap_madvise(bitmap_num, start, len);
	}
}

void
_voucher_activity_heap_pressure_warn(void)
{
	if (!_voucher_activity_heap_can_madvise() || !_voucher_activity_heap) {
		return;
	}
	volatile _voucher_activity_bitmap_base_t *bitmap, *pressure_locked_bitmap;
	bitmap = vam_buffer_bitmap();
	pressure_locked_bitmap = vam_pressure_locked_bitmap();

	// number of bitmaps needed to map the current buffer limit =
	// ceil(buffer limit / bits per bitmap)
	size_t nbuffers = _voucher_heap_buffer_limit();
	size_t nbitmaps_quot = nbuffers / _voucher_activity_bits_per_bitmap_base_t;
	size_t nbitmaps_rem = nbuffers % _voucher_activity_bits_per_bitmap_base_t;
	size_t nbitmaps = nbitmaps_quot + ((nbitmaps_rem == 0) ? 0 : 1);

	for (size_t i = 0; i < nbitmaps; i++) {
		_voucher_activity_bitmap_base_t got_bits;
		got_bits = dispatch_atomic_or_orig(&bitmap[i], ~((typeof(bitmap[i]))0),
				relaxed);
		got_bits = ~got_bits; // Now 1 means 'acquired this one, madvise it'
		_voucher_activity_heap_madvise_contiguous(i, got_bits);
		pressure_locked_bitmap[i] |= got_bits;
	}
}

void
_voucher_activity_heap_pressure_normal(void)
{
	if (!_voucher_activity_heap_can_madvise() || !_voucher_activity_heap) {
		return;
	}
	volatile _voucher_activity_bitmap_base_t *bitmap, *pressure_locked_bitmap;
	bitmap = vam_buffer_bitmap();
	pressure_locked_bitmap = vam_pressure_locked_bitmap();
	for (size_t i = 0; i < _voucher_activity_bitmaps_per_heap; i++) {
		_voucher_activity_bitmap_base_t free_bits = pressure_locked_bitmap[i];
		pressure_locked_bitmap[i] = 0;
		if (free_bits != 0) {
			(void)dispatch_atomic_and(&bitmap[i], ~free_bits, release);
		}
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_buffer_init(_voucher_activity_t act,
		_voucher_activity_buffer_header_t buffer, bool reuse)
{
	if (!reuse) {
		buffer->vabh_flags = _voucher_activity_trace_flag_buffer_header;
		buffer->vabh_activity_id = act->va_id;
	}
	buffer->vabh_timestamp = _voucher_activity_timestamp();
	buffer->vabh_next_tracepoint_idx = 1;
	buffer->vabh_sequence_no = dispatch_atomic_inc2o(act, va_max_sequence_no,
			relaxed);
}

static _voucher_activity_buffer_header_t
_voucher_activity_buffer_alloc_slow(_voucher_activity_t act,
		_voucher_activity_buffer_header_t current)
{
	_voucher_activity_buffer_header_t buffer;
	_voucher_activity_lock_lock(va_buffers_lock(act)); // TODO: revisit locking
	buffer = act->va_current_buffer;
	if (buffer != current) goto out;
	buffer = TAILQ_FIRST(&act->va_buffers);
	if (buffer) {
		_voucher_activity_buffer_init(act, buffer, true);
		if (buffer != TAILQ_LAST(&act->va_buffers,
				_voucher_activity_buffer_list_s)) {
			TAILQ_REMOVE(&act->va_buffers, buffer, vabh_list);
			TAILQ_INSERT_TAIL(&act->va_buffers, buffer, vabh_list);
		}
	}
	if (!dispatch_atomic_cmpxchgv2o(act, va_current_buffer, current, buffer,
			&current, release)) {
		if (buffer) {
			TAILQ_REMOVE(&act->va_buffers, buffer, vabh_list);
			_voucher_activity_heap_buffer_free(buffer);
		}
		buffer = current;
	}
out:
	_voucher_activity_lock_unlock(va_buffers_lock(act));
	_dispatch_voucher_activity_debug("buffer reuse %p", act, buffer);
	return buffer;
}

static _voucher_activity_buffer_header_t
_voucher_activity_buffer_alloc(_voucher_activity_t act,
		_voucher_activity_buffer_header_t current)
{
	_voucher_activity_buffer_header_t buffer = NULL;
	if (act->va_max_sequence_no < act->va_buffer_limit) {
		buffer = _voucher_activity_heap_buffer_alloc();
	}
	if (!buffer) return _voucher_activity_buffer_alloc_slow(act, current);
	_voucher_activity_buffer_init(act, buffer, false);
	if (dispatch_atomic_cmpxchgv2o(act, va_current_buffer, current, buffer,
			&current, release)) {
		_voucher_activity_lock_lock(va_buffers_lock(act));
		TAILQ_INSERT_TAIL(&act->va_buffers, buffer, vabh_list);
		_voucher_activity_lock_unlock(va_buffers_lock(act));
	} else {
		_voucher_activity_heap_buffer_free(buffer);
		buffer = current;
	}
	_dispatch_voucher_activity_debug("buffer alloc %p", act, buffer);
	return buffer;
}

#pragma mark -
#pragma mark _voucher_activity_t

#define _voucher_activity_ordered_insert(_act, head, field) do { \
		typeof(_act) _vai; \
		TAILQ_FOREACH(_vai, (head), field) { \
			if (_act->va_id < _vai->va_id) break; \
		} \
		if (_vai) { \
			TAILQ_INSERT_BEFORE(_vai, _act, field); \
		} else { \
			TAILQ_INSERT_TAIL((head), _act, field); \
		} } while (0);

static void _voucher_activity_dispose(_voucher_activity_t act);
static _voucher_activity_t _voucher_atm_activity_mark_used(
		_voucher_activity_t act);
static void _voucher_atm_activity_mark_unused(_voucher_activity_t act);
static _voucher_atm_t _voucher_atm_copy(atm_aid_t atm_id);
static inline void _voucher_atm_release(_voucher_atm_t vatm);
static void _voucher_atm_activity_insert(_voucher_atm_t vatm,
		_voucher_activity_t act);
static void _voucher_atm_activity_remove(_voucher_activity_t act);
static atm_aid_t _voucher_mach_voucher_get_atm_id(mach_voucher_t kv);

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_activity_copy(_voucher_activity_t act)
{
	int use_cnt = dispatch_atomic_inc2o(act, va_use_count, relaxed);
	_dispatch_voucher_activity_debug("retain  -> %d", act, use_cnt + 1);
	if (slowpath(use_cnt < 0)) {
		_dispatch_voucher_activity_debug("overrelease", act);
		DISPATCH_CRASH("Activity overrelease");
	}
	return (use_cnt == 0);
}

DISPATCH_ALWAYS_INLINE
static inline _voucher_activity_t
_voucher_activity_retain(_voucher_activity_t act)
{
	if (_voucher_activity_copy(act)) {
		_dispatch_voucher_activity_debug("invalid resurrection", act);
		DISPATCH_CRASH("Invalid activity resurrection");
	}
	return act;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_release(_voucher_activity_t act)
{
	int use_cnt = dispatch_atomic_dec2o(act, va_use_count, relaxed);
	_dispatch_voucher_activity_debug("release -> %d", act, use_cnt + 1);
	if (fastpath(use_cnt >= 0)) {
		return;
	}
	if (slowpath(use_cnt < -1)) {
		_dispatch_voucher_activity_debug("overrelease", act);
		DISPATCH_CRASH("Activity overrelease");
	}
	return _voucher_atm_activity_mark_unused(act);
}

DISPATCH_ALWAYS_INLINE
static inline _voucher_activity_t
_voucher_activity_atm_retain(_voucher_activity_t act)
{
	int refcnt = dispatch_atomic_inc2o(act, va_refcnt, relaxed);
	_dispatch_voucher_activity_debug("atm retain  -> %d", act, refcnt + 1);
	if (slowpath(refcnt <= 0)) {
		_dispatch_voucher_activity_debug("atm resurrection", act);
		DISPATCH_CRASH("Activity ATM resurrection");
	}
	return act;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_activity_atm_release(_voucher_activity_t act)
{
	int refcnt = dispatch_atomic_dec2o(act, va_refcnt, relaxed);
	_dispatch_voucher_activity_debug("atm release -> %d", act, refcnt + 1);
	if (fastpath(refcnt >= 0)) {
		return;
	}
	if (slowpath(refcnt < -1)) {
		_dispatch_voucher_activity_debug("atm overrelease", act);
		DISPATCH_CRASH("Activity ATM overrelease");
	}
	return _voucher_activity_dispose(act);
}

static inline _voucher_activity_t
_voucher_activity_get(voucher_t v)
{
	_voucher_activity_t act;
	act = v && v->v_activity ? v->v_activity : _voucher_activity_default;
	return act;
}

static _voucher_activity_t
_voucher_activity_find(voucher_activity_id_t va_id, uint32_t hash)
{
	// assumes vam_activities_lock held
	_voucher_activity_t act;
	TAILQ_FOREACH(act, vam_activities(hash), va_list){
		if (act->va_id == va_id) break;
	}
	return act;
}

static _voucher_activity_t
_voucher_activity_copy_from_id(voucher_activity_id_t va_id)
{
	bool resurrect = false;
	uint32_t hash = VACTID_HASH(va_id);
	_voucher_activity_lock_lock(vam_activities_lock());
	_voucher_activity_t act = _voucher_activity_find(va_id, hash);
	if (act) {
		resurrect = _voucher_activity_copy(act);
		_dispatch_voucher_activity_debug("copy from id 0x%llx", act, va_id);
	}
	_voucher_activity_lock_unlock(vam_activities_lock());
	if (resurrect) return _voucher_atm_activity_mark_used(act);
	return act;
}

static _voucher_activity_t
_voucher_activity_try_insert(_voucher_activity_t act_new)
{
	bool resurrect = false;
	voucher_activity_id_t va_id = act_new->va_id;
	uint32_t hash = VACTID_HASH(va_id);
	_voucher_activity_lock_lock(vam_activities_lock());
	_voucher_activity_t act = _voucher_activity_find(va_id, hash);
	if (act) {
		resurrect = _voucher_activity_copy(act);
		_dispatch_voucher_activity_debug("try insert: failed (%p)", act,act_new);
	} else {
		if (slowpath(_TAILQ_IS_ENQUEUED(act_new, va_list))) {
			_dispatch_voucher_activity_debug("corruption", act_new);
			DISPATCH_CRASH("Activity corruption");
		}
		TAILQ_INSERT_TAIL(vam_activities(hash), act_new, va_list);
		_dispatch_voucher_activity_debug("try insert: succeeded", act_new);
	}
	_voucher_activity_lock_unlock(vam_activities_lock());
	if (resurrect) return _voucher_atm_activity_mark_used(act);
	return act;
}

static bool
_voucher_activity_try_remove(_voucher_activity_t act)
{
	bool r;
	voucher_activity_id_t va_id = act->va_id;
	uint32_t hash = VACTID_HASH(va_id);
	_voucher_activity_lock_lock(vam_activities_lock());
	if (slowpath(!va_id)) {
		_dispatch_voucher_activity_debug("corruption", act);
		DISPATCH_CRASH("Activity corruption");
	}
	if ((r = (dispatch_atomic_load2o(act, va_use_count, seq_cst) < 0 &&
			_TAILQ_IS_ENQUEUED(act, va_list)))) {
		TAILQ_REMOVE(vam_activities(hash), act, va_list);
		_TAILQ_MARK_NOT_ENQUEUED(act, va_list);
		act->va_list.tqe_next = (void*)~0ull;
	}
	_dispatch_voucher_activity_debug("try remove: %s", act, r ? "succeeded" :
			"failed");
	_voucher_activity_lock_unlock(vam_activities_lock());
	return r;
}

static _voucher_activity_t
_voucher_activity_create_with_atm(_voucher_atm_t vatm,
		voucher_activity_id_t va_id, voucher_activity_trace_id_t trace_id,
		uint64_t location, _voucher_activity_buffer_header_t buffer)
{
	if (!buffer) buffer = _voucher_activity_heap_buffer_alloc();
	if (!buffer) {
		_dispatch_voucher_atm_debug("no buffer", vatm);
		_voucher_atm_release(vatm); // consume vatm reference
		return NULL;
	}
	if (!trace_id) trace_id = _voucher_activity_trace_id_release;
	_voucher_activity_tracepoint_t vat = (_voucher_activity_tracepoint_t)buffer;
	_voucher_activity_tracepoint_init_with_id(vat, trace_id, ~1ull);
	_voucher_activity_t act = (_voucher_activity_t)buffer;
	act->va_flags = _voucher_activity_trace_flag_buffer_header |
			_voucher_activity_trace_flag_activity |
			_voucher_activity_trace_flag_start |
			_voucher_activity_trace_flag_wide_first;
	act->vabh_next_tracepoint_idx = sizeof(*act)/sizeof(*vat);
	act->va_max_sequence_no = 0;
	act->va_id = va_id ? va_id : VATM_ACTID(vatm, 0);
	act->va_use_count = 0;
	act->va_buffer_limit = _voucher_activity_buffer_limit();
	TAILQ_INIT(&act->va_buffers);
	act->va_flags2 = _voucher_activity_trace_flag_activity |
			_voucher_activity_trace_flag_wide_second;
#if __LP64__
	act->va_flags3 = act->va_flags2;
#endif
	act->va_refcnt = 0;
	act->va_location = location;
	act->va_current_buffer = buffer;
	act->va_atm = vatm; // transfer vatm reference
	_voucher_activity_lock_init(va_buffers_lock(act));
	_TAILQ_MARK_NOT_ENQUEUED(act, va_list);
	_TAILQ_MARK_NOT_ENQUEUED(act, va_atm_list);
	_TAILQ_MARK_NOT_ENQUEUED(act, va_atm_used_list);
	_voucher_activity_t actx = _voucher_activity_try_insert(act);
	if (actx) {
		_voucher_activity_dispose(act);
		act = actx;
	} else {
		_voucher_atm_activity_insert(vatm, act);
	}
	_dispatch_voucher_activity_debug("create", act);
	return act;
}

static void
_voucher_activity_dispose(_voucher_activity_t act)
{
	_dispatch_voucher_activity_debug("dispose", act);
	_voucher_atm_release(act->va_atm);
	if (slowpath(_TAILQ_IS_ENQUEUED(act, va_list))) {
		_dispatch_voucher_activity_debug("corruption", act);
		DISPATCH_CRASH("Activity corruption");
	}
	act->va_list.tqe_next = DISPATCH_OBJECT_LISTLESS;
	dispatch_assert(!_TAILQ_IS_ENQUEUED(act, va_atm_list));
	dispatch_assert(!_TAILQ_IS_ENQUEUED(act, va_atm_used_list));
	_voucher_activity_buffer_header_t buffer, tmp;
	TAILQ_FOREACH_SAFE(buffer, &act->va_buffers, vabh_list, tmp) {
		_dispatch_voucher_activity_debug("buffer free %p", act, buffer);
		TAILQ_REMOVE(&act->va_buffers, buffer, vabh_list);
		_voucher_activity_heap_buffer_free(buffer);
	}
	buffer = (_voucher_activity_buffer_header_t)act;
	_voucher_activity_heap_buffer_free(buffer);
}

static void
_voucher_activity_collect(_voucher_activity_t act)
{
	_dispatch_voucher_activity_debug("collect", act);
	if (_voucher_activity_try_remove(act)) {
		_voucher_atm_activity_remove(act);
	}
}

static _voucher_activity_t
_voucher_activity_copy_from_mach_voucher(mach_voucher_t kv,
		voucher_activity_id_t va_id)
{
	dispatch_once_f(&_voucher_activity_heap_pred, NULL,
			_voucher_activity_heap_init);
	if (_voucher_activity_disabled()) return NULL;
	_voucher_activity_t act = NULL;
	if (dispatch_assume(va_id)) {
		if ((act = _voucher_activity_copy_from_id(va_id))) return act;
	}
	atm_aid_t atm_id = _voucher_mach_voucher_get_atm_id(kv);
	if (!dispatch_assume(atm_id)) return NULL;
	_voucher_activity_buffer_header_t buffer;
	buffer = _voucher_activity_heap_buffer_alloc();
	if (!buffer) return NULL;
	_dispatch_kvoucher_debug("atm copy/create from <%lld>", kv, atm_id);
	_voucher_atm_t vatm = _voucher_atm_copy(atm_id);
	if (!vatm) vatm = _voucher_atm_create(kv, atm_id);
	if (!vatm) {
		_voucher_activity_heap_buffer_free(buffer);
		return NULL;
	}
	if (VACTID_BASEID(va_id) != VATMID2ACTID(atm_id)) va_id = 0;
	// consumes vatm reference:
	act = _voucher_activity_create_with_atm(vatm, va_id, 0, 0, buffer);
	_dispatch_voucher_activity_debug("copy from kvoucher[0x%08x]", act, kv);
	return act;
}

#pragma mark -
#pragma mark _voucher_atm_mailbox

DISPATCH_ALWAYS_INLINE
static inline atm_mailbox_offset_t
_voucher_atm_mailbox_alloc(void)
{
	atm_mailbox_offset_t mailbox_offset = MAILBOX_OFFSET_UNSET;
	size_t index;
	index = _voucher_activity_bitmap_set_first_unset_bit(vam_atm_mbox_bitmap());
	if (index < NO_BITS_WERE_UNSET) {
		mailbox_offset = index * sizeof(atm_mailbox_offset_t);
#if DISPATCH_DEBUG && DISPATCH_VOUCHER_ACTIVITY_DEBUG
		_dispatch_debug("mailbox alloc %zd (%lld)", index, mailbox_offset);
#endif
	}
	return mailbox_offset;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_atm_mailbox_free(atm_mailbox_offset_t mailbox_offset)
{
	if (mailbox_offset == MAILBOX_OFFSET_UNSET) return;
	size_t index = (size_t)mailbox_offset / sizeof(atm_mailbox_offset_t);
#if DISPATCH_DEBUG && DISPATCH_VOUCHER_ACTIVITY_DEBUG
	_dispatch_debug("mailbox free %zd (%lld)", index, mailbox_offset);
#endif
	_voucher_activity_bitmap_clear_bit(vam_atm_mbox_bitmap(), index);
}

DISPATCH_ALWAYS_INLINE
static inline bool
_voucher_atm_mailbox_set(atm_mailbox_offset_t mailbox_offset,
		atm_subaid32_t subaid, bool max_present)
{
	if (mailbox_offset == MAILBOX_OFFSET_UNSET) return false;
	char *mailbox_base = (char*)_voucher_activity_heap->vam_kernel_metadata;
	atm_subaid32_t *mailbox = (atm_subaid32_t*)(mailbox_base + mailbox_offset);
	if (max_present) mailbox++; // second atm_subaid32_t in atm_mailbox_offset_t
	if (*mailbox == subaid) return false;
	*mailbox = subaid;
	return true;
}

#pragma mark -
#pragma mark _voucher_atm_t

static bool _voucher_atm_try_remove(_voucher_atm_t vatm);
static void _voucher_atm_dispose(_voucher_atm_t vatm, bool unregister);
static inline void _voucher_atm_collect_if_needed(bool updated);

DISPATCH_ALWAYS_INLINE
static inline _voucher_atm_t
_voucher_atm_retain(_voucher_atm_t vatm)
{
	// assumes vam_atms_lock or vam_base_atm_lock held
	int refcnt = dispatch_atomic_inc2o(vatm, vatm_refcnt, relaxed);
	_dispatch_voucher_atm_debug("retain  -> %d", vatm, refcnt + 1);
	if (slowpath(refcnt < 0)) {
		_dispatch_voucher_atm_debug("overrelease", vatm);
		DISPATCH_CRASH("ATM overrelease");
	}
	return vatm;
}

DISPATCH_ALWAYS_INLINE
static inline void
_voucher_atm_release(_voucher_atm_t vatm)
{
	int refcnt = dispatch_atomic_dec2o(vatm, vatm_refcnt, relaxed);
	_dispatch_voucher_atm_debug("release -> %d", vatm, refcnt + 1);
	if (fastpath(refcnt >= 0)) {
		return;
	}
	if (slowpath(refcnt < -1)) {
		_dispatch_voucher_atm_debug("overrelease", vatm);
		DISPATCH_CRASH("ATM overrelease");
	}
	if (_voucher_atm_try_remove(vatm)) {
		_voucher_atm_dispose(vatm, true);
	}
}

static _voucher_atm_t
_voucher_atm_find(atm_aid_t atm_id, uint32_t hash)
{
	// assumes vam_atms_lock held
	_voucher_atm_t vatm;
	TAILQ_FOREACH(vatm, vam_atms(hash), vatm_list){
		if (vatm->vatm_id == atm_id) break;
	}
	return vatm;
}

static _voucher_atm_t
_voucher_atm_copy(atm_aid_t atm_id)
{
	uint32_t hash = VATMID_HASH(atm_id);
	_voucher_activity_lock_lock(vam_atms_lock());
	_voucher_atm_t vatm = _voucher_atm_find(atm_id, hash);
	if (vatm) {
		_voucher_atm_retain(vatm);
		_dispatch_voucher_atm_debug("copy", vatm);
	}
	_voucher_activity_lock_unlock(vam_atms_lock());
	return vatm;
}

static _voucher_atm_t
_voucher_atm_try_insert(_voucher_atm_t vatm_new)
{
	atm_aid_t atm_id = vatm_new->vatm_id;
	uint32_t hash = VATMID_HASH(atm_id);
	_voucher_activity_lock_lock(vam_atms_lock());
	_voucher_atm_t vatm = _voucher_atm_find(atm_id, hash);
	if (vatm) {
		_voucher_atm_retain(vatm);
		_dispatch_voucher_atm_debug("try insert: failed (%p)", vatm, vatm_new);
	} else {
		if (slowpath(_TAILQ_IS_ENQUEUED(vatm_new, vatm_list))) {
			_dispatch_voucher_atm_debug("corruption", vatm_new);
			DISPATCH_CRASH("ATM corruption");
		}
		TAILQ_INSERT_TAIL(vam_atms(hash), vatm_new, vatm_list);
		_dispatch_voucher_atm_debug("try insert: succeeded", vatm_new);
	}
	_voucher_activity_lock_unlock(vam_atms_lock());
	return vatm;
}

static bool
_voucher_atm_try_remove(_voucher_atm_t vatm)
{
	bool r;
	atm_aid_t atm_id = vatm->vatm_id;
	uint32_t hash = VATMID_HASH(atm_id);
	_voucher_activity_lock_lock(vam_atms_lock());
	if (slowpath(!atm_id)) {
		_dispatch_voucher_atm_debug("corruption", vatm);
		DISPATCH_CRASH("ATM corruption");
	}
	if ((r = (dispatch_atomic_load2o(vatm, vatm_refcnt, seq_cst) < 0 &&
			_TAILQ_IS_ENQUEUED(vatm, vatm_list)))) {
		TAILQ_REMOVE(vam_atms(hash), vatm, vatm_list);
		_TAILQ_MARK_NOT_ENQUEUED(vatm, vatm_list);
		vatm->vatm_list.tqe_next = (void*)~0ull;
	}
	_dispatch_voucher_atm_debug("try remove: %s", vatm, r ? "succeeded" :
			"failed");
	_voucher_activity_lock_unlock(vam_atms_lock());
	return r;
}

static bool
_voucher_atm_update_mailbox(_voucher_atm_t vatm)
{
	// Update kernel mailbox with largest allocated subaid for this atm_id
	// assumes atm_activities_lock held
	_voucher_activity_t act = TAILQ_LAST(vatm_activities(vatm),
			_voucher_atm_activities_s);
	atm_subaid32_t subaid = act ? VACTID_SUBID(act->va_id) : 0;
	bool r = _voucher_atm_mailbox_set(vatm->vatm_mailbox_offset, subaid, true);
	if (r) {
		_dispatch_voucher_atm_debug("update max-present subaid 0x%x", vatm,
				subaid);
	}
	return r;
}

static bool
_voucher_atm_update_used_mailbox(_voucher_atm_t vatm)
{
	// Update kernel mailbox with smallest in-use subaid for this atm_id
	// assumes atm_activities_lock held
	_voucher_activity_t act = TAILQ_FIRST(vatm_used_activities(vatm));
	atm_subaid32_t subaid = act ? VACTID_SUBID(act->va_id) : ATM_SUBAID32_MAX;
	bool r = _voucher_atm_mailbox_set(vatm->vatm_mailbox_offset, subaid, false);
	if (r) {
		_dispatch_voucher_atm_debug("update min-used subaid 0x%x", vatm,
				subaid);
	}
	return r;
}

static void
_voucher_atm_activity_insert(_voucher_atm_t vatm, _voucher_activity_t act)
{
	_voucher_activity_lock_lock(vatm_activities_lock(vatm));
	if (!_TAILQ_IS_ENQUEUED(act, va_atm_list)) {
		_voucher_activity_ordered_insert(act, vatm_activities(vatm),
				va_atm_list);
		_voucher_atm_update_mailbox(vatm);
	}
	if (!_TAILQ_IS_ENQUEUED(act, va_atm_used_list)) {
		_voucher_activity_ordered_insert(act, vatm_used_activities(vatm),
				va_atm_used_list);
		_voucher_atm_update_used_mailbox(vatm);
	}
	_dispatch_voucher_activity_debug("atm insert", act);
	_voucher_activity_lock_unlock(vatm_activities_lock(vatm));
}

static void
_voucher_atm_activity_remove(_voucher_activity_t act)
{
	_voucher_atm_t vatm = act->va_atm;
	_voucher_activity_lock_lock(vatm_activities_lock(vatm));
	_dispatch_voucher_activity_debug("atm remove", act);
	if (_TAILQ_IS_ENQUEUED(act, va_atm_used_list)) {
		TAILQ_REMOVE(vatm_activities(vatm), act, va_atm_used_list);
		_TAILQ_MARK_NOT_ENQUEUED(act, va_atm_used_list);
		_voucher_atm_update_used_mailbox(vatm);
	}
	if (_TAILQ_IS_ENQUEUED(act, va_atm_list)) {
		TAILQ_REMOVE(vatm_activities(vatm), act, va_atm_list);
		_TAILQ_MARK_NOT_ENQUEUED(act, va_atm_list);
		_voucher_atm_update_mailbox(vatm);
		// Balance initial creation refcnt. Caller must hold additional
		// reference to ensure this does not release vatm before the unlock,
		// see _voucher_atm_activity_collect
		_voucher_activity_atm_release(act);
	}
	_voucher_activity_lock_unlock(vatm_activities_lock(vatm));
}

static _voucher_activity_t
_voucher_atm_activity_mark_used(_voucher_activity_t act)
{
	_voucher_atm_t vatm = act->va_atm;
	_voucher_activity_lock_lock(vatm_activities_lock(vatm));
	if (!_TAILQ_IS_ENQUEUED(act, va_atm_used_list)) {
		_voucher_activity_ordered_insert(act, vatm_used_activities(vatm),
				va_atm_used_list);
		_voucher_atm_update_used_mailbox(vatm);
		_dispatch_voucher_activity_debug("mark used", act);
	}
	_voucher_activity_lock_unlock(vatm_activities_lock(vatm));
	return act;
}

static void
_voucher_atm_activity_mark_unused(_voucher_activity_t act)
{
	bool atm_collect = false, updated = false;
	_voucher_atm_t vatm = act->va_atm;
	_voucher_activity_lock_lock(vatm_activities_lock(vatm));
	if (_TAILQ_IS_ENQUEUED(act, va_atm_used_list)) {
		_dispatch_voucher_activity_debug("mark unused", act);
		TAILQ_REMOVE(&vatm->vatm_used_activities, act, va_atm_used_list);
		_TAILQ_MARK_NOT_ENQUEUED(act, va_atm_used_list);
		atm_collect = true;
		_voucher_atm_retain(vatm);
		updated = _voucher_atm_update_used_mailbox(vatm);
	}
	_voucher_activity_lock_unlock(vatm_activities_lock(vatm));
	if (atm_collect) {
		_voucher_atm_release(vatm);
		_voucher_atm_collect_if_needed(updated);
	}
}

static void
_voucher_atm_activity_collect(_voucher_atm_t vatm, atm_subaid32_t min_subaid)
{
	_dispatch_voucher_atm_debug("collect min subaid 0x%x", vatm, min_subaid);
	voucher_activity_id_t min_va_id = VATM_ACTID(vatm, min_subaid);
	_voucher_activity_t act;
	do {
		_voucher_activity_lock_lock(vatm_activities_lock(vatm));
		TAILQ_FOREACH(act, vatm_activities(vatm), va_atm_list) {
			if (act->va_id >= min_va_id) {
				act = NULL;
				break;
			}
			if (!_TAILQ_IS_ENQUEUED(act, va_atm_used_list)) {
				_voucher_activity_atm_retain(act);
				break;
			}
		}
		_voucher_activity_lock_unlock(vatm_activities_lock(vatm));
		if (act) {
			_voucher_activity_collect(act);
			_voucher_activity_atm_release(act);
		}
	} while (act);
}

DISPATCH_NOINLINE
static void
_voucher_atm_collect(void)
{
	_voucher_atm_t vatms[_voucher_atm_mailboxes], vatm;
	atm_aid_t aids[_voucher_atm_mailboxes];
	mach_atm_subaid_t subaids[_voucher_atm_mailboxes];
	uint32_t i, a = 0, s;

	_voucher_activity_lock_lock(vam_atms_lock());
	for (i = 0; i < _voucher_activity_hash_size; i++) {
		TAILQ_FOREACH(vatm, vam_atms(i), vatm_list){
			if (vatm == _voucher_activity_heap->vam_default_activity_atm ||
					vatm->vatm_mailbox_offset == MAILBOX_OFFSET_UNSET) continue;
			_dispatch_voucher_atm_debug("find min subaid", vatm);
			vatms[a] = _voucher_atm_retain(vatm);
			aids[a] = vatm->vatm_id;
			if (++a == _voucher_atm_mailboxes) goto out;
		}
	}
out:
	_voucher_activity_lock_unlock(vam_atms_lock());
	if (!a) return;
	kern_return_t kr;
	mach_voucher_t kv = vatms[0]->vatm_kvoucher;
	mach_voucher_attr_content_t kvc_in = (mach_voucher_attr_content_t)&aids;
	mach_voucher_attr_content_size_t kvc_in_size = sizeof(atm_aid_t) * a;
	mach_voucher_attr_content_t kvc_out = (mach_voucher_attr_content_t)&subaids;
	mach_voucher_attr_content_size_t kvc_out_size = sizeof(mach_atm_subaid_t)*a;
	kr = mach_voucher_attr_command(kv, MACH_VOUCHER_ATTR_KEY_ATM,
			ATM_FIND_MIN_SUB_AID, kvc_in, kvc_in_size, kvc_out, &kvc_out_size);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
	s = kvc_out_size / sizeof(mach_atm_subaid_t);
#if DISPATCH_DEBUG && DISPATCH_VOUCHER_ACTIVITY_DEBUG
	_dispatch_debug("found min subaids (%u out of %u)", s, a);
#endif
	for (i = 0; i < a; i++) {
		if (i < s) _voucher_atm_activity_collect(vatms[i],
				(atm_subaid32_t)subaids[i]);
		_voucher_atm_release(vatms[i]);
	}
}

static inline void
_voucher_atm_collect_if_needed(bool updated)
{
	long level;
	if (updated) {
		level = dispatch_atomic_add(&_voucher_atm_collect_level, 2ul, relaxed);
	} else {
		level = _voucher_atm_collect_level;
		if (!level) return;
	}
	if (level & 1 || level <= _voucher_atm_collect_threshold) return;
	if (!dispatch_atomic_cmpxchg(&_voucher_atm_collect_level, level, level + 1,
			acquire)) return;
#if DISPATCH_DEBUG && DISPATCH_VOUCHER_ACTIVITY_DEBUG
	_dispatch_debug("atm collect: reached level %ld", level/2);
#endif
	if (slowpath(level < 0)) {
		DISPATCH_CRASH("ATM collection level corruption");
	}
	_voucher_atm_collect();
	dispatch_atomic_sub(&_voucher_atm_collect_level, level + 1, release);
}

DISPATCH_NOINLINE
static void
_voucher_atm_fault(mach_voucher_attr_command_t kvc_cmd)
{
	_voucher_activity_t act = _voucher_activity_get(_voucher_get());
	mach_voucher_t kv = _voucher_activity_get_atm_mach_voucher(act);
	if (!kv) return;

	kern_return_t kr;
	mach_atm_subaid_t subaid = VACTID_SUBID(act->va_id);
	mach_voucher_attr_content_t kvc_in = (mach_voucher_attr_content_t)&subaid;
	mach_voucher_attr_content_size_t kvc_in_size = sizeof(mach_atm_subaid_t);
	mach_voucher_attr_content_t kvc_out = (mach_voucher_attr_content_t)&subaid;
	mach_voucher_attr_content_size_t kvc_out_size = sizeof(mach_atm_subaid_t);
	kr = mach_voucher_attr_command(kv, MACH_VOUCHER_ATTR_KEY_ATM,
			kvc_cmd, kvc_in, kvc_in_size, kvc_out, &kvc_out_size);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
}

static atm_aid_t
_voucher_mach_voucher_get_atm_id(mach_voucher_t kv)
{
	kern_return_t kr;
	atm_aid_t atm_id = 0;
	mach_voucher_attr_content_t kvc = (mach_voucher_attr_content_t)&atm_id;
    mach_voucher_attr_content_size_t kvc_size = sizeof(atm_id);
	kr = mach_voucher_extract_attr_content(kv, MACH_VOUCHER_ATTR_KEY_ATM, kvc,
			&kvc_size);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
    return atm_id;
}

static mach_voucher_t
_voucher_atm_mach_voucher_create(atm_aid_t *atm_id_ptr)
{
	kern_return_t kr;
	mach_voucher_t kv;
	static const mach_voucher_attr_recipe_data_t atm_create_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_ATM,
		.command = MACH_VOUCHER_ATTR_ATM_CREATE,
	};
	kr = _voucher_create_mach_voucher(&atm_create_recipe,
			sizeof(atm_create_recipe), &kv);
	if (dispatch_assume_zero(kr)) {
		DISPATCH_CLIENT_CRASH("Could not create ATM mach voucher");
	}
	atm_aid_t atm_id = _voucher_mach_voucher_get_atm_id(kv);
	if (!dispatch_assume(atm_id)) {
		DISPATCH_CLIENT_CRASH("Could not extract ATM ID");
	}
	_dispatch_kvoucher_debug("atm create <%lld>", kv, atm_id);
	*atm_id_ptr = atm_id;
	return kv;
}

static void
_voucher_atm_mailbox_mach_voucher_register(_voucher_atm_t vatm,
		mach_voucher_t kv)
{
	_dispatch_voucher_atm_debug("mailbox register %lld with kvoucher[0x%08x]",
			vatm, vatm->vatm_mailbox_offset, kv);
	kern_return_t kr;
	mach_voucher_t akv;
	atm_mailbox_offset_t offset = vatm->vatm_mailbox_offset;
	mach_voucher_attr_recipe_t vr;
	size_t vr_size;
	static const mach_voucher_attr_recipe_data_t atm_register_recipe = {
		.key = MACH_VOUCHER_ATTR_KEY_ATM,
		.command = MACH_VOUCHER_ATTR_ATM_REGISTER,
		.content_size = sizeof(offset),
	};
	vr_size = sizeof(atm_register_recipe) + atm_register_recipe.content_size;
	vr = alloca(vr_size);
	*vr = atm_register_recipe;
	vr->previous_voucher = kv;
	memcpy(&vr->content, &offset, sizeof(offset));
	kr = _voucher_create_mach_voucher(vr, vr_size, &akv);
	if (dispatch_assume_zero(kr)) {
		DISPATCH_CLIENT_CRASH("Could not register ATM ID");
	}
	if (!vatm->vatm_kvoucher) {
		vatm->vatm_kvoucher = akv;
	} else {
#if !RDAR_17510224
		if (akv != vatm->vatm_kvoucher) {
			DISPATCH_CRASH("Unexpected mach voucher returned by ATM ID "
					"registration");
		}
		_voucher_dealloc_mach_voucher(akv);
#else
		DISPATCH_CRASH("Registered invalid ATM object");
#endif
	}
	_dispatch_voucher_atm_debug("mailbox registered %lld", vatm,
			vatm->vatm_mailbox_offset);
}

static void
_voucher_atm_mailbox_register(_voucher_atm_t vatm)
{
	mach_voucher_t kv = vatm->vatm_kvoucher;
	if (!kv) return;
#if !RDAR_17510224
	_voucher_atm_mailbox_mach_voucher_register(vatm, kv);
#else // RDAR_17510224
	_dispatch_voucher_atm_debug("mailbox register %lld", vatm,
			vatm->vatm_mailbox_offset);
	kern_return_t kr;
	atm_mailbox_offset_t offset = vatm->vatm_mailbox_offset;
	mach_voucher_attr_content_t kvc_in = (mach_voucher_attr_content_t)&offset;
	mach_voucher_attr_content_size_t kvc_in_size = sizeof(offset);
	mach_voucher_attr_content_t kvc_out = NULL;
	mach_voucher_attr_content_size_t kvc_out_size = 0;
	kr = mach_voucher_attr_command(kv, MACH_VOUCHER_ATTR_KEY_ATM,
			ATM_ACTION_REGISTER, kvc_in, kvc_in_size, kvc_out,
			&kvc_out_size);
	DISPATCH_VERIFY_MIG(kr);
	if (dispatch_assume_zero(kr)) {
		DISPATCH_CLIENT_CRASH("Could not register ATM ID");
	}
	_dispatch_voucher_atm_debug("mailbox registered %lld", vatm,
			vatm->vatm_mailbox_offset);
#endif // RDAR_17510224
}

static bool
_voucher_atm_mailbox_unregister(_voucher_atm_t vatm)
{
	if (vatm->vatm_mailbox_offset == MAILBOX_OFFSET_UNSET) return false;
	_dispatch_voucher_atm_debug("mailbox unregister %lld", vatm,
			vatm->vatm_mailbox_offset);
	mach_voucher_t kv = vatm->vatm_kvoucher;
	dispatch_assert(kv);
	kern_return_t kr;
	atm_mailbox_offset_t offset = vatm->vatm_mailbox_offset;
	mach_voucher_attr_content_t kvc_in = (mach_voucher_attr_content_t)&offset;
	mach_voucher_attr_content_size_t kvc_in_size = sizeof(offset);
	mach_voucher_attr_content_t kvc_out = NULL;
	mach_voucher_attr_content_size_t kvc_out_size = 0;
	kr = mach_voucher_attr_command(kv, MACH_VOUCHER_ATTR_KEY_ATM,
			ATM_ACTION_UNREGISTER, kvc_in, kvc_in_size, kvc_out, &kvc_out_size);
	DISPATCH_VERIFY_MIG(kr);
	if (kr && kr != KERN_INVALID_VALUE) {
		(void)dispatch_assume_zero(kr);
		DISPATCH_CLIENT_CRASH("Could not unregister ATM ID");
	}
	_dispatch_voucher_atm_debug("mailbox unregistered %lld", vatm,
			vatm->vatm_mailbox_offset);
	return true;
}

static _voucher_atm_t
_voucher_atm_create(mach_voucher_t kv, atm_aid_t atm_id)
{
	atm_mailbox_offset_t mailbox_offset = _voucher_atm_mailbox_alloc();
	if (kv && mailbox_offset == MAILBOX_OFFSET_UNSET) return NULL;
	_voucher_atm_t vatm = _dispatch_calloc(1ul, sizeof(struct _voucher_atm_s));
	if (!kv) {
		kv = _voucher_atm_mach_voucher_create(&atm_id);
		if (mailbox_offset == MAILBOX_OFFSET_UNSET) {
			_voucher_dealloc_mach_voucher(kv);
		} else {
			vatm->vatm_kvoucher = kv;
		}
		kv = MACH_VOUCHER_NULL;
	}
	vatm->vatm_id = atm_id;
	vatm->vatm_mailbox_offset = mailbox_offset;
	_voucher_activity_lock_init(vatm_activities_lock(vatm));
	TAILQ_INIT(&vatm->vatm_activities);
	TAILQ_INIT(&vatm->vatm_used_activities);
	_voucher_atm_mailbox_set(mailbox_offset, 0, true);
	_voucher_atm_mailbox_set(mailbox_offset, ATM_SUBAID32_MAX, false);
	_voucher_atm_t vatmx = _voucher_atm_try_insert(vatm);
	if (vatmx) {
		_voucher_atm_dispose(vatm, false);
		vatm = vatmx;
	} else if (kv) {
		_voucher_atm_mailbox_mach_voucher_register(vatm, kv);
	} else {
		_voucher_atm_mailbox_register(vatm);
	}
	_dispatch_voucher_atm_debug("create with kvoucher[0x%08x]", vatm, kv);
	return vatm;
}

static void
_voucher_atm_dispose(_voucher_atm_t vatm, bool unregister)
{
	_dispatch_voucher_atm_debug("dispose", vatm);
	dispatch_assert(TAILQ_EMPTY(&vatm->vatm_activities));
	dispatch_assert(TAILQ_EMPTY(&vatm->vatm_used_activities));
	if (slowpath(_TAILQ_IS_ENQUEUED(vatm, vatm_list))) {
		_dispatch_voucher_atm_debug("corruption", vatm);
		DISPATCH_CRASH("ATM corruption");
	}
	vatm->vatm_list.tqe_next = DISPATCH_OBJECT_LISTLESS;
	bool free_mailbox = (vatm->vatm_mailbox_offset != MAILBOX_OFFSET_UNSET);
	if (vatm->vatm_kvoucher) {
		if (unregister) free_mailbox = _voucher_atm_mailbox_unregister(vatm);
		_voucher_dealloc_mach_voucher(vatm->vatm_kvoucher);
		vatm->vatm_kvoucher = MACH_VOUCHER_NULL;
	}
	if (free_mailbox) {
		_voucher_atm_mailbox_free(vatm->vatm_mailbox_offset);
		vatm->vatm_mailbox_offset = MAILBOX_OFFSET_UNSET;
	}
	free(vatm);
}

static inline mach_voucher_t
_voucher_activity_get_atm_mach_voucher(_voucher_activity_t act)
{
	mach_voucher_t kv;
	kv = act && act->va_atm ? act->va_atm->vatm_kvoucher : MACH_VOUCHER_NULL;
	return kv;
}

DISPATCH_NOINLINE
static _voucher_atm_t
_voucher_atm_base_copy_and_activity_id_make(voucher_activity_id_t *va_id_ptr)
{
	_voucher_atm_subid_t subid;
	_voucher_atm_t vatm, vatm_old = NULL, vatm_new = NULL;
	if (_voucher_activity_heap->vam_base_atm_subid_max == 1) {
		vatm = _voucher_atm_create(0, 0);
		subid = 1;
		goto out;
	}
	_voucher_activity_lock_lock(vam_base_atm_lock());
	vatm = _voucher_activity_heap->vam_base_atm;
retry:
	_voucher_atm_retain(vatm);
	subid = _voucher_activity_heap->vam_base_atm_subid;
	if (subid++ >= _voucher_activity_heap->vam_base_atm_subid_max) {
		_voucher_activity_lock_unlock(vam_base_atm_lock());
		if (!vatm_new) vatm_new = _voucher_atm_create(0, 0);
		_voucher_activity_lock_lock(vam_base_atm_lock());
		_voucher_atm_release(vatm);
		vatm_old = vatm;
		vatm = _voucher_activity_heap->vam_base_atm;
		if (vatm != vatm_old) {
			vatm_old = NULL;
			goto retry;
		}
		_voucher_activity_heap->vam_base_atm = vatm = vatm_new;
		_voucher_activity_heap->vam_base_atm_subid = subid = 1;
		vatm_new = NULL;
		_voucher_atm_retain(vatm);
		_dispatch_voucher_atm_debug("base replace", vatm);
	} else {
		_voucher_activity_heap->vam_base_atm_subid = subid;
		_dispatch_voucher_atm_debug("base copy", vatm);
	}
	_voucher_activity_lock_unlock(vam_base_atm_lock());
	if (vatm_old) _voucher_atm_release(vatm_old);
	if (vatm_new) _voucher_atm_release(vatm_new);
out:
	*va_id_ptr = VATM_ACTID(vatm, subid);
	return vatm;
}

static voucher_activity_id_t
_voucher_atm_nested_atm_id_make(void)
{
	atm_aid_t atm_id;
	mach_voucher_t kv = _voucher_atm_mach_voucher_create(&atm_id);
	_voucher_dealloc_mach_voucher(kv); // just need the unique ID
	return VATMID2ACTID(atm_id);
}

static voucher_activity_id_t
_voucher_atm_nested_activity_id_make(void)
{
	voucher_activity_id_t va_id, va_id_old, va_id_new;
	_voucher_atm_subid_t subid;
	_voucher_activity_lock_lock(vam_nested_atm_lock());
	va_id = _voucher_activity_heap->vam_nested_atm_id;
retry:
	subid = _voucher_activity_heap->vam_nested_atm_subid;
	if (subid++ >= VATM_SUBID_MAX) {
		_voucher_activity_lock_unlock(vam_nested_atm_lock());
		va_id_new = _voucher_atm_nested_atm_id_make();
		va_id_old = va_id;
		_voucher_activity_lock_lock(vam_nested_atm_lock());
		va_id = _voucher_activity_heap->vam_nested_atm_id;
		if (va_id != va_id_old) goto retry;
		_voucher_activity_heap->vam_nested_atm_id = va_id = va_id_new;
		subid = 1;
	}
	_voucher_activity_heap->vam_nested_atm_subid = subid;
	_voucher_activity_lock_unlock(vam_nested_atm_lock());
	return va_id + subid;
}

#pragma mark -
#pragma mark voucher_activity_id_t

voucher_activity_id_t
voucher_activity_start_with_location(voucher_activity_trace_id_t trace_id,
		uint64_t location, voucher_activity_flag_t flags)
{
	dispatch_once_f(&_voucher_activity_heap_pred, NULL,
			_voucher_activity_heap_init);
	if (!_voucher_activity_trace_id_enabled(trace_id)) return 0;
	voucher_activity_id_t va_id = 0, va_base_id = 0;
	_voucher_atm_t vatm = NULL;
	_voucher_activity_t act = NULL;
	_voucher_activity_tracepoint_t vat = NULL;
	unsigned int activities = 1, oactivities = 0;
	voucher_t ov = _voucher_get();
	if (!(flags & voucher_activity_flag_force) && ov && ov->v_activities) {
		oactivities = ov->v_activities;
		activities += oactivities;
		if (activities > _voucher_max_activities) {
			va_id = _voucher_atm_nested_activity_id_make();
			goto out;
		}
	}
	if (activities == 1) {
		vatm = _voucher_atm_base_copy_and_activity_id_make(&va_id);
		if (vatm->vatm_kvoucher) {
			// consumes vatm reference:
			act = _voucher_activity_create_with_atm(vatm, va_id, trace_id,
					location, NULL);
			vat = (_voucher_activity_tracepoint_t)act;
		} else {
			_voucher_atm_release(vatm);
		}
		if (!act) {
			activities++;
			// default to _voucher_activity_default base activity
			va_base_id = _voucher_activity_default->va_id;
		}
	}
	pthread_priority_t priority = _voucher_get_priority(ov);
	mach_voucher_attr_recipe_size_t extra = ov ? _voucher_extra_size(ov) : 0;
	voucher_t v = _voucher_alloc(activities, priority, extra);
	if (extra) {
		memcpy(_voucher_extra_recipes(v), _voucher_extra_recipes(ov), extra);
	}
	if (ov && ov->v_kvoucher) {
		voucher_t kvb = ov->v_kvbase ? ov->v_kvbase : ov;
		v->v_kvbase = _voucher_retain(kvb);
		v->v_kvoucher = kvb->v_kvoucher;
	}
	voucher_activity_id_t *activity_ids = _voucher_activity_ids(v);
	if (oactivities) {
		memcpy(activity_ids, _voucher_activity_ids(ov),
				oactivities * sizeof(voucher_activity_id_t));
	}
	if (!va_id) {
		va_id = _voucher_atm_nested_activity_id_make();
		if (ov && ov->v_activity) {
			act = _voucher_activity_retain(ov->v_activity);
		}
	}
	if (va_base_id) activity_ids[0] = va_base_id;
	activity_ids[activities-1] = va_id;
	v->v_activity = act;
	_voucher_swap(ov, v);
	if (vat) return va_id; // new _voucher_activity_s contains trace info
out:
	vat = _voucher_activity_trace_with_id(trace_id);
	if (vat) {
		vat->vat_flags |= _voucher_activity_trace_flag_activity |
				_voucher_activity_trace_flag_start;
		vat->vat_data[0] = va_id;
	}
	return va_id;
}

voucher_activity_id_t
voucher_activity_start(voucher_activity_trace_id_t trace_id,
		voucher_activity_flag_t flags)
{
	return voucher_activity_start_with_location(trace_id, 0, flags);
}

void
voucher_activity_end(voucher_activity_id_t va_id)
{
	if (!va_id) return;
	_voucher_activity_tracepoint_t vat;
	vat = _voucher_activity_trace_with_id(_voucher_activity_trace_id_release);
	if (vat) {
		vat->vat_flags |= _voucher_activity_trace_flag_activity |
				_voucher_activity_trace_flag_end;
		vat->vat_data[0] = va_id;
	}
	voucher_t v = _voucher_get();
	if (!v) return;
	unsigned int activities =  v->v_activities, act_idx = activities;
	voucher_activity_id_t *activity_ids = _voucher_activity_ids(v);
	while (act_idx) {
		if (activity_ids[act_idx-1] == va_id) break;
		act_idx--;
	}
	if (!act_idx) return; // activity_id not found
	pthread_priority_t priority = _voucher_get_priority(v);
	mach_voucher_attr_recipe_size_t extra = _voucher_extra_size(v);
	voucher_t nv = NULL;
	if (act_idx > 1 || activities == 1) --activities;
	if (priority || activities || extra || v->v_kvoucher) {
		nv = _voucher_alloc(activities, priority, extra);
		if (extra) {
			memcpy(_voucher_extra_recipes(nv), _voucher_extra_recipes(v),extra);
		}
	}
	if (v->v_kvoucher) {
		voucher_t kvb = v->v_kvbase ? v->v_kvbase : v;
		nv->v_kvbase = _voucher_retain(kvb);
		nv->v_kvoucher = kvb->v_kvoucher;
	}
	bool atm_collect = !activities;
	if (activities) {
		voucher_activity_id_t *new_activity_ids = _voucher_activity_ids(nv);
		if (act_idx == 1 && _voucher_activity_default) {
			atm_collect = true;
			// default to _voucher_activity_default base activity
			new_activity_ids[0] = _voucher_activity_default->va_id;
			memcpy(&new_activity_ids[1], &activity_ids[1],
					(activities - 1) * sizeof(voucher_activity_id_t));
		} else {
			if (v->v_activity) {
				nv->v_activity = _voucher_activity_retain(v->v_activity);
			}
			memcpy(new_activity_ids, activity_ids,
					--act_idx * sizeof(voucher_activity_id_t));
			if (act_idx < activities) {
				memcpy(&new_activity_ids[act_idx], &activity_ids[act_idx+1],
						(activities - act_idx) * sizeof(voucher_activity_id_t));
			}
		}
	}
	_voucher_swap(v, nv);
}

unsigned int
voucher_get_activities(voucher_activity_id_t *entries, unsigned int *count)
{
	voucher_t v = _voucher_get();
	if (!v || !count) return 0;
	unsigned int activities = v->v_activities;
	if (*count < activities) activities = *count;
	*count = v->v_activities;
	voucher_activity_id_t *activity_ids = _voucher_activity_ids(v);
	if (activities && entries) {
		memcpy(entries, activity_ids, activities *
				sizeof(voucher_activity_id_t));
	}
	return activities;
}

uint8_t
voucher_activity_get_namespace(void)
{
	voucher_t v = _voucher_get();
	if (!v || !v->v_activity) return 0;
	return v->v_activity->va_namespace;
}

DISPATCH_NOINLINE
_voucher_activity_tracepoint_t
_voucher_activity_tracepoint_get_slow(unsigned int slots)
{
	_voucher_activity_t act;
	_voucher_activity_buffer_header_t vab;
	_voucher_activity_tracepoint_t vat = NULL;
	voucher_t v = _voucher_get();
	if (v && v->v_activity) {
		act = v->v_activity;
	} else {
		dispatch_once_f(&_voucher_activity_heap_pred, NULL,
				_voucher_activity_heap_init);
		if (_voucher_activity_disabled()) return NULL;
		act = _voucher_activity_default;
	}
	vab = act->va_current_buffer;
	if (vab && vab->vabh_next_tracepoint_idx <=
			_voucher_activity_tracepoints_per_buffer) {
		goto retry; // another slowpath raced us
	}
	do {
		vab = _voucher_activity_buffer_alloc(act, vab);
		if (!vab) break;
retry:
		vat = _voucher_activity_buffer_tracepoint_get(vab, slots);
	} while (!vat);
	return vat;
}

static inline void
_voucher_activity_trace_fault(voucher_activity_trace_id_t trace_id)
{
	if (!slowpath(_voucher_activity_trace_id_is_subtype(trace_id, error))) {
		return;
	}
	mach_voucher_attr_command_t atm_cmd = ATM_ACTION_COLLECT;
	if (_voucher_activity_trace_id_is_subtype(trace_id, fault)) {
		atm_cmd = ATM_ACTION_LOGFAIL;
	}
	return _voucher_atm_fault(atm_cmd);
}

uint64_t
voucher_activity_trace(voucher_activity_trace_id_t trace_id, uint64_t location,
		void *buffer, size_t length)
{
	if (!_voucher_activity_trace_id_enabled(trace_id)) return 0;
	_voucher_activity_tracepoint_t vat;
	const unsigned int slots = length <= sizeof(vat->vat_data) ? 1 : 2;
	vat = _voucher_activity_tracepoint_get(slots);
	if (!vat) vat = _voucher_activity_tracepoint_get_slow(slots);
	if (!vat) return 0;
	uint64_t timestamp = _voucher_activity_tracepoint_init_with_id(vat,
			trace_id, location);
	void *tbuf = vat->vat_data;
	size_t tlen = sizeof(vat->vat_data);
	if (length < tlen) {
		memcpy(tbuf, buffer, length);
	} else {
		memcpy(tbuf, buffer, tlen);
	}
	if (length > tlen) {
		vat->vat_flags |= _voucher_activity_trace_flag_wide_first;
		buffer += tlen;
		length -= tlen;
		(++vat)->vat_flags = _voucher_activity_trace_flag_tracepoint |
				_voucher_activity_trace_flag_wide_second;
		vat->vat_type = 0; vat->vat_namespace = 0;
		tbuf = (void*)vat + offsetof(typeof(*vat), vat_code);
		tlen = sizeof(*vat) - offsetof(typeof(*vat), vat_code);
		if (length < tlen) {
			memcpy(tbuf, buffer, length);
		} else {
			memcpy(tbuf, buffer, tlen);
		}
	}
	_voucher_activity_trace_fault(trace_id);
	return timestamp;
}

uint64_t
voucher_activity_trace_args(voucher_activity_trace_id_t trace_id,
		uint64_t location, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
		uintptr_t arg4)
{
	if (!_voucher_activity_trace_id_enabled(trace_id)) return 0;
	_voucher_activity_tracepoint_t vat;
	vat = _voucher_activity_tracepoint_get(1);
	if (!vat) vat = _voucher_activity_tracepoint_get_slow(1);
	if (!vat) return 0;
	uint64_t timestamp = _voucher_activity_tracepoint_init_with_id(vat,
			trace_id, location);
	vat->vat_flags |= _voucher_activity_trace_flag_tracepoint_args;
	vat->vat_data[0] = arg1;
	vat->vat_data[1] = arg2;
	vat->vat_data[2] = arg3;
	vat->vat_data[3] = arg4;
	_voucher_activity_trace_fault(trace_id);
	return timestamp;
}

#pragma mark -
#pragma mark _voucher_debug

size_t
_voucher_debug(voucher_t v, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	#define bufprintf(...) \
			offset += dsnprintf(&buf[offset], bufsiz - offset, ##__VA_ARGS__)
	bufprintf("voucher[%p] = { xrefcnt = 0x%x, refcnt = 0x%x, ", v,
			v->os_obj_xref_cnt + 1, v->os_obj_ref_cnt + 1);

	if (v->v_kvbase) {
		bufprintf("base voucher %p, ", v->v_kvbase);
	}
	if (v->v_kvoucher) {
		bufprintf("kvoucher%s 0x%x, ", v->v_kvoucher == v->v_ipc_kvoucher ?
				" & ipc kvoucher" : "", v->v_kvoucher);
	}
	if (v->v_ipc_kvoucher && v->v_ipc_kvoucher != v->v_kvoucher) {
		bufprintf("ipc kvoucher 0x%x, ", v->v_ipc_kvoucher);
	}
	if (v->v_has_priority) {
		bufprintf("QOS 0x%x, ", *_voucher_priority(v));
	}
	if (v->v_activities) {
		voucher_activity_id_t *activity_ids = _voucher_activity_ids(v);
		bufprintf("activity IDs = { ");
		unsigned int i;
		for (i = 0; i < v->v_activities; i++) {
			bufprintf("0x%llx, ", *activity_ids++);
		}
		bufprintf("}, ");
	}
	if (v->v_activity) {
		_voucher_activity_t va = v->v_activity;
		_voucher_atm_t vatm = va->va_atm;
		bufprintf("activity[%p] = { ID 0x%llx, use %d, atm[%p] = { "
				"AID 0x%llx, ref %d, kvoucher 0x%x } }, ", va, va->va_id,
				va->va_use_count + 1, va->va_atm, vatm->vatm_id,
				vatm->vatm_refcnt + 1, vatm->vatm_kvoucher);
	}
	bufprintf("}");
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
#endif

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

void
voucher_replace_default_voucher(void)
{
}

void
voucher_decrement_importance_count4CF(voucher_t v)
{
	(void)v;
}

void
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
voucher_create_with_mach_msg(mach_msg_header_t *msg)
{
	(void)msg;
	return NULL;
}

#if VOUCHER_ENABLE_GET_MACH_VOUCHER
mach_voucher_t
voucher_get_mach_voucher(voucher_t voucher)
{
	(void)voucher;
	return 0;
}
#endif

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

void
_voucher_atfork_child(void)
{
}

void
_voucher_init(void)
{
}

void*
voucher_activity_get_metadata_buffer(size_t *length)
{
	*length = 0;
	return NULL;
}

void
_voucher_activity_heap_pressure_normal(void)
{
}

void
_voucher_activity_heap_pressure_warn(void)
{
}

voucher_activity_id_t
voucher_activity_start_with_location(voucher_activity_trace_id_t trace_id,
		uint64_t location, voucher_activity_flag_t flags)
{
	(void)trace_id; (void)location; (void)flags;
	return 0;
}

voucher_activity_id_t
voucher_activity_start(voucher_activity_trace_id_t trace_id,
		voucher_activity_flag_t flags)
{
	(void)trace_id; (void)flags;
	return 0;
}

void
voucher_activity_end(voucher_activity_id_t activity_id)
{
	(void)activity_id;
}

unsigned int
voucher_get_activities(voucher_activity_id_t *entries, unsigned int *count)
{
	(void)entries; (void)count;
	return 0;
}

uint8_t
voucher_activity_get_namespace(void)
{
	return 0;
}

uint64_t
voucher_activity_trace(voucher_activity_trace_id_t trace_id, uint64_t location,
		void *buffer, size_t length)
{
	(void)trace_id; (void)location; (void)buffer; (void)length;
	return 0;
}

uint64_t
voucher_activity_trace_args(voucher_activity_trace_id_t trace_id,
		uint64_t location, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
		uintptr_t arg4)
{
	(void)trace_id; (void)location;
	(void)arg1; (void)arg2; (void)arg3; (void)arg4;
	return 0;
}

size_t
_voucher_debug(voucher_t v, char* buf, size_t bufsiz)
{
	(void)v; (void)buf; (void)bufsiz;
	return 0;
}

#endif // VOUCHER_USE_MACH_VOUCHER
