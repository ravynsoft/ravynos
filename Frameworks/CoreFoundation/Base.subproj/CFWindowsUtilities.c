/*	
	CFWindowsUtilities.c
	Copyright (c) 2008-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Tony Parker
*/

#if TARGET_OS_WIN32
    
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

