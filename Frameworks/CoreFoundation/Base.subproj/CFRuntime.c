/*	CFRuntime.c
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Michael LeHew
*/

#define ENABLE_ZOMBIES 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFRuntime.h>
#include "CFRuntime_Internal.h"
#include "CFInternal.h"
#include "CFBasicHash.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <CoreFoundation/CFUUID.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFURLComponents.h>
#if TARGET_OS_MAC
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <crt_externs.h>
#include <unistd.h>
#include <sys/stat.h>
#include <CoreFoundation/CFStringDefaultEncoding.h>
#endif
#include <CoreFoundation/CFUUID.h>
#include <CoreFoundation/CFTimeZone.h>
#include <CoreFoundation/CFCalendar.h>
#if TARGET_OS_IPHONE
// This isn't in the embedded runtime.h header
OBJC_EXPORT void *objc_destructInstance(id obj);
#endif


#if TARGET_OS_WIN32
#include <Shellapi.h>
#include <pathcch.h>
#endif

enum {
// retain/release recording constants -- must match values
// used by OA for now; probably will change in the future
__kCFRetainEvent = 28,
__kCFReleaseEvent = 29
};

#if TARGET_OS_WIN32 || TARGET_OS_LINUX
#include <malloc.h>
#elif TARGET_OS_BSD
#include <stdlib.h> // malloc()
#else
#include <malloc/malloc.h>
#endif

#define FAKE_INSTRUMENTS 0

#if TARGET_OS_MAC
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

#endif

extern void __HALT(void);

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

#if !__OBJC2__
//<rdar://problem/26305326> __attribute__((__cleanup__())) fails to link on 32 bit Mac
CF_PRIVATE void objc_terminate(void) {
    abort();
}
#endif

// the lock does not protect most reading of these; we just leak the old table to allow read-only accesses to continue to work
static os_unfair_lock __CFBigRuntimeFunnel = OS_UNFAIR_LOCK_INIT;

_CFClassTables __CFRuntimeClassTables __attribute__((aligned)) = {
    .classTable = {
    [_kCFRuntimeIDNotAType] = &__CFNotATypeClass,
    [_kCFRuntimeIDCFType] = &__CFTypeClass,
    [_kCFRuntimeIDCFAllocator] = &__CFAllocatorClass,
    [_kCFRuntimeIDCFBasicHash] = &__CFBasicHashClass,
    [_kCFRuntimeIDCFBag] = &__CFBagClass,
    [_kCFRuntimeIDCFString] = &__CFStringClass,
    [_kCFRuntimeIDCFNull] = &__CFNullClass,
    [_kCFRuntimeIDCFSet] = &__CFSetClass,
    [_kCFRuntimeIDCFDictionary] = &__CFDictionaryClass,
    [_kCFRuntimeIDCFArray] = &__CFArrayClass,
    [_kCFRuntimeIDCFData] = &__CFDataClass,
    [_kCFRuntimeIDCFBoolean] = &__CFBooleanClass,
    [_kCFRuntimeIDCFNumber] = &__CFNumberClass,
    [_kCFRuntimeIDCFBinaryHeap] = &__CFBinaryHeapClass,
    [_kCFRuntimeIDCFBitVector] = &__CFBitVectorClass,
    [_kCFRuntimeIDCFUUID] = &__CFUUIDClass,
    [_kCFRuntimeIDCFStorage] = &__CFStorageClass,
    [_kCFRuntimeIDCFTree] = &__CFTreeClass,
    [_kCFRuntimeIDCFError] = &__CFErrorClass,
    [_kCFRuntimeIDCFLocale] = &__CFLocaleClass,
    [_kCFRuntimeIDCFDateFormatter] = &__CFDateFormatterClass,
    [_kCFRuntimeIDCFNumberFormatter] = &__CFNumberFormatterClass,
    [_kCFRuntimeIDCFCalendar] = &__CFCalendarClass,
    [_kCFRuntimeIDCFDateIntervalFormatter] = &__CFDateIntervalFormatterClass,
    [_kCFRuntimeIDCFDate] = &__CFDateClass,
    [_kCFRuntimeIDCFTimeZone] = &__CFTimeZoneClass,
    [_kCFRuntimeIDCFKeyedArchiverUID] = &__CFKeyedArchiverUIDClass,
    
#if TARGET_OS_OSX && DEPLOYMENT_RUNTIME_OBJC
    [_kCFRuntimeIDCFXMLParser] = &__CFXMLParserClass,
    [_kCFRuntimeIDCFXMLNode] = &__CFXMLNodeClass,
#endif // TARGET_OS_OSX
    
    [_kCFRuntimeIDCFBundle] = &__CFBundleClass,
    [_kCFRuntimeIDCFPFactory] = &__CFPFactoryClass,
    [_kCFRuntimeIDCFPlugInInstance] = &__CFPlugInInstanceClass,

    [_kCFRuntimeIDCFPreferencesDomain] = &__CFPreferencesDomainClass,

#if TARGET_OS_MAC
    [_kCFRuntimeIDCFMachPort] = &__CFMachPortClass,
#endif



    [_kCFRuntimeIDCFRunLoopMode] = &__CFRunLoopModeClass,
    [_kCFRuntimeIDCFRunLoop] = &__CFRunLoopClass,
    [_kCFRuntimeIDCFRunLoopSource] = &__CFRunLoopSourceClass,
    [_kCFRuntimeIDCFRunLoopObserver] = &__CFRunLoopObserverClass,
    [_kCFRuntimeIDCFRunLoopTimer] = &__CFRunLoopTimerClass,
    [_kCFRuntimeIDCFSocket] = &__CFSocketClass,
    [_kCFRuntimeIDCFReadStream] = &__CFReadStreamClass,
    [_kCFRuntimeIDCFWriteStream] = &__CFWriteStreamClass,
    [_kCFRuntimeIDCFAttributedString] = &__CFAttributedStringClass,
    [_kCFRuntimeIDCFRunArray] = &__CFRunArrayClass,
    [_kCFRuntimeIDCFCharacterSet] = &__CFCharacterSetClass,
    
    
    [_kCFRuntimeIDCFURL] = &__CFURLClass,

    
    [_kCFRuntimeIDCFURLComponents] = &__CFURLComponentsClass,
    [_kCFRuntimeIDCFDateComponents] = &__CFDateComponentsClass,
    [_kCFRuntimeIDCFRelativeDateTimeFormatter] = &__CFRelativeDateTimeFormatterClass,
    [_kCFRuntimeIDCFListFormatter] = &__CFListFormatterClass,
    },
    .objCClassTable = {0}
};

static int32_t __CFRuntimeClassTableCount = _kCFRuntimeStartingClassID;

#if (TARGET_OS_MAC && !TARGET_OS_IPHONE && !__x86_64h__) // Match parity with private header
bool (*__CFObjCIsCollectable)(void *) = NULL;
#else
#endif

#if DEPLOYMENT_RUNTIME_SWIFT
// The constant string class reference is set at link time to _NSCFConstantString
void *__CFConstantStringClassReferencePtr = &_CF_CONSTANT_STRING_SWIFT_CLASS;
#else
#if !__CONSTANT_CFSTRINGS__
// Compiler uses this symbol name; must match compiler built-in decl, so we use 'int'
#if TARGET_RT_64_BIT
int __CFConstantStringClassReference[24] = {0};
#else
int __CFConstantStringClassReference[12] = {0};
#endif
#endif

#if TARGET_RT_64_BIT
int __CFConstantStringClassReference[24] = {0};
#else
int __CFConstantStringClassReference[12] = {0};
#endif

void *__CFConstantStringClassReferencePtr = NULL;
#endif

Boolean _CFIsObjC(CFTypeID typeID, void *obj) {
    return CF_IS_OBJC(typeID, obj);
}

CFTypeID _CFRuntimeRegisterClass(const CFRuntimeClass * const cls) {
    // NOTE: If you are adding a type to CF itself, please use a constant value (see CFRuntime_Internal.h)
// className must be pure ASCII string, non-null
    if ((cls->version & _kCFRuntimeCustomRefCount) && !cls->refcount) {
       CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeRegisterClass() given inconsistent class '%s'.  Program will crash soon."), cls->className);
       return _kCFRuntimeNotATypeID;
    }
    os_unfair_lock_lock_with_options(&__CFBigRuntimeFunnel, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    if (__CFMaxRuntimeTypes <= __CFRuntimeClassTableCount) {
	CFLog(kCFLogLevelWarning, CFSTR("*** CoreFoundation class table full; registration failing for class '%s'.  Program will crash soon."), cls->className);
        os_unfair_lock_unlock(&__CFBigRuntimeFunnel);
	return _kCFRuntimeNotATypeID;
    }
    if (__CFRuntimeClassTableSize <= __CFRuntimeClassTableCount) {
	CFLog(kCFLogLevelWarning, CFSTR("*** CoreFoundation class table full; registration failing for class '%s'.  Program will crash soon."), cls->className);
        os_unfair_lock_unlock(&__CFBigRuntimeFunnel);
	return _kCFRuntimeNotATypeID;
    }
    __CFRuntimeClassTable[__CFRuntimeClassTableCount++] = (CFRuntimeClass *)cls;
    CFTypeID typeID = __CFRuntimeClassTableCount - 1;
    os_unfair_lock_unlock(&__CFBigRuntimeFunnel);
    return typeID;
}

void _CFRuntimeBridgeTypeToClass(CFTypeID cf_typeID, const void *cls_ref) {
    os_unfair_lock_lock(&__CFBigRuntimeFunnel);
    __CFRuntimeObjCClassTable[cf_typeID] = (uintptr_t)cls_ref;
    os_unfair_lock_unlock(&__CFBigRuntimeFunnel);
}

const CFRuntimeClass * _CFRuntimeGetClassWithTypeID(CFTypeID typeID) {
    return __CFRuntimeClassTable[typeID]; // hopelessly unthreadsafe
}

void _CFRuntimeUnregisterClassWithTypeID(CFTypeID typeID) {
    os_unfair_lock_lock_with_options(&__CFBigRuntimeFunnel, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    __CFRuntimeClassTable[typeID] = NULL;
    os_unfair_lock_unlock(&__CFBigRuntimeFunnel);
}


#if defined(DEBUG) || defined(ENABLE_ZOMBIES)

uint8_t __CFZombieEnabled = 0;
uint8_t __CFDeallocateZombies = 0;

extern void __CFZombifyNSObject(void);  // from NSObject.m

void _CFEnableZombies(void) {
}

#endif /* DEBUG */

/*
 Layout of CF info field
 
  3      2       1
  1      4       6        7      0
  |      |       |        |      |
  rrrrrrrrCDX?ttttttttttttaIIIIIII
 
 r = retain count
 C = custom RC
 D = deallocating
 X = deallocated
 t = type ID (12 bits, although only 10 are used because the class table size is 1024)
 a = if set, use system default allocator
 I = type-specific info bits (6 bits available)
 
 On 64 bit, also includes 32 bits more of retain count from 32-63
 */

#if __CF_BIG_ENDIAN__
#define RC_INCREMENT		(1ULL)
#define RC_CUSTOM_RC_BIT	(0x800000ULL << 32)
#define RC_DEALLOCATING_BIT	(0x400000ULL << 32)
#define RC_DEALLOCATED_BIT	(0x200000ULL << 32)
#else
#define RC_INCREMENT		(1ULL << 32)
#define RC_CUSTOM_RC_BIT	(0x800000ULL)
#define RC_DEALLOCATING_BIT	(0x400000ULL)
#define RC_DEALLOCATED_BIT	(0x200000ULL)
#endif

#if TARGET_RT_64_BIT
#define HIGH_RC_START 32
#define HIGH_RC_END 63
#endif

#define LOW_RC_START 24
#define LOW_RC_END 31
#define TYPE_ID_START 8
#define TYPE_ID_END 17

CF_INLINE CFTypeID __CFTypeIDFromInfo(__CFInfoType info) {
    // yes, 10 bits masked off, though 12 bits are there for the type field; __CFRuntimeClassTableSize is 1024
    return (info & __CFInfoMask(TYPE_ID_END, TYPE_ID_START)) >> TYPE_ID_START;
}

/// Get the retain count from the low 32-bit field (the only one stored inline in 32 bit, and unused except for a marker in 64 bit)
CF_INLINE uint16_t __CFLowRCFromInfo(__CFInfoType info) {
    return __CFBitfieldGetValue(info, LOW_RC_END, LOW_RC_START);
}

#if TARGET_RT_64_BIT
/// Get the retain count from the high 32-bit field (only present in 64 bit)
CF_INLINE uint32_t __CFHighRCFromInfo(__CFInfoType info) {
    return __CFBitfield64GetValue(info, HIGH_RC_END, HIGH_RC_START);
}
#endif

CF_INLINE CFRuntimeBase *_cf_aligned_calloc(size_t align, CFIndex size, const char *className) {
    CFRuntimeBase *memory;
    
#if TARGET_OS_MAC
    memory = malloc_zone_memalign(malloc_default_zone(), align, size);
#elif TARGET_OS_LINUX
    int result = posix_memalign((void **)&memory, /*alignment*/ align, size);
    int error = errno;
    enum { errorStringBufferLength = 64 };
    char errorStringBuffer[errorStringBufferLength] = "unknown error";
    const char *errorStringPointer = errorStringBuffer;
    //The GNU-specific version returns a pointer, which may or may not be the pointer passed in. The XSI version returns int. See strerror(3) on Linux.
#if ! ( (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE )
    errorStringPointer =
#endif
    strerror_r(errno, errorStringBuffer, errorStringBufferLength);
    CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeCreateInstance() tried to allocate an instance of '%s', which requires %zu-byte alignment, but memory could not be so allocated: %s"), className, align, errorStringPointer);
#elif TARGET_OS_WIN32
    CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeCreateInstance() tried to allocate an instance of '%s', which requires %zu-byte alignment, but aligned memory is not supported on this platform"), className, align);
    memory = (CFRuntimeBase *)calloc(1, size);
#else
    CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeCreateInstance() tried to allocate an instance of '%s', which requires %zu-byte alignment, but aligned memory is not supported on this platform"), className, align);
    memory = NULL;
#endif
    
    return memory;
}

CFTypeRef _CFRuntimeCreateInstance(CFAllocatorRef allocator, CFTypeID typeID, CFIndex extraBytes, unsigned char *category) {
#if DEPLOYMENT_RUNTIME_SWIFT
    // Under the Swift runtime, all CFTypeRefs are _NSCFTypes or a toll-free bridged type
    
    extern  void *swift_allocObject(uintptr_t metadata, size_t requiredSize, size_t requiredAlignmentMask);
    uintptr_t isa = __CFRuntimeObjCClassTable[typeID];
    CFIndex size = sizeof(CFRuntimeBase) + extraBytes;
    const CFRuntimeClass *cls = __CFRuntimeClassTable[typeID];

#if !defined(__APPLE__) && (defined(__i686__) || (defined(__arm__) && !defined(__aarch64__)) || defined(_M_IX86) || defined(_M_ARM))
    // Linux and Windows 32-bit targets perform 8-byte alignment by default.
    static const kDefaultAlignment = 8;
#else
    static const kDefaultAlignment = 16;
#endif

    // Ensure that we get the alignment correct for various targets.  In the
    // case that we are over-aligned `swift_allocObject` will go through a
    // different allocator to ensure that the pointer is suitably aligned.  When
    // we subsequently release the pointer we do not tag that release to go
    // through the overalign'ed path.  This may result in a cross-domainf free
    // and a resultant heap corruption.
    size_t align = (cls->version & _kCFRuntimeRequiresAlignment) ? cls->requiredAlignment : kDefaultAlignment;
    
    CFRuntimeBase *memory = (CFRuntimeBase *)swift_allocObject(isa, size, align - 1);
    
    // Zero the rest of the memory, starting at cfinfo
    memset(&memory->_cfinfoa, 0, size - (sizeof(memory->_cfisa) + sizeof(memory->_swift_rc)));

    // Set up the cfinfo struct
    uint64_t *cfinfop = (uint64_t *)&(memory->_cfinfoa);
    // The 0x80 means we use the default allocator
    *cfinfop = ((typeID << 8) | (0x80));

    return memory;
#else
    if (__CFRuntimeClassTableSize <= typeID) HALT;
    CFAssert1(typeID != _kCFRuntimeNotATypeID, __kCFLogAssertion, "%s(): Uninitialized type id", __PRETTY_FUNCTION__);
    CFRuntimeClass const *cls = __CFRuntimeClassTable[typeID];
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
    // To preserve 16 byte alignment when using custom allocators, we always place the CFAllocatorRef 16 bytes before the CFType. n.b. we assume sizeof(CFAllocatorRef) < 16
    CFIndex size = sizeof(CFRuntimeBase) + extraBytes + (usesSystemDefaultAllocator ? 0 : 16);
    size = (size + 0xF) & ~0xF;	// CF objects are multiples of 16 in size
    // CFType version 0 objects are unscanned by default since they don't have write-barriers and hard retain their innards
    // CFType version 1 objects are scanned and use hand coded write-barriers to store collectable storage within
    Boolean needsClear = false;
    CFRuntimeBase *memory = NULL;
    if (cls->version & _kCFRuntimeRequiresAlignment) {
        memory = _cf_aligned_calloc(align, size, cls->className);
    } else if (__CFAllocatorRespectsHintZeroWhenAllocating(allocator)) {
        memory = (CFRuntimeBase *)CFAllocatorAllocate(allocator, size, _CFAllocatorHintZeroWhenAllocating);
    } else {
        memory = (CFRuntimeBase *)CFAllocatorAllocate(allocator, size, 0);
        needsClear = true;
    }

    if (NULL == memory) {
	return NULL;
    } else if (needsClear) {
        memset(memory, 0, size);
    }

    if (__CFOASafe && category) {
	__CFSetLastAllocationEventName(memory, (char *)category);
    } else if (__CFOASafe) {
	__CFSetLastAllocationEventName(memory, (char *)cls->className);
    }
    if (!usesSystemDefaultAllocator) {
        // add space to hold allocator ref for non-standard allocators.
        // This means the allocator is 16 bytes before the result. See the line where we added 16 bytes above, when !usesSystemDefaultAllocator
        // This retain is balanced in _CFRelease
	*(CFAllocatorRef *)((char *)memory) = (CFAllocatorRef)CFRetain(realAllocator);
	memory = (CFRuntimeBase *)((char *)memory + 16);
    }
    
    // No need for atomic operations here - memory is currently private to this thread
    uint32_t typeIDMasked = (uint32_t)typeID << 8;
    uint32_t usesDefaultAllocatorMasked = usesSystemDefaultAllocator ? 0x80 : 0x00;
#if TARGET_RT_64_BIT
    if (customRC) {
        // The top 32 bits of the word are all FF
        // The rc bits in the lower 32 are 0xFF
        memory->_cfinfoa = (uint64_t)((0xFFFFFFFFULL << 32) | ((uint32_t)(0xFFu << 24) | RC_CUSTOM_RC_BIT | typeIDMasked | usesDefaultAllocatorMasked));
    } else {
        // The top 32 bits of the word start at 1
        // The rc bits in the lower 32 are 0
        memory->_cfinfoa = (uint64_t)((1ULL << 32) | typeIDMasked | usesDefaultAllocatorMasked);
    }
#else
    if (customRC) {
        // The rc bits in the lower 32 are 0xFF
        memory->_cfinfoa = (uint32_t)((0xFFu << 24) | RC_CUSTOM_RC_BIT | typeIDMasked | usesDefaultAllocatorMasked);
    } else {
        // The rc bits in the lower 32 are 1
        memory->_cfinfoa = (uint32_t)((1 << 24) | typeIDMasked | usesDefaultAllocatorMasked);
    }
#endif
    memory->_cfisa = __CFISAForTypeID(typeID);
    if (NULL != cls->init) {
	(cls->init)(memory);
    }
    CFTypeRef cftype = (CFTypeRef)memory;
#ifdef __clang_analyzer__
    // The analyzer doesn't understand that we've manually set the retain count of this new CFTypeRef to 1.
    CFRetain(cftype);
#endif
    return cftype;
#endif
}

#if DEPLOYMENT_RUNTIME_SWIFT
#else
void _CFRuntimeInitStaticInstance(void *ptr, CFTypeID typeID) {
    CFAssert1(typeID != _kCFRuntimeNotATypeID, __kCFLogAssertion, "%s(): Uninitialized type id", __PRETTY_FUNCTION__);
    if (__CFRuntimeClassTableSize <= typeID) HALT;
    CFRuntimeClass const *cfClass = __CFRuntimeClassTable[typeID];
    Boolean customRC = !!(cfClass->version & _kCFRuntimeCustomRefCount);
    if (customRC) {
        CFLog(kCFLogLevelError, CFSTR("*** Cannot initialize a static instance to a class (%s) with custom ref counting"), cfClass->className);
        return;
    }
    CFRuntimeBase *memory = (CFRuntimeBase *)ptr;
    // No need for atomic operations here - memory is currently private to this thread
    uint32_t typeIDMasked = (uint32_t)typeID << 8;
    uint32_t usesDefaultAllocatorMasked = 0x80;
#if TARGET_RT_64_BIT
    if (customRC) {
        // The top 32 bits of the word are the retain count
        memory->_cfinfoa = (uint64_t)((0xFFFFFFFFULL << 32) | (uint32_t)((0xFF << 24) | RC_CUSTOM_RC_BIT | typeIDMasked | usesDefaultAllocatorMasked));
    } else {
        memory->_cfinfoa = (uint64_t)(typeIDMasked | usesDefaultAllocatorMasked);
    }
#else
    if (customRC) {
        // Retain count is in cfinfo
        memory->_cfinfoa = (uint32_t)((0xFF << 24) | RC_CUSTOM_RC_BIT | typeIDMasked | usesDefaultAllocatorMasked);
    } else {
        memory->_cfinfoa = (uint32_t)(typeIDMasked | usesDefaultAllocatorMasked);
    }
#endif
    memory->_cfisa = 0;
    if (NULL != cfClass->init) {
       (cfClass->init)(memory);
    }
}
#endif

void _CFRuntimeSetInstanceTypeID(CFTypeRef cf, CFTypeID newTypeID) {
    if (__CFRuntimeClassTableSize <= newTypeID) HALT;
    __CFInfoType info = ((CFRuntimeBase *)cf)->_cfinfoa;
    CFTypeID currTypeID = __CFTypeIDFromInfo(info);
    CFRuntimeClass const *newcfClass = __CFRuntimeClassTable[newTypeID];
    Boolean newCustomRC = (newcfClass->version & _kCFRuntimeCustomRefCount);
    CFRuntimeClass const *currcfClass = __CFRuntimeClassTable[currTypeID];
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
    __CFRuntimeSetValue(cf, TYPE_ID_END, TYPE_ID_START, newTypeID);
}

CF_PRIVATE void _CFRuntimeSetInstanceTypeIDAndIsa(CFTypeRef cf, CFTypeID newTypeID) {
    _CFRuntimeSetInstanceTypeID(cf, newTypeID);
#if DEPLOYMENT_RUNTIME_SWIFT
    if (_CFTypeGetClass(cf) != __CFISAForTypeID(newTypeID)) {
        ((CFSwiftRef)cf)->isa = (uintptr_t)__CFISAForTypeID(newTypeID);
    }
#endif
}


#if !TARGET_RT_64_BIT
enum {
    __kCFObjectRetainedEvent = 12,
    __kCFObjectReleasedEvent = 13
};

#define NUM_EXTERN_TABLES 1
#define EXTERN_TABLE_IDX(O) 0

// we disguise pointers so that programs like 'leaks' forget about these references
#define DISGUISE(O) (~(uintptr_t)(O))

static struct {
    CFLock_t lock;
    CFBasicHashRef table;
//    uint8_t padding[64 - sizeof(CFBasicHashRef) - sizeof(CFLock_t)];
} __NSRetainCounters[NUM_EXTERN_TABLES];

static uintptr_t __CFDoExternRefOperation(uintptr_t op, id obj) {
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
#endif

CF_EXPORT CFTypeID CFNumberGetTypeID(void);

CF_INLINE CFTypeID __CFGenericTypeID_inline(const void *cf);

CFTypeID __CFGenericTypeID(const void *cf) {
    return __CFGenericTypeID_inline(cf);
}

CFTypeID CFTypeGetTypeID(void) {
    return _kCFRuntimeIDCFType;
}

CF_PRIVATE void __CFGenericValidateType_(CFTypeRef cf, CFTypeID type, const char *func) {
#if DEPLOYMENT_RUNTIME_SWIFT
    if (cf && CF_IS_SWIFT(type, (CFSwiftRef)cf)) return;
#endif
    if (cf && CF_IS_OBJC(type, cf)) return;
    CFAssert2((cf != NULL) && (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]) && (_kCFRuntimeIDNotAType != __CFGenericTypeID_inline(cf)) && (_kCFRuntimeIDCFType != __CFGenericTypeID_inline(cf)), __kCFLogAssertion, "%s(): pointer %p is not a CF object", func, cf); \
    CFAssert3(__CFGenericTypeID_inline(cf) == type, __kCFLogAssertion, "%s(): pointer %p is not a %s", func, cf, __CFRuntimeClassTable[type]->className);	\
}

#define __CFGenericAssertIsCF(cf) \
    CFAssert2(cf != NULL && (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]) && (_kCFRuntimeIDNotAType != __CFGenericTypeID_inline(cf)) && (_kCFRuntimeIDCFType != __CFGenericTypeID_inline(cf)), __kCFLogAssertion, "%s(): pointer %p is not a CF object", __PRETTY_FUNCTION__, cf);

#if DEPLOYMENT_RUNTIME_SWIFT

CF_INLINE Boolean CFTYPE_IS_SWIFT(const void *obj) {
    CFTypeID typeID = __CFGenericTypeID_inline(obj);
    return CF_IS_SWIFT(typeID, obj);
}

#define CFTYPE_SWIFT_FUNCDISPATCH0(rettype, obj, fn) \
    if (CFTYPE_IS_SWIFT(obj)) return (rettype)__CFSwiftBridge.fn((CFSwiftRef)obj);

#define CFTYPE_SWIFT_FUNCDISPATCH1(rettype, obj, fn, a1) \
    if (CFTYPE_IS_SWIFT(obj)) return (rettype)__CFSwiftBridge.fn((CFSwiftRef)obj, a1);

#else

#define CFTYPE_IS_SWIFT(obj) (0)
#define CFTYPE_SWIFT_FUNCDISPATCH0(rettype, obj, fn) do {} while(0)
#define CFTYPE_SWIFT_FUNCDISPATCH1(rettype, obj, fn, a1) do {} while (0)

#endif


#define CFTYPE_IS_OBJC(obj) (false)
#define CFTYPE_OBJC_FUNCDISPATCH0(rettype, obj, sel) do {} while (0)
#define CFTYPE_OBJC_FUNCDISPATCH1(rettype, obj, sel, a1) do {} while (0)


CFTypeID CFGetTypeID(CFTypeRef cf) {
#if defined(DEBUG)
    if (NULL == cf) { CRSetCrashLogMessage("*** CFGetTypeID() called with NULL ***"); HALT; }
#endif
    CFTYPE_OBJC_FUNCDISPATCH0(CFTypeID, cf, _cfTypeID);
    CFTYPE_SWIFT_FUNCDISPATCH0(CFTypeID, cf, NSObject._cfTypeID);
    
    __CFGenericAssertIsCF(cf);
    return __CFGenericTypeID_inline(cf);
}

CF_PRIVATE CFTypeID _CFGetNonObjCTypeID(CFTypeRef cf) {
    __CFGenericAssertIsCF(cf);
    return __CFGenericTypeID_inline(cf);
}

static const char *const _CFGetTypeIDDescription(CFTypeID type) {
    if (type < __CFRuntimeClassTableCount &&
        NULL != __CFRuntimeClassTable[type] &&
        _kCFRuntimeIDNotAType != type &&
        _kCFRuntimeIDCFType != type) {
        return __CFRuntimeClassTable[type]->className;
    } else {
        return NULL;
    }
}

__attribute__((cold, noinline, noreturn, not_tail_called))
CF_PRIVATE void _CFAssertMismatchedTypeID(CFTypeID expected, CFTypeID actual) {
    char msg[255];
    const char *const expectedName = _CFGetTypeIDDescription(expected) ?: "<unknown>";
    const char *const actualName = _CFGetTypeIDDescription(actual) ?: "<unknown>";
    snprintf(msg, 255, "Expected typeID %lu (%s) does not match actual typeID %lu (%s)", expected, expectedName, actual, actualName);
    HALT_MSG(msg);
}

CF_INLINE CFTypeID __CFGenericTypeID_inline(const void *cf) {
    return __CFTypeIDFromInfo(atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa)));
}


CFStringRef CFCopyTypeIDDescription(CFTypeID type) {
    CFAssert2((NULL != __CFRuntimeClassTable[type]) && _kCFRuntimeIDNotAType != type && _kCFRuntimeIDCFType != type, __kCFLogAssertion, "%s(): type %lu is not a CF type ID", __PRETTY_FUNCTION__, type);
    return CFStringCreateWithCString(kCFAllocatorSystemDefault, __CFRuntimeClassTable[type]->className, kCFStringEncodingASCII);
}

// Bit 31 (highest bit) in second word of cf instance indicates external ref count

static CFTypeRef _CFRetain(CFTypeRef cf, Boolean tryR);

CFTypeRef _CFNonObjCRetain(CFTypeRef cf) {
    __CFGenericAssertIsCF(cf);
    return _CFRetain(cf, false);
}

CFTypeRef CFRetain(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFRetain() called with NULL ***"); HALT; }
    if (cf) __CFGenericAssertIsCF(cf);
    return _CFRetain(cf, false);
}

CFTypeRef CFAutorelease(CFTypeRef __attribute__((cf_consumed)) cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFAutorelease() called with NULL ***"); HALT; }
    return cf;
}

static void _CFRelease(CFTypeRef CF_RELEASES_ARGUMENT cf);

void _CFNonObjCRelease(CFTypeRef cf) {
    __CFGenericAssertIsCF(cf);
    _CFRelease(cf);
}

void CFRelease(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFRelease() called with NULL ***"); HALT; }
    if (cf) __CFGenericAssertIsCF(cf);
    _CFRelease(cf);
}


CF_PRIVATE void __CFAllocatorDeallocate(CFTypeRef cf);

CF_PRIVATE const void *__CFStringCollectionCopy(CFAllocatorRef allocator, const void *ptr) {
    if (NULL == ptr) { CRSetCrashLogMessage("*** __CFStringCollectionCopy() called with NULL ***"); HALT; }
    CFStringRef theString = (CFStringRef)ptr;
    CFStringRef result = CFStringCreateCopy((allocator), theString);
    return (const void *)result;
}

extern void CFCollection_non_gc_storage_error(void);

CF_PRIVATE const void *__CFTypeCollectionRetain(CFAllocatorRef allocator, const void *ptr) {
    if (NULL == ptr) { CRSetCrashLogMessage("*** __CFTypeCollectionRetain() called with NULL; likely a collection has been corrupted ***"); HALT; }
    CFTypeRef cf = (CFTypeRef)ptr;
    return CFRetain(cf);
}


CF_PRIVATE void __CFTypeCollectionRelease(CFAllocatorRef allocator, const void *ptr) {
    if (NULL == ptr) { CRSetCrashLogMessage("*** __CFTypeCollectionRelease() called with NULL; likely a collection has been corrupted ***"); HALT; }
    CFTypeRef cf = (CFTypeRef)ptr;
    CFRelease(cf);
}

#if !TARGET_RT_64_BIT
static CFLock_t __CFRuntimeExternRefCountTableLock = CFLockInit;
#endif

#if DEPLOYMENT_RUNTIME_SWIFT
// using CFGetRetainCount is very dangerous; there is no real reason to use it in the swift version of CF.
#else
static uint64_t __CFGetFullRetainCount(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** __CFGetFullRetainCount() called with NULL ***"); HALT; }
#if TARGET_RT_64_BIT
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    uint32_t rc = __CFHighRCFromInfo(info);
    if (0 == rc) {
        return (uint64_t)0x0fffffffffffffffULL;
    }
    return rc;
#else
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    uint32_t lowBits = __CFLowRCFromInfo(info);
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

CF_PRIVATE Boolean __CFRuntimeIsConstant(CFTypeRef cf) {
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    uint32_t rc;
#if TARGET_RT_64_BIT
    rc = __CFHighRCFromInfo(info);
#else
    rc = __CFLowRCFromInfo(info);
#endif
    return rc == 0;
}

/// This is for use during initialization only.
CF_PRIVATE void __CFRuntimeSetRC(CFTypeRef cf, uint32_t rc) {
    // No real need for atomics or CAS here, memory is private to thread so far
    __CFInfoType info = ((CFRuntimeBase *)cf)->_cfinfoa;
#if TARGET_RT_64_BIT
    __CFBitfield64SetValue(info, HIGH_RC_END, HIGH_RC_START, rc);
#else
    __CFBitfieldSetValue(info, LOW_RC_END, LOW_RC_START, rc);
#endif
    ((CFRuntimeBase *)cf)->_cfinfoa = info;
}

CFIndex CFGetRetainCount(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFGetRetainCount() called with NULL ***"); HALT; }
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    if (info & RC_CUSTOM_RC_BIT) { // custom ref counting for object
        CFTypeID typeID = __CFTypeIDFromInfo(info);
        CFRuntimeClass const *cfClass = __CFRuntimeClassTable[typeID];
        uint32_t (*refcount)(intptr_t, CFTypeRef) = cfClass->refcount;
        if (!refcount || !(cfClass->version & _kCFRuntimeCustomRefCount) || __CFLowRCFromInfo(info) != 0xFF) {
            HALT; // bogus object
        }
#if TARGET_RT_64_BIT
        if (__CFHighRCFromInfo(info) != 0xFFFFFFFFU) {
            CRSetCrashLogMessage("Detected bogus CFTypeRef");
            HALT; // bogus object
        }
#endif
        uint32_t rc = refcount(0, cf);
#if TARGET_RT_64_BIT
        return (CFIndex)rc;
#else
        return (rc < LONG_MAX) ? (CFIndex)rc : (CFIndex)LONG_MAX;
#endif
    }
    uint64_t rc = __CFGetFullRetainCount(cf);
    return (rc < (uint64_t)LONG_MAX) ? (CFIndex)rc : (CFIndex)LONG_MAX;
}
#endif

// Even though we no longer support GC, leave implementations of these functions in for compatibility.
CFTypeRef CFMakeCollectable(CFTypeRef cf) {
    return cf;
}

CFTypeRef CFMakeUncollectable(CFTypeRef cf) {
    return cf;
}

Boolean _CFNonObjCEqual(CFTypeRef cf1, CFTypeRef cf2) {
    //cf1 is guaranteed to be non-NULL and non-ObjC, cf2 is unknown
    if (cf1 == cf2) return true;
    if (NULL == cf2) { CRSetCrashLogMessage("*** CFEqual() called with NULL second argument ***"); HALT; }
    CFTYPE_OBJC_FUNCDISPATCH1(Boolean, cf2, isEqual:, cf1);
    CFTYPE_SWIFT_FUNCDISPATCH1(Boolean, cf2, NSObject.isEqual, (CFSwiftRef)cf1);
    __CFGenericAssertIsCF(cf1);
    __CFGenericAssertIsCF(cf2);
    if (__CFGenericTypeID_inline(cf1) != __CFGenericTypeID_inline(cf2)) return false;
    if (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf1)]->equal) {
        return __CFRuntimeClassTable[__CFGenericTypeID_inline(cf1)]->equal(cf1, cf2);
    }
    return false;
}

Boolean CFEqual(CFTypeRef cf1, CFTypeRef cf2) {
    if (NULL == cf1) { CRSetCrashLogMessage("*** CFEqual() called with NULL first argument ***"); HALT; }
    if (cf1 == cf2) return true;
    if (NULL == cf2) { CRSetCrashLogMessage("*** CFEqual() called with NULL second argument ***"); HALT; }
    CFTYPE_OBJC_FUNCDISPATCH1(Boolean, cf1, isEqual:, cf2);
    CFTYPE_OBJC_FUNCDISPATCH1(Boolean, cf2, isEqual:, cf1);
    CFTYPE_SWIFT_FUNCDISPATCH1(Boolean, cf1, NSObject.isEqual, (CFSwiftRef)cf2);
    CFTYPE_SWIFT_FUNCDISPATCH1(Boolean, cf2, NSObject.isEqual, (CFSwiftRef)cf1);
    __CFGenericAssertIsCF(cf1);
    __CFGenericAssertIsCF(cf2);
    if (__CFGenericTypeID_inline(cf1) != __CFGenericTypeID_inline(cf2)) return false;
    if (NULL != __CFRuntimeClassTable[__CFGenericTypeID_inline(cf1)]->equal) {
	return __CFRuntimeClassTable[__CFGenericTypeID_inline(cf1)]->equal(cf1, cf2);
    }
    return false;
}

CFHashCode _CFNonObjCHash(CFTypeRef cf) {
    __CFGenericAssertIsCF(cf);
    CFHashCode (*hash)(CFTypeRef cf) = __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->hash;
    if (NULL != hash) {
        return hash(cf);
    }
    return (CFHashCode)cf;
}

CFHashCode CFHash(CFTypeRef cf) {
    if (NULL == cf) { CRSetCrashLogMessage("*** CFHash() called with NULL ***"); HALT; }
    CFTYPE_OBJC_FUNCDISPATCH0(CFHashCode, cf, hash);
    CFTYPE_SWIFT_FUNCDISPATCH0(CFHashCode, cf, NSObject.hash);
    __CFGenericAssertIsCF(cf);
    CFHashCode (*hash)(CFTypeRef cf) = __CFRuntimeClassTable[__CFGenericTypeID_inline(cf)]->hash; 
    if (NULL != hash) {
	return hash(cf);
    }
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
    if (_kCFRuntimeIDCFAllocator == __CFGenericTypeID_inline(cf)) {
	return __CFAllocatorGetAllocator(cf);
    }
    return __CFGetAllocator(cf);
}


extern CFTypeID CFBinaryHeapGetTypeID(void);
extern CFTypeID CFBitVectorGetTypeID(void);
extern CFTypeID CFTreeGetTypeID(void);
extern CFTypeID CFPlugInInstanceGetTypeID(void);
extern CFTypeID CFStringTokenizerGetTypeID(void);
extern CFTypeID CFStorageGetTypeID(void);
#if TARGET_OS_LINUX || TARGET_OS_BSD || (TARGET_OS_OSX && !DEPLOYMENT_RUNTIME_OBJC)
CF_PRIVATE void __CFTSDInitialize(void);
#endif
#if TARGET_OS_WIN32
// From CFPlatform.c
CF_PRIVATE void __CFTSDWindowsCleanup(void);
#endif

#if TARGET_OS_MAC
_Atomic(uint8_t) __CF_FORK_STATE = 0;
char * __crashreporter_info__ = NULL; // Keep this symbol, since it was exported and other things may be linking against it, like GraphicsServices.framework on iOS
__asm(".desc ___crashreporter_info__, 0x10");

static void __cf_atfork_prepare(void) {
    if (pthread_is_threaded_np()) {
        atomic_fetch_or(&__CF_FORK_STATE, __CF_FORK_STATE_MULTITHREADED_FLAG);
    } else {
        atomic_fetch_and(&__CF_FORK_STATE, ~__CF_FORK_STATE_MULTITHREADED_FLAG);
    }
}

static void __cf_atfork_child(void) {
    // Ideally, child-side atfork handlers should be async-cancel-safe, as fork()
    // is async-cancel-safe and can be called from signal handlers.  See also
    // http://standards.ieee.org/reading/ieee/interp/1003-1c-95_int/pasc-1003.1c-37.html
    // This is not a problem for CF.
    uint8_t const state = atomic_load_explicit(&__CF_FORK_STATE, memory_order_relaxed);
    if (state & __CF_FORK_STATE_CF_USED_FLAG) {
        atomic_fetch_or(&__CF_FORK_STATE, __CF_FORK_STATE_FORKED_FLAG);
#if TARGET_OS_OSX
        if (state & __CF_FORK_STATE_MULTITHREADED_FLAG) {
            CRSetCrashLogMessage2("*** multi-threaded process forked ***");
        } else {
            CRSetCrashLogMessage2("*** single-threaded process forked ***");
        }
#endif
    }
}

CF_PRIVATE void __CF_USED(void) {
    atomic_fetch_or(&__CF_FORK_STATE, __CF_FORK_STATE_CF_USED_FLAG);
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
void *__CFAppleLanguages = NULL;

// do not cache CFFIXED_USER_HOME or HOME, there are situations where they can change

static struct {
    const char *name;
    const char *value;
} __CFEnv[] = {
    {"USER", NULL},
#if TARGET_OS_WIN32
    {"HOMEPATH", NULL},
    {"HOMEDRIVE", NULL},
    {"USERNAME", NULL},
#endif
    {"TZFILE", NULL},
    {"TZ", NULL},
    {"NEXT_ROOT", NULL},
    {"DYLD_IMAGE_SUFFIX", NULL},
    {"CFProcessPath", NULL},
    {"CFNETWORK_LIBRARY_PATH", NULL},
    {"CFUUIDVersionNumber", NULL},
    {"CFBundleDisableStringsSharing", NULL},
    {"CFCharacterSetCheckForExpandedSet", NULL},
    {"CF_CHARSET_PATH", NULL},
    {"__CF_USER_TEXT_ENCODING", NULL},
    {"APPLE_FRAMEWORKS_ROOT", NULL},
    {"IPHONE_SIMULATOR_ROOT", NULL},
#if !DEPLOYMENT_RUNTIME_OBJC
    {"HOME", NULL},
    {"XDG_DATA_HOME", NULL},
    {"XDG_CONFIG_HOME", NULL},
    {"XDG_DATA_DIRS", NULL},
    {"XDG_CONFIG_DIRS", NULL},
    {"XDG_CACHE_HOME", NULL},
    {"XDG_RUNTIME_DIR", NULL},
#endif
    {NULL, NULL}, // the last one is for optional "COMMAND_MODE" "legacy", do not use this slot, insert before
};

CF_PRIVATE const char *__CFgetenv(const char *n) {
    for (CFIndex idx = 0; idx < sizeof(__CFEnv) / sizeof(__CFEnv[0]); idx++) {
	if (__CFEnv[idx].name && 0 == strcmp(n, __CFEnv[idx].name)) return __CFEnv[idx].value;
    }
    return getenv(n);
}

CF_PRIVATE const char *__CFgetenvIfNotRestricted(const char *n) {
    if (__CFProcessIsRestricted()) return NULL;     // !!! Assumption being this is faster than checking the env
    return __CFgetenv(n);
}

CF_PRIVATE Boolean __CFProcessIsRestricted(void) {
    return issetugid();
}

#if TARGET_OS_WIN32
#define kNilPthreadT  INVALID_HANDLE_VALUE
#else
#define kNilPthreadT  (_CFThreadRef)0
#endif


// Even though we no longer support GC, we leave this exported symbol to avoid lockstep dependencies.
// Match parity with private header
#if (TARGET_OS_MAC && !TARGET_OS_IPHONE && !__x86_64h__)
CF_EXPORT bool kCFUseCollectableAllocator;
bool kCFUseCollectableAllocator = false;
#endif


Boolean __CFProphylacticAutofsAccess = false;
static Boolean __CFInitializing = 0;
Boolean __CFInitialized = 0;

// move the next 2 lines down into the #if below, and make it static, after Foundation gets off this symbol on other platforms. 
CF_EXPORT _CFThreadRef _CFMainPThread;
_CFThreadRef _CFMainPThread = kNilPthreadT;
#if TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_BSD

CF_EXPORT _CFThreadRef _CF_pthread_main_thread_np(void);
_CFThreadRef _CF_pthread_main_thread_np(void) {
    return _CFMainPThread;
}
#define pthread_main_thread_np() _CF_pthread_main_thread_np()

#endif



#if TARGET_OS_LINUX || TARGET_OS_BSD
static void __CFInitialize(void) __attribute__ ((constructor));
#endif
#if TARGET_OS_WIN32
CF_EXPORT
#endif

CF_PRIVATE os_unfair_recursive_lock CFPlugInGlobalDataLock;

void __CFInitialize(void) {
    if (!__CFInitialized && !__CFInitializing) {
        __CFInitializing = 1;

    // This is a no-op on Darwin, but is needed on Linux and Windows.
    _CFPerformDynamicInitOfOSRecursiveLock(&CFPlugInGlobalDataLock);

#if TARGET_OS_WIN32
        if (!pthread_main_np()) HALT;   // CoreFoundation must be initialized on the main thread

        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
                        GetCurrentProcess(), &_CFMainPThread, 0, FALSE,
                        DUPLICATE_SAME_ACCESS);
#else
        // move this next line up into the #if above after Foundation gets off this symbol. Also: <rdar://problem/39622745> Stop using _CFMainPThread
        _CFMainPThread = pthread_self();
#endif

#if TARGET_OS_WIN32
        // Must not call any CF functions
        __CFTSDWindowsInitialize();
#elif TARGET_OS_LINUX || TARGET_OS_BSD || (TARGET_OS_MAC && !DEPLOYMENT_RUNTIME_OBJC)
        __CFTSDInitialize();
#endif
        __CFProphylacticAutofsAccess = true;

        for (CFIndex idx = 0; idx < sizeof(__CFEnv) / sizeof(__CFEnv[0]); idx++) {
            if (__CFEnv[idx].name) {
                char *r = NULL;
                if ((r = getenv(__CFEnv[idx].name))) {
                    __CFEnv[idx].value = r;
                }
            }
        }
        
#if TARGET_OS_MAC
	UInt32 s, r;
	__CFStringGetUserDefaultEncoding(&s, &r); // force the potential setenv to occur early
	pthread_atfork(__cf_atfork_prepare, NULL, __cf_atfork_child);
#endif


#if DEPLOYMENT_RUNTIME_SWIFT        
        extern uintptr_t __CFSwiftGetBaseClass(void);
        
        uintptr_t NSCFType = __CFSwiftGetBaseClass();
        for (CFIndex idx = 1; idx < __CFRuntimeClassTableSize; idx++) {
            __CFRuntimeObjCClassTable[idx] = NSCFType;
        }
#endif
        

#if TARGET_OS_MAC
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


#if !TARGET_RT_64_BIT
	for (CFIndex idx = 0; idx < NUM_EXTERN_TABLES; idx++) {
            CFBasicHashCallbacks callbacks = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	    __NSRetainCounters[idx].table = CFBasicHashCreate(kCFAllocatorSystemDefault, kCFBasicHashHasCounts | kCFBasicHashLinearHashing | kCFBasicHashAggressiveGrowth, &callbacks);
	    CFBasicHashSetCapacity(__NSRetainCounters[idx].table, 40);
	    __NSRetainCounters[idx].lock = CFLockInit;
	}
#endif

        /*** _CFRuntimeCreateInstance() can finally be called generally after this line. ***/

        CFNumberGetTypeID();		// NB: This does other work

        __CFCharacterSetInitialize();
        __CFDateInitialize();
        
#if DEPLOYMENT_RUNTIME_SWIFT
        extern void __CFInitializeSwift(void);
        __CFInitializeSwift();
#endif
        

        {
            CFIndex idx, cnt = 0;
            char **args = NULL;
#if TARGET_OS_MAC
            args = *_NSGetArgv();
            cnt = *_NSGetArgc();
#elif TARGET_OS_WIN32
            wchar_t *commandLine = GetCommandLineW();
            // result is actually pointer to wchar_t *, make sure to account for that below
            args = (char **)CommandLineToArgvW(commandLine, (int *)&cnt);
#endif
            CFIndex count;
            CFStringRef *list, buffer[256];
            list = (cnt <= 256) ? buffer : (CFStringRef *)malloc(cnt * sizeof(CFStringRef));
            for (idx = 0, count = 0; idx < cnt; idx++) {
                if (NULL == args[idx]) continue;
#if TARGET_OS_WIN32
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
#if TARGET_OS_WIN32
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

#if defined(DEBUG) && TARGET_OS_MAC
        CFLog(kCFLogLevelWarning, CFSTR("Assertions enabled"));
#endif

        __CFProphylacticAutofsAccess = false;

        
        __CFInitializing = 0;
        __CFInitialized = 1;
    }
}

#if TARGET_OS_WIN32

CF_PRIVATE void __CFStringCleanup(void);
CF_PRIVATE void __CFSocketCleanup(void);
CF_PRIVATE void __CFStreamCleanup(void);

static CFBundleRef RegisterCoreFoundationBundle(void) {
    wchar_t path[MAX_PATH+1];
    path[0] = path[1] = 0;
    DWORD wResult;
    CFIndex idx;

    HMODULE ourModule = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&RegisterCoreFoundationBundle, &ourModule);
    CFAssert(ourModule, __kCFLogAssertion, "GetModuleHandleExW failed");

    wResult = GetModuleFileNameW(ourModule, path, MAX_PATH+1);
    CFAssert1(wResult > 0, __kCFLogAssertion, "GetModuleFileName failed: %d", GetLastError());
    CFAssert1(wResult < MAX_PATH+1, __kCFLogAssertion, "GetModuleFileName result truncated: %s", path);

    // strip off last component, the DLL name
    PathCchRemoveFileSpec(path, wResult);

    CFStringRef fsPath = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar*)path, wcslen(path));
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

#if TARGET_OS_WIN32
        // No CF functions should access TSD after this is called
        __CFTSDWindowsCleanup();
#endif

	// do these last
	if (cfBundle) CFRelease(cfBundle);
        __CFStringCleanup();
    }
    return TRUE;
}

#endif

#if DEPLOYMENT_RUNTIME_SWIFT
extern void swift_retain(void *);
#endif

// For "tryR==true", a return of NULL means "failed".
static CFTypeRef _CFRetain(CFTypeRef cf, Boolean tryR) {
#if DEPLOYMENT_RUNTIME_SWIFT
    // We always call through to swift_retain, since all CFTypeRefs are at least _NSCFType objects
    swift_retain((void *)cf);
    return cf;
#else
    // It's important to load a 64-bit value from cfinfo when running in 64 bit - if we only fetch 32 bits then it's possible we did not atomically fetch the deallocating/deallocated flag and the retain count together (19256102). Therefore it is after this load that we check the deallocating/deallocated flag and the const-ness.
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    if (info & RC_CUSTOM_RC_BIT) {
        if (tryR) return NULL;
        CFTypeID typeID = __CFTypeIDFromInfo(info);
        CFRuntimeClass const *cfClass = __CFRuntimeClassTable[typeID];
        uint32_t (*refcount)(intptr_t, CFTypeRef) = cfClass->refcount;
        if (!refcount || !(cfClass->version & _kCFRuntimeCustomRefCount) || __CFLowRCFromInfo(info) != 0xFF) {
            CRSetCrashLogMessage("Detected bogus CFTypeRef");
            HALT; // bogus object
        }
#if TARGET_RT_64_BIT
        // Custom RC always has high bits all set
        if (__CFHighRCFromInfo(info) != 0xFFFFFFFFU) {
            CRSetCrashLogMessage("Detected bogus CFTypeRef");
            HALT; // bogus object
        }
#endif
        refcount(+1, cf);
    } else {
#if TARGET_RT_64_BIT
        __CFInfoType newInfo = info;
        do {
            if (__builtin_expect(tryR && (info & (RC_DEALLOCATING_BIT | RC_DEALLOCATED_BIT)), false)) {
                // This object is marked for deallocation
                return NULL;
            }
            
            if (__CFHighRCFromInfo(info) == 0) {
                // Constant CFTypeRef
                return cf;
            }
            
            if (__builtin_expect(__CFHighRCFromInfo(info) == ~0U, false)) {
                // Overflow will occur upon add. Turn into constant CFTypeRef (rc == 0). Retain will do nothing, but neither will release.
                __CFBitfield64SetValue(newInfo, HIGH_RC_END, HIGH_RC_START, 0);
            }
            
            // Increment the retain count and swap into place
            newInfo = info + RC_INCREMENT;
        } while (!atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo));
#else
        CFIndex rc = __CFLowRCFromInfo(info);
        if (__builtin_expect(0 == rc, 0)) return cf;    // Constant CFTypeRef
        bool success = 0;
        do {
            // if already deallocating, don't allow new retain
            if (tryR && (info & RC_DEALLOCATING_BIT)) return NULL;
            __CFInfoType newInfo = info;
            newInfo += (1 << LOW_RC_START);
            rc = __CFLowRCFromInfo(newInfo);
            if (__builtin_expect((rc & 0x7f) == 0, 0)) {
                /* Roll over another bit to the external ref count
                 Real ref count = low 7 bits of retain count in info  + external ref count << 6
                 Bit 8 of low bits indicates that external ref count is in use.
                 External ref count is shifted by 6 rather than 7 so that we can set the low
                 bits to to 1100 0000 rather than 1000 0000.
                 This prevents needing to access the external ref count for successive retains and releases
                 when the composite retain count is right around a multiple of 1 << 7.
                 */
                newInfo = info;
                __CFBitfieldSetValue(newInfo, LOW_RC_END, LOW_RC_START, ((1 << 7) | (1 << 6)));
                __CFLock(&__CFRuntimeExternRefCountTableLock);
                success = atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo);
                if (__builtin_expect(success, 1)) {
                    __CFDoExternRefOperation(350, (id)cf);
                }
                __CFUnlock(&__CFRuntimeExternRefCountTableLock);
            } else {
                success = atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo);
            }
        } while (__builtin_expect(!success, 0));
#endif
    }
    if (__builtin_expect(__CFOASafe, 0)) {
	__CFRecordAllocationEvent(__kCFRetainEvent, (void *)cf, 0, CFGetRetainCount(cf), NULL);
    }
    return cf;
#endif
}

#if DEPLOYMENT_RUNTIME_SWIFT
#else
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
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    if (info & RC_CUSTOM_RC_BIT) {
        return true;   // lie for now; weak references to these objects cannot be formed
    }
    return (info & RC_DEALLOCATING_BIT) ? true : false;
}
#endif

#if DEPLOYMENT_RUNTIME_SWIFT
extern void swift_release(void *);
#endif

static void _CFRelease(CFTypeRef CF_RELEASES_ARGUMENT cf) {
#if DEPLOYMENT_RUNTIME_SWIFT
    // We always call through to swift_release, since all CFTypeRefs are at least _NSCFType objects
    swift_release((void *)cf);
#else
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    CFTypeID typeID = __CFTypeIDFromInfo(info);
    if (info & RC_DEALLOCATED_BIT) {
        char msg[256];

        // Extra checking of values here because we're already in memory-corruption land
        if (typeID < __CFRuntimeClassTableSize) {
            CFRuntimeClass const *cfClass = __CFRuntimeClassTable[typeID];
            snprintf(msg, 256, "Detected over-release of a CFTypeRef %p (%lu / %s)", cf, typeID, cfClass ? cfClass->className : "unknown");
        } else {
            snprintf(msg, 256, "Detected over-release of a CFTypeRef %p (unknown type)", cf);
        }
        CRSetCrashLogMessage(msg);
        HALT;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
    CFIndex start_rc = __builtin_expect(__CFOASafe, 0) ? CFGetRetainCount(cf) : 0;
#pragma GCC diagnostic pop
    Boolean isAllocator = (_kCFRuntimeIDCFAllocator == typeID);

    if (info & RC_CUSTOM_RC_BIT) { // custom ref counting for object
        CFRuntimeClass const *cfClass = __CFRuntimeClassTable[typeID];
        uint32_t (*refcount)(intptr_t, CFTypeRef) = cfClass->refcount;
        if (!refcount || !(cfClass->version & _kCFRuntimeCustomRefCount) || __CFLowRCFromInfo(info) != 0xFF) {
            CRSetCrashLogMessage("Detected bogus CFTypeRef");
            HALT; // bogus object
        }
#if TARGET_RT_64_BIT
        if (__CFHighRCFromInfo(info) != 0xFFFFFFFFU) {
            CRSetCrashLogMessage("Detected bogus CFTypeRef");
            HALT; // bogus object
        }
#endif
        refcount(-1, cf);
    } else {
#if TARGET_RT_64_BIT
        uint32_t rc;
        __CFInfoType newInfo;
    again:;
        do {
            rc = __CFHighRCFromInfo(info);
            if (0 == rc) {
                return;        // Constant CFTypeRef
            }
            if (1 == rc) {
                CFRuntimeClass const *cfClass = __CFRuntimeClassTable[typeID];
                if ((cfClass->version & _kCFRuntimeResourcefulObject) && cfClass->reclaim != NULL) {
                    cfClass->reclaim(cf);
                }
                newInfo = info | RC_DEALLOCATING_BIT;
                if (!atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo)) {
                    goto again;
                }
                void (*func)(CFTypeRef) = __CFRuntimeClassTable[typeID]->finalize;
                if (NULL != func) {
                    func(cf);
                }
                // Any further ref-count changes after this point are operating on a finalized object
                // Re-load in case the finalizer call changed the ref count (blech!)
                info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
                rc = __CFHighRCFromInfo(info);
                if (isAllocator || (1 == rc)) {
                    do {
                        // hammer until it takes; trying to retain the object on another thread at this point? too late!
                        newInfo = (info | RC_DEALLOCATED_BIT) - RC_INCREMENT;
                    } while (!atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo));
                    goto really_free;
                }
                
                // drop the deallocating bit; racey, but this resurrection stuff isn't thread-safe anyway
                do {
                    info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
                    newInfo = info & ~RC_DEALLOCATING_BIT;
                } while (!atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo));
                goto again; // still need to have the effect of a CFRelease
            }
            newInfo = info - RC_INCREMENT;
        } while (!atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo));
#else
    again:;
        CFIndex rc = __CFLowRCFromInfo(info);
        if (0 == rc) {
            return;        // Constant CFTypeRef
        }
        bool success = 0;
        Boolean whack = false;
        do {
            rc = __CFLowRCFromInfo(info);
            if (1 == rc) {
                // we think cf should be deallocated
                __CFInfoType newInfo = info | RC_DEALLOCATING_BIT;
                success = atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo);
                if (success) whack = true;
            } else {
                // not yet junk
                __CFInfoType newInfo = info;
                if (rc == (1 << 7)) {
                    // Time to remove a bit from the external ref count
                    __CFLock(&__CFRuntimeExternRefCountTableLock);
                    CFIndex rcHighBitsCnt = __CFDoExternRefOperation(500, (id)cf);
                    if (1 == rcHighBitsCnt) {
                        __CFBitfieldSetValue(newInfo, LOW_RC_END, LOW_RC_START, (1 << 6) - 1);
                    } else {
                        __CFBitfieldSetValue(newInfo, LOW_RC_END, LOW_RC_START, ((1 << 6) | (1 << 7)) - 1);
                    }
                    success = atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo);
                    if (success) {
                        __CFDoExternRefOperation(450, (id)cf);
                    }
                    __CFUnlock(&__CFRuntimeExternRefCountTableLock);
                } else {
                    newInfo -= (1 << LOW_RC_START);
                    success = atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo);
                }
            }
        } while (!success);
        
        if (whack) {
            CFRuntimeClass const *cfClass = __CFRuntimeClassTable[typeID];
            if ((cfClass->version & _kCFRuntimeResourcefulObject) && cfClass->reclaim != NULL) {
                cfClass->reclaim(cf);
            }
            
            if (isAllocator) {
                goto really_free;
            } else {
                void (*func)(CFTypeRef) = __CFRuntimeClassTable[typeID]->finalize;
                if (NULL != func) {
                    func(cf);
                }
                // Any further ref-count changes after this point are operating on a finalized object
                info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
                __CFInfoType newInfo;
                rc = __CFLowRCFromInfo(info);
                if (1 == rc) {
                    do {
                        // hammer until it takes; trying to retain the object on another thread at this point? too late!
                        newInfo = info | RC_DEALLOCATED_BIT;
                    } while (!atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo));
                    goto really_free;
                }
                
                // drop the deallocating bit; racey, but this resurrection stuff isn't thread-safe anyway
                do {
                    newInfo = info & ~(RC_DEALLOCATING_BIT);
                } while (!atomic_compare_exchange_strong(&(((CFRuntimeBase *)cf)->_cfinfoa), &info, newInfo));
                goto again;
            }
        }
#endif
    }
    if (__builtin_expect(__CFOASafe, 0)) {
	__CFRecordAllocationEvent(__kCFReleaseEvent, (void *)cf, 0, start_rc - 1, NULL);
    }
    return;

    really_free:;
    if (__builtin_expect(__CFOASafe, 0)) {
	// do not use CFGetRetainCount() because cf has been freed if it was an allocator
	__CFRecordAllocationEvent(__kCFReleaseEvent, (void *)cf, 0, 0, NULL);
    }
    // cannot zombify allocators, which get deallocated by __CFAllocatorDeallocate (finalize)
    if (isAllocator) {
        __CFAllocatorDeallocate((void *)cf);
    } else {
	CFAllocatorRef allocator = kCFAllocatorSystemDefault;
        CFAllocatorRef allocatorToRelease = NULL;
	Boolean usesSystemDefaultAllocator = true;

	if (!__CFRuntimeGetFlag(cf, 7)) {
            allocator = CFGetAllocator(cf);
            usesSystemDefaultAllocator = _CFAllocatorIsSystemDefault(allocator);

            if (__kCFAllocatorTypeID_CONST != __CFGenericTypeID_inline(cf)) {
                allocatorToRelease = (CFAllocatorRef _Nonnull)__CFGetAllocator(cf);
            }
	}

	{
            // To preserve 16 byte alignment when using custom allocators, we always place the CFAllocatorRef 16 bytes before the CFType. Here we need to make sure we pass the original pointer back to the allocator for deallocating (which included the space for holding the pointer to the allocator itself).
	    CFAllocatorDeallocate(allocator, (uint8_t *)cf - (usesSystemDefaultAllocator ? 0 : 16));
	}

        // This release is balanced in _CFRuntimeCreateInstance, where we retain the allocator used. The analyzer doesn't understand this bit.
#ifndef __clang_analyzer__
	if (allocatorToRelease) {
            CFRelease(allocatorToRelease);
        }
#endif
    }
#endif
}


#if DEPLOYMENT_RUNTIME_SWIFT
struct _CFSwiftBridge __CFSwiftBridge = { { NULL } };

struct _NSCFXMLBridgeStrong __NSCFXMLBridgeStrong = {
  CFArrayGetCount,
  CFArrayGetValueAtIndex,
  CFErrorCreate,
  CFStringCreateWithCString,
  CFStringCreateMutable,
  CFStringAppend,
  CFStringAppendCString,
  CFStringGetLength,
  CFStringGetMaximumSizeForEncoding,
  CFStringGetCString,
  CFDataCreateWithBytesNoCopy,
  CFRelease,
  CFStringCreateWithBytes,
  CFArrayCreateMutable,
  CFArrayAppendValue,
  CFDataGetLength,
  CFDataGetBytePtr,
  CFDictionaryCreateMutable,
  CFDictionarySetValue,
  &kCFAllocatorSystemDefault,
  &kCFAllocatorNull,
  &kCFCopyStringDictionaryKeyCallBacks,
  &kCFTypeDictionaryValueCallBacks,
  &kCFErrorLocalizedDescriptionKey,
};

struct _NSCFXMLBridgeUntyped __NSCFXMLBridgeUntyped = {
  CFArrayGetCount,
  CFArrayGetValueAtIndex,
  CFErrorCreate,
  CFStringCreateWithCString,
  CFStringCreateMutable,
  CFStringAppend,
  CFStringAppendCString,
  CFStringGetLength,
  CFStringGetMaximumSizeForEncoding,
  CFStringGetCString,
  CFDataCreateWithBytesNoCopy,
  CFRelease,
  CFStringCreateWithBytes,
  CFArrayCreateMutable,
  CFArrayAppendValue,
  CFDataGetLength,
  CFDataGetBytePtr,
  CFDictionaryCreateMutable,
  CFDictionarySetValue,
  &kCFAllocatorSystemDefault,
  &kCFAllocatorNull,
  &kCFCopyStringDictionaryKeyCallBacks,
  &kCFTypeDictionaryValueCallBacks,
  &kCFErrorLocalizedDescriptionKey,
};

// Call out to the CF-level finalizer, because the object is going to go away.
CF_CROSS_PLATFORM_EXPORT void _CFDeinit(CFTypeRef cf) {
    __CFInfoType info = atomic_load(&(((CFRuntimeBase *)cf)->_cfinfoa));
    CFTypeID typeID = __CFTypeIDFromInfo(info);
    void (*func)(CFTypeRef) = __CFRuntimeClassTable[typeID]->finalize;
    if (NULL != func) {
        func(cf);
    }
}

bool _CFIsSwift(CFTypeID type, CFSwiftRef obj) {
    if (type == _kCFRuntimeNotATypeID) {
        return false;
    }
    if (obj->isa == (uintptr_t)__CFConstantStringClassReferencePtr) return false;
    return obj->isa != __CFRuntimeObjCClassTable[type];
}

const char *_NSPrintForDebugger(void *cf) {
    if (CF_IS_SWIFT(_kCFRuntimeNotATypeID, cf)) {
        return "Not a CF Type";
    } else {
        CFStringRef desc = CFCopyDescription((CFTypeRef)cf);
        if (!desc) {
            return "<no description>";
        }
        const char *cheapResult = CFStringGetCStringPtr((CFTypeRef)cf, kCFStringEncodingUTF8);
        if (cheapResult) {
            return cheapResult;
        }
        
        CFIndex bufferSize = 0;
        CFIndex numberConverted = CFStringGetBytes((CFStringRef)cf, CFRangeMake(0, CFStringGetLength((CFStringRef)cf)), kCFStringEncodingUTF8, 0, false, NULL, 0, &bufferSize);
        const char *result = malloc(bufferSize);
        if (!result) {
            return "<unable to fetch description>";
        }
        CFStringGetBytes((CFStringRef)cf, CFRangeMake(0, CFStringGetLength((CFStringRef)cf)), kCFStringEncodingUTF8, 0, false, (UInt8 *)result, bufferSize, NULL);
        // Yes, this leaks
        return result;
    }
}
        
CFHashCode __CFHashDouble(double d) {
    return _CFHashDouble(d);
}

void * _Nullable _CFSwiftRetain(void *_Nullable t) {
    if (t != NULL) {
        swift_retain((void *)t);
        return t;
    } else {
        return NULL;
    }
}

void _CFSwiftRelease(void *_Nullable t) {
    if (t != NULL) {
        swift_release(t);
    }
}

#endif

#undef __CFGenericAssertIsCF


