/*
 * Copyright (c) 2008, 2012 Apple Inc. All rights reserved.
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

#ifndef __KEXTMANAGERPRIV_H__
#define __KEXTMANAGERPRIV_H__

#include <CoreFoundation/CoreFoundation.h>
#include <TargetConditionals.h>
#if !TARGET_OS_IPHONE
#include <Security/Authorization.h>
#endif /* !TARGET_OS_IPHONE */

#include <sys/cdefs.h>

__BEGIN_DECLS

#define kKextLoadIdentifierKey   CFSTR("KextLoadIdentifier")
#define kKextLoadPathKey         CFSTR("KextLoadPath")
#define kKextLoadDependenciesKey CFSTR("KextLoadDependencyPaths")

#define kExtPathKey             CFSTR("ExtPath")
#define kExtEnabledKey          CFSTR("ExtEnabled")

CFArrayRef _KextManagerCreatePropertyValueArray(
    CFAllocatorRef allocator,
    CFStringRef    propertyKey);

/*
 * This is part of a private, entitled interface between kextd, which manages
 * the lifecycle of kernel and userspace driver extensions, and sysextd,
 * which manages the installation of third-party system extensions.
 * If you are not sysextd or kextd, you should not use these functions.
 * They are liable to change at any time.
 */

/* Validate an extension in-place on the filesystem. */
OSReturn _KextManagerValidateExtension(CFStringRef extPath);
/* Update an extension's enablement state. */
OSReturn _KextManagerUpdateExtension(CFStringRef extPath, bool extIsEnabled);
/* Ask kextd to stop an extension - it can say no if it is unable. */
OSReturn _KextManagerStopExtension(CFStringRef extPath);

__END_DECLS

#endif /* __KEXTMANAGERPRIV_H__ */
