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

/*	ForFoundationOnly.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !CF_BUILDING_CF && !NSBUILDINGFOUNDATION
    #error The header file ForFoundationOnly.h is for the exclusive use of the
    #error CoreFoundation and Foundation projects.  No other project should include it.
#endif

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
#include <CoreFoundation/CFNumberFormatter.h>
#include <limits.h>

// NOTE: miscellaneous declarations are at the end

// ---- CFRuntime material ----------------------------------------

CF_EXTERN_C_BEGIN

#if DEPLOYMENT_TARGET_LINUX
#include <malloc.h>
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <malloc/malloc.h>
#endif

CF_EXTERN_C_END

// ---- CFBundle material ----------------------------------------

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS
#include <CoreFoundation/CFBundlePriv.h>

CF_EXTERN_C_BEGIN

CF_EXPORT const CFStringRef _kCFBundleExecutablePathKey;
CF_EXPORT const CFStringRef _kCFBundleInfoPlistURLKey;
CF_EXPORT const CFStringRef _kCFBundleRawInfoPlistURLKey;
CF_EXPORT const CFStringRef _kCFBundleNumericVersionKey;
CF_EXPORT const CFStringRef _kCFBundleResourcesFileMappedKey;
CF_EXPORT const CFStringRef _kCFBundleAllowMixedLocalizationsKey;
CF_EXPORT const CFStringRef _kCFBundleInitialPathKey;
CF_EXPORT const CFStringRef _kCFBundleResolvedPathKey;
CF_EXPORT const CFStringRef _kCFBundlePrincipalClassKey;

#if __BLOCKS__
CF_EXPORT CFTypeRef _CFBundleCopyFindResources(CFBundleRef bundle, CFURLRef bundleURL, CFArrayRef _unused_pass_null_, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subPath, CFStringRef lproj, Boolean returnArray, Boolean localized, Boolean (^predicate)(CFStringRef filename, Boolean *stop));
#endif

CF_EXPORT Boolean _CFBundleLoadExecutableAndReturnError(CFBundleRef bundle, Boolean forceGlobal, CFErrorRef *error);
CF_EXPORT CFErrorRef _CFBundleCreateError(CFAllocatorRef allocator, CFBundleRef bundle, CFIndex code);

CF_EXTERN_C_END

#endif

// ---- CFPreferences material ----------------------------------------

#define DEBUG_PREFERENCES_MEMORY 0
 
#if DEBUG_PREFERENCES_MEMORY
#include "../Tests/CFCountingAllocator.h"
#endif

CF_EXTERN_C_BEGIN

extern void _CFPreferencesPurgeDomainCache(void);

typedef struct {
    void *	(*createDomain)(CFAllocatorRef allocator, CFTypeRef context);
    void	(*freeDomain)(CFAllocatorRef allocator, CFTypeRef context, void *domain);
    CFTypeRef	(*fetchValue)(CFTypeRef context, void *domain, CFStringRef key); // Caller releases
    void	(*writeValue)(CFTypeRef context, void *domain, CFStringRef key, CFTypeRef value);
    Boolean	(*synchronize)(CFTypeRef context, void *domain);
    void	(*getKeysAndValues)(CFAllocatorRef alloc, CFTypeRef context, void *domain, void **buf[], CFIndex *numKeyValuePairs);
    CFDictionaryRef (*copyDomainDictionary)(CFTypeRef context, void *domain);
    /* HACK - see comment on _CFPreferencesDomainSetIsWorldReadable(), below */
    void	(*setIsWorldReadable)(CFTypeRef context, void *domain, Boolean isWorldReadable);
} _CFPreferencesDomainCallBacks;

CF_EXPORT CFAllocatorRef __CFPreferencesAllocator(void);
CF_EXPORT  const _CFPreferencesDomainCallBacks __kCFVolatileDomainCallBacks;
CF_EXPORT const _CFPreferencesDomainCallBacks __kCFXMLPropertyListDomainCallBacks;

typedef struct __CFPreferencesDomain * CFPreferencesDomainRef;

CF_EXPORT CFPreferencesDomainRef _CFPreferencesDomainCreate(CFTypeRef context, const _CFPreferencesDomainCallBacks *callBacks);
CF_EXPORT CFPreferencesDomainRef _CFPreferencesStandardDomain(CFStringRef domainName, CFStringRef userName, CFStringRef hostName);

CF_EXPORT CFTypeRef _CFPreferencesDomainCreateValueForKey(CFPreferencesDomainRef domain, CFStringRef key);
CF_EXPORT void _CFPreferencesDomainSet(CFPreferencesDomainRef domain, CFStringRef key, CFTypeRef value);
CF_EXPORT Boolean _CFPreferencesDomainSynchronize(CFPreferencesDomainRef domain);

CF_EXPORT CFArrayRef _CFPreferencesCreateDomainList(CFStringRef userName, CFStringRef hostName);
CF_EXPORT Boolean _CFSynchronizeDomainCache(void);

CF_EXPORT void _CFPreferencesDomainSetDictionary(CFPreferencesDomainRef domain, CFDictionaryRef dict);
CF_EXPORT CFDictionaryRef _CFPreferencesDomainDeepCopyDictionary(CFPreferencesDomainRef domain);
CF_EXPORT Boolean _CFPreferencesDomainExists(CFStringRef domainName, CFStringRef userName, CFStringRef hostName);

/* HACK - this is to work around the fact that individual domains lose the information about their user/host/app triplet at creation time.  We should find a better way to propogate this information.  REW, 1/13/00 */
CF_EXPORT void _CFPreferencesDomainSetIsWorldReadable(CFPreferencesDomainRef domain, Boolean isWorldReadable);

typedef struct {
    CFMutableArrayRef _search;  // the search list; an array of _CFPreferencesDomains
    CFMutableDictionaryRef _dictRep; // Mutable; a collapsed view of the search list, expressed as a single dictionary
    CFStringRef _appName;
} _CFApplicationPreferences;

CF_EXPORT _CFApplicationPreferences *_CFStandardApplicationPreferences(CFStringRef appName);
CF_EXPORT _CFApplicationPreferences *_CFApplicationPreferencesCreateWithUser(CFStringRef userName, CFStringRef appName);
CF_EXPORT void _CFDeallocateApplicationPreferences(_CFApplicationPreferences *self);
CF_EXPORT CFTypeRef _CFApplicationPreferencesCreateValueForKey(_CFApplicationPreferences *prefs, CFStringRef key);
CF_EXPORT void _CFApplicationPreferencesSet(_CFApplicationPreferences *self, CFStringRef defaultName, CFTypeRef value);
CF_EXPORT void _CFApplicationPreferencesRemove(_CFApplicationPreferences *self, CFStringRef defaultName);
CF_EXPORT Boolean _CFApplicationPreferencesSynchronize(_CFApplicationPreferences *self);
CF_EXPORT void _CFApplicationPreferencesUpdate(_CFApplicationPreferences *self); // same as updateDictRep
CF_EXPORT CFDictionaryRef _CFApplicationPreferencesCopyRepresentation3(_CFApplicationPreferences *self, CFDictionaryRef hint, CFDictionaryRef insertion, CFPreferencesDomainRef afterDomain);
CF_EXPORT CFDictionaryRef _CFApplicationPreferencesCopyRepresentationWithHint(_CFApplicationPreferences *self, CFDictionaryRef hint); // same as dictRep
CF_EXPORT void _CFApplicationPreferencesSetStandardSearchList(_CFApplicationPreferences *appPreferences);
CF_EXPORT void _CFApplicationPreferencesSetCacheForApp(_CFApplicationPreferences *appPrefs, CFStringRef appName);
CF_EXPORT void _CFApplicationPreferencesAddSuitePreferences(_CFApplicationPreferences *appPrefs, CFStringRef suiteName);
CF_EXPORT void _CFApplicationPreferencesRemoveSuitePreferences(_CFApplicationPreferences *appPrefs, CFStringRef suiteName);

CF_EXPORT void _CFApplicationPreferencesAddDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef domain, Boolean addAtTop);
CF_EXPORT Boolean _CFApplicationPreferencesContainsDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef domain);
CF_EXPORT void _CFApplicationPreferencesRemoveDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef domain);

CF_EXPORT CFTypeRef _CFApplicationPreferencesSearchDownToDomain(_CFApplicationPreferences *self, CFPreferencesDomainRef stopper, CFStringRef key);


CF_EXTERN_C_END



// ---- CFString material ----------------------------------------

#include <CoreFoundation/CFStringEncodingExt.h>

#define NSSTRING_BOUNDSERROR \
    [NSException raise:NSRangeException format:@"%@: Range or index out of bounds", __CFExceptionProem((id)self, _cmd)]

#define NSSTRING_RANGEERROR(range, len) \
    [NSException raise:NSRangeException format:@"%@: Range {%lu, %lu} out of bounds; string length %lu", __CFExceptionProem((id)self, _cmd), (unsigned long)range.location, (unsigned long)range.length, (unsigned long)len]

#define NSSTRING_INDEXERROR(index, len) \
    [NSException raise:NSRangeException format:@"%@: Index %lu out of bounds; string length %lu", __CFExceptionProem((id)self, _cmd), (unsigned long)index, (unsigned long)len]

// This can be made into an exception for post-10.10 apps
#define NSSTRING_POSSIBLE_RANGEERROR(range, len)     \
    if (__CFStringNoteErrors()) {       \
        static bool warnonce = false;   \
        if (!warnonce) {                \
            warnonce = true;            \
            CFLog(kCFLogLevelWarning, CFSTR("*** %@: Range {%lu, %lu} out of bounds; string length %lu. This will become an exception for apps linked after 10.10 and iOS 8. Warning shown once per app execution."), __CFExceptionProem((id)self, _cmd), (unsigned long)range.location, (unsigned long)range.length, (unsigned long)len);        \
    }   \
}

#define NSSTRING_ILLEGALREQUESTERROR \
    [NSException raise:NSInvalidArgumentException format:@"Can't call %s in %@", sel_getName(_cmd), object_getClass((id)self)]

#define NSSTRING_INVALIDMUTATIONERROR \
    [NSException raise:NSInvalidArgumentException format:@"Attempt to mutate immutable object with %s", sel_getName(_cmd)]

#define NSSTRING_NULLCSTRINGERROR \
    [NSException raise:NSInvalidArgumentException format:@"%@: NULL cString", __CFExceptionProem((id)self, _cmd)]

#define NSSTRING_NILSTRINGERROR \
    [NSException raise:NSInvalidArgumentException format:@"%@: nil argument", __CFExceptionProem((id)self, _cmd)]


CF_EXTERN_C_BEGIN

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
CF_EXPORT CFIndex __CFStringEncodeByteStream(CFStringRef string, CFIndex rangeLoc, CFIndex rangeLen, Boolean generatingExternalFile, CFStringEncoding encoding, char lossByte, UInt8 *buffer, CFIndex max, CFIndex *usedBufLen);

CF_EXPORT CFStringRef __CFStringCreateImmutableFunnel2(CFAllocatorRef alloc, const void *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean possiblyExternalFormat, Boolean tryToReduceUnicode, Boolean hasLengthByte, Boolean hasNullByte, Boolean noCopy, CFAllocatorRef contentsDeallocator);

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
        UInt8 *ascii;
	UniChar *unicode;
    } chars;
    Boolean isASCII;	/* This really does mean 7-bit ASCII, not _NSDefaultCStringEncoding() */
    Boolean shouldFreeChars;	/* If the number of bytes exceeds __kCFVarWidthLocalBufferSize, bytes are allocated */
    Boolean _unused1;
    Boolean _unused2;
    CFAllocatorRef allocator;	/* Use this allocator to allocate, reallocate, and deallocate the bytes */
    CFIndex numChars;	/* This is in terms of ascii or unicode; that is, if isASCII, it is number of 7-bit chars; otherwise it is number of UniChars; note that the actual allocated space might be larger */
    UInt8 localBuffer[__kCFVarWidthLocalBufferSize];	/* private; 168 ISO2022JP chars, 504 Unicode chars, 1008 ASCII chars */
} CFVarWidthCharBuffer;


/* Convert a byte stream to ASCII (7-bit!) or Unicode, with a CFVarWidthCharBuffer struct on the stack. false return indicates an error occured during the conversion. Depending on .isASCII, follow .chars.ascii or .chars.unicode.  If .shouldFreeChars is returned as true, free the returned buffer when done with it.  If useClientsMemoryPtr is provided as non-NULL, and the provided memory can be used as is, this is set to true, and the .ascii or .unicode buffer in CFVarWidthCharBuffer is set to bytes.
!!! If the stream is Unicode and has no BOM, the data is assumed to be big endian! Could be trouble on Intel if someone didn't follow that assumption.
!!! __CFStringDecodeByteStream2() needs to be deprecated and removed post-Jaguar.
*/
CF_EXPORT Boolean __CFStringDecodeByteStream2(const UInt8 *bytes, UInt32 len, CFStringEncoding encoding, Boolean alwaysUnicode, CFVarWidthCharBuffer *buffer, Boolean *useClientsMemoryPtr);
CF_EXPORT Boolean __CFStringDecodeByteStream3(const UInt8 *bytes, CFIndex len, CFStringEncoding encoding, Boolean alwaysUnicode, CFVarWidthCharBuffer *buffer, Boolean *useClientsMemoryPtr, UInt32 converterFlags);


/* Convert single byte to Unicode; assumes one-to-one correspondence (that is, can only be used with 1-byte encodings). You can use the function if it's not NULL.
*/
CF_EXPORT Boolean (*__CFCharToUniCharFunc)(UInt32 flags, UInt8 ch, UniChar *unicodeChar);

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


/* These two allow specifying an alternate description function (instead of CFCopyDescription); used by NSString
*/
CF_EXPORT void _CFStringAppendFormatAndArgumentsAux(CFMutableStringRef outputString, CFStringRef (*copyDescFunc)(void *, const void *loc), CFDictionaryRef formatOptions, CFStringRef formatString, va_list args);
CF_EXPORT CFStringRef  _CFStringCreateWithFormatAndArgumentsAux(CFAllocatorRef alloc, CFStringRef (*copyDescFunc)(void *, const void *loc), CFDictionaryRef formatOptions, CFStringRef format, va_list arguments);

CF_EXPORT void _CFStringAppendFormatAndArgumentsAux2(CFMutableStringRef outputString, CFStringRef (*copyDescFunc)(void *, const void *loc), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, CFStringRef formatString, va_list args);
CF_EXPORT CFStringRef  _CFStringCreateWithFormatAndArgumentsAux2(CFAllocatorRef alloc, CFStringRef (*copyDescFunc)(void *, const void *loc), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, CFStringRef format, va_list arguments);

/* For NSString (and NSAttributedString) usage, mutate with isMutable check
*/
enum {_CFStringErrNone = 0, _CFStringErrNotMutable = 1, _CFStringErrNilArg = 2, _CFStringErrBounds = 3};
CF_EXPORT int __CFStringCheckAndReplace(CFMutableStringRef str, CFRange range, CFStringRef replacement);
CF_EXPORT Boolean __CFStringNoteErrors(void);		// Should string errors raise?

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


CF_EXTERN_C_END


// ---- Binary plist material ----------------------------------------

typedef const struct __CFKeyedArchiverUID * CFKeyedArchiverUIDRef;
CF_EXPORT CFTypeID _CFKeyedArchiverUIDGetTypeID(void);
CF_EXPORT CFKeyedArchiverUIDRef _CFKeyedArchiverUIDCreate(CFAllocatorRef allocator, uint32_t value);
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
CF_EXPORT bool __CFBinaryPlistGetOffsetForValueFromArray2(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFIndex idx, uint64_t *offset, CFMutableDictionaryRef objects);
CF_EXPORT bool __CFBinaryPlistGetOffsetForValueFromDictionary3(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFTypeRef key, uint64_t *koffset, uint64_t *voffset, Boolean unused, CFMutableDictionaryRef objects);
CF_EXPORT bool __CFBinaryPlistCreateObject(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFAllocatorRef allocator, CFOptionFlags mutabilityOption, CFMutableDictionaryRef objects, CFPropertyListRef *plist);
CF_EXPORT CFIndex __CFBinaryPlistWriteToStream(CFPropertyListRef plist, CFTypeRef stream);
CF_EXPORT CFIndex __CFBinaryPlistWriteToStreamWithEstimate(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate); // will be removed soon
CF_EXPORT CFIndex __CFBinaryPlistWriteToStreamWithOptions(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate, CFOptionFlags options); // will be removed soon
CF_EXPORT CFIndex __CFBinaryPlistWrite(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate, CFOptionFlags options, CFErrorRef *error);

// ---- Used by property list parsing in Foundation

CF_EXPORT CFTypeRef _CFPropertyListCreateFromXMLData(CFAllocatorRef allocator, CFDataRef xmlData, CFOptionFlags option, CFStringRef *errorString, Boolean allowNewTypes, CFPropertyListFormat *format);

CF_EXPORT CFTypeRef _CFPropertyListCreateFromXMLString(CFAllocatorRef allocator, CFStringRef xmlString, CFOptionFlags option, CFStringRef *errorString, Boolean allowNewTypes, CFPropertyListFormat *format);

// ---- Sudden Termination material ----------------------------------------

#if DEPLOYMENT_TARGET_MACOSX

CF_EXPORT void _CFSuddenTerminationDisable(void);
CF_EXPORT void _CFSuddenTerminationEnable(void);
CF_EXPORT void _CFSuddenTerminationExitIfTerminationEnabled(int exitStatus);
CF_EXPORT void _CFSuddenTerminationExitWhenTerminationEnabled(int exitStatus);
CF_EXPORT size_t _CFSuddenTerminationDisablingCount(void);

#endif

// ---- Thread-specific data --------------------------------------------

// Get some thread specific data from a pre-assigned slot.
CF_EXPORT void *_CFGetTSD(uint32_t slot);

// Set some thread specific data in a pre-assigned slot. Don't pick a random value. Make sure you're using a slot that is unique. Pass in a destructor to free this data, or NULL if none is needed. Unlike pthread TSD, the destructor is per-thread.
CF_EXPORT void *_CFSetTSD(uint32_t slot, void *newVal, void (*destructor)(void *));

#if DEPLOYMENT_TARGET_WINDOWS
// ---- Windows-specific material ---------------------------------------

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
#endif

// ---- Miscellaneous material ----------------------------------------

#include <CoreFoundation/CFBag.h>
#include <CoreFoundation/CFSet.h>
#include <math.h>

CF_EXTERN_C_BEGIN

CF_EXPORT CFTypeID CFTypeGetTypeID(void);

CF_EXPORT void _CFArraySetCapacity(CFMutableArrayRef array, CFIndex cap);
CF_EXPORT void _CFBagSetCapacity(CFMutableBagRef bag, CFIndex cap);
CF_EXPORT void _CFDictionarySetCapacity(CFMutableDictionaryRef dict, CFIndex cap);
CF_EXPORT void _CFSetSetCapacity(CFMutableSetRef set, CFIndex cap);

CF_EXPORT void CFCharacterSetCompact(CFMutableCharacterSetRef theSet);
CF_EXPORT void CFCharacterSetFast(CFMutableCharacterSetRef theSet);

CF_EXPORT const void *_CFArrayCheckAndGetValueAtIndex(CFArrayRef array, CFIndex idx);
CF_EXPORT void _CFArrayReplaceValues(CFMutableArrayRef array, CFRange range, const void **newValues, CFIndex newCount);


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

CF_INLINE CFHashCode _CFHashDouble(double d) {
    double dInt;
    if (d < 0) d = -d;
    dInt = floor(d+0.5);
    CFHashCode integralHash = HASHFACTOR * (CFHashCode)fmod(dInt, (double)ULONG_MAX);
    return (CFHashCode)(integralHash + (CFHashCode)((d - dInt) * ULONG_MAX));
}


/* These four functions are used by NSError in formatting error descriptions. They take NS or CFError as arguments and return a retained CFString or NULL.
*/ 
CF_EXPORT CFStringRef _CFErrorCreateLocalizedDescription(CFErrorRef err);
CF_EXPORT CFStringRef _CFErrorCreateLocalizedFailureReason(CFErrorRef err);
CF_EXPORT CFStringRef _CFErrorCreateLocalizedRecoverySuggestion(CFErrorRef err);
CF_EXPORT CFStringRef _CFErrorCreateDebugDescription(CFErrorRef err);

CF_EXPORT void *__CFURLReservedPtr(CFURLRef  url);
CF_EXPORT void __CFURLSetReservedPtr(CFURLRef  url, void *ptr);
CF_EXPORT CFStringEncoding _CFURLGetEncoding(CFURLRef url);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS
CF_EXPORT Boolean _CFRunLoopFinished(CFRunLoopRef rl, CFStringRef mode);
#endif

CF_EXPORT CFIndex _CFStreamInstanceSize(void);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
typedef struct {
    mach_vm_address_t address;
    mach_vm_size_t size;
    mach_port_t port;
    bool purgeable;
    bool corporeal;
    bool volatyle;
    uintptr_t reserved;
} CFDiscorporateMemory;

extern kern_return_t _CFDiscorporateMemoryAllocate(CFDiscorporateMemory *hm, size_t size, bool purgeable);
extern kern_return_t _CFDiscorporateMemoryDeallocate(CFDiscorporateMemory *hm);
extern kern_return_t _CFDiscorporateMemoryDematerialize(CFDiscorporateMemory *hm);
extern kern_return_t _CFDiscorporateMemoryMaterialize(CFDiscorporateMemory *hm);
#endif

enum {
    kCFNumberFormatterOrdinalStyle = 6,
    kCFNumberFormatterDurationStyle = 7,
};

// This is for NSNumberFormatter use only!
CF_EXPORT void *_CFNumberFormatterGetFormatter(CFNumberFormatterRef formatter);

CF_EXPORT CFRange _CFDataFindBytes(CFDataRef data, CFDataRef dataToFind, CFRange searchRange, CFDataSearchFlags compareOptions);


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    #if !defined(__CFReadTSR)
    #include <mach/mach_time.h>
    #define __CFReadTSR() mach_absolute_time()
    #endif
#elif DEPLOYMENT_TARGET_WINDOWS
#if 0
CF_INLINE UInt64 __CFReadTSR(void) {
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&freq);
    return freq.QuadPart;
}
#endif
#endif


CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_FORFOUNDATIONONLY__ */

