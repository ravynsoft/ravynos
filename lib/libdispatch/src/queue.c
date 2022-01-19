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


#include "internal.h"
#if HAVE_MACH
#include "protocol.h"
#endif

#if (!HAVE_PTHREAD_WORKQUEUES || DISPATCH_DEBUG) && \
		!defined(DISPATCH_ENABLE_THREAD_POOL)
#define DISPATCH_ENABLE_THREAD_POOL 1
#endif
#if DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES || DISPATCH_ENABLE_THREAD_POOL
#define DISPATCH_USE_PTHREAD_POOL 1
#endif
#if HAVE_PTHREAD_WORKQUEUES && (!HAVE_PTHREAD_WORKQUEUE_QOS || DISPATCH_DEBUG) \
		&& !defined(DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK)
#define DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK 1
#endif
#if HAVE_PTHREAD_WORKQUEUES && DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK && \
		!HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP && \
		!defined(DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK)
#define DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK 1
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS && !DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
#undef HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
#define HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP 0
#endif
#if HAVE_PTHREAD_WORKQUEUES && DISPATCH_USE_PTHREAD_POOL && \
		!DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
#define pthread_workqueue_t void*
#endif

static void _dispatch_cache_cleanup(void *value);
static void _dispatch_async_f_redirect(dispatch_queue_t dq,
		dispatch_continuation_t dc, pthread_priority_t pp);
static void _dispatch_queue_cleanup(void *ctxt);
static inline void _dispatch_queue_wakeup_global2(dispatch_queue_t dq,
		unsigned int n);
static inline void _dispatch_queue_wakeup_global(dispatch_queue_t dq);
static inline _dispatch_thread_semaphore_t
		_dispatch_queue_drain_one_barrier_sync(dispatch_queue_t dq);
static inline bool _dispatch_queue_prepare_override(dispatch_queue_t dq,
		dispatch_queue_t tq, pthread_priority_t p);
static inline void _dispatch_queue_push_override(dispatch_queue_t dq,
		dispatch_queue_t tq, pthread_priority_t p);
#if HAVE_PTHREAD_WORKQUEUES
static void _dispatch_worker_thread4(void *context);
#if HAVE_PTHREAD_WORKQUEUE_QOS
static void _dispatch_worker_thread3(pthread_priority_t priority);
#endif
#if HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
static void _dispatch_worker_thread2(int priority, int options, void *context);
#endif
#endif
#if DISPATCH_USE_PTHREAD_POOL
static void *_dispatch_worker_thread(void *context);
static int _dispatch_pthread_sigmask(int how, sigset_t *set, sigset_t *oset);
#endif

#if DISPATCH_COCOA_COMPAT
static dispatch_once_t _dispatch_main_q_port_pred;
static dispatch_queue_t _dispatch_main_queue_wakeup(void);
unsigned long _dispatch_runloop_queue_wakeup(dispatch_queue_t dq);
static void _dispatch_runloop_queue_port_init(void *ctxt);
static void _dispatch_runloop_queue_port_dispose(dispatch_queue_t dq);
#endif

static void _dispatch_root_queues_init(void *context);
static dispatch_once_t _dispatch_root_queues_pred;

#pragma mark -
#pragma mark dispatch_root_queue

struct dispatch_pthread_root_queue_context_s {
	pthread_attr_t dpq_thread_attr;
	dispatch_block_t dpq_thread_configure;
	struct dispatch_semaphore_s dpq_thread_mediator;
};
typedef struct dispatch_pthread_root_queue_context_s *
		dispatch_pthread_root_queue_context_t;

#if DISPATCH_ENABLE_THREAD_POOL
static struct dispatch_pthread_root_queue_context_s
		_dispatch_pthread_root_queue_contexts[] = {
	[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS_OVERCOMMIT] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT] = {
		.dpq_thread_mediator = {
			.do_vtable = DISPATCH_VTABLE(semaphore),
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	}},
};
#endif

#define MAX_PTHREAD_COUNT 255

struct dispatch_root_queue_context_s {
	union {
		struct {
			unsigned int volatile dgq_pending;
#if HAVE_PTHREAD_WORKQUEUES
			qos_class_t dgq_qos;
			int dgq_wq_priority, dgq_wq_options;
#if DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK || DISPATCH_USE_PTHREAD_POOL
			pthread_workqueue_t dgq_kworkqueue;
#endif
#endif // HAVE_PTHREAD_WORKQUEUES
#if DISPATCH_USE_PTHREAD_POOL
			void *dgq_ctxt;
			uint32_t volatile dgq_thread_pool_size;
#endif
		};
		char _dgq_pad[DISPATCH_CACHELINE_SIZE];
	};
};
typedef struct dispatch_root_queue_context_s *dispatch_root_queue_context_t;

DISPATCH_CACHELINE_ALIGN
static struct dispatch_root_queue_context_s _dispatch_root_queue_contexts[] = {
	[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_MAINTENANCE,
		.dgq_wq_priority = WORKQ_BG_PRIOQUEUE,
		.dgq_wq_options = 0,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS_OVERCOMMIT] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_MAINTENANCE,
		.dgq_wq_priority = WORKQ_BG_PRIOQUEUE,
		.dgq_wq_options = WORKQ_ADDTHREADS_OPTION_OVERCOMMIT,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS_OVERCOMMIT],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_BACKGROUND,
		.dgq_wq_priority = WORKQ_BG_PRIOQUEUE,
		.dgq_wq_options = 0,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_BACKGROUND,
		.dgq_wq_priority = WORKQ_BG_PRIOQUEUE,
		.dgq_wq_options = WORKQ_ADDTHREADS_OPTION_OVERCOMMIT,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_UTILITY,
		.dgq_wq_priority = WORKQ_LOW_PRIOQUEUE,
		.dgq_wq_options = 0,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_UTILITY,
		.dgq_wq_priority = WORKQ_LOW_PRIOQUEUE,
		.dgq_wq_options = WORKQ_ADDTHREADS_OPTION_OVERCOMMIT,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_DEFAULT,
		.dgq_wq_priority = WORKQ_DEFAULT_PRIOQUEUE,
		.dgq_wq_options = 0,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_DEFAULT,
		.dgq_wq_priority = WORKQ_DEFAULT_PRIOQUEUE,
		.dgq_wq_options = WORKQ_ADDTHREADS_OPTION_OVERCOMMIT,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_USER_INITIATED,
		.dgq_wq_priority = WORKQ_HIGH_PRIOQUEUE,
		.dgq_wq_options = 0,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_USER_INITIATED,
		.dgq_wq_priority = WORKQ_HIGH_PRIOQUEUE,
		.dgq_wq_options = WORKQ_ADDTHREADS_OPTION_OVERCOMMIT,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_USER_INTERACTIVE,
		.dgq_wq_priority = WORKQ_HIGH_PRIOQUEUE,
		.dgq_wq_options = 0,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS],
#endif
	}}},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT] = {{{
#if HAVE_PTHREAD_WORKQUEUES
		.dgq_qos = _DISPATCH_QOS_CLASS_USER_INTERACTIVE,
		.dgq_wq_priority = WORKQ_HIGH_PRIOQUEUE,
		.dgq_wq_options = WORKQ_ADDTHREADS_OPTION_OVERCOMMIT,
#endif
#if DISPATCH_ENABLE_THREAD_POOL
		.dgq_ctxt = &_dispatch_pthread_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT],
#endif
	}}},
};

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
// dq_running is set to 2 so that barrier operations go through the slow path
DISPATCH_CACHELINE_ALIGN
struct dispatch_queue_s _dispatch_root_queues[] = {
	[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS],
		.dq_label = "com.apple.root.maintenance-qos",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 4,
	},
	[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS_OVERCOMMIT] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS_OVERCOMMIT],
		.dq_label = "com.apple.root.maintenance-qos.overcommit",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 5,
	},
	[DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS],
		.dq_label = "com.apple.root.background-qos",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 6,
	},
	[DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT],
		.dq_label = "com.apple.root.background-qos.overcommit",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 7,
	},
	[DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS],
		.dq_label = "com.apple.root.utility-qos",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 8,
	},
	[DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT],
		.dq_label = "com.apple.root.utility-qos.overcommit",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 9,
	},
	[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS],
		.dq_label = "com.apple.root.default-qos",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 10,
	},
	[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT],
		.dq_label = "com.apple.root.default-qos.overcommit",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 11,
	},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS],
		.dq_label = "com.apple.root.user-initiated-qos",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 12,
	},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT],
		.dq_label = "com.apple.root.user-initiated-qos.overcommit",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 13,
	},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS],
		.dq_label = "com.apple.root.user-interactive-qos",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 14,
	},
	[DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT] = {
		.do_vtable = DISPATCH_VTABLE(queue_root),
		.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
		.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
		.do_ctxt = &_dispatch_root_queue_contexts[
				DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT],
		.dq_label = "com.apple.root.user-interactive-qos.overcommit",
		.dq_running = 2,
		.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
		.dq_serialnum = 15,
	},
};

#if HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
static const dispatch_queue_t _dispatch_wq2root_queues[][2] = {
	[WORKQ_BG_PRIOQUEUE][0] = &_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS],
	[WORKQ_BG_PRIOQUEUE][WORKQ_ADDTHREADS_OPTION_OVERCOMMIT] =
			&_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS_OVERCOMMIT],
	[WORKQ_LOW_PRIOQUEUE][0] = &_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS],
	[WORKQ_LOW_PRIOQUEUE][WORKQ_ADDTHREADS_OPTION_OVERCOMMIT] =
			&_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_UTILITY_QOS_OVERCOMMIT],
	[WORKQ_DEFAULT_PRIOQUEUE][0] = &_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS],
	[WORKQ_DEFAULT_PRIOQUEUE][WORKQ_ADDTHREADS_OPTION_OVERCOMMIT] =
			&_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT],
	[WORKQ_HIGH_PRIOQUEUE][0] = &_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS],
	[WORKQ_HIGH_PRIOQUEUE][WORKQ_ADDTHREADS_OPTION_OVERCOMMIT] =
			&_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS_OVERCOMMIT],
};
#endif // HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP

#define DISPATCH_PRIORITY_COUNT 5

enum {
	// No DISPATCH_PRIORITY_IDX_MAINTENANCE define because there is no legacy
	// maintenance priority
	DISPATCH_PRIORITY_IDX_BACKGROUND = 0,
	DISPATCH_PRIORITY_IDX_NON_INTERACTIVE,
	DISPATCH_PRIORITY_IDX_LOW,
	DISPATCH_PRIORITY_IDX_DEFAULT,
	DISPATCH_PRIORITY_IDX_HIGH,
};

static qos_class_t _dispatch_priority2qos[] = {
	[DISPATCH_PRIORITY_IDX_BACKGROUND] = _DISPATCH_QOS_CLASS_BACKGROUND,
	[DISPATCH_PRIORITY_IDX_NON_INTERACTIVE] = _DISPATCH_QOS_CLASS_UTILITY,
	[DISPATCH_PRIORITY_IDX_LOW] = _DISPATCH_QOS_CLASS_UTILITY,
	[DISPATCH_PRIORITY_IDX_DEFAULT] = _DISPATCH_QOS_CLASS_DEFAULT,
	[DISPATCH_PRIORITY_IDX_HIGH] = _DISPATCH_QOS_CLASS_USER_INITIATED,
};

#if HAVE_PTHREAD_WORKQUEUE_QOS
static const int _dispatch_priority2wq[] = {
	[DISPATCH_PRIORITY_IDX_BACKGROUND] = WORKQ_BG_PRIOQUEUE,
	[DISPATCH_PRIORITY_IDX_NON_INTERACTIVE] = WORKQ_NON_INTERACTIVE_PRIOQUEUE,
	[DISPATCH_PRIORITY_IDX_LOW] = WORKQ_LOW_PRIOQUEUE,
	[DISPATCH_PRIORITY_IDX_DEFAULT] = WORKQ_DEFAULT_PRIOQUEUE,
	[DISPATCH_PRIORITY_IDX_HIGH] = WORKQ_HIGH_PRIOQUEUE,
};
#endif

#if DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES
static struct dispatch_queue_s _dispatch_mgr_root_queue;
#else
#define _dispatch_mgr_root_queue \
		_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_HIGH_OVERCOMMIT_PRIORITY]
#endif

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
DISPATCH_CACHELINE_ALIGN
struct dispatch_queue_s _dispatch_mgr_q = {
	.do_vtable = DISPATCH_VTABLE(queue_mgr),
	.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
	.do_targetq = &_dispatch_mgr_root_queue,
	.dq_label = "com.apple.libdispatch-manager",
	.dq_width = 1,
	.dq_is_thread_bound = 1,
	.dq_serialnum = 2,
};

dispatch_queue_t
dispatch_get_global_queue(long priority, unsigned long flags)
{
	if (flags & ~(unsigned long)DISPATCH_QUEUE_OVERCOMMIT) {
		return NULL;
	}
	dispatch_once_f(&_dispatch_root_queues_pred, NULL,
			_dispatch_root_queues_init);
	qos_class_t qos;
	switch (priority) {
#if !RDAR_17878963 || DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
	case _DISPATCH_QOS_CLASS_MAINTENANCE:
		if (!_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS]
				.dq_priority) {
			// map maintenance to background on old kernel
			qos = _dispatch_priority2qos[DISPATCH_PRIORITY_IDX_BACKGROUND];
		} else {
			qos = (qos_class_t)priority;
		}
		break;
#endif // RDAR_17878963 || DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
	case DISPATCH_QUEUE_PRIORITY_BACKGROUND:
		qos = _dispatch_priority2qos[DISPATCH_PRIORITY_IDX_BACKGROUND];
		break;
	case DISPATCH_QUEUE_PRIORITY_NON_INTERACTIVE:
		qos = _dispatch_priority2qos[DISPATCH_PRIORITY_IDX_NON_INTERACTIVE];
		break;
	case DISPATCH_QUEUE_PRIORITY_LOW:
		qos = _dispatch_priority2qos[DISPATCH_PRIORITY_IDX_LOW];
		break;
	case DISPATCH_QUEUE_PRIORITY_DEFAULT:
		qos = _dispatch_priority2qos[DISPATCH_PRIORITY_IDX_DEFAULT];
		break;
	case DISPATCH_QUEUE_PRIORITY_HIGH:
		qos = _dispatch_priority2qos[DISPATCH_PRIORITY_IDX_HIGH];
		break;
	case _DISPATCH_QOS_CLASS_USER_INTERACTIVE:
#if DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
		if (!_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS]
				.dq_priority) {
			qos = _dispatch_priority2qos[DISPATCH_PRIORITY_IDX_HIGH];
			break;
		}
#endif
		// fallthrough
	default:
		qos = (qos_class_t)priority;
		break;
	}
	return _dispatch_get_root_queue(qos, flags & DISPATCH_QUEUE_OVERCOMMIT);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
_dispatch_get_current_queue(void)
{
	return _dispatch_queue_get_current() ?:
			_dispatch_get_root_queue(_DISPATCH_QOS_CLASS_DEFAULT, true);
}

dispatch_queue_t
dispatch_get_current_queue(void)
{
	return _dispatch_get_current_queue();
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_targets_queue(dispatch_queue_t dq1, dispatch_queue_t dq2)
{
	while (dq1) {
		if (dq1 == dq2) {
			return true;
		}
		dq1 = dq1->do_targetq;
	}
	return false;
}

#define DISPATCH_ASSERT_QUEUE_MESSAGE "BUG in client of libdispatch: " \
		"Assertion failed: Block was run on an unexpected queue"

DISPATCH_NOINLINE
static void
_dispatch_assert_queue_fail(dispatch_queue_t dq, bool expected)
{
	char *msg;
	asprintf(&msg, "%s\n%s queue: 0x%p[%s]", DISPATCH_ASSERT_QUEUE_MESSAGE,
			expected ? "Expected" : "Unexpected", dq, dq->dq_label ?
			dq->dq_label : "");
	_dispatch_log("%s", msg);
	_dispatch_set_crash_log_message(msg);
	_dispatch_hardware_crash();
	free(msg);
}

void
dispatch_assert_queue(dispatch_queue_t dq)
{
	if (slowpath(!dq) || slowpath(!(dx_metatype(dq) == _DISPATCH_QUEUE_TYPE))) {
		DISPATCH_CLIENT_CRASH("invalid queue passed to "
				"dispatch_assert_queue()");
	}
	dispatch_queue_t cq = _dispatch_queue_get_current();
	if (fastpath(cq) && fastpath(_dispatch_queue_targets_queue(cq, dq))) {
		return;
	}
	_dispatch_assert_queue_fail(dq, true);
}

void
dispatch_assert_queue_not(dispatch_queue_t dq)
{
	if (slowpath(!dq) || slowpath(!(dx_metatype(dq) == _DISPATCH_QUEUE_TYPE))) {
		DISPATCH_CLIENT_CRASH("invalid queue passed to "
				"dispatch_assert_queue_not()");
	}
	dispatch_queue_t cq = _dispatch_queue_get_current();
	if (slowpath(cq) && slowpath(_dispatch_queue_targets_queue(cq, dq))) {
		_dispatch_assert_queue_fail(dq, false);
	}
}

#if DISPATCH_DEBUG && DISPATCH_ROOT_QUEUE_DEBUG
#define _dispatch_root_queue_debug(...) _dispatch_debug(__VA_ARGS__)
#define _dispatch_debug_root_queue(...) dispatch_debug_queue(__VA_ARGS__)
#else
#define _dispatch_root_queue_debug(...)
#define _dispatch_debug_root_queue(...)
#endif

#pragma mark -
#pragma mark dispatch_init

#if HAVE_PTHREAD_WORKQUEUE_QOS
int _dispatch_set_qos_class_enabled;
pthread_priority_t _dispatch_background_priority;
pthread_priority_t _dispatch_user_initiated_priority;

static void
_dispatch_root_queues_init_qos(int supported)
{
	pthread_priority_t p;
	qos_class_t qos;
	unsigned int i;
	for (i = 0; i < DISPATCH_PRIORITY_COUNT; i++) {
		p = _pthread_qos_class_encode_workqueue(_dispatch_priority2wq[i], 0);
		qos = _pthread_qos_class_decode(p, NULL, NULL);
		dispatch_assert(qos != _DISPATCH_QOS_CLASS_UNSPECIFIED);
		_dispatch_priority2qos[i] = qos;
	}
	for (i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
		qos = _dispatch_root_queue_contexts[i].dgq_qos;
		if (qos == _DISPATCH_QOS_CLASS_MAINTENANCE &&
				!(supported & WORKQ_FEATURE_MAINTENANCE)) {
			continue;
		}
		unsigned long flags = i & 1 ? _PTHREAD_PRIORITY_OVERCOMMIT_FLAG : 0;
		flags |= _PTHREAD_PRIORITY_ROOTQUEUE_FLAG;
		if (i == DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS ||
				i == DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT) {
			flags |= _PTHREAD_PRIORITY_DEFAULTQUEUE_FLAG;
		}
		p = _pthread_qos_class_encode(qos, 0, flags);
		_dispatch_root_queues[i].dq_priority = p;
	}
	p = _pthread_qos_class_encode(qos_class_main(), 0, 0);
	_dispatch_main_q.dq_priority = p;
	_dispatch_queue_set_override_priority(&_dispatch_main_q);
	_dispatch_background_priority = _dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_BACKGROUND_QOS].dq_priority &
			~_PTHREAD_PRIORITY_FLAGS_MASK;
	_dispatch_user_initiated_priority = _dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_USER_INITIATED_QOS].dq_priority &
			~_PTHREAD_PRIORITY_FLAGS_MASK;
	if (!slowpath(getenv("LIBDISPATCH_DISABLE_SET_QOS"))) {
		_dispatch_set_qos_class_enabled = 1;
	}
}
#endif

static inline bool
_dispatch_root_queues_init_workq(void)
{
	bool result = false;
#if HAVE_PTHREAD_WORKQUEUES
	bool disable_wq = false;
#if DISPATCH_ENABLE_THREAD_POOL && DISPATCH_DEBUG
	disable_wq = slowpath(getenv("LIBDISPATCH_DISABLE_KWQ"));
#endif
	int r;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	bool disable_qos = false;
#if DISPATCH_DEBUG
	disable_qos = slowpath(getenv("LIBDISPATCH_DISABLE_QOS"));
#endif
	if (!disable_qos && !disable_wq) {
		r = _pthread_workqueue_supported();
		int supported = r;
		if (r & WORKQ_FEATURE_FINEPRIO) {
			r = _pthread_workqueue_init(_dispatch_worker_thread3,
					offsetof(struct dispatch_queue_s, dq_serialnum), 0);
			result = !r;
			if (result) _dispatch_root_queues_init_qos(supported);
		}
	}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS
#if HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
	if (!result && !disable_wq) {
#if PTHREAD_WORKQUEUE_SPI_VERSION >= 20121218
		pthread_workqueue_setdispatchoffset_np(
				offsetof(struct dispatch_queue_s, dq_serialnum));
#endif
		r = pthread_workqueue_setdispatch_np(_dispatch_worker_thread2);
#if !DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
		(void)dispatch_assume_zero(r);
#endif
		result = !r;
	}
#endif // HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
#if DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK || DISPATCH_USE_PTHREAD_POOL
	if (!result) {
#if DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
		pthread_workqueue_attr_t pwq_attr;
		if (!disable_wq) {
			r = pthread_workqueue_attr_init_np(&pwq_attr);
			(void)dispatch_assume_zero(r);
		}
#endif
		int i;
		for (i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
			pthread_workqueue_t pwq = NULL;
			dispatch_root_queue_context_t qc;
			qc = &_dispatch_root_queue_contexts[i];
#if DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
			if (!disable_wq) {
				r = pthread_workqueue_attr_setqueuepriority_np(&pwq_attr,
						qc->dgq_wq_priority);
				(void)dispatch_assume_zero(r);
				r = pthread_workqueue_attr_setovercommit_np(&pwq_attr,
						qc->dgq_wq_options &
						WORKQ_ADDTHREADS_OPTION_OVERCOMMIT);
				(void)dispatch_assume_zero(r);
				r = pthread_workqueue_create_np(&pwq, &pwq_attr);
				(void)dispatch_assume_zero(r);
				result = result || dispatch_assume(pwq);
			}
#endif // DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
			qc->dgq_kworkqueue = pwq ? pwq : (void*)(~0ul);
		}
#if DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
		if (!disable_wq) {
			r = pthread_workqueue_attr_destroy_np(&pwq_attr);
			(void)dispatch_assume_zero(r);
		}
#endif
	}
#endif // DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK || DISPATCH_ENABLE_THREAD_POOL
#endif // HAVE_PTHREAD_WORKQUEUES
	return result;
}

#if DISPATCH_USE_PTHREAD_POOL
static inline void
_dispatch_root_queue_init_pthread_pool(dispatch_root_queue_context_t qc,
		uint8_t pool_size, bool overcommit)
{
	dispatch_pthread_root_queue_context_t pqc = qc->dgq_ctxt;
	uint32_t thread_pool_size = overcommit ? MAX_PTHREAD_COUNT :
			dispatch_hw_config(active_cpus);
	if (slowpath(pool_size) && pool_size < thread_pool_size) {
		thread_pool_size = pool_size;
	}
	qc->dgq_thread_pool_size = thread_pool_size;
#ifndef __FreeBSD__
	if (qc->dgq_qos) {
		(void)dispatch_assume_zero(pthread_attr_init(&pqc->dpq_thread_attr));
		(void)dispatch_assume_zero(pthread_attr_setdetachstate(
				&pqc->dpq_thread_attr, PTHREAD_CREATE_DETACHED));
#if HAVE_PTHREAD_WORKQUEUE_QOS
		(void)dispatch_assume_zero(pthread_attr_set_qos_class_np(
				&pqc->dpq_thread_attr, qc->dgq_qos, 0));
#endif
	}
#endif
#if USE_MACH_SEM
	// override the default FIFO behavior for the pool semaphores
	kern_return_t kr = semaphore_create(mach_task_self(),
			&pqc->dpq_thread_mediator.dsema_port, SYNC_POLICY_LIFO, 0);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
	(void)dispatch_assume(pqc->dpq_thread_mediator.dsema_port);
#elif USE_POSIX_SEM
	/* XXXRW: POSIX semaphores don't support LIFO? */
	int ret = sem_init(&pqc->dpq_thread_mediator.dsema_sem, 0, 0);
	(void)dispatch_assume_zero(ret);
#endif
}
#endif // DISPATCH_USE_PTHREAD_POOL

static dispatch_once_t _dispatch_root_queues_pred;

static void
_dispatch_root_queues_init(void *context DISPATCH_UNUSED)
{
	_dispatch_safe_fork = false;
	if (!_dispatch_root_queues_init_workq()) {
#if DISPATCH_ENABLE_THREAD_POOL
		int i;
		for (i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
			bool overcommit = true;
#if TARGET_OS_EMBEDDED
			// some software hangs if the non-overcommitting queues do not
			// overcommit when threads block. Someday, this behavior should
			// apply to all platforms
			if (!(i & 1)) {
				overcommit = false;
			}
#endif
			_dispatch_root_queue_init_pthread_pool(
					&_dispatch_root_queue_contexts[i], 0, overcommit);
		}
#else
		DISPATCH_CRASH("Root queue initialization failed");
#endif // DISPATCH_ENABLE_THREAD_POOL
	}
}

#define countof(x) (sizeof(x) / sizeof(x[0]))

DISPATCH_EXPORT DISPATCH_NOTHROW
void
libdispatch_init(void)
{
	dispatch_assert(DISPATCH_QUEUE_QOS_COUNT == 6);
	dispatch_assert(DISPATCH_ROOT_QUEUE_COUNT == 12);

	dispatch_assert(DISPATCH_QUEUE_PRIORITY_LOW ==
			-DISPATCH_QUEUE_PRIORITY_HIGH);
	dispatch_assert(countof(_dispatch_root_queues) ==
			DISPATCH_ROOT_QUEUE_COUNT);
	dispatch_assert(countof(_dispatch_root_queue_contexts) ==
			DISPATCH_ROOT_QUEUE_COUNT);
	dispatch_assert(countof(_dispatch_priority2qos) ==
			DISPATCH_PRIORITY_COUNT);
#if HAVE_PTHREAD_WORKQUEUE_QOS
	dispatch_assert(countof(_dispatch_priority2wq) ==
			DISPATCH_PRIORITY_COUNT);
#endif
#if HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
	dispatch_assert(sizeof(_dispatch_wq2root_queues) /
			sizeof(_dispatch_wq2root_queues[0][0]) ==
			WORKQ_NUM_PRIOQUEUE * 2);
#endif
#if DISPATCH_ENABLE_THREAD_POOL
	dispatch_assert(countof(_dispatch_pthread_root_queue_contexts) ==
			DISPATCH_ROOT_QUEUE_COUNT);
#endif

	dispatch_assert(offsetof(struct dispatch_continuation_s, do_next) ==
			offsetof(struct dispatch_object_s, do_next));
	dispatch_assert(sizeof(struct dispatch_apply_s) <=
			DISPATCH_CONTINUATION_SIZE);
	dispatch_assert(sizeof(struct dispatch_queue_s) % DISPATCH_CACHELINE_SIZE
			== 0);
	dispatch_assert(sizeof(struct dispatch_root_queue_context_s) %
			DISPATCH_CACHELINE_SIZE == 0);

	_dispatch_thread_key_create(&dispatch_queue_key, _dispatch_queue_cleanup);
	_dispatch_thread_key_create(&dispatch_voucher_key, _voucher_thread_cleanup);
	_dispatch_thread_key_create(&dispatch_cache_key, _dispatch_cache_cleanup);
	_dispatch_thread_key_create(&dispatch_io_key, NULL);
	_dispatch_thread_key_create(&dispatch_apply_key, NULL);
	_dispatch_thread_key_create(&dispatch_defaultpriority_key, NULL);
#if DISPATCH_PERF_MON && !DISPATCH_INTROSPECTION
	_dispatch_thread_key_create(&dispatch_bcounter_key, NULL);
#endif
#if !DISPATCH_USE_OS_SEMAPHORE_CACHE
	_dispatch_thread_key_create(&dispatch_sema4_key,
			(void (*)(void *))_dispatch_thread_semaphore_dispose);
#endif

#if DISPATCH_USE_RESOLVERS // rdar://problem/8541707
	_dispatch_main_q.do_targetq = &_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT];
#endif

	_dispatch_thread_setspecific(dispatch_queue_key, &_dispatch_main_q);
	_dispatch_queue_set_bound_thread(&_dispatch_main_q);

#if DISPATCH_USE_PTHREAD_ATFORK
	(void)dispatch_assume_zero(pthread_atfork(dispatch_atfork_prepare,
			dispatch_atfork_parent, dispatch_atfork_child));
#endif

	_dispatch_hw_config_init();
	_dispatch_vtable_init();
	_os_object_init();
	_voucher_init();
	_dispatch_introspection_init();
}

#if HAVE_MACH
static dispatch_once_t _dispatch_mach_host_port_pred;
static mach_port_t _dispatch_mach_host_port;

static void
_dispatch_mach_host_port_init(void *ctxt DISPATCH_UNUSED)
{
	kern_return_t kr;
	mach_port_t mp, mhp = mach_host_self();
	kr = host_get_host_port(mhp, &mp);
	DISPATCH_VERIFY_MIG(kr);
	if (!kr) {
		// mach_host_self returned the HOST_PRIV port
		kr = mach_port_deallocate(mach_task_self(), mhp);
		DISPATCH_VERIFY_MIG(kr);
		(void)dispatch_assume_zero(kr);
		mhp = mp;
	} else if (kr != KERN_INVALID_ARGUMENT) {
		(void)dispatch_assume_zero(kr);
	}
	if (!dispatch_assume(mhp)) {
		DISPATCH_CRASH("Could not get unprivileged host port");
	}
	_dispatch_mach_host_port = mhp;
}

mach_port_t
_dispatch_get_mach_host_port(void)
{
	dispatch_once_f(&_dispatch_mach_host_port_pred, NULL,
			_dispatch_mach_host_port_init);
	return _dispatch_mach_host_port;
}
#endif

DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_atfork_child(void)
{
	void *crash = (void *)0x100;
	size_t i;

#if HAVE_MACH
	_dispatch_mach_host_port_pred = 0;
	_dispatch_mach_host_port = MACH_VOUCHER_NULL;
#endif
	_voucher_atfork_child();
	if (_dispatch_safe_fork) {
		return;
	}
	_dispatch_child_of_unsafe_fork = true;

	_dispatch_main_q.dq_items_head = crash;
	_dispatch_main_q.dq_items_tail = crash;

	_dispatch_mgr_q.dq_items_head = crash;
	_dispatch_mgr_q.dq_items_tail = crash;

	for (i = 0; i < DISPATCH_ROOT_QUEUE_COUNT; i++) {
		_dispatch_root_queues[i].dq_items_head = crash;
		_dispatch_root_queues[i].dq_items_tail = crash;
	}
}

#pragma mark -
#pragma mark dispatch_queue_attr_t

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_qos_class_valid(dispatch_qos_class_t qos_class, int relative_priority)
{
	qos_class_t qos = (qos_class_t)qos_class;
	switch (qos) {
	case _DISPATCH_QOS_CLASS_MAINTENANCE:
	case _DISPATCH_QOS_CLASS_BACKGROUND:
	case _DISPATCH_QOS_CLASS_UTILITY:
	case _DISPATCH_QOS_CLASS_DEFAULT:
	case _DISPATCH_QOS_CLASS_USER_INITIATED:
	case _DISPATCH_QOS_CLASS_USER_INTERACTIVE:
	case _DISPATCH_QOS_CLASS_UNSPECIFIED:
		break;
	default:
		return false;
	}
	if (relative_priority > 0 || relative_priority < QOS_MIN_RELATIVE_PRIORITY){
		return false;
	}
	return true;
}

#define DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(qos) \
		[_DISPATCH_QOS_CLASS_##qos] = DQA_INDEX_QOS_CLASS_##qos

static const
_dispatch_queue_attr_index_qos_class_t _dispatch_queue_attr_qos2idx[] = {
	DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(UNSPECIFIED),
	DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(MAINTENANCE),
	DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(BACKGROUND),
	DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(UTILITY),
	DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(DEFAULT),
	DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(USER_INITIATED),
	DISPATCH_QUEUE_ATTR_QOS2IDX_INITIALIZER(USER_INTERACTIVE),
};

#define DISPATCH_QUEUE_ATTR_OVERCOMMIT2IDX(overcommit) \
		(overcommit ? DQA_INDEX_OVERCOMMIT : DQA_INDEX_NON_OVERCOMMIT)

#define DISPATCH_QUEUE_ATTR_CONCURRENT2IDX(concurrent) \
		(concurrent ? DQA_INDEX_CONCURRENT : DQA_INDEX_SERIAL)

#define DISPATCH_QUEUE_ATTR_PRIO2IDX(prio) (-(prio))

#define DISPATCH_QUEUE_ATTR_QOS2IDX(qos) (_dispatch_queue_attr_qos2idx[(qos)])

static inline dispatch_queue_attr_t
_dispatch_get_queue_attr(qos_class_t qos, int prio, bool overcommit,
		bool concurrent)
{
	return (dispatch_queue_attr_t)&_dispatch_queue_attrs
			[DISPATCH_QUEUE_ATTR_QOS2IDX(qos)]
			[DISPATCH_QUEUE_ATTR_PRIO2IDX(prio)]
			[DISPATCH_QUEUE_ATTR_OVERCOMMIT2IDX(overcommit)]
			[DISPATCH_QUEUE_ATTR_CONCURRENT2IDX(concurrent)];
}

dispatch_queue_attr_t
dispatch_queue_attr_make_with_qos_class(dispatch_queue_attr_t dqa,
		dispatch_qos_class_t qos_class, int relative_priority)
{
	if (!_dispatch_qos_class_valid(qos_class, relative_priority)) return NULL;
	if (!slowpath(dqa)) {
		dqa = _dispatch_get_queue_attr(0, 0, false, false);
	} else if (dqa->do_vtable != DISPATCH_VTABLE(queue_attr)) {
		DISPATCH_CLIENT_CRASH("Invalid queue attribute");
	}
	return _dispatch_get_queue_attr(qos_class, relative_priority,
			dqa->dqa_overcommit, dqa->dqa_concurrent);
}

dispatch_queue_attr_t
dispatch_queue_attr_make_with_overcommit(dispatch_queue_attr_t dqa,
		bool overcommit)
{
	if (!slowpath(dqa)) {
		dqa = _dispatch_get_queue_attr(0, 0, false, false);
	} else if (dqa->do_vtable != DISPATCH_VTABLE(queue_attr)) {
		DISPATCH_CLIENT_CRASH("Invalid queue attribute");
	}
	return _dispatch_get_queue_attr(dqa->dqa_qos_class,
			dqa->dqa_relative_priority, overcommit, dqa->dqa_concurrent);
}

#pragma mark -
#pragma mark dispatch_queue_t

// skip zero
// 1 - main_q
// 2 - mgr_q
// 3 - mgr_root_q
// 4,5,6,7,8,9,10,11,12,13,14,15 - global queues
// we use 'xadd' on Intel, so the initial value == next assigned
unsigned long volatile _dispatch_queue_serial_numbers = 16;

dispatch_queue_t
dispatch_queue_create_with_target(const char *label, dispatch_queue_attr_t dqa,
		dispatch_queue_t tq)
{
#if DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
	// Be sure the root queue priorities are set
	dispatch_once_f(&_dispatch_root_queues_pred, NULL,
			_dispatch_root_queues_init);
#endif
	bool disallow_tq = (slowpath(dqa) && dqa != DISPATCH_QUEUE_CONCURRENT);
	if (!slowpath(dqa)) {
		dqa = _dispatch_get_queue_attr(0, 0, false, false);
	} else if (dqa->do_vtable != DISPATCH_VTABLE(queue_attr)) {
		DISPATCH_CLIENT_CRASH("Invalid queue attribute");
	}
	dispatch_queue_t dq = _dispatch_alloc(DISPATCH_VTABLE(queue),
			sizeof(struct dispatch_queue_s) - DISPATCH_QUEUE_CACHELINE_PAD);
	_dispatch_queue_init(dq);
	if (label) {
		dq->dq_label = strdup(label);
	}
	qos_class_t qos = dqa->dqa_qos_class;
	bool overcommit = dqa->dqa_overcommit;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	dq->dq_priority = _pthread_qos_class_encode(qos, dqa->dqa_relative_priority,
			overcommit);
#endif
	if (dqa->dqa_concurrent) {
		dq->dq_width = DISPATCH_QUEUE_WIDTH_MAX;
	} else {
		// Default serial queue target queue is overcommit!
		overcommit = true;
	}
	if (!tq) {
		if (qos == _DISPATCH_QOS_CLASS_UNSPECIFIED) {
			qos = _DISPATCH_QOS_CLASS_DEFAULT;
		}
#if DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
		if (qos == _DISPATCH_QOS_CLASS_USER_INTERACTIVE &&
				!_dispatch_root_queues[
				DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS].dq_priority) {
			qos = _DISPATCH_QOS_CLASS_USER_INITIATED;
		}
#endif
		bool maintenance_fallback = false;
#if DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
		maintenance_fallback = true;
#endif // DISPATCH_USE_NOQOS_WORKQUEUE_FALLBACK
		if (maintenance_fallback) {
			if (qos == _DISPATCH_QOS_CLASS_MAINTENANCE &&
					!_dispatch_root_queues[
					DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS].dq_priority) {
				qos = _DISPATCH_QOS_CLASS_BACKGROUND;
			}
		}

		tq = _dispatch_get_root_queue(qos, overcommit);
		if (slowpath(!tq)) {
			DISPATCH_CLIENT_CRASH("Invalid queue attribute");
		}
	} else {
		_dispatch_retain(tq);
		if (disallow_tq) {
			// TODO: override target queue's qos/overcommit ?
			DISPATCH_CLIENT_CRASH("Invalid combination of target queue & "
					"queue attribute");
		}
		_dispatch_queue_priority_inherit_from_target(dq, tq);
	}
	_dispatch_queue_set_override_priority(dq);
	dq->do_targetq = tq;
	_dispatch_object_debug(dq, "%s", __func__);
	return _dispatch_introspection_queue_create(dq);
}

dispatch_queue_t
dispatch_queue_create(const char *label, dispatch_queue_attr_t attr)
{
	return dispatch_queue_create_with_target(label, attr,
			DISPATCH_TARGET_QUEUE_DEFAULT);
}

void
_dispatch_queue_destroy(dispatch_object_t dou)
{
	dispatch_queue_t dq = dou._dq;
	if (slowpath(dq == _dispatch_queue_get_current())) {
		DISPATCH_CRASH("Release of a queue by itself");
	}
	if (slowpath(dq->dq_items_tail)) {
		DISPATCH_CRASH("Release of a queue while items are enqueued");
	}

	// trash the tail queue so that use after free will crash
	dq->dq_items_tail = (void *)0x200;

	dispatch_queue_t dqsq = dispatch_atomic_xchg2o(dq, dq_specific_q,
			(void *)0x200, relaxed);
	if (dqsq) {
		_dispatch_release(dqsq);
	}
}

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
void
_dispatch_queue_dispose(dispatch_queue_t dq)
{
	_dispatch_object_debug(dq, "%s", __func__);
	_dispatch_introspection_queue_dispose(dq);
	if (dq->dq_label) {
		free((void*)dq->dq_label);
	}
	_dispatch_queue_destroy(dq);
}

const char *
dispatch_queue_get_label(dispatch_queue_t dq)
{
	if (slowpath(dq == DISPATCH_CURRENT_QUEUE_LABEL)) {
		dq = _dispatch_get_current_queue();
	}
	return dq->dq_label ? dq->dq_label : "";
}

qos_class_t
dispatch_queue_get_qos_class(dispatch_queue_t dq, int *relative_priority_ptr)
{
	qos_class_t qos = _DISPATCH_QOS_CLASS_UNSPECIFIED;
	int relative_priority = 0;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pthread_priority_t dqp = dq->dq_priority;
	if (dqp & _PTHREAD_PRIORITY_INHERIT_FLAG) dqp = 0;
	qos = _pthread_qos_class_decode(dqp, &relative_priority, NULL);
#else
	(void)dq;
#endif
	if (relative_priority_ptr) *relative_priority_ptr = relative_priority;
	return qos;
}

static void
_dispatch_queue_set_width2(void *ctxt)
{
	int w = (int)(intptr_t)ctxt; // intentional truncation
	uint32_t tmp;
	dispatch_queue_t dq = _dispatch_queue_get_current();

	if (w == 1 || w == 0) {
		dq->dq_width = 1;
		_dispatch_object_debug(dq, "%s", __func__);
		return;
	}
	if (w > 0) {
		tmp = (unsigned int)w;
	} else switch (w) {
	case DISPATCH_QUEUE_WIDTH_MAX_PHYSICAL_CPUS:
		tmp = dispatch_hw_config(physical_cpus);
		break;
	case DISPATCH_QUEUE_WIDTH_ACTIVE_CPUS:
		tmp = dispatch_hw_config(active_cpus);
		break;
	default:
		// fall through
	case DISPATCH_QUEUE_WIDTH_MAX_LOGICAL_CPUS:
		tmp = dispatch_hw_config(logical_cpus);
		break;
	}
	if (tmp > DISPATCH_QUEUE_WIDTH_MAX / 2) {
		tmp = DISPATCH_QUEUE_WIDTH_MAX / 2;
	}
	// multiply by two since the running count is inc/dec by two
	// (the low bit == barrier)
	dq->dq_width = (typeof(dq->dq_width))(tmp * 2);
	_dispatch_object_debug(dq, "%s", __func__);
}

void
dispatch_queue_set_width(dispatch_queue_t dq, long width)
{
	if (slowpath(dq->do_ref_cnt == DISPATCH_OBJECT_GLOBAL_REFCNT) ||
			slowpath(dx_type(dq) == DISPATCH_QUEUE_ROOT_TYPE)) {
		return;
	}
	_dispatch_barrier_trysync_f(dq, (void*)(intptr_t)width,
			_dispatch_queue_set_width2);
}

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
static void
_dispatch_set_target_queue2(void *ctxt)
{
	dispatch_queue_t prev_dq, dq = _dispatch_queue_get_current(), tq = ctxt;
	mach_port_t th;

	while (!dispatch_atomic_cmpxchgv2o(dq, dq_tqthread, MACH_PORT_NULL,
			_dispatch_thread_port(), &th, acquire)) {
		_dispatch_thread_switch(th, DISPATCH_YIELD_THREAD_SWITCH_OPTION,
				DISPATCH_CONTENTION_USLEEP_START);
	}
	_dispatch_queue_priority_inherit_from_target(dq, tq);
	prev_dq = dq->do_targetq;
	dq->do_targetq = tq;
	_dispatch_release(prev_dq);
	_dispatch_object_debug(dq, "%s", __func__);
	dispatch_atomic_store2o(dq, dq_tqthread, MACH_PORT_NULL, release);
}

void
dispatch_set_target_queue(dispatch_object_t dou, dispatch_queue_t dq)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_set_target_queue, dou, dq);
	if (slowpath(dou._do->do_ref_cnt == DISPATCH_OBJECT_GLOBAL_REFCNT) ||
			slowpath(dx_type(dou._do) == DISPATCH_QUEUE_ROOT_TYPE)) {
		return;
	}
	unsigned long type = dx_metatype(dou._do);
	if (slowpath(!dq)) {
		bool is_concurrent_q = (type == _DISPATCH_QUEUE_TYPE &&
				slowpath(dou._dq->dq_width > 1));
		dq = _dispatch_get_root_queue(_DISPATCH_QOS_CLASS_DEFAULT,
				!is_concurrent_q);
	}
	// TODO: put into the vtable
	switch(type) {
	case _DISPATCH_QUEUE_TYPE:
	case _DISPATCH_SOURCE_TYPE:
		_dispatch_retain(dq);
		return _dispatch_barrier_trysync_f(dou._dq, dq,
				_dispatch_set_target_queue2);
	case _DISPATCH_IO_TYPE:
		return _dispatch_io_set_target_queue(dou._dchannel, dq);
	default: {
		dispatch_queue_t prev_dq;
		_dispatch_retain(dq);
		prev_dq = dispatch_atomic_xchg2o(dou._do, do_targetq, dq, release);
		if (prev_dq) _dispatch_release(prev_dq);
		_dispatch_object_debug(dou._do, "%s", __func__);
		return;
		}
	}
}

#pragma mark -
#pragma mark dispatch_pthread_root_queue

#if DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES
static struct dispatch_pthread_root_queue_context_s
		_dispatch_mgr_root_queue_pthread_context;
static struct dispatch_root_queue_context_s
		_dispatch_mgr_root_queue_context = {{{
#if HAVE_PTHREAD_WORKQUEUES
	.dgq_kworkqueue = (void*)(~0ul),
#endif
	.dgq_ctxt = &_dispatch_mgr_root_queue_pthread_context,
	.dgq_thread_pool_size = 1,
}}};
static struct dispatch_queue_s _dispatch_mgr_root_queue = {
	.do_vtable = DISPATCH_VTABLE(queue_root),
	.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
	.do_ctxt = &_dispatch_mgr_root_queue_context,
	.dq_label = "com.apple.root.libdispatch-manager",
	.dq_running = 2,
	.dq_width = DISPATCH_QUEUE_WIDTH_MAX,
	.dq_serialnum = 3,
};
static struct {
	volatile int prio;
	int default_prio;
	int policy;
	pthread_t tid;
} _dispatch_mgr_sched;
static dispatch_once_t _dispatch_mgr_sched_pred;

static void
_dispatch_mgr_sched_init(void *ctxt DISPATCH_UNUSED)
{
	struct sched_param param;
	pthread_attr_t *attr;
	attr = &_dispatch_mgr_root_queue_pthread_context.dpq_thread_attr;
	(void)dispatch_assume_zero(pthread_attr_init(attr));
	(void)dispatch_assume_zero(pthread_attr_getschedpolicy(attr,
			&_dispatch_mgr_sched.policy));
	(void)dispatch_assume_zero(pthread_attr_getschedparam(attr, &param));
	 // legacy priority calls allowed when requesting above default priority
	_dispatch_mgr_sched.default_prio = param.sched_priority;
	_dispatch_mgr_sched.prio = _dispatch_mgr_sched.default_prio;
}

DISPATCH_NOINLINE
static pthread_t *
_dispatch_mgr_root_queue_init(void)
{
	dispatch_once_f(&_dispatch_mgr_sched_pred, NULL, _dispatch_mgr_sched_init);
	struct sched_param param;
	pthread_attr_t *attr;
	attr = &_dispatch_mgr_root_queue_pthread_context.dpq_thread_attr;
	(void)dispatch_assume_zero(pthread_attr_setdetachstate(attr,
			PTHREAD_CREATE_DETACHED));
#if !DISPATCH_DEBUG
	(void)dispatch_assume_zero(pthread_attr_setstacksize(attr, 64 * 1024));
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (_dispatch_set_qos_class_enabled) {
		qos_class_t qos = qos_class_main();
		(void)dispatch_assume_zero(pthread_attr_set_qos_class_np(attr, qos, 0));
		_dispatch_mgr_q.dq_priority = _pthread_qos_class_encode(qos, 0, 0);
		_dispatch_queue_set_override_priority(&_dispatch_mgr_q);
	}
#endif
	param.sched_priority = _dispatch_mgr_sched.prio;
	if (param.sched_priority > _dispatch_mgr_sched.default_prio) {
		(void)dispatch_assume_zero(pthread_attr_setschedparam(attr, &param));
	}
	return &_dispatch_mgr_sched.tid;
}

static inline void
_dispatch_mgr_priority_apply(void)
{
	struct sched_param param;
	do {
		param.sched_priority = _dispatch_mgr_sched.prio;
		if (param.sched_priority > _dispatch_mgr_sched.default_prio) {
			(void)dispatch_assume_zero(pthread_setschedparam(
					_dispatch_mgr_sched.tid, _dispatch_mgr_sched.policy,
					&param));
		}
	} while (_dispatch_mgr_sched.prio > param.sched_priority);
}

DISPATCH_NOINLINE
void
_dispatch_mgr_priority_init(void)
{
	struct sched_param param;
	pthread_attr_t *attr;
	attr = &_dispatch_mgr_root_queue_pthread_context.dpq_thread_attr;
	(void)dispatch_assume_zero(pthread_attr_getschedparam(attr, &param));
	if (slowpath(_dispatch_mgr_sched.prio > param.sched_priority)) {
		return _dispatch_mgr_priority_apply();
	}
}

DISPATCH_NOINLINE
static void
_dispatch_mgr_priority_raise(const pthread_attr_t *attr)
{
	dispatch_once_f(&_dispatch_mgr_sched_pred, NULL, _dispatch_mgr_sched_init);
	struct sched_param param;
	(void)dispatch_assume_zero(pthread_attr_getschedparam(attr, &param));
	int p = _dispatch_mgr_sched.prio;
	do if (p >= param.sched_priority) {
		return;
	} while (slowpath(!dispatch_atomic_cmpxchgvw2o(&_dispatch_mgr_sched, prio,
			p, param.sched_priority, &p, relaxed)));
	if (_dispatch_mgr_sched.tid) {
		return _dispatch_mgr_priority_apply();
	}
}

dispatch_queue_t
dispatch_pthread_root_queue_create(const char *label, unsigned long flags,
		const pthread_attr_t *attr, dispatch_block_t configure)
{
	dispatch_queue_t dq;
	dispatch_root_queue_context_t qc;
	dispatch_pthread_root_queue_context_t pqc;
	size_t dqs;
	uint8_t pool_size = flags & _DISPATCH_PTHREAD_ROOT_QUEUE_FLAG_POOL_SIZE ?
			(uint8_t)(flags & ~_DISPATCH_PTHREAD_ROOT_QUEUE_FLAG_POOL_SIZE) : 0;

	dqs = sizeof(struct dispatch_queue_s) - DISPATCH_QUEUE_CACHELINE_PAD;
	dq = _dispatch_alloc(DISPATCH_VTABLE(queue_root), dqs +
			sizeof(struct dispatch_root_queue_context_s) +
			sizeof(struct dispatch_pthread_root_queue_context_s));
	qc = (void*)((uint8_t *)dq + dqs);
	pqc = (void*)((uint8_t *)qc + sizeof(struct dispatch_root_queue_context_s));

	_dispatch_queue_init(dq);
	if (label) {
		dq->dq_label = strdup(label);
	}

	dq->do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK;
	dq->do_ctxt = qc;
	dq->do_targetq = NULL;
	dq->dq_running = 2;
	dq->dq_width = DISPATCH_QUEUE_WIDTH_MAX;

	pqc->dpq_thread_mediator.do_vtable = DISPATCH_VTABLE(semaphore);
	qc->dgq_ctxt = pqc;
#if HAVE_PTHREAD_WORKQUEUES
	qc->dgq_kworkqueue = (void*)(~0ul);
#endif
	_dispatch_root_queue_init_pthread_pool(qc, pool_size, true);

	if (attr) {
		memcpy(&pqc->dpq_thread_attr, attr, sizeof(pthread_attr_t));
#if HAVE_PTHREAD_WORKQUEUE_QOS
		qos_class_t qos = 0;
		if (!pthread_attr_get_qos_class_np(&pqc->dpq_thread_attr, &qos, NULL)
				&& qos > _DISPATCH_QOS_CLASS_DEFAULT) {
			DISPATCH_CLIENT_CRASH("pthread root queues do not support "
					"explicit QoS attributes");
		}
#endif
		_dispatch_mgr_priority_raise(&pqc->dpq_thread_attr);
	} else {
		(void)dispatch_assume_zero(pthread_attr_init(&pqc->dpq_thread_attr));
	}
	(void)dispatch_assume_zero(pthread_attr_setdetachstate(
			&pqc->dpq_thread_attr, PTHREAD_CREATE_DETACHED));
	if (configure) {
		pqc->dpq_thread_configure = _dispatch_Block_copy(configure);
	}
	_dispatch_object_debug(dq, "%s", __func__);
	return _dispatch_introspection_queue_create(dq);
}
#endif

void
_dispatch_pthread_root_queue_dispose(dispatch_queue_t dq)
{
	if (slowpath(dq->do_ref_cnt == DISPATCH_OBJECT_GLOBAL_REFCNT)) {
		DISPATCH_CRASH("Global root queue disposed");
	}
	_dispatch_object_debug(dq, "%s", __func__);
	_dispatch_introspection_queue_dispose(dq);
#if DISPATCH_USE_PTHREAD_POOL
	dispatch_root_queue_context_t qc = dq->do_ctxt;
	dispatch_pthread_root_queue_context_t pqc = qc->dgq_ctxt;

	pthread_attr_destroy(&pqc->dpq_thread_attr);
	_dispatch_semaphore_dispose(&pqc->dpq_thread_mediator);
	if (pqc->dpq_thread_configure) {
		Block_release(pqc->dpq_thread_configure);
	}
	dq->do_targetq = _dispatch_get_root_queue(_DISPATCH_QOS_CLASS_DEFAULT,
			false);
#endif
	if (dq->dq_label) {
		free((void*)dq->dq_label);
	}
	_dispatch_queue_destroy(dq);
}

#pragma mark -
#pragma mark dispatch_queue_specific

struct dispatch_queue_specific_queue_s {
	DISPATCH_STRUCT_HEADER(queue_specific_queue);
	DISPATCH_QUEUE_HEADER;
	TAILQ_HEAD(dispatch_queue_specific_head_s,
			dispatch_queue_specific_s) dqsq_contexts;
};

struct dispatch_queue_specific_s {
	const void *dqs_key;
	void *dqs_ctxt;
	dispatch_function_t dqs_destructor;
	TAILQ_ENTRY(dispatch_queue_specific_s) dqs_list;
};
DISPATCH_DECL(dispatch_queue_specific);

void
_dispatch_queue_specific_queue_dispose(dispatch_queue_specific_queue_t dqsq)
{
	dispatch_queue_specific_t dqs, tmp;

	TAILQ_FOREACH_SAFE(dqs, &dqsq->dqsq_contexts, dqs_list, tmp) {
		if (dqs->dqs_destructor) {
			dispatch_async_f(_dispatch_get_root_queue(
					_DISPATCH_QOS_CLASS_DEFAULT, false), dqs->dqs_ctxt,
					dqs->dqs_destructor);
		}
		free(dqs);
	}
	_dispatch_queue_destroy((dispatch_queue_t)dqsq);
}

static void
_dispatch_queue_init_specific(dispatch_queue_t dq)
{
	dispatch_queue_specific_queue_t dqsq;

	dqsq = _dispatch_alloc(DISPATCH_VTABLE(queue_specific_queue),
			sizeof(struct dispatch_queue_specific_queue_s));
	_dispatch_queue_init((dispatch_queue_t)dqsq);
	dqsq->do_xref_cnt = -1;
	dqsq->do_targetq = _dispatch_get_root_queue(
			_DISPATCH_QOS_CLASS_USER_INITIATED, true);
	dqsq->dq_width = DISPATCH_QUEUE_WIDTH_MAX;
	dqsq->dq_label = "queue-specific";
	TAILQ_INIT(&dqsq->dqsq_contexts);
	if (slowpath(!dispatch_atomic_cmpxchg2o(dq, dq_specific_q, NULL,
			(dispatch_queue_t)dqsq, release))) {
		_dispatch_release((dispatch_queue_t)dqsq);
	}
}

static void
_dispatch_queue_set_specific(void *ctxt)
{
	dispatch_queue_specific_t dqs, dqsn = ctxt;
	dispatch_queue_specific_queue_t dqsq =
			(dispatch_queue_specific_queue_t)_dispatch_queue_get_current();

	TAILQ_FOREACH(dqs, &dqsq->dqsq_contexts, dqs_list) {
		if (dqs->dqs_key == dqsn->dqs_key) {
			// Destroy previous context for existing key
			if (dqs->dqs_destructor) {
				dispatch_async_f(_dispatch_get_root_queue(
						_DISPATCH_QOS_CLASS_DEFAULT, false), dqs->dqs_ctxt,
						dqs->dqs_destructor);
			}
			if (dqsn->dqs_ctxt) {
				// Copy new context for existing key
				dqs->dqs_ctxt = dqsn->dqs_ctxt;
				dqs->dqs_destructor = dqsn->dqs_destructor;
			} else {
				// Remove context storage for existing key
				TAILQ_REMOVE(&dqsq->dqsq_contexts, dqs, dqs_list);
				free(dqs);
			}
			return free(dqsn);
		}
	}
	// Insert context storage for new key
	TAILQ_INSERT_TAIL(&dqsq->dqsq_contexts, dqsn, dqs_list);
}

DISPATCH_NOINLINE
void
dispatch_queue_set_specific(dispatch_queue_t dq, const void *key,
	void *ctxt, dispatch_function_t destructor)
{
	if (slowpath(!key)) {
		return;
	}
	dispatch_queue_specific_t dqs;

	dqs = _dispatch_calloc(1, sizeof(struct dispatch_queue_specific_s));
	dqs->dqs_key = key;
	dqs->dqs_ctxt = ctxt;
	dqs->dqs_destructor = destructor;
	if (slowpath(!dq->dq_specific_q)) {
		_dispatch_queue_init_specific(dq);
	}
	_dispatch_barrier_trysync_f(dq->dq_specific_q, dqs,
			_dispatch_queue_set_specific);
}

static void
_dispatch_queue_get_specific(void *ctxt)
{
	void **ctxtp = ctxt;
	void *key = *ctxtp;
	dispatch_queue_specific_queue_t dqsq =
			(dispatch_queue_specific_queue_t)_dispatch_queue_get_current();
	dispatch_queue_specific_t dqs;

	TAILQ_FOREACH(dqs, &dqsq->dqsq_contexts, dqs_list) {
		if (dqs->dqs_key == key) {
			*ctxtp = dqs->dqs_ctxt;
			return;
		}
	}
	*ctxtp = NULL;
}

DISPATCH_NOINLINE
void *
dispatch_queue_get_specific(dispatch_queue_t dq, const void *key)
{
	if (slowpath(!key)) {
		return NULL;
	}
	void *ctxt = NULL;

	if (fastpath(dq->dq_specific_q)) {
		ctxt = (void *)key;
		dispatch_sync_f(dq->dq_specific_q, &ctxt, _dispatch_queue_get_specific);
	}
	return ctxt;
}

DISPATCH_NOINLINE
void *
dispatch_get_specific(const void *key)
{
	if (slowpath(!key)) {
		return NULL;
	}
	void *ctxt = NULL;
	dispatch_queue_t dq = _dispatch_queue_get_current();

	while (slowpath(dq)) {
		if (slowpath(dq->dq_specific_q)) {
			ctxt = (void *)key;
			dispatch_sync_f(dq->dq_specific_q, &ctxt,
					_dispatch_queue_get_specific);
			if (ctxt) break;
		}
		dq = dq->do_targetq;
	}
	return ctxt;
}

#pragma mark -
#pragma mark dispatch_queue_debug

size_t
_dispatch_queue_debug_attr(dispatch_queue_t dq, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	dispatch_queue_t target = dq->do_targetq;
	offset += dsnprintf(buf, bufsiz, "target = %s[%p], width = 0x%x, "
			"running = 0x%x, barrier = %d ", target && target->dq_label ?
			target->dq_label : "", target, dq->dq_width / 2,
			dq->dq_running / 2, dq->dq_running & 1);
	if (dq->dq_is_thread_bound) {
		offset += dsnprintf(buf, bufsiz, ", thread = 0x%x ",
				_dispatch_queue_get_bound_thread(dq));
	}
	return offset;
}

size_t
dispatch_queue_debug(dispatch_queue_t dq, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%s[%p] = { ",
			dq->dq_label ? dq->dq_label : dx_kind(dq), dq);
	offset += _dispatch_object_debug_attr(dq, &buf[offset], bufsiz - offset);
	offset += _dispatch_queue_debug_attr(dq, &buf[offset], bufsiz - offset);
	offset += dsnprintf(&buf[offset], bufsiz - offset, "}");
	return offset;
}

#if DISPATCH_DEBUG
void
dispatch_debug_queue(dispatch_queue_t dq, const char* str) {
	if (fastpath(dq)) {
		_dispatch_object_debug(dq, "%s", str);
	} else {
		_dispatch_log("queue[NULL]: %s", str);
	}
}
#endif

#if DISPATCH_PERF_MON && !DISPATCH_INTROSPECTION
static OSSpinLock _dispatch_stats_lock;
static struct {
	uint64_t time_total;
	uint64_t count_total;
	uint64_t thread_total;
} _dispatch_stats[65]; // ffs*/fls*() returns zero when no bits are set

static void
_dispatch_queue_merge_stats(uint64_t start)
{
	uint64_t delta = _dispatch_absolute_time() - start;
	unsigned long count;

	count = (unsigned long)_dispatch_thread_getspecific(dispatch_bcounter_key);
	_dispatch_thread_setspecific(dispatch_bcounter_key, NULL);

	int bucket = flsl((long)count);

	// 64-bit counters on 32-bit require a lock or a queue
	OSSpinLockLock(&_dispatch_stats_lock);

	_dispatch_stats[bucket].time_total += delta;
	_dispatch_stats[bucket].count_total += count;
	_dispatch_stats[bucket].thread_total++;

	OSSpinLockUnlock(&_dispatch_stats_lock);
}
#endif

#pragma mark -
#pragma mark dispatch_continuation_t

static void
_dispatch_force_cache_cleanup(void)
{
	dispatch_continuation_t dc;
	dc = _dispatch_thread_getspecific(dispatch_cache_key);
	if (dc) {
		_dispatch_thread_setspecific(dispatch_cache_key, NULL);
		_dispatch_cache_cleanup(dc);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_cache_cleanup(void *value)
{
	dispatch_continuation_t dc, next_dc = value;

	while ((dc = next_dc)) {
		next_dc = dc->do_next;
		_dispatch_continuation_free_to_heap(dc);
	}
}

#if DISPATCH_USE_MEMORYSTATUS_SOURCE
int _dispatch_continuation_cache_limit = DISPATCH_CONTINUATION_CACHE_LIMIT;

DISPATCH_NOINLINE
void
_dispatch_continuation_free_to_cache_limit(dispatch_continuation_t dc)
{
	_dispatch_continuation_free_to_heap(dc);
	dispatch_continuation_t next_dc;
	dc = _dispatch_thread_getspecific(dispatch_cache_key);
	int cnt;
	if (!dc || (cnt = dc->dc_cache_cnt -
			_dispatch_continuation_cache_limit) <= 0){
		return;
	}
	do {
		next_dc = dc->do_next;
		_dispatch_continuation_free_to_heap(dc);
	} while (--cnt && (dc = next_dc));
	_dispatch_thread_setspecific(dispatch_cache_key, next_dc);
}
#endif

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline void
_dispatch_continuation_redirect(dispatch_queue_t dq, dispatch_object_t dou)
{
	dispatch_continuation_t dc = dou._dc;

	(void)dispatch_atomic_add2o(dq, dq_running, 2, acquire);
	if (!DISPATCH_OBJ_IS_VTABLE(dc) &&
			(long)dc->do_vtable & DISPATCH_OBJ_SYNC_SLOW_BIT) {
		_dispatch_trace_continuation_pop(dq, dou);
		_dispatch_thread_semaphore_signal(
				(_dispatch_thread_semaphore_t)dc->dc_other);
		_dispatch_introspection_queue_item_complete(dou);
	} else {
		_dispatch_async_f_redirect(dq, dc,
				_dispatch_queue_get_override_priority(dq));
	}
	_dispatch_perfmon_workitem_inc();
}

#pragma mark -
#pragma mark dispatch_block_create

#if __BLOCKS__

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_block_flags_valid(dispatch_block_flags_t flags)
{
	return ((flags & ~DISPATCH_BLOCK_API_MASK) == 0);
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_block_flags_t
_dispatch_block_normalize_flags(dispatch_block_flags_t flags)
{
	if (flags & (DISPATCH_BLOCK_NO_VOUCHER|DISPATCH_BLOCK_DETACHED)) {
		flags |= DISPATCH_BLOCK_HAS_VOUCHER;
	}
	if (flags & (DISPATCH_BLOCK_NO_QOS_CLASS|DISPATCH_BLOCK_DETACHED)) {
		flags |= DISPATCH_BLOCK_HAS_PRIORITY;
	}
	return flags;
}

static inline dispatch_block_t
_dispatch_block_create_with_voucher_and_priority(dispatch_block_flags_t flags,
		voucher_t voucher, pthread_priority_t pri, dispatch_block_t block)
{
	flags = _dispatch_block_normalize_flags(flags);
	voucher_t cv = NULL;
	bool assign = (flags & DISPATCH_BLOCK_ASSIGN_CURRENT);
	if (assign && !(flags & DISPATCH_BLOCK_HAS_VOUCHER)) {
		voucher = cv = voucher_copy();
		flags |= DISPATCH_BLOCK_HAS_VOUCHER;
	}
	if (assign && !(flags & DISPATCH_BLOCK_HAS_PRIORITY)) {
		pri = _dispatch_priority_propagate();
		flags |= DISPATCH_BLOCK_HAS_PRIORITY;
	}
	dispatch_block_t db = _dispatch_block_create(flags, voucher, pri, block);
	if (cv) _voucher_release(cv);
#if DISPATCH_DEBUG
	dispatch_assert(_dispatch_block_get_data(db));
#endif
	return db;
}

dispatch_block_t
dispatch_block_create(dispatch_block_flags_t flags, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) return NULL;
	return _dispatch_block_create_with_voucher_and_priority(flags, NULL, 0,
			block);
}

dispatch_block_t
dispatch_block_create_with_qos_class(dispatch_block_flags_t flags,
		dispatch_qos_class_t qos_class, int relative_priority,
		dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) return NULL;
	if (!_dispatch_qos_class_valid(qos_class, relative_priority)) return NULL;
	flags |= DISPATCH_BLOCK_HAS_PRIORITY;
	pthread_priority_t pri = 0;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pri = _pthread_qos_class_encode(qos_class, relative_priority, 0);
#endif
	return _dispatch_block_create_with_voucher_and_priority(flags, NULL,
			pri, block);
}

dispatch_block_t
dispatch_block_create_with_voucher(dispatch_block_flags_t flags,
		voucher_t voucher, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) return NULL;
	flags |= DISPATCH_BLOCK_HAS_VOUCHER;
	return _dispatch_block_create_with_voucher_and_priority(flags, voucher, 0,
			block);
}

dispatch_block_t
dispatch_block_create_with_voucher_and_qos_class(dispatch_block_flags_t flags,
		voucher_t voucher, dispatch_qos_class_t qos_class,
		int relative_priority, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) return NULL;
	if (!_dispatch_qos_class_valid(qos_class, relative_priority)) return NULL;
	flags |= (DISPATCH_BLOCK_HAS_VOUCHER|DISPATCH_BLOCK_HAS_PRIORITY);
	pthread_priority_t pri = 0;
#if HAVE_PTHREAD_WORKQUEUE_QOS
	pri = _pthread_qos_class_encode(qos_class, relative_priority, 0);
#endif
	return _dispatch_block_create_with_voucher_and_priority(flags, voucher,
			pri, block);
}

void
dispatch_block_perform(dispatch_block_flags_t flags, dispatch_block_t block)
{
	if (!_dispatch_block_flags_valid(flags)) {
		DISPATCH_CLIENT_CRASH("Invalid flags passed to "
				"dispatch_block_perform()");
	}
	flags = _dispatch_block_normalize_flags(flags);
	struct dispatch_block_private_data_s dbpds =
			DISPATCH_BLOCK_PRIVATE_DATA_INITIALIZER(flags, NULL, 0, block);
	dbpds.dbpd_atomic_flags |= DBF_PERFORM; // no group_leave at end of invoke
	return _dispatch_block_invoke(&dbpds);
}

#define _dbpd_group(dbpd) ((dispatch_group_t)&(dbpd)->dbpd_group)

void
_dispatch_block_invoke(const struct dispatch_block_private_data_s *dbcpd)
{
	dispatch_block_private_data_t dbpd = (dispatch_block_private_data_t)dbcpd;
	dispatch_block_flags_t flags = dbpd->dbpd_flags;
	unsigned int atomic_flags = dbpd->dbpd_atomic_flags;
	if (slowpath(atomic_flags & DBF_WAITED)) {
		DISPATCH_CLIENT_CRASH("A block object may not be both run more "
				"than once and waited for");
	}
	if (atomic_flags & DBF_CANCELED) goto out;

	pthread_priority_t op = DISPATCH_NO_PRIORITY, p = DISPATCH_NO_PRIORITY;
	unsigned long override = 0;
	if (flags & DISPATCH_BLOCK_HAS_PRIORITY) {
		op = _dispatch_get_priority();
		p = dbpd->dbpd_priority;
		override = (flags & DISPATCH_BLOCK_ENFORCE_QOS_CLASS) ||
				!(flags & DISPATCH_BLOCK_INHERIT_QOS_CLASS) ?
				 DISPATCH_PRIORITY_ENFORCE : 0;
	}
	voucher_t ov, v = DISPATCH_NO_VOUCHER;
	if (flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		v = dbpd->dbpd_voucher;
		if (v) _voucher_retain(v);
	}
	ov = _dispatch_adopt_priority_and_voucher(p, v, override);
	dbpd->dbpd_thread = _dispatch_thread_port();
	dbpd->dbpd_block();
	_dispatch_set_priority_and_replace_voucher(op, ov);
out:
	if ((atomic_flags & DBF_PERFORM) == 0) {
		if (dispatch_atomic_inc2o(dbpd, dbpd_performed, acquire) == 1) {
			dispatch_group_leave(_dbpd_group(dbpd));
		}
	}
}

static void
_dispatch_block_sync_invoke(void *block)
{
	dispatch_block_t b = block;
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(b);
	dispatch_block_flags_t flags = dbpd->dbpd_flags;
	unsigned int atomic_flags = dbpd->dbpd_atomic_flags;
	if (slowpath(atomic_flags & DBF_WAITED)) {
		DISPATCH_CLIENT_CRASH("A block object may not be both run more "
				"than once and waited for");
	}
	if (atomic_flags & DBF_CANCELED) goto out;

	pthread_priority_t op = DISPATCH_NO_PRIORITY, p = DISPATCH_NO_PRIORITY;
	unsigned long override = 0;
	if (flags & DISPATCH_BLOCK_HAS_PRIORITY) {
		op = _dispatch_get_priority();
		p = dbpd->dbpd_priority;
		override = (flags & DISPATCH_BLOCK_ENFORCE_QOS_CLASS) ||
				!(flags & DISPATCH_BLOCK_INHERIT_QOS_CLASS) ?
				 DISPATCH_PRIORITY_ENFORCE : 0;
	}
	voucher_t ov, v = DISPATCH_NO_VOUCHER;
	if (flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		v = dbpd->dbpd_voucher;
		if (v) _voucher_retain(v);
	}
	ov = _dispatch_adopt_priority_and_voucher(p, v, override);
	dbpd->dbpd_block();
	_dispatch_set_priority_and_replace_voucher(op, ov);
out:
	if ((atomic_flags & DBF_PERFORM) == 0) {
		if (dispatch_atomic_inc2o(dbpd, dbpd_performed, acquire) == 1) {
			dispatch_group_leave(_dbpd_group(dbpd));
		}
	}

	dispatch_queue_t dq = _dispatch_queue_get_current();
	if (dispatch_atomic_cmpxchg2o(dbpd, dbpd_queue, dq, NULL, acquire)) {
		// balances dispatch_{,barrier_,}sync
		_dispatch_release(dq);
	}
}

static void
_dispatch_block_async_invoke_and_release(void *block)
{
	dispatch_block_t b = block;
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(b);
	dispatch_block_flags_t flags = dbpd->dbpd_flags;
	unsigned int atomic_flags = dbpd->dbpd_atomic_flags;
	if (slowpath(atomic_flags & DBF_WAITED)) {
		DISPATCH_CLIENT_CRASH("A block object may not be both run more "
				"than once and waited for");
	}
	if (atomic_flags & DBF_CANCELED) goto out;

	pthread_priority_t p = DISPATCH_NO_PRIORITY;
	unsigned long override = 0;
	if (flags & DISPATCH_BLOCK_HAS_PRIORITY) {
		override = (flags & DISPATCH_BLOCK_ENFORCE_QOS_CLASS) ?
				DISPATCH_PRIORITY_ENFORCE : 0;
		p = dbpd->dbpd_priority;
	}
	voucher_t v = DISPATCH_NO_VOUCHER;
	if (flags & DISPATCH_BLOCK_HAS_VOUCHER) {
		v = dbpd->dbpd_voucher;
		if (v) _voucher_retain(v);
	}
	_dispatch_adopt_priority_and_replace_voucher(p, v, override);
	dbpd->dbpd_block();
out:
	if ((atomic_flags & DBF_PERFORM) == 0) {
		if (dispatch_atomic_inc2o(dbpd, dbpd_performed, acquire) == 1) {
			dispatch_group_leave(_dbpd_group(dbpd));
		}
	}
	dispatch_queue_t dq = _dispatch_queue_get_current();
	if (dispatch_atomic_cmpxchg2o(dbpd, dbpd_queue, dq, NULL, acquire)) {
		// balances dispatch_{,barrier_,group_}async
		_dispatch_release(dq);
	}
	Block_release(b);
}

void
dispatch_block_cancel(dispatch_block_t db)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (!dbpd) {
		DISPATCH_CLIENT_CRASH("Invalid block object passed to "
				"dispatch_block_cancel()");
	}
	(void)dispatch_atomic_or2o(dbpd, dbpd_atomic_flags, DBF_CANCELED, relaxed);
}

long
dispatch_block_testcancel(dispatch_block_t db)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (!dbpd) {
		DISPATCH_CLIENT_CRASH("Invalid block object passed to "
				"dispatch_block_testcancel()");
	}
	return (bool)(dbpd->dbpd_atomic_flags & DBF_CANCELED);
}

long
dispatch_block_wait(dispatch_block_t db, dispatch_time_t timeout)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (!dbpd) {
		DISPATCH_CLIENT_CRASH("Invalid block object passed to "
				"dispatch_block_wait()");
	}

	unsigned int flags = dispatch_atomic_or_orig2o(dbpd, dbpd_atomic_flags,
			DBF_WAITING, relaxed);
	if (slowpath(flags & (DBF_WAITED | DBF_WAITING))) {
		DISPATCH_CLIENT_CRASH("A block object may not be waited for "
				"more than once");
	}

	// <rdar://problem/17703192> If we know the queue where this block is
	// enqueued, or the thread that's executing it, then we should boost
	// it here.

	pthread_priority_t pp = _dispatch_get_priority();

	dispatch_queue_t boost_dq;
	boost_dq = dispatch_atomic_xchg2o(dbpd, dbpd_queue, NULL, acquire);
	if (boost_dq) {
		// release balances dispatch_{,barrier_,group_}async.
		// Can't put the queue back in the timeout case: the block might
		// finish after we fell out of group_wait and see our NULL, so
		// neither of us would ever release. Side effect: After a _wait
		// that times out, subsequent waits will not boost the qos of the
		// still-running block.
		_dispatch_queue_wakeup_with_qos_and_release(boost_dq, pp);
	}

	mach_port_t boost_th = dbpd->dbpd_thread;
	if (boost_th) {
		_dispatch_thread_override_start(boost_th, pp);
	}

	int performed = dispatch_atomic_load2o(dbpd, dbpd_performed, relaxed);
	if (slowpath(performed > 1 || (boost_th && boost_dq))) {
		DISPATCH_CLIENT_CRASH("A block object may not be both run more "
				"than once and waited for");
	}

	long ret = dispatch_group_wait(_dbpd_group(dbpd), timeout);

	if (boost_th) {
		_dispatch_thread_override_end(boost_th);
	}

	if (ret) {
		// timed out: reverse our changes
		(void)dispatch_atomic_and2o(dbpd, dbpd_atomic_flags,
				~DBF_WAITING, relaxed);
	} else {
		(void)dispatch_atomic_or2o(dbpd, dbpd_atomic_flags,
				DBF_WAITED, relaxed);
		// don't need to re-test here: the second call would see
		// the first call's WAITING
	}

	return ret;
}

void
dispatch_block_notify(dispatch_block_t db, dispatch_queue_t queue,
		dispatch_block_t notification_block)
{
	dispatch_block_private_data_t dbpd = _dispatch_block_get_data(db);
	if (!dbpd) {
		DISPATCH_CLIENT_CRASH("Invalid block object passed to "
				"dispatch_block_notify()");
	}
	int performed = dispatch_atomic_load2o(dbpd, dbpd_performed, relaxed);
	if (slowpath(performed > 1)) {
		DISPATCH_CLIENT_CRASH("A block object may not be both run more "
				"than once and observed");
	}

	return dispatch_group_notify(_dbpd_group(dbpd), queue, notification_block);
}

#endif // __BLOCKS__

#pragma mark -
#pragma mark dispatch_barrier_async

DISPATCH_NOINLINE
static void
_dispatch_barrier_async_f_slow(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp,
		dispatch_block_flags_t flags)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc_from_heap();

	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT | DISPATCH_OBJ_BARRIER_BIT);
	dc->dc_func = func;
	dc->dc_ctxt = ctxt;
	_dispatch_continuation_voucher_set(dc, flags);
	_dispatch_continuation_priority_set(dc, pp, flags);

	pp = _dispatch_continuation_get_override_priority(dq, dc);

	_dispatch_queue_push(dq, dc, pp);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_barrier_async_f2(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp,
		dispatch_block_flags_t flags)
{
	dispatch_continuation_t dc;

	dc = fastpath(_dispatch_continuation_alloc_cacheonly());
	if (!dc) {
		return _dispatch_barrier_async_f_slow(dq, ctxt, func, pp, flags);
	}

	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT | DISPATCH_OBJ_BARRIER_BIT);
	dc->dc_func = func;
	dc->dc_ctxt = ctxt;
	_dispatch_continuation_voucher_set(dc, flags);
	_dispatch_continuation_priority_set(dc, pp, flags);

	pp = _dispatch_continuation_get_override_priority(dq, dc);

	_dispatch_queue_push(dq, dc, pp);
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_async_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp,
		dispatch_block_flags_t flags)
{
	return _dispatch_barrier_async_f2(dq, ctxt, func, pp, flags);
}

DISPATCH_NOINLINE
void
dispatch_barrier_async_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	return _dispatch_barrier_async_f2(dq, ctxt, func, 0, 0);
}

DISPATCH_NOINLINE
void
_dispatch_barrier_async_detached_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	return _dispatch_barrier_async_f2(dq, ctxt, func, 0,
			DISPATCH_BLOCK_NO_QOS_CLASS|DISPATCH_BLOCK_NO_VOUCHER);
}

#ifdef __BLOCKS__
void
dispatch_barrier_async(dispatch_queue_t dq, void (^work)(void))
{
	dispatch_function_t func = _dispatch_call_block_and_release;
	pthread_priority_t pp = 0;
	dispatch_block_flags_t flags = 0;
	if (slowpath(_dispatch_block_has_private_data(work))) {
		func = _dispatch_block_async_invoke_and_release;
		pp = _dispatch_block_get_priority(work);
		flags = _dispatch_block_get_flags(work);
		// balanced in d_block_async_invoke_and_release or d_block_wait
		if (dispatch_atomic_cmpxchg2o(_dispatch_block_get_data(work),
					dbpd_queue, NULL, dq, release)) {
			_dispatch_retain(dq);
		}
	}
	_dispatch_barrier_async_f(dq, _dispatch_Block_copy(work), func, pp, flags);
}
#endif

#pragma mark -
#pragma mark dispatch_async

void
_dispatch_async_redirect_invoke(void *ctxt)
{
	struct dispatch_continuation_s *dc = ctxt;
	struct dispatch_continuation_s *other_dc = dc->dc_other;
	dispatch_queue_t old_dq, dq = dc->dc_data, rq;

	old_dq = _dispatch_thread_getspecific(dispatch_queue_key);
	_dispatch_thread_setspecific(dispatch_queue_key, dq);
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(dq->dq_priority);
	_dispatch_continuation_pop(other_dc);
	_dispatch_reset_defaultpriority(old_dp);
	_dispatch_thread_setspecific(dispatch_queue_key, old_dq);

	rq = dq->do_targetq;
	while (slowpath(rq->do_targetq) && rq != old_dq) {
		if (dispatch_atomic_sub2o(rq, dq_running, 2, relaxed) == 0) {
			_dispatch_queue_wakeup(rq);
		}
		rq = rq->do_targetq;
	}

	if (dispatch_atomic_sub2o(dq, dq_running, 2, relaxed) == 0) {
		_dispatch_queue_wakeup(dq);
	}
	_dispatch_release(dq);
}

static inline void
_dispatch_async_f_redirect2(dispatch_queue_t dq, dispatch_continuation_t dc,
		pthread_priority_t pp)
{
	uint32_t running = 2;

	// Find the queue to redirect to
	do {
		if (slowpath(dq->dq_items_tail) ||
				slowpath(DISPATCH_OBJECT_SUSPENDED(dq)) ||
				slowpath(dq->dq_width == 1)) {
			break;
		}
		running = dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
		if (slowpath(running & 1) || slowpath(running > dq->dq_width)) {
			running = dispatch_atomic_sub2o(dq, dq_running, 2, relaxed);
			break;
		}
		dq = dq->do_targetq;
	} while (slowpath(dq->do_targetq));

	_dispatch_queue_push_wakeup(dq, dc, pp, running == 0);
}

DISPATCH_NOINLINE
static void
_dispatch_async_f_redirect(dispatch_queue_t dq,
		dispatch_continuation_t other_dc, pthread_priority_t pp)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc();

	dc->do_vtable = (void *)DISPATCH_OBJ_ASYNC_BIT;
	dc->dc_func = _dispatch_async_redirect_invoke;
	dc->dc_ctxt = dc;
	dc->dc_data = dq;
	dc->dc_other = other_dc;
	dc->dc_priority = 0;
	dc->dc_voucher = NULL;

	_dispatch_retain(dq);
	dq = dq->do_targetq;
	if (slowpath(dq->do_targetq)) {
		return _dispatch_async_f_redirect2(dq, dc, pp);
	}

	_dispatch_queue_push(dq, dc, pp);
}

DISPATCH_NOINLINE
static void
_dispatch_async_f2(dispatch_queue_t dq, dispatch_continuation_t dc,
		pthread_priority_t pp)
{
	uint32_t running = 2;

	do {
		if (slowpath(dq->dq_items_tail)
				|| slowpath(DISPATCH_OBJECT_SUSPENDED(dq))) {
			break;
		}
		running = dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
		if (slowpath(running > dq->dq_width)) {
			running = dispatch_atomic_sub2o(dq, dq_running, 2, relaxed);
			break;
		}
		if (!slowpath(running & 1)) {
			return _dispatch_async_f_redirect(dq, dc, pp);
		}
		running = dispatch_atomic_sub2o(dq, dq_running, 2, relaxed);
		// We might get lucky and find that the barrier has ended by now
	} while (!(running & 1));

	_dispatch_queue_push_wakeup(dq, dc, pp, running == 0);
}

DISPATCH_NOINLINE
static void
_dispatch_async_f_slow(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp,
		dispatch_block_flags_t flags)
{
	dispatch_continuation_t dc = _dispatch_continuation_alloc_from_heap();

	dc->do_vtable = (void *)DISPATCH_OBJ_ASYNC_BIT;
	dc->dc_func = func;
	dc->dc_ctxt = ctxt;
	_dispatch_continuation_voucher_set(dc, flags);
	_dispatch_continuation_priority_set(dc, pp, flags);

	pp = _dispatch_continuation_get_override_priority(dq, dc);

	// No fastpath/slowpath hint because we simply don't know
	if (dq->do_targetq) {
		return _dispatch_async_f2(dq, dc, pp);
	}

	_dispatch_queue_push(dq, dc, pp);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_async_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func,
		pthread_priority_t pp, dispatch_block_flags_t flags)
{
	dispatch_continuation_t dc;

	// No fastpath/slowpath hint because we simply don't know
	if (dq->dq_width == 1 || flags & DISPATCH_BLOCK_BARRIER) {
		return _dispatch_barrier_async_f(dq, ctxt, func, pp, flags);
	}

	dc = fastpath(_dispatch_continuation_alloc_cacheonly());
	if (!dc) {
		return _dispatch_async_f_slow(dq, ctxt, func, pp, flags);
	}

	dc->do_vtable = (void *)DISPATCH_OBJ_ASYNC_BIT;
	dc->dc_func = func;
	dc->dc_ctxt = ctxt;
	_dispatch_continuation_voucher_set(dc, flags);
	_dispatch_continuation_priority_set(dc, pp, flags);

	pp = _dispatch_continuation_get_override_priority(dq, dc);

	// No fastpath/slowpath hint because we simply don't know
	if (dq->do_targetq) {
		return _dispatch_async_f2(dq, dc, pp);
	}

	_dispatch_queue_push(dq, dc, pp);
}

DISPATCH_NOINLINE
void
dispatch_async_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func)
{
	return _dispatch_async_f(dq, ctxt, func, 0, 0);
}

#ifdef __BLOCKS__
void
dispatch_async(dispatch_queue_t dq, void (^work)(void))
{
	dispatch_function_t func = _dispatch_call_block_and_release;
	dispatch_block_flags_t flags = 0;
	pthread_priority_t pp = 0;
	if (slowpath(_dispatch_block_has_private_data(work))) {
		func = _dispatch_block_async_invoke_and_release;
		pp = _dispatch_block_get_priority(work);
		flags = _dispatch_block_get_flags(work);
		// balanced in d_block_async_invoke_and_release or d_block_wait
		if (dispatch_atomic_cmpxchg2o(_dispatch_block_get_data(work),
					dbpd_queue, NULL, dq, release)) {
			_dispatch_retain(dq);
		}
	}
	_dispatch_async_f(dq, _dispatch_Block_copy(work), func, pp, flags);
}
#endif

#pragma mark -
#pragma mark dispatch_group_async

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_group_async_f(dispatch_group_t dg, dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp,
		dispatch_block_flags_t flags)
{
	dispatch_continuation_t dc;

	_dispatch_retain(dg);
	dispatch_group_enter(dg);

	dc = _dispatch_continuation_alloc();

	unsigned long barrier = (flags & DISPATCH_BLOCK_BARRIER) ?
			DISPATCH_OBJ_BARRIER_BIT : 0;
	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT | DISPATCH_OBJ_GROUP_BIT |
			barrier);
	dc->dc_func = func;
	dc->dc_ctxt = ctxt;
	dc->dc_data = dg;
	_dispatch_continuation_voucher_set(dc, flags);
	_dispatch_continuation_priority_set(dc, pp, flags);

	pp = _dispatch_continuation_get_override_priority(dq, dc);

	// No fastpath/slowpath hint because we simply don't know
	if (dq->dq_width != 1 && !barrier && dq->do_targetq) {
		return _dispatch_async_f2(dq, dc, pp);
	}

	_dispatch_queue_push(dq, dc, pp);
}

DISPATCH_NOINLINE
void
dispatch_group_async_f(dispatch_group_t dg, dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	return _dispatch_group_async_f(dg, dq, ctxt, func, 0, 0);
}

#ifdef __BLOCKS__
void
dispatch_group_async(dispatch_group_t dg, dispatch_queue_t dq,
		dispatch_block_t db)
{
	dispatch_function_t func = _dispatch_call_block_and_release;
	dispatch_block_flags_t flags = 0;
	pthread_priority_t pp = 0;
	if (slowpath(_dispatch_block_has_private_data(db))) {
		func = _dispatch_block_async_invoke_and_release;
		pp = _dispatch_block_get_priority(db);
		flags = _dispatch_block_get_flags(db);
		// balanced in d_block_async_invoke_and_release or d_block_wait
		if (dispatch_atomic_cmpxchg2o(_dispatch_block_get_data(db),
					dbpd_queue, NULL, dq, release)) {
			_dispatch_retain(dq);
		}
	}
	_dispatch_group_async_f(dg, dq, _dispatch_Block_copy(db), func, pp, flags);
}
#endif

#pragma mark -
#pragma mark dispatch_function_invoke

static void _dispatch_sync_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_function_invoke(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	dispatch_queue_t old_dq = _dispatch_thread_getspecific(dispatch_queue_key);
	_dispatch_thread_setspecific(dispatch_queue_key, dq);
	_dispatch_client_callout(ctxt, func);
	_dispatch_perfmon_workitem_inc();
	_dispatch_thread_setspecific(dispatch_queue_key, old_dq);
}

void
_dispatch_sync_recurse_invoke(void *ctxt)
{
	dispatch_continuation_t dc = ctxt;
	_dispatch_function_invoke(dc->dc_data, dc->dc_ctxt, dc->dc_func);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_function_recurse(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp)
{
	struct dispatch_continuation_s dc = {
		.dc_data = dq,
		.dc_func = func,
		.dc_ctxt = ctxt,
	};
	_dispatch_sync_f(dq->do_targetq, &dc, _dispatch_sync_recurse_invoke, pp);
}

#pragma mark -
#pragma mark dispatch_barrier_sync

static void _dispatch_sync_f_invoke(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func);

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline _dispatch_thread_semaphore_t
_dispatch_barrier_sync_f_pop(dispatch_queue_t dq, dispatch_object_t dou,
		bool lock)
{
	_dispatch_thread_semaphore_t sema;
	dispatch_continuation_t dc = dou._dc;
	mach_port_t th;

	if (DISPATCH_OBJ_IS_VTABLE(dc) || ((long)dc->do_vtable &
			(DISPATCH_OBJ_BARRIER_BIT | DISPATCH_OBJ_SYNC_SLOW_BIT)) !=
			(DISPATCH_OBJ_BARRIER_BIT | DISPATCH_OBJ_SYNC_SLOW_BIT)) {
		return 0;
	}
	_dispatch_trace_continuation_pop(dq, dc);
	_dispatch_perfmon_workitem_inc();

	th = (mach_port_t)dc->dc_data;
	dc = dc->dc_ctxt;
	dq = dc->dc_data;
	sema = (_dispatch_thread_semaphore_t)dc->dc_other;
	if (lock) {
		(void)dispatch_atomic_add2o(dq, do_suspend_cnt,
				DISPATCH_OBJECT_SUSPEND_INTERVAL, relaxed);
		// rdar://problem/9032024 running lock must be held until sync_f_slow
		// returns
		(void)dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
	}
	_dispatch_introspection_queue_item_complete(dou);
	_dispatch_wqthread_override_start(th,
			_dispatch_queue_get_override_priority(dq));
	return sema ? sema : MACH_PORT_DEAD;
}

static void
_dispatch_barrier_sync_f_slow_invoke(void *ctxt)
{
	dispatch_continuation_t dc = ctxt;
	dispatch_queue_t dq = dc->dc_data;
	_dispatch_thread_semaphore_t sema;
	sema = (_dispatch_thread_semaphore_t)dc->dc_other;

	dispatch_assert(dq == _dispatch_queue_get_current());
#if DISPATCH_COCOA_COMPAT
	if (slowpath(dq->dq_is_thread_bound)) {
		// The queue is bound to a non-dispatch thread (e.g. main thread)
		_dispatch_continuation_voucher_adopt(dc);
		_dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
		dispatch_atomic_store2o(dc, dc_func, NULL, release);
		_dispatch_thread_semaphore_signal(sema); // release
		return;
	}
#endif
	(void)dispatch_atomic_add2o(dq, do_suspend_cnt,
			DISPATCH_OBJECT_SUSPEND_INTERVAL, relaxed);
	// rdar://9032024 running lock must be held until sync_f_slow returns
	(void)dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
	_dispatch_thread_semaphore_signal(sema); // release
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_f_slow(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp)
{
	if (slowpath(!dq->do_targetq)) {
		// the global concurrent queues do not need strict ordering
		(void)dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
		return _dispatch_sync_f_invoke(dq, ctxt, func);
	}
	if (!pp) pp = (_dispatch_get_priority() | _PTHREAD_PRIORITY_ENFORCE_FLAG);
	_dispatch_thread_semaphore_t sema = _dispatch_get_thread_semaphore();
	struct dispatch_continuation_s dc = {
		.dc_data = dq,
#if DISPATCH_COCOA_COMPAT
		.dc_func = func,
		.dc_ctxt = ctxt,
#endif
		.dc_other = (void*)sema,
	};
#if DISPATCH_COCOA_COMPAT
	// It's preferred to execute synchronous blocks on the current thread
	// due to thread-local side effects, garbage collection, etc. However,
	// blocks submitted to the main thread MUST be run on the main thread
	if (slowpath(dq->dq_is_thread_bound)) {
		_dispatch_continuation_voucher_set(&dc, 0);
	}
#endif
	struct dispatch_continuation_s dbss = {
		.do_vtable = (void *)(DISPATCH_OBJ_BARRIER_BIT |
				DISPATCH_OBJ_SYNC_SLOW_BIT),
		.dc_func = _dispatch_barrier_sync_f_slow_invoke,
		.dc_ctxt = &dc,
		.dc_data = (void*)(uintptr_t)_dispatch_thread_port(),
		.dc_priority = pp,
	};
	_dispatch_queue_push(dq, &dbss,
			_dispatch_continuation_get_override_priority(dq, &dbss));

	_dispatch_thread_semaphore_wait(sema); // acquire
	_dispatch_put_thread_semaphore(sema);

#if DISPATCH_COCOA_COMPAT
	// Queue bound to a non-dispatch thread
	if (dc.dc_func == NULL) {
		return;
	}
#endif

	_dispatch_queue_set_thread(dq);
	if (slowpath(dq->do_targetq->do_targetq)) {
		_dispatch_function_recurse(dq, ctxt, func, pp);
	} else {
		_dispatch_function_invoke(dq, ctxt, func);
	}
	_dispatch_queue_clear_thread(dq);

	if (fastpath(dq->do_suspend_cnt < 2 * DISPATCH_OBJECT_SUSPEND_INTERVAL) &&
			dq->dq_running == 2) {
		// rdar://problem/8290662 "lock transfer"
		sema = _dispatch_queue_drain_one_barrier_sync(dq);
		if (sema) {
			_dispatch_thread_semaphore_signal(sema); // release
			return;
		}
	}
	(void)dispatch_atomic_sub2o(dq, do_suspend_cnt,
			DISPATCH_OBJECT_SUSPEND_INTERVAL, release);
	if (slowpath(dispatch_atomic_sub2o(dq, dq_running, 2, relaxed) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_f2(dispatch_queue_t dq)
{
	if (!slowpath(DISPATCH_OBJECT_SUSPENDED(dq))) {
		// rdar://problem/8290662 "lock transfer"
		_dispatch_thread_semaphore_t sema;
		sema = _dispatch_queue_drain_one_barrier_sync(dq);
		if (sema) {
			(void)dispatch_atomic_add2o(dq, do_suspend_cnt,
					DISPATCH_OBJECT_SUSPEND_INTERVAL, relaxed);
			// rdar://9032024 running lock must be held until sync_f_slow
			// returns: increment by 2 and decrement by 1
			(void)dispatch_atomic_inc2o(dq, dq_running, relaxed);
			_dispatch_thread_semaphore_signal(sema);
			return;
		}
	}
	if (slowpath(dispatch_atomic_dec2o(dq, dq_running, release) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_f_invoke(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	_dispatch_queue_set_thread(dq);
	_dispatch_function_invoke(dq, ctxt, func);
	_dispatch_queue_clear_thread(dq);
	if (slowpath(dq->dq_items_tail)) {
		return _dispatch_barrier_sync_f2(dq);
	}
	if (slowpath(dispatch_atomic_dec2o(dq, dq_running, release) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_f_recurse(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp)
{
	_dispatch_queue_set_thread(dq);
	_dispatch_function_recurse(dq, ctxt, func, pp);
	_dispatch_queue_clear_thread(dq);
	if (slowpath(dq->dq_items_tail)) {
		return _dispatch_barrier_sync_f2(dq);
	}
	if (slowpath(dispatch_atomic_dec2o(dq, dq_running, release) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp)
{
	// 1) ensure that this thread hasn't enqueued anything ahead of this call
	// 2) the queue is not suspended
	if (slowpath(dq->dq_items_tail) || slowpath(DISPATCH_OBJECT_SUSPENDED(dq))){
		return _dispatch_barrier_sync_f_slow(dq, ctxt, func, pp);
	}
	if (slowpath(!dispatch_atomic_cmpxchg2o(dq, dq_running, 0, 1, acquire))) {
		// global concurrent queues and queues bound to non-dispatch threads
		// always fall into the slow case
		return _dispatch_barrier_sync_f_slow(dq, ctxt, func, pp);
	}
	if (slowpath(dq->do_targetq->do_targetq)) {
		return _dispatch_barrier_sync_f_recurse(dq, ctxt, func, pp);
	}
	_dispatch_barrier_sync_f_invoke(dq, ctxt, func);
}

DISPATCH_NOINLINE
void
dispatch_barrier_sync_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	// 1) ensure that this thread hasn't enqueued anything ahead of this call
	// 2) the queue is not suspended
	if (slowpath(dq->dq_items_tail) || slowpath(DISPATCH_OBJECT_SUSPENDED(dq))){
		return _dispatch_barrier_sync_f_slow(dq, ctxt, func, 0);
	}
	if (slowpath(!dispatch_atomic_cmpxchg2o(dq, dq_running, 0, 1, acquire))) {
		// global concurrent queues and queues bound to non-dispatch threads
		// always fall into the slow case
		return _dispatch_barrier_sync_f_slow(dq, ctxt, func, 0);
	}
	if (slowpath(dq->do_targetq->do_targetq)) {
		return _dispatch_barrier_sync_f_recurse(dq, ctxt, func, 0);
	}
	_dispatch_barrier_sync_f_invoke(dq, ctxt, func);
}

#ifdef __BLOCKS__
DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_slow(dispatch_queue_t dq, void (^work)(void))
{
	bool has_pd = _dispatch_block_has_private_data(work);
	dispatch_function_t func = _dispatch_Block_invoke(work);
	pthread_priority_t pp = 0;
	if (has_pd) {
		func = _dispatch_block_sync_invoke;
		pp = _dispatch_block_get_priority(work);
		dispatch_block_flags_t flags = _dispatch_block_get_flags(work);
		if (flags & DISPATCH_BLOCK_HAS_PRIORITY) {
			pthread_priority_t tp = _dispatch_get_priority();
			if (pp < tp) {
				pp = tp | _PTHREAD_PRIORITY_ENFORCE_FLAG;
			} else if (!(flags & DISPATCH_BLOCK_INHERIT_QOS_CLASS)) {
				pp |= _PTHREAD_PRIORITY_ENFORCE_FLAG;
			}
		}
		// balanced in d_block_sync_invoke or d_block_wait
		if (dispatch_atomic_cmpxchg2o(_dispatch_block_get_data(work),
					dbpd_queue, NULL, dq, release)) {
			_dispatch_retain(dq);
		}
#if DISPATCH_COCOA_COMPAT
	} else if (dq->dq_is_thread_bound && dispatch_begin_thread_4GC) {
		// Blocks submitted to the main queue MUST be run on the main thread,
		// under GC we must Block_copy in order to notify the thread-local
		// garbage collector that the objects are transferring to another thread
		// rdar://problem/7176237&7181849&7458685
		work = _dispatch_Block_copy(work);
		func = _dispatch_call_block_and_release;
	}
#endif
	_dispatch_barrier_sync_f(dq, work, func, pp);
}

void
dispatch_barrier_sync(dispatch_queue_t dq, void (^work)(void))
{
	if (slowpath(dq->dq_is_thread_bound) ||
			slowpath(_dispatch_block_has_private_data(work))) {
		return _dispatch_barrier_sync_slow(dq, work);
	}
	dispatch_barrier_sync_f(dq, work, _dispatch_Block_invoke(work));
}
#endif

DISPATCH_NOINLINE
static void
_dispatch_barrier_trysync_f_invoke(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	_dispatch_queue_set_thread(dq);
	_dispatch_function_invoke(dq, ctxt, func);
	_dispatch_queue_clear_thread(dq);
	if (slowpath(dispatch_atomic_dec2o(dq, dq_running, release) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_NOINLINE
void
_dispatch_barrier_trysync_f(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	// Use for mutation of queue-/source-internal state only, ignores target
	// queue hierarchy!
	if (slowpath(dq->dq_items_tail) || slowpath(DISPATCH_OBJECT_SUSPENDED(dq))
			|| slowpath(!dispatch_atomic_cmpxchg2o(dq, dq_running, 0, 1,
					acquire))) {
		return _dispatch_barrier_async_detached_f(dq, ctxt, func);
	}
	_dispatch_barrier_trysync_f_invoke(dq, ctxt, func);
}

#pragma mark -
#pragma mark dispatch_sync

DISPATCH_NOINLINE
static void
_dispatch_sync_f_slow(dispatch_queue_t dq, void *ctxt, dispatch_function_t func,
		pthread_priority_t pp, bool wakeup)
{
	if (!pp) pp = (_dispatch_get_priority() | _PTHREAD_PRIORITY_ENFORCE_FLAG);
	_dispatch_thread_semaphore_t sema = _dispatch_get_thread_semaphore();
	struct dispatch_continuation_s dc = {
		.do_vtable = (void*)DISPATCH_OBJ_SYNC_SLOW_BIT,
#if DISPATCH_INTROSPECTION
		.dc_func = func,
		.dc_ctxt = ctxt,
		.dc_data = (void*)(uintptr_t)_dispatch_thread_port(),
#endif
		.dc_other = (void*)sema,
		.dc_priority = pp,
	};
	_dispatch_queue_push_wakeup(dq, &dc,
			_dispatch_continuation_get_override_priority(dq, &dc), wakeup);

	_dispatch_thread_semaphore_wait(sema);
	_dispatch_put_thread_semaphore(sema);

	if (slowpath(dq->do_targetq->do_targetq)) {
		_dispatch_function_recurse(dq, ctxt, func, pp);
	} else {
		_dispatch_function_invoke(dq, ctxt, func);
	}

	if (slowpath(dispatch_atomic_sub2o(dq, dq_running, 2, relaxed) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_sync_f_invoke(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func)
{
	_dispatch_function_invoke(dq, ctxt, func);
	if (slowpath(dispatch_atomic_sub2o(dq, dq_running, 2, relaxed) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

DISPATCH_NOINLINE
static void
_dispatch_sync_f_recurse(dispatch_queue_t dq, void *ctxt,
		dispatch_function_t func, pthread_priority_t pp)
{
	_dispatch_function_recurse(dq, ctxt, func, pp);
	if (slowpath(dispatch_atomic_sub2o(dq, dq_running, 2, relaxed) == 0)) {
		_dispatch_queue_wakeup(dq);
	}
}

static inline void
_dispatch_sync_f2(dispatch_queue_t dq, void *ctxt, dispatch_function_t func,
		pthread_priority_t pp)
{
	// 1) ensure that this thread hasn't enqueued anything ahead of this call
	// 2) the queue is not suspended
	if (slowpath(dq->dq_items_tail) || slowpath(DISPATCH_OBJECT_SUSPENDED(dq))){
		return _dispatch_sync_f_slow(dq, ctxt, func, pp, false);
	}
	uint32_t running = dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
	// re-check suspension after barrier check <rdar://problem/15242126>
	if (slowpath(running & 1) || _dispatch_object_suspended(dq)) {
		running = dispatch_atomic_sub2o(dq, dq_running, 2, relaxed);
		return _dispatch_sync_f_slow(dq, ctxt, func, pp, running == 0);
	}
	if (slowpath(dq->do_targetq->do_targetq)) {
		return _dispatch_sync_f_recurse(dq, ctxt, func, pp);
	}
	_dispatch_sync_f_invoke(dq, ctxt, func);
}

DISPATCH_NOINLINE
static void
_dispatch_sync_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func,
		pthread_priority_t pp)
{
	if (fastpath(dq->dq_width == 1)) {
		return _dispatch_barrier_sync_f(dq, ctxt, func, pp);
	}
	if (slowpath(!dq->do_targetq)) {
		// the global concurrent queues do not need strict ordering
		(void)dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
		return _dispatch_sync_f_invoke(dq, ctxt, func);
	}
	_dispatch_sync_f2(dq, ctxt, func, pp);
}

DISPATCH_NOINLINE
void
dispatch_sync_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func)
{
	if (fastpath(dq->dq_width == 1)) {
		return dispatch_barrier_sync_f(dq, ctxt, func);
	}
	if (slowpath(!dq->do_targetq)) {
		// the global concurrent queues do not need strict ordering
		(void)dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
		return _dispatch_sync_f_invoke(dq, ctxt, func);
	}
	_dispatch_sync_f2(dq, ctxt, func, 0);
}

#ifdef __BLOCKS__
DISPATCH_NOINLINE
static void
_dispatch_sync_slow(dispatch_queue_t dq, void (^work)(void))
{
	bool has_pd = _dispatch_block_has_private_data(work);
	if (has_pd && (_dispatch_block_get_flags(work) & DISPATCH_BLOCK_BARRIER)) {
		return _dispatch_barrier_sync_slow(dq, work);
	}
	dispatch_function_t func = _dispatch_Block_invoke(work);
	pthread_priority_t pp = 0;
	if (has_pd) {
		func = _dispatch_block_sync_invoke;
		pp = _dispatch_block_get_priority(work);
		dispatch_block_flags_t flags = _dispatch_block_get_flags(work);
		if (flags & DISPATCH_BLOCK_HAS_PRIORITY) {
			pthread_priority_t tp = _dispatch_get_priority();
			if (pp < tp) {
				pp = tp | _PTHREAD_PRIORITY_ENFORCE_FLAG;
			} else if (!(flags & DISPATCH_BLOCK_INHERIT_QOS_CLASS)) {
				pp |= _PTHREAD_PRIORITY_ENFORCE_FLAG;
			}
		}
		// balanced in d_block_sync_invoke or d_block_wait
		if (dispatch_atomic_cmpxchg2o(_dispatch_block_get_data(work),
					dbpd_queue, NULL, dq, release)) {
			_dispatch_retain(dq);
		}
#if DISPATCH_COCOA_COMPAT
	} else if (dq->dq_is_thread_bound && dispatch_begin_thread_4GC) {
		// Blocks submitted to the main queue MUST be run on the main thread,
		// under GC we must Block_copy in order to notify the thread-local
		// garbage collector that the objects are transferring to another thread
		// rdar://problem/7176237&7181849&7458685
		work = _dispatch_Block_copy(work);
		func = _dispatch_call_block_and_release;
#endif
	}
	if (slowpath(!dq->do_targetq)) {
		// the global concurrent queues do not need strict ordering
		(void)dispatch_atomic_add2o(dq, dq_running, 2, relaxed);
		return _dispatch_sync_f_invoke(dq, work, func);
	}
	_dispatch_sync_f2(dq, work, func, pp);
}

void
dispatch_sync(dispatch_queue_t dq, void (^work)(void))
{
	if (fastpath(dq->dq_width == 1)) {
		return dispatch_barrier_sync(dq, work);
	}
	if (slowpath(dq->dq_is_thread_bound) ||
			slowpath(_dispatch_block_has_private_data(work)) ) {
		return _dispatch_sync_slow(dq, work);
	}
	dispatch_sync_f(dq, work, _dispatch_Block_invoke(work));
}
#endif

#pragma mark -
#pragma mark dispatch_after

void
_dispatch_after_timer_callback(void *ctxt)
{
	dispatch_continuation_t dc = ctxt, dc1;
	dispatch_source_t ds = dc->dc_data;
	dc1 = _dispatch_continuation_free_cacheonly(dc);
	_dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
	dispatch_source_cancel(ds);
	dispatch_release(ds);
	if (slowpath(dc1)) {
		_dispatch_continuation_free_to_cache_limit(dc1);
	}
}

DISPATCH_NOINLINE
void
dispatch_after_f(dispatch_time_t when, dispatch_queue_t queue, void *ctxt,
		dispatch_function_t func)
{
	uint64_t delta, leeway;
	dispatch_source_t ds;

	if (when == DISPATCH_TIME_FOREVER) {
#if DISPATCH_DEBUG
		DISPATCH_CLIENT_CRASH(
				"dispatch_after_f() called with 'when' == infinity");
#endif
		return;
	}

	delta = _dispatch_timeout(when);
	if (delta == 0) {
		return dispatch_async_f(queue, ctxt, func);
	}
	leeway = delta / 10; // <rdar://problem/13447496>
	if (leeway < NSEC_PER_MSEC) leeway = NSEC_PER_MSEC;
	if (leeway > 60 * NSEC_PER_SEC) leeway = 60 * NSEC_PER_SEC;

	// this function can and should be optimized to not use a dispatch source
	ds = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	dispatch_assert(ds);

	// TODO: don't use a separate continuation & voucher
	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT);
	dc->dc_func = func;
	dc->dc_ctxt = ctxt;
	dc->dc_data = ds;

	dispatch_set_context(ds, dc);
	dispatch_source_set_event_handler_f(ds, _dispatch_after_timer_callback);
	dispatch_source_set_timer(ds, when, DISPATCH_TIME_FOREVER, leeway);
	dispatch_resume(ds);
}

#ifdef __BLOCKS__
void
dispatch_after(dispatch_time_t when, dispatch_queue_t queue,
		dispatch_block_t work)
{
	// test before the copy of the block
	if (when == DISPATCH_TIME_FOREVER) {
#if DISPATCH_DEBUG
		DISPATCH_CLIENT_CRASH(
				"dispatch_after() called with 'when' == infinity");
#endif
		return;
	}
	dispatch_after_f(when, queue, _dispatch_Block_copy(work),
			_dispatch_call_block_and_release);
}
#endif

#pragma mark -
#pragma mark dispatch_queue_push

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_queue_push_list_slow2(dispatch_queue_t dq, pthread_priority_t pp,
		struct dispatch_object_s *obj, bool retained)
{
	// The queue must be retained before dq_items_head is written in order
	// to ensure that the reference is still valid when _dispatch_wakeup is
	// called. Otherwise, if preempted between the assignment to
	// dq_items_head and _dispatch_wakeup, the blocks submitted to the
	// queue may release the last reference to the queue when invoked by
	// _dispatch_queue_drain. <rdar://problem/6932776>
	if (!retained) _dispatch_retain(dq);
	dq->dq_items_head = obj;
	return _dispatch_queue_wakeup_with_qos_and_release(dq, pp);
}

DISPATCH_NOINLINE
void
_dispatch_queue_push_list_slow(dispatch_queue_t dq, pthread_priority_t pp,
		struct dispatch_object_s *obj, unsigned int n, bool retained)
{
	if (dx_type(dq) == DISPATCH_QUEUE_ROOT_TYPE && !dq->dq_is_thread_bound) {
		dispatch_assert(!retained);
		dispatch_atomic_store2o(dq, dq_items_head, obj, relaxed);
		return _dispatch_queue_wakeup_global2(dq, n);
	}
	_dispatch_queue_push_list_slow2(dq, pp, obj, retained);
}

DISPATCH_NOINLINE
void
_dispatch_queue_push_slow(dispatch_queue_t dq, pthread_priority_t pp,
		struct dispatch_object_s *obj, bool retained)
{
	if (dx_type(dq) == DISPATCH_QUEUE_ROOT_TYPE && !dq->dq_is_thread_bound) {
		dispatch_assert(!retained);
		dispatch_atomic_store2o(dq, dq_items_head, obj, relaxed);
		return _dispatch_queue_wakeup_global(dq);
	}
	_dispatch_queue_push_list_slow2(dq, pp, obj, retained);
}

#pragma mark -
#pragma mark dispatch_queue_probe

unsigned long
_dispatch_queue_probe(dispatch_queue_t dq)
{
	return _dispatch_queue_class_probe(dq);
}

#if DISPATCH_COCOA_COMPAT
unsigned long
_dispatch_runloop_queue_probe(dispatch_queue_t dq)
{
	if (_dispatch_queue_class_probe(dq)) {
		if (dq->do_xref_cnt == -1) return true; // <rdar://problem/14026816>
		return _dispatch_runloop_queue_wakeup(dq);
	}
	return false;
}
#endif

unsigned long
_dispatch_mgr_queue_probe(dispatch_queue_t dq)
{
	if (_dispatch_queue_class_probe(dq)) {
		return _dispatch_mgr_wakeup(dq);
	}
	return false;
}

unsigned long
_dispatch_root_queue_probe(dispatch_queue_t dq)
{
	_dispatch_queue_wakeup_global(dq);
	return false;
}

#pragma mark -
#pragma mark dispatch_wakeup

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
dispatch_queue_t
_dispatch_wakeup(dispatch_object_t dou)
{
	unsigned long type = dx_metatype(dou._do);
	if (type == _DISPATCH_QUEUE_TYPE || type == _DISPATCH_SOURCE_TYPE) {
		return _dispatch_queue_wakeup(dou._dq);
	}
	if (_dispatch_object_suspended(dou)) {
		return NULL;
	}
	if (!dx_probe(dou._do)) {
		return NULL;
	}
	if (!dispatch_atomic_cmpxchg2o(dou._do, do_suspend_cnt, 0,
			DISPATCH_OBJECT_SUSPEND_LOCK, acquire)) {
		return NULL;
	}
	_dispatch_retain(dou._do);
	dispatch_queue_t tq = dou._do->do_targetq;
	_dispatch_queue_push(tq, dou._do, 0);
	return tq;	// libdispatch does not need this, but the Instrument DTrace
				// probe does
}

#if DISPATCH_COCOA_COMPAT
static inline void
_dispatch_runloop_queue_wakeup_thread(dispatch_queue_t dq)
{
	mach_port_t mp = (mach_port_t)dq->do_ctxt;
	if (!mp) {
		return;
	}
	kern_return_t kr = _dispatch_send_wakeup_runloop_thread(mp, 0);
	switch (kr) {
	case MACH_SEND_TIMEOUT:
	case MACH_SEND_TIMED_OUT:
	case MACH_SEND_INVALID_DEST:
		break;
	default:
		(void)dispatch_assume_zero(kr);
		break;
	}
}

DISPATCH_NOINLINE DISPATCH_WEAK
unsigned long
_dispatch_runloop_queue_wakeup(dispatch_queue_t dq)
{
	_dispatch_runloop_queue_wakeup_thread(dq);
	return false;
}

DISPATCH_NOINLINE
static dispatch_queue_t
_dispatch_main_queue_wakeup(void)
{
	dispatch_queue_t dq = &_dispatch_main_q;
	if (!dq->dq_is_thread_bound) {
		return NULL;
	}
	dispatch_once_f(&_dispatch_main_q_port_pred, dq,
			_dispatch_runloop_queue_port_init);
	_dispatch_runloop_queue_wakeup_thread(dq);
	return NULL;
}
#endif

DISPATCH_NOINLINE
static void
_dispatch_queue_wakeup_global_slow(dispatch_queue_t dq, unsigned int n)
{
	dispatch_root_queue_context_t qc = dq->do_ctxt;
	uint32_t i = n;
	int r;

	_dispatch_debug_root_queue(dq, __func__);
	dispatch_once_f(&_dispatch_root_queues_pred, NULL,
			_dispatch_root_queues_init);

#if HAVE_PTHREAD_WORKQUEUES
#if DISPATCH_USE_PTHREAD_POOL
	if (qc->dgq_kworkqueue != (void*)(~0ul))
#endif
	{
		_dispatch_root_queue_debug("requesting new worker thread for global "
				"queue: %p", dq);
#if DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
		if (qc->dgq_kworkqueue) {
			pthread_workitem_handle_t wh;
			unsigned int gen_cnt;
			do {
				r = pthread_workqueue_additem_np(qc->dgq_kworkqueue,
						_dispatch_worker_thread4, dq, &wh, &gen_cnt);
				(void)dispatch_assume_zero(r);
			} while (--i);
			return;
		}
#endif // DISPATCH_USE_LEGACY_WORKQUEUE_FALLBACK
#if HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
		if (!dq->dq_priority) {
			r = pthread_workqueue_addthreads_np(qc->dgq_wq_priority,
					qc->dgq_wq_options, (int)i);
			(void)dispatch_assume_zero(r);
			return;
		}
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS
		r = _pthread_workqueue_addthreads((int)i, dq->dq_priority);
		(void)dispatch_assume_zero(r);
#endif
		return;
	}
#endif // HAVE_PTHREAD_WORKQUEUES
#if DISPATCH_USE_PTHREAD_POOL
	dispatch_pthread_root_queue_context_t pqc = qc->dgq_ctxt;
	if (fastpath(pqc->dpq_thread_mediator.do_vtable)) {
		while (dispatch_semaphore_signal(&pqc->dpq_thread_mediator)) {
			if (!--i) {
				return;
			}
		}
	}
	uint32_t j, t_count;
	// seq_cst with atomic store to tail <rdar://problem/16932833>
	t_count = dispatch_atomic_load2o(qc, dgq_thread_pool_size, seq_cst);
	do {
		if (!t_count) {
			_dispatch_root_queue_debug("pthread pool is full for root queue: "
					"%p", dq);
			return;
		}
		j = i > t_count ? t_count : i;
	} while (!dispatch_atomic_cmpxchgvw2o(qc, dgq_thread_pool_size, t_count,
			t_count - j, &t_count, acquire));

	pthread_attr_t *attr = &pqc->dpq_thread_attr;
	pthread_t tid, *pthr = &tid;
#if DISPATCH_ENABLE_PTHREAD_ROOT_QUEUES
	if (slowpath(dq == &_dispatch_mgr_root_queue)) {
		pthr = _dispatch_mgr_root_queue_init();
	}
#endif
	do {
		_dispatch_retain(dq);
		while ((r = pthread_create(pthr, attr, _dispatch_worker_thread, dq))) {
			if (r != EAGAIN) {
				(void)dispatch_assume_zero(r);
			}
			_dispatch_temporary_resource_shortage();
		}
	} while (--j);
#endif // DISPATCH_USE_PTHREAD_POOL
}

static inline void
_dispatch_queue_wakeup_global2(dispatch_queue_t dq, unsigned int n)
{
	if (!_dispatch_queue_class_probe(dq)) {
		return;
	}
#if HAVE_PTHREAD_WORKQUEUES
	dispatch_root_queue_context_t qc = dq->do_ctxt;
	if (
#if DISPATCH_USE_PTHREAD_POOL
			(qc->dgq_kworkqueue != (void*)(~0ul)) &&
#endif
			!dispatch_atomic_cmpxchg2o(qc, dgq_pending, 0, n, relaxed)) {
		_dispatch_root_queue_debug("worker thread request still pending for "
				"global queue: %p", dq);
		return;
	}
#endif // HAVE_PTHREAD_WORKQUEUES
	return 	_dispatch_queue_wakeup_global_slow(dq, n);
}

static inline void
_dispatch_queue_wakeup_global(dispatch_queue_t dq)
{
	return _dispatch_queue_wakeup_global2(dq, 1);
}

#pragma mark -
#pragma mark dispatch_queue_invoke

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
dispatch_queue_invoke2(dispatch_object_t dou,
		_dispatch_thread_semaphore_t *sema_ptr)
{
	dispatch_queue_t dq = dou._dq;
	dispatch_queue_t otq = dq->do_targetq;
	dispatch_queue_t cq = _dispatch_queue_get_current();

	if (slowpath(cq != otq)) {
		return otq;
	}

	*sema_ptr = _dispatch_queue_drain(dq);

	if (slowpath(otq != dq->do_targetq)) {
		// An item on the queue changed the target queue
		return dq->do_targetq;
	}
	return NULL;
}

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
DISPATCH_NOINLINE
void
_dispatch_queue_invoke(dispatch_queue_t dq)
{
	_dispatch_queue_class_invoke(dq, dispatch_queue_invoke2);
}

#pragma mark -
#pragma mark dispatch_queue_drain

DISPATCH_ALWAYS_INLINE
static inline struct dispatch_object_s*
_dispatch_queue_head(dispatch_queue_t dq)
{
	struct dispatch_object_s *dc;
	_dispatch_wait_until(dc = fastpath(dq->dq_items_head));
	return dc;
}

DISPATCH_ALWAYS_INLINE
static inline struct dispatch_object_s*
_dispatch_queue_next(dispatch_queue_t dq, struct dispatch_object_s *dc)
{
	struct dispatch_object_s *next_dc;
	next_dc = fastpath(dc->do_next);
	dq->dq_items_head = next_dc;
	if (!next_dc && !dispatch_atomic_cmpxchg2o(dq, dq_items_tail, dc, NULL,
			relaxed)) {
		_dispatch_wait_until(next_dc = fastpath(dc->do_next));
		dq->dq_items_head = next_dc;
	}
	return next_dc;
}

_dispatch_thread_semaphore_t
_dispatch_queue_drain(dispatch_object_t dou)
{
	dispatch_queue_t dq = dou._dq, orig_tq, old_dq;
	old_dq = _dispatch_thread_getspecific(dispatch_queue_key);
	struct dispatch_object_s *dc, *next_dc;
	_dispatch_thread_semaphore_t sema = 0;

	// Continue draining sources after target queue change rdar://8928171
	bool check_tq = (dx_type(dq) != DISPATCH_SOURCE_KEVENT_TYPE);

	orig_tq = dq->do_targetq;

	_dispatch_thread_setspecific(dispatch_queue_key, dq);
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(dq->dq_priority);

	pthread_priority_t op = _dispatch_queue_get_override_priority(dq);
	pthread_priority_t dp = _dispatch_get_defaultpriority();
	dp &= _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	if (op > dp) {
		_dispatch_wqthread_override_start(dq->dq_thread, op);
	}

	//dispatch_debug_queue(dq, __func__);

	while (dq->dq_items_tail) {
		dc = _dispatch_queue_head(dq);
		do {
			if (DISPATCH_OBJECT_SUSPENDED(dq)) {
				goto out;
			}
			if (dq->dq_running > dq->dq_width) {
				goto out;
			}
			if (slowpath(orig_tq != dq->do_targetq) && check_tq) {
				goto out;
			}
			bool redirect = false;
			if (!fastpath(dq->dq_width == 1)) {
				if (!DISPATCH_OBJ_IS_VTABLE(dc) &&
						(long)dc->do_vtable & DISPATCH_OBJ_BARRIER_BIT) {
					if (dq->dq_running > 1) {
						goto out;
					}
				} else {
					redirect = true;
				}
			}
			next_dc = _dispatch_queue_next(dq, dc);
			if (redirect) {
				_dispatch_continuation_redirect(dq, dc);
				continue;
			}
			if ((sema = _dispatch_barrier_sync_f_pop(dq, dc, true))) {
				goto out;
			}
			_dispatch_continuation_pop(dc);
			_dispatch_perfmon_workitem_inc();
		} while ((dc = next_dc));
	}

out:
	_dispatch_reset_defaultpriority(old_dp);
	_dispatch_thread_setspecific(dispatch_queue_key, old_dq);
	return sema;
}

#if DISPATCH_COCOA_COMPAT
static void
_dispatch_main_queue_drain(void)
{
	dispatch_queue_t dq = &_dispatch_main_q;
	if (!dq->dq_items_tail) {
		return;
	}
	struct dispatch_continuation_s marker = {
		.do_vtable = NULL,
	};
	struct dispatch_object_s *dmarker = (void*)&marker;
	_dispatch_queue_push_notrace(dq, dmarker, 0);

	_dispatch_perfmon_start();
	dispatch_queue_t old_dq = _dispatch_thread_getspecific(dispatch_queue_key);
	_dispatch_thread_setspecific(dispatch_queue_key, dq);
	pthread_priority_t old_pri = _dispatch_get_priority();
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(old_pri);
	voucher_t voucher = _voucher_copy();

	struct dispatch_object_s *dc, *next_dc;
	dc = _dispatch_queue_head(dq);
	do {
		next_dc = _dispatch_queue_next(dq, dc);
		if (dc == dmarker) {
			goto out;
		}
		_dispatch_continuation_pop(dc);
		_dispatch_perfmon_workitem_inc();
	} while ((dc = next_dc));
	DISPATCH_CRASH("Main queue corruption");

out:
	if (next_dc) {
		_dispatch_main_queue_wakeup();
	}
	_dispatch_voucher_debug("main queue restore", voucher);
	_dispatch_set_priority_and_replace_voucher(old_pri, voucher);
	_dispatch_queue_reset_override_priority(dq);
	_dispatch_reset_defaultpriority(old_dp);
	_dispatch_thread_setspecific(dispatch_queue_key, old_dq);
	_dispatch_perfmon_end();
	_dispatch_force_cache_cleanup();
}

static bool
_dispatch_runloop_queue_drain_one(dispatch_queue_t dq)
{
	if (!dq->dq_items_tail) {
		return false;
	}
	_dispatch_perfmon_start();
	dispatch_queue_t old_dq = _dispatch_thread_getspecific(dispatch_queue_key);
	_dispatch_thread_setspecific(dispatch_queue_key, dq);
	pthread_priority_t old_pri = _dispatch_get_priority();
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(old_pri);
	voucher_t voucher = _voucher_copy();

	struct dispatch_object_s *dc, *next_dc;
	dc = _dispatch_queue_head(dq);
	next_dc = _dispatch_queue_next(dq, dc);
	_dispatch_continuation_pop(dc);
	_dispatch_perfmon_workitem_inc();

	_dispatch_voucher_debug("runloop queue restore", voucher);
	_dispatch_set_priority_and_replace_voucher(old_pri, voucher);
	_dispatch_reset_defaultpriority(old_dp);
	_dispatch_thread_setspecific(dispatch_queue_key, old_dq);
	_dispatch_perfmon_end();
	_dispatch_force_cache_cleanup();
	return next_dc;
}
#endif

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline _dispatch_thread_semaphore_t
_dispatch_queue_drain_one_barrier_sync(dispatch_queue_t dq)
{
	// rdar://problem/8290662 "lock transfer"
	struct dispatch_object_s *dc;
	_dispatch_thread_semaphore_t sema;

	// queue is locked, or suspended and not being drained
	dc = dq->dq_items_head;
	if (slowpath(!dc) || !(sema = _dispatch_barrier_sync_f_pop(dq, dc, false))){
		return 0;
	}
	// dequeue dc, it is a barrier sync
	(void)_dispatch_queue_next(dq, dc);
	return sema;
}

void
_dispatch_mgr_queue_drain(void)
{
	dispatch_queue_t dq = &_dispatch_mgr_q;
	if (!dq->dq_items_tail) {
		return _dispatch_force_cache_cleanup();
	}
	_dispatch_perfmon_start();
	if (slowpath(_dispatch_queue_drain(dq))) {
		DISPATCH_CRASH("Sync onto manager queue");
	}
	_dispatch_voucher_debug("mgr queue clear", NULL);
	_voucher_clear();
	_dispatch_queue_reset_override_priority(dq);
	_dispatch_reset_defaultpriority_override();
	_dispatch_perfmon_end();
	_dispatch_force_cache_cleanup();
}

#pragma mark -
#pragma mark _dispatch_queue_wakeup_with_qos

DISPATCH_NOINLINE
static dispatch_queue_t
_dispatch_queue_wakeup_with_qos_slow(dispatch_queue_t dq, pthread_priority_t pp,
		bool retained)
{
	if (!dx_probe(dq) && (dq->dq_is_thread_bound || !dq->dq_thread)) {
		if (retained) _dispatch_release(dq);
		return NULL;
	}
	pp &= _PTHREAD_PRIORITY_QOS_CLASS_MASK;
	bool override = _dispatch_queue_override_priority(dq, pp);
	if (override && dq->dq_running > 1) {
		override = false;
	}

	if (!dispatch_atomic_cmpxchg2o(dq, do_suspend_cnt, 0,
			DISPATCH_OBJECT_SUSPEND_LOCK, acquire)) {
#if DISPATCH_COCOA_COMPAT
		if (dq == &_dispatch_main_q && dq->dq_is_thread_bound) {
			return _dispatch_main_queue_wakeup();
		}
#endif
		if (override) {
			mach_port_t th;
			// <rdar://problem/17735825> to traverse the tq chain safely we must
			// lock it to ensure it cannot change, unless the queue is running
			// and we can just override the thread itself
			if (dq->dq_thread) {
				_dispatch_wqthread_override_start(dq->dq_thread, pp);
			} else if (!dispatch_atomic_cmpxchgv2o(dq, dq_tqthread,
					MACH_PORT_NULL, _dispatch_thread_port(), &th, acquire)) {
				// already locked, override the owner, trysync will do a queue
				// wakeup when it returns.
				_dispatch_wqthread_override_start(th, pp);
			} else {
				dispatch_queue_t tq = dq->do_targetq;
				if (_dispatch_queue_prepare_override(dq, tq, pp)) {
					_dispatch_queue_push_override(dq, tq, pp);
				} else {
					_dispatch_queue_wakeup_with_qos(tq, pp);
				}
				dispatch_atomic_store2o(dq, dq_tqthread, MACH_PORT_NULL,
						release);
			}
		}
		if (retained) _dispatch_release(dq);
		return NULL;
	}
	dispatch_queue_t tq = dq->do_targetq;
	if (!retained) _dispatch_retain(dq);
	if (override) {
		override = _dispatch_queue_prepare_override(dq, tq, pp);
	}
	_dispatch_queue_push(tq, dq, pp);
	if (override) {
		_dispatch_queue_push_override(dq, tq, pp);
	}
	return tq;	// libdispatch does not need this, but the Instrument DTrace
				// probe does
}

DISPATCH_ALWAYS_INLINE
static inline dispatch_queue_t
_dispatch_queue_wakeup_with_qos2(dispatch_queue_t dq, pthread_priority_t pp,
		bool retained)
{
	if (_dispatch_object_suspended(dq)) {
		_dispatch_queue_override_priority(dq, pp);
		if (retained) _dispatch_release(dq);
		return NULL;
	}
	return _dispatch_queue_wakeup_with_qos_slow(dq, pp, retained);
}

DISPATCH_NOINLINE
void
_dispatch_queue_wakeup_with_qos_and_release(dispatch_queue_t dq,
		pthread_priority_t pp)
{
	(void)_dispatch_queue_wakeup_with_qos2(dq, pp, true);
}

DISPATCH_NOINLINE
void
_dispatch_queue_wakeup_with_qos(dispatch_queue_t dq, pthread_priority_t pp)
{
	(void)_dispatch_queue_wakeup_with_qos2(dq, pp, false);
}

DISPATCH_NOINLINE
dispatch_queue_t
_dispatch_queue_wakeup(dispatch_queue_t dq)
{
	return _dispatch_queue_wakeup_with_qos2(dq,
			_dispatch_queue_get_override_priority(dq), false);
}

#if HAVE_PTHREAD_WORKQUEUE_QOS
static void
_dispatch_queue_override_invoke(void *ctxt)
{
	dispatch_continuation_t dc = (dispatch_continuation_t)ctxt;
	dispatch_queue_t dq = dc->dc_data;
	pthread_priority_t p = 0;

	if (!slowpath(DISPATCH_OBJECT_SUSPENDED(dq)) &&
		fastpath(dispatch_atomic_cmpxchg2o(dq, dq_running, 0, 1, acquire))) {
		_dispatch_queue_set_thread(dq);

		_dispatch_object_debug(dq, "stolen onto thread 0x%x, 0x%lx",
				dq->dq_thread, _dispatch_get_defaultpriority());

		pthread_priority_t old_dp = _dispatch_get_defaultpriority();
		_dispatch_reset_defaultpriority(dc->dc_priority);

		dispatch_queue_t tq = NULL;
		_dispatch_thread_semaphore_t sema = 0;
		tq = dispatch_queue_invoke2(dq, &sema);

		_dispatch_queue_clear_thread(dq);
		_dispatch_reset_defaultpriority(old_dp);

		uint32_t running = dispatch_atomic_dec2o(dq, dq_running, release);
		if (sema) {
			_dispatch_thread_semaphore_signal(sema);
		} else if (!tq && running == 0) {
			p = _dispatch_queue_reset_override_priority(dq);
			if (p > (dq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK)) {
				_dispatch_wqthread_override_reset();
			}
		}
		_dispatch_introspection_queue_item_complete(dq);
		if (running == 0) {
			return _dispatch_queue_wakeup_with_qos_and_release(dq, p);
		}
	} else {
		mach_port_t th = dq->dq_thread;
		if (th) {
			p = _dispatch_queue_get_override_priority(dq);
			_dispatch_object_debug(dq, "overriding thr 0x%x to priority 0x%lx",
					th, p);
			_dispatch_wqthread_override_start(th, p);
		}
	}
	_dispatch_release(dq); // added when we pushed the override block
}
#endif

static inline bool
_dispatch_queue_prepare_override(dispatch_queue_t dq, dispatch_queue_t tq,
		pthread_priority_t p)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	if (dx_type(tq) != DISPATCH_QUEUE_ROOT_TYPE || !tq->dq_priority) {
		return false;
	}
	if (p <= (dq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK)) {
		return false;
	}
	if (p <= (tq->dq_priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK)) {
		return false;
	}
	_dispatch_retain(dq);
	return true;
#else
	(void)dq; (void)tq; (void)p;
	return false;
#endif
}

static inline void
_dispatch_queue_push_override(dispatch_queue_t dq, dispatch_queue_t tq,
		pthread_priority_t p)
{
#if HAVE_PTHREAD_WORKQUEUE_QOS
	unsigned int qosbit, idx, overcommit;
	overcommit = (tq->dq_priority & _PTHREAD_PRIORITY_OVERCOMMIT_FLAG) ? 1 : 0;
	qosbit = (p & _PTHREAD_PRIORITY_QOS_CLASS_MASK) >>
			_PTHREAD_PRIORITY_QOS_CLASS_SHIFT;
	idx = (unsigned int)__builtin_ffs((int)qosbit);
	if (!idx || idx > DISPATCH_QUEUE_QOS_COUNT) {
		DISPATCH_CRASH("Corrupted override priority");
	}
	dispatch_queue_t rq = &_dispatch_root_queues[((idx-1) << 1) | overcommit];

	dispatch_continuation_t dc = _dispatch_continuation_alloc();
	dc->do_vtable = (void *)(DISPATCH_OBJ_ASYNC_BIT | DISPATCH_OBJ_BARRIER_BIT);
	dc->dc_func = _dispatch_queue_override_invoke;
	dc->dc_ctxt = dc;
	dc->dc_priority = tq->dq_priority;
	dc->dc_voucher = NULL;
	dc->dc_data = dq;
	// dq retained by _dispatch_queue_prepare_override

	_dispatch_queue_push(rq, dc, 0);
#else
	(void)dq; (void)tq; (void)p;
#endif
}

#pragma mark -
#pragma mark dispatch_root_queue_drain

DISPATCH_NOINLINE
static bool
_dispatch_queue_concurrent_drain_one_slow(dispatch_queue_t dq)
{
	dispatch_root_queue_context_t qc = dq->do_ctxt;
	struct dispatch_object_s *const mediator = (void *)~0ul;
	bool pending = false, available = true;
	unsigned int sleep_time = DISPATCH_CONTENTION_USLEEP_START;

	do {
		// Spin for a short while in case the contention is temporary -- e.g.
		// when starting up after dispatch_apply, or when executing a few
		// short continuations in a row.
		if (_dispatch_contention_wait_until(dq->dq_items_head != mediator)) {
			goto out;
		}
		// Since we have serious contention, we need to back off.
		if (!pending) {
			// Mark this queue as pending to avoid requests for further threads
			(void)dispatch_atomic_inc2o(qc, dgq_pending, relaxed);
			pending = true;
		}
		_dispatch_contention_usleep(sleep_time);
		if (fastpath(dq->dq_items_head != mediator)) goto out;
		sleep_time *= 2;
	} while (sleep_time < DISPATCH_CONTENTION_USLEEP_MAX);

	// The ratio of work to libdispatch overhead must be bad. This
	// scenario implies that there are too many threads in the pool.
	// Create a new pending thread and then exit this thread.
	// The kernel will grant a new thread when the load subsides.
	_dispatch_debug("contention on global queue: %p", dq);
	available = false;
out:
	if (pending) {
		(void)dispatch_atomic_dec2o(qc, dgq_pending, relaxed);
	}
	if (!available) {
		_dispatch_queue_wakeup_global(dq);
	}
	return available;
}

DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_queue_concurrent_drain_one2(dispatch_queue_t dq)
{
	// Wait for queue head and tail to be both non-empty or both empty
	bool available; // <rdar://problem/15917893>
	_dispatch_wait_until((dq->dq_items_head != NULL) ==
			(available = (dq->dq_items_tail != NULL)));
	return available;
}

DISPATCH_ALWAYS_INLINE_NDEBUG
static inline struct dispatch_object_s *
_dispatch_queue_concurrent_drain_one(dispatch_queue_t dq)
{
	struct dispatch_object_s *head, *next, *const mediator = (void *)~0ul;

start:
	// The mediator value acts both as a "lock" and a signal
	head = dispatch_atomic_xchg2o(dq, dq_items_head, mediator, relaxed);

	if (slowpath(head == NULL)) {
		// The first xchg on the tail will tell the enqueueing thread that it
		// is safe to blindly write out to the head pointer. A cmpxchg honors
		// the algorithm.
		if (slowpath(!dispatch_atomic_cmpxchg2o(dq, dq_items_head, mediator,
				NULL, relaxed))) {
			goto start;
		}
		if (slowpath(dq->dq_items_tail) && // <rdar://problem/14416349>
				_dispatch_queue_concurrent_drain_one2(dq)) {
			goto start;
		}
		_dispatch_root_queue_debug("no work on global queue: %p", dq);
		return NULL;
	}

	if (slowpath(head == mediator)) {
		// This thread lost the race for ownership of the queue.
		if (fastpath(_dispatch_queue_concurrent_drain_one_slow(dq))) {
			goto start;
		}
		return NULL;
	}

	// Restore the head pointer to a sane value before returning.
	// If 'next' is NULL, then this item _might_ be the last item.
	next = fastpath(head->do_next);

	if (slowpath(!next)) {
		dispatch_atomic_store2o(dq, dq_items_head, NULL, relaxed);

		if (dispatch_atomic_cmpxchg2o(dq, dq_items_tail, head, NULL, relaxed)) {
			// both head and tail are NULL now
			goto out;
		}
		// There must be a next item now.
		_dispatch_wait_until(next = head->do_next);
	}

	dispatch_atomic_store2o(dq, dq_items_head, next, relaxed);
	_dispatch_queue_wakeup_global(dq);
out:
	return head;
}

static void
_dispatch_root_queue_drain(dispatch_queue_t dq)
{
#if DISPATCH_DEBUG
	if (_dispatch_thread_getspecific(dispatch_queue_key)) {
		DISPATCH_CRASH("Premature thread recycling");
	}
#endif
	_dispatch_thread_setspecific(dispatch_queue_key, dq);
	pthread_priority_t old_pri = _dispatch_get_priority();
	pthread_priority_t pri = dq->dq_priority ? dq->dq_priority : old_pri;
	pthread_priority_t old_dp = _dispatch_set_defaultpriority(pri);

#if DISPATCH_COCOA_COMPAT
	// ensure that high-level memory management techniques do not leak/crash
	if (dispatch_begin_thread_4GC) {
		dispatch_begin_thread_4GC();
	}
	void *pool = _dispatch_autorelease_pool_push();
#endif // DISPATCH_COCOA_COMPAT

	_dispatch_perfmon_start();
	struct dispatch_object_s *item;
	bool reset = false;
	while ((item = fastpath(_dispatch_queue_concurrent_drain_one(dq)))) {
		if (reset) _dispatch_wqthread_override_reset();
		_dispatch_continuation_pop(item);
		reset = _dispatch_reset_defaultpriority_override();
	}
	_dispatch_voucher_debug("root queue clear", NULL);
	_dispatch_set_priority_and_replace_voucher(old_pri, NULL);
	_dispatch_reset_defaultpriority(old_dp);
	_dispatch_perfmon_end();

#if DISPATCH_COCOA_COMPAT
	_dispatch_autorelease_pool_pop(pool);
	if (dispatch_end_thread_4GC) {
		dispatch_end_thread_4GC();
	}
#endif // DISPATCH_COCOA_COMPAT

	_dispatch_thread_setspecific(dispatch_queue_key, NULL);
}

#pragma mark -
#pragma mark dispatch_worker_thread

#if HAVE_PTHREAD_WORKQUEUES
static void
_dispatch_worker_thread4(void *context)
{
	dispatch_queue_t dq = context;
	dispatch_root_queue_context_t qc = dq->do_ctxt;

	_dispatch_introspection_thread_add();
	int pending = (int)dispatch_atomic_dec2o(qc, dgq_pending, relaxed);
	dispatch_assert(pending >= 0);
	_dispatch_root_queue_drain(dq);
	__asm__(""); // prevent tailcall (for Instrument DTrace probe)
}

#if HAVE_PTHREAD_WORKQUEUE_QOS
static void
_dispatch_worker_thread3(pthread_priority_t priority)
{
	// Reset priority TSD to workaround <rdar://problem/17825261>
	_dispatch_thread_setspecific(dispatch_priority_key,
			(void*)(uintptr_t)(priority & ~_PTHREAD_PRIORITY_FLAGS_MASK));
	unsigned int overcommit, qosbit, idx;
	overcommit = (priority & _PTHREAD_PRIORITY_OVERCOMMIT_FLAG) ? 1 : 0;
	qosbit = (priority & _PTHREAD_PRIORITY_QOS_CLASS_MASK) >>
			_PTHREAD_PRIORITY_QOS_CLASS_SHIFT;
	if (!_dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_MAINTENANCE_QOS].
			dq_priority) {
		// If kernel doesn't support maintenance, bottom bit is background.
		// Shift to our idea of where background bit is.
		qosbit <<= 1;
	}
	idx = (unsigned int)__builtin_ffs((int)qosbit);
	dispatch_assert(idx > 0 && idx < DISPATCH_QUEUE_QOS_COUNT+1);
	dispatch_queue_t dq = &_dispatch_root_queues[((idx-1) << 1) | overcommit];
	return _dispatch_worker_thread4(dq);
}
#endif // HAVE_PTHREAD_WORKQUEUE_QOS

#if HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
static void
_dispatch_worker_thread2(int priority, int options,
		void *context DISPATCH_UNUSED)
{
	dispatch_assert(priority >= 0 && priority < WORKQ_NUM_PRIOQUEUE);
	dispatch_assert(!(options & ~WORKQ_ADDTHREADS_OPTION_OVERCOMMIT));
	dispatch_queue_t dq = _dispatch_wq2root_queues[priority][options];

	return _dispatch_worker_thread4(dq);
}
#endif // HAVE_PTHREAD_WORKQUEUE_SETDISPATCH_NP
#endif // HAVE_PTHREAD_WORKQUEUES

#if DISPATCH_USE_PTHREAD_POOL
// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
static void *
_dispatch_worker_thread(void *context)
{
	dispatch_queue_t dq = context;
	dispatch_root_queue_context_t qc = dq->do_ctxt;
	dispatch_pthread_root_queue_context_t pqc = qc->dgq_ctxt;

	if (pqc->dpq_thread_configure) {
		pqc->dpq_thread_configure();
	}

	sigset_t mask;
	int r;
	// workaround tweaks the kernel workqueue does for us
	r = sigfillset(&mask);
	(void)dispatch_assume_zero(r);
	r = _dispatch_pthread_sigmask(SIG_BLOCK, &mask, NULL);
	(void)dispatch_assume_zero(r);
	_dispatch_introspection_thread_add();

	const int64_t timeout = 5ull * NSEC_PER_SEC;
	do {
		_dispatch_root_queue_drain(dq);
	} while (dispatch_semaphore_wait(&pqc->dpq_thread_mediator,
			dispatch_time(0, timeout)) == 0);

	(void)dispatch_atomic_inc2o(qc, dgq_thread_pool_size, release);
	_dispatch_queue_wakeup_global(dq);
	_dispatch_release(dq);

	return NULL;
}

int
_dispatch_pthread_sigmask(int how, sigset_t *set, sigset_t *oset)
{
	int r;

	/* Workaround: 6269619 Not all signals can be delivered on any thread */

	r = sigdelset(set, SIGILL);
	(void)dispatch_assume_zero(r);
	r = sigdelset(set, SIGTRAP);
	(void)dispatch_assume_zero(r);
#if HAVE_DECL_SIGEMT
	r = sigdelset(set, SIGEMT);
	(void)dispatch_assume_zero(r);
#endif
	r = sigdelset(set, SIGFPE);
	(void)dispatch_assume_zero(r);
	r = sigdelset(set, SIGBUS);
	(void)dispatch_assume_zero(r);
	r = sigdelset(set, SIGSEGV);
	(void)dispatch_assume_zero(r);
	r = sigdelset(set, SIGSYS);
	(void)dispatch_assume_zero(r);
	r = sigdelset(set, SIGPIPE);
	(void)dispatch_assume_zero(r);

	return pthread_sigmask(how, set, oset);
}
#endif // DISPATCH_USE_PTHREAD_POOL

#pragma mark -
#pragma mark dispatch_runloop_queue

static bool _dispatch_program_is_probably_callback_driven;

#if DISPATCH_COCOA_COMPAT

dispatch_queue_t
_dispatch_runloop_root_queue_create_4CF(const char *label, unsigned long flags)
{
	dispatch_queue_t dq;
	size_t dqs;

	if (slowpath(flags)) {
		return NULL;
	}
	dqs = sizeof(struct dispatch_queue_s) - DISPATCH_QUEUE_CACHELINE_PAD;
	dq = _dispatch_alloc(DISPATCH_VTABLE(queue_runloop), dqs);
	_dispatch_queue_init(dq);
	dq->do_targetq = _dispatch_get_root_queue(_DISPATCH_QOS_CLASS_DEFAULT,true);
	dq->dq_label = label ? label : "runloop-queue"; // no-copy contract
	dq->do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK;
	dq->dq_running = 1;
	dq->dq_is_thread_bound = 1;
	_dispatch_runloop_queue_port_init(dq);
	_dispatch_queue_set_bound_thread(dq);
	_dispatch_object_debug(dq, "%s", __func__);
	return _dispatch_introspection_queue_create(dq);
}

void
_dispatch_runloop_queue_xref_dispose(dispatch_queue_t dq)
{
	_dispatch_object_debug(dq, "%s", __func__);
	(void)dispatch_atomic_dec2o(dq, dq_running, relaxed);
	unsigned int suspend_cnt = dispatch_atomic_sub2o(dq, do_suspend_cnt,
			DISPATCH_OBJECT_SUSPEND_LOCK, release);
	_dispatch_queue_clear_bound_thread(dq);
	if (suspend_cnt == 0) {
		_dispatch_queue_wakeup(dq);
	}
}

void
_dispatch_runloop_queue_dispose(dispatch_queue_t dq)
{
	_dispatch_object_debug(dq, "%s", __func__);
	_dispatch_introspection_queue_dispose(dq);
	_dispatch_runloop_queue_port_dispose(dq);
	_dispatch_queue_destroy(dq);
}

bool
_dispatch_runloop_root_queue_perform_4CF(dispatch_queue_t dq)
{
	if (slowpath(dq->do_vtable != DISPATCH_VTABLE(queue_runloop))) {
		DISPATCH_CLIENT_CRASH("Not a runloop queue");
	}
	dispatch_retain(dq);
	bool r = _dispatch_runloop_queue_drain_one(dq);
	dispatch_release(dq);
	return r;
}

void
_dispatch_runloop_root_queue_wakeup_4CF(dispatch_queue_t dq)
{
	if (slowpath(dq->do_vtable != DISPATCH_VTABLE(queue_runloop))) {
		DISPATCH_CLIENT_CRASH("Not a runloop queue");
	}
	_dispatch_runloop_queue_probe(dq);
}

mach_port_t
_dispatch_runloop_root_queue_get_port_4CF(dispatch_queue_t dq)
{
	if (slowpath(dq->do_vtable != DISPATCH_VTABLE(queue_runloop))) {
		DISPATCH_CLIENT_CRASH("Not a runloop queue");
	}
	return (mach_port_t)dq->do_ctxt;
}

static void
_dispatch_runloop_queue_port_init(void *ctxt)
{
	dispatch_queue_t dq = (dispatch_queue_t)ctxt;
	mach_port_t mp;
	kern_return_t kr;

	_dispatch_safe_fork = false;
	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &mp);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
	kr = mach_port_insert_right(mach_task_self(), mp, mp,
			MACH_MSG_TYPE_MAKE_SEND);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
	if (dq != &_dispatch_main_q) {
		struct mach_port_limits limits = {
			.mpl_qlimit = 1,
		};
		kr = mach_port_set_attributes(mach_task_self(), mp,
				MACH_PORT_LIMITS_INFO, (mach_port_info_t)&limits,
				sizeof(limits));
		DISPATCH_VERIFY_MIG(kr);
		(void)dispatch_assume_zero(kr);
	}
	dq->do_ctxt = (void*)(uintptr_t)mp;

	_dispatch_program_is_probably_callback_driven = true;
}

static void
_dispatch_runloop_queue_port_dispose(dispatch_queue_t dq)
{
	mach_port_t mp = (mach_port_t)dq->do_ctxt;
	if (!mp) {
		return;
	}
	dq->do_ctxt = NULL;
	kern_return_t kr = mach_port_deallocate(mach_task_self(), mp);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
#ifndef __FreeBSD__
	/* XXX: https://bugs.freenas.org/issues/10145 */
	kr = mach_port_mod_refs(mach_task_self(), mp, MACH_PORT_RIGHT_RECEIVE, -1);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
#endif
}

#pragma mark -
#pragma mark dispatch_main_queue

mach_port_t
_dispatch_get_main_queue_port_4CF(void)
{
	dispatch_queue_t dq = &_dispatch_main_q;
	dispatch_once_f(&_dispatch_main_q_port_pred, dq,
			_dispatch_runloop_queue_port_init);
	return (mach_port_t)dq->do_ctxt;
}

static bool main_q_is_draining;

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
DISPATCH_NOINLINE
static void
_dispatch_queue_set_mainq_drain_state(bool arg)
{
	main_q_is_draining = arg;
}

void
_dispatch_main_queue_callback_4CF(mach_msg_header_t *msg DISPATCH_UNUSED)
{
	if (main_q_is_draining) {
		return;
	}
	_dispatch_queue_set_mainq_drain_state(true);
	_dispatch_main_queue_drain();
	_dispatch_queue_set_mainq_drain_state(false);
}

#endif

void
dispatch_main(void)
{
#if HAVE_PTHREAD_MAIN_NP
	if (pthread_main_np()) {
#endif
		_dispatch_object_debug(&_dispatch_main_q, "%s", __func__);
		_dispatch_program_is_probably_callback_driven = true;
		pthread_exit(NULL);
		DISPATCH_CRASH("pthread_exit() returned");
#if HAVE_PTHREAD_MAIN_NP
	}
	DISPATCH_CLIENT_CRASH("dispatch_main() must be called on the main thread");
#endif
}

DISPATCH_NOINLINE DISPATCH_NORETURN
static void
_dispatch_sigsuspend(void)
{
	static const sigset_t mask;

	for (;;) {
		sigsuspend(&mask);
	}
}

DISPATCH_NORETURN
static void
_dispatch_sig_thread(void *ctxt DISPATCH_UNUSED)
{
	// never returns, so burn bridges behind us
	_dispatch_clear_stack(0);
	_dispatch_sigsuspend();
}

DISPATCH_NOINLINE
static void
_dispatch_queue_cleanup2(void)
{
	dispatch_queue_t dq = &_dispatch_main_q;
	(void)dispatch_atomic_dec2o(dq, dq_running, relaxed);
	unsigned int suspend_cnt = dispatch_atomic_sub2o(dq, do_suspend_cnt,
			DISPATCH_OBJECT_SUSPEND_LOCK, release);
	dq->dq_is_thread_bound = 0;
	if (suspend_cnt == 0) {
		_dispatch_queue_wakeup(dq);
	}

	// overload the "probably" variable to mean that dispatch_main() or
	// similar non-POSIX API was called
	// this has to run before the DISPATCH_COCOA_COMPAT below
	if (_dispatch_program_is_probably_callback_driven) {
		_dispatch_barrier_async_detached_f(_dispatch_get_root_queue(
				_DISPATCH_QOS_CLASS_DEFAULT, true), NULL, _dispatch_sig_thread);
		sleep(1); // workaround 6778970
	}

#if DISPATCH_COCOA_COMPAT
	dispatch_once_f(&_dispatch_main_q_port_pred, dq,
			_dispatch_runloop_queue_port_init);
	_dispatch_runloop_queue_port_dispose(dq);
#endif
}

static void
_dispatch_queue_cleanup(void *ctxt)
{
	if (ctxt == &_dispatch_main_q) {
		return _dispatch_queue_cleanup2();
	}
	// POSIX defines that destructors are only called if 'ctxt' is non-null
	DISPATCH_CRASH("Premature thread exit while a dispatch queue is running");
}
