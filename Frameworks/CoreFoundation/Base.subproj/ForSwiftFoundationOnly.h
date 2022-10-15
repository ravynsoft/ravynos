// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//


#ifndef __COREFOUNDATION_FORSWIFTFOUNDATIONONLY__
#define __COREFOUNDATION_FORSWIFTFOUNDATIONONLY__ 1

#if !defined(CF_PRIVATE)
#define CF_PRIVATE extern __attribute__((__visibility__("hidden")))
#endif

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFLocking.h>
#include <CoreFoundation/CFLocaleInternal.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFRegularExpression.h>
#include <CoreFoundation/CFLogUtilities.h>
#include <CoreFoundation/CFDateIntervalFormatter.h>
#include <CoreFoundation/ForFoundationOnly.h>
#include <CoreFoundation/CFCharacterSetPriv.h>
#include <CoreFoundation/CFURLPriv.h>
#include <CoreFoundation/CFURLComponents.h>
#include <CoreFoundation/CFRunArray.h>
#include <CoreFoundation/CFDateComponents.h>

#if TARGET_OS_WIN32
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif !TARGET_OS_WASI
#include <fts.h>
#endif
#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#if _POSIX_THREADS
#include <pthread.h>
#endif
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <dirent.h>
#endif

#include <CoreFoundation/CFCalendar_Internal.h>

#if __has_include(<execinfo.h>)
#include <execinfo.h>
#endif

#if __has_include(<malloc/malloc.h>)
#include <malloc/malloc.h>
#endif

#if TARGET_OS_ANDROID
#include <errno.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <termios.h>
#elif TARGET_OS_LINUX
#include <errno.h>
#include <features.h>
#include <termios.h>

#if __GLIBC_PREREQ(2, 28) == 0
// required for statx() system call, glibc >=2.28 wraps the kernel function
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/stat.h>
#include <linux/fs.h>
#define AT_STATX_SYNC_AS_STAT   0x0000  /* - Do whatever stat() does */
#endif //__GLIBC_PREREQ(2. 28)

#ifndef __NR_statx
#include <sys/stat.h>
#endif // not __NR_statx

#endif // TARGET_OS_LINUX

#include <stdlib.h>


_CF_EXPORT_SCOPE_BEGIN

CF_PRIVATE Boolean __CFAllocatorRespectsHintZeroWhenAllocating(CFAllocatorRef _Nullable allocator);

CF_CROSS_PLATFORM_EXPORT Boolean _CFCalendarGetNextWeekend(CFCalendarRef calendar, _CFCalendarWeekendRange *range);
CF_CROSS_PLATFORM_EXPORT void _CFCalendarEnumerateDates(CFCalendarRef calendar, CFDateRef start, CFDateComponentsRef matchingComponents, CFOptionFlags opts, void (^block)(CFDateRef _Nullable, Boolean, Boolean*));
CF_EXPORT void CFCalendarSetGregorianStartDate(CFCalendarRef calendar, CFDateRef _Nullable date);
CF_EXPORT _Nullable CFDateRef CFCalendarCopyGregorianStartDate(CFCalendarRef calendar);

struct __CFSwiftObject {
    uintptr_t isa;
};

typedef struct __CFSwiftObject *CFSwiftRef;

CF_EXPORT bool _CFIsSwift(CFTypeID type, CFSwiftRef obj);
CF_EXPORT void _CFDeinit(CFTypeRef cf);

struct _NSObjectBridge {
    CFTypeID (*_cfTypeID)(CFTypeRef object);
    CFHashCode (*hash)(CFTypeRef object);
    bool (*isEqual)(CFTypeRef object, CFTypeRef other);
    _Nonnull CFTypeRef (*_Nonnull copyWithZone)(_Nonnull CFTypeRef object, _Nullable CFTypeRef zone);
};

struct _NSArrayBridge {
    CFIndex (*_Nonnull count)(CFTypeRef obj);
    _Nonnull CFTypeRef (*_Nonnull objectAtIndex)(CFTypeRef obj, CFIndex index);
    void (*_Nonnull getObjects)(CFTypeRef array, CFRange range, CFTypeRef _Nullable *_Nonnull values);
    Boolean (*_Nonnull isSubclassOfNSMutableArray)(CFTypeRef array);
};

struct _NSMutableArrayBridge {
    void (*addObject)(CFTypeRef array, CFTypeRef value);
    void (*setObject)(CFTypeRef array, CFTypeRef value, CFIndex idx);
    void (*replaceObjectAtIndex)(CFTypeRef array, CFIndex idx, CFTypeRef value);
    void (*insertObject)(CFTypeRef array, CFIndex idx, CFTypeRef value);
    void (*exchangeObjectAtIndex)(CFTypeRef array, CFIndex idx1, CFIndex idx2);
    void (*removeObjectAtIndex)(CFTypeRef array, CFIndex idx);
    void (*removeAllObjects)(CFTypeRef array);
    void (*replaceObjectsInRange)(CFTypeRef array, CFRange range, CFTypeRef _Nonnull * _Nonnull newValues, CFIndex newCount);
};

struct _NSDictionaryBridge {
    CFIndex (*count)(CFTypeRef dictionary);
    CFIndex (*countForKey)(CFTypeRef dictionary, CFTypeRef key);
    bool (*containsKey)(CFTypeRef dictionary, CFTypeRef key);
    _Nullable CFTypeRef (*_Nonnull objectForKey)(CFTypeRef dictionary, CFTypeRef key);
    bool (*_getValueIfPresent)(CFTypeRef dictionary, CFTypeRef key, CFTypeRef _Nullable *_Nullable value);
    CFIndex (*__getValue)(CFTypeRef dictionary, CFTypeRef value, CFTypeRef key);
    bool (*containsObject)(CFTypeRef dictionary, CFTypeRef value);
    CFIndex (*countForObject)(CFTypeRef dictionary, CFTypeRef value);
    void (*getObjects)(CFTypeRef dictionary, CFTypeRef _Nullable *_Nullable valuebuf, CFTypeRef _Nullable *_Nullable keybuf);
    void (*__apply)(CFTypeRef dictionary, void (*applier)(CFTypeRef key, CFTypeRef value, void *context), void *context);
    void (*enumerateKeysAndObjectsWithOptions)(CFTypeRef dictionary, CFIndex options, void (^block)(const void *key, const void *value, Boolean *stop));
    _Nonnull CFTypeRef (*_Nonnull copy)(CFTypeRef obj);
};

struct _NSMutableDictionaryBridge {
    void (*__addObject)(CFTypeRef dictionary, CFTypeRef key, CFTypeRef value);
    void (*replaceObject)(CFTypeRef dictionary, CFTypeRef key, CFTypeRef value);
    void (*__setObject)(CFTypeRef dictionary, CFTypeRef key, CFTypeRef value);
    void (*removeObjectForKey)(CFTypeRef dictionary, CFTypeRef key);
    void (*removeAllObjects)(CFTypeRef dictionary);
};

struct _NSSetBridge {
    CFIndex (*_Nonnull count)(CFTypeRef obj);
    bool (*containsObject)(CFTypeRef set, CFTypeRef value);
    _Nullable CFTypeRef (*_Nonnull __getValue)(CFTypeRef set, CFTypeRef value, CFTypeRef key);
    bool (*getValueIfPresent)(CFTypeRef set, CFTypeRef object, CFTypeRef _Nullable *_Nullable value);
    void (*getObjects)(CFTypeRef set, CFTypeRef _Nullable *_Nullable values);
    void (*__apply)(CFTypeRef set, void (*applier)(CFTypeRef value, void *context), void *context);
    _Nonnull CFTypeRef (*_Nonnull copy)(CFTypeRef obj);
    CFIndex (*_Nonnull countForKey)(CFTypeRef obj, CFTypeRef key);
    _Nullable CFTypeRef (*_Nonnull member)(CFTypeRef obj, CFTypeRef value);
};

struct _NSMutableSetBridge {
    void (*addObject)(CFTypeRef set, CFTypeRef value);
    void (*replaceObject)(CFTypeRef set, CFTypeRef value);
    void (*setObject)(CFTypeRef set, CFTypeRef value);
    void (*removeObject)(CFTypeRef set, CFTypeRef value);
    void (*removeAllObjects)(CFTypeRef set);
};

struct _NSStringBridge {
    _Nonnull CFTypeRef (*_Nonnull _createSubstringWithRange)(CFTypeRef str, CFRange range);
    _Nonnull CFTypeRef (*_Nonnull copy)(CFTypeRef str);
    _Nonnull CFTypeRef (*_Nonnull mutableCopy)(CFTypeRef str);
    CFIndex (*length)(CFTypeRef str);
    UniChar (*characterAtIndex)(CFTypeRef str, CFIndex idx);
    void (*getCharacters)(CFTypeRef str, CFRange range, UniChar *buffer);
    CFIndex (*__getBytes)(CFTypeRef str, CFStringEncoding encoding, CFRange range, uint8_t *_Nullable buffer, CFIndex maxBufLen, CFIndex *_Nullable usedBufLen);
    const char *_Nullable (*_Nonnull _fastCStringContents)(CFTypeRef str, bool nullTerminated);
    const UniChar *_Nullable (*_Nonnull _fastCharacterContents)(CFTypeRef str);
    bool (*_getCString)(CFTypeRef str, char *buffer, size_t len, UInt32 encoding);
    bool (*_encodingCantBeStoredInEightBitCFString)(CFTypeRef str);
};

struct _NSMutableStringBridge {
    void (*insertString)(CFTypeRef str, CFIndex idx, CFTypeRef inserted);
    void (*deleteCharactersInRange)(CFTypeRef str, CFRange range);
    void (*replaceCharactersInRange)(CFTypeRef str, CFRange range, CFTypeRef replacement);
    void (*setString)(CFTypeRef str, CFTypeRef replacement);
    void (*appendString)(CFTypeRef str, CFTypeRef appended);
    void (*appendCharacters)(CFTypeRef str, const UniChar *chars, CFIndex appendLength);
    void (*_cfAppendCString)(CFTypeRef str, const char *chars, CFIndex appendLength);
};

#if !TARGET_OS_WASI
struct _NSRunLoop {
    _Nonnull CFTypeRef (*_Nonnull _new)(CFRunLoopRef rl);
};
#endif

struct _NSCharacterSetBridge {
    _Nullable CFCharacterSetRef (*_Nonnull _expandedCFCharacterSet)(CFTypeRef cset);
    _Nonnull CFDataRef (*_Nonnull _retainedBitmapRepresentation)(CFTypeRef cset);
    
    bool (*_Nonnull characterIsMember)(CFTypeRef cset, UniChar ch);
    _Nonnull CFMutableCharacterSetRef (*_Nonnull mutableCopy)(CFTypeRef cset);
    bool (*_Nonnull longCharacterIsMember)(CFTypeRef cset, UTF32Char ch);
    bool (*_Nonnull hasMemberInPlane)(CFTypeRef cset, uint8_t thePlane);
    _Nonnull CFCharacterSetRef (*_Nonnull invertedSet)(CFTypeRef cset);
};

struct _NSMutableCharacterSetBridge {
    void (*_Nonnull addCharactersInRange)(CFTypeRef cset, CFRange range);
    void (*_Nonnull removeCharactersInRange)(CFTypeRef cset, CFRange range);
    void (*_Nonnull addCharactersInString)(CFTypeRef cset, CFStringRef string);
    void (*_Nonnull removeCharactersInString)(CFTypeRef cset, CFStringRef string);
    void (*_Nonnull formUnionWithCharacterSet)(CFTypeRef cset, CFTypeRef other);
    void (*_Nonnull formIntersectionWithCharacterSet)(CFTypeRef cset, CFTypeRef other);
    void (*_Nonnull invert)(CFTypeRef cset);
};

struct _NSNumberBridge {
    CFNumberType (*_Nonnull _cfNumberGetType)(CFTypeRef number);
    bool (*_Nonnull boolValue)(CFTypeRef number);
    bool (*_Nonnull _getValue)(CFTypeRef number, void *value, CFNumberType type);
};

struct _NSDataBridge {
    _Nonnull CFTypeRef (*_Nonnull copy)(CFTypeRef obj);
    CFIndex (*_Nonnull length)(CFTypeRef obj);
    const void *_Nullable (*_Nonnull bytes)(CFTypeRef obj);
    void *_Nullable (*_Nonnull mutableBytes)(CFTypeRef obj);
    void (*_Nonnull getBytes)(CFTypeRef obj, CFRange range, void *buffer);
    void (*_Nonnull setLength)(CFTypeRef obj, CFIndex newLength);
    void (*_Nonnull increaseLengthBy)(CFTypeRef obj, CFIndex extraLength);
    void (*_Nonnull appendBytes)(CFTypeRef obj, const void *bytes, CFIndex length);
    void (*_Nonnull replaceBytes)(CFTypeRef obj, CFRange range, const void *_Nullable newBytes, CFIndex newLength);
};

struct _NSCalendarBridge {
    _Nonnull CFTypeRef (*_Nonnull calendarIdentifier)(CFTypeRef obj);
    _Nullable CFTypeRef (*_Nonnull copyLocale)(CFTypeRef obj);
    void (*_Nonnull setLocale)(CFTypeRef obj, CFTypeRef _Nullable locale);
    _Nonnull CFTypeRef (*_Nonnull copyTimeZone)(CFTypeRef obj);
    void (*_Nonnull setTimeZone)(CFTypeRef obj, CFTypeRef _Nonnull timeZone);
    CFIndex (*_Nonnull firstWeekday)(CFTypeRef obj);
    void (*_Nonnull setFirstWeekday)(CFTypeRef obj, CFIndex firstWeekday);
    CFIndex (*_Nonnull minimumDaysInFirstWeek)(CFTypeRef obj);
    void (*_Nonnull setMinimumDaysInFirstWeek)(CFTypeRef obj, CFIndex minimumDays);
    _Nullable CFTypeRef (*_Nonnull copyGregorianStartDate)(CFTypeRef obj);
    void (*_Nonnull setGregorianStartDate)(CFTypeRef obj, CFTypeRef _Nullable date);
};

struct _NSURLBridge {
    Boolean (*_Nonnull copyResourcePropertyForKey)(CFTypeRef url, CFStringRef key, CFTypeRef _Nullable *_Nullable propertyValueTypeRefPtr, CFErrorRef *error);
    CFDictionaryRef _Nullable (*_Nonnull copyResourcePropertiesForKeys)(CFTypeRef url, CFArrayRef keys, CFErrorRef *error);
    Boolean (*_Nonnull setResourcePropertyForKey)(CFTypeRef url, CFStringRef key, CFTypeRef _Nullable propertyValue, CFErrorRef *error);
    Boolean (*_Nonnull setResourcePropertiesForKeys)(CFTypeRef url, CFDictionaryRef keyedPropertyValues, CFErrorRef *error);
    void (*_Nonnull clearResourcePropertyCacheForKey)(CFTypeRef url, CFStringRef key);
    void (*_Nonnull clearResourcePropertyCache)(CFTypeRef url);
    void (*_Nonnull setTemporaryResourceValueForKey)(CFTypeRef url, CFStringRef key, CFTypeRef propertyValue);
    Boolean (*_Nonnull resourceIsReachable)(CFTypeRef url, CFErrorRef *error);
};

struct _CFSwiftBridge {
    struct _NSObjectBridge NSObject;
    struct _NSArrayBridge NSArray;
    struct _NSMutableArrayBridge NSMutableArray;
    struct _NSDictionaryBridge NSDictionary;
    struct _NSMutableDictionaryBridge NSMutableDictionary;
    struct _NSSetBridge NSSet;
    struct _NSMutableSetBridge NSMutableSet;
    struct _NSStringBridge NSString;
    struct _NSMutableStringBridge NSMutableString;
#if !TARGET_OS_WASI
    struct _NSRunLoop NSRunLoop;
#endif
    struct _NSCharacterSetBridge NSCharacterSet;
    struct _NSMutableCharacterSetBridge NSMutableCharacterSet;
    struct _NSNumberBridge NSNumber;
    struct _NSDataBridge NSData;
    struct _NSCalendarBridge NSCalendar;
    struct _NSURLBridge NSURL;
};

struct _NSCFXMLBridgeStrong {
    CFIndex (* _Nonnull CFArrayGetCount)(CFArrayRef);
    const void * _Null_unspecified (* _Nonnull CFArrayGetValueAtIndex)(CFArrayRef, CFIndex);
    _Null_unspecified CFErrorRef (* _Nonnull CFErrorCreate)(CFAllocatorRef _Nullable, CFStringRef, CFIndex, CFDictionaryRef _Nullable);
    _Null_unspecified CFStringRef (* _Nonnull CFStringCreateWithCString)(CFAllocatorRef _Nullable, const char * _Null_unspecified, CFStringEncoding);
    _Null_unspecified CFMutableStringRef (* _Nonnull CFStringCreateMutable)(CFAllocatorRef _Nullable, CFIndex);
    void (* _Nonnull CFStringAppend)(CFMutableStringRef, CFStringRef);
    void (* _Nonnull CFStringAppendCString)(CFMutableStringRef, const char * _Null_unspecified, CFStringEncoding);
    CFIndex (* _Nonnull CFStringGetLength)(CFStringRef);
    CFIndex (* _Nonnull CFStringGetMaximumSizeForEncoding)(CFIndex, CFStringEncoding);
    Boolean (* _Nonnull CFStringGetCString)(CFStringRef, char *, CFIndex, CFStringEncoding);
    _Null_unspecified CFDataRef (* _Nonnull CFDataCreateWithBytesNoCopy)(CFAllocatorRef _Nullable, const uint8_t *, CFIndex, CFAllocatorRef);
    void (* _Nonnull CFRelease)(CFTypeRef);
    _Null_unspecified CFStringRef (* _Nonnull CFStringCreateWithBytes)(CFAllocatorRef _Nullable, const UInt8 *, CFIndex, CFStringEncoding, Boolean);
    _Null_unspecified CFMutableArrayRef (* _Nonnull CFArrayCreateMutable)(CFAllocatorRef _Nullable, CFIndex, const CFArrayCallBacks *_Nullable);
    void (* _Nonnull CFArrayAppendValue)(CFMutableArrayRef, const void *);
    CFIndex (* _Nonnull CFDataGetLength)(CFDataRef);
    const uint8_t * _Null_unspecified (* _Nonnull CFDataGetBytePtr)(CFDataRef);
    _Null_unspecified CFMutableDictionaryRef (* _Nonnull CFDictionaryCreateMutable)(CFAllocatorRef _Nullable, CFIndex, const CFDictionaryKeyCallBacks *, const CFDictionaryValueCallBacks *);
    void (* _Nonnull CFDictionarySetValue)(CFMutableDictionaryRef, const void * _Null_Unspecified, const void * _Null_unspecified);
    const _Null_unspecified CFAllocatorRef * _Nonnull kCFAllocatorSystemDefault;
    const _Null_unspecified CFAllocatorRef * _Nonnull kCFAllocatorNull;
    const CFDictionaryKeyCallBacks * _Nonnull kCFCopyStringDictionaryKeyCallBacks;
    const CFDictionaryValueCallBacks * _Nonnull kCFTypeDictionaryValueCallBacks;
    _Null_unspecified const CFStringRef * _Nonnull kCFErrorLocalizedDescriptionKey;
};

struct _NSCFXMLBridgeUntyped {
    void *CFArrayGetCount;
    void *CFArrayGetValueAtIndex;
    void *CFErrorCreate;
    void *CFStringCreateWithCString;
    void *CFStringCreateMutable;
    void *CFStringAppend;
    void *CFStringAppendCString;
    void *CFStringGetLength;
    void *CFStringGetMaximumSizeForEncoding;
    void *CFStringGetCString;
    void *CFDataCreateWithBytesNoCopy;
    void *CFRelease;
    void *CFStringCreateWithBytes;
    void *CFArrayCreateMutable;
    void *CFArrayAppendValue;
    void *CFDataGetLength;
    void *CFDataGetBytePtr;
    void *CFDictionaryCreateMutable;
    void *CFDictionarySetValue;
    void *kCFAllocatorSystemDefault;
    void *kCFAllocatorNull;
    void *kCFCopyStringDictionaryKeyCallBacks;
    void *kCFTypeDictionaryValueCallBacks;
    void *kCFErrorLocalizedDescriptionKey;
};

CF_EXPORT struct _NSCFXMLBridgeStrong __NSCFXMLBridgeStrong;
CF_EXPORT struct _NSCFXMLBridgeUntyped __NSCFXMLBridgeUntyped;

CF_EXPORT struct _CFSwiftBridge __CFSwiftBridge;

CF_PRIVATE void *_Nullable _CFSwiftRetain(void *_Nullable t);
CF_PRIVATE void _CFSwiftRelease(void *_Nullable t);

CF_EXPORT void _CFRuntimeBridgeTypeToClass(CFTypeID type, const void *isa);

CF_EXPORT CFNumberType _CFNumberGetType2(CFNumberRef number);

typedef	unsigned char __cf_uuid[16];
typedef	char __cf_uuid_string[37];
typedef __cf_uuid _cf_uuid_t;
typedef __cf_uuid_string _cf_uuid_string_t;

CF_EXPORT void _cf_uuid_clear(_cf_uuid_t _Nonnull uu);
CF_EXPORT int _cf_uuid_compare(const _cf_uuid_t _Nonnull uu1, const _cf_uuid_t _Nonnull uu2);
CF_EXPORT void _cf_uuid_copy(_cf_uuid_t _Nonnull dst, const _cf_uuid_t _Nonnull src);
CF_EXPORT void _cf_uuid_generate(_cf_uuid_t _Nonnull out);
CF_EXPORT void _cf_uuid_generate_random(_cf_uuid_t _Nonnull out);
CF_EXPORT void _cf_uuid_generate_time(_cf_uuid_t _Nonnull out);
CF_EXPORT int _cf_uuid_is_null(const _cf_uuid_t _Nonnull uu);
CF_EXPORT int _cf_uuid_parse(const _cf_uuid_string_t _Nonnull in, _cf_uuid_t _Nonnull uu);
CF_EXPORT void _cf_uuid_unparse(const _cf_uuid_t _Nonnull uu, _cf_uuid_string_t _Nonnull out);
CF_EXPORT void _cf_uuid_unparse_lower(const _cf_uuid_t _Nonnull uu, _cf_uuid_string_t _Nonnull out);
CF_EXPORT void _cf_uuid_unparse_upper(const _cf_uuid_t _Nonnull uu, _cf_uuid_string_t _Nonnull out);


CF_PRIVATE CFStringRef _CFProcessNameString(void);
CF_PRIVATE CFIndex __CFProcessorCount(void);
CF_PRIVATE uint64_t __CFMemorySize(void);
CF_PRIVATE CFIndex __CFActiveProcessorCount(void);
CF_CROSS_PLATFORM_EXPORT CFStringRef CFCopyFullUserName(void);

#if !TARGET_OS_WASI
extern CFWriteStreamRef _CFWriteStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd);
#endif

#if !__COREFOUNDATION_FORFOUNDATIONONLY__
typedef const struct __CFKeyedArchiverUID * CFKeyedArchiverUIDRef;
extern CFTypeID _CFKeyedArchiverUIDGetTypeID(void);
extern CFKeyedArchiverUIDRef _CFKeyedArchiverUIDCreate(CFAllocatorRef allocator, uint32_t value);
extern uint32_t _CFKeyedArchiverUIDGetValue(CFKeyedArchiverUIDRef uid);
#endif

extern CFIndex __CFBinaryPlistWriteToStream(CFPropertyListRef plist, CFTypeRef stream);
CF_CROSS_PLATFORM_EXPORT CFDataRef _CFPropertyListCreateXMLDataWithExtras(CFAllocatorRef allocator, CFPropertyListRef propertyList);

#if !TARGET_OS_WASI
extern CFWriteStreamRef _CFWriteStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd);
#endif

CF_EXPORT char *_Nullable *_Nonnull _CFEnviron(void);

CF_EXPORT void CFLog1(CFLogLevel lev, CFStringRef message);

#if TARGET_OS_WIN32
typedef void *_CFThreadRef;
typedef struct _CFThreadAttributes {
  unsigned long dwSizeOfAttributes;
  unsigned long dwThreadStackReservation;
} _CFThreadAttributes;
typedef unsigned long _CFThreadSpecificKey;
#elif _POSIX_THREADS
typedef pthread_t _CFThreadRef;
typedef pthread_attr_t _CFThreadAttributes;
typedef pthread_key_t _CFThreadSpecificKey;
#endif

CF_CROSS_PLATFORM_EXPORT Boolean _CFIsMainThread(void);
CF_EXPORT _CFThreadRef _CFMainPThread;

CF_EXPORT CFHashCode __CFHashDouble(double d);

#if __BLOCKS__
CF_CROSS_PLATFORM_EXPORT void CFSortIndexes(CFIndex *indexBuffer, CFIndex count, CFOptionFlags opts, CFComparisonResult (^cmp)(CFIndex, CFIndex));
#endif

CF_EXPORT CFTypeRef _Nullable _CFThreadSpecificGet(_CFThreadSpecificKey key);
CF_EXPORT void _CFThreadSpecificSet(_CFThreadSpecificKey key, CFTypeRef _Nullable value);
CF_EXPORT _CFThreadSpecificKey _CFThreadSpecificKeyCreate(void);

CF_EXPORT _CFThreadRef _CFThreadCreate(const _CFThreadAttributes attrs, void *_Nullable (* _Nonnull startfn)(void *_Nullable), void *_CF_RESTRICT _Nullable context);

CF_CROSS_PLATFORM_EXPORT int _CFThreadSetName(_CFThreadRef thread, const char *_Nonnull name);
CF_CROSS_PLATFORM_EXPORT int _CFThreadGetName(char *_Nonnull buf, int length);

CF_EXPORT Boolean _CFCharacterSetIsLongCharacterMember(CFCharacterSetRef theSet, UTF32Char theChar);
CF_EXPORT CFCharacterSetRef _CFCharacterSetCreateCopy(CFAllocatorRef alloc, CFCharacterSetRef theSet);
CF_EXPORT CFMutableCharacterSetRef _CFCharacterSetCreateMutableCopy(CFAllocatorRef alloc, CFCharacterSetRef theSet);
CF_CROSS_PLATFORM_EXPORT void _CFCharacterSetInitCopyingSet(CFAllocatorRef alloc, CFMutableCharacterSetRef cset, CFCharacterSetRef theSet, bool isMutable, bool validateSubclasses);

#if !TARGET_OS_WASI
CF_EXPORT _Nullable CFErrorRef CFReadStreamCopyError(CFReadStreamRef _Null_unspecified stream);

CF_EXPORT _Nullable CFErrorRef CFWriteStreamCopyError(CFWriteStreamRef _Null_unspecified stream);

CF_CROSS_PLATFORM_EXPORT CFStringRef _Nullable _CFBundleCopyExecutablePath(CFBundleRef bundle);
CF_CROSS_PLATFORM_EXPORT Boolean _CFBundleSupportsFHSBundles(void);
CF_CROSS_PLATFORM_EXPORT Boolean _CFBundleSupportsFreestandingBundles(void);
CF_CROSS_PLATFORM_EXPORT CFStringRef _Nullable _CFBundleCopyLoadedImagePathForAddress(const void *p);
#endif

CF_CROSS_PLATFORM_EXPORT CFStringRef __CFTimeZoneCopyDataVersionString(void);

CF_CROSS_PLATFORM_EXPORT void *_Nullable _CFURLCopyResourceInfo(CFURLRef url);
CF_CROSS_PLATFORM_EXPORT void *_CFURLCopyResourceInfoInitializingAtomicallyIfNeeded(CFURLRef url, CFTypeRef initialValue);
CF_CROSS_PLATFORM_EXPORT void _CFURLSetResourceInfo(CFURLRef url, CFTypeRef resourceInfo);

// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
// Version 0.8

// note: All paths set in these environment variables must be absolute.

/// a single base directory relative to which user-specific data files should be written. This directory is defined by the environment variable $XDG_DATA_HOME.
CF_EXPORT CFStringRef _CFXDGCreateDataHomePath(void);

/// a single base directory relative to which user-specific configuration files should be written. This directory is defined by the environment variable $XDG_CONFIG_HOME.
CF_EXPORT CFStringRef _CFXDGCreateConfigHomePath(void);

/// a set of preference ordered base directories relative to which data files should be searched. This set of directories is defined by the environment variable $XDG_DATA_DIRS.
CF_EXPORT CFArrayRef _CFXDGCreateDataDirectoriesPaths(void);

/// a set of preference ordered base directories relative to which configuration files should be searched. This set of directories is defined by the environment variable $XDG_CONFIG_DIRS.
CF_EXPORT CFArrayRef _CFXDGCreateConfigDirectoriesPaths(void);

/// a single base directory relative to which user-specific non-essential (cached) data should be written. This directory is defined by the environment variable $XDG_CACHE_HOME.
CF_EXPORT CFStringRef _CFXDGCreateCacheDirectoryPath(void);

/// a single base directory relative to which user-specific runtime files and other file objects should be placed. This directory is defined by the environment variable $XDG_RUNTIME_DIR.
CF_EXPORT CFStringRef _CFXDGCreateRuntimeDirectoryPath(void);

CF_CROSS_PLATFORM_EXPORT void __CFURLComponentsDeallocate(CFTypeRef cf);

typedef struct {
    void *_Nonnull memory;
    size_t capacity;
    _Bool onStack;
} _ConditionalAllocationBuffer;

static inline _Bool _resizeConditionalAllocationBuffer(_ConditionalAllocationBuffer *_Nonnull buffer, size_t amt) {
#if TARGET_OS_MAC
    size_t amount = malloc_good_size(amt);
#else
    size_t amount = amt;
#endif
    if (amount <= buffer->capacity) { return true; }
    void *newMemory;
    if (buffer->onStack) {
        newMemory = malloc(amount);
        if (newMemory == NULL) { return false; }
        memcpy(newMemory, buffer->memory, buffer->capacity);
        buffer->onStack = false;
    } else {
        newMemory = realloc(buffer->memory, amount);
        if (newMemory == NULL) { return false; }
    }
    if (newMemory == NULL) { return false; }
    buffer->memory = newMemory;
    buffer->capacity = amount;
    return true;
}

#if TARGET_OS_WASI
static inline _Bool _withStackOrHeapBuffer(size_t amount, void (__attribute__((noescape)) ^ _Nonnull applier)(_ConditionalAllocationBuffer *_Nonnull)) {
    _ConditionalAllocationBuffer buffer;
    buffer.capacity = amount;
    buffer.onStack = false;
    buffer.memory = malloc(buffer.capacity);
    if (buffer.memory == NULL) { return false; }
    applier(&buffer);
    free(buffer.memory);
    return true;
}
#else
static inline _Bool _withStackOrHeapBuffer(size_t amount, void (__attribute__((noescape)) ^ _Nonnull applier)(_ConditionalAllocationBuffer *_Nonnull)) {
    _ConditionalAllocationBuffer buffer;
#if TARGET_OS_MAC
    buffer.capacity = malloc_good_size(amount);
#else
    buffer.capacity = amount;
#endif
    buffer.onStack = (_CFIsMainThread() != 0 ? buffer.capacity < 2048 : buffer.capacity < 512);
    buffer.memory = buffer.onStack ? alloca(buffer.capacity) : malloc(buffer.capacity);
    if (buffer.memory == NULL) { return false; }
    applier(&buffer);
    if (!buffer.onStack) {
        free(buffer.memory);
    }
    return true;
}
#endif

static inline _Bool _withStackOrHeapBufferWithResultInArguments(size_t amount, void (__attribute__((noescape)) ^ _Nonnull applier)(void *_Nonnull memory, size_t capacity, _Bool onStack)) {
    return _withStackOrHeapBuffer(amount, ^(_ConditionalAllocationBuffer *buffer) {
        applier(buffer->memory, buffer->capacity, buffer->onStack);
    });
}

#pragma mark - Character Set

CF_CROSS_PLATFORM_EXPORT CFIndex __CFCharDigitValue(UniChar ch);

#pragma mark - File Functions

#if TARGET_OS_WIN32
CF_CROSS_PLATFORM_EXPORT int _CFOpenFileWithMode(const unsigned short *path, int opts, mode_t mode);
#elif !TARGET_OS_WASI
CF_CROSS_PLATFORM_EXPORT int _CFOpenFileWithMode(const char *path, int opts, mode_t mode);
#endif
CF_CROSS_PLATFORM_EXPORT void *_CFReallocf(void *ptr, size_t size);
CF_CROSS_PLATFORM_EXPORT int _CFOpenFile(const char *path, int opts);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
static inline int _direntNameLength(struct dirent *entry) {
#ifdef _D_EXACT_NAMLEN  // defined on Linux
    return _D_EXACT_NAMLEN(entry);
#elif TARGET_OS_ANDROID
    return strlen(entry->d_name);
#else
    return entry->d_namlen;
#endif
}

// major() and minor() might be implemented as macros or functions.
static inline unsigned int _dev_major(dev_t rdev) {
    return major(rdev);
}

static inline unsigned int _dev_minor(dev_t rdev) {
    return minor(rdev);
}

#endif


#if TARGET_OS_LINUX
#ifdef __NR_statx

// There is no glibc statx() function, it must be called using syscall().

static inline int
_statx(int dfd, const char *filename, unsigned int flags, unsigned int mask, struct statx *buffer) {
    int ret = syscall(__NR_statx, dfd, filename, flags, mask, buffer);
    return ret == 0 ? ret : errno;
}

// At the moment the only extra information statx() is used for is to get the btime (file creation time).
// This function is here instead of in FileManager.swift because there is no way of setting a conditional
// define that could be used with a #if in the Swift code.
static inline int
_stat_with_btime(const char *filename, struct stat *buffer, struct timespec *btime) {
    struct statx statx_buffer = {0};
    *btime = (struct timespec) {0};

    int ret = _statx(AT_FDCWD, filename, AT_SYMLINK_NOFOLLOW | AT_STATX_SYNC_AS_STAT, STATX_ALL, &statx_buffer);
    if (ret == 0) {
        *buffer = (struct stat) {
            .st_dev = makedev(statx_buffer.stx_dev_major, statx_buffer.stx_dev_minor),
            .st_ino = statx_buffer.stx_ino,
            .st_mode = statx_buffer.stx_mode,
            .st_nlink = statx_buffer.stx_nlink,
            .st_uid = statx_buffer.stx_uid,
            .st_gid = statx_buffer.stx_gid,
            .st_rdev = makedev(statx_buffer.stx_rdev_major, statx_buffer.stx_rdev_minor),
            .st_size = statx_buffer.stx_size,
            .st_blksize = statx_buffer.stx_blksize,
            .st_blocks = statx_buffer.stx_blocks,
            .st_atim = { .tv_sec = statx_buffer.stx_atime.tv_sec, .tv_nsec = statx_buffer.stx_atime.tv_nsec },
            .st_mtim = { .tv_sec = statx_buffer.stx_mtime.tv_sec, .tv_nsec = statx_buffer.stx_mtime.tv_nsec },
            .st_ctim = { .tv_sec = statx_buffer.stx_ctime.tv_sec, .tv_nsec = statx_buffer.stx_ctime.tv_nsec },
        };
        // Check that stx_btime was set in the response, not all filesystems support it.
        if (statx_buffer.stx_mask & STATX_BTIME) {
            *btime = (struct timespec) {
                .tv_sec = statx_buffer.stx_btime.tv_sec,
                .tv_nsec = statx_buffer.stx_btime.tv_nsec
            };
        } else {
            *btime = (struct timespec) {
                .tv_sec = 0,
                .tv_nsec = 0
            };
        }
    }
    return ret;
}
#else

// Dummy version when compiled where struct statx is not defined in the headers.
// Just calles lstat() instead.
static inline int
_stat_with_btime(const char *filename, struct stat *buffer, struct timespec *btime) {
    *btime = (struct timespec) {0};
    return lstat(filename, buffer) == 0 ? 0 : errno;
}
#endif // __NR_statx

static unsigned int const _CF_renameat2_RENAME_EXCHANGE = 1 << 1;
#ifdef SYS_renameat2
static _Bool const _CFHasRenameat2 = 1;
static inline int _CF_renameat2(int olddirfd, const char *_Nonnull oldpath,
                                int newdirfd, const char *_Nonnull newpath, unsigned int flags) {
    return syscall(SYS_renameat2, olddirfd, oldpath, newdirfd, newpath, flags);
}
#else
static _Bool const _CFHasRenameat2 = 0;
static inline int _CF_renameat2(int olddirfd, const char *_Nonnull oldpath,
                                int newdirfd, const char *_Nonnull newpath, unsigned int flags) {
    return ENOSYS;
}
#endif // __SYS_renameat2


#endif // TARGET_OS_LINUX

#if __HAS_STATX
#warning "Enabling statx"
#endif

#if TARGET_OS_WIN32
CF_EXPORT void __CFSocketInitializeWinSock(void);

typedef struct _REPARSE_DATA_BUFFER {
    unsigned long  ReparseTag;
    unsigned short ReparseDataLength;
    unsigned short Reserved;
    union {
        struct {
            unsigned short SubstituteNameOffset;
            unsigned short SubstituteNameLength;
            unsigned short PrintNameOffset;
            unsigned short PrintNameLength;
            unsigned long  Flags;
            short          PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            unsigned short SubstituteNameOffset;
            unsigned short SubstituteNameLength;
            unsigned short PrintNameOffset;
            unsigned short PrintNameLength;
            short          PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            unsigned char DataBuffer[1];
        } GenericReparseBuffer;
    } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER;
#endif

#if !TARGET_OS_WIN32
typedef void * _CFPosixSpawnFileActionsRef;
typedef void * _CFPosixSpawnAttrRef;
CF_EXPORT _CFPosixSpawnFileActionsRef _CFPosixSpawnFileActionsAlloc(void);
CF_EXPORT int _CFPosixSpawnFileActionsInit(_CFPosixSpawnFileActionsRef file_actions);
CF_EXPORT int _CFPosixSpawnFileActionsDestroy(_CFPosixSpawnFileActionsRef file_actions);
CF_EXPORT void _CFPosixSpawnFileActionsDealloc(_CFPosixSpawnFileActionsRef file_actions);
CF_EXPORT int _CFPosixSpawnFileActionsAddDup2(_CFPosixSpawnFileActionsRef file_actions, int filedes, int newfiledes);
CF_EXPORT int _CFPosixSpawnFileActionsAddClose(_CFPosixSpawnFileActionsRef file_actions, int filedes);
#ifdef __cplusplus
CF_EXPORT int _CFPosixSpawn(pid_t *_CF_RESTRICT pid, const char *_CF_RESTRICT path, _CFPosixSpawnFileActionsRef file_actions, _CFPosixSpawnAttrRef _Nullable _CF_RESTRICT attrp, char *const argv[], char *const envp[]);
#else
CF_EXPORT int _CFPosixSpawn(pid_t *_CF_RESTRICT pid, const char *_CF_RESTRICT path, _CFPosixSpawnFileActionsRef file_actions, _CFPosixSpawnAttrRef _Nullable _CF_RESTRICT attrp, char *_Nullable const argv[_Nullable _CF_RESTRICT], char *_Nullable const envp[_Nullable _CF_RESTRICT]);
#endif // __cplusplus
#endif // !TARGET_OS_WIN32

_CF_EXPORT_SCOPE_END

#endif /* __COREFOUNDATION_FORSWIFTFOUNDATIONONLY__ */
