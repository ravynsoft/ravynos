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

// Contains exported global data and initialization & other routines that must
// only exist once in the shared library even when resolvers are used.

// NOTE: this file must not contain any atomic operations

#include "internal.h"

#if HAVE_MACH
#include "protocolServer.h"
#endif

#pragma mark -
#pragma mark dispatch_init

#if USE_LIBDISPATCH_INIT_CONSTRUCTOR
DISPATCH_NOTHROW __attribute__((constructor))
void
_libdispatch_init(void);

DISPATCH_NOTHROW
void
_libdispatch_init(void)
{
	libdispatch_init();
}
#endif

#if !defined(_WIN32)
DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_atfork_prepare(void)
{
	_os_object_atfork_prepare();
}

DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_atfork_parent(void)
{
	_os_object_atfork_parent();
}

DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_atfork_child(void)
{
	_os_object_atfork_child();
	_voucher_atfork_child();
	_dispatch_event_loop_atfork_child();
	if (_dispatch_is_multithreaded_inline()) {
		_dispatch_child_of_unsafe_fork = true;
	}
	_dispatch_queue_atfork_child();
	// clear the _PROHIBIT and _MULTITHREADED bits if set
	_dispatch_unsafe_fork = 0;
}

int
_dispatch_sigmask(void)
{
	sigset_t mask;
	int r = 0;

	/* Workaround: 6269619 Not all signals can be delivered on any thread */
	r |= sigfillset(&mask);
	r |= sigdelset(&mask, SIGILL);
	r |= sigdelset(&mask, SIGTRAP);
#if HAVE_DECL_SIGEMT
	r |= sigdelset(&mask, SIGEMT);
#endif
	r |= sigdelset(&mask, SIGFPE);
	r |= sigdelset(&mask, SIGBUS);
	r |= sigdelset(&mask, SIGSEGV);
	r |= sigdelset(&mask, SIGSYS);
	r |= sigdelset(&mask, SIGPIPE);
	r |= sigdelset(&mask, SIGPROF);
	r |= pthread_sigmask(SIG_BLOCK, &mask, NULL);
	return dispatch_assume_zero(r);
}
#endif

#pragma mark -
#pragma mark dispatch_globals

DISPATCH_HIDE_SYMBOL(dispatch_assert_queue, 10.12, 10.0, 10.0, 3.0);
DISPATCH_HIDE_SYMBOL(dispatch_assert_queue_not, 10.12, 10.0, 10.0, 3.0);
DISPATCH_HIDE_SYMBOL(dispatch_queue_create_with_target, 10.12, 10.0, 10.0, 3.0);

#if DISPATCH_COCOA_COMPAT
void *(*_dispatch_begin_NSAutoReleasePool)(void);
void (*_dispatch_end_NSAutoReleasePool)(void *);
#endif

#if DISPATCH_USE_THREAD_LOCAL_STORAGE
_Thread_local struct dispatch_tsd __dispatch_tsd;
#if defined(_WIN32)
DWORD __dispatch_tsd_key;
#else
pthread_key_t __dispatch_tsd_key;
#endif
#elif !DISPATCH_USE_DIRECT_TSD
pthread_key_t dispatch_queue_key;
pthread_key_t dispatch_frame_key;
pthread_key_t dispatch_cache_key;
pthread_key_t dispatch_context_key;
pthread_key_t dispatch_pthread_root_queue_observer_hooks_key;
pthread_key_t dispatch_basepri_key;
#if DISPATCH_INTROSPECTION
pthread_key_t dispatch_introspection_key;
#elif DISPATCH_PERF_MON
pthread_key_t dispatch_bcounter_key;
#endif
pthread_key_t dispatch_wlh_key;
pthread_key_t dispatch_voucher_key;
pthread_key_t dispatch_deferred_items_key;
#endif // !DISPATCH_USE_DIRECT_TSD && !DISPATCH_USE_THREAD_LOCAL_STORAGE

#if VOUCHER_USE_MACH_VOUCHER
dispatch_once_t _voucher_task_mach_voucher_pred;
mach_voucher_t _voucher_task_mach_voucher;
#if !VOUCHER_USE_EMPTY_MACH_BASE_VOUCHER
mach_voucher_t _voucher_default_task_mach_voucher;
#endif
dispatch_once_t _firehose_task_buffer_pred;
firehose_buffer_t _firehose_task_buffer;
const uint32_t _firehose_spi_version = OS_FIREHOSE_SPI_VERSION;
uint64_t _voucher_unique_pid;
voucher_activity_hooks_t _voucher_libtrace_hooks;
dispatch_mach_t _voucher_activity_debug_channel;
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS && DISPATCH_DEBUG
bool _dispatch_set_qos_class_enabled;
#endif
#if DISPATCH_USE_KEVENT_WORKQUEUE && DISPATCH_USE_MGR_THREAD
bool _dispatch_kevent_workqueue_enabled = 1;
#endif

DISPATCH_HW_CONFIG();
uint8_t _dispatch_unsafe_fork;
uint8_t _dispatch_mode;
bool _dispatch_child_of_unsafe_fork;
#if DISPATCH_USE_MEMORYPRESSURE_SOURCE
bool _dispatch_memory_warn;
int _dispatch_continuation_cache_limit = DISPATCH_CONTINUATION_CACHE_LIMIT;
#endif

DISPATCH_NOINLINE
bool
_dispatch_is_multithreaded(void)
{
	return _dispatch_is_multithreaded_inline();
}

DISPATCH_NOINLINE
bool
_dispatch_is_fork_of_multithreaded_parent(void)
{
	return _dispatch_child_of_unsafe_fork;
}

const struct dispatch_queue_offsets_s dispatch_queue_offsets = {
	.dqo_version = 6,
	.dqo_label = offsetof(struct dispatch_queue_s, dq_label),
	.dqo_label_size = sizeof(((dispatch_queue_t)NULL)->dq_label),
	.dqo_flags = 0,
	.dqo_flags_size = 0,
	.dqo_serialnum = offsetof(struct dispatch_queue_s, dq_serialnum),
	.dqo_serialnum_size = sizeof(((dispatch_queue_t)NULL)->dq_serialnum),
	.dqo_width = offsetof(struct dispatch_queue_s, dq_width),
	.dqo_width_size = sizeof(((dispatch_queue_t)NULL)->dq_width),
	.dqo_running = 0,
	.dqo_running_size = 0,
	.dqo_suspend_cnt = 0,
	.dqo_suspend_cnt_size = 0,
	.dqo_target_queue = offsetof(struct dispatch_queue_s, do_targetq),
	.dqo_target_queue_size = sizeof(((dispatch_queue_t)NULL)->do_targetq),
	.dqo_priority = 0,
	.dqo_priority_size = 0,
};

#if TARGET_OS_MAC
const struct dispatch_allocator_layout_s dispatch_allocator_layout = {
	.dal_version = 1,
#if DISPATCH_ALLOCATOR
	.dal_allocator_zone = &_dispatch_main_heap,
	.dal_deferred_free_isa = &_dispatch_main_heap,
	.dal_allocation_size = DISPATCH_CONTINUATION_SIZE,
	.dal_magazine_size = BYTES_PER_MAGAZINE,
#if PACK_FIRST_PAGE_WITH_CONTINUATIONS
	.dal_first_allocation_offset =
			offsetof(struct dispatch_magazine_s, fp_conts),
#else
	.dal_first_allocation_offset =
			offsetof(struct dispatch_magazine_s, conts),
#endif
	.dal_allocation_isa_offset =
			offsetof(struct dispatch_continuation_s, dc_flags),
	.dal_enumerator = &_dispatch_allocator_enumerate,
#endif // DISPATCH_ALLOCATOR
};
#endif

#if DISPATCH_USE_DIRECT_TSD
const struct dispatch_tsd_indexes_s dispatch_tsd_indexes = {
	.dti_version = 3,
	.dti_queue_index = dispatch_queue_key,
	.dti_voucher_index = dispatch_voucher_key,
	.dti_qos_class_index = dispatch_priority_key,
	.dti_continuation_cache_index = dispatch_cache_key,
};
#endif // DISPATCH_USE_DIRECT_TSD

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
struct dispatch_queue_static_s _dispatch_main_q = {
	DISPATCH_GLOBAL_OBJECT_HEADER(queue_main),
#if !DISPATCH_USE_RESOLVERS
	.do_targetq = _dispatch_get_default_queue(true),
#endif
	.dq_state = DISPATCH_QUEUE_STATE_INIT_VALUE(1) |
			DISPATCH_QUEUE_ROLE_BASE_ANON,
	.dq_label = "com.apple.main-thread",
	.dq_atomic_flags = DQF_THREAD_BOUND | DQF_WIDTH(1),
	.dq_serialnum = 1,
};

#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
static struct dispatch_pthread_root_queue_context_s
_dispatch_mgr_root_queue_pthread_context;

struct dispatch_queue_global_s _dispatch_mgr_root_queue = {
	DISPATCH_GLOBAL_OBJECT_HEADER(queue_global),
	.dq_state = DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE,
	.do_ctxt = &_dispatch_mgr_root_queue_pthread_context,
	.dq_label = "com.apple.root.libdispatch-manager",
	.dq_atomic_flags = DQF_WIDTH(DISPATCH_QUEUE_WIDTH_POOL),
	.dq_priority = DISPATCH_PRIORITY_FLAG_MANAGER |
			DISPATCH_PRIORITY_SATURATED_OVERRIDE,
	.dq_serialnum = 3,
	.dgq_thread_pool_size = 1,
};
#else
#define _dispatch_mgr_root_queue _dispatch_root_queues[\
		DISPATCH_ROOT_QUEUE_IDX_USER_INTERACTIVE_QOS_OVERCOMMIT]
#endif

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
struct dispatch_queue_static_s _dispatch_mgr_q = {
	DISPATCH_GLOBAL_OBJECT_HEADER(queue_mgr),
	.dq_state = DISPATCH_QUEUE_STATE_INIT_VALUE(1) |
			DISPATCH_QUEUE_ROLE_BASE_ANON,
	.do_ctxt = (void *)-1,
	.do_targetq = _dispatch_mgr_root_queue._as_dq,
	.dq_label = "com.apple.libdispatch-manager",
	.dq_atomic_flags = DQF_WIDTH(1),
	.dq_priority = DISPATCH_PRIORITY_FLAG_MANAGER |
			DISPATCH_PRIORITY_SATURATED_OVERRIDE,
	.dq_serialnum = 2,
};

#if DISPATCH_USE_INTERNAL_WORKQUEUE
static struct dispatch_pthread_root_queue_context_s
		_dispatch_pthread_root_queue_contexts[DISPATCH_ROOT_QUEUE_COUNT];
#define _dispatch_root_queue_ctxt(n) &_dispatch_pthread_root_queue_contexts[n]
#else
#define _dispatch_root_queue_ctxt(n) NULL
#endif // DISPATCH_USE_INTERNAL_WORKQUEUE

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
struct dispatch_queue_global_s _dispatch_root_queues[] = {
#define _DISPATCH_ROOT_QUEUE_IDX(n, flags) \
		((flags & DISPATCH_PRIORITY_FLAG_OVERCOMMIT) ? \
		DISPATCH_ROOT_QUEUE_IDX_##n##_QOS_OVERCOMMIT : \
		DISPATCH_ROOT_QUEUE_IDX_##n##_QOS)
#define _DISPATCH_ROOT_QUEUE_ENTRY(n, flags, ...) \
	[_DISPATCH_ROOT_QUEUE_IDX(n, flags)] = { \
		DISPATCH_GLOBAL_OBJECT_HEADER(queue_global), \
		.dq_state = DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE, \
		.do_ctxt = _dispatch_root_queue_ctxt(_DISPATCH_ROOT_QUEUE_IDX(n, flags)), \
		.dq_atomic_flags = DQF_WIDTH(DISPATCH_QUEUE_WIDTH_POOL), \
		.dq_priority = flags | ((flags & DISPATCH_PRIORITY_FLAG_FALLBACK) ? \
				_dispatch_priority_make_fallback(DISPATCH_QOS_##n) : \
				_dispatch_priority_make(DISPATCH_QOS_##n, 0)), \
		__VA_ARGS__ \
	}
	_DISPATCH_ROOT_QUEUE_ENTRY(MAINTENANCE, 0,
		.dq_label = "com.apple.root.maintenance-qos",
		.dq_serialnum = 4,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(MAINTENANCE, DISPATCH_PRIORITY_FLAG_OVERCOMMIT,
		.dq_label = "com.apple.root.maintenance-qos.overcommit",
		.dq_serialnum = 5,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(BACKGROUND, 0,
		.dq_label = "com.apple.root.background-qos",
		.dq_serialnum = 6,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(BACKGROUND, DISPATCH_PRIORITY_FLAG_OVERCOMMIT,
		.dq_label = "com.apple.root.background-qos.overcommit",
		.dq_serialnum = 7,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(UTILITY, 0,
		.dq_label = "com.apple.root.utility-qos",
		.dq_serialnum = 8,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(UTILITY, DISPATCH_PRIORITY_FLAG_OVERCOMMIT,
		.dq_label = "com.apple.root.utility-qos.overcommit",
		.dq_serialnum = 9,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(DEFAULT, DISPATCH_PRIORITY_FLAG_FALLBACK,
		.dq_label = "com.apple.root.default-qos",
		.dq_serialnum = 10,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(DEFAULT,
			DISPATCH_PRIORITY_FLAG_FALLBACK | DISPATCH_PRIORITY_FLAG_OVERCOMMIT,
		.dq_label = "com.apple.root.default-qos.overcommit",
		.dq_serialnum = 11,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(USER_INITIATED, 0,
		.dq_label = "com.apple.root.user-initiated-qos",
		.dq_serialnum = 12,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(USER_INITIATED, DISPATCH_PRIORITY_FLAG_OVERCOMMIT,
		.dq_label = "com.apple.root.user-initiated-qos.overcommit",
		.dq_serialnum = 13,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(USER_INTERACTIVE, 0,
		.dq_label = "com.apple.root.user-interactive-qos",
		.dq_serialnum = 14,
	),
	_DISPATCH_ROOT_QUEUE_ENTRY(USER_INTERACTIVE, DISPATCH_PRIORITY_FLAG_OVERCOMMIT,
		.dq_label = "com.apple.root.user-interactive-qos.overcommit",
		.dq_serialnum = 15,
	),
};

unsigned long volatile _dispatch_queue_serial_numbers =
		DISPATCH_QUEUE_SERIAL_NUMBER_INIT;


dispatch_queue_global_t
dispatch_get_global_queue(long priority, unsigned long flags)
{
	dispatch_assert(countof(_dispatch_root_queues) ==
			DISPATCH_ROOT_QUEUE_COUNT);

	if (flags & ~(unsigned long)DISPATCH_QUEUE_OVERCOMMIT) {
		return DISPATCH_BAD_INPUT;
	}
	dispatch_qos_t qos = _dispatch_qos_from_queue_priority(priority);
#if !HAVE_PTHREAD_WORKQUEUE_QOS
	if (qos == QOS_CLASS_MAINTENANCE) {
		qos = DISPATCH_QOS_BACKGROUND;
	} else if (qos == QOS_CLASS_USER_INTERACTIVE) {
		qos = DISPATCH_QOS_USER_INITIATED;
	}
#endif
	if (qos == DISPATCH_QOS_UNSPECIFIED) {
		return DISPATCH_BAD_INPUT;
	}
	return _dispatch_get_root_queue(qos, flags & DISPATCH_QUEUE_OVERCOMMIT);
}

dispatch_queue_t
dispatch_get_current_queue(void)
{
	return _dispatch_queue_get_current_or_default();
}

#pragma mark -
#pragma mark dispatch_queue_attr_t

// DISPATCH_QUEUE_CONCURRENT resp. _dispatch_queue_attr_concurrent is aliased
// to array member [0] and their properties must match!
const struct dispatch_queue_attr_s _dispatch_queue_attrs[] = {
	[0 ... DISPATCH_QUEUE_ATTR_COUNT - 1] = {
		DISPATCH_GLOBAL_OBJECT_HEADER(queue_attr),
	},
};

#if DISPATCH_VARIANT_STATIC
// <rdar://problem/16778703>
struct dispatch_queue_attr_s _dispatch_queue_attr_concurrent = {
	DISPATCH_GLOBAL_OBJECT_HEADER(queue_attr),
};
#endif // DISPATCH_VARIANT_STATIC

// _dispatch_queue_attr_concurrent is aliased using libdispatch.aliases
// and the -alias_list linker option on Darwin but needs to be done manually
// for other platforms.
#ifndef __APPLE__
extern struct dispatch_queue_attr_s _dispatch_queue_attr_concurrent
	__attribute__((__alias__("_dispatch_queue_attrs")));
#endif

dispatch_queue_attr_info_t
_dispatch_queue_attr_to_info(dispatch_queue_attr_t dqa)
{
	dispatch_queue_attr_info_t dqai = { };

	if (!dqa) return dqai;

#if DISPATCH_VARIANT_STATIC
	if (dqa == &_dispatch_queue_attr_concurrent) {
		dqai.dqai_concurrent = true;
		return dqai;
	}
#endif

	if (dqa < _dispatch_queue_attrs ||
			dqa >= &_dispatch_queue_attrs[DISPATCH_QUEUE_ATTR_COUNT]) {
		DISPATCH_CLIENT_CRASH(dqa->do_vtable, "Invalid queue attribute");
	}

	size_t idx = (size_t)(dqa - _dispatch_queue_attrs);

	dqai.dqai_inactive = (idx % DISPATCH_QUEUE_ATTR_INACTIVE_COUNT);
	idx /= DISPATCH_QUEUE_ATTR_INACTIVE_COUNT;

	dqai.dqai_concurrent = !(idx % DISPATCH_QUEUE_ATTR_CONCURRENCY_COUNT);
	idx /= DISPATCH_QUEUE_ATTR_CONCURRENCY_COUNT;

	dqai.dqai_relpri = -(int)(idx % DISPATCH_QUEUE_ATTR_PRIO_COUNT);
	idx /= DISPATCH_QUEUE_ATTR_PRIO_COUNT;

	dqai.dqai_qos = idx % DISPATCH_QUEUE_ATTR_QOS_COUNT;
	idx /= DISPATCH_QUEUE_ATTR_QOS_COUNT;

	dqai.dqai_autorelease_frequency =
			idx % DISPATCH_QUEUE_ATTR_AUTORELEASE_FREQUENCY_COUNT;
	idx /= DISPATCH_QUEUE_ATTR_AUTORELEASE_FREQUENCY_COUNT;

	dqai.dqai_overcommit = idx % DISPATCH_QUEUE_ATTR_OVERCOMMIT_COUNT;
	idx /= DISPATCH_QUEUE_ATTR_OVERCOMMIT_COUNT;

	return dqai;
}

static dispatch_queue_attr_t
_dispatch_queue_attr_from_info(dispatch_queue_attr_info_t dqai)
{
	size_t idx = 0;

	idx *= DISPATCH_QUEUE_ATTR_OVERCOMMIT_COUNT;
	idx += dqai.dqai_overcommit;

	idx *= DISPATCH_QUEUE_ATTR_AUTORELEASE_FREQUENCY_COUNT;
	idx += dqai.dqai_autorelease_frequency;

	idx *= DISPATCH_QUEUE_ATTR_QOS_COUNT;
	idx += dqai.dqai_qos;

	idx *= DISPATCH_QUEUE_ATTR_PRIO_COUNT;
	idx += (size_t)(-dqai.dqai_relpri);

	idx *= DISPATCH_QUEUE_ATTR_CONCURRENCY_COUNT;
	idx += !dqai.dqai_concurrent;

	idx *= DISPATCH_QUEUE_ATTR_INACTIVE_COUNT;
	idx += dqai.dqai_inactive;

	return (dispatch_queue_attr_t)&_dispatch_queue_attrs[idx];
}

dispatch_queue_attr_t
dispatch_queue_attr_make_with_qos_class(dispatch_queue_attr_t dqa,
		dispatch_qos_class_t qos_class, int relpri)
{
	if (!_dispatch_qos_class_valid(qos_class, relpri)) {
		return (dispatch_queue_attr_t)dqa;
	}
	dispatch_queue_attr_info_t dqai = _dispatch_queue_attr_to_info(dqa);
	dqai.dqai_qos = _dispatch_qos_from_qos_class(qos_class);
	dqai.dqai_relpri = relpri;
	return _dispatch_queue_attr_from_info(dqai);
}

dispatch_queue_attr_t
dispatch_queue_attr_make_initially_inactive(dispatch_queue_attr_t dqa)
{
	dispatch_queue_attr_info_t dqai = _dispatch_queue_attr_to_info(dqa);
	dqai.dqai_inactive = true;
	return _dispatch_queue_attr_from_info(dqai);
}

dispatch_queue_attr_t
dispatch_queue_attr_make_with_overcommit(dispatch_queue_attr_t dqa,
		bool overcommit)
{
	dispatch_queue_attr_info_t dqai = _dispatch_queue_attr_to_info(dqa);
	if (overcommit) {
		dqai.dqai_overcommit = _dispatch_queue_attr_overcommit_enabled;
	} else {
		dqai.dqai_overcommit = _dispatch_queue_attr_overcommit_disabled;
	}
	return _dispatch_queue_attr_from_info(dqai);
}

dispatch_queue_attr_t
dispatch_queue_attr_make_with_autorelease_frequency(dispatch_queue_attr_t dqa,
		dispatch_autorelease_frequency_t frequency)
{
	switch (frequency) {
	case DISPATCH_AUTORELEASE_FREQUENCY_INHERIT:
	case DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM:
	case DISPATCH_AUTORELEASE_FREQUENCY_NEVER:
		break;
	default:
		return (dispatch_queue_attr_t)dqa;
	}
	dispatch_queue_attr_info_t dqai = _dispatch_queue_attr_to_info(dqa);
	dqai.dqai_autorelease_frequency = (uint16_t)frequency;
	return _dispatch_queue_attr_from_info(dqai);
}

#pragma mark -
#pragma mark dispatch_vtables

DISPATCH_NOINLINE
static void
_dispatch_object_no_dispose(dispatch_object_t dou,
		DISPATCH_UNUSED bool *allow_free)
{
	DISPATCH_INTERNAL_CRASH(dx_type(dou._do), "do_dispose called");
}

DISPATCH_NOINLINE
static size_t
_dispatch_object_missing_debug(DISPATCH_UNUSED dispatch_object_t dou,
		char *buf, size_t bufsiz)
{
	return strlcpy(buf, "missing do_debug vtable slot: ", bufsiz);
}

DISPATCH_NOINLINE
static void
_dispatch_object_no_invoke(dispatch_object_t dou,
		DISPATCH_UNUSED dispatch_invoke_context_t dic,
		DISPATCH_UNUSED dispatch_invoke_flags_t flags)
{
	DISPATCH_INTERNAL_CRASH(dx_type(dou._do), "do_invoke called");
}

/*
 * Dispatch object cluster
 */

DISPATCH_VTABLE_INSTANCE(semaphore,
	.do_type        = DISPATCH_SEMAPHORE_TYPE,
	.do_dispose     = _dispatch_semaphore_dispose,
	.do_debug       = _dispatch_semaphore_debug,
	.do_invoke      = _dispatch_object_no_invoke,
);

DISPATCH_VTABLE_INSTANCE(group,
	.do_type        = DISPATCH_GROUP_TYPE,
	.do_dispose     = _dispatch_group_dispose,
	.do_debug       = _dispatch_group_debug,
	.do_invoke      = _dispatch_object_no_invoke,
);

#if !DISPATCH_DATA_IS_BRIDGED_TO_NSDATA
DISPATCH_VTABLE_INSTANCE(data,
	.do_type        = DISPATCH_DATA_TYPE,
	.do_dispose     = _dispatch_data_dispose,
	.do_debug       = _dispatch_data_debug,
	.do_invoke      = _dispatch_object_no_invoke,
);
#endif

DISPATCH_VTABLE_INSTANCE(queue_attr,
	.do_type        = DISPATCH_QUEUE_ATTR_TYPE,
	.do_dispose     = _dispatch_object_no_dispose,
	.do_debug       = _dispatch_object_missing_debug,
	.do_invoke      = _dispatch_object_no_invoke,
);

#if HAVE_MACH
DISPATCH_VTABLE_INSTANCE(mach_msg,
	.do_type        = DISPATCH_MACH_MSG_TYPE,
	.do_dispose     = _dispatch_mach_msg_dispose,
	.do_debug       = _dispatch_mach_msg_debug,
	.do_invoke      = _dispatch_mach_msg_invoke,
);
#endif // HAVE_MACH

DISPATCH_VTABLE_INSTANCE(io,
	.do_type        = DISPATCH_IO_TYPE,
	.do_dispose     = _dispatch_io_dispose,
	.do_debug       = _dispatch_io_debug,
	.do_invoke      = _dispatch_object_no_invoke,
);

DISPATCH_VTABLE_INSTANCE(operation,
	.do_type        = DISPATCH_OPERATION_TYPE,
	.do_dispose     = _dispatch_operation_dispose,
	.do_debug       = _dispatch_operation_debug,
	.do_invoke      = _dispatch_object_no_invoke,
);

DISPATCH_VTABLE_INSTANCE(disk,
	.do_type        = DISPATCH_DISK_TYPE,
	.do_dispose     = _dispatch_disk_dispose,
	.do_debug       = _dispatch_object_missing_debug,
	.do_invoke      = _dispatch_object_no_invoke,
);

/*
 * Dispatch queue cluster
 */

DISPATCH_NOINLINE
static void
_dispatch_queue_no_activate(dispatch_queue_class_t dqu)
{
	DISPATCH_INTERNAL_CRASH(dx_type(dqu._dq), "dq_activate called");
}

DISPATCH_VTABLE_INSTANCE(queue,
	// This is the base class for queues, no objects of this type are made
	.do_type        = _DISPATCH_QUEUE_CLUSTER,
	.do_dispose     = _dispatch_object_no_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_object_no_invoke,

	.dq_activate    = _dispatch_queue_no_activate,
);

DISPATCH_VTABLE_INSTANCE(workloop,
	.do_type        = DISPATCH_WORKLOOP_TYPE,
	.do_dispose     = _dispatch_workloop_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_workloop_invoke,

	.dq_activate    = _dispatch_queue_no_activate,
	.dq_wakeup      = _dispatch_workloop_wakeup,
	.dq_push        = _dispatch_workloop_push,
);

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_serial, lane,
	.do_type        = DISPATCH_QUEUE_SERIAL_TYPE,
	.do_dispose     = _dispatch_lane_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_lane_invoke,

	.dq_activate    = _dispatch_lane_activate,
	.dq_wakeup      = _dispatch_lane_wakeup,
	.dq_push        = _dispatch_lane_push,
);

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_concurrent, lane,
	.do_type        = DISPATCH_QUEUE_CONCURRENT_TYPE,
	.do_dispose     = _dispatch_lane_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_lane_invoke,

	.dq_activate    = _dispatch_lane_activate,
	.dq_wakeup      = _dispatch_lane_wakeup,
	.dq_push        = _dispatch_lane_concurrent_push,
);

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_global, lane,
	.do_type        = DISPATCH_QUEUE_GLOBAL_ROOT_TYPE,
	.do_dispose     = _dispatch_object_no_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_object_no_invoke,

	.dq_activate    = _dispatch_queue_no_activate,
	.dq_wakeup      = _dispatch_root_queue_wakeup,
	.dq_push        = _dispatch_root_queue_push,
);

#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_pthread_root, lane,
	.do_type        = DISPATCH_QUEUE_PTHREAD_ROOT_TYPE,
	.do_dispose     = _dispatch_pthread_root_queue_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_object_no_invoke,

	.dq_activate    = _dispatch_queue_no_activate,
	.dq_wakeup      = _dispatch_root_queue_wakeup,
	.dq_push        = _dispatch_root_queue_push,
);
#endif // DISPATCH_USE_PTHREAD_ROOT_QUEUES

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_mgr, lane,
	.do_type        = DISPATCH_QUEUE_MGR_TYPE,
	.do_dispose     = _dispatch_object_no_dispose,
	.do_debug       = _dispatch_queue_debug,
#if DISPATCH_USE_MGR_THREAD
	.do_invoke      = _dispatch_mgr_thread,
#else
	.do_invoke      = _dispatch_object_no_invoke,
#endif

	.dq_activate    = _dispatch_queue_no_activate,
	.dq_wakeup      = _dispatch_mgr_queue_wakeup,
	.dq_push        = _dispatch_mgr_queue_push,
);

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_main, lane,
	.do_type        = DISPATCH_QUEUE_MAIN_TYPE,
	.do_dispose     = _dispatch_lane_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_lane_invoke,

	.dq_activate    = _dispatch_queue_no_activate,
	.dq_wakeup      = _dispatch_main_queue_wakeup,
	.dq_push        = _dispatch_main_queue_push,
);

#if DISPATCH_COCOA_COMPAT
DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_runloop, lane,
	.do_type        = DISPATCH_QUEUE_RUNLOOP_TYPE,
	.do_dispose     = _dispatch_runloop_queue_dispose,
	.do_debug       = _dispatch_queue_debug,
	.do_invoke      = _dispatch_lane_invoke,

	.dq_activate    = _dispatch_queue_no_activate,
	.dq_wakeup      = _dispatch_runloop_queue_wakeup,
	.dq_push        = _dispatch_lane_push,
);
#endif

DISPATCH_VTABLE_INSTANCE(source,
	.do_type        = DISPATCH_SOURCE_KEVENT_TYPE,
	.do_dispose     = _dispatch_source_dispose,
	.do_debug       = _dispatch_source_debug,
	.do_invoke      = _dispatch_source_invoke,

	.dq_activate    = _dispatch_source_activate,
	.dq_wakeup      = _dispatch_source_wakeup,
	.dq_push        = _dispatch_lane_push,
);

DISPATCH_VTABLE_INSTANCE(channel,
	.do_type        = DISPATCH_CHANNEL_TYPE,
	.do_dispose     = _dispatch_channel_dispose,
	.do_debug       = _dispatch_channel_debug,
	.do_invoke      = _dispatch_channel_invoke,

	.dq_activate    = _dispatch_lane_activate,
	.dq_wakeup      = _dispatch_channel_wakeup,
	.dq_push        = _dispatch_lane_push,
);

#if HAVE_MACH
DISPATCH_VTABLE_INSTANCE(mach,
	.do_type        = DISPATCH_MACH_CHANNEL_TYPE,
	.do_dispose     = _dispatch_mach_dispose,
	.do_debug       = _dispatch_mach_debug,
	.do_invoke      = _dispatch_mach_invoke,

	.dq_activate    = _dispatch_mach_activate,
	.dq_wakeup      = _dispatch_mach_wakeup,
	.dq_push        = _dispatch_lane_push,
);
#endif // HAVE_MACH

void
_dispatch_vtable_init(void)
{
#if OS_OBJECT_HAVE_OBJC2
	// ObjC classes and dispatch vtables are co-located via linker order and
	// alias files, verify correct layout during initialization rdar://10640168
	dispatch_assert((char*)&DISPATCH_CONCAT(_,DISPATCH_CLASS(semaphore_vtable))
			- (char*)DISPATCH_VTABLE(semaphore) ==
			offsetof(struct dispatch_semaphore_vtable_s, _os_obj_vtable));
#endif // USE_OBJC
}

#pragma mark -
#pragma mark dispatch_data globals

const dispatch_block_t _dispatch_data_destructor_free = ^{
	DISPATCH_INTERNAL_CRASH(0, "free destructor called");
};

const dispatch_block_t _dispatch_data_destructor_none = ^{
	DISPATCH_INTERNAL_CRASH(0, "none destructor called");
};

#if !HAVE_MACH
const dispatch_block_t _dispatch_data_destructor_munmap = ^{
	DISPATCH_INTERNAL_CRASH(0, "munmap destructor called");
};
#else
// _dispatch_data_destructor_munmap is a linker alias to the following
const dispatch_block_t _dispatch_data_destructor_vm_deallocate = ^{
	DISPATCH_INTERNAL_CRASH(0, "vmdeallocate destructor called");
};
#endif

const dispatch_block_t _dispatch_data_destructor_inline = ^{
	DISPATCH_INTERNAL_CRASH(0, "inline destructor called");
};

struct dispatch_data_s _dispatch_data_empty = {
#if DISPATCH_DATA_IS_BRIDGED_TO_NSDATA
	.do_vtable = DISPATCH_DATA_EMPTY_CLASS,
#else
	DISPATCH_GLOBAL_OBJECT_HEADER(data),
	.do_next = DISPATCH_OBJECT_LISTLESS,
#endif
};

#pragma mark -
#pragma mark dispatch_bug

static char _dispatch_build[16];

static void
_dispatch_build_init(void *context DISPATCH_UNUSED)
{
#ifdef __APPLE__
	int mib[] = { CTL_KERN, KERN_OSVERSION };
	size_t bufsz = sizeof(_dispatch_build);

	sysctl(mib, 2, _dispatch_build, &bufsz, NULL, 0);
#if TARGET_OS_SIMULATOR
	char *sim_version = getenv("SIMULATOR_RUNTIME_BUILD_VERSION");
	if (sim_version) {
		(void)strlcat(_dispatch_build, " ", sizeof(_dispatch_build));
		(void)strlcat(_dispatch_build, sim_version, sizeof(_dispatch_build));
	}
#endif // TARGET_OS_SIMULATOR

#else
	/*
	 * XXXRW: What to do here for !Mac OS X?
	 */
	memset(_dispatch_build, 0, sizeof(_dispatch_build));
#endif // __APPLE__
}

static dispatch_once_t _dispatch_build_pred;

bool
_dispatch_parse_bool(const char *v)
{
	return strcasecmp(v, "YES") == 0 || strcasecmp(v, "Y") == 0 ||
			strcasecmp(v, "TRUE") == 0 || atoi(v);
}

DISPATCH_NOINLINE
bool
_dispatch_getenv_bool(const char *env, bool default_v)
{
	const char *v = getenv(env);

	return v ? _dispatch_parse_bool(v) : default_v;
}

char*
_dispatch_get_build(void)
{
	dispatch_once_f(&_dispatch_build_pred, NULL, _dispatch_build_init);
	return _dispatch_build;
}

#define _dispatch_bug_log_is_repeated() ({ \
		static void *last_seen; \
		void *previous = last_seen; \
		last_seen =__builtin_return_address(0); \
		last_seen == previous; \
	})

#if HAVE_OS_FAULT_WITH_PAYLOAD
__attribute__((__format__(__printf__,2,3)))
static void
_dispatch_fault(const char *reason, const char *fmt, ...)
{
	char buf[1024];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (_dispatch_mode & DISPATCH_MODE_STRICT) {
#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
	} else if (!(_dispatch_mode & DISPATCH_MODE_NO_FAULTS)) {
		os_fault_with_payload(OS_REASON_LIBSYSTEM,
				OS_REASON_LIBSYSTEM_CODE_FAULT,
				buf, (uint32_t)strlen(buf) + 1, reason, 0);
#else
		(void)reason;
#endif
	}
}
#else
#define _dispatch_fault(reason, fmt, ...)
#endif // HAVE_OS_FAULT_WITH_PAYLOAD

#define _dispatch_log_fault(reason, fmt, ...)  ({ \
		if (!_dispatch_bug_log_is_repeated()) { \
			_dispatch_log(fmt, ##__VA_ARGS__); \
			_dispatch_fault(reason, fmt, ##__VA_ARGS__); \
			if (_dispatch_mode & DISPATCH_MODE_STRICT) { \
				DISPATCH_CLIENT_CRASH(0, reason); \
			} \
		} \
	})

void
_dispatch_bug(size_t line, long val)
{
	dispatch_once_f(&_dispatch_build_pred, NULL, _dispatch_build_init);

	if (_dispatch_bug_log_is_repeated()) return;

	_dispatch_log("BUG in libdispatch: %s - %lu - 0x%lx",
			_dispatch_build, (unsigned long)line, val);
}

#if HAVE_MACH
void
_dispatch_bug_mach_client(const char *msg, mach_msg_return_t kr)
{
	_dispatch_log_fault("LIBDISPATCH_STRICT: _dispatch_bug_mach_client",
			"BUG in libdispatch client: %s %s - 0x%x", msg,
			mach_error_string(kr), kr);
}
#endif

void *
_dispatch_continuation_get_function_symbol(dispatch_continuation_t dc)
{
	if (dc->dc_flags & DC_FLAG_BLOCK_WITH_PRIVATE_DATA) {
		dispatch_block_private_data_t dpbd = _dispatch_block_get_data(dc->dc_ctxt);
		return _dispatch_Block_invoke(dpbd->dbpd_block);
	}
	if (dc->dc_flags & DC_FLAG_BLOCK) {
		return _dispatch_Block_invoke(dc->dc_ctxt);
	}
	return dc->dc_func;
}

void
_dispatch_bug_kevent_client(const char *msg, const char *filter,
		const char *operation, int err, uint64_t ident, uint64_t udata,
		dispatch_unote_t du)
{
	dispatch_continuation_t dc;
	dispatch_object_t dou;
	void *func = NULL;

	if (du._du) {
		dou._do = _dispatch_wref2ptr(du._du->du_owner_wref);
		switch (dx_type(dou._do)) {
		case DISPATCH_SOURCE_KEVENT_TYPE:
			dc = du._dr->ds_handler[DS_EVENT_HANDLER];
			if (dc) func = _dispatch_continuation_get_function_symbol(dc);
			break;
		case DISPATCH_MACH_CHANNEL_TYPE:
			func = du._dmrr->dmrr_handler_func;
			break;
		}
		filter = dux_type(du._du)->dst_kind;
	}

	if (operation && err) {
		_dispatch_log_fault("LIBDISPATCH_STRICT: _dispatch_bug_kevent_client",
				"BUG in libdispatch client: %s %s: \"%s\" - 0x%x "
				"{ 0x%llx[%s], ident: %lld / 0x%llx, handler: %p }",
				msg, operation, strerror(err), err,
				udata, filter, ident, ident, func);
	} else if (operation) {
		_dispatch_log_fault("LIBDISPATCH_STRICT: _dispatch_bug_kevent_client",
				"BUG in libdispatch client: %s %s"
				"{ 0x%llx[%s], ident: %lld / 0x%llx, handler: %p }",
				msg, operation, udata, filter, ident, ident, func);
	} else {
		_dispatch_log_fault("LIBDISPATCH_STRICT: _dispatch_bug_kevent_client",
				"BUG in libdispatch: %s: \"%s\" - 0x%x"
				"{ 0x%llx[%s], ident: %lld / 0x%llx, handler: %p }",
				msg, strerror(err), err, udata, filter, ident, ident, func);
	}
}

void
_dispatch_bug_kevent_vanished(dispatch_unote_t du)
{
	dispatch_continuation_t dc;
	dispatch_object_t dou;
	void *func = NULL;

	dou._do = _dispatch_wref2ptr(du._du->du_owner_wref);
	switch (dx_type(dou._do)) {
	case DISPATCH_SOURCE_KEVENT_TYPE:
		dc = du._dr->ds_handler[DS_EVENT_HANDLER];
		if (dc) func = _dispatch_continuation_get_function_symbol(dc);
		break;
	case DISPATCH_MACH_CHANNEL_TYPE:
		func = du._dmrr->dmrr_handler_func;
		break;
	}
	_dispatch_log_fault("LIBDISPATCH_STRICT: _dispatch_bug_kevent_vanished",
			"BUG in libdispatch client: %s, monitored resource vanished before "
			"the source cancel handler was invoked "
			"{ %p[%s], ident: %d / 0x%x, handler: %p }",
			dux_type(du._du)->dst_kind, dou._dq,
			dou._dq->dq_label ? dou._dq->dq_label : "<unknown>",
			du._du->du_ident, du._du->du_ident, func);
}

DISPATCH_NOINLINE DISPATCH_WEAK
void
_dispatch_bug_deprecated(const char *msg)
{
	_dispatch_log_fault("LIBDISPATCH_STRICT: _dispatch_bug_deprecated",
			"DEPRECATED USE in libdispatch client: %s; "
			"set a breakpoint on _dispatch_bug_deprecated to debug", msg);
}

void
_dispatch_abort(size_t line, long val)
{
	_dispatch_bug(line, val);
	abort();
}

#if !DISPATCH_USE_OS_DEBUG_LOG

#pragma mark -
#pragma mark dispatch_log

static int dispatch_logfile = -1;
static bool dispatch_log_disabled;
#if DISPATCH_DEBUG
static uint64_t dispatch_log_basetime;
#endif
static dispatch_once_t _dispatch_logv_pred;

static void
_dispatch_logv_init(void *context DISPATCH_UNUSED)
{
#if DISPATCH_DEBUG
	bool log_to_file = true;
#else
	bool log_to_file = false;
#endif
	char *e = getenv("LIBDISPATCH_LOG");
	if (e) {
		if (strcmp(e, "YES") == 0) {
			// default
		} else if (strcmp(e, "NO") == 0) {
			dispatch_log_disabled = true;
		} else if (strcmp(e, "syslog") == 0) {
			log_to_file = false;
		} else if (strcmp(e, "file") == 0) {
			log_to_file = true;
		} else if (strcmp(e, "stderr") == 0) {
			log_to_file = true;
#if defined(_WIN32)
			dispatch_logfile = _fileno(stderr);
#else
			dispatch_logfile = STDERR_FILENO;
#endif
		}
	}
	if (!dispatch_log_disabled) {
		if (log_to_file && dispatch_logfile == -1) {
#if defined(_WIN32)
			char path[MAX_PATH + 1] = {0};
			DWORD dwLength = GetTempPathA(MAX_PATH, path);
			dispatch_assert(dwLength <= MAX_PATH + 1);
			snprintf(&path[dwLength], MAX_PATH - dwLength, "libdispatch.%d.log",
					GetCurrentProcessId());
			dispatch_logfile = _open(path, O_WRONLY | O_APPEND | O_CREAT, 0666);
#else
			char path[PATH_MAX];
			snprintf(path, sizeof(path), "/var/tmp/libdispatch.%d.log",
					getpid());
			dispatch_logfile = open(path, O_WRONLY | O_APPEND | O_CREAT |
					O_NOFOLLOW | O_CLOEXEC, 0666);
#endif
		}
		if (dispatch_logfile != -1) {
			struct timeval tv;
#if defined(_WIN32)
			DWORD dwTime = GetTickCount();
			tv.tv_sec = dwTime / 1000;
			tv.tv_usec = 1000 * (dwTime % 1000);
#else
			gettimeofday(&tv, NULL);
#endif
#if DISPATCH_DEBUG
			dispatch_log_basetime = _dispatch_uptime();
#endif
#if defined(_WIN32)
			FILE *pLogFile = _fdopen(dispatch_logfile, "w");

			char szProgramName[MAX_PATH + 1] = {0};
			GetModuleFileNameA(NULL, szProgramName, MAX_PATH);

			fprintf(pLogFile, "=== log file opened for %s[%lu] at "
					"%ld.%06u ===\n", szProgramName, GetCurrentProcessId(),
					tv.tv_sec, (int)tv.tv_usec);
			fclose(pLogFile);
#else
			dprintf(dispatch_logfile, "=== log file opened for %s[%u] at "
					"%ld.%06u ===\n", getprogname() ?: "", getpid(),
					tv.tv_sec, (int)tv.tv_usec);
#endif
		}
	}
}

static inline void
_dispatch_log_file(char *buf, size_t len)
{
	ssize_t r;

	buf[len++] = '\n';
retry:
#if defined(_WIN32)
	dispatch_assert(len <= UINT_MAX);
	r = _write(dispatch_logfile, buf, (unsigned int)len);
#else
	r = write(dispatch_logfile, buf, len);
#endif
	if (unlikely(r == -1) && errno == EINTR) {
		goto retry;
	}
}

DISPATCH_NOINLINE
static void
_dispatch_logv_file(const char *msg, va_list ap)
{
	char buf[2048];
	size_t bufsiz = sizeof(buf), offset = 0;
	int r;

#if DISPATCH_DEBUG
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%llu\t",
			(unsigned long long)_dispatch_uptime() - dispatch_log_basetime);
#endif
	r = vsnprintf(&buf[offset], bufsiz - offset, msg, ap);
	if (r < 0) return;
	offset += (size_t)r;
	if (offset > bufsiz - 1) {
		offset = bufsiz - 1;
	}
	_dispatch_log_file(buf, offset);
}

#if DISPATCH_USE_SIMPLE_ASL
static inline void
_dispatch_syslog(const char *msg)
{
	_simple_asl_log(ASL_LEVEL_NOTICE, "com.apple.libsystem.libdispatch", msg);
}

static inline void
_dispatch_vsyslog(const char *msg, va_list ap)
{
	char *str;
	vasprintf(&str, msg, ap);
	if (str) {
		_dispatch_syslog(str);
		free(str);
	}
}
#elif defined(_WIN32)
static inline void
_dispatch_syslog(const char *msg)
{
	OutputDebugStringA(msg);
}

static inline void
_dispatch_vsyslog(const char *msg, va_list ap)
{
	va_list argp;

	va_copy(argp, ap);

	int length = _vscprintf(msg, ap);
	if (length == -1)
		return;

	char *buffer = malloc((size_t)length + 1);
	if (buffer == NULL)
		return;

	_vsnprintf(buffer, (size_t)length + 1, msg, argp);

	va_end(argp);

	_dispatch_syslog(buffer);

	free(buffer);
}
#else // DISPATCH_USE_SIMPLE_ASL
static inline void
_dispatch_syslog(const char *msg)
{
	syslog(LOG_NOTICE, "%s", msg);
}

static inline void
_dispatch_vsyslog(const char *msg, va_list ap)
{
	vsyslog(LOG_NOTICE, msg, ap);
}
#endif // DISPATCH_USE_SIMPLE_ASL

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_logv(const char *msg, size_t len, va_list *ap_ptr)
{
	dispatch_once_f(&_dispatch_logv_pred, NULL, _dispatch_logv_init);
	if (unlikely(dispatch_log_disabled)) {
		return;
	}
	if (unlikely(dispatch_logfile != -1)) {
		if (!ap_ptr) {
			return _dispatch_log_file((char*)msg, len);
		}
		return _dispatch_logv_file(msg, *ap_ptr);
	}
	if (!ap_ptr) {
		return _dispatch_syslog(msg);
	}
	return _dispatch_vsyslog(msg, *ap_ptr);
}

DISPATCH_NOINLINE
void
_dispatch_log(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	_dispatch_logv(msg, 0, &ap);
	va_end(ap);
}

#endif // DISPATCH_USE_OS_DEBUG_LOG

#pragma mark -
#pragma mark dispatch_debug

static size_t
_dispatch_object_debug2(dispatch_object_t dou, char* buf, size_t bufsiz)
{
	DISPATCH_OBJECT_TFB(_dispatch_objc_debug, dou, buf, bufsiz);
	return dx_debug(dou._do, buf, bufsiz);
}

DISPATCH_NOINLINE
static void
_dispatch_debugv(dispatch_object_t dou, const char *msg, va_list ap)
{
	char buf[2048];
	size_t bufsiz = sizeof(buf), offset = 0;
	int r;
#if DISPATCH_DEBUG && !DISPATCH_USE_OS_DEBUG_LOG
	offset += dsnprintf(&buf[offset], bufsiz - offset, "%llu\t\t%p\t",
			(unsigned long long)_dispatch_uptime() - dispatch_log_basetime,
			(void *)_dispatch_thread_self());
#endif
	if (dou._do) {
		offset += _dispatch_object_debug2(dou, &buf[offset], bufsiz - offset);
		dispatch_assert(offset + 2 < bufsiz);
		buf[offset++] = ':';
		buf[offset++] = ' ';
		buf[offset]   = '\0';
	} else {
		offset += strlcpy(&buf[offset], "NULL: ", bufsiz - offset);
	}
	r = vsnprintf(&buf[offset], bufsiz - offset, msg, ap);
#if !DISPATCH_USE_OS_DEBUG_LOG
	size_t len = offset + (r < 0 ? 0 : (size_t)r);
	if (len > bufsiz - 1) {
		len = bufsiz - 1;
	}
	_dispatch_logv(buf, len, NULL);
#else
	_dispatch_log("%s", buf);
#endif
}

DISPATCH_NOINLINE
void
dispatch_debugv(dispatch_object_t dou, const char *msg, va_list ap)
{
	_dispatch_debugv(dou, msg, ap);
}

DISPATCH_NOINLINE
void
dispatch_debug(dispatch_object_t dou, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	_dispatch_debugv(dou, msg, ap);
	va_end(ap);
}

#if DISPATCH_DEBUG
DISPATCH_NOINLINE
void
_dispatch_object_debug(dispatch_object_t dou, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	_dispatch_debugv(dou._do, msg, ap);
	va_end(ap);
}
#endif

#pragma mark -
#pragma mark dispatch_calloc

DISPATCH_NOINLINE
void
_dispatch_temporary_resource_shortage(void)
{
	sleep(1);
	__asm__ __volatile__("");  // prevent tailcall
}

void *
_dispatch_calloc(size_t num_items, size_t size)
{
	void *buf;
	while (unlikely(!(buf = calloc(num_items, size)))) {
		_dispatch_temporary_resource_shortage();
	}
	return buf;
}

/*
 * If the source string is mutable, allocates memory and copies the contents.
 * Otherwise returns the source string.
 */
const char *
_dispatch_strdup_if_mutable(const char *str)
{
#if HAVE_DYLD_IS_MEMORY_IMMUTABLE
	size_t size = strlen(str) + 1;
	if (unlikely(!_dyld_is_memory_immutable(str, size))) {
		char *clone = (char *) malloc(size);
		if (dispatch_assume(clone)) {
			memcpy(clone, str, size);
		}
		return clone;
	}
	return str;
#else
	return strdup(str);
#endif
}

#pragma mark -
#pragma mark dispatch_block_t

#ifdef __BLOCKS__

void *
(_dispatch_Block_copy)(void *db)
{
	dispatch_block_t rval;

	if (likely(db)) {
		while (unlikely(!(rval = Block_copy(db)))) {
			_dispatch_temporary_resource_shortage();
		}
		return rval;
	}
	DISPATCH_CLIENT_CRASH(0, "NULL was passed where a block should have been");
}

void
_dispatch_call_block_and_release(void *block)
{
	void (^b)(void) = block;
	b();
	Block_release(b);
}

#endif // __BLOCKS__

#pragma mark -
#pragma mark dispatch_client_callout

// Abort on uncaught exceptions thrown from client callouts rdar://8577499
#if DISPATCH_USE_CLIENT_CALLOUT && (__USING_SJLJ_EXCEPTIONS__ || !USE_OBJC || \
		OS_OBJECT_HAVE_OBJC1)
// On platforms with SjLj exceptions, avoid the SjLj overhead on every callout
// by clearing the unwinder's TSD pointer to the handler stack around callouts

#define _dispatch_get_tsd_base()
#define _dispatch_get_unwind_tsd() (NULL)
#define _dispatch_set_unwind_tsd(u) do {(void)(u);} while (0)
#define _dispatch_free_unwind_tsd()

#undef _dispatch_client_callout
DISPATCH_NOINLINE
void
_dispatch_client_callout(void *ctxt, dispatch_function_t f)
{
	_dispatch_get_tsd_base();
	void *u = _dispatch_get_unwind_tsd();
	if (likely(!u)) return f(ctxt);
	_dispatch_set_unwind_tsd(NULL);
	f(ctxt);
	_dispatch_free_unwind_tsd();
	_dispatch_set_unwind_tsd(u);
}

#undef _dispatch_client_callout2
DISPATCH_NOINLINE
void
_dispatch_client_callout2(void *ctxt, size_t i, void (*f)(void *, size_t))
{
	_dispatch_get_tsd_base();
	void *u = _dispatch_get_unwind_tsd();
	if (likely(!u)) return f(ctxt, i);
	_dispatch_set_unwind_tsd(NULL);
	f(ctxt, i);
	_dispatch_free_unwind_tsd();
	_dispatch_set_unwind_tsd(u);
}

#if HAVE_MACH

#undef _dispatch_client_callout3
DISPATCH_NOINLINE
void
_dispatch_client_callout3(void *ctxt, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t dmsg, dispatch_mach_async_reply_callback_t f)
{
	_dispatch_get_tsd_base();
	void *u = _dispatch_get_unwind_tsd();
	if (likely(!u)) return f(ctxt, reason, dmsg);
	_dispatch_set_unwind_tsd(NULL);
	f(ctxt, reason, dmsg);
	_dispatch_free_unwind_tsd();
	_dispatch_set_unwind_tsd(u);
}

#undef _dispatch_client_callout4
void
_dispatch_client_callout4(void *ctxt, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t dmsg, mach_error_t error,
		dispatch_mach_handler_function_t f)
{
	_dispatch_get_tsd_base();
	void *u = _dispatch_get_unwind_tsd();
	if (likely(!u)) return f(ctxt, reason, dmsg, error);
	_dispatch_set_unwind_tsd(NULL);
	f(ctxt, reason, dmsg, error);
	_dispatch_free_unwind_tsd();
	_dispatch_set_unwind_tsd(u);
}
#endif // HAVE_MACH

#endif // DISPATCH_USE_CLIENT_CALLOUT

#pragma mark -
#pragma mark _os_object_t no_objc

#if !USE_OBJC

static const _os_object_vtable_s _os_object_vtable;

void
_os_object_init(void)
{
	return;
}

inline _os_object_t
_os_object_alloc_realized(const void *cls, size_t size)
{
	_os_object_t obj;
	dispatch_assert(size >= sizeof(struct _os_object_s));
	while (unlikely(!(obj = calloc(1u, size)))) {
		_dispatch_temporary_resource_shortage();
	}
	obj->os_obj_isa = cls;
	return obj;
}

_os_object_t
_os_object_alloc(const void *cls, size_t size)
{
	if (!cls) cls = &_os_object_vtable;
	return _os_object_alloc_realized(cls, size);
}

void
_os_object_dealloc(_os_object_t obj)
{
	*((void *volatile*)&obj->os_obj_isa) = (void *)0x200;
	return free(obj);
}

void
_os_object_xref_dispose(_os_object_t obj)
{
	_os_object_xrefcnt_dispose_barrier(obj);
	if (likely(obj->os_obj_isa->_os_obj_xref_dispose)) {
		return obj->os_obj_isa->_os_obj_xref_dispose(obj);
	}
	return _os_object_release_internal(obj);
}

void
_os_object_dispose(_os_object_t obj)
{
	_os_object_refcnt_dispose_barrier(obj);
	if (likely(obj->os_obj_isa->_os_obj_dispose)) {
		return obj->os_obj_isa->_os_obj_dispose(obj);
	}
	return _os_object_dealloc(obj);
}

void*
os_retain(void *obj)
{
	if (likely(obj)) {
		return _os_object_retain(obj);
	}
	return obj;
}

#undef os_release
void
os_release(void *obj)
{
	if (likely(obj)) {
		return _os_object_release(obj);
	}
}

void
_os_object_atfork_prepare(void)
{
	return;
}

void
_os_object_atfork_parent(void)
{
	return;
}

void
_os_object_atfork_child(void)
{
	return;
}

#pragma mark -
#pragma mark dispatch_autorelease_pool no_objc

#if DISPATCH_COCOA_COMPAT

void*
_dispatch_autorelease_pool_push(void)
{
	void *pool = NULL;
	if (_dispatch_begin_NSAutoReleasePool) {
		pool = _dispatch_begin_NSAutoReleasePool();
	}
	return pool;
}

void
_dispatch_autorelease_pool_pop(void *pool)
{
	if (_dispatch_end_NSAutoReleasePool) {
		_dispatch_end_NSAutoReleasePool(pool);
	}
}

void
_dispatch_last_resort_autorelease_pool_push(dispatch_invoke_context_t dic)
{
	dic->dic_autorelease_pool = _dispatch_autorelease_pool_push();
}

void
_dispatch_last_resort_autorelease_pool_pop(dispatch_invoke_context_t dic)
{
	_dispatch_autorelease_pool_pop(dic->dic_autorelease_pool);
	dic->dic_autorelease_pool = NULL;
}

#endif // DISPATCH_COCOA_COMPAT
#endif // !USE_OBJC

#pragma mark -
#pragma mark dispatch_mig
#if HAVE_MACH

void *
dispatch_mach_msg_get_context(mach_msg_header_t *msg)
{
	mach_msg_context_trailer_t *tp;
	void *context = NULL;

	tp = (mach_msg_context_trailer_t *)((uint8_t *)msg +
			round_msg(msg->msgh_size));
	if (tp->msgh_trailer_size >=
			(mach_msg_size_t)sizeof(mach_msg_context_trailer_t)) {
		context = (void *)(uintptr_t)tp->msgh_context;
	}
	return context;
}

kern_return_t
_dispatch_wakeup_runloop_thread(mach_port_t mp DISPATCH_UNUSED)
{
	// dummy function just to pop a runloop thread out of mach_msg()
	return 0;
}

kern_return_t
_dispatch_consume_send_once_right(mach_port_t mp DISPATCH_UNUSED)
{
	// dummy function to consume a send-once right
	return 0;
}

kern_return_t
_dispatch_mach_notify_port_destroyed(mach_port_t notify DISPATCH_UNUSED,
		mach_port_t name)
{
	DISPATCH_INTERNAL_CRASH(name, "unexpected receipt of port-destroyed");
	return KERN_FAILURE;
}

kern_return_t
_dispatch_mach_notify_no_senders(mach_port_t notify DISPATCH_UNUSED,
		mach_port_mscount_t mscnt)
{
	DISPATCH_INTERNAL_CRASH(mscnt, "unexpected receipt of no-more-senders");
	return KERN_FAILURE;
}

kern_return_t
_dispatch_mach_notify_send_once(mach_port_t notify DISPATCH_UNUSED)
{
	// we only register for dead-name notifications
	// some code deallocated our send-once right without consuming it
#if DISPATCH_DEBUG
	_dispatch_log("Corruption: An app/library deleted a libdispatch "
			"dead-name notification");
#endif
	return KERN_SUCCESS;
}

#endif // HAVE_MACH
#pragma mark -
#pragma mark dispatch to XPC callbacks
#if HAVE_MACH

// Default dmxh_direct_message_handler callback that does not handle
// messages inline.
static bool
_dispatch_mach_xpc_no_handle_message(
		void *_Nullable context DISPATCH_UNUSED,
		dispatch_mach_reason_t reason DISPATCH_UNUSED,
		dispatch_mach_msg_t message DISPATCH_UNUSED,
		mach_error_t error DISPATCH_UNUSED)
{
	return false;
}

// Default dmxh_msg_context_reply_queue callback that returns a NULL queue.
static dispatch_queue_t
_dispatch_mach_msg_context_no_async_reply_queue(
		void *_Nonnull msg_context DISPATCH_UNUSED)
{
	return NULL;
}

// Default dmxh_async_reply_handler callback that crashes when called.
DISPATCH_NORETURN
static void
_dispatch_mach_default_async_reply_handler(void *context DISPATCH_UNUSED,
		dispatch_mach_reason_t reason DISPATCH_UNUSED,
		dispatch_mach_msg_t message DISPATCH_UNUSED)
{
	DISPATCH_CLIENT_CRASH(_dispatch_mach_xpc_hooks,
			"_dispatch_mach_default_async_reply_handler called");
}

// Default dmxh_enable_sigterm_notification callback that enables delivery of
// SIGTERM notifications (for backwards compatibility).
static bool
_dispatch_mach_enable_sigterm(void *_Nullable context DISPATCH_UNUSED)
{
	return true;
}

// Callbacks from dispatch to XPC. The default is to not support any callbacks.
const struct dispatch_mach_xpc_hooks_s _dispatch_mach_xpc_hooks_default = {
	.version = DISPATCH_MACH_XPC_HOOKS_VERSION,
	.dmxh_direct_message_handler = &_dispatch_mach_xpc_no_handle_message,
	.dmxh_msg_context_reply_queue =
			&_dispatch_mach_msg_context_no_async_reply_queue,
	.dmxh_async_reply_handler = &_dispatch_mach_default_async_reply_handler,
	.dmxh_enable_sigterm_notification = &_dispatch_mach_enable_sigterm,
};

dispatch_mach_xpc_hooks_t _dispatch_mach_xpc_hooks =
		&_dispatch_mach_xpc_hooks_default;

#endif // HAVE_MACH
