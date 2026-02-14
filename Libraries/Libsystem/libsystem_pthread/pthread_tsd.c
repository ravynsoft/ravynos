/*
 * Copyright (c) 2000-2003, 2007, 2012 Apple Inc. All rights reserved.
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
/*
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991
 *              All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*
 * MkLinux
 */

/*
 * POSIX Pthread Library
 *   Thread Specific Data support
 *   NB: pthread_getspecific() is in a separate assembly file
 */

#include "internal.h"
#include <TargetConditionals.h>

#ifndef PTHREAD_KEY_LEGACY_SUPPORT
#if TARGET_OS_DRIVERKIT
#define PTHREAD_KEY_LEGACY_SUPPORT 0
#else
#define PTHREAD_KEY_LEGACY_SUPPORT 1
#endif // TARGET_OS_DRIVERKIT
#endif // PTHREAD_KEY_LEGACY_SUPPORT

#if !VARIANT_DYLD
// __pthread_tsd_first is first static key managed by libpthread.
// __pthread_tsd_max is the (observed) end of static key destructors.
// __pthread_tsd_start is the start of dynamic keys.
// __pthread_tsd_end is the end of dynamic keys.

static const int __pthread_tsd_first = __TSD_RESERVED_MAX + 1;
static const int __pthread_tsd_start = _INTERNAL_POSIX_THREAD_KEYS_MAX;
static const int __pthread_tsd_end = _INTERNAL_POSIX_THREAD_KEYS_END;

static int __pthread_tsd_max = __pthread_tsd_first;
static _pthread_lock __pthread_tsd_lock = _PTHREAD_LOCK_INITIALIZER;
#if PTHREAD_KEY_LEGACY_SUPPORT
static bool __pthread_key_legacy_behaviour = 0;
static bool __pthread_key_legacy_behaviour_log = 0;
#else
#define __pthread_key_legacy_behaviour 0
#define _pthread_tsd_cleanup_legacy(...)
#endif // PTHREAD_KEY_LEGACY_SUPPORT

// Omit support for pthread key destructors in the static archive for dyld.
// dyld does not create and destroy threads so these are not necessary.
//
// We store the bit-wise negation of the destructor so that a quick non-zero
// test can be used to determine if the destructor has been set, even if it is
// NULL. This means that a destructor of value ~0x0ull cannot be used. That
// shouldn't be a problem in practice since it isn't a valid function address.

static struct {
	uintptr_t destructor;
} _pthread_keys[_INTERNAL_POSIX_THREAD_KEYS_END];

// The pthread_tsd destruction order can be reverted to the old (pre-10.11) order
// by setting this environment variable.
void
_pthread_key_global_init(const char *envp[])
{
#if PTHREAD_KEY_LEGACY_SUPPORT
	if (_simple_getenv(envp, "PTHREAD_KEY_LEGACY_DESTRUCTOR_ORDER")) {
		__pthread_key_legacy_behaviour = true;
	}
	if (_simple_getenv(envp, "PTHREAD_KEY_LEGACY_DESTRUCTOR_ORDER_LOG")) {
		__pthread_key_legacy_behaviour_log = true;
	}
#endif // PTHREAD_KEY_LEGACY_SUPPORT
}

// Returns true if successful, false if destructor was already set.
static bool
_pthread_key_set_destructor(pthread_key_t key, void (*destructor)(void *))
{
	uintptr_t *ptr = &_pthread_keys[key].destructor;
	uintptr_t value = ~(uintptr_t)destructor;
	if (*ptr == 0) {
		*ptr = value;
		return true;
	}
	return false;
}

// Returns true if successful, false if the destructor was not set.
static bool
_pthread_key_unset_destructor(pthread_key_t key)
{
	uintptr_t *ptr = &_pthread_keys[key].destructor;
	if (*ptr != 0) {
		*ptr = 0;
		return true;
	}
	return false;
}

// Returns true if successful, false if the destructor was not set.
static bool
_pthread_key_get_destructor(pthread_key_t key, void (**destructor)(void *))
{
	uintptr_t value = _pthread_keys[key].destructor;
	if (destructor) {
		*destructor = (void (*)(void *))(~value);
	}
	return (value != 0);
}

int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	int res = EAGAIN; // Returns EAGAIN if key cannot be allocated.
	pthread_key_t k;

	_PTHREAD_LOCK(__pthread_tsd_lock);
	for (k = __pthread_tsd_start; k < __pthread_tsd_end; k++) {
		if (_pthread_key_set_destructor(k, destructor)) {
			*key = k;
			res = 0;
			break;
		}
	}
	_PTHREAD_UNLOCK(__pthread_tsd_lock);

	return res;
}

int
pthread_key_delete(pthread_key_t key)
{
	int res = EINVAL; // Returns EINVAL if key is not allocated.

	_PTHREAD_LOCK(__pthread_tsd_lock);
	if (key >= __pthread_tsd_start && key < __pthread_tsd_end) {
		if (_pthread_key_unset_destructor(key)) {
			struct _pthread *p;
			_PTHREAD_LOCK(_pthread_list_lock);
			TAILQ_FOREACH(p, &__pthread_head, tl_plist) {
				// No lock for word-sized write.
				p->tsd[key] = 0;
			}
			_PTHREAD_UNLOCK(_pthread_list_lock);
			res = 0;
		}
	}
	_PTHREAD_UNLOCK(__pthread_tsd_lock);

	return res;
}
#endif // !VARIANT_DYLD

int
pthread_setspecific(pthread_key_t key, const void *value)
{
	int res = EINVAL;

#if !VARIANT_DYLD
	if (key >= __pthread_tsd_first && key < __pthread_tsd_end) {
		bool created = _pthread_key_get_destructor(key, NULL);
		if (key < __pthread_tsd_start || created) {
			struct _pthread *self = pthread_self();
			self->tsd[key] = (void *)value;
			res = 0;

			if (key < __pthread_tsd_start) {
				// XXX: is this really necessary?
				_pthread_key_set_destructor(key, NULL);
			}
			if (key > self->max_tsd_key) {
				self->max_tsd_key = (uint16_t)key;
			}
		}
	}
#endif // !VARIANT_DYLD

	return res;
}

int
_pthread_setspecific_static(pthread_key_t key, void *value)
{
	int res = EINVAL;

#if !VARIANT_DYLD
	if (key < __pthread_tsd_start) {
		_pthread_setspecific_direct(key, value);
		res = 0;
	}
#endif // !VARIANT_DYLD

	return res;
}

void*
pthread_getspecific(pthread_key_t key)
{
	return _pthread_getspecific_direct(key);
}

#if !VARIANT_DYLD
static void
_pthread_tsd_cleanup_key(pthread_t self, pthread_key_t key)
{
	void (*destructor)(void *);
	if (_pthread_key_get_destructor(key, &destructor)) {
		void **ptr = &self->tsd[key];
		void *value = *ptr;
		if (value) {
			*ptr = NULL;
			if (destructor) {
				destructor(value);
			}
		}
	}
}
#endif // !VARIANT_DYLD

#if !VARIANT_DYLD
static void
_pthread_tsd_cleanup_new(pthread_t self)
{
	int j;

	// clean up all keys
	for (j = 0; j < PTHREAD_DESTRUCTOR_ITERATIONS; j++) {
		pthread_key_t k;
		for (k = __pthread_tsd_start; k <= self->max_tsd_key; k++) {
			_pthread_tsd_cleanup_key(self, k);
		}

		for (k = __pthread_tsd_first; k <= __pthread_tsd_max; k++) {
			_pthread_tsd_cleanup_key(self, k);
		}
	}

	self->max_tsd_key = 0;
}

#if PTHREAD_KEY_LEGACY_SUPPORT
#import <_simple.h>
#import <dlfcn.h>
static void
_pthread_tsd_behaviour_check(pthread_t self)
{
	// Iterate from dynamic-key start to dynamic-key end, if the key has both
	// a desctructor and a value then _pthread_tsd_cleanup_key would cause
	// us to re-trigger the destructor.
	Dl_info i;
	pthread_key_t k;

	for (k = __pthread_tsd_start; k < __pthread_tsd_end; k++) {
		void (*destructor)(void *);
		if (_pthread_key_get_destructor(k, &destructor)) {
			void **ptr = &self->tsd[k];
			void *value = *ptr;
			if (value && destructor) {
				_simple_asl_log(ASL_LEVEL_ERR, "pthread_tsd",
						"warning: dynamic tsd keys dirty after static key cleanup loop.");

				if (dladdr(destructor, &i) == 0) {
					_simple_asl_log(ASL_LEVEL_ERR, "pthread_tsd", i.dli_fname);
					_simple_asl_log(ASL_LEVEL_ERR, "pthread_tsd", i.dli_saddr);
				}
			}
		}
	}

}

static void
_pthread_tsd_cleanup_legacy(pthread_t self)
{
	int j;

	// clean up dynamic keys first
	for (j = 0; j < PTHREAD_DESTRUCTOR_ITERATIONS; j++) {
		pthread_key_t k;
		for (k = __pthread_tsd_start; k <= self->max_tsd_key; k++) {
			_pthread_tsd_cleanup_key(self, k);
		}
	}

	self->max_tsd_key = 0;

	// clean up static keys
	for (j = 0; j < PTHREAD_DESTRUCTOR_ITERATIONS; j++) {
		pthread_key_t k;
		for (k = __pthread_tsd_first; k <= __pthread_tsd_max; k++) {
			_pthread_tsd_cleanup_key(self, k);
		}

		if (__pthread_key_legacy_behaviour_log != 0 && self->max_tsd_key != 0) {
			// max_tsd_key got dirtied, either by static or dynamic keys being
			// reset. check for any dirty dynamic keys.
			_pthread_tsd_behaviour_check(self);
		}
	}
}
#endif // PTHREAD_KEY_LEGACY_SUPPORT
#endif // !VARIANT_DYLD

void
_pthread_tsd_cleanup(pthread_t self)
{
#if !VARIANT_DYLD

	// unless __pthread_key_legacy_behaviour == 1, use the new pthread key
	// destructor order: (dynamic -> static) x5 -> (GC x5)

	if (__pthread_key_legacy_behaviour == 0) {
		_pthread_tsd_cleanup_new(self);
	} else {
		_pthread_tsd_cleanup_legacy(self);
	}
#endif // !VARIANT_DYLD
}

#if !VARIANT_DYLD
// XXX: key should be pthread_key_t
int
pthread_key_init_np(int key, void (*destructor)(void *))
{
	int res = EINVAL; // Returns EINVAL if key is out of range.
	if (key >= __pthread_tsd_first && key < __pthread_tsd_start) {
		_PTHREAD_LOCK(__pthread_tsd_lock);
		_pthread_key_set_destructor(key, destructor);
		if (key > __pthread_tsd_max) {
			__pthread_tsd_max = key;
		}
		_PTHREAD_UNLOCK(__pthread_tsd_lock);
		res = 0;
	}
	return res;
}
#endif // !VARIANT_DYLD

#undef pthread_self
pthread_t
pthread_self(void)
{
	pthread_t self = _pthread_self_direct();
	_pthread_validate_signature(self);
	return self;
}
