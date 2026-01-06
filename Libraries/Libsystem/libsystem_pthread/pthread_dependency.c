/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#include "resolver.h"
#include "internal.h"
#include "dependency_private.h"
#include <sys/ulock.h>

#define PREREQUISITE_FULFILLED  (~0u)

PTHREAD_NOEXPORT
void _pthread_dependency_fulfill_slow(pthread_dependency_t *pr, uint32_t old);

OS_ALWAYS_INLINE
static inline mach_port_t
_pthread_dependency_self(void)
{
	void *v = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_MACH_THREAD_SELF);
	return (mach_port_t)(uintptr_t)v;
}

void
pthread_dependency_init_np(pthread_dependency_t *pr, pthread_t pth,
		pthread_dependency_attr_t *attrs)
{
	if (attrs) *(volatile char *)attrs;
	*pr = (pthread_dependency_t)PTHREAD_DEPENDENCY_INITIALIZER_NP(pth);
}

OS_NOINLINE
void
_pthread_dependency_fulfill_slow(pthread_dependency_t *pr, uint32_t old)
{
	if (old == PREREQUISITE_FULFILLED) {
		PTHREAD_CLIENT_CRASH(0, "Fufilling pthread_dependency_t twice");
	}
	if (os_unlikely(old != _pthread_dependency_self())) {
		PTHREAD_CLIENT_CRASH(old, "Fulfilled a dependency "
				"not owned by current thread");
	}

	int ret = __ulock_wake(UL_UNFAIR_LOCK | ULF_NO_ERRNO, &pr->__pdep_opaque1, 0);
	switch (-ret) {
	case 0:
	case ENOENT:
		return;
	default:
		PTHREAD_INTERNAL_CRASH(-ret, "__ulock_wake() failed");
	}
}


void
pthread_dependency_fulfill_np(pthread_dependency_t *pr, void *value)
{
	uint32_t old;

	pr->__pdep_opaque2 = (uint64_t)(uintptr_t)value;
	old = os_atomic_xchg(&pr->__pdep_opaque1, PREREQUISITE_FULFILLED, release);

	if (old != 0) _pthread_dependency_fulfill_slow(pr, old);
}

void *
pthread_dependency_wait_np(pthread_dependency_t *pr)
{
	if (os_atomic_cmpxchg(&pr->__pdep_opaque1, 0, pr->__pdep_owner, relaxed)) {
		int ret;
	again:
		ret = __ulock_wait(UL_UNFAIR_LOCK | ULF_NO_ERRNO, &pr->__pdep_opaque1,
				pr->__pdep_owner, 0);
		switch (-ret) {
		case EFAULT:
		case EINTR:
		case 0:
			if (pr->__pdep_opaque1 == pr->__pdep_owner) goto again;
			break;
		case EOWNERDEAD:
			PTHREAD_CLIENT_CRASH(pr->__pdep_owner, "Waiting on orphaned dependency");
		default:
			PTHREAD_CLIENT_CRASH(-ret, "__ulock_wait() failed");
		}
	}

	uint32_t cur = os_atomic_load(&pr->__pdep_opaque1, acquire);
	if (cur == PREREQUISITE_FULFILLED) {
		return (void *)(uintptr_t)pr->__pdep_opaque2;
	}
	PTHREAD_CLIENT_CRASH(cur, "Corrupted pthread_dependency_t");
}

