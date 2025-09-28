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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_SHIMS_YIELD__
#define __DISPATCH_SHIMS_YIELD__

#pragma mark -
#pragma mark _dispatch_wait_until

#if DISPATCH_HW_CONFIG_UP
#define _dispatch_wait_until(c) do { \
		int _spins = 0; \
		while (!(c)) { \
			_spins++; \
			_dispatch_preemption_yield(_spins); \
		} } while (0)
#elif TARGET_OS_EMBEDDED
// <rdar://problem/15440575>
#ifndef DISPATCH_WAIT_SPINS
#define DISPATCH_WAIT_SPINS 1024
#endif
#define _dispatch_wait_until(c) do { \
		int _spins = -(DISPATCH_WAIT_SPINS); \
		while (!(c)) { \
			if (slowpath(_spins++ >= 0)) { \
				_dispatch_preemption_yield(_spins); \
			} else { \
				dispatch_hardware_pause(); \
			} \
		} } while (0)
#else
#define _dispatch_wait_until(c) do { \
		while (!(c)) { \
			dispatch_hardware_pause(); \
		} } while (0)
#endif

#pragma mark -
#pragma mark _dispatch_contention_wait_until

#if DISPATCH_HW_CONFIG_UP
#define _dispatch_contention_wait_until(c) false
#else
#ifndef DISPATCH_CONTENTION_SPINS_MAX
#define DISPATCH_CONTENTION_SPINS_MAX (128 - 1)
#endif
#ifndef DISPATCH_CONTENTION_SPINS_MIN
#define DISPATCH_CONTENTION_SPINS_MIN (32 - 1)
#endif
#if TARGET_OS_EMBEDDED
#define _dispatch_contention_spins() \
		((DISPATCH_CONTENTION_SPINS_MIN) + ((DISPATCH_CONTENTION_SPINS_MAX) - \
		(DISPATCH_CONTENTION_SPINS_MIN)) / 2)
#else
// Use randomness to prevent threads from resonating at the same
// frequency and permanently contending. All threads sharing the same
// seed value is safe with the FreeBSD rand_r implementation.
#define _dispatch_contention_spins() ({ \
		static unsigned int _seed; \
		((unsigned int)rand_r(&_seed) & (DISPATCH_CONTENTION_SPINS_MAX)) | \
				(DISPATCH_CONTENTION_SPINS_MIN); })
#endif
#define _dispatch_contention_wait_until(c) ({ \
		bool _out = false; \
		unsigned int _spins = _dispatch_contention_spins(); \
		while (_spins--) { \
			dispatch_hardware_pause(); \
			if ((_out = fastpath(c))) break; \
		}; _out; })
#endif

#pragma mark -
#pragma mark dispatch_hardware_pause

#if defined(__x86_64__) || defined(__i386__)
#define dispatch_hardware_pause() __asm__("pause")
#elif (defined(__arm__) && defined(_ARM_ARCH_7) && defined(__thumb__)) || \
		defined(__arm64__)
#define dispatch_hardware_pause() __asm__("yield")
#define dispatch_hardware_wfe()   __asm__("wfe")
#else
#define dispatch_hardware_pause() __asm__("")
#endif

#pragma mark -
#pragma mark _dispatch_preemption_yield

#if HAVE_MACH
#if defined(SWITCH_OPTION_OSLOCK_DEPRESS) && !(TARGET_IPHONE_SIMULATOR && \
		IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED < 1090)
#define DISPATCH_YIELD_THREAD_SWITCH_OPTION SWITCH_OPTION_OSLOCK_DEPRESS
#else
#define DISPATCH_YIELD_THREAD_SWITCH_OPTION SWITCH_OPTION_DEPRESS
#endif
#define _dispatch_preemption_yield(n) _dispatch_thread_switch(MACH_PORT_NULL, \
		DISPATCH_YIELD_THREAD_SWITCH_OPTION, (mach_msg_timeout_t)(n))
#else
#define _dispatch_preemption_yield(n) pthread_yield_np()
#endif // HAVE_MACH

#pragma mark -
#pragma mark _dispatch_contention_usleep

#ifndef DISPATCH_CONTENTION_USLEEP_START
#define DISPATCH_CONTENTION_USLEEP_START 500
#endif
#ifndef DISPATCH_CONTENTION_USLEEP_MAX
#define DISPATCH_CONTENTION_USLEEP_MAX 100000
#endif

#if HAVE_MACH
#if defined(SWITCH_OPTION_DISPATCH_CONTENTION) && !(TARGET_IPHONE_SIMULATOR && \
		IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED < 1090)
#define _dispatch_contention_usleep(u) _dispatch_thread_switch(MACH_PORT_NULL, \
		SWITCH_OPTION_DISPATCH_CONTENTION, (u))
#else
#define _dispatch_contention_usleep(u) _dispatch_thread_switch(MACH_PORT_NULL, \
		SWITCH_OPTION_WAIT, (((u)-1)/1000)+1)
#endif
#else
#define _dispatch_contention_usleep(u) usleep((u))
#endif // HAVE_MACH

#pragma mark -
#pragma mark _dispatch_thread_switch

#if HAVE_MACH
#define _dispatch_thread_switch(thread_name, option, option_time) \
		thread_switch((thread_name), (option), (option_time))

#endif // HAVE_MACH

#endif // __DISPATCH_SHIMS_YIELD__
