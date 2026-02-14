/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef __OS_INTERNAL_H__
#define __OS_INTERNAL_H__

#define __OS_ALLOC_INDIRECT__

#include <TargetConditionals.h>
#include <machine/cpu_capabilities.h>

#include "os/base_private.h"
#include "os/semaphore_private.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#if defined(__arm__)
#include <arm/arch.h>
#endif
#include <mach/thread_switch.h>

#define likely(x) os_likely(x)
#define unlikely(x) os_unlikely(x)

#define __OS_CRASH__(rc, msg)  ({ \
		_os_set_crash_log_cause_and_message(rc, msg); \
		os_prevent_tail_call_optimization(); \
		__builtin_trap(); \
	})

#define __LIBPLATFORM_CLIENT_CRASH__(rc, msg) \
		__OS_CRASH__(rc, "BUG IN CLIENT OF LIBPLATFORM: " msg)
#define __LIBPLATFORM_INTERNAL_CRASH__(rc, msg) \
		__OS_CRASH__(rc, "BUG IN LIBPLATFORM: " msg)

#define __OS_EXPOSE_INTERNALS__ 1
#include "os/internal/internal_shared.h"
#include "yield.h"

#define OS_NOEXPORT extern __attribute__((__visibility__("hidden")))

#define OS_VARIANT(f, v) OS_CONCAT(f, OS_CONCAT($VARIANT$, v))

#define _OS_ATOMIC_ALIAS_PRIVATE_EXTERN(n)
#define OS_ATOMIC_EXPORT OS_EXPORT
#define _OS_ATOMIC_ALIAS_GLOBL(n) \
		".globl _" OS_STRINGIFY(n) "\n\t"
#ifdef __thumb__
#define _OS_ATOMIC_ALIAS_THUMB(n) \
		".thumb_func _" OS_STRINGIFY(n) "\n\t"
#else
#define _OS_ATOMIC_ALIAS_THUMB(n)
#endif
#define _OS_ATOMIC_ALIAS_SET(n, o) \
		".set _" OS_STRINGIFY(n) ", _" OS_STRINGIFY(o)

#define OS_ATOMIC_ALIAS(n, o) __asm__( \
		_OS_ATOMIC_ALIAS_PRIVATE_EXTERN(n) \
		_OS_ATOMIC_ALIAS_GLOBL(n) \
		_OS_ATOMIC_ALIAS_THUMB(n) \
		_OS_ATOMIC_ALIAS_SET(n, o))

#define OS_ATOMIC_EXPORT_ALIAS(n, o) __asm__( \
		_OS_ATOMIC_ALIAS_GLOBL(n) \
		_OS_ATOMIC_ALIAS_THUMB(n) \
		_OS_ATOMIC_ALIAS_SET(n, o))

#define _OS_VARIANT_RESOLVER(s, v, ...) \
	__attribute__((visibility(OS_STRINGIFY(v)))) extern void* s(void); \
	void* s(void) { \
	__asm__(".symbol_resolver _" OS_STRINGIFY(s)); \
		__VA_ARGS__ \
	}

#endif // __OS_INTERNAL_H__
