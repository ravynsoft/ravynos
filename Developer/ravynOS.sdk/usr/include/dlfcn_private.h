/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#ifndef _DLFCN_PRIVATE_H_
#define _DLFCN_PRIVATE_H_

#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * For use in NSCreateObjectFileImageFromMemory()
 */
#define RTLD_UNLOADABLE 0x80000000

/*
 * Internal interface for dlopen; intended to help audit internal use of
 * dlopen.
 */
#if __i386__
/*
 * i386 makes tail calls difficult, and RTLD_NEXT requires that dlopen_audited
 * tail call dlopen.  Just directly call dlopen on i386.
 */
#define dlopen_audited(__path, __mode) dlopen(__path, __mode)
#else
extern void * dlopen_audited(const char * __path, int __mode) __DYLDDL_DRIVERKIT_UNAVAILABLE;
#endif


/*
 * Sometimes dlopen() looks at who called it (such as for @rpath and @loader_path).
 * This SPI allows you to simulate dlopen() being called by other code.
 * Available in macOS 11.0 and iOS 14.0 and later.
 */
extern void* dlopen_from(const char* __path, int __mode, void* __addressInCaller) __DYLDDL_DRIVERKIT_UNAVAILABLE;


#ifdef __cplusplus
}
#endif

#endif /* _DLFCN_PRIVATE_H_ */
