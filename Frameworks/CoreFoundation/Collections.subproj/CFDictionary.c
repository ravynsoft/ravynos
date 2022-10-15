/*	CFDictionary.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 Responsibility: Michael LeHew
 */

#include <CoreFoundation/CFDictionary.h>
#include "CFInternal.h"
#include "CFBasicHash.h"
#include <assert.h>
#include <CoreFoundation/CFString.h>
#include "CFRuntime_Internal.h"


const CFDictionaryKeyCallBacks kCFTypeDictionaryKeyCallBacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, CFCopyDescription, CFEqual, CFHash};
const CFDictionaryKeyCallBacks kCFCopyStringDictionaryKeyCallBacks = {0, __CFStringCollectionCopy, __CFTypeCollectionRelease, CFCopyDescription, CFEqual, CFHash};
const CFDictionaryValueCallBacks kCFTypeDictionaryValueCallBacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, CFCopyDescription, CFEqual};

CF_PRIVATE CFDictionaryKeyCallBacks __CFDictionaryGetKeyCallbacks(CFSetRef hc) {
    CFBasicHashCallbacks hashCallbacks = __CFBasicHashGetCallbacks(hc);
    CFDictionaryKeyCallBacks keyCallbacks = {
        .version = 0,
        .retain = (CFSetRetainCallBack)hashCallbacks.retainKey,
        .release = (CFSetReleaseCallBack)hashCallbacks.releaseKey,
        .equal = (CFSetEqualCallBack)hashCallbacks.equateKeys,
        .hash = (CFSetHashCallBack)hashCallbacks.hashKey,
        .copyDescription = (CFSetCopyDescriptionCallBack)hashCallbacks.copyKeyDescription
    };
    return keyCallbacks;
}

CF_PRIVATE CFDictionaryValueCallBacks __CFDictionaryGetValueCallbacks(CFSetRef hc) {
    CFBasicHashCallbacks hashCallbacks = __CFBasicHashGetCallbacks(hc);
    CFDictionaryValueCallBacks valueCallbacks = {
        .version = 0,
        .retain = (CFSetRetainCallBack)hashCallbacks.retainValue,
        .release = (CFSetReleaseCallBack)hashCallbacks.releaseValue,
        .equal = (CFSetEqualCallBack)hashCallbacks.equateValues,
        .copyDescription = (CFSetCopyDescriptionCallBack)hashCallbacks.copyValueDescription
    };
    return valueCallbacks;
}

static Boolean __CFDictionaryEqual(CFTypeRef cf1, CFTypeRef cf2) {
    return __CFBasicHashEqual((CFBasicHashRef)cf1, (CFBasicHashRef)cf2);
}

static CFHashCode __CFDictionaryHash(CFTypeRef cf) {
    return __CFBasicHashHash((CFBasicHashRef)cf);
}

static CFStringRef __CFDictionaryCopyDescription(CFTypeRef cf) {
    return __CFBasicHashCopyDescription((CFBasicHashRef)cf);
}

static void __CFDictionaryDeallocate(CFTypeRef cf) {
    __CFBasicHashDeallocate((CFBasicHashRef)cf);
}

const CFRuntimeClass __CFDictionaryClass = {
    _kCFRuntimeScannedObject,
    "CFDictionary",
    NULL,        // init
    NULL,        // copy
    __CFDictionaryDeallocate,
    __CFDictionaryEqual,
    __CFDictionaryHash,
    NULL,        //
    __CFDictionaryCopyDescription
};

CFTypeID CFDictionaryGetTypeID(void) {
    return _kCFRuntimeIDCFDictionary;
}

static CFBasicHashRef __CFDictionaryCreateGeneric(CFAllocatorRef allocator, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks, Boolean useValueCB) {
    CFOptionFlags flags = kCFBasicHashLinearHashing | kCFBasicHashHasKeys; // kCFBasicHashExponentialHashing
    
    CFBasicHashCallbacks callbacks;
    callbacks.retainKey = keyCallBacks ? (uintptr_t (*)(CFAllocatorRef, uintptr_t))keyCallBacks->retain : NULL;
    callbacks.releaseKey = keyCallBacks ? (void (*)(CFAllocatorRef, uintptr_t))keyCallBacks->release : NULL;
    callbacks.equateKeys = keyCallBacks ? (Boolean (*)(uintptr_t, uintptr_t))keyCallBacks->equal : NULL;
    callbacks.hashKey = keyCallBacks ? (CFHashCode (*)(uintptr_t))keyCallBacks->hash : NULL;
    callbacks.getIndirectKey = NULL;
    callbacks.copyKeyDescription = keyCallBacks ? (CFStringRef (*)(uintptr_t))keyCallBacks->copyDescription : NULL;
    callbacks.retainValue = useValueCB ? (valueCallBacks ? (uintptr_t (*)(CFAllocatorRef, uintptr_t))valueCallBacks->retain : NULL) : (callbacks.retainKey);
    callbacks.releaseValue = useValueCB ? (valueCallBacks ? (void (*)(CFAllocatorRef, uintptr_t))valueCallBacks->release : NULL) : (callbacks.releaseKey);
    callbacks.equateValues = useValueCB ? (valueCallBacks ? (Boolean (*)(uintptr_t, uintptr_t))valueCallBacks->equal : NULL) : (callbacks.equateKeys);
    callbacks.copyValueDescription = useValueCB ? (valueCallBacks ? (CFStringRef (*)(uintptr_t))valueCallBacks->copyDescription : NULL) : (callbacks.copyKeyDescription);
    
    CFBasicHashRef ht = CFBasicHashCreate(allocator, flags, &callbacks);
    return ht;
}

CF_PRIVATE CFDictionaryRef __CFDictionaryCreateTransfer(CFAllocatorRef allocator, void const **klist, void const **vlist, CFIndex numValues) {
    CFTypeID typeID = _kCFRuntimeIDCFDictionary;
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%ld) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    CFOptionFlags flags = kCFBasicHashLinearHashing | kCFBasicHashHasKeys; // kCFBasicHashExponentialHashing
    
    CFBasicHashCallbacks callbacks;
    callbacks.retainKey = (uintptr_t (*)(CFAllocatorRef, uintptr_t))kCFTypeDictionaryKeyCallBacks.retain;
    callbacks.releaseKey = (void (*)(CFAllocatorRef, uintptr_t))kCFTypeDictionaryKeyCallBacks.release;
    callbacks.equateKeys = (Boolean (*)(uintptr_t, uintptr_t))kCFTypeDictionaryKeyCallBacks.equal;
    callbacks.hashKey = (CFHashCode (*)(uintptr_t))kCFTypeDictionaryKeyCallBacks.hash;
    callbacks.getIndirectKey = NULL;
    callbacks.copyKeyDescription = (CFStringRef (*)(uintptr_t))kCFTypeDictionaryKeyCallBacks.copyDescription;
    callbacks.retainValue = (uintptr_t (*)(CFAllocatorRef, uintptr_t))kCFTypeDictionaryValueCallBacks.retain;
    callbacks.releaseValue = (void (*)(CFAllocatorRef, uintptr_t))kCFTypeDictionaryValueCallBacks.release;
    callbacks.equateValues = (Boolean (*)(uintptr_t, uintptr_t))kCFTypeDictionaryValueCallBacks.equal;
    callbacks.copyValueDescription = (CFStringRef (*)(uintptr_t))kCFTypeDictionaryValueCallBacks.copyDescription;
    
    CFBasicHashRef ht = CFBasicHashCreate(allocator, flags, &callbacks);
    CFBasicHashSuppressRC(ht);
    if (0 < numValues) CFBasicHashSetCapacity(ht, numValues);
    for (CFIndex idx = 0; idx < numValues; idx++) {
        CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
    }
    CFBasicHashUnsuppressRC(ht);
    CFBasicHashMakeImmutable(ht);
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFDictionary (immutable)");
    return (CFDictionaryRef)ht;
}

CFDictionaryRef CFDictionaryCreate(CFAllocatorRef allocator, void const **klist, void const **vlist, CFIndex numValues, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks) {
    CFTypeID typeID = _kCFRuntimeIDCFDictionary;
    CFAssert2(0 <= numValues, __kCFLogAssertion, "%s(): numValues (%ld) cannot be less than zero", __PRETTY_FUNCTION__, numValues);
    CFBasicHashRef ht = __CFDictionaryCreateGeneric(allocator, keyCallBacks, valueCallBacks, true);
    if (!ht) return NULL;
    if (0 < numValues) CFBasicHashSetCapacity(ht, numValues);
    for (CFIndex idx = 0; idx < numValues; idx++) {
        CFBasicHashAddValue(ht, (uintptr_t)klist[idx], (uintptr_t)vlist[idx]);
    }
    CFBasicHashMakeImmutable(ht);
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFDictionary (immutable)");
    return (CFDictionaryRef)ht;
}

CFMutableDictionaryRef CFDictionaryCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks) {
    CFTypeID typeID = _kCFRuntimeIDCFDictionary;
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFBasicHashRef ht = __CFDictionaryCreateGeneric(allocator, keyCallBacks, valueCallBacks, true);
    if (!ht) return NULL;
    if (capacity > 0) {
        if (capacity > 1000) capacity = 1000;
        CFBasicHashSetCapacity(ht, capacity);
    }
    _CFRuntimeSetInstanceTypeIDAndIsa(ht, typeID);
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFDictionary (mutable)");
    return (CFMutableDictionaryRef)ht;
}

CFDictionaryRef CFDictionaryCreateCopy(CFAllocatorRef allocator, CFDictionaryRef other) {
    CFTypeID typeID = _kCFRuntimeIDCFDictionary;
    CFAssert1(other, __kCFLogAssertion, "%s(): other CFDictionary cannot be NULL", __PRETTY_FUNCTION__);
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
        if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFDictionary (immutable)");
        return (CFDictionaryRef)ht;
    }
    return (CFDictionaryRef)ht;
}

CFMutableDictionaryRef CFDictionaryCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFDictionaryRef other) {
    CFTypeID typeID = _kCFRuntimeIDCFDictionary;
    CFAssert1(other, __kCFLogAssertion, "%s(): other CFDictionary cannot be NULL", __PRETTY_FUNCTION__);
    __CFGenericValidateType(other, typeID);
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFBasicHashRef ht = NULL;
    Boolean const isObjC = CF_IS_OBJC(typeID, other);
    if (isObjC && _CFAllocatorIsSystemDefault(allocator)) {
        return (CFMutableDictionaryRef)CF_OBJC_CALLV((id)other, _cfMutableCopy);
    }
    if (isObjC || CF_IS_SWIFT(typeID, other)) {
        CFIndex numValues = CFDictionaryGetCount(other);
        void const *vbuffer[256], *kbuffer[256];
        void const **vlist = (numValues <= 256) ? vbuffer : (void const **)CFAllocatorAllocate(kCFAllocatorSystemDefault, numValues * sizeof(void const *), 0);
        void const **klist = (numValues <= 256) ? kbuffer : (void const **)CFAllocatorAllocate(kCFAllocatorSystemDefault, numValues * sizeof(void const *), 0);
        CFDictionaryGetKeysAndValues(other, klist, vlist);
        ht = __CFDictionaryCreateGeneric(allocator, & kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks, true);
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
    if (__CFOASafe) __CFSetLastAllocationEventName(ht, "CFDictionary (mutable)");
    return (CFMutableDictionaryRef)ht;
}

CFIndex CFDictionaryGetCount(CFDictionaryRef hc) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), CFIndex, (CFSwiftRef)hc, NSDictionary.count);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, CFIndex, (NSDictionary *)hc, count);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    return CFBasicHashGetCount((CFBasicHashRef)hc);
}

CFIndex CFDictionaryGetCountOfKey(CFDictionaryRef hc, void const *key) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), CFIndex, (CFSwiftRef)hc, NSDictionary.countForKey, key);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, CFIndex, (NSDictionary *)hc, countForKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    return CFBasicHashGetCountOfKey((CFBasicHashRef)hc, (uintptr_t)key);
}

Boolean CFDictionaryContainsKey(CFDictionaryRef hc, void const *key) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), char, (CFSwiftRef)hc, NSDictionary.containsKey, key);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, char, (NSDictionary *)hc, containsKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    return (0 < CFBasicHashGetCountOfKey((CFBasicHashRef)hc, (uintptr_t)key));
}

void const *CFDictionaryGetValue(CFDictionaryRef hc, void const *key) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void const *, (CFSwiftRef)hc, NSDictionary.objectForKey, key);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void const *, (NSDictionary *)hc, objectForKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFBasicHashBucket bkt = CFBasicHashFindBucket((CFBasicHashRef)hc, (uintptr_t)key);
    return (0 < bkt.count ? (void const *)bkt.weak_value : 0);
}

Boolean CFDictionaryGetValueIfPresent(CFDictionaryRef hc, void const *key, void const **value) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), Boolean, (CFSwiftRef)hc, NSDictionary.__getValue, value, key);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, Boolean, (NSDictionary *)hc, __getValue:(id *)value forKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFBasicHashBucket bkt = CFBasicHashFindBucket((CFBasicHashRef)hc, (uintptr_t)key);
    if (0 < bkt.count) {
        if (value) {
            *value = (void const *)bkt.weak_value;
        }
        return true;
    }
    return false;
}

CFIndex CFDictionaryGetCountOfValue(CFDictionaryRef hc, void const *value) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), CFIndex, (CFSwiftRef)hc, NSDictionary.count);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, CFIndex, (NSDictionary *)hc, countForObject:(id)value);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    return CFBasicHashGetCountOfValue((CFBasicHashRef)hc, (uintptr_t)value);
}

Boolean CFDictionaryContainsValue(CFDictionaryRef hc, void const *value) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), char, (CFSwiftRef)hc, NSDictionary.containsObject, value);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, char, (NSDictionary *)hc, containsObject:(id)value);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    return (0 < CFBasicHashGetCountOfValue((CFBasicHashRef)hc, (uintptr_t)value));
}

CF_EXPORT Boolean CFDictionaryGetKeyIfPresent(CFDictionaryRef hc, void const *key, void const **actualkey) {
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFBasicHashBucket bkt = CFBasicHashFindBucket((CFBasicHashRef)hc, (uintptr_t)key);
    if (0 < bkt.count) {
        if (actualkey) {
            *actualkey = (void const *)bkt.weak_key;
        }
        return true;
    }
    return false;
}

void CFDictionaryGetKeysAndValues(CFDictionaryRef hc, void const **keybuf, void const **valuebuf) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSDictionary.getObjects, valuebuf, keybuf);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSDictionary *)hc, getObjects:(id *)valuebuf andKeys:(id *)keybuf);
#pragma GCC diagnostic pop
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFBasicHashGetElements((CFBasicHashRef)hc, CFDictionaryGetCount(hc), (uintptr_t *)valuebuf, (uintptr_t *)keybuf);
}

void CFDictionaryApplyFunction(CFDictionaryRef hc, CFDictionaryApplierFunction applier, void *context) {
    FAULT_CALLBACK((void **)&(applier));
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSDictionary.__apply, applier, context);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSDictionary *)hc, __apply:(void (*)(const void *, const void *, void *))applier context:(void *)context);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFBasicHashApply((CFBasicHashRef)hc, ^(CFBasicHashBucket bkt) {
        INVOKE_CALLBACK3(applier, (void const *)bkt.weak_key, (void const *)bkt.weak_value, context);
        return (Boolean)true;
    });
}

CF_PRIVATE void CFDictionaryApply(CFDictionaryRef hc, void (^block)(const void *key, const void *value, Boolean *stop)) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSDictionary.enumerateKeysAndObjectsWithOptions, 0, block);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSDictionary *)hc, enumerateKeysAndObjectsWithOptions:0 usingBlock:(void (^ _Nonnull)(id _Nonnull, id _Nonnull, BOOL * _Nonnull))block);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFBasicHashApply((CFBasicHashRef)hc, ^(CFBasicHashBucket bkt) {
        Boolean stop = false;
        block((void const *)bkt.weak_key, (void const *)bkt.weak_value, &stop);
        if (stop) return (Boolean)false;
        return (Boolean)true;
    });
}
    
// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT unsigned long _CFDictionaryFastEnumeration(CFDictionaryRef hc, struct __objcFastEnumerationStateEquivalent *state, void *stackbuffer, unsigned long count) {
    if (CF_IS_SWIFT(_kCFRuntimeIDCFDictionary, hc)) return 0;
    if (CF_IS_OBJC(_kCFRuntimeIDCFDictionary, hc)) return 0;
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    return __CFBasicHashFastEnumeration((CFBasicHashRef)hc, (struct __objcFastEnumerationStateEquivalent2 *)state, stackbuffer, count);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT Boolean _CFDictionaryIsMutable(CFDictionaryRef hc) {
    if (CF_IS_SWIFT(_kCFRuntimeIDCFDictionary, hc)) return false;
    if (CF_IS_OBJC(_kCFRuntimeIDCFDictionary, hc)) return false;
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    return CFBasicHashIsMutable((CFBasicHashRef)hc);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT void _CFDictionarySetCapacity(CFMutableDictionaryRef hc, CFIndex cap) {
    if (CF_IS_SWIFT(_kCFRuntimeIDCFDictionary, hc)) return;
    if (CF_IS_OBJC(_kCFRuntimeIDCFDictionary, hc)) return;
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    CFAssert3(CFDictionaryGetCount(hc) <= cap, __kCFLogAssertion, "%s(): desired capacity (%ld) is less than count (%ld)", __PRETTY_FUNCTION__, cap, CFDictionaryGetCount(hc));
    CFBasicHashSetCapacity((CFBasicHashRef)hc, cap);
}

CF_INLINE CFIndex __CFDictionaryGetKVOBit(CFDictionaryRef hc) {
    return __CFRuntimeGetFlag(hc, 0);
}

CF_INLINE void __CFDictionarySetKVOBit(CFDictionaryRef hc, CFIndex bit) {
    __CFRuntimeSetFlag(hc, 0, ((uintptr_t)bit & 0x1));
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT CFIndex _CFDictionaryGetKVOBit(CFDictionaryRef hc) {
    return __CFDictionaryGetKVOBit(hc);
}

// This function is for Foundation's benefit; no one else should use it.
CF_EXPORT void _CFDictionarySetKVOBit(CFDictionaryRef hc, CFIndex bit) {
    __CFDictionarySetKVOBit(hc, bit);
}


#if !defined(CF_OBJC_KVO_WILLCHANGE)
#define CF_OBJC_KVO_WILLCHANGE(obj, key)
#define CF_OBJC_KVO_DIDCHANGE(obj, key)
#define CF_OBJC_KVO_WILLCHANGEALL(obj)
#define CF_OBJC_KVO_DIDCHANGEALL(obj)
#endif

void CFDictionaryAddValue(CFMutableDictionaryRef hc, void const *key, void const *value) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSMutableDictionary.__addObject, key, value);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSMutableDictionary *)hc, __addObject:(id)value forKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CF_OBJC_KVO_WILLCHANGE(hc, key);
    CFBasicHashAddValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
    CF_OBJC_KVO_DIDCHANGE(hc, key);
}

void CFDictionaryReplaceValue(CFMutableDictionaryRef hc, void const *key, void const *value) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSMutableDictionary.replaceObject, key, value);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSMutableDictionary *)hc, replaceObject:(id)value forKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CF_OBJC_KVO_WILLCHANGE(hc, key);
    CFBasicHashReplaceValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
    CF_OBJC_KVO_DIDCHANGE(hc, key);
}

void CFDictionarySetValue(CFMutableDictionaryRef hc, void const *key, void const *value) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSMutableDictionary.__setObject, key, value);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSMutableDictionary *)hc, __setObject:(id)value forKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CF_OBJC_KVO_WILLCHANGE(hc, key);
    //#warning this for a dictionary used to not replace the key
    CFBasicHashSetValue((CFBasicHashRef)hc, (uintptr_t)key, (uintptr_t)value);
    CF_OBJC_KVO_DIDCHANGE(hc, key);
}

void CFDictionaryRemoveValue(CFMutableDictionaryRef hc, void const *key) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSMutableDictionary.removeObjectForKey, key);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSMutableDictionary *)hc, removeObjectForKey:(id)key);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CF_OBJC_KVO_WILLCHANGE(hc, key);
    CFBasicHashRemoveValue((CFBasicHashRef)hc, (uintptr_t)key);
    CF_OBJC_KVO_DIDCHANGE(hc, key);
}

void CFDictionaryRemoveAllValues(CFMutableDictionaryRef hc) {
    CF_SWIFT_FUNCDISPATCHV(CFDictionaryGetTypeID(), void, (CFSwiftRef)hc, NSMutableDictionary.removeAllObjects);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFDictionary, void, (NSMutableDictionary *)hc, removeAllObjects);
    __CFGenericValidateType(hc, CFDictionaryGetTypeID());
    CFAssert2(CFBasicHashIsMutable((CFBasicHashRef)hc), __kCFLogAssertion, "%s(): immutable collection %p passed to mutating operation", __PRETTY_FUNCTION__, hc);
    if (!CFBasicHashIsMutable((CFBasicHashRef)hc)) {
        CFLog(3, CFSTR("%s(): immutable collection %p given to mutating function"), __PRETTY_FUNCTION__, hc);
    }
    CF_OBJC_KVO_WILLCHANGEALL(hc);
    CFBasicHashRemoveAllValues((CFBasicHashRef)hc);
    CF_OBJC_KVO_DIDCHANGEALL(hc);
}

#undef CF_OBJC_KVO_WILLCHANGE
#undef CF_OBJC_KVO_DIDCHANGE
#undef CF_OBJC_KVO_WILLCHANGEALL
#undef CF_OBJC_KVO_DIDCHANGEALL

