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

/*	
    CFWindowsUtilities.c
    Copyright (c) 2008-2014, Apple Inc. All rights reserved.
    Responsibility: Tony Parker
*/

#if DEPLOYMENT_TARGET_WINDOWS
    
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFString.h>
#include "CFInternal.h"
#include "CFPriv.h"

#include <shlobj.h>

#include <sys/stat.h>

CF_EXPORT bool OSAtomicCompareAndSwapPtr(void *oldp, void *newp, void *volatile *dst) 
{ 
    return oldp == InterlockedCompareExchangePointer(dst, newp, oldp);
}

CF_EXPORT bool OSAtomicCompareAndSwapLong(long oldl, long newl, long volatile *dst) 
{ 
    return oldl == InterlockedCompareExchange(dst, newl, oldl);
}

CF_EXPORT bool OSAtomicCompareAndSwapPtrBarrier(void *oldp, void *newp, void *volatile *dst) 
{ 
    return oldp == InterlockedCompareExchangePointer(dst, newp, oldp);
}

CF_EXPORT int32_t OSAtomicDecrement32Barrier(volatile int32_t *dst)
{
    return InterlockedDecrement((volatile long *)dst);
}

CF_EXPORT int32_t OSAtomicIncrement32Barrier(volatile int32_t *dst)
{
    return InterlockedIncrement((volatile long *)dst);
}

CF_EXPORT int32_t OSAtomicAdd32Barrier( int32_t theAmount, volatile int32_t *theValue ) {
    return (InterlockedExchangeAdd((volatile LONG *)theValue, theAmount) + theAmount);
}

CF_EXPORT bool OSAtomicCompareAndSwap32Barrier(int32_t oldValue, int32_t newValue, volatile int32_t *theValue) {
    return oldValue == InterlockedCompareExchange((long *)theValue, newValue, oldValue);
}

CF_EXPORT int32_t OSAtomicAdd32( int32_t theAmount, volatile int32_t *theValue ) {
    return (InterlockedExchangeAdd((volatile LONG *)theValue, theAmount) + theAmount);
}

CF_EXPORT int32_t OSAtomicIncrement32(volatile int32_t *theValue) {
    return InterlockedIncrement((volatile long *)theValue);
}

CF_EXPORT int32_t OSAtomicDecrement32(volatile int32_t *theValue) {
    return InterlockedDecrement((volatile long *)theValue);
}

// These 64-bit versions of InterlockedCompareExchange are only available on client Vista and later, so we can't use them (yet).
/*
CF_EXPORT bool OSAtomicCompareAndSwap64( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue ) {
    return __oldValue == InterlockedCompareExchange64((volatile LONGLONG *)__theValue, __newValue, __oldValue);
}

CF_EXPORT bool OSAtomicCompareAndSwap64Barrier( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue ) {
    return __oldValue == InterlockedCompareExchange64((volatile LONGLONG *)__theValue, __newValue, __oldValue);
}

CF_EXPORT int64_t OSAtomicAdd64( int64_t __theAmount, volatile int64_t *__theValue ) {
    return (InterlockedExchangeAdd64((volatile LONGLONG *)__theValue, __theAmount) + __theAmount);
}

CF_EXPORT int64_t OSAtomicAdd64Barrier( int64_t __theAmount, volatile int64_t *__theValue ) {
    retun (InterlockedExchangeAdd64((volatile LONGLONG *)__theValue, __theAmount) + __theAmount);
}
 */

void OSMemoryBarrier() {
    MemoryBarrier();
}

void _CFGetFrameworkPath(wchar_t *path, int maxLength) {
#ifdef _DEBUG
    // might be nice to get this from the project file at some point
    wchar_t *DLLFileName = L"CoreFoundation_debug.dll";
#else
    wchar_t *DLLFileName = L"CoreFoundation.dll";
#endif
    path[0] = path[1] = 0;
    DWORD wResult;
    CFIndex idx;
    HMODULE ourModule = GetModuleHandleW(DLLFileName);
    
    CFAssert(ourModule, __kCFLogAssertion, "GetModuleHandle failed");
    
    wResult = GetModuleFileNameW(ourModule, path, maxLength);
    CFAssert1(wResult > 0, __kCFLogAssertion, "GetModuleFileName failed: %d", GetLastError());
    CFAssert1(wResult < maxLength, __kCFLogAssertion, "GetModuleFileName result truncated: %s", path);
    
    // strip off last component, the DLL name
    for (idx = wResult - 1; idx; idx--) {
        if ('\\' == path[idx]) {
            path[idx] = '\0';
            break;
        }
    }
}


#endif

