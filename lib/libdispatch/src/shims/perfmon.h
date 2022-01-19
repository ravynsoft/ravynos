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

#if DISPATCH_PERF_MON && !DISPATCH_INTROSPECTION

#if defined (USE_APPLE_TSD_OPTIMIZATIONS) && defined(SIMULATE_5491082) && \
		(defined(__i386__) || defined(__x86_64__))
#ifdef __LP64__
#define _dispatch_perfmon_workitem_inc() asm("incq %%gs:%0" : "+m" \
		(*(void **)(dispatch_bcounter_key * sizeof(void *) + \
		_PTHREAD_TSD_OFFSET)) :: "cc")
#define _dispatch_perfmon_workitem_dec() asm("decq %%gs:%0" : "+m" \
		(*(void **)(dispatch_bcounter_key * sizeof(void *) + \
		_PTHREAD_TSD_OFFSET)) :: "cc")
#else
#define _dispatch_perfmon_workitem_inc() asm("incl %%gs:%0" : "+m" \
		(*(void **)(dispatch_bcounter_key * sizeof(void *) + \
		_PTHREAD_TSD_OFFSET)) :: "cc")
#define _dispatch_perfmon_workitem_dec() asm("decl %%gs:%0" : "+m" \
		(*(void **)(dispatch_bcounter_key * sizeof(void *) + \
		_PTHREAD_TSD_OFFSET)) :: "cc")
#endif
#else /* !USE_APPLE_TSD_OPTIMIZATIONS */
static inline void
_dispatch_perfmon_workitem_inc(void)
{
	unsigned long cnt;
	cnt = (unsigned long)_dispatch_thread_getspecific(dispatch_bcounter_key);
	_dispatch_thread_setspecific(dispatch_bcounter_key, (void *)++cnt);
}
static inline void
_dispatch_perfmon_workitem_dec(void)
{
	unsigned long cnt;
	cnt = (unsigned long)_dispatch_thread_getspecific(dispatch_bcounter_key);
	_dispatch_thread_setspecific(dispatch_bcounter_key, (void *)--cnt);
}
#endif /* USE_APPLE_TSD_OPTIMIZATIONS */

#define _dispatch_perfmon_start() \
		uint64_t start = _dispatch_absolute_time()
#define _dispatch_perfmon_end() \
		_dispatch_queue_merge_stats(start)
#else

#define _dispatch_perfmon_workitem_inc()
#define _dispatch_perfmon_workitem_dec()
#define _dispatch_perfmon_start()
#define _dispatch_perfmon_end()

#endif // DISPATCH_PERF_MON

#endif
