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

#ifndef __DISPATCH_SHIMS_PERFMON__
#define __DISPATCH_SHIMS_PERFMON__

#if DISPATCH_PERF_MON
#if DISPATCH_INTROSPECTION
#error invalid configuration
#endif

typedef enum {
	perfmon_thread_no_trace = 0,
	perfmon_thread_event_no_steal,	// 1) Event threads that couldn't steal
	perfmon_thread_event_steal,		// 2) Event threads failing to steal very late
	perfmon_thread_worker_non_oc,	// 3) Non overcommit threads finding
									//		nothing on the root queues
	perfmon_thread_worker_oc,		// 4) Overcommit thread finding nothing to do
	perfmon_thread_manager,
} perfmon_thread_type;

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_perfmon_workitem_inc(void)
{
	unsigned long cnt;
	cnt = (unsigned long)_dispatch_thread_getspecific(dispatch_bcounter_key);
	_dispatch_thread_setspecific(dispatch_bcounter_key, (void *)++cnt);
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_perfmon_workitem_dec(void)
{
	unsigned long cnt;
	cnt = (unsigned long)_dispatch_thread_getspecific(dispatch_bcounter_key);
	_dispatch_thread_setspecific(dispatch_bcounter_key, (void *)--cnt);
}

#define DISPATCH_PERF_MON_ARGS_PROTO  , uint64_t perfmon_start
#define DISPATCH_PERF_MON_ARGS        , perfmon_start
#define DISPATCH_PERF_MON_VAR         uint64_t perfmon_start;
#define DISPATCH_PERF_MON_VAR_INIT    uint64_t perfmon_start = 0;

#define _dispatch_perfmon_start_impl(trace) ({ \
		if (trace) _dispatch_ktrace0(DISPATCH_PERF_MON_worker_thread_start); \
		perfmon_start = _dispatch_uptime(); \
	})
#define _dispatch_perfmon_start() \
		DISPATCH_PERF_MON_VAR _dispatch_perfmon_start_impl(true)
#define _dispatch_perfmon_start_notrace() \
		DISPATCH_PERF_MON_VAR _dispatch_perfmon_start_impl(false)
#define _dispatch_perfmon_end(thread_type) \
		_dispatch_queue_merge_stats(perfmon_start, true, thread_type)
#define _dispatch_perfmon_end_notrace() \
		_dispatch_queue_merge_stats(perfmon_start, false, perfmon_thread_no_trace)

void _dispatch_queue_merge_stats(uint64_t start, bool trace, perfmon_thread_type type);

#else

#define DISPATCH_PERF_MON_ARGS_PROTO
#define DISPATCH_PERF_MON_ARGS
#define DISPATCH_PERF_MON_VAR
#define DISPATCH_PERF_MON_VAR_INIT
#define _dispatch_perfmon_workitem_inc()
#define _dispatch_perfmon_workitem_dec()
#define _dispatch_perfmon_start_impl(trace)
#define _dispatch_perfmon_start()
#define _dispatch_perfmon_end(thread_type)
#define _dispatch_perfmon_start_notrace()
#define _dispatch_perfmon_end_notrace()

#endif // DISPATCH_PERF_MON

#endif
