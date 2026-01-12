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

/*	CFString.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Ali Ozer
        
!!! For performance reasons, it's important that all functions marked CF_INLINE in this file are inlined.
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFStringEncodingConverterExt.h>
#include <CoreFoundation/CFUniChar.h>
#include <CoreFoundation/CFUnicodeDecomposition.h>
#include <CoreFoundation/CFUnicodePrecomposition.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFNumberFormatter.h>
#include "CFInternal.h"
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#include "CFLocaleInternal.h"
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
#include <unistd.h>
#endif

#if defined(__GNUC__)
#define LONG_DOUBLE_SUPPORT 1
#else
#define LONG_DOUBLE_SUPPORT 0
#endif



#define USE_STRING_ROM 0


#ifndef INSTRUMENT_SHARED_STRINGS
#define INSTRUMENT_SHARED_STRINGS 0
#endif

CF_PRIVATE const CFStringRef __kCFLocaleCollatorID;

#if INSTRUMENT_SHARED_STRINGS
#include <sys/stat.h> /* for umask() */

static void __CFRecordStringAllocationEvent(const char *encoding, const char *bytes, CFIndex byteCount) {
    static CFLock_t lock = CFLockInit;
    
    if (memchr(bytes, '\n', byteCount)) return; //never record string allocation events for strings with newlines, because those confuse our parser and because they'll never go into the ROM
    
    __CFLock(&lock);
    static int fd;
    if (! fd) {
	extern char **_NSGetProgname(void);
	const char *name = *_NSGetProgname();
	if (! name) name = "UNKNOWN";
	umask(0);
	char path[1024];
	snprintf(path, sizeof(path), "/tmp/CFSharedStringInstrumentation_%s_%d.txt", name, getpid());
	fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0666);
	if (fd <= 0) {
	    int error = errno;
	    const char *errString = strerror(error);
	    fprintf(stderr, "open() failed with error %d (%s)\n", error, errString);
	}
    }
    if (fd > 0) {
	char *buffer = NULL;
	char formatString[256];
	snprintf(formatString, sizeof(formatString), "%%-8d\t%%-16s\t%%.%lds\n", byteCount);
	int resultCount = asprintf(&buffer, formatString, getpid(), encoding, bytes);
	if (buffer && resultCount > 0) write(fd, buffer, resultCount);
	else puts("Couldn't record allocation event");
	free(buffer);
    }
    __CFUnlock(&lock);
}
#endif //INSTRUMENT_SHARED_STRINGS

typedef Boolean (*UNI_CHAR_FUNC)(UInt32 flags, UInt8 ch, UniChar *unicodeChar);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
extern size_t malloc_good_size(size_t size);
#endif
extern void __CFStrConvertBytesToUnicode(const uint8_t *bytes, UniChar *buffer, CFIndex numChars);

static void __CFStringAppendFormatCore(CFMutableStringRef outputString, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, CFDictionaryRef stringsDictConfig, CFStringRef formatString, CFIndex initialArgPosition, const void *origValues, CFIndex originalValuesSize, va_list args);

#if defined(DEBUG)

// We put this into C & Pascal strings if we can't convert
#define CONVERSIONFAILURESTR "CFString conversion failed"

// We set this to true when purging the constant string table, so CFStringDeallocate doesn't assert
static Boolean __CFConstantStringTableBeingFreed = false;

#endif



// This section is for CFString compatibility and other behaviors...

static CFOptionFlags _CFStringCompatibilityMask = 0;

void _CFStringSetCompatibility(CFOptionFlags mask) {
    _CFStringCompatibilityMask |= mask;
}

__attribute__((used))
CF_INLINE Boolean __CFStringGetCompatibility(CFOptionFlags mask) {
    return (_CFStringCompatibilityMask & mask) == mask;
}



// Two constant strings used by CFString; these are initialized in CFStringInitialize
CONST_STRING_DECL(kCFEmptyString, "")

// This is separate for C++
struct __notInlineMutable {
    void *buffer;
    CFIndex length;
        CFIndex capacity;                           // Capacity in bytes
    unsigned int hasGap:1;                      // Currently unused
    unsigned int isFixedCapacity:1;
    unsigned int isExternalMutable:1;
    unsigned int capacityProvidedExternally:1;
#if __LP64__
    unsigned long desiredCapacity:60;
#else
    unsigned long desiredCapacity:28;
#endif
    CFAllocatorRef contentsAllocator;           // Optional
};                             // The only mutable variant for CFString


/* !!! Never do sizeof(CFString); the union is here just to make it easier to access some fields.
*/
struct __CFString {
    CFRuntimeBase base;
    union {	// In many cases the allocated structs are smaller than these
	struct __inline1 {
	    CFIndex length;
        } inline1;                                      // Bytes follow the length
	struct __notInlineImmutable1 {
	    void *buffer;                               // Note that the buffer is in the same place for all non-inline variants of CFString
	    CFIndex length;                             
	    CFAllocatorRef contentsDeallocator;		// Optional; just the dealloc func is used
	} notInlineImmutable1;                          // This is the usual not-inline immutable CFString
	struct __notInlineImmutable2 {
	    void *buffer;
	    CFAllocatorRef contentsDeallocator;		// Optional; just the dealloc func is used
	} notInlineImmutable2;                          // This is the not-inline immutable CFString when length is stored with the contents (first byte)
	struct __notInlineMutable notInlineMutable;
    } variants;
};

/* 
I = is immutable
E = not inline contents
U = is Unicode
N = has NULL byte
L = has length byte
D = explicit deallocator for contents (for mutable objects, allocator)
C = length field is CFIndex (rather than UInt32); only meaningful for 64-bit, really
    if needed this bit (valuable real-estate) can be given up for another bit elsewhere, since this info is needed just for 64-bit

Also need (only for mutable)
F = is fixed
G = has gap
Cap, DesCap = capacity

B7 B6 B5 B4 B3 B2 B1 B0
         U  N  L  C  I

B6 B5
 0  0   inline contents
 0  1   E (freed with default allocator)
 1  0   E (not freed)
 1  1   E D

!!! Note: Constant CFStrings use the bit patterns:
C8 (11001000 = default allocator, not inline, not freed contents; 8-bit; has NULL byte; doesn't have length; is immutable)
D0 (11010000 = default allocator, not inline, not freed contents; Unicode; is immutable)
The bit usages should not be modified in a way that would effect these bit patterns.
*/

enum {
    __kCFFreeContentsWhenDoneMask = 0x020,
        __kCFFreeContentsWhenDone = 0x020,
    __kCFContentsMask = 0x060,
	__kCFHasInlineContents = 0x000,
	__kCFNotInlineContentsNoFree = 0x040,		// Don't free
	__kCFNotInlineContentsDefaultFree = 0x020,	// Use allocator's free function
	__kCFNotInlineContentsCustomFree = 0x060,		// Use a specially provided free function
    __kCFHasContentsAllocatorMask = 0x060,
        __kCFHasContentsAllocator = 0x060,		// (For mutable strings) use a specially provided allocator
    __kCFHasContentsDeallocatorMask = 0x060,
        __kCFHasContentsDeallocator = 0x060,
    __kCFIsMutableMask = 0x01,
	__kCFIsMutable = 0x01,
    __kCFIsUnicodeMask = 0x10,
	__kCFIsUnicode = 0x10,
    __kCFHasNullByteMask = 0x08,
	__kCFHasNullByte = 0x08,
    __kCFHasLengthByteMask = 0x04,
	__kCFHasLengthByte = 0x04,
    // !!! Bit 0x02 has been freed up
};


// !!! Assumptions:
// Mutable strings are not inline
// Compile-time constant strings are not inline
// Mutable strings always have explicit length (but they might also have length byte and null byte)
// If there is an explicit length, always use that instead of the length byte (length byte is useful for quickly returning pascal strings)
// Never look at the length byte for the length; use __CFStrLength or __CFStrLength2

/* The following set of functions and macros need to be updated on change to the bit configuration
*/
CF_INLINE Boolean __CFStrIsMutable(CFStringRef str)                 {return (str->base._cfinfo[CF_INFO_BITS] & __kCFIsMutableMask) == __kCFIsMutable;}
CF_INLINE Boolean __CFStrIsInline(CFStringRef str)                  {return (str->base._cfinfo[CF_INFO_BITS] & __kCFContentsMask) == __kCFHasInlineContents;}
CF_INLINE Boolean __CFStrFreeContentsWhenDone(CFStringRef str)      {return (str->base._cfinfo[CF_INFO_BITS] & __kCFFreeContentsWhenDoneMask) == __kCFFreeContentsWhenDone;}
CF_INLINE Boolean __CFStrHasContentsDeallocator(CFStringRef str)    {return (str->base._cfinfo[CF_INFO_BITS] & __kCFHasContentsDeallocatorMask) == __kCFHasContentsDeallocator;}
CF_INLINE Boolean __CFStrIsUnicode(CFStringRef str)                 {return (str->base._cfinfo[CF_INFO_BITS] & __kCFIsUnicodeMask) == __kCFIsUnicode;}
CF_INLINE Boolean __CFStrIsEightBit(CFStringRef str)                {return (str->base._cfinfo[CF_INFO_BITS] & __kCFIsUnicodeMask) != __kCFIsUnicode;}
CF_INLINE Boolean __CFStrHasNullByte(CFStringRef str)               {return (str->base._cfinfo[CF_INFO_BITS] & __kCFHasNullByteMask) == __kCFHasNullByte;}
CF_INLINE Boolean __CFStrHasLengthByte(CFStringRef str)             {return (str->base._cfinfo[CF_INFO_BITS] & __kCFHasLengthByteMask) == __kCFHasLengthByte;}
CF_INLINE Boolean __CFStrHasExplicitLength(CFStringRef str)         {return (str->base._cfinfo[CF_INFO_BITS] & (__kCFIsMutableMask | __kCFHasLengthByteMask)) != __kCFHasLengthByte;}	// Has explicit length if (1) mutable or (2) not mutable and no length byte
CF_INLINE Boolean __CFStrIsConstant(CFStringRef str) {
#if __LP64__
    return str->base._rc == 0;
#else
    return (str->base._cfinfo[CF_RC_BITS]) == 0;
#endif
}

CF_INLINE SInt32 __CFStrSkipAnyLengthByte(CFStringRef str)          {return ((str->base._cfinfo[CF_INFO_BITS] & __kCFHasLengthByteMask) == __kCFHasLengthByte) ? 1 : 0;}	// Number of bytes to skip over the length byte in the contents

/* Returns ptr to the buffer (which might include the length byte).
*/
CF_INLINE const void *__CFStrContents(CFStringRef str) {
    if (__CFStrIsInline(str)) {
	return (const void *)(((uintptr_t)&(str->variants)) + (__CFStrHasExplicitLength(str) ? sizeof(CFIndex) : 0));
    } else {	// Not inline; pointer is always word 2
	return str->variants.notInlineImmutable1.buffer;
    }
}

static CFAllocatorRef *__CFStrContentsDeallocatorPtr(CFStringRef str) {
    return __CFStrHasExplicitLength(str) ? &(((CFMutableStringRef)str)->variants.notInlineImmutable1.contentsDeallocator) : &(((CFMutableStringRef)str)->variants.notInlineImmutable2.contentsDeallocator); }

// Assumption: Called with immutable strings only, and on strings that are known to have a contentsDeallocator
CF_INLINE CFAllocatorRef __CFStrContentsDeallocator(CFStringRef str) {
    return *__CFStrContentsDeallocatorPtr(str); 
}

// Assumption: Called with immutable strings only, and on strings that are known to have a contentsDeallocator
CF_INLINE void __CFStrSetContentsDeallocator(CFStringRef str, CFAllocatorRef allocator) {
    if (!(0 || 0)) CFRetain(allocator);
    *__CFStrContentsDeallocatorPtr(str) = allocator;
}

static CFAllocatorRef *__CFStrContentsAllocatorPtr(CFStringRef str) {
    CFAssert(!__CFStrIsInline(str), __kCFLogAssertion, "Asking for contents allocator of inline string");
    CFAssert(__CFStrIsMutable(str), __kCFLogAssertion, "Asking for contents allocator of an immutable string");
    return (CFAllocatorRef *)&(str->variants.notInlineMutable.contentsAllocator);
}

// Assumption: Called with strings that have a contents allocator; also, contents allocator follows custom
CF_INLINE CFAllocatorRef __CFStrContentsAllocator(CFMutableStringRef str) {
    return *(__CFStrContentsAllocatorPtr(str));
}

// Assumption: Called with strings that have a contents allocator; also, contents allocator follows custom
CF_INLINE void __CFStrSetContentsAllocator(CFMutableStringRef str, CFAllocatorRef allocator) {
    if (!(0 || 0)) CFRetain(allocator);
    *(__CFStrContentsAllocatorPtr(str)) = allocator;
}

/* Returns length; use __CFStrLength2 if contents buffer pointer has already been computed.
*/
CF_INLINE CFIndex __CFStrLength(CFStringRef str) {
    if (__CFStrHasExplicitLength(str)) {
	if (__CFStrIsInline(str)) {
            return str->variants.inline1.length;
	} else {
            return str->variants.notInlineImmutable1.length;
 	}
    } else {
	return (CFIndex)(*((uint8_t *)__CFStrContents(str)));
    }
}

CF_INLINE CFIndex __CFStrLength2(CFStringRef str, const void *buffer) {
    if (__CFStrHasExplicitLength(str)) {
        if (__CFStrIsInline(str)) {
            return str->variants.inline1.length;
	} else {
            return str->variants.notInlineImmutable1.length;
 	}
    } else {
        return (CFIndex)(*((uint8_t *)buffer));
    }
}


Boolean __CFStringIsEightBit(CFStringRef str) {
    return __CFStrIsEightBit(str);
}

/* Sets the content pointer for immutable or mutable strings.
*/
CF_INLINE void __CFStrSetContentPtr(CFStringRef str, const void *p) {
    // XXX_PCB catch all writes for mutable string case.
    __CFAssignWithWriteBarrier((void **)&((CFMutableStringRef)str)->variants.notInlineImmutable1.buffer, (void *)p);
}
CF_INLINE void __CFStrSetInfoBits(CFStringRef str, UInt32 v)		{__CFBitfieldSetValue(((CFMutableStringRef)str)->base._cfinfo[CF_INFO_BITS], 6, 0, v);}

CF_INLINE void __CFStrSetExplicitLength(CFStringRef str, CFIndex v) {
    if (__CFStrIsInline(str)) {
	((CFMutableStringRef)str)->variants.inline1.length = v;
    } else {
	((CFMutableStringRef)str)->variants.notInlineImmutable1.length = v;
    }
}

CF_INLINE void __CFStrSetUnicode(CFMutableStringRef str)		    {str->base._cfinfo[CF_INFO_BITS] |= __kCFIsUnicode;}
CF_INLINE void __CFStrClearUnicode(CFMutableStringRef str)		    {str->base._cfinfo[CF_INFO_BITS] &= ~__kCFIsUnicode;}
CF_INLINE void __CFStrSetHasLengthAndNullBytes(CFMutableStringRef str)	    {str->base._cfinfo[CF_INFO_BITS] |= (__kCFHasLengthByte | __kCFHasNullByte);}
CF_INLINE void __CFStrClearHasLengthAndNullBytes(CFMutableStringRef str)    {str->base._cfinfo[CF_INFO_BITS] &= ~(__kCFHasLengthByte | __kCFHasNullByte);}


// Assumption: The following set of inlines (using str->variants.notInlineMutable) are called with mutable strings only
CF_INLINE Boolean __CFStrIsFixed(CFStringRef str)   		{return str->variants.notInlineMutable.isFixedCapacity;}
CF_INLINE Boolean __CFStrIsExternalMutable(CFStringRef str)	{return str->variants.notInlineMutable.isExternalMutable;}
CF_INLINE Boolean __CFStrHasContentsAllocator(CFStringRef str)	{return (str->base._cfinfo[CF_INFO_BITS] & __kCFHasContentsAllocatorMask) == __kCFHasContentsAllocator;}
CF_INLINE void __CFStrSetIsFixed(CFMutableStringRef str)		    {str->variants.notInlineMutable.isFixedCapacity = 1;}
CF_INLINE void __CFStrSetIsExternalMutable(CFMutableStringRef str)	    {str->variants.notInlineMutable.isExternalMutable = 1;}
//CF_INLINE void __CFStrSetHasGap(CFMutableStringRef str)			    {str->variants.notInlineMutable.hasGap = 1;} currently unused

// If capacity is provided externally, we only change it when we need to grow beyond it
CF_INLINE Boolean __CFStrCapacityProvidedExternally(CFStringRef str)   		{return str->variants.notInlineMutable.capacityProvidedExternally;}
CF_INLINE void __CFStrSetCapacityProvidedExternally(CFMutableStringRef str)	{str->variants.notInlineMutable.capacityProvidedExternally = 1;}
CF_INLINE void __CFStrClearCapacityProvidedExternally(CFMutableStringRef str)	{str->variants.notInlineMutable.capacityProvidedExternally = 0;}

// "Capacity" is stored in number of bytes, not characters. It indicates the total number of bytes in the contents buffer.
CF_INLINE CFIndex __CFStrCapacity(CFStringRef str)				{return str->variants.notInlineMutable.capacity;}
CF_INLINE void __CFStrSetCapacity(CFMutableStringRef str, CFIndex cap)		{str->variants.notInlineMutable.capacity = cap;}

// "Desired capacity" is in number of characters; it is the client requested capacity; if fixed, it is the upper bound on the mutable string backing store.
CF_INLINE CFIndex __CFStrDesiredCapacity(CFStringRef str)			{return str->variants.notInlineMutable.desiredCapacity;}
CF_INLINE void __CFStrSetDesiredCapacity(CFMutableStringRef str, CFIndex size)	{str->variants.notInlineMutable.desiredCapacity = size;}


static void *__CFStrAllocateMutableContents(CFMutableStringRef str, CFIndex size) {
    void *ptr;
    CFAllocatorRef alloc = (__CFStrHasContentsAllocator(str)) ? __CFStrContentsAllocator(str) : __CFGetAllocator(str);
    ptr = CFAllocatorAllocate(alloc, size, 0);
    if (__CFOASafe) __CFSetLastAllocationEventName(ptr, "CFString (store)");
    return ptr;
}

static void __CFStrDeallocateMutableContents(CFMutableStringRef str, void *buffer) {
    CFAllocatorRef alloc = (__CFStrHasContentsAllocator(str)) ? __CFStrContentsAllocator(str) : __CFGetAllocator(str);
    if (__CFStrIsMutable(str) && __CFStrHasContentsAllocator(str) && (0)) {
        // do nothing
    } else if (CF_IS_COLLECTABLE_ALLOCATOR(alloc)) {
        // GC:  for finalization safety, let collector reclaim the buffer in the next GC cycle.
        auto_zone_release(objc_collectableZone(), buffer);
    } else {
        CFAllocatorDeallocate(alloc, buffer);
    }
}




/* CFString specific init flags
   Note that you cannot count on the external buffer not being copied.
   Also, if you specify an external buffer, you should not change it behind the CFString's back.
*/
enum {
    __kCFThinUnicodeIfPossible = 0x1000000,		/* See if the Unicode contents can be thinned down to 8-bit */
    kCFStringPascal = 0x10000,				/* Indicating that the string data has a Pascal string structure (length byte at start) */
    kCFStringNoCopyProvidedContents = 0x20000,		/* Don't copy the provided string contents if possible; free it when no longer needed */
    kCFStringNoCopyNoFreeProvidedContents = 0x30000	/* Don't copy the provided string contents if possible; don't free it when no longer needed */
};

/* System Encoding.
*/
static CFStringEncoding __CFDefaultSystemEncoding = kCFStringEncodingInvalidId;
static CFStringEncoding __CFDefaultFileSystemEncoding = kCFStringEncodingInvalidId;
CFStringEncoding __CFDefaultEightBitStringEncoding = kCFStringEncodingInvalidId;


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#define __defaultEncoding kCFStringEncodingMacRoman
#elif DEPLOYMENT_TARGET_WINDOWS
#define __defaultEncoding kCFStringEncodingWindowsLatin1
#else
#warning This value must match __CFGetConverter condition in CFStringEncodingConverter.c
#define __defaultEncoding kCFStringEncodingISOLatin1
#endif

CFStringEncoding CFStringGetSystemEncoding(void) {
    if (__CFDefaultSystemEncoding == kCFStringEncodingInvalidId) {
        __CFDefaultSystemEncoding = __defaultEncoding; 
        const CFStringEncodingConverter *converter = CFStringEncodingGetConverter(__CFDefaultSystemEncoding);
        __CFSetCharToUniCharFunc(converter->encodingClass == kCFStringEncodingConverterCheapEightBit ? (UNI_CHAR_FUNC)converter->toUnicode : NULL);
    }
    return __CFDefaultSystemEncoding;
}

// Fast version for internal use

CF_INLINE CFStringEncoding __CFStringGetSystemEncoding(void) {
    if (__CFDefaultSystemEncoding == kCFStringEncodingInvalidId) (void)CFStringGetSystemEncoding();
    return __CFDefaultSystemEncoding;
}

CFStringEncoding CFStringFileSystemEncoding(void) {
    if (__CFDefaultFileSystemEncoding == kCFStringEncodingInvalidId) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS
        __CFDefaultFileSystemEncoding = kCFStringEncodingUTF8;
#else
        __CFDefaultFileSystemEncoding = CFStringGetSystemEncoding();
#endif
    }

    return __CFDefaultFileSystemEncoding;
}

/* ??? Is returning length when no other answer is available the right thing?
   !!! All of the (length > (LONG_MAX / N)) type checks are to avoid wrap-around and eventual malloc overflow in the client
*/
CFIndex CFStringGetMaximumSizeForEncoding(CFIndex length, CFStringEncoding encoding) {
    if (encoding == kCFStringEncodingUTF8) {
            return (length > (LONG_MAX / 3)) ? kCFNotFound : (length * 3);
    } else if ((encoding == kCFStringEncodingUTF32) || (encoding == kCFStringEncodingUTF32BE) || (encoding == kCFStringEncodingUTF32LE)) { // UTF-32
        return (length > (LONG_MAX / sizeof(UTF32Char))) ? kCFNotFound : (length * sizeof(UTF32Char));
    } else {
        encoding &= 0xFFF; // Mask off non-base part
    }
    switch (encoding) {
        case kCFStringEncodingUnicode:
            return (length > (LONG_MAX / sizeof(UniChar))) ? kCFNotFound : (length * sizeof(UniChar));

        case kCFStringEncodingNonLossyASCII:
            return (length > (LONG_MAX / 6)) ? kCFNotFound : (length * 6);      // 1 Unichar can expand to 6 bytes

        case kCFStringEncodingMacRoman:
        case kCFStringEncodingWindowsLatin1:
        case kCFStringEncodingISOLatin1:
        case kCFStringEncodingNextStepLatin:
        case kCFStringEncodingASCII:
            return length / sizeof(uint8_t);

        default:
            return length / sizeof(uint8_t);
    }
}


/* Returns whether the indicated encoding can be stored in 8-bit chars
*/
CF_INLINE Boolean __CFStrEncodingCanBeStoredInEightBit(CFStringEncoding encoding) {
    switch (encoding & 0xFFF) { // just use encoding base
        case kCFStringEncodingInvalidId:
        case kCFStringEncodingUnicode:
        case kCFStringEncodingNonLossyASCII:
            return false;

        case kCFStringEncodingMacRoman:
        case kCFStringEncodingWindowsLatin1:
        case kCFStringEncodingISOLatin1:
        case kCFStringEncodingNextStepLatin:
        case kCFStringEncodingASCII:
            return true;

        default: return false;
    }
}

/* Returns the encoding used in eight bit CFStrings (can't be any encoding which isn't 1-to-1 with Unicode)
   For 10.9-linked apps, we've set this encoding to ASCII for all cases; see <rdar://problem/3597233>
*/
CFStringEncoding __CFStringComputeEightBitStringEncoding(void) {
    // This flag prevents recursive entry into __CFStringComputeEightBitStringEncoding
    static Boolean __CFStringIsBeingInitialized2 = false;
    if (__CFStringIsBeingInitialized2) return kCFStringEncodingASCII;
    __CFStringIsBeingInitialized2 = true;
    
    Boolean useAscii = true;
    __CFStringIsBeingInitialized2 = false;
    if (useAscii) {
        __CFDefaultEightBitStringEncoding = kCFStringEncodingASCII;
    } else {
        if (__CFDefaultEightBitStringEncoding == kCFStringEncodingInvalidId) {
            CFStringEncoding systemEncoding = CFStringGetSystemEncoding();
            if (systemEncoding == kCFStringEncodingInvalidId) { // We're right in the middle of querying system encoding from default database. Delaying to set until system encoding is determined.
              return kCFStringEncodingASCII;
            } else if (__CFStrEncodingCanBeStoredInEightBit(systemEncoding)) {
                __CFDefaultEightBitStringEncoding = systemEncoding;
            } else {
                __CFDefaultEightBitStringEncoding = kCFStringEncodingASCII;
            }
        }
    }
    return __CFDefaultEightBitStringEncoding;
}

/* Returns whether the provided bytes can be stored in ASCII
*/
CF_INLINE Boolean __CFBytesInASCII(const uint8_t *bytes, CFIndex len) {
#if __LP64__
    /* A bit of unrolling; go by 32s, 16s, and 8s first */
    while (len >= 32) {
        uint64_t val = *(const uint64_t *)bytes;
        uint64_t hiBits = (val & 0x8080808080808080ULL);    // More efficient to collect this rather than do a conditional at every step
        bytes += 8;
        val = *(const uint64_t *)bytes;
        hiBits |= (val & 0x8080808080808080ULL);
        bytes += 8;
        val = *(const uint64_t *)bytes;
        hiBits |= (val & 0x8080808080808080ULL);
        bytes += 8;
        val = *(const uint64_t *)bytes;
        if (hiBits | (val & 0x8080808080808080ULL)) return false;
        bytes += 8;
        len -= 32;
    }

    while (len >= 16) {
        uint64_t val = *(const uint64_t *)bytes;
        uint64_t hiBits = (val & 0x8080808080808080ULL);
        bytes += 8;
        val = *(const uint64_t *)bytes;
        if (hiBits | (val & 0x8080808080808080ULL)) return false;
        bytes += 8;
        len -= 16;
    }

    while (len >= 8) {
        uint64_t val = *(const uint64_t *)bytes;
        if (val & 0x8080808080808080ULL) return false;
        bytes += 8;
        len -= 8;
    }
#endif
    /* Go by 4s */
    while (len >= 4) {
        uint32_t val = *(const uint32_t *)bytes;
        if (val & 0x80808080U) return false;
        bytes += 4;
        len -= 4;
    }
    /* Handle the rest one byte at a time */
    while (len--) {
        if (*bytes++ & 0x80) return false;
    }

    return true;
}

/* Returns whether the provided 8-bit string in the specified encoding can be stored in an 8-bit CFString. 
*/
CF_INLINE Boolean __CFCanUseEightBitCFStringForBytes(const uint8_t *bytes, CFIndex len, CFStringEncoding encoding) {
    // If the encoding is the same as the 8-bit CFString encoding, we can just use the bytes as-is.
    // One exception is ASCII, which unfortunately needs to mean ISOLatin1 for compatibility reasons <rdar://problem/5458321>.
    if (encoding == __CFStringGetEightBitStringEncoding() && encoding != kCFStringEncodingASCII) return true;
    if (__CFStringEncodingIsSupersetOfASCII(encoding) && __CFBytesInASCII(bytes, len)) return true;
    return false;
}


/* Returns whether a length byte can be tacked on to a string of the indicated length.
*/
CF_INLINE Boolean __CFCanUseLengthByte(CFIndex len) {
#define __kCFMaxPascalStrLen 255	
    return (len <= __kCFMaxPascalStrLen) ? true : false;
}

/* Various string assertions
*/
#define __CFAssertIsString(cf) __CFGenericValidateType(cf, __kCFStringTypeID)
#define __CFAssertIndexIsInStringBounds(cf, idx) CFAssert3((idx) >= 0 && (idx) < __CFStrLength(cf), __kCFLogAssertion, "%s(): string index %d out of bounds (length %d)", __PRETTY_FUNCTION__, idx, __CFStrLength(cf))
#define __CFAssertRangeIsInStringBounds(cf, idx, count) CFAssert4((idx) >= 0 && (idx + count) <= __CFStrLength(cf), __kCFLogAssertion, "%s(): string range %d,%d out of bounds (length %d)", __PRETTY_FUNCTION__, idx, count, __CFStrLength(cf))
#define __CFAssertIsStringAndMutable(cf) {__CFGenericValidateType(cf, __kCFStringTypeID); CFAssert1(__CFStrIsMutable(cf), __kCFLogAssertion, "%s(): string not mutable", __PRETTY_FUNCTION__);}
#define __CFAssertIsStringAndExternalMutable(cf) {__CFGenericValidateType(cf, __kCFStringTypeID); CFAssert1(__CFStrIsMutable(cf) && __CFStrIsExternalMutable(cf), __kCFLogAssertion, "%s(): string not external mutable", __PRETTY_FUNCTION__);}
#define __CFAssertIsNotNegative(idx) CFAssert2(idx >= 0, __kCFLogAssertion, "%s(): index %d is negative", __PRETTY_FUNCTION__, idx)
#define __CFAssertIfFixedLengthIsOK(cf, reqLen) CFAssert2(!__CFStrIsFixed(cf) || (reqLen <= __CFStrDesiredCapacity(cf)), __kCFLogAssertion, "%s(): length %d too large", __PRETTY_FUNCTION__, reqLen)


/* Basic algorithm is to shrink memory when capacity is SHRINKFACTOR times the required capacity or to allocate memory when the capacity is less than GROWFACTOR times the required capacity.  This function will return -1 if the new capacity is just too big (> LONG_MAX).
Additional complications are applied in the following order:
- desiredCapacity, which is the minimum (except initially things can be at zero)
- rounding up to factor of 8
- compressing (to fit the number if 16 bits), which effectively rounds up to factor of 256
- we need to make sure GROWFACTOR computation doesn't suffer from overflow issues on 32-bit, hence the casting to unsigned. Normally for required capacity of C bytes, the allocated space is (3C+1)/2. If C > ULONG_MAX/3, we instead simply return LONG_MAX
*/
#define SHRINKFACTOR(c) (c / 2)

#if __LP64__
#define GROWFACTOR(c) ((c * 3 + 1) / 2)
#else
#define GROWFACTOR(c) (((c) >= (ULONG_MAX / 3UL)) ? __CFMax(LONG_MAX - 4095, (c)) : (((unsigned long)c * 3 + 1) / 2))
#endif

CF_INLINE CFIndex __CFStrNewCapacity(CFMutableStringRef str, unsigned long reqCapacity, CFIndex capacity, Boolean leaveExtraRoom, CFIndex charSize) {
    if (capacity != 0 || reqCapacity != 0) {	/* If initially zero, and space not needed, leave it at that... */
        if ((capacity < reqCapacity) ||		/* We definitely need the room... */
            (!__CFStrCapacityProvidedExternally(str) && 	/* Assuming we control the capacity... */
		((reqCapacity < SHRINKFACTOR(capacity)) ||		/* ...we have too much room! */
                 (!leaveExtraRoom && (reqCapacity < capacity))))) {	/* ...we need to eliminate the extra space... */
	    if (reqCapacity > LONG_MAX) return -1;  /* Too big any way you cut it */
            unsigned long newCapacity = leaveExtraRoom ? GROWFACTOR(reqCapacity) : reqCapacity;	/* Grow by 3/2 if extra room is desired */
	    CFIndex desiredCapacity = __CFStrDesiredCapacity(str) * charSize;
            if (newCapacity < desiredCapacity) {	/* If less than desired, bump up to desired */
                newCapacity = desiredCapacity;
            } else if (__CFStrIsFixed(str)) {		/* Otherwise, if fixed, no need to go above the desired (fixed) capacity */
                newCapacity = __CFMax(desiredCapacity, reqCapacity);	/* !!! So, fixed is not really fixed, but "tight" */
            }
	    if (__CFStrHasContentsAllocator(str)) {	/* Also apply any preferred size from the allocator  */
                newCapacity = CFAllocatorGetPreferredSizeForSize(__CFStrContentsAllocator(str), newCapacity, 0);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
            } else {
                newCapacity = malloc_good_size(newCapacity);
#endif
            }
            return (newCapacity > LONG_MAX) ? -1 : (CFIndex)newCapacity; // If packing: __CFStrUnpackNumber(__CFStrPackNumber(newCapacity));
        }
    }
    return capacity;
}


/* rearrangeBlocks() rearranges the blocks of data within the buffer so that they are "evenly spaced". buffer is assumed to have enough room for the result.
  numBlocks is current total number of blocks within buffer.
  blockSize is the size of each block in bytes
  ranges and numRanges hold the ranges that are no longer needed; ranges are stored sorted in increasing order, and don't overlap
  insertLength is the final spacing between the remaining blocks

Example: buffer = A B C D E F G H, blockSize = 1, ranges = { (2,1) , (4,2) }  (so we want to "delete" C and E F), fromEnd = NO
if insertLength = 4, result = A B ? ? ? ? D ? ? ? ? G H
if insertLength = 0, result = A B D G H

Example: buffer = A B C D E F G H I J K L M N O P Q R S T U, blockSize = 1, ranges { (1,1), (3,1), (5,11), (17,1), (19,1) }, fromEnd = NO
if insertLength = 3, result = A ? ? ? C ? ? ? E ? ? ? Q ? ? ? S ? ? ? U

*/
typedef struct _CFStringDeferredRange {
    CFIndex beginning;
    CFIndex length;
    CFIndex shift;
} CFStringDeferredRange;

typedef struct _CFStringStackInfo {
    CFIndex capacity;		// Capacity (if capacity == count, need to realloc to add another)
    CFIndex count;			// Number of elements actually stored
    CFStringDeferredRange *stack;
    Boolean hasMalloced;	// Indicates "stack" is allocated and needs to be deallocated when done
    char _padding[3];
} CFStringStackInfo;

CF_INLINE void pop (CFStringStackInfo *si, CFStringDeferredRange *topRange) {
    si->count = si->count - 1;
    *topRange = si->stack[si->count];
}

CF_INLINE void push (CFStringStackInfo *si, const CFStringDeferredRange *newRange) {
    if (si->count == si->capacity) {
        // increase size of the stack
        si->capacity = (si->capacity + 4) * 2;
        if (si->hasMalloced) {
            si->stack = (CFStringDeferredRange *)CFAllocatorReallocate(kCFAllocatorSystemDefault, si->stack, si->capacity * sizeof(CFStringDeferredRange), 0);
        } else {
            CFStringDeferredRange *newStack = (CFStringDeferredRange *)CFAllocatorAllocate(kCFAllocatorSystemDefault, si->capacity * sizeof(CFStringDeferredRange), 0);
            memmove(newStack, si->stack, si->count * sizeof(CFStringDeferredRange));
            si->stack = newStack;
            si->hasMalloced = true;
        }
    }
    si->stack[si->count] = *newRange;
    si->count = si->count + 1;
}

static void rearrangeBlocks(
	uint8_t *buffer, 
	CFIndex numBlocks, 
	CFIndex blockSize,
	const CFRange *ranges, 
	CFIndex numRanges, 
	CFIndex insertLength) {

#define origStackSize 10
    CFStringDeferredRange origStack[origStackSize];
    CFStringStackInfo si = {origStackSize, 0, origStack, false, {0, 0, 0}};
    CFStringDeferredRange currentNonRange = {0, 0, 0};
    CFIndex currentRange = 0;
    CFIndex amountShifted = 0;
    
    // must have at least 1 range left.
    
    while (currentRange < numRanges) {
        currentNonRange.beginning = (ranges[currentRange].location + ranges[currentRange].length) * blockSize;
        if ((numRanges - currentRange) == 1) {
            // at the end.
            currentNonRange.length = numBlocks * blockSize - currentNonRange.beginning;
            if (currentNonRange.length == 0) break;
        } else {
            currentNonRange.length = (ranges[currentRange + 1].location * blockSize) - currentNonRange.beginning;
        }
        currentNonRange.shift = amountShifted + (insertLength * blockSize) - (ranges[currentRange].length * blockSize);
        amountShifted = currentNonRange.shift;
        if (amountShifted <= 0) {
            // process current item and rest of stack
            if (currentNonRange.shift && currentNonRange.length) memmove (&buffer[currentNonRange.beginning + currentNonRange.shift], &buffer[currentNonRange.beginning], currentNonRange.length);
            while (si.count > 0) {
                pop (&si, &currentNonRange);  // currentNonRange now equals the top element of the stack.	
                if (currentNonRange.shift && currentNonRange.length) memmove (&buffer[currentNonRange.beginning + currentNonRange.shift], &buffer[currentNonRange.beginning], currentNonRange.length);
            }
        } else {
            // add currentNonRange to stack.
            push (&si, &currentNonRange);
        }
        currentRange++;
    }
    
    // no more ranges.  if anything is on the stack, process.

    while (si.count > 0) {
        pop (&si, &currentNonRange);  // currentNonRange now equals the top element of the stack.	
        if (currentNonRange.shift && currentNonRange.length) memmove (&buffer[currentNonRange.beginning + currentNonRange.shift], &buffer[currentNonRange.beginning], currentNonRange.length);
    }
    if (si.hasMalloced) CFAllocatorDeallocate (kCFAllocatorSystemDefault, si.stack);
}

/* See comments for rearrangeBlocks(); this is the same, but the string is assembled in another buffer (dstBuffer), so the algorithm is much easier. We also take care of the case where the source is not-Unicode but destination is. (The reverse case is not supported.)
*/
static void copyBlocks(
	const uint8_t *srcBuffer, 
        uint8_t *dstBuffer,
	CFIndex srcLength, 
        Boolean srcIsUnicode,
        Boolean dstIsUnicode,
	const CFRange *ranges, 
	CFIndex numRanges, 
	CFIndex insertLength) {
        
    CFIndex srcLocationInBytes = 0;	// in order to avoid multiplying all the time, this is in terms of bytes, not blocks
    CFIndex dstLocationInBytes = 0;	// ditto
    CFIndex srcBlockSize = srcIsUnicode ? sizeof(UniChar) : sizeof(uint8_t);
    CFIndex insertLengthInBytes = insertLength * (dstIsUnicode ? sizeof(UniChar) : sizeof(uint8_t));
    CFIndex rangeIndex = 0;
    CFIndex srcToDstMultiplier = (srcIsUnicode == dstIsUnicode) ? 1 : (sizeof(UniChar) / sizeof(uint8_t));
        
    // Loop over the ranges, copying the range to be preserved (right before each range)
    while (rangeIndex < numRanges) {
        CFIndex srcLengthInBytes = ranges[rangeIndex].location * srcBlockSize - srcLocationInBytes;	// srcLengthInBytes is in terms of bytes, not blocks; represents length of region to be preserved
        if (srcLengthInBytes > 0) {
            if (srcIsUnicode == dstIsUnicode) {
                memmove(dstBuffer + dstLocationInBytes, srcBuffer + srcLocationInBytes, srcLengthInBytes);
            } else {
                __CFStrConvertBytesToUnicode(srcBuffer + srcLocationInBytes, (UniChar *)(dstBuffer + dstLocationInBytes), srcLengthInBytes);
            }
        }
        srcLocationInBytes += srcLengthInBytes + ranges[rangeIndex].length * srcBlockSize;	// Skip over the just-copied and to-be-deleted stuff
        dstLocationInBytes += srcLengthInBytes * srcToDstMultiplier + insertLengthInBytes;
        rangeIndex++;
    }

    // Do last range (the one beyond last range)
    if (srcLocationInBytes < srcLength * srcBlockSize) {
        if (srcIsUnicode == dstIsUnicode) {
            memmove(dstBuffer + dstLocationInBytes, srcBuffer + srcLocationInBytes, srcLength * srcBlockSize - srcLocationInBytes);
        } else {
            __CFStrConvertBytesToUnicode(srcBuffer + srcLocationInBytes, (UniChar *)(dstBuffer + dstLocationInBytes), srcLength * srcBlockSize - srcLocationInBytes);
        }
    }
}

/* Call the callback; if it doesn't exist or returns false, then log
*/
static void __CFStringHandleOutOfMemory(CFTypeRef obj) {
    CFStringRef msg = CFSTR("Out of memory. We suggest restarting the application. If you have an unsaved document, create a backup copy in Finder, then try to save.");
    {
	CFLog(kCFLogLevelCritical, CFSTR("%@"), msg);
    }
}

/* Reallocates the backing store of the string to accomodate the new length. Space is reserved or characters are deleted as indicated by insertLength and the ranges in deleteRanges. The length is updated to reflect the new state. Will also maintain a length byte and a null byte in 8-bit strings. If length cannot fit in length byte, the space will still be reserved, but will be 0. (Hence the reason the length byte should never be looked at as length unless there is no explicit length.)
*/
static void __CFStringChangeSizeMultiple(CFMutableStringRef str, const CFRange *deleteRanges, CFIndex numDeleteRanges, CFIndex insertLength, Boolean makeUnicode) {
    const uint8_t *curContents = (uint8_t *)__CFStrContents(str);
    CFIndex curLength = curContents ? __CFStrLength2(str, curContents) : 0;
    unsigned long newLength;	// We use unsigned to better keep track of overflow
    
    // Compute new length of the string
    if (numDeleteRanges == 1) {
        newLength = curLength + insertLength - deleteRanges[0].length;
    } else {
        CFIndex cnt;
        newLength = curLength + insertLength * numDeleteRanges;
        for (cnt = 0; cnt < numDeleteRanges; cnt++) newLength -= deleteRanges[cnt].length;
    }

    __CFAssertIfFixedLengthIsOK(str, newLength);

    if (newLength == 0) {
        // An somewhat optimized code-path for this special case, with the following implicit values:
        // newIsUnicode = false
        // useLengthAndNullBytes = false
        // newCharSize = sizeof(uint8_t)
        // If the newCapacity happens to be the same as the old, we don't free the buffer; otherwise we just free it totally
        // instead of doing a potentially useless reallocation (as the needed capacity later might turn out to be different anyway)
        CFIndex curCapacity = __CFStrCapacity(str);
        CFIndex newCapacity = __CFStrNewCapacity(str, 0, curCapacity, true, sizeof(uint8_t));
        if (newCapacity != curCapacity) {	// If we're reallocing anyway (larger or smaller --- larger could happen if desired capacity was changed in the meantime), let's just free it all
            if (curContents) __CFStrDeallocateMutableContents(str, (uint8_t *)curContents);
            __CFStrSetContentPtr(str, NULL);
            __CFStrSetCapacity(str, 0);
            __CFStrClearCapacityProvidedExternally(str);
            __CFStrClearHasLengthAndNullBytes(str);
            if (!__CFStrIsExternalMutable(str)) __CFStrClearUnicode(str);	// External mutable implies Unicode
        } else {
            if (!__CFStrIsExternalMutable(str)) {
                __CFStrClearUnicode(str);
                if (curCapacity >= (int)(sizeof(uint8_t) * 2)) {	// If there's room 
                    __CFStrSetHasLengthAndNullBytes(str);
                    ((uint8_t *)curContents)[0] = ((uint8_t *)curContents)[1] = 0;
                } else {
                    __CFStrClearHasLengthAndNullBytes(str);
                }
            }
        }
        __CFStrSetExplicitLength(str, 0);
    } else {	/* This else-clause assumes newLength > 0 */
        Boolean oldIsUnicode = __CFStrIsUnicode(str);
        Boolean newIsUnicode = makeUnicode || (oldIsUnicode /* && (newLength > 0) - implicit */ ) || __CFStrIsExternalMutable(str);
        CFIndex newCharSize = newIsUnicode ? sizeof(UniChar) : sizeof(uint8_t);
        Boolean useLengthAndNullBytes = !newIsUnicode /* && (newLength > 0) - implicit */;
        CFIndex numExtraBytes = useLengthAndNullBytes ? 2 : 0;	/* 2 extra bytes to keep the length byte & null... */
        CFIndex curCapacity = __CFStrCapacity(str);
	if (newLength > (LONG_MAX - numExtraBytes) / newCharSize) __CFStringHandleOutOfMemory(str);	// Does not return
        CFIndex newCapacity = __CFStrNewCapacity(str, newLength * newCharSize + numExtraBytes, curCapacity, true, newCharSize);
	if (newCapacity == -1) __CFStringHandleOutOfMemory(str);	// Does not return
        Boolean allocNewBuffer = (newCapacity != curCapacity) || (curLength > 0 && !oldIsUnicode && newIsUnicode);	/* We alloc new buffer if oldIsUnicode != newIsUnicode because the contents have to be copied */
	uint8_t *newContents;
	if (allocNewBuffer) {
	    newContents = (uint8_t *)__CFStrAllocateMutableContents(str, newCapacity);
	    if (!newContents) {	    // Try allocating without extra room
		newCapacity = __CFStrNewCapacity(str, newLength * newCharSize + numExtraBytes, curCapacity, false, newCharSize);
		// Since we checked for this above, it shouldn't be the case here, but just in case
    		if (newCapacity == -1) __CFStringHandleOutOfMemory(str);    // Does not return
		newContents = (uint8_t *)__CFStrAllocateMutableContents(str, newCapacity);
		if (!newContents) __CFStringHandleOutOfMemory(str);	    // Does not return
	    }
	} else {
	    newContents = (uint8_t *)curContents;
	}

        Boolean hasLengthAndNullBytes = __CFStrHasLengthByte(str);
    
        CFAssert1(hasLengthAndNullBytes == __CFStrHasNullByte(str), __kCFLogAssertion, "%s(): Invalid state in 8-bit string", __PRETTY_FUNCTION__);
    
        // Calculate pointers to the actual string content (skipping over the length byte, if present).  Note that keeping a reference to the base is needed for newContents under GC, since the copy may take a long time.
        const uint8_t *curContentsBody = hasLengthAndNullBytes ? (curContents+1) : curContents;
        uint8_t *newContentsBody = useLengthAndNullBytes ? (newContents+1) : newContents;
        
        if (curContents) {
            if (oldIsUnicode == newIsUnicode) {
                if (newContentsBody == curContentsBody) {
                    rearrangeBlocks(newContentsBody, curLength, newCharSize, deleteRanges, numDeleteRanges, insertLength);
                } else {
                    copyBlocks(curContentsBody, newContentsBody, curLength, oldIsUnicode, newIsUnicode, deleteRanges, numDeleteRanges, insertLength);
                }
            } else if (newIsUnicode) {	/* this implies we have a new buffer */
                copyBlocks(curContentsBody, newContentsBody, curLength, oldIsUnicode, newIsUnicode, deleteRanges, numDeleteRanges, insertLength);
            }
            if (allocNewBuffer && __CFStrFreeContentsWhenDone(str)) __CFStrDeallocateMutableContents(str, (void *)curContents);
        }
        
        if (!newIsUnicode) {
            if (useLengthAndNullBytes) {
                newContentsBody[newLength] = 0;	/* Always have null byte, if not unicode */
                newContents[0] = __CFCanUseLengthByte(newLength) ? (uint8_t)newLength : 0;
                if (!hasLengthAndNullBytes) __CFStrSetHasLengthAndNullBytes(str);
            } else {
                if (hasLengthAndNullBytes) __CFStrClearHasLengthAndNullBytes(str);
            }
            if (oldIsUnicode) __CFStrClearUnicode(str);
        } else {	// New is unicode...
            if (!oldIsUnicode) __CFStrSetUnicode(str);
            if (hasLengthAndNullBytes) __CFStrClearHasLengthAndNullBytes(str);
        }
        __CFStrSetExplicitLength(str, newLength);
        
        if (allocNewBuffer) {
            __CFStrSetCapacity(str, newCapacity);
            __CFStrClearCapacityProvidedExternally(str);
            __CFStrSetContentPtr(str, newContents);
        }
    }
}

/* Same as above, but takes one range (very common case)
*/
CF_INLINE void __CFStringChangeSize(CFMutableStringRef str, CFRange range, CFIndex insertLength, Boolean makeUnicode) {
    __CFStringChangeSizeMultiple(str, &range, 1, insertLength, makeUnicode);
}


#if defined(DEBUG)
static Boolean __CFStrIsConstantString(CFStringRef str);
#endif

static void __CFStringDeallocate(CFTypeRef cf) {
    CFStringRef str = (CFStringRef)cf;

    // If in DEBUG mode, check to see if the string a CFSTR, and complain.
    CFAssert1(__CFConstantStringTableBeingFreed || !__CFStrIsConstantString((CFStringRef)cf), __kCFLogAssertion, "Tried to deallocate CFSTR(\"%@\")", str);

    if (!__CFStrIsInline(str)) {
        uint8_t *contents;
	Boolean isMutable = __CFStrIsMutable(str);
        if (__CFStrFreeContentsWhenDone(str) && (contents = (uint8_t *)__CFStrContents(str))) {
            if (isMutable) {
	        __CFStrDeallocateMutableContents((CFMutableStringRef)str, contents);
	    } else {
		if (__CFStrHasContentsDeallocator(str)) {
                    CFAllocatorRef allocator = __CFStrContentsDeallocator(str);
		    CFAllocatorDeallocate(allocator, contents);
		    if (!(0 || 0 )) CFRelease(allocator);
		} else {
		    CFAllocatorRef alloc = __CFGetAllocator(str);
		    CFAllocatorDeallocate(alloc, contents);
		}
	    }
	}
	if (isMutable && __CFStrHasContentsAllocator(str)) {
            CFAllocatorRef allocator = __CFStrContentsAllocator((CFMutableStringRef)str);
            if (!(0 || 0)) CFRelease(allocator);
        }
    }
}

static Boolean __CFStringEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFStringRef str1 = (CFStringRef)cf1;
    CFStringRef str2 = (CFStringRef)cf2;
    const uint8_t *contents1;
    const uint8_t *contents2;
    CFIndex len1;

    /* !!! We do not need IsString assertions, as the CFBase runtime assures this */
    /* !!! We do not need == test, as the CFBase runtime assures this */

    contents1 = (uint8_t *)__CFStrContents(str1);
    contents2 = (uint8_t *)__CFStrContents(str2);
    len1 = __CFStrLength2(str1, contents1);

    if (len1 != __CFStrLength2(str2, contents2)) return false;

    contents1 += __CFStrSkipAnyLengthByte(str1);
    contents2 += __CFStrSkipAnyLengthByte(str2);

    if (__CFStrIsEightBit(str1) && __CFStrIsEightBit(str2)) {
        return memcmp((const char *)contents1, (const char *)contents2, len1) ? false : true;
    } else if (__CFStrIsEightBit(str1)) {	/* One string has Unicode contents */
        CFStringInlineBuffer buf;
	CFIndex buf_idx = 0;

        CFStringInitInlineBuffer(str1, &buf, CFRangeMake(0, len1));
	for (buf_idx = 0; buf_idx < len1; buf_idx++) {
	    if (__CFStringGetCharacterFromInlineBufferQuick(&buf, buf_idx) != ((UniChar *)contents2)[buf_idx]) return false;
  	}
    } else if (__CFStrIsEightBit(str2)) {	/* One string has Unicode contents */
        CFStringInlineBuffer buf;
	CFIndex buf_idx = 0;

        CFStringInitInlineBuffer(str2, &buf, CFRangeMake(0, len1));
        for (buf_idx = 0; buf_idx < len1; buf_idx++) {
            if (__CFStringGetCharacterFromInlineBufferQuick(&buf, buf_idx) != ((UniChar *)contents1)[buf_idx]) return false;
        }
    } else {					/* Both strings have Unicode contents */
	CFIndex idx;
        for (idx = 0; idx < len1; idx++) {
            if (((UniChar *)contents1)[idx] != ((UniChar *)contents2)[idx]) return false;
        }
    }
    return true;
}


/* String hashing: Should give the same results whatever the encoding; so we hash UniChars.
If the length is less than or equal to 96, then the hash function is simply the 
following (n is the nth UniChar character, starting from 0):
   
  hash(-1) = length
  hash(n) = hash(n-1) * 257 + unichar(n);
  Hash = hash(length-1) * ((length & 31) + 1)

If the length is greater than 96, then the above algorithm applies to 
characters 0..31, (length/2)-16..(length/2)+15, and length-32..length-1, inclusive;
thus the first, middle, and last 32 characters.

Note that the loops below are unrolled; and: 257^2 = 66049; 257^3 = 16974593; 257^4 = 4362470401;  67503105 is 257^4 - 256^4
If hashcode is changed from UInt32 to something else, this last piece needs to be readjusted.  
!!! We haven't updated for LP64 yet

NOTE: The hash algorithm used to be duplicated in CF and Foundation; but now it should only be in the four functions below.

Hash function was changed between Panther and Tiger, and Tiger and Leopard.
*/
#define HashEverythingLimit 96

#define HashNextFourUniChars(accessStart, accessEnd, pointer) \
    {result = result * 67503105 + (accessStart 0 accessEnd) * 16974593  + (accessStart 1 accessEnd) * 66049  + (accessStart 2 accessEnd) * 257 + (accessStart 3 accessEnd); pointer += 4;}

#define HashNextUniChar(accessStart, accessEnd, pointer) \
    {result = result * 257 + (accessStart 0 accessEnd); pointer++;}


/* In this function, actualLen is the length of the original string; but len is the number of characters in buffer. The buffer is expected to contain the parts of the string relevant to hashing.
*/
CF_INLINE CFHashCode __CFStrHashCharacters(const UniChar *uContents, CFIndex len, CFIndex actualLen) {
    CFHashCode result = actualLen;
    if (len <= HashEverythingLimit) {
        const UniChar *end4 = uContents + (len & ~3);
        const UniChar *end = uContents + len;
        while (uContents < end4) HashNextFourUniChars(uContents[, ], uContents); 	// First count in fours
        while (uContents < end) HashNextUniChar(uContents[, ], uContents);		// Then for the last <4 chars, count in ones...
    } else {
        const UniChar *contents, *end;
	contents = uContents;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(contents[, ], contents);
	contents = uContents + (len >> 1) - 16;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(contents[, ], contents);
	end = uContents + len;
        contents = end - 32;
        while (contents < end) HashNextFourUniChars(contents[, ], contents);
    }
    return result + (result << (actualLen & 31));
}

/* This hashes cString in the eight bit string encoding. It also includes the little debug-time sanity check.
*/
CF_INLINE CFHashCode __CFStrHashEightBit(const uint8_t *cContents, CFIndex len) {
#if defined(DEBUG)
    if (!__CFCharToUniCharFunc) {	// A little sanity verification: If this is not set, trying to hash high byte chars would be a bad idea
        CFIndex cnt;
        Boolean err = false;
        if (len <= HashEverythingLimit) {
            for (cnt = 0; cnt < len; cnt++) if (cContents[cnt] >= 128) err = true;
        } else {
            for (cnt = 0; cnt < 32; cnt++) if (cContents[cnt] >= 128) err = true;
            for (cnt = (len >> 1) - 16; cnt < (len >> 1) + 16; cnt++) if (cContents[cnt] >= 128) err = true;
            for (cnt = (len - 32); cnt < len; cnt++) if (cContents[cnt] >= 128) err = true;
        }
        if (err) {
            // Can't do log here, as it might be too early
            fprintf(stderr, "Warning: CFHash() attempting to hash CFString containing high bytes before properly initialized to do so\n");
        }
    }
#endif
    CFHashCode result = len;
    if (len <= HashEverythingLimit) {
        const uint8_t *end4 = cContents + (len & ~3);
        const uint8_t *end = cContents + len;
        while (cContents < end4) HashNextFourUniChars(__CFCharToUniCharTable[cContents[, ]], cContents); 	// First count in fours
        while (cContents < end) HashNextUniChar(__CFCharToUniCharTable[cContents[, ]], cContents);		// Then for the last <4 chars, count in ones...
    } else {
	const uint8_t *contents, *end;
	contents = cContents;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(__CFCharToUniCharTable[contents[, ]], contents);
	contents = cContents + (len >> 1) - 16;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(__CFCharToUniCharTable[contents[, ]], contents);
	end = cContents + len;
        contents = end - 32;
        while (contents < end) HashNextFourUniChars(__CFCharToUniCharTable[contents[, ]], contents);
    }
    return result + (result << (len & 31));
}

// This is for NSStringROMKeySet.
CF_PRIVATE CFHashCode __CFStrHashEightBit2(const uint8_t *cContents, CFIndex len) {
    return __CFStrHashEightBit(cContents, len);
}

CFHashCode CFStringHashISOLatin1CString(const uint8_t *bytes, CFIndex len) {
    CFHashCode result = len;
    if (len <= HashEverythingLimit) {
        const uint8_t *end4 = bytes + (len & ~3);
        const uint8_t *end = bytes + len;
        while (bytes < end4) HashNextFourUniChars(bytes[, ], bytes); 	// First count in fours
        while (bytes < end) HashNextUniChar(bytes[, ], bytes);		// Then for the last <4 chars, count in ones...
    } else {
        const uint8_t *contents, *end;
	contents = bytes;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(contents[, ], contents);
	contents = bytes + (len >> 1) - 16;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(contents[, ], contents);
	end = bytes + len;
        contents = end - 32;
        while (contents < end) HashNextFourUniChars(contents[, ], contents);
    }
    return result + (result << (len & 31));
}

CFHashCode CFStringHashCString(const uint8_t *bytes, CFIndex len) {
    return __CFStrHashEightBit(bytes, len);
}

CFHashCode CFStringHashCharacters(const UniChar *characters, CFIndex len) {
    return __CFStrHashCharacters(characters, len, len);
}

/* This is meant to be called from NSString or subclassers only. It is an error for this to be called without the ObjC runtime or an argument which is not an NSString or subclass. It can be called with NSCFString, although that would be inefficient (causing indirection) and won't normally happen anyway, as NSCFString overrides hash.
*/
CFHashCode CFStringHashNSString(CFStringRef str) {
    UniChar buffer[HashEverythingLimit];
    CFIndex bufLen;		// Number of characters in the buffer for hashing
    CFIndex len = 0;	// Actual length of the string
    
    len = CF_OBJC_CALLV((NSString *)str, length);
    if (len <= HashEverythingLimit) {
        (void)CF_OBJC_CALLV((NSString *)str, getCharacters:buffer range:NSMakeRange(0, len));
        bufLen = len;
    } else {
        (void)CF_OBJC_CALLV((NSString *)str, getCharacters:buffer range:NSMakeRange(0, 32));
        (void)CF_OBJC_CALLV((NSString *)str, getCharacters:buffer+32 range:NSMakeRange((len >> 1) - 16, 32));
        (void)CF_OBJC_CALLV((NSString *)str, getCharacters:buffer+64 range:NSMakeRange(len - 32, 32));
        bufLen = HashEverythingLimit;
    }
    return __CFStrHashCharacters(buffer, bufLen, len);
}

CFHashCode __CFStringHash(CFTypeRef cf) {
    /* !!! We do not need an IsString assertion here, as this is called by the CFBase runtime only */
    CFStringRef str = (CFStringRef)cf;
    const uint8_t *contents = (uint8_t *)__CFStrContents(str);
    CFIndex len = __CFStrLength2(str, contents);

    if (__CFStrIsEightBit(str)) {
        contents += __CFStrSkipAnyLengthByte(str);
        return __CFStrHashEightBit(contents, len);
    } else {
        return __CFStrHashCharacters((const UniChar *)contents, len, len);
    }
}


static CFStringRef __CFStringCopyDescription(CFTypeRef cf) {
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFString %p [%p]>{contents = \"%@\"}"), cf, __CFGetAllocator(cf), cf);
}

static CFStringRef __CFStringCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    return (CFStringRef)CFStringCreateCopy(__CFGetAllocator(cf), (CFStringRef)cf);
}

static CFTypeID __kCFStringTypeID = _kCFRuntimeNotATypeID;

typedef CFTypeRef (*CF_STRING_CREATE_COPY)(CFAllocatorRef alloc, CFTypeRef theString);

static const CFRuntimeClass __CFStringClass = {
    _kCFRuntimeScannedObject,
    "CFString",
    NULL,      // init
    (CF_STRING_CREATE_COPY)CFStringCreateCopy,
    __CFStringDeallocate,
    __CFStringEqual,
    __CFStringHash,
    __CFStringCopyFormattingDescription,
    __CFStringCopyDescription
};

CF_PRIVATE void __CFStringInitialize(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFStringTypeID = _CFRuntimeRegisterClass(&__CFStringClass); });
}


CFTypeID CFStringGetTypeID(void) {
    return __kCFStringTypeID;
}


static Boolean CFStrIsUnicode(CFStringRef str) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, Boolean, (NSString *)str, _encodingCantBeStoredInEightBitCFString);
    return __CFStrIsUnicode(str);
}

    
#define ALLOCATORSFREEFUNC ((CFAllocatorRef)-1)

/* contentsDeallocator indicates how to free the data if it's noCopy == true:
	kCFAllocatorNull: don't free
	ALLOCATORSFREEFUNC: free with main allocator's free func (don't pass in the real func ptr here)
	NULL: default allocator
	otherwise it's the allocator that should be used (it will be explicitly stored)
   if noCopy == false, then freeFunc should be ALLOCATORSFREEFUNC
   hasLengthByte, hasNullByte: refers to bytes; used only if encoding != Unicode
   possiblyExternalFormat indicates that the bytes might have BOM and be swapped
   tryToReduceUnicode means that the Unicode should be checked to see if it contains just ASCII (and reduce it if so)
   numBytes contains the actual number of bytes in "bytes", including Length byte, 
	BUT not the NULL byte at the end
   bytes should not contain BOM characters
   !!! Various flags should be combined to reduce number of arguments, if possible
*/
CF_PRIVATE CFStringRef __CFStringCreateImmutableFunnel3(
                        CFAllocatorRef alloc, const void *bytes, CFIndex numBytes, CFStringEncoding encoding,
                        Boolean possiblyExternalFormat, Boolean tryToReduceUnicode, Boolean hasLengthByte, Boolean hasNullByte, Boolean noCopy,
                        CFAllocatorRef contentsDeallocator, UInt32 converterFlags) {
    
    CFMutableStringRef str = NULL;
    CFVarWidthCharBuffer vBuf;
    CFIndex size;
    Boolean useLengthByte = false;
    Boolean useNullByte = false;
    Boolean useInlineData = false;

#if INSTRUMENT_SHARED_STRINGS
    const char *recordedEncoding;
    char encodingBuffer[128];
    if (encoding == kCFStringEncodingUnicode) recordedEncoding = "Unicode";
    else if (encoding == kCFStringEncodingASCII) recordedEncoding = "ASCII";
    else if (encoding == kCFStringEncodingUTF8) recordedEncoding = "UTF8";
    else if (encoding == kCFStringEncodingMacRoman) recordedEncoding = "MacRoman";
    else {
	snprintf(encodingBuffer, sizeof(encodingBuffer), "0x%lX", (unsigned long)encoding);
	recordedEncoding = encodingBuffer;
    }
#endif

    if (alloc == NULL) alloc = __CFGetDefaultAllocator();

    if (contentsDeallocator == ALLOCATORSFREEFUNC) {
	contentsDeallocator = alloc;
    } else if (contentsDeallocator == NULL) {
	contentsDeallocator = __CFGetDefaultAllocator();
    }

    if ((NULL != kCFEmptyString) && (numBytes == 0) && _CFAllocatorIsSystemDefault(alloc)) {	// If we are using the system default allocator, and the string is empty, then use the empty string!
	if (noCopy && (contentsDeallocator != kCFAllocatorNull)) {	// See 2365208... This change was done after Sonata; before we didn't free the bytes at all (leak).
	    CFAllocatorDeallocate(contentsDeallocator, (void *)bytes); 
	}
	return (CFStringRef)CFRetain(kCFEmptyString);	// Quick exit; won't catch all empty strings, but most
    }

    // At this point, contentsDeallocator is either same as alloc, or kCFAllocatorNull, or something else, but not NULL

    vBuf.shouldFreeChars = false;	// We use this to remember to free the buffer possibly allocated by decode

    // Record whether we're starting out with an ASCII-superset string, because we need to know this later for the string ROM; this may get changed later if we successfully convert down from Unicode.  We only record this once because __CFCanUseEightBitCFStringForBytes() can be expensive.
    Boolean stringSupportsEightBitCFRepresentation = encoding != kCFStringEncodingUnicode && __CFCanUseEightBitCFStringForBytes((const uint8_t *)bytes, numBytes, encoding);
    
    // We may also change noCopy within this function if we have to decode the string into an external buffer.  We do not want to avoid the use of the string ROM merely because we tried to be efficient and reuse the decoded buffer for the CFString's external storage.  Therefore, we use this variable to track whether we actually can ignore the noCopy flag (which may or may not be set anyways).
    Boolean stringROMShouldIgnoreNoCopy = false;

    // First check to see if the data needs to be converted...
    // ??? We could be more efficient here and in some cases (Unicode data) eliminate a copy

    if ((encoding == kCFStringEncodingUnicode && possiblyExternalFormat) || (encoding != kCFStringEncodingUnicode && ! stringSupportsEightBitCFRepresentation)) {
        const void *realBytes = (uint8_t *) bytes + (hasLengthByte ? 1 : 0);
        CFIndex realNumBytes = numBytes - (hasLengthByte ? 1 : 0);
        Boolean usingPassedInMemory = false;

	vBuf.allocator = kCFAllocatorSystemDefault;	// We don't want to use client's allocator for temp stuff
        vBuf.chars.unicode = NULL;	// This will cause the decode function to allocate memory if necessary

        if (!__CFStringDecodeByteStream3((const uint8_t *)realBytes, realNumBytes, encoding, false, &vBuf, &usingPassedInMemory, converterFlags)) {
	    // Note that if the string can't be created, we don't free the buffer, even if there is a contents deallocator. This is on purpose.
	    return NULL;
	}

        encoding = vBuf.isASCII ? kCFStringEncodingASCII : kCFStringEncodingUnicode;
	
	// Update our flag according to whether the decoded buffer is ASCII
	stringSupportsEightBitCFRepresentation = vBuf.isASCII;
	
        if (!usingPassedInMemory) {

	    // Because __CFStringDecodeByteStream3() allocated our buffer, it's OK for us to free it if we can get the string from the ROM.
	    stringROMShouldIgnoreNoCopy = true;

            // Make the parameters fit the new situation
            numBytes = vBuf.isASCII ? vBuf.numChars : (vBuf.numChars * sizeof(UniChar));
            hasLengthByte = hasNullByte = false;

            // Get rid of the original buffer if its not being used
            if (noCopy && (contentsDeallocator != kCFAllocatorNull)) {
                CFAllocatorDeallocate(contentsDeallocator, (void *)bytes);
            }
            contentsDeallocator = alloc;	// At this point we are using the string's allocator, as the original buffer is gone...

            // See if we can reuse any storage the decode func might have allocated
            // We do this only for Unicode, as otherwise we would not have NULL and Length bytes

            if (vBuf.shouldFreeChars && (alloc == vBuf.allocator) && encoding == kCFStringEncodingUnicode) {
                vBuf.shouldFreeChars = false;	// Transferring ownership to the CFString
                bytes = CFAllocatorReallocate(vBuf.allocator, (void *)vBuf.chars.unicode, numBytes, 0);	// Tighten up the storage
                noCopy = true;
#if INSTRUMENT_SHARED_STRINGS
		if (encoding == kCFStringEncodingASCII) recordedEncoding = "ForeignASCII-NoCopy";
		else recordedEncoding = "ForeignUnicode-NoCopy";
#endif
            } else {
#if INSTRUMENT_SHARED_STRINGS
		if (encoding == kCFStringEncodingASCII) recordedEncoding = "ForeignASCII-Copy";
		else recordedEncoding = "ForeignUnicode-Copy";
#endif
                bytes = vBuf.chars.unicode;
                noCopy = false;			// Can't do noCopy anymore
                // If vBuf.shouldFreeChars is true, the buffer will be freed as intended near the end of this func
            }

        }

	// At this point, all necessary input arguments have been changed to reflect the new state

    } else if (encoding == kCFStringEncodingUnicode && tryToReduceUnicode) {	// Check to see if we can reduce Unicode to ASCII
        CFIndex cnt;
        CFIndex len = numBytes / sizeof(UniChar);
        Boolean allASCII = true;

        for (cnt = 0; cnt < len; cnt++) if (((const UniChar *)bytes)[cnt] > 127) {
            allASCII = false;
            break;
        }

        if (allASCII) {	// Yes we can!
            uint8_t *ptr, *mem;
            Boolean newHasLengthByte = __CFCanUseLengthByte(len);
            numBytes = (len + 1 + (newHasLengthByte ? 1 : 0)) * sizeof(uint8_t);	// NULL and possible length byte
            // See if we can use that temporary local buffer in vBuf...
	    if (numBytes >= __kCFVarWidthLocalBufferSize) {
		mem = ptr = (uint8_t *)CFAllocatorAllocate(alloc, numBytes, 0);
		if (__CFOASafe) __CFSetLastAllocationEventName(mem, "CFString (store)");
	    } else {
		mem = ptr = (uint8_t *)(vBuf.localBuffer);
	    }
	    if (mem) {	// If we can't allocate memory for some reason, use what we had (that is, as if we didn't have all ASCII)
		// Copy the Unicode bytes into the new ASCII buffer
		hasLengthByte = newHasLengthByte;
		hasNullByte = true;
		if (hasLengthByte) *ptr++ = (uint8_t)len;
		for (cnt = 0; cnt < len; cnt++) ptr[cnt] = (uint8_t)(((const UniChar *)bytes)[cnt]);
		ptr[len] = 0;
		if (noCopy && (contentsDeallocator != kCFAllocatorNull)) {
		    CFAllocatorDeallocate(contentsDeallocator, (void *)bytes);
		}
		// Now make everything look like we had an ASCII buffer to start with
		bytes = mem;
		encoding = kCFStringEncodingASCII;
		contentsDeallocator = alloc;	// At this point we are using the string's allocator, as the original buffer is gone...
		noCopy = (numBytes >= __kCFVarWidthLocalBufferSize);	// If we had to allocate it, make sure it's kept around
		numBytes--;		// Should not contain the NULL byte at end...
		stringSupportsEightBitCFRepresentation = true; // We're ASCII now!
		stringROMShouldIgnoreNoCopy = true; // We allocated this buffer, so we should feel free to get rid of it if we can use the string ROM
#if INSTRUMENT_SHARED_STRINGS
		recordedEncoding = "U->A";
#endif
	    }
        }

        // At this point, all necessary input arguments have been changed to reflect the new state
    }

#if USE_STRING_ROM || ENABLE_TAGGED_POINTER_STRINGS || INSTRUMENT_SHARED_STRINGS
    CFIndex lengthByte = (hasLengthByte ? 1 : 0);
    CFIndex realNumBytes = numBytes - lengthByte;
    const uint8_t *realBytes = bytes + lengthByte;
#endif


    if (!str) {
        // Now determine the necessary size
#if INSTRUMENT_SHARED_STRINGS || USE_STRING_ROM
        Boolean stringSupportsROM = stringSupportsEightBitCFRepresentation;
#endif

#if INSTRUMENT_SHARED_STRINGS
        if (stringSupportsROM) __CFRecordStringAllocationEvent(recordedEncoding, realBytes, realNumBytes);
#endif

#if USE_STRING_ROM
        CFStringRef romResult = NULL;


        if (stringSupportsROM) {
            // Disable the string ROM if necessary
            static char sDisableStringROM = -1;
            if (sDisableStringROM == -1) sDisableStringROM = !! __CFgetenv("CFStringDisableROM");

            if (sDisableStringROM == 0) romResult = __CFSearchStringROM((const char *)realBytes, realNumBytes);
        }
        /* if we get a result from our ROM, and noCopy is set, then deallocate the buffer immediately */
        if (romResult) {
            if (noCopy && (contentsDeallocator != kCFAllocatorNull)) {
                CFAllocatorDeallocate(contentsDeallocator, (void *)bytes);
            }
            
            /* these don't get used again, but clear them for consistency */
            noCopy = false;
            bytes = NULL;
            
            /* set our result to the ROM result which is not really mutable, of course, but that's OK because we don't try to modify it. */
            str = (CFMutableStringRef)romResult;
            
#if INSTRUMENT_TAGGED_POINTER_STRINGS
            _CFTaggedPointerStringStats.stringROMCount++;
#endif
        }

        if (! romResult) {
#else
        if (1) {
#endif
            
#if INSTRUMENT_SHARED_STRINGS
            if (stringSupportsROM) __CFRecordStringAllocationEvent(recordedEncoding, realBytes, realNumBytes);
#endif
#if INSTRUMENT_TAGGED_POINTER_STRINGS
            _CFTaggedPointerStringStats.otherStringCount++;
#endif
            
            // Now determine the necessary size

            if (noCopy) {

                size = sizeof(void *);				// Pointer to the buffer
                if ((0) || (contentsDeallocator != alloc && contentsDeallocator != kCFAllocatorNull)) {
                    size += sizeof(void *);	// The contentsDeallocator
                }
                if (!hasLengthByte) size += sizeof(CFIndex);	// Explicit length
                useLengthByte = hasLengthByte;
                useNullByte = hasNullByte;

            } else {	// Inline data; reserve space for it

                useInlineData = true;
                size = numBytes;

                if (hasLengthByte || (encoding != kCFStringEncodingUnicode && __CFCanUseLengthByte(numBytes))) {
                    useLengthByte = true;
                    if (!hasLengthByte) size += 1;
                } else {
                    size += sizeof(CFIndex);	// Explicit length
                }	    
                if (hasNullByte || encoding != kCFStringEncodingUnicode) {
                    useNullByte = true;
                    size += 1;
                }
            }

#ifdef STRING_SIZE_STATS
            // Dump alloced CFString size info every so often
            static int cnt = 0;
            static unsigned sizes[256] = {0};
            int allocedSize = size + sizeof(CFRuntimeBase);
            if (allocedSize < 255) sizes[allocedSize]++; else sizes[255]++;
            if ((++cnt % 1000) == 0) {
                printf ("\nTotal: %d\n", cnt);
                int i; for (i = 0; i < 256; i++) printf("%03d: %5d%s", i, sizes[i], ((i % 8) == 7) ? "\n" : " ");
            }
#endif
         
            // Finally, allocate!

            str = (CFMutableStringRef)_CFRuntimeCreateInstance(alloc, __kCFStringTypeID, size, NULL);
            if (str) {
                if (__CFOASafe) __CFSetLastAllocationEventName(str, "CFString (immutable)");

                CFOptionFlags allocBits = (0) ? __kCFHasContentsDeallocator : (contentsDeallocator == alloc ? __kCFNotInlineContentsDefaultFree : (contentsDeallocator == kCFAllocatorNull ? __kCFNotInlineContentsNoFree : __kCFNotInlineContentsCustomFree));
                __CFStrSetInfoBits(str,
                                    (useInlineData ? __kCFHasInlineContents : allocBits) |
                                    ((encoding == kCFStringEncodingUnicode) ? __kCFIsUnicode : 0) |
                                    (useNullByte ? __kCFHasNullByte : 0) |
                                    (useLengthByte ? __kCFHasLengthByte : 0));

                if (!useLengthByte) {
                    CFIndex length = numBytes - (hasLengthByte ? 1 : 0);
                    if (encoding == kCFStringEncodingUnicode) length /= sizeof(UniChar);
                    __CFStrSetExplicitLength(str, length);
                }

                if (useInlineData) {
                    uint8_t *contents = (uint8_t *)__CFStrContents(str);
                    if (useLengthByte && !hasLengthByte) *contents++ = (uint8_t)numBytes;
                    memmove(contents, bytes, numBytes);
                    if (useNullByte) contents[numBytes] = 0;
                } else {
                    __CFStrSetContentPtr(str, bytes);
                    if (__CFStrHasContentsDeallocator(str)) __CFStrSetContentsDeallocator(str, contentsDeallocator); 
                }
            } else {
                if (noCopy && (contentsDeallocator != kCFAllocatorNull)) {
                    CFAllocatorDeallocate(contentsDeallocator, (void *)bytes); 
                }
            }
        }
    }
    if (vBuf.shouldFreeChars) CFAllocatorDeallocate(vBuf.allocator, (void *)bytes);

#if 0
#warning Debug code
    const uint8_t *contents = (uint8_t *)__CFStrContents(str);
    CFIndex len = __CFStrLength2(str, contents);

    if (__CFStrIsEightBit(str)) {
        contents += __CFStrSkipAnyLengthByte(str);
        if (!__CFBytesInASCII(contents, len)) {
	  printf("CFString with 8 bit backing store not ASCII: %p, \"%.*s\"\n", str, (int)len, contents);
        }
    }
#endif

    return str;
}

/* !!! __CFStringCreateImmutableFunnel2() is kept around for compatibility; it should be deprecated
*/
CFStringRef __CFStringCreateImmutableFunnel2(
                                             CFAllocatorRef alloc, const void *bytes, CFIndex numBytes, CFStringEncoding encoding,
                                             Boolean possiblyExternalFormat, Boolean tryToReduceUnicode, Boolean hasLengthByte, Boolean hasNullByte, Boolean noCopy,
                                             CFAllocatorRef contentsDeallocator) {
    return __CFStringCreateImmutableFunnel3(alloc, bytes, numBytes, encoding, possiblyExternalFormat, tryToReduceUnicode, hasLengthByte, hasNullByte, noCopy, contentsDeallocator, 0);
}



CFStringRef  CFStringCreateWithPascalString(CFAllocatorRef alloc, ConstStringPtr pStr, CFStringEncoding encoding) {
    CFIndex len = (CFIndex)(*(uint8_t *)pStr);
    return __CFStringCreateImmutableFunnel3(alloc, pStr, len+1, encoding, false, false, true, false, false, ALLOCATORSFREEFUNC, 0);
}


CFStringRef  CFStringCreateWithCString(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding) {
    CFIndex len = strlen(cStr);
    return __CFStringCreateImmutableFunnel3(alloc, cStr, len, encoding, false, false, false, true, false, ALLOCATORSFREEFUNC, 0);
}

CFStringRef  CFStringCreateWithPascalStringNoCopy(CFAllocatorRef alloc, ConstStringPtr pStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator) {
    CFIndex len = (CFIndex)(*(uint8_t *)pStr);
    return __CFStringCreateImmutableFunnel3(alloc, pStr, len+1, encoding, false, false, true, false, true, contentsDeallocator, 0);
}


CFStringRef  CFStringCreateWithCStringNoCopy(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator) {
    CFIndex len = strlen(cStr);
    return __CFStringCreateImmutableFunnel3(alloc, cStr, len, encoding, false, false, false, true, true, contentsDeallocator, 0);
}


CFStringRef  CFStringCreateWithCharacters(CFAllocatorRef alloc, const UniChar *chars, CFIndex numChars) {
    return __CFStringCreateImmutableFunnel3(alloc, chars, numChars * sizeof(UniChar), kCFStringEncodingUnicode, false, true, false, false, false, ALLOCATORSFREEFUNC, 0);
}


CFStringRef  CFStringCreateWithCharactersNoCopy(CFAllocatorRef alloc, const UniChar *chars, CFIndex numChars, CFAllocatorRef contentsDeallocator) {
    return __CFStringCreateImmutableFunnel3(alloc, chars, numChars * sizeof(UniChar), kCFStringEncodingUnicode, false, false, false, false, true, contentsDeallocator, 0);
}


CFStringRef  CFStringCreateWithBytes(CFAllocatorRef alloc, const uint8_t *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean externalFormat) {
    return __CFStringCreateImmutableFunnel3(alloc, bytes, numBytes, encoding, externalFormat, true, false, false, false, ALLOCATORSFREEFUNC, 0);
}

CFStringRef  _CFStringCreateWithBytesNoCopy(CFAllocatorRef alloc, const uint8_t *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean externalFormat, CFAllocatorRef contentsDeallocator) {
    return __CFStringCreateImmutableFunnel3(alloc, bytes, numBytes, encoding, externalFormat, true, false, false, true, contentsDeallocator, 0);
}

CFStringRef  CFStringCreateWithBytesNoCopy(CFAllocatorRef alloc, const uint8_t *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean externalFormat, CFAllocatorRef contentsDeallocator) {
    return __CFStringCreateImmutableFunnel3(alloc, bytes, numBytes, encoding, externalFormat, true, false, false, true, contentsDeallocator, 0);
}

CFStringRef  CFStringCreateWithFormatAndArguments(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef format, va_list arguments) {
    return _CFStringCreateWithFormatAndArgumentsAux2(alloc, NULL, NULL, formatOptions, format, arguments);
}

CFStringRef  _CFStringCreateWithFormatAndArgumentsAux2(CFAllocatorRef alloc, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool , bool *), CFDictionaryRef formatOptions, CFStringRef format, va_list arguments) {
    CFStringRef str;
    CFMutableStringRef outputString = CFStringCreateMutable(kCFAllocatorSystemDefault, 0); //should use alloc if no copy/release
    __CFStrSetDesiredCapacity(outputString, 120);	// Given this will be tightened later, choosing a larger working string is fine
    __CFStringAppendFormatCore(outputString, copyDescFunc, contextDescFunc, formatOptions, NULL, format, 0, NULL, 0, arguments);
    // ??? copy/release should not be necessary here -- just make immutable, compress if possible
    // (However, this does make the string inline, and cause the supplied allocator to be used...)
    str = (CFStringRef)CFStringCreateCopy(alloc, outputString);
    CFRelease(outputString);
    return str;
}
    
CFStringRef  _CFStringCreateWithFormatAndArgumentsAux(CFAllocatorRef alloc, CFStringRef (*copyDescFunc)(void *, const void *), CFDictionaryRef formatOptions, CFStringRef format, va_list arguments) {
    return _CFStringCreateWithFormatAndArgumentsAux2(alloc, copyDescFunc, NULL, formatOptions, format, arguments);
}

CFStringRef  CFStringCreateWithFormat(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef format, ...) {
    CFStringRef result;
    va_list argList;

    va_start(argList, format);
    result = CFStringCreateWithFormatAndArguments(alloc, formatOptions, format, argList);
    va_end(argList);

    return result;
}

CFStringRef CFStringCreateWithSubstring(CFAllocatorRef alloc, CFStringRef str, CFRange range) {
//      CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, CFStringRef , (NSString *)str, _createSubstringWithRange:NSMakeRange(range.location, range.length));

    __CFAssertIsString(str);
    __CFAssertRangeIsInStringBounds(str, range.location, range.length);

    if ((range.location == 0) && (range.length == __CFStrLength(str))) {	/* The substring is the whole string... */
	return (CFStringRef)CFStringCreateCopy(alloc, str);
    } else if (__CFStrIsEightBit(str)) {
	const uint8_t *contents = (const uint8_t *)__CFStrContents(str);
        return __CFStringCreateImmutableFunnel3(alloc, contents + range.location + __CFStrSkipAnyLengthByte(str), range.length, __CFStringGetEightBitStringEncoding(), false, false, false, false, false, ALLOCATORSFREEFUNC, 0);
    } else {
	const UniChar *contents = (UniChar *)__CFStrContents(str);
        return __CFStringCreateImmutableFunnel3(alloc, contents + range.location, range.length * sizeof(UniChar), kCFStringEncodingUnicode, false, true, false, false, false, ALLOCATORSFREEFUNC, 0);
    }
}

CFStringRef CFStringCreateCopy(CFAllocatorRef alloc, CFStringRef str) {
//  CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, CFStringRef, (NSString *)str, copy);

    __CFAssertIsString(str);
    if (!__CFStrIsMutable((CFStringRef)str) && 								// If the string is not mutable
        ((alloc ? alloc : __CFGetDefaultAllocator()) == __CFGetAllocator(str)) &&		//  and it has the same allocator as the one we're using
        (__CFStrIsInline((CFStringRef)str) || __CFStrFreeContentsWhenDone((CFStringRef)str) || __CFStrIsConstant((CFStringRef)str))) {	//  and the characters are inline, or are owned by the string, or the string is constant
        if (!(kCFUseCollectableAllocator && (0))) CFRetain(str);			// Then just retain instead of making a true copy
	return str;
    }
    if (__CFStrIsEightBit((CFStringRef)str)) {
        const uint8_t *contents = (const uint8_t *)__CFStrContents((CFStringRef)str);
        return __CFStringCreateImmutableFunnel3(alloc, contents + __CFStrSkipAnyLengthByte((CFStringRef)str), __CFStrLength2((CFStringRef)str, contents), __CFStringGetEightBitStringEncoding(), false, false, false, false, false, ALLOCATORSFREEFUNC, 0);
    } else {
        const UniChar *contents = (const UniChar *)__CFStrContents((CFStringRef)str);
        return __CFStringCreateImmutableFunnel3(alloc, contents, __CFStrLength2((CFStringRef)str, contents) * sizeof(UniChar), kCFStringEncodingUnicode, false, true, false, false, false, ALLOCATORSFREEFUNC, 0);
    }
}



/*** Constant string stuff... ***/

/* Table which holds constant strings created with CFSTR, when -fconstant-cfstrings option is not used. These dynamically created constant strings are stored in constantStringTable. The keys are the 8-bit constant C-strings from the compiler; the values are the CFStrings created for them. _CFSTRLock protects this table.
*/
static CFMutableDictionaryRef constantStringTable = NULL;
static CFLock_t _CFSTRLock = CFLockInit;

static CFStringRef __cStrCopyDescription(const void *ptr) {
    return CFStringCreateWithCStringNoCopy(kCFAllocatorSystemDefault, (const char *)ptr, __CFStringGetEightBitStringEncoding(), kCFAllocatorNull);
}

static Boolean __cStrEqual(const void *ptr1, const void *ptr2) {
    return (strcmp((const char *)ptr1, (const char *)ptr2) == 0);
}

static CFHashCode __cStrHash(const void *ptr) {
    // It doesn't quite matter if we convert to Unicode correctly, as long as we do it consistently    
    const char *cStr = (const char *)ptr;
    CFIndex len = strlen(cStr);
    CFHashCode result = 0;
    if (len <= 4) {	// All chars
        unsigned cnt = len;
        while (cnt--) result += (result << 8) + *cStr++;
    } else {		// First and last 2 chars
        result += (result << 8) + cStr[0];
        result += (result << 8) + cStr[1];
        result += (result << 8) + cStr[len-2];
        result += (result << 8) + cStr[len-1];
    }
    result += (result << (len & 31));
    return result;    
}


CFStringRef __CFStringMakeConstantString(const char *cStr) {
    CFStringRef result;
#if defined(DEBUG)
    // StringTest checks that we share kCFEmptyString, which is defeated by constantStringAllocatorForDebugging 
    if ('\0' == *cStr) return kCFEmptyString;
#endif
    if (constantStringTable == NULL) {
        CFDictionaryKeyCallBacks constantStringCallBacks = {0, NULL, NULL, __cStrCopyDescription, __cStrEqual, __cStrHash};
        CFDictionaryValueCallBacks constantStringValueCallBacks = kCFTypeDictionaryValueCallBacks;
        constantStringValueCallBacks.equal = NULL;      // So that we only find strings that are ==
	CFMutableDictionaryRef table = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &constantStringCallBacks, &constantStringValueCallBacks);
	_CFDictionarySetCapacity(table, 2500);          // avoid lots of rehashing
	__CFLock(&_CFSTRLock);
	if (constantStringTable == NULL) constantStringTable = table;
	__CFUnlock(&_CFSTRLock);
	if (constantStringTable != table) CFRelease(table);
    }

    __CFLock(&_CFSTRLock);
    if ((result = (CFStringRef)CFDictionaryGetValue(constantStringTable, cStr))) {
	__CFUnlock(&_CFSTRLock);
    } else {
	__CFUnlock(&_CFSTRLock);

        {
            char *key = NULL;
            Boolean isASCII = true;
            // Given this code path is rarer these days, OK to do this extra work to verify the strings
            const char *tmp = cStr;
            while (*tmp) {
                if (*(tmp++) & 0x80) {
                    isASCII = false;
                    break;
                }
            }
            if (!isASCII) {
                CFMutableStringRef ms = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
                tmp = cStr;
                while (*tmp) {
                    CFStringAppendFormat(ms, NULL, (*tmp & 0x80) ? CFSTR("\\%3o") : CFSTR("%1c"), *tmp);
                    tmp++;
                }
                CFLog(kCFLogLevelWarning, CFSTR("WARNING: CFSTR(\"%@\") has non-7 bit chars, interpreting using MacOS Roman encoding for now, but this will change. Please eliminate usages of non-7 bit chars (including escaped characters above \\177 octal) in CFSTR()."), ms);
                CFRelease(ms);
            }
            // Treat non-7 bit chars in CFSTR() as MacOSRoman, for compatibility
            result = CFStringCreateWithCString(kCFAllocatorSystemDefault, cStr, kCFStringEncodingMacRoman);
            if (result == NULL) {
                CFLog(__kCFLogAssertion, CFSTR("Can't interpret CFSTR() as MacOS Roman, crashing"));
                HALT;
            }
            Boolean isTaggedPointerString = CF_IS_OBJC(__kCFStringTypeID, result);
            
            if (!isTaggedPointerString) {
                if (__CFOASafe) __CFSetLastAllocationEventName((void *)result, "CFString (CFSTR)");
                if (__CFStrIsEightBit(result)) key = (char *)__CFStrContents(result) + __CFStrSkipAnyLengthByte(result);
            }
            if (!key) { // Either the string is not 8-bit or it's a tagged pointer string
                CFIndex keySize = strlen(cStr) + 1;
                key = (char *)CFAllocatorAllocate(kCFAllocatorSystemDefault, keySize, 0);
                if (__CFOASafe) __CFSetLastAllocationEventName((void *)key, "CFString (CFSTR key)");
                strlcpy(key, cStr, keySize);	// !!! We will leak this, if the string is removed from the table (or table is freed)
            }

            {
                CFStringRef resultToBeReleased = result;
                CFIndex count;
                __CFLock(&_CFSTRLock);
                count = CFDictionaryGetCount(constantStringTable);
                CFDictionaryAddValue(constantStringTable, key, result);
                if (CFDictionaryGetCount(constantStringTable) == count) { // add did nothing, someone already put it there
                    result = (CFStringRef)CFDictionaryGetValue(constantStringTable, key);
                } else if (!isTaggedPointerString) {
#if __LP64__
                    ((struct __CFString *)result)->base._rc = 0;
#else
                    ((struct __CFString *)result)->base._cfinfo[CF_RC_BITS] = 0;
#endif
                }
                __CFUnlock(&_CFSTRLock);
                // This either eliminates the extra retain on the freshly created string, or frees it, if it was actually not inserted into the table
                CFRelease(resultToBeReleased);
            }
	}
    }
    return result;
}

#if defined(DEBUG)
static Boolean __CFStrIsConstantString(CFStringRef str) {
    Boolean found = false;
    if (constantStringTable) {
	__CFLock(&_CFSTRLock);
	found = CFDictionaryContainsValue(constantStringTable, str);
	__CFUnlock(&_CFSTRLock);
    }
    return found;
}
#endif


#if DEPLOYMENT_TARGET_WINDOWS
void __CFStringCleanup (void) {
    /* in case library is unloaded, release store for the constant string table */
    if (constantStringTable != NULL) {
#if defined(DEBUG)
    	__CFConstantStringTableBeingFreed = true;
        CFRelease(constantStringTable);
        __CFConstantStringTableBeingFreed = false;
#else 
        CFRelease(constantStringTable);
#endif
        constantStringTable = NULL;
    }
}
#endif


// Can pass in NSString as replacement string
// Call with numRanges > 0, and incrementing ranges

static void __CFStringReplaceMultiple(CFMutableStringRef str, CFRange *ranges, CFIndex numRanges, CFStringRef replacement) {
    int cnt;
    CFStringRef copy = NULL;
    if (replacement == str) copy = replacement = CFStringCreateCopy(kCFAllocatorSystemDefault, replacement);   // Very special and hopefully rare case
    CFIndex replacementLength = CFStringGetLength(replacement);

    __CFStringChangeSizeMultiple(str, ranges, numRanges, replacementLength, (replacementLength > 0) && CFStrIsUnicode(replacement));

    if (__CFStrIsUnicode(str)) {
        UniChar *contents = (UniChar *)__CFStrContents(str);
        UniChar *firstReplacement = contents + ranges[0].location;
        // Extract the replacementString into the first location, then copy from there
        CFStringGetCharacters(replacement, CFRangeMake(0, replacementLength), firstReplacement);
        for (cnt = 1; cnt < numRanges; cnt++) {
            // The ranges are in terms of the original string; so offset by the change in length due to insertion
            contents += replacementLength - ranges[cnt - 1].length;
            memmove(contents + ranges[cnt].location, firstReplacement, replacementLength * sizeof(UniChar));
        }
    } else {
        uint8_t *contents = (uint8_t *)__CFStrContents(str);
        uint8_t *firstReplacement = contents + ranges[0].location + __CFStrSkipAnyLengthByte(str);
        // Extract the replacementString into the first location, then copy from there
        CFStringGetBytes(replacement, CFRangeMake(0, replacementLength), __CFStringGetEightBitStringEncoding(), 0, false, firstReplacement, replacementLength, NULL);
        contents += __CFStrSkipAnyLengthByte(str);	// Now contents will simply track the location to insert next string into
        for (cnt = 1; cnt < numRanges; cnt++) {
            // The ranges are in terms of the original string; so offset by the change in length due to insertion
            contents += replacementLength - ranges[cnt - 1].length;
            memmove(contents + ranges[cnt].location, firstReplacement, replacementLength);
        }
    }
    if (copy) CFRelease(copy);
}

// Can pass in NSString as replacement string

CF_INLINE void __CFStringReplace(CFMutableStringRef str, CFRange range, CFStringRef replacement) {
    CFStringRef copy = NULL;
    if (replacement == str) copy = replacement = (CFStringRef)CFStringCreateCopy(kCFAllocatorSystemDefault, replacement);   // Very special and hopefully rare case
    CFIndex replacementLength = CFStringGetLength(replacement);

    __CFStringChangeSize(str, range, replacementLength, (replacementLength > 0) && CFStrIsUnicode(replacement));

    if (__CFStrIsUnicode(str)) {
        UniChar *contents = (UniChar *)__CFStrContents(str);
        CFStringGetCharacters(replacement, CFRangeMake(0, replacementLength), contents + range.location);
    } else {
        uint8_t *contents = (uint8_t *)__CFStrContents(str);
        CFStringGetBytes(replacement, CFRangeMake(0, replacementLength), __CFStringGetEightBitStringEncoding(), 0, false, contents + range.location + __CFStrSkipAnyLengthByte(str), replacementLength, NULL);
    }

    if (copy) CFRelease(copy);
}

/* If client does not provide a minimum capacity
*/
#define DEFAULTMINCAPACITY 32

CF_INLINE CFMutableStringRef __CFStringCreateMutableFunnel(CFAllocatorRef alloc, CFIndex maxLength, UInt32 additionalInfoBits) {
    CFMutableStringRef str;
    if ((0)) additionalInfoBits |= __kCFHasContentsAllocator;
    Boolean hasExternalContentsAllocator = (additionalInfoBits & __kCFHasContentsAllocator) ? true : false;

    if (alloc == NULL) alloc = __CFGetDefaultAllocator();

    // Note that if there is an externalContentsAllocator, then we also have the storage for the string allocator...
    str = (CFMutableStringRef)_CFRuntimeCreateInstance(alloc, __kCFStringTypeID, sizeof(struct __notInlineMutable) - (hasExternalContentsAllocator ? 0 : sizeof(CFAllocatorRef)), NULL);
    if (str) {
        if (__CFOASafe) __CFSetLastAllocationEventName(str, "CFString (mutable)");

        __CFStrSetInfoBits(str, __kCFIsMutable | additionalInfoBits);
        str->variants.notInlineMutable.buffer = NULL;
        __CFStrSetExplicitLength(str, 0);
	str->variants.notInlineMutable.hasGap = str->variants.notInlineMutable.isFixedCapacity = str->variants.notInlineMutable.isExternalMutable = str->variants.notInlineMutable.capacityProvidedExternally = 0;
	if (maxLength != 0) __CFStrSetIsFixed(str);
        __CFStrSetDesiredCapacity(str, (maxLength == 0) ? DEFAULTMINCAPACITY : maxLength);
        __CFStrSetCapacity(str, 0);
        if (__CFStrHasContentsAllocator(str)) {
            // contents allocator starts out as the string's own allocator
            __CFStrSetContentsAllocator(str, alloc);
        }
    }
    return str;
}

CFMutableStringRef CFStringCreateMutableWithExternalCharactersNoCopy(CFAllocatorRef alloc, UniChar *chars, CFIndex numChars, CFIndex capacity, CFAllocatorRef externalCharactersAllocator) {
    CFOptionFlags contentsAllocationBits = externalCharactersAllocator ? ((externalCharactersAllocator == kCFAllocatorNull) ? __kCFNotInlineContentsNoFree : __kCFHasContentsAllocator) : __kCFNotInlineContentsDefaultFree;
    CFMutableStringRef string = __CFStringCreateMutableFunnel(alloc, 0, contentsAllocationBits | __kCFIsUnicode);
    if (string) {
	__CFStrSetIsExternalMutable(string);
        if (__CFStrHasContentsAllocator(string)) {
            CFAllocatorRef allocator = __CFStrContentsAllocator((CFMutableStringRef)string);
            if (!(0 || 0)) CFRelease(allocator);
            __CFStrSetContentsAllocator(string, externalCharactersAllocator);
        }
        CFStringSetExternalCharactersNoCopy(string, chars, numChars, capacity);
    }
    return string;
}
 
CFMutableStringRef CFStringCreateMutable(CFAllocatorRef alloc, CFIndex maxLength) {
    return __CFStringCreateMutableFunnel(alloc, maxLength, __kCFNotInlineContentsDefaultFree);
}

CFMutableStringRef  CFStringCreateMutableCopy(CFAllocatorRef alloc, CFIndex maxLength, CFStringRef string) {
    CFMutableStringRef newString;

    //  CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, CFMutableStringRef, (NSString *)string, mutableCopy);

    __CFAssertIsString(string);

    newString = CFStringCreateMutable(alloc, maxLength);
    __CFStringReplace(newString, CFRangeMake(0, 0), string);

    return newString;
}


CF_PRIVATE void _CFStrSetDesiredCapacity(CFMutableStringRef str, CFIndex len) {
    __CFAssertIsStringAndMutable(str);
    __CFStrSetDesiredCapacity(str, len);
}


/* This one is for CF
*/
CFIndex CFStringGetLength(CFStringRef str) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, CFIndex, (NSString *)str, length);

    __CFAssertIsString(str);
    return __CFStrLength(str);
}

/* This one is for NSCFString; it does not ObjC dispatch or assertion check
*/
CFIndex _CFStringGetLength2(CFStringRef str) {
    return __CFStrLength(str);
}


/* Guts of CFStringGetCharacterAtIndex(); called from the two functions below. Don't call it from elsewhere.
*/
CF_INLINE UniChar __CFStringGetCharacterAtIndexGuts(CFStringRef str, CFIndex idx, const uint8_t *contents) {
    if (__CFStrIsEightBit(str)) {
        contents += __CFStrSkipAnyLengthByte(str);
#if defined(DEBUG)
        if (!__CFCharToUniCharFunc && (contents[idx] >= 128)) {
            // Can't do log here, as it might be too early
            fprintf(stderr, "Warning: CFStringGetCharacterAtIndex() attempted on CFString containing high bytes before properly initialized to do so\n");
        }
#endif
	return __CFCharToUniCharTable[contents[idx]];
    }

    return ((UniChar *)contents)[idx];
}

/* This one is for the CF API
*/
UniChar CFStringGetCharacterAtIndex(CFStringRef str, CFIndex idx) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, UniChar, (NSString *)str, characterAtIndex:(NSUInteger)idx);

    __CFAssertIsString(str);
    __CFAssertIndexIsInStringBounds(str, idx);
    return __CFStringGetCharacterAtIndexGuts(str, idx, (const uint8_t *)__CFStrContents(str));
}

/* This one is for NSCFString usage; it doesn't do ObjC dispatch; but it does do range check
*/
int _CFStringCheckAndGetCharacterAtIndex(CFStringRef str, CFIndex idx, UniChar *ch) {
    const uint8_t *contents = (const uint8_t *)__CFStrContents(str);
    if (idx >= __CFStrLength2(str, contents) && __CFStringNoteErrors()) return _CFStringErrBounds;
    *ch = __CFStringGetCharacterAtIndexGuts(str, idx, contents);
    return _CFStringErrNone;
}


/* Guts of CFStringGetCharacters(); called from the two functions below. Don't call it from elsewhere.
*/
CF_INLINE void __CFStringGetCharactersGuts(CFStringRef str, CFRange range, UniChar *buffer, const uint8_t *contents) {
    if (__CFStrIsEightBit(str)) {
        __CFStrConvertBytesToUnicode(((uint8_t *)contents) + (range.location + __CFStrSkipAnyLengthByte(str)), buffer, range.length);
    } else {
        const UniChar *uContents = ((UniChar *)contents) + range.location;
        memmove(buffer, uContents, range.length * sizeof(UniChar));
    }
}

/* This one is for the CF API
*/
void CFStringGetCharacters(CFStringRef str, CFRange range, UniChar *buffer) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSString *)str, getCharacters:(unichar *)buffer range:NSMakeRange(range.location, range.length));

    __CFAssertIsString(str);
    __CFAssertRangeIsInStringBounds(str, range.location, range.length);
    __CFStringGetCharactersGuts(str, range, buffer, (const uint8_t *)__CFStrContents(str));
}

/* This one is for NSCFString usage; it doesn't do ObjC dispatch; but it does do range check
*/
int _CFStringCheckAndGetCharacters(CFStringRef str, CFRange range, UniChar *buffer) {
     const uint8_t *contents = (const uint8_t *)__CFStrContents(str);
     if (range.location + range.length > __CFStrLength2(str, contents) && __CFStringNoteErrors()) return _CFStringErrBounds;
     __CFStringGetCharactersGuts(str, range, buffer, contents);
     return _CFStringErrNone;
}


CFIndex CFStringGetBytes(CFStringRef str, CFRange range, CFStringEncoding encoding, uint8_t lossByte, Boolean isExternalRepresentation, uint8_t *buffer, CFIndex maxBufLen, CFIndex *usedBufLen) {

    __CFAssertIsNotNegative(maxBufLen);
    
    {
        __CFAssertIsString(str);
        __CFAssertRangeIsInStringBounds(str, range.location, range.length);

        if (__CFStrIsEightBit(str) && ((__CFStringGetEightBitStringEncoding() == encoding) || (__CFStringGetEightBitStringEncoding() == kCFStringEncodingASCII && __CFStringEncodingIsSupersetOfASCII(encoding)))) {	// Requested encoding is equal to the encoding in string
            const unsigned char *contents = (const unsigned char *)__CFStrContents(str);
            CFIndex cLength = range.length;

            if (buffer) {
                if (cLength > maxBufLen) cLength = maxBufLen;
                memmove(buffer, contents + __CFStrSkipAnyLengthByte(str) + range.location, cLength);
            }
            if (usedBufLen) *usedBufLen = cLength;

            return cLength;
        }
    }

    return __CFStringEncodeByteStream(str, range.location, range.length, isExternalRepresentation, encoding, lossByte, buffer, maxBufLen, usedBufLen);
}


ConstStringPtr CFStringGetPascalStringPtr (CFStringRef str, CFStringEncoding encoding) {

    if (!CF_IS_OBJC(__kCFStringTypeID, str)) {	/* ??? Hope the compiler optimizes this away if OBJC_MAPPINGS is not on */
        __CFAssertIsString(str);
        if (__CFStrHasLengthByte(str) && __CFStrIsEightBit(str) && ((__CFStringGetEightBitStringEncoding() == encoding) || (__CFStringGetEightBitStringEncoding() == kCFStringEncodingASCII && __CFStringEncodingIsSupersetOfASCII(encoding)))) {	// Requested encoding is equal to the encoding in string || the contents is in ASCII
	    const uint8_t *contents = (const uint8_t *)__CFStrContents(str);
	    if (__CFStrHasExplicitLength(str) && (__CFStrLength2(str, contents) != (SInt32)(*contents))) return NULL;	// Invalid length byte
	    return (ConstStringPtr)contents;
	}
	// ??? Also check for encoding = SystemEncoding and perhaps bytes are all ASCII?
    }
    return NULL;
}


const char * CFStringGetCStringPtr(CFStringRef str, CFStringEncoding encoding) {

    if (encoding != __CFStringGetEightBitStringEncoding() && (kCFStringEncodingASCII != __CFStringGetEightBitStringEncoding() || !__CFStringEncodingIsSupersetOfASCII(encoding))) return NULL;
    // ??? Also check for encoding = SystemEncoding and perhaps bytes are all ASCII?

    if (str == NULL) return NULL;   // Should really just crash, but for compatibility... see <rdar://problem/12340248>
    
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, const char *, (NSString *)str, _fastCStringContents:true);

    __CFAssertIsString(str);

    if (__CFStrHasNullByte(str)) {
        // Note: this is called a lot, 27000 times to open a small xcode project with one file open.
        // Of these uses about 1500 are for cStrings/utf8strings.
#if 0
        // Only sometimes when the stars are aligned will this call return a gc pointer
        // under GC we can only really return a pointer to the start of a GC buffer for cString use
        // (Is there a simpler way to ask if contents isGC?)
        CFAllocatorRef alloc = (__CFStrHasContentsAllocator(str)) ? __CFStrContentsAllocator(str) : __CFGetAllocator(str);
        if (CF_IS_COLLECTABLE_ALLOCATOR(alloc)) {
            if (__CFStrSkipAnyLengthByte(str) != 0 || !__CFStrIsMutable(str)) {
                static int counter = 0;
                printf("CFString %dth unsafe safe string %s\n", ++counter, __CFStrContents(str) + __CFStrSkipAnyLengthByte(str));
                return NULL;
            }
        }
#endif
	return (const char *)__CFStrContents(str) + __CFStrSkipAnyLengthByte(str);
    } else {
	return NULL;
    }
}


const UniChar *CFStringGetCharactersPtr(CFStringRef str) {

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, const UniChar *, (NSString *)str, _fastCharacterContents);
    
    __CFAssertIsString(str);
    if (__CFStrIsUnicode(str)) return (const UniChar *)__CFStrContents(str);
    return NULL;
}


Boolean CFStringGetPascalString(CFStringRef str, Str255 buffer, CFIndex bufferSize, CFStringEncoding encoding) {
    CFIndex length;
    CFIndex usedLen;

    __CFAssertIsNotNegative(bufferSize);
    if (bufferSize < 1) return false;

    if (CF_IS_OBJC(__kCFStringTypeID, str)) {	/* ??? Hope the compiler optimizes this away if OBJC_MAPPINGS is not on */
	length = CFStringGetLength(str);
        if (!__CFCanUseLengthByte(length)) return false; // Can't fit into pstring
    } else {
	const uint8_t *contents;

        __CFAssertIsString(str);

        contents = (const uint8_t *)__CFStrContents(str);
        length = __CFStrLength2(str, contents);

        if (!__CFCanUseLengthByte(length)) return false; // Can't fit into pstring

        if (__CFStrIsEightBit(str) && ((__CFStringGetEightBitStringEncoding() == encoding) || (__CFStringGetEightBitStringEncoding() == kCFStringEncodingASCII && __CFStringEncodingIsSupersetOfASCII(encoding)))) {	// Requested encoding is equal to the encoding in string
	    if (length >= bufferSize) return false;
            memmove((void*)(1 + (const char*)buffer), (__CFStrSkipAnyLengthByte(str) + contents), length);
            *buffer = (unsigned char)length;
	    return true;
	}
    }

	if (__CFStringEncodeByteStream(str, 0, length, false, encoding, false, (UInt8 *)(1 + (uint8_t *)buffer), bufferSize - 1, &usedLen) != length) {

#if defined(DEBUG)
	if (bufferSize > 0) {
	    strlcpy((char *)buffer + 1, CONVERSIONFAILURESTR, bufferSize - 1);
	    buffer[0] = (unsigned char)((CFIndex)sizeof(CONVERSIONFAILURESTR) < (bufferSize - 1) ? (CFIndex)sizeof(CONVERSIONFAILURESTR) : (bufferSize - 1));
	}
#else
	if (bufferSize > 0) buffer[0] = 0;
#endif
	return false;
    }
    *buffer = (unsigned char)usedLen;
    return true;
}
                                   
Boolean CFStringGetCString(CFStringRef str, char *buffer, CFIndex bufferSize, CFStringEncoding encoding) {
    const uint8_t *contents;
    CFIndex len;

    __CFAssertIsNotNegative(bufferSize);
    if (bufferSize < 1) return false;

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, Boolean, (NSString *)str, _getCString:buffer maxLength:(NSUInteger)bufferSize - 1 encoding:encoding);

    __CFAssertIsString(str);

    contents = (const uint8_t *)__CFStrContents(str);
    len = __CFStrLength2(str, contents);

    if (__CFStrIsEightBit(str) && ((__CFStringGetEightBitStringEncoding() == encoding) || (__CFStringGetEightBitStringEncoding() == kCFStringEncodingASCII && __CFStringEncodingIsSupersetOfASCII(encoding)))) {	// Requested encoding is equal to the encoding in string
        if (len >= bufferSize) return false;
	memmove(buffer, contents + __CFStrSkipAnyLengthByte(str), len);
	buffer[len] = 0;
        return true;
    } else {
        CFIndex usedLen;

        if (__CFStringEncodeByteStream(str, 0, len, false, encoding, false, (unsigned char*) buffer, bufferSize - 1, &usedLen) == len) {
            buffer[usedLen] = '\0';
            return true;
        } else {
#if defined(DEBUG)
            strlcpy(buffer, CONVERSIONFAILURESTR, bufferSize);
#else
	    if (bufferSize > 0) buffer[0] = 0;
#endif
            return false;
        }
    }
}

extern Boolean __CFLocaleGetNullLocale(struct __CFLocale *locale);
extern void __CFLocaleSetNullLocale(struct __CFLocale *locale);

static const char *_CFStrGetLanguageIdentifierForLocale(CFLocaleRef locale, bool collatorOnly) {
    CFStringRef localeID;
    const char *langID = NULL;
    static const void *lastLocale = NULL;
    static const char *lastLangID = NULL;
    static CFLock_t lock = CFLockInit;

    if (__CFLocaleGetNullLocale((struct __CFLocale *)locale)) return NULL;

    __CFLock(&lock);
    if ((NULL != lastLocale) && (lastLocale == locale)) {
        __CFUnlock(&lock);
        return lastLangID;
    }
    __CFUnlock(&lock);

    localeID = (CFStringRef)CFLocaleGetValue(locale, __kCFLocaleCollatorID);
    CFIndex length = CFStringGetLength(localeID);

    if (!collatorOnly) {
        if ((length < 2) || ((4 == length) && CFEqual(localeID, CFSTR("root")))) {
            localeID = (CFStringRef)CFLocaleGetIdentifier(locale);
            length = CFStringGetLength(localeID);
        }
    }

    if (length > 1) {
        uint8_t buffer[2];
        const uint8_t *contents = (const uint8_t *)CFStringGetCStringPtr(localeID, kCFStringEncodingUTF8);
        if (!contents) {
            if (2 == CFStringGetBytes(localeID, CFRangeMake(0,2), kCFStringEncodingUTF8, 0, false, buffer, sizeof(buffer), NULL)) contents = buffer;
        }
        if (contents) {
            const char *string = (const char *)contents;
            if (!strncmp(string, "az", 2)) { // Azerbaijani
                langID = "az";
            } else if (!strncmp(string, "lt", 2)) { // Lithuanian
                langID = "lt";
            } else if (!strncmp(string, "tr", 2)) { // Turkish
                langID = "tr";
            } else if (!strncmp(string, "nl", 2)) { // Dutch
                langID = "nl";
            } else if (!strncmp(string, "el", 2)) { // Greek
                langID = "el";
            }
        }
    }

    if (langID == NULL) __CFLocaleSetNullLocale((struct __CFLocale *)locale);

    __CFLock(&lock);
    lastLocale = locale;
    lastLangID = langID;
    __CFUnlock(&lock);

    return langID;
}

CF_INLINE bool _CFCanUseLocale(CFLocaleRef locale) {
    if (locale) {
        return true;
    }
    return false;
}

#define MAX_CASE_MAPPING_BUF (8)
#define ZERO_WIDTH_JOINER (0x200D)
#define COMBINING_GRAPHEME_JOINER (0x034F)
// Hangul ranges
#define HANGUL_CHOSEONG_START (0x1100)
#define HANGUL_CHOSEONG_END (0x115F)
#define HANGUL_JUNGSEONG_START (0x1160)
#define HANGUL_JUNGSEONG_END (0x11A2)
#define HANGUL_JONGSEONG_START (0x11A8)
#define HANGUL_JONGSEONG_END (0x11F9)

#define HANGUL_SYLLABLE_START (0xAC00)
#define HANGUL_SYLLABLE_END (0xD7AF)


// Returns the length of characters filled into outCharacters. If no change, returns 0. maxBufLen shoule be at least 8
static CFIndex __CFStringFoldCharacterClusterAtIndex(UTF32Char character, CFStringInlineBuffer *buffer, CFIndex index, CFOptionFlags flags, const uint8_t *langCode, UTF32Char *outCharacters, CFIndex maxBufferLength, CFIndex *consumedLength) {
    CFIndex filledLength = 0, currentIndex = index;

    if (0 != character) {
        UTF16Char lowSurrogate;
        CFIndex planeNo = (character >> 16);
        bool isTurkikCapitalI = false;
        static const uint8_t *decompBMP = NULL;
        static const uint8_t *graphemeBMP = NULL;

        if (NULL == decompBMP) {
            decompBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, 0);
            graphemeBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, 0);
        }

        currentIndex += ((character > 0xFFFF) ? 2 : 1);
        
        if ((character < 0x0080) && ((NULL == langCode) || (character != 'I'))) { // ASCII
            if ((flags & kCFCompareCaseInsensitive) && (character >= 'A') && (character <= 'Z')) {
                character += ('a' - 'A');
                *outCharacters = character;
                filledLength = 1;
            }
        } else {
            // do width-insensitive mapping
            if ((flags & kCFCompareWidthInsensitive) && (character >= 0xFF00) && (character <= 0xFFEF)) {
                (void)CFUniCharCompatibilityDecompose(&character, 1, 1);
                *outCharacters = character;
                filledLength = 1;
            }

            // map surrogates
            if ((0 == planeNo) && CFUniCharIsSurrogateHighCharacter(character) && CFUniCharIsSurrogateLowCharacter((lowSurrogate = CFStringGetCharacterFromInlineBuffer(buffer, currentIndex)))) {
                character = CFUniCharGetLongCharacterForSurrogatePair(character, lowSurrogate);
                ++currentIndex;
                planeNo = (character >> 16);
            }

            // decompose
            if (flags & (kCFCompareDiacriticInsensitive|kCFCompareNonliteral)) {
                if (CFUniCharIsMemberOfBitmap(character, ((0 == planeNo) ? decompBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, planeNo)))) {
                    UTF32Char original = character;

                    filledLength = CFUniCharDecomposeCharacter(character, outCharacters, maxBufferLength);
                    character = *outCharacters;

                    if ((flags & kCFCompareDiacriticInsensitive) && (character < 0x0510)) {
                        filledLength = 1; // reset if Roman, Greek, Cyrillic
                    } else if (0 == (flags & kCFCompareNonliteral)) {
                        character = original;
                        filledLength = 0;
                    }
                }
            }

            // fold case
            if (flags & kCFCompareCaseInsensitive) {
                const uint8_t *nonBaseBitmap;
                bool filterNonBase = (((flags & kCFCompareDiacriticInsensitive) && (character < 0x0510)) ? true : false);
                static const uint8_t *lowerBMP = NULL;
                static const uint8_t *caseFoldBMP = NULL;
                
                if (NULL == lowerBMP) {
                    lowerBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharHasNonSelfLowercaseCharacterSet, 0);
                    caseFoldBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharHasNonSelfCaseFoldingCharacterSet, 0);
                }

                if ((NULL != langCode) && ('I' == character) && ((0 == strcmp((const char *)langCode, "tr")) || (0 == strcmp((const char *)langCode, "az")))) { // do Turkik special-casing
                    if (filledLength > 1) {
                        if (0x0307 == outCharacters[1]) {
                            if (--filledLength > 1) memmove((outCharacters + 1), (outCharacters + 2), sizeof(UTF32Char) * (filledLength - 1));
                            character = *outCharacters = 'i';
                            isTurkikCapitalI = true;
                        }
                    } else if (0x0307 == CFStringGetCharacterFromInlineBuffer(buffer, currentIndex)) {
                        character = *outCharacters = 'i';
                        filledLength = 1;
                        ++currentIndex;
                        isTurkikCapitalI = true;
                    }
                }
                if (!isTurkikCapitalI && (CFUniCharIsMemberOfBitmap(character, ((0 == planeNo) ? lowerBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharHasNonSelfLowercaseCharacterSet, planeNo))) || CFUniCharIsMemberOfBitmap(character, ((0 == planeNo) ? caseFoldBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharHasNonSelfCaseFoldingCharacterSet, planeNo))))) {
                    UTF16Char caseFoldBuffer[MAX_CASE_MAPPING_BUF];
                    const UTF16Char *bufferP = caseFoldBuffer, *bufferLimit;
                    UTF32Char *outCharactersP = outCharacters;
                    uint32_t bufferLength = CFUniCharMapCaseTo(character, caseFoldBuffer, MAX_CASE_MAPPING_BUF, kCFUniCharCaseFold, 0, langCode);
                    
                    bufferLimit = bufferP + bufferLength;
                    
                    if (filledLength > 0) --filledLength; // decrement filledLength (will add back later)

                    // make space for casefold characters
                    if ((filledLength > 0) && (bufferLength > 1)) {
                        CFIndex totalScalerLength = 0;
                        
                        while (bufferP < bufferLimit) {
                            if (CFUniCharIsSurrogateHighCharacter(*(bufferP++)) && (bufferP < bufferLimit) && CFUniCharIsSurrogateLowCharacter(*bufferP)) ++bufferP;
                            ++totalScalerLength;
                        }
                        memmove(outCharacters + totalScalerLength, outCharacters + 1, filledLength * sizeof(UTF32Char));
                        bufferP = caseFoldBuffer;
                    }

                    // fill
                    while (bufferP < bufferLimit) {
                        character = *(bufferP++);
                        if (CFUniCharIsSurrogateHighCharacter(character) && (bufferP < bufferLimit) && CFUniCharIsSurrogateLowCharacter(*bufferP)) {
                            character = CFUniCharGetLongCharacterForSurrogatePair(character, *(bufferP++));
                            nonBaseBitmap = CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (character >> 16));
                        } else {
                            nonBaseBitmap = graphemeBMP;
                        }

                        if (!filterNonBase || !CFUniCharIsMemberOfBitmap(character, nonBaseBitmap)) {
                            *(outCharactersP++) = character;
                            ++filledLength;
                        }
                    }
                }
            }
        }

        // collect following combining marks
        if (flags & (kCFCompareDiacriticInsensitive|kCFCompareNonliteral)) {
            const uint8_t *nonBaseBitmap;
            const uint8_t *decompBitmap;
            bool doFill = (((flags & kCFCompareDiacriticInsensitive) && (character < 0x0510)) ? false : true);

            if (0 == filledLength) {
                *outCharacters = character; // filledLength will be updated below on demand
                
                if (doFill) { // check if really needs to fill
                    UTF32Char nonBaseCharacter = CFStringGetCharacterFromInlineBuffer(buffer, currentIndex);
                    
                    if (CFUniCharIsSurrogateHighCharacter(nonBaseCharacter) && CFUniCharIsSurrogateLowCharacter((lowSurrogate = CFStringGetCharacterFromInlineBuffer(buffer, currentIndex + 1)))) {
                        nonBaseCharacter = CFUniCharGetLongCharacterForSurrogatePair(nonBaseCharacter, lowSurrogate);
                        nonBaseBitmap = CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (nonBaseCharacter >> 16));
                        decompBitmap = CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, (nonBaseCharacter >> 16));
                    } else {
                        nonBaseBitmap = graphemeBMP;
                        decompBitmap = decompBMP;
                    }
                    
                    if (CFUniCharIsMemberOfBitmap(nonBaseCharacter, nonBaseBitmap)) {
                        filledLength = 1; // For the base character
                        
                        if ((0 == (flags & kCFCompareDiacriticInsensitive)) || (nonBaseCharacter > 0x050F)) {
                            if (CFUniCharIsMemberOfBitmap(nonBaseCharacter, decompBitmap)) {
                                filledLength += CFUniCharDecomposeCharacter(nonBaseCharacter, &(outCharacters[filledLength]), maxBufferLength - filledLength);
                            } else {
                                outCharacters[filledLength++] = nonBaseCharacter;
                            }
                        }
                        currentIndex += ((nonBaseBitmap == graphemeBMP) ? 1 : 2);
                    } else {
                        doFill = false;
                    }
                }
            }
            
            while (filledLength < maxBufferLength) { // do the rest
                character = CFStringGetCharacterFromInlineBuffer(buffer, currentIndex);
                
                if (CFUniCharIsSurrogateHighCharacter(character) && CFUniCharIsSurrogateLowCharacter((lowSurrogate = CFStringGetCharacterFromInlineBuffer(buffer, currentIndex + 1)))) {
                    character = CFUniCharGetLongCharacterForSurrogatePair(character, lowSurrogate);
                    nonBaseBitmap = CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (character >> 16));
                    decompBitmap = CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, (character >> 16));
                } else {
                    nonBaseBitmap = graphemeBMP;
                    decompBitmap = decompBMP;
                }
                if (isTurkikCapitalI) {
                    isTurkikCapitalI = false;
                } else if (CFUniCharIsMemberOfBitmap(character, nonBaseBitmap)) {
                    if (doFill) {
                        if (CFUniCharIsMemberOfBitmap(character, decompBitmap)) {
                            CFIndex currentLength = CFUniCharDecomposeCharacter(character, &(outCharacters[filledLength]), maxBufferLength - filledLength);
                            
                            if (0 == currentLength) break; // didn't fit
                            
                            filledLength += currentLength;
                        } else {
                            outCharacters[filledLength++] = character;
                        }
                    } else if (0 == filledLength) {
                        filledLength = 1; // For the base character
                    }
                    currentIndex += ((nonBaseBitmap == graphemeBMP) ? 1 : 2);
                } else {
                    break;
                }
            }

            if (filledLength > 1) {
                UTF32Char *sortCharactersLimit = outCharacters + filledLength;
                UTF32Char *sortCharacters = sortCharactersLimit - 1;

                while ((outCharacters < sortCharacters) && CFUniCharIsMemberOfBitmap(*sortCharacters, ((*sortCharacters < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (*sortCharacters >> 16))))) --sortCharacters;

                if ((sortCharactersLimit - sortCharacters) > 1) CFUniCharPrioritySort(sortCharacters, (sortCharactersLimit - sortCharacters)); // priority sort
            }
        }
    }
    
    if ((filledLength > 0) && (NULL != consumedLength)) *consumedLength = (currentIndex - index);
    
    return filledLength;
}

static bool __CFStringFillCharacterSetInlineBuffer(CFCharacterSetInlineBuffer *buffer, CFStringCompareFlags compareOptions) {
    if (0 != (compareOptions & kCFCompareIgnoreNonAlphanumeric)) {
	static CFCharacterSetRef nonAlnumChars = NULL;

	if (NULL == nonAlnumChars) {
	    CFMutableCharacterSetRef cset = CFCharacterSetCreateMutableCopy(kCFAllocatorSystemDefault, CFCharacterSetGetPredefined(kCFCharacterSetAlphaNumeric));
	    CFCharacterSetInvert(cset);
	    if (!OSAtomicCompareAndSwapPtrBarrier(NULL, cset, (void **)&nonAlnumChars)) CFRelease(cset);
	}

	CFCharacterSetInitInlineBuffer(nonAlnumChars, buffer);

	return true;
    }

    return false;
}

#define kCFStringStackBufferLength (__kCFStringInlineBufferLength)

CFComparisonResult CFStringCompareWithOptionsAndLocale(CFStringRef string, CFStringRef string2, CFRange rangeToCompare, CFStringCompareFlags compareOptions, CFLocaleRef locale) {
    /* No objc dispatch needed here since CFStringInlineBuffer works with both CFString and NSString */
    UTF32Char strBuf1[kCFStringStackBufferLength];
    UTF32Char strBuf2[kCFStringStackBufferLength];
    CFStringInlineBuffer inlineBuf1, inlineBuf2;
    UTF32Char str1Char, str2Char;
    CFIndex str1UsedLen, str2UsedLen;
    CFIndex str1Index = 0, str2Index = 0, strBuf1Index = 0, strBuf2Index = 0, strBuf1Len = 0, strBuf2Len = 0;
    CFIndex str1LocalizedIndex = 0, str2LocalizedIndex = 0;
    CFIndex forcedIndex1 = 0, forcedIndex2 = 0;
    CFIndex str2Len = CFStringGetLength(string2);
    bool caseInsensitive = ((compareOptions & kCFCompareCaseInsensitive) ? true : false);
    bool diacriticsInsensitive = ((compareOptions & kCFCompareDiacriticInsensitive) ? true : false);
    bool equalityOptions = ((compareOptions & (kCFCompareCaseInsensitive|kCFCompareNonliteral|kCFCompareDiacriticInsensitive|kCFCompareWidthInsensitive)) ? true : false);
    bool numerically = ((compareOptions & kCFCompareNumerically) ? true : false);
    bool forceOrdering = ((compareOptions & kCFCompareForcedOrdering) ? true : false);
    const uint8_t *langCode;
    CFComparisonResult compareResult = kCFCompareEqualTo;
    UTF16Char otherChar;
    Boolean freeLocale = false;
    CFCharacterSetInlineBuffer *ignoredChars = NULL;
    CFCharacterSetInlineBuffer csetBuffer;
    bool numericEquivalence = false;

    if ((compareOptions & kCFCompareLocalized) && (NULL == locale)) {
        locale = CFLocaleCopyCurrent();
	freeLocale = true;
    }

    langCode = ((NULL == locale) ? NULL : (const uint8_t *)_CFStrGetLanguageIdentifierForLocale(locale, true));

    if (__CFStringFillCharacterSetInlineBuffer(&csetBuffer, compareOptions)) {
	ignoredChars = &csetBuffer;
	equalityOptions = true;
    }

    if ((NULL == locale) && (NULL == ignoredChars) && !numerically) { // could do binary comp (be careful when adding new flags)
        CFStringEncoding eightBitEncoding = __CFStringGetEightBitStringEncoding();
        const uint8_t *str1Bytes = (const uint8_t *)CFStringGetCStringPtr(string, eightBitEncoding);
        const uint8_t *str2Bytes = (const uint8_t *)CFStringGetCStringPtr(string2, eightBitEncoding);
        CFIndex factor = sizeof(uint8_t);

        if ((NULL != str1Bytes) && (NULL != str2Bytes)) {
            compareOptions &= ~kCFCompareNonliteral; // remove non-literal

            if ((kCFStringEncodingASCII == eightBitEncoding) && (false == forceOrdering)) {
                if (caseInsensitive) {
                    int cmpResult = strncasecmp_l((const char *)str1Bytes + rangeToCompare.location, (const char *)str2Bytes, __CFMin(rangeToCompare.length, str2Len), NULL);
                    
                    if (0 == cmpResult) cmpResult = rangeToCompare.length - str2Len;
                    
                    return ((0 == cmpResult) ? kCFCompareEqualTo : ((cmpResult < 0) ? kCFCompareLessThan : kCFCompareGreaterThan));
                }
            } else if (caseInsensitive || diacriticsInsensitive) {
                CFIndex limitLength = __CFMin(rangeToCompare.length, str2Len);

                str1Bytes += rangeToCompare.location;

                while (str1Index < limitLength) {
                    str1Char = str1Bytes[str1Index];
                    str2Char = str2Bytes[str1Index];

                    if (str1Char != str2Char) {
                        if ((str1Char < 0x80) && (str2Char < 0x80)) {
			    if (forceOrdering && (kCFCompareEqualTo == compareResult) && (str1Char != str2Char)) compareResult = ((str1Char < str2Char) ? kCFCompareLessThan : kCFCompareGreaterThan);
			    if (caseInsensitive) {
				if ((str1Char >= 'A') && (str1Char <= 'Z')) str1Char += ('a' - 'A');
				if ((str2Char >= 'A') && (str2Char <= 'Z')) str2Char += ('a' - 'A');
			    }

                            if (str1Char != str2Char) return ((str1Char < str2Char) ? kCFCompareLessThan : kCFCompareGreaterThan);
                        } else {
                            str1Bytes = NULL;
                            break;
                        }
                    }
                    ++str1Index;
                }

                str2Index = str1Index;
                
                if (str1Index == limitLength) {
                    int cmpResult = rangeToCompare.length - str2Len;
                    
                    return ((0 == cmpResult) ? compareResult : ((cmpResult < 0) ? kCFCompareLessThan : kCFCompareGreaterThan));
                }
            }
        } else if (!equalityOptions && (NULL == str1Bytes) && (NULL == str2Bytes)) {
            str1Bytes = (const uint8_t *)CFStringGetCharactersPtr(string);
            str2Bytes = (const uint8_t *)CFStringGetCharactersPtr(string2);
            factor = sizeof(UTF16Char);
#if __LITTLE_ENDIAN__
            if ((NULL != str1Bytes) && (NULL != str2Bytes)) { // we cannot use memcmp
                const UTF16Char *str1 = ((const UTF16Char *)str1Bytes) + rangeToCompare.location;
                const UTF16Char *str1Limit = str1 + __CFMin(rangeToCompare.length, str2Len);
                const UTF16Char *str2 = (const UTF16Char *)str2Bytes;
                CFIndex cmpResult = 0;

                while ((0 == cmpResult) && (str1 < str1Limit)) cmpResult = (CFIndex)*(str1++) - (CFIndex)*(str2++);

                if (0 == cmpResult) cmpResult = rangeToCompare.length - str2Len;
                
                return ((0 == cmpResult) ? kCFCompareEqualTo : ((cmpResult < 0) ? kCFCompareLessThan : kCFCompareGreaterThan));
            }
#endif /* __LITTLE_ENDIAN__ */
        }
        if ((NULL != str1Bytes) && (NULL != str2Bytes)) {
            int cmpResult = memcmp(str1Bytes + (rangeToCompare.location * factor), str2Bytes, __CFMin(rangeToCompare.length, str2Len) * factor);
            
            if (0 == cmpResult) cmpResult = rangeToCompare.length - str2Len;
            
            return ((0 == cmpResult) ? kCFCompareEqualTo : ((cmpResult < 0) ? kCFCompareLessThan : kCFCompareGreaterThan));
        }
    }
    
    const uint8_t *graphemeBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, 0);
    
    CFStringInitInlineBuffer(string, &inlineBuf1, rangeToCompare);
    CFStringInitInlineBuffer(string2, &inlineBuf2, CFRangeMake(0, str2Len));

    if (NULL != locale) {
	str1LocalizedIndex = str1Index;
	str2LocalizedIndex = str2Index;

	// We temporarily disable kCFCompareDiacriticInsensitive for SL <rdar://problem/6767096>. Should be revisited in NMOS <rdar://problem/7003830>
	if (forceOrdering) {
	    diacriticsInsensitive = false;
	    compareOptions &= ~kCFCompareDiacriticInsensitive;
	}
    }
    while ((str1Index < rangeToCompare.length) && (str2Index < str2Len)) {
        if (strBuf1Len == 0) {
            str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index);
            if (caseInsensitive && (str1Char >= 'A') && (str1Char <= 'Z') && ((NULL == langCode) || (str1Char != 'I')) && ((false == forceOrdering) || (kCFCompareEqualTo != compareResult))) str1Char += ('a' - 'A');
            str1UsedLen = 1;
        } else {
            str1Char = strBuf1[strBuf1Index++];
        }
        if (strBuf2Len == 0) {
            str2Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index);
            if (caseInsensitive && (str2Char >= 'A') && (str2Char <= 'Z') && ((NULL == langCode) || (str2Char != 'I')) && ((false == forceOrdering) || (kCFCompareEqualTo != compareResult))) str2Char += ('a' - 'A');
            str2UsedLen = 1;
        } else {
            str2Char = strBuf2[strBuf2Index++];
        }

        if (numerically && ((0 == strBuf1Len) && (str1Char <= '9') && (str1Char >= '0')) && ((0 == strBuf2Len) && (str2Char <= '9') && (str2Char >= '0'))) { // If both are not ASCII digits, then don't do numerical comparison here
            uint64_t intValue1 = 0, intValue2 = 0;	// !!! Doesn't work if numbers are > max uint64_t
            CFIndex str1NumRangeIndex = str1Index;
            CFIndex str2NumRangeIndex = str2Index;

            do {
                intValue1 = (intValue1 * 10) + (str1Char - '0');
                str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, ++str1Index);
            } while ((str1Char <= '9') && (str1Char >= '0'));

            do {
                intValue2 = intValue2 * 10 + (str2Char - '0');
                str2Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, ++str2Index);
            } while ((str2Char <= '9') && (str2Char >= '0'));

            if (intValue1 == intValue2) {
                if (forceOrdering && (kCFCompareEqualTo == compareResult) && ((str1Index - str1NumRangeIndex) != (str2Index - str2NumRangeIndex))) {
                    compareResult = (((str1Index - str1NumRangeIndex) < (str2Index - str2NumRangeIndex)) ? kCFCompareLessThan : kCFCompareGreaterThan);
                    numericEquivalence = true;
                    forcedIndex1 = str1NumRangeIndex;
                    forcedIndex2 = str2NumRangeIndex;
                }

                continue;
            } else if (intValue1 < intValue2) {
		if (freeLocale && locale) {
		    CFRelease(locale);
		}
                return kCFCompareLessThan;
            } else {
		if (freeLocale && locale) {
		    CFRelease(locale);
		}
                return kCFCompareGreaterThan;
            }
        }

        if (str1Char != str2Char) {
            if (!equalityOptions) {
		compareResult = ((NULL == locale) ? ((str1Char < str2Char) ? kCFCompareLessThan : kCFCompareGreaterThan) : _CFCompareStringsWithLocale(&inlineBuf1, CFRangeMake(str1Index, rangeToCompare.length - str1Index), &inlineBuf2, CFRangeMake(str2Index, str2Len - str2Index), compareOptions, locale));
                if (freeLocale && locale) {
                    CFRelease(locale);
                }
		return compareResult;
	    }

            if (forceOrdering && (kCFCompareEqualTo == compareResult)) {
		compareResult = ((str1Char < str2Char) ? kCFCompareLessThan : kCFCompareGreaterThan);
		forcedIndex1 = str1LocalizedIndex;
		forcedIndex2 = str2LocalizedIndex;
	    }

            if ((str1Char < 0x80) && (str2Char < 0x80) && (NULL == ignoredChars)) {
                if (NULL != locale) {
		    compareResult = _CFCompareStringsWithLocale(&inlineBuf1, CFRangeMake(str1Index, rangeToCompare.length - str1Index), &inlineBuf2, CFRangeMake(str2Index, str2Len - str2Index), compareOptions, locale);
		    if (freeLocale && locale) {
			CFRelease(locale);
		    }
		    return compareResult;
                } else if (!caseInsensitive) {
		    if (freeLocale && locale) {
			CFRelease(locale);
		    }
                    return ((str1Char < str2Char) ? kCFCompareLessThan : kCFCompareGreaterThan);
                }
            }

            if (CFUniCharIsSurrogateHighCharacter(str1Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index + 1)))) {
                str1Char = CFUniCharGetLongCharacterForSurrogatePair(str1Char, otherChar);
                str1UsedLen = 2;
            }
            
            if (CFUniCharIsSurrogateHighCharacter(str2Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index + 1)))) {
                str2Char = CFUniCharGetLongCharacterForSurrogatePair(str2Char, otherChar);
                str2UsedLen = 2;
            }
            
	    if (NULL != ignoredChars) {
		if (CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str1Char)) {
		    if ((strBuf1Len > 0) && (strBuf1Index == strBuf1Len)) strBuf1Len = 0;
		    if (strBuf1Len == 0) str1Index += str1UsedLen;
		    if (strBuf2Len > 0) --strBuf2Index;
		    continue;
		}
		if (CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str2Char)) {
		    if ((strBuf2Len > 0) && (strBuf2Index == strBuf2Len)) strBuf2Len = 0;
		    if (strBuf2Len == 0) str2Index += str2UsedLen;
		    if (strBuf1Len > 0) -- strBuf1Index;
		    continue;
		}	    
	    }
	    
            if (diacriticsInsensitive && (str1Index > 0)) {
                bool str1Skip = false;
                bool str2Skip = false;
                
                if ((0 == strBuf1Len) && CFUniCharIsMemberOfBitmap(str1Char, ((str1Char < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (str1Char >> 16))))) {
                    str1Char = str2Char;
                    str1Skip = true;
                }
                if ((0 == strBuf2Len) && CFUniCharIsMemberOfBitmap(str2Char, ((str2Char < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (str2Char >> 16))))) {
                    str2Char = str1Char;
                    str2Skip = true;
                }
                
                if (str1Skip != str2Skip) {
                    if (str1Skip) str2Index -= str2UsedLen;
                    if (str2Skip) str1Index -= str1UsedLen;
                }
            }

            if (str1Char != str2Char) {
                if (0 == strBuf1Len) {
                    strBuf1Len = __CFStringFoldCharacterClusterAtIndex(str1Char, &inlineBuf1, str1Index, compareOptions, langCode, strBuf1, kCFStringStackBufferLength, &str1UsedLen);
                    if (strBuf1Len > 0) {
                        str1Char = *strBuf1;
                        strBuf1Index = 1;
                    }
                }
                
                if ((0 == strBuf1Len) && (0 < strBuf2Len)) {
		    compareResult =  ((NULL == locale) ? ((str1Char < str2Char) ? kCFCompareLessThan : kCFCompareGreaterThan) : _CFCompareStringsWithLocale(&inlineBuf1, CFRangeMake(str1LocalizedIndex, rangeToCompare.length - str1LocalizedIndex), &inlineBuf2, CFRangeMake(str2LocalizedIndex, str2Len - str2LocalizedIndex), compareOptions, locale));
		    if (freeLocale && locale) {
			CFRelease(locale);
		    }
		    return compareResult;
		}
                
                if ((0 == strBuf2Len) && ((0 == strBuf1Len) || (str1Char != str2Char))) {
                    strBuf2Len = __CFStringFoldCharacterClusterAtIndex(str2Char, &inlineBuf2, str2Index, compareOptions, langCode, strBuf2, kCFStringStackBufferLength, &str2UsedLen);
                    if (strBuf2Len > 0) {
                        str2Char = *strBuf2;
                        strBuf2Index = 1;
                    }
                    if ((0 == strBuf2Len) || (str1Char != str2Char)) {
			compareResult = ((NULL == locale) ? ((str1Char < str2Char) ? kCFCompareLessThan : kCFCompareGreaterThan) : _CFCompareStringsWithLocale(&inlineBuf1, CFRangeMake(str1LocalizedIndex, rangeToCompare.length - str1LocalizedIndex), &inlineBuf2, CFRangeMake(str2LocalizedIndex, str2Len - str2LocalizedIndex), compareOptions, locale));
			if (freeLocale && locale) {
			    CFRelease(locale);
			}
			return compareResult;
		    }
                }
            }
            
            if ((strBuf1Len > 0) && (strBuf2Len > 0)) {
                while ((strBuf1Index < strBuf1Len) && (strBuf2Index < strBuf2Len)) {
                    if (strBuf1[strBuf1Index] != strBuf2[strBuf2Index]) break;
                    ++strBuf1Index; ++strBuf2Index;
                }
                if ((strBuf1Index < strBuf1Len) && (strBuf2Index < strBuf2Len)) {
		    CFComparisonResult res = ((NULL == locale) ? ((strBuf1[strBuf1Index] < strBuf2[strBuf2Index]) ? kCFCompareLessThan : kCFCompareGreaterThan) : _CFCompareStringsWithLocale(&inlineBuf1, CFRangeMake(str1LocalizedIndex, rangeToCompare.length - str1LocalizedIndex), &inlineBuf2, CFRangeMake(str2LocalizedIndex, str2Len - str2LocalizedIndex), compareOptions, locale));
		    if (freeLocale && locale) {
			CFRelease(locale);
		    }
		    return res;
		}
            }
        }
        
        if ((strBuf1Len > 0) && (strBuf1Index == strBuf1Len)) strBuf1Len = 0;
        if ((strBuf2Len > 0) && (strBuf2Index == strBuf2Len)) strBuf2Len = 0;
        
        if (strBuf1Len == 0) str1Index += str1UsedLen;
        if (strBuf2Len == 0) str2Index += str2UsedLen;
	if ((strBuf1Len == 0) && (strBuf2Len == 0)) {
	    str1LocalizedIndex = str1Index;
	    str2LocalizedIndex = str2Index;
	}
    }

    if (diacriticsInsensitive || (NULL != ignoredChars)) {
        while (str1Index < rangeToCompare.length) {
            str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index);
            if ((str1Char < 0x80) && (NULL == ignoredChars)) break; // found ASCII

            if (CFUniCharIsSurrogateHighCharacter(str1Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index + 1)))) str1Char = CFUniCharGetLongCharacterForSurrogatePair(str1Char, otherChar);

            if ((!diacriticsInsensitive || !CFUniCharIsMemberOfBitmap(str1Char, ((str1Char < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (str1Char >> 16))))) && ((NULL == ignoredChars) || !CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str1Char))) break;

            str1Index += ((str1Char < 0x10000) ? 1 : 2);
        }

        while (str2Index < str2Len) {
            str2Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index);
            if ((str2Char < 0x80) && (NULL == ignoredChars)) break; // found ASCII
                
            if (CFUniCharIsSurrogateHighCharacter(str2Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index + 1)))) str2Char = CFUniCharGetLongCharacterForSurrogatePair(str2Char, otherChar);

            if ((!diacriticsInsensitive || !CFUniCharIsMemberOfBitmap(str2Char, ((str2Char < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (str2Char >> 16))))) && ((NULL == ignoredChars) || !CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str2Char))) break;

            str2Index += ((str2Char < 0x10000) ? 1 : 2);
        }
    }
    // Need to recalc localized result here for forced ordering, ICU cannot do numericEquivalence
    if (!numericEquivalence && (NULL != locale) && (kCFCompareEqualTo != compareResult) && (str1Index == rangeToCompare.length) && (str2Index == str2Len)) compareResult = _CFCompareStringsWithLocale(&inlineBuf1, CFRangeMake(forcedIndex1, rangeToCompare.length - forcedIndex1), &inlineBuf2, CFRangeMake(forcedIndex2, str2Len - forcedIndex2), compareOptions, locale);

    if (freeLocale && locale) {
	CFRelease(locale);
    }

    return ((str1Index < rangeToCompare.length) ? kCFCompareGreaterThan : ((str2Index < str2Len) ? kCFCompareLessThan : compareResult));
}


CFComparisonResult CFStringCompareWithOptions(CFStringRef string, CFStringRef string2, CFRange rangeToCompare, CFStringCompareFlags compareOptions) { return CFStringCompareWithOptionsAndLocale(string, string2, rangeToCompare, compareOptions, NULL); }

CFComparisonResult CFStringCompare(CFStringRef string, CFStringRef str2, CFStringCompareFlags options) {
    return CFStringCompareWithOptions(string, str2, CFRangeMake(0, CFStringGetLength(string)), options);
}

Boolean CFStringFindWithOptionsAndLocale(CFStringRef string, CFStringRef stringToFind, CFRange rangeToSearch, CFStringCompareFlags compareOptions, CFLocaleRef locale, CFRange *result)  {
    /* No objc dispatch needed here since CFStringInlineBuffer works with both CFString and NSString */
    CFIndex findStrLen = CFStringGetLength(stringToFind);
    Boolean didFind = false;
    bool lengthVariants = ((compareOptions & (kCFCompareCaseInsensitive|kCFCompareNonliteral|kCFCompareDiacriticInsensitive)) ? true : false);
    CFCharacterSetInlineBuffer *ignoredChars = NULL;
    CFCharacterSetInlineBuffer csetBuffer;

    if (__CFStringFillCharacterSetInlineBuffer(&csetBuffer, compareOptions)) {
	ignoredChars = &csetBuffer;
	lengthVariants = true;
    }

    if ((findStrLen > 0) && (rangeToSearch.length > 0) && ((findStrLen <= rangeToSearch.length) || lengthVariants)) {
        UTF32Char strBuf1[kCFStringStackBufferLength];
        UTF32Char strBuf2[kCFStringStackBufferLength];
        CFStringInlineBuffer inlineBuf1, inlineBuf2;
        UTF32Char str1Char = 0, str2Char = 0;
        CFStringEncoding eightBitEncoding = __CFStringGetEightBitStringEncoding();
        const uint8_t *str1Bytes = (const uint8_t *)CFStringGetCStringPtr(string, eightBitEncoding);
        const uint8_t *str2Bytes = (const uint8_t *)CFStringGetCStringPtr(stringToFind, eightBitEncoding);
        const UTF32Char *characters, *charactersLimit;
        const uint8_t *langCode = NULL;
        CFIndex fromLoc, toLoc;
        CFIndex str1Index, str2Index;
        CFIndex strBuf1Len, strBuf2Len;
	CFIndex maxStr1Index = (rangeToSearch.location + rangeToSearch.length);
        bool equalityOptions = ((lengthVariants || (compareOptions & kCFCompareWidthInsensitive)) ? true : false);
        bool caseInsensitive = ((compareOptions & kCFCompareCaseInsensitive) ? true : false);
	bool forwardAnchor = ((kCFCompareAnchored == (compareOptions & (kCFCompareBackwards|kCFCompareAnchored))) ? true : false);
	bool backwardAnchor = (((kCFCompareBackwards|kCFCompareAnchored) == (compareOptions & (kCFCompareBackwards|kCFCompareAnchored))) ? true : false);
        int8_t delta;

        if (NULL == locale) {
            if (compareOptions & kCFCompareLocalized) {
                CFLocaleRef currentLocale = CFLocaleCopyCurrent();
                langCode = (const uint8_t *)_CFStrGetLanguageIdentifierForLocale(currentLocale, true);
		CFRelease(currentLocale);
            }
        } else {
            langCode = (const uint8_t *)_CFStrGetLanguageIdentifierForLocale(locale, true);
        }

        CFStringInitInlineBuffer(string, &inlineBuf1, CFRangeMake(0, rangeToSearch.location + rangeToSearch.length));
        CFStringInitInlineBuffer(stringToFind, &inlineBuf2, CFRangeMake(0, findStrLen));

        if (compareOptions & kCFCompareBackwards) {
            fromLoc = rangeToSearch.location + rangeToSearch.length - (lengthVariants ? 1 : findStrLen);
            toLoc = (((compareOptions & kCFCompareAnchored) && !lengthVariants) ? fromLoc : rangeToSearch.location);
        } else {
            fromLoc = rangeToSearch.location;
            toLoc = ((compareOptions & kCFCompareAnchored) ? fromLoc : rangeToSearch.location + rangeToSearch.length - (lengthVariants ? 1 : findStrLen));
        }
        
        delta = ((fromLoc <= toLoc) ? 1 : -1);

        if ((NULL != str1Bytes) && (NULL != str2Bytes)) {
            uint8_t str1Byte, str2Byte;

            while (1) {
                str1Index = fromLoc;
                str2Index = 0;

                while ((str1Index < maxStr1Index) && (str2Index < findStrLen)) {
                    str1Byte = str1Bytes[str1Index];
                    str2Byte = str2Bytes[str2Index];

                    if (str1Byte != str2Byte) {
                        if (equalityOptions) {
                            if ((str1Byte < 0x80) && ((NULL == langCode) || ('I' != str1Byte))) {
                                if (caseInsensitive && (str1Byte >= 'A') && (str1Byte <= 'Z')) str1Byte += ('a' - 'A');
                                *strBuf1 = str1Byte;
                                strBuf1Len = 1;
                            } else {
                                str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index);
                                strBuf1Len = __CFStringFoldCharacterClusterAtIndex(str1Char, &inlineBuf1, str1Index, compareOptions, langCode, strBuf1, kCFStringStackBufferLength, NULL);
                                if (1 > strBuf1Len) {
                                    *strBuf1 = str1Char;
                                    strBuf1Len = 1;
                                }
                            }

			    if ((NULL != ignoredChars) && (forwardAnchor || (str1Index != fromLoc)) && CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, ((str1Byte < 0x80) ? str1Byte : str1Char))) {
				++str1Index;
				continue;
			    }

                            if ((str2Byte < 0x80) && ((NULL == langCode) || ('I' != str2Byte))) {
                                if (caseInsensitive && (str2Byte >= 'A') && (str2Byte <= 'Z')) str2Byte += ('a' - 'A');
                                *strBuf2 = str2Byte;
                                strBuf2Len = 1;
                            } else {
                                str2Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index);
                                strBuf2Len = __CFStringFoldCharacterClusterAtIndex(str2Char, &inlineBuf2, str2Index, compareOptions, langCode, strBuf2, kCFStringStackBufferLength, NULL);
                                if (1 > strBuf2Len) {
                                    *strBuf2 = str2Char;
                                    strBuf2Len = 1;
                                }
                            }

			    if ((NULL != ignoredChars) && CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, ((str2Byte < 0x80) ? str2Byte : str2Char))) {
				++str2Index;
				continue;
			    }

                            if ((1 == strBuf1Len) && (1 == strBuf2Len)) { // normal case
                                if (*strBuf1 != *strBuf2) break;
                            } else {
                                CFIndex delta;

                                if (!caseInsensitive && (strBuf1Len != strBuf2Len)) break;
                                if (memcmp(strBuf1, strBuf2, sizeof(UTF32Char) * __CFMin(strBuf1Len, strBuf2Len))) break;

                                if (strBuf1Len < strBuf2Len) {
                                    delta = strBuf2Len - strBuf1Len;

                                    if ((str1Index + strBuf1Len + delta) > maxStr1Index) break;

                                    characters = &(strBuf2[strBuf1Len]);
                                    charactersLimit = characters + delta;

                                    while (characters < charactersLimit) {
                                        strBuf1Len = __CFStringFoldCharacterClusterAtIndex(CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index + 1), &inlineBuf1, str1Index + 1, compareOptions, langCode, strBuf1, kCFStringStackBufferLength, NULL);
                                        if ((strBuf1Len > 0) || (*characters != *strBuf1)) break;
                                        ++characters; ++str1Index;
                                    }
                                    if (characters < charactersLimit) break;
                                } else if (strBuf2Len < strBuf1Len) {
                                    delta = strBuf1Len - strBuf2Len;
                                    
                                    if ((str2Index + strBuf2Len + delta) > findStrLen) break;
                                    
                                    characters = &(strBuf1[strBuf2Len]);
                                    charactersLimit = characters + delta;
                                    
                                    while (characters < charactersLimit) {
                                        strBuf2Len = __CFStringFoldCharacterClusterAtIndex(CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str1Index + 1), &inlineBuf2, str2Index + 1, compareOptions, langCode, strBuf2, kCFStringStackBufferLength, NULL);
                                        if ((strBuf2Len > 0) || (*characters != *strBuf2)) break;
                                        ++characters; ++str2Index;
                                    }
                                    if (characters < charactersLimit) break;
                                }
                            }
                        } else {
                            break;
                        }
                    }
                    ++str1Index; ++str2Index;
                }

		if ((NULL != ignoredChars) && (str1Index == maxStr1Index) && (str2Index < findStrLen)) { // Process the stringToFind tail
		    while (str2Index < findStrLen) {
			str2Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index);

			if (!CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str2Char)) break;
			++str2Index;
		    }
		}

                if (str2Index == findStrLen) {
		    if ((NULL != ignoredChars) && backwardAnchor && (str1Index < maxStr1Index)) { // Process the anchor tail
			while (str1Index < maxStr1Index) {
			    str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index);
			    
			    if (!CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str1Char)) break;
			    ++str1Index;
			}
		    }

                    if (!backwardAnchor || (str1Index == maxStr1Index)) {
                        didFind = true;
                        if (NULL != result) *result = CFRangeMake(fromLoc, str1Index - fromLoc);
                    }
                    break;
                }

                if (fromLoc == toLoc) break;
                fromLoc += delta;
            }
        } else if (equalityOptions) {
            UTF16Char otherChar;
            CFIndex str1UsedLen, str2UsedLen, strBuf1Index = 0, strBuf2Index = 0;
            bool diacriticsInsensitive = ((compareOptions & kCFCompareDiacriticInsensitive) ? true : false);
            const uint8_t *graphemeBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, 0);
            const uint8_t *combClassBMP = (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, 0);

            while (1) {
                str1Index = fromLoc;
                str2Index = 0;

                strBuf1Len = strBuf2Len = 0;

                while (str2Index < findStrLen) {
                    if (strBuf1Len == 0) {
                        str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index);
                        if (caseInsensitive && (str1Char >= 'A') && (str1Char <= 'Z') && ((NULL == langCode) || (str1Char != 'I'))) str1Char += ('a' - 'A');
                        str1UsedLen = 1;
                    } else {
                        str1Char = strBuf1[strBuf1Index++];
                    }
                    if (strBuf2Len == 0) {
                        str2Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index);
                        if (caseInsensitive && (str2Char >= 'A') && (str2Char <= 'Z') && ((NULL == langCode) || (str2Char != 'I'))) str2Char += ('a' - 'A');
                        str2UsedLen = 1;
                    } else {
                        str2Char = strBuf2[strBuf2Index++];
                    }

                    if (str1Char != str2Char) {
                        if ((str1Char < 0x80) && (str2Char < 0x80) && (NULL == ignoredChars) && ((NULL == langCode) || !caseInsensitive)) break;

                        if (CFUniCharIsSurrogateHighCharacter(str1Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index + 1)))) {
                            str1Char = CFUniCharGetLongCharacterForSurrogatePair(str1Char, otherChar);
                            str1UsedLen = 2;
                        }

                        if (CFUniCharIsSurrogateHighCharacter(str2Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index + 1)))) {
                            str2Char = CFUniCharGetLongCharacterForSurrogatePair(str2Char, otherChar);
                            str2UsedLen = 2;
                        }

			if (NULL != ignoredChars) {
			    if ((forwardAnchor || (str1Index != fromLoc)) && (str1Index < maxStr1Index) && CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str1Char)) {
				if ((strBuf1Len > 0) && (strBuf1Index == strBuf1Len)) strBuf1Len = 0;
				if (strBuf1Len == 0) str1Index += str1UsedLen;
				if (strBuf2Len > 0) --strBuf2Index;
				continue;
			    }
			    if (CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str2Char)) {
				if ((strBuf2Len > 0) && (strBuf2Index == strBuf2Len)) strBuf2Len = 0;
				if (strBuf2Len == 0) str2Index += str2UsedLen;
				if (strBuf1Len > 0) -- strBuf1Index;
				continue;
			    }	    
			}
			
                        if (diacriticsInsensitive && (str1Index > fromLoc)) {
                            bool str1Skip = false;
                            bool str2Skip = false;

                            if ((0 == strBuf1Len) && CFUniCharIsMemberOfBitmap(str1Char, ((str1Char < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (str1Char >> 16))))) {
                                str1Char = str2Char;
                                str1Skip = true;
                            }
                            if ((0 == strBuf2Len) && CFUniCharIsMemberOfBitmap(str2Char, ((str2Char < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (str2Char >> 16))))) {
                                str2Char = str1Char;
                                str2Skip = true;
                            }

                            if (str1Skip != str2Skip) {
                                if (str1Skip) str2Index -= str2UsedLen;
                                if (str2Skip) str1Index -= str1UsedLen;
                            }
                        }

                        if (str1Char != str2Char) {
                            if (0 == strBuf1Len) {
                                strBuf1Len = __CFStringFoldCharacterClusterAtIndex(str1Char, &inlineBuf1, str1Index, compareOptions, langCode, strBuf1, kCFStringStackBufferLength, &str1UsedLen);
                                if (strBuf1Len > 0) {
                                    str1Char = *strBuf1;
                                    strBuf1Index = 1;
                                }
                            }

                            if ((0 == strBuf1Len) && (0 < strBuf2Len)) break;

                            if ((0 == strBuf2Len) && ((0 == strBuf1Len) || (str1Char != str2Char))) {
                                strBuf2Len = __CFStringFoldCharacterClusterAtIndex(str2Char, &inlineBuf2, str2Index, compareOptions, langCode, strBuf2, kCFStringStackBufferLength, &str2UsedLen);
                                if ((0 == strBuf2Len) || (str1Char != *strBuf2)) break;
                                strBuf2Index = 1;
                            }
                        }

                        if ((strBuf1Len > 0) && (strBuf2Len > 0)) {
                            while ((strBuf1Index < strBuf1Len) && (strBuf2Index < strBuf2Len)) {
                                if (strBuf1[strBuf1Index] != strBuf2[strBuf2Index]) break;
                                ++strBuf1Index; ++strBuf2Index;
                            }
                            if ((strBuf1Index < strBuf1Len) && (strBuf2Index < strBuf2Len)) break;
                        }
                    }
                    
                    if ((strBuf1Len > 0) && (strBuf1Index == strBuf1Len)) strBuf1Len = 0;
                    if ((strBuf2Len > 0) && (strBuf2Index == strBuf2Len)) strBuf2Len = 0;

                    if (strBuf1Len == 0) str1Index += str1UsedLen;
                    if (strBuf2Len == 0) str2Index += str2UsedLen;
                }

		if ((NULL != ignoredChars) && (str1Index == maxStr1Index) && (str2Index < findStrLen)) { // Process the stringToFind tail
		    while (str2Index < findStrLen) {
			str2Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index);
                        if (CFUniCharIsSurrogateHighCharacter(str2Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index + 1)))) {
                            str2Char = CFUniCharGetLongCharacterForSurrogatePair(str2Char, otherChar);
                        }
			if (!CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str2Char)) break;
			str2Index += ((str2Char < 0x10000) ? 1 : 2);
		    }
		}

                if (str2Index == findStrLen) {
                    bool match = true;

                    if (strBuf1Len > 0) {
                        match = false;

                        if (diacriticsInsensitive && (strBuf1[0] < 0x0510)) {
                            while (strBuf1Index < strBuf1Len) {
                                if (!CFUniCharIsMemberOfBitmap(strBuf1[strBuf1Index], ((strBuf1[strBuf1Index] < 0x10000) ? graphemeBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, (strBuf1[strBuf1Index] >> 16))))) break;
                                ++strBuf1Index;
                            }

                            if (strBuf1Index == strBuf1Len) {
                                str1Index += str1UsedLen;
                                match = true;
                            }
                        }
                    }

                    if (match && (compareOptions & (kCFCompareDiacriticInsensitive|kCFCompareNonliteral)) && (str1Index < maxStr1Index)) {
                        const uint8_t *nonBaseBitmap;

                        str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index);
                        
                        if (CFUniCharIsSurrogateHighCharacter(str1Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index + 1)))) {
                            str1Char = CFUniCharGetLongCharacterForSurrogatePair(str1Char, otherChar);
                            nonBaseBitmap = CFUniCharGetBitmapPtrForPlane(kCFUniCharGraphemeExtendCharacterSet, (str1Char >> 16));
                        } else {
                            nonBaseBitmap = graphemeBMP;
                        }

                        if (CFUniCharIsMemberOfBitmap(str1Char, nonBaseBitmap)) {
                            if (diacriticsInsensitive) {
                                if (str1Char < 0x10000) {
                                    CFIndex index = str1Index;

                                    do {
                                        str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, --index);
                                    } while (CFUniCharIsMemberOfBitmap(str1Char, graphemeBMP), (rangeToSearch.location < index));

                                    if (str1Char < 0x0510) {
                                        while (++str1Index < maxStr1Index) if (!CFUniCharIsMemberOfBitmap(CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index), graphemeBMP)) break;
                                    }
                                }
                            } else {
                                match = false;
                            }
                        } else if (!diacriticsInsensitive) {
                            otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index - 1);
                            
                            // this is assuming viramas are only in BMP ???
                            if ((str1Char == COMBINING_GRAPHEME_JOINER) || (otherChar == COMBINING_GRAPHEME_JOINER) || (otherChar == ZERO_WIDTH_JOINER) || ((otherChar >= HANGUL_CHOSEONG_START) && (otherChar <= HANGUL_JONGSEONG_END)) || (CFUniCharGetCombiningPropertyForCharacter(otherChar, combClassBMP) == 9)) {
                                CFRange clusterRange = CFStringGetRangeOfCharacterClusterAtIndex(string, str1Index - 1, kCFStringGraphemeCluster);
                                
                                if (str1Index < (clusterRange.location + clusterRange.length)) match = false;
                            }
                        }
                    }

                    if (match) {
			if ((NULL != ignoredChars) && backwardAnchor && (str1Index < maxStr1Index)) { // Process the anchor tail
			    while (str1Index < maxStr1Index) {
				str1Char = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index);
				if (CFUniCharIsSurrogateHighCharacter(str1Char) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index + 1)))) {
				    str1Char = CFUniCharGetLongCharacterForSurrogatePair(str1Char, otherChar);
				}
				if (!CFCharacterSetInlineBufferIsLongCharacterMember(ignoredChars, str1Char)) break;
				str1Index += ((str1Char < 0x10000) ? 1 : 2);
			    }
			}

                        if (!backwardAnchor || (str1Index == maxStr1Index)) {
                            didFind = true;
                            if (NULL != result) *result = CFRangeMake(fromLoc, str1Index - fromLoc);
                        }
                        break;
                    }
                }
                
                if (fromLoc == toLoc) break;
                fromLoc += delta;
            }
        } else {
            while (1) {
                str1Index = fromLoc;
                str2Index = 0;
                
                while (str2Index < findStrLen) {
                    if (CFStringGetCharacterFromInlineBuffer(&inlineBuf1, str1Index) != CFStringGetCharacterFromInlineBuffer(&inlineBuf2, str2Index)) break;

                    ++str1Index; ++str2Index;
                }
                
                if (str2Index == findStrLen) {
                    didFind = true;
                    if (NULL != result) *result = CFRangeMake(fromLoc, findStrLen);
                    break;
                }
                
                if (fromLoc == toLoc) break;
                fromLoc += delta;
            }
        }
    }

    return didFind;
}


Boolean CFStringFindWithOptions(CFStringRef string, CFStringRef stringToFind, CFRange rangeToSearch, CFStringCompareFlags compareOptions, CFRange *result) { return CFStringFindWithOptionsAndLocale(string, stringToFind, rangeToSearch, compareOptions, NULL, result); }

// Functions to deal with special arrays of CFRange, CFDataRef, created by CFStringCreateArrayWithFindResults()

static const void *__rangeRetain(CFAllocatorRef allocator, const void *ptr) {
    CFRetain(*(CFDataRef *)((uint8_t *)ptr + sizeof(CFRange)));
    return ptr;
}

static void __rangeRelease(CFAllocatorRef allocator, const void *ptr) {
    CFRelease(*(CFDataRef *)((uint8_t *)ptr + sizeof(CFRange)));
}

static CFStringRef __rangeCopyDescription(const void *ptr) {
    CFRange range = *(CFRange *)ptr;
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("{%ld, %ld}"), (long)range.location, (long)range.length);
}

static Boolean	__rangeEqual(const void *ptr1, const void *ptr2) {
    CFRange range1 = *(CFRange *)ptr1;
    CFRange range2 = *(CFRange *)ptr2;
    return (range1.location == range2.location) && (range1.length == range2.length);
}


CFArrayRef CFStringCreateArrayWithFindResults(CFAllocatorRef alloc, CFStringRef string, CFStringRef stringToFind, CFRange rangeToSearch, CFStringCompareFlags compareOptions) {
    CFRange foundRange;
    Boolean backwards = ((compareOptions & kCFCompareBackwards) != 0);
    UInt32 endIndex = rangeToSearch.location + rangeToSearch.length;
    CFMutableDataRef rangeStorage = NULL;	// Basically an array of CFRange, CFDataRef (packed)
    uint8_t *rangeStorageBytes = NULL;
    CFIndex foundCount = 0;
    CFIndex capacity = 0;		// Number of CFRange, CFDataRef element slots in rangeStorage
    
    if (alloc == NULL) alloc = __CFGetDefaultAllocator();

    while ((rangeToSearch.length > 0) && CFStringFindWithOptions(string, stringToFind, rangeToSearch, compareOptions, &foundRange)) {
	// Determine the next range
        if (backwards) {
            rangeToSearch.length = foundRange.location - rangeToSearch.location;
        } else {
            rangeToSearch.location = foundRange.location + foundRange.length;
            rangeToSearch.length = endIndex - rangeToSearch.location;
        }

	// If necessary, grow the data and squirrel away the found range 
	if (foundCount >= capacity) {
	    if (rangeStorage == NULL) rangeStorage = CFDataCreateMutable(alloc, 0);
	    capacity = (capacity + 4) * 2;
	    CFDataSetLength(rangeStorage, capacity * (sizeof(CFRange) + sizeof(CFDataRef)));
	    rangeStorageBytes = (uint8_t *)CFDataGetMutableBytePtr(rangeStorage) + foundCount * (sizeof(CFRange) + sizeof(CFDataRef));
	}
	memmove(rangeStorageBytes, &foundRange, sizeof(CFRange));	// The range
	memmove(rangeStorageBytes + sizeof(CFRange), &rangeStorage, sizeof(CFDataRef));	// The data
	rangeStorageBytes += (sizeof(CFRange) + sizeof(CFDataRef));
	foundCount++;
    }

    if (foundCount > 0) {
	CFIndex cnt;
	CFMutableArrayRef array;
        const CFArrayCallBacks callbacks = {0, __rangeRetain, __rangeRelease, __rangeCopyDescription, __rangeEqual};

	CFDataSetLength(rangeStorage, foundCount * (sizeof(CFRange) + sizeof(CFDataRef)));	// Tighten storage up
	rangeStorageBytes = (uint8_t *)CFDataGetMutableBytePtr(rangeStorage);

        array = CFArrayCreateMutable(alloc, foundCount * sizeof(CFRange *), &callbacks);
	for (cnt = 0; cnt < foundCount; cnt++) {
	    // Each element points to the appropriate CFRange in the CFData
	    CFArrayAppendValue(array, rangeStorageBytes + cnt * (sizeof(CFRange) + sizeof(CFDataRef)));
	}
        CFRelease(rangeStorage);		// We want the data to go away when all CFRanges inside it are released...
        return array;
    } else {
        return NULL;
    }
}


CFRange CFStringFind(CFStringRef string, CFStringRef stringToFind, CFStringCompareFlags compareOptions) {
    CFRange foundRange;

    if (CFStringFindWithOptions(string, stringToFind, CFRangeMake(0, CFStringGetLength(string)), compareOptions, &foundRange)) {
        return foundRange;
    } else {
        return CFRangeMake(kCFNotFound, 0);
    }
}

Boolean CFStringHasPrefix(CFStringRef string, CFStringRef prefix) {
    return CFStringFindWithOptions(string, prefix, CFRangeMake(0, CFStringGetLength(string)), kCFCompareAnchored, NULL);
}

Boolean CFStringHasSuffix(CFStringRef string, CFStringRef suffix) {
    return CFStringFindWithOptions(string, suffix, CFRangeMake(0, CFStringGetLength(string)), kCFCompareAnchored|kCFCompareBackwards, NULL);
}

#define MAX_TRANSCODING_LENGTH 4

#define HANGUL_JONGSEONG_COUNT (28)

CF_INLINE bool _CFStringIsHangulLVT(UTF32Char character) {
    return (((character - HANGUL_SYLLABLE_START) % HANGUL_JONGSEONG_COUNT) ? true : false);
}

static uint8_t __CFTranscodingHintLength[] = {
    2, 3, 4, 4, 4, 4, 4, 2, 2, 2, 2, 4, 0, 0, 0, 0
};

enum {
    kCFStringHangulStateL,
    kCFStringHangulStateV,
    kCFStringHangulStateT,
    kCFStringHangulStateLV,
    kCFStringHangulStateLVT,
    kCFStringHangulStateBreak
};

static const CFCharacterSetInlineBuffer *__CFStringGetFitzpatrickModifierBaseCharacterSet(void) {
    static CFCharacterSetInlineBuffer buffer;
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ // based on UTR#51 1.0 (draft 7) for Unicode 8.0
        /*
         U+261D WHITE UP POINTING INDEX
         U+2639 WHITE FROWNING FACE…U+263A WHITE SMILING FACE
         U+270A RAISED FIST…U+270D WRITING HAND
         U+1F385 FATHER CHRISTMAS
         U+1F3C2 SNOWBOARDER…U+1F3C4 SURFER
         U+1F3C7 HORSE RACING
         U+1F3CA SWIMMER
         U+1F442 EAR…U+1F443 NOSE
         U+1F446 WHITE UP POINTING BACKHAND INDEX…U+1F450 OPEN HANDS SIGN
         U+1F466 BOY…U+1F469 WOMAN
         U+1F46E POLICE OFFICER…U+1F478 PRINCESS
         U+1F47C BABY ANGEL
         U+1F47F IMP
         U+1F481 INFORMATION DESK PERSON…U+1F482 GUARDSMAN
         U+1F483 DANCER
         U+1F485 NAIL POLISH
         U+1F486 FACE MASSAGE…U+1F487 HAIRCUT
         U+1F4AA FLEXED BICEPS
         U+1F590 RAISED HAND WITH FINGERS SPLAYED
         U+1F595 REVERSED HAND WITH MIDDLE FINGER EXTENDED…U+1F596 RAISED HAND WITH PART BETWEEN MIDDLE AND RING FINGERS
         U+1F600 GRINNING FACE…U+1F637 FACE WITH MEDICAL MASK
         U+1F641 SLIGHTLY FROWNING FACE…U+1F642 SLIGHTLY SMILING FACE
         U+1F645 FACE WITH NO GOOD GESTURE…U+1F647 PERSON BOWING DEEPLY
         U+1F64B HAPPY PERSON RAISING ONE HAND
         U+1F64C PERSON RAISING BOTH HANDS IN CELEBRATION
         U+1F64D PERSON FROWNING…U+1F64E PERSON WITH POUTING FACE
         U+1F64F PERSON WITH FOLDED HANDS
         U+1F6A3 ROWBOAT
         U+1F6B4 BICYCLIST…U+1F6B6 PEDESTRIAN
         U+1F6C0 BATH
         */
        CFMutableCharacterSetRef cset = CFCharacterSetCreateMutable(NULL);
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x261D, 1)); // WHITE UP POINTING INDEX
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x2639, 2)); // WHITE FROWNING FACE ~ WHITE SMILING FACE
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x270A, 4)); // RAISED FIST ~ WRITING HAND
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F385, 1)); // FATHER CHRISTMAS
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F3C2, 3)); // SNOWBOARDER ~ SURFER
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F3C7, 1)); // HORSE RACING
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F3CA, 1)); // SWIMMER
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F442, 2)); // EAR ~ NOSE
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F446, 0x1F451 - 0x1F446)); // WHITE UP POINTING BACKHAND INDEX ~ OPEN HANDS SIGN
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F466, 4)); // BOY ~ WOMAN
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F46E, 0x1F479 - 0x1F46E)); // POLICE OFFICER ~ PRINCESS
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F47C, 1)); // BABY ANGEL
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F47F, 1)); // IMP
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F481, 3)); // INFORMATION DESK PERSON ~ DANCER
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F485, 3)); // NAIL POLISH ~ HAIRCUT
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F4AA, 1)); // FLEXED BICEPS
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F590, 1)); // RAISED HAND WITH FINGERS SPLAYED
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F595, 2)); // REVERSED HAND WITH MIDDLE FINGER EXTENDED ~ RAISED HAND WITH PART BETWEEN MIDDLE AND RING FINGERS
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F600, 0x1F638 - 0x1F600)); // GRINNING FACE ~ FACE WITH MEDICAL MASK
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F641, 2)); // SLIGHTLY FROWNING FACE ~ SLIGHTLY SMILING FACE
        
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F645, 3)); // FACE WITH NO GOOD GESTURE ~ PERSON BOWING DEEPLY
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F64B, 0x1F650 - 0x1F64B)); // HAPPY PERSON RAISING ONE HAND ~ PERSON WITH FOLDED HANDS
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F6A3, 1)); // ROWBOAT
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F6B4, 0x1F6B7 - 0x1F6B4)); // BICYCLIST ~ PEDESTRIAN
        CFCharacterSetAddCharactersInRange(cset, CFRangeMake(0x1F6C0, 1)); // BATH
        CFCharacterSetCompact(cset);
        CFCharacterSetInitInlineBuffer(cset, &buffer);
    });

    return (const CFCharacterSetInlineBuffer *)&buffer;
}

static inline bool __CFStringIsFitzpatrickModifiers(UTF32Char character) { return ((character >= 0x1F3FB) && (character <= 0x1F3FF) ? true : false); }
static inline bool __CFStringIsBaseForFitzpatrickModifiers(UTF32Char character) {
    if (((character >= 0x2600) && (character < 0x27C0)) || ((character >= 0x1F300) && (character < 0x1F700))) { // Misc symbols, dingbats, & emoticons
        return (CFCharacterSetInlineBufferIsLongCharacterMember(__CFStringGetFitzpatrickModifierBaseCharacterSet(), character) ? true : false);
    }

    return false;
}

static inline bool __CFStringIsFamilySequenceBaseCharacterHigh(UTF16Char character) { return (character == 0xD83D) ? true : false; }
static inline bool __CFStringIsFamilySequenceBaseCharacterLow(UTF16Char character) { return (((character >= 0xDC66) && (character <= 0xDC69)) || (character == 0xDC8B) ? true : false); }
static inline bool __CFStringIsFamilySequenceCluster(CFStringInlineBuffer *buffer, CFRange range) {
    UTF16Char character = CFStringGetCharacterFromInlineBuffer(buffer, range.location);

    if (character == 0x2764) { // HEART
        return true;
    } else if (range.length > 1) {
        if (__CFStringIsFamilySequenceBaseCharacterHigh(character) && __CFStringIsFamilySequenceBaseCharacterLow(CFStringGetCharacterFromInlineBuffer(buffer, range.location + 1))) return true;
    }
    return false;
}

static CFRange _CFStringInlineBufferGetComposedRange(CFStringInlineBuffer *buffer, CFIndex start, CFStringCharacterClusterType type, const uint8_t *bmpBitmap, CFIndex csetType) {
    CFIndex end = start + 1;
    const uint8_t *bitmap = bmpBitmap;
    UTF32Char character;
    UTF16Char otherSurrogate;
    uint8_t step;

    character = CFStringGetCharacterFromInlineBuffer(buffer, start);

    // We don't combine characters in Armenian ~ Limbu range for backward deletion
    if ((type != kCFStringBackwardDeletionCluster) || (character < 0x0530) || (character > 0x194F)) {
        // Check if the current is surrogate
        if (CFUniCharIsSurrogateHighCharacter(character) && CFUniCharIsSurrogateLowCharacter((otherSurrogate = CFStringGetCharacterFromInlineBuffer(buffer, start + 1)))) {
            ++end;
            character = CFUniCharGetLongCharacterForSurrogatePair(character, otherSurrogate);
            bitmap = CFUniCharGetBitmapPtrForPlane(csetType, (character >> 16));
        }

        // Extend backward
        while (start > 0) {
            if ((type == kCFStringBackwardDeletionCluster) && (character >= 0x0530) && (character < 0x1950)) break;

            if (character < 0x10000) { // the first round could be already be non-BMP
                if (CFUniCharIsSurrogateLowCharacter(character) && CFUniCharIsSurrogateHighCharacter((otherSurrogate = CFStringGetCharacterFromInlineBuffer(buffer, start - 1)))) {
                    character = CFUniCharGetLongCharacterForSurrogatePair(otherSurrogate, character);
                    bitmap = CFUniCharGetBitmapPtrForPlane(csetType, (character >> 16));
                    if (--start == 0) break; // starting with non-BMP combining mark
                } else {
                    bitmap = bmpBitmap;
                }
            }

            if (__CFStringIsFitzpatrickModifiers(character) && (start > 0)) {
                UTF32Char baseCharacter = CFStringGetCharacterFromInlineBuffer(buffer, start - 1);

                if (CFUniCharIsSurrogateLowCharacter(baseCharacter) && ((start - 1) > 0)) {
                    UTF16Char otherCharacter = CFStringGetCharacterFromInlineBuffer(buffer, start - 2);

                    if (CFUniCharIsSurrogateHighCharacter(otherCharacter)) baseCharacter = CFUniCharGetLongCharacterForSurrogatePair(otherCharacter, baseCharacter);
                }

                if (!__CFStringIsBaseForFitzpatrickModifiers(baseCharacter)) break;
            } else {
                if (!CFUniCharIsMemberOfBitmap(character, bitmap) && (character != 0xFF9E) && (character != 0xFF9F) && ((character & 0x1FFFF0) != 0xF870)) break;
            }
    
            --start;
    
            character = CFStringGetCharacterFromInlineBuffer(buffer, start);
        }
    }

    // Hangul
    if (((character >= HANGUL_CHOSEONG_START) && (character <= HANGUL_JONGSEONG_END)) || ((character >= HANGUL_SYLLABLE_START) && (character <= HANGUL_SYLLABLE_END))) {
        uint8_t state;
        uint8_t initialState;

        if (character < HANGUL_JUNGSEONG_START) {
            state = kCFStringHangulStateL;
        } else if (character < HANGUL_JONGSEONG_START) {
            state = kCFStringHangulStateV;
        } else if (character < HANGUL_SYLLABLE_START) {
            state = kCFStringHangulStateT;
        } else {
            state = (_CFStringIsHangulLVT(character) ? kCFStringHangulStateLVT : kCFStringHangulStateLV);
        }
        initialState = state;

        // Extend backward
        while (((character = CFStringGetCharacterFromInlineBuffer(buffer, start - 1)) >= HANGUL_CHOSEONG_START) && (character <= HANGUL_SYLLABLE_END) && ((character <= HANGUL_JONGSEONG_END) || (character >= HANGUL_SYLLABLE_START))) {
            switch (state) {
            case kCFStringHangulStateV:
                if (character <= HANGUL_CHOSEONG_END) {
                    state = kCFStringHangulStateL;
                } else if ((character >= HANGUL_SYLLABLE_START) && (character <= HANGUL_SYLLABLE_END) && !_CFStringIsHangulLVT(character)) {
                    state = kCFStringHangulStateLV;
                } else if (character > HANGUL_JUNGSEONG_END) {
                    state = kCFStringHangulStateBreak;
                }
                break;

            case kCFStringHangulStateT:
                if ((character >= HANGUL_JUNGSEONG_START) && (character <= HANGUL_JUNGSEONG_END)) {
                    state = kCFStringHangulStateV;
                } else if ((character >= HANGUL_SYLLABLE_START) && (character <= HANGUL_SYLLABLE_END)) {
                    state = (_CFStringIsHangulLVT(character) ? kCFStringHangulStateLVT : kCFStringHangulStateLV);
                } else if (character < HANGUL_JUNGSEONG_START) {
                    state = kCFStringHangulStateBreak;
                }
                break;

            default:
                state = ((character < HANGUL_JUNGSEONG_START) ? kCFStringHangulStateL : kCFStringHangulStateBreak);
                break;
            }

            if (state == kCFStringHangulStateBreak) break;
            --start;
        }

        // Extend forward
        state = initialState;
        while (((character = CFStringGetCharacterFromInlineBuffer(buffer, end)) > 0) && (((character >= HANGUL_CHOSEONG_START) && (character <= HANGUL_JONGSEONG_END)) || ((character >= HANGUL_SYLLABLE_START) && (character <= HANGUL_SYLLABLE_END)))) {
            switch (state) {
            case kCFStringHangulStateLV:
            case kCFStringHangulStateV:
                if ((character >= HANGUL_JUNGSEONG_START) && (character <= HANGUL_JONGSEONG_END)) {
                    state = ((character < HANGUL_JONGSEONG_START) ? kCFStringHangulStateV : kCFStringHangulStateT);
                } else {
                    state = kCFStringHangulStateBreak;
                }
                break;

            case kCFStringHangulStateLVT:
            case kCFStringHangulStateT:
                state = (((character >= HANGUL_JONGSEONG_START) && (character <= HANGUL_JONGSEONG_END)) ? kCFStringHangulStateT : kCFStringHangulStateBreak);
                break;

            default:
                if (character < HANGUL_JUNGSEONG_START) {
                    state = kCFStringHangulStateL;
                } else if (character < HANGUL_JONGSEONG_START) {
                    state = kCFStringHangulStateV;
                } else if (character >= HANGUL_SYLLABLE_START) {
                    state = (_CFStringIsHangulLVT(character) ? kCFStringHangulStateLVT : kCFStringHangulStateLV);
                } else {
                    state = kCFStringHangulStateBreak;
                }
                break;
            }

            if (state == kCFStringHangulStateBreak) break;
            ++end;
        }
    }

    bool prevIsFitzpatrickBase = __CFStringIsBaseForFitzpatrickModifiers(character);

    // Extend forward
    while ((character = CFStringGetCharacterFromInlineBuffer(buffer, end)) > 0) {
        if ((type == kCFStringBackwardDeletionCluster) && (character >= 0x0530) && (character < 0x1950)) break;
    
        if (CFUniCharIsSurrogateHighCharacter(character) && CFUniCharIsSurrogateLowCharacter((otherSurrogate = CFStringGetCharacterFromInlineBuffer(buffer, end + 1)))) {
            character = CFUniCharGetLongCharacterForSurrogatePair(character, otherSurrogate);
            bitmap = CFUniCharGetBitmapPtrForPlane(csetType, (character >> 16));
            step = 2;
        } else {
            bitmap = bmpBitmap;
            step  = 1;
        }

        if ((!prevIsFitzpatrickBase || !__CFStringIsFitzpatrickModifiers(character)) && !CFUniCharIsMemberOfBitmap(character, bitmap) && (character != 0xFF9E) && (character != 0xFF9F) && ((character & 0x1FFFF0) != 0xF870)) break;

        prevIsFitzpatrickBase = __CFStringIsBaseForFitzpatrickModifiers(character);

        end += step;
    } 

    return CFRangeMake(start, end - start);
}

CF_INLINE bool _CFStringIsVirama(UTF32Char character, const uint8_t *combClassBMP) {
    return ((character == COMBINING_GRAPHEME_JOINER) || (CFUniCharGetCombiningPropertyForCharacter(character, (const uint8_t *)((character < 0x10000) ? combClassBMP : CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (character >> 16)))) == 9) ? true : false);
}

CFRange CFStringGetRangeOfCharacterClusterAtIndex(CFStringRef string, CFIndex charIndex, CFStringCharacterClusterType type) {
    CFRange range;
    CFIndex currentIndex;
    CFIndex length = CFStringGetLength(string);
    CFIndex csetType = ((kCFStringGraphemeCluster == type) ? kCFUniCharGraphemeExtendCharacterSet : kCFUniCharNonBaseCharacterSet);
    CFStringInlineBuffer stringBuffer;
    const uint8_t *bmpBitmap;
    const uint8_t *letterBMP;
    static const uint8_t *combClassBMP = NULL;
    UTF32Char character;
    UTF16Char otherSurrogate;

    if (charIndex >= length) return CFRangeMake(kCFNotFound, 0);

    /* Fast case.  If we're eight-bit, it's either the default encoding is cheap or the content is all ASCII.  Watch out when (or if) adding more 8bit Mac-scripts in CFStringEncodingConverters
    */
    if (!CF_IS_OBJC(__kCFStringTypeID, string) && __CFStrIsEightBit(string)) return CFRangeMake(charIndex, 1);

    bmpBitmap = CFUniCharGetBitmapPtrForPlane(csetType, 0);
    letterBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharLetterCharacterSet, 0);
    if (NULL == combClassBMP) combClassBMP = (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, 0);

    CFStringInitInlineBuffer(string, &stringBuffer, CFRangeMake(0, length));

    // Get composed character sequence first
    range = _CFStringInlineBufferGetComposedRange(&stringBuffer, charIndex, type, bmpBitmap, csetType);

    // Do grapheme joiners
    if (type < kCFStringCursorMovementCluster) {
        const uint8_t *letter = letterBMP;

        // Check to see if we have a letter at the beginning of initial cluster
        character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, range.location);

        if ((range.length > 1) && CFUniCharIsSurrogateHighCharacter(character) && CFUniCharIsSurrogateLowCharacter((otherSurrogate = CFStringGetCharacterFromInlineBuffer(&stringBuffer, range.location + 1)))) {
            character = CFUniCharGetLongCharacterForSurrogatePair(character, otherSurrogate);
            letter = CFUniCharGetBitmapPtrForPlane(kCFUniCharLetterCharacterSet, (character >> 16));
        }

        if ((character == ZERO_WIDTH_JOINER) || CFUniCharIsMemberOfBitmap(character, letter)) {
            CFRange otherRange;

            // Check if preceded by grapheme joiners (U034F and viramas)
            otherRange.location = currentIndex = range.location;

            while (currentIndex > 1) {
                character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, --currentIndex);
    
                // ??? We're assuming viramas only in BMP
                if ((_CFStringIsVirama(character, combClassBMP) || ((character == ZERO_WIDTH_JOINER) && _CFStringIsVirama(CFStringGetCharacterFromInlineBuffer(&stringBuffer, --currentIndex), combClassBMP))) && (currentIndex > 0)) {
                    --currentIndex;                
                } else {
                    break;
                }

                currentIndex = _CFStringInlineBufferGetComposedRange(&stringBuffer, currentIndex, type, bmpBitmap, csetType).location;
    
                character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex);
    
                if (CFUniCharIsSurrogateLowCharacter(character) && CFUniCharIsSurrogateHighCharacter((otherSurrogate = CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex - 1)))) {
                    character = CFUniCharGetLongCharacterForSurrogatePair(character, otherSurrogate);
                    letter = CFUniCharGetBitmapPtrForPlane(kCFUniCharLetterCharacterSet, (character >> 16));
                    --currentIndex;
                } else {
                    letter = letterBMP;
                }

                if (!CFUniCharIsMemberOfBitmap(character, letter)) break;
                range.location = currentIndex;
            }

            range.length += otherRange.location - range.location;

            // Check if followed by grapheme joiners
            if ((range.length > 1) && ((range.location + range.length) < length)) {
                otherRange = range;
                currentIndex = otherRange.location + otherRange.length;

                do {
                    character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex - 1);

                    // ??? We're assuming viramas only in BMP
                    if ((character != ZERO_WIDTH_JOINER) && !_CFStringIsVirama(character, combClassBMP)) break;

                    character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex);

                    if (character == ZERO_WIDTH_JOINER) character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, ++currentIndex);

                    if (CFUniCharIsSurrogateHighCharacter(character) && CFUniCharIsSurrogateLowCharacter((otherSurrogate = CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex + 1)))) {
                        character = CFUniCharGetLongCharacterForSurrogatePair(character, otherSurrogate);
                        letter = CFUniCharGetBitmapPtrForPlane(kCFUniCharLetterCharacterSet, (character >> 16));
                    } else {
                        letter = letterBMP;
                    }
        
                    // We only conjoin letters
                    if (!CFUniCharIsMemberOfBitmap(character, letter)) break;
                    otherRange = _CFStringInlineBufferGetComposedRange(&stringBuffer, currentIndex, type, bmpBitmap, csetType);
                    currentIndex = otherRange.location + otherRange.length;
                } while ((otherRange.location + otherRange.length) < length);
                range.length = currentIndex - range.location;
            }
        }
    }

    // Check if we're part of prefix transcoding hints
    CFIndex otherIndex;
    
    currentIndex = (range.location + range.length) - (MAX_TRANSCODING_LENGTH + 1);
    if (currentIndex < 0) currentIndex = 0;
    
    while (currentIndex <= range.location) {
        character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex);
        
        if ((character & 0x1FFFF0) == 0xF860) { // transcoding hint
            otherIndex = currentIndex + __CFTranscodingHintLength[(character - 0xF860)] + 1;
            if (otherIndex >= (range.location + range.length)) {
                if (otherIndex <= length) {
                    range.location = currentIndex;
                    range.length = otherIndex - currentIndex;
                }
                break;
            }
        }
        ++currentIndex;
    }

    // Family face sequence
    CFRange aCluster;

    if (range.location > 1) { // there are more than 2 chars
        character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, range.location);

        if (__CFStringIsFamilySequenceCluster(&stringBuffer, range) || (character == ZERO_WIDTH_JOINER)) { // extend backward
            currentIndex = (character == ZERO_WIDTH_JOINER) ? range.location + 1 : range.location;

            while ((currentIndex > 1) && (ZERO_WIDTH_JOINER == CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex - 1))) {
                aCluster = _CFStringInlineBufferGetComposedRange(&stringBuffer, currentIndex - 2, type, bmpBitmap, csetType);

                if (__CFStringIsFamilySequenceCluster(&stringBuffer, aCluster) && (aCluster.location < range.location)) {
                    currentIndex = aCluster.location;
                } else {
                    break;
                }
            }

            if (currentIndex < range.location) {
                range.length += range.location - currentIndex;
                range.location = currentIndex;
            }
        }
    }

    // Extend forward
    if (range.location + range.length < length) {
        currentIndex = range.location + range.length - 1;
        character = CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex);

        if ((ZERO_WIDTH_JOINER == character) || __CFStringIsFamilySequenceCluster(&stringBuffer, _CFStringInlineBufferGetComposedRange(&stringBuffer, currentIndex, type, bmpBitmap, csetType))) {

            if (ZERO_WIDTH_JOINER != character) ++currentIndex; // move to the end of cluster

            while (((currentIndex + 1) < length) && (ZERO_WIDTH_JOINER == CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex))) {
                aCluster = _CFStringInlineBufferGetComposedRange(&stringBuffer, currentIndex + 1, type, bmpBitmap, csetType);
                if (__CFStringIsFamilySequenceCluster(&stringBuffer, aCluster)) {
                    currentIndex = aCluster.location + aCluster.length;
                    if ((aCluster.length > 1) && (ZERO_WIDTH_JOINER == CFStringGetCharacterFromInlineBuffer(&stringBuffer, currentIndex - 1))) --currentIndex;
                } else {
                    break;
                }
            }
            if (currentIndex > (range.location + range.length)) range.length = currentIndex - range.location;
        }
    }
    return range;
}

CFRange CFStringGetRangeOfComposedCharactersAtIndex(CFStringRef theString, CFIndex theIndex) {
    return CFStringGetRangeOfCharacterClusterAtIndex(theString, theIndex, kCFStringComposedCharacterCluster);
}

/*!
	@function CFStringFindCharacterFromSet
	Query the range of characters contained in the specified character set.
	@param theString The CFString which is to be searched.  If this
                		parameter is not a valid CFString, the behavior is
              		undefined.
	@param theSet The CFCharacterSet against which the membership
			of characters is checked.  If this parameter is not a valid
			CFCharacterSet, the behavior is undefined.
	@param range The range of characters within the string to search. If
			the range location or end point (defined by the location
			plus length minus 1) are outside the index space of the
			string (0 to N-1 inclusive, where N is the length of the
			string), the behavior is undefined. If the range length is
			negative, the behavior is undefined. The range may be empty
			(length 0), in which case no search is performed.
	@param searchOptions The bitwise-or'ed option flags to control
			the search behavior.  The supported options are
			kCFCompareBackwards andkCFCompareAnchored.
			If other option flags are specified, the behavior
                        is undefined.
	@param result The pointer to a CFRange supplied by the caller in
			which the search result is stored.  If a pointer to an invalid
			memory is specified, the behavior is undefined.
	@result true, if at least a character which is a member of the character
			set is found and result is filled, otherwise, false.
*/
#define SURROGATE_START 0xD800
#define SURROGATE_END 0xDFFF

CF_EXPORT Boolean CFStringFindCharacterFromSet(CFStringRef theString, CFCharacterSetRef theSet, CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFRange *result) {
    CFStringInlineBuffer stringBuffer;
    CFCharacterSetInlineBuffer csetBuffer;
    UniChar ch;
    CFIndex step;
    CFIndex fromLoc, toLoc, cnt;	// fromLoc and toLoc are inclusive
    Boolean found = false;
    Boolean done = false;

//#warning FIX ME !! Should support kCFCompareNonliteral

    if ((rangeToSearch.location + rangeToSearch.length > CFStringGetLength(theString)) || (rangeToSearch.length == 0)) return false;

    if (searchOptions & kCFCompareBackwards) {
        fromLoc = rangeToSearch.location + rangeToSearch.length - 1;
        toLoc = rangeToSearch.location;
    } else {       
        fromLoc = rangeToSearch.location;
        toLoc = rangeToSearch.location + rangeToSearch.length - 1;
    }
    if (searchOptions & kCFCompareAnchored) {
	toLoc = fromLoc;
    }

    step = (fromLoc <= toLoc) ? 1 : -1;
    cnt = fromLoc;
    
    CFStringInitInlineBuffer(theString, &stringBuffer, rangeToSearch);
    CFCharacterSetInitInlineBuffer(theSet, &csetBuffer);

    do {
	ch = CFStringGetCharacterFromInlineBuffer(&stringBuffer, cnt - rangeToSearch.location);
        if ((ch >= SURROGATE_START) && (ch <= SURROGATE_END)) {
            int otherCharIndex = cnt + step;

            if (((step < 0) && (otherCharIndex < toLoc)) || ((step > 0) && (otherCharIndex > toLoc))) {
                done = true;
            } else {
                UniChar highChar;
                UniChar lowChar = CFStringGetCharacterFromInlineBuffer(&stringBuffer, otherCharIndex - rangeToSearch.location);

                if (cnt < otherCharIndex) {
                    highChar = ch;
                } else {
                    highChar = lowChar;
                    lowChar = ch;
                }

                if (CFUniCharIsSurrogateHighCharacter(highChar) && CFUniCharIsSurrogateLowCharacter(lowChar) && CFCharacterSetInlineBufferIsLongCharacterMember(&csetBuffer, CFUniCharGetLongCharacterForSurrogatePair(highChar, lowChar))) {
                    if (result) *result = CFRangeMake((cnt < otherCharIndex ? cnt : otherCharIndex), 2);
                    return true;
                } else if (otherCharIndex == toLoc) {
                    done = true;
                } else {
                    cnt = otherCharIndex + step;
                }
            }
        } else if (CFCharacterSetInlineBufferIsLongCharacterMember(&csetBuffer, ch)) {
	    done = found = true;
        } else if (cnt == toLoc) {
            done = true;
        } else {
            cnt += step;
        }
    } while (!done);

    if (found && result) *result = CFRangeMake(cnt, 1);
    return found;
}

/* Line range code */

#define CarriageReturn '\r'	/* 0x0d */
#define NewLine '\n'		/* 0x0a */
#define NextLine 0x0085
#define LineSeparator 0x2028
#define ParaSeparator 0x2029

CF_INLINE Boolean isALineSeparatorTypeCharacter(UniChar ch, Boolean includeLineEndings) {
    if (ch > CarriageReturn && ch < NextLine) return false;	/* Quick test to cover most chars */
    return (ch == NewLine || ch == CarriageReturn || ch == ParaSeparator || (includeLineEndings && (ch == NextLine || ch == LineSeparator))) ? true : false;
}

static void __CFStringGetLineOrParagraphBounds(CFStringRef string, CFRange range, CFIndex *lineBeginIndex, CFIndex *lineEndIndex, CFIndex *contentsEndIndex, Boolean includeLineEndings) {
    CFIndex len;
    CFStringInlineBuffer buf;
    UniChar ch;

    __CFAssertIsString(string);
    __CFAssertRangeIsInStringBounds(string, range.location, range.length);

    len = __CFStrLength(string);

    if (lineBeginIndex) {
        CFIndex start;
        if (range.location == 0) {
            start = 0;
        } else {
            CFStringInitInlineBuffer(string, &buf, CFRangeMake(0, len));
	    CFIndex buf_idx = range.location;

            /* Take care of the special case where start happens to fall right between \r and \n */
            ch = CFStringGetCharacterFromInlineBuffer(&buf, buf_idx);
            buf_idx--;
            if ((ch == NewLine) && (CFStringGetCharacterFromInlineBuffer(&buf, buf_idx) == CarriageReturn)) {
                buf_idx--;
            }
            while (1) {
                if (buf_idx < 0) {
                    start = 0;
                    break;
                } else if (isALineSeparatorTypeCharacter(CFStringGetCharacterFromInlineBuffer(&buf, buf_idx), includeLineEndings)) {
                    start = buf_idx + 1;
                    break;
                } else {
                    buf_idx--;
                }
            }
        }
        *lineBeginIndex = start;
    }

    /* Now find the ending point */
    if (lineEndIndex || contentsEndIndex) {
        CFIndex endOfContents, lineSeparatorLength = 1;	/* 1 by default */
        CFStringInitInlineBuffer(string, &buf, CFRangeMake(0, len));
	CFIndex buf_idx = range.location + range.length - (range.length ? 1 : 0);
        /* First look at the last char in the range (if the range is zero length, the char after the range) to see if we're already on or within a end of line sequence... */
        ch = __CFStringGetCharacterFromInlineBufferAux(&buf, buf_idx);
        if (ch == NewLine) {
            endOfContents = buf_idx;
            buf_idx--;
            if (__CFStringGetCharacterFromInlineBufferAux(&buf, buf_idx) == CarriageReturn) {
                lineSeparatorLength = 2;
                endOfContents--;
            }
        } else {
            while (1) {
                if (isALineSeparatorTypeCharacter(ch, includeLineEndings)) {
                    endOfContents = buf_idx;	/* This is actually end of contentsRange */
                    buf_idx++;	/* OK for this to go past the end */
                    if ((ch == CarriageReturn) && (__CFStringGetCharacterFromInlineBufferAux(&buf, buf_idx) == NewLine)) {
                        lineSeparatorLength = 2;
                    }
                    break;
                } else if (buf_idx >= len) {
                    endOfContents = len;
                    lineSeparatorLength = 0;
                    break;
                } else {
                    buf_idx++;
                    ch = __CFStringGetCharacterFromInlineBufferAux(&buf, buf_idx);
                }
            }
        }
        if (contentsEndIndex) *contentsEndIndex = endOfContents;
        if (lineEndIndex) *lineEndIndex = endOfContents + lineSeparatorLength;
    }
}

void CFStringGetLineBounds(CFStringRef string, CFRange range, CFIndex *lineBeginIndex, CFIndex *lineEndIndex, CFIndex *contentsEndIndex) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSString *)string, getLineStart:(NSUInteger *)lineBeginIndex end:(NSUInteger *)lineEndIndex contentsEnd:(NSUInteger *)contentsEndIndex forRange:NSMakeRange(range.location, range.length));
    __CFStringGetLineOrParagraphBounds(string, range, lineBeginIndex, lineEndIndex, contentsEndIndex, true);
}

void CFStringGetParagraphBounds(CFStringRef string, CFRange range, CFIndex *parBeginIndex, CFIndex *parEndIndex, CFIndex *contentsEndIndex) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSString *)string, getParagraphStart:(NSUInteger *)parBeginIndex end:(NSUInteger *)parEndIndex contentsEnd:(NSUInteger *)contentsEndIndex forRange:NSMakeRange(range.location, range.length));
    __CFStringGetLineOrParagraphBounds(string, range, parBeginIndex, parEndIndex, contentsEndIndex, false);
}


CFStringRef CFStringCreateByCombiningStrings(CFAllocatorRef alloc, CFArrayRef array, CFStringRef separatorString) {
    CFIndex numChars;
    CFIndex separatorNumByte;
    CFIndex stringCount = CFArrayGetCount(array);
    Boolean isSepCFString = !CF_IS_OBJC(__kCFStringTypeID, separatorString); 
    Boolean canBeEightbit = isSepCFString && __CFStrIsEightBit(separatorString);
    CFIndex idx;
    CFStringRef otherString;
    void *buffer;
    uint8_t *bufPtr;
    const void *separatorContents = NULL;

    if (stringCount == 0) {
        return CFStringCreateWithCharacters(alloc, NULL, 0);
    } else if (stringCount == 1) {
        return (CFStringRef)CFStringCreateCopy(alloc, (CFStringRef)CFArrayGetValueAtIndex(array, 0));
    }

    if (alloc == NULL) alloc = __CFGetDefaultAllocator();

    numChars = CFStringGetLength(separatorString) * (stringCount - 1);
    for (idx = 0; idx < stringCount; idx++) {
        otherString = (CFStringRef)CFArrayGetValueAtIndex(array, idx);
        numChars += CFStringGetLength(otherString);
	// canBeEightbit is already false if the separator is an NSString...
        if (CF_IS_OBJC(__kCFStringTypeID, otherString) || ! __CFStrIsEightBit(otherString)) canBeEightbit = false;
    }

    buffer = (uint8_t *)CFAllocatorAllocate(alloc, canBeEightbit ? ((numChars + 1) * sizeof(uint8_t)) : (numChars * sizeof(UniChar)), 0);
	bufPtr = (uint8_t *)buffer;
    if (__CFOASafe) __CFSetLastAllocationEventName(buffer, "CFString (store)");
    separatorNumByte = CFStringGetLength(separatorString) * (canBeEightbit ? sizeof(uint8_t) : sizeof(UniChar));

    for (idx = 0; idx < stringCount; idx++) {
        if (idx) { // add separator here unless first string
            if (separatorContents) {
                memmove(bufPtr, separatorContents, separatorNumByte);
            } else {
                if (!isSepCFString) { // NSString
                    CFStringGetCharacters(separatorString, CFRangeMake(0, CFStringGetLength(separatorString)), (UniChar *)bufPtr);
                } else if (canBeEightbit) {
                    memmove(bufPtr, (const uint8_t *)__CFStrContents(separatorString) + __CFStrSkipAnyLengthByte(separatorString), separatorNumByte);
                } else {	
                    __CFStrConvertBytesToUnicode((uint8_t *)__CFStrContents(separatorString) + __CFStrSkipAnyLengthByte(separatorString), (UniChar *)bufPtr, __CFStrLength(separatorString));
                }
                separatorContents = bufPtr;
            }
            bufPtr += separatorNumByte;
        }

        otherString = (CFStringRef )CFArrayGetValueAtIndex(array, idx);
        if (CF_IS_OBJC(__kCFStringTypeID, otherString)) {
            CFIndex otherLength = CFStringGetLength(otherString);
            CFStringGetCharacters(otherString, CFRangeMake(0, otherLength), (UniChar *)bufPtr);
            bufPtr += otherLength * sizeof(UniChar);
        } else {
            const uint8_t * otherContents = (const uint8_t *)__CFStrContents(otherString);
            CFIndex otherNumByte = __CFStrLength2(otherString, otherContents) * (canBeEightbit ? sizeof(uint8_t) : sizeof(UniChar));

            if (canBeEightbit || __CFStrIsUnicode(otherString)) {
                memmove(bufPtr, otherContents + __CFStrSkipAnyLengthByte(otherString), otherNumByte);
            } else {
                __CFStrConvertBytesToUnicode(otherContents + __CFStrSkipAnyLengthByte(otherString), (UniChar *)bufPtr, __CFStrLength2(otherString, otherContents));
            }
            bufPtr += otherNumByte;
        }
    }
    if (canBeEightbit) *bufPtr = 0; // NULL byte;

    return canBeEightbit ? 
		CFStringCreateWithCStringNoCopy(alloc, (const char*)buffer, __CFStringGetEightBitStringEncoding(), alloc) : 
		CFStringCreateWithCharactersNoCopy(alloc, (UniChar *)buffer, numChars, alloc);
}


CFArrayRef CFStringCreateArrayBySeparatingStrings(CFAllocatorRef alloc, CFStringRef string, CFStringRef separatorString) {
    CFArrayRef separatorRanges;
    CFIndex length = CFStringGetLength(string);
    /* No objc dispatch needed here since CFStringCreateArrayWithFindResults() works with both CFString and NSString */
    if (!(separatorRanges = CFStringCreateArrayWithFindResults(alloc, string, separatorString, CFRangeMake(0, length), 0))) {
        return CFArrayCreate(alloc, (const void **)&string, 1, & kCFTypeArrayCallBacks);
    } else {
        CFIndex idx;
        CFIndex count = CFArrayGetCount(separatorRanges);
        CFIndex startIndex = 0;
        CFIndex numChars;
        CFMutableArrayRef array = CFArrayCreateMutable(alloc, count + 2, & kCFTypeArrayCallBacks);
        const CFRange *currentRange;
        CFStringRef substring;

        for (idx = 0;idx < count;idx++) {
            currentRange = (const CFRange *)CFArrayGetValueAtIndex(separatorRanges, idx);
            numChars = currentRange->location - startIndex;
            substring = CFStringCreateWithSubstring(alloc, string, CFRangeMake(startIndex, numChars));
            CFArrayAppendValue(array, substring);
            CFRelease(substring);
            startIndex = currentRange->location + currentRange->length;
        }
        substring = CFStringCreateWithSubstring(alloc, string, CFRangeMake(startIndex, length - startIndex));
        CFArrayAppendValue(array, substring);
        CFRelease(substring);

	CFRelease(separatorRanges);
        
        return array;
    }
}

CFStringRef CFStringCreateFromExternalRepresentation(CFAllocatorRef alloc, CFDataRef data, CFStringEncoding encoding) {
    return CFStringCreateWithBytes(alloc, CFDataGetBytePtr(data), CFDataGetLength(data), encoding, true);
}


CFDataRef CFStringCreateExternalRepresentation(CFAllocatorRef alloc, CFStringRef string, CFStringEncoding encoding, uint8_t lossByte) {
    CFIndex length;
    CFIndex guessedByteLength;
    uint8_t *bytes;
    CFIndex usedLength;
    SInt32 result;

    if (CF_IS_OBJC(__kCFStringTypeID, string)) {	/* ??? Hope the compiler optimizes this away if OBJC_MAPPINGS is not on */
	length = CFStringGetLength(string);
    } else {
        __CFAssertIsString(string);
        length = __CFStrLength(string);
        if (__CFStrIsEightBit(string) && ((__CFStringGetEightBitStringEncoding() == encoding) || (__CFStringGetEightBitStringEncoding() == kCFStringEncodingASCII && __CFStringEncodingIsSupersetOfASCII(encoding)))) {	// Requested encoding is equal to the encoding in string
            return CFDataCreate(alloc, ((uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string)), __CFStrLength(string));
        }
    }

    if (alloc == NULL) alloc = __CFGetDefaultAllocator();

    if (((encoding & 0x0FFF) == kCFStringEncodingUnicode) && ((encoding == kCFStringEncodingUnicode) || ((encoding > kCFStringEncodingUTF8) && (encoding <= kCFStringEncodingUTF32LE)))) {
        guessedByteLength = (length + 1) * ((((encoding >> 26)  & 2) == 0) ? sizeof(UTF16Char) : sizeof(UTF32Char)); // UTF32 format has the bit set
    } else if (((guessedByteLength = CFStringGetMaximumSizeForEncoding(length, encoding)) > length) && !CF_IS_OBJC(__kCFStringTypeID, string)) { // Multi byte encoding
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
        if (__CFStrIsUnicode(string)) {
            CFIndex aLength = CFStringEncodingByteLengthForCharacters(encoding, kCFStringEncodingPrependBOM, __CFStrContents(string), __CFStrLength(string));
            if (aLength > 0) guessedByteLength = aLength;
        } else {
#endif
        result = __CFStringEncodeByteStream(string, 0, length, true, encoding, lossByte, NULL, LONG_MAX, &guessedByteLength);
	// if result == length, we always succeed
	//   otherwise, if result == 0, we fail
	//   otherwise, if there was a lossByte but still result != length, we fail
        if ((result != length) && (!result || !lossByte)) return NULL;
        if (guessedByteLength == length && __CFStrIsEightBit(string) && __CFStringEncodingIsSupersetOfASCII(encoding)) { // It's all ASCII !!
            return CFDataCreate(alloc, ((uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string)), __CFStrLength(string));
        }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
        }
#endif
    }
    bytes = (uint8_t *)CFAllocatorAllocate(alloc, guessedByteLength, 0);
    if (__CFOASafe) __CFSetLastAllocationEventName(bytes, "CFData (store)");

    result = __CFStringEncodeByteStream(string, 0, length, true, encoding, lossByte, bytes, guessedByteLength, &usedLength);

    if ((result != length) && (!result || !lossByte)) {		// see comment above about what this means
        CFAllocatorDeallocate(alloc, bytes);
        return NULL;
    }

    return CFDataCreateWithBytesNoCopy(alloc, (uint8_t *)bytes, usedLength, alloc);
}


CFStringEncoding CFStringGetSmallestEncoding(CFStringRef str) {
    CFIndex len;
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, CFStringEncoding, (NSString *)str, _smallestEncodingInCFStringEncoding);
    __CFAssertIsString(str);

    if (__CFStrIsEightBit(str)) return __CFStringGetEightBitStringEncoding();
    len = __CFStrLength(str);
    if (__CFStringEncodeByteStream(str, 0, len, false, __CFStringGetEightBitStringEncoding(), 0, NULL, LONG_MAX, NULL) == len) return __CFStringGetEightBitStringEncoding();
    if ((__CFStringGetEightBitStringEncoding() != __CFStringGetSystemEncoding()) && (__CFStringEncodeByteStream(str, 0, len, false, __CFStringGetSystemEncoding(), 0, NULL, LONG_MAX, NULL) == len)) return __CFStringGetSystemEncoding();
    return kCFStringEncodingUnicode;	/* ??? */
}


CFStringEncoding CFStringGetFastestEncoding(CFStringRef str) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, CFStringEncoding, (NSString *)str, _fastestEncodingInCFStringEncoding);
    __CFAssertIsString(str);
    return __CFStrIsEightBit(str) ? __CFStringGetEightBitStringEncoding() : kCFStringEncodingUnicode;	/* ??? */
}


SInt32 CFStringGetIntValue(CFStringRef str) {
    Boolean success;
    SInt32 result;
    SInt32 idx = 0;
    CFStringInlineBuffer buf;
    CFStringInitInlineBuffer(str, &buf, CFRangeMake(0, CFStringGetLength(str)));
    success = __CFStringScanInteger(&buf, NULL, &idx, false, &result);
    return success ? result : 0;
}


double CFStringGetDoubleValue(CFStringRef str) {
    Boolean success;
    double result;
    SInt32 idx = 0;
    CFStringInlineBuffer buf;
    CFStringInitInlineBuffer(str, &buf, CFRangeMake(0, CFStringGetLength(str)));
    success = __CFStringScanDouble(&buf, NULL, &idx, &result);
    return success ? result : 0.0;
}


/*** Mutable functions... ***/

void CFStringSetExternalCharactersNoCopy(CFMutableStringRef string, UniChar *chars, CFIndex length, CFIndex capacity) {
    __CFAssertIsNotNegative(length);
    __CFAssertIsStringAndExternalMutable(string);
    CFAssert4((length <= capacity) && ((capacity == 0) || ((capacity > 0) && chars)), __kCFLogAssertion, "%s(): Invalid args: characters %p length %d capacity %d", __PRETTY_FUNCTION__, chars, length, capacity);
    __CFStrSetContentPtr(string, chars);
    __CFStrSetExplicitLength(string, length);
    __CFStrSetCapacity(string, capacity * sizeof(UniChar));
    __CFStrSetCapacityProvidedExternally(string);
}



void CFStringInsert(CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, insertString:(NSString *)insertedStr atIndex:(NSUInteger)idx);
    __CFAssertIsStringAndMutable(str);
    CFAssert3(idx >= 0 && idx <= __CFStrLength(str), __kCFLogAssertion, "%s(): string index %d out of bounds (length %d)", __PRETTY_FUNCTION__, idx, __CFStrLength(str));
    __CFStringReplace(str, CFRangeMake(idx, 0), insertedStr);
}


void CFStringDelete(CFMutableStringRef str, CFRange range) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, deleteCharactersInRange:NSMakeRange(range.location, range.length));
    __CFAssertIsStringAndMutable(str);
    __CFAssertRangeIsInStringBounds(str, range.location, range.length);
    __CFStringChangeSize(str, range, 0, false);
}


void CFStringReplace(CFMutableStringRef str, CFRange range, CFStringRef replacement) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, replaceCharactersInRange:NSMakeRange(range.location, range.length) withString:(NSString *)replacement);
    __CFAssertIsStringAndMutable(str);
    __CFAssertRangeIsInStringBounds(str, range.location, range.length);
    __CFStringReplace(str, range, replacement);
}


void CFStringReplaceAll(CFMutableStringRef str, CFStringRef replacement) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, setString:(NSString *)replacement);
    __CFAssertIsStringAndMutable(str);
    __CFStringReplace(str, CFRangeMake(0, __CFStrLength(str)), replacement);
}


void CFStringAppend(CFMutableStringRef str, CFStringRef appended) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, appendString:(NSString *)appended);
    __CFAssertIsStringAndMutable(str);
    __CFStringReplace(str, CFRangeMake(__CFStrLength(str), 0), appended);
}


void CFStringAppendCharacters(CFMutableStringRef str, const UniChar *chars, CFIndex appendedLength) {
    CFIndex strLength, idx;

    __CFAssertIsNotNegative(appendedLength);

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, appendCharacters:chars length:(NSUInteger)appendedLength);

    __CFAssertIsStringAndMutable(str);

    strLength = __CFStrLength(str);
    if (__CFStrIsUnicode(str)) {
	__CFStringChangeSize(str, CFRangeMake(strLength, 0), appendedLength, true);
	memmove((UniChar *)__CFStrContents(str) + strLength, chars, appendedLength * sizeof(UniChar));
    } else {
	uint8_t *contents;
	bool isASCII = true;
	for (idx = 0; isASCII && idx < appendedLength; idx++) isASCII = (chars[idx] < 0x80);
	__CFStringChangeSize(str, CFRangeMake(strLength, 0), appendedLength, !isASCII);
	if (!isASCII) {
	    memmove((UniChar *)__CFStrContents(str) + strLength, chars, appendedLength * sizeof(UniChar));
	} else {
	    contents = (uint8_t *)__CFStrContents(str) + strLength + __CFStrSkipAnyLengthByte(str);
	    for (idx = 0; idx < appendedLength; idx++) contents[idx] = (uint8_t)chars[idx];
	}
    }
}


void __CFStringAppendBytes(CFMutableStringRef str, const char *cStr, CFIndex appendedLength, CFStringEncoding encoding) {
    Boolean appendedIsUnicode = false;
    Boolean freeCStrWhenDone = false;
    Boolean demoteAppendedUnicode = false;
    CFVarWidthCharBuffer vBuf;

    __CFAssertIsNotNegative(appendedLength);

    if (encoding == kCFStringEncodingASCII || encoding == __CFStringGetEightBitStringEncoding()) {
	// appendedLength now denotes length in UniChars
    } else if (encoding == kCFStringEncodingUnicode) {
	UniChar *chars = (UniChar *)cStr;
	CFIndex idx, length = appendedLength / sizeof(UniChar);
	bool isASCII = true;
	for (idx = 0; isASCII && idx < length; idx++) isASCII = (chars[idx] < 0x80);
	if (!isASCII) {
	    appendedIsUnicode = true;
	} else {
	    demoteAppendedUnicode = true;
	}
	appendedLength = length;
    } else {
        Boolean usingPassedInMemory = false;

	vBuf.allocator = __CFGetDefaultAllocator();	// We don't want to use client's allocator for temp stuff
        vBuf.chars.unicode = NULL;	// This will cause the decode function to allocate memory if necessary

        if (!__CFStringDecodeByteStream3((const uint8_t *)cStr, appendedLength, encoding, __CFStrIsUnicode(str), &vBuf, &usingPassedInMemory, 0)) {
	    CFAssert1(0, __kCFLogAssertion, "Supplied bytes could not be converted specified encoding %d", encoding);
	    return;
	}

	// If not ASCII, appendedLength now denotes length in UniChars
	appendedLength = vBuf.numChars;
	appendedIsUnicode = !vBuf.isASCII;
	cStr = (const char *)vBuf.chars.ascii;
	freeCStrWhenDone = !usingPassedInMemory && vBuf.shouldFreeChars;
    }

    if (CF_IS_OBJC(__kCFStringTypeID, str)) {
	if (!appendedIsUnicode && !demoteAppendedUnicode) {
	    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, _cfAppendCString:(const unsigned char *)cStr length:(NSInteger)appendedLength);
	} else {
	    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)str, appendCharacters:(const unichar *)cStr length:(NSUInteger)appendedLength);
	}
    } else {
        CFIndex strLength;
        __CFAssertIsStringAndMutable(str);
        strLength = __CFStrLength(str);

        __CFStringChangeSize(str, CFRangeMake(strLength, 0), appendedLength, appendedIsUnicode || __CFStrIsUnicode(str));

        if (__CFStrIsUnicode(str)) {
            UniChar *contents = (UniChar *)__CFStrContents(str);
            if (appendedIsUnicode) {
                memmove(contents + strLength, cStr, appendedLength * sizeof(UniChar));
            } else {
                __CFStrConvertBytesToUnicode((const uint8_t *)cStr, contents + strLength, appendedLength);
            }
        } else {
	    if (demoteAppendedUnicode) {
		UniChar *chars = (UniChar *)cStr;
		CFIndex idx;
		uint8_t *contents = (uint8_t *)__CFStrContents(str) + strLength + __CFStrSkipAnyLengthByte(str);
		for (idx = 0; idx < appendedLength; idx++) contents[idx] = (uint8_t)chars[idx];
	    } else {
		uint8_t *contents = (uint8_t *)__CFStrContents(str);
		memmove(contents + strLength + __CFStrSkipAnyLengthByte(str), cStr, appendedLength);
	    }
        }
    }

    if (freeCStrWhenDone) CFAllocatorDeallocate(__CFGetDefaultAllocator(), (void *)cStr);
}

void CFStringAppendPascalString(CFMutableStringRef str, ConstStringPtr pStr, CFStringEncoding encoding) {
    __CFStringAppendBytes(str, (const char *)(pStr + 1), (CFIndex)*pStr, encoding);
}

void CFStringAppendCString(CFMutableStringRef str, const char *cStr, CFStringEncoding encoding) {
    __CFStringAppendBytes(str, cStr, strlen(cStr), encoding);
}


void CFStringAppendFormat(CFMutableStringRef str, CFDictionaryRef formatOptions, CFStringRef format, ...) {
    va_list argList;

    va_start(argList, format);
    CFStringAppendFormatAndArguments(str, formatOptions, format, argList);
    va_end(argList);
}


CFIndex CFStringFindAndReplace(CFMutableStringRef string, CFStringRef stringToFind, CFStringRef replacementString, CFRange rangeToSearch, CFStringCompareFlags compareOptions) {
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, CFIndex, (NSMutableString *)string, replaceOccurrencesOfString:(NSString *)stringToFind withString:(NSString *)replacementString options:(NSStringCompareOptions)compareOptions range:NSMakeRange(rangeToSearch.location, rangeToSearch.length));
    CFRange foundRange;
    Boolean backwards = ((compareOptions & kCFCompareBackwards) != 0);
    UInt32 endIndex = rangeToSearch.location + rangeToSearch.length;
#define MAX_RANGES_ON_STACK (1000 / sizeof(CFRange))
    CFRange rangeBuffer[MAX_RANGES_ON_STACK];	// Used to avoid allocating memory
    CFRange *ranges = rangeBuffer;
    CFIndex foundCount = 0;
    CFIndex capacity = MAX_RANGES_ON_STACK;

    __CFAssertIsStringAndMutable(string);
    __CFAssertRangeIsInStringBounds(string, rangeToSearch.location, rangeToSearch.length);

    // Note: This code is very similar to the one in CFStringCreateArrayWithFindResults().
    while ((rangeToSearch.length > 0) && CFStringFindWithOptions(string, stringToFind, rangeToSearch, compareOptions, &foundRange)) {
	// Determine the next range
        if (backwards) {
            rangeToSearch.length = foundRange.location - rangeToSearch.location;
        } else {
            rangeToSearch.location = foundRange.location + foundRange.length;
            rangeToSearch.length = endIndex - rangeToSearch.location;
        }

	// If necessary, grow the array 
	if (foundCount >= capacity) {
            bool firstAlloc = (ranges == rangeBuffer) ? true : false;
            capacity = (capacity + 4) * 2;
            // Note that reallocate with NULL previous pointer is same as allocate
            ranges = (CFRange *)CFAllocatorReallocate(kCFAllocatorSystemDefault, firstAlloc ? NULL : ranges, capacity * sizeof(CFRange), 0);
            if (firstAlloc) memmove(ranges, rangeBuffer, MAX_RANGES_ON_STACK * sizeof(CFRange));
	}
        ranges[foundCount] = foundRange;
	foundCount++;
    }

    if (foundCount > 0) {
        if (backwards) {	// Reorder the ranges to be incrementing (better to do this here, then to check other places)
            int head = 0;
            int tail = foundCount - 1;
            while (head < tail) {
                CFRange temp = ranges[head];
                ranges[head] = ranges[tail];
                ranges[tail] = temp;
                head++;
                tail--;
            }
        }
        __CFStringReplaceMultiple(string, ranges, foundCount, replacementString);
        if (ranges != rangeBuffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, ranges);
    }
    
    return foundCount;
}


// This function is here for NSString purposes
// It allows checking for mutability before mutating; this allows NSString to catch invalid mutations

int __CFStringCheckAndReplace(CFMutableStringRef str, CFRange range, CFStringRef replacement) {
    if (!__CFStrIsMutable(str)) return _CFStringErrNotMutable;	// These three ifs are always here, for NSString usage
    if (!replacement && __CFStringNoteErrors()) return _CFStringErrNilArg;
    // This attempts to catch bad ranges including those described in 3375535 (-1,1)
    unsigned long endOfRange = (unsigned long)(range.location) + (unsigned long)(range.length);		// NSRange uses unsigned quantities, hence the casting
    if (((endOfRange > (unsigned long)__CFStrLength(str)) || (endOfRange < (unsigned long)(range.location))) && __CFStringNoteErrors()) return _CFStringErrBounds;

    __CFAssertIsStringAndMutable(str);
    __CFAssertRangeIsInStringBounds(str, range.location, range.length);
    __CFStringReplace(str, range, replacement);
    return _CFStringErrNone;
}

// This function determines whether errors which would cause string exceptions should
// be ignored or not

Boolean __CFStringNoteErrors(void) {
    return true;
}



void CFStringPad(CFMutableStringRef string, CFStringRef padString, CFIndex length, CFIndex indexIntoPad) {
    CFIndex originalLength;
    
    __CFAssertIsNotNegative(length);
    __CFAssertIsNotNegative(indexIntoPad);
 
    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)string, _cfPad:padString length:(uint32_t)length padIndex:(uint32_t)indexIntoPad);

    __CFAssertIsStringAndMutable(string);

    originalLength = __CFStrLength(string);
    if (length < originalLength) {
        __CFStringChangeSize(string, CFRangeMake(length, originalLength - length), 0, false);
    } else if (originalLength < length) {
        uint8_t *contents;
        Boolean isUnicode;
        CFIndex charSize;
        CFIndex padStringLength;
        CFIndex padLength;
        CFIndex padRemaining = length - originalLength;
        
        if (CF_IS_OBJC(__kCFStringTypeID, padString)) {	/* ??? Hope the compiler optimizes this away if OBJC_MAPPINGS is not on */
            padStringLength = CFStringGetLength(padString);
            isUnicode = true;	/* !!! Bad for now */
        } else {
            __CFAssertIsString(padString);
            padStringLength = __CFStrLength(padString);
            isUnicode = __CFStrIsUnicode(string) || __CFStrIsUnicode(padString);
        }

        charSize = isUnicode ? sizeof(UniChar) : sizeof(uint8_t);
        
        __CFStringChangeSize(string, CFRangeMake(originalLength, 0), padRemaining, isUnicode);

        contents = (uint8_t *)__CFStrContents(string) + charSize * originalLength + __CFStrSkipAnyLengthByte(string);
        padLength = padStringLength - indexIntoPad;
        padLength = padRemaining < padLength ? padRemaining : padLength;

        while (padRemaining > 0) {
            if (isUnicode) {
                CFStringGetCharacters(padString, CFRangeMake(indexIntoPad, padLength), (UniChar *)contents);
            } else {
                CFStringGetBytes(padString, CFRangeMake(indexIntoPad, padLength), __CFStringGetEightBitStringEncoding(), 0, false, contents, padRemaining * charSize, NULL);
            }
            contents += padLength * charSize;
            padRemaining -= padLength;
            indexIntoPad = 0;
            padLength = padRemaining < padLength ? padRemaining : padStringLength;
        }
    }
}

void CFStringTrim(CFMutableStringRef string, CFStringRef trimString) {
    CFRange range;
    CFIndex newStartIndex;
    CFIndex length;

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)string, _cfTrim:trimString);

    __CFAssertIsStringAndMutable(string);
    __CFAssertIsString(trimString);

    newStartIndex = 0;
    length = __CFStrLength(string);

    while (CFStringFindWithOptions(string, trimString, CFRangeMake(newStartIndex, length - newStartIndex), kCFCompareAnchored, &range)) {
        newStartIndex = range.location + range.length;
    }

    if (newStartIndex < length) {
        CFIndex charSize = __CFStrIsUnicode(string) ? sizeof(UniChar) : sizeof(uint8_t);
        uint8_t *contents = (uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string);

        length -= newStartIndex;
        if (CFStringGetLength(trimString) < length) {
            while (CFStringFindWithOptions(string, trimString, CFRangeMake(newStartIndex, length), kCFCompareAnchored|kCFCompareBackwards, &range)) {
                length = range.location - newStartIndex;
            }
        }
        memmove(contents, contents + newStartIndex * charSize, length * charSize);
        __CFStringChangeSize(string, CFRangeMake(length, __CFStrLength(string) - length), 0, false);
    } else { // Only trimString in string, trim all
        __CFStringChangeSize(string, CFRangeMake(0, length), 0, false);
    }
}

void CFStringTrimWhitespace(CFMutableStringRef string) {
    CFIndex newStartIndex;
    CFIndex length;
    CFStringInlineBuffer buffer;

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)string, _cfTrimWS);

    __CFAssertIsStringAndMutable(string);

    newStartIndex = 0;
    length = __CFStrLength(string);

    CFStringInitInlineBuffer(string, &buffer, CFRangeMake(0, length));
    CFIndex buffer_idx = 0;

    while (buffer_idx < length && CFUniCharIsMemberOf(__CFStringGetCharacterFromInlineBufferQuick(&buffer, buffer_idx), kCFUniCharWhitespaceAndNewlineCharacterSet))
        buffer_idx++;
    newStartIndex = buffer_idx;

    if (newStartIndex < length) {
        uint8_t *contents = (uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string);
        CFIndex charSize = (__CFStrIsUnicode(string) ? sizeof(UniChar) : sizeof(uint8_t));

        buffer_idx = length - 1;
        while (0 <= buffer_idx && CFUniCharIsMemberOf(__CFStringGetCharacterFromInlineBufferQuick(&buffer, buffer_idx), kCFUniCharWhitespaceAndNewlineCharacterSet))
            buffer_idx--;
        length = buffer_idx - newStartIndex + 1;

        memmove(contents, contents + newStartIndex * charSize, length * charSize);
        __CFStringChangeSize(string, CFRangeMake(length, __CFStrLength(string) - length), 0, false);
    } else { // Whitespace only string
        __CFStringChangeSize(string, CFRangeMake(0, length), 0, false);
    }
}

void CFStringLowercase(CFMutableStringRef string, CFLocaleRef locale) {
    CFIndex currentIndex = 0;
    CFIndex length;
    const uint8_t *langCode;
    Boolean isEightBit = __CFStrIsEightBit(string);

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)string, _cfLowercase:(const void *)locale);

    __CFAssertIsStringAndMutable(string);

    length = __CFStrLength(string);

    langCode = (const uint8_t *)(_CFCanUseLocale(locale) ? _CFStrGetLanguageIdentifierForLocale(locale, false) : NULL);

    if (!langCode && isEightBit) {
        uint8_t *contents = (uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string);
        for (;currentIndex < length;currentIndex++) {
            if (contents[currentIndex] >= 'A' && contents[currentIndex] <= 'Z') {
                contents[currentIndex] += 'a' - 'A';
            } else if (contents[currentIndex] > 127) {
                break;
            }
        }
    }

    if (currentIndex < length) {
        UTF16Char *contents;
        UniChar mappedCharacters[MAX_CASE_MAPPING_BUF];
        CFIndex mappedLength;
        UTF32Char currentChar;
        UInt32 flags = 0;

        if (isEightBit) __CFStringChangeSize(string, CFRangeMake(0, 0), 0, true);

        contents = (UniChar *)__CFStrContents(string);

        for (;currentIndex < length;currentIndex++) {

            if (CFUniCharIsSurrogateHighCharacter(contents[currentIndex]) && (currentIndex + 1 < length) && CFUniCharIsSurrogateLowCharacter(contents[currentIndex + 1])) {
                currentChar = CFUniCharGetLongCharacterForSurrogatePair(contents[currentIndex], contents[currentIndex + 1]);
            } else {
                currentChar = contents[currentIndex];
            }
            flags = ((langCode || (currentChar == 0x03A3)) ? CFUniCharGetConditionalCaseMappingFlags(currentChar, contents, currentIndex, length, kCFUniCharToLowercase, langCode, flags) : 0);

            mappedLength = CFUniCharMapCaseTo(currentChar, mappedCharacters, MAX_CASE_MAPPING_BUF, kCFUniCharToLowercase, flags, langCode);
            if (mappedLength > 0) contents[currentIndex] = *mappedCharacters;

            if (currentChar > 0xFFFF) { // Non-BMP char
                switch (mappedLength) {
                    case 0:
                    __CFStringChangeSize(string, CFRangeMake(currentIndex, 2), 0, true);
                    contents = (UniChar *)__CFStrContents(string);
                    length -= 2;
                    break;

                    case 1:
                    __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 1), 0, true);
                    contents = (UniChar *)__CFStrContents(string);
                    --length;
                    break;

                    case 2:
                    contents[++currentIndex] = mappedCharacters[1];
                    break;

                    default:
                    --mappedLength; // Skip the current char
                    __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 0), mappedLength - 1, true);
                    contents = (UniChar *)__CFStrContents(string);
                    memmove(contents + currentIndex + 1, mappedCharacters + 1, mappedLength * sizeof(UniChar));
                    length += (mappedLength - 1);
                    currentIndex += mappedLength;
                    break;
                }
            } else if (mappedLength == 0) {
                __CFStringChangeSize(string, CFRangeMake(currentIndex, 1), 0, true);
                contents = (UniChar *)__CFStrContents(string);
                --length;
            } else if (mappedLength > 1) {
                --mappedLength; // Skip the current char
                __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 0), mappedLength, true);
                contents = (UniChar *)__CFStrContents(string);
                memmove(contents + currentIndex + 1, mappedCharacters + 1, mappedLength * sizeof(UniChar));
                length += mappedLength;
                currentIndex += mappedLength;
            }
        }
    }
}

void CFStringUppercase(CFMutableStringRef string, CFLocaleRef locale) {
    CFIndex currentIndex = 0;
    CFIndex length;
    const uint8_t *langCode;
    Boolean isEightBit = __CFStrIsEightBit(string);

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)string, _cfUppercase:(const void *)locale);

    __CFAssertIsStringAndMutable(string);

    length = __CFStrLength(string);

    langCode = (const uint8_t *)(_CFCanUseLocale(locale) ? _CFStrGetLanguageIdentifierForLocale(locale, false) : NULL);

    if (!langCode && isEightBit) {
        uint8_t *contents = (uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string);
        for (;currentIndex < length;currentIndex++) {
            if (contents[currentIndex] >= 'a' && contents[currentIndex] <= 'z') {
                contents[currentIndex] -= 'a' - 'A';
            } else if (contents[currentIndex] > 127) {
                break;
            }
        }
    }

    if (currentIndex < length) {
        UniChar *contents;
        UniChar mappedCharacters[MAX_CASE_MAPPING_BUF];
        CFIndex mappedLength;
        UTF32Char currentChar;
        UInt32 flags = 0;

        if (isEightBit) __CFStringChangeSize(string, CFRangeMake(0, 0), 0, true);

        contents = (UniChar *)__CFStrContents(string);

        for (;currentIndex < length;currentIndex++) {
            if (CFUniCharIsSurrogateHighCharacter(contents[currentIndex]) && (currentIndex + 1 < length) && CFUniCharIsSurrogateLowCharacter(contents[currentIndex + 1])) {
                currentChar = CFUniCharGetLongCharacterForSurrogatePair(contents[currentIndex], contents[currentIndex + 1]);
            } else {
                currentChar = contents[currentIndex];
            }

            flags = (langCode ? CFUniCharGetConditionalCaseMappingFlags(currentChar, contents, currentIndex, length, kCFUniCharToUppercase, langCode, flags) : 0);

            mappedLength = CFUniCharMapCaseTo(currentChar, mappedCharacters, MAX_CASE_MAPPING_BUF, kCFUniCharToUppercase, flags, langCode);
            if (mappedLength > 0) contents[currentIndex] = *mappedCharacters;

            if (currentChar > 0xFFFF) { // Non-BMP char
                switch (mappedLength) {
                    case 0:
                    __CFStringChangeSize(string, CFRangeMake(currentIndex, 2), 0, true);
                    contents = (UniChar *)__CFStrContents(string);
                    length -= 2;
                    break;

                    case 1:
                    __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 1), 0, true);
                    contents = (UniChar *)__CFStrContents(string);
                    --length;
                    break;

                    case 2:
                    contents[++currentIndex] = mappedCharacters[1];
                    break;

                    default:
                    --mappedLength; // Skip the current char
                    __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 0), mappedLength - 1, true);
                    contents = (UniChar *)__CFStrContents(string);
                    memmove(contents + currentIndex + 1, mappedCharacters + 1, mappedLength * sizeof(UniChar));
                    length += (mappedLength - 1);
                    currentIndex += mappedLength;
                    break;
                }
            } else if (mappedLength == 0) {
                __CFStringChangeSize(string, CFRangeMake(currentIndex, 1), 0, true);
                contents = (UniChar *)__CFStrContents(string);
                --length;
            } else if (mappedLength > 1) {
                --mappedLength; // Skip the current char
                __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 0), mappedLength, true);
                contents = (UniChar *)__CFStrContents(string);
                memmove(contents + currentIndex + 1, mappedCharacters + 1, mappedLength * sizeof(UniChar));
                length += mappedLength;
                currentIndex += mappedLength;
            }
        }
    }
}


void CFStringCapitalize(CFMutableStringRef string, CFLocaleRef locale) {
    CFIndex currentIndex = 0;
    CFIndex length;
    const uint8_t *langCode;
    Boolean isEightBit = __CFStrIsEightBit(string);
    Boolean isLastCased = false;
    const uint8_t *caseIgnorableForBMP;

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)string, _cfCapitalize:(const void *)locale);

    __CFAssertIsStringAndMutable(string);

    length = __CFStrLength(string);

    caseIgnorableForBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharCaseIgnorableCharacterSet, 0);

    langCode = (const uint8_t *)(_CFCanUseLocale(locale) ? _CFStrGetLanguageIdentifierForLocale(locale, false) : NULL);

    if (!langCode && isEightBit) {
        uint8_t *contents = (uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string);
        for (;currentIndex < length;currentIndex++) {
            if (contents[currentIndex] > 127) {
                break;
            } else if (contents[currentIndex] >= 'A' && contents[currentIndex] <= 'Z') {
                contents[currentIndex] += (isLastCased ? 'a' - 'A' : 0);
                isLastCased = true;
            } else if (contents[currentIndex] >= 'a' && contents[currentIndex] <= 'z') {
                contents[currentIndex] -= (!isLastCased ? 'a' - 'A' : 0);
                isLastCased = true;
            } else if (!CFUniCharIsMemberOfBitmap(contents[currentIndex], caseIgnorableForBMP)) {
                isLastCased = false;
            }
        }
    }

    if (currentIndex < length) {
        UniChar *contents;
        UniChar mappedCharacters[MAX_CASE_MAPPING_BUF];
        CFIndex mappedLength;
        UTF32Char currentChar;
        UInt32 flags = 0;

        if (isEightBit) __CFStringChangeSize(string, CFRangeMake(0, 0), 0, true);

        contents = (UniChar *)__CFStrContents(string);

        for (;currentIndex < length;currentIndex++) {
            if (CFUniCharIsSurrogateHighCharacter(contents[currentIndex]) && (currentIndex + 1 < length) && CFUniCharIsSurrogateLowCharacter(contents[currentIndex + 1])) {
                currentChar = CFUniCharGetLongCharacterForSurrogatePair(contents[currentIndex], contents[currentIndex + 1]);
            } else {
                currentChar = contents[currentIndex];
            }
            flags = ((langCode || ((currentChar == 0x03A3) && isLastCased)) ? CFUniCharGetConditionalCaseMappingFlags(currentChar, contents, currentIndex, length, (isLastCased ? kCFUniCharToLowercase : kCFUniCharToTitlecase), langCode, flags) : 0);

            mappedLength = CFUniCharMapCaseTo(currentChar, mappedCharacters, MAX_CASE_MAPPING_BUF, (isLastCased ? kCFUniCharToLowercase : kCFUniCharToTitlecase), flags, langCode);
            if (mappedLength > 0) contents[currentIndex] = *mappedCharacters;

            if (currentChar > 0xFFFF) { // Non-BMP char
                switch (mappedLength) {
                    case 0:
                    __CFStringChangeSize(string, CFRangeMake(currentIndex, 2), 0, true);
                    contents = (UniChar *)__CFStrContents(string);
                    length -= 2;
                    break;

                    case 1:
                    __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 1), 0, true);
                    contents = (UniChar *)__CFStrContents(string);
                    --length;
                    break;

                    case 2:
                    contents[++currentIndex] = mappedCharacters[1];
                    break;

                    default:
                    --mappedLength; // Skip the current char
                    __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 0), mappedLength - 1, true);
                    contents = (UniChar *)__CFStrContents(string);
                    memmove(contents + currentIndex + 1, mappedCharacters + 1, mappedLength * sizeof(UniChar));
                    length += (mappedLength - 1);
                    currentIndex += mappedLength;
                    break;
                }
            } else if (mappedLength == 0) {
                __CFStringChangeSize(string, CFRangeMake(currentIndex, 1), 0, true);
                contents = (UniChar *)__CFStrContents(string);
                --length;
            } else if (mappedLength > 1) {
                --mappedLength; // Skip the current char
                __CFStringChangeSize(string, CFRangeMake(currentIndex + 1, 0), mappedLength, true);
                contents = (UniChar *)__CFStrContents(string);
                memmove(contents + currentIndex + 1, mappedCharacters + 1, mappedLength * sizeof(UniChar));
                length += mappedLength;
                currentIndex += mappedLength;
            }

            if (!((currentChar > 0xFFFF) ? CFUniCharIsMemberOf(currentChar, kCFUniCharCaseIgnorableCharacterSet) : CFUniCharIsMemberOfBitmap(currentChar, caseIgnorableForBMP))) { // We have non-caseignorable here
                isLastCased = ((CFUniCharIsMemberOf(currentChar, kCFUniCharUppercaseLetterCharacterSet) || CFUniCharIsMemberOf(currentChar, kCFUniCharLowercaseLetterCharacterSet)) ? true : false);
            }
        }
    }
}


#define MAX_DECOMP_BUF 64

#define HANGUL_SBASE 0xAC00
#define HANGUL_LBASE 0x1100
#define HANGUL_VBASE 0x1161
#define HANGUL_TBASE 0x11A7
#define HANGUL_SCOUNT 11172
#define HANGUL_LCOUNT 19
#define HANGUL_VCOUNT 21
#define HANGUL_TCOUNT 28
#define HANGUL_NCOUNT (HANGUL_VCOUNT * HANGUL_TCOUNT)

CF_INLINE uint32_t __CFGetUTF16Length(const UTF32Char *characters, uint32_t utf32Length) {
    const UTF32Char *limit = characters + utf32Length;
    uint32_t length = 0;

    while (characters < limit) length += (*(characters++) > 0xFFFF ? 2 : 1);

    return length;
}

CF_INLINE void __CFFillInUTF16(const UTF32Char *characters, UTF16Char *dst, uint32_t utf32Length) {
    const UTF32Char *limit = characters + utf32Length;
    UTF32Char currentChar;

    while (characters < limit) {
        currentChar = *(characters++);
        if (currentChar > 0xFFFF) {
            currentChar -= 0x10000;
            *(dst++) = (UTF16Char)((currentChar >> 10) + 0xD800UL);
            *(dst++) = (UTF16Char)((currentChar & 0x3FF) + 0xDC00UL);
        } else {
            *(dst++) = currentChar;
        }
    }
}

void CFStringNormalize(CFMutableStringRef string, CFStringNormalizationForm theForm) {
    CFIndex currentIndex = 0;
    CFIndex length;
    bool needToReorder = true;

    CF_OBJC_FUNCDISPATCHV(__kCFStringTypeID, void, (NSMutableString *)string, _cfNormalize:theForm);

    __CFAssertIsStringAndMutable(string);

    length = __CFStrLength(string);

    if (__CFStrIsEightBit(string)) {
        uint8_t *contents;

        if (theForm == kCFStringNormalizationFormC) return; // 8bit form has no decomposition

        contents = (uint8_t *)__CFStrContents(string) + __CFStrSkipAnyLengthByte(string);

        for (;currentIndex < length;currentIndex++) {
            if (contents[currentIndex] > 127) {
                __CFStringChangeSize(string, CFRangeMake(0, 0), 0, true); // need to do harm way
                needToReorder = false;
                break;
            }
        }
    }

    if (currentIndex < length) {
        UTF16Char *limit = (UTF16Char *)__CFStrContents(string) + length;
        UTF16Char *contents = (UTF16Char *)__CFStrContents(string) + currentIndex;
        UTF32Char buffer[MAX_DECOMP_BUF];
        UTF32Char *mappedCharacters = buffer;
        CFIndex allocatedLength = MAX_DECOMP_BUF;
        CFIndex mappedLength;
        CFIndex currentLength;
        UTF32Char currentChar;
        const uint8_t *decompBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, 0);
        const uint8_t *nonBaseBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, 0);
        const uint8_t *combiningBMP = (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, 0);

        while (contents < limit) {
            if (CFUniCharIsSurrogateHighCharacter(*contents) && (contents + 1 < limit) && CFUniCharIsSurrogateLowCharacter(*(contents + 1))) {
                currentChar = CFUniCharGetLongCharacterForSurrogatePair(*contents, *(contents + 1));
                currentLength = 2;
                contents += 2;
            } else {
                currentChar = *(contents++);
                currentLength = 1;
            }

            mappedLength = 0;

            if (CFUniCharIsMemberOfBitmap(currentChar, ((currentChar < 0x10000) ? decompBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, (currentChar >> 16)))) && (0 == CFUniCharGetCombiningPropertyForCharacter(currentChar, ((currentChar < 0x10000) ? combiningBMP : (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (currentChar >> 16)))))) {
                if ((theForm & kCFStringNormalizationFormC) == 0 || currentChar < HANGUL_SBASE || currentChar > (HANGUL_SBASE + HANGUL_SCOUNT)) { // We don't have to decompose Hangul Syllables if we're precomposing again
                    mappedLength = CFUniCharDecomposeCharacter(currentChar, mappedCharacters, MAX_DECOMP_BUF);
                }
            }

            if ((needToReorder || (theForm & kCFStringNormalizationFormC)) && ((contents < limit) || (mappedLength == 0))) {
                if (mappedLength > 0) {
                    if (CFUniCharIsSurrogateHighCharacter(*contents) && (contents + 1 < limit) && CFUniCharIsSurrogateLowCharacter(*(contents + 1))) {
                        currentChar = CFUniCharGetLongCharacterForSurrogatePair(*contents, *(contents + 1));
                    } else {
                        currentChar = *contents;
                    }
                }

                if (0 != CFUniCharGetCombiningPropertyForCharacter(currentChar, (const uint8_t *)((currentChar < 0x10000) ? combiningBMP : CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (currentChar >> 16))))) {
                    uint32_t decompLength;

                    if (mappedLength == 0) {
                        contents -= (currentChar & 0xFFFF0000 ? 2 : 1);
                        if (currentIndex > 0) {
                            if (CFUniCharIsSurrogateLowCharacter(*(contents - 1)) && (currentIndex > 1) && CFUniCharIsSurrogateHighCharacter(*(contents - 2))) {
                                *mappedCharacters = CFUniCharGetLongCharacterForSurrogatePair(*(contents - 2), *(contents - 1));
                                currentIndex -= 2;
                                currentLength += 2;
                            } else {
                                *mappedCharacters = *(contents - 1);
                                --currentIndex;
                                ++currentLength;
                            }
                            mappedLength = 1;
                        }
                    } else {
                        currentLength += (currentChar & 0xFFFF0000 ? 2 : 1);
                    }
                    contents += (currentChar & 0xFFFF0000 ? 2 : 1);

                    if (CFUniCharIsMemberOfBitmap(currentChar, ((currentChar < 0x10000) ? decompBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, (currentChar >> 16))))) { // Vietnamese accent, etc.
                        decompLength = CFUniCharDecomposeCharacter(currentChar, mappedCharacters + mappedLength, MAX_DECOMP_BUF - mappedLength);
                        mappedLength += decompLength;
                    } else {
                        mappedCharacters[mappedLength++] = currentChar;
                    }

                    while (contents < limit) {
                        if (CFUniCharIsSurrogateHighCharacter(*contents) && (contents + 1 < limit) && CFUniCharIsSurrogateLowCharacter(*(contents + 1))) {
                            currentChar = CFUniCharGetLongCharacterForSurrogatePair(*contents, *(contents + 1));
                        } else {
                            currentChar = *contents;
                        }
                        if (0 == CFUniCharGetCombiningPropertyForCharacter(currentChar, (const uint8_t *)((currentChar < 0x10000) ? combiningBMP : CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (currentChar >> 16))))) break;
                        if (currentChar & 0xFFFF0000) {
                            contents += 2;
                            currentLength += 2;
                        } else {
                            ++contents;
                            ++currentLength;
                        }
                        if (mappedLength == allocatedLength) {
                            allocatedLength += MAX_DECOMP_BUF;
                            if (mappedCharacters == buffer) {
                                mappedCharacters = (UTF32Char *)CFAllocatorAllocate(kCFAllocatorSystemDefault, allocatedLength * sizeof(UTF32Char), 0);
                                memmove(mappedCharacters, buffer, MAX_DECOMP_BUF * sizeof(UTF32Char));
                            } else {
                                mappedCharacters = (UTF32Char *)CFAllocatorReallocate(kCFAllocatorSystemDefault, mappedCharacters, allocatedLength * sizeof(UTF32Char), 0);
                            }
                        }
                        if (CFUniCharIsMemberOfBitmap(currentChar, ((currentChar < 0x10000) ? decompBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharCanonicalDecomposableCharacterSet, (currentChar >> 16))))) { // Vietnamese accent, etc.
                            decompLength = CFUniCharDecomposeCharacter(currentChar, mappedCharacters + mappedLength, MAX_DECOMP_BUF - mappedLength);
                            mappedLength += decompLength;
                        } else {
                            mappedCharacters[mappedLength++] = currentChar;
                        }
                    }
                }
                if (needToReorder && mappedLength > 1) CFUniCharPrioritySort(mappedCharacters, mappedLength);
            }

            if (theForm & kCFStringNormalizationFormKD) {
                CFIndex newLength = 0;

                if (mappedLength == 0 && CFUniCharIsMemberOf(currentChar, kCFUniCharCompatibilityDecomposableCharacterSet)) {
                    mappedCharacters[mappedLength++] = currentChar;
                }
                while (newLength < mappedLength) {
                    newLength = CFUniCharCompatibilityDecompose(mappedCharacters, mappedLength, allocatedLength);
                    if (newLength == 0) {
                        allocatedLength += MAX_DECOMP_BUF;
                        if (mappedCharacters == buffer) {
                            mappedCharacters = (UTF32Char *)CFAllocatorAllocate(kCFAllocatorSystemDefault, allocatedLength * sizeof(UTF32Char), 0);
                            memmove(mappedCharacters, buffer, MAX_DECOMP_BUF * sizeof(UTF32Char));
                        } else {
                            mappedCharacters = (UTF32Char *)CFAllocatorReallocate(kCFAllocatorSystemDefault, mappedCharacters, allocatedLength * sizeof(UTF32Char), 0);
                        }
                    }
                }
                mappedLength = newLength;
            }

            if (theForm & kCFStringNormalizationFormC) {
                UTF32Char nextChar;

                if (mappedLength > 1) {
                    CFIndex consumedLength = 1;
                    UTF32Char *currentBase = mappedCharacters;
                    uint8_t currentClass, lastClass = 0;
                    bool didCombine = false;

                    currentChar = *mappedCharacters;

                    while (consumedLength < mappedLength) {
                        nextChar = mappedCharacters[consumedLength];
                        currentClass = CFUniCharGetCombiningPropertyForCharacter(nextChar, (const uint8_t *)((nextChar < 0x10000) ? combiningBMP : CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (nextChar >> 16))));

                        if (theForm & kCFStringNormalizationFormKD) {
                            if ((currentChar >= HANGUL_LBASE) && (currentChar < (HANGUL_LBASE + 0xFF))) {
                                SInt8 lIndex = currentChar - HANGUL_LBASE;

                                if ((0 <= lIndex) && (lIndex <= HANGUL_LCOUNT)) {
                                    SInt16 vIndex = nextChar - HANGUL_VBASE;

                                    if ((vIndex >= 0) && (vIndex <= HANGUL_VCOUNT)) {
                                        SInt16 tIndex = 0;
                                        CFIndex usedLength = mappedLength;

                                        mappedCharacters[consumedLength++] = 0xFFFD;

                                        if (consumedLength < mappedLength) {
                                            tIndex = mappedCharacters[consumedLength] - HANGUL_TBASE;
                                            if ((tIndex < 0) || (tIndex > HANGUL_TCOUNT)) {
                                                tIndex = 0;
                                            } else {
                                                mappedCharacters[consumedLength++] = 0xFFFD;
                                            }
                                        }
                                        *currentBase = (lIndex * HANGUL_VCOUNT + vIndex) * HANGUL_TCOUNT + tIndex + HANGUL_SBASE;

                                        while (--usedLength > 0) {
                                            if (mappedCharacters[usedLength] == 0xFFFD) {
                                                --mappedLength;
                                                --consumedLength;
                                                memmove(mappedCharacters + usedLength, mappedCharacters + usedLength + 1, (mappedLength - usedLength) * sizeof(UTF32Char));
                                            }
                                        }
                                        currentBase = mappedCharacters + consumedLength;
                                        currentChar = *currentBase;
                                        ++consumedLength;

                                        continue;
                                    }
                                }
                            }
                            if (!CFUniCharIsMemberOfBitmap(nextChar, ((nextChar < 0x10000) ? nonBaseBMP : CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, (nextChar >> 16))))) {
                                *currentBase = currentChar;
                                currentBase = mappedCharacters + consumedLength;
                                currentChar = nextChar;
                                ++consumedLength;
                                continue;
                            }
                        }

                        if ((lastClass == 0) || (currentClass > lastClass)) {
                            nextChar = CFUniCharPrecomposeCharacter(currentChar, nextChar);
                            if (nextChar == 0xFFFD) {
                                lastClass = currentClass;
                            } else {
                                mappedCharacters[consumedLength] = 0xFFFD;
                                didCombine = true;
                                currentChar = nextChar;
                            }
                        }
                        ++consumedLength;
                    }

                    *currentBase = currentChar;
                    if (didCombine) {
                        consumedLength = mappedLength;
                        while (--consumedLength > 0) {
                            if (mappedCharacters[consumedLength] == 0xFFFD) {
                                --mappedLength;
                                memmove(mappedCharacters + consumedLength, mappedCharacters + consumedLength + 1, (mappedLength - consumedLength) * sizeof(UTF32Char));
                            }
                        }
                    }
                } else if ((currentChar >= HANGUL_LBASE) && (currentChar < (HANGUL_LBASE + 0xFF))) { // Hangul Jamo
                    SInt8 lIndex = currentChar - HANGUL_LBASE;

                    if ((contents < limit) && (0 <= lIndex) && (lIndex <= HANGUL_LCOUNT)) {
                        SInt16 vIndex = *contents - HANGUL_VBASE;

                        if ((vIndex >= 0) && (vIndex <= HANGUL_VCOUNT)) {
                            SInt16 tIndex = 0;

                            ++contents; ++currentLength;

                            if (contents < limit) {
                                tIndex = *contents - HANGUL_TBASE;
                                if ((tIndex < 0) || (tIndex > HANGUL_TCOUNT)) {
                                    tIndex = 0;
                                } else {
                                    ++contents; ++currentLength;
                                }
                            }
                            *mappedCharacters = (lIndex * HANGUL_VCOUNT + vIndex) * HANGUL_TCOUNT + tIndex + HANGUL_SBASE;
                            mappedLength = 1;
                        }
                    }
                } else { // collect class 0 non-base characters
                    while (contents < limit) {
                        nextChar = *contents;
                        if (CFUniCharIsSurrogateHighCharacter(nextChar) && ((contents + 1) < limit) && CFUniCharIsSurrogateLowCharacter(*(contents + 1))) {
                            nextChar = CFUniCharGetLongCharacterForSurrogatePair(nextChar, *(contents + 1));
                            if (!CFUniCharIsMemberOfBitmap(nextChar, (const uint8_t *)CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, (nextChar >> 16))) || (0 != CFUniCharGetCombiningPropertyForCharacter(nextChar, (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (nextChar >> 16))))) break;
                        } else {
                            if (!CFUniCharIsMemberOfBitmap(nextChar, nonBaseBMP) || (0 != CFUniCharGetCombiningPropertyForCharacter(nextChar, combiningBMP))) break;
                        }
                        currentChar = CFUniCharPrecomposeCharacter(currentChar, nextChar);
                        if (0xFFFD == currentChar) break;

                        if (nextChar < 0x10000) {
                            ++contents; ++currentLength;
                        } else {
                            contents += 2;
                            currentLength += 2;
                        }

                        *mappedCharacters = currentChar;
                        mappedLength = 1;
                    }
                }
            }

            if (mappedLength > 0) {
                CFIndex utf16Length = __CFGetUTF16Length(mappedCharacters, mappedLength);

                if (utf16Length != currentLength) {
                    __CFStringChangeSize(string, CFRangeMake(currentIndex, currentLength), utf16Length, true);
                    currentLength = utf16Length;
                }
                contents = (UTF16Char *)__CFStrContents(string);
                limit = contents + __CFStrLength(string);
                contents += currentIndex;
                __CFFillInUTF16(mappedCharacters, contents, mappedLength);
                contents += utf16Length;
            }
            currentIndex += currentLength;
        }

        if (mappedCharacters != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, mappedCharacters);
    }
}

void CFStringFold(CFMutableStringRef theString, CFStringCompareFlags theFlags, CFLocaleRef locale) {
    CFStringInlineBuffer stringBuffer;
    CFIndex length = CFStringGetLength(theString);
    CFIndex currentIndex = 0;
    CFIndex bufferLength = 0;
    UTF32Char buffer[kCFStringStackBufferLength];
    const uint8_t *cString;
    const uint8_t *langCode;
    CFStringEncoding eightBitEncoding;
    bool caseInsensitive = ((theFlags & kCFCompareCaseInsensitive) ? true : false);
    bool isObjc = CF_IS_OBJC(__kCFStringTypeID, theString);
    CFLocaleRef theLocale = locale;

    if ((theFlags & kCFCompareLocalized) && (NULL == locale)) {
        theLocale = CFLocaleCopyCurrent();
    }

    theFlags &= (kCFCompareCaseInsensitive|kCFCompareDiacriticInsensitive|kCFCompareWidthInsensitive);
    
    if ((0 == theFlags) || (0 == length)) goto bail; // nothing to do

    langCode = ((NULL == theLocale) ? NULL : (const uint8_t *)_CFStrGetLanguageIdentifierForLocale(theLocale, true));

    eightBitEncoding = __CFStringGetEightBitStringEncoding();
    cString = (const uint8_t *)CFStringGetCStringPtr(theString, eightBitEncoding);

    if ((NULL != cString) && !caseInsensitive && (kCFStringEncodingASCII == eightBitEncoding)) goto bail; // All ASCII

    CFStringInitInlineBuffer(theString, &stringBuffer, CFRangeMake(0, length));

    if ((NULL != cString) && (theFlags & (kCFCompareCaseInsensitive|kCFCompareDiacriticInsensitive))) {
        const uint8_t *cStringPtr = cString;
        const uint8_t *cStringLimit = cString + length;
        uint8_t *cStringContents = (isObjc ? NULL : (uint8_t *)__CFStrContents(theString) + __CFStrSkipAnyLengthByte(theString));
        
        while (cStringPtr < cStringLimit) {
            if ((*cStringPtr < 0x80) && (NULL == langCode)) {
                if (caseInsensitive && (*cStringPtr >= 'A') && (*cStringPtr <= 'Z')) {
                    if (NULL == cStringContents) {
                        break;
                    } else {
                        cStringContents[cStringPtr - cString] += ('a' - 'A');
                    }
                }
            } else {
                if ((bufferLength = __CFStringFoldCharacterClusterAtIndex((UTF32Char)__CFCharToUniCharTable[*cStringPtr], &stringBuffer, cStringPtr - cString, theFlags, langCode, buffer, kCFStringStackBufferLength, NULL)) > 0) {
                    if ((*buffer > 0x7F) || (bufferLength > 1) || (NULL == cStringContents)) break;
                    cStringContents[cStringPtr - cString] = *buffer;
                }
            }
            ++cStringPtr;
        }
        
        currentIndex = cStringPtr - cString;
    }

    if (currentIndex < length) {
        UTF16Char *contents;

        if (isObjc) {
            CFMutableStringRef cfString;
            CFRange range = CFRangeMake(currentIndex, length - currentIndex);

            contents = (UTF16Char *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(UTF16Char) * range.length, 0);

            CFStringGetCharacters(theString, range, contents);

            cfString = CFStringCreateMutableWithExternalCharactersNoCopy(kCFAllocatorSystemDefault, contents, range.length, range.length, NULL);

            CFStringFold(cfString, theFlags, theLocale);

            CFStringReplace(theString, range, cfString);

            CFRelease(cfString);
        } else {
            const UTF32Char *characters;
            const UTF32Char *charactersLimit;
            UTF32Char character;
            CFIndex consumedLength;

            contents = NULL;

            if (bufferLength > 0) {
                __CFStringChangeSize(theString, CFRangeMake(currentIndex + 1, 0), bufferLength - 1, true);
                length = __CFStrLength(theString);
                CFStringInitInlineBuffer(theString, &stringBuffer, CFRangeMake(0, length));

                contents = (UTF16Char *)__CFStrContents(theString) + currentIndex;
                characters = buffer;
                charactersLimit = characters + bufferLength;
                while (characters < charactersLimit) *(contents++) = (UTF16Char)*(characters++);
                ++currentIndex;
            }

            while (currentIndex < length) {
                character = __CFStringGetCharacterFromInlineBufferQuick(&stringBuffer, currentIndex);

                consumedLength = 0;

                if ((NULL == langCode) && (character < 0x80) && (0 == (theFlags & kCFCompareDiacriticInsensitive))) {
                    if (caseInsensitive && (character >= 'A') && (character <= 'Z')) {
                        consumedLength = 1;
                        bufferLength = 1;
                        *buffer = character + ('a' - 'A');
                    }
                } else {
                    if (CFUniCharIsSurrogateHighCharacter(character) && ((currentIndex + 1) < length)) {
                        UTF16Char lowSurrogate = __CFStringGetCharacterFromInlineBufferQuick(&stringBuffer, currentIndex + 1);
                        if (CFUniCharIsSurrogateLowCharacter(lowSurrogate)) character = CFUniCharGetLongCharacterForSurrogatePair(character, lowSurrogate);
                    }

                    bufferLength = __CFStringFoldCharacterClusterAtIndex(character, &stringBuffer, currentIndex, theFlags, langCode, buffer, kCFStringStackBufferLength, &consumedLength);
                }

                if (consumedLength > 0) {
                    CFIndex utf16Length = bufferLength;

                    characters = buffer;
                    charactersLimit = characters + bufferLength;

                    while (characters < charactersLimit) if (*(characters++) > 0xFFFF) ++utf16Length; // Extend bufferLength to the UTF-16 length

                    if ((utf16Length != consumedLength) || __CFStrIsEightBit(theString)) {
                        CFRange range;
                        CFIndex insertLength;

                        if (consumedLength < utf16Length) { // Need to expand
                            range = CFRangeMake(currentIndex + consumedLength, 0);
                            insertLength = utf16Length - consumedLength;
                        } else {
                            range = CFRangeMake(currentIndex + utf16Length, consumedLength - utf16Length);
                            insertLength = 0;
                        }
                        __CFStringChangeSize(theString, range, insertLength, true);
                        length = __CFStrLength(theString);
                        CFStringInitInlineBuffer(theString, &stringBuffer, CFRangeMake(0, length));
                    }

                    (void)CFUniCharFromUTF32(buffer, bufferLength, (UTF16Char *)__CFStrContents(theString) + currentIndex, true, __CF_BIG_ENDIAN__);

                    currentIndex += utf16Length;
                } else {
                    ++currentIndex;
                }
            }
        }
    }

    bail:
    if (NULL == locale && theLocale) {
        CFRelease(theLocale);
    }
}

enum {
	kCFStringFormatZeroFlag = (1 << 0),         // if not, padding is space char
	kCFStringFormatMinusFlag = (1 << 1),        // if not, no flag implied
	kCFStringFormatPlusFlag = (1 << 2),         // if not, no flag implied, overrides space
	kCFStringFormatSpaceFlag = (1 << 3),        // if not, no flag implied
	kCFStringFormatExternalSpecFlag = (1 << 4), // using config dict
        kCFStringFormatLocalizable = (1 << 5)       // explicitly mark the specs we can localize
};

typedef struct {
    int16_t size;
    int16_t type;
    SInt32 loc;
    SInt32 len;
    SInt32 widthArg;
    SInt32 precArg;
    uint32_t flags;
    int8_t mainArgNum;
    int8_t precArgNum;
    int8_t widthArgNum;
    int8_t configDictIndex;
    int8_t numericFormatStyle;        // Only set for localizable numeric quantities
} CFFormatSpec;

typedef struct {
    int16_t type;
    int16_t size;
    union {
	int64_t int64Value;
	double doubleValue;
#if LONG_DOUBLE_SUPPORT
	long double longDoubleValue;
#endif
	void *pointerValue;
    } value;
} CFPrintValue;

enum {
    CFFormatDefaultSize = 0,
    CFFormatSize1 = 1,
    CFFormatSize2 = 2,
    CFFormatSize4 = 3,
    CFFormatSize8 = 4,
    CFFormatSize16 = 5,
#if __LP64__
    CFFormatSizeLong = CFFormatSize8,
    CFFormatSizePointer = CFFormatSize8
#else
    CFFormatSizeLong = CFFormatSize4,
    CFFormatSizePointer = CFFormatSize4
#endif
};
        
enum {
    CFFormatStyleDecimal = (1 << 0),
    CFFormatStyleScientific = (1 << 1),
    CFFormatStyleDecimalOrScientific = CFFormatStyleDecimal|CFFormatStyleScientific,
    CFFormatStyleUnsigned = (1 << 2)
};

enum {
    CFFormatLiteralType = 32,
    CFFormatLongType = 33,
    CFFormatDoubleType = 34,
    CFFormatPointerType = 35,
    CFFormatObjectType = 36,		/* handled specially */	/* ??? not used anymore, can be removed? */
    CFFormatCFType = 37,		/* handled specially */
    CFFormatUnicharsType = 38,		/* handled specially */
    CFFormatCharsType = 39,		/* handled specially */
    CFFormatPascalCharsType = 40,	/* handled specially */
    CFFormatSingleUnicharType = 41,	/* handled specially */
    CFFormatDummyPointerType = 42	/* special case for %n */
};

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
/* Only come in here if spec->type is CFFormatLongType or CFFormatDoubleType. Pass in 0 for width or precision if not specified. Returns false if couldn't do the format (with the assumption the caller falls back to unlocalized).
*/
static Boolean __CFStringFormatLocalizedNumber(CFMutableStringRef output, CFLocaleRef locale, const CFPrintValue *values, const CFFormatSpec *spec, SInt32 width, SInt32 precision, Boolean hasPrecision) {
    static CFLock_t formatterLock = CFLockInit;
    // These formatters are recached if the locale argument is different
    static CFNumberFormatterRef decimalFormatter = NULL;
    static CFNumberFormatterRef scientificFormatter = NULL;
    static CFNumberFormatterRef gFormatter = NULL;  // for %g
    static SInt32 groupingSize = 0;
    static SInt32 secondaryGroupingSize = 0;
    
    // !!! This code should be removed before shipping
    static char disableLocalizedFormatting = -1;
    if (disableLocalizedFormatting == -1) disableLocalizedFormatting = (getenv("CFStringDisableLocalizedNumberFormatting") != NULL) ? 1 : 0;
    if (disableLocalizedFormatting) return false;

    CFNumberFormatterRef formatter;
    
    __CFLock(&formatterLock);   // We use the formatter from one thread at one time; if this proves to be a bottleneck we need to get fancier

    switch (spec->numericFormatStyle) {
        case CFFormatStyleUnsigned:
        case CFFormatStyleDecimal:
                if (!decimalFormatter || !CFEqual(CFNumberFormatterGetLocale(decimalFormatter), locale)) {  // cache or recache if the locale is different
                    if (decimalFormatter) CFRelease(decimalFormatter);
                    decimalFormatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterDecimalStyle);  // since this is shared, remember to reset all its properties!
                }
                formatter = decimalFormatter;
                break;
        case CFFormatStyleScientific:
                if (!scientificFormatter || !CFEqual(CFNumberFormatterGetLocale(scientificFormatter), locale)) {  // cache or recache if the locale is different
                    if (scientificFormatter) CFRelease(scientificFormatter);
                    scientificFormatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterScientificStyle);
                    CFStringRef pattern = CFSTR("#E+00");     // the default pattern does not have the sign if the exponent is positive and it is single digit
                    CFNumberFormatterSetFormat(scientificFormatter, pattern);
                    CFNumberFormatterSetProperty(scientificFormatter, kCFNumberFormatterUseSignificantDigitsKey, kCFBooleanTrue);
                }
                formatter = scientificFormatter;
                break;
        case CFFormatStyleDecimalOrScientific:
                if (!gFormatter || !CFEqual(CFNumberFormatterGetLocale(gFormatter), locale)) {  // cache or recache if the locale is different
                    if (gFormatter) CFRelease(gFormatter);
                    gFormatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterDecimalStyle);
                    // when we update the locale in gFormatter, we also need to update the two grouping sizes
                    CFNumberRef num = (CFNumberRef) CFNumberFormatterCopyProperty(gFormatter, kCFNumberFormatterGroupingSizeKey);
                    CFNumberGetValue(num, kCFNumberSInt32Type, &groupingSize);
                    CFRelease(num);
                    num = (CFNumberRef) CFNumberFormatterCopyProperty(gFormatter, kCFNumberFormatterSecondaryGroupingSizeKey);
                    CFNumberGetValue(num, kCFNumberSInt32Type, &secondaryGroupingSize);
                    CFRelease(num);
                }
                formatter = gFormatter;
                break;
    }
    
    SInt32 prec = hasPrecision ? precision : ((spec->type == CFFormatLongType) ? 0 : 6); // default precision of printf is 6
    
    // pattern must be set before setting width and padding
    // otherwise, the pattern will take over those settings
    if (spec->numericFormatStyle == CFFormatStyleDecimalOrScientific) {
        if (prec == 0) prec = 1; // at least one sig fig
        CFMutableStringRef pattern = CFStringCreateMutable(NULL, 0);
        // use significant digits pattern
        CFStringAppendCString(pattern, "@", kCFStringEncodingASCII);
        CFStringPad(pattern, CFSTR("#"), prec, 0);
        double targetValue = values[spec->mainArgNum].value.doubleValue;
#if LONG_DOUBLE_SUPPORT
        if (CFFormatSize16 == values[spec->mainArgNum].size) {
            targetValue = values[spec->mainArgNum].value.longDoubleValue; // losing precision
        }
#endif
        double max = pow(10.0, (double)prec);   // if the value requires more digits than the number of sig figs, we need to use scientific format
        double min = 0.0001;                    // if the value is less than 10E-4, scientific format is the shorter form
        if (((targetValue > 0 && (targetValue > max || targetValue < min)) || (targetValue < 0 && (targetValue < -max || targetValue > -min)))){
            CFStringAppendCString(pattern, "E+00", kCFStringEncodingASCII);
        } else if (prec > groupingSize && groupingSize != 0) {
            CFStringInsert(pattern, prec-groupingSize, CFSTR(","));  // if we are not using scientific format, we need to set the pattern to use grouping separator
            if (secondaryGroupingSize != 0 && prec > (groupingSize + secondaryGroupingSize)) CFStringInsert(pattern, prec-groupingSize-secondaryGroupingSize, CFSTR(","));
        }
        CFNumberFormatterSetFormat(formatter, pattern);
        CFRelease(pattern);
    }
    
    // clear the padding, we will add it later if we need it
    const SInt32 z = 0;
    CFNumberRef zero = CFNumberCreate(NULL, kCFNumberSInt32Type, &z);
    CFNumberFormatterSetProperty(formatter, kCFNumberFormatterFormatWidthKey, zero);
    
    CFNumberRef tmp = CFNumberCreate(NULL, kCFNumberSInt32Type, &prec);
    CFNumberFormatterSetProperty(formatter, kCFNumberFormatterMaxFractionDigitsKey, tmp);
    if (spec->type == CFFormatDoubleType) {
        CFNumberFormatterSetProperty(formatter, kCFNumberFormatterMinFractionDigitsKey, tmp);
    } else {
        CFNumberFormatterSetProperty(formatter, kCFNumberFormatterMinFractionDigitsKey, zero);
    }
    CFRelease(tmp);
    CFRelease(zero);

    Boolean isNegative = false;
    switch (values[spec->mainArgNum].type) {
        case CFFormatLongType:
            if (values[spec->mainArgNum].value.int64Value < 0) isNegative = true;
            break;
        case CFFormatDoubleType:
#if LONG_DOUBLE_SUPPORT
            if ((CFFormatSize16 == values[spec->mainArgNum].size) && (values[spec->mainArgNum].value.longDoubleValue < 0)) isNegative = true;
            else
#endif
            if (values[spec->mainArgNum].value.doubleValue < 0) isNegative = true;
            break;
    }

    CFStringRef pattern = CFNumberFormatterGetFormat(formatter);
    if ((spec->flags & kCFStringFormatPlusFlag) && !isNegative) {
        if (CFStringGetCharacterAtIndex(pattern, 0) != '+') {
            CFMutableStringRef newPattern = CFStringCreateMutableCopy(NULL, 0, CFSTR("+"));
            CFStringAppend(newPattern, pattern);
            CFNumberFormatterSetFormat(formatter, newPattern);
            CFRelease(newPattern);
        }
    } else {
        if (CFStringGetCharacterAtIndex(pattern, 0) == '+') {
            CFStringRef newPattern = CFStringCreateWithSubstring(NULL, pattern, CFRangeMake(1, CFStringGetLength(pattern)-1));
            CFNumberFormatterSetFormat(formatter, newPattern);
            CFRelease(newPattern);
        }
    }

    // width == 0 seems to be CFNumberFormatter's default setting
    if (hasPrecision && spec->type == CFFormatLongType) { // if we have precision and %d or %u, we pad 0 according to precision first
        tmp = CFNumberCreate(NULL, kCFNumberSInt32Type, &prec);
    } else {
        tmp = CFNumberCreate(NULL, kCFNumberSInt32Type, &width); 
    }
    CFNumberFormatterSetProperty(formatter, kCFNumberFormatterFormatWidthKey, tmp);
    CFRelease(tmp);
    
    // ??? use the right zero here for Arabic
    Boolean padZero = spec->flags & kCFStringFormatZeroFlag;
    if (hasPrecision && spec->type == CFFormatLongType) {  // if we have precision and %d or %u, we pad 0
        padZero = true;
    }
    CFNumberFormatterSetProperty(formatter, kCFNumberFormatterPaddingCharacterKey, padZero ? CFSTR("0") : CFSTR(" "));
    
    
    // Left (default) or right padding
    SInt32 p = (spec->flags & kCFStringFormatMinusFlag) ? kCFNumberFormatterPadAfterSuffix : (padZero ? kCFNumberFormatterPadAfterPrefix : kCFNumberFormatterPadBeforePrefix);
    if (hasPrecision && spec->type == CFFormatLongType) {
        SInt32 tmpP = kCFNumberFormatterPadAfterPrefix;
        tmp = CFNumberCreate(NULL, kCFNumberSInt32Type, &tmpP);
    } else {
        tmp = CFNumberCreate(NULL, kCFNumberSInt32Type, &p);
    }
    CFNumberFormatterSetProperty(formatter, kCFNumberFormatterPaddingPositionKey, tmp);
    CFRelease(tmp);

    if (spec->numericFormatStyle == CFFormatStyleScientific) {
        prec++;  // for %e, precision+1 is the number of sig fig
        tmp = CFNumberCreate(NULL, kCFNumberSInt32Type, &prec);
        CFNumberFormatterSetProperty(formatter, kCFNumberFormatterMinSignificantDigitsKey, tmp);
        CFNumberFormatterSetProperty(formatter, kCFNumberFormatterMaxSignificantDigitsKey, tmp);
        CFRelease(tmp);
    }
    
    CFStringRef localizedNumberString = NULL;
    switch (spec->type) {
        case CFFormatLongType:
            // ??? Need to do unsigned
            localizedNumberString = CFNumberFormatterCreateStringWithValue(NULL, formatter, kCFNumberSInt64Type, &(values[spec->mainArgNum].value.int64Value));
            break;
        case CFFormatDoubleType: {
#if LONG_DOUBLE_SUPPORT
            if (CFFormatSize16 == values[spec->mainArgNum].size) {
                double doubleValue = values[spec->mainArgNum].value.longDoubleValue; // losing precision
                localizedNumberString = CFNumberFormatterCreateStringWithValue(NULL, formatter, kCFNumberDoubleType, &doubleValue);
            } else
#endif
            {
                localizedNumberString = CFNumberFormatterCreateStringWithValue(NULL, formatter, kCFNumberDoubleType, &(values[spec->mainArgNum].value.doubleValue));
            }
            break;
        }
    }
    __CFUnlock(&formatterLock);
    
    if (localizedNumberString) {
        // we need to pad space if we have %d or %u
        if (spec->type == CFFormatLongType && hasPrecision && CFStringGetLength(localizedNumberString) < width) {
            CFMutableStringRef finalStr = NULL;
            if (p == kCFNumberFormatterPadAfterSuffix) {
                finalStr = CFStringCreateMutableCopy(NULL, 0, localizedNumberString);
                CFStringPad(finalStr, CFSTR(" "), width, 0);
            } else {
                finalStr = CFStringCreateMutable(NULL, 0);
                CFStringPad(finalStr, CFSTR(" "), width - CFStringGetLength(localizedNumberString), 0);
                CFStringAppend(finalStr, localizedNumberString);
            }
            CFRelease(localizedNumberString);
            localizedNumberString = finalStr;
        }
        CFStringAppend(output, localizedNumberString);
        CFRelease(localizedNumberString);
        return true;
    }
    return false;
}
#endif

CF_INLINE void __CFParseFormatSpec(const UniChar *uformat, const uint8_t *cformat, SInt32 *fmtIdx, SInt32 fmtLen, CFFormatSpec *spec, CFStringRef *configKeyPointer) {
    Boolean seenDot = false;
    Boolean seenSharp = false;
    CFIndex keyIndex = kCFNotFound;

    for (;;) {
	UniChar ch;
	if (fmtLen <= *fmtIdx) return;	/* no type */
        if (cformat) ch = (UniChar)cformat[(*fmtIdx)++]; else ch = uformat[(*fmtIdx)++];

	if (keyIndex >= 0) {
	    if ((ch < '0') || ((ch > '9') && (ch < 'A')) || ((ch > 'Z') && (ch < 'a') && (ch != '_')) || (ch > 'z')) {
		if (ch == '@') { // found the key
		    CFIndex length = (*fmtIdx) - 1 - keyIndex;

		    spec->flags |= kCFStringFormatExternalSpecFlag;
		    spec->type = CFFormatCFType;
		    spec->size = CFFormatSizePointer;  // 4 or 8 depending on LP64

		    if ((NULL != configKeyPointer) && (length > 0)) {
			if (cformat) {
			    *configKeyPointer = CFStringCreateWithBytes(NULL, cformat + keyIndex, length, __CFStringGetEightBitStringEncoding(), FALSE);
			} else {
			    *configKeyPointer = CFStringCreateWithCharactersNoCopy(NULL, uformat + keyIndex, length, kCFAllocatorNull);
			}
		    }
		    return;
		}
		keyIndex = kCFNotFound;
	    }
	    continue;
	}

reswtch:switch (ch) {
	case '#':	// ignored for now
	    seenSharp = true;
	    break;
	case 0x20:
	    if (!(spec->flags & kCFStringFormatPlusFlag)) spec->flags |= kCFStringFormatSpaceFlag;
	    break;
	case '-':
	    spec->flags |= kCFStringFormatMinusFlag;
	    spec->flags &= ~kCFStringFormatZeroFlag;	// remove zero flag
	    break;
	case '+':
	    spec->flags |= kCFStringFormatPlusFlag;
	    spec->flags &= ~kCFStringFormatSpaceFlag;	// remove space flag
	    break;
	case '0':
            if (seenDot) {    // after we see '.' and then we see '0', it is 0 precision. We should not see '.' after '0' if '0' is the zero padding flag
                spec->precArg = 0;
                break;
            }
	    if (!(spec->flags & kCFStringFormatMinusFlag)) spec->flags |= kCFStringFormatZeroFlag;
	    break;
	case 'h':
	    if (*fmtIdx < fmtLen) {
		// fetch next character, don't increment fmtIdx
		if (cformat) ch = (UniChar)cformat[(*fmtIdx)]; else ch = uformat[(*fmtIdx)];
		if ('h' == ch) {	// 'hh' for char, like 'c'
		    (*fmtIdx)++;
		    spec->size = CFFormatSize1;
		    break;
		}
	    }
	    spec->size = CFFormatSize2;
	    break;
	case 'l':
	    if (*fmtIdx < fmtLen) {
		// fetch next character, don't increment fmtIdx
		if (cformat) ch = (UniChar)cformat[(*fmtIdx)]; else ch = uformat[(*fmtIdx)];
		if ('l' == ch) {	// 'll' for long long, like 'q'
		    (*fmtIdx)++;
		    spec->size = CFFormatSize8;
		    break;
		}
	    }
	    spec->size = CFFormatSizeLong;  // 4 or 8 depending on LP64
	    break;
#if LONG_DOUBLE_SUPPORT
	case 'L':
	    spec->size = CFFormatSize16;
	    break;
#endif
	case 'q':
	    spec->size = CFFormatSize8;
	    break;
	case 't': case 'z':
	    spec->size = CFFormatSizeLong;  // 4 or 8 depending on LP64
	    break;
	case 'j':
	    spec->size = CFFormatSize8; 
	    break;
	case 'c':
	    spec->type = CFFormatLongType;
	    spec->size = CFFormatSize1;
	    return;
        case 'D': case 'd': case 'i': case 'U': case 'u':
            // we can localize all but octal or hex
            if (_CFExecutableLinkedOnOrAfter(CFSystemVersionMountainLion)) spec->flags |= kCFStringFormatLocalizable;
            spec->numericFormatStyle = CFFormatStyleDecimal;
            if (ch == 'u' || ch == 'U') spec->numericFormatStyle = CFFormatStyleUnsigned;
            // fall thru
        case 'O': case 'o': case 'x': case 'X':
            spec->type = CFFormatLongType;
            // Seems like if spec->size == 0, we should spec->size = CFFormatSize4. However, 0 is handled correctly.
	    return;
        case 'f': case 'F': case 'g': case 'G': case 'e': case 'E': {
                // we can localize all but hex float output
                if (_CFExecutableLinkedOnOrAfter(CFSystemVersionMountainLion)) spec->flags |= kCFStringFormatLocalizable;
                char lch = (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch;
                spec->numericFormatStyle = ((lch == 'e' || lch == 'g') ? CFFormatStyleScientific : 0) | ((lch == 'f' || lch == 'g') ? CFFormatStyleDecimal : 0);
                if (seenDot && spec->precArg == -1 && spec->precArgNum == -1) { // for the cases that we have '.' but no precision followed, not even '*'
                    spec->precArg = 0;
                }
            }
            // fall thru
        case 'a': case 'A':
	    spec->type = CFFormatDoubleType;
	    if (spec->size != CFFormatSize16) spec->size = CFFormatSize8;
	    return;
	case 'n':		/* %n is not handled correctly; for Leopard or newer apps, we disable it further */
	    spec->type = 1 ? CFFormatDummyPointerType : CFFormatPointerType;
	    spec->size = CFFormatSizePointer;  // 4 or 8 depending on LP64
	    return;
	case 'p':	
	    spec->type = CFFormatPointerType;
	    spec->size = CFFormatSizePointer;  // 4 or 8 depending on LP64
	    return;
	case 's':
	    spec->type = CFFormatCharsType;
	    spec->size = CFFormatSizePointer;  // 4 or 8 depending on LP64
	    return;
	case 'S':
	    spec->type = CFFormatUnicharsType;
	    spec->size = CFFormatSizePointer;  // 4 or 8 depending on LP64
	    return;
        case 'C':
            spec->type = CFFormatSingleUnicharType;
            spec->size = CFFormatSize2;
            return;
	case 'P':
	    spec->type = CFFormatPascalCharsType;
	    spec->size = CFFormatSizePointer;  // 4 or 8 depending on LP64
	    return;
	case '@':
	    if (seenSharp) {
		seenSharp = false;
		keyIndex = *fmtIdx;
		break;
	    } else {
		spec->type = CFFormatCFType;
		spec->size = CFFormatSizePointer;  // 4 or 8 depending on LP64
		return;
	    }
	case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
	    int64_t number = 0;
	    do {
		number = 10 * number + (ch - '0');
                if (cformat) ch = (UniChar)cformat[(*fmtIdx)++]; else ch = uformat[(*fmtIdx)++];
	    } while ((UInt32)(ch - '0') <= 9);
	    if ('$' == ch) {
		if (-2 == spec->precArgNum) {
		    spec->precArgNum = (int8_t)number - 1;	// Arg numbers start from 1
		} else if (-2 == spec->widthArgNum) {
		    spec->widthArgNum = (int8_t)number - 1;	// Arg numbers start from 1
		} else {
		    spec->mainArgNum = (int8_t)number - 1;	// Arg numbers start from 1
		}
		break;
	    } else if (seenDot) {	/* else it's either precision or width */
		spec->precArg = (SInt32)number;
	    } else {
		spec->widthArg = (SInt32)number;
	    }
	    goto reswtch;
	}
	case '*':
	    spec->widthArgNum = -2;
	    break;
	case '.':
	    seenDot = true;
            if (cformat) ch = (UniChar)cformat[(*fmtIdx)++]; else ch = uformat[(*fmtIdx)++];
	    if ('*' == ch) {
		spec->precArgNum = -2;
		break;
	    }
	    goto reswtch;
	default:
	    spec->type = CFFormatLiteralType;
	    return;
	}
    }
}

/* ??? %s depends on handling of encodings by __CFStringAppendBytes
*/
void CFStringAppendFormatAndArguments(CFMutableStringRef outputString, CFDictionaryRef formatOptions, CFStringRef formatString, va_list args) {
    __CFStringAppendFormatCore(outputString, NULL, NULL, formatOptions, NULL, formatString, 0, NULL, 0, args);
}
        
// Length of the buffer to call sprintf() with
#define BUFFER_LEN 512

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define SNPRINTF(TYPE, WHAT) {				\
    TYPE value = (TYPE) WHAT;				\
    if (-1 != specs[curSpec].widthArgNum) {		\
	if (-1 != specs[curSpec].precArgNum) {		\
	    snprintf_l(buffer, BUFFER_LEN-1, NULL, formatBuffer, width, precision, value); \
	} else {					\
	    snprintf_l(buffer, BUFFER_LEN-1, NULL, formatBuffer, width, value); \
	}						\
    } else {						\
	if (-1 != specs[curSpec].precArgNum) {		\
	    snprintf_l(buffer, BUFFER_LEN-1, NULL, formatBuffer, precision, value); \
        } else {					\
	    snprintf_l(buffer, BUFFER_LEN-1, NULL, formatBuffer, value);	\
	}						\
    }}
#else
#define SNPRINTF(TYPE, WHAT) {				\
    TYPE value = (TYPE) WHAT;				\
    if (-1 != specs[curSpec].widthArgNum) {		\
	if (-1 != specs[curSpec].precArgNum) {		\
	    sprintf(buffer, formatBuffer, width, precision, value); \
	} else {					\
	    sprintf(buffer, formatBuffer, width, value); \
	}						\
    } else {						\
	if (-1 != specs[curSpec].precArgNum) {		\
	    sprintf(buffer, formatBuffer, precision, value); \
        } else {					\
	    sprintf(buffer, formatBuffer, value);	\
	}						\
    }}
#endif

void _CFStringAppendFormatAndArgumentsAux2(CFMutableStringRef outputString, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, CFStringRef formatString, va_list args) {
    __CFStringAppendFormatCore(outputString, copyDescFunc, contextDescFunc, formatOptions, NULL, formatString, 0, NULL, 0, args);
}
    
void _CFStringAppendFormatAndArgumentsAux(CFMutableStringRef outputString, CFStringRef (*copyDescFunc)(void *, const void *), CFDictionaryRef formatOptions, CFStringRef formatString, va_list args) {
    _CFStringAppendFormatAndArgumentsAux2(outputString, copyDescFunc, NULL, formatOptions, formatString, args);
}
    
static void __CFStringAppendFormatCore(CFMutableStringRef outputString, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, CFDictionaryRef stringsDictConfig, CFStringRef formatString, CFIndex initialArgPosition, const void *origValues, CFIndex originalValuesSize, va_list args) {
    SInt32 numSpecs, sizeSpecs, sizeArgNum, formatIdx, curSpec, argNum;
    CFIndex formatLen;
#define FORMAT_BUFFER_LEN 400
    const uint8_t *cformat = NULL;
    const UniChar *uformat = NULL;
    UniChar *formatChars = NULL;
    UniChar localFormatBuffer[FORMAT_BUFFER_LEN];
    
#define VPRINTF_BUFFER_LEN 61
    CFFormatSpec localSpecsBuffer[VPRINTF_BUFFER_LEN];
    CFFormatSpec *specs;
    CFPrintValue localValuesBuffer[VPRINTF_BUFFER_LEN];
    CFPrintValue *values;
    const CFPrintValue *originalValues = (const CFPrintValue *)origValues;
    CFDictionaryRef localConfigs[VPRINTF_BUFFER_LEN];
    CFDictionaryRef *configs;
    CFIndex numConfigs;
    CFAllocatorRef tmpAlloc = NULL;
    intmax_t dummyLocation;	    // A place for %n to do its thing in; should be the widest possible int value

    numSpecs = 0;
    sizeSpecs = 0;
    sizeArgNum = 0;
    numConfigs = 0;
    specs = NULL;
    values = NULL;
    configs = NULL;


    formatLen = CFStringGetLength(formatString);
    if (!CF_IS_OBJC(__kCFStringTypeID, formatString)) {
        __CFAssertIsString(formatString);
        if (!__CFStrIsUnicode(formatString)) {
            cformat = (const uint8_t *)__CFStrContents(formatString);
            if (cformat) cformat += __CFStrSkipAnyLengthByte(formatString);
        } else {
            uformat = (const UniChar *)__CFStrContents(formatString);
        }
    }
    if (!cformat && !uformat) {
        formatChars = (formatLen > FORMAT_BUFFER_LEN) ? (UniChar *)CFAllocatorAllocate(tmpAlloc = __CFGetDefaultAllocator(), formatLen * sizeof(UniChar), 0) : localFormatBuffer; 
	if (formatChars != localFormatBuffer && __CFOASafe) __CFSetLastAllocationEventName(formatChars, "CFString (temp)");
        CFStringGetCharacters(formatString, CFRangeMake(0, formatLen), formatChars);
        uformat = formatChars;
    }

    /* Compute an upper bound for the number of format specifications */
    if (cformat) {
        for (formatIdx = 0; formatIdx < formatLen; formatIdx++) if ('%' == cformat[formatIdx]) sizeSpecs++;
    } else {
        for (formatIdx = 0; formatIdx < formatLen; formatIdx++) if ('%' == uformat[formatIdx]) sizeSpecs++;
    }
    tmpAlloc = __CFGetDefaultAllocator();
    specs = ((2 * sizeSpecs + 1) > VPRINTF_BUFFER_LEN) ? (CFFormatSpec *)CFAllocatorAllocate(tmpAlloc, (2 * sizeSpecs + 1) * sizeof(CFFormatSpec), 0) : localSpecsBuffer;
    if (specs != localSpecsBuffer && __CFOASafe) __CFSetLastAllocationEventName(specs, "CFString (temp)");

    configs = ((sizeSpecs < VPRINTF_BUFFER_LEN) ? localConfigs : (CFDictionaryRef *)CFAllocatorAllocate(tmpAlloc, sizeof(CFStringRef) * sizeSpecs, 0));

    /* Collect format specification information from the format string */
    for (curSpec = 0, formatIdx = 0; formatIdx < formatLen; curSpec++) {
	SInt32 newFmtIdx;
	specs[curSpec].loc = formatIdx;
	specs[curSpec].len = 0;
	specs[curSpec].size = 0;
	specs[curSpec].type = 0;
	specs[curSpec].flags = 0;
	specs[curSpec].widthArg = -1;
	specs[curSpec].precArg = -1;
	specs[curSpec].mainArgNum = -1;
	specs[curSpec].precArgNum = -1;
	specs[curSpec].widthArgNum = -1;
	specs[curSpec].configDictIndex = -1;
        if (cformat) {
            for (newFmtIdx = formatIdx; newFmtIdx < formatLen && '%' != cformat[newFmtIdx]; newFmtIdx++);
        } else {
            for (newFmtIdx = formatIdx; newFmtIdx < formatLen && '%' != uformat[newFmtIdx]; newFmtIdx++);
        }
	if (newFmtIdx != formatIdx) {	/* Literal chunk */
	    specs[curSpec].type = CFFormatLiteralType;
	    specs[curSpec].len = newFmtIdx - formatIdx;
	} else {
	    CFStringRef configKey = NULL;
	    newFmtIdx++;	/* Skip % */
	    __CFParseFormatSpec(uformat, cformat, &newFmtIdx, formatLen, &(specs[curSpec]), &configKey);
            if (CFFormatLiteralType == specs[curSpec].type) {
		specs[curSpec].loc = formatIdx + 1;
		specs[curSpec].len = 1;
	    } else {
		specs[curSpec].len = newFmtIdx - formatIdx;
	    }
	}
	formatIdx = newFmtIdx;

// fprintf(stderr, "specs[%d] = {\n  size = %d,\n  type = %d,\n  loc = %d,\n  len = %d,\n  mainArgNum = %d,\n  precArgNum = %d,\n  widthArgNum = %d\n}\n", curSpec, specs[curSpec].size, specs[curSpec].type, specs[curSpec].loc, specs[curSpec].len, specs[curSpec].mainArgNum, specs[curSpec].precArgNum, specs[curSpec].widthArgNum);

    }
    numSpecs = curSpec;

    // Max of three args per spec, reasoning thus: 1 width, 1 prec, 1 value
    sizeArgNum = ((NULL == originalValues) ? (3 * sizeSpecs + 1) : originalValuesSize);

    values = (sizeArgNum > VPRINTF_BUFFER_LEN) ? (CFPrintValue *)CFAllocatorAllocate(tmpAlloc, sizeArgNum * sizeof(CFPrintValue), 0) : localValuesBuffer;
    if (values != localValuesBuffer && __CFOASafe) __CFSetLastAllocationEventName(values, "CFString (temp)");
    memset(values, 0, sizeArgNum * sizeof(CFPrintValue));

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
    // va_copy is a C99 extension. No support on Windows
    va_list copiedArgs;
    if (numConfigs > 0) va_copy(copiedArgs, args); // we need to preserve the original state for passing down
#endif

    /* Compute values array */
    argNum = initialArgPosition;
    for (curSpec = 0; curSpec < numSpecs; curSpec++) {
	SInt32 newMaxArgNum;
	if (0 == specs[curSpec].type) continue;
	if (CFFormatLiteralType == specs[curSpec].type) continue;
	newMaxArgNum = sizeArgNum;
	if (newMaxArgNum < specs[curSpec].mainArgNum) {
	    newMaxArgNum = specs[curSpec].mainArgNum;
	}
	if (newMaxArgNum < specs[curSpec].precArgNum) {
	    newMaxArgNum = specs[curSpec].precArgNum;
	}
	if (newMaxArgNum < specs[curSpec].widthArgNum) {
	    newMaxArgNum = specs[curSpec].widthArgNum;
	}
	if (sizeArgNum < newMaxArgNum) {
	    if (specs != localSpecsBuffer) CFAllocatorDeallocate(tmpAlloc, specs);
	    if (values != localValuesBuffer) CFAllocatorDeallocate(tmpAlloc, values);
	    if (formatChars && (formatChars != localFormatBuffer)) CFAllocatorDeallocate(tmpAlloc, formatChars);
	    return;  // more args than we expected!
	}
	/* It is actually incorrect to reorder some specs and not all; we just do some random garbage here */
	if (-2 == specs[curSpec].widthArgNum) {
	    specs[curSpec].widthArgNum = argNum++;
	}
	if (-2 == specs[curSpec].precArgNum) {
	    specs[curSpec].precArgNum = argNum++;
	}
	if (-1 == specs[curSpec].mainArgNum) {
	    specs[curSpec].mainArgNum = argNum++;
	}

	values[specs[curSpec].mainArgNum].size = specs[curSpec].size;
	values[specs[curSpec].mainArgNum].type = specs[curSpec].type;


	if (-1 != specs[curSpec].widthArgNum) {
	    values[specs[curSpec].widthArgNum].size = 0;
	    values[specs[curSpec].widthArgNum].type = CFFormatLongType;
	}
	if (-1 != specs[curSpec].precArgNum) {
	    values[specs[curSpec].precArgNum].size = 0;
	    values[specs[curSpec].precArgNum].type = CFFormatLongType;
	}
    }

    /* Collect the arguments in correct type from vararg list */
    for (argNum = 0; argNum < sizeArgNum; argNum++) {
	if ((NULL != originalValues) && (0 == values[argNum].type)) values[argNum] = originalValues[argNum];
	switch (values[argNum].type) {
	case 0:
	case CFFormatLiteralType:
	    break;
	case CFFormatLongType:
        case CFFormatSingleUnicharType:
	    if (CFFormatSize1 == values[argNum].size) {
		values[argNum].value.int64Value = (int64_t)(int8_t)va_arg(args, int);
	    } else if (CFFormatSize2 == values[argNum].size) {
		values[argNum].value.int64Value = (int64_t)(int16_t)va_arg(args, int);
	    } else if (CFFormatSize4 == values[argNum].size) {
		values[argNum].value.int64Value = (int64_t)va_arg(args, int32_t);
	    } else if (CFFormatSize8 == values[argNum].size) {
		values[argNum].value.int64Value = (int64_t)va_arg(args, int64_t);
	    } else {
		values[argNum].value.int64Value = (int64_t)va_arg(args, int);
	    }
	    break;
	case CFFormatDoubleType:
#if LONG_DOUBLE_SUPPORT
	    if (CFFormatSize16 == values[argNum].size) {
		values[argNum].value.longDoubleValue = va_arg(args, long double);
	    } else 
#endif
	    {
		values[argNum].value.doubleValue = va_arg(args, double);
	    }
	    break;
	case CFFormatPointerType:
	case CFFormatObjectType:
	case CFFormatCFType:
	case CFFormatUnicharsType:
	case CFFormatCharsType:
	case CFFormatPascalCharsType:
	    values[argNum].value.pointerValue = va_arg(args, void *);
	    break;
	case CFFormatDummyPointerType:
	    (void)va_arg(args, void *);	    // Skip the provided argument
	    values[argNum].value.pointerValue = &dummyLocation;
	    break;
	}
    }
    va_end(args);

    /* Format the pieces together */

    if (NULL == originalValues) {
	originalValues = values;
	originalValuesSize = sizeArgNum;
    }

    SInt32 numSpecsContext = 0;
    CFFormatSpec *specsContext = (CFFormatSpec *)calloc(numSpecs, sizeof(CFFormatSpec));
    static const CFStringRef replacement = CFSTR("%@NSCONTEXT");
    
    for (curSpec = 0; curSpec < numSpecs; curSpec++) {
	SInt32 width = 0, precision = 0;
	UniChar *up, ch;
	Boolean hasWidth = false, hasPrecision = false;

	// widthArgNum and widthArg are never set at the same time; same for precArg*
	if (-1 != specs[curSpec].widthArgNum) {
	    width = (SInt32)values[specs[curSpec].widthArgNum].value.int64Value;
	    hasWidth = true;
	}
	if (-1 != specs[curSpec].precArgNum) {
	    precision = (SInt32)values[specs[curSpec].precArgNum].value.int64Value;
	    hasPrecision = true;
	}
	if (-1 != specs[curSpec].widthArg) {
	    width = specs[curSpec].widthArg;
	    hasWidth = true;
	}
	if (-1 != specs[curSpec].precArg) {
	    precision = specs[curSpec].precArg;
	    hasPrecision = true;
	}

	switch (specs[curSpec].type) {
	case CFFormatLongType:
	case CFFormatDoubleType:
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
            if (formatOptions && (specs[curSpec].flags & kCFStringFormatLocalizable) && (CFGetTypeID(formatOptions) == CFLocaleGetTypeID())) {    // We have a locale, so we do localized formatting
                if (__CFStringFormatLocalizedNumber(outputString, (CFLocaleRef)formatOptions, values, &specs[curSpec], width, precision, hasPrecision)) break;
            }
            /* Otherwise fall-thru to the next case! */
#endif
         case CFFormatPointerType: {
                char formatBuffer[128];
#if defined(__GNUC__)
                char buffer[BUFFER_LEN + width + precision];
#else
                char stackBuffer[BUFFER_LEN];
                char *dynamicBuffer = NULL;
                char *buffer = stackBuffer;
                if (256+width+precision > BUFFER_LEN) {
                    dynamicBuffer = (char *)CFAllocatorAllocate(kCFAllocatorSystemDefault, 256+width+precision, 0);
                    buffer = dynamicBuffer;
                }
#endif
                SInt32 cidx, idx, loc;
		Boolean appended = false;
                loc = specs[curSpec].loc;
                // In preparation to call snprintf(), copy the format string out
                if (cformat) {
                    for (idx = 0, cidx = 0; cidx < specs[curSpec].len; idx++, cidx++) {
                        if ('$' == cformat[loc + cidx]) {
                            for (idx--; '0' <= formatBuffer[idx] && formatBuffer[idx] <= '9'; idx--);
                        } else {
                            formatBuffer[idx] = cformat[loc + cidx];
                        }
                    }
                } else {
                    for (idx = 0, cidx = 0; cidx < specs[curSpec].len; idx++, cidx++) {
                        if ('$' == uformat[loc + cidx]) {
                            for (idx--; '0' <= formatBuffer[idx] && formatBuffer[idx] <= '9'; idx--);
                        } else {
                            formatBuffer[idx] = (int8_t)uformat[loc + cidx];
                        }
                    }
                }
                formatBuffer[idx] = '\0';
		// Should modify format buffer here if necessary; for example, to translate %qd to
		// the equivalent, on architectures which do not have %q.
                buffer[sizeof(buffer) - 1] = '\0';
                switch (specs[curSpec].type) {
                    case CFFormatLongType:
                        if (CFFormatSize8 == specs[curSpec].size) {
                            SNPRINTF(int64_t, values[specs[curSpec].mainArgNum].value.int64Value)
                        } else {
                            SNPRINTF(SInt32, values[specs[curSpec].mainArgNum].value.int64Value)
                        }
                        break;
                    case CFFormatPointerType:
                    case CFFormatDummyPointerType:
                        SNPRINTF(void *, values[specs[curSpec].mainArgNum].value.pointerValue)
                        break;

                    case CFFormatDoubleType:
#if LONG_DOUBLE_SUPPORT
                        if (CFFormatSize16 == specs[curSpec].size) {
			    SNPRINTF(long double, values[specs[curSpec].mainArgNum].value.longDoubleValue)
			} else 
#endif
			{
			    SNPRINTF(double, values[specs[curSpec].mainArgNum].value.doubleValue)
			}
			// See if we need to localize the decimal point
                        if (formatOptions) {	// We have localization info
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
			    CFStringRef decimalSeparator = (CFGetTypeID(formatOptions) == CFLocaleGetTypeID()) ? (CFStringRef)CFLocaleGetValue((CFLocaleRef)formatOptions, kCFLocaleDecimalSeparatorKey) : (CFStringRef)CFDictionaryGetValue(formatOptions, CFSTR("NSDecimalSeparator"));
#else
                            CFStringRef decimalSeparator = CFSTR(".");
#endif
                            if (decimalSeparator != NULL) {	// We have a decimal separator in there
                                CFIndex decimalPointLoc = 0;
                                while (buffer[decimalPointLoc] != 0 && buffer[decimalPointLoc] != '.') decimalPointLoc++;
                                if (buffer[decimalPointLoc] == '.') {	// And we have a decimal point in the formatted string
                                    buffer[decimalPointLoc] = 0;
                                    CFStringAppendCString(outputString, (const char *)buffer, __CFStringGetEightBitStringEncoding());
                                    CFStringAppend(outputString, decimalSeparator);
                                    CFStringAppendCString(outputString, (const char *)(buffer + decimalPointLoc + 1), __CFStringGetEightBitStringEncoding());
                                    appended = true;
                                }
                            }
                        }
                        break;
                }
                if (!appended) CFStringAppendCString(outputString, (const char *)buffer, __CFStringGetEightBitStringEncoding());            
#if !defined(__GNUC__)
                if (dynamicBuffer) {
                        CFAllocatorDeallocate(kCFAllocatorSystemDefault, dynamicBuffer);
                }
#endif
            }
            break;
	case CFFormatLiteralType:
            if (cformat) {
                __CFStringAppendBytes(outputString, (const char *)(cformat+specs[curSpec].loc), specs[curSpec].len, __CFStringGetEightBitStringEncoding());
            } else {
                CFStringAppendCharacters(outputString, uformat+specs[curSpec].loc, specs[curSpec].len);
            }
	    break;
	case CFFormatPascalCharsType:
        case CFFormatCharsType:
	    if (values[specs[curSpec].mainArgNum].value.pointerValue == NULL) {
		CFStringAppendCString(outputString, "(null)", kCFStringEncodingASCII);
            } else {
                int len;
                const char *str = (const char *)values[specs[curSpec].mainArgNum].value.pointerValue;
                if (specs[curSpec].type == CFFormatPascalCharsType) {	// Pascal string case
                    len = ((unsigned char *)str)[0];
                    str++;
                    if (hasPrecision && precision < len) len = precision;
                } else {	// C-string case
                    if (!hasPrecision) {	// No precision, so rely on the terminating null character
                        len = strlen(str);
                    } else {	// Don't blindly call strlen() if there is a precision; the string might not have a terminating null (3131988)
                        const char *terminatingNull = (const char *)memchr(str, 0, precision);	// Basically strlen() on only the first precision characters of str
                        if (terminatingNull) {	// There was a null in the first precision characters
                            len = terminatingNull - str;
                        } else {
                            len = precision;
                        }
                    }
                }
		// Since the spec says the behavior of the ' ', '0', '#', and '+' flags is undefined for
		// '%s', and since we have ignored them in the past, the behavior is hereby cast in stone
		// to ignore those flags (and, say, never pad with '0' instead of space).
		if (specs[curSpec].flags & kCFStringFormatMinusFlag) {
		    __CFStringAppendBytes(outputString, str, len, __CFStringGetSystemEncoding());
		    if (hasWidth && width > len) {
			int w = width - len;	// We need this many spaces; do it ten at a time
			do {__CFStringAppendBytes(outputString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
		    }
		} else {
		    if (hasWidth && width > len) {
			int w = width - len;	// We need this many spaces; do it ten at a time
			do {__CFStringAppendBytes(outputString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
		    }
		    __CFStringAppendBytes(outputString, str, len, __CFStringGetSystemEncoding());
		}
	    }
            break;
        case CFFormatSingleUnicharType:
            ch = (UniChar)values[specs[curSpec].mainArgNum].value.int64Value;
            CFStringAppendCharacters(outputString, &ch, 1);
            break;
        case CFFormatUnicharsType:
            //??? need to handle width, precision, and padding arguments
            up = (UniChar *)values[specs[curSpec].mainArgNum].value.pointerValue;
            if (NULL == up) {
                CFStringAppendCString(outputString, "(null)", kCFStringEncodingASCII);
            } else {
                int len;
                for (len = 0; 0 != up[len]; len++);
		// Since the spec says the behavior of the ' ', '0', '#', and '+' flags is undefined for
		// '%s', and since we have ignored them in the past, the behavior is hereby cast in stone
		// to ignore those flags (and, say, never pad with '0' instead of space).
                if (hasPrecision && precision < len) len = precision;
		if (specs[curSpec].flags & kCFStringFormatMinusFlag) {
		    CFStringAppendCharacters(outputString, up, len);
		    if (hasWidth && width > len) {
			int w = width - len;	// We need this many spaces; do it ten at a time
			do {__CFStringAppendBytes(outputString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
		    }
		} else {
		    if (hasWidth && width > len) {
			int w = width - len;	// We need this many spaces; do it ten at a time
			do {__CFStringAppendBytes(outputString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
		    }
		    CFStringAppendCharacters(outputString, up, len);
		}
            }
            break;
	case CFFormatCFType:
	case CFFormatObjectType:
	    if (specs[curSpec].configDictIndex != -1) { // config dict
		CFTypeRef object = NULL;
		switch (values[specs[curSpec].mainArgNum].type) {
		    case CFFormatLongType:
			object = CFNumberCreate(tmpAlloc, kCFNumberSInt64Type, &(values[specs[curSpec].mainArgNum].value.int64Value));
			break;
			
		    case CFFormatDoubleType:
#if LONG_DOUBLE_SUPPORT
			if (CFFormatSize16 == values[specs[curSpec].mainArgNum].size) {
			    double aValue = values[specs[curSpec].mainArgNum].value.longDoubleValue; // losing precision

			    object = CFNumberCreate(tmpAlloc, kCFNumberDoubleType, &aValue);
			} else
#endif
			{
			    object = CFNumberCreate(tmpAlloc, kCFNumberDoubleType, &(values[specs[curSpec].mainArgNum].value.doubleValue));
			}
			break;

		    case CFFormatPointerType:
			object = CFNumberCreate(tmpAlloc, kCFNumberCFIndexType, &(values[specs[curSpec].mainArgNum].value.pointerValue));
			break;

		    case CFFormatPascalCharsType:
		    case CFFormatCharsType:
			if (NULL != values[specs[curSpec].mainArgNum].value.pointerValue) {
			    CFMutableStringRef aString = CFStringCreateMutable(tmpAlloc, 0);
			    int len;
			    const char *str = (const char *)values[specs[curSpec].mainArgNum].value.pointerValue;
			    if (specs[curSpec].type == CFFormatPascalCharsType) {	// Pascal string case
				len = ((unsigned char *)str)[0];
				str++;
				if (hasPrecision && precision < len) len = precision;
			    } else {	// C-string case
				if (!hasPrecision) {	// No precision, so rely on the terminating null character
				    len = strlen(str);
				} else {	// Don't blindly call strlen() if there is a precision; the string might not have a terminating null (3131988)
				    const char *terminatingNull = (const char *)memchr(str, 0, precision);	// Basically strlen() on only the first precision characters of str
				    if (terminatingNull) {	// There was a null in the first precision characters
					len = terminatingNull - str;
				    } else {
					len = precision;
				    }
				}
			    }
			    // Since the spec says the behavior of the ' ', '0', '#', and '+' flags is undefined for
			    // '%s', and since we have ignored them in the past, the behavior is hereby cast in stone
			    // to ignore those flags (and, say, never pad with '0' instead of space).
			    if (specs[curSpec].flags & kCFStringFormatMinusFlag) {
				__CFStringAppendBytes(aString, str, len, __CFStringGetSystemEncoding());
				if (hasWidth && width > len) {
				    int w = width - len;	// We need this many spaces; do it ten at a time
				    do {__CFStringAppendBytes(aString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
				}
			    } else {
				if (hasWidth && width > len) {
				    int w = width - len;	// We need this many spaces; do it ten at a time
				    do {__CFStringAppendBytes(aString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
				}
				__CFStringAppendBytes(aString, str, len, __CFStringGetSystemEncoding());
			    }

			    object = aString;
			}
			break;

		    case CFFormatSingleUnicharType:
			ch = (UniChar)values[specs[curSpec].mainArgNum].value.int64Value;
			object = CFStringCreateWithCharactersNoCopy(tmpAlloc, &ch, 1, kCFAllocatorNull);
			break;

		    case CFFormatUnicharsType:
			//??? need to handle width, precision, and padding arguments
			up = (UniChar *)values[specs[curSpec].mainArgNum].value.pointerValue;
			if (NULL != up) {
			    CFMutableStringRef aString = CFStringCreateMutable(tmpAlloc, 0);
			    int len;
			    for (len = 0; 0 != up[len]; len++);
			    // Since the spec says the behavior of the ' ', '0', '#', and '+' flags is undefined for
			    // '%s', and since we have ignored them in the past, the behavior is hereby cast in stone
			    // to ignore those flags (and, say, never pad with '0' instead of space).
			    if (hasPrecision && precision < len) len = precision;
			    if (specs[curSpec].flags & kCFStringFormatMinusFlag) {
				CFStringAppendCharacters(aString, up, len);
				if (hasWidth && width > len) {
				    int w = width - len;	// We need this many spaces; do it ten at a time
				    do {__CFStringAppendBytes(aString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
				}
			    } else {
				if (hasWidth && width > len) {
				    int w = width - len;	// We need this many spaces; do it ten at a time
				    do {__CFStringAppendBytes(aString, "          ", (w > 10 ? 10 : w), kCFStringEncodingASCII);} while ((w -= 10) > 0);
				}
				CFStringAppendCharacters(aString, up, len);
			    }
			    object = aString;
			}
			break;

		    case CFFormatCFType:
		    case CFFormatObjectType:
			if (NULL != values[specs[curSpec].mainArgNum].value.pointerValue) object = CFRetain(values[specs[curSpec].mainArgNum].value.pointerValue);
			break;
		}

		if (NULL != object) CFRelease(object);

	    } else if (NULL != values[specs[curSpec].mainArgNum].value.pointerValue) {
                CFStringRef str = NULL;
                if (contextDescFunc) {
                    bool found = NO;
                    str = contextDescFunc(values[specs[curSpec].mainArgNum].value.pointerValue, formatString, replacement, NO, &found);
                    if (found) {
                        str = CFRetain(replacement);
                        specsContext[numSpecsContext] = specs[curSpec];
                        numSpecsContext++;
                    }
                }
                if (!str) {
                    if (copyDescFunc) {
                        str = copyDescFunc(values[specs[curSpec].mainArgNum].value.pointerValue, formatOptions);
                    } else {
                        str = __CFCopyFormattingDescription(values[specs[curSpec].mainArgNum].value.pointerValue, formatOptions);
                        if (NULL == str) {
                            str = CFCopyDescription(values[specs[curSpec].mainArgNum].value.pointerValue);
                        }
                    }
                }
                if (str) {
                    CFStringAppend(outputString, str);
                    CFRelease(str);
                } else {
                    CFStringAppendCString(outputString, "(null description)", kCFStringEncodingASCII);
                }
            } else {
		CFStringAppendCString(outputString, "(null)", kCFStringEncodingASCII);
            }
            break;
        }
    }
    
    for (SInt32 i = 0; i < numSpecsContext; i++) {
        CFRange range = CFStringFind(outputString, replacement, 0);
        CFStringRef str = contextDescFunc(values[specsContext[i].mainArgNum].value.pointerValue, outputString, replacement, true, NULL);
        if (str) {
            CFStringReplace(outputString, range, str);
            CFRelease(str);
        }
    }
    
    free(specsContext);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
    // va_copy is a C99 extension. No support on Windows
    if (numConfigs > 0) va_end(copiedArgs);
#endif 
    if (specs != localSpecsBuffer) CFAllocatorDeallocate(tmpAlloc, specs);
    if (values != localValuesBuffer) CFAllocatorDeallocate(tmpAlloc, values);
    if (formatChars && (formatChars != localFormatBuffer)) CFAllocatorDeallocate(tmpAlloc, formatChars);
    if (configs != localConfigs) CFAllocatorDeallocate(tmpAlloc, configs);
}

#undef SNPRINTF

void CFShowStr(CFStringRef str) {
    CFAllocatorRef alloc;

    if (!str) {
	fprintf(stdout, "(null)\n");
	return;
    }

    if (CF_IS_OBJC(__kCFStringTypeID, str)) {
        fprintf(stdout, "This is an NSString, not CFString\n");
        return;
    }

    alloc = CFGetAllocator(str);

    fprintf(stdout, "\nLength %d\nIsEightBit %d\n", (int)__CFStrLength(str), __CFStrIsEightBit(str));
    fprintf(stdout, "HasLengthByte %d\nHasNullByte %d\nInlineContents %d\n",
            __CFStrHasLengthByte(str), __CFStrHasNullByte(str), __CFStrIsInline(str));

    fprintf(stdout, "Allocator ");
    if (alloc != kCFAllocatorSystemDefault) {
        fprintf(stdout, "%p\n", (void *)alloc);
    } else {
        fprintf(stdout, "SystemDefault\n");
    }
    fprintf(stdout, "Mutable %d\n", __CFStrIsMutable(str));
    if (!__CFStrIsMutable(str) && __CFStrHasContentsDeallocator(str)) {
        if (__CFStrContentsDeallocator(str)) fprintf(stdout, "ContentsDeallocatorFunc %p\n", (void *)__CFStrContentsDeallocator(str));
        else fprintf(stdout, "ContentsDeallocatorFunc None\n");
    } else if (__CFStrIsMutable(str) && __CFStrHasContentsAllocator(str)) {
        fprintf(stdout, "ExternalContentsAllocator %p\n", (void *)__CFStrContentsAllocator((CFMutableStringRef)str));
    }

    if (__CFStrIsMutable(str)) {
        fprintf(stdout, "CurrentCapacity %d\n%sCapacity %d\n", (int)__CFStrCapacity(str), __CFStrIsFixed(str) ? "Fixed" : "Desired", (int)__CFStrDesiredCapacity(str));
    }
    fprintf(stdout, "Contents %p\n", (void *)__CFStrContents(str));
}



#undef HANGUL_SBASE
#undef HANGUL_LBASE
#undef HANGUL_VBASE
#undef HANGUL_TBASE
#undef HANGUL_SCOUNT
#undef HANGUL_LCOUNT
#undef HANGUL_VCOUNT
#undef HANGUL_TCOUNT
#undef HANGUL_NCOUNT

