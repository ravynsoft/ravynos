/*	CFSet.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 Responsibility: Michael LeHew
 */

#include <CoreFoundation/CFSet.h>
#include "CFInternal.h"
#include "CFBasicHash.h"
#include <CoreFoundation/CFString.h>
#include "CFRuntime_Internal.h"


const CFSetCallBacks kCFTypeSetCallBacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, CFCopyDescription, CFEqual, CFHash};
const CFSetCallBacks kCFCopyStringSetCallBacks = {0, __CFStringCollectionCopy, __CFTypeCollectionRelease, CFCopyDescription, CFEqual, CFHash};

CF_PRIVATE CFSetCallBacks __CFSetGetCallbacks(CFSetRef hc) {
    CFBasicHashCallbacks hashCallbacks = __CFBasicHashGetCallbacks(hc);
    CFSetCallBacks setCallbacks = {
        .version = 0,
        .retain = (CFSetRetainCallBack)hashCallbacks.retainKey,
        .release = (CFSetReleaseCallBack)hashCallbacks.releaseKey,
        .equal = (CFSetEqualCallBack)hashCallbacks.equateKeys,
        .hash = (CFSetHashCallBack)hashCallbacks.hashKey,
        .copyDescription = (CFSetCopyDescriptionCallBack)hashCallbacks.copyKeyDescription
    };
    return setCallbacks;
}

static Boolean __CFSetEqual(CFTypeRef cf1, CFTypeRef cf2) {
    return __CFBasicHashEqual((CFBasicHashRef)cf1, (CFBasicHashRef)cf2);
}

static CFHashCode __CFSetHash(CFTypeRef cf) {
    return __CFBasicHashHash((CFBasicHashRef)cf);
}

static CFStringRef __CFSetCopyDescription(CFTypeRef cf) {
    return __CFBasicHashCopyDescription((CFBasicHashRef)cf);
}

static void __CFSetDeallocate(CFTypeRef cf) {
    __CFBasicHashDeallocate((CFBasicHashRef)cf);
}

const CFRuntimeClass __CFSetClass = {
    _kCFRuntimeScannedObject,
    "CFSet",
    NULL,        // init
    NULL,        // copy
    __CFSetDeallocate,
    __CFSetEqual,
    __CFSetHash,
    NULL,        //
    __CFSetCopyDescription
};

CFTypeID CFSetGetTypeID(void) {
    return _kCFRuntimeIDCFSet;
}

static CFBasicHashRef __CFSetCreateGeneric(CFAllocatorRef allocator, const CFSetCallBacks *inCallbacks) {
    CFOptionFlags flags = kCFBasicHashLinearHashing; // kCFBasicHashExponentialHashing
    
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

CF_PRIVATE CFSetRef __CFSetCreateTransfer(CFAllocatorRef allocator, const void **klist, CFIndex numValues) {
    const void **vlist = klist;
    
    CFTypeID typeID = CFSetGetTypeID();
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%ld) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    CFOptionFlags flags = kCFBasicHashLinearHashing; // kCFBasicHashExponentialHashing
    
    CFBasicHashCallbacks callbacks;
    callbacks.retainKey = (uintptr_t (*)(CFAllocatorRef, uintptr_t))kCFTypeSetCallBacks.retain;
    callbacks.releaseKey = (void (*)(CFAllocatorRef, uintptr_t))kCFTypeSetCallBacks.release;
    callbacks.equateKeys = (Boolean (*)(uintptr_t, uintptr_t))kCFTypeSetCallBacks.equal;
    callbacks.hashKey = (CFHashCode (*)(uintptr_t))kCFTypeSetCallBacks.hash;
    callbacks.getIndirectKey = NULL;
    callbacks.copyKeyDescription = (CFStringRef (*)(uintptr_t))kCFTypeSetCallBacks.copyDescription;
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
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFSet (immutable)");
    return (CFSetRef)ht;
}

CFSetRef CFSetCreate(CFAllocatorRef allocator, const void **klist, CFIndex numValues, const CFSetCallBacks *callbacks) {
    const void **vlist = klist;
    
    CFTypeID typeID = CFSetGetTypeID();
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%ld) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    CFBasicHashRef ht = __CFSetCreateGeneric(allocator, callbacks);
    if (!ht) return NULL;
    if (0 < numValues) CFBasicHashSetCapacity(ht, numValues);
    for (CFIndex idx = 0; idx < numValues; idx++) {
        CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
    }
    CFBasicHashMakeImmutable(ht);
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFSet (immutable)");
    return (CFSetRef)ht;
}

CFMutableSetRef CFSetCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFSetCallBacks *callbacks) {
    CFTypeID typeID = CFSetGetTypeID();
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFBasicHashRef ht = __CFSetCreateGeneric(allocator, callbacks);
    if (!ht) return NULL;
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFSet (mutable)");
    return (CFMutableSetRef)ht;
}

CFSetRef CFSetCreateCopy(CFAllocatorRef allocator, CFSetRef other) {
    CFTypeID typeID = CFSetGetTypeID();
    CFAssert1(other, __kCFLogAssertion, "%s(): other CFSet cannot be NULL", __PRETTY_FUNCTION__);
    __CFGenericValidateType(other, typeID);
    Boolean markImmutable = false;
    CFBasicHashRef ht = NULL;
    if (CF_IS_OBJC(typeID, other)) {
        ht = (CFBasicHashRef)CF_OBJC_CALLV((id)other, copyWithZone:NULL);
    } else if (CF_IS_SWIFT(typeID, other)) {
        ht = (CFBasicHashRef)CF_SWIFT_CALLV(other, NSObject.copyWithZone, nil);
    } else { // non-objc types
        ht = CFBasicHashCreateCopy(allocator, (CFBasicHashRef)other);
        markImmutable = true;
    }
    if (ht && markImmutable) {
        CFBasicHashMakeImmutable(ht);
        _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
        if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFSet (immutable)");
        return (CFSetRef)ht;
    }
    return (CFSetRef)ht;
}

CFMutableSetRef CFSetCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFSetRef other) {
    CFTypeID typeID = CFSetGetTypeID();
    CFAssert1(other, __kCFLogAssertion, "%s(): other CFSet cannot be NULL", __PRETTY_FUNCTION__);
    __CFGenericValidateType(other, typeID);
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFBasicHashRef ht = NULL;
    if (CF_IS_OBJC(typeID, other) || CF_IS_SWIFT(typeID, other)) {
        CFIndex numValues = CFSetGetCount(other);
        const void *vbuffer[256], *kbuffer[256];
        const void **vlist = (numValues <= 256) ? vbuffer : (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, numValues * sizeof(const void *), 0);
        const void **klist = vlist;
        CFSetGetValues(other, vlist);
        ht = __CFSetCreateGeneric(allocator, & kCFTypeSetCallBacks);
        if (ht && 0 < numValues) CFBasicHashSetCapacity(ht, numValues);
        for (CFIndex idx = 0; ht && idx < numValues; idx++) {
            CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
        }
        if (klist != kbuffer && klist != vlist) CFAllocatorDeallocate(kCFAllocatorSystemDefault, klist);
        if (vlist != vbuffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, vlist);
    } else {
        ht = CFBasicHashCreateCopy(allocator, (CFBasicHashRef)other);
    }
    if (!ht) return NULL;
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFSet (mutable)");
    return (CFMutableSetRef)ht;
}

CFIndex CFSetGetCount(CFSetRef hc) {
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), CFIndex, (CFSwiftRef)hc, NSSet.count);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), CFIndex, (NSSet *)hc, count);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    return CFBasicHashGetCount((CFBasicHashRef)hc);
}

CFIndex CFSetGetCountOfValue(CFSetRef hc, const void *key) {
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), CFIndex, (CFSwiftRef)hc, NSSet.countForKey, key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), CFIndex, (NSSet *)hc, countForObject:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    return CFBasicHashGetCountOfKey((CFBasicHashRef)hc, (uintptr_t)key);
}

Boolean CFSetContainsValue(CFSetRef hc, const void *key) {
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), char, (CFSwiftRef)hc, NSSet.containsObject, (CFSwiftRef)key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), char, (NSSet *)hc, containsObject:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    return (0 < CFBasicHashGetCountOfKey((CFBasicHashRef)hc, (uintptr_t)key));
}

const void *CFSetGetValue(CFSetRef hc, const void *key) {
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), const void *, (CFSwiftRef)hc, NSSet.member, key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), const void *, (NSSet *)hc, member:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFBasicHashBucket bkt = CFBasicHashFindBucket((CFBasicHashRef)hc, (uintptr_t)key);
    return (0 < bkt.count ? (const void *)bkt.weak_value : 0);
}

Boolean CFSetGetValueIfPresent(CFSetRef hc, const void *key, const void **value) {
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), Boolean, (CFSwiftRef)hc, NSSet.__getValue, value, key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), Boolean, (NSSet *)hc, __getValue:(id *)value forObj:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFBasicHashBucket bkt = CFBasicHashFindBucket((CFBasicHashRef)hc, (uintptr_t)key);
    if (0 < bkt.count) {
        if (value) {
            *value = (const void *)bkt.weak_value;
        }
        return true;
    }
    return false;
}

void CFSetGetValues(CFSetRef hc, const void **keybuf) {
    const void **valuebuf = 0;
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), void, (CFSwiftRef)hc, NSSet.getObjects, keybuf);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), void, (NSSet *)hc, getObjects:(id *)keybuf);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFBasicHashGetElements((CFBasicHashRef)hc, CFSetGetCount(hc), (uintptr_t *)valuebuf, (uintptr_t *)keybuf);
}

void CFSetApplyFunction(CFSetRef hc, CFSetApplierFunction applier, void *context) {
    FAULT_CALLBACK((void **)&(applier));
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), void, (CFSwiftRef)hc, NSSet.__apply, applier, context);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), void, (NSSet *)hc, __applyValues:(void (*)(const void *, void *))applier context:(void *)context);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFBasicHashApply((CFBasicHashRef)hc, ^(CFBasicHashBucket bkt) {
        INVOKE_CALLBACK2(applier, (const void *)bkt.weak_value, context);
        return (Boolean)true;
    });
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT unsigned long _CFSetFastEnumeration(CFSetRef hc, struct __objcFastEnumerationStateEquivalent *state, void *stackbuffer, unsigned long count) {
    if (CF_IS_SWIFT(CFSetGetTypeID(), hc)) return 0;
    if (CF_IS_OBJC(CFSetGetTypeID(), hc)) return 0;
    __CFGenericValidateType(hc, CFSetGetTypeID());
    return __CFBasicHashFastEnumeration((CFBasicHashRef)hc, (struct __objcFastEnumerationStateEquivalent2 *)state, stackbuffer, count);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT Boolean _CFSetIsMutable(CFSetRef hc) {
    if (CF_IS_SWIFT(CFSetGetTypeID(), hc)) return false;
    if (CF_IS_OBJC(CFSetGetTypeID(), hc)) return false;
    __CFGenericValidateType(hc, CFSetGetTypeID());
    return CFBasicHashIsMutable((CFBasicHashRef)hc);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT void _CFSetSetCapacity(CFMutableSetRef hc, CFIndex cap) {
    if (CF_IS_SWIFT(CFSetGetTypeID(), hc)) return;
    if (CF_IS_OBJC(CFSetGetTypeID(), hc)) return;
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    CFAssert3(CFSetGetCount(hc) <= cap, __kCFLogAssertion, "%s(): desired capacity (%ld) is less than count (%ld)", __PRETTY_FUNCTION__, cap, CFSetGetCount(hc));
    CFBasicHashSetCapacity((CFBasicHashRef)hc, cap);
}

void CFSetAddValue(CFMutableSetRef hc, const void *key) {
    const void *value = key;
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), void, (CFSwiftRef)hc, NSMutableSet.addObject, (CFSwiftRef)key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), void, (NSMutableSet *)hc, addObject:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashAddValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
}

void CFSetReplaceValue(CFMutableSetRef hc, const void *key) {
    const void *value = key;
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), void, (CFSwiftRef)hc, NSMutableSet.replaceObject, (CFSwiftRef)key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), void, (NSMutableSet *)hc, replaceObject:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashReplaceValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
}

void CFSetSetValue(CFMutableSetRef hc, const void *key) {
    const void *value = key;
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), void, (CFSwiftRef)hc, NSMutableSet.setObject, (CFSwiftRef)key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), void, (NSMutableSet *)hc, setObject:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashSetValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
}

void CFSetRemoveValue(CFMutableSetRef hc, const void *key) {
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), void, (CFSwiftRef)hc, NSMutableSet.removeObject, (CFSwiftRef)key);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), void, (NSMutableSet *)hc, removeObject:(id)key);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashRemoveValue((CFBasicHashRef)hc, (uintptr_t)key);
}

void CFSetRemoveAllValues(CFMutableSetRef hc) {
    CF_SWIFT_FUNCDISPATCHV(CFSetGetTypeID(), void, (CFSwiftRef)hc, NSMutableSet.removeAllObjects);
    CF_OBJC_FUNCDISPATCHV(CFSetGetTypeID(), void, (NSMutableSet *)hc, removeAllObjects);
    __CFGenericValidateType(hc, CFSetGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CFBasicHashRemoveAllValues((CFBasicHashRef)hc);
}
