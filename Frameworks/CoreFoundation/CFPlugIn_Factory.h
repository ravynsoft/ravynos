/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFPlugIn_Factory.h
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFPLUGIN_FACTORY__)
#define __COREFOUNDATION_CFPLUGIN_FACTORY__ 1

#include "CFBundle_Internal.h"

CF_EXTERN_C_BEGIN

typedef struct __CFPFactory *_CFPFactoryRef;

extern _CFPFactoryRef _CFPFactoryCreate(CFAllocatorRef allocator, CFUUIDRef factoryID, CFPlugInFactoryFunction func);
extern _CFPFactoryRef _CFPFactoryCreateByName(CFAllocatorRef allocator, CFUUIDRef factoryID, CFPlugInRef plugIn, CFStringRef funcName);

extern _CFPFactoryRef _CFPFactoryFind(CFUUIDRef factoryID, Boolean enabled);

extern CFUUIDRef _CFPFactoryCopyFactoryID(_CFPFactoryRef factory);
extern CFPlugInRef _CFPFactoryCopyPlugIn(_CFPFactoryRef factory);

extern void *_CFPFactoryCreateInstance(CFAllocatorRef allocator, _CFPFactoryRef factory, CFUUIDRef typeID);
extern void _CFPFactoryDisable(_CFPFactoryRef factory);

extern void _CFPFactoryFlushFunctionCache(_CFPFactoryRef factory);

extern void _CFPFactoryAddType(_CFPFactoryRef factory, CFUUIDRef typeID);
extern void _CFPFactoryRemoveType(_CFPFactoryRef factory, CFUUIDRef typeID);

extern Boolean _CFPFactorySupportsType(_CFPFactoryRef factory, CFUUIDRef typeID);
extern CFArrayRef _CFPFactoryFindCopyForType(CFUUIDRef typeID);

/* These methods are called by CFPlugInInstance when an instance is created or destroyed.  If a factory's instance count goes to 0 and the factory has been disabled, the factory is destroyed. */
extern void _CFPFactoryAddInstance(_CFPFactoryRef factory);
extern void _CFPFactoryRemoveInstance(_CFPFactoryRef factory);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFPLUGIN_FACTORY__ */

