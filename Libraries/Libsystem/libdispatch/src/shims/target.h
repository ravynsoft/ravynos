/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

// These are the portable dispatch version requirements macros, isolated from
// the rest of the C internal headers to be suitable for inclusion in MIG defs,
// asm, etc.

#ifndef __DISPATCH_SHIMS_TARGET__
#define __DISPATCH_SHIMS_TARGET__

#ifdef __APPLE__
#include <Availability.h>
#include <TargetConditionals.h>

#if TARGET_OS_OSX
#  define DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(x) \
		(__MAC_OS_X_VERSION_MIN_REQUIRED >= (x))
#  if !DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(101200)
#    error "OS X hosts older than OS X 10.12 aren't supported anymore"
#  endif // !DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(101200)
#elif TARGET_OS_SIMULATOR
#  define DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(x) \
		(IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED >= (x))
#  if !DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(101200)
#    error "Simulator hosts older than OS X 10.12 aren't supported anymore"
#  endif // !DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(101200)
#else
#  define DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(x) 1
#  if !TARGET_OS_DRIVERKIT && __IPHONE_OS_VERSION_MIN_REQUIRED < 90000
#    error "iOS hosts older than iOS 9.0 aren't supported anymore"
#  endif
#endif

#else // !__APPLE__
#define DISPATCH_MIN_REQUIRED_OSX_AT_LEAST(x) 0
#endif // !__APPLE__

#endif // __DISPATCH_SHIMS_TARGET__
