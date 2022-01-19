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

#if defined(__BLOCKS__) && !defined(DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES)
#define DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES 1 // <rdar://problem/10719357>
#endif

/* x86 & cortex-a8 have a 64 byte cacheline */
#define DISPATCH_CACHELINE_SIZE 64u
#define ROUND_UP_TO_CACHELINE_SIZE(x) \
		(((x) + (DISPATCH_CACHELINE_SIZE - 1u)) & \
		~(DISPATCH_CACHELINE_SIZE - 1u))
#define DISPATCH_CACHELINE_ALIGN \
		__attribute__((__aligned__(DISPATCH_CACHELINE_SIZE)))


#pragma mark -
#pragma mark dispatch_queue_t

#define DISPATCH_QUEUE_HEADER \
	uint32_t volatile dq_running; \
	struct dispatch_object_s *volatile dq_items_head; \
	/* LP64 global queue cacheline boundary */ \
	struct dispatch_object_s *volatile dq_items_tail; \
	dispatch_queue_t dq_specific_q; \
	uint16_t dq_width; \
	uint16_t dq_is_thread_bound:1; \
	pthread_priority_t dq_priority; \
	mach_port_t dq_thread; \
	mach_port_t volatile dq_tqthread; \
	uint32_t volatile dq_override; \
	unsigned long dq_serialnum; \
	const char *dq_label; \
	DISPATCH_INTROSPECTION_QUEUE_LIST;

#define DISPATCH_QUEUE_WIDTH_MAX UINT16_MAX

#define DISPATCH_QUEUE_CACHELINE_PADDING \
		char _dq_pad[DISPATCH_QUEUE_CACHELINE_PAD]
#ifdef __LP64__
#define DISPATCH_QUEUE_CACHELINE_PAD (( \
		(0*sizeof(void*) - DISPATCH_INTROSPECTION_QUEUE_LIST_SIZE) \
		+ DISPATCH_CACHELINE_SIZE) % DISPATCH_CACHELINE_SIZE)
#else
#define DISPATCH_QUEUE_CACHELINE_PAD (( \
		(13*sizeof(void*) - DISPATCH_INTROSPECTION_QUEUE_LIST_SIZE) \
		+ DISPATCH_CACHELINE_SIZE) % DISPATCH_CACHELINE_SIZE)
#endif

DISPATCH_CLASS_DECL(queue);
struct dispatch_queue_s {
	DISPATCH_STRUCT_HEADER(queue);
	DISPATCH_QUEUE_HEADER;
	DISPATCH_QUEUE_CACHELINE_PADDING; // for static queues only
};

DISPATCH_INTERNAL_SUBCLASS_DECL(queue_root, queue);
DISPATCH_INTERNAL_SUBCLASS_DECL(queue_runloop, queue);
DISPATCH_INTERNAL_SUBCLASS_DECL(queue_mgr, queue);

DISPATCH_DECL_INTERNAL_SUBCLASS(dispatch_queue_specific_queue, dispatch_queue);
DISPATCH_CLASS_DECL(queue_specific_queue);

void _dispatch_queue_destroy(dispatch_object_t dou);
void _dispatch_queue_dispose(dispatch_queue_t dq);
void _dispatch_queue_invoke(dispatch_queue_t dq);
void _dispatch_queue_push_list_slow(dispatch_queue_t dq,
		pthread_priority_t pp, struct dispatch_object_s *obj, unsigned int n,
		bool retained);
void _dispatch_queue_push_slow(dispatch_queue_t dq,
		pthread_priority_t pp, struct dispatch_object_s *obj, bool retained);
unsigned long _dispatch_queue_probe(dispatch_queue_t dq);
dispatch_queue_t _dispatch_wakeup(dispatch_object_t dou);
dispatch_queue_t _dispatch_queue_wakeup(dispatch_queue_t dq);
void _dispatch_queue_wakeup_with_qos(dispatch_queue_t dq,
		pthread_priority_t pp);
void _dispatch_queue_wakeup_with_qos_and_release(dispatch_queue_t dq,
		pthread_priority_t pp);
_dispatch_thread_semaphore_t _dispatch_queue_drain(dispatch_object_t dou);
void _dispatch_queue_specific_queue_dispose(dispatch_queue_specific_queue_t
		dqsq);
unsigned long _dispatch_root_queue_probe(dispatch_queue_t dq);
void _dispatch_pthread_root_queue_dispose(dispatch_queue_t dq);
unsigned long _dispatch_runloop_queue_probe(dispatch_queue_t dq);
void _dispatch_runloop_queue_xref_dispose(dispatch_queue_t dq);
void _dispatch_runloop_queue_dispose(dispatch_queue_t dq);
void _dispatch_mgr_queue_drain(void);
unsigned long _dispatch_mgr_queue_probe(dispatch_queue_t dq);
#if DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES
void _dispatch_mgr_priority_init(void);
#else
static inline void _dispatch_mgr_priority_init(void) {}
#endif
void _dispatch_after_timer_callback(void *ctxt);
void _dispatch_async_redirect_invoke(void *ctxt);
void _dispatch_sync_recurse_invoke(void *ctxt);
void _dispatch_apply_invoke(void *ctxt);
void _dispatch_apply_redirect_invoke(void *ctxt);
void _dispatch_barrier_async_detached_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func);
void _dispatch_barrier_trysync_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func);

#if DISPATCH_DEBUG
void dispatch_debug_queue(dispatch_queue_t dq, const char* str);
#else
static inline void dispatch_debug_queue(dispatch_queue_t dq DISPATCH_UNUSED,
		const char* str DISPATCH_UNUSED) {}
#endif

size_t dispatch_queue_debug(dispatch_queue_t dq, char* buf, size_t bufsiz);
size_t _dispatch_queue_debug_attr(dispatch_queue_t dq, char* buf,
		size_t bufsiz);

#define DISPATCH_QUEUE_QOS_COUNT 6
#define DISPATCH_ROOT_QUEUE_COUNT (DISPATCH_QUEUE_QOS_COUNT * 2)

// must be in lowest to highest qos order (as encoded in pthread_priority_t)
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
};

extern unsigned long volatile _dispatch_queue_serial_numbers;
extern struct dispatch_queue_s _dispatch_root_queues[];
extern struct dispatch_queue_s _dispatch_mgr_q;

#if HAVE_PTHREAD_WORKQUEUE_QOS
extern pthread_priority_t _dispatch_background_priority;
extern pthread_priority_t _dispatch_user_initiated_priority;
#endif

#pragma mark -
#pragma mark dispatch_queue_attr_t

DISPATCH_CLASS_DECL(queue_attr);
struct dispatch_queue_attr_s {
	DISPATCH_STRUCT_HEADER(queue_attr);
	qos_class_t dqa_qos_class;
	int dqa_relative_priority;
	unsigned int dqa_overcommit:1, dqa_concurrent:1;
};

enum {
	DQA_INDEX_NON_OVERCOMMIT = 0,
	DQA_INDEX_OVERCOMMIT,
};

enum {
	DQA_INDEX_CONCURRENT = 0,
	DQA_INDEX_SERIAL,
};

#define DISPATCH_QUEUE_ATTR_PRIO_COUNT (1 - QOS_MIN_RELATIVE_PRIORITY)

typedef enum {
	DQA_INDEX_QOS_CLASS_UNSPECIFIED = 0,
	DQA_INDEX_QOS_CLASS_MAINTENANCE,
	DQA_INDEX_QOS_CLASS_BACKGROUND,
	DQA_INDEX_QOS_CLASS_UTILITY,
	DQA_INDEX_QOS_CLASS_DEFAULT,
	DQA_INDEX_QOS_CLASS_USER_INITIATED,
	DQA_INDEX_QOS_CLASS_USER_INTERACTIVE,
} _dispatch_queue_attr_index_qos_class_t;

extern const struct dispatch_queue_attr_s _dispatch_queue_attrs[]
		[DISPATCH_QUEUE_ATTR_PRIO_COUNT][2][2];

#pragma mark -
#pragma mark dispatch_continuation_t

// If dc_vtable is less than 127, then the object is a continuation.
// Otherwise, the object has a private layout and memory management rules. The
// layout until after 'do_next' must align with normal objects.
#if __LP64__
#define DISPATCH_CONTINUATION_HEADER(x) \
	const void *do_vtable; \
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
	void *dc_other;
#define _DISPATCH_SIZEOF_PTR 8
#else
#define DISPATCH_CONTINUATION_HEADER(x) \
	const void *do_vtable; \
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
	void *dc_other;
#define _DISPATCH_SIZEOF_PTR 4
#endif
#define _DISPATCH_CONTINUATION_PTRS 8
#if DISPATCH_HW_CONFIG_UP
// UP devices don't contend on continuations so we don't need to force them to
// occupy a whole cacheline (which is intended to avoid contention)
#define DISPATCH_CONTINUATION_SIZE \
		(_DISPATCH_CONTINUATION_PTRS * _DISPATCH_SIZEOF_PTR)
#else
#define DISPATCH_CONTINUATION_SIZE  ROUND_UP_TO_CACHELINE_SIZE( \
		(_DISPATCH_CONTINUATION_PTRS * _DISPATCH_SIZEOF_PTR))
#endif
#define ROUND_UP_TO_CONTINUATION_SIZE(x) \
		(((x) + (DISPATCH_CONTINUATION_SIZE - 1u)) & \
		~(DISPATCH_CONTINUATION_SIZE - 1u))

#define DISPATCH_OBJ_ASYNC_BIT		0x1
#define DISPATCH_OBJ_BARRIER_BIT	0x2
#define DISPATCH_OBJ_GROUP_BIT		0x4
#define DISPATCH_OBJ_SYNC_SLOW_BIT	0x8
#define DISPATCH_OBJ_BLOCK_RELEASE_BIT 0x10
#define DISPATCH_OBJ_CTXT_FETCH_BIT 0x20
#define DISPATCH_OBJ_HAS_VOUCHER_BIT 0x80
// vtables are pointers far away from the low page in memory
#define DISPATCH_OBJ_IS_VTABLE(x) ((unsigned long)(x)->do_vtable > 0xfful)

struct dispatch_continuation_s {
	DISPATCH_CONTINUATION_HEADER(continuation);
};
typedef struct dispatch_continuation_s *dispatch_continuation_t;

#ifndef DISPATCH_CONTINUATION_CACHE_LIMIT
#if TARGET_OS_EMBEDDED
#define DISPATCH_CONTINUATION_CACHE_LIMIT 112 // one 256k heap for 64 threads
#define DISPATCH_CONTINUATION_CACHE_LIMIT_MEMORYSTATUS_PRESSURE_WARN 16
#else
#define DISPATCH_CONTINUATION_CACHE_LIMIT 65536
#define DISPATCH_CONTINUATION_CACHE_LIMIT_MEMORYSTATUS_PRESSURE_WARN 128
#endif
#endif

dispatch_continuation_t _dispatch_continuation_alloc_from_heap(void);
void _dispatch_continuation_free_to_heap(dispatch_continuation_t c);

#if DISPATCH_USE_MEMORYSTATUS_SOURCE
extern int _dispatch_continuation_cache_limit;
void _dispatch_continuation_free_to_cache_limit(dispatch_continuation_t c);
#else
#define _dispatch_continuation_cache_limit DISPATCH_CONTINUATION_CACHE_LIMIT
#define _dispatch_continuation_free_to_cache_limit(c) \
		_dispatch_continuation_free_to_heap(c)
#endif

#pragma mark -
#pragma mark dispatch_apply_t

struct dispatch_apply_s {
	size_t volatile da_index, da_todo;
	size_t da_iterations, da_nested;
	dispatch_continuation_t da_dc;
	_dispatch_thread_semaphore_t da_sema;
	uint32_t da_thr_cnt;
};
typedef struct dispatch_apply_s *dispatch_apply_t;

#pragma mark -
#pragma mark dispatch_block_t

#ifdef __BLOCKS__

#define DISPATCH_BLOCK_API_MASK (0x80u - 1)
#define DISPATCH_BLOCK_HAS_VOUCHER (1u << 31)
#define DISPATCH_BLOCK_HAS_PRIORITY (1u << 30)

struct dispatch_block_private_data_s {
	unsigned long dbpd_magic;
	dispatch_block_flags_t dbpd_flags;
	unsigned int volatile dbpd_atomic_flags;
	int volatile dbpd_performed;
	pthread_priority_t dbpd_priority;
	voucher_t dbpd_voucher;
	dispatch_block_t dbpd_block;
	struct dispatch_semaphore_s dbpd_group;
	dispatch_queue_t volatile dbpd_queue;
	mach_port_t dbpd_thread;
};
typedef struct dispatch_block_private_data_s *dispatch_block_private_data_t;

// dbpd_atomic_flags bits
#define DBF_CANCELED 1u // block has been cancelled
#define DBF_WAITING 2u // dispatch_block_wait has begun
#define DBF_WAITED 4u // dispatch_block_wait has finished without timeout
#define DBF_PERFORM 8u // dispatch_block_perform: don't group_leave

#define DISPATCH_BLOCK_PRIVATE_DATA_MAGIC 0xD159B10C // 0xDISPatch_BLOCk

#define DISPATCH_BLOCK_PRIVATE_DATA_INITIALIZER(flags, voucher, prio, block) \
		{ \
			.dbpd_magic = DISPATCH_BLOCK_PRIVATE_DATA_MAGIC, \
			.dbpd_flags = (flags), \
			.dbpd_priority = (prio), \
			.dbpd_voucher = (voucher), \
			.dbpd_block = (block), \
			.dbpd_group = DISPATCH_GROUP_INITIALIZER(1), \
		}

dispatch_block_t _dispatch_block_create(dispatch_block_flags_t flags,
		voucher_t voucher, pthread_priority_t priority, dispatch_block_t block);
void _dispatch_block_invoke(const struct dispatch_block_private_data_s *dbcpd);

#endif /* __BLOCKS__ */

#endif
