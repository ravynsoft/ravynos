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

/*	CFRuntime.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#define ENABLE_ZOMBIES 1

#include <CoreFoundation/CFRuntime.h>
#include "CFInternal.h"
#include "CFBasicHash.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CFUUID.h>
#include <CoreFoundation/CFCalendar.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <crt_externs.h>
#include <unistd.h>
#include <sys/stat.h>
#include <CoreFoundation/CFStringDefaultEncoding.h>
#endif
#if DEPLOYMENT_TARGET_EMBEDDED
// This isn't in the embedded runtime.h header
OBJC_EXPORT void *objc_destructInstance(id obj);
#endif


#if DEPLOYMENT_TARGET_WINDOWS
#include <Shellapi.h>
#endif

enum {
// retain/release recording constants -- must match values
// used by OA for now; probably will change in the future
__kCFRetainEvent = 28,
__kCFReleaseEvent = 29
};

#if DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#include <malloc.h>
#else
#include <malloc/malloc.h>
#endif

#define FAKE_INSTRUMENTS 0

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
CF_PRIVATE void __CFOAInitializeNSObject(void);  // from NSObject.m

bool __CFOASafe = false;

void (*__CFObjectAllocRecordAllocationFunction)(int, void *, int64_t , uint64_t, const char *) = NULL;
void (*__CFObjectAllocSetLastAllocEventNameFunction)(void *, const char *) = NULL;

void __CFOAInitialize(void) {
}

void __CFRecordAllocationEvent(int eventnum, void *ptr, int64_t size, uint64_t data, const char *classname) {
    if (!__CFOASafe || !__CFObjectAllocRecordAllocationFunction) return;
    __CFObjectAllocRecordAllocationFunction(eventnum, ptr, size, data, classname);
}

void __CFSetLastAllocationEventName(void *ptr, const char *classname) {
    if (!__CFOASafe || !__CFObjectAllocSetLastAllocEventNameFunction) return;
    __CFObjectAllocSetLastAllocEventNameFunction(ptr, classname);
}

#elif FAKE_INSTRUMENTS

CF_EXPORT bool __CFOASafe = true;

void __CFOAInitialize(void) { }

void __CFRecordAllocationEvent(int eventnum, void *ptr, int64_t size, uint64_t data, const char *classname) {
    if (!__CFOASafe) return;
    if (!classname) classname = "(no class)";
    const char *event = "unknown event";
    switch (eventnum) {
        case 21:
            event = "zombie";
            break;
        case 13:
        case __kCFReleaseEvent:
            event = "release";
            break;
        case 12:
        case __kCFRetainEvent:
            event = "retain";
            break;
    }
    fprintf(stdout, "event,%d,%s,%p,%ld,%lu,%s\n", eventnum, event, ptr, (long)size, (unsigned long)data, classname);
}

void __CFSetLastAllocationEventName(void *ptr, const char *classname) {
    if (!__CFOASafe) return;
    if (!classname) classname = "(no class)";
    fprintf(stdout, "name,%p,%s\n", ptr, classname ? classname : "(no class)");
}

#else

bool __CFOASafe = false;

void __CFOAInitialize(void) { }
void __CFRecordAllocationEvent(int eventnum, void *ptr, int64_t size, uint64_t data, const char *classname) { }
void __CFSetLastAllocationEventName(void *ptr, const char *classname) { }

#endif

extern void __HALT(void);

static CFTypeID __kCFNotATypeTypeID = _kCFRuntimeNotATypeID;

#if !defined (__cplusplus)
static const CFRuntimeClass __CFNotATypeClass = {
    0,
    "Not A Type",
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT
};

static CFTypeID __kCFTypeTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFTypeClass = {
    0,
    "CFType",
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT,
    (void *)__HALT
};
#else
void SIG1(CFTypeRef){__HALT();};;
CFTypeRef SIG2(CFAllocatorRef,CFTypeRef){__HALT();return NULL;};
Boolean SIG3(CFTypeRef,CFTypeRef){__HALT();return FALSE;};
CFHashCode SIG4(CFTypeRef){__HALT(); return 0;};
CFStringRef SIG5(CFTypeRef,CFDictionaryRef){__HALT();return NULL;};
CFStringRef SIG6(CFTypeRef){__HALT();return NULL;};

static const CFRuntimeClass __CFNotATypeClass = {
    0,
    "Not A Type",
    SIG1,
    SIG2,
    SIG1,
    SIG3,
    SIG4,
    SIG5,
    SIG6
};

static CFTypeID __kCFTypeTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFTypeClass = {
    0,
    "CFType",
    SIG1,
    SIG2,
    SIG1,
    SIG3,
    SIG4,
    SIG5,
    SIG6
};
#endif //__cplusplus

// the lock does not protect most reading of these; we just leak the old table to allow read-only accesses to continue to work
static CFLock_t __CFBigRuntimeFunnel = CFLockInit;
CF_PRIVATE CFRuntimeClass * __CFRuntimeClassTable[__CFRuntimeClassTableSize] = {0};
CF_PRIVATE int32_t __CFRuntimeClassTableCount = 0;

CF_PRIVATE uintptr_t __CFRuntimeObjCClassTable[__CFRuntimeClassTableSize] = {0};

#if !defined(__CFObjCIsCollectable)
bool (*__CFObjCIsCollectable)(void *) = NULL;
#endif

#if !__CONSTANT_CFSTRINGS__ || DEPLOYMENT_TARGET_EMBEDDED_MINI
// Compiler uses this symbol name; must match compiler built-in decl, so we use 'int'
#if __LP64__
int __CFConstantStringClassReference[24] = {0};
#else
int __CFConstantStringClassReference[12] = {0};
#endif
#endif

#if __LP64__
int __CFConstantStringClassReference[24] = {0};
#else
int __CFConstantStringClassReference[12] = {0};
#endif

void *__CFConstantStringClassReferencePtr = NULL;

Boolean _CFIsObjC(CFTypeID typeID, void *obj) {
    return CF_IS_OBJC(typeID, obj);
}

CFTypeID _CFRuntimeRegisterClass(const CFRuntimeClass * const cls) {
// className must be pure ASCII string, non-null
    if ((cls->version & _kCFRuntimeCustomRefCount) && !cls->refcount) {
       CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeRegisterClass() given inconsistent class '%s'.  Program will crash soon."), cls->className);
       return _kCFRuntimeNotATypeID;
    }
    __CFLock(&__CFBigRuntimeFunnel);
    if (__CFMaxRuntimeTypes <= __CFRuntimeClassTableCount) {
	CFLog(kCFLogLevelWarning, CFSTR("*** CoreFoundation class table full; registration failing for class '%s'.  Program will crash soon."), cls->className);
        __CFUnlock(&__CFBigRuntimeFunnel);
	return _kCFRuntimeNotATypeID;
    }
    if (__CFRuntimeClassTableSize <= __CFRuntimeClassTableCount) {
	CFLog(kCFLogLevelWarning, CFSTR("*** CoreFoundation class table full; registration failing for class '%s'.  Program will crash soon."), cls->className);
        __CFUnlock(&__CFBigRuntimeFunnel);
	return _kCFRuntimeNotATypeID;
    }
    __CFRuntimeClassTable[__CFRuntimeClassTableCount++] = (CFRuntimeClass *)cls;
    CFTypeID typeID = __CFRuntimeClassTableCount - 1;
    __CFUnlock(&__CFBigRuntimeFunnel);
    return typeID;
}


const CFRuntimeClass * _CFRuntimeGetClassWithTypeID(CFTypeID typeID) {
    return __CFRuntimeClassTable[typeID]; // hopelessly unthreadsafe
}

void _CFRuntimeUnregisterClassWithTypeID(CFTypeID typeID) {
    __CFLock(&__CFBigRuntimeFunnel);
    __CFRuntimeClassTable[typeID] = NULL;
    __CFUnlock(&__CFBigRuntimeFunnel);
}


#if defined(DEBUG) || defined(ENABLE_ZOMBIES)

CF_PRIVATE uint8_t __CFZombieEnabled = 0;
CF_PRIVATE uint8_t __CFDeallocateZombies = 0;

extern void __CFZombifyNSObject(void);  // from NSObject.m

void _CFEnableZombies(void) {
}

#endif /* DEBUG */

// XXX_PCB:  use the class version field as a bitmask, to allow classes to opt-in for GC scanning.

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
CF_INLINE CFOptionFlags CF_GET_COLLECTABLE_MEMORY_TYPE(const CFRuntimeClass *cls)
{
    return ((cls->version & _kCFRuntimeScannedObject) ? __kCFAllocatorGCScannedMemory : 0) | __kCFAllocatorGCObjectMemory;
}
#else
#define CF_GET_COLLECTABLE_MEMORY_TYPE(x) (0)
#endif

CFTypeRef _CFRuntimeCreateInstance(CFAllocatorRef allocator, CFTypeID typeID, CFIndex extraBytes, unsigned char *category) {
    if (__CFRuntimeClassTableSize <= typeID) HALT;
    CFAssert1(typeID != _kCFRuntimeNotATypeID, __kCFLogAssertion, "%s(): Uninitialized type id", __PRETTY_FUNCTION__);
    CFRuntimeClass *cls = __CFRuntimeClassTable[typeID];
    if (NULL == cls) {
	return NULL;
    }
    if (cls->version & _kCFRuntimeRequiresAlignment) {
        allocator = kCFAllocatorSystemDefault;
    }
    Boolean customRC = !!(cls->version & _kCFRuntimeCustomRefCount);
    if (customRC && !cls->refcount) {
        CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeCreateInstance() found inconsistent class '%s'."), cls->className);
        return NULL;
    }
    CFAllocatorRef realAllocator = (NULL == allocator) ? __CFGetDefaultAllocator() : allocator;
    if (kCFAllocatorNull == realAllocator) {
	return NULL;
    }
    Boolean usesSystemDefaultAllocator = _CFAllocatorIsSystemDefault(realAllocator);
    size_t align = (cls->version & _kCFRuntimeRequiresAlignment) ? cls->requiredAlignment : 16;
    CFIndex size = sizeof(CFRuntimeBase) + extraBytes + (usesSystemDefaultAllocator ? 0 : sizeof(CFAllocatorRef));
    size = (size + 0xF) & ~0xF;	// CF objects are multiples of 16 in size
    // CFType version 0 objects are unscanned by default since they don't have write-barriers and hard retain their innards
    // CFType version 1 objects are scanned and use hand coded write-barriers to store collectable storage within
    CFRuntimeBase *memory = NULL;
    if (cls->version & _kCFRuntimeRequiresAlignment) {
        memory = malloc_zone_memalign(malloc_default_zone(), align, size);
    } else {
        memory = (CFRuntimeBase *)CFAllocatorAllocate(allocator, size, CF_GET_COLLECTABLE_MEMORY_TYPE(cls));
    }
    if (NULL == memory) {
	return NULL;
    }
    if (!kCFUseCollectableAllocator || !CF_IS_COLLECTABLE_ALLOCATOR(allocator) || !(CF_GET_COLLECTABLE_MEMORY_TYPE(cls) & __kCFAllocatorGCScannedMemory)) {
	memset(memory, 0, size);
    }
    if (__CFOASafe && category) {
	__CFSetLastAllocationEventName(memory, (char *)category);
    } else if (__CFOASafe) {
	__CFSetLastAllocationEventName(memory, (char *)cls->className);
    }
    if (!usesSystemDefaultAllocator) {
        // add space to hold allocator ref for non-standard allocators.
        // (this screws up 8 byte alignment but seems to work)
	*(CFAllocatorRef *)((char *)memory) = (CFAllocatorRef)CFRetain(realAllocator);
	memory = (CFRuntimeBase *)((char *)memory + sizeof(CFAllocatorRef));
    }
    uint32_t rc = 0;
#if __LP64__
    if (!kCFUseCollectableAllocator || (1 && 1)) {
        memory->_rc = 1;
    }
    if (customRC) {
        memory->_rc = 0xFFFFFFFFU;
        rc = 0xFF;
    }
#else
    if (!kCFUseCollectableAllocator || (1 && 1)) {
        rc = 1;
    }
    if (customRC) {
        rc = 0xFF;
    }
#endif
    uint32_t *cfinfop = (uint32_t *)&(memory->_cfinfo);
    *cfinfop = (uint32_t)((rc << 24) | (customRC ? 0x800000 : 0x0) | ((uint32_t)typeID << 8) | (usesSystemDefaultAllocator ? 0x80 : 0x00));
    memory->_cfisa = 0;
    if (NULL != cls->init) {
	(cls->init)(memory);
    }
    return memory;
}

void _CFRuntimeInitStaticInstance(void *ptr, CFTypeID typeID) {
    CFAssert1(typeID != _kCFRuntimeNotATypeID, __kCFLogAssertion, "%s(): Uninitialized type id", __PRETTY_FUNCTION__);
    if (__CFRuntimeClassTableSize <= typeID) HALT;
    CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
    Boolean customRC = !!(cfClass->version & _kCFRuntimeCustomRefCount);
    if (customRC) {
        CFLog(kCFLogLevelError, CFSTR("*** Cannot initialize a static instance to a class (%s) with custom ref counting"), cfClass->className);
        return;
    }
    CFRuntimeBase *memory = (CFRuntimeBase *)ptr;
    uint32_t *cfinfop = (uint32_t *)&(memory->_cfinfo);
    *cfinfop = (uint32_t)(((customRC ? 0xFF : 0) << 24) | (customRC ? 0x800000 : 0x0) | ((uint32_t)typeID << 8) | 0x80);
#if __LP64__
    memory->_rc = customRC ? 0xFFFFFFFFU : 0x0;
#endif
    memory->_cfisa = 0;
    if (NULL != cfClass->init) {
       (cfClass->init)(memory);
    }
}

void _CFRuntimeSetInstanceTypeID(CFTypeRef cf, CFTypeID newTypeID) {
    if (__CFRuntimeClassTableSize <= newTypeID) HALT;
    uint32_t *cfinfop = (uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    CFTypeID currTypeID = (*cfinfop >> 8) & 0x03FF; // mask up to 0x0FFF
    CFRuntimeClass *newcfClass = __CFRuntimeClassTable[newTypeID];
    Boolean newCustomRC = (newcfClass->version & _kCFRuntimeCustomRefCount);
    CFRuntimeClass *currcfClass = __CFRuntimeClassTable[currTypeID];
    Boolean currCustomRC = (currcfClass->version & _kCFRuntimeCustomRefCount);
    if (currCustomRC || (0 != currTypeID && newCustomRC)) {
        CFLog(kCFLogLevelError, CFSTR("*** Cannot change the CFTypeID of a %s to a %s due to custom ref counting"), currcfClass->className, newcfClass->className);
        return;
    }
    // Going from current type ID of 0 to anything is allowed, but if
    // the object has somehow already been retained and the transition
    // is to a class doing custom ref counting, the ref count isn't
    // transferred and there will probably be a crash later when the
    // object is freed too early.
    *cfinfop = (*cfinfop & 0xFFF000FFU) | ((uint32_t)newTypeID << 8);
}

CF_PRIVATE void _CFRuntimeSetInstanceTypeIDAndIsa(CFTypeRef cf, CFTypeID newTypeID) {
    _CFRuntimeSetInstanceTypeID(cf, newTypeID);
}


enum {
    __kCFObjectRetainedEvent = 12,
    __kCFObjectReleasedEvent = 13
};

#if DEPLOYMENT_TARGET_MACOSX
#define NUM_EXTERN_TABLES 8
#define EXTERN_TABLE_IDX(O) (((uintptr_t)(O) >> 8) & 0x7)
#elif DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#define NUM_EXTERN_TABLES 1
#define EXTERN_TABLE_IDX(O) 0
#else
#error
#endif

// we disguise pointers so that programs like 'leaks' forget about these references
#define DISGUISE(O) (~(uintptr_t)(O))

static struct {
    CFLock_t lock;
    CFBasicHashRef table;
//    uint8_t padding[64 - sizeof(CFBasicHashRef) - sizeof(CFLock_t)];
} __NSRetainCounters[NUM_EXTERN_TABLES];

CF_EXPORT uintptr_t __CFDoExternRefOperation(uintptr_t op, id obj) {
    if (nil == obj) HALT;
    uintptr_t idx = EXTERN_TABLE_IDX(obj);
    uintptr_t disguised = DISGUISE(obj);
    CFLock_t *lock = &__NSRetainCounters[idx].lock;
    CFBasicHashRef table = __NSRetainCounters[idx].table;
    uintptr_t count;
    switch (op) {
    case 300:   // increment
    case 350:   // increment, no event
        __CFLock(lock);
	CFBasicHashAddValue(table, disguised, disguised);
        __CFUnlock(lock);
        if (__CFOASafe && op != 350) __CFRecordAllocationEvent(__kCFObjectRetainedEvent, obj, 0, 0, NULL);
        return (uintptr_t)obj;
    case 400:   // decrement
        if (__CFOASafe) __CFRecordAllocationEvent(__kCFObjectReleasedEvent, obj, 0, 0, NULL);
    case 450:   // decrement, no event
        __CFLock(lock);
        count = (uintptr_t)CFBasicHashRemoveValue(table, disguised);
        __CFUnlock(lock);
        return 0 == count;
    case 500:
        __CFLock(lock);
        count = (uintptr_t)CFBasicHashGetCountOfKey(table, disguised);
        __CFUnlock(lock);
        return count;
    }
    return 0;
}

CF_EXPORT CFTypeID CFNumberGetTypeID(void);

CF_INLINE CFTypeID __CFGenericTypeID_inline(const void *cf) {
    // yes, 10 bits masked off, though 12 bits are there for the type field; __CFRuntimeClassTableSize is 1024
    uint32_t *cfinfop = (uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    CFTypeID typeID = (*cfinfop >> 8) & 0x03FF; // mask up to 0x0FFF
    return typeID;
}

CFTypeID __CFGenericTypeID(const void *cf) {
    return __CFGenericTypeID_inline(cf);
}

CFTypeID CFTypeGetTypeID(void) {
    return __kCFTypeTypeID;
}

CF_PRIVATE void __CFGenericValidateType_(CFTypeRef cf, CFTypeID type, const char *func) {
    if (cf && CF_IS_OBJC(type, cf)) return;
    CFAssert2((cf != NULL) && (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]) && (__kCFNotATypeTypeID != __CFGenericTypeID_inline(cf)) && (__kCFTypeTypeID != __CFGenericTypeID_inline(cf)), __kCFLogAssertion, "%s(): pointer %p is not a CF object", func, cf); \
    CFAssert3(__CFGenericTypeID_inline(cf) == type, __kCFLogAssertion, "%s(): pointer %p is not a %s", func, cf, __CFRuntimeClassTable[type]->className);	\
}

#define __CFGenericAssertIsCF(cf) \
    CFAssert2(cf != NULL && (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]) && (__kCFNotATypeTypeID != __CFGenericTypeID_inline(cf)) && (__kCFTypeTypeID != __CFGenericTypeID_inline(cf)), __kCFLogAssertion, "%s(): pointer %p is not a CF object", __PRETTY_FUNCTION__, cf);


#define CFTYPE_IS_OBJC(obj) (false)
#define CFTYPE_OBJC_FUNCDISPATCH0(rettype, obj, sel) do {} while (0)
#define CFTYPE_OBJC_FUNCDISPATCH1(rettype, obj, sel, a1) do {} while (0)


CFTypeID CFGetTypeID(CFTypeRef cf) {
#if defined(DEBUG)
    if (NULL == cf) { CRSetCrashLogMessage("*** CFGetTypeID() called with NULL ***"); HALT; }
#endif
    CFTYPE_OBJC_FUNCDISPATCH0(CFTypeID, cf, _cfTypeID);
    __CFGenericAssertIsCF(cf);
    return __CFGenericTypeID_inline(cf);
}

CFStringRef CFCopyTypeIDDescription(CFTypeID type) {
    CFAssert2((NULL != __CFRuntimeClassTable[type]) && __kCFNotATypeTypeID != type && __kCFTypeTypeID != type, __kCFLogAssertion, "%s(): type %d is not a CF type ID", __PRETTY_FUNCTION__, type);
    return CFStringCreateWithCString(kCFAllocatorSystemDefault, __CFRuntimeClassTable[type]->className, kCFStringEncodingASCII);
}

// Bit 31 (highest bit) in second word of cf instance indicates external ref count

static CFTypeRef _CFRetain(CFTypeRef cf, Boolean tryR);

CFTypeRef CFRetain(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFRetain() called with NULL ***"); HALT; }
    if (cf) __CFGenericAssertIsCF(cf);
    return _CFRetain(cf, false);
}

CFTypeRef CFAutorelease(CFTypeRef __attribute__((cf_consumed)) cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFAutorelease() called with NULL ***"); HALT; }
    return cf;
}

static void _CFRelease(CFTypeRef cf);

void CFRelease(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFRelease() called with NULL ***"); HALT; }
#if 0
    void **addrs[2] = {&&start, &&end};
    start:;
    if (addrs[0] <= __builtin_return_address(0) && __builtin_return_address(0) <= addrs[1]) {
	CFLog(3, CFSTR("*** WARNING: Recursion in CFRelease(%p) : %p '%s' : 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx 0x%08lx"), cf, object_getClass(cf), object_getClassName(cf), ((uintptr_t *)cf)[0], ((uintptr_t *)cf)[1], ((uintptr_t *)cf)[2], ((uintptr_t *)cf)[3], ((uintptr_t *)cf)[4], ((uintptr_t *)cf)[5]);
	HALT;
    }
#endif
    if (cf) __CFGenericAssertIsCF(cf);
    _CFRelease(cf);
#if 0
    end:;
#endif
}


CF_PRIVATE void __CFAllocatorDeallocate(CFTypeRef cf);

CF_PRIVATE const void *__CFStringCollectionCopy(CFAllocatorRef allocator, const void *ptr) {
    if (NULL == ptr) { CRSetCrashLogMessage("*** __CFStringCollectionCopy() called with NULL ***"); HALT; }
    CFStringRef theString = (CFStringRef)ptr;
    CFStringRef result = CFStringCreateCopy((allocator), theString);
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        result = (CFStringRef)CFMakeCollectable(result);
    }
    return (const void *)result;
}

extern void CFCollection_non_gc_storage_error(void);

CF_PRIVATE const void *__CFTypeCollectionRetain(CFAllocatorRef allocator, const void *ptr) {
    if (NULL == ptr) { CRSetCrashLogMessage("*** __CFTypeCollectionRetain() called with NULL; likely a collection has been corrupted ***"); HALT; }
    CFTypeRef cf = (CFTypeRef)ptr;
    // only collections allocated in the GC zone can opt-out of reference counting.
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        if (CFTYPE_IS_OBJC(cf)) return cf;  // do nothing for OBJC objects.
        if (auto_zone_is_valid_pointer(objc_collectableZone(), ptr)) {
            CFRuntimeClass *cfClass = __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)];
            if (cfClass->version & _kCFRuntimeResourcefulObject) {
                // GC: If this a CF object in the GC heap that is marked resourceful, then
                // it must be retained keep it alive in a CF collection.
                CFRetain(cf);
            }
            else
                ;   // don't retain normal CF objects
            return cf;
        } else {
            // support constant CFTypeRef objects.
#if __LP64__
            uint32_t lowBits = ((CFRuntimeBase *)cf)->_rc;
#else
            uint32_t lowBits = ((CFRuntimeBase *)cf)->_cfinfo[CF_RC_BITS];
#endif
            if (lowBits == 0) return cf;
            // complain about non-GC objects in GC containers.
            CFLog(kCFLogLevelWarning, CFSTR("storing a non-GC object %p in a GC collection, break on CFCollection_non_gc_storage_error to debug."), cf);
            CFCollection_non_gc_storage_error();
            // XXX should halt, except Patrick is using this somewhere.
            // HALT;
        }
    }
    return CFRetain(cf);
}


CF_PRIVATE void __CFTypeCollectionRelease(CFAllocatorRef allocator, const void *ptr) {
    if (NULL == ptr) { CRSetCrashLogMessage("*** __CFTypeCollectionRelease() called with NULL; likely a collection has been corrupted ***"); HALT; }
    CFTypeRef cf = (CFTypeRef)ptr;
    // only collections allocated in the GC zone can opt-out of reference counting.
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        if (CFTYPE_IS_OBJC(cf)) return; // do nothing for OBJC objects.
        if (auto_zone_is_valid_pointer(objc_collectableZone(), cf)) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
            // GC: If this a CF object in the GC heap that is marked uncollectable, then
            // must balance the retain done in __CFTypeCollectionRetain().
            CFRuntimeClass *cfClass = __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)];
            if (cfClass->version & _kCFRuntimeResourcefulObject) {
                // reclaim is called by _CFRelease(), which must be called to keep the
                // CF and GC retain counts in sync.
                CFRelease(cf);
            } else {
                // avoid releasing normal CF objects.  Like other collections, for example
            }
            return;
#endif
        } else {
            // support constant CFTypeRef objects.
#if __LP64__
            uint32_t lowBits = ((CFRuntimeBase *)cf)->_rc;
#else
            uint32_t lowBits = ((CFRuntimeBase *)cf)->_cfinfo[CF_RC_BITS];
#endif
            if (lowBits == 0) return;
        }
    }
    CFRelease(cf);
}

#if !__LP64__
static CFLock_t __CFRuntimeExternRefCountTableLock = CFLockInit;
#endif

static uint64_t __CFGetFullRetainCount(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** __CFGetFullRetainCount() called with NULL ***"); HALT; }
#if __LP64__
    uint32_t lowBits = ((CFRuntimeBase *)cf)->_rc;
    if (0 == lowBits) {
        return (uint64_t)0x0fffffffffffffffULL;
    }
    return lowBits;
#else
    uint32_t lowBits = ((CFRuntimeBase *)cf)->_cfinfo[CF_RC_BITS];
    if (0 == lowBits) {
        return (uint64_t)0x0fffffffffffffffULL;
    }
    uint64_t highBits = 0;
    if ((lowBits & 0x80) != 0) {
        highBits = __CFDoExternRefOperation(500, (id)cf);
    }
    uint64_t compositeRC = (lowBits & 0x7f) + (highBits << 6);
    return compositeRC;
#endif
}

CFIndex CFGetRetainCount(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFGetRetainCount() called with NULL ***"); HALT; }
    uint32_t cfinfo = *(uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    if (cfinfo & 0x800000) { // custom ref counting for object
        CFTypeID typeID = (cfinfo >> 8) & 0x03FF; // mask up to 0x0FFF
        CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
        uint32_t (*refcount)(intptr_t, CFTypeRef) = cfClass->refcount;
        if (!refcount || !(cfClass->version & _kCFRuntimeCustomRefCount) || (((CFRuntimeBase *)cf)->_cfinfo[CF_RC_BITS] != 0xFF)) {
            HALT; // bogus object
        }
#if __LP64__
        if (((CFRuntimeBase *)cf)->_rc != 0xFFFFFFFFU) {
            HALT; // bogus object
        }
#endif
        uint32_t rc = refcount(0, cf);
#if __LP64__
        return (CFIndex)rc;
#else
        return (rc < LONG_MAX) ? (CFIndex)rc : (CFIndex)LONG_MAX;
#endif
    }
    uint64_t rc = __CFGetFullRetainCount(cf);
    return (rc < (uint64_t)LONG_MAX) ? (CFIndex)rc : (CFIndex)LONG_MAX;
}

CFTypeRef CFMakeCollectable(CFTypeRef cf) {
    if (NULL == cf) return NULL;
    return cf;
}

CFTypeRef CFMakeUncollectable(CFTypeRef cf) {
    if (NULL == cf) return NULL;
    if (CF_IS_COLLECTABLE(cf)) {
        CFRetain(cf);
    }
    return cf;
}

Boolean CFEqual(CFTypeRef cf1, CFTypeRef cf2) {
    if (NULL == cf1) { CRSetCrashLogMessage("*** CFEqual() called with NULL first argument ***"); HALT; }
    if (NULL == cf2) { CRSetCrashLogMessage("*** CFEqual() called with NULL second argument ***"); HALT; }
    if (cf1 == cf2) return true;
    CFTYPE_OBJC_FUNCDISPATCH1(Boolean, cf1, isEqual:, cf2);
    CFTYPE_OBJC_FUNCDISPATCH1(Boolean, cf2, isEqual:, cf1);
    __CFGenericAssertIsCF(cf1);
    __CFGenericAssertIsCF(cf2);
    if (__CFGenericTypeID_inline(cf1) != __CFGenericTypeID_inline(cf2)) return false;
    if (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf1)]->equal) {
	return __CFRuntimeClassTable[__CFGenericTypeID_inline(cf1)]->equal(cf1, cf2);
    }
    return false;
}

CFHashCode CFHash(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFHash() called with NULL ***"); HALT; }
    CFTYPE_OBJC_FUNCDISPATCH0(CFHashCode, cf, hash);
    __CFGenericAssertIsCF(cf);
    CFHashCode (*hash)(CFTypeRef cf) = __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->hash; 
    if (NULL != hash) {
	return hash(cf);
    }
    if (CF_IS_COLLECTABLE(cf)) return (CFHashCode)_object_getExternalHash((id)cf);
    return (CFHashCode)cf;
}

// definition: produces a normally non-NULL debugging description of the object
CFStringRef CFCopyDescription(CFTypeRef cf) {
    if (NULL == cf) return NULL;
    // CFTYPE_OBJC_FUNCDISPATCH0(CFStringRef, cf, _copyDescription);  // XXX returns 0 refcounted item under GC
    __CFGenericAssertIsCF(cf);
    if (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->copyDebugDesc) {
	CFStringRef result = __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->copyDebugDesc(cf);
	if (NULL != result) return result;
    }
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<%s %p [%p]>"), __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->className, cf, CFGetAllocator(cf));
}

// Definition: if type produces a formatting description, return that string, otherwise NULL
CF_PRIVATE CFStringRef __CFCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    if (NULL == cf) return NULL;
    __CFGenericAssertIsCF(cf);
    if (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->copyFormattingDesc) {
	return __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->copyFormattingDesc(cf, formatOptions);
    }
    return NULL;
}

extern CFAllocatorRef __CFAllocatorGetAllocator(CFTypeRef);

CFAllocatorRef CFGetAllocator(CFTypeRef cf) {
    if (NULL == cf) return kCFAllocatorSystemDefault;
    if (__kCFAllocatorTypeID_CONST == __CFGenericTypeID_inline(cf)) {
	return __CFAllocatorGetAllocator(cf);
    }
    return __CFGetAllocator(cf);
}


extern CFTypeID CFBinaryHeapGetTypeID();
extern CFTypeID CFBitVectorGetTypeID();
extern CFTypeID CFTreeGetTypeID();
extern CFTypeID CFPlugInInstanceGetTypeID();
extern CFTypeID CFStringTokenizerGetTypeID();
extern CFTypeID CFStorageGetTypeID(void);
extern void __CFAllocatorInitialize(void);
extern void __CFStringInitialize(void);
extern void __CFCharacterSetInitialize(void);
extern void __CFPFactoryInitialize(void);
extern void __CFPlugInInitialize(void);
#if DEPLOYMENT_TARGET_LINUX
CF_PRIVATE void __CFTSDLinuxInitialize();
#endif
#if DEPLOYMENT_TARGET_WINDOWS
// From CFPlatform.c
CF_PRIVATE void __CFTSDWindowsInitialize(void);
CF_PRIVATE void __CFTSDWindowsCleanup(void);
CF_PRIVATE void __CFFinalizeWindowsThreadData();
#endif
extern void __CFStreamInitialize(void);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
extern void __CFXPreferencesInitialize(void);
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
CF_PRIVATE uint8_t __CF120290 = false;
CF_PRIVATE uint8_t __CF120291 = false;
CF_PRIVATE uint8_t __CF120293 = false;
CF_PRIVATE char * __crashreporter_info__ = NULL; // Keep this symbol, since it was exported and other things may be linking against it, like GraphicsServices.framework on iOS
asm(".desc ___crashreporter_info__, 0x10");

static void __01121__(void) {
    __CF120291 = pthread_is_threaded_np() ? true : false;
}

static void __01123__(void) {
    // Ideally, child-side atfork handlers should be async-cancel-safe, as fork()
    // is async-cancel-safe and can be called from signal handlers.  See also
    // http://standards.ieee.org/reading/ieee/interp/1003-1c-95_int/pasc-1003.1c-37.html
    // This is not a problem for CF.
    if (__CF120290) {
	__CF120293 = true;
#if DEPLOYMENT_TARGET_MACOSX
	if (__CF120291) {
	    CRSetCrashLogMessage2("*** multi-threaded process forked ***");
	} else {
	    CRSetCrashLogMessage2("*** single-threaded process forked ***");
	}
#endif
    }
}

#define EXEC_WARNING_STRING_1 "The process has forked and you cannot use this CoreFoundation functionality safely. You MUST exec().\n"
#define EXEC_WARNING_STRING_2 "Break on __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__() to debug.\n"

CF_PRIVATE void __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(void) {
    write(2, EXEC_WARNING_STRING_1, sizeof(EXEC_WARNING_STRING_1) - 1);
    write(2, EXEC_WARNING_STRING_2, sizeof(EXEC_WARNING_STRING_2) - 1);
//    HALT;
}
#endif


CF_EXPORT const void *__CFArgStuff;
const void *__CFArgStuff = NULL;
CF_PRIVATE void *__CFAppleLanguages = NULL;

// do not cache CFFIXED_USER_HOME or HOME, there are situations where they can change

static struct {
    const char *name;
    const char *value;
} __CFEnv[] = {
    {"PATH", NULL},
    {"USER", NULL},
    {"HOMEPATH", NULL},
    {"HOMEDRIVE", NULL},
    {"USERNAME", NULL},
    {"TZFILE", NULL},
    {"TZ", NULL},
    {"NEXT_ROOT", NULL},
    {"DYLD_IMAGE_SUFFIX", NULL},
    {"CFProcessPath", NULL},
    {"CFNETWORK_LIBRARY_PATH", NULL},
    {"CFUUIDVersionNumber", NULL},
    {"CFDebugNamedDataSharing", NULL},
    {"CFPropertyListAllowImmutableCollections", NULL},
    {"CFBundleUseDYLD", NULL},
    {"CFBundleDisableStringsSharing", NULL},
    {"CFCharacterSetCheckForExpandedSet", NULL},
    {"__CF_DEBUG_EXPANDED_SET", NULL},
    {"CFStringDisableROM", NULL},
    {"CF_CHARSET_PATH", NULL},
    {"__CF_USER_TEXT_ENCODING", NULL},
    {"CFNumberDisableCache", NULL},
    {"__CFPREFERENCES_AVOID_DAEMON", NULL},
    {"APPLE_FRAMEWORKS_ROOT", NULL},
    {NULL, NULL}, // the last one is for optional "COMMAND_MODE" "legacy", do not use this slot, insert before
};

CF_PRIVATE const char *__CFgetenv(const char *n) {
    for (CFIndex idx = 0; idx < sizeof(__CFEnv) / sizeof(__CFEnv[0]); idx++) {
	if (__CFEnv[idx].name && 0 == strcmp(n, __CFEnv[idx].name)) return __CFEnv[idx].value;
    }
    return getenv(n);
}

CF_PRIVATE Boolean __CFProcessIsRestricted() {
    return issetugid();
}

#if DEPLOYMENT_TARGET_WINDOWS
#define kNilPthreadT  { nil, nil }
#else
#define kNilPthreadT  (pthread_t)0
#endif


#undef kCFUseCollectableAllocator
CF_EXPORT bool kCFUseCollectableAllocator;
bool kCFUseCollectableAllocator = false;

CF_PRIVATE Boolean __CFProphylacticAutofsAccess = false;
CF_PRIVATE Boolean __CFInitializing = 0;
CF_PRIVATE Boolean __CFInitialized = 0;

// move the next 2 lines down into the #if below, and make it static, after Foundation gets off this symbol on other platforms
CF_EXPORT pthread_t _CFMainPThread;
pthread_t _CFMainPThread = kNilPthreadT;
#if DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_IPHONESIMULATOR

CF_EXPORT pthread_t _CF_pthread_main_thread_np(void);
pthread_t _CF_pthread_main_thread_np(void) {
    return _CFMainPThread;
}
#define pthread_main_thread_np() _CF_pthread_main_thread_np()

#endif

#if DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
static void __CFInitialize(void) __attribute__ ((constructor));
static
#endif
#if DEPLOYMENT_TARGET_WINDOWS
CF_EXPORT
#endif
void __CFInitialize(void) {

    if (!__CFInitialized && !__CFInitializing) {
        __CFInitializing = 1;

#if DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_IPHONESIMULATOR
        if (!pthread_main_np()) HALT;   // CoreFoundation must be initialized on the main thread
#endif
	// move this next line up into the #if above after Foundation gets off this symbol
        _CFMainPThread = pthread_self();

#if DEPLOYMENT_TARGET_WINDOWS
        // Must not call any CF functions
        __CFTSDWindowsInitialize();
#elif DEPLOYMENT_TARGET_LINUX
        __CFTSDLinuxInitialize();
#endif
        
        __CFProphylacticAutofsAccess = true;

        for (CFIndex idx = 0; idx < sizeof(__CFEnv) / sizeof(__CFEnv[0]); idx++) {
            __CFEnv[idx].value = __CFEnv[idx].name ? getenv(__CFEnv[idx].name) : NULL;
        }
        
#if !defined(kCFUseCollectableAllocator)
        kCFUseCollectableAllocator = objc_collectingEnabled();
#endif
        if (kCFUseCollectableAllocator) {
#if !defined(__CFObjCIsCollectable)
            __CFObjCIsCollectable = (bool (*)(void *))objc_isAuto;
#endif
        }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
	UInt32 s, r;
	__CFStringGetUserDefaultEncoding(&s, &r); // force the potential setenv to occur early
	pthread_atfork(__01121__, NULL, __01123__);
#endif


        memset(__CFRuntimeClassTable, 0, sizeof(__CFRuntimeClassTable));
        memset(__CFRuntimeObjCClassTable, 0, sizeof(__CFRuntimeObjCClassTable));


        /* Here so that two runtime classes get indices 0, 1. */
        __kCFNotATypeTypeID = _CFRuntimeRegisterClass(&__CFNotATypeClass);
        __kCFTypeTypeID = _CFRuntimeRegisterClass(&__CFTypeClass);

        /* Here so that __kCFAllocatorTypeID gets index 2. */
        __CFAllocatorInitialize();

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        {
            CFIndex idx, cnt;
            char **args = *_NSGetArgv();
            cnt = *_NSGetArgc();
            for (idx = 1; idx < cnt - 1; idx++) {
                if (NULL == args[idx]) continue;
                if (0 == strcmp(args[idx], "-AppleLanguages") && args[idx + 1]) {
                    CFIndex length = strlen(args[idx + 1]);
                    __CFAppleLanguages = malloc(length + 1);
                    memmove(__CFAppleLanguages, args[idx + 1], length + 1);
                    break;
                }
            }
        }
#endif


        CFBasicHashGetTypeID();
        CFBagGetTypeID();

	for (CFIndex idx = 0; idx < NUM_EXTERN_TABLES; idx++) {
            CFBasicHashCallbacks callbacks = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	    __NSRetainCounters[idx].table = CFBasicHashCreate(kCFAllocatorSystemDefault, kCFBasicHashHasCounts | kCFBasicHashLinearHashing | kCFBasicHashAggressiveGrowth, &callbacks);
	    CFBasicHashSetCapacity(__NSRetainCounters[idx].table, 40);
	    __NSRetainCounters[idx].lock = CFLockInit;
	}

        /*** _CFRuntimeCreateInstance() can finally be called generally after this line. ***/

        __CFRuntimeClassTableCount = 7;
        __CFStringInitialize();		// CFString's TypeID must be 0x7, now and forever
    
        __CFRuntimeClassTableCount = 16;
        CFNullGetTypeID();		// See above for hard-coding of this position
        CFSetGetTypeID();		// See above for hard-coding of this position
        CFDictionaryGetTypeID();	// See above for hard-coding of this position
        CFArrayGetTypeID();		// See above for hard-coding of this position
        CFDataGetTypeID();		// See above for hard-coding of this position
        CFBooleanGetTypeID();		// See above for hard-coding of this position
        CFNumberGetTypeID();		// See above for hard-coding of this position

        CFBinaryHeapGetTypeID();
        CFBitVectorGetTypeID();
        __CFCharacterSetInitialize();
        CFStorageGetTypeID();
        CFErrorGetTypeID();
        CFTreeGetTypeID();
        CFURLGetTypeID();
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
        CFBundleGetTypeID();
        __CFPFactoryInitialize();
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        __CFPlugInInitialize();
        CFPlugInInstanceGetTypeID();
#endif
        CFUUIDGetTypeID();
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS
	CFMessagePortGetTypeID();
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        CFMachPortGetTypeID();
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
        __CFStreamInitialize();
#endif
#if DEPLOYMENT_TARGET_WINDOWS
        CFWindowsNamedPipeGetTypeID();
#endif
        
        CFDateGetTypeID();

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS
        CFRunLoopGetTypeID();
        CFRunLoopObserverGetTypeID();
        CFRunLoopSourceGetTypeID();
        CFRunLoopTimerGetTypeID();
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        CFTimeZoneGetTypeID();
        CFCalendarGetTypeID();
#if DEPLOYMENT_TARGET_LINUX
        CFTimeZoneGetTypeID();
        CFCalendarGetTypeID();
#endif
#endif

        {
            CFIndex idx, cnt = 0;
            char **args = NULL;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
            args = *_NSGetArgv();
            cnt = *_NSGetArgc();
#elif DEPLOYMENT_TARGET_WINDOWS
            wchar_t *commandLine = GetCommandLineW();
            // result is actually pointer to wchar_t *, make sure to account for that below
            args = (char **)CommandLineToArgvW(commandLine, (int *)&cnt);
#endif
            CFIndex count;
            CFStringRef *list, buffer[256];
            list = (cnt <= 256) ? buffer : (CFStringRef *)malloc(cnt * sizeof(CFStringRef));
            for (idx = 0, count = 0; idx < cnt; idx++) {
                if (NULL == args[idx]) continue;
#if DEPLOYMENT_TARGET_WINDOWS
                list[count] = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (const UniChar *)args[idx], wcslen((wchar_t *)args[idx]));
#else
                list[count] = CFStringCreateWithCString(kCFAllocatorSystemDefault, args[idx], kCFStringEncodingUTF8);
                if (NULL == list[count]) {
                    list[count] = CFStringCreateWithCString(kCFAllocatorSystemDefault, args[idx], kCFStringEncodingISOLatin1);
                    // We CANNOT use the string SystemEncoding here;
                    // Do not argue: it is not initialized yet, but these
                    // arguments MUST be initialized before it is.
                    // We should just ignore the argument if the UTF-8
                    // conversion fails, but out of charity we try once
                    // more with ISO Latin1, a standard unix encoding.
                }
#endif
                if (NULL != list[count]) count++;
            }
            __CFArgStuff = CFArrayCreate(kCFAllocatorSystemDefault, (const void **)list, count, &kCFTypeArrayCallBacks);
            if (list != buffer) free(list);
#if DEPLOYMENT_TARGET_WINDOWS
            LocalFree(args);
#endif
        }

        _CFProcessPath();	// cache this early

        __CFOAInitialize();
        

        if (__CFRuntimeClassTableCount < 256) __CFRuntimeClassTableCount = 256;


#if defined(DEBUG) || defined(ENABLE_ZOMBIES)
        const char *value = __CFgetenv("NSZombieEnabled");
        if (value && (*value == 'Y' || *value == 'y')) _CFEnableZombies();
        value = __CFgetenv("NSDeallocateZombies");
        if (value && (*value == 'Y' || *value == 'y')) __CFDeallocateZombies = 0xff;
#endif

#if defined(DEBUG) && (DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED)
        CFLog(kCFLogLevelWarning, CFSTR("Assertions enabled"));
#endif

        __CFProphylacticAutofsAccess = false;
        __CFInitializing = 0;
        __CFInitialized = 1;
    }
}


#if DEPLOYMENT_TARGET_WINDOWS

CF_PRIVATE void __CFStringCleanup(void);
CF_PRIVATE void __CFSocketCleanup(void);
CF_PRIVATE void __CFUniCharCleanup(void);
CF_PRIVATE void __CFStreamCleanup(void);

static CFBundleRef RegisterCoreFoundationBundle(void) {
#ifdef _DEBUG
    // might be nice to get this from the project file at some point
    wchar_t *DLLFileName = (wchar_t *)L"CoreFoundation_debug.dll";
#else
    wchar_t *DLLFileName = (wchar_t *)L"CoreFoundation.dll";
#endif
    wchar_t path[MAX_PATH+1];
    path[0] = path[1] = 0;
    DWORD wResult;
    CFIndex idx;
    HMODULE ourModule = GetModuleHandleW(DLLFileName);

    CFAssert(ourModule, __kCFLogAssertion, "GetModuleHandle failed");

    wResult = GetModuleFileNameW(ourModule, path, MAX_PATH+1);
    CFAssert1(wResult > 0, __kCFLogAssertion, "GetModuleFileName failed: %d", GetLastError());
    CFAssert1(wResult < MAX_PATH+1, __kCFLogAssertion, "GetModuleFileName result truncated: %s", path);

    // strip off last component, the DLL name
    for (idx = wResult - 1; idx; idx--) {
        if ('\\' == path[idx]) {
            path[idx] = '\0';
            break;
        }
    }

    CFStringRef fsPath = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar*)path, idx);
    CFURLRef dllURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, fsPath, kCFURLWindowsPathStyle, TRUE);
    CFURLRef bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, dllURL, CFSTR("CoreFoundation.resources"), TRUE);
    CFRelease(fsPath);
    CFRelease(dllURL);

    // this registers us so subsequent calls to CFBundleGetBundleWithIdentifier will succeed
    CFBundleRef bundle = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
    CFRelease(bundleURL);

    return bundle;
}


#define DLL_PROCESS_ATTACH   1
#define DLL_THREAD_ATTACH    2
#define DLL_THREAD_DETACH    3
#define DLL_PROCESS_DETACH   0

int DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved ) {
    static CFBundleRef cfBundle = NULL;
    if (dwReason == DLL_PROCESS_ATTACH) {
	__CFInitialize();
        cfBundle = RegisterCoreFoundationBundle();
    } else if (dwReason == DLL_PROCESS_DETACH && pReserved == 0) {
        // Only cleanup if we are being unloaded dynamically (pReserved == 0) <rdar://problem/7480873>
        __CFStreamCleanup();
        __CFSocketCleanup();
        __CFUniCharCleanup();

#if DEPLOYMENT_TARGET_WINDOWS
        // No CF functions should access TSD after this is called
        __CFTSDWindowsCleanup();
#endif

	// do these last
	if (cfBundle) CFRelease(cfBundle);
        __CFStringCleanup();
    } else if (dwReason == DLL_THREAD_DETACH) {
        __CFFinalizeWindowsThreadData();
    }
    return TRUE;
}

#endif

#if __CF_BIG_ENDIAN__
#define RC_INCREMENT		(1ULL)
#define RC_MASK			(0xFFFFFFFFULL)
#define RC_GET(V)		((V) & RC_MASK)
#define RC_DEALLOCATING_BIT	(0x400000ULL << 32)
#define RC_DEALLOCATED_BIT	(0x200000ULL << 32)
#else
#define RC_INCREMENT		(1ULL << 32)
#define RC_MASK			(0xFFFFFFFFULL << 32)
#define RC_GET(V)		(((V) & RC_MASK) >> 32)
#define RC_DEALLOCATING_BIT	(0x400000ULL)
#define RC_DEALLOCATED_BIT	(0x200000ULL)
#endif

#if !DEPLOYMENT_TARGET_WINDOWS && __LP64__
static bool (*CAS64)(int64_t, int64_t, volatile int64_t *) = OSAtomicCompareAndSwap64Barrier;
#else
static bool (*CAS32)(int32_t, int32_t, volatile int32_t *) = OSAtomicCompareAndSwap32Barrier;
#endif

// For "tryR==true", a return of NULL means "failed".
static CFTypeRef _CFRetain(CFTypeRef cf, Boolean tryR) {
    uint32_t cfinfo = *(uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    if (cfinfo & 0x800000) { // custom ref counting for object
        if (tryR) return NULL;
        CFTypeID typeID = (cfinfo >> 8) & 0x03FF; // mask up to 0x0FFF
        CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
        uint32_t (*refcount)(intptr_t, CFTypeRef) = cfClass->refcount;
        if (!refcount || !(cfClass->version & _kCFRuntimeCustomRefCount) || (((CFRuntimeBase *)cf)->_cfinfo[CF_RC_BITS] != 0xFF)) {
            HALT; // bogus object
        }
#if __LP64__
        if (((CFRuntimeBase *)cf)->_rc != 0xFFFFFFFFU) {
            HALT; // bogus object
        }
#endif
        refcount(+1, cf);
        return cf;
    }

    Boolean didAuto = false;
    if (tryR && (cfinfo & (0x400000 | 0x200000))) return NULL; // deallocating or deallocated
#if __LP64__
    if (0 == ((CFRuntimeBase *)cf)->_rc && !CF_IS_COLLECTABLE(cf)) return cf;	// Constant CFTypeRef
#if !DEPLOYMENT_TARGET_WINDOWS
    uint64_t allBits;
    do {
        allBits = *(uint64_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
        if (tryR && (allBits & RC_DEALLOCATING_BIT)) return NULL;
    } while (!CAS64(allBits, allBits + RC_INCREMENT, (int64_t *)&((CFRuntimeBase *)cf)->_cfinfo));
    // GC:  0 --> 1 transition? then add a GC retain count, to root the object. we'll remove it on the 1 --> 0 transition.
    if (RC_GET(allBits) == 0 && CF_IS_COLLECTABLE(cf)) {
	auto_zone_retain(objc_collectableZone(), (void*)cf);
	didAuto = true;
    }
#else
    uint32_t lowBits;
    do {
	lowBits = ((CFRuntimeBase *)cf)->_rc;
    } while (!CAS32(lowBits, lowBits + 1, (int32_t *)&((CFRuntimeBase *)cf)->_rc));
    // GC:  0 --> 1 transition? then add a GC retain count, to root the object. we'll remove it on the 1 --> 0 transition.
    if (lowBits == 0 && CF_IS_COLLECTABLE(cf)) {
	auto_zone_retain(objc_collectableZone(), (void*)cf);
	didAuto = true;
    }
#endif
#else
#define RC_START 24
#define RC_END 31
    CFIndex rcLowBits = __CFBitfieldGetValue(cfinfo, RC_END, RC_START);
    if (__builtin_expect(0 == rcLowBits, 0) && !CF_IS_COLLECTABLE(cf)) return cf;	// Constant CFTypeRef
    volatile uint32_t *infoLocation = (uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    bool success = 0;
    do {
        cfinfo = *infoLocation;
#if !DEPLOYMENT_TARGET_WINDOWS
        // if already deallocating, don't allow new retain
        if (tryR && (cfinfo & 0x400000)) return NULL;
#endif
        uint32_t prospectiveNewInfo = cfinfo; // don't want compiler to generate prospectiveNewInfo = *infoLocation.  This is why infoLocation is declared as a pointer to volatile memory.
        prospectiveNewInfo += (1 << RC_START);
        rcLowBits = __CFBitfieldGetValue(prospectiveNewInfo, RC_END, RC_START);
        if (__builtin_expect((rcLowBits & 0x7f) == 0, 0)) {
            /* Roll over another bit to the external ref count
             Real ref count = low 7 bits of info[CF_RC_BITS]  + external ref count << 6
             Bit 8 of low bits indicates that external ref count is in use.
             External ref count is shifted by 6 rather than 7 so that we can set the low
		bits to to 1100 0000 rather than 1000 0000.
		This prevents needing to access the external ref count for successive retains and releases
		when the composite retain count is right around a multiple of 1 << 7.
             */
            prospectiveNewInfo = cfinfo;
            __CFBitfieldSetValue(prospectiveNewInfo, RC_END, RC_START, ((1 << 7) | (1 << 6)));
            __CFLock(&__CFRuntimeExternRefCountTableLock);
            success = CAS32(*(int32_t *)& cfinfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
            if (__builtin_expect(success, 1)) {
                __CFDoExternRefOperation(350, (id)cf);
            }
            __CFUnlock(&__CFRuntimeExternRefCountTableLock);
        } else {
            success = CAS32(*(int32_t *)& cfinfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
            // XXX_PCB:  0 --> 1 transition? then add a GC retain count, to root the object. we'll remove it on the 1 --> 0 transition.
            if (success && __CFBitfieldGetValue(cfinfo, RC_END, RC_START) == 0 && CF_IS_COLLECTABLE(cf)) {
		auto_zone_retain(objc_collectableZone(), (void*)cf);
		didAuto = true;
	    }
        }
    } while (__builtin_expect(!success, 0));
#endif
    if (!didAuto && __builtin_expect(__CFOASafe, 0)) {
	__CFRecordAllocationEvent(__kCFRetainEvent, (void *)cf, 0, CFGetRetainCount(cf), NULL);
    }
    return cf;
}

// Never called under GC, only called via ARR weak subsystem; a return of NULL is failure
CFTypeRef _CFTryRetain(CFTypeRef cf) {
    if (NULL == cf) return NULL;
#if OBJC_HAVE_TAGGED_POINTERS
    if (_objc_isTaggedPointer(cf)) return cf; // success
#endif
    return _CFRetain(cf, true);
}

Boolean _CFIsDeallocating(CFTypeRef cf) {
    if (NULL == cf) return false;
#if OBJC_HAVE_TAGGED_POINTERS
    if (_objc_isTaggedPointer(cf)) return false;
#endif
    uint32_t cfinfo = *(uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    if (cfinfo & 0x800000) { // custom ref counting for object
        return true;   // lie for now; this weak references to these objects cannot be formed
    }
    return (cfinfo & 0x400000) ? true : false;
}

static void _CFRelease(CFTypeRef cf) {

    uint32_t cfinfo = *(uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    if (cfinfo & 0x200000) return; // deallocated, or not a cf object
    CFTypeID typeID = (cfinfo >> 8) & 0x03FF; // mask up to 0x0FFF
    if (cfinfo & 0x800000) { // custom ref counting for object
        CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
        uint32_t (*refcount)(intptr_t, CFTypeRef) = cfClass->refcount;
        if (!refcount || !(cfClass->version & _kCFRuntimeCustomRefCount) || (((CFRuntimeBase *)cf)->_cfinfo[CF_RC_BITS] != 0xFF)) {
            HALT; // bogus object
        }
#if __LP64__
        if (((CFRuntimeBase *)cf)->_rc != 0xFFFFFFFFU) {
            HALT; // bogus object
        }
#endif
        refcount(-1, cf);
        return;
    }

    CFIndex start_rc = __builtin_expect(__CFOASafe, 0) ? CFGetRetainCount(cf) : 0;
    Boolean isAllocator = (__kCFAllocatorTypeID_CONST == typeID);
    Boolean didAuto = false;
#if __LP64__
#if !DEPLOYMENT_TARGET_WINDOWS
    uint32_t lowBits;
    uint64_t allBits;
    again:;
    do {
        allBits = *(uint64_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
	lowBits = RC_GET(allBits);
	if (0 == lowBits) {
	    if (CF_IS_COLLECTABLE(cf)) auto_zone_release(objc_collectableZone(), (void*)cf);
	    return;        // Constant CFTypeRef
	}
        if (1 == lowBits) {
            CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
            if ((cfClass->version & _kCFRuntimeResourcefulObject) && cfClass->reclaim != NULL) {
                cfClass->reclaim(cf);
            }
	    if (!CF_IS_COLLECTABLE(cf)) {
		uint64_t newAllBits = allBits | RC_DEALLOCATING_BIT;
		if (!CAS64(allBits, newAllBits, (int64_t *)&((CFRuntimeBase *)cf)->_cfinfo)) {
		    goto again;
		}
                void (*func)(CFTypeRef) = __CFRuntimeClassTable[typeID]->finalize;
	        if (NULL != func) {
		    func(cf);
	        }
		// Any further ref-count changes after this point are operating on a finalized object
		allBits = *(uint64_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
		lowBits = RC_GET(allBits);
	        if (isAllocator || (1 == lowBits)) {
		    do { // hammer until it takes; trying to retain the object on another thread at this point? too late!
                        allBits = *(uint64_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
	            } while (!CAS64(allBits, (allBits | RC_DEALLOCATED_BIT) - RC_INCREMENT, (int64_t *)&((CFRuntimeBase *)cf)->_cfinfo));
		    goto really_free;
	        }
                Boolean success = false;
                do { // drop the deallocating bit; racey, but this resurrection stuff isn't thread-safe anyway
                    allBits = *(uint64_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
                    uint64_t newAllBits = allBits & ~RC_DEALLOCATING_BIT;
                    success = CAS64(allBits, newAllBits, (int64_t *)&((CFRuntimeBase *)cf)->_cfinfo);
                } while (!success);
		goto again; // still need to have the effect of a CFRelease
            }
        }
    } while (!CAS64(allBits, allBits - RC_INCREMENT, (int64_t *)&((CFRuntimeBase *)cf)->_cfinfo));
    if (lowBits == 1 && CF_IS_COLLECTABLE(cf)) {
        // GC:  release the collector's hold over the object, which will call the finalize function later on.
	auto_zone_release(objc_collectableZone(), (void*)cf);
        didAuto = true;
    }
#else
    uint32_t lowBits;
    do {
	lowBits = ((CFRuntimeBase *)cf)->_rc;
	if (0 == lowBits) {
	    if (CF_IS_COLLECTABLE(cf)) auto_zone_release(objc_collectableZone(), (void*)cf);
	    return;        // Constant CFTypeRef
	}
	if (1 == lowBits) {
	    // CANNOT WRITE ANY NEW VALUE INTO [CF_RC_BITS] UNTIL AFTER FINALIZATION
            CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
            if ((cfClass->version & _kCFRuntimeResourcefulObject) && cfClass->reclaim != NULL) {
                cfClass->reclaim(cf);
            }
	    if (!CF_IS_COLLECTABLE(cf)) {
                void (*func)(CFTypeRef) = __CFRuntimeClassTable[typeID]->finalize;
	        if (NULL != func) {
		    func(cf);
	        }
	        if (isAllocator || CAS32(1, 0, (int32_t *)&((CFRuntimeBase *)cf)->_rc)) {
		    goto really_free;
	        }
	    }
	}
    } while (!CAS32(lowBits, lowBits - 1, (int32_t *)&((CFRuntimeBase *)cf)->_rc));
    if (lowBits == 1 && CF_IS_COLLECTABLE(cf)) {
        // GC:  release the collector's hold over the object, which will call the finalize function later on.
	auto_zone_release(objc_collectableZone(), (void*)cf);
        didAuto = true;
    }
#endif
#else
#if !DEPLOYMENT_TARGET_WINDOWS
    again:;
    volatile uint32_t *infoLocation = (uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    CFIndex rcLowBits = __CFBitfieldGetValue(cfinfo, RC_END, RC_START);
    if (0 == rcLowBits) {
        if (CF_IS_COLLECTABLE(cf)) auto_zone_release(objc_collectableZone(), (void*)cf);
        return;        // Constant CFTypeRef
    }
    bool success = 0;
    Boolean whack = false;
    do {
        cfinfo = *infoLocation;
        rcLowBits = __CFBitfieldGetValue(cfinfo, RC_END, RC_START);
        if (1 == rcLowBits) {
            // we think cf should be deallocated
            uint32_t prospectiveNewInfo = cfinfo | (0x400000);
            if (CF_IS_COLLECTABLE(cf)) {
                prospectiveNewInfo -= (1 << RC_START);
            }
            success = CAS32(*(int32_t *)& cfinfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
            if (success) whack = true;
        } else {
            // not yet junk
            uint32_t prospectiveNewInfo = cfinfo; // don't want compiler to generate prospectiveNewInfo = *infoLocation.  This is why infoLocation is declared as a pointer to volatile memory.
            if ((1 << 7) == rcLowBits) {
                // Time to remove a bit from the external ref count
                __CFLock(&__CFRuntimeExternRefCountTableLock);
                CFIndex rcHighBitsCnt = __CFDoExternRefOperation(500, (id)cf);
                if (1 == rcHighBitsCnt) {
                    __CFBitfieldSetValue(prospectiveNewInfo, RC_END, RC_START, (1 << 6) - 1);
                } else {
                    __CFBitfieldSetValue(prospectiveNewInfo, RC_END, RC_START, ((1 << 6) | (1 << 7)) - 1);
                }
                success = CAS32(*(int32_t *)& cfinfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
                if (success) {
                    __CFDoExternRefOperation(450, (id)cf);
                }
                __CFUnlock(&__CFRuntimeExternRefCountTableLock);
            } else {
                prospectiveNewInfo -= (1 << RC_START);
                success = CAS32(*(int32_t *)& cfinfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
            }
        }
    } while (!success);

    if (whack) {
            CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
            if ((cfClass->version & _kCFRuntimeResourcefulObject) && cfClass->reclaim != NULL) {
                cfClass->reclaim(cf);
            }
            if (CF_IS_COLLECTABLE(cf)) {
                // GC:  release the collector's hold over the object, which will call the finalize function later on.
                auto_zone_release(objc_collectableZone(), (void*)cf);
                didAuto = true;
            } else {
                if (isAllocator) {
                    goto really_free;
                } else {
                    void (*func)(CFTypeRef) = __CFRuntimeClassTable[typeID]->finalize;
                    if (NULL != func) {
                        func(cf);
                    }
		    // Any further ref-count changes after this point are operating on a finalized object
                    rcLowBits = __CFBitfieldGetValue(*infoLocation, RC_END, RC_START);
                    if (1 == rcLowBits) {
                        do { // hammer until it takes; trying to retain the object on another thread at this point? too late!
                            cfinfo = *infoLocation;
                        } while (!CAS32(cfinfo, cfinfo | 0x200000, (int32_t *)infoLocation));
                        goto really_free;
                    }
                    do { // drop the deallocating bit; racey, but this resurrection stuff isn't thread-safe anyway
                        cfinfo = *infoLocation;
                        uint32_t prospectiveNewInfo = (cfinfo & ~(0x400000));
                        success = CAS32(*(int32_t *)& cfinfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
                    } while (!success);
                    goto again;
                }
            }
     }
#else
    volatile uint32_t *infoLocation = (uint32_t *)&(((CFRuntimeBase *)cf)->_cfinfo);
    CFIndex rcLowBits = __CFBitfieldGetValue(*infoLocation, RC_END, RC_START);
    if (0 == rcLowBits) {
        if (CF_IS_COLLECTABLE(cf)) auto_zone_release(objc_collectableZone(), (void*)cf);
        return;        // Constant CFTypeRef
    }
    bool success = 0;
    do {
        uint32_t initialCheckInfo = *infoLocation;
        rcLowBits = __CFBitfieldGetValue(initialCheckInfo, RC_END, RC_START);
        if (1 == rcLowBits) {
            // we think cf should be deallocated
	    // CANNOT WRITE ANY NEW VALUE INTO [CF_RC_BITS] UNTIL AFTER FINALIZATION
	    CFRuntimeClass *cfClass = __CFRuntimeClassTable[typeID];
	    if ((cfClass->version & _kCFRuntimeResourcefulObject) && cfClass->reclaim != NULL) {
		cfClass->reclaim(cf);
	    }
	    if (CF_IS_COLLECTABLE(cf)) {
                uint32_t prospectiveNewInfo = initialCheckInfo - (1 << RC_START);
                success = CAS32(*(int32_t *)&initialCheckInfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
                // GC:  release the collector's hold over the object, which will call the finalize function later on.
                if (success) {
		    auto_zone_release(objc_collectableZone(), (void*)cf);
		    didAuto = true;
		}
             } else {
		if (isAllocator) {
		    goto really_free;
		} else {
                    void (*func)(CFTypeRef) = __CFRuntimeClassTable[typeID]->finalize;
                    if (NULL != func) {
		        func(cf);
                    }
		    // We recheck rcLowBits to see if the object has been retained again during
		    // the finalization process.  This allows for the finalizer to resurrect,
		    // but the main point is to allow finalizers to be able to manage the
		    // removal of objects from uniquing caches, which may race with other threads
		    // which are allocating (looking up and finding) objects from those caches,
		    // which (that thread) would be the thing doing the extra retain in that case.
		    rcLowBits = __CFBitfieldGetValue(*infoLocation, RC_END, RC_START);
		    success = (1 == rcLowBits);
		    if (success) {
			goto really_free;
		    }
		}
            }
        } else {
            // not yet junk
            uint32_t prospectiveNewInfo = initialCheckInfo; // don't want compiler to generate prospectiveNewInfo = *infoLocation.  This is why infoLocation is declared as a pointer to volatile memory.
            if ((1 << 7) == rcLowBits) {
                // Time to remove a bit from the external ref count
                __CFLock(&__CFRuntimeExternRefCountTableLock);
                CFIndex rcHighBitsCnt = __CFDoExternRefOperation(500, (id)cf);
                if (1 == rcHighBitsCnt) {
                    __CFBitfieldSetValue(prospectiveNewInfo, RC_END, RC_START, (1 << 6) - 1);
                } else {
                    __CFBitfieldSetValue(prospectiveNewInfo, RC_END, RC_START, ((1 << 6) | (1 << 7)) - 1);
                }
                success = CAS32(*(int32_t *)&initialCheckInfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
                if (success) {
		    __CFDoExternRefOperation(450, (id)cf);
                }
                __CFUnlock(&__CFRuntimeExternRefCountTableLock);
            } else {
                prospectiveNewInfo -= (1 << RC_START);
                success = CAS32(*(int32_t *)&initialCheckInfo, *(int32_t *)&prospectiveNewInfo, (int32_t *)infoLocation);
            }
        }
    } while (!success);
#endif
#endif
    if (!didAuto && __builtin_expect(__CFOASafe, 0)) {
	__CFRecordAllocationEvent(__kCFReleaseEvent, (void *)cf, 0, start_rc - 1, NULL);
    }
    return;

    really_free:;
    if (!didAuto && __builtin_expect(__CFOASafe, 0)) {
	// do not use CFGetRetainCount() because cf has been freed if it was an allocator
	__CFRecordAllocationEvent(__kCFReleaseEvent, (void *)cf, 0, 0, NULL);
    }
    // cannot zombify allocators, which get deallocated by __CFAllocatorDeallocate (finalize)
    if (isAllocator) {
        __CFAllocatorDeallocate((void *)cf);
    } else {
	CFAllocatorRef allocator = kCFAllocatorSystemDefault;
	Boolean usesSystemDefaultAllocator = true;

	if (!__CFBitfieldGetValue(((const CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 7, 7)) {
	    allocator = CFGetAllocator(cf);
            usesSystemDefaultAllocator = _CFAllocatorIsSystemDefault(allocator);
	}

	{
	    CFAllocatorDeallocate(allocator, (uint8_t *)cf - (usesSystemDefaultAllocator ? 0 : sizeof(CFAllocatorRef)));
	}

	if (kCFAllocatorSystemDefault != allocator) {
	    CFRelease(allocator);
	}
    }
}

#undef __kCFAllocatorTypeID_CONST
#undef __CFGenericAssertIsCF


