/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __PTHREAD_PRIVATE_H__
#define __PTHREAD_PRIVATE_H__

#include <sys/cdefs.h>
#include <Availability.h>
#include <pthread/tsd_private.h>

/* get the thread specific errno value */
__header_always_inline int
_pthread_get_errno_direct(void)
{
	return *(int*)_pthread_getspecific_direct(_PTHREAD_TSD_SLOT_ERRNO);
}

/* set the thread specific errno value */
__header_always_inline void
_pthread_set_errno_direct(int value)
{
	*((int*)_pthread_getspecific_direct(_PTHREAD_TSD_SLOT_ERRNO)) = value;
}

__API_AVAILABLE(macos(10.9), ios(7.0))
pthread_t pthread_main_thread_np(void);

struct _libpthread_functions {
	unsigned long version;
	void (*exit)(int); // added with version=1
	void *(*malloc)(size_t); // added with version=2
	void (*free)(void *); // added with version=2
};

/*!
 * @function pthread_chdir_np
 *
 * @abstract
 * Sets the per-thread current working directory.
 *
 * @discussion
 * This will set the per-thread current working directory to the provided path.
 * If this is used on a workqueue (dispatch) thread, it MUST be unset with
 * pthread_fchdir_np(-1) before returning.
 *
 * posix_spawn_file_actions_addchdir_np is a better approach if this call would
 * only be used to spawn a new process with a given working directory.
 *
 * @param path
 * The path of the new working directory.
 *
 * @result
 * 0 upon success, -1 upon error and errno is set.
 */
__API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
int pthread_chdir_np(const char *path);

/*!
 * @function pthread_fchdir_np
 *
 * @abstract
 * Sets the per-thread current working directory.
 *
 * @discussion
 * This will set the per-thread current working directory to the provided
 * directory fd.  If this is used on a workqueue (dispatch) thread, it MUST be
 * unset with pthread_fchdir_np(-1) before returning.
 *
 * posix_spawn_file_actions_addfchdir_np is a better approach if this call would
 * only be used to spawn a new process with a given working directory.
 *
 * @param fd
 * A file descriptor to the new working directory.  Pass -1 to unset the
 * per-thread working directory.
 *
 * @result
 * 0 upon success, -1 upon error and errno is set.
 */
__API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
int pthread_fchdir_np(int fd);

__API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
int pthread_attr_setcpupercent_np(pthread_attr_t * __restrict, int, unsigned long);

__API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0))
int pthread_current_stack_contains_np(const void *, size_t);

#ifdef _os_tsd_get_base

#ifdef __LP64__
#define _PTHREAD_STRUCT_DIRECT_THREADID_OFFSET -8
#else
#define _PTHREAD_STRUCT_DIRECT_THREADID_OFFSET -16
#endif

/* N.B. DO NOT USE UNLESS YOU ARE REBUILT AS PART OF AN OS TRAIN WORLDBUILD */
__header_always_inline uint64_t
_pthread_threadid_self_np_direct(void)
{
#ifndef __i386__
	if (_pthread_has_direct_tsd()) {
#ifdef OS_GS_RELATIVE
		return *(uint64_t OS_GS_RELATIVE *)(_PTHREAD_STRUCT_DIRECT_THREADID_OFFSET);
#else
		return *(uint64_t*)((char *)_os_tsd_get_base() + _PTHREAD_STRUCT_DIRECT_THREADID_OFFSET);
#endif
	}
#endif
	uint64_t threadid = 0;
	pthread_threadid_np(NULL, &threadid);
	return threadid;
}

#endif // _os_tsd_get_base

#endif // __PTHREAD_PRIVATE_H__
