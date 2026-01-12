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

/*	CFPriv.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

/*
        APPLE SPI:  NOT TO BE USED OUTSIDE APPLE!
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



#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_LINUX)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
#include <CoreFoundation/CFMachPort.h>
#include <CoreFoundation/CFMessagePort.h>
#endif

#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE) || TARGET_OS_WIN32
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFSocket.h>
#include <CoreFoundation/CFBundlePriv.h>
#endif

CF_EXTERN_C_BEGIN

CF_EXPORT intptr_t _CFDoOperation(intptr_t code, intptr_t subcode1, intptr_t subcode2);

CF_EXPORT void _CFRuntimeSetCFMPresent(void *a);

CF_EXPORT const char *_CFProcessPath(void);
CF_EXPORT const char **_CFGetProcessPath(void);
CF_EXPORT const char **_CFGetProgname(void);


#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_LINUX))
CF_EXPORT void _CFRunLoopSetCurrent(CFRunLoopRef rl);
#endif

#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_LINUX)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
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

#if !__LP64__
#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
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

/* These return NULL on MacOS 8 */
// This one leaks the returned string in order to be thread-safe.
// CF cannot help you in this matter if you continue to use this SPI.
CF_EXPORT
CFStringRef CFGetUserName(void);

CF_EXPORT
CFStringRef CFCopyUserName(void);

CF_EXPORT
CFURLRef CFCopyHomeDirectoryURLForUser(CFStringRef uName);	/* Pass NULL for the current user's home directory */


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

CF_EXPORT
CFArrayRef CFCopySearchPathForDirectoriesInDomains(CFSearchPathDirectory directory, CFSearchPathDomainMask domainMask, Boolean expandTilde);


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

/* _CFExecutableLinkedOnOrAfter(releaseVersionName) will return YES if the current executable seems to be linked on or after the specified release. Example: If you specify CFSystemVersionPuma (10.1), you will get back true for executables linked on Puma or Jaguar(10.2), but false for those linked on Cheetah (10.0) or any of its software updates (10.0.x). You will also get back false for any app whose version info could not be figured out.
    This function caches its results, so no need to cache at call sites.

  Note that for non-MACH this function always returns true.
*/
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

extern void __CFSetCharToUniCharFunc(Boolean (*func)(UInt32 flags, UInt8 ch, UniChar *unicodeChar));
extern UniChar __CFCharToUniCharTable[256];


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

CF_EXPORT CFTypeRef _CFTryRetain(CFTypeRef cf);
CF_EXPORT Boolean _CFIsDeallocating(CFTypeRef cf);

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

#if TARGET_OS_WIN32
CF_EXPORT CFMutableStringRef _CFCreateApplicationRepositoryPath(CFAllocatorRef alloc, int nFolder);
#endif

#if TARGET_OS_WIN32
#include <CoreFoundation/CFMessagePort.h>

#define CF_MESSAGE_PORT_CLONE_MESSAGE_ID -1209
CF_EXPORT CFMessagePortRef	CFMessagePortCreateUber(CFAllocatorRef allocator, CFStringRef name, CFMessagePortCallBack callout, CFMessagePortContext *context, Boolean *shouldFreeInfo, Boolean isRemote);
CF_EXPORT void CFMessagePortSetCloneCallout(CFMessagePortRef ms, CFMessagePortCallBack cloneCallout);
#endif

#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_LINUX)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
#include <CoreFoundation/CFMessagePort.h>

CF_EXPORT CFMessagePortRef CFMessagePortCreatePerProcessLocal(CFAllocatorRef allocator, CFStringRef name, CFMessagePortCallBack callout, CFMessagePortContext *context, Boolean *shouldFreeInfo);
CF_EXPORT CFMessagePortRef CFMessagePortCreatePerProcessRemote(CFAllocatorRef allocator, CFStringRef name, CFIndex pid);


typedef CFDataRef (*CFMessagePortCallBackEx)(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info, void *trailer, uintptr_t);

CF_EXPORT CFMessagePortRef _CFMessagePortCreateLocalEx(CFAllocatorRef allocator, CFStringRef name, Boolean perPID, uintptr_t unused, CFMessagePortCallBackEx callout2, CFMessagePortContext *context, Boolean *shouldFreeInfo);

#endif

#if TARGET_OS_MAC || TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_LINUX
#include <pthread.h>
#else
// Avoid including the pthread header
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
struct timespec { long tv_sec; long tv_nsec; };
#endif
#endif

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
CF_EXPORT bool _CFPropertyListCreateFiltered(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFSetRef keyPaths, CFPropertyListRef *value, CFErrorRef *error) CF_AVAILABLE(10_8, 6_0);

#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE) || TARGET_OS_WIN32

// Returns a subset of a bundle's Info.plist. The keyPaths follow the same rules as above CFPropertyList function. This function takes platform and product keys into account.
typedef CF_OPTIONS(CFOptionFlags, _CFBundleFilteredPlistOptions) {
    _CFBundleFilteredPlistMemoryMapped = 1
} CF_ENUM_AVAILABLE(10_8, 6_0);

CF_EXPORT CFPropertyListRef _CFBundleCreateFilteredInfoPlist(CFBundleRef bundle, CFSetRef keyPaths, _CFBundleFilteredPlistOptions options) CF_AVAILABLE(10_8, 6_0);
CF_EXPORT CFPropertyListRef _CFBundleCreateFilteredLocalizedInfoPlist(CFBundleRef bundle, CFSetRef keyPaths, CFStringRef localizationName, _CFBundleFilteredPlistOptions options) CF_AVAILABLE(10_8, 6_0);
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

#if (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
// Available for internal use on embedded
CF_EXPORT CFNotificationCenterRef CFNotificationCenterGetDistributedCenter(void);
#endif

CF_EXPORT const CFStringRef kCFNumberFormatterUsesCharacterDirection CF_AVAILABLE(10_9, 6_0);	// CFBoolean
CF_EXPORT const CFStringRef kCFDateFormatterUsesCharacterDirection CF_AVAILABLE(10_9, 6_0);	// CFBoolean


CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFPRIV__ */

