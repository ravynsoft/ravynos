/*	ForFoundationOnly.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

// The header file ForFoundationOnly.h is for the exclusive use of the
// CoreFoundation and Foundation projects.  No other project should include it.

#if !defined(__COREFOUNDATION_FORFOUNDATIONONLY__)
#define __COREFOUNDATION_FORFOUNDATIONONLY__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFStringEncodingExt.h>
#include <CoreFoundation/CFStringEncodingConverterExt.h>
#include <CoreFoundation/CFNumberFormatter.h>
#include <CoreFoundation/CFBag.h>
#include <CoreFoundation/CFCalendar.h>

#if !TARGET_OS_WASI
#include <CoreFoundation/CFStreamPriv.h>
#include <CoreFoundation/CFRuntime.h>
#endif
#include <math.h>
#include <limits.h>

#define _CF_EXPORT_SCOPE_BEGIN \
CF_EXTERN_C_BEGIN \
CF_ASSUME_NONNULL_BEGIN \
CF_IMPLICIT_BRIDGING_ENABLED


#define _CF_EXPORT_SCOPE_END \
CF_EXTERN_C_END \
CF_ASSUME_NONNULL_END \
CF_IMPLICIT_BRIDGING_DISABLED

// NOTE: miscellaneous declarations are at the end

#pragma mark - CFRuntime

#if TARGET_OS_LINUX
#include <malloc.h>
#elif TARGET_OS_BSD
#include <stdlib.h> // malloc()
#elif TARGET_OS_MAC
#include <malloc.h>
#include <mach/mach_time.h>
#endif

#if (INCLUDE_OBJC || TARGET_OS_MAC || TARGET_OS_WIN32) && !DEPLOYMENT_RUNTIME_SWIFT
#include <objc/message.h>
#endif

#if __BLOCKS__
/* These functions implement standard error handling for reallocation. Their parameters match their unsafe variants (realloc/CFAllocatorReallocate). They differ from reallocf as they provide a chance for you to clean up a buffers contents (in addition to freeing the buffer, etc.)
 
   The optional reallocationFailureHandler is called only when the reallocation fails (with the original buffer passed in, so you can clean up the buffer/throw/abort/etc.
 
   The outRecovered BOOL can be used to control what happens after the handler is called. If left unset, and the reallocationFailureHandler does unwind-the-stack/abort, we use the standard CF out-of-memory flow.  If set to YES, then __CFSafelyReallocate{,WithAllocator} will return NULL.
 */
CF_EXPORT void *_Nonnull __CFSafelyReallocate(void * _Nullable destination, size_t newCapacity, void (^_Nullable reallocationFailureHandler)(void *_Nonnull original, bool *_Nonnull outRecovered));
CF_EXPORT void *_Nonnull __CFSafelyReallocateWithAllocator(CFAllocatorRef _Nullable, void * _Nullable destination, size_t newCapacity, CFOptionFlags options, void (^_Nullable reallocationFailureHandler)(void *_Nonnull original, bool *_Nonnull outRecovered));
#endif

Boolean __CFAllocatorRespectsHintZeroWhenAllocating(CFAllocatorRef _Nullable allocator);
typedef CF_ENUM(CFOptionFlags, _CFAllocatorHint) {
    _CFAllocatorHintZeroWhenAllocating = 1
};

// Arguments to these are id, but this header is non-Objc
#ifdef __OBJC__
#define NSISARGTYPE id _Nullable
#else
#define NSISARGTYPE void * _Nullable
#define BOOL _Bool
#endif

CF_EXPORT BOOL _NSIsNSArray(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSData(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSDate(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSDictionary(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSObject(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSOrderedSet(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSNumber(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSSet(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSString(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSTimeZone(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSValue(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSCFConstantString(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSIndexSet(NSISARGTYPE arg);
CF_EXPORT BOOL _NSIsNSAttributedString(NSISARGTYPE arg);

#if !TARGET_OS_WASI
#pragma mark - CFBundle

#include <CoreFoundation/CFBundlePriv.h>

#define _CFBundleDefaultStringTableName CFSTR("Localizable")

_CF_EXPORT_SCOPE_BEGIN

CF_EXPORT const CFStringRef _kCFBundleExecutablePathKey;
CF_EXPORT const CFStringRef _kCFBundleInfoPlistURLKey;
CF_EXPORT const CFStringRef _kCFBundleRawInfoPlistURLKey;
CF_EXPORT const CFStringRef _kCFBundleNumericVersionKey;
CF_EXPORT const CFStringRef _kCFBundleResourcesFileMappedKey;
CF_EXPORT const CFStringRef _kCFBundleAllowMixedLocalizationsKey;
CF_EXPORT const CFStringRef _kCFBundlePrincipalClassKey;

#if __BLOCKS__
CF_EXPORT CFTypeRef _Nullable _CFBundleCopyFindResources(CFBundleRef _Nullable bundle, CFURLRef _Nullable bundleURL, CFArrayRef _Nullable _unused_pass_null_, CFStringRef _Nullable resourceName, CFStringRef _Nullable resourceType, CFStringRef _Nullable subPath, CFStringRef _Nullable lproj, Boolean returnArray, Boolean localized, Boolean (^_Nullable predicate)(CFStringRef filename, Boolean *_Nullable stop));
#endif

CF_EXPORT Boolean _CFBundleLoadExecutableAndReturnError(CFBundleRef bundle, Boolean forceGlobal, CFErrorRef _Nullable *_Nullable error);
CF_EXPORT CFErrorRef _CFBundleCreateError(CFAllocatorRef _Nullable allocator, CFBundleRef bundle, CFIndex code);

_CF_EXPORT_SCOPE_END
#endif

#pragma mark - CFUUID

_CF_EXPORT_SCOPE_BEGIN

/// Compares UUID bytes using a secure constant-time comparison rdar://47657832.
/// Ensure that `lhs` and `rhs` are 128-bytes (`CFUUIDBytes` or `uuid_t`) for the comparision to be valid.
CF_INLINE Boolean __CFisEqualUUIDBytes(const void * const lhs, const void * const rhs) {
    uint64_t lhsBytes[2];
    memcpy(lhsBytes, lhs, sizeof(lhsBytes));

    uint64_t rhsBytes[2];
    memcpy(rhsBytes, rhs, sizeof(rhsBytes));

    uint64_t const equal = (lhsBytes[0] ^ rhsBytes[0]) | (lhsBytes[1] ^ rhsBytes[1]);
    return equal == 0;
}

/// Compares UUID bytes order using a secure constant-time comparison
/// Ensure that `lhs` and `rhs` are 128-bytes (`CFUUIDBytes` or `uuid_t`) for the comparison to be valid.
CF_INLINE int __CFCompareUUIDBytes(const void * const lhs, const void * const rhs) {
    unsigned char lhsCharBytes[16];
    memcpy(lhsCharBytes, lhs, sizeof(lhsCharBytes));
    
    unsigned char rhsCharBytes[16];
    memcpy(rhsCharBytes, rhs, sizeof(rhsCharBytes));
    
    int result = 0, diff;
    for (int i = sizeof(lhsCharBytes) - 1; i >= 0; i--) {
        diff = lhsCharBytes[i] - rhsCharBytes[i];
        // Constant time, no branching equivalent of
        // if (diff != 0) {
        //     result = diff;
        // }
        result = (result & -!diff) | diff;
    }
    
    return result;
}

_CF_EXPORT_SCOPE_END

#pragma mark - CFPreferences

#define DEBUG_PREFERENCES_MEMORY 0
 
#if DEBUG_PREFERENCES_MEMORY
#include "../Tests/CFCountingAllocator.h"
#endif

_CF_EXPORT_SCOPE_BEGIN

typedef struct {
    void *_Null_unspecified 	(*_Null_unspecified createDomain)(CFAllocatorRef _Nullable allocator, CFTypeRef context);
    void	(*_Null_unspecified freeDomain)(CFAllocatorRef _Nullable allocator, CFTypeRef context, void *domain);
    CFTypeRef _Null_unspecified 	(*_Null_unspecified fetchValue)(CFTypeRef context, void *domain, CFStringRef key); // Caller releases
    void	(*_Null_unspecified writeValue)(CFTypeRef context, void *domain, CFStringRef key, CFTypeRef value);
    Boolean	(*_Null_unspecified synchronize)(CFTypeRef context, void *domain);
    void	(*_Null_unspecified getKeysAndValues)(CFAllocatorRef _Nullable alloc, CFTypeRef context, void *domain, void *_Null_unspecified * _Null_unspecified buf[_Null_unspecified], CFIndex *numKeyValuePairs);
    CFDictionaryRef _Null_unspecified  (*_Null_unspecified copyDomainDictionary)(CFTypeRef context, void *domain);
    /* HACK - this is to work around the fact that individual domains lose the information about their user/host/app triplet at creation time.  We should find a better way to propagate this information. */
    void	(*setIsWorldReadable)(CFTypeRef context, void *domain, Boolean isWorldReadable);
} _CFPreferencesDomainCallBacks;

typedef struct CF_BRIDGED_MUTABLE_TYPE(id) __CFPreferencesDomain * CFPreferencesDomainRef;

typedef struct {
    CFMutableArrayRef _search;  // the search list; an array of _CFPreferencesDomains
    CFMutableDictionaryRef _dictRep; // Mutable; a collapsed view of the search list, expressed as a single dictionary
    CFStringRef _appName;
} _CFApplicationPreferences;

CF_EXPORT Boolean _CFPreferencesGetBooleanValueWithValue(CFPropertyListRef _Nullable value, Boolean * _Nullable keyExistsAndHasValidFormat);

_CF_EXPORT_SCOPE_END



#pragma mark - CFString

#include <CoreFoundation/CFStringEncodingExt.h>

#define NSSTRING_BOUNDSERROR do {\
    _CFThrowFormattedException((CFStringRef)NSInvalidArgumentException, CFSTR("%@: Range or index out of bounds"), __CFExceptionProem(self, _cmd));\
} while (0)

#define NSSTRING_RANGEERROR(range, len) do { \
    _CFThrowFormattedException((CFStringRef)NSInvalidArgumentException, CFSTR("%@: Range {%lu, %lu} out of bounds; string length %lu%s"), __CFExceptionProem((id)self, _cmd), (unsigned long)range.location, (unsigned long)range.length, (unsigned long)len, ((range.length == __kCFStringInlineBufferLength) ? " (Note that the indicated range may be smaller than the original range passed to the API)" : ""));\
} while (0)

#define NSSTRING_INDEXERROR(index, len) do { \
    _CFThrowFormattedException((CFStringRef)NSInvalidArgumentException, CFSTR("%@: Index %lu out of bounds; string length %lu"), __CFExceptionProem((id)self, _cmd), (unsigned long)index, (unsigned long)len);\
} while(0)

// This can be made into an exception for post-10.10 apps
#define NSSTRING_POSSIBLE_RANGEERROR(range, len)     \
    do {       \
        static bool warnonce = false;   \
        if (!warnonce) {                \
            warnonce = true;            \
            CFLog(kCFLogLevelWarning, CFSTR("*** %@: Range {%lu, %lu} out of bounds; string length %lu. This will become an exception for apps linked after 10.10 and iOS 8. Warning shown once per app execution."), __CFExceptionProem((id)self, _cmd), (unsigned long)range.location, (unsigned long)range.length, (unsigned long)len);        \
    }   \
} while(0)

#define NSSTRING_ILLEGALREQUESTERROR do {\
    _CFThrowFormattedException((CFStringRef)NSInvalidArgumentException, CFSTR("Can't call %s in %@"), sel_getName(_cmd), object_getClass((id)self));\
} while(0)

#define NSSTRING_INVALIDMUTATIONERROR  do {\
    _CFThrowFormattedException((CFStringRef)NSInvalidArgumentException, CFSTR("Attempt to mutate immutable object with %s"), sel_getName(_cmd));\
} while(0)

#define NSSTRING_NULLCSTRINGERROR do {\
    _CFThrowFormattedException((CFStringRef)NSInvalidArgumentException, CFSTR("%@: NULL cString"), __CFExceptionProem((id)self, _cmd));\
} while(0)

#define NSSTRING_NILSTRINGERROR do {\
    _CFThrowFormattedException((CFStringRef)NSInvalidArgumentException, CFSTR("%@: nil argument"), __CFExceptionProem(self, _cmd));\
} while(0)


_CF_EXPORT_SCOPE_BEGIN

/* Create a byte stream from a CFString backing. Can convert a string piece at a
   time into a fixed size buffer. Returns number of characters converted.
   Characters that cannot be converted to the specified encoding are represented
   with the char specified by lossByte; if 0, then lossy conversion is not allowed
   and conversion stops, returning partial results.
   generatingExternalFile indicates that any extra stuff to allow this data to be
   persistent (for instance, BOM) should be included. 
   Pass buffer==NULL if you don't care about the converted string (but just the
   convertability, or number of bytes required, indicated by usedBufLen).
   Does not zero-terminate. If you want to create Pascal or C string, allow one
   extra byte at start or end.
*/
CF_EXPORT CFIndex __CFStringEncodeByteStream(CFStringRef string, CFIndex rangeLoc, CFIndex rangeLen, Boolean generatingExternalFile, CFStringEncoding encoding, uint8_t lossByte, UInt8 *_Nullable buffer, CFIndex max, CFIndex *_Nullable usedBufLen);

CF_EXPORT CFStringRef __CFStringCreateImmutableFunnel2(CFAllocatorRef _Nullable alloc, const void *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean possiblyExternalFormat, Boolean tryToReduceUnicode, Boolean hasLengthByte, Boolean hasNullByte, Boolean noCopy, CFAllocatorRef _Nullable contentsDeallocator);

CF_EXPORT void __CFStringAppendBytes(CFMutableStringRef str, const char *cStr, CFIndex appendedLength, CFStringEncoding encoding);

CF_INLINE Boolean __CFStringEncodingIsSupersetOfASCII(CFStringEncoding encoding) {
    switch (encoding & 0x0000FF00) {
	case 0x0: // MacOS Script range
            // Symbol & bidi encodings are not ASCII superset
            if (encoding == kCFStringEncodingMacJapanese || encoding == kCFStringEncodingMacArabic || encoding == kCFStringEncodingMacHebrew || encoding == kCFStringEncodingMacUkrainian || encoding == kCFStringEncodingMacSymbol || encoding == kCFStringEncodingMacDingbats) return false;
            return true;

        case 0x100: // Unicode range
            if (encoding != kCFStringEncodingUTF8) return false;
            return true;

        case 0x200: // ISO range
            if (encoding == kCFStringEncodingISOLatinArabic) return false;
            return true;
            
        case 0x600: // National standards range
            if (encoding != kCFStringEncodingASCII) return false;
            return true;

        case 0x800: // ISO 2022 range
            return false; // It's modal encoding

        case 0xA00: // Misc standard range
            if ((encoding == kCFStringEncodingShiftJIS) || (encoding == kCFStringEncodingHZ_GB_2312) || (encoding == kCFStringEncodingUTF7_IMAP)) return false;
            return true;

        case 0xB00:
            if (encoding == kCFStringEncodingNonLossyASCII) return false;
            return true;

        case 0xC00: // EBCDIC
            return false;

        default:
            return ((encoding & 0x0000FF00) > 0x0C00 ? false : true);
    }
}


/* Desperately using extern here */
CF_EXPORT CFStringEncoding __CFDefaultEightBitStringEncoding;
CF_EXPORT CFStringEncoding __CFStringComputeEightBitStringEncoding(void);

CF_INLINE CFStringEncoding __CFStringGetEightBitStringEncoding(void) {
    if (__CFDefaultEightBitStringEncoding == kCFStringEncodingInvalidId) __CFStringComputeEightBitStringEncoding();
    return __CFDefaultEightBitStringEncoding;
}

enum {
     __kCFVarWidthLocalBufferSize = 1008
};

typedef struct {      /* A simple struct to maintain ASCII/Unicode versions of the same buffer. */
     union {
        UInt8 * _Nullable ascii;
	UniChar * _Nullable unicode;
    } chars;
    Boolean isASCII;	/* This really does mean 7-bit ASCII, not _NSDefaultCStringEncoding() */
    Boolean shouldFreeChars;	/* If the number of bytes exceeds __kCFVarWidthLocalBufferSize, bytes are allocated */
    Boolean _unused1;
    Boolean _unused2;
    CFAllocatorRef _Nullable allocator;	/* Use this allocator to allocate, reallocate, and deallocate the bytes */
    CFIndex numChars;	/* This is in terms of ascii or unicode; that is, if isASCII, it is number of 7-bit chars; otherwise it is number of UniChars; note that the actual allocated space might be larger */
    UInt8 localBuffer[__kCFVarWidthLocalBufferSize];	/* private; 168 ISO2022JP chars, 504 Unicode chars, 1008 ASCII chars */
} CFVarWidthCharBuffer;


/* Convert a byte stream to ASCII (7-bit!) or Unicode, with a CFVarWidthCharBuffer struct on the stack. false return indicates an error occurred during the conversion. Depending on .isASCII, follow .chars.ascii or .chars.unicode.  If .shouldFreeChars is returned as true, free the returned buffer when done with it.  If useClientsMemoryPtr is provided as non-NULL, and the provided memory can be used as is, this is set to true, and the .ascii or .unicode buffer in CFVarWidthCharBuffer is set to bytes.
!!! If the stream is Unicode and has no BOM, the data is assumed to be big endian! Could be trouble on Intel if someone didn't follow that assumption.
!!! __CFStringDecodeByteStream2() needs to be deprecated and removed post-Jaguar.
*/
CF_EXPORT Boolean __CFStringDecodeByteStream2(const UInt8 *bytes, UInt32 len, CFStringEncoding encoding, Boolean alwaysUnicode, CFVarWidthCharBuffer *buffer, Boolean *_Nullable useClientsMemoryPtr);
CF_EXPORT Boolean __CFStringDecodeByteStream3(const UInt8 *bytes, CFIndex len, CFStringEncoding encoding, Boolean alwaysUnicode, CFVarWidthCharBuffer *buffer, Boolean *_Nullable useClientsMemoryPtr, UInt32 converterFlags);


/* Convert single byte to Unicode; assumes one-to-one correspondence (that is, can only be used with 1-byte encodings). You can use the function if it's not NULL.
*/
CF_EXPORT CFStringEncodingCheapEightBitToUnicodeProc __CFCharToUniCharFunc;

/* Built-in constant char to unichar tables that help avoid dynamic allocation and dirtying of memory in common scenarios */
#if TARGET_OS_OSX || TARGET_OS_IPHONE
CF_EXPORT UniChar const __CFMacRomanCharToUnicharTable[256];
#endif
CF_EXPORT UniChar const __CFIdempotentCharToUniCharTable[256];

/* Character class functions UnicodeData-2_1_5.txt
*/
CF_INLINE Boolean __CFIsWhitespace(UniChar theChar) {
    return ((theChar < 0x21) || (theChar > 0x7E && theChar < 0xA1) || (theChar >= 0x2000 && theChar <= 0x200B) || (theChar == 0x3000)) ? true : false;
}

/* Same as CFStringGetCharacterFromInlineBuffer() but returns 0xFFFF on out of bounds access
*/
CF_INLINE UniChar __CFStringGetCharacterFromInlineBufferAux(CFStringInlineBuffer *buf, CFIndex idx) {
    if (idx < 0 || idx >= buf->rangeToBuffer.length) return 0xFFFF;
    if (buf->directUniCharBuffer) return buf->directUniCharBuffer[idx + buf->rangeToBuffer.location];
    if (buf->directCStringBuffer) return (UniChar)(buf->directCStringBuffer[idx + buf->rangeToBuffer.location]);
    if (idx >= buf->bufferedRangeEnd || idx < buf->bufferedRangeStart) {
	if ((buf->bufferedRangeStart = idx - 4) < 0) buf->bufferedRangeStart = 0;
	buf->bufferedRangeEnd = buf->bufferedRangeStart + __kCFStringInlineBufferLength;
	if (buf->bufferedRangeEnd > buf->rangeToBuffer.length) buf->bufferedRangeEnd = buf->rangeToBuffer.length;
	CFStringGetCharacters(buf->theString, CFRangeMake(buf->rangeToBuffer.location + buf->bufferedRangeStart, buf->bufferedRangeEnd - buf->bufferedRangeStart), buf->buffer);
    }
    return buf->buffer[idx - buf->bufferedRangeStart];
}

/* Same as CFStringGetCharacterFromInlineBuffer(), but without the bounds checking (will return garbage or crash)
*/
CF_INLINE UniChar __CFStringGetCharacterFromInlineBufferQuick(CFStringInlineBuffer *buf, CFIndex idx) {
    if (buf->directUniCharBuffer) return buf->directUniCharBuffer[idx + buf->rangeToBuffer.location];
    if (buf->directCStringBuffer) return (UniChar)(buf->directCStringBuffer[idx + buf->rangeToBuffer.location]);
    if (idx >= buf->bufferedRangeEnd || idx < buf->bufferedRangeStart) {
	if ((buf->bufferedRangeStart = idx - 4) < 0) buf->bufferedRangeStart = 0;
	buf->bufferedRangeEnd = buf->bufferedRangeStart + __kCFStringInlineBufferLength;
	if (buf->bufferedRangeEnd > buf->rangeToBuffer.length) buf->bufferedRangeEnd = buf->rangeToBuffer.length;
	CFStringGetCharacters(buf->theString, CFRangeMake(buf->rangeToBuffer.location + buf->bufferedRangeStart, buf->bufferedRangeEnd - buf->bufferedRangeStart), buf->buffer);
    }
    return buf->buffer[idx - buf->bufferedRangeStart];
}

/*
 This behaves exactly like CFStringGetCStringPtr except in three ways:
 1) It allows specifying that the resulting C string is not required to be terminated
 2) It's faster
 3) It *only* works on real CFStrings (e.g. _NSCFString, _NSCFConstantString), not NSString subclasses or tagged pointer strings
 Generally this means it can only be safely used if preceded by a CF_IS_OBJC check (which CFStringGetCStringPtr normally does itself) or from inside _NSCF{Constant}String
 */
const char * _CFNonObjCStringGetCStringPtr(CFStringRef str, CFStringEncoding encoding, Boolean requiresNullTermination);

/* These two allow specifying an alternate description function (instead of CFCopyDescription); used by NSString
*/
CF_EXPORT void _CFStringAppendFormatAndArgumentsAux(CFMutableStringRef outputString, CFStringRef _Nonnull (*_Nullable copyDescFunc)(void *, const void *loc), CFDictionaryRef _Nullable formatOptions, CFStringRef formatString, va_list args);
CF_EXPORT CFStringRef  _CFStringCreateWithFormatAndArgumentsAux(CFAllocatorRef _Nullable alloc, CFStringRef _Nonnull (*_Nullable copyDescFunc)(void *, const void *loc), CFDictionaryRef _Nullable formatOptions, CFStringRef format, va_list arguments);

CF_EXPORT void _CFStringAppendFormatAndArgumentsAux2(CFMutableStringRef outputString, CFStringRef _Nonnull (*_Nullable copyDescFunc)(void *, const void *loc), CFStringRef _Nonnull (*_Nullable contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef _Nullable formatOptions, CFStringRef formatString, va_list args);
CF_EXPORT CFStringRef _Nullable _CFStringCreateWithFormatAndArgumentsAux2(CFAllocatorRef _Nullable alloc, CFStringRef _Nonnull (*_Nullable copyDescFunc)(void *, const void *loc), CFStringRef _Nonnull (*_Nullable contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef _Nullable formatOptions, CFStringRef format, va_list arguments);
CF_EXPORT CFStringRef _Nullable CFStringCreateStringWithValidatedFormat(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef validFormatSpecifiers, CFStringRef format, va_list arguments, CFErrorRef _Nullable *_Nullable errorPtr) API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));

CF_EXPORT CFDictionaryRef _Nullable _CFStringGetFormatSpecifierConfiguration(CFStringRef aFormatString);

CF_EXPORT CFStringRef const _kCFStringFormatMetadataReplacementIndexKey;
CF_EXPORT CFStringRef const _kCFStringFormatMetadataSpecifierRangeLocationInFormatStringKey;
CF_EXPORT CFStringRef const _kCFStringFormatMetadataSpecifierRangeLengthInFormatStringKey;
CF_EXPORT CFStringRef const _kCFStringFormatMetadataReplacementRangeLocationKey;
CF_EXPORT CFStringRef const _kCFStringFormatMetadataReplacementRangeLengthKey;
CF_EXPORT CFStringRef const _kCFStringFormatMetadataArgumentObjectKey;
CF_EXPORT CFStringRef const _kCFStringFormatMetadataArgumentNumberKey;
CF_EXPORT CFStringRef _Nullable _CFStringCreateWithFormatAndArgumentsReturningMetadata(CFAllocatorRef _Nullable alloc, CFStringRef _Nonnull (*_Nullable copyDescFunc)(void *, const void *loc), CFStringRef _Nonnull (*_Nullable contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef _Nullable formatOptions, CFDictionaryRef _Nullable formatConfiguration, CFStringRef format, CFArrayRef _Nullable *_Nullable outMetadata, va_list arguments);

/* For NSString (and NSAttributedString) usage, mutate with isMutable check
*/
enum {_CFStringErrNone = 0, _CFStringErrNotMutable = 1, _CFStringErrNilArg = 2, _CFStringErrBounds = 3};
CF_EXPORT int __CFStringCheckAndReplace(CFMutableStringRef str, CFRange range, CFStringRef replacement);
CF_EXPORT Boolean __CFStringNoteErrors(void);		// Always returns `true`.

/* For NSString usage, guarantees that the contents can be extracted as 8-bit bytes in the __CFStringGetEightBitStringEncoding().
*/
CF_EXPORT Boolean __CFStringIsEightBit(CFStringRef str);

/* For NSCFString usage, these do range check (where applicable) but don't check for ObjC dispatch
*/
CF_EXPORT int _CFStringCheckAndGetCharacterAtIndex(CFStringRef str, CFIndex idx, UniChar *ch);
CF_EXPORT int _CFStringCheckAndGetCharacters(CFStringRef str, CFRange range, UniChar *buffer);
CF_EXPORT CFIndex _CFStringGetLength2(CFStringRef str);
CF_EXPORT CFHashCode __CFStringHash(CFTypeRef cf);
CF_EXPORT CFHashCode CFStringHashISOLatin1CString(const uint8_t *bytes, CFIndex len);
CF_EXPORT CFHashCode CFStringHashCString(const uint8_t *bytes, CFIndex len);
CF_EXPORT CFHashCode CFStringHashCharacters(const UniChar *characters, CFIndex len);
CF_EXPORT CFHashCode CFStringHashNSString(CFStringRef str);


_CF_EXPORT_SCOPE_END


#pragma mark - Binary plist

_CF_EXPORT_SCOPE_BEGIN
typedef const struct CF_BRIDGED_TYPE(_NSKeyedArchiverUID) __CFKeyedArchiverUID * CFKeyedArchiverUIDRef;
CF_EXPORT CFTypeID _CFKeyedArchiverUIDGetTypeID(void);
CF_EXPORT CFKeyedArchiverUIDRef _Nullable _CFKeyedArchiverUIDCreate(CFAllocatorRef _Nullable allocator, uint32_t value);
CF_EXPORT uint32_t _CFKeyedArchiverUIDGetValue(CFKeyedArchiverUIDRef uid);


enum {
    kCFBinaryPlistMarkerNull = 0x00,
    kCFBinaryPlistMarkerFalse = 0x08,
    kCFBinaryPlistMarkerTrue = 0x09,
    kCFBinaryPlistMarkerFill = 0x0F,
    kCFBinaryPlistMarkerInt = 0x10,
    kCFBinaryPlistMarkerReal = 0x20,
    kCFBinaryPlistMarkerDate = 0x33,
    kCFBinaryPlistMarkerData = 0x40,
    kCFBinaryPlistMarkerASCIIString = 0x50,
    kCFBinaryPlistMarkerUnicode16String = 0x60,
    kCFBinaryPlistMarkerUID = 0x80,
    kCFBinaryPlistMarkerArray = 0xA0,
    kCFBinaryPlistMarkerSet = 0xC0,
    kCFBinaryPlistMarkerDict = 0xD0
};

typedef struct {
    uint8_t	_magic[6];
    uint8_t	_version[2];
} CFBinaryPlistHeader;

typedef struct {
    uint8_t	_unused[5];
    uint8_t     _sortVersion;
    uint8_t	_offsetIntSize;
    uint8_t	_objectRefSize;
    uint64_t	_numObjects;
    uint64_t	_topObject;
    uint64_t	_offsetTableOffset;
} CFBinaryPlistTrailer;


CF_EXPORT bool __CFBinaryPlistGetTopLevelInfo(const uint8_t *databytes, uint64_t datalen, uint8_t *marker, uint64_t *offset, CFBinaryPlistTrailer *trailer);
CF_EXPORT bool __CFBinaryPlistGetOffsetForValueFromArray2(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFIndex idx, uint64_t *offset, CFMutableDictionaryRef _Nullable unused);
CF_EXPORT bool __CFBinaryPlistGetOffsetForValueFromDictionary3(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFTypeRef key, uint64_t *_Nullable koffset, uint64_t *_Nullable voffset, Boolean unused, CFMutableDictionaryRef _Nullable unused2);
CF_EXPORT bool __CFBinaryPlistCreateObject(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFAllocatorRef _Nullable allocator, CFOptionFlags mutabilityOption, CFMutableDictionaryRef objects, CFPropertyListRef _Nullable * _Nonnull plist);
CF_EXPORT CFIndex __CFBinaryPlistWriteToStream(CFPropertyListRef plist, CFTypeRef stream);
CF_EXPORT CFIndex __CFBinaryPlistWriteToStreamWithEstimate(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate); // will be removed soon
CF_EXPORT CFIndex __CFBinaryPlistWriteToStreamWithOptions(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate, CFOptionFlags options); // will be removed soon
CF_EXPORT CFIndex __CFBinaryPlistWrite(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate, CFOptionFlags options, CFErrorRef _Nullable *_Nullable error);

#pragma mark - Property list parsing in Foundation

CF_EXPORT CFTypeRef _Nullable _CFPropertyListCreateFromXMLData(CFAllocatorRef _Nullable allocator, CFDataRef xmlData, CFOptionFlags option, CFStringRef _Nullable * _Nullable errorString, Boolean allowNewTypes, CFPropertyListFormat *_Nullable format);

CF_EXPORT CFTypeRef _CFPropertyListCreateFromXMLString(CFAllocatorRef _Nullable allocator, CFStringRef xmlString, CFOptionFlags option, CFStringRef _Nullable * _Nullable errorString, Boolean allowNewTypes, CFPropertyListFormat *_Nullable format);

#pragma mark - Sudden Termination

CF_EXPORT void _CFSuddenTerminationDisable(void);
CF_EXPORT void _CFSuddenTerminationEnable(void);

CF_EXPORT void _CFSuddenTerminationExitIfTerminationEnabled(int exitStatus);
CF_EXPORT void _CFSuddenTerminationExitWhenTerminationEnabled(int exitStatus);
CF_EXPORT size_t _CFSuddenTerminationDisablingCount(void);

#pragma mark - Thread-specific data

// Get some thread specific data from a pre-assigned slot.
CF_EXPORT void *_Nullable _CFGetTSDCreateIfNeeded(uint32_t slot, Boolean create) CF_RETURNS_NOT_RETAINED;
CF_EXPORT void *_Nullable _CFGetTSD(uint32_t slot);

// Set some thread specific data in a pre-assigned slot. Don't pick a random value. Make sure you're using a slot that is unique. Pass in a destructor to free this data, or NULL if none is needed. Unlike pthread TSD, the destructor is per-thread.
CF_EXPORT void *_Nullable _CFSetTSD(uint32_t slot, void * _Nullable newVal, void (*_Nullable destructor)(void * _Nullable));

#pragma mark - CFError userInfoProvider

/* This callback block is consulted if a key is not present in the userInfo dictionary. Note that setting a callback for the same domain again simply replaces the previous callback. The block should return autoreleased results.  Note that this functionality is now available as API on NSError; that should be used instead.
 */
#if __BLOCKS__
typedef CFTypeRef _Nonnull (^CFErrorUserInfoKeyCallBackBlock)(CFErrorRef err, CFStringRef key);
CF_EXPORT void CFErrorSetCallBackBlockForDomain(CFStringRef domainName, CFErrorUserInfoKeyCallBackBlock _Nullable provider) API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
CF_EXPORT CFErrorUserInfoKeyCallBackBlock CFErrorGetCallBackBlockForDomain(CFStringRef domainName) API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
CF_EXPORT CFErrorUserInfoKeyCallBackBlock CFErrorCopyCallBackBlockForDomain(CFStringRef domainName) API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));
#endif

/* This variant of the error domain callback has been deprecated and will be removed.  This callback function is consulted if a key is not present in the userInfo dictionary. Note that setting a callback for the same domain again simply replaces the previous callback. Set NULL as the callback to remove it. The callback function should return a result that is retained.
 */
typedef CFTypeRef _Nonnull (*CFErrorUserInfoKeyCallBack)(CFErrorRef err, CFStringRef key);
CF_EXPORT void CFErrorSetCallBackForDomain(CFStringRef domainName, CFErrorUserInfoKeyCallBack _Nullable callBack) API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CF_EXPORT CFErrorUserInfoKeyCallBack _Nullable CFErrorGetCallBackForDomain(CFStringRef domainName) API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));

_CF_EXPORT_SCOPE_END

// ---- Windows-specific material ---------------------------------------
#if TARGET_OS_WIN32

#include <sys/stat.h>

_CF_EXPORT_SCOPE_BEGIN

// These are replacements for POSIX calls on Windows, ensuring that the UTF8 parameters are converted to UTF16 before being passed to Windows
CF_EXPORT int _NS_stat(const char *name, struct _stat *st);
CF_EXPORT int _NS_mkdir(const char *name);
CF_EXPORT int _NS_rmdir(const char *name);
CF_EXPORT int _NS_chmod(const char *name, int mode);
CF_EXPORT int _NS_unlink(const char *name);
CF_EXPORT char *_NS_getcwd(char *dstbuf, size_t size);     // Warning: this doesn't support dstbuf as null even though 'getcwd' does
CF_EXPORT char *_NS_getenv(const char *name);
CF_EXPORT int _NS_rename(const char *oldName, const char *newName);
CF_EXPORT int _NS_open(const char *name, int oflag, int pmode);
CF_EXPORT int _NS_mkstemp(char *name, int bufSize);

_CF_EXPORT_SCOPE_END

#endif

// ---- Miscellaneous material ----------------------------------------

_CF_EXPORT_SCOPE_BEGIN

CF_EXPORT CFTypeID CFTypeGetTypeID(void);

CF_EXPORT void _CFArraySetCapacity(CFMutableArrayRef array, CFIndex cap);
CF_EXPORT void _CFBagSetCapacity(CFMutableBagRef bag, CFIndex cap);
CF_EXPORT void _CFDictionarySetCapacity(CFMutableDictionaryRef dict, CFIndex cap);
CF_EXPORT void _CFSetSetCapacity(CFMutableSetRef set, CFIndex cap);
CF_EXPORT CFIndex _CFBagGetUniqueCount(CFBagRef hc);

CF_EXPORT const void *_CFArrayCheckAndGetValueAtIndex(CFArrayRef array, CFIndex idx, Boolean *outOfBounds);
CF_EXPORT void _CFArrayReplaceValues(CFMutableArrayRef array, CFRange range, const void *_Nullable * _Nullable newValues, CFIndex newCount);


#if TARGET_OS_MAC
/* Enumeration
 Call CFStartSearchPathEnumeration() once, then call
 CFGetNextSearchPathEnumeration() one or more times with the returned state.
 The return value of CFGetNextSearchPathEnumeration() should be used as
 the state next time around.
 When CFGetNextSearchPathEnumeration() returns 0, you're done.
*/
typedef CFIndex CFSearchPathEnumerationState;
CF_EXPORT CFSearchPathEnumerationState __CFStartSearchPathEnumeration(CFSearchPathDirectory dir, CFSearchPathDomainMask domainMask);
CF_EXPORT CFSearchPathEnumerationState __CFGetNextSearchPathEnumeration(CFSearchPathEnumerationState state, UInt8 *path, CFIndex pathSize);
#endif

/* For use by NSNumber and CFNumber.
  Hashing algorithm for CFNumber:
  M = Max CFHashCode (assumed to be unsigned)
  For positive integral values: (N * HASHFACTOR) mod M
  For negative integral values: ((-N) * HASHFACTOR) mod M
  For floating point numbers that are not integral: hash(integral part) + hash(float part * M)
  HASHFACTOR is 2654435761, from Knuth's multiplicative method
*/
#define HASHFACTOR 2654435761U

CF_INLINE CFHashCode _CFHashInt(long i) {
    return ((i > 0) ? (CFHashCode)(i) : (CFHashCode)(-i)) * HASHFACTOR;
}

CF_INLINE CFHashCode _CFHashDouble(const double d) {
    const double positive = (d < 0) ? -d : d;
    const double positiveInt = floor(positive + 0.5);
    const double fractional = (positive - positiveInt) * (double)ULONG_MAX; // Casting `ULONG_MAX` to 'double' changes value from `18446744073709551615` to `18446744073709551616` [-Wimplicit-int-float-conversion]
    CFHashCode result = HASHFACTOR * (CFHashCode)fmod(positiveInt, (double)ULONG_MAX);
    if (fractional < 0) {
        // UBSan: Certain negative floating-point numbers are unrepresentable as 'unsigned long' which enters into undefined behavior territory in C. Thus we ensure it is positive, cast and then subtract as an integer where numbers behave correctly.
        result += -((CFHashCode)(fabs(fractional)));
    } else if (fractional > 0) {
        // Caveat: the > 0 is incredibly important [28612173]
        result += (CFHashCode)fractional;
    }
    return result;
}


CF_CROSS_PLATFORM_EXPORT void _CFNumberInitBool(CFNumberRef result, Boolean value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitInt8(CFNumberRef result, int8_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitUInt8(CFNumberRef result, uint8_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitInt16(CFNumberRef result, int16_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitUInt16(CFNumberRef result, uint16_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitInt32(CFNumberRef result, int32_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitUInt32(CFNumberRef result, uint32_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitInt(CFNumberRef result, long value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitUInt(CFNumberRef result, unsigned long value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitInt64(CFNumberRef result, int64_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitUInt64(CFNumberRef result, uint64_t value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitFloat(CFNumberRef result, float value);
CF_CROSS_PLATFORM_EXPORT void _CFNumberInitDouble(CFNumberRef result, double value);

CF_EXPORT CFNumberType _CFNumberGetType2(CFNumberRef number);

/* These four functions are used by NSError in formatting error descriptions. They take NS or CFError as arguments and return a retained CFString or NULL.
*/ 
CF_EXPORT CFStringRef _CFErrorCreateLocalizedDescription(CFErrorRef err);
CF_EXPORT CFStringRef _CFErrorCreateLocalizedFailureReason(CFErrorRef err);
CF_EXPORT CFStringRef _CFErrorCreateLocalizedRecoverySuggestion(CFErrorRef err);
CF_EXPORT CFStringRef _CFErrorCreateDebugDescription(CFErrorRef err);
CF_EXPORT CFStringRef _CFErrorCreateRedactedDescription(CFErrorRef err);

CF_EXPORT void *__CFURLReservedPtr(CFURLRef  url);
CF_EXPORT void __CFURLSetReservedPtr(CFURLRef  url, void *_Nullable ptr);
CF_EXPORT CFStringEncoding _CFURLGetEncoding(CFURLRef url);

CF_CROSS_PLATFORM_EXPORT void _CFURLInitWithFileSystemPathRelativeToBase(CFURLRef url, CFStringRef fileSystemPath, CFURLPathStyle pathStyle, Boolean isDirectory, _Nullable CFURLRef baseURL);
CF_CROSS_PLATFORM_EXPORT Boolean _CFURLInitWithURLString(CFURLRef url, CFStringRef string, Boolean checkForLegalCharacters, _Nullable CFURLRef baseURL);
CF_CROSS_PLATFORM_EXPORT Boolean _CFURLInitAbsoluteURLWithBytes(CFURLRef url, const UInt8 *relativeURLBytes, CFIndex length, CFStringEncoding encoding, _Nullable CFURLRef baseURL);

#if !TARGET_OS_WASI
CF_EXPORT Boolean _CFRunLoopFinished(CFRunLoopRef rl, CFStringRef mode);
CF_EXPORT CFTypeRef _CFRunLoopGet2(CFRunLoopRef rl);
CF_EXPORT Boolean _CFRunLoopIsCurrent(CFRunLoopRef rl);

CF_EXPORT CFIndex _CFStreamInstanceSize(void);
CF_EXPORT void _CFReadStreamInitialize(CFReadStreamRef readStream);
CF_EXPORT void _CFWriteStreamInitialize(CFWriteStreamRef writeStream);
CF_EXPORT void _CFReadStreamDeallocate(CFReadStreamRef readStream);
CF_EXPORT void _CFWriteStreamDeallocate(CFWriteStreamRef writeStream);
CF_EXPORT CFReadStreamRef CFReadStreamCreateWithData(_Nullable CFAllocatorRef alloc, CFDataRef data);
#endif

#if TARGET_OS_MAC
typedef struct {
    mach_vm_address_t address;
    mach_vm_size_t size;
    mach_vm_address_t map_address;
    bool purgeable;
    bool volatyle;
    uintptr_t reserved;
} CFDiscorporateMemory;

extern kern_return_t _CFDiscorporateMemoryAllocate(CFDiscorporateMemory *hm, size_t size, bool purgeable);
extern kern_return_t _CFDiscorporateMemoryDeallocate(CFDiscorporateMemory *hm);
extern kern_return_t _CFDiscorporateMemoryDematerialize(CFDiscorporateMemory *hm);
extern kern_return_t _CFDiscorporateMemoryMaterialize(CFDiscorporateMemory *hm);
#endif

enum {
    kCFNumberFormatterDurationStyle = 7,
};

// This is for NSNumberFormatter use only!
CF_EXPORT void *_CFNumberFormatterGetFormatter(CFNumberFormatterRef formatter);

CF_CROSS_PLATFORM_EXPORT void _CFDataInit(CFMutableDataRef memory, CFOptionFlags variety, CFIndex capacity, const uint8_t *_Nullable bytes, CFIndex length, Boolean noCopy);
CF_EXPORT CFRange _CFDataFindBytes(CFDataRef data, CFDataRef dataToFind, CFRange searchRange, CFDataSearchFlags compareOptions);


#if TARGET_OS_MAC
    #if !defined(__CFReadTSR)
    #define __CFReadTSR() mach_absolute_time()
    #endif
#elif TARGET_OS_WIN32
#if 0
CF_INLINE UInt64 __CFReadTSR(void) {
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&freq);
    return freq.QuadPart;
}
#endif
#endif

#if TARGET_OS_MAC

/* Identical to CFStringGetFileSystemRepresentation, but returns additional information about the failure.
 */
typedef CF_ENUM(CFIndex, _CFStringFileSystemRepresentationError) {
    _kCFStringFileSystemRepresentationErrorNone = 0,            // 'characterIndex' is undefined.
    _kCFStringFileSystemRepresentationErrorBufferFull,          // 'characterIndex' is undefined.
    _kCFStringFileSystemRepresentationErrorEmbeddedNull,        // 'characterIndex' == index of first NULL character in 'buffer'.
    _kCFStringFileSystemRepresentationErrorUnpairedSurrogate    // 'characterIndex' == index of first unpaired surrogate in 'string'.
};
CF_EXPORT _CFStringFileSystemRepresentationError _CFStringGetFileSystemRepresentationWithErrorStatus(CFStringRef string, char *buffer, CFIndex maxBufLen, CFIndex *_Nullable characterIndex);

#endif



CF_EXPORT CFTimeInterval CFGetSystemUptime(void);
CF_EXPORT CFStringRef CFCopySystemVersionString(void);
CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary(void);

typedef struct {
    CFIndex majorVersion;
    CFIndex minorVersion;
    CFIndex patchVersion;
} CFOperatingSystemVersion;

CF_EXPORT CFOperatingSystemVersion _CFOperatingSystemVersionGetCurrent(void);

CF_EXPORT Boolean _CFOperatingSystemVersionIsAtLeastVersion(CFOperatingSystemVersion version);

CF_CROSS_PLATFORM_EXPORT Boolean _CFLocaleInit(CFLocaleRef locale, CFStringRef identifier);

/// Returns a result similar to `CFLocaleCopyPreferredLanguages` but by specifically reading the preferences for `kCFPreferencesCurrentUser` as opposed to walking up the preferences chain. This is needed by specific callers (e.g. `+[NSLocale setPreferredLanguages:]`) to check whether the defaults being set have changed from what’s already set.
CF_EXPORT CFArrayRef _CFLocaleCopyPreferredLanguagesForCurrentUser(void);

CF_CROSS_PLATFORM_EXPORT Boolean _CFTimeZoneInit(CFTimeZoneRef timeZone, CFStringRef name, _Nullable CFDataRef data);

CF_CROSS_PLATFORM_EXPORT Boolean _CFCharacterSetInitWithCharactersInRange(CFMutableCharacterSetRef cset, CFRange theRange);
CF_CROSS_PLATFORM_EXPORT Boolean _CFCharacterSetInitWithCharactersInString(CFMutableCharacterSetRef cset, CFStringRef theString);
CF_CROSS_PLATFORM_EXPORT Boolean _CFCharacterSetInitMutable(CFMutableCharacterSetRef cset);
CF_CROSS_PLATFORM_EXPORT Boolean _CFCharacterSetInitWithBitmapRepresentation(CFMutableCharacterSetRef cset, CFDataRef theData);

CF_EXPORT void * _Nullable __CFTSANTagMutableArray;
CF_EXPORT void * _Nullable __CFTSANTagMutableDictionary;
CF_EXPORT void * _Nullable __CFTSANTagMutableSet;
CF_EXPORT void * _Nullable __CFTSANTagMutableOrderedSet;
CF_EXPORT void * _Nullable __CFTSANTagMutableData;

CF_EXPORT void * _Nullable _CFRegisterThreadSanitizerTag(char *name);
CF_EXPORT void _CFAssignThreadSanitizerTag(void *const _Nonnull ptr, void *const tag);

CF_EXPORT void (* _Nullable __cf_tsanReadFunction)(void *, void *, void *);
#define _CFRecordReadForDataOwnedBy(P, T) do {\
  if (__builtin_expect(__cf_tsanReadFunction != NULL, false)) {\
    __cf_tsanReadFunction((P), __builtin_return_address(0), (T));\
  }\
} while(0)

CF_EXPORT void (* _Nullable __cf_tsanWriteFunction)(void *, void *, void *);
#define _CFRecordWriteForDataOwnedBy(P, T) do {\
  if (__builtin_expect(__cf_tsanWriteFunction != NULL, false)) {\
    __cf_tsanWriteFunction((P), __builtin_return_address(0), (T));\
  }\
} while (0)

CF_EXPORT void *_CFCreateArrayStorage(size_t numPointers, Boolean zeroed, size_t *actualNumPointers) CF_RETURNS_NOT_RETAINED;



#if DEPLOYMENT_RUNTIME_SWIFT
// --- Static class references for Swift use; implements {DECLARE_,}STATIC_CLASS_REF.

#if TARGET_OS_MAC
#define STATIC_CLASS_PREFIX $s15SwiftFoundation
#else
#define STATIC_CLASS_PREFIX $s10Foundation
#endif

#define STATIC_CLASS_NAME_LENGTH_LOOKUP___NSCFType 10
#define STATIC_CLASS_NAME_LOOKUP___NSCFType __NSCFTypeCN

#define STATIC_CLASS_NAME_LENGTH_LOOKUP_NSNull 6
#define STATIC_CLASS_NAME_LOOKUP_NSNull NSNullCN

#define STATIC_CLASS_NAME_LENGTH_LOOKUP___NSCFBoolean 13
#define STATIC_CLASS_NAME_LOOKUP___NSCFBoolean __NSCFBooleanCN

#define STATIC_CLASS_NAME_LENGTH_LOOKUP___NSCFNumber 8
#define STATIC_CLASS_NAME_LOOKUP___NSCFNumber NSNumberCN

#define STATIC_CLASS_NAME_LENGTH_LOOKUP_NSMutableData 13
#define STATIC_CLASS_NAME_LOOKUP_NSMutableData NSMutableDataCN

#define STATIC_CLASS_NAME_CONCAT_INNER(x,y) x ## y
#define STATIC_CLASS_NAME_CONCAT(x,y) STATIC_CLASS_NAME_CONCAT_INNER(x,y)

#define STATIC_CLASS_NAME_LOOKUP(CLASSNAME) STATIC_CLASS_NAME_CONCAT(STATIC_CLASS_NAME_LOOKUP_, CLASSNAME)
#define STATIC_CLASS_NAME_LENGTH_LOOKUP(CLASSNAME) STATIC_CLASS_NAME_CONCAT(STATIC_CLASS_NAME_LENGTH_LOOKUP_, CLASSNAME)

#define STATIC_CLASS_NAME(CLASSNAME) STATIC_CLASS_NAME_CONCAT(STATIC_CLASS_NAME_LENGTH_LOOKUP(CLASSNAME), STATIC_CLASS_NAME_LOOKUP(CLASSNAME))

#define DECLARE_STATIC_CLASS_REF(CLASSNAME) extern void STATIC_CLASS_NAME_CONCAT(STATIC_CLASS_PREFIX, STATIC_CLASS_NAME(CLASSNAME))
#define STATIC_CLASS_REF(CLASSNAME) &(STATIC_CLASS_NAME_CONCAT(STATIC_CLASS_PREFIX, STATIC_CLASS_NAME(CLASSNAME)))

#else // if !DEPLOYMENT_RUNTIME_SWIFT

// We don't need static class refs if CF is used standalone, as there's no Swift or ObjC runtime to interoperate with.
#define STATIC_CLASS_REF(...) NULL
#define DECLARE_STATIC_CLASS_REF(...)

#endif


_CF_EXPORT_SCOPE_END

#if __OBJC__

#define _scoped_id_array(N, C, Z, S) \
    size_t N ## _count__ = (C); \
    if (N ## _count__ > LONG_MAX / sizeof(id)) { \
        CFStringRef reason = CFStringCreateWithFormat(NULL, NULL, CFSTR("*** attempt to create a temporary id buffer which is too large or with a negative count (%lu) -- possibly data is corrupt"), N ## _count__); \
        NSException *e = [NSException exceptionWithName:NSGenericException reason:(NSString *)reason userInfo:nil]; \
        CFRelease(reason); \
        @throw e; \
    } \
    Boolean N ## _is_stack__ = (N ## _count__ <= 256) && (S); \
    if (N ## _count__ == 0) { \
        N ## _count__ = 1; \
    } \
    id N ## _scopedbuffer__ [N ## _is_stack__ ? N ## _count__ : 1]; \
    if (N ## _is_stack__ && (Z)) { \
        memset(N ## _scopedbuffer__, 0, N ## _count__ * sizeof(id)); \
    } \
    size_t N ## _unused__; \
    id * __attribute__((cleanup(_scoped_id_array_cleanup))) N ## _mallocbuffer__ = N ## _is_stack__ ? NULL : (id *)_CFCreateArrayStorage(N ## _count__, (Z), & N ## _unused__); \
    id * N = N ## _is_stack__ ? N ## _scopedbuffer__ : N ## _mallocbuffer__; \
    do {} while (0)

// These macros create an array that is 1) either stack or buffer allocated, depending on size, and 2) automatically cleaned up at the end of the lexical scope it is declared in.
#define scoped_id_array(N, C) _scoped_id_array(N, C, false, true)
#define scoped_and_zeroed_id_array(N, C) _scoped_id_array(N, C, true, true)

#define scoped_heap_id_array(N, C) _scoped_id_array(N, C, false, false)

// This macro either returns the buffer while simultaneously passing responsibility for freeing it to the caller, or it returns NULL if the buffer exists on the stack, and therefore can't pass ownership.
#define try_adopt_scoped_id_array(N) (N ## _mallocbuffer__ ? ({id *tmp = N ## _mallocbuffer__; N ## _mallocbuffer__ = NULL; tmp;}) : NULL)

CF_INLINE void _scoped_id_array_cleanup(id _Nonnull * _Nullable * _Nonnull mallocedbuffer) {
    // Maybe be NULL, but free(NULL) is well defined as a no-op.
    free(*mallocedbuffer);
}
#endif

// Define NS_DIRECT / NS_DIRECT_MEMBERS Internally for CoreFoundation.

#ifndef NS_DIRECT
    #if __has_attribute(objc_direct)
        #define NS_DIRECT __attribute__((objc_direct))
    #else
        #define NS_DIRECT
    #endif
#endif

#ifndef NS_DIRECT_MEMBERS
    #if __has_attribute(objc_direct_members)
        #define NS_DIRECT_MEMBERS __attribute__((objc_direct_members))
    #else
        #define NS_DIRECT_MEMBERS
    #endif
#endif

#endif /* ! __COREFOUNDATION_FORFOUNDATIONONLY__ */

