/*	CFPriv.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

/*
        APPLE SPI:  NOT TO BE USED OUTSIDE APPLE!*
 
        *or swift-corelibs-foundation
*/

#if !defined(__COREFOUNDATION_CFPRIV__)
#define __COREFOUNDATION_CFPRIV__ 1

#include <string.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFSet.h>
#include <math.h>

#define CF_CROSS_PLATFORM_EXPORT extern

#if TARGET_OS_WIN32
  // No C99 support
  #define _CF_RESTRICT
#else
  #if defined(__cplusplus)
    #define _CF_RESTRICT __restrict__
  #else
    #define _CF_RESTRICT restrict
  #endif
#endif



#if (TARGET_OS_MAC && !(TARGET_OS_IPHONE || TARGET_OS_LINUX)) || TARGET_OS_IPHONE
#include <CoreFoundation/CFMachPort.h>
#include <CoreFoundation/CFMessagePort.h>
#endif

#if !TARGET_OS_WASI
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFSocket.h>
#endif
#include <CoreFoundation/CFBundlePriv.h>


CF_EXTERN_C_BEGIN

CF_EXPORT void _CFRuntimeSetCFMPresent(void *a);

#if !TARGET_OS_WASI
CF_EXPORT const char *_CFProcessPath(void);
CF_EXPORT const char **_CFGetProcessPath(void);
CF_EXPORT const char **_CFGetProgname(void);

#if !TARGET_OS_WIN32
#include <sys/types.h>

CF_EXPORT void _CFGetUGIDs(uid_t *euid, gid_t *egid);
CF_EXPORT uid_t _CFGetEUID(void);
CF_EXPORT uid_t _CFGetEGID(void);
#endif
#endif

#if (TARGET_OS_MAC && !(TARGET_OS_IPHONE || TARGET_OS_LINUX))
CF_EXPORT void _CFRunLoopSetCurrent(CFRunLoopRef rl);
#endif


#if (TARGET_OS_MAC && !(TARGET_OS_IPHONE || TARGET_OS_LINUX)) || TARGET_OS_IPHONE
CF_EXPORT CFRunLoopRef CFRunLoopGetMain(void);
CF_EXPORT SInt32 CFRunLoopRunSpecific(CFRunLoopRef rl, CFStringRef modeName, CFTimeInterval seconds, Boolean returnAfterSourceHandled);


CF_EXPORT void _CFRunLoopStopMode(CFRunLoopRef rl, CFStringRef modeName);

CF_EXPORT CFIndex CFMachPortGetQueuedMessageCount(CFMachPortRef mp);

CF_EXPORT CFPropertyListRef _CFURLCopyPropertyListRepresentation(CFURLRef url);
#endif
CF_EXPORT CFPropertyListRef _CFURLCopyPropertyListRepresentation(CFURLRef url);
CF_EXPORT CFURLRef _CFURLCreateFromPropertyListRepresentation(CFAllocatorRef alloc, CFPropertyListRef pListRepresentation);

CF_EXPORT void CFPreferencesFlushCaches(void);





#if TARGET_OS_WIN32
CF_EXPORT Boolean _CFURLGetWideFileSystemRepresentation(CFURLRef url, Boolean resolveAgainstBase, wchar_t *buffer, CFIndex bufferLength);
#endif

#if !TARGET_RT_64_BIT
#if TARGET_OS_OSX
struct FSSpec;
CF_EXPORT
Boolean _CFGetFSSpecFromURL(CFAllocatorRef alloc, CFURLRef url, struct FSSpec *spec);

CF_EXPORT
CFURLRef _CFCreateURLFromFSSpec(CFAllocatorRef alloc, const struct FSSpec *voidspec, Boolean isDirectory);
#endif
#endif

typedef CF_ENUM(CFIndex, CFURLComponentDecomposition) {
	kCFURLComponentDecompositionNonHierarchical,
	kCFURLComponentDecompositionRFC1808, /* use this for RFC 1738 decompositions as well */
	kCFURLComponentDecompositionRFC2396
};

typedef struct {
	CFStringRef scheme;
	CFStringRef schemeSpecific;
} CFURLComponentsNonHierarchical;

typedef struct {
	CFStringRef scheme;
	CFStringRef user;
	CFStringRef password;
	CFStringRef host;
	CFIndex port; /* kCFNotFound means ignore/omit */
	CFArrayRef pathComponents;
	CFStringRef parameterString;
	CFStringRef query;
	CFStringRef fragment;
	CFURLRef baseURL;
} CFURLComponentsRFC1808;

typedef struct {
	CFStringRef scheme;

	/* if the registered name form of the net location is used, userinfo is NULL, port is kCFNotFound, and host is the entire registered name. */
	CFStringRef userinfo;
	CFStringRef host;
	CFIndex port;

	CFArrayRef pathComponents;
	CFStringRef query;
	CFStringRef fragment;
	CFURLRef baseURL;
} CFURLComponentsRFC2396;

/* Fills components and returns TRUE if the URL can be decomposed according to decompositionType; FALSE (leaving components unchanged) otherwise.  components should be a pointer to the CFURLComponents struct defined above that matches decompositionStyle */
CF_EXPORT
Boolean _CFURLCopyComponents(CFURLRef url, CFURLComponentDecomposition decompositionType, void *components);

/* Creates and returns the URL described by components; components should point to the CFURLComponents struct defined above that matches decompositionType. */
CF_EXPORT
CFURLRef _CFURLCreateFromComponents(CFAllocatorRef alloc, CFURLComponentDecomposition decompositionType, const void *components);
#define CFURLCopyComponents _CFURLCopyComponents
#define CFURLCreateFromComponents _CFURLCreateFromComponents



CF_EXPORT Boolean _CFStringGetFileSystemRepresentation(CFStringRef string, UInt8 *buffer, CFIndex maxBufLen);

/* If this is publicized, we might need to create a GetBytesPtr type function as well. */
CF_EXPORT CFStringRef _CFStringCreateWithBytesNoCopy(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean externalFormat, CFAllocatorRef contentsDeallocator);

#if !TARGET_OS_WASI
/* These return NULL on MacOS 8 */
// This one leaks the returned string in order to be thread-safe.
// CF cannot help you in this matter if you continue to use this SPI.
CF_EXPORT
CFStringRef CFGetUserName(void);

CF_EXPORT
CFStringRef CFCopyUserName(void);

CF_EXPORT
CFURLRef CFCopyHomeDirectoryURLForUser(CFStringRef uName);	/* Pass NULL for the current user's home directory */
#endif


/*
	CFCopySearchPathForDirectoriesInDomains returns the various
	standard system directories where apps, resources, etc get
	installed. Because queries can return multiple directories,
	you get back a CFArray (which you should free when done) of
	CFURLs. The directories are returned in search path order;
	that is, the first place to look is returned first. This API
	may return directories that do not exist yet. If NSUserDomain
	is included in a query, then the results will contain "~" to
	refer to the user's directory. Specify expandTilde to expand
	this to the current user's home. Some calls might return no
	directories!
	??? On MacOS 8 this function currently returns an empty array.
*/
typedef CF_ENUM(CFIndex, CFSearchPathDirectory) {
    kCFApplicationDirectory = 1,	/* supported applications (Applications) */
    kCFDemoApplicationDirectory,	/* unsupported applications, demonstration versions (Demos) */
    kCFDeveloperApplicationDirectory,	/* developer applications (Developer/Applications) */
    kCFAdminApplicationDirectory,	/* system and network administration applications (Administration) */
    kCFLibraryDirectory, 		/* various user-visible documentation, support, and configuration files, resources (Library) */
    kCFDeveloperDirectory,		/* developer resources (Developer) */
    kCFUserDirectory,			/* user home directories (Users) */
    kCFDocumentationDirectory,		/* documentation (Documentation) */
    kCFDocumentDirectory,		/* documents (Library/Documents) */

    kCFCoreServiceDirectory = 10,            // location of CoreServices directory (System/Library/CoreServices)
    kCFAutosavedInformationDirectory = 11,   // location of autosaved documents (Documents/Autosaved)
    kCFDesktopDirectory = 12,                // location of user's desktop
    kCFCachesDirectory = 13,                 // location of discardable cache files (Library/Caches)
    kCFApplicationSupportDirectory = 14,     // location of application support files (plug-ins, etc) (Library/Application Support)
    kCFDownloadsDirectory = 15,              // location of the user's "Downloads" directory
    kCFInputMethodsDirectory = 16,           // input methods (Library/Input Methods)
    kCFMoviesDirectory = 17,                 // location of user's Movies directory (~/Movies)
    kCFMusicDirectory = 18,                  // location of user's Music directory (~/Music)
    kCFPicturesDirectory = 19,               // location of user's Pictures directory (~/Pictures)
    kCFPrinterDescriptionDirectory = 20,     // location of system's PPDs directory (Library/Printers/PPDs)
    kCFSharedPublicDirectory = 21,           // location of user's Public sharing directory (~/Public)
    kCFPreferencePanesDirectory = 22,        // location of the PreferencePanes directory for use with System Preferences (Library/PreferencePanes)

    kCFAllApplicationsDirectory = 100,	/* all directories where applications can occur (ie Applications, Demos, Administration, Developer/Applications) */
    kCFAllLibrariesDirectory = 101	/* all directories where resources can occur (Library, Developer) */
};

typedef CF_OPTIONS(CFOptionFlags, CFSearchPathDomainMask) {
    kCFUserDomainMask = 1,	/* user's home directory --- place to install user's personal items (~) */
    kCFLocalDomainMask = 2,	/* local to the current machine --- place to install items available to everyone on this machine (/Local) */
    kCFNetworkDomainMask = 4, 	/* publically available location in the local area network --- place to install items available on the network (/Network) */
    kCFSystemDomainMask = 8,	/* provided by Apple, unmodifiable (/System) */
    kCFAllDomainsMask = 0x0ffff	/* all domains: all of the above and more, future items */
};

#if TARGET_OS_MAC || TARGET_OS_EMBEDDED
CF_EXPORT
CFArrayRef CFCopySearchPathForDirectoriesInDomains(CFSearchPathDirectory directory, CFSearchPathDomainMask domainMask, Boolean expandTilde);
#endif


/* Obsolete keys */
CF_EXPORT const CFStringRef kCFFileURLExists;
CF_EXPORT const CFStringRef kCFFileURLPOSIXMode;
CF_EXPORT const CFStringRef kCFFileURLSize;
CF_EXPORT const CFStringRef kCFFileURLDirectoryContents;
CF_EXPORT const CFStringRef kCFFileURLLastModificationTime;
CF_EXPORT const CFStringRef kCFHTTPURLStatusCode;
CF_EXPORT const CFStringRef kCFHTTPURLStatusLine;


/* System Version file access */
CF_EXPORT CFStringRef CFCopySystemVersionString(void);			// Human-readable string containing both marketing and build version
CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary(void);
CF_EXPORT CFDictionaryRef _CFCopyServerVersionDictionary(void);

// Returns the 'true' contents of SystemVersion.plist even when running in apps linked before 10.16
CF_EXPORT CFDictionaryRef _CFCopySystemVersionPlatformDictionary(void) API_AVAILABLE(macos(10.16), ios(14.0), watchos(7.0), tvos(14.0));

CF_EXPORT CFStringRef _CFCopySystemVersionDictionaryValue(CFStringRef key) API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
CF_EXPORT const CFStringRef _kCFSystemVersionProductNameKey;
CF_EXPORT const CFStringRef _kCFSystemVersionProductCopyrightKey;
CF_EXPORT const CFStringRef _kCFSystemVersionProductVersionKey;
CF_EXPORT const CFStringRef _kCFSystemVersionProductVersionExtraKey;
CF_EXPORT const CFStringRef _kCFSystemVersionProductUserVisibleVersionKey;	// For loginwindow; see 2987512
CF_EXPORT const CFStringRef _kCFSystemVersionBuildVersionKey;		
CF_EXPORT const CFStringRef _kCFSystemVersionProductVersionStringKey;	// Localized string for the string "Version"
CF_EXPORT const CFStringRef _kCFSystemVersionBuildStringKey;		// Localized string for the string "Build"


CF_EXPORT void CFMergeSortArray(void *list, CFIndex count, CFIndex elementSize, CFComparatorFunction comparator, void *context);
CF_EXPORT void CFQSortArray(void *list, CFIndex count, CFIndex elementSize, CFComparatorFunction comparator, void *context);

CF_EXPORT CFHashCode CFHashBytes(UInt8 *bytes, CFIndex length);

// For non-Darwin platforms _CFExecutableLinkedOnOrAfter(…) always returns true.

typedef CF_ENUM(CFIndex, CFSystemVersion) {
    CFSystemVersionCheetah = 0,         /* 10.0 */
    CFSystemVersionPuma = 1,            /* 10.1 */
    CFSystemVersionJaguar = 2,          /* 10.2 */
    CFSystemVersionPanther = 3,         /* 10.3 */
    CFSystemVersionTiger = 4,           /* 10.4 */
    CFSystemVersionLeopard = 5,         /* 10.5 */
    CFSystemVersionSnowLeopard = 6,	/* 10.6 */
    CFSystemVersionLion = 7,		/* 10.7 */
    CFSystemVersionMountainLion = 8,    /* 10.8 */
    CFSystemVersionMax,                 /* This should bump up when new entries are added */

};

CF_EXPORT Boolean _CFExecutableLinkedOnOrAfter(CFSystemVersion version);

typedef CF_ENUM(CFIndex, CFStringCharacterClusterType) {
    kCFStringGraphemeCluster = 1, /* Unicode Grapheme Cluster */
    kCFStringComposedCharacterCluster = 2, /* Compose all non-base (including spacing marks) */
    kCFStringCursorMovementCluster = 3, /* Cluster suitable for cursor movements */
    kCFStringBackwardDeletionCluster = 4 /* Cluster suitable for backward deletion */
};

CF_EXPORT CFRange CFStringGetRangeOfCharacterClusterAtIndex(CFStringRef string, CFIndex charIndex, CFStringCharacterClusterType type);

// Compatibility kCFCompare flags. Use the new public kCFCompareDiacriticInsensitive
enum {
    kCFCompareDiacriticsInsensitive = 128 /* Use kCFCompareDiacriticInsensitive */
};

/* kCFCompare flags planned to be publicized (Aki 10/20/2008 Does not work with kCFCompareForceOrdering/CFStringFold). see <rdar://problem/6305147>)
 */
enum {
    kCFCompareIgnoreNonAlphanumeric = (1UL << 16), // Ignores characters NOT in kCFCharacterSetAlphaNumeric
};


/* CFStringEncoding SPI */
/* When set, CF encoding conversion engine keeps ASCII compatibility. (i.e. ASCII backslash <-> Unicode backslash in MacJapanese */
CF_EXPORT void _CFStringEncodingSetForceASCIICompatibility(Boolean flag);

#if defined(CF_INLINE)
CF_INLINE const UniChar *CFStringGetCharactersPtrFromInlineBuffer(CFStringInlineBuffer *buf, CFRange desiredRange) {
    if ((desiredRange.location < 0) || ((desiredRange.location + desiredRange.length) > buf->rangeToBuffer.length)) return NULL;

    if (buf->directUniCharBuffer) {
        return buf->directUniCharBuffer + buf->rangeToBuffer.location + desiredRange.location;
    } else {
        if (desiredRange.length > __kCFStringInlineBufferLength) return NULL;

        if (((desiredRange.location + desiredRange.length) > buf->bufferedRangeEnd) || (desiredRange.location < buf->bufferedRangeStart)) {
            buf->bufferedRangeStart = desiredRange.location;
            buf->bufferedRangeEnd = buf->bufferedRangeStart + __kCFStringInlineBufferLength;
            if (buf->bufferedRangeEnd > buf->rangeToBuffer.length) buf->bufferedRangeEnd = buf->rangeToBuffer.length;
            CFIndex location = buf->rangeToBuffer.location + buf->bufferedRangeStart;
            CFIndex length = buf->bufferedRangeEnd - buf->bufferedRangeStart;
            if (buf->directCStringBuffer) {
                UniChar *bufPtr = buf->buffer;
                while (length--) *bufPtr++ = (UniChar)buf->directCStringBuffer[location++];
            } else {
                CFStringGetCharacters(buf->theString, CFRangeMake(location, length), buf->buffer);
            }
        }

        return buf->buffer + (desiredRange.location - buf->bufferedRangeStart);
    }
}

CF_INLINE void CFStringGetCharactersFromInlineBuffer(CFStringInlineBuffer *buf, CFRange desiredRange, UniChar *outBuf) {
    if (buf->directUniCharBuffer) {
        memmove(outBuf, buf->directUniCharBuffer + buf->rangeToBuffer.location + desiredRange.location, desiredRange.length * sizeof(UniChar));
    } else {
        if ((desiredRange.location >= buf->bufferedRangeStart) && (desiredRange.location < buf->bufferedRangeEnd)) {
            CFIndex bufLen = desiredRange.length;

            if (bufLen > (buf->bufferedRangeEnd - desiredRange.location)) bufLen = (buf->bufferedRangeEnd - desiredRange.location);

            memmove(outBuf, buf->buffer + (desiredRange.location - buf->bufferedRangeStart), bufLen * sizeof(UniChar));
            outBuf += bufLen; desiredRange.location += bufLen; desiredRange.length -= bufLen;
        } else {
            CFIndex desiredRangeMax = (desiredRange.location + desiredRange.length);

            if ((desiredRangeMax > buf->bufferedRangeStart) && (desiredRangeMax < buf->bufferedRangeEnd)) {
                desiredRange.length = (buf->bufferedRangeStart - desiredRange.location);
                memmove(outBuf + desiredRange.length, buf->buffer, (desiredRangeMax - buf->bufferedRangeStart) * sizeof(UniChar));
            }
        }

        if (desiredRange.length > 0) {
            CFIndex location = buf->rangeToBuffer.location + desiredRange.location;
            CFIndex length = desiredRange.length;
            if (buf->directCStringBuffer) {
                UniChar *bufPtr = outBuf;
                while (length--) *bufPtr++ = (UniChar)buf->directCStringBuffer[location++];
            } else {
                CFStringGetCharacters(buf->theString, CFRangeMake(location, length), outBuf);
            }
        }
    }
}

#else
#define CFStringGetCharactersPtrFromInlineBuffer(buf, desiredRange) ((buf)->directUniCharBuffer ? (buf)->directUniCharBuffer + (buf)->rangeToBuffer.location + desiredRange.location : NULL)

#define CFStringGetCharactersFromInlineBuffer(buf, desiredRange, outBuf) \
    if (buf->directUniCharBuffer) memmove(outBuf, (buf)->directUniCharBuffer + (buf)->rangeToBuffer.location + desiredRange.location, desiredRange.length * sizeof(UniChar)); \
    else CFStringGetCharacters((buf)->theString, CFRangeMake((buf)->rangeToBuffer.location + desiredRange.location, desiredRange.length), outBuf);

#endif /* CF_INLINE */


#if defined(CF_INLINE)

#ifndef __kCFStringAppendBufferLength
    #define __kCFStringAppendBufferLength 1024
#endif
typedef struct {
    UniChar buffer[__kCFStringAppendBufferLength];
    CFIndex bufferIndex;
    CFMutableStringRef theString;
} CFStringAppendBuffer;


// Initializes CFStringAppendBuffer with new mutable string.
CF_INLINE void CFStringInitAppendBuffer(CFAllocatorRef alloc, CFStringAppendBuffer *buf)
{
    buf->bufferIndex = 0;
    buf->theString = CFStringCreateMutable(alloc, 0);
}

// Releases an initialized CFStringAppendBuffer
CF_INLINE void CFStringReleaseAppendBuffer(CFStringAppendBuffer *buf)
{
    if ( buf->theString ) {
        CFRelease(buf->theString);
        buf->theString = NULL;
    }
}

// Appends the characters of a string to the CFStringAppendBuffer.
CF_INLINE void CFStringAppendStringToAppendBuffer(CFStringAppendBuffer *buf, CFStringRef appendedString)
{
    CFIndex numChars = CFStringGetLength(appendedString);
    if ( numChars > __kCFStringAppendBufferLength ) {
        if ( buf->bufferIndex ) {
            CFStringAppendCharacters(buf->theString, buf->buffer, buf->bufferIndex);
            buf->bufferIndex = 0;
        }
        CFStringAppend(buf->theString, appendedString);
    }
    else {
        if ( (buf->bufferIndex + numChars) > __kCFStringAppendBufferLength ) {
            CFStringAppendCharacters(buf->theString, buf->buffer, buf->bufferIndex);
            buf->bufferIndex = 0;
        }
        CFStringGetCharacters(appendedString, CFRangeMake(0, numChars), &buf->buffer[buf->bufferIndex]);
        buf->bufferIndex += numChars;
    }
}

// Appends a buffer of Unicode characters to the CFStringAppendBuffer.
CF_INLINE void CFStringAppendCharactersToAppendBuffer(CFStringAppendBuffer *buf, const UniChar *chars, CFIndex numChars)
{
    if ( numChars > __kCFStringAppendBufferLength ) {
        if ( buf->bufferIndex ) {
            CFStringAppendCharacters(buf->theString, buf->buffer, buf->bufferIndex);
            buf->bufferIndex = 0;
        }
        CFStringAppendCharacters(buf->theString, chars, numChars);
    }
    else {
        if ( (buf->bufferIndex + numChars) > __kCFStringAppendBufferLength ) {
            CFStringAppendCharacters(buf->theString, buf->buffer, buf->bufferIndex);
            buf->bufferIndex = 0;
        }
        memcpy(&buf->buffer[buf->bufferIndex], chars, numChars * sizeof(UniChar));
        buf->bufferIndex += numChars;
    }
}

// Returns a mutable string from the CFStringAppendBuffer.
CF_INLINE CFMutableStringRef CFStringCreateMutableWithAppendBuffer(CFStringAppendBuffer *buf)
{
    if ( buf->bufferIndex ) {
        CFStringAppendCharacters(buf->theString, buf->buffer, buf->bufferIndex);
        buf->bufferIndex = 0;
    }
    CFMutableStringRef result = buf->theString;
    buf->theString = NULL;
    return ( result );
}

#endif /* CF_INLINE */

/*
 CFCharacterSetInlineBuffer related declarations
 */
/*!
@typedef CFCharacterSetInlineBuffer
 @field cset The character set this inline buffer is initialized with.
 The object is not retained by the structure.
 @field flags The field is a bit mask that carries various settings.
 @field rangeStart The beginning of the character range that contains all members.
 It is guaranteed that there is no member below this value.
 @field rangeLimit The end of the character range that contains all members.
 It is guaranteed that there is no member above and equal to this value.
 @field bitmap The bitmap data representing the membership of the Basic Multilingual Plane characters.
 If NULL, all BMP characters inside the range are members of the character set.
 */
typedef struct {
    CFCharacterSetRef cset;
    uint32_t flags;
    uint32_t rangeStart;
    uint32_t rangeLimit;
    const uint8_t *bitmap;
} CFCharacterSetInlineBuffer;

// Bits for flags field
enum {
    kCFCharacterSetIsCompactBitmap = (1UL << 0),
    kCFCharacterSetNoBitmapAvailable = (1UL << 1),
    kCFCharacterSetIsInverted = (1UL << 2)
};

/*!
@function CFCharacterSetInitInlineBuffer
 Initializes buffer with cset.
 @param cset The character set used to initialized the buffer.
 If this parameter is not a valid CFCharacterSet, the behavior is undefined.
 @param buffer The reference to the inline buffer to be initialized.
 */
CF_EXPORT
void CFCharacterSetInitInlineBuffer(CFCharacterSetRef cset, CFCharacterSetInlineBuffer *buffer);

/*!
@function CFCharacterSetInlineBufferIsLongCharacterMember
 Reports whether or not the UTF-32 character is in the character set.
	@param buffer The reference to the inline buffer to be searched.
	@param character The UTF-32 character for which to test against the
 character set.
 @result true, if the value is in the character set, otherwise false.
 */
#if defined(CF_INLINE)
CF_INLINE bool CFCharacterSetInlineBufferIsLongCharacterMember(const CFCharacterSetInlineBuffer *buffer, UTF32Char character) {
    bool isInverted = ((0 == (buffer->flags & kCFCharacterSetIsInverted)) ? false : true);

    if ((character >= buffer->rangeStart) && (character < buffer->rangeLimit)) {
        if ((character > 0xFFFF) || (0 != (buffer->flags & kCFCharacterSetNoBitmapAvailable))) return (CFCharacterSetIsLongCharacterMember(buffer->cset, character) != 0);
        if (NULL == buffer->bitmap) {
            if (0 == (buffer->flags & kCFCharacterSetIsCompactBitmap)) isInverted = !isInverted;
        } else if (0 == (buffer->flags & kCFCharacterSetIsCompactBitmap)) {
            if (buffer->bitmap[character >> 3] & (1UL << (character & 7))) isInverted = !isInverted;
        } else {
            uint8_t value = buffer->bitmap[character >> 8];
            
            if (value == 0xFF) {
                isInverted = !isInverted;
            } else if (value > 0) {
                const uint8_t *segment = buffer->bitmap + (256 + (32 * (value - 1)));
                character &= 0xFF;
                if (segment[character >> 3] & (1UL << (character % 8))) isInverted = !isInverted;
            }
        }
    }
    return isInverted;
}
#else /* CF_INLINE */
#define CFCharacterSetInlineBufferIsLongCharacterMember(buffer, character) (CFCharacterSetIsLongCharacterMember(buffer->cset, character))
#endif /* CF_INLINE */


#if TARGET_OS_WIN32
CF_EXPORT CFMutableStringRef _CFCreateApplicationRepositoryPath(CFAllocatorRef alloc, int nFolder);
#endif

#if DEPLOYMENT_RUNTIME_SWIFT
#else
CF_EXPORT CFTypeRef _CFTryRetain(CFTypeRef cf);
CF_EXPORT Boolean _CFIsDeallocating(CFTypeRef cf);
#endif

// The following functions can be used when you know for certain that the types involved are not objc types. Should only be used in the macro in NSPrivateDecls.h. You cannot generally guess which "CF" objects might secretly be ObjC ones.
CF_EXPORT Boolean _CFNonObjCEqual(CFTypeRef cf1, CFTypeRef cf2);
CF_EXPORT CFTypeRef _CFNonObjCRetain(CFTypeRef cf);
CF_EXPORT void _CFNonObjCRelease(CFTypeRef cf);
CF_EXPORT CFHashCode _CFNonObjCHash(CFTypeRef cf);

/*
 CFLocaleGetLanguageRegionEncodingForLocaleIdentifier gets the appropriate language and region codes,
 and the default legacy script code and encoding, for the specified locale (or language) string.
 Returns false if CFLocale has no information about the given locale; otherwise may set
 *langCode and/or *regCode to -1 if there is no appropriate legacy value for the locale.
 This is a replacement for the CFBundle SPI CFBundleGetLocalizationInfoForLocalization (which was intended to be temporary and transitional);
 this function is more up-to-date in its handling of locale strings, and is in CFLocale where this functionality should belong. Compared
 to CFBundleGetLocalizationInfoForLocalization, this function does not spcially interpret a NULL localeIdentifier to mean use the single most
 preferred localization in the current context (this function returns NO for a NULL localeIdentifier); and in this function
 langCode, regCode, and scriptCode are all SInt16* (not SInt32* like the equivalent parameters in CFBundleGetLocalizationInfoForLocalization).
*/
CF_EXPORT
Boolean CFLocaleGetLanguageRegionEncodingForLocaleIdentifier(CFStringRef localeIdentifier, LangCode *langCode, RegionCode *regCode, ScriptCode *scriptCode, CFStringEncoding *stringEncoding);

CF_EXPORT void _CFCalendarResetCurrent(void);

#if TARGET_OS_WIN32
CF_EXPORT CFMutableStringRef _CFCreateApplicationRepositoryPath(CFAllocatorRef alloc, int nFolder);
#endif


#if (TARGET_OS_MAC && !(TARGET_OS_IPHONE || TARGET_OS_LINUX)) || TARGET_OS_IPHONE
#include <CoreFoundation/CFMessagePort.h>

CF_EXPORT CFMessagePortRef CFMessagePortCreatePerProcessLocal(CFAllocatorRef allocator, CFStringRef name, CFMessagePortCallBack callout, CFMessagePortContext *context, Boolean *shouldFreeInfo);
CF_EXPORT CFMessagePortRef CFMessagePortCreatePerProcessRemote(CFAllocatorRef allocator, CFStringRef name, CFIndex pid);


typedef CFDataRef (*CFMessagePortCallBackEx)(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info, void *trailer, uintptr_t);

CF_EXPORT CFMessagePortRef _CFMessagePortCreateLocalEx(CFAllocatorRef allocator, CFStringRef name, Boolean perPID, uintptr_t unused, CFMessagePortCallBackEx callout2, CFMessagePortContext *context, Boolean *shouldFreeInfo);

#endif

#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#if _POSIX_THREADS
#include <pthread.h>
#endif
#include <time.h>

CF_INLINE CFAbsoluteTime _CFAbsoluteTimeFromFileTimeSpec(struct timespec ts) {
    return (CFAbsoluteTime)((CFTimeInterval)ts.tv_sec - kCFAbsoluteTimeIntervalSince1970) + (1.0e-9 * (CFTimeInterval)ts.tv_nsec);
}

CF_INLINE struct timespec _CFFileTimeSpecFromAbsoluteTime(CFAbsoluteTime at) {
   struct timespec ts;
   double sec = 0.0;
   double frac = modf(at, &sec);
   if (frac < 0.0) {
       frac += 1.0;
       sec -= 1.0;
   }
#if TARGET_OS_WIN32
   ts.tv_sec = (long)(sec + kCFAbsoluteTimeIntervalSince1970);
#else
   ts.tv_sec = (time_t)(sec + kCFAbsoluteTimeIntervalSince1970);
#endif
   ts.tv_nsec = (long)(1000000000UL * frac + 0.5);
   return ts;
}

// The 'filtered' function below is preferred to this older one
CF_EXPORT bool _CFPropertyListCreateSingleValue(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFStringRef keyPath, CFPropertyListRef *value, CFErrorRef *error);

// Returns a subset of the property list, only including the keyPaths in the CFSet. If the top level object is not a dictionary, you will get back an empty dictionary as the result.
CF_EXPORT bool _CFPropertyListCreateFiltered(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFSetRef keyPaths, CFPropertyListRef *value, CFErrorRef *error) API_AVAILABLE(macos(10.8), ios(6.0), watchos(2.0), tvos(9.0));

// Returns a set of the keys of the top-level dictionary of a plist. Optimized for bplist (though it works with XML too).  Only supports string keys. 
CF_EXPORT CFSetRef _CFPropertyListCopyTopLevelKeys(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFErrorRef *outError) API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));

// Returns 'true' if the given 'data' can be determined to be a valid property list. If possible (right now, this means binary plist only) it does this without maintaining the entire object graph for lower overall memory usage.
CF_EXPORT bool _CFPropertyListValidateData(CFDataRef data, CFTypeID *outTopLevelTypeID) API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));

// Returns a subset of a bundle's Info.plist. The keyPaths follow the same rules as above CFPropertyList function. This function takes platform and product keys into account.
typedef CF_OPTIONS(CFOptionFlags, _CFBundleFilteredPlistOptions) {
    _CFBundleFilteredPlistMemoryMapped = 1
} API_AVAILABLE(macos(10.8), ios(6.0), watchos(2.0), tvos(9.0));

#if !TARGET_OS_WASI
CF_EXPORT CFPropertyListRef _CFBundleCreateFilteredInfoPlist(CFBundleRef bundle, CFSetRef keyPaths, _CFBundleFilteredPlistOptions options) API_AVAILABLE(macos(10.8), ios(6.0), watchos(2.0), tvos(9.0));
CF_EXPORT CFPropertyListRef _CFBundleCreateFilteredLocalizedInfoPlist(CFBundleRef bundle, CFSetRef keyPaths, CFStringRef localizationName, _CFBundleFilteredPlistOptions options) API_AVAILABLE(macos(10.8), ios(6.0), watchos(2.0), tvos(9.0));
#endif

#if TARGET_OS_WIN32
#include <CoreFoundation/CFNotificationCenter.h>

CF_EXPORT CFStringRef _CFGetWindowsAppleAppDataDirectory(void);
CF_EXPORT CFArrayRef _CFGetWindowsBinaryDirectories(void);
CF_EXPORT CFStringRef _CFGetWindowsAppleSystemLibraryDirectory(void);

// If your Windows application does not use a CFRunLoop on the main thread (perhaps because it is reserved for handling UI events via Windows API), then call this function to make distributed notifications arrive using a different run loop.
CF_EXPORT void _CFNotificationCenterSetRunLoop(CFNotificationCenterRef nc, CFRunLoopRef rl);

CF_EXPORT uint32_t /*DWORD*/ _CFRunLoopGetWindowsMessageQueueMask(CFRunLoopRef rl, CFStringRef modeName);
CF_EXPORT void _CFRunLoopSetWindowsMessageQueueMask(CFRunLoopRef rl, uint32_t /*DWORD*/ mask, CFStringRef modeName);

CF_EXPORT uint32_t /*DWORD*/ _CFRunLoopGetWindowsThreadID(CFRunLoopRef rl);

typedef void (*CFWindowsMessageQueueHandler)(void);

// Run Loop parameter must be the current thread's run loop for the next two functions; you cannot use another thread's run loop
CF_EXPORT CFWindowsMessageQueueHandler _CFRunLoopGetWindowsMessageQueueHandler(CFRunLoopRef rl, CFStringRef modeName);
CF_EXPORT void _CFRunLoopSetWindowsMessageQueueHandler(CFRunLoopRef rl, CFStringRef modeName, CFWindowsMessageQueueHandler func);

#endif


CF_EXPORT CFArrayRef CFDateFormatterCreateDateFormatsFromTemplates(CFAllocatorRef allocator, CFArrayRef tmplates, CFOptionFlags options, CFLocaleRef locale);

#if TARGET_OS_IPHONE
// Available for internal use on embedded
CF_EXPORT CFNotificationCenterRef CFNotificationCenterGetDistributedCenter(void);
#endif

CF_EXPORT const CFStringRef kCFNumberFormatterUsesCharacterDirection API_AVAILABLE(macos(10.9), ios(6.0), watchos(2.0), tvos(9.0));	// CFBoolean
CF_EXPORT const CFStringRef kCFDateFormatterUsesCharacterDirection API_AVAILABLE(macos(10.9), ios(6.0), watchos(2.0), tvos(9.0));	// CFBoolean


CF_EXPORT void _CFGetPathExtensionRangesFromPathComponentUniChars(const UniChar *uchars, CFIndex ucharsLength, CFRange *outPrimaryExtRange, CFRange *outSecondaryExtRange) API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT void _CFGetPathExtensionRangesFromPathComponent(CFStringRef inName, CFRange *outPrimaryExtRange, CFRange *outSecondaryExtRange) API_AVAILABLE(macos(10.11), ios(9.0), watchos(2.0), tvos(9.0));
CF_EXPORT Boolean _CFExtensionUniCharsIsValidToAppend(const UniChar *uchars, CFIndex ucharsLength) API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
CF_EXPORT Boolean _CFExtensionIsValidToAppend(CFStringRef extension) API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));


#if !DEPLOYMENT_RUNTIME_OBJC

// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
// Version 0.8

// note: All paths set in these environment variables must be absolute.

/// a single base directory relative to which user-specific data files should be written. This directory is defined by the environment variable $XDG_DATA_HOME.
CF_EXPORT CFStringRef _CFXDGCreateDataHomePath(void) CF_RETURNS_RETAINED;

/// a single base directory relative to which user-specific configuration files should be written. This directory is defined by the environment variable $XDG_CONFIG_HOME.
CF_EXPORT CFStringRef _CFXDGCreateConfigHomePath(void) CF_RETURNS_RETAINED;

/// a set of preference ordered base directories relative to which data files should be searched. This set of directories is defined by the environment variable $XDG_DATA_DIRS.
CF_EXPORT CFArrayRef _CFXDGCreateDataDirectoriesPaths(void) CF_RETURNS_RETAINED;

/// a set of preference ordered base directories relative to which configuration files should be searched. This set of directories is defined by the environment variable $XDG_CONFIG_DIRS.
CF_EXPORT CFArrayRef _CFXDGCreateConfigDirectoriesPaths(void) CF_RETURNS_RETAINED;

/// a single base directory relative to which user-specific non-essential (cached) data should be written. This directory is defined by the environment variable $XDG_CACHE_HOME.
CF_EXPORT CFStringRef _CFXDGCreateCacheDirectoryPath(void) CF_RETURNS_RETAINED;

/// a single base directory relative to which user-specific runtime files and other file objects should be placed. This directory is defined by the environment variable $XDG_RUNTIME_DIR.
CF_EXPORT CFStringRef _CFXDGCreateRuntimeDirectoryPath(void) CF_RETURNS_RETAINED;

#endif // !DEPLOYMENT_RUNTIME_OBJC


/// Retrieve a local handle for an inserted (DYLD_INSERT_LIBRARIES) or interposing library.
CF_EXPORT void * _CFGetHandleForInsertedOrInterposingLibrary(char const *namePrefix) API_AVAILABLE(ios(13.0), macos(10.15), watchos(6.0), tvos(13.0));

CF_EXPORT Boolean _CFRunLoopPerCalloutAutoreleasepoolEnabled(void) API_AVAILABLE(macos(10.16), ios(14.0), watchos(7.0), tvos(14.0));
CF_EXPORT Boolean _CFRunLoopSetPerCalloutAutoreleasepoolEnabled(Boolean enabled) API_AVAILABLE(macos(10.16), ios(14.0), watchos(7.0), tvos(14.0));

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFPRIV__ */

