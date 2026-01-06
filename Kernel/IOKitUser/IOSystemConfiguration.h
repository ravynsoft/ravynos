/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#ifndef _IOKIT_PWRMGT_IOSYSTEMCONFIGURATION_H
#define _IOKIT_PWRMGT_IOSYSTEMCONFIGURATION_H

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPreferencesPrivate.h>
#include <SystemConfiguration/SCDynamicStorePrivate.h>
#include <SystemConfiguration/SCValidation.h>
#include <TargetConditionals.h>

extern const CFStringRef		_io_kSCCompAnyRegex;
#undef  kSCCompAnyRegex
#define kSCCompAnyRegex                     _io_kSCCompAnyRegex

extern const CFStringRef		_io_kSCDynamicStoreDomainState;
#undef  kSCDynamicStoreDomainState
#define kSCDynamicStoreDomainState          _io_kSCDynamicStoreDomainState

#if TARGET_OS_IPHONE
    /* These prototypes are for embedded only 
     * SystemConfiguration headers define each of these function for OS */

__private_extern__ int _io_SCError();
#define SCError                             _io_SCError

__private_extern__ Boolean _io_SCDynamicStoreAddWatchedKey(
    SCDynamicStoreRef           store,
    CFStringRef                 key,
    Boolean                      isRegex);
#define SCDynamicStoreAddWatchedKey         _io_SCDynamicStoreAddWatchedKey

__private_extern__ CFDictionaryRef _io_SCDynamicStoreCopyMultiple(
    SCDynamicStoreRef           store,
    CFArrayRef                  keys,
    CFArrayRef                  patterns);
#define SCDynamicStoreCopyMultiple           _io_SCDynamicStoreCopyMultiple

__private_extern__ CFPropertyListRef _io_SCDynamicStoreCopyValue(
    SCDynamicStoreRef           store,
    CFStringRef                 key);
#define SCDynamicStoreCopyValue             _io_SCDynamicStoreCopyValue

__private_extern__ SCDynamicStoreRef _io_SCDynamicStoreCreate(
    CFAllocatorRef              allocator,
    CFStringRef                 name,
    SCDynamicStoreCallBack      callout,
    SCDynamicStoreContext       *context);
#define SCDynamicStoreCreate                _io_SCDynamicStoreCreate

__private_extern__ CFRunLoopSourceRef _io_SCDynamicStoreCreateRunLoopSource(
    CFAllocatorRef              allocator,
    SCDynamicStoreRef           store,
    CFIndex                     order);
#define SCDynamicStoreCreateRunLoopSource   _io_SCDynamicStoreCreateRunLoopSource

__private_extern__ CFStringRef _io_SCDynamicStoreKeyCreate(
    CFAllocatorRef              allocator,
    CFStringRef                 fmt, ...);
#define SCDynamicStoreKeyCreate             _io_SCDynamicStoreKeyCreate    

__private_extern__ CFStringRef _io_SCDynamicStoreKeyCreatePreferences(
    CFAllocatorRef              allocator,
    CFStringRef                 prefsID,
    SCPreferencesKeyType        keyType);
#define SCDynamicStoreKeyCreatePreferences  _io_SCDynamicStoreKeyCreatePreferences

__private_extern__ Boolean _io_SCDynamicStoreSetNotificationKeys(
    SCDynamicStoreRef           store,
    CFArrayRef                  keys,
    CFArrayRef                  patterns);
#define SCDynamicStoreSetNotificationKeys   _io_SCDynamicStoreSetNotificationKeys

__private_extern__ Boolean _io_SCDynamicStoreSetValue(
    SCDynamicStoreRef           store,
    CFStringRef                 key,
    CFPropertyListRef           value);
#define SCDynamicStoreSetValue              _io_SCDynamicStoreSetValue

__private_extern__ Boolean _io_SCDynamicStoreNotifyValue(
    SCDynamicStoreRef           store,
    CFStringRef                 key);
#define SCDynamicStoreNotifyValue           _io_SCDynamicStoreNotifyValue

__private_extern__ Boolean _io_SCPreferencesApplyChanges(
    SCPreferencesRef            prefs);
#define SCPreferencesApplyChanges           _io_SCPreferencesApplyChanges

__private_extern__ Boolean _io_SCPreferencesCommitChanges(
    SCPreferencesRef            prefs);
#define SCPreferencesCommitChanges          _io_SCPreferencesCommitChanges

__private_extern__ SCPreferencesRef _io_SCPreferencesCreate(
    CFAllocatorRef              allocator,
    CFStringRef                 name,
    CFStringRef                 prefsID);
#define SCPreferencesCreate                 _io_SCPreferencesCreate    

__private_extern__ SCPreferencesRef _io_SCPreferencesCreateWithAuthorization(
    CFAllocatorRef              allocator,
    CFStringRef                 name,
    CFStringRef                 prefsID,
    AuthorizationRef            authorization);
#define SCPreferencesCreateWithAuthorization _io_SCPreferencesCreateWithAuthorization

__private_extern__ CFPropertyListRef _io_SCPreferencesGetValue(
    SCPreferencesRef            prefs,
    CFStringRef                 key);
#define SCPreferencesGetValue               _io_SCPreferencesGetValue

__private_extern__ Boolean _io_SCPreferencesLock(
    SCPreferencesRef            prefs,
    Boolean                     wait);
#define SCPreferencesLock                   _io_SCPreferencesLock

__private_extern__ Boolean _io_SCPreferencesRemoveValue(
    SCPreferencesRef            prefs,
    CFStringRef                 key);
#define SCPreferencesRemoveValue            _io_SCPreferencesRemoveValue

__private_extern__ Boolean _io_SCPreferencesSetValue(
    SCPreferencesRef            prefs,
    CFStringRef                 key,
    CFPropertyListRef           value);
#define SCPreferencesSetValue               _io_SCPreferencesSetValue

__private_extern__ Boolean _io_SCPreferencesUnlock(
    SCPreferencesRef            prefs);
#define SCPreferencesUnlock                 _io_SCPreferencesUnlock

#endif // TARGET_OS_IPHONE

#endif // _IOKIT_PWRMGT_IOSYSTEMCONFIGURATION_H


