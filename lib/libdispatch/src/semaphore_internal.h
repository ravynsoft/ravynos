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

#ifndef __DISPATCH_SEMAPHORE_INTERNAL__
#define __DISPATCH_SEMAPHORE_INTERNAL__

struct dispatch_queue_s;

DISPATCH_CLASS_DECL(semaphore);
struct dispatch_semaphore_s {
	DISPATCH_STRUCT_HEADER(semaphore);
#if USE_MACH_SEM
	semaphore_t dsema_port;
#elif USE_POSIX_SEM
	sem_t dsema_sem;
#elif USE_WIN32_SEM
	HANDLE dsema_handle;
#else
#error "No supported semaphore type"
#endif
	long dsema_orig;
	long volatile dsema_value;
	union {
		long volatile dsema_sent_ksignals;
		long volatile dsema_group_waiters;
	};
	struct dispatch_continuation_s *volatile dsema_notify_head;
	struct dispatch_continuation_s *volatile dsema_notify_tail;
};

DISPATCH_CLASS_DECL(group);

#define DISPATCH_GROUP_INITIALIZER(s) \
		{ \
			.do_vtable = (const void*)DISPATCH_VTABLE(group), \
			.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT, \
			.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT, \
			.dsema_value = LONG_MAX - (s), \
			.dsema_orig = LONG_MAX, \
		}

void _dispatch_semaphore_dispose(dispatch_object_t dou);
size_t _dispatch_semaphore_debug(dispatch_object_t dou, char *buf,
		size_t bufsiz);

typedef uintptr_t _dispatch_thread_semaphore_t;

_dispatch_thread_semaphore_t _dispatch_thread_semaphore_create(void);
void _dispatch_thread_semaphore_dispose(_dispatch_thread_semaphore_t);
void _dispatch_thread_semaphore_wait(_dispatch_thread_semaphore_t);
void _dispatch_thread_semaphore_signal(_dispatch_thread_semaphore_t);

DISPATCH_ALWAYS_INLINE
static inline _dispatch_thread_semaphore_t
_dispatch_get_thread_semaphore(void)
{
	_dispatch_thread_semaphore_t sema = (_dispatch_thread_semaphore_t)
			_dispatch_thread_getspecific(dispatch_sema4_key);
	if (slowpath(!sema)) {
		return _dispatch_thread_semaphore_create();
	}
	_dispatch_thread_setspecific(dispatch_sema4_key, NULL);
	return sema;
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_put_thread_semaphore(_dispatch_thread_semaphore_t sema)
{
	_dispatch_thread_semaphore_t old_sema = (_dispatch_thread_semaphore_t)
			_dispatch_thread_getspecific(dispatch_sema4_key);
	_dispatch_thread_setspecific(dispatch_sema4_key, (void*)sema);
	if (slowpath(old_sema)) {
		return _dispatch_thread_semaphore_dispose(old_sema);
	}
}

#endif
