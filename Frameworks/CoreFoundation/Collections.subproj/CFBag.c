/*	CFBag.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 Responsibility: Michael LeHew
 */

#include <CoreFoundation/CFBag.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include "CFBasicHash.h"
#include <CoreFoundation/CFString.h>


const CFBagCallBacks kCFTypeBagCallBacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, CFCopyDescription, CFEqual, CFHash};
const CFBagCallBacks kCFCopyStringBagCallBacks = {0, __CFStringCollectionCopy, __CFTypeCollectionRelease, CFCopyDescription, CFEqual, CFHash};

static Boolean __CFBagEqual(CFTypeRef cf1, CFTypeRef cf2) {
    return __CFBasicHashEqual((CFBasicHashRef)cf1, (CFBasicHashRef)cf2);
}

static CFHashCode __CFBagHash(CFTypeRef cf) {
    return __CFBasicHashHash((CFBasicHashRef)cf);
}

static CFStringRef __CFBagCopyDescription(CFTypeRef cf) {
    return __CFBasicHashCopyDescription((CFBasicHashRef)cf);
}

static void __CFBagDeallocate(CFTypeRef cf) {
    __CFBasicHashDeallocate((CFBasicHashRef)cf);
}

const CFRuntimeClass __CFBagClass = {
    _kCFRuntimeScannedObject,
    "CFBag",
    NULL,        // init
    NULL,        // copy
    __CFBagDeallocate,
    __CFBagEqual,
    __CFBagHash,
    NULL,        //
    __CFBagCopyDescription
};

CFTypeID CFBagGetTypeID(void) {
    return _kCFRuntimeIDCFBag;
}

static CFBasicHashRef __CFBagCreateGeneric(CFAllocatorRef allocator, CFBagCallBacks const *const inCallbacks) {
    CFOptionFlags flags = kCFBasicHashLinearHashing | kCFBasicHashHasCounts; // kCFBasicHashExponentialHashing
    
    CFBasicHashCallbacks callbacks;
    callbacks.retainKey = inCallbacks ? (uintptr_t (*)(CFAllocatorRef, uintptr_t))inCallbacks->retain : NULL;
    callbacks.releaseKey = inCallbacks ? (void (*)(CFAllocatorRef, uintptr_t))inCallbacks->release : NULL;
    callbacks.equateKeys = inCallbacks ? (Boolean (*)(uintptr_t, uintptr_t))inCallbacks->equal : NULL;
    callbacks.hashKey = inCallbacks ? (CFHashCode (*)(uintptr_t))inCallbacks->hash : NULL;
    callbacks.getIndirectKey = NULL;
    callbacks.copyKeyDescription = inCallbacks ? (CFStringRef (*)(uintptr_t))inCallbacks->copyDescription : NULL;
    callbacks.retainValue = callbacks.retainKey;
    callbacks.releaseValue = callbacks.releaseKey;
    callbacks.equateValues = callbacks.equateKeys;
    callbacks.copyValueDescription = callbacks.copyKeyDescription;
    
    CFBasicHashRef ht = CFBasicHashCreate(allocator, flags, &callbacks);
    return ht;
}

CF_PRIVATE CFBagRef __CFBagCreateTransfer(CFAllocatorRef allocator, const void **klist, CFIndex numValues) {
    const void **vlist = klist;
    CFTypeID typeID = CFBagGetTypeID();
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%ld) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    CFOptionFlags flags = kCFBasicHashLinearHashing | kCFBasicHashHasCounts; // kCFBasicHashExponentialHashing
    
    CFBasicHashCallbacks callbacks;
    callbacks.retainKey = (uintptr_t (*)(CFAllocatorRef, uintptr_t))kCFTypeBagCallBacks.retain;
    callbacks.releaseKey = (void (*)(CFAllocatorRef, uintptr_t))kCFTypeBagCallBacks.release;
    callbacks.equateKeys = (Boolean (*)(uintptr_t, uintptr_t))kCFTypeBagCallBacks.equal;
    callbacks.hashKey = (CFHashCode (*)(uintptr_t))kCFTypeBagCallBacks.hash;
    callbacks.getIndirectKey = NULL;
    callbacks.copyKeyDescription = (CFStringRef (*)(uintptr_t))kCFTypeBagCallBacks.copyDescription;
    callbacks.retainValue = callbacks.retainKey;
    callbacks.releaseValue = callbacks.releaseKey;
    callbacks.equateValues = callbacks.equateKeys;
    callbacks.copyValueDescription = callbacks.copyKeyDescription;
    
    CFBasicHashRef ht = CFBasicHashCreate(allocator, flags, &callbacks);
    CFBasicHashSuppressRC(ht);
    if (0 < numValues) CFBasicHashSetCapacity(ht, numValues);
    for (CFIndex idx = 0; idx < numValues; idx++) {
        CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
    }
    CFBasicHashUnsuppressRC(ht);
    CFBasicHashMakeImmutable(ht);
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFBag (immutable)");
    return (CFBagRef)ht;
}

CFBagRef CFBagCreate(CFAllocatorRef allocator, const void **klist, CFIndex numValues, const CFBagCallBacks *callbacks) {
    const void **vlist = klist;
    CFTypeID typeID = CFBagGetTypeID();
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%ld) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    CFBasicHashRef ht = __CFBagCreateGeneric(allocator, callbacks);
    if (!ht) return NULL;
    if (0 < numValues) CFBasicHashSetCapacity(ht, numValues);
    for (CFIndex idx = 0; idx < numValues; idx++) {
        CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
    }
    CFBasicHashMakeImmutable(ht);
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFBag (immutable)");
    return (CFBagRef)ht;
}

CFMutableBagRef CFBagCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFBagCallBacks *callbacks) {
    CFTypeID typeID = CFBagGetTypeID();
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFBasicHashRef ht = __CFBagCreateGeneric(allocator, callbacks);
    if (!ht) return NULL;
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFBag (mutable)");
    return (CFMutableBagRef)ht;
}

CFBagRef CFBagCreateCopy(CFAllocatorRef allocator, CFBagRef other) {
    CFTypeID typeID = CFBagGetTypeID();
    CFAssert1(other, __kCFLogAssertion, "%s(): other CFBag cannot be NULL", __PRETTY_FUNCTION__);
    __CFGenericValidateType(other, typeID);
    Boolean markImmutable = false;
    CFBasicHashRef ht = NULL;
    if (CF_IS_OBJC(typeID, other)) {
        CFIndex numValues = CFBagGetCount(other);
        const void *vbuffer[256];
        Boolean const useStack = numValues <= 256;
        const void **vlist = (useStack) ? vbuffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, numValues * sizeof(const void *), 0);
        const void **klist = vlist;
        CFBagGetValues(other, vlist);
        ht = __CFBagCreateGeneric(allocator, &kCFTypeBagCallBacks);
        if (ht && 0 < numValues) CFBasicHashSetCapacity(ht, numValues);
        for (CFIndex idx = 0; ht && idx < numValues; idx++) {
            CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
        }
        if (!useStack) CFAllocatorDeallocate(kCFAllocatorSystemDefault, vlist);
        markImmutable = true;
    } else { // non-objc types
        ht = CFBasicHashCreateCopy(allocator, (CFBasicHashRef)other);
        markImmutable = true;
    }
    if (ht && markImmutable) {
        CFBasicHashMakeImmutable(ht);
        _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
        if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFBag (immutable)");
        return (CFBagRef)ht;
    }
    return (CFBagRef)ht;
}

CFMutableBagRef CFBagCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFBagRef other) {
    CFTypeID typeID = CFBagGetTypeID();
    CFAssert1(other, __kCFLogAssertion, "%s(): other CFBag cannot be NULL", __PRETTY_FUNCTION__);
    __CFGenericValidateType(other, typeID);
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFBasicHashRef ht = NULL;
    if (CF_IS_OBJC(typeID, other) || CF_IS_SWIFT(typeID, other)) {
        CFIndex numValues = CFBagGetCount(other);
        const void *vbuffer[256], *kbuffer[256];
        Boolean const useStack = numValues <= 256;
        const void **vlist = (useStack) ? vbuffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, numValues * sizeof(const void *), 0);
        const void **klist = vlist;
        CFBagGetValues(other, vlist);
        ht = __CFBagCreateGeneric(allocator, & kCFTypeBagCallBacks);
        if (ht && 0 < numValues) CFBasicHashSetCapacity(ht, numValues);
        for (CFIndex idx = 0; ht && idx < numValues; idx++) {
            CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
        }
        if (klist != kbuffer && klist != vlist) CFAllocatorDeallocate(kCFAllocatorSystemDefault, klist);
        if (!useStack) CFAllocatorDeallocate(kCFAllocatorSystemDefault, vlist);
    } else {
        ht = CFBasicHashCreateCopy(allocator, (CFBasicHashRef)other);
    }
    if (!ht) return NULL;
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFBag (mutable)");
    return (CFMutableBagRef)ht;
}

CFIndex CFBagGetCount(CFBagRef hc) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    return CFBasicHashGetCount((CFBasicHashRef)hc);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT CFIndex _CFBagGetUniqueCount(CFBagRef hc) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    return CFBasicHashGetUsedBucketCount((CFBasicHashRef)hc);
}

CFIndex CFBagGetCountOfValue(CFBagRef hc, const void *key) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    return CFBasicHashGetCountOfKey((CFBasicHashRef)hc, (uintptr_t)key);
}

Boolean CFBagContainsValue(CFBagRef hc, const void *key) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    return (0 < CFBasicHashGetCountOfKey((CFBasicHashRef)hc, (uintptr_t)key));
}

const void *CFBagGetValue(CFBagRef hc, const void *key) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFBasicHashBucket bkt = CFBasicHashFindBucket((CFBasicHashRef)hc, (uintptr_t)key);
    return (0 < bkt.count ? (const void *)bkt.weak_value : 0);
}

Boolean CFBagGetValueIfPresent(CFBagRef hc, const void *key, const void **value) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFBasicHashBucket bkt = CFBasicHashFindBucket((CFBasicHashRef)hc, (uintptr_t)key);
    if (0 < bkt.count) {
        if (value) {
            *value = (const void *)bkt.weak_value;
        }
        return true;
    }
    return false;
}

void CFBagGetValues(CFBagRef hc, const void **keybuf) {
    const void **valuebuf = 0;
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFBasicHashGetElements((CFBasicHashRef)hc, CFBagGetCount(hc), (uintptr_t *)valuebuf, (uintptr_t *)keybuf);
}

void CFBagApplyFunction(CFBagRef hc, CFBagApplierFunction applier, void *context) {
    FAULT_CALLBACK((void **)&(applier));
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFBasicHashApply((CFBasicHashRef)hc, ^(CFBasicHashBucket bkt) {
        for (CFIndex cnt = bkt.count; cnt--;) {
            INVOKE_CALLBACK2(applier, (const void *)bkt.weak_value, context);
        }
        return (Boolean)true;
    });
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT unsigned long _CFBagFastEnumeration(CFBagRef hc, struct __objcFastEnumerationStateEquivalent *state, void *stackbuffer, unsigned long count) {
    if (CF_IS_SWIFT(CFBagGetTypeID(), hc)) return 0;
    if (CF_IS_OBJC(CFBagGetTypeID(), hc)) return 0;
    __CFGenericValidateType(hc, CFBagGetTypeID());
    return __CFBasicHashFastEnumeration((CFBasicHashRef)hc, (struct __objcFastEnumerationStateEquivalent2 *)state, stackbuffer, count);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT Boolean _CFBagIsMutable(CFBagRef hc) {
    if (CF_IS_SWIFT(CFBagGetTypeID(), hc)) return false;
    if (CF_IS_OBJC(CFBagGetTypeID(), hc)) return false;
    __CFGenericValidateType(hc, CFBagGetTypeID());
    return CFBasicHashIsMutable((CFBasicHashRef)hc);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT void _CFBagSetCapacity(CFMutableBagRef hc, CFIndex cap) {
    if (CF_IS_SWIFT(CFBagGetTypeID(), hc)) return;
    if (CF_IS_OBJC(CFBagGetTypeID(), hc)) return;
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    CFAssert3(CFBagGetCount(hc) <= cap, __kCFLogAssertion, "%s(): desired capacity (%ld) is less than count (%ld)", __PRETTY_FUNCTION__, cap, CFBagGetCount(hc));
    CFBasicHashSetCapacity((CFBasicHashRef)hc, cap);
}

void CFBagAddValue(CFMutableBagRef hc, const void *key) {
    const void *value = key;
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashAddValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
}

void CFBagReplaceValue(CFMutableBagRef hc, const void *key) {
    const void *value = key;
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashReplaceValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
}

void CFBagSetValue(CFMutableBagRef hc, const void *key) {
    const void *value = key;
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashSetValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
}

void CFBagRemoveValue(CFMutableBagRef hc, const void *key) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashRemoveValue((CFBasicHashRef)hc, (uintptr_t)key);
}

void CFBagRemoveAllValues(CFMutableBagRef hc) {
    __CFGenericValidateType(hc, CFBagGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashRemoveAllValues((CFBasicHashRef)hc);
}
