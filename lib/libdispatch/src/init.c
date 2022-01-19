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

DISPATCH_EXPORT DISPATCH_NOTHROW
void
_libdispatch_init(void)
{
	libdispatch_init();
}
#endif

DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_atfork_prepare(void)
{
}

DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_atfork_parent(void)
{
}

#pragma mark -
#pragma mark dispatch_globals

#if DISPATCH_COCOA_COMPAT
void (*dispatch_begin_thread_4GC)(void);
void (*dispatch_end_thread_4GC)(void);
void *(*_dispatch_begin_NSAutoReleasePool)(void);
void (*_dispatch_end_NSAutoReleasePool)(void *);
#endif

#if !DISPATCH_USE_DIRECT_TSD
pthread_key_t dispatch_queue_key;
pthread_key_t dispatch_sema4_key;
pthread_key_t dispatch_cache_key;
pthread_key_t dispatch_io_key;
pthread_key_t dispatch_apply_key;
pthread_key_t dispatch_defaultpriority_key;
#if DISPATCH_INTROSPECTION
pthread_key_t dispatch_introspection_key;
#elif DISPATCH_PERF_MON
pthread_key_t dispatch_bcounter_key;
#endif
#endif // !DISPATCH_USE_DIRECT_TSD

#if VOUCHER_USE_MACH_VOUCHER
dispatch_once_t _voucher_task_mach_voucher_pred;
mach_voucher_t _voucher_task_mach_voucher;
_voucher_activity_t _voucher_activity_default;
#endif
voucher_activity_mode_t _voucher_activity_mode;
int _dispatch_set_qos_class_enabled;


DISPATCH_NOINLINE
voucher_activity_mode_t
voucher_activity_get_mode(void)
{
	return _voucher_activity_mode;
}

void
voucher_activity_set_mode_4libtrace(voucher_activity_mode_t mode)
{
	if (_voucher_activity_disabled()) return;
	_voucher_activity_mode = mode;
}

DISPATCH_HW_CONFIG();
bool _dispatch_safe_fork = true, _dispatch_child_of_unsafe_fork;

DISPATCH_NOINLINE
bool
_dispatch_is_multithreaded(void)
{
	return !_dispatch_safe_fork;
}

DISPATCH_NOINLINE
bool
_dispatch_is_fork_of_multithreaded_parent(void)
{
	return _dispatch_child_of_unsafe_fork;
}

const struct dispatch_queue_offsets_s dispatch_queue_offsets = {
	.dqo_version = 5,
	.dqo_label = offsetof(struct dispatch_queue_s, dq_label),
	.dqo_label_size = sizeof(((dispatch_queue_t)NULL)->dq_label),
	.dqo_flags = 0,
	.dqo_flags_size = 0,
	.dqo_serialnum = offsetof(struct dispatch_queue_s, dq_serialnum),
	.dqo_serialnum_size = sizeof(((dispatch_queue_t)NULL)->dq_serialnum),
	.dqo_width = offsetof(struct dispatch_queue_s, dq_width),
	.dqo_width_size = sizeof(((dispatch_queue_t)NULL)->dq_width),
	.dqo_running = offsetof(struct dispatch_queue_s, dq_running),
	.dqo_running_size = sizeof(((dispatch_queue_t)NULL)->dq_running),
	.dqo_suspend_cnt = offsetof(struct dispatch_queue_s, do_suspend_cnt),
	.dqo_suspend_cnt_size = sizeof(((dispatch_queue_t)NULL)->do_suspend_cnt),
	.dqo_target_queue = offsetof(struct dispatch_queue_s, do_targetq),
	.dqo_target_queue_size = sizeof(((dispatch_queue_t)NULL)->do_targetq),
	.dqo_priority = offsetof(struct dispatch_queue_s, dq_priority),
	.dqo_priority_size = sizeof(((dispatch_queue_t)NULL)->dq_priority),
};

#if VOUCHER_USE_MACH_VOUCHER
const struct voucher_offsets_s voucher_offsets = {
	.vo_version = 1,
	.vo_activity_ids_count = offsetof(struct voucher_s, v_activities),
	.vo_activity_ids_count_size = sizeof(((voucher_t)NULL)->v_activities),
	.vo_activity_ids_array = (uint16_t)_voucher_activity_ids((voucher_t)(NULL)),
	.vo_activity_ids_array_entry_size = sizeof(voucher_activity_id_t),
};
#else // VOUCHER_USE_MACH_VOUCHER
const struct voucher_offsets_s voucher_offsets = {
	.vo_version = 0,
};
int dispatch_voucher_key;
#endif // VOUCHER_USE_MACH_VOUCHER

#if DISPATCH_USE_DIRECT_TSD
const struct dispatch_tsd_indexes_s dispatch_tsd_indexes = {
	.dti_version = 2,
	.dti_queue_index = dispatch_queue_key,
	.dti_voucher_index = dispatch_voucher_key,
	.dti_qos_class_index = dispatch_priority_key,
};
#endif // DISPATCH_USE_DIRECT_TSD

// 6618342 Contact the team that owns the Instrument DTrace probe before
//         renaming this symbol
DISPATCH_CACHELINE_ALIGN
struct dispatch_queue_s _dispatch_main_q = {
	.do_vtable = DISPATCH_VTABLE(queue),
#if !DISPATCH_USE_RESOLVERS
	.do_targetq = &_dispatch_root_queues[
			DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS_OVERCOMMIT],
#endif
	.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_suspend_cnt = DISPATCH_OBJECT_SUSPEND_LOCK,
	.dq_label = "com.apple.main-thread",
	.dq_running = 1,
	.dq_width = 1,
	.dq_is_thread_bound = 1,
	.dq_serialnum = 1,
};

struct dispatch_queue_attr_s _dispatch_queue_attr_concurrent = {
	.do_vtable = DISPATCH_VTABLE(queue_attr),
	.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_next = DISPATCH_OBJECT_LISTLESS,
	.dqa_concurrent = 1,
};

#pragma mark -
#pragma mark dispatch_queue_attr_t

#define DISPATCH_QUEUE_ATTR_INITIALIZER(qos, prio, overcommit, concurrent) \
		{ \
			.do_vtable = DISPATCH_VTABLE(queue_attr), \
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT, \
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT, \
			.do_next = DISPATCH_OBJECT_LISTLESS, \
			.dqa_qos_class = (qos), \
			.dqa_relative_priority = (qos) ? (prio) : 0, \
			.dqa_overcommit = (overcommit), \
			.dqa_concurrent = (concurrent), \
		}

#define DISPATCH_QUEUE_ATTR_KIND_INIT(qos, prio) \
		{ \
			[DQA_INDEX_NON_OVERCOMMIT][DQA_INDEX_CONCURRENT] = \
					DISPATCH_QUEUE_ATTR_INITIALIZER(qos, prio, 0, 1), \
			[DQA_INDEX_NON_OVERCOMMIT][DQA_INDEX_SERIAL] = \
					DISPATCH_QUEUE_ATTR_INITIALIZER(qos, prio, 0, 0), \
			[DQA_INDEX_OVERCOMMIT][DQA_INDEX_CONCURRENT] = \
					DISPATCH_QUEUE_ATTR_INITIALIZER(qos, prio, 1, 1), \
			[DQA_INDEX_OVERCOMMIT][DQA_INDEX_SERIAL] = \
					DISPATCH_QUEUE_ATTR_INITIALIZER(qos, prio, 1, 0), \
		}

#define DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, prio) \
		[prio] = DISPATCH_QUEUE_ATTR_KIND_INIT(qos, -(prio))

#define DISPATCH_QUEUE_ATTR_PRIO_INIT(qos) \
		{ \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 0), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 1), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 2), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 3), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 4), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 5), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 6), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 7), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 8), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 9), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 10), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 11), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 12), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 13), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 14), \
			DISPATCH_QUEUE_ATTR_PRIO_INITIALIZER(qos, 15), \
		}

#define DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(qos) \
		[DQA_INDEX_QOS_CLASS_##qos] = \
				DISPATCH_QUEUE_ATTR_PRIO_INIT(_DISPATCH_QOS_CLASS_##qos)

const struct dispatch_queue_attr_s _dispatch_queue_attrs[]
		[DISPATCH_QUEUE_ATTR_PRIO_COUNT][2][2] = {
	DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(UNSPECIFIED),
	DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(MAINTENANCE),
	DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(BACKGROUND),
	DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(UTILITY),
	DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(DEFAULT),
	DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(USER_INITIATED),
	DISPATCH_QUEUE_ATTR_QOS_INITIALIZER(USER_INTERACTIVE),
};


#pragma mark -
#pragma mark dispatch_vtables

DISPATCH_VTABLE_INSTANCE(semaphore,
	.do_type = DISPATCH_SEMAPHORE_TYPE,
	.do_kind = "semaphore",
	.do_dispose = _dispatch_semaphore_dispose,
	.do_debug = _dispatch_semaphore_debug,
);

DISPATCH_VTABLE_INSTANCE(group,
	.do_type = DISPATCH_GROUP_TYPE,
	.do_kind = "group",
	.do_dispose = _dispatch_semaphore_dispose,
	.do_debug = _dispatch_semaphore_debug,
);

DISPATCH_VTABLE_INSTANCE(queue,
	.do_type = DISPATCH_QUEUE_TYPE,
	.do_kind = "queue",
	.do_dispose = _dispatch_queue_dispose,
	.do_invoke = _dispatch_queue_invoke,
	.do_probe = _dispatch_queue_probe,
	.do_debug = dispatch_queue_debug,
);

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_root, queue,
	.do_type = DISPATCH_QUEUE_ROOT_TYPE,
	.do_kind = "global-queue",
	.do_dispose = _dispatch_pthread_root_queue_dispose,
	.do_probe = _dispatch_root_queue_probe,
	.do_debug = dispatch_queue_debug,
);

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_runloop, queue,
	.do_type = DISPATCH_QUEUE_ROOT_TYPE,
	.do_kind = "runloop-queue",
	.do_dispose = _dispatch_runloop_queue_dispose,
	.do_invoke = _dispatch_queue_invoke,
	.do_probe = _dispatch_runloop_queue_probe,
	.do_debug = dispatch_queue_debug,
);

DISPATCH_VTABLE_SUBCLASS_INSTANCE(queue_mgr, queue,
	.do_type = DISPATCH_QUEUE_MGR_TYPE,
	.do_kind = "mgr-queue",
	.do_invoke = _dispatch_mgr_thread,
	.do_probe = _dispatch_mgr_queue_probe,
	.do_debug = dispatch_queue_debug,
);

DISPATCH_VTABLE_INSTANCE(queue_specific_queue,
	.do_type = DISPATCH_QUEUE_SPECIFIC_TYPE,
	.do_kind = "queue-context",
	.do_dispose = _dispatch_queue_specific_queue_dispose,
	.do_invoke = (void*)_dispatch_queue_invoke,
	.do_probe = (void *)_dispatch_queue_probe,
	.do_debug = (void *)dispatch_queue_debug,
);

DISPATCH_VTABLE_INSTANCE(queue_attr,
	.do_type = DISPATCH_QUEUE_ATTR_TYPE,
	.do_kind = "queue-attr",
);

DISPATCH_VTABLE_INSTANCE(source,
	.do_type = DISPATCH_SOURCE_KEVENT_TYPE,
	.do_kind = "kevent-source",
	.do_dispose = _dispatch_source_dispose,
	.do_invoke = _dispatch_source_invoke,
	.do_probe = _dispatch_source_probe,
	.do_debug = _dispatch_source_debug,
);

DISPATCH_VTABLE_INSTANCE(mach,
	.do_type = DISPATCH_MACH_CHANNEL_TYPE,
	.do_kind = "mach-channel",
	.do_dispose = _dispatch_mach_dispose,
	.do_invoke = _dispatch_mach_invoke,
	.do_probe = _dispatch_mach_probe,
	.do_debug = _dispatch_mach_debug,
);

DISPATCH_VTABLE_INSTANCE(mach_msg,
	.do_type = DISPATCH_MACH_MSG_TYPE,
	.do_kind = "mach-msg",
	.do_dispose = _dispatch_mach_msg_dispose,
	.do_invoke = _dispatch_mach_msg_invoke,
	.do_debug = _dispatch_mach_msg_debug,
);

#if !USE_OBJC
DISPATCH_VTABLE_INSTANCE(data,
	.do_type = DISPATCH_DATA_TYPE,
	.do_kind = "data",
	.do_dispose = _dispatch_data_dispose,
	.do_debug = _dispatch_data_debug,
);
#endif

DISPATCH_VTABLE_INSTANCE(io,
	.do_type = DISPATCH_IO_TYPE,
	.do_kind = "channel",
	.do_dispose = _dispatch_io_dispose,
	.do_debug = _dispatch_io_debug,
);

DISPATCH_VTABLE_INSTANCE(operation,
	.do_type = DISPATCH_OPERATION_TYPE,
	.do_kind = "operation",
	.do_dispose = _dispatch_operation_dispose,
	.do_debug = _dispatch_operation_debug,
);

DISPATCH_VTABLE_INSTANCE(disk,
	.do_type = DISPATCH_DISK_TYPE,
	.do_kind = "disk",
	.do_dispose = _dispatch_disk_dispose,
);

void
_dispatch_vtable_init(void)
{
#if USE_OBJC
	// ObjC classes and dispatch vtables are co-located via linker order and
	// alias files, verify correct layout during initialization rdar://10640168
	DISPATCH_OBJC_CLASS_DECL(semaphore);
	dispatch_assert((char*)DISPATCH_VTABLE(semaphore) -
			(char*)DISPATCH_OBJC_CLASS(semaphore) == 0);
	dispatch_assert((char*)&DISPATCH_CONCAT(_,DISPATCH_CLASS(semaphore_vtable))
			- (char*)DISPATCH_OBJC_CLASS(semaphore) ==
			sizeof(_os_object_class_s));
#endif // USE_OBJC
}

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
#else
	/*
	 * XXXRW: What to do here for !Mac OS X?
	 */
	memset(_dispatch_build, 0, sizeof(_dispatch_build));
#endif
}

static dispatch_once_t _dispatch_build_pred;

char*
_dispatch_get_build(void)
{
	dispatch_once_f(&_dispatch_build_pred, NULL, _dispatch_build_init);
	return _dispatch_build;
}

#define _dispatch_bug_log(msg, ...) do { \
	static void *last_seen; \
	void *ra = __builtin_return_address(0); \
	if (last_seen != ra) { \
		last_seen = ra; \
		_dispatch_log(msg, ##__VA_ARGS__); \
	} \
} while(0)

void
_dispatch_bug(size_t line, long val)
{
	dispatch_once_f(&_dispatch_build_pred, NULL, _dispatch_build_init);
	_dispatch_bug_log("BUG in libdispatch: %s - %lu - 0x%lx",
			_dispatch_build, (unsigned long)line, val);
}

void
_dispatch_bug_client(const char* msg)
{
	_dispatch_bug_log("BUG in libdispatch client: %s", msg);
}

void
_dispatch_bug_mach_client(const char* msg, mach_msg_return_t kr)
{
	_dispatch_bug_log("BUG in libdispatch client: %s %s - 0x%x", msg,
			mach_error_string(kr), kr);
}

void
_dispatch_bug_kevent_client(const char* msg, const char* filter,
		const char *operation, int err)
{
	_dispatch_bug_log("BUG in libdispatch client: %s[%s] %s: \"%s\" - 0x%x",
			msg, filter, operation, strerror(err), err);
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
			dispatch_logfile = STDERR_FILENO;
		}
	}
	if (!dispatch_log_disabled) {
		if (log_to_file && dispatch_logfile == -1) {
			char path[PATH_MAX];
			snprintf(path, sizeof(path), "/var/tmp/libdispatch.%d.log",
					getpid());
			dispatch_logfile = open(path, O_WRONLY | O_APPEND | O_CREAT |
					O_NOFOLLOW | O_CLOEXEC, 0666);
		}
		if (dispatch_logfile != -1) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			dprintf(dispatch_logfile, "=== log file opened for %s[%u] at "
					"%ld.%06u ===\n", getprogname() ?: "", getpid(),
					tv.tv_sec, tv.tv_usec);
		}
	}
}

static inline void
_dispatch_log_file(char *buf, size_t len)
{
	ssize_t r;

	buf[len++] = '\n';
retry:
	r = write(dispatch_logfile, buf, len);
	if (slowpath(r == -1) && errno == EINTR) {
		goto retry;
	}
}

DISPATCH_NOINLINE
static void
_dispatch_logv_file(const char *msg, va_list ap)
{
	char buf[2048];
	int r = vsnprintf(buf, sizeof(buf), msg, ap);
	if (r < 0) return;
	size_t len = (size_t)r;
	if (len > sizeof(buf) - 1) {
		len = sizeof(buf) - 1;
	}
	_dispatch_log_file(buf, len);
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
	if (slowpath(dispatch_log_disabled)) {
		return;
	}
	if (slowpath(dispatch_logfile != -1)) {
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
	if (dou._do->do_vtable->do_debug) {
		return dx_debug(dou._do, buf, bufsiz);
	}
	return strlcpy(buf, "NULL vtable slot: ", bufsiz);
}

DISPATCH_NOINLINE
static void
_dispatch_debugv(dispatch_object_t dou, const char *msg, va_list ap)
{
	char buf[2048];
	int r;
	size_t offs;
	if (dou._do) {
		offs = _dispatch_object_debug2(dou, buf, sizeof(buf));
		dispatch_assert(offs + 2 < sizeof(buf));
		buf[offs++] = ':';
		buf[offs++] = ' ';
		buf[offs]   = '\0';
	} else {
		offs = strlcpy(buf, "NULL: ", sizeof(buf));
	}
	r = vsnprintf(buf + offs, sizeof(buf) - offs, msg, ap);
#if !DISPATCH_USE_OS_DEBUG_LOG
	size_t len = offs + (r < 0 ? 0 : (size_t)r);
	if (len > sizeof(buf) - 1) {
		len = sizeof(buf) - 1;
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
}

void *
_dispatch_calloc(size_t num_items, size_t size)
{
	void *buf;
	while (!fastpath(buf = calloc(num_items, size))) {
		_dispatch_temporary_resource_shortage();
	}
	return buf;
}

#pragma mark -
#pragma mark dispatch_block_t

#ifdef __BLOCKS__

#undef _dispatch_Block_copy
dispatch_block_t
_dispatch_Block_copy(dispatch_block_t db)
{
	dispatch_block_t rval;

	if (fastpath(db)) {
		while (!fastpath(rval = Block_copy(db))) {
			_dispatch_temporary_resource_shortage();
		}
		return rval;
	}
	DISPATCH_CLIENT_CRASH("NULL was passed where a block should have been");
}

void
_dispatch_call_block_and_release(void *block)
{
	void (^b)(void) = block;
	b();
	Block_release(b);
}

#pragma mark -
#pragma mark _dispatch_block_create no_objc

#if !USE_OBJC
struct dispatch_queue_attr_s _dispatch_queue_attr_concurrent;
#ifdef notyet
// The compiler hides the name of the function it generates, and changes it if
// we try to reference it directly, but the linker still sees it.
extern void DISPATCH_BLOCK_SPECIAL_INVOKE(void *)
		asm("____dispatch_block_create_block_invoke");
void (*_dispatch_block_special_invoke)(void*) = DISPATCH_BLOCK_SPECIAL_INVOKE;
#endif

dispatch_block_t
_dispatch_block_create(dispatch_block_flags_t flags, voucher_t voucher,
		pthread_priority_t pri, dispatch_block_t block)
{
	dispatch_block_t copy_block = _dispatch_Block_copy(block); // 17094902
	(void)voucher; // No voucher capture! (requires ObjC runtime)
	struct dispatch_block_private_data_s dbpds =
			DISPATCH_BLOCK_PRIVATE_DATA_INITIALIZER(flags, NULL, pri, copy_block);
	dispatch_block_t new_block = _dispatch_Block_copy(^{
		// Capture object references, which retains copy_block.
		// All retained objects must be captured by the *block*. We
		// cannot borrow any references, because the block might be
		// called zero or several times, so Block_release() is the
		// only place that can release retained objects.
		(void)copy_block;
		_dispatch_block_invoke(&dbpds);
	});
	Block_release(copy_block);
	return new_block;
}

#endif // !USE_OBJC

#endif // __BLOCKS__

#pragma mark -
#pragma mark dispatch_client_callout

// Abort on uncaught exceptions thrown from client callouts rdar://8577499
#if DISPATCH_USE_CLIENT_CALLOUT && (__USING_SJLJ_EXCEPTIONS__ || !USE_OBJC)
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
	if (fastpath(!u)) return f(ctxt);
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
	if (fastpath(!u)) return f(ctxt, i);
	_dispatch_set_unwind_tsd(NULL);
	f(ctxt, i);
	_dispatch_free_unwind_tsd();
	_dispatch_set_unwind_tsd(u);
}

#undef _dispatch_client_callout3
bool
_dispatch_client_callout3(void *ctxt, dispatch_data_t region, size_t offset,
		const void *buffer, size_t size, dispatch_data_applier_function_t f)
{
	_dispatch_get_tsd_base();
	void *u = _dispatch_get_unwind_tsd();
	if (fastpath(!u)) return f(ctxt, region, offset, buffer, size);
	_dispatch_set_unwind_tsd(NULL);
	bool res = f(ctxt, region, offset, buffer, size);
	_dispatch_free_unwind_tsd();
	_dispatch_set_unwind_tsd(u);
	return res;
}

#undef _dispatch_client_callout4
void
_dispatch_client_callout4(void *ctxt, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t dmsg, mach_error_t error,
		dispatch_mach_handler_function_t f)
{
	_dispatch_get_tsd_base();
	void *u = _dispatch_get_unwind_tsd();
	if (fastpath(!u)) return f(ctxt, reason, dmsg, error);
	_dispatch_set_unwind_tsd(NULL);
	f(ctxt, reason, dmsg, error);
	_dispatch_free_unwind_tsd();
	_dispatch_set_unwind_tsd(u);
}

#endif // DISPATCH_USE_CLIENT_CALLOUT

#pragma mark -
#pragma mark _os_object_t no_objc

#if !USE_OBJC

static const _os_object_class_s _os_object_class;

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
	while (!fastpath(obj = calloc(1u, size))) {
		_dispatch_temporary_resource_shortage();
	}
	obj->os_obj_isa = cls;
	return obj;
}

_os_object_t
_os_object_alloc(const void *cls, size_t size)
{
	if (!cls) cls = &_os_object_class;
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
	if (fastpath(obj->os_obj_isa->_os_obj_xref_dispose)) {
		return obj->os_obj_isa->_os_obj_xref_dispose(obj);
	}
	return _os_object_release_internal(obj);
}

void
_os_object_dispose(_os_object_t obj)
{
	if (fastpath(obj->os_obj_isa->_os_obj_dispose)) {
		return obj->os_obj_isa->_os_obj_dispose(obj);
	}
	return _os_object_dealloc(obj);
}

void*
os_retain(void *obj)
{
	if (fastpath(obj)) {
		return _os_object_retain(obj);
	}
	return obj;
}

#undef os_release
void
os_release(void *obj)
{
	if (fastpath(obj)) {
		return _os_object_release(obj);
	}
}

#pragma mark -
#pragma mark dispatch_autorelease_pool no_objc

#if DISPATCH_COCOA_COMPAT

void *_dispatch_autorelease_pool_push(void) {
	void *pool = NULL;
	if (_dispatch_begin_NSAutoReleasePool) {
		pool = _dispatch_begin_NSAutoReleasePool();
	}
	return pool;
}

void _dispatch_autorelease_pool_pop(void *pool) {
	if (_dispatch_end_NSAutoReleasePool) {
		_dispatch_end_NSAutoReleasePool(pool);
	}
}

#endif // DISPATCH_COCOA_COMPAT
#endif // !USE_OBJC

#pragma mark -
#pragma mark dispatch_source_types

static void
dispatch_source_type_timer_init(dispatch_source_t ds,
	dispatch_source_type_t type DISPATCH_UNUSED,
	uintptr_t handle DISPATCH_UNUSED,
	unsigned long mask,
	dispatch_queue_t q)
{
	if (fastpath(!ds->ds_refs)) {
		ds->ds_refs = _dispatch_calloc(1ul,
				sizeof(struct dispatch_timer_source_refs_s));
	}
	ds->ds_needs_rearm = true;
	ds->ds_is_timer = true;
	if (q == dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0)
			|| q == dispatch_get_global_queue(
			DISPATCH_QUEUE_PRIORITY_BACKGROUND, DISPATCH_QUEUE_OVERCOMMIT)){
		mask |= DISPATCH_TIMER_BACKGROUND; // <rdar://problem/12200216>
	}
	ds_timer(ds->ds_refs).flags = mask;
}

const struct dispatch_source_type_s _dispatch_source_type_timer = {
	.ke = {
		.filter = DISPATCH_EVFILT_TIMER,
	},
	.mask = DISPATCH_TIMER_STRICT|DISPATCH_TIMER_BACKGROUND|
			DISPATCH_TIMER_WALL_CLOCK,
	.init = dispatch_source_type_timer_init,
};

static void
dispatch_source_type_timer_with_aggregate_init(dispatch_source_t ds,
	dispatch_source_type_t type, uintptr_t handle, unsigned long mask,
	dispatch_queue_t q)
{
	ds->ds_refs = _dispatch_calloc(1ul,
			sizeof(struct dispatch_timer_source_aggregate_refs_s));
	dispatch_source_type_timer_init(ds, type, handle, mask, q);
	ds_timer(ds->ds_refs).flags |= DISPATCH_TIMER_WITH_AGGREGATE;
	ds->dq_specific_q = (void*)handle;
	_dispatch_retain(ds->dq_specific_q);
}

const struct dispatch_source_type_s _dispatch_source_type_timer_with_aggregate={
	.ke = {
		.filter = DISPATCH_EVFILT_TIMER,
		.ident = ~0ull,
	},
	.mask = DISPATCH_TIMER_STRICT|DISPATCH_TIMER_BACKGROUND,
	.init = dispatch_source_type_timer_with_aggregate_init,
};

static void
dispatch_source_type_interval_init(dispatch_source_t ds,
	dispatch_source_type_t type, uintptr_t handle, unsigned long mask,
	dispatch_queue_t q)
{
	dispatch_source_type_timer_init(ds, type, handle, mask, q);
	ds_timer(ds->ds_refs).flags |= DISPATCH_TIMER_INTERVAL;
	unsigned long ident = _dispatch_source_timer_idx(ds->ds_refs);
	ds->ds_dkev->dk_kevent.ident = ds->ds_ident_hack = ident;
	_dispatch_source_set_interval(ds, handle);
}

const struct dispatch_source_type_s _dispatch_source_type_interval = {
	.ke = {
		.filter = DISPATCH_EVFILT_TIMER,
		.ident = ~0ull,
	},
	.mask = DISPATCH_TIMER_STRICT|DISPATCH_TIMER_BACKGROUND|
			DISPATCH_INTERVAL_UI_ANIMATION,
	.init = dispatch_source_type_interval_init,
};

const struct dispatch_source_type_s _dispatch_source_type_read = {
	.ke = {
		.filter = EVFILT_READ,
		.flags = EV_DISPATCH,
	},
};

const struct dispatch_source_type_s _dispatch_source_type_write = {
	.ke = {
		.filter = EVFILT_WRITE,
		.flags = EV_DISPATCH,
	},
};

#if DISPATCH_USE_MEMORYSTATUS

#if TARGET_IPHONE_SIMULATOR // rdar://problem/9219483
static int _dispatch_ios_simulator_memory_warnings_fd = -1;
static void
_dispatch_ios_simulator_memorypressure_init(void *context DISPATCH_UNUSED)
{
	char *e = getenv("SIMULATOR_MEMORY_WARNINGS");
	if (!e) return;
	_dispatch_ios_simulator_memory_warnings_fd = open(e, O_EVTONLY);
	if (_dispatch_ios_simulator_memory_warnings_fd == -1) {
		(void)dispatch_assume_zero(errno);
	}
}
#endif

static void
dispatch_source_type_memorystatus_init(dispatch_source_t ds,
	dispatch_source_type_t type DISPATCH_UNUSED,
	uintptr_t handle DISPATCH_UNUSED,
	unsigned long mask DISPATCH_UNUSED,
	dispatch_queue_t q DISPATCH_UNUSED)
{
#if TARGET_IPHONE_SIMULATOR
	static dispatch_once_t pred;
	dispatch_once_f(&pred, NULL, _dispatch_ios_simulator_memorypressure_init);
	handle = (uintptr_t)_dispatch_ios_simulator_memory_warnings_fd;
	mask = NOTE_ATTRIB;
	ds->ds_dkev->dk_kevent.filter = EVFILT_VNODE;
	ds->ds_dkev->dk_kevent.ident = handle;
	ds->ds_dkev->dk_kevent.flags |= EV_CLEAR;
	ds->ds_dkev->dk_kevent.fflags = (uint32_t)mask;
	ds->ds_ident_hack = handle;
	ds->ds_pending_data_mask = mask;
	ds->ds_memorystatus_override = 1;
#endif
	ds->ds_is_level = false;
}

#ifndef NOTE_MEMORYSTATUS_LOW_SWAP
#define NOTE_MEMORYSTATUS_LOW_SWAP 0x8
#endif

const struct dispatch_source_type_s _dispatch_source_type_memorystatus = {
	.ke = {
		.filter = EVFILT_MEMORYSTATUS,
		.flags = EV_DISPATCH,
	},
	.mask = NOTE_MEMORYSTATUS_PRESSURE_NORMAL|NOTE_MEMORYSTATUS_PRESSURE_WARN
			|NOTE_MEMORYSTATUS_PRESSURE_CRITICAL|NOTE_MEMORYSTATUS_LOW_SWAP,
	.init = dispatch_source_type_memorystatus_init,
};

static void
dispatch_source_type_vm_init(dispatch_source_t ds,
	dispatch_source_type_t type,
	uintptr_t handle,
	unsigned long mask,
	dispatch_queue_t q)
{
	// Map legacy vm pressure to memorystatus warning rdar://problem/15907505
	mask = NOTE_MEMORYSTATUS_PRESSURE_WARN;
	ds->ds_dkev->dk_kevent.fflags = (uint32_t)mask;
	ds->ds_pending_data_mask = mask;
	ds->ds_vmpressure_override = 1;
	dispatch_source_type_memorystatus_init(ds, type, handle, mask, q);
}

const struct dispatch_source_type_s _dispatch_source_type_vm = {
	.ke = {
		.filter = EVFILT_MEMORYSTATUS,
		.flags = EV_DISPATCH,
	},
	.mask = NOTE_VM_PRESSURE,
	.init = dispatch_source_type_vm_init,
};

#elif DISPATCH_USE_VM_PRESSURE

static void
dispatch_source_type_vm_init(dispatch_source_t ds,
	dispatch_source_type_t type DISPATCH_UNUSED,
	uintptr_t handle DISPATCH_UNUSED,
	unsigned long mask DISPATCH_UNUSED,
	dispatch_queue_t q DISPATCH_UNUSED)
{
	ds->ds_is_level = false;
}

const struct dispatch_source_type_s _dispatch_source_type_vm = {
	.ke = {
		.filter = EVFILT_VM,
		.flags = EV_DISPATCH,
	},
	.mask = NOTE_VM_PRESSURE,
	.init = dispatch_source_type_vm_init,
};

#endif // DISPATCH_USE_VM_PRESSURE

const struct dispatch_source_type_s _dispatch_source_type_proc = {
	.ke = {
		.filter = EVFILT_PROC,
		.flags = EV_CLEAR,
	},
	.mask = NOTE_EXIT|NOTE_FORK|NOTE_EXEC
#if HAVE_DECL_NOTE_SIGNAL
			|NOTE_SIGNAL
#endif
#if HAVE_DECL_NOTE_REAP
			|NOTE_REAP
#endif
			,
};

const struct dispatch_source_type_s _dispatch_source_type_signal = {
	.ke = {
		.filter = EVFILT_SIGNAL,
	},
};

const struct dispatch_source_type_s _dispatch_source_type_vnode = {
	.ke = {
		.filter = EVFILT_VNODE,
		.flags = EV_CLEAR,
	},
	.mask = NOTE_DELETE|NOTE_WRITE|NOTE_EXTEND|NOTE_ATTRIB|NOTE_LINK|
			NOTE_RENAME|NOTE_REVOKE
#if HAVE_DECL_NOTE_NONE
			|NOTE_NONE
#endif
			,
};

const struct dispatch_source_type_s _dispatch_source_type_vfs = {
	.ke = {
		.filter = EVFILT_FS,
		.flags = EV_CLEAR,
	},
	.mask = VQ_NOTRESP|VQ_NEEDAUTH|VQ_LOWDISK|VQ_MOUNT|VQ_UNMOUNT|VQ_DEAD|
			VQ_ASSIST|VQ_NOTRESPLOCK
#if HAVE_DECL_VQ_UPDATE
			|VQ_UPDATE
#endif
#if HAVE_DECL_VQ_VERYLOWDISK
			|VQ_VERYLOWDISK
#endif
			,
};

const struct dispatch_source_type_s _dispatch_source_type_sock = {
#ifdef EVFILT_SOCK
	.ke = {
		.filter = EVFILT_SOCK,
		.flags = EV_CLEAR,
	},
	.mask = NOTE_CONNRESET |  NOTE_READCLOSED | NOTE_WRITECLOSED |
		NOTE_TIMEOUT | NOTE_NOSRCADDR |  NOTE_IFDENIED | NOTE_SUSPEND |
		NOTE_RESUME | NOTE_KEEPALIVE
#ifdef NOTE_ADAPTIVE_WTIMO
		| NOTE_ADAPTIVE_WTIMO | NOTE_ADAPTIVE_RTIMO
#endif
#ifdef NOTE_CONNECTED
		| NOTE_CONNECTED | NOTE_DISCONNECTED | NOTE_CONNINFO_UPDATED
#endif
		,
#endif // EVFILT_SOCK
};

const struct dispatch_source_type_s _dispatch_source_type_data_add = {
	.ke = {
		.filter = DISPATCH_EVFILT_CUSTOM_ADD,
	},
};

const struct dispatch_source_type_s _dispatch_source_type_data_or = {
	.ke = {
		.filter = DISPATCH_EVFILT_CUSTOM_OR,
		.flags = EV_CLEAR,
		.fflags = ~0u,
	},
};

#if HAVE_MACH

static void
dispatch_source_type_mach_send_init(dispatch_source_t ds,
	dispatch_source_type_t type DISPATCH_UNUSED,
	uintptr_t handle DISPATCH_UNUSED, unsigned long mask,
	dispatch_queue_t q DISPATCH_UNUSED)
{
	if (!mask) {
		// Preserve legacy behavior that (mask == 0) => DISPATCH_MACH_SEND_DEAD
		ds->ds_dkev->dk_kevent.fflags = DISPATCH_MACH_SEND_DEAD;
		ds->ds_pending_data_mask = DISPATCH_MACH_SEND_DEAD;
	}
}

const struct dispatch_source_type_s _dispatch_source_type_mach_send = {
	.ke = {
		.filter = DISPATCH_EVFILT_MACH_NOTIFICATION,
		.flags = EV_CLEAR,
	},
	.mask = DISPATCH_MACH_SEND_DEAD|DISPATCH_MACH_SEND_POSSIBLE,
	.init = dispatch_source_type_mach_send_init,
};

static void
dispatch_source_type_mach_recv_init(dispatch_source_t ds,
	dispatch_source_type_t type DISPATCH_UNUSED,
	uintptr_t handle DISPATCH_UNUSED,
	unsigned long mask DISPATCH_UNUSED,
	dispatch_queue_t q DISPATCH_UNUSED)
{
	ds->ds_is_level = false;
}

const struct dispatch_source_type_s _dispatch_source_type_mach_recv = {
	.ke = {
		.filter = EVFILT_MACHPORT,
		.flags = EV_DISPATCH,
		.fflags = DISPATCH_MACH_RECV_MESSAGE,
	},
	.init = dispatch_source_type_mach_recv_init,
};

#pragma mark -
#pragma mark dispatch_mig

void *
dispatch_mach_msg_get_context(mach_msg_header_t *msg)
{
	mach_msg_context_trailer_t *tp;
	void *context = NULL;

	tp = (mach_msg_context_trailer_t *)((uint64_t *)msg +
			round_msg(msg->msgh_size)/8);
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
	kern_return_t kr;
	// this function should never be called
	(void)dispatch_assume_zero(name);
	kr = mach_port_mod_refs(mach_task_self(), name, MACH_PORT_RIGHT_RECEIVE,-1);
	DISPATCH_VERIFY_MIG(kr);
	(void)dispatch_assume_zero(kr);
	return KERN_SUCCESS;
}

kern_return_t
_dispatch_mach_notify_no_senders(mach_port_t notify,
		mach_port_mscount_t mscnt DISPATCH_UNUSED)
{
	// this function should never be called
	(void)dispatch_assume_zero(notify);
	return KERN_SUCCESS;
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
