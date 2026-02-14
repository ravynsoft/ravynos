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

#ifndef __DISPATCH_OBJECT_INTERNAL__
#define __DISPATCH_OBJECT_INTERNAL__

#if !OS_OBJECT_USE_OBJC
#define OS_OBJECT_DECL(name)  DISPATCH_DECL(name)
#define OS_OBJECT_DECL_SUBCLASS(name, super)  DISPATCH_DECL(name)
#endif

#if USE_OBJC
#define OS_OBJECT_EXTRA_VTABLE_SYMBOL(name) _OS_##name##_vtable
#define DISPATCH_CLASS_SYMBOL(name) OS_dispatch_##name##_class
#define DISPATCH_CLASS_RAW_SYMBOL_NAME(name) \
		OS_OBJC_CLASS_RAW_SYMBOL_NAME(DISPATCH_CLASS(name))
#else
#define OS_OBJECT_CLASS_SYMBOL(name) _##name##_vtable
#define OS_OBJC_CLASS_RAW_SYMBOL_NAME(name) \
		"__" OS_STRINGIFY(name) "_vtable"
#define DISPATCH_CLASS_SYMBOL(name) _dispatch_##name##_vtable
#define DISPATCH_CLASS_RAW_SYMBOL_NAME(name) \
		"__dispatch_" OS_STRINGIFY(name) "_vtable"
#endif

#define DISPATCH_CLASS(name) OS_dispatch_##name
#if USE_OBJC
#define DISPATCH_OBJC_CLASS_DECL(name) \
		extern void *DISPATCH_CLASS_SYMBOL(name) \
				__asm__(DISPATCH_CLASS_RAW_SYMBOL_NAME(name))
#endif

// define a new proper class
#define OS_OBJECT_CLASS_DECL(name, ...) \
		struct name##_s; \
		struct name##_extra_vtable_s { \
			__VA_ARGS__; \
		}; \
		struct name##_vtable_s { \
			_OS_OBJECT_CLASS_HEADER(); \
			struct name##_extra_vtable_s _os_obj_vtable; \
		}; \
		OS_OBJECT_EXTRA_VTABLE_DECL(name, name) \
		extern const struct name##_vtable_s OS_OBJECT_CLASS_SYMBOL(name) \
				__asm__(OS_OBJC_CLASS_RAW_SYMBOL_NAME(OS_OBJECT_CLASS(name)))

#if OS_OBJECT_SWIFT3
#define OS_OBJECT_INTERNAL_CLASS_DECL(name, super, ...) \
		OS_OBJECT_OBJC_RUNTIME_VISIBLE \
		OS_OBJECT_DECL_IMPL_CLASS(name, OS_OBJECT_CLASS(super)); \
		OS_OBJECT_CLASS_DECL(name, ## __VA_ARGS__)
#elif OS_OBJECT_USE_OBJC
#define OS_OBJECT_INTERNAL_CLASS_DECL(name, super, ...) \
		OS_OBJECT_DECL(name); \
		OS_OBJECT_CLASS_DECL(name, ## __VA_ARGS__)
#else
#define OS_OBJECT_INTERNAL_CLASS_DECL(name, super, ...) \
		typedef struct name##_s *name##_t; \
		OS_OBJECT_CLASS_DECL(name, ## __VA_ARGS__)
#endif

#define DISPATCH_CLASS_DECL_BARE(name, cluster) \
		OS_OBJECT_CLASS_DECL(dispatch_##name, \
		DISPATCH_##cluster##_VTABLE_HEADER(dispatch_##name))

#define DISPATCH_CLASS_DECL(name, cluster) \
		_OS_OBJECT_DECL_PROTOCOL(dispatch_##name, dispatch_object) \
		_OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL(dispatch_##name, dispatch_##name) \
		DISPATCH_CLASS_DECL_BARE(name, cluster)

#define DISPATCH_SUBCLASS_DECL(name, super, ctype) \
		_OS_OBJECT_DECL_PROTOCOL(dispatch_##name, dispatch_##super); \
		_OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL(dispatch_##name, dispatch_##name) \
		OS_OBJECT_SUBCLASS_DECL(dispatch_##name, dispatch_##ctype)

#define DISPATCH_INTERNAL_CLASS_DECL(name, cluster) \
		DISPATCH_DECL(dispatch_##name); \
		DISPATCH_CLASS_DECL(name, cluster)

// define a new subclass used in a cluster
#define OS_OBJECT_SUBCLASS_DECL(name, ctype) \
		struct name##_s; \
		OS_OBJECT_EXTRA_VTABLE_DECL(name, ctype) \
		extern const struct ctype##_vtable_s OS_OBJECT_CLASS_SYMBOL(name) \
				__asm__(OS_OBJC_CLASS_RAW_SYMBOL_NAME(OS_OBJECT_CLASS(name)))

#if OS_OBJECT_SWIFT3
// define a new internal subclass used in a class cluster
#define OS_OBJECT_INTERNAL_SUBCLASS_DECL(name, super, ctype) \
		_OS_OBJECT_DECL_PROTOCOL(name, super); \
		_OS_OBJECT_DECL_SUBCLASS_INTERFACE(name, super) \
		OS_OBJECT_SUBCLASS_DECL(name, ctype)
#else
// define a new internal subclass used in a class cluster
#define OS_OBJECT_INTERNAL_SUBCLASS_DECL(name, super, ctype) \
		OS_OBJECT_DECL_SUBCLASS(name, ctype); \
		_OS_OBJECT_DECL_SUBCLASS_INTERFACE(name, super) \
		OS_OBJECT_SUBCLASS_DECL(name, ctype)
#endif

#define DISPATCH_INTERNAL_SUBCLASS_DECL(name, super, ctype) \
		OS_OBJECT_INTERNAL_SUBCLASS_DECL(dispatch_##name, dispatch_##super, \
				dispatch_##ctype)

// vtable symbols
#define OS_OBJECT_VTABLE(name)		(&OS_OBJECT_CLASS_SYMBOL(name))
#define DISPATCH_OBJC_CLASS(name)	(&DISPATCH_CLASS_SYMBOL(name))

// vtables for subclasses used in a class cluster
#if USE_OBJC
// ObjC classes and dispatch vtables are co-located via linker order and alias
// files rdar://10640168
#if OS_OBJECT_HAVE_OBJC2
#define OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(name, ctype, xdispose, dispose, ...) \
		__attribute__((section("__DATA,__objc_data"), used)) \
		const struct ctype##_extra_vtable_s \
		OS_OBJECT_EXTRA_VTABLE_SYMBOL(name) = { __VA_ARGS__ }
#define OS_OBJECT_EXTRA_VTABLE_DECL(name, ctype)
#define DISPATCH_VTABLE(name) DISPATCH_OBJC_CLASS(name)
#else
#define OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(name, ctype, xdispose, dispose, ...) \
		const struct ctype##_vtable_s \
		OS_OBJECT_EXTRA_VTABLE_SYMBOL(name) = { \
			._os_obj_objc_isa = &OS_OBJECT_CLASS_SYMBOL(name), \
			._os_obj_vtable = { __VA_ARGS__ }, \
		}
#define OS_OBJECT_EXTRA_VTABLE_DECL(name, ctype) \
		extern const struct ctype##_vtable_s \
				OS_OBJECT_EXTRA_VTABLE_SYMBOL(name);
#define DISPATCH_VTABLE(name) &OS_OBJECT_EXTRA_VTABLE_SYMBOL(dispatch_##name)
#endif // OS_OBJECT_HAVE_OBJC2
#else
#define OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(name, ctype, xdispose, dispose, ...) \
		const struct ctype##_vtable_s OS_OBJECT_CLASS_SYMBOL(name) = { \
			._os_obj_xref_dispose = xdispose, \
			._os_obj_dispose = dispose, \
			._os_obj_vtable = { __VA_ARGS__ }, \
		}
#define OS_OBJECT_EXTRA_VTABLE_DECL(name, ctype)
#define DISPATCH_VTABLE(name) DISPATCH_OBJC_CLASS(name)
#endif // USE_OBJC

// vtables for proper classes
#define OS_OBJECT_VTABLE_INSTANCE(name, xdispose, dispose, ...) \
		OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(name, name, \
				xdispose, dispose, __VA_ARGS__)

#define DISPATCH_VTABLE_INSTANCE(name, ...) \
		DISPATCH_VTABLE_SUBCLASS_INSTANCE(name, name, __VA_ARGS__)

#if USE_OBJC
#define DISPATCH_VTABLE_SUBCLASS_INSTANCE(name, ctype, ...) \
		OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(dispatch_##name, dispatch_##ctype, \
				_dispatch_xref_dispose, _dispatch_dispose, __VA_ARGS__)

#define DISPATCH_OBJECT_VTABLE_HEADER(x) \
	unsigned long const do_type; \
	void (*const do_dispose)(struct x##_s *, bool *allow_free); \
	size_t (*const do_debug)(struct x##_s *, char *, size_t); \
	void (*const do_invoke)(struct x##_s *, dispatch_invoke_context_t, \
			dispatch_invoke_flags_t)
#else
#define DISPATCH_VTABLE_SUBCLASS_INSTANCE(name, ctype, ...) \
		OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(dispatch_##name, dispatch_##ctype, \
				_dispatch_xref_dispose, _dispatch_dispose, \
				.do_kind = #name, __VA_ARGS__)

#define DISPATCH_OBJECT_VTABLE_HEADER(x) \
	unsigned long const do_type; \
	const char *const do_kind; \
	void (*const do_dispose)(struct x##_s *, bool *allow_free); \
	size_t (*const do_debug)(struct x##_s *, char *, size_t); \
	void (*const do_invoke)(struct x##_s *, dispatch_invoke_context_t, \
			dispatch_invoke_flags_t)
#endif

#define DISPATCH_QUEUE_VTABLE_HEADER(x); \
	DISPATCH_OBJECT_VTABLE_HEADER(x); \
	void (*const dq_activate)(dispatch_queue_class_t); \
	void (*const dq_wakeup)(dispatch_queue_class_t, dispatch_qos_t, \
			dispatch_wakeup_flags_t); \
	void (*const dq_push)(dispatch_queue_class_t, dispatch_object_t, \
			dispatch_qos_t)

#define dx_vtable(x) (&(x)->do_vtable->_os_obj_vtable)
#define dx_type(x) dx_vtable(x)->do_type
#define dx_metatype(x) (dx_vtable(x)->do_type & _DISPATCH_META_TYPE_MASK)
#define dx_cluster(x) (dx_vtable(x)->do_type & _DISPATCH_TYPE_CLUSTER_MASK)
#define dx_hastypeflag(x, f) (dx_vtable(x)->do_type & _DISPATCH_##f##_TYPEFLAG)
#define dx_debug(x, y, z) dx_vtable(x)->do_debug((x), (y), (z))
#define dx_dispose(x, y) dx_vtable(x)->do_dispose(x, y)
#define dx_invoke(x, y, z) dx_vtable(x)->do_invoke(x, y, z)
#define dx_push(x, y, z) dx_vtable(x)->dq_push(x, y, z)
#define dx_wakeup(x, y, z) dx_vtable(x)->dq_wakeup(x, y, z)

#define DISPATCH_OBJECT_GLOBAL_REFCNT		_OS_OBJECT_GLOBAL_REFCNT

#if OS_OBJECT_HAVE_OBJC1
#define DISPATCH_GLOBAL_OBJECT_HEADER(name) \
	.do_vtable = DISPATCH_VTABLE(name), \
	._objc_isa = DISPATCH_OBJC_CLASS(name), \
	.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT, \
	.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT
#else
#define DISPATCH_GLOBAL_OBJECT_HEADER(name) \
	.do_vtable = DISPATCH_VTABLE(name), \
	.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT, \
	.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT
#endif

#if DISPATCH_SIZEOF_PTR == 8
// the bottom nibble must not be zero, the rest of the bits should be random
// we sign extend the 64-bit version so that a better instruction encoding is
// generated on Intel
#define DISPATCH_OBJECT_LISTLESS ((void *)0xffffffff89abcdef)
#else
#define DISPATCH_OBJECT_LISTLESS ((void *)0x89abcdef)
#endif

DISPATCH_OPTIONS(dispatch_wakeup_flags, uint32_t,
	// The caller of dx_wakeup owns two internal refcounts on the object being
	// woken up. Two are needed for WLH wakeups where two threads need
	// the object to remain valid in a non-coordinated way
	// - the thread doing the poke for the duration of the poke
	// - drainers for the duration of their drain
	DISPATCH_WAKEUP_CONSUME_2               = 0x00000001,

	// Some change to the object needs to be published to drainers.
	// If the drainer isn't the same thread, some scheme such as the dispatch
	// queue DIRTY bit must be used and a release barrier likely has to be
	// involved before dx_wakeup returns
	DISPATCH_WAKEUP_MAKE_DIRTY              = 0x00000002,

	// This wakeup is made by a sync owner that still holds the drain lock
	DISPATCH_WAKEUP_BARRIER_COMPLETE        = 0x00000004,

	// This wakeup is caused by a dispatch_block_wait()
	DISPATCH_WAKEUP_BLOCK_WAIT              = 0x00000008,

	// This wakeup may cause the source to leave its DSF_NEEDS_EVENT state
	DISPATCH_WAKEUP_EVENT                   = 0x00000010,

	// This wakeup is allowed to clear the ACTIVATING state of the object
	DISPATCH_WAKEUP_CLEAR_ACTIVATING        = 0x00000020,
);

typedef struct dispatch_invoke_context_s {
#if DISPATCH_USE_WORKQUEUE_NARROWING
	uint64_t dic_next_narrow_check;
#endif
	struct dispatch_object_s *dic_barrier_waiter;
	dispatch_qos_t dic_barrier_waiter_bucket;
#if DISPATCH_COCOA_COMPAT
	void *dic_autorelease_pool;
#endif
} dispatch_invoke_context_s, *dispatch_invoke_context_t;

#if DISPATCH_USE_WORKQUEUE_NARROWING
#define DISPATCH_THREAD_IS_NARROWING 1

#define dispatch_with_disabled_narrowing(dic, ...) ({ \
		uint64_t suspend_narrow_check = dic->dic_next_narrow_check; \
		dic->dic_next_narrow_check = 0; \
		__VA_ARGS__; \
		dic->dic_next_narrow_check = suspend_narrow_check; \
	})
#else
#define dispatch_with_disabled_narrowing(dic, ...) __VA_ARGS__
#endif

DISPATCH_OPTIONS(dispatch_invoke_flags, uint32_t,
	DISPATCH_INVOKE_NONE					= 0x00000000,

	// Invoke modes
	//
	// @const DISPATCH_INVOKE_STEALING
	// This invoke is a stealer, meaning that it doesn't own the
	// enqueue lock at drain lock time.
	//
	// @const DISPATCH_INVOKE_WLH
	// This invoke is for a bottom WLH
	//
	DISPATCH_INVOKE_STEALING				= 0x00000001,
	DISPATCH_INVOKE_WLH						= 0x00000002,

	// Misc flags
	//
	// @const DISPATCH_INVOKE_ASYNC_REPLY
	// An asynchronous reply to a message is being handled.
	//
	// @const DISPATCH_INVOKE_DISALLOW_SYNC_WAITERS
	// The next serial drain should not allow sync waiters.
	//
	DISPATCH_INVOKE_ASYNC_REPLY				= 0x00000004,
	DISPATCH_INVOKE_DISALLOW_SYNC_WAITERS	= 0x00000008,

	// Below this point flags are propagated to recursive calls to drain(),
	// continuation pop() or dx_invoke().
#define _DISPATCH_INVOKE_PROPAGATE_MASK		  0xffff0000u

	// Drain modes
	//
	// @const DISPATCH_INVOKE_WORKER_DRAIN
	// Invoke has been issued by a worker thread (work queue thread, or
	// pthread root queue) drain. This flag is NOT set when the main queue,
	// manager queue or runloop queues are drained
	//
	// @const DISPATCH_INVOKE_REDIRECTING_DRAIN
	// Has only been draining concurrent queues so far
	// Implies DISPATCH_INVOKE_WORKER_DRAIN
	//
	// @const DISPATCH_INVOKE_MANAGER_DRAIN
	// We're draining from a manager context
	//
	// @const DISPATCH_INVOKE_THREAD_BOUND
	// We're draining from the context of a thread-bound queue (main thread)
	//
	// @const DISPATCH_INVOKE_WORKLOOP_DRAIN
	// The queue at the bottom of this drain is a workloop that supports
	// reordering.
	//
	DISPATCH_INVOKE_WORKER_DRAIN			= 0x00010000,
	DISPATCH_INVOKE_REDIRECTING_DRAIN		= 0x00020000,
	DISPATCH_INVOKE_MANAGER_DRAIN			= 0x00040000,
	DISPATCH_INVOKE_THREAD_BOUND			= 0x00080000,
	DISPATCH_INVOKE_WORKLOOP_DRAIN			= 0x00100000,
#define _DISPATCH_INVOKE_DRAIN_MODE_MASK	  0x00ff0000u

	// Autoreleasing modes
	//
	// @const DISPATCH_INVOKE_AUTORELEASE_ALWAYS
	// Always use autoreleasepools around callouts
	//
	// @const DISPATCH_INVOKE_AUTORELEASE_NEVER
	// Never use autoreleasepools around callouts
	//
	DISPATCH_INVOKE_AUTORELEASE_ALWAYS		= 0x01000000,
	DISPATCH_INVOKE_AUTORELEASE_NEVER		= 0x02000000,
#define _DISPATCH_INVOKE_AUTORELEASE_MASK	  0x03000000u
);

DISPATCH_OPTIONS(dispatch_object_flags, unsigned long,
	_DISPATCH_META_TYPE_MASK		= 0x000000ff, // mask for object meta-types
	_DISPATCH_TYPE_CLUSTER_MASK		= 0x000000f0, // mask for the cluster type
	_DISPATCH_SUB_TYPE_MASK			= 0x0000ff00, // mask for object sub-types
	_DISPATCH_TYPEFLAGS_MASK		= 0x00ff0000, // mask for object typeflags

	_DISPATCH_OBJECT_CLUSTER        = 0x00000000, // dispatch object cluster
	_DISPATCH_CONTINUATION_TYPE		= 0x00000000, // meta-type for continuations
	_DISPATCH_SEMAPHORE_TYPE		= 0x00000001, // meta-type for semaphores
	_DISPATCH_NODE_TYPE				= 0x00000002, // meta-type for data node
	_DISPATCH_IO_TYPE				= 0x00000003, // meta-type for io channels
	_DISPATCH_OPERATION_TYPE		= 0x00000004, // meta-type for io operations
	_DISPATCH_DISK_TYPE				= 0x00000005, // meta-type for io disks

	_DISPATCH_QUEUE_CLUSTER         = 0x00000010, // dispatch queue cluster
	_DISPATCH_LANE_TYPE				= 0x00000011, // meta-type for lanes
	_DISPATCH_WORKLOOP_TYPE			= 0x00000012, // meta-type for workloops
	_DISPATCH_SOURCE_TYPE			= 0x00000013, // meta-type for sources

	// QUEUE_ROOT is set on root queues (queues with a NULL do_targetq)
	// QUEUE_BASE is set on hierarchy bases, these always target a root queue
	// NO_CONTEXT is set on types not supporting dispatch_{get,set}_context
	_DISPATCH_QUEUE_ROOT_TYPEFLAG	= 0x00010000,
	_DISPATCH_QUEUE_BASE_TYPEFLAG	= 0x00020000,
	_DISPATCH_NO_CONTEXT_TYPEFLAG	= 0x00040000,

#define DISPATCH_OBJECT_SUBTYPE(ty, base) (_DISPATCH_##base##_TYPE | (ty) << 8)
#define DISPATCH_CONTINUATION_TYPE(name) \
		DISPATCH_OBJECT_SUBTYPE(DC_##name##_TYPE, CONTINUATION)

	DISPATCH_SEMAPHORE_TYPE				= DISPATCH_OBJECT_SUBTYPE(1, SEMAPHORE),
	DISPATCH_GROUP_TYPE					= DISPATCH_OBJECT_SUBTYPE(2, SEMAPHORE),

	DISPATCH_DATA_TYPE					= DISPATCH_OBJECT_SUBTYPE(1, NODE),
	DISPATCH_MACH_MSG_TYPE				= DISPATCH_OBJECT_SUBTYPE(2, NODE),
	DISPATCH_QUEUE_ATTR_TYPE			= DISPATCH_OBJECT_SUBTYPE(3, NODE),

	DISPATCH_IO_TYPE					= DISPATCH_OBJECT_SUBTYPE(0, IO),
	DISPATCH_OPERATION_TYPE				= DISPATCH_OBJECT_SUBTYPE(0, OPERATION),
	DISPATCH_DISK_TYPE					= DISPATCH_OBJECT_SUBTYPE(0, DISK),

	DISPATCH_QUEUE_SERIAL_TYPE			= DISPATCH_OBJECT_SUBTYPE(1, LANE),
	DISPATCH_QUEUE_CONCURRENT_TYPE		= DISPATCH_OBJECT_SUBTYPE(2, LANE),
	DISPATCH_QUEUE_GLOBAL_ROOT_TYPE		= DISPATCH_OBJECT_SUBTYPE(3, LANE) |
			_DISPATCH_QUEUE_ROOT_TYPEFLAG | _DISPATCH_NO_CONTEXT_TYPEFLAG,
	DISPATCH_QUEUE_PTHREAD_ROOT_TYPE	= DISPATCH_OBJECT_SUBTYPE(4, LANE) |
			_DISPATCH_QUEUE_ROOT_TYPEFLAG | _DISPATCH_NO_CONTEXT_TYPEFLAG,
	DISPATCH_QUEUE_MGR_TYPE				= DISPATCH_OBJECT_SUBTYPE(5, LANE) |
			_DISPATCH_QUEUE_BASE_TYPEFLAG | _DISPATCH_NO_CONTEXT_TYPEFLAG,
	DISPATCH_QUEUE_MAIN_TYPE			= DISPATCH_OBJECT_SUBTYPE(6, LANE) |
			_DISPATCH_QUEUE_BASE_TYPEFLAG | _DISPATCH_NO_CONTEXT_TYPEFLAG,
	DISPATCH_QUEUE_RUNLOOP_TYPE			= DISPATCH_OBJECT_SUBTYPE(7, LANE) |
			_DISPATCH_QUEUE_BASE_TYPEFLAG | _DISPATCH_NO_CONTEXT_TYPEFLAG,
	DISPATCH_QUEUE_NETWORK_EVENT_TYPE	= DISPATCH_OBJECT_SUBTYPE(8, LANE) |
			_DISPATCH_QUEUE_BASE_TYPEFLAG,

	DISPATCH_WORKLOOP_TYPE				= DISPATCH_OBJECT_SUBTYPE(0, WORKLOOP) |
			_DISPATCH_QUEUE_BASE_TYPEFLAG,

	DISPATCH_SOURCE_KEVENT_TYPE			= DISPATCH_OBJECT_SUBTYPE(1, SOURCE),
	DISPATCH_CHANNEL_TYPE				= DISPATCH_OBJECT_SUBTYPE(2, SOURCE),
	DISPATCH_MACH_CHANNEL_TYPE			= DISPATCH_OBJECT_SUBTYPE(3, SOURCE),
);

typedef struct _os_object_vtable_s {
	_OS_OBJECT_CLASS_HEADER();
} _os_object_vtable_s;

typedef struct _os_object_s {
	_OS_OBJECT_HEADER(
	const _os_object_vtable_s *os_obj_isa,
	os_obj_ref_cnt,
	os_obj_xref_cnt);
} _os_object_s;

#if OS_OBJECT_HAVE_OBJC1
#define OS_OBJECT_STRUCT_HEADER(x) \
	_OS_OBJECT_HEADER(\
	const void *_objc_isa, \
	do_ref_cnt, \
	do_xref_cnt); \
	const struct x##_vtable_s *do_vtable
#else
#define OS_OBJECT_STRUCT_HEADER(x) \
	_OS_OBJECT_HEADER(\
	const struct x##_vtable_s *do_vtable, \
	do_ref_cnt, \
	do_xref_cnt)
#endif

#define _DISPATCH_OBJECT_HEADER(x) \
	struct _os_object_s _as_os_obj[0]; \
	OS_OBJECT_STRUCT_HEADER(dispatch_##x); \
	struct dispatch_##x##_s *volatile do_next; \
	struct dispatch_queue_s *do_targetq; \
	void *do_ctxt; \
	void *do_finalizer

#define DISPATCH_OBJECT_HEADER(x) \
	struct dispatch_object_s _as_do[0]; \
	_DISPATCH_OBJECT_HEADER(x)

// Swift-unavailable -init requires method in each class.
#define DISPATCH_UNAVAILABLE_INIT() \
	- (instancetype)init { \
		DISPATCH_CLIENT_CRASH(0, "-init called directly"); \
		return [super init]; \
	}

#define DISPATCH_OBJECT_USES_XREF_DISPOSE() \
		OS_OBJECT_USES_XREF_DISPOSE()

_OS_OBJECT_DECL_PROTOCOL(dispatch_object, object);
DISPATCH_CLASS_DECL_BARE(object, OBJECT);

struct dispatch_object_s {
	_DISPATCH_OBJECT_HEADER(object);
};

DISPATCH_COLD
size_t _dispatch_object_debug_attr(dispatch_object_t dou, char* buf,
		size_t bufsiz);
void *_dispatch_object_alloc(const void *vtable, size_t size);
void _dispatch_object_finalize(dispatch_object_t dou);
void _dispatch_object_dealloc(dispatch_object_t dou);
void _dispatch_xref_dispose(dispatch_object_t dou);
void _dispatch_dispose(dispatch_object_t dou);
#if DISPATCH_COCOA_COMPAT
#if USE_OBJC
#include <objc/runtime.h>
#if __has_include(<objc/objc-internal.h>)
#include <objc/objc-internal.h>
#else
extern void *objc_autoreleasePoolPush(void);
extern void objc_autoreleasePoolPop(void *context);
#endif // __has_include(<objc/objc-internal.h>)
#define _dispatch_autorelease_pool_push() \
		objc_autoreleasePoolPush()
#define _dispatch_autorelease_pool_pop(context) \
		objc_autoreleasePoolPop(context)
#else
void *_dispatch_autorelease_pool_push(void);
void _dispatch_autorelease_pool_pop(void *context);
#endif
void _dispatch_last_resort_autorelease_pool_push(dispatch_invoke_context_t dic);
void _dispatch_last_resort_autorelease_pool_pop(dispatch_invoke_context_t dic);

#define dispatch_invoke_with_autoreleasepool(flags, ...)  ({ \
		void *pool = NULL; \
		if ((flags) & DISPATCH_INVOKE_AUTORELEASE_ALWAYS) { \
			pool = _dispatch_autorelease_pool_push(); \
			DISPATCH_COMPILER_CAN_ASSUME(pool); \
		}; \
		__VA_ARGS__; \
		if (pool) _dispatch_autorelease_pool_pop(pool); \
	})
#else
#define dispatch_invoke_with_autoreleasepool(flags, ...) \
	do { (void)flags; __VA_ARGS__; } while (0)
#endif

#if USE_OBJC
OS_OBJECT_OBJC_CLASS_DECL(object);
#endif

#if OS_OBJECT_HAVE_OBJC2
// ObjC toll-free bridging, keep in sync with libdispatch.order file
//
// This is required by the dispatch_data_t/NSData bridging, which is not
// supported on the old runtime.
#define DISPATCH_OBJECT_TFB(f, o, ...) \
	if (unlikely(((uintptr_t)((o)._os_obj->os_obj_isa) & 1) || \
			(Class)((o)._os_obj->os_obj_isa) < \
					(Class)OS_OBJECT_VTABLE(dispatch_object) || \
			(Class)((o)._os_obj->os_obj_isa) >= \
					(Class)OS_OBJECT_VTABLE(object))) { \
		return f((o), ##__VA_ARGS__); \
	}

id _dispatch_objc_alloc(Class cls, size_t size);
void _dispatch_objc_retain(dispatch_object_t dou);
void _dispatch_objc_release(dispatch_object_t dou);
void _dispatch_objc_set_context(dispatch_object_t dou, void *context);
void *_dispatch_objc_get_context(dispatch_object_t dou);
void _dispatch_objc_set_finalizer_f(dispatch_object_t dou,
		dispatch_function_t finalizer);
void _dispatch_objc_set_target_queue(dispatch_object_t dou,
		dispatch_queue_t queue);
void _dispatch_objc_suspend(dispatch_object_t dou);
void _dispatch_objc_resume(dispatch_object_t dou);
void _dispatch_objc_activate(dispatch_object_t dou);
DISPATCH_COLD
size_t _dispatch_objc_debug(dispatch_object_t dou, char* buf, size_t bufsiz);

#if __OBJC2__
@interface NSObject (DISPATCH_CONCAT(_,DISPATCH_CLASS(object)))
- (void)_setContext:(void*)context;
- (void*)_getContext;
- (void)_setFinalizer:(dispatch_function_t)finalizer;
- (void)_setTargetQueue:(dispatch_queue_t)queue;
- (void)_suspend;
- (void)_resume;
- (void)_activate;
@end
#endif // __OBJC2__
#else
#define DISPATCH_OBJECT_TFB(f, o, ...)
#endif // OS_OBJECT_HAVE_OBJC2

#pragma mark -
#pragma mark _os_object_s

/*
 * Low level _os_atomic_refcnt_* actions
 *
 * _os_atomic_refcnt_inc2o(o, f):
 *   performs a refcount increment and returns the new refcount value
 *
 * _os_atomic_refcnt_dec2o(o, f):
 *   performs a refcount decrement and returns the new refcount value
 *
 * _os_atomic_refcnt_dispose_barrier2o(o, f):
 *   a barrier to perform prior to tearing down an object when the refcount
 *   reached -1.
 */
#define _os_atomic_refcnt_perform2o(o, f, op, n, m)   ({ \
		__typeof__(o) _o = (o); \
		int _ref_cnt = _o->f; \
		if (likely(_ref_cnt != _OS_OBJECT_GLOBAL_REFCNT)) { \
			_ref_cnt = os_atomic_##op##2o(_o, f, n, m); \
		} \
		_ref_cnt; \
	})

#define _os_atomic_refcnt_add_orig2o(o, m, n) \
		_os_atomic_refcnt_perform2o(o, m, add_orig, n, relaxed)

#define _os_atomic_refcnt_sub2o(o, m, n) \
		_os_atomic_refcnt_perform2o(o, m, sub, n, release)

#define _os_atomic_refcnt_dispose_barrier2o(o, m) \
		(void)os_atomic_load2o(o, m, acquire)


/*
 * Higher level _os_object_{x,}refcnt_* actions
 *
 * _os_atomic_{x,}refcnt_inc_orig(o):
 *   increment the external (resp. internal) refcount and
 *   returns the old refcount value
 *
 * _os_atomic_{x,}refcnt_dec(o):
 *   decrement the external (resp. internal) refcount and
 *   returns the new refcount value
 *
 * _os_atomic_{x,}refcnt_dispose_barrier(o):
 *   performs the pre-teardown barrier for the external
 *   (resp. internal) refcount
 *
 */
#define _os_object_xrefcnt_inc_orig(o) \
		_os_atomic_refcnt_add_orig2o(o, os_obj_xref_cnt, 1)

#define _os_object_xrefcnt_dec(o) \
		_os_atomic_refcnt_sub2o(o, os_obj_xref_cnt, 1)

#define _os_object_xrefcnt_dispose_barrier(o) \
		_os_atomic_refcnt_dispose_barrier2o(o, os_obj_xref_cnt)

#define _os_object_refcnt_add_orig(o, n) \
		_os_atomic_refcnt_add_orig2o(o, os_obj_ref_cnt, n)

#define _os_object_refcnt_sub(o, n) \
		_os_atomic_refcnt_sub2o(o, os_obj_ref_cnt, n)

#define _os_object_refcnt_dispose_barrier(o) \
		_os_atomic_refcnt_dispose_barrier2o(o, os_obj_ref_cnt)

void _os_object_atfork_child(void);
void _os_object_atfork_parent(void);
void _os_object_atfork_prepare(void);
void _os_object_init(void);
unsigned long _os_object_retain_count(_os_object_t obj);
bool _os_object_retain_weak(_os_object_t obj);
bool _os_object_allows_weak_reference(_os_object_t obj);
void _os_object_dispose(_os_object_t obj);
void _os_object_xref_dispose(_os_object_t obj);

#endif // __DISPATCH_OBJECT_INTERNAL__
