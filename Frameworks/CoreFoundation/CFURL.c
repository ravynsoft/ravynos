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

/*	CFURL.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Jim Luther/Chris Linn
*/

#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFCharacterSetPriv.h>
#include <CoreFoundation/CFNumber.h>
#include "CFInternal.h"
#include <CoreFoundation/CFStringEncodingConverter.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#if DEPLOYMENT_TARGET_MACOSX
#include <CoreFoundation/CFNumberFormatter.h>
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <CoreFoundation/CFURLPriv.h>
#endif

#ifndef DEBUG_URL_MEMORY_USAGE
// enables various statistical counters which can be displayed with __CFURLDumpMemRecord().
#define DEBUG_URL_MEMORY_USAGE 0
#endif

#ifndef DEBUG_URL_INITIALIZER_LOGGING
// enables logging in URL initializer. You get to see the inputs and output for each URL created.
#define DEBUG_URL_INITIALIZER_LOGGING 0
#endif


static CFArrayRef WindowsPathToURLComponents(CFStringRef path, CFAllocatorRef alloc, Boolean isDir, Boolean isAbsolute);
static CFStringRef WindowsPathToURLPath(CFStringRef path, CFAllocatorRef alloc, Boolean isDir, Boolean isAbsolute);
static CFStringRef POSIXPathToURLPath(CFStringRef path, CFAllocatorRef alloc, Boolean isDirectory, Boolean isAbsolute, Boolean *posixAndUrlPathsMatch);
static CFStringRef CreateStringFromFileSystemRepresentationByAddingPercentEscapes(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex numBytes, Boolean isDirectory, Boolean isAbsolute, Boolean windowsPath, Boolean *addedPercentEncoding);
CFStringRef CFURLCreateStringWithFileSystemPath(CFAllocatorRef allocator, CFURLRef anURL, CFURLPathStyle fsType, Boolean resolveAgainstBase);
CF_EXPORT CFURLRef _CFURLCreateCurrentDirectoryURL(CFAllocatorRef allocator);
#if DEPLOYMENT_TARGET_MACOSX
static Boolean _CFURLHasFileURLScheme(CFURLRef url, Boolean *hasScheme);
#endif



// When __CONSTANT_CFSTRINGS__ is not defined, we have separate macros for static and exported constant strings, but
// when it is defined, we must prefix with static to prevent the string from being exported
#ifdef __CONSTANT_CFSTRINGS__
static CONST_STRING_DECL(kCFURLHTTPScheme, "http")
static CONST_STRING_DECL(kCFURLHTTPSScheme, "https")
static CONST_STRING_DECL(kCFURLFileScheme, "file")
static CONST_STRING_DECL(kCFURLDataScheme, "data")
static CONST_STRING_DECL(kCFURLFTPScheme, "ftp")
static CONST_STRING_DECL(kCFURLLocalhost, "localhost")
#else
CONST_STRING_DECL(kCFURLHTTPScheme, "http")
CONST_STRING_DECL(kCFURLHTTPSScheme, "https")
CONST_STRING_DECL(kCFURLFileScheme, "file")
CONST_STRING_DECL(kCFURLDataScheme, "data")
CONST_STRING_DECL(kCFURLFTPScheme, "ftp")
CONST_STRING_DECL(kCFURLLocalhost, "localhost")
#endif

#if DEBUG_URL_MEMORY_USAGE
static uint numURLs = 0;                    // number of URLs allocated
static uint numDealloced = 0;               // number of URLs deallocated
static uint numURLsParsed = 0;              // number of URLs created from a string which had to be parsed
static uint numExtraDataAllocated = 0;      // number of URLs with additional data -- either because URLHandle was used, or because a sanitizedString was needed
static uint numURLsWithBaseURL = 0;         // number of URLs with a baseURL
static uint numNonUTF8EncodedURLs = 0;      // number of URLs that don't have UTF8 encoding
#endif

/* The bit flags in myURL->_flags */
// component bits
#define HAS_SCHEME                      (0x00000001)
#define HAS_USER                        (0x00000002)
#define HAS_PASSWORD                    (0x00000004)
#define HAS_HOST                        (0x00000008)
#define HAS_PORT                        (0x00000010)
#define HAS_PATH                        (0x00000020)
#define HAS_PARAMETERS                  (0x00000040)
#define HAS_QUERY                       (0x00000080)
#define HAS_FRAGMENT                    (0x00000100)
#define MAX_COMPONENTS                  9
// various boolean flags
#define IS_IPV6_ENCODED                 (0x00000400)
#define IS_DIRECTORY                    (0x00000800)
#define IS_CANONICAL_FILE_URL           (0x00001000) // if set, the URL is a file URL in the form "file://<absolute_percent_encoded_path>" (it was created from a file system path or representation)
#define PATH_HAS_FILE_ID                (0x00002000)
#define IS_DECOMPOSABLE                 (0x00004000)
#define POSIX_AND_URL_PATHS_MATCH       (0x00008000) // POSIX_AND_URL_PATHS_MATCH will only be true if the URL path and the POSIX path are identical, character for character, except for the presence/absence of a trailing slash on directories
#define ORIGINAL_AND_URL_STRINGS_MATCH  (0x00010000)
// scheme bits and amount to shift it to translate to the kXXXXScheme enums
#define SCHEME_TYPE_MASK                (0xE0000000)
#define SCHEME_SHIFT                    29
enum {
    kHasUncommonScheme  = 0,    // scheme is uncommon or scheme isn't in the canonical form (all lower case)
    kHasHttpScheme      = 1,
    kHasHttpsScheme     = 2,
    kHasFileScheme      = 3,
    kHasDataScheme      = 4,
    kHasFtpScheme       = 5,
    kMaxScheme
};
// accessors for the scheme bits in _flags
CF_INLINE UInt32 _getSchemeTypeFromFlags(UInt32 flags);
CF_INLINE void _setSchemeTypeInFlags(UInt32 *flags, UInt32 schemeType);

// Other useful defines
#define NET_LOCATION_MASK (HAS_HOST | HAS_USER | HAS_PASSWORD | HAS_PORT)
#define RESOURCE_SPECIFIER_MASK  (HAS_PARAMETERS | HAS_QUERY | HAS_FRAGMENT)
// These flags can be compared for equality since these are all set once when the CFURL is created.
// IS_CANONICAL_FILE_URL cannot be compared since we don't always create the URL string.
// POSIX_AND_URL_PATHS_MATCH cannot be compared because it may not be set
// ORIGINAL_AND_URL_STRINGS_MATCH cannot be compared because it gets set on demand later.
#define EQUAL_FLAGS_MASK (HAS_SCHEME | HAS_USER | HAS_PASSWORD | HAS_HOST | HAS_PORT | HAS_PATH | HAS_PARAMETERS | HAS_QUERY | HAS_FRAGMENT | IS_IPV6_ENCODED | IS_DIRECTORY | PATH_HAS_FILE_ID | IS_DECOMPOSABLE | SCHEME_TYPE_MASK )

// The value of FULL_URL_REPRESENTATION must not be in the CFURLPathStyle enums. Also, its value is exposed via _CFURLCopyPropertyListRepresentation to the Finder so don't change it.  
#define FULL_URL_REPRESENTATION (0xF)

/* The bit flags in _CFURLAdditionalData->_additionalDataFlags */
/* If ORIGINAL_AND_URL_STRINGS_MATCH in myURL->_flags is false, these bits determine where they differ. XXXX_DIFFERS must match the HAS_XXXX */
#define SCHEME_DIFFERS                  HAS_SCHEME      // Scheme can actually never differ because if there were escaped characters prior to the colon, we'd interpret the string as a relative path
#define USER_DIFFERS                    HAS_USER
#define PASSWORD_DIFFERS                HAS_PASSWORD
#define HOST_DIFFERS                    HAS_HOST
#define PORT_DIFFERS                    HAS_PORT        // Port can actually never differ because if there were a non-digit following a colon in the net location, we'd interpret the whole net location as the host 
#define PATH_DIFFERS                    HAS_PATH        // unused
#define PARAMETERS_DIFFER               HAS_PARAMETERS  // unused
#define QUERY_DIFFER                    HAS_QUERY       // unused
#define FRAGMENT_DIFFER                 HAS_FRAGMENT    // unused

#define FILE_ID_PREFIX ".file"
#define FILE_ID_KEY "id"
#define FILE_ID_PREAMBLE "/.file/id="
#define FILE_ID_PREAMBLE_LENGTH 10

#define FILE_PREFIX "file://"
static const UInt8 fileURLPrefix[] = FILE_PREFIX;

// FILE_PREFIX_WITH_AUTHORITY and fileURLPrefixWithAuthority are only needed because some code incorrectly expects file URLs to have a host of "localhost", so if the application is linked on or before OS X 10.9 or iOS 7.0, we add "localhost" to file path URLs we create.
#define FILE_PREFIX_WITH_AUTHORITY "file://localhost"
static const UInt8 fileURLPrefixWithAuthority[] = FILE_PREFIX_WITH_AUTHORITY;

static Boolean AddAuthorityToFileURL(void)
{
    static Boolean result = false;
    return ( result );
}

//	In order to reduce the sizeof ( __CFURL ), move these items into a seperate structure which is
//	only allocated when necessary.  In my tests, it's almost never needed -- very rarely does a CFURL have
//	either a sanitized string or a reserved pointer for URLHandle.
struct _CFURLAdditionalData {
    void *_reserved; // Reserved for URLHandle's use.
    CFStringRef _sanitizedString; // The fully compliant RFC string.  This is only non-NULL if ORIGINAL_AND_URL_STRINGS_MATCH is false.
    UInt32 _additionalDataFlags; // these flags only apply to things we need to keep state for in _CFURLAdditionalData (like the XXXX_DIFFERS flags)
};

struct __CFURL {
    CFRuntimeBase _cfBase;
    UInt32 _flags;
    CFStringEncoding _encoding; // The encoding to use when asked to remove percent escapes
    CFStringRef _string; // Never NULL
    CFURLRef _base;
    struct _CFURLAdditionalData* _extra;
    void *_resourceInfo;    // For use by CoreServicesInternal to cache property values. Retained and released by CFURL.
    CFRange _ranges[1]; // variable length (1 to 9) array of ranges
};


CF_INLINE void* _getReserved ( const struct __CFURL* url )
{
    if ( url && url->_extra ) {
        return ( url->_extra->_reserved );
    }
    else {
        return ( NULL );
    }
}

CF_INLINE CFStringRef _getSanitizedString(const struct __CFURL* url)
{
    if ( url && url->_extra ) {
        return ( url->_extra->_sanitizedString );
    }
    else {
	return ( NULL );
    }
}

CF_INLINE UInt32 _getAdditionalDataFlags(const struct __CFURL* url)
{
    if ( url && url->_extra ) {
        return ( url->_extra->_additionalDataFlags );
    }
    else {
	return ( 0 );
    }
}

CF_INLINE void* _getResourceInfo ( const struct __CFURL* url )
{
    if ( url ) {
        return url->_resourceInfo;
    }
    else {
        return NULL;
    }
}

static void _CFURLAllocateExtraDataspace( struct __CFURL* url )
{	
    if ( url && ! url->_extra )
    {	struct _CFURLAdditionalData* extra = (struct _CFURLAdditionalData*) CFAllocatorAllocate( CFGetAllocator( url), sizeof( struct _CFURLAdditionalData ), __kCFAllocatorGCScannedMemory);
	
	extra->_reserved = _getReserved( url );
        extra->_additionalDataFlags = _getAdditionalDataFlags(url);
	extra->_sanitizedString = _getSanitizedString(url);
	
	url->_extra = extra;
	
	#if DEBUG_URL_MEMORY_USAGE
	numExtraDataAllocated ++;
	#endif
    }
}

CF_INLINE void _setReserved ( struct __CFURL* url, void* reserved )
{
    if ( url )
    {
        // Don't allocate extra space if we're just going to be storing NULL
        if ( !url->_extra && reserved )
            _CFURLAllocateExtraDataspace( url );
        
        if ( url->_extra )
            __CFAssignWithWriteBarrier((void **)&url->_extra->_reserved, reserved);
    }
}

CF_INLINE void _setSanitizedString( struct __CFURL* url, CFMutableStringRef sanitizedString )
{
    if ( url )
    {
        // Don't allocate extra space if we're just going to be storing NULL
        if ( !url->_extra && sanitizedString ) {
            _CFURLAllocateExtraDataspace( url );
        }
        
        if ( url->_extra ) {
            if ( url->_extra->_sanitizedString ) {
                CFRelease(url->_extra->_sanitizedString);
            }
            url->_extra->_sanitizedString = CFStringCreateCopy(CFGetAllocator(url), sanitizedString);

        }
    }
}

CF_INLINE void _setAdditionalDataFlags(struct __CFURL* url, UInt32 additionalDataFlags)
{
    if ( url )
    {
        // Don't allocate extra space if we're just going to be storing 0
        if ( !url->_extra && (additionalDataFlags != 0) ) {
            _CFURLAllocateExtraDataspace( url );
        }
        
        if ( url->_extra ) {
            url->_extra->_additionalDataFlags = additionalDataFlags;
        }
    }
}

CF_INLINE void _setResourceInfo ( struct __CFURL* url, void* resourceInfo )
{
    // Must be atomic
    // Never a GC object
    if ( url && OSAtomicCompareAndSwapPtrBarrier( NULL, resourceInfo, &url->_resourceInfo )) {
	CFRetain( resourceInfo );
    }
}

CF_INLINE UInt32 _getSchemeTypeFromFlags(UInt32 flags)
{
    return ( (flags & SCHEME_TYPE_MASK) >> SCHEME_SHIFT );
}

CF_INLINE void _setSchemeTypeInFlags(UInt32 *flags, UInt32 schemeType)
{
    CFAssert2((schemeType >= kHasUncommonScheme) &&  (schemeType < kMaxScheme), __kCFLogAssertion, "%s(): Received bad schemeType %d", __PRETTY_FUNCTION__, schemeType);
    *flags = (*flags & ~SCHEME_TYPE_MASK) + (schemeType << SCHEME_SHIFT);
}

static Boolean _pathHasFileIDPrefix(CFStringRef path);
static CFStringRef _resolveFileSystemPaths(CFStringRef relativePath, CFStringRef basePath, Boolean baseIsDir, CFURLPathStyle fsType, CFAllocatorRef alloc);
static void _parseComponents(CFAllocatorRef alloc, CFStringRef string, CFURLRef baseURL, UInt32 *theFlags, CFRange *packedRanges, uint8_t *numberOfRanges);
static CFRange _rangeForComponent(UInt32 flags, const CFRange *ranges, UInt32 compFlag);
static CFRange _netLocationRange(UInt32 flags, const CFRange *ranges);
static UInt32 _firstResourceSpecifierFlag(UInt32 flags);
static void computeSanitizedString(CFURLRef url);
static CFStringRef correctedComponent(CFStringRef component, UInt32 compFlag, CFStringEncoding enc);
static CFMutableStringRef resolveAbsoluteURLString(CFAllocatorRef alloc, CFStringRef relString, UInt32 relFlags, const CFRange *relRanges, CFStringRef baseString, UInt32 baseFlags, const CFRange *baseRanges);
static CFStringRef _resolvedPath(UniChar *pathStr, UniChar *end, UniChar pathDelimiter, Boolean stripLeadingDotDots, Boolean stripTrailingDelimiter, CFAllocatorRef alloc);


enum {
	VALID = 1,
	ALPHA = 2,
	PATHVALID = 4,
	SCHEME = 8,
	HEXDIGIT = 16
};

static const unsigned char sURLValidCharacters[128] = {
    /* nul   0 */   0,
    /* soh   1 */   0,
    /* stx   2 */   0,
    /* etx   3 */   0,
    /* eot   4 */   0,
    /* enq   5 */   0,
    /* ack   6 */   0,
    /* bel   7 */   0,
    /* bs    8 */   0,
    /* ht    9 */   0,
    /* nl   10 */   0,
    /* vt   11 */   0,
    /* np   12 */   0,
    /* cr   13 */   0,
    /* so   14 */   0,
    /* si   15 */   0,
    /* dle  16 */   0,
    /* dc1  17 */   0,
    /* dc2  18 */   0,
    /* dc3  19 */   0,
    /* dc4  20 */   0,
    /* nak  21 */   0,
    /* syn  22 */   0,
    /* etb  23 */   0,
    /* can  24 */   0,
    /* em   25 */   0,
    /* sub  26 */   0,
    /* esc  27 */   0,
    /* fs   28 */   0,
    /* gs   29 */   0,
    /* rs   30 */   0,
    /* us   31 */   0,
    /* sp   32 */   0,
    /* '!'  33 */   VALID | PATHVALID ,
    /* '"'  34 */   0,
    /* '#'  35 */   0,
    /* '$'  36 */   VALID | PATHVALID ,
    /* '%'  37 */   0,
    /* '&'  38 */   VALID | PATHVALID ,
    /* '''  39 */   VALID | PATHVALID ,
    /* '('  40 */   VALID | PATHVALID ,
    /* ')'  41 */   VALID | PATHVALID ,
    /* '*'  42 */   VALID | PATHVALID ,
    /* '+'  43 */   VALID | SCHEME | PATHVALID ,
    /* ','  44 */   VALID | PATHVALID ,
    /* '-'  45 */   VALID | SCHEME | PATHVALID ,
    /* '.'  46 */   VALID | SCHEME | PATHVALID ,
    /* '/'  47 */   VALID | PATHVALID ,
    /* '0'  48 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '1'  49 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '2'  50 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '3'  51 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '4'  52 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '5'  53 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '6'  54 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '7'  55 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '8'  56 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* '9'  57 */   VALID | SCHEME | PATHVALID | HEXDIGIT ,
    /* ':'  58 */   VALID ,
    /* ';'  59 */   VALID ,
    /* '<'  60 */   0,
    /* '='  61 */   VALID | PATHVALID ,
    /* '>'  62 */   0,
    /* '?'  63 */   VALID ,
    /* '@'  64 */   VALID ,
    /* 'A'  65 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'B'  66 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'C'  67 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'D'  68 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'E'  69 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'F'  70 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'G'  71 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'H'  72 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'I'  73 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'J'  74 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'K'  75 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'L'  76 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'M'  77 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'N'  78 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'O'  79 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'P'  80 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'Q'  81 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'R'  82 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'S'  83 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'T'  84 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'U'  85 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'V'  86 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'W'  87 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'X'  88 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'Y'  89 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'Z'  90 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* '['  91 */   0,
    /* '\'  92 */   0,
    /* ']'  93 */   0,
    /* '^'  94 */   0,
    /* '_'  95 */   VALID | PATHVALID ,
    /* '`'  96 */   0,
    /* 'a'  97 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'b'  98 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'c'  99 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'd' 100 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'e' 101 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'f' 102 */   VALID | ALPHA | SCHEME | PATHVALID | HEXDIGIT ,
    /* 'g' 103 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'h' 104 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'i' 105 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'j' 106 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'k' 107 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'l' 108 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'm' 109 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'n' 110 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'o' 111 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'p' 112 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'q' 113 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'r' 114 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 's' 115 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 't' 116 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'u' 117 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'v' 118 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'w' 119 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'x' 120 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'y' 121 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* 'z' 122 */   VALID | ALPHA | SCHEME | PATHVALID ,
    /* '{' 123 */   0,
    /* '|' 124 */   0,
    /* '}' 125 */   0,
    /* '~' 126 */   VALID | PATHVALID ,
    /* del 127 */   0,
};

CF_INLINE Boolean isURLLegalCharacter(UniChar ch) {
    return (ch <= 127) ? ((sURLValidCharacters[ch] & VALID) != 0) : false;
}

CF_INLINE Boolean scheme_valid(UniChar ch) {
    return (ch <= 127) ? ((sURLValidCharacters[ch] & SCHEME) != 0) : false;
}

CF_INLINE Boolean isALPHA(UniChar ch) {
    return (ch <= 127) ? ((sURLValidCharacters[ch] & ALPHA) != 0) : false;
}
/*
 Currently unused, but left in for symmetry/informative purposes
CF_INLINE Boolean isPathLegalCharacter(UniChar ch) {
    return (ch <= 127) ? ((sURLValidCharacters[ch] & PATHVALID) != 0) : false;
}
*/
CF_INLINE Boolean isHexDigit(UniChar ch) {
    return (ch <= 127) ? ((sURLValidCharacters[ch] & HEXDIGIT) != 0) : false;
}

// Returns false if ch1 or ch2 isn't properly formatted
CF_INLINE Boolean _translateBytes(UniChar ch1, UniChar ch2, uint8_t *result) {
    *result = 0;
    if (ch1 >= '0' && ch1 <= '9') *result += (ch1 - '0');
    else if (ch1 >= 'a' && ch1 <= 'f') *result += 10 + ch1 - 'a';
    else if (ch1 >= 'A' && ch1 <= 'F') *result += 10 + ch1 - 'A';
    else return false;

    *result  = (*result) << 4;
    if (ch2 >= '0' && ch2 <= '9') *result += (ch2 - '0');
    else if (ch2 >= 'a' && ch2 <= 'f') *result += 10 + ch2 - 'a';
    else if (ch2 >= 'A' && ch2 <= 'F') *result += 10 + ch2 - 'A';
    else return false;

    return true;
}

CF_INLINE Boolean _haveTestedOriginalString(CFURLRef url) {
    return ((url->_flags & ORIGINAL_AND_URL_STRINGS_MATCH) != 0) || (_getSanitizedString(url) != NULL);
}

enum {
    IS_PCHAR = 0x01,
};

static const unsigned char sURLValidBytes[256] = {
    /* nul   0 */   0,
    /* soh   1 */   0,
    /* stx   2 */   0,
    /* etx   3 */   0,
    /* eot   4 */   0,
    /* enq   5 */   0,
    /* ack   6 */   0,
    /* bel   7 */   0,
    /* bs    8 */   0,
    /* ht    9 */   0,
    /* nl   10 */   0,
    /* vt   11 */   0,
    /* np   12 */   0,
    /* cr   13 */   0,
    /* so   14 */   0,
    /* si   15 */   0,
    /* dle  16 */   0,
    /* dc1  17 */   0,
    /* dc2  18 */   0,
    /* dc3  19 */   0,
    /* dc4  20 */   0,
    /* nak  21 */   0,
    /* syn  22 */   0,
    /* etb  23 */   0,
    /* can  24 */   0,
    /* em   25 */   0,
    /* sub  26 */   0,
    /* esc  27 */   0,
    /* fs   28 */   0,
    /* gs   29 */   0,
    /* rs   30 */   0,
    /* us   31 */   0,
    /* sp   32 */   0,
    /* '!'  33 */   IS_PCHAR,
    /* '"'  34 */   0,
    /* '#'  35 */   0,
    /* '$'  36 */   IS_PCHAR,
    /* '%'  37 */   0,
    /* '&'  38 */   IS_PCHAR,
    /* '''  39 */   IS_PCHAR,
    /* '('  40 */   IS_PCHAR,
    /* ')'  41 */   IS_PCHAR,
    /* '*'  42 */   IS_PCHAR,
    /* '+'  43 */   IS_PCHAR,
    /* ','  44 */   IS_PCHAR,
    /* '-'  45 */   IS_PCHAR,
    /* '.'  46 */   IS_PCHAR,
    /* '/'  47 */   IS_PCHAR,   // not really a pchar -- it's the segment delimiter
    /* '0'  48 */   IS_PCHAR,
    /* '1'  49 */   IS_PCHAR,
    /* '2'  50 */   IS_PCHAR,
    /* '3'  51 */   IS_PCHAR,
    /* '4'  52 */   IS_PCHAR,
    /* '5'  53 */   IS_PCHAR,
    /* '6'  54 */   IS_PCHAR,
    /* '7'  55 */   IS_PCHAR,
    /* '8'  56 */   IS_PCHAR,
    /* '9'  57 */   IS_PCHAR,
    /* ':'  58 */   IS_PCHAR,
    /* ';'  59 */   0,          // we need to percent-escape ';' in file system paths so it won't be mistaken for the start of the obsolete param rule (rfc2396) that CFURL still supports
    /* '<'  60 */   0,
    /* '='  61 */   IS_PCHAR,
    /* '>'  62 */   0,
    /* '?'  63 */   0,
    /* '@'  64 */   IS_PCHAR,
    /* 'A'  65 */   IS_PCHAR,
    /* 'B'  66 */   IS_PCHAR,
    /* 'C'  67 */   IS_PCHAR,
    /* 'D'  68 */   IS_PCHAR,
    /* 'E'  69 */   IS_PCHAR,
    /* 'F'  70 */   IS_PCHAR,
    /* 'G'  71 */   IS_PCHAR,
    /* 'H'  72 */   IS_PCHAR,
    /* 'I'  73 */   IS_PCHAR,
    /* 'J'  74 */   IS_PCHAR,
    /* 'K'  75 */   IS_PCHAR,
    /* 'L'  76 */   IS_PCHAR,
    /* 'M'  77 */   IS_PCHAR,
    /* 'N'  78 */   IS_PCHAR,
    /* 'O'  79 */   IS_PCHAR,
    /* 'P'  80 */   IS_PCHAR,
    /* 'Q'  81 */   IS_PCHAR,
    /* 'R'  82 */   IS_PCHAR,
    /* 'S'  83 */   IS_PCHAR,
    /* 'T'  84 */   IS_PCHAR,
    /* 'U'  85 */   IS_PCHAR,
    /* 'V'  86 */   IS_PCHAR,
    /* 'W'  87 */   IS_PCHAR,
    /* 'X'  88 */   IS_PCHAR,
    /* 'Y'  89 */   IS_PCHAR,
    /* 'Z'  90 */   IS_PCHAR,
    /* '['  91 */   0,
    /* '\'  92 */   0,
    /* ']'  93 */   0,
    /* '^'  94 */   0,
    /* '_'  95 */   IS_PCHAR,
    /* '`'  96 */   0,
    /* 'a'  97 */   IS_PCHAR,
    /* 'b'  98 */   IS_PCHAR,
    /* 'c'  99 */   IS_PCHAR,
    /* 'd' 100 */   IS_PCHAR,
    /* 'e' 101 */   IS_PCHAR,
    /* 'f' 102 */   IS_PCHAR,
    /* 'g' 103 */   IS_PCHAR,
    /* 'h' 104 */   IS_PCHAR,
    /* 'i' 105 */   IS_PCHAR,
    /* 'j' 106 */   IS_PCHAR,
    /* 'k' 107 */   IS_PCHAR,
    /* 'l' 108 */   IS_PCHAR,
    /* 'm' 109 */   IS_PCHAR,
    /* 'n' 110 */   IS_PCHAR,
    /* 'o' 111 */   IS_PCHAR,
    /* 'p' 112 */   IS_PCHAR,
    /* 'q' 113 */   IS_PCHAR,
    /* 'r' 114 */   IS_PCHAR,
    /* 's' 115 */   IS_PCHAR,
    /* 't' 116 */   IS_PCHAR,
    /* 'u' 117 */   IS_PCHAR,
    /* 'v' 118 */   IS_PCHAR,
    /* 'w' 119 */   IS_PCHAR,
    /* 'x' 120 */   IS_PCHAR,
    /* 'y' 121 */   IS_PCHAR,
    /* 'z' 122 */   IS_PCHAR,
    /* '{' 123 */   0,
    /* '|' 124 */   0,
    /* '}' 125 */   0,
    /* '~' 126 */   IS_PCHAR,
    /* del 127 */   0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
};

CF_INLINE Boolean is_pchar(unsigned char ch) {
    return ( (sURLValidBytes[ch] & IS_PCHAR) != 0 );
}


/*
 CreateStringFromFileSystemRepresentationByAddingPercentEscapes creates a CFString
 for the path-absolute form of a URI path component from the native file system representation.
 Note: this code uses '/' path separators
 
 The rules for path-absolute from rfc3986 are:
 path-absolute = "/" [ segment-nz *( "/" segment ) ]
 segment       = *pchar
 segment-nz    = 1*pchar
 pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
 pct-encoded   = "%" HEXDIG HEXDIG
 unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
 sub-delims    = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
 */
static CFStringRef CreateStringFromFileSystemRepresentationByAddingPercentEscapes(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex numBytes, Boolean isDirectory, Boolean isAbsolute, Boolean windowsPath, Boolean *addedPercentEncoding)
{
    static const UInt8 hexchars[] = "0123456789ABCDEF";
    const UInt8 *fileURLPrefixPtr;
    size_t fileURLPrefixLength;
    if ( AddAuthorityToFileURL() ) {
        fileURLPrefixPtr = fileURLPrefixWithAuthority;
        fileURLPrefixLength = sizeof(fileURLPrefixWithAuthority);
    }
    else {
        fileURLPrefixPtr = fileURLPrefix;
        fileURLPrefixLength = sizeof(fileURLPrefix);
    }
    STACK_BUFFER_DECL(UInt8, stackBuf, (PATH_MAX * 3) + (isAbsolute ? fileURLPrefixLength : 0) + (isDirectory ? 1 : 0)); // worst case is every byte needs to be percent-escaped
    UInt8 *bufStartPtr;
    UInt8 *bufBytePtr;
    const UInt8 *bytePtr = bytes;
    CFIndex idx;
    CFStringRef result;
    Boolean addedPercent = FALSE;
   
    // choose a buffer to percent-escape into.
    if ( numBytes <= PATH_MAX ) {
        bufStartPtr = &stackBuf[0];
    }
    else {
        // worst case is every byte needs to be percent-escaped
        bufStartPtr = (UInt8 *)malloc((numBytes * 3) + (isAbsolute ? fileURLPrefixLength : 0) + (isDirectory ? 1 : 0));
    }
    
    if ( bufStartPtr != NULL ) {
        if ( isAbsolute ) {
            // start with the fileURLPrefix
            strcpy((char *)bufStartPtr, (char *)fileURLPrefixPtr);
            bufBytePtr = bufStartPtr + fileURLPrefixLength - 1;
        }
        else {
            bufBytePtr = bufStartPtr;
        }
        
        if ( !windowsPath ) {
            for ( idx = 0; (idx < numBytes) && (*bytePtr != 0); ++idx ) {
                if ( is_pchar(*bytePtr) ) {
                    *bufBytePtr++ = *bytePtr;
                }
                else {
                    *bufBytePtr++ = '%';
                    *bufBytePtr++ = hexchars[*bytePtr >> 4];
                    *bufBytePtr++ = hexchars[*bytePtr & 0x0f];
                    addedPercent = TRUE;
                }
                ++bytePtr;
            }
        }
        else {
            for ( idx = 0; (idx < numBytes) && (*bytePtr != 0); ++idx ) {
                if ( is_pchar(*bytePtr) && (*bytePtr != '/') ) {    // percent-escape the forward slash if this is a windowsPath
                    *bufBytePtr++ = *bytePtr;
                }
                else {
                    *bufBytePtr++ = '%';
                    *bufBytePtr++ = hexchars[*bytePtr >> 4];
                    *bufBytePtr++ = hexchars[*bytePtr & 0x0f];
                    addedPercent = TRUE;
                }
                ++bytePtr;
            }
        }
        
        // did we convert numBytes?
        if ( idx != numBytes ) {
            // no, but it's OK if the remaining bytes are all nul (embedded nul bytes are not allowed)
            const UInt8 *nullBytePtr = bytePtr;
            for ( /* start where we left off */; (idx < numBytes) && (*nullBytePtr == '\0'); ++idx, ++nullBytePtr ) {
                // do nothing
            }
        }
        
        if ( idx == numBytes ) {
            if ( isDirectory ) {
                // if it is a directory and it doesn't end with PATH_SEP, append a PATH_SEP.
                if ( bytes[numBytes-1] != '/' ) {
                    *bufBytePtr++ = '/';
                }
            }
            else {
                // it is not a directory: remove any pathDelim characters at end (leaving at least one character)
                while ( (numBytes > 1) && (bytes[numBytes-1] == '/') ) {
                    --bufBytePtr;
                    --numBytes;
                }
            }
            
            // create the result
            result = CFStringCreateWithBytes(alloc, bufStartPtr, (CFIndex)(bufBytePtr-bufStartPtr), kCFStringEncodingUTF8, FALSE);
        }
        else {
            // the remaining bytes were not all nul
            result = NULL;
        }
        
        // free the buffer if we malloc'd it
        if ( bufStartPtr != &stackBuf[0] ) {
            free(bufStartPtr);
        }
    }
    else {
        result = NULL;
    }

    if ( addedPercentEncoding ) {
        *addedPercentEncoding = addedPercent;
    }
    
    return ( result );
}

// Returns NULL if str cannot be converted for whatever reason, str if str contains no characters in need of escaping, or a newly-created string with the appropriate % escape codes in place.  Caller must always release the returned string.
CF_INLINE CFStringRef _replacePathIllegalCharacters(CFStringRef str, CFAllocatorRef alloc, Boolean preserveSlashes) {
    CFStringRef result = NULL;
    STACK_BUFFER_DECL(char, buffer, PATH_MAX);
    if ( CFStringGetCString(str, buffer, PATH_MAX, kCFStringEncodingUTF8) ) {
        result = CreateStringFromFileSystemRepresentationByAddingPercentEscapes(kCFAllocatorDefault, (const UInt8 *)buffer, strlen(buffer), FALSE, FALSE, !preserveSlashes, NULL);
    }
    return result;
}

static Boolean _appendPercentEscapesForCharacter(UniChar ch, CFStringEncoding encoding, CFMutableStringRef str) {
    uint8_t bytes[6]; // 6 bytes is the maximum a single character could require in UTF8 (most common case); other encodings could require more
    uint8_t *bytePtr = bytes, *currByte;
    CFIndex byteLength;
    CFAllocatorRef alloc = NULL;
    if (CFStringEncodingUnicodeToBytes(encoding, 0, &ch, 1, NULL, bytePtr, 6, &byteLength) != kCFStringEncodingConversionSuccess) {
        byteLength = CFStringEncodingByteLengthForCharacters(encoding, 0, &ch, 1);
        if (byteLength <= 6) {
            // The encoding cannot accomodate the character
            return false;
        }
        alloc = CFGetAllocator(str);
        bytePtr = (uint8_t *)CFAllocatorAllocate(alloc, byteLength, 0);
        if (!bytePtr || CFStringEncodingUnicodeToBytes(encoding, 0, &ch, 1, NULL, bytePtr, byteLength, &byteLength) != kCFStringEncodingConversionSuccess) {
            if (bytePtr) CFAllocatorDeallocate(alloc, bytePtr);
            return false;
        }
    }
    for (currByte = bytePtr; currByte < bytePtr + byteLength; currByte ++) {
        UniChar escapeSequence[3] = {'%', '\0', '\0'};
        unsigned char high, low;
        high = ((*currByte) & 0xf0) >> 4;
        low = (*currByte) & 0x0f;
        escapeSequence[1] = (high < 10) ? '0' + high : 'A' + high - 10;
        escapeSequence[2] = (low < 10) ? '0' + low : 'A' + low - 10;
        CFStringAppendCharacters(str, escapeSequence, 3);
    }
    if (bytePtr != bytes) {
        CFAllocatorDeallocate(alloc, bytePtr);
    }
    return true;
}

static CFStringRef UnescapeAllWithUTF8(CFAllocatorRef alloc, CFStringRef originalString)
{
    CFStringRef result = NULL;
    CFIndex strLength = CFStringGetLength(originalString);
    CFIndex maxBufferSize = CFStringGetMaximumSizeForEncoding(strLength, kCFStringEncodingUTF8);
    CFIndex stackBufferSize = 2096;
    STACK_BUFFER_DECL(UInt8, escapedStackBuf, stackBufferSize *2);
    UInt8 *escapedBuf;
    UInt8 *unescapedBuf;
    // choose a buffer to percent-escape into.
    if ( maxBufferSize <= stackBufferSize ) {
        escapedBuf = &escapedStackBuf[0];
    }
    else {
        escapedBuf = (UInt8 *)malloc(maxBufferSize * 2);
    }
    if ( escapedBuf ) {
        CFIndex charsConverted;
        CFIndex usedBufLen;
        unescapedBuf = &escapedBuf[maxBufferSize];
        charsConverted = CFStringGetBytes(originalString, CFRangeMake(0, strLength), kCFStringEncodingUTF8, 0, false, escapedBuf, maxBufferSize, &usedBufLen);
        if ( charsConverted ) {
            // 0x80 marks invalid hex digits so this table can validate the digits while getting the values
            static const UInt8 hexvalues[] = {
		/* 00 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 08 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 10 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 18 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 20 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 28 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 30 */  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		/* 38 */  0x08, 0x09, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 40 */  0x80, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x80,
		/* 48 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 50 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 58 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 60 */  0x80, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x80,
		/* 68 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 70 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 78 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		
		/* 80 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 88 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 90 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* 98 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* A0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* A8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* B0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* B8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* C0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* C8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* D0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* D8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* E0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* E8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* F0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		/* F8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
            };
            UInt8 *bufStartPtr;
            UInt8 *bufPtr;
            const UInt8 *bytePtr = escapedBuf;
            CFIndex idx;
            
            bufPtr = bufStartPtr = unescapedBuf;
            Boolean conversionOK = TRUE;
            
            for ( idx = 0; (idx < usedBufLen) && conversionOK; ++idx ) {
                switch ( *bytePtr ) {
                    case '%':
                        idx += 2;
                        if ( idx < usedBufLen ) {
                            UInt8 hex1, hex2;
                            // skip over %
                            bytePtr++;
                            // get the hex digits
                            hex1 = hexvalues[*bytePtr++];
                            hex2 = hexvalues[*bytePtr++];
                            // validate them
                            if ( ((hex1 | hex2) & 0x80) == 0 ) {
                                // convert hex digits
                                *bufPtr = (hex1 << 4) + hex2;
                            }
                            else {
                                conversionOK = FALSE;
                            }
                        }
                        else {
                            conversionOK = FALSE;
                        }
                        break;
                    default:
                        // copy everything else
                        *bufPtr = *bytePtr++;
                        break;
                }
                ++bufPtr;
            }
            if ( conversionOK ) {
                result = CFStringCreateWithBytes(alloc, unescapedBuf, bufPtr - bufStartPtr, kCFStringEncodingUTF8, false);
            }
        }
        
        // free the buffer if we malloc'd it
        if ( escapedBuf != &escapedStackBuf[0] ) {
            free(escapedBuf);
        }
    }
    return ( result );
}

// Uses UTF-8 to translate all percent escape sequences; returns NULL if it encounters a format failure.  May return the original string.
CFStringRef  CFURLCreateStringByReplacingPercentEscapes(CFAllocatorRef alloc, CFStringRef  originalString, CFStringRef  charactersToLeaveEscaped) {
    CFMutableStringRef newStr = NULL;
    CFIndex length;
    CFIndex mark = 0;
    CFRange percentRange, searchRange;
    CFStringRef escapedStr = NULL;
    CFMutableStringRef strForEscapedChar = NULL;
    UniChar escapedChar;
    Boolean escapeAll = (charactersToLeaveEscaped && CFStringGetLength(charactersToLeaveEscaped) == 0);
    Boolean failed = false;
    
    if (!originalString) return NULL;
    
    length = CFStringGetLength(originalString);
    
    if ((length == 0) || (charactersToLeaveEscaped == NULL)) {
        return (CFStringRef)CFStringCreateCopy(alloc, originalString);
    }
    
    if ( escapeAll ) {
        return ( UnescapeAllWithUTF8(alloc, originalString) );
    }
	
    searchRange = CFRangeMake(0, length);

    while (!failed && CFStringFindWithOptions(originalString, CFSTR("%"), searchRange, 0, &percentRange)) {
        uint8_t bytes[4]; // Single UTF-8 character could require up to 4 bytes.
        uint8_t numBytesExpected;
        UniChar ch1, ch2;

        escapedStr = NULL;
        // Make sure we have at least 2 more characters
        if (length - percentRange.location < 3) { failed = true; break; }

        // if we don't have at least 2 more characters, we can't interpret the percent escape code,
        // so we assume the percent character is legit, and let it pass into the string
        ch1 = CFStringGetCharacterAtIndex(originalString, percentRange.location+1);
        ch2 = CFStringGetCharacterAtIndex(originalString, percentRange.location+2);
        if (!_translateBytes(ch1, ch2, bytes)) { failed = true;  break; }
        if (!(bytes[0] & 0x80)) {
            numBytesExpected = 1;
        } else if (!(bytes[0] & 0x20)) {
            numBytesExpected = 2;
        } else if (!(bytes[0] & 0x10)) {
            numBytesExpected = 3;
        } else {
            numBytesExpected = 4;
        }
        if (numBytesExpected == 1) {
            // one byte sequence (most common case); handle this specially
            escapedChar = bytes[0];
            if (!strForEscapedChar) {
                strForEscapedChar = CFStringCreateMutableWithExternalCharactersNoCopy(alloc, &escapedChar, 1, 1, kCFAllocatorNull);
            }
            escapedStr = (CFStringRef)CFRetain(strForEscapedChar);
        } else {
            CFIndex j;
            // Make sure up front that we have enough characters
            if (length < percentRange.location + numBytesExpected * 3) { failed = true; break; }
            for (j = 1; j < numBytesExpected; j ++) {
                if (CFStringGetCharacterAtIndex(originalString, percentRange.location + 3*j) != '%') { failed = true; break; }
                ch1 = CFStringGetCharacterAtIndex(originalString, percentRange.location + 3*j + 1);
                ch2 = CFStringGetCharacterAtIndex(originalString, percentRange.location + 3*j + 2);
                if (!_translateBytes(ch1, ch2, bytes+j)) { failed = true; break; }
            }

            // FIXME: This uses about 1/3 of the time spent in CFURLCreateStringByReplacingPercentEscapes
            // !!! We should do the low-level bit-twiddling ourselves; this is expensive!  REW, 6/10/99
            escapedStr = CFStringCreateWithBytes(alloc, bytes, numBytesExpected, kCFStringEncodingUTF8, false);
            if (!escapedStr) {
                failed = true;
            } else if (CFStringGetLength(escapedStr) == 0 && numBytesExpected == 3 && bytes[0] == 0xef && bytes[1] == 0xbb && bytes[2] == 0xbf) {
                // Somehow, the UCS-2 BOM got translated in to a UTF8 string
                escapedChar = 0xfeff;
                if (!strForEscapedChar) {
                    strForEscapedChar = CFStringCreateMutableWithExternalCharactersNoCopy(alloc, &escapedChar, 1, 1, kCFAllocatorNull);
                }
                CFRelease(escapedStr);
                escapedStr = (CFStringRef)CFRetain(strForEscapedChar);
            }
            if (failed) break;
        }

        // The new character is in escapedChar; the number of percent escapes it took is in numBytesExpected.
        searchRange.location = percentRange.location + 3 * numBytesExpected;
        searchRange.length = length - searchRange.location;
        
        if (!escapeAll) {
            if (CFStringFind(charactersToLeaveEscaped, escapedStr, 0).location != kCFNotFound) {
                if (escapedStr) {
                    CFRelease(escapedStr);
                    escapedStr = NULL;
                }
                continue;
            } 
        }
        
        if (!newStr) {
            newStr = CFStringCreateMutable(alloc, length);
        }
        if (percentRange.location - mark > 0) {
            // FIXME: The creation of this temporary string is unfortunate.
            CFStringRef substring = CFStringCreateWithSubstring(alloc, originalString, CFRangeMake(mark, percentRange.location - mark));
            CFStringAppend(newStr, substring);
            CFRelease(substring);
        }
        CFStringAppend(newStr, escapedStr);
        if (escapedStr) {
            CFRelease(escapedStr);
            escapedStr = NULL;
        }
        mark = searchRange.location;// We need mark to be the index of the first character beyond the escape sequence
    }

    if (escapedStr) CFRelease(escapedStr);
    if (strForEscapedChar) CFRelease(strForEscapedChar);
    if (failed) {
        if (newStr) CFRelease(newStr);
        return NULL;
    } else if (newStr) {
        if (mark < length) {
            // Need to cat on the remainder of the string
            CFStringRef substring = CFStringCreateWithSubstring(alloc, originalString, CFRangeMake(mark, length - mark));
            CFStringAppend(newStr, substring);
            CFRelease(substring);
        }
        return newStr;
    } else {
	return (CFStringRef)CFStringCreateCopy(alloc, originalString);
    }
}

CF_EXPORT
CFStringRef CFURLCreateStringByReplacingPercentEscapesUsingEncoding(CFAllocatorRef alloc, CFStringRef  originalString, CFStringRef  charactersToLeaveEscaped, CFStringEncoding enc) {
    if (enc == kCFStringEncodingUTF8) {
        return CFURLCreateStringByReplacingPercentEscapes(alloc, originalString, charactersToLeaveEscaped);
    } else {
        CFMutableStringRef newStr = NULL;
        CFMutableStringRef escapedStr = NULL;
        CFIndex length;
        CFIndex mark = 0;
        CFRange percentRange, searchRange;
        Boolean escapeAll = (charactersToLeaveEscaped && CFStringGetLength(charactersToLeaveEscaped) == 0);
        Boolean failed = false;
        uint8_t byteBuffer[8];
        uint8_t *bytes = byteBuffer;
        int capacityOfBytes = 8;
        
        if (!originalString) return NULL;
    
        if (charactersToLeaveEscaped == NULL) {
            return (CFStringRef)CFStringCreateCopy(alloc, originalString);
        }
    
        length = CFStringGetLength(originalString);
        searchRange = CFRangeMake(0, length);
    
        while (!failed && CFStringFindWithOptions(originalString, CFSTR("%"), searchRange, 0, &percentRange)) {
            UniChar ch1, ch2;
            CFIndex percentLoc = percentRange.location;
            CFStringRef convertedString;
            int numBytesUsed = 0;
            do {
                // Make sure we have at least 2 more characters
                if (length - percentLoc < 3) { failed = true; break; }
    
                if (numBytesUsed == capacityOfBytes) {
                    if (bytes == byteBuffer) {
                        bytes = (uint8_t *)CFAllocatorAllocate(alloc, 16 * sizeof(uint8_t), 0);
                        memmove(bytes, byteBuffer, capacityOfBytes);
                        capacityOfBytes = 16;
                    } else {
			void *oldbytes = bytes;
			int oldcap = capacityOfBytes;
                        capacityOfBytes = 2*capacityOfBytes;
                        bytes = (uint8_t *)CFAllocatorAllocate(alloc, capacityOfBytes * sizeof(uint8_t), 0);
			memmove(bytes, oldbytes, oldcap);
                        CFAllocatorDeallocate(alloc, oldbytes);
                    }
                }
                percentLoc ++;
                ch1 = CFStringGetCharacterAtIndex(originalString, percentLoc);
                percentLoc ++;
                ch2 = CFStringGetCharacterAtIndex(originalString, percentLoc);
                percentLoc ++;
                if (!_translateBytes(ch1, ch2, bytes + numBytesUsed)) { failed = true;  break; }
                numBytesUsed ++;
            } while (CFStringGetCharacterAtIndex(originalString, percentLoc) == '%');
            searchRange.location = percentLoc;
            searchRange.length = length - searchRange.location;

            if (failed) break;
            convertedString = CFStringCreateWithBytes(alloc, bytes, numBytesUsed, enc, false);
            if (!convertedString) {
                failed = true;
                break;
            }
    
            if (!newStr) {
                newStr = CFStringCreateMutable(alloc, length);
            }
            if (percentRange.location - mark > 0) {
                // FIXME: The creation of this temporary string is unfortunate.
                CFStringRef substring = CFStringCreateWithSubstring(alloc, originalString, CFRangeMake(mark, percentRange.location - mark));
                CFStringAppend(newStr, substring);
                CFRelease(substring);
            }

            if (escapeAll) {
                CFStringAppend(newStr, convertedString);
            } else {
                CFIndex i, c = CFStringGetLength(convertedString);
                if (!escapedStr) {
                    escapedStr = CFStringCreateMutableWithExternalCharactersNoCopy(alloc, &ch1, 1, 1, kCFAllocatorNull);
                }
                for (i = 0; i < c; i ++) {
                    ch1 = CFStringGetCharacterAtIndex(convertedString, i);
                    if (CFStringFind(charactersToLeaveEscaped, escapedStr, 0).location == kCFNotFound) {
                        CFStringAppendCharacters(newStr, &ch1, 1);
                    } else {
                        // Must regenerate the escape sequence for this character; because we started with percent escapes, we know this call cannot fail
                        _appendPercentEscapesForCharacter(ch1, enc, newStr);
                    }
                }
            }
	    CFRelease(convertedString);
            mark = searchRange.location;// We need mark to be the index of the first character beyond the escape sequence
        }
    
        if (escapedStr) CFRelease(escapedStr);
        if (bytes != byteBuffer) CFAllocatorDeallocate(alloc, bytes);
        if (failed) {
            if (newStr) CFRelease(newStr);
            return NULL;
        } else if (newStr) {
            if (mark < length) {
                // Need to cat on the remainder of the string
                CFStringRef substring = CFStringCreateWithSubstring(alloc, originalString, CFRangeMake(mark, length - mark));
                CFStringAppend(newStr, substring);
                CFRelease(substring);
            }
            return newStr;
        } else {
            return (CFStringRef)CFStringCreateCopy(alloc, originalString);
        }
    }
}
    
static Boolean _stringContainsCharacter(CFStringRef string, UniChar ch) {
    CFIndex i, c = CFStringGetLength(string);
    CFStringInlineBuffer buf;
    CFStringInitInlineBuffer(string, &buf, CFRangeMake(0, c));
    for (i = 0; i < c; i ++) if (__CFStringGetCharacterFromInlineBufferQuick(&buf, i) == ch) return true;
    return false;
}

// Note: charactersToLeaveUnescaped and legalURLCharactersToBeEscaped only work for characters which can be represented as a single UTF16 codepoint.
CF_EXPORT CFStringRef CFURLCreateStringByAddingPercentEscapes(CFAllocatorRef allocator, CFStringRef originalString, CFStringRef charactersToLeaveUnescaped, CFStringRef legalURLCharactersToBeEscaped, CFStringEncoding encoding) {
    CFMutableStringRef newString = NULL;
    CFIndex idx, length;
    CFStringInlineBuffer buf;
    enum {
        kCharBufferMax = 4096,
    };
    STACK_BUFFER_DECL(UniChar, charBuffer, kCharBufferMax);
    CFIndex charBufferIndex = 0;

    if (!originalString) return NULL;
    length = CFStringGetLength(originalString);
    if (length == 0) return (CFStringRef)CFStringCreateCopy(allocator, originalString);
    CFStringInitInlineBuffer(originalString, &buf, CFRangeMake(0, length));

    for (idx = 0; idx < length; idx ++) {
        UniChar ch = __CFStringGetCharacterFromInlineBufferQuick(&buf, idx);
        Boolean shouldReplace = (isURLLegalCharacter(ch) == false);
        if (shouldReplace) {
            if (charactersToLeaveUnescaped && _stringContainsCharacter(charactersToLeaveUnescaped, ch)) {
                shouldReplace = false;
            }
        } else if (legalURLCharactersToBeEscaped && _stringContainsCharacter(legalURLCharactersToBeEscaped, ch)) {
            shouldReplace = true;
        }
        
        if (shouldReplace) {
            enum {
                kMaxBytesPerUniChar = 8,    // 8 bytes is the maximum a single UniChar can require in any current encodings; future encodings could require more
                kMaxPercentEncodedUniChars = kMaxBytesPerUniChar * 3
            };
            
            static const UInt8 hexchars[] = "0123456789ABCDEF";
            uint8_t bytes[kMaxBytesPerUniChar];
            uint8_t *bytePtr = bytes;
            uint8_t *currByte;
            uint8_t *endPtr;
            CFIndex byteLength;

            // Perform the replacement
            if ( !newString ) {
                newString = CFStringCreateMutableCopy(CFGetAllocator(originalString), 0, originalString);
                CFStringDelete(newString, CFRangeMake(idx, length-idx));
            }
            // make sure charBuffer has enough room
            if ( charBufferIndex >= (kCharBufferMax - kMaxPercentEncodedUniChars) ) {
                // make room
                CFStringAppendCharacters(newString, charBuffer, charBufferIndex);
                charBufferIndex = 0;
            }
            
            // convert the UniChar to bytes
            if ( CFStringEncodingUnicodeToBytes(encoding, 0, &ch, 1, NULL, bytePtr, 8, &byteLength) == kCFStringEncodingConversionSuccess ) {
                // percent-encode the bytes
                endPtr = bytePtr + byteLength;
                for ( currByte = bytePtr; currByte < endPtr; currByte++ ) {
                    charBuffer[charBufferIndex++] = '%';
                    charBuffer[charBufferIndex++] = hexchars[*currByte >> 4];
                    charBuffer[charBufferIndex++] = hexchars[*currByte & 0x0f];
                }
            }
            else {
                // FIXME: once CFString supports finding glyph boundaries walk by glyph boundaries instead of by unichars
                if ( encoding == kCFStringEncodingUTF8 && CFCharacterSetIsSurrogateHighCharacter(ch) && idx + 1 < length && CFCharacterSetIsSurrogateLowCharacter(__CFStringGetCharacterFromInlineBufferQuick(&buf, idx+1)) ) {
                    UniChar surrogate[2];
                    uint8_t *currByte;
                    
                    surrogate[0] = ch;
                    surrogate[1] = __CFStringGetCharacterFromInlineBufferQuick(&buf, idx+1);
                    // Aki sez it should never take more than 6 bytes
                    if (CFStringEncodingUnicodeToBytes(kCFStringEncodingUTF8, 0, surrogate, 2, NULL, bytes, 6, &byteLength) == kCFStringEncodingConversionSuccess) {
                        endPtr = bytePtr + byteLength;
                        for ( currByte = bytes; currByte < endPtr; currByte++ ) {
                            charBuffer[charBufferIndex++] = '%';
                            charBuffer[charBufferIndex++] = hexchars[*currByte >> 4];
                            charBuffer[charBufferIndex++] = hexchars[*currByte & 0x0f];
                        }
                        idx++; // We consumed 2 characters, not 1
                    }
                    else {
                        // surrogate pair conversion failed
                        break;
                    }
                } else {
                    // not a surrogate pair
                    break;
                }
            }
        } else if (newString) {
            charBuffer[charBufferIndex++] = ch;
            if ( charBufferIndex == kCharBufferMax ) {
                CFStringAppendCharacters(newString, charBuffer, charBufferIndex);
                charBufferIndex = 0;
            }
        }
    }
    if (idx < length) {
        // Ran into an encoding failure
        if (newString) CFRelease(newString);
        return NULL;
    } else if (newString) {
        if ( charBufferIndex != 0 ) {
            CFStringAppendCharacters(newString, charBuffer, charBufferIndex);
        }
        return newString;
    } else {
        return (CFStringRef)CFStringCreateCopy(CFGetAllocator(originalString), originalString);
    }
}

static Boolean __CFURLEqual(CFTypeRef  cf1, CFTypeRef  cf2) {
    Boolean result;
    CFURLRef  url1 = (CFURLRef)cf1;
    CFURLRef  url2 = (CFURLRef)cf2;
    
    __CFGenericValidateType(cf1, CFURLGetTypeID());
    __CFGenericValidateType(cf2, CFURLGetTypeID());

    if ( url1 == url2 ) {
        result = true;
    }
    else {
        if ( (url1->_flags & EQUAL_FLAGS_MASK) != (url2->_flags & EQUAL_FLAGS_MASK) ) {
            result = false;
        }
        else {
            if ( (url1->_base && !url2->_base) ||
                (!url1->_base && url2->_base) ||
                (url1->_base && url2->_base && !CFEqual(url1->_base, url2->_base)) ) {
                result = false;
            }
            else {
                // no base urls, so compare the URL strings
                // Do not compare the original strings; compare the sanatized strings.
                result = CFEqual(CFURLGetString(url1), CFURLGetString(url2));
            }
        }
    }
    return ( result ) ;
}

static CFHashCode __CFURLHash(CFTypeRef cf)
{
    CFHashCode result;
    
    if ( cf ) {
        // use the CFHashCode of the URL
        result = CFHash(CFURLGetString((CFURLRef)cf));
    }
    else {
        // no object, no hashcode
        result = 0;
    }
    
    return ( result );
}

static CFStringRef CreateTruncatedURLString(CFAllocatorRef alloc, CFStringRef urlString, CFIndex maxLength, CFIndex suffixLength)
{
    CFStringRef result;
    
    CFIndex len = CFStringGetLength(urlString);
    if ( len <= maxLength ) {
        // return the retained urlString
        result = CFStringCreateCopy(alloc, urlString);
    }
    else {
        CFStringRef start = CFStringCreateWithSubstring(alloc, urlString, CFRangeMake(0, maxLength - suffixLength));
        CFStringRef end = CFStringCreateWithSubstring(alloc, urlString, CFRangeMake(len - suffixLength, suffixLength));
        result = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@ ... %@"), start, end);
        if ( start ) {
            CFRelease(start);
        }
        if ( (end) ) {
            CFRelease(end);
        }
    }
    return ( result );
}

static CFStringRef  __CFURLCopyFormattingDescription(CFTypeRef  cf, CFDictionaryRef formatOptions) {
    CFStringRef result;
    CFURLRef  url = (CFURLRef)cf;
    __CFGenericValidateType(cf, CFURLGetTypeID());
    CFAllocatorRef alloc = CFGetAllocator(url);

    Boolean isDataURL = false;
    CFStringRef scheme = CFURLCopyScheme(url);
    if ( scheme ) {
        isDataURL = CFStringCompare(scheme, kCFURLDataScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo;
        CFRelease(scheme);
    }
    
    if ( !isDataURL ) {
        if (!url->_base) {
            {
                result = CFStringCreateCopy(alloc, url->_string);
            }
        } else {
            // Do not dereference url->_base; it may be an ObjC object
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@ -- %@"), url->_string, url->_base);
        }
    }
    else {
        if ( !url->_base ) {
            result = CreateTruncatedURLString(alloc, url->_string, 128, 8);
        }
        else {
            CFStringRef urlString = CreateTruncatedURLString(alloc, url->_string, 128, 8);
            CFStringRef baseString = CreateTruncatedURLString(alloc, CFURLGetString(url->_base), 128, 8);
            // Do not dereference url->_base; it may be an ObjC object
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@ -- %@"), urlString, baseString);
            if ( urlString ) {
                CFRelease(urlString);
            }
            if ( baseString ) {
                CFRelease(baseString);
            }
        }
    }
    return ( result );
}


static CFStringRef __CFURLCopyDescription(CFTypeRef cf) {
    CFURLRef url = (CFURLRef)cf;
    CFStringRef result;
    CFAllocatorRef alloc = CFGetAllocator(url);
    Boolean isDataURL = false;
    CFStringRef scheme = CFURLCopyScheme(url);
    
    if ( scheme ) {
        isDataURL = CFStringCompare(scheme, kCFURLDataScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo;
        CFRelease(scheme);
    }
    
    if ( !isDataURL ) {
        if ( url->_base) {
            CFStringRef baseString = CFCopyDescription(url->_base);
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("<CFURL %p [%p]>{string = %@, encoding = %u\n\tbase = %@}"), cf, alloc, url->_string, (unsigned int)(url->_encoding), baseString);
            CFRelease(baseString);
        } else {
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("<CFURL %p [%p]>{string = %@, encoding = %u, base = (null)}"), cf, alloc, url->_string, (unsigned int)(url->_encoding));
        }
    }
    else {
        CFStringRef urlString = CreateTruncatedURLString(alloc, url->_string, 128, 8);
        if ( url->_base) {
            CFStringRef baseString = CFCopyDescription(url->_base);
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("<CFURL %p [%p]>{string = %@, encoding = %u\n\tbase = %@}"), cf, alloc, urlString, (unsigned int)(url->_encoding), baseString);
            CFRelease(baseString);
        } else {
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("<CFURL %p [%p]>{string = %@, encoding = %u, base = (null)}"), cf, alloc, urlString, (unsigned int)(url->_encoding));
        }
        CFRelease(urlString);
    }
    return result;
}

#if DEBUG_URL_MEMORY_USAGE
extern __attribute((used)) void __CFURLDumpMemRecord(void) {
    syslog(LOG_ERR, "%d URLs created; %d destroyed; %d URLs parsed; %d urls had 'extra' data allocated; %d had base urls; %d were not UTF8 encoded", numURLs, numDealloced, numURLsParsed, numExtraDataAllocated, numURLsWithBaseURL, numNonUTF8EncodedURLs);
}
#endif

static void __CFURLDeallocate(CFTypeRef  cf) {
    CFURLRef  url = (CFURLRef)cf;
    CFAllocatorRef alloc;
    __CFGenericValidateType(cf, CFURLGetTypeID());
    alloc = CFGetAllocator(url);
#if DEBUG_URL_MEMORY_USAGE
    numDealloced ++;
#endif
    if (url->_string) CFRelease(url->_string); // GC: 3879914
    if (url->_base) CFRelease(url->_base);
    CFStringRef sanitizedString = _getSanitizedString(url);
    if (sanitizedString) CFRelease(sanitizedString);
    if ( url->_extra != NULL ) CFAllocatorDeallocate( alloc, url->_extra );
    if (_getResourceInfo(url)) CFRelease(_getResourceInfo(url));
}

static CFTypeID __kCFURLTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFURLClass = {
    0,                                  // version
    "CFURL",                            // className
    NULL,                               // init
    NULL,                               // copy
    __CFURLDeallocate,                  // finalize
    __CFURLEqual,                       // equal
    __CFURLHash,                        // hash
    __CFURLCopyFormattingDescription,   // copyFormattingDesc
    __CFURLCopyDescription,             // copyDebugDesc
    NULL,                               // reclaim
    NULL,                               // refcount
};

/* Toll-free bridging support; get the true CFURL from an NSURL */
CF_INLINE CFURLRef _CFURLFromNSURL(CFURLRef url) {
    CF_OBJC_FUNCDISPATCHV(CFURLGetTypeID(), CFURLRef, (NSURL *)url, _cfurl);
    return url;
}

CFTypeID CFURLGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFURLTypeID = _CFRuntimeRegisterClass(&__CFURLClass); });
    return __kCFURLTypeID;
}

CF_PRIVATE void CFShowURL(CFURLRef url) {
    if (!url) {
        fprintf(stdout, "(null)\n");
        return;
    }
    fprintf(stdout, "<CFURL %p>{", (const void*)url);
    if (CF_IS_OBJC(CFURLGetTypeID(), url)) {
        fprintf(stdout, "ObjC bridged object}\n");
        return;
    }
    fprintf(stdout, "\n\tRelative string: ");
    CFShow(url->_string);
    fprintf(stdout, "\tBase URL: ");
    if (url->_base) {
        fprintf(stdout, "<%p> ", (const void*)url->_base);
        CFShow(url->_base);
    } else {
        fprintf(stdout, "(null)\n");
    }
    fprintf(stdout, "\tFlags: 0x%x\n}\n", (unsigned int)url->_flags);
}


/***************************************************/
/* URL creation and String/Data creation from URLS */
/***************************************************/
static void constructBuffers(CFAllocatorRef alloc, CFStringRef string, UInt8 *inBuffer, CFIndex inBufferSize, const char **cstring, const UniChar **ustring, Boolean *useCString, Boolean *freeCharacters) {
    CFIndex neededLength;
    CFIndex length;
    CFRange rg;

    *cstring = CFStringGetCStringPtr(string, kCFStringEncodingISOLatin1);
    if (*cstring) {
        *ustring = NULL;
        *useCString = true;
        *freeCharacters = false;
        return;
    } 

    *ustring = CFStringGetCharactersPtr(string);
    if (*ustring) {
        *useCString = false;
        *freeCharacters = false;
        return;
    } 

    length = CFStringGetLength(string);
    rg = CFRangeMake(0, length);
    CFStringGetBytes(string, rg, kCFStringEncodingISOLatin1, 0, false, NULL, INT_MAX, &neededLength);
    if (neededLength == length) {
        char *buf;
        if ( (inBuffer != NULL) && (length <= inBufferSize) ) {
            buf = (char *)inBuffer;
            *freeCharacters = false;
        }
        else {
            buf = (char *)CFAllocatorAllocate(alloc, length, 0);
            *freeCharacters = true;
        }
        CFStringGetBytes(string, rg, kCFStringEncodingISOLatin1, 0, false, (uint8_t *)buf, length, NULL);
        *cstring = buf;
        *useCString = true;
    } else {
        UniChar *buf;
        if ( (inBuffer != NULL) && ((length * sizeof(UniChar)) <= inBufferSize) ) {
            buf = (UniChar *)inBuffer;
            *freeCharacters = false;
        }
        else {
            buf = (UniChar *)CFAllocatorAllocate(alloc, length * sizeof(UniChar), 0);
            *freeCharacters = true;
        }
        CFStringGetCharacters(string, rg, buf);
        *ustring = buf;
        *useCString = false;
    }
}

static void _parseComponentsCString(CFAllocatorRef alloc, CFURLRef baseURL, CFIndex cfStringLength, const char *characterArray, UInt32 *theFlags, CFRange *packedRanges, uint8_t *numberOfRanges)
#define CFURL_INCLUDE_PARSE_COMPONENTS
#include "CFURL.inc.h"
#undef CFURL_INCLUDE_PARSE_COMPONENTS

static void _parseComponentsUString(CFAllocatorRef alloc, CFURLRef baseURL, CFIndex cfStringLength, const UniChar *characterArray, UInt32 *theFlags, CFRange *packedRanges, uint8_t *numberOfRanges)
#define CFURL_INCLUDE_PARSE_COMPONENTS
#include "CFURL.inc.h"
#undef CFURL_INCLUDE_PARSE_COMPONENTS

static void _parseComponents(CFAllocatorRef alloc, CFStringRef string, CFURLRef baseURL, UInt32 *theFlags, CFRange *packedRanges, uint8_t *numberOfRanges)
{
    CFIndex cfStringLength = CFStringGetLength(string);
    Boolean useCString, freeCharacters;
    const char *cstring = NULL;
    const UniChar *ustring = NULL;
    CFIndex stackBufferSize = 4096;
    STACK_BUFFER_DECL(UInt8, stackBuffer, stackBufferSize);
    
    constructBuffers(alloc, string, stackBuffer, stackBufferSize, &cstring, &ustring, &useCString, &freeCharacters);
    
    if (useCString) {
        _parseComponentsCString(alloc, baseURL, cfStringLength, cstring, theFlags, packedRanges, numberOfRanges);
    }
    else {
        _parseComponentsUString(alloc, baseURL, cfStringLength, ustring, theFlags, packedRanges, numberOfRanges);
    }
    
    if (freeCharacters) {
        CFAllocatorDeallocate(alloc, useCString ? (void *)cstring : (void *)ustring);
    }
}

static Boolean scanCharactersCString(CFAllocatorRef alloc, CFMutableStringRef *escapedString, UInt32 *flags, const char *characterArray, Boolean useCString, CFIndex base, CFIndex end, CFIndex *mark, UInt32 componentFlag, CFStringEncoding encoding)
#define CFURL_INCLUDE_SCAN_CHARACTERS
#include "CFURL.inc.h"
#undef CFURL_INCLUDE_SCAN_CHARACTERS

static Boolean scanCharactersUString(CFAllocatorRef alloc, CFMutableStringRef *escapedString, UInt32 *flags, const UniChar *characterArray, Boolean useCString, CFIndex base, CFIndex end, CFIndex *mark, UInt32 componentFlag, CFStringEncoding encoding)
#define CFURL_INCLUDE_SCAN_CHARACTERS
#include "CFURL.inc.h"
#undef CFURL_INCLUDE_SCAN_CHARACTERS

static Boolean scanCharacters(CFAllocatorRef alloc, CFMutableStringRef *escapedString, UInt32 *flags, const char *cstring, const UniChar *ustring, Boolean useCString, CFIndex base, CFIndex end, CFIndex *mark, UInt32 componentFlag, CFStringEncoding encoding) {
    if ( useCString ) {
        return ( scanCharactersCString(alloc, escapedString, flags, cstring, useCString, base, end, mark, componentFlag, encoding));
    }
    else {
        return ( scanCharactersUString(alloc, escapedString, flags, ustring, useCString, base, end, mark, componentFlag, encoding));
    }
}

static void computeSanitizedString(CFURLRef url) {
    CFAllocatorRef alloc = CFGetAllocator(url);
    CFIndex string_length = CFStringGetLength(url->_string);
    Boolean useCString, freeCharacters;
    const char *cstring = NULL;
    const UniChar *ustring = NULL;
    CFIndex base; // where to scan from
    CFIndex mark; // first character not-yet copied to sanitized string
    CFIndex stackBufferSize = 4096;
    STACK_BUFFER_DECL(UInt8, stackBuffer, stackBufferSize);
    CFMutableStringRef sanitizedString = NULL;
    UInt32 additionalDataFlags = 0;

    constructBuffers(alloc, url->_string, stackBuffer, stackBufferSize, &cstring, &ustring, &useCString, &freeCharacters);
    if (!(url->_flags & IS_DECOMPOSABLE)) {
        // Impossible to have a problem character in the scheme
        base = _rangeForComponent(url->_flags, url->_ranges, HAS_SCHEME).length + 1;
        mark = 0;
        if (!scanCharacters(alloc, &sanitizedString, &additionalDataFlags, cstring, ustring, useCString, base, string_length, &mark, 0, url->_encoding)) {
            ((struct __CFURL *)url)->_flags |= ORIGINAL_AND_URL_STRINGS_MATCH;
        }
        if ( sanitizedString ) {
            _setAdditionalDataFlags((struct __CFURL*)url, additionalDataFlags);
        }
    } else {
        // Go component by component
        CFIndex currentComponent = HAS_USER;
        mark = 0;
        while (currentComponent < (HAS_FRAGMENT << 1)) {
            CFRange componentRange = _rangeForComponent(url->_flags, url->_ranges, currentComponent);
            if (componentRange.location != kCFNotFound) {
                scanCharacters(alloc, &sanitizedString, &additionalDataFlags, cstring, ustring, useCString, componentRange.location, componentRange.location + componentRange.length, &mark, currentComponent, url->_encoding);
            }
            currentComponent = currentComponent << 1;
        }
        if (sanitizedString) {
            _setAdditionalDataFlags((struct __CFURL*)url, additionalDataFlags);
        } else {
            ((struct __CFURL *)url)->_flags |= ORIGINAL_AND_URL_STRINGS_MATCH;
        }
    }
    if (sanitizedString && mark != string_length) {
        if (useCString) {
            __CFStringAppendBytes(sanitizedString, (char *)&(cstring[mark]), string_length - mark, kCFStringEncodingISOLatin1);
        } else {
            CFStringAppendCharacters(sanitizedString, &(ustring[mark]), string_length - mark);
        }
    }
    if ( sanitizedString ) {
        _setSanitizedString((struct __CFURL*) url, sanitizedString);
        CFRelease(sanitizedString);
    }
    if (freeCharacters) {
        CFAllocatorDeallocate(alloc, useCString ? (void *)cstring : (void *)ustring);
    }
}


static CFStringRef correctedComponent(CFStringRef comp, UInt32 compFlag, CFStringEncoding enc) {
    CFAllocatorRef alloc = CFGetAllocator(comp);
    CFIndex string_length = CFStringGetLength(comp);
    Boolean useCString, freeCharacters;
    const char *cstring = NULL;
    const UniChar *ustring = NULL;
    CFIndex mark = 0; // first character not-yet copied to sanitized string
    CFMutableStringRef result = NULL;
    CFIndex stackBufferSize = 1024;
    STACK_BUFFER_DECL(UInt8, stackBuffer, stackBufferSize);

    constructBuffers(alloc, comp, stackBuffer, stackBufferSize, &cstring, &ustring, &useCString, &freeCharacters);
    scanCharacters(alloc, &result, NULL, cstring, ustring, useCString, 0, string_length, &mark, compFlag, enc);
    if (result) {
        if (mark < string_length) {
            if (useCString) {
                __CFStringAppendBytes(result, (char *)&(cstring[mark]), string_length - mark, kCFStringEncodingISOLatin1);
            } else {
                CFStringAppendCharacters(result, &(ustring[mark]), string_length - mark);
            }
        }
    } else {
        // This should nevr happen
        CFRetain(comp);
        result = (CFMutableStringRef)comp;
    }
    if (freeCharacters) {
        CFAllocatorDeallocate(alloc, useCString ? (void *)cstring : (void *)ustring);
    }
    return result;
}


static struct __CFURL * _CFURLAlloc(CFAllocatorRef allocator, uint8_t numberOfRanges);
static CFURLRef _CFURLCreateWithURLString(CFAllocatorRef allocator, CFStringRef string, Boolean checkForLegalCharacters, CFURLRef baseURL);
static CFURLRef _CFURLCreateWithFileSystemPath(CFAllocatorRef allocator, CFStringRef fileSystemPath, CFURLPathStyle pathStyle, Boolean isDirectory, CFURLRef baseURL);
static CFURLRef _CFURLCreateWithFileSystemRepresentation(CFAllocatorRef allocator, const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory, CFURLRef baseURL);

static struct __CFURL * _CFURLAlloc(CFAllocatorRef allocator, uint8_t numberOfRanges) {
    struct __CFURL *url;
#if DEBUG_URL_MEMORY_USAGE
    numURLs ++;
//    if ( numURLs % 1000 == 0 ) {
//        __CFURLDumpMemRecord();
//    }
#endif
    CFIndex extraBytes = offsetof(struct __CFURL, _ranges[numberOfRanges]) - sizeof(CFRuntimeBase);
    url = (struct __CFURL *)_CFRuntimeCreateInstance(allocator, CFURLGetTypeID(), extraBytes, NULL);
    if (url) {
	url->_flags = 0;
	url->_encoding = kCFStringEncodingUTF8;
	url->_string = NULL;
	url->_base = NULL;
	url->_extra = NULL;
	url->_resourceInfo = NULL;
    }
    return url;
}

static Boolean _CFStringIsLegalURLString(CFStringRef string) {
    Boolean result = true;
    if ( string ) {
        CFStringInlineBuffer stringBuffer;
        Boolean sawHash = false;
        CFIndex idx = 0;
        CFIndex checkHexDigit = 0;
        CFIndex length;
        
        length = CFStringGetLength(string);
        {
            CFStringInitInlineBuffer(string, &stringBuffer, CFRangeMake(0, length));
            
            while ( idx < length ) {
                CFIndex rangeLength;
                const UniChar *chPtr;
                
                rangeLength = (idx + __kCFStringInlineBufferLength <= length) ? __kCFStringInlineBufferLength : length - idx;
                chPtr = CFStringGetCharactersPtrFromInlineBuffer(&stringBuffer, CFRangeMake(idx, rangeLength));
                for ( CFIndex rangeIdx = 0; rangeIdx < rangeLength; ++rangeIdx, ++chPtr ) {
                    if ( !checkHexDigit ) {
                        if ( *chPtr == '%' ) {
                            // percent encoded? make sure there at least 2 characters left to check
                            if ( (idx + rangeIdx + 2) < length ) {
                                // the next 2 characters must be HEXDIG
                                checkHexDigit = 2;
                                continue;
                            }
                            else {
                                result = false;
                                break;
                            }
                        }
                        if ( *chPtr == '[' || *chPtr == ']' ) {
                            continue; // IPV6 support (RFC 2732) DCJ June/10/2002
                        }
                        if ( *chPtr == '#' ) {
                            // only allow one # character
                            if ( !sawHash ) {
                                sawHash = true;
                                continue;
                            }
                            else {
                                result = false;
                                break;
                            }
                        }
#if DEPLOYMENT_TARGET_WINDOWS
                        // <rdar://problem/7134119> CF on Windows: CFURLCreateWithString should work with | in path on Windows
                        if ( isURLLegalCharacter(*chPtr) || *chPtr == '|' ) {
                            continue;
                        }
#else
                        if ( isURLLegalCharacter(*chPtr) ) {
                            continue;
                        }
#endif
                        else {
                            result = false;
                            break;
                        }
                    }
                    else {
                        if ( isHexDigit(*chPtr) ) {
                            --checkHexDigit;
                            continue;
                        }
                        else {
                            result = false;
                            break;
                        }
                    }
                }
                
                if ( !result ) {
                    break; // out of main while loop
                }
                
                idx += rangeLength;
            }
        }
    }
    else {
        CFAssert(false, __kCFLogAssertion, "Cannot create an CFURL from a NULL string");
        result = false;
    }
    
    return ( result );
}

/* initializes a URL object with a URL string */
static CFURLRef _CFURLCreateWithURLString(CFAllocatorRef allocator, CFStringRef string, Boolean checkForLegalCharacters, CFURLRef baseURL)
{
    struct __CFURL *result = NULL;
#if DEBUG_URL_INITIALIZER_LOGGING
    CFStringRef input_string = string ? CFRetain(string) : NULL;
    Boolean input_checkForLegalCharacters = checkForLegalCharacters;
    CFURLRef input_baseURL = baseURL ? CFRetain(baseURL) : NULL;
#endif
    if ( checkForLegalCharacters ? _CFStringIsLegalURLString(string) : true ) {
        // determine if URL is absolute (i.e. it has a scheme and the scheme characters are valid
        CFStringInlineBuffer stringBuffer;
        CFIndex length = CFStringGetLength(string);
        CFIndex idx = 0;
        Boolean isAbsolute = false;
        Boolean schemeCharsValid = true;

        CFStringInitInlineBuffer(string, &stringBuffer, CFRangeMake(0, length));
        // the first character of the scheme must be ALPHA
        if ( (length > 0) && isALPHA(__CFStringGetCharacterFromInlineBufferQuick(&stringBuffer, 0)) ) {
            // stop looking if we hit the end of the string, find a colon (isAbsolute), or find a non-scheme character (schemeCharsValid)
            while (idx < length && schemeCharsValid && !isAbsolute) {
                CFIndex rangeLength = (idx + __kCFStringInlineBufferLength <= length) ? __kCFStringInlineBufferLength : length - idx;
                const UniChar *chPtr = CFStringGetCharactersPtrFromInlineBuffer(&stringBuffer, CFRangeMake(idx, rangeLength));
                for (CFIndex i = 0; i < rangeLength; ++i, ++chPtr) {
                    if ( *chPtr == ':' ) {
                        isAbsolute = true;
                        break;
                    }
                    if ( !scheme_valid(*chPtr) ) {
                        schemeCharsValid = false;
                        break;
                    }
                }
                if ( isAbsolute ) {
                    baseURL = NULL;
                    break;
                }
                idx += rangeLength;
            }
        }
        
#if DEBUG_URL_MEMORY_USAGE
        numURLsParsed++;
#endif
        // get the range flags, ranges and range count from the parser
        uint8_t numberOfRanges;
        UInt32 flags = 0;
        CFRange packedRanges[MAX_COMPONENTS];
        
        _parseComponents(allocator, string, baseURL, &flags, packedRanges, &numberOfRanges);
        // allocate the URL object with the appropriate number of ranges
        result = _CFURLAlloc(allocator, numberOfRanges);
        if ( result ) {
            result->_flags = flags;
            memcpy(result->_ranges, packedRanges, sizeof(CFRange) * numberOfRanges);
            result->_string = CFStringCreateCopy(allocator, string);
            result->_base = baseURL ? CFURLCopyAbsoluteURL(baseURL) : NULL;
#if DEBUG_URL_MEMORY_USAGE
            if ( url->_base ) {
                numURLsWithBaseURL ++;
            }
#endif
        }
    }
#if DEBUG_URL_INITIALIZER_LOGGING
    CFLog(kCFLogLevelError, CFSTR("_CFURLCreateWithURLString (in) string '%@', checkForLegalCharacters %@, baseURL %@\n\t_CFURLCreateWithURLString (out) result %@"), input_string, input_checkForLegalCharacters ? CFSTR("YES") : CFSTR("NO"), input_baseURL, result);
    if ( input_string ) {
        CFRelease(input_string);
    }
    if ( input_baseURL ) {
        CFRelease(input_baseURL);
    }
#endif   
    return ( result );
}

/* initializes a URL object with a file system path */
static CFURLRef _CFURLCreateWithFileSystemPath(CFAllocatorRef allocator, CFStringRef fileSystemPath, CFURLPathStyle pathStyle, Boolean isDirectory, CFURLRef baseURL)
{
#if DEBUG_URL_INITIALIZER_LOGGING
    CFStringRef input_fileSystemPath = fileSystemPath ? CFRetain(fileSystemPath) : NULL;
    CFURLPathStyle input_pathStyle = pathStyle;
    Boolean input_isDirectory = isDirectory;
    CFURLRef input_baseURL = baseURL ? CFRetain(baseURL) : NULL;
#endif
    CFAssert1(fileSystemPath != NULL, __kCFLogAssertion, "%s(): NULL path string not permitted", __PRETTY_FUNCTION__);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    CFAssert2(pathStyle == kCFURLPOSIXPathStyle || pathStyle == kCFURLHFSPathStyle || pathStyle == kCFURLWindowsPathStyle, __kCFLogAssertion, "%s(): encountered unknown path style %d", __PRETTY_FUNCTION__, pathStyle);
#pragma GCC diagnostic pop
    
    struct __CFURL *result = NULL;
    CFStringRef urlString = NULL;
    Boolean isAbsolute;
    Boolean isFileReferencePath = false;
    Boolean posixAndUrlPathsMatch = false;
    Boolean releaseBaseURL = false;
    CFIndex len = CFStringGetLength(fileSystemPath);
    
    if (len > 0) {
        // Determine if fileSystemPath is an absolute path. If kCFURLPOSIXPathStyle, determine if it is a file reference path.
        // Then, convert the fileSystemPath to a urlString. The urlString returned will have a pathDelim at the end if isDirectory
        // was true and no pathDelim if isDirectory was false (unless the urlPath is "/").
        // If isAbsolute, "file://" will be prepended to the urlString.
        switch (pathStyle) {
            case kCFURLPOSIXPathStyle:
                isAbsolute = (len > 0 && CFStringGetCharacterAtIndex(fileSystemPath, 0) == '/');
                isFileReferencePath = _pathHasFileIDPrefix(fileSystemPath);
                urlString = POSIXPathToURLPath(fileSystemPath, allocator, isDirectory, isAbsolute, &posixAndUrlPathsMatch);
                break;
                
                
            case kCFURLWindowsPathStyle:
            {
                // this is what _CFURLInitFSPath() did (with a small change to handle absolute paths that begin with a single backslash)
                UniChar firstChar = 0 < len ? CFStringGetCharacterAtIndex(fileSystemPath, 0) : 0;
                UniChar secondChar = 1 < len ? CFStringGetCharacterAtIndex(fileSystemPath, 1) : 0;
                Boolean isDrive = isALPHA(firstChar) && (secondChar == ':' || secondChar == '|');
                
                if ( isDrive) {
                    // A disk designator
                    isAbsolute = true;
                    urlString = WindowsPathToURLPath(fileSystemPath, allocator, isDirectory, isAbsolute);
                }
                else if ( firstChar == '\\' ) {
                    // Either a UNC name of any format (which always start with two backslash characters), or an absolute path which starts with a single backslash.
                    isAbsolute = true;
                    urlString = WindowsPathToURLPath(fileSystemPath, allocator, isDirectory, isAbsolute);
                }
                else if (firstChar == '/') {
                    // there's probably code that passes POSIX paths in but wrongly specifies kCFURLWindowsPathStyle.
                    pathStyle = kCFURLPOSIXPathStyle;
                    isAbsolute = true;
                    urlString = POSIXPathToURLPath(fileSystemPath, allocator, isDirectory, isAbsolute, &posixAndUrlPathsMatch);
                }
                else {
                    isAbsolute = false;
                    urlString = WindowsPathToURLPath(fileSystemPath, allocator, isDirectory, isAbsolute);
                }
            }
                break;
        }
        
        CFAssert2(urlString != NULL, __kCFLogAssertion, "%s(): Encountered malformed file system URL %@", __PRETTY_FUNCTION__, urlString);
        
        if ( urlString ) {
            if ( isAbsolute ) {
                // if fileSystemPath is an absolute path, ignore baseURL (if provided)
                baseURL = NULL;
            }
            else if ( baseURL == NULL ) {
                // if fileSystemPath is a relative path and no baseURL is provided, use the current working directory
                baseURL = _CFURLCreateCurrentDirectoryURL(allocator);
                releaseBaseURL = true;
            }
            
            // override isDirectory if the path is to root
            if ( !isDirectory && (len == 1) && (CFStringGetCharacterAtIndex(urlString, 0) == '/') ) {
                // Override isDirectory
                isDirectory = true;
            }
            
            // allocate the URL object with the appropriate number of ranges
            result = _CFURLAlloc(allocator, isAbsolute ? (AddAuthorityToFileURL() ? 3 : 2) : 1);
            if ( result ) {
                result->_string = CFStringCreateCopy(allocator, urlString);
                result->_base = baseURL ? CFURLCopyAbsoluteURL(baseURL) : NULL;
#if DEBUG_URL_MEMORY_USAGE
                if ( url->_base ) {
                    numURLsWithBaseURL ++;
                }
#endif
                // hard coded parse
                if ( isAbsolute ) {
                    UInt32 flags = IS_DECOMPOSABLE | HAS_SCHEME | HAS_PATH | ORIGINAL_AND_URL_STRINGS_MATCH;
                    flags |= (isDirectory ? IS_DIRECTORY : 0);
                    if ( isFileReferencePath ) {
                        // if the URL is a file reference URL, don't set IS_CANONICAL_FILE_URL or POSIX_AND_URL_PATHS_MATCH
                        flags |= PATH_HAS_FILE_ID;
                    }
                    else {
                        // only posix style paths can be canonical because POSIXPathToURLPath() converts the string to file system representation
                        flags |= ((pathStyle == kCFURLPOSIXPathStyle) ? IS_CANONICAL_FILE_URL : 0);
                        flags |= (posixAndUrlPathsMatch ? POSIX_AND_URL_PATHS_MATCH : 0);
                    }
                    _setSchemeTypeInFlags(&flags, kHasFileScheme);
                    
                    if ( AddAuthorityToFileURL() ) {
                        result->_flags = flags | HAS_HOST;
                        result->_ranges[0] = CFRangeMake(0, 4); // scheme "file"
                        result->_ranges[1] = CFRangeMake(7, 9); // host "localhost"
                        result->_ranges[2] = CFRangeMake(16, CFStringGetLength(urlString)- 16);
                    }
                    else {
                        result->_flags = flags;
                        result->_ranges[0] = CFRangeMake(0, 4); // scheme "file"
                        result->_ranges[1] = CFRangeMake(7, CFStringGetLength(urlString)- 7);
                    }
                } else {
                    result->_flags = (isDirectory ? IS_DIRECTORY : 0) | IS_DECOMPOSABLE | HAS_PATH | ORIGINAL_AND_URL_STRINGS_MATCH;
                    result->_ranges[0] = CFRangeMake(0, CFStringGetLength(result->_string));
                }
            }
            
            if ( releaseBaseURL && baseURL ) {
                CFRelease(baseURL);
            }
            
            CFRelease(urlString);
        }
    }
    else if ( baseURL ) {
        result = (struct __CFURL *)CFRetain(baseURL);
    }
#if DEBUG_URL_INITIALIZER_LOGGING
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    CFLog(kCFLogLevelError, CFSTR("_CFURLCreateWithFileSystemPath (in) fileSystemPath '%@', pathStyle %@, isDirectory %@, baseURL %@\n\t_CFURLCreateWithFileSystemPath (out) result %@"), input_fileSystemPath, input_pathStyle == kCFURLPOSIXPathStyle ? CFSTR("POSIX") : input_pathStyle == kCFURLHFSPathStyle ? CFSTR("HFS"): CFSTR("Windows"), input_isDirectory ? CFSTR("YES") : CFSTR("NO"), input_baseURL, result);
#pragma GCC diagnostic pop
    if ( input_fileSystemPath ) {
        CFRelease(input_fileSystemPath);
    }
    if ( input_baseURL ) {
        CFRelease(input_baseURL);
    }
#endif
    return ( result );
}

/* initializes a URL object with the file system representation */
static CFURLRef _CFURLCreateWithFileSystemRepresentation(CFAllocatorRef allocator, const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory, CFURLRef baseURL)
{
#if DEBUG_URL_INITIALIZER_LOGGING
    STACK_BUFFER_DECL(UInt8, input_buffer, bufLen);
    CFIndex input_bufLen = bufLen;
    Boolean input_isDirectory = isDirectory;
    CFURLRef input_baseURL = baseURL ? CFRetain(baseURL) : NULL;
    memcpy(input_buffer, buffer, bufLen);
#endif
    struct __CFURL *result = NULL;
    if (bufLen > 0) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
        Boolean isAbsolute = bufLen && (*buffer == '/');
        Boolean addedPercentEncoding;
        Boolean releaseBaseURL = false;
        
        if ( isAbsolute ) {
            // if buffer contains an absolute path, ignore baseURL (if provided)
            baseURL = NULL;
        }
        else if ( baseURL == NULL ) {
            // if buffer contains a relative path and no baseURL is provided, use the current working directory
            baseURL = _CFURLCreateCurrentDirectoryURL(allocator);
            releaseBaseURL = true;
        }
        CFStringRef urlString = CreateStringFromFileSystemRepresentationByAddingPercentEscapes(allocator, buffer, bufLen, isDirectory, isAbsolute, false /*windowsPath*/, &addedPercentEncoding);
        if ( urlString ) {
            // allocate the URL object with the appropriate number of ranges
            result = _CFURLAlloc(allocator, isAbsolute ? (AddAuthorityToFileURL() ? 3 : 2) : 1);
            if ( result ) {
                result->_string = CFStringCreateCopy(allocator, urlString);
                result->_base = baseURL ? CFURLCopyAbsoluteURL(baseURL) : NULL;
#if DEBUG_URL_MEMORY_USAGE
                if ( result->_base ) {
                    numURLsWithBaseURL ++;
                }
#endif
                // hard coded parse
                if ( isAbsolute ) {
                    if ( AddAuthorityToFileURL() ) {
                        result->_flags = (addedPercentEncoding ? 0 : POSIX_AND_URL_PATHS_MATCH ) | (isDirectory ? IS_DIRECTORY : 0) | IS_DECOMPOSABLE | HAS_SCHEME | HAS_HOST | HAS_PATH | ORIGINAL_AND_URL_STRINGS_MATCH | IS_CANONICAL_FILE_URL;
                        _setSchemeTypeInFlags(&result->_flags, kHasFileScheme);
                        result->_ranges[0] = CFRangeMake(0, 4); // scheme "file"
                        result->_ranges[1] = CFRangeMake(7, 9); // host "localhost"
                        result->_ranges[2] = CFRangeMake(16, CFStringGetLength(urlString)- 16);
                    }
                    else {
                        result->_flags = (addedPercentEncoding ? 0 : POSIX_AND_URL_PATHS_MATCH ) | (isDirectory ? IS_DIRECTORY : 0) | IS_DECOMPOSABLE | HAS_SCHEME | HAS_PATH | ORIGINAL_AND_URL_STRINGS_MATCH | IS_CANONICAL_FILE_URL;
                        _setSchemeTypeInFlags(&result->_flags, kHasFileScheme);
                        result->_ranges[0] = CFRangeMake(0, 4); // scheme "file"
                        result->_ranges[1] = CFRangeMake(7, CFStringGetLength(urlString)- 7);
                    }
                } else {
                    result->_flags = (addedPercentEncoding ? 0 : POSIX_AND_URL_PATHS_MATCH ) | (isDirectory ? IS_DIRECTORY : 0) | IS_DECOMPOSABLE | HAS_PATH | ORIGINAL_AND_URL_STRINGS_MATCH;
                    *(result->_ranges) = CFRangeMake(0, CFStringGetLength(result->_string));
                }
            }
            if ( releaseBaseURL && baseURL ) {
                CFRelease(baseURL);
            }
            CFRelease(urlString);
        }
#elif DEPLOYMENT_TARGET_WINDOWS
        CFStringRef filePath = CFStringCreateWithBytes(allocator, buffer, bufLen, CFStringFileSystemEncoding(), false);
        if ( filePath ) {
            result = _CFURLCreateWithFileSystemPath(allocator, filePath, kCFURLWindowsPathStyle, isDirectory, baseURL);
            CFRelease(filePath);
        }
#endif
    }
    else if ( baseURL ) {
        result = (struct __CFURL *)CFRetain(baseURL);
    }
#if DEBUG_URL_INITIALIZER_LOGGING
    CFLog(kCFLogLevelError, CFSTR("_CFURLCreateWithFileSystemRepresentation (in) buffer '%*s', isDirectory %@, baseURL %@\n\t_CFURLCreateWithFileSystemRepresentation (out) result %@"), input_bufLen, input_buffer, input_isDirectory ? CFSTR("YES") : CFSTR("NO"), input_baseURL, result);
    if ( input_baseURL ) {
        CFRelease(input_baseURL);
    }
#endif
    return ( result );
}

// encoding will be used both to interpret the bytes of URLBytes, and to interpret any percent-escapes within the bytes.
CFURLRef CFURLCreateWithBytes(CFAllocatorRef allocator, const uint8_t *URLBytes, CFIndex length, CFStringEncoding encoding, CFURLRef baseURL) {
    CFStringRef  urlString = CFStringCreateWithBytes(allocator, URLBytes, length, encoding, false);
    CFURLRef result = NULL;
    if ( urlString ) {
            result = _CFURLCreateWithURLString(allocator, urlString, false /* checkForLegalCharacters */, baseURL);
            if ( result ) {
                if (encoding != kCFStringEncodingUTF8) {
                    ((struct __CFURL *)result)->_encoding = encoding;
#if DEBUG_URL_MEMORY_USAGE
                    numNonUTF8EncodedURLs++;
#endif
                }
            }
        CFRelease(urlString); // it's retained by result, now.
    }
    return ( result );
}

CFDataRef CFURLCreateData(CFAllocatorRef allocator, CFURLRef  url, CFStringEncoding encoding, Boolean escapeWhitespace) {
    CFDataRef result = NULL;
    if ( url ) {
        CFStringRef myStr = CFURLGetString(url);
        if ( myStr ) {
            result = CFStringCreateExternalRepresentation(allocator, myStr, encoding, 0);
        }
    }
    return result;
}

// Any escape sequences in URLString will be interpreted via UTF-8.
CFURLRef CFURLCreateWithString(CFAllocatorRef allocator, CFStringRef  URLString, CFURLRef  baseURL) {
    CFURLRef url = NULL;
    if ( URLString ) {
            url = _CFURLCreateWithURLString(allocator, URLString, true /* checkForLegalCharacters */, baseURL);
    }
    return ( url );
}

static CFURLRef _CFURLCreateWithArbitraryString(CFAllocatorRef allocator, CFStringRef URLString, CFURLRef baseURL) {
    CFURLRef url = NULL;
    if ( URLString ) {
            url = _CFURLCreateWithURLString(allocator, URLString, false /* checkForLegalCharacters */, baseURL);
    }
    return ( url );
}

CFURLRef CFURLCreateAbsoluteURLWithBytes(CFAllocatorRef alloc, const UInt8 *relativeURLBytes, CFIndex length, CFStringEncoding encoding, CFURLRef baseURL, Boolean useCompatibilityMode) {
    CFURLRef result = NULL;
    
    /*
     CFURLCreateAbsoluteURLWithBytes() and useCompatibilityMode is for:
         <rdar://problem/2711611> Problem with '|' in url and calling CFURLCreateWithString()
         <rdar://problem/3085893> CFURL resolves "../" URLs at top level in a way that is not the same as web browsers
         <rdar://problem/3085920> CFURL API should be more helpful for accepting URLs with technically-bad characters
         <rdar://problem/3205656> Safari needs CFURL to deal with google.com URLs that end in "%"
         <rdar://problem/3219233> Safari needs CFURL to not remove path when relative URL is just a query string
         <rdar://problem/3219240> Safari needs CFURL to support "compatibility" resolution of partial URLs with schemes
     
     If useCompatibilityMode is true, the rules historically used on the web are used to resolve relativeString against baseURL - these rules are generally listed in the RFC as optional or alternate interpretations.  Otherwise, the strict rules from the RFC are used.
     
     The major differences are that in compatibility mode, we are lenient of the scheme appearing in relative portion, leading "../" components are removed from the final URL's path, and if the relative portion contains only resource specifier pieces (query, parameters, and fragment), then the last path component of the base URL will not be deleted
     */

    // if not useCompatibilityMode, use CFURLCreateWithBytes and then CFURLCopyAbsoluteURL if there's a baseURL
    if ( !useCompatibilityMode ) {
        CFURLRef url = CFURLCreateWithBytes(alloc, relativeURLBytes, length, encoding, baseURL);
        if ( url != NULL ) {
            if ( baseURL != NULL ) {
                result = CFURLCopyAbsoluteURL(url);
                CFRelease(url);
            } else {
                result = url;
            }
        }
    } else {
        UInt32 absFlags = 0;
        CFRange absRanges[MAX_COMPONENTS];
        uint8_t numberOfRanges;
        CFStringRef absString = NULL;
        Boolean absStringIsMutable = false;
        CFURLRef absURL;
        CFStringRef relativeString;
        
        relativeString = CFStringCreateWithBytes(alloc, relativeURLBytes, length, encoding, false);
        if ( relativeString != NULL ) {
            if (!baseURL) {
                absString = relativeString;
            } else {
                UniChar ch = 0;
                if ( CFStringGetLength(relativeString) > 0 ) {
                    ch = CFStringGetCharacterAtIndex(relativeString, 0);
                }
                if (ch == '?' || ch == ';' || ch == '#') {
                    // Nothing but parameter + query + fragment; append to the baseURL string
                    CFStringRef baseString;
                    if (CF_IS_OBJC(CFURLGetTypeID(), baseURL)) {
                        baseString = CFURLGetString(baseURL);
                    } else {
                        baseString = baseURL->_string;
                    }
                    absString = CFStringCreateMutable(alloc, CFStringGetLength(baseString) + CFStringGetLength(relativeString));
                    CFStringAppend((CFMutableStringRef)absString, baseString);
                    CFStringAppend((CFMutableStringRef)absString, relativeString);
                    absStringIsMutable = true;
                } else {
                    UInt32 relFlags = 0;
                    CFRange relRanges[MAX_COMPONENTS];
                    CFStringRef relString = NULL;
                    _parseComponents(alloc, relativeString, baseURL, &relFlags, relRanges, &numberOfRanges);
                    if (relFlags & HAS_SCHEME) {
                        CFStringRef baseScheme = CFURLCopyScheme(baseURL);
                        CFRange relSchemeRange = _rangeForComponent(relFlags, relRanges, HAS_SCHEME);
                        if (baseScheme && CFStringGetLength(baseScheme) == relSchemeRange.length && CFStringHasPrefix(relativeString, baseScheme)) {
                            relString = CFStringCreateWithSubstring(alloc, relativeString, CFRangeMake(relSchemeRange.length+1, CFStringGetLength(relativeString) - relSchemeRange.length - 1));
                            relFlags = 0;
                            _parseComponents(alloc, relString, baseURL, &relFlags, relRanges, &numberOfRanges);
                        } else {
                            // Discard the base string; the relative string is absolute and we're not in the funky edge case where the schemes match
                            CFRetain(relativeString);
                            absString = relativeString;
                        }
                        if (baseScheme) CFRelease(baseScheme);
                    } else {
                        CFRetain(relativeString);
                        relString = relativeString;
                    }
                    if (!absString) {
                        if (!CF_IS_OBJC(CFURLGetTypeID(), baseURL)) {
                            absString = resolveAbsoluteURLString(alloc, relString, relFlags, relRanges, baseURL->_string, baseURL->_flags, baseURL->_ranges);
                        } else {
                            CFStringRef baseString;
                            UInt32 baseFlags = 0;
                            CFRange baseRanges[MAX_COMPONENTS];
                            if (CF_IS_OBJC(CFURLGetTypeID(), baseURL)) {
                                baseString = CFURLGetString(baseURL);
                            } else {
                                baseString = baseURL->_string;
                            }
                            _parseComponents(alloc, baseString, NULL, &baseFlags, baseRanges, &numberOfRanges);
                            absString = resolveAbsoluteURLString(alloc, relString, relFlags, relRanges, baseString, baseFlags, baseRanges);
                        }
                        absStringIsMutable = true;
                    }
                    if (relString) CFRelease(relString);
                }
                CFRelease(relativeString);
            }            
        }
        if ( absString ) {
            _parseComponents(alloc, absString, NULL, &absFlags, absRanges, &numberOfRanges);
            if (absFlags & HAS_PATH) {
                CFRange pathRg = _rangeForComponent(absFlags, absRanges, HAS_PATH);
                // This is expensive, but it allows us to reuse _resolvedPath.  It should be cleaned up to get this allocation removed at some point. - REW
                UniChar *buf = (UniChar *)CFAllocatorAllocate(alloc, sizeof(UniChar) * (pathRg.length + 1), 0);
                CFStringRef newPath;
                CFStringGetCharacters(absString, pathRg, buf);
                buf[pathRg.length] = '\0';
                newPath = _resolvedPath(buf, buf + pathRg.length, '/', true, false, alloc);
                if (CFStringGetLength(newPath) != pathRg.length) {
                    if (!absStringIsMutable) {
                        CFStringRef tmp = CFStringCreateMutableCopy(alloc, CFStringGetLength(absString), absString);
                        CFRelease(absString);
                        absString = tmp;
                    }
                    CFStringReplace((CFMutableStringRef)absString, pathRg, newPath);
                }
                CFRelease(newPath);
                // Do not deallocate buf; newPath took ownership of it.
            }
            absURL = _CFURLCreateWithArbitraryString(alloc, absString, NULL);
            CFRelease(absString);
            if (absURL) {
                ((struct __CFURL *)absURL)->_encoding = encoding;
#if DEBUG_URL_MEMORY_USAGE
                if ( encoding != kCFStringEncodingUTF8 ) {
                    numNonUTF8EncodedURLs++;
                }
#endif
            }
            result = absURL;
        }
    }
    
    return ( result );
}

/* This function is this way because I pulled it out of _resolvedURLPath (so that _resolvedFileSystemPath could use it), and I didn't want to spend a bunch of energy reworking the code.  So instead of being a bit more intelligent about inputs, it just demands a slightly perverse set of parameters, to match the old _resolvedURLPath code.  -- REW, 6/14/99 */
static CFStringRef _resolvedPath(UniChar *pathStr, UniChar *end, UniChar pathDelimiter, Boolean stripLeadingDotDots, Boolean stripTrailingDelimiter, CFAllocatorRef alloc) {
    UniChar *idx = pathStr;
    while (idx < end) {
        if (*idx == '.') {
            if (idx+1 == end) {
                if (idx != pathStr) {
                    *idx = '\0';
                    end = idx;
                }
                break;
            } else if (*(idx+1) == pathDelimiter) {
                if (idx + 2 != end || idx != pathStr) {
                    memmove(idx, idx+2, (end-(idx+2)+1) * sizeof(UniChar));
                    end -= 2;
                    continue;
                } else {
                    // Do not delete the sole path component
                    break;
                }
            } else if (( end-idx >= 2 ) &&  *(idx+1) == '.' && (idx+2 == end || (( end-idx > 2 ) && *(idx+2) == pathDelimiter))) {
                if (idx - pathStr >= 2) {
                    // Need at least 2 characters between index and pathStr, because we know if index != newPath, then *(index-1) == pathDelimiter, and we need something before that to compact out.
                    UniChar *lastDelim = idx-2;
                    while (lastDelim >= pathStr && *lastDelim != pathDelimiter) lastDelim --;
                    lastDelim ++;
                    if (lastDelim != idx && (idx-lastDelim != 3 || *lastDelim != '.' || *(lastDelim +1) != '.')) {
                        // We have a genuine component to compact out
                        if (idx+2 != end) {
                            unsigned numCharsToMove = end - (idx+3) + 1; // +1 to move the '\0' as well
                            memmove(lastDelim, idx+3, numCharsToMove * sizeof(UniChar));
                            end -= (idx + 3 - lastDelim);
                            idx = lastDelim;
                            continue;
                        } else if (lastDelim != pathStr) {
                            *lastDelim = '\0';
                            end = lastDelim;
                            break;
                        } else {
                            // Don't allow the path string to devolve to the empty string.  Fall back to "." instead. - REW
                            pathStr[0] = '.';
                            pathStr[1] = '/';
                            pathStr[2] = '\0';
                            end = &pathStr[3];
                            break;
                        }
                    }
                } else if (stripLeadingDotDots) {
                    if (idx + 3 != end) {
                        unsigned numCharsToMove = end - (idx + 3) + 1;
                        memmove(idx, idx+3, numCharsToMove * sizeof(UniChar));
                        end -= 3;
                        continue;
                    } else {
                        // Do not devolve the last path component
                        break;
                    }
                }
            }
        }
		while (idx < end && *idx != pathDelimiter) idx ++;
        idx ++;
    }
    if (stripTrailingDelimiter && end > pathStr && end-1 != pathStr && *(end-1) == pathDelimiter) {
        end --;
    }
    // return an zero-length string if end < pathStr
    return CFStringCreateWithCharactersNoCopy(alloc, pathStr, end >= pathStr ? end - pathStr : 0, alloc);
}

static CFMutableStringRef resolveAbsoluteURLStringBuffer(CFAllocatorRef alloc, CFStringRef relString, UInt32 relFlags, const CFRange *relRanges, CFStringRef baseString, UInt32 baseFlags, const CFRange *baseRanges, UniChar *buf)
{
    CFStringAppendBuffer appendBuffer;
    UniChar chars[2];
    CFStringInitAppendBuffer(alloc, &appendBuffer);
    CFRange rg;
    
    rg = _rangeForComponent(baseFlags, baseRanges, HAS_SCHEME);
    if (rg.location != kCFNotFound) {
        CFStringGetCharacters(baseString, rg, buf);
        CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
        chars[0] = ':';
        CFStringAppendCharactersToAppendBuffer(&appendBuffer, chars, 1);
    }
    
    if (relFlags & NET_LOCATION_MASK) {
        CFStringAppendStringToAppendBuffer(&appendBuffer, relString);
    } else {
        chars[0] = '/';
        chars[1] = '/';
        CFStringAppendCharactersToAppendBuffer(&appendBuffer, chars, 2);
        rg = _netLocationRange(baseFlags, baseRanges);
        if (rg.location != kCFNotFound) {
            CFStringGetCharacters(baseString, rg, buf);
            CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
        }
        
        if (relFlags & HAS_PATH) {
            CFRange relPathRg = _rangeForComponent(relFlags, relRanges, HAS_PATH);
            CFRange basePathRg = _rangeForComponent(baseFlags, baseRanges, HAS_PATH);
            CFStringRef newPath;
            Boolean useRelPath = false;
            Boolean useBasePath = false;
            if (basePathRg.location == kCFNotFound) {
                useRelPath = true;
            } else if (relPathRg.length == 0) {
                useBasePath = true;
            } else if (CFStringGetCharacterAtIndex(relString, relPathRg.location) == '/') {
                useRelPath = true;
            } else if (basePathRg.location == kCFNotFound || basePathRg.length == 0) {
                useRelPath = true;
            }
            if (useRelPath) {
                newPath = CFStringCreateWithSubstring(alloc, relString, relPathRg);
            } else if (useBasePath) {
                newPath = CFStringCreateWithSubstring(alloc, baseString, basePathRg);
            } else {
                // FIXME: Get rid of this allocation
                UniChar *newPathBuf = (UniChar *)CFAllocatorAllocate(alloc, sizeof(UniChar) * (relPathRg.length + basePathRg.length + 1), 0);
                UniChar *idx, *end;
                CFStringGetCharacters(baseString, basePathRg, newPathBuf);
                idx = newPathBuf + basePathRg.length - 1;
                while (idx != newPathBuf && *idx != '/') idx --;
                if (*idx == '/') idx ++;
                CFStringGetCharacters(relString, relPathRg, idx);
                end = idx + relPathRg.length;
                *end = 0;
                newPath = _resolvedPath(newPathBuf, end, '/', false, false, alloc);
            }
            /* Under Win32 absolute path can begin with letter
             * so we have to add one '/' to the newString
             * (Sergey Zubarev)
             */
            // No - the input strings here are URL path strings, not Win32 paths.
            // Absolute paths should have had a '/' prepended before this point.
            // I have removed Sergey Zubarev's change and left his comment (and
            // this one) as a record. - REW, 1/5/2004
            
            // if the relative URL does not begin with a slash and
            // the base does not end with a slash, add a slash
            if ((basePathRg.location == kCFNotFound || basePathRg.length == 0) && CFStringGetCharacterAtIndex(newPath, 0) != '/') {
                chars[0] = '/';
                CFStringAppendCharactersToAppendBuffer(&appendBuffer, chars, 1);
            }
            
            CFStringAppendStringToAppendBuffer(&appendBuffer, newPath);
            CFRelease(newPath);
            rg.location = relPathRg.location + relPathRg.length;
            rg.length = CFStringGetLength(relString);
            if (rg.length > rg.location) {
                rg.length -= rg.location;
                CFStringGetCharacters(relString, rg, buf);
                CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
            }
        } else {
            rg = _rangeForComponent(baseFlags, baseRanges, HAS_PATH);
            if (rg.location != kCFNotFound) {
                CFStringGetCharacters(baseString, rg, buf);
                CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
            }
            
            if (!(relFlags & RESOURCE_SPECIFIER_MASK)) {
                // ???  Can this ever happen?
                UInt32 rsrcFlag = _firstResourceSpecifierFlag(baseFlags);
                if (rsrcFlag) {
                    rg.location = _rangeForComponent(baseFlags, baseRanges, rsrcFlag).location;
                    rg.length = CFStringGetLength(baseString) - rg.location;
                    rg.location --; // To pick up the separator
                    rg.length ++;
                    CFStringGetCharacters(baseString, rg, buf);
                    CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
                }
            } else if (relFlags & HAS_PARAMETERS) {
                rg = _rangeForComponent(relFlags, relRanges, HAS_PARAMETERS);
                rg.location --; // To get the semicolon that starts the parameters
                rg.length = CFStringGetLength(relString) - rg.location;
                CFStringGetCharacters(relString, rg, buf);
                CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
            } else {
                // Sigh; we have to resolve these against one another
                rg = _rangeForComponent(baseFlags, baseRanges, HAS_PARAMETERS);
                if (rg.location != kCFNotFound) {
                    chars[0] = ';';
                    CFStringAppendCharactersToAppendBuffer(&appendBuffer, chars, 1);
                    CFStringGetCharacters(baseString, rg, buf);
                    CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
                }
                rg = _rangeForComponent(relFlags, relRanges, HAS_QUERY);
                if (rg.location != kCFNotFound) {
                    chars[0] = '?';
                    CFStringAppendCharactersToAppendBuffer(&appendBuffer, chars, 1);
                    CFStringGetCharacters(relString, rg, buf);
                    CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
                } else {
                    rg = _rangeForComponent(baseFlags, baseRanges, HAS_QUERY);
                    if (rg.location != kCFNotFound) {
                        chars[0] = '?';
                        CFStringAppendCharactersToAppendBuffer(&appendBuffer, chars, 1);
                        CFStringGetCharacters(baseString, rg, buf);
                        CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
                    }
                }
                // Only the relative portion of the URL can supply the fragment; otherwise, what would be in the relativeURL?
                rg = _rangeForComponent(relFlags, relRanges, HAS_FRAGMENT);
                if (rg.location != kCFNotFound) {
                    chars[0] = '#';
                    CFStringAppendCharactersToAppendBuffer(&appendBuffer, chars, 1);
                    CFStringGetCharacters(relString, rg, buf);
                    CFStringAppendCharactersToAppendBuffer(&appendBuffer, buf, rg.length);
                }
            }
        }
    }
    return CFStringCreateMutableWithAppendBuffer(&appendBuffer);
}

static CFMutableStringRef resolveAbsoluteURLString(CFAllocatorRef alloc, CFStringRef relString, UInt32 relFlags, const CFRange *relRanges, CFStringRef baseString, UInt32 baseFlags, const CFRange *baseRanges) {
    CFMutableStringRef result;
    CFIndex bufLen = CFStringGetLength(baseString) + CFStringGetLength(relString); // Overkill, but guarantees we never allocate again
    if ( bufLen <= 1024 ) {
        STACK_BUFFER_DECL(UniChar, buf, bufLen);
        result = resolveAbsoluteURLStringBuffer(alloc, relString, relFlags, relRanges, baseString, baseFlags, baseRanges, buf);
        return ( result );
    }
    else {
        UniChar *buf = (UniChar *)CFAllocatorAllocate(alloc, bufLen * sizeof(UniChar), 0);
        result = resolveAbsoluteURLStringBuffer(alloc, relString, relFlags, relRanges, baseString, baseFlags, baseRanges, buf);
        CFAllocatorDeallocate(alloc, buf);
        return ( result );
    }
}

CFURLRef CFURLCopyAbsoluteURL(CFURLRef  relativeURL) {
    CFURLRef  anURL, base;
    CFAllocatorRef alloc = CFGetAllocator(relativeURL);
    CFStringRef baseString, newString;
    UInt32 baseFlags;
    CFRange ranges[MAX_COMPONENTS];
    uint8_t numberOfRanges;
    const CFRange *baseRanges;
    Boolean baseIsObjC;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    Boolean filePathURLCreated = false;
#endif
    
    CFAssert1(relativeURL != NULL, __kCFLogAssertion, "%s(): Cannot create an absolute URL from a NULL relative URL", __PRETTY_FUNCTION__);
    if (CF_IS_OBJC(CFURLGetTypeID(), relativeURL)) {
        anURL = (CFURLRef) CF_OBJC_CALLV((NSURL *)relativeURL, absoluteURL);
        if (anURL) CFRetain(anURL);
        return anURL;
    } 

    __CFGenericValidateType(relativeURL, CFURLGetTypeID());

    base = relativeURL->_base;
    if (!base) {
        return (CFURLRef)CFRetain(relativeURL);
    }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    else if ( CFURLIsFileReferenceURL(base) && !CFURLHasDirectoryPath(base) ) {
        // 16695827 - If the base URL is a file reference URL which doesn't end with a slash, we have to convert it to a file path URL before we can make it absolute.
        base = CFURLCreateFilePathURL(alloc, base, NULL);
        if ( !base ) {
            // could not convert file reference URL to file path URL -- fail will NULL
            return NULL;
        }
        filePathURLCreated = true;
    }
#endif
    
    baseIsObjC = CF_IS_OBJC(CFURLGetTypeID(), base);

    if (!baseIsObjC) {
        baseString = base->_string;
        baseFlags = base->_flags;
        baseRanges = base->_ranges;
    } else {
        baseString = CFURLGetString(base);
        baseFlags = 0;
        _parseComponents(alloc, baseString, NULL, &baseFlags, ranges, &numberOfRanges);
        baseRanges = ranges;
    }
    
    newString = resolveAbsoluteURLString(alloc, relativeURL->_string, relativeURL->_flags, relativeURL->_ranges, baseString, baseFlags, baseRanges);
    anURL = _CFURLCreateWithArbitraryString(alloc, newString, NULL);
    CFRelease(newString);
    ((struct __CFURL *)anURL)->_encoding = relativeURL->_encoding;
#if DEBUG_URL_MEMORY_USAGE
    if ( relativeURL->_encoding != kCFStringEncodingUTF8 ) {
	numNonUTF8EncodedURLs++;
    }
#endif
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if ( filePathURLCreated ) {
        CFRelease(base);
    }
#endif
    
    return anURL;
}


/*******************/
/* Basic accessors */
/*******************/
CFStringEncoding _CFURLGetEncoding(CFURLRef url) {
    return url->_encoding;
}

Boolean CFURLCanBeDecomposed(CFURLRef  anURL) {
    anURL = _CFURLFromNSURL(anURL);
    return ((anURL->_flags & IS_DECOMPOSABLE) != 0);
}

CFStringRef  CFURLGetString(CFURLRef  url) {
    CF_OBJC_FUNCDISPATCHV(CFURLGetTypeID(), CFStringRef, (NSURL *)url, relativeString);
    if (!_haveTestedOriginalString(url)) {
        computeSanitizedString(url);
    }
    if (url->_flags & ORIGINAL_AND_URL_STRINGS_MATCH) {
        return url->_string;
    } else {
        return _getSanitizedString( url );
    }
}

CFIndex CFURLGetBytes(CFURLRef url, UInt8 *buffer, CFIndex bufferLength) {
    CFIndex length, charsConverted, usedLength;
    CFStringRef string;
    CFStringEncoding enc;
    if (CF_IS_OBJC(CFURLGetTypeID(), url)) {
        string = CFURLGetString(url);
        enc = kCFStringEncodingUTF8;
    } else {
        string = url->_string;
        enc = url->_encoding;
    }
    length = CFStringGetLength(string);
    charsConverted = CFStringGetBytes(string, CFRangeMake(0, length), enc, 0, false, buffer, bufferLength, &usedLength);
    if (charsConverted != length) {
        return -1;
    } else {
        return usedLength;
    }
}

CFURLRef  CFURLGetBaseURL(CFURLRef  anURL) {
    CF_OBJC_FUNCDISPATCHV(CFURLGetTypeID(), CFURLRef, (NSURL *)anURL, baseURL);
    return anURL->_base;
}

// Assumes the URL is already parsed
static CFRange _rangeForComponent(UInt32 flags, const CFRange *ranges, UInt32 compFlag) {
    UInt32 idx = 0;
    if (!(flags & compFlag)) return CFRangeMake(kCFNotFound, 0);
    while (!(compFlag & 1)) {
        compFlag = compFlag >> 1;
        if (flags & 1) {
            idx ++;
        }
        flags = flags >> 1;
    }
    return ranges[idx];
}
 
static CFStringRef _retainedComponentString(CFURLRef url, UInt32 compFlag, Boolean fromOriginalString, Boolean removePercentEscapes) {
    CFRange rg;
    CFStringRef comp;
    CFAllocatorRef alloc = CFGetAllocator(url);
    if (removePercentEscapes) {
        fromOriginalString = true;
    }
    rg = _rangeForComponent(url->_flags, url->_ranges, compFlag);
    if (rg.location == kCFNotFound) {
        comp = NULL;
    }
    else {
        if ( compFlag & HAS_SCHEME ) {
            switch ( _getSchemeTypeFromFlags(url->_flags) ) {
                case kHasHttpScheme:
                    comp = (CFStringRef)CFRetain(kCFURLHTTPScheme);
                    break;
                    
                case kHasHttpsScheme:
                    comp = (CFStringRef)CFRetain(kCFURLHTTPSScheme);
                    break;
                    
                case kHasFileScheme:
                    comp = (CFStringRef)CFRetain(kCFURLFileScheme);
                    break;
                    
                case kHasDataScheme:
                    comp = (CFStringRef)CFRetain(kCFURLDataScheme);
                    break;
                    
                case kHasFtpScheme:
                    comp = (CFStringRef)CFRetain(kCFURLFTPScheme);
                    break;
                    
                default:
                    comp = CFStringCreateWithSubstring(alloc, url->_string, rg);
                    break;
            }
        }
        else {
            comp = CFStringCreateWithSubstring(alloc, url->_string, rg);
        }
        
        if (comp && !fromOriginalString) {
            if (!_haveTestedOriginalString(url)) {
                computeSanitizedString(url);
            }
            if (!(url->_flags & ORIGINAL_AND_URL_STRINGS_MATCH) && (_getAdditionalDataFlags(url) & compFlag)) {
                CFStringRef newComp = correctedComponent(comp, compFlag, url->_encoding);
                CFRelease(comp);
                comp = newComp;
            }
        }
        if (comp && removePercentEscapes) {
            CFStringRef tmp;
            if (url->_encoding == kCFStringEncodingUTF8) {
                tmp = CFURLCreateStringByReplacingPercentEscapes(alloc, comp, CFSTR(""));
            } else {
                tmp = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(alloc, comp, CFSTR(""), url->_encoding);
            }
            CFRelease(comp);
            comp = tmp;
        }
        
    }
    return comp;
}

CFStringRef  CFURLCopyScheme(CFURLRef  anURL) {
    CFStringRef scheme;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        scheme = (CFStringRef) CF_OBJC_CALLV((NSURL *)anURL, scheme);
        if ( scheme ) {
            CFRetain(scheme);
        }
    }
    else {
        switch ( _getSchemeTypeFromFlags(anURL->_flags) ) {
            case kHasHttpScheme:
                scheme = (CFStringRef)CFRetain(kCFURLHTTPScheme);
                break;
                
            case kHasHttpsScheme:
                scheme = (CFStringRef)CFRetain(kCFURLHTTPSScheme);
                break;
                
            case kHasFileScheme:
                scheme = (CFStringRef)CFRetain(kCFURLFileScheme);
                break;
                
            case kHasDataScheme:
                scheme = (CFStringRef)CFRetain(kCFURLDataScheme);
                break;
                
            case kHasFtpScheme:
                scheme = (CFStringRef)CFRetain(kCFURLFTPScheme);
                break;
                
            default:
                scheme = _retainedComponentString(anURL, HAS_SCHEME, true, false);
                if ( !scheme ) {
                    if (anURL->_base) {
                        scheme = CFURLCopyScheme(anURL->_base);
                    } else {
                        scheme = NULL;
                    }
                }
                break;
        }
    }
    return ( scheme );
}

static CFRange _netLocationRange(UInt32 flags, const CFRange *ranges) {
    CFRange netRgs[4];
    CFRange netRg = {kCFNotFound, 0};
    CFIndex i, c = 4;

    if ((flags & NET_LOCATION_MASK) == 0) return CFRangeMake(kCFNotFound, 0);

    netRgs[0] = _rangeForComponent(flags, ranges, HAS_USER);
    netRgs[1] = _rangeForComponent(flags, ranges, HAS_PASSWORD);
    netRgs[2] = _rangeForComponent(flags, ranges, HAS_HOST);
    netRgs[3] = _rangeForComponent(flags, ranges, HAS_PORT);
    for (i = 0; i < c; i ++) {
        if (netRgs[i].location == kCFNotFound) continue;
        if (netRg.location == kCFNotFound) {
            netRg = netRgs[i];
        } else {
            netRg.length = netRgs[i].location + netRgs[i].length - netRg.location;
        }
    }
    return netRg;
}

CFStringRef CFURLCopyNetLocation(CFURLRef  anURL) {
    anURL = _CFURLFromNSURL(anURL);
    if (anURL->_flags & NET_LOCATION_MASK) {
        // We provide the net location
        CFRange netRg = _netLocationRange(anURL->_flags, anURL->_ranges);
        CFStringRef netLoc;
        if (!_haveTestedOriginalString(anURL)) {
            computeSanitizedString(anURL);
        }
        if (!(anURL->_flags & ORIGINAL_AND_URL_STRINGS_MATCH) && (_getAdditionalDataFlags(anURL) & (USER_DIFFERS | PASSWORD_DIFFERS | HOST_DIFFERS | PORT_DIFFERS))) {
            // Only thing that can come before the net location is the scheme.  It's impossible for the scheme to contain percent escapes.  Therefore, we can use the location of netRg in _sanatizedString, just not the length. 
            CFRange netLocEnd;
            CFStringRef sanitizedString = _getSanitizedString(anURL);
            netRg.length = CFStringGetLength(sanitizedString) - netRg.location;
            if (CFStringFindWithOptions(sanitizedString, CFSTR("/"), netRg, 0, &netLocEnd)) {
                netRg.length = netLocEnd.location - netRg.location;
            }
            netLoc = CFStringCreateWithSubstring(CFGetAllocator(anURL), sanitizedString, netRg);
        } else {
            netLoc = CFStringCreateWithSubstring(CFGetAllocator(anURL), anURL->_string, netRg);
        }
        return netLoc;
    } else if (anURL->_base) {
        return CFURLCopyNetLocation(anURL->_base);
    } else {
        return NULL;
    }
}

// NOTE - if you want an absolute path, you must first get the absolute URL.  If you want a file system path, use the file system methods above.
CFStringRef  CFURLCopyPath(CFURLRef  anURL) {
    anURL = _CFURLFromNSURL(anURL);
    return _retainedComponentString(anURL, HAS_PATH, false, false);
}

/* NULL if CFURLCanBeDecomposed(anURL) is false; also does not resolve the URL against its base.  See also CFCreateAbsoluteURL().  Note that, strictly speaking, any leading '/' is not considered part of the URL's path, although its presence or absence determines whether the path is absolute.  CFURLCopyPath()'s return value includes any leading slash (giving the path the normal POSIX appearance); CFURLCopyStrictPath()'s return value omits any leading slash, and uses isAbsolute to report whether the URL's path is absolute.

  CFURLCopyFileSystemPath() returns the URL's path as a file system path for the given path style.  All percent escape sequences are replaced.  The URL is not resolved against its base before computing the path.
*/
CFStringRef CFURLCopyStrictPath(CFURLRef anURL, Boolean *isAbsolute) {
    CFStringRef path = CFURLCopyPath(anURL);
    if (!path || CFStringGetLength(path) == 0) {
        if (path) CFRelease(path);
        if (isAbsolute) *isAbsolute = false;
        return NULL;
    }
    if (CFStringGetCharacterAtIndex(path, 0) == '/') {
        CFStringRef tmp;
        if (isAbsolute) *isAbsolute = true;
        tmp = CFStringCreateWithSubstring(CFGetAllocator(path), path, CFRangeMake(1, CFStringGetLength(path)-1));
        CFRelease(path);
        path = tmp;
    } else {
        if (isAbsolute) *isAbsolute = false;
    }
    return path;
}

Boolean CFURLHasDirectoryPath(CFURLRef  anURL) {
    anURL = _CFURLFromNSURL(anURL);
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    if (!anURL->_base || (anURL->_flags & (HAS_PATH | NET_LOCATION_MASK))) {
        return ((anURL->_flags & IS_DIRECTORY) != 0);
    }
    else {
        return CFURLHasDirectoryPath(anURL->_base);
    }
}

static UInt32 _firstResourceSpecifierFlag(UInt32 flags) {
    UInt32 firstRsrcSpecFlag = 0;
    UInt32 flag = HAS_FRAGMENT;
    while (flag != HAS_PATH) {
        if (flags & flag) {
            firstRsrcSpecFlag = flag;
        }
        flag = flag >> 1;
    }
    return firstRsrcSpecFlag;
}

CFStringRef  CFURLCopyResourceSpecifier(CFURLRef  anURL) {
    anURL = _CFURLFromNSURL(anURL);
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    if (!(anURL->_flags & IS_DECOMPOSABLE)) {
        CFRange schemeRg = _rangeForComponent(anURL->_flags, anURL->_ranges, HAS_SCHEME);
        CFIndex base = schemeRg.location + schemeRg.length + 1;
        if (!_haveTestedOriginalString(anURL)) {
            computeSanitizedString(anURL);
        }
        
        CFStringRef sanitizedString = _getSanitizedString(anURL);
        
        if (sanitizedString) {
            // It is impossible to have a percent escape in the scheme (if there were one, we would have considered the URL a relativeURL with a  colon in the path instead), so this range computation is always safe.
            return CFStringCreateWithSubstring(CFGetAllocator(anURL), sanitizedString, CFRangeMake(base, CFStringGetLength(sanitizedString)-base));
        } else {
            return CFStringCreateWithSubstring(CFGetAllocator(anURL), anURL->_string, CFRangeMake(base, CFStringGetLength(anURL->_string)-base));
        }
    } else {
        UInt32 firstRsrcSpecFlag = _firstResourceSpecifierFlag(anURL->_flags);
        UInt32 flag;
        if (firstRsrcSpecFlag) {
            Boolean canUseOriginalString = true;
            Boolean canUseSanitizedString = true;
            CFAllocatorRef alloc = CFGetAllocator(anURL);
            if (!_haveTestedOriginalString(anURL)) {
                computeSanitizedString(anURL);
            }
            
            UInt32 additionalDataFlags = _getAdditionalDataFlags(anURL);
            CFStringRef sanitizedString = _getSanitizedString(anURL);
            
            if (!(anURL->_flags & ORIGINAL_AND_URL_STRINGS_MATCH)) {
                // See if any pieces in the resource specifier differ between sanitized string and original string
                for (flag = firstRsrcSpecFlag; flag != (HAS_FRAGMENT << 1); flag = flag << 1) {
                    if (additionalDataFlags & flag) {
                        canUseOriginalString = false;
                        break;
                    }
                }
            }
            if (!canUseOriginalString) {
                // If none of the pieces prior to the first resource specifier flag differ, then we can use the offset from the original string as the offset in the sanitized string.
                for (flag = firstRsrcSpecFlag >> 1; flag != 0; flag = flag >> 1) {
                    if (additionalDataFlags & flag) {
                        canUseSanitizedString = false;
                        break;
                    }
                }
            }
            if (canUseOriginalString) {
                CFRange rg = _rangeForComponent(anURL->_flags, anURL->_ranges, firstRsrcSpecFlag);
                rg.location --; // Include the character that demarcates the component
                rg.length = CFStringGetLength(anURL->_string) - rg.location;
                return CFStringCreateWithSubstring(alloc, anURL->_string, rg);
            } else if (canUseSanitizedString) {
                CFRange rg = _rangeForComponent(anURL->_flags, anURL->_ranges, firstRsrcSpecFlag);
                rg.location --; // Include the character that demarcates the component
                rg.length = CFStringGetLength(sanitizedString) - rg.location;
                return CFStringCreateWithSubstring(alloc, sanitizedString, rg);
            } else {
                // Must compute the correct string to return; just reparse....
                UInt32 sanFlags = 0;
                CFRange sanRanges[MAX_COMPONENTS];
                uint8_t numberOfRanges;
                CFRange rg;
                _parseComponents(alloc, sanitizedString, anURL->_base, &sanFlags, sanRanges, &numberOfRanges);
                rg = _rangeForComponent(sanFlags, sanRanges, firstRsrcSpecFlag);
                rg.location --; // Include the character that demarcates the component
                rg.length = CFStringGetLength(sanitizedString) - rg.location;
                return CFStringCreateWithSubstring(CFGetAllocator(anURL), sanitizedString, rg);
            }
        } else {
            // The resource specifier cannot possibly come from the base.
            return NULL;
        }
    }
}

/*************************************/
/* Accessors that create new objects */
/*************************************/

// For the next four methods, it is important to realize that, if a URL supplies any part of the net location (host, user, port, or password), it must supply all of the net location (i.e. none of it comes from its base URL).  Also, it is impossible for a URL to be relative, supply none of the net location, and still have its (empty) net location take precedence over its base URL (because there's nothing that precedes the net location except the scheme, and if the URL supplied the scheme, it would be absolute, and there would be no base).
CFStringRef  CFURLCopyHostName(CFURLRef  anURL) {
    CFStringRef tmp;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        tmp = (CFStringRef) CF_OBJC_CALLV((NSURL *)anURL, host);
        if (tmp) CFRetain(tmp);
        return tmp;
    } 
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    tmp = _retainedComponentString(anURL, HAS_HOST, true, true);
    if (tmp) {
        if (anURL->_flags & IS_IPV6_ENCODED) {
            // Have to strip off the brackets to get the true hostname.
            // Assume that to be legal the first and last characters are brackets!
            CFStringRef	strippedHost = CFStringCreateWithSubstring(CFGetAllocator(anURL), tmp, CFRangeMake(1, CFStringGetLength(tmp) - 2));
            CFRelease(tmp);
            tmp = strippedHost;
        }
        return tmp;
    } else if (anURL->_base && !(anURL->_flags & NET_LOCATION_MASK) && !(anURL->_flags & HAS_SCHEME)) {
        return CFURLCopyHostName(anURL->_base);
    } else {
        return NULL;
    }
}

// Return -1 to indicate no port is specified
SInt32 CFURLGetPortNumber(CFURLRef  anURL) {
    CFStringRef port;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        CFNumberRef cfPort = (CFNumberRef) CF_OBJC_CALLV((NSURL *)anURL, port);
        SInt32 num;
        if (cfPort && CFNumberGetValue(cfPort, kCFNumberSInt32Type, &num)) return num;
        return -1;
    } 
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    port = _retainedComponentString(anURL, HAS_PORT, true, false);
    if (port) {
        SInt32 portNum, idx, length = CFStringGetLength(port);
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(port, &buf, CFRangeMake(0, length));
        idx = 0;
        if (!__CFStringScanInteger(&buf, NULL, &idx, false, &portNum) || (idx != length)) {
            portNum = -1;
        }
        CFRelease(port);
        return portNum;
    } else if (anURL->_base && !(anURL->_flags & NET_LOCATION_MASK) && !(anURL->_flags & HAS_SCHEME)) {
        return CFURLGetPortNumber(anURL->_base);
    } else {
        return -1;
    }
}

CFStringRef  CFURLCopyUserName(CFURLRef  anURL) {
    CFStringRef user;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        user = (CFStringRef) CF_OBJC_CALLV((NSURL *)anURL, user);
        if (user) CFRetain(user);
        return user;
    } 
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    user = _retainedComponentString(anURL, HAS_USER, true, true);
    if (user) {
        return user;
    } else if (anURL->_base && !(anURL->_flags & NET_LOCATION_MASK) && !(anURL->_flags & HAS_SCHEME)) {
        return CFURLCopyUserName(anURL->_base);
    } else {
        return NULL;
    }
}

CFStringRef  CFURLCopyPassword(CFURLRef  anURL) {
    CFStringRef passwd;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        passwd = (CFStringRef) CF_OBJC_CALLV((NSURL *)anURL, password);
        if (passwd) CFRetain(passwd);
        return passwd;
    } 
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    passwd = _retainedComponentString(anURL, HAS_PASSWORD, true, true);
    if (passwd) {
        return passwd;
    } else if (anURL->_base && !(anURL->_flags & NET_LOCATION_MASK) && !(anURL->_flags & HAS_SCHEME)) {
        return CFURLCopyPassword(anURL->_base);
    } else {
        return NULL;
    }
}

// The NSURL methods do not deal with escaping escape characters at all; therefore, in order to properly bridge NSURL methods, and still provide the escaping behavior that we want, we need to create functions that match the ObjC behavior exactly, and have the public CFURL... functions call these. -- REW, 10/29/98

static CFStringRef  _unescapedParameterString(CFURLRef  anURL) {
    CFStringRef str;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        str = (CFStringRef) CF_OBJC_CALLV((NSURL *)anURL, parameterString);
        if (str) CFRetain(str);
        return str;
    } 
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    str = _retainedComponentString(anURL, HAS_PARAMETERS, false, false);
    if (str) return str;
    if (!(anURL->_flags & IS_DECOMPOSABLE)) return NULL;
    if (!anURL->_base || (anURL->_flags & (NET_LOCATION_MASK | HAS_PATH | HAS_SCHEME))) {
        return NULL;
        // Parameter string definitely coming from the relative portion of the URL
    }
    return _unescapedParameterString( anURL->_base);
}

CFStringRef  CFURLCopyParameterString(CFURLRef  anURL, CFStringRef charactersToLeaveEscaped) {
    CFStringRef  param = _unescapedParameterString(anURL);
    if (param) {
        CFStringRef result;
        if (anURL->_encoding == kCFStringEncodingUTF8) {
            result = CFURLCreateStringByReplacingPercentEscapes(CFGetAllocator(anURL), param, charactersToLeaveEscaped);
        } else {
            result = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(CFGetAllocator(anURL), param, charactersToLeaveEscaped, anURL->_encoding);
        }
        CFRelease(param);
        return result;
    }
    return NULL;
}

static CFStringRef  _unescapedQueryString(CFURLRef  anURL) {
    CFStringRef str;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        str = (CFStringRef) CF_OBJC_CALLV((NSURL *)anURL, query);
        if (str) CFRetain(str);
        return str;
    } 
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    str = _retainedComponentString(anURL, HAS_QUERY, false, false);
    if (str) return str;
    if (!(anURL->_flags & IS_DECOMPOSABLE)) return NULL;
    if (!anURL->_base || (anURL->_flags & (HAS_SCHEME | NET_LOCATION_MASK | HAS_PATH | HAS_PARAMETERS))) {
        return NULL;
    }
    return _unescapedQueryString(anURL->_base);
}

CFStringRef  CFURLCopyQueryString(CFURLRef  anURL, CFStringRef  charactersToLeaveEscaped) {
    CFStringRef  query = _unescapedQueryString(anURL);
    if (query) {
        CFStringRef tmp;
        if (anURL->_encoding == kCFStringEncodingUTF8) {
            tmp = CFURLCreateStringByReplacingPercentEscapes(CFGetAllocator(anURL), query, charactersToLeaveEscaped);
        } else {
            tmp = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(CFGetAllocator(anURL), query, charactersToLeaveEscaped, anURL->_encoding);
        }
        CFRelease(query);
        return tmp;
    }
    return NULL;
}

// Fragments are NEVER taken from a base URL
static CFStringRef  _unescapedFragment(CFURLRef  anURL) {
    CFStringRef str;
    if (CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        str = (CFStringRef) CF_OBJC_CALLV((NSURL *)anURL, fragment);
        if (str) CFRetain(str);
        return str;
    } 
    __CFGenericValidateType(anURL, CFURLGetTypeID());
    str = _retainedComponentString(anURL, HAS_FRAGMENT, false, false);
    return str;
}

CFStringRef  CFURLCopyFragment(CFURLRef  anURL, CFStringRef  charactersToLeaveEscaped) {
    CFStringRef  fragment = _unescapedFragment(anURL);
    if (fragment) {
        CFStringRef tmp;
        if (anURL->_encoding == kCFStringEncodingUTF8) {
            tmp = CFURLCreateStringByReplacingPercentEscapes(CFGetAllocator(anURL), fragment, charactersToLeaveEscaped);
        } else {
            tmp = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(CFGetAllocator(anURL), fragment, charactersToLeaveEscaped, anURL->_encoding);
        }
        CFRelease(fragment);
        return tmp;
    }
    return NULL;
}

static CFIndex insertionLocationForMask(CFURLRef url, CFOptionFlags mask) {
    CFIndex firstMaskFlag = 1;
    CFIndex lastComponentBeforeMask = 0;
    while (firstMaskFlag <= HAS_FRAGMENT) {
        if (firstMaskFlag & mask) break;
        if (url->_flags & firstMaskFlag) lastComponentBeforeMask = firstMaskFlag;
        firstMaskFlag = firstMaskFlag << 1;
    }
    if (lastComponentBeforeMask == 0) {
        // mask includes HAS_SCHEME
        return 0;
    } else if (lastComponentBeforeMask == HAS_SCHEME) {
        // Do not have to worry about the non-decomposable case here.  However, we must be prepared for the degenerate
        // case file:/path/immediately/without/host
        CFRange schemeRg = _rangeForComponent(url->_flags, url->_ranges, HAS_SCHEME);
        CFRange pathRg = _rangeForComponent(url->_flags, url->_ranges, HAS_PATH);
        if (schemeRg.length + 1 == pathRg.location) {
            return schemeRg.length + 1;
        } else {
            return schemeRg.length + 3;
        }
    } else {
        // For all other components, the separator precedes the component, so there's no need
        // to add extra chars to get to the next insertion point
        CFRange rg = _rangeForComponent(url->_flags, url->_ranges, lastComponentBeforeMask);
        return rg.location + rg.length;
    }
}

static CFRange _CFURLGetCharRangeForMask(CFURLRef url, CFOptionFlags mask, CFRange *charRangeWithSeparators) {
    CFOptionFlags currentOption;
    CFOptionFlags firstMaskFlag = HAS_SCHEME;
    Boolean haveReachedMask = false;
    CFIndex beforeMask = 0;
    CFIndex afterMask = kCFNotFound;
    const CFRange *currRange = url->_ranges;
    CFRange maskRange = {kCFNotFound, 0};
    for (currentOption = 1; currentOption <= HAS_FRAGMENT; currentOption = currentOption << 1) {
        if (!haveReachedMask && (currentOption & mask) != 0) {
            firstMaskFlag = currentOption;
            haveReachedMask = true;
        }
        if (!(url->_flags & currentOption)) continue;
        if (!haveReachedMask) {
            beforeMask = currRange->location + currRange->length;
        } else if (currentOption <= mask) {
            if (maskRange.location == kCFNotFound) {
                maskRange = *currRange;
            } else {
                maskRange.length = currRange->location + currRange->length - maskRange.location;
            }
        } else {
            afterMask = currRange->location;
            break;
        }
        currRange ++;
    }
    if (afterMask == kCFNotFound) {
        afterMask = maskRange.location + maskRange.length;
    }
    charRangeWithSeparators->location = beforeMask;
    charRangeWithSeparators->length = afterMask - beforeMask;
    return maskRange;
}

static CFRange _getCharRangeInDecomposableURL(CFURLRef url, CFURLComponentType component, CFRange *rangeIncludingSeparators) {
    CFOptionFlags mask;
    switch (component) {
        case kCFURLComponentScheme: 
            mask = HAS_SCHEME; 
            break;
        case kCFURLComponentNetLocation: 
            mask = NET_LOCATION_MASK; 
            break;
        case kCFURLComponentPath: 
            mask = HAS_PATH; 
            break;
        case kCFURLComponentResourceSpecifier: 
            mask = RESOURCE_SPECIFIER_MASK; 
            break;
        case kCFURLComponentUser: 
            mask = HAS_USER; 
            break;
        case kCFURLComponentPassword:
            mask = HAS_PASSWORD;
            break;
        case kCFURLComponentUserInfo:
            mask = HAS_USER | HAS_PASSWORD;
            break;
        case kCFURLComponentHost:
            mask = HAS_HOST;
            break;
        case kCFURLComponentPort:
            mask = HAS_PORT;
            break;
        case kCFURLComponentParameterString:
            mask = HAS_PARAMETERS;
            break;
        case kCFURLComponentQuery:
            mask = HAS_QUERY;
            break;
        case kCFURLComponentFragment:
            mask = HAS_FRAGMENT;
            break;
        default:
            rangeIncludingSeparators->location = kCFNotFound;
            rangeIncludingSeparators->length = 0;
            return CFRangeMake(kCFNotFound, 0);
    }

    if ((url->_flags & mask) == 0) {
        rangeIncludingSeparators->location = insertionLocationForMask(url, mask);
        rangeIncludingSeparators->length = 0;
        return CFRangeMake(kCFNotFound, 0);
    } else {
        return _CFURLGetCharRangeForMask(url, mask, rangeIncludingSeparators);
    }
}

static CFRange _getCharRangeInNonDecomposableURL(CFURLRef url, CFURLComponentType component, CFRange *rangeIncludingSeparators) {
    if (component == kCFURLComponentScheme) {
        CFRange schemeRg = _rangeForComponent(url->_flags, url->_ranges, HAS_SCHEME);
        rangeIncludingSeparators->location = 0;
        rangeIncludingSeparators->length = schemeRg.length + 1;
        return schemeRg;
    } else if (component == kCFURLComponentResourceSpecifier) {
        CFRange schemeRg = _rangeForComponent(url->_flags, url->_ranges, HAS_SCHEME);
        CFIndex stringLength = CFStringGetLength(url->_string);
        if (schemeRg.length + 1 == stringLength) {
            rangeIncludingSeparators->location = schemeRg.length + 1;
            rangeIncludingSeparators->length = 0;
            return CFRangeMake(kCFNotFound, 0);
        } else {
            rangeIncludingSeparators->location = schemeRg.length;
            rangeIncludingSeparators->length = stringLength - schemeRg.length;
            return CFRangeMake(schemeRg.length + 1, rangeIncludingSeparators->length - 1);
        }
    } else {
        rangeIncludingSeparators->location = kCFNotFound;
        rangeIncludingSeparators->length = 0;
        return CFRangeMake(kCFNotFound, 0);
    }
    
}

CFRange CFURLGetByteRangeForComponent(CFURLRef url, CFURLComponentType component, CFRange *rangeIncludingSeparators) {
    CFRange charRange, charRangeWithSeparators;
    CFRange byteRange;
    CFAssert2(component > 0 && component < 13, __kCFLogAssertion, "%s(): passed invalid component %d", __PRETTY_FUNCTION__, component);
    url = _CFURLFromNSURL(url);

    if (!(url->_flags & IS_DECOMPOSABLE)) {
        // Special-case this because non-decomposable URLs have a slightly strange flags setup
        charRange = _getCharRangeInNonDecomposableURL(url, component, &charRangeWithSeparators);
    } else {
        charRange = _getCharRangeInDecomposableURL(url, component, &charRangeWithSeparators);
    }
    
    if (charRangeWithSeparators.location == kCFNotFound) {
        if (rangeIncludingSeparators) {
            rangeIncludingSeparators->location = kCFNotFound;
            rangeIncludingSeparators->length = 0;
        }
        return CFRangeMake(kCFNotFound, 0);
    } else if (rangeIncludingSeparators) {
        CFStringGetBytes(url->_string, CFRangeMake(0, charRangeWithSeparators.location), url->_encoding, 0, false, NULL, 0, &(rangeIncludingSeparators->location));

        if (charRange.location == kCFNotFound) {
            byteRange = charRange;
            CFStringGetBytes(url->_string, charRangeWithSeparators, url->_encoding, 0, false, NULL, 0, &(rangeIncludingSeparators->length));
        } else {
            CFIndex maxCharRange = charRange.location + charRange.length;
            CFIndex maxCharRangeWithSeparators = charRangeWithSeparators.location + charRangeWithSeparators.length;

            if (charRangeWithSeparators.location == charRange.location) {
                byteRange.location = rangeIncludingSeparators->location;
            } else {
                CFIndex numBytes;
                CFStringGetBytes(url->_string, CFRangeMake(charRangeWithSeparators.location, charRange.location - charRangeWithSeparators.location), url->_encoding, 0, false, NULL, 0, &numBytes);
                byteRange.location = charRangeWithSeparators.location + numBytes;
            }
            CFStringGetBytes(url->_string, charRange, url->_encoding, 0, false, NULL, 0, &(byteRange.length));
            if (maxCharRangeWithSeparators == maxCharRange) {
                rangeIncludingSeparators->length = byteRange.location + byteRange.length - rangeIncludingSeparators->location;
            } else {
                CFIndex numBytes;
                CFRange rg;
                rg.location = maxCharRange;
                rg.length = maxCharRangeWithSeparators - rg.location;
                CFStringGetBytes(url->_string, rg, url->_encoding, 0, false, NULL, 0, &numBytes);
                rangeIncludingSeparators->length = byteRange.location + byteRange.length + numBytes - rangeIncludingSeparators->location;
            }
        }
    } else if (charRange.location == kCFNotFound) {
        byteRange = charRange;
    } else {
        CFStringGetBytes(url->_string, CFRangeMake(0, charRange.location), url->_encoding, 0, false, NULL, 0, &(byteRange.location));
        CFStringGetBytes(url->_string, charRange, url->_encoding, 0, false, NULL, 0, &(byteRange.length));
    }
    return byteRange;
}

/* Component support */

static Boolean decomposeToNonHierarchical(CFURLRef url, CFURLComponentsNonHierarchical *components) {
    if ( CFURLGetBaseURL(url) != NULL)  {
        components->scheme = NULL;
    } else {
        components->scheme = CFURLCopyScheme(url);
    }
    components->schemeSpecific = CFURLCopyResourceSpecifier(url);
    return true;
}

static CFURLRef composeFromNonHierarchical(CFAllocatorRef alloc, const CFURLComponentsNonHierarchical *components) {
    CFStringRef str;
    if (components->scheme) {
        UniChar ch = ':';
        str = CFStringCreateMutableCopy(alloc, CFStringGetLength(components->scheme) + 1 + (components->schemeSpecific ? CFStringGetLength(components->schemeSpecific): 0), components->scheme);
        CFStringAppendCharacters((CFMutableStringRef)str, &ch, 1);
        if (components->schemeSpecific) CFStringAppend((CFMutableStringRef)str, components->schemeSpecific);
    } else if (components->schemeSpecific) {
        str = components->schemeSpecific;
        CFRetain(str);
    } else {
        str = NULL;
    }
    if (str) {
        CFURLRef url = CFURLCreateWithString(alloc, str, NULL);
        CFRelease(str);
        return url;
    } else {
        return NULL;
    }
}

static Boolean decomposeToRFC1808(CFURLRef url, CFURLComponentsRFC1808 *components) {
    CFAllocatorRef alloc = CFGetAllocator(url);
    static CFStringRef emptyStr = NULL;
    if (!emptyStr) {
        emptyStr = CFSTR("");
    }

    if (!CFURLCanBeDecomposed(url)) {
        return false;
    }
    
    CFStringRef path = CFURLCopyPath(url);
    if (path) {
        components->pathComponents = CFStringCreateArrayBySeparatingStrings(alloc, path, CFSTR("/"));
        CFRelease(path);
    } else {
        components->pathComponents = NULL;
    }
    components->baseURL = CFURLGetBaseURL(url);
    if (components->baseURL)  {
        CFRetain(components->baseURL);
        components->scheme = NULL;
    } else {
        components->scheme = _retainedComponentString(url, HAS_SCHEME, true, false);
    }
    components->user = _retainedComponentString(url, HAS_USER, false, false);
    components->password = _retainedComponentString(url, HAS_PASSWORD, false, false);
    components->host = _retainedComponentString(url, HAS_HOST, false, false);
    if (url->_flags & HAS_PORT) {
        components->port = CFURLGetPortNumber(url);
    } else {
        components->port = kCFNotFound;
    }
    components->parameterString = _retainedComponentString(url, HAS_PARAMETERS, false, false);
    components->query = _retainedComponentString(url, HAS_QUERY, false, false);
    components->fragment = _retainedComponentString(url, HAS_FRAGMENT, false, false);
    return true;
}

static CFURLRef composeFromRFC1808(CFAllocatorRef alloc, const CFURLComponentsRFC1808 *comp) {
    CFMutableStringRef urlString = CFStringCreateMutable(alloc, 0);
    CFURLRef base = comp->baseURL;
    CFURLRef url;
    Boolean hadPrePathComponent = false;
    if (comp->scheme) {
        base = NULL;
        CFStringAppend(urlString, comp->scheme);
        CFStringAppend(urlString, CFSTR("://"));
        hadPrePathComponent = true;
    }
    if (comp->user || comp->password) {
        if (comp->user) {
            CFStringAppend(urlString, comp->user);
        }
        if (comp->password) {
            CFStringAppend(urlString, CFSTR(":"));
            CFStringAppend(urlString, comp->password);
        }
        CFStringAppend(urlString, CFSTR("@"));
        hadPrePathComponent = true;
    }
    if (comp->host) {
        CFStringAppend(urlString, comp->host);
        hadPrePathComponent = true;
    }
    if (comp->port != kCFNotFound) {
        CFStringAppendFormat(urlString, NULL, CFSTR(":%ld"), (long)comp->port);
        hadPrePathComponent = true;
    }

    if (hadPrePathComponent && (comp->pathComponents == NULL || CFArrayGetCount( comp->pathComponents ) == 0 || CFStringGetLength((CFStringRef)CFArrayGetValueAtIndex(comp->pathComponents, 0)) != 0)) {
        CFStringAppend(urlString, CFSTR("/"));
    }
    if (comp->pathComponents) {
        CFStringRef pathStr = CFStringCreateByCombiningStrings(alloc, comp->pathComponents, CFSTR("/"));
        CFStringAppend(urlString, pathStr);
        CFRelease(pathStr);
    }
    if (comp->parameterString) {
        CFStringAppend(urlString, CFSTR(";"));
        CFStringAppend(urlString, comp->parameterString);
    }
    if (comp->query) {
        CFStringAppend(urlString, CFSTR("?"));
        CFStringAppend(urlString, comp->query);
    }
    if (comp->fragment) {
        CFStringAppend(urlString, CFSTR("#"));
        CFStringAppend(urlString, comp->fragment);
    }
    url = CFURLCreateWithString(alloc, urlString, base);
    CFRelease(urlString);
    return url;
}

static Boolean decomposeToRFC2396(CFURLRef url, CFURLComponentsRFC2396 *comp) {
    CFAllocatorRef alloc = CFGetAllocator(url);
    CFURLComponentsRFC1808 oldComp;
    CFStringRef tmpStr;
    if (!decomposeToRFC1808(url, &oldComp)) {
        return false;
    }
    comp->scheme = oldComp.scheme;
    if (oldComp.user) {
        if (oldComp.password) {
            comp->userinfo = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@:%@"), oldComp.user, oldComp.password);
            CFRelease(oldComp.password);
            CFRelease(oldComp.user);
        } else {
            comp->userinfo = oldComp.user;
        }
    } else {
        comp->userinfo = NULL;
    }
    comp->host = oldComp.host;
    comp->port = oldComp.port;
    if (!oldComp.parameterString) {
        comp->pathComponents = oldComp.pathComponents;
    } else {
        int length = CFArrayGetCount(oldComp.pathComponents);
        comp->pathComponents = CFArrayCreateMutableCopy(alloc, length, oldComp.pathComponents);
        tmpStr = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@;%@"), CFArrayGetValueAtIndex(comp->pathComponents, length - 1), oldComp.parameterString);
        CFArraySetValueAtIndex((CFMutableArrayRef)comp->pathComponents, length - 1, tmpStr);
        CFRelease(tmpStr);
        CFRelease(oldComp.pathComponents);
        CFRelease(oldComp.parameterString);
    }
    comp->query = oldComp.query;
    comp->fragment = oldComp.fragment;
    comp->baseURL = oldComp.baseURL;
    return true;
}

static CFURLRef composeFromRFC2396(CFAllocatorRef alloc, const CFURLComponentsRFC2396 *comp) {
    CFMutableStringRef urlString = CFStringCreateMutable(alloc, 0);
    CFURLRef base = comp->baseURL;
    CFURLRef url;
    Boolean hadPrePathComponent = false;
    if (comp->scheme) {
        base = NULL;
        CFStringAppend(urlString, comp->scheme);
        CFStringAppend(urlString, CFSTR("://"));
        hadPrePathComponent = true;
    }
    if (comp->userinfo) {
        CFStringAppend(urlString, comp->userinfo);
        CFStringAppend(urlString, CFSTR("@"));
        hadPrePathComponent = true;
    }
    if (comp->host) {
        CFStringAppend(urlString, comp->host);
        if (comp->port != kCFNotFound) {
            CFStringAppendFormat(urlString, NULL, CFSTR(":%ld"), (long)comp->port);
        }
        hadPrePathComponent = true;
    }
    if (hadPrePathComponent && (comp->pathComponents == NULL || CFStringGetLength((CFStringRef)CFArrayGetValueAtIndex(comp->pathComponents, 0)) != 0)) {
        CFStringAppend(urlString, CFSTR("/"));
    }
    if (comp->pathComponents) {
        CFStringRef pathStr = CFStringCreateByCombiningStrings(alloc, comp->pathComponents, CFSTR("/"));
        CFStringAppend(urlString, pathStr);
        CFRelease(pathStr);
    }
    if (comp->query) {
        CFStringAppend(urlString, CFSTR("?"));
        CFStringAppend(urlString, comp->query);
    }
    if (comp->fragment) {
        CFStringAppend(urlString, CFSTR("#"));
        CFStringAppend(urlString, comp->fragment);
    }
    url = CFURLCreateWithString(alloc, urlString, base);
    CFRelease(urlString);
    return url;
}

#undef CFURLCopyComponents
#undef CFURLCreateFromComponents

CF_EXPORT
Boolean _CFURLCopyComponents(CFURLRef url, CFURLComponentDecomposition decompositionType, void *components) {
    url = _CFURLFromNSURL(url);
    switch (decompositionType) {
    case kCFURLComponentDecompositionNonHierarchical:
        return decomposeToNonHierarchical(url, (CFURLComponentsNonHierarchical *)components);
    case kCFURLComponentDecompositionRFC1808:
        return decomposeToRFC1808(url, (CFURLComponentsRFC1808 *)components);
    case kCFURLComponentDecompositionRFC2396:
        return decomposeToRFC2396(url, (CFURLComponentsRFC2396 *)components);
    default:
        return false;
    }
}

CF_EXPORT
CFURLRef _CFURLCreateFromComponents(CFAllocatorRef alloc, CFURLComponentDecomposition decompositionType, const void *components) {
    switch (decompositionType) {
    case kCFURLComponentDecompositionNonHierarchical:
        return composeFromNonHierarchical(alloc, (const CFURLComponentsNonHierarchical *)components);
    case kCFURLComponentDecompositionRFC1808:
        return composeFromRFC1808(alloc, (const CFURLComponentsRFC1808 *)components);
    case kCFURLComponentDecompositionRFC2396:
        return composeFromRFC2396(alloc, (const CFURLComponentsRFC2396 *)components);
    default:
        return NULL;
    }
}

CF_EXPORT void *__CFURLReservedPtr(CFURLRef  url) {
    // called with CFURL (not NSURL) from Foundation
    return _getReserved(url);
}

CF_EXPORT void __CFURLSetReservedPtr(CFURLRef  url, void *ptr) {
    // called with CFURL (not NSURL) from Foundation
    _setReserved ( (struct __CFURL*) url, ptr );
}

CF_EXPORT void *__CFURLResourceInfoPtr(CFURLRef url) {
    if ( url ) {
        url = _CFURLFromNSURL(url);
        return _getResourceInfo(url);
    }
    else {
        return NULL;
    }
}

CF_EXPORT void __CFURLSetResourceInfoPtr(CFURLRef url, void *ptr) {
    if ( url ) {
        url = _CFURLFromNSURL(url);
        _setResourceInfo ( (struct __CFURL*) url, ptr );
    }
}

/* File system stuff */

/* HFSPath<->URLPath functions at the bottom of the file */
static CFArrayRef WindowsPathToURLComponents(CFStringRef path, CFAllocatorRef alloc, Boolean isDir, Boolean isAbsolute) {
    CFArrayRef tmp;
    CFMutableArrayRef urlComponents = NULL;
    CFIndex i=0;

    tmp = CFStringCreateArrayBySeparatingStrings(alloc, path, CFSTR("\\"));
    urlComponents = CFArrayCreateMutableCopy(alloc, 0, tmp);
    CFRelease(tmp);

    CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(urlComponents, 0);
    if (isAbsolute && CFStringGetLength(str) == 2 && CFStringGetCharacterAtIndex(str, 1) == ':') {
        i = 1; // Skip over the drive letter 
    }
    CFIndex c;
    for (c = CFArrayGetCount(urlComponents); i < c; i ++) {
        CFStringRef fileComp = (CFStringRef)CFArrayGetValueAtIndex(urlComponents,i);
        CFStringRef urlComp = _replacePathIllegalCharacters(fileComp, alloc, false);
        if (!urlComp) {
            // Couldn't decode fileComp
            CFRelease(urlComponents);
            return NULL;
        }
        if (urlComp != fileComp) {
            CFArraySetValueAtIndex(urlComponents, i, urlComp);
        }
        CFRelease(urlComp);
    }

    if (isDir) {
        if (CFStringGetLength((CFStringRef)CFArrayGetValueAtIndex(urlComponents, CFArrayGetCount(urlComponents) - 1)) != 0)
            CFArrayAppendValue(urlComponents, CFSTR(""));
    }
    if (isAbsolute) {
        if ( AddAuthorityToFileURL() ) {
            CFArrayInsertValueAtIndex(urlComponents, 0, CFSTR(FILE_PREFIX_WITH_AUTHORITY));
        }
        else {
            CFArrayInsertValueAtIndex(urlComponents, 0, CFSTR(FILE_PREFIX));
        }
    }
    return urlComponents;
}

static CFStringRef WindowsPathToURLPath(CFStringRef path, CFAllocatorRef alloc, Boolean isDir, Boolean isAbsolute) {
    CFArrayRef urlComponents;
    CFStringRef str;

    if (CFStringGetLength(path) == 0) return CFStringCreateWithCString(alloc, "", kCFStringEncodingASCII);
    urlComponents = WindowsPathToURLComponents(path, alloc, isDir, isAbsolute);
    if (!urlComponents) return CFStringCreateWithCString(alloc, "", kCFStringEncodingASCII);

    // WindowsPathToURLComponents already added percent escapes for us; no need to add them again here.
    str = CFStringCreateByCombiningStrings(alloc, urlComponents, CFSTR("/"));
    CFRelease(urlComponents);
    return str;
}

static CFStringRef POSIXPathToURLPath(CFStringRef path, CFAllocatorRef alloc, Boolean isDirectory, Boolean isAbsolute, Boolean *posixAndUrlPathsMatch) {
    Boolean addedPercentEncoding;
    CFStringRef pathString = NULL;
    STACK_BUFFER_DECL(char, buffer, PATH_MAX);
    if ( CFStringGetFileSystemRepresentation(path, buffer, PATH_MAX) ) {
        pathString = CreateStringFromFileSystemRepresentationByAddingPercentEscapes(kCFAllocatorDefault, (const UInt8 *)buffer, strlen(buffer), isDirectory, isAbsolute, false /* windowsPath */, &addedPercentEncoding);
    }
    
    if ( posixAndUrlPathsMatch ) {
        *posixAndUrlPathsMatch = !addedPercentEncoding;
    }
    return pathString;
}

static CFStringRef URLPathToPOSIXPath(CFStringRef path, CFAllocatorRef allocator, CFStringEncoding encoding) {
    // This is the easiest case; just remove the percent escape codes and we're done
    CFStringRef result = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(allocator, path, CFSTR(""), encoding);
    if (result) {
        CFIndex length = CFStringGetLength(result);
        if (length > 1 && CFStringGetCharacterAtIndex(result, length-1) == '/') {
            CFStringRef tmp = CFStringCreateWithSubstring(allocator, result, CFRangeMake(0, length-1));
            CFRelease(result);
            result = tmp;
        }
    }
    return result;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
static Boolean CanonicalFileURLStringToFileSystemRepresentation(CFStringRef str, UInt8 *inBuffer, CFIndex inBufferLen)
{
    size_t fileURLPrefixLength;
    if ( AddAuthorityToFileURL() ) {
        fileURLPrefixLength = sizeof(fileURLPrefixWithAuthority);
    }
    else {
        fileURLPrefixLength = sizeof(fileURLPrefix);
    }
    Boolean result;
    if ( inBuffer && inBufferLen ) {
        STACK_BUFFER_DECL(UInt8, stackEscapedBuf, PATH_MAX * 3);    // worst case size is every unicode code point could be a 3-byte UTF8 sequence
        UInt8 *escapedBuf;
        CFIndex strLength = CFStringGetLength(str) - (fileURLPrefixLength - 1);
        if ( strLength != 0 ) {
            CFIndex maxBufLength = strLength * 3;
            CFIndex usedBufLen;
            CFIndex charsConverted;
            if ( strLength <= PATH_MAX ) {
                escapedBuf = &stackEscapedBuf[0];
            }
            else {
                // worst case size is every unicode code point could be a 3-byte UTF8 sequence
                escapedBuf = (UInt8 *)malloc(maxBufLength);
            }
            if ( escapedBuf != NULL ) {
                charsConverted = CFStringGetBytes(str, CFRangeMake(fileURLPrefixLength - 1, strLength), kCFStringEncodingUTF8, 0, false, escapedBuf, maxBufLength, &usedBufLen);
                if ( charsConverted ) {
                    static const UInt8 hexvalues[] = {
                        /* 00 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 08 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 10 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 18 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 20 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 28 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 30 */  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                        /* 38 */  0x8, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 40 */  0x0, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0,
                        /* 48 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 50 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 58 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 60 */  0x0, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0,
                        /* 68 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 70 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 78 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        
                        /* 80 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 88 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 90 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* 98 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* A0 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* A8 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* B0 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* B8 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* C0 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* C8 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* D0 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* D8 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* E0 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* E8 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* F0 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        /* F8 */  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                    };
                    UInt8 *bufStartPtr;
                    UInt8 *bufEndPtr;
                    UInt8 *bufPtr;
                    const UInt8 *bytePtr = escapedBuf;
                    CFIndex idx;
                    Boolean trailingSlash = false;
                    
                    bufPtr = bufStartPtr = inBuffer;
                    bufEndPtr = inBuffer + inBufferLen;
                    result = TRUE;
                    
                    for ( idx = 0; (idx < usedBufLen) && result; ++idx ) {
                        if ( bufPtr == bufEndPtr ) {
                            // ooops, ran out of inBuffer
                            *bufStartPtr = '\0';
                            result = FALSE;
                        }
                        else {
                            switch ( *bytePtr ) {
                                case '%':
                                    idx += 2;
                                    if ( idx < usedBufLen ) {
                                        // skip over %
                                        bytePtr++;
                                        // convert hex digits
                                        *bufPtr = hexvalues[*bytePtr++] << 4;
                                        *bufPtr += hexvalues[*bytePtr++];
                                        trailingSlash = (*bufPtr == '/');
                                    }
                                    break;
                                default:
                                    // copy everything else
                                    *bufPtr = *bytePtr++;
                                    trailingSlash = (*bufPtr == '/');
                                    break;
                            }
                            ++bufPtr;
                        }
                    }
                    if ( result ) {
                        // remove trailing slash (if any)
                        if ( (bufPtr > (bufStartPtr + 1)) && trailingSlash ) {
                            --bufPtr;
                        }
                        if ( bufPtr < bufEndPtr ) {
                            *bufPtr = '\0';
                        }
                    }
                }
                else {
                    // CFStringGetBytes failed
                    result = FALSE;
                }
                
                // free the buffer if we malloc'd it
                if ( escapedBuf != &stackEscapedBuf[0] ) {
                    free(escapedBuf);
                }
            }
            else {
                // could not allocate escapedBuf
                result = FALSE;
            }
        }
        else {
            // str was zero characters
            *inBuffer = '\0';
            result = TRUE;
        }
    }
    else {
        // no inBuffer or inBufferLen is zero
        result = FALSE;
    }
    
    return ( result );
}
#endif

#if DEPLOYMENT_TARGET_WINDOWS
// From CFPlatform.c
extern CFStringRef CFCreateWindowsDrivePathFromVolumeName(CFStringRef volNameStr);
#endif

static CFStringRef URLPathToWindowsPath(CFStringRef path, CFAllocatorRef allocator, CFStringEncoding encoding) {
    // Check for a drive letter, then flip all the slashes
    CFStringRef result;
    CFArrayRef tmp = CFStringCreateArrayBySeparatingStrings(allocator, path, CFSTR("/"));
    SInt32 count = CFArrayGetCount(tmp);
    CFMutableArrayRef components = CFArrayCreateMutableCopy(allocator, count, tmp);
    CFStringRef newPath;
    
    
    
    CFRelease(tmp);
    if (CFStringGetLength((CFStringRef)CFArrayGetValueAtIndex(components,count-1)) == 0) {
        CFArrayRemoveValueAtIndex(components, count-1);
        count --;
    }
    
    if (count > 1 && CFStringGetLength((CFStringRef)CFArrayGetValueAtIndex(components, 0)) == 0) {
        // Absolute path; we need to check for a drive letter in the second component, and if so, remove the first component
        CFStringRef firstComponent = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(allocator, (CFStringRef)CFArrayGetValueAtIndex(components, 1), CFSTR(""), encoding);
        UniChar ch;

	{
            if (firstComponent) {
		if (CFStringGetLength(firstComponent) == 2 && ((ch = CFStringGetCharacterAtIndex(firstComponent, 1)) == '|' || ch == ':')) {
		    // Drive letter
		    CFArrayRemoveValueAtIndex(components, 0);
		    if (ch == '|') {
			CFStringRef driveStr = CFStringCreateWithFormat(allocator, NULL, CFSTR("%c:"), CFStringGetCharacterAtIndex(firstComponent, 0));
			CFArraySetValueAtIndex(components, 0, driveStr);
			CFRelease(driveStr);
		    }
		}
#if DEPLOYMENT_TARGET_WINDOWS
		else {
		    // From <rdar://problem/5623405> [DEFECT] CFURL returns a Windows path that contains volume name instead of a drive letter
		    // we need to replace the volume name (it is not valid on Windows) with the drive mounting point path
		    // remove the first component and set the component with the drive letter to be the first component
		    CFStringRef driveRootPath = CFCreateWindowsDrivePathFromVolumeName(firstComponent);
		    
		    if (driveRootPath) {
			// remove trailing slash
			if (CFStringHasSuffix(driveRootPath, CFSTR("\\"))) {
			    CFStringRef newDriveRootPath = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, driveRootPath, CFRangeMake(0, CFStringGetLength(driveRootPath) - 1));
			    CFRelease(driveRootPath);
			    driveRootPath = newDriveRootPath;
			}
			
			// replace the first component of the path with the drive path
			CFArrayRemoveValueAtIndex(components, 0);
			CFArraySetValueAtIndex(components, 0, driveRootPath);
			
			CFRelease(driveRootPath);
		    }
		}
#endif
	    }
        }
        if ( firstComponent ) {
            CFRelease(firstComponent);
        }
    }
    newPath = CFStringCreateByCombiningStrings(allocator, components, CFSTR("\\"));
    CFRelease(components);
    result = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(allocator, newPath, CFSTR(""), encoding);
    CFRelease(newPath);
    return result;
}



// Caller must release the returned string
static CFStringRef _resolveFileSystemPaths(CFStringRef relativePath, CFStringRef basePath, Boolean baseIsDir, CFURLPathStyle fsType, CFAllocatorRef alloc) {
    CFIndex baseLen = CFStringGetLength(basePath);
    CFIndex relLen = CFStringGetLength(relativePath);
    UniChar pathDelimiter = '/';
    UniChar *buf = (UniChar *)CFAllocatorAllocate(alloc, sizeof(UniChar)*(relLen + baseLen + 2), 0);
    CFStringGetCharacters(basePath, CFRangeMake(0, baseLen), buf);
    if (baseIsDir) {
        if (buf[baseLen-1] != pathDelimiter) {
            buf[baseLen] = pathDelimiter;
            baseLen ++;
        }
    } else {
        UniChar *ptr = buf + baseLen - 1;
        while (ptr > buf && *ptr != pathDelimiter) {
            ptr --;
        }
        baseLen = ptr - buf + 1;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    if (fsType == kCFURLHFSPathStyle) {
#pragma GCC diagnostic pop
        // HFS relative paths will begin with a colon, so we must remove the trailing colon from the base path first.
        baseLen --;
    }
    CFStringGetCharacters(relativePath, CFRangeMake(0, relLen), buf + baseLen);
    *(buf + baseLen + relLen) = '\0';
    return _resolvedPath(buf, buf + baseLen + relLen, pathDelimiter, false, true, alloc);
}

CFURLRef _CFURLCreateCurrentDirectoryURL(CFAllocatorRef allocator) {
    CFURLRef url = NULL;
    uint8_t buf[CFMaxPathSize + 1];
    if (_CFGetCurrentDirectory((char *)buf, CFMaxPathLength)) {
        url = CFURLCreateFromFileSystemRepresentation(allocator, buf, strlen((char *)buf), true);
    }
    return url;
}

CFURLRef CFURLCreateWithFileSystemPath(CFAllocatorRef allocator, CFStringRef filePath, CFURLPathStyle fsType, Boolean isDirectory) {
    CFURLRef result;
    
    result = _CFURLCreateWithFileSystemPath(allocator, filePath, fsType, isDirectory, NULL);
    
    return ( result );
}

CF_EXPORT CFURLRef CFURLCreateWithFileSystemPathRelativeToBase(CFAllocatorRef allocator, CFStringRef filePath, CFURLPathStyle fsType, Boolean isDirectory, CFURLRef baseURL) {
    CFURLRef result;
    
    result = _CFURLCreateWithFileSystemPath(allocator, filePath, fsType, isDirectory, baseURL);

    return ( result );
}

static Boolean _pathHasFileIDPrefix( CFStringRef path )
{
    // path is not NULL, path has prefix "/.file/" and has at least one character following the prefix.
#ifdef __CONSTANT_STRINGS__
    static const 
#endif
    CFStringRef fileIDPrefix = CFSTR( "/" FILE_ID_PREFIX "/" );
    return path && CFStringHasPrefix( path, fileIDPrefix ) && CFStringGetLength( path ) > CFStringGetLength( fileIDPrefix );
}


CF_EXPORT CFStringRef CFURLCopyFileSystemPath(CFURLRef anURL, CFURLPathStyle pathStyle) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    CFAssert2(pathStyle == kCFURLPOSIXPathStyle || pathStyle == kCFURLHFSPathStyle || pathStyle == kCFURLWindowsPathStyle, __kCFLogAssertion, "%s(): Encountered unknown path style %d", __PRETTY_FUNCTION__, pathStyle);
#pragma GCC diagnostic pop
    
    CFStringRef result = NULL;
    CFAllocatorRef alloc = CFGetAllocator(anURL);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
    Boolean isCanonicalFileURL = false;
    
    if ( (pathStyle == kCFURLPOSIXPathStyle) && (CFURLGetBaseURL(anURL) == NULL) ) {
        if ( !CF_IS_OBJC(CFURLGetTypeID(), anURL) ) {
            // We can grope the ivars
            isCanonicalFileURL = ((anURL->_flags & IS_CANONICAL_FILE_URL) != 0);
            if ( isCanonicalFileURL ) {
                STACK_BUFFER_DECL(UInt8, buffer, PATH_MAX + 1);
                if ( CanonicalFileURLStringToFileSystemRepresentation(anURL->_string, buffer, PATH_MAX + 1) ) {
                    result = CFStringCreateWithBytes(alloc, buffer, strlen((char *)buffer), kCFStringEncodingUTF8, false);
                }
            }
        }
    }
    if ( ! result ) {
        // fall back to slower way.
        result = CFURLCreateStringWithFileSystemPath(alloc, anURL, pathStyle, false);
    }
#else // !DEPLOYMENT_TARGET_MACOSX
    result = CFURLCreateStringWithFileSystemPath(alloc, anURL, pathStyle, false);
#endif // !DEPLOYMENT_TARGET_MACOSX
    
    return ( result );
}


// There is no matching ObjC method for this functionality; because this function sits on top of the CFURL primitives, it's o.k. not to check for the need to dispatch an ObjC method instead, but this means care must be taken that this function never call anything that will result in dereferencing anURL without first checking for an ObjC dispatch.  -- REW, 10/29/98
CFStringRef CFURLCreateStringWithFileSystemPath(CFAllocatorRef allocator, CFURLRef anURL, CFURLPathStyle fsType, Boolean resolveAgainstBase) {
    CFURLRef base = resolveAgainstBase ? CFURLGetBaseURL(anURL) : NULL;
    CFStringRef basePath = base ? CFURLCreateStringWithFileSystemPath(allocator, base, fsType, false) : NULL;
    CFStringRef relPath = NULL;
    
    if (!CF_IS_OBJC(CFURLGetTypeID(), anURL)) {
        // We can grope the ivars
        if (fsType == kCFURLPOSIXPathStyle) {
            if (anURL->_flags & POSIX_AND_URL_PATHS_MATCH) {
                relPath = _retainedComponentString(anURL, HAS_PATH, true, true);
            }
        }
    }

    if (relPath == NULL) {
        CFStringRef urlPath = CFURLCopyPath(anURL);
        CFStringEncoding enc = anURL->_encoding;
        if (urlPath) {
            switch (fsType) {
                case kCFURLPOSIXPathStyle:
                    relPath = URLPathToPOSIXPath(urlPath, allocator, enc);
                    break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
                case kCFURLHFSPathStyle:
#pragma GCC diagnostic pop
		    relPath = NULL;
                    break;
                case kCFURLWindowsPathStyle:
                    relPath = URLPathToWindowsPath(urlPath, allocator, enc);
                    break;
                default:
                    CFAssert2(true, __kCFLogAssertion, "%s(): Received unknown path type %d", __PRETTY_FUNCTION__, fsType);
            }
            CFRelease(urlPath);
        }            
    }
	
    //	For Tiger, leave this behavior in for all path types.  For Leopard, it would be nice to remove this entirely
    //	and do a linked-on-or-later check so we don't break third parties.
    //	See <rdar://problem/4003028> Converting volume name from POSIX to HFS form fails and
    //	<rdar://problem/4018895> CF needs to back out 4003028 for icky details.
    if ( relPath && CFURLHasDirectoryPath(anURL) && CFStringGetLength(relPath) > 1 && CFStringGetCharacterAtIndex(relPath, CFStringGetLength(relPath)-1) == '/') {
        CFStringRef tmp = CFStringCreateWithSubstring(allocator, relPath, CFRangeMake(0, CFStringGetLength(relPath)-1));
        CFRelease(relPath);
        relPath = tmp;
    }
    
    if ( relPath ) {
        // relPath is not absolute if it is zero length or doesn't start with a slash
        Boolean relPathIsRelative = ((CFStringGetLength(relPath) != 0) ? (CFStringGetCharacterAtIndex(relPath, 0) != '/') : TRUE);
        if ( basePath && relPathIsRelative ) {
            // we have both basePath and relPath, and relPath is not absolute -- resolve them
            CFStringRef result = _resolveFileSystemPaths(relPath, basePath, CFURLHasDirectoryPath(base), fsType, allocator);
            CFRelease(basePath);
            CFRelease(relPath);
            return result;
        }
        else {
            // we only have the relPath or relpath is absolute -- return it
            if ( basePath ) {
                CFRelease(basePath);
            }
            return relPath;
        }
    }
    else if ( basePath ) {
        // we only have the basePath --- return it
        return basePath;
    }
    else {
        // we have nothing to return
        return NULL;
    }
}

Boolean CFURLGetFileSystemRepresentation(CFURLRef url, Boolean resolveAgainstBase, uint8_t *buffer, CFIndex bufLen) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_WINDOWS
    CFAllocatorRef alloc = CFGetAllocator(url);
    CFStringRef path;

    if (!url) return false;
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
    if ( !resolveAgainstBase || (CFURLGetBaseURL(url) == NULL) ) {
        if (!CF_IS_OBJC(CFURLGetTypeID(), url)) {
            // We can grope the ivars
            if ( url->_flags & IS_CANONICAL_FILE_URL ) {
                return CanonicalFileURLStringToFileSystemRepresentation(url->_string, buffer, bufLen);
            }
        }
    }
    // else fall back to slower way.
    path = CFURLCreateStringWithFileSystemPath(alloc, url, kCFURLPOSIXPathStyle, resolveAgainstBase);
    if (path) {
        Boolean convResult = _CFStringGetFileSystemRepresentation(path, buffer, bufLen);
        CFRelease(path);
        return convResult;
    }
#elif DEPLOYMENT_TARGET_WINDOWS
    path = CFURLCreateStringWithFileSystemPath(alloc, url, kCFURLWindowsPathStyle, resolveAgainstBase);
    if (path) {
        CFIndex usedLen;
        CFIndex pathLen = CFStringGetLength(path);
        CFIndex numConverted = CFStringGetBytes(path, CFRangeMake(0, pathLen), CFStringFileSystemEncoding(), 0, true, buffer, bufLen-1, &usedLen); // -1 because we need one byte to zero-terminate.
        CFRelease(path);
        if (numConverted == pathLen) {
            buffer[usedLen] = '\0';
            return true;
        }
    }
#endif
    return false;
}

#if DEPLOYMENT_TARGET_WINDOWS
CF_EXPORT Boolean _CFURLGetWideFileSystemRepresentation(CFURLRef url, Boolean resolveAgainstBase, wchar_t *buffer, CFIndex bufferLength) {
	CFStringRef path = CFURLCreateStringWithFileSystemPath(CFGetAllocator(url), url, kCFURLWindowsPathStyle, resolveAgainstBase);
	CFIndex pathLength, charsConverted, usedBufLen;
	if (!path) return false;
	pathLength = CFStringGetLength(path);
	if (pathLength+1 > bufferLength) {
		CFRelease(path);
		return false;
	}
	charsConverted = CFStringGetBytes(path, CFRangeMake(0, pathLength), kCFStringEncodingUTF16, 0, false, (UInt8 *)buffer, bufferLength*sizeof(wchar_t), &usedBufLen);
//	CFStringGetCharacters(path, CFRangeMake(0, pathLength), (UniChar *)buffer);
	CFRelease(path);
	if (charsConverted != pathLength || usedBufLen%sizeof(wchar_t) != 0) {
		return false;
	} else {
		buffer[usedBufLen/sizeof(wchar_t)] = 0;
//		buffer[pathLength] = 0;
		return true;
	}
}
#endif

CFURLRef CFURLCreateFromFileSystemRepresentation(CFAllocatorRef allocator, const uint8_t *buffer, CFIndex bufLen, Boolean isDirectory) {
    CFURLRef result;
    
    result = _CFURLCreateWithFileSystemRepresentation(allocator, buffer, bufLen, isDirectory, NULL);
    
    return ( result );
}

CF_EXPORT CFURLRef CFURLCreateFromFileSystemRepresentationRelativeToBase(CFAllocatorRef allocator, const uint8_t *buffer, CFIndex bufLen, Boolean isDirectory, CFURLRef baseURL) {
    CFURLRef result;
    
    result = _CFURLCreateWithFileSystemRepresentation(allocator, buffer, bufLen, isDirectory, baseURL);
    
    return ( result );
}


/******************************/
/* Support for path utilities */
/******************************/

// Assumes url is a CFURL (not an Obj-C NSURL)
static CFRange _rangeOfLastPathComponent(CFURLRef url) {
    CFRange pathRg, componentRg;
    
    pathRg = _rangeForComponent(url->_flags, url->_ranges, HAS_PATH);

    if (pathRg.location == kCFNotFound || pathRg.length == 0) {
        // No path
        return pathRg;
    }
    if (CFStringGetCharacterAtIndex(url->_string, pathRg.location + pathRg.length - 1) == '/') {
        pathRg.length --;
        if (pathRg.length == 0) {
            pathRg.length ++;
            return pathRg;
        }
    }
    if (CFStringFindWithOptions(url->_string, CFSTR("/"), pathRg, kCFCompareBackwards, &componentRg)) {
        componentRg.location ++;
        componentRg.length = pathRg.location + pathRg.length - componentRg.location;
    } else {
        componentRg = pathRg;
    }
    return componentRg;
}

CFStringRef CFURLCopyLastPathComponent(CFURLRef url) {
    CFStringRef result;

    if (CF_IS_OBJC(CFURLGetTypeID(), url)) {
        CFStringRef path = CFURLCreateStringWithFileSystemPath(CFGetAllocator(url), url, kCFURLPOSIXPathStyle, false);
        CFIndex length;
        CFRange rg, compRg;
        if (!path) return NULL;
        rg = CFRangeMake(0, CFStringGetLength(path));
	if ( rg.length == 0 ) return path;
        length = rg.length; // Remember this for comparison later
        if (CFStringGetCharacterAtIndex(path, rg.length - 1) == '/' ) {
            rg.length --;
        }
	if ( rg.length == 0 )
	{
	    //	If we have reduced the string to empty, then it's "/", and that's what we return as
	    //	the last path component.
	   return path;
	}
        else if (CFStringFindWithOptions(path, CFSTR("/"), rg, kCFCompareBackwards, &compRg)) {
            rg.length = rg.location + rg.length - (compRg.location+1);
            rg.location = compRg.location + 1;
        }
        if (rg.location == 0 && rg.length == length) {
            result = path;
        } else {
            result = CFStringCreateWithSubstring(CFGetAllocator(url), path, rg);
            CFRelease(path);
        }
    } else {
        Boolean filePathURLCreated = false;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        if ( CFURLIsFileReferenceURL(url) ) {
            // use a file path URL or fail
            CFURLRef filePathURL = CFURLCreateFilePathURL(CFGetAllocator(url), url, NULL);
            if ( filePathURL ) {
                filePathURLCreated = TRUE;
                url = filePathURL;
            }
            else {
                return NULL;
            }
        }
#endif
        
        CFRange rg = _rangeOfLastPathComponent(url);
        if (rg.location == kCFNotFound || rg.length == 0) {
            // No path
            if ( filePathURLCreated ) {
                CFRelease(url);
            }
            return (CFStringRef)CFRetain(CFSTR(""));
        }
        if (rg.length == 1 && CFStringGetCharacterAtIndex(url->_string, rg.location) == '/') {
            if ( filePathURLCreated ) {
                CFRelease(url);
            }
            return (CFStringRef)CFRetain(CFSTR("/"));
        }
        result = CFStringCreateWithSubstring(CFGetAllocator(url), url->_string, rg);
        if (!(url->_flags & POSIX_AND_URL_PATHS_MATCH)) {
            CFStringRef tmp;
            if (url->_encoding == kCFStringEncodingUTF8) {
                tmp = CFURLCreateStringByReplacingPercentEscapes(CFGetAllocator(url), result, CFSTR(""));
            } else {
                tmp = CFURLCreateStringByReplacingPercentEscapesUsingEncoding(CFGetAllocator(url), result, CFSTR(""), url->_encoding);
            }
            CFRelease(result);
            result = tmp;
        }
        if ( filePathURLCreated ) {
            CFRelease(url);
        }
    }
    return result;
}

CFStringRef CFURLCopyPathExtension(CFURLRef url) {
    CFStringRef lastPathComp = CFURLCopyLastPathComponent(url);
    CFStringRef ext = NULL;

    if (lastPathComp) {
        CFRange rg = CFStringFind(lastPathComp, CFSTR("."), kCFCompareBackwards);
        if (rg.location != kCFNotFound) {
            rg.location ++;
            rg.length = CFStringGetLength(lastPathComp) - rg.location;
            if (rg.length > 0) {
                ext = CFStringCreateWithSubstring(CFGetAllocator(url), lastPathComp, rg);
            } else {
                ext = (CFStringRef)CFRetain(CFSTR(""));
            }
        }
        CFRelease(lastPathComp);
    }
    return ext;
}

CFURLRef CFURLCreateCopyAppendingPathComponent(CFAllocatorRef allocator, CFURLRef url, CFStringRef pathComponent, Boolean isDirectory) {
    CFURLRef result;
    url = _CFURLFromNSURL(url);
    __CFGenericValidateType(url, CFURLGetTypeID());
    CFAssert1(pathComponent != NULL, __kCFLogAssertion, "%s(): Cannot be called with a NULL component to append", __PRETTY_FUNCTION__);

    Boolean filePathURLCreated = false;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if ( CFURLIsFileReferenceURL(url) ) {
        // use a file path URL if possible (only because this is appending a path component)
        CFURLRef filePathURL = CFURLCreateFilePathURL(allocator, url, NULL);
        if ( filePathURL ) {
            filePathURLCreated = TRUE;
            url = filePathURL;
        }
    }
#endif

    CFMutableStringRef newString;
    CFStringRef newComp;
    CFRange pathRg;
    if (!(url->_flags & HAS_PATH)) {
        result = NULL;
    }
    else {
        newString = CFStringCreateMutableCopy(allocator, 0, url->_string);
        newComp = CFURLCreateStringByAddingPercentEscapes(allocator, pathComponent, NULL, CFSTR(";?"), url->_encoding);
        pathRg = _rangeForComponent(url->_flags, url->_ranges, HAS_PATH);
        if ( (!pathRg.length || CFStringGetCharacterAtIndex(url->_string, pathRg.location + pathRg.length - 1) != '/') && (CFStringGetCharacterAtIndex(newComp, 0) != '/') ) {
            CFStringInsert(newString, pathRg.location + pathRg.length, CFSTR("/"));
            pathRg.length ++;
        }
        CFStringInsert(newString, pathRg.location + pathRg.length, newComp);
        if (isDirectory) {
            CFStringInsert(newString, pathRg.location + pathRg.length + CFStringGetLength(newComp), CFSTR("/"));
        }
        CFRelease(newComp);
        result = _CFURLCreateWithArbitraryString(allocator, newString, url->_base);
        CFRelease(newString);
    }
    if ( filePathURLCreated ) {
        CFRelease(url);
    }
    return result;
}

CFURLRef CFURLCreateCopyDeletingLastPathComponent(CFAllocatorRef allocator, CFURLRef url) {
    CFURLRef result;
    CFMutableStringRef newString;
    CFRange lastCompRg, pathRg;
    Boolean appendDotDot = false;

    url = _CFURLFromNSURL(url);
    CFAssert1(url != NULL, __kCFLogAssertion, "%s(): NULL argument not allowed", __PRETTY_FUNCTION__);
    __CFGenericValidateType(url, CFURLGetTypeID());

    Boolean filePathURLCreated = false;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if ( CFURLIsFileReferenceURL(url) ) {
        // use a file path URL or fail
        CFURLRef filePathURL = CFURLCreateFilePathURL(allocator, url, NULL);
        if ( filePathURL ) {
            filePathURLCreated = TRUE;
            url = filePathURL;
        }
        else {
            return NULL;
        }
    }
#endif
    
    if (!(url->_flags & HAS_PATH)) {
        if ( filePathURLCreated ) {
            CFRelease(url);
        }
        return NULL;
    }
    pathRg = _rangeForComponent(url->_flags, url->_ranges, HAS_PATH);
    lastCompRg = _rangeOfLastPathComponent(url);
    if (lastCompRg.length == 0) {
        appendDotDot = true;
    } else if (lastCompRg.length == 1) {
        UniChar ch = CFStringGetCharacterAtIndex(url->_string, lastCompRg.location);
        if (ch == '.' || ch == '/') {
            appendDotDot = true;
        }
    } else if (lastCompRg.length == 2 && CFStringGetCharacterAtIndex(url->_string, lastCompRg.location) == '.' && CFStringGetCharacterAtIndex(url->_string, lastCompRg.location+1) == '.') {
        appendDotDot = true;
    }

    newString = CFStringCreateMutableCopy(allocator, 0, url->_string);
    if (appendDotDot) {
        CFIndex delta = 0;
        if (pathRg.length > 0 && CFStringGetCharacterAtIndex(url->_string, pathRg.location + pathRg.length - 1) != '/') {
            CFStringInsert(newString, pathRg.location + pathRg.length, CFSTR("/"));
            delta ++;
        }
        CFStringInsert(newString, pathRg.location + pathRg.length + delta, CFSTR(".."));
        delta += 2;
        CFStringInsert(newString, pathRg.location + pathRg.length + delta, CFSTR("/"));
        delta ++;
        // We know we have "/../" at the end of the path; we wish to know if that's immediately preceded by "/." (but that "/." doesn't start the string), in which case we want to delete the "/.".
        if (pathRg.length + delta > 4 && CFStringGetCharacterAtIndex(newString, pathRg.location + pathRg.length + delta - 5) == '.') {
            if (pathRg.length+delta > 7 && CFStringGetCharacterAtIndex(newString, pathRg.location + pathRg.length + delta - 6) == '/') {
                CFStringDelete(newString, CFRangeMake(pathRg.location + pathRg.length + delta - 6, 2));
            } else if (pathRg.length+delta == 5) {
                CFStringDelete(newString, CFRangeMake(pathRg.location + pathRg.length + delta - 5, 2));
            }
        }
    } else if (lastCompRg.location == pathRg.location) {
        CFStringReplace(newString, pathRg, CFSTR("."));
        CFStringInsert(newString, 1, CFSTR("/"));
    } else {
        CFStringDelete(newString, CFRangeMake(lastCompRg.location, pathRg.location + pathRg.length - lastCompRg.location));
    }
    result = _CFURLCreateWithArbitraryString(allocator, newString, url->_base);
    CFRelease(newString);
    if ( filePathURLCreated ) {
        CFRelease(url);
    }
    return result;
}

CFURLRef CFURLCreateCopyAppendingPathExtension(CFAllocatorRef allocator, CFURLRef url, CFStringRef extension) {
    CFMutableStringRef newString;
    CFURLRef result;
    CFRange rg;
    
    CFAssert1(url != NULL && extension != NULL, __kCFLogAssertion, "%s(): NULL argument not allowed", __PRETTY_FUNCTION__);
    url = _CFURLFromNSURL(url);
    __CFGenericValidateType(url, CFURLGetTypeID());
    __CFGenericValidateType(extension, CFStringGetTypeID());

    Boolean filePathURLCreated = false;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if ( CFURLIsFileReferenceURL(url) ) {
        // use a file path URL or fail
        CFURLRef filePathURL = CFURLCreateFilePathURL(allocator, url, NULL);
        if ( filePathURL ) {
            filePathURLCreated = TRUE;
            url = filePathURL;
        }
        else {
            return NULL;
        }
    }
#endif
    
    rg = _rangeOfLastPathComponent(url);
    if (rg.location < 0) {
        if ( filePathURLCreated ) {
            CFRelease(url);
        }
        return NULL; // No path
    }

    newString = CFStringCreateMutableCopy(allocator, 0, url->_string);
    CFStringInsert(newString, rg.location + rg.length, CFSTR("."));
    CFStringRef newExt = CFURLCreateStringByAddingPercentEscapes(allocator, extension, NULL, CFSTR(";?/"), url->_encoding);
    CFStringInsert(newString, rg.location + rg.length + 1, newExt);
    CFRelease(newExt);
    result =  _CFURLCreateWithArbitraryString(allocator, newString, url->_base);
    CFRelease(newString);
    if ( filePathURLCreated ) {
        CFRelease(url);
    }
    return result;
}

CFURLRef CFURLCreateCopyDeletingPathExtension(CFAllocatorRef allocator, CFURLRef url) {
    CFRange rg, dotRg;
    CFURLRef result;

    CFAssert1(url != NULL, __kCFLogAssertion, "%s(): NULL argument not allowed", __PRETTY_FUNCTION__);
    url = _CFURLFromNSURL(url);
    __CFGenericValidateType(url, CFURLGetTypeID());
    
    Boolean filePathURLCreated = false;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if ( CFURLIsFileReferenceURL(url) ) {
        // use a file path URL or fail
        CFURLRef filePathURL = CFURLCreateFilePathURL(allocator, url, NULL);
        if ( filePathURL ) {
            filePathURLCreated = TRUE;
            url = filePathURL;
        }
        else {
            return NULL;
        }
    }
#endif
    
    rg = _rangeOfLastPathComponent(url);
    if (rg.location < 0) {
        result = NULL;
    } else if (rg.length && CFStringFindWithOptions(url->_string, CFSTR("."), rg, kCFCompareBackwards, &dotRg)) {
        CFMutableStringRef newString = CFStringCreateMutableCopy(allocator, 0, url->_string);
        dotRg.length = rg.location + rg.length - dotRg.location;
        CFStringDelete(newString, dotRg);
        result = _CFURLCreateWithArbitraryString(allocator, newString, url->_base);
        CFRelease(newString);
    } else {
        result = (CFURLRef)CFRetain(url);
    }
    if ( filePathURLCreated ) {
        CFRelease(url);
    }
    return result;
}



// keys and vals must have space for at least 4 key/value pairs.  No argument can be NULL.
// Caller must release values, but not keys
static void __CFURLCopyPropertyListKeysAndValues(CFURLRef url, CFTypeRef *keys, CFTypeRef *vals, CFIndex *count) {
    CFAllocatorRef alloc = CFGetAllocator(url);
    CFURLRef base = CFURLGetBaseURL(url);
    keys[0] = CFSTR("_CFURLStringType");
    keys[1] = CFSTR("_CFURLString");
    keys[2] = CFSTR("_CFURLBaseStringType");
    keys[3] = CFSTR("_CFURLBaseURLString");
    if (CF_IS_OBJC(CFURLGetTypeID(), url)) {
        SInt32 urlType = FULL_URL_REPRESENTATION;
        vals[0] = CFNumberCreate(alloc, kCFNumberSInt32Type, &urlType);
        vals[1] = CFURLGetString(url);
    } else {
        SInt32 urlType = FULL_URL_REPRESENTATION;
        vals[0] = CFNumberCreate(alloc, kCFNumberSInt32Type, &urlType);
        if (url->_flags & IS_DIRECTORY) {
            if (CFStringGetCharacterAtIndex(url->_string, CFStringGetLength(url->_string) - 1) == '/') {
                vals[1] = CFRetain(url->_string);
            } else {
                vals[1] = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@%c"), url->_string, '/');
            }
        } else {
            if (CFStringGetCharacterAtIndex(url->_string, CFStringGetLength(url->_string) - 1) != '/') {
                vals[1] = CFRetain(url->_string);
            } else {
                vals[1] = CFStringCreateWithSubstring(alloc, url->_string, CFRangeMake(0, CFStringGetLength(url->_string) - 1));
            }
        }
    }
    if (base != NULL) {
        if (CF_IS_OBJC(CFURLGetTypeID(), base)) {
            SInt32 urlType = FULL_URL_REPRESENTATION;
            vals[2] = CFNumberCreate(alloc, kCFNumberSInt32Type, &urlType);
            vals[3] = CFURLGetString(base);
        } else {
            SInt32 urlType = FULL_URL_REPRESENTATION;
            vals[2] = CFNumberCreate(alloc, kCFNumberSInt32Type, &urlType);
            if (base->_flags & IS_DIRECTORY) {
                if (CFStringGetCharacterAtIndex(base->_string, CFStringGetLength(base->_string) - 1) == '/') {
                    vals[3] = CFRetain(base->_string);
                } else {
                    vals[3] = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@%c"), base->_string, '/');
                }
            } else {
                if (CFStringGetCharacterAtIndex(base->_string, CFStringGetLength(base->_string) - 1) != '/') {
                    vals[3] = CFRetain(base->_string);
                } else {
                    vals[3] = CFStringCreateWithSubstring(alloc, base->_string, CFRangeMake(0, CFStringGetLength(base->_string) - 1));
                }
            }
        }
        *count = 4;
    } else {
        *count = 2;
    }
}

// Private API for Finder to use
CFPropertyListRef _CFURLCopyPropertyListRepresentation(CFURLRef url) {
    CFTypeRef keys[4], vals[4];
    CFDictionaryRef dict;
    CFIndex count, idx;
    __CFURLCopyPropertyListKeysAndValues(url, keys, vals, &count);
    dict = CFDictionaryCreate(CFGetAllocator(url), keys, vals, count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    for (idx = 0; idx < count; idx ++) {
        CFRelease(vals[idx]);
    }
    return dict;
}

CFURLRef _CFURLCreateFromPropertyListRepresentation(CFAllocatorRef alloc, CFPropertyListRef pListRepresentation) {
    CFStringRef baseString, string;
    CFNumberRef baseTypeNum, urlTypeNum;
    SInt32 baseType, urlType;
    CFURLRef baseURL = NULL, url;
    CFDictionaryRef dict = (CFDictionaryRef)pListRepresentation;

    // Start by getting all the pieces and verifying they're of the correct type.
    if (CFGetTypeID(pListRepresentation) != CFDictionaryGetTypeID()) {
        return NULL;
    }
    string = (CFStringRef)CFDictionaryGetValue(dict, CFSTR("_CFURLString"));
    if (!string || CFGetTypeID(string) != CFStringGetTypeID()) {
        return NULL;
    }
    urlTypeNum = (CFNumberRef)CFDictionaryGetValue(dict, CFSTR("_CFURLStringType"));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    if (!urlTypeNum || CFGetTypeID(urlTypeNum) != CFNumberGetTypeID() || !CFNumberGetValue(urlTypeNum, kCFNumberSInt32Type, &urlType) || (urlType != FULL_URL_REPRESENTATION && urlType != kCFURLPOSIXPathStyle && urlType != kCFURLHFSPathStyle && urlType != kCFURLWindowsPathStyle)) {
#pragma GCC diagnostic pop
        return NULL;
    }
    baseString = (CFStringRef)CFDictionaryGetValue(dict, CFSTR("_CFURLBaseURLString"));
    if (baseString) {
        if (CFGetTypeID(baseString) != CFStringGetTypeID()) {
            return NULL;
        }
        baseTypeNum = (CFNumberRef)CFDictionaryGetValue(dict, CFSTR("_CFURLBaseStringType"));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        if (!baseTypeNum || CFGetTypeID(baseTypeNum) != CFNumberGetTypeID() || !CFNumberGetValue(baseTypeNum, kCFNumberSInt32Type, &baseType) ||
            (baseType != FULL_URL_REPRESENTATION && baseType != kCFURLPOSIXPathStyle && baseType != kCFURLHFSPathStyle && baseType != kCFURLWindowsPathStyle)) {
#pragma GCC diagnostic pop
            return NULL;
        }
        if (baseType == FULL_URL_REPRESENTATION) {
            baseURL = _CFURLCreateWithArbitraryString(alloc, baseString, NULL);
        } else {
            baseURL = CFURLCreateWithFileSystemPathRelativeToBase(alloc, baseString, (CFURLPathStyle)baseType, CFStringGetCharacterAtIndex(baseString, CFStringGetLength(baseString)-1) == '/', NULL);
        }
    }
    if (urlType == FULL_URL_REPRESENTATION) {
        url = _CFURLCreateWithArbitraryString(alloc, string, baseURL);
    } else {
        url = CFURLCreateWithFileSystemPathRelativeToBase(alloc, string, (CFURLPathStyle)urlType, CFStringGetCharacterAtIndex(string, CFStringGetLength(string)-1) == '/', baseURL);
    }
    if (baseURL) CFRelease(baseURL);
    return url;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
Boolean _CFURLIsFileReferenceURL(CFURLRef url)
{
    return ( CFURLIsFileReferenceURL(url) );
}

Boolean CFURLIsFileReferenceURL(CFURLRef url)
{
    // returns TRUE if url is is a file URL whose path starts with a file ID reference
    Boolean result = false;
    CFURLRef baseURL = CFURLGetBaseURL(url);
    if ( baseURL ) {
	result = CFURLIsFileReferenceURL(baseURL);
    }
    else {
        if ( CF_IS_OBJC(CFURLGetTypeID(), url) ) {
            result = (Boolean) CF_OBJC_CALLV((NSURL *)url, isFileReferenceURL);
        }
        else {
            result = ((_getSchemeTypeFromFlags(url->_flags) == kHasFileScheme) && ((url->_flags & PATH_HAS_FILE_ID) != 0));
        }
    }
    return ( result );
}

static Boolean _CFURLHasFileURLScheme(CFURLRef url, Boolean *hasScheme)
{
    Boolean result;
    CFURLRef baseURL = CFURLGetBaseURL(url);
    
    if ( baseURL ) {
	result = _CFURLHasFileURLScheme(baseURL, hasScheme);
    }
    else {
        if ( CF_IS_OBJC(CFURLGetTypeID(), url) || (_getSchemeTypeFromFlags(url->_flags) == kHasUncommonScheme) ) {
            // if it's not a CFURL or the scheme is not a common canonical-form scheme, determine the scheme the slower way.
            CFStringRef scheme = CFURLCopyScheme(url);
            if ( scheme ) {
                if ( scheme == kCFURLFileScheme ) {
                    result = true;
                }
                else {
                    result = CFStringCompare(scheme, kCFURLFileScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo;
                }
                if ( hasScheme ) {
                    *hasScheme = true;
                }
                CFRelease(scheme);
            }
            else {
                if ( hasScheme ) {
                    *hasScheme = false;
                }
                result = false;
            }
        }
        else {
            if ( hasScheme ) {
                *hasScheme = (url->_flags & HAS_SCHEME) != 0;
            }
            result = (_getSchemeTypeFromFlags(url->_flags) == kHasFileScheme);
        }
    }
    return ( result );
}

Boolean _CFURLIsFileURL(CFURLRef url)
{
    Boolean result = _CFURLHasFileURLScheme(url, NULL);
    return ( result );
}

CFURLRef CFURLCreateFilePathURL(CFAllocatorRef alloc, CFURLRef url, CFErrorRef *error)
{
    CFURLRef result = NULL;
    Boolean hasScheme;
    if (!_CFURLHasFileURLScheme(url, &hasScheme)) {
        if ( !hasScheme ) {
            CFLog(kCFLogLevelWarning, CFSTR("CFURLCreateFilePathURL failed because it was passed this URL which has no scheme: %@"), url);
        }
        if ( error ) {
            *error = CFErrorCreate( kCFAllocatorDefault, kCFErrorDomainCocoa, kCFURLReadUnsupportedSchemeError, NULL );
        }
        result = NULL;
    } else {
	// File URL. Form of the path is unknown. Make a new URL.
	CFStringRef newURLString;
	CFStringRef netLoc;
	CFStringRef fsPath;
	CFStringRef rSpec;
        
	if ( CFURLGetBaseURL( url )) {
	    CFURLRef absURL = CFURLCopyAbsoluteURL( url );
	    fsPath = CFURLCreateStringWithFileSystemPath(CFGetAllocator(absURL), absURL, kCFURLPOSIXPathStyle, false);
	    netLoc = CFURLCopyNetLocation( absURL );
	    rSpec = CFURLCopyResourceSpecifier( absURL );
	    CFRelease( absURL );
	} else {
	    fsPath = CFURLCreateStringWithFileSystemPath(CFGetAllocator(url), url, kCFURLPOSIXPathStyle, false);
	    netLoc = CFURLCopyNetLocation( url );
	    rSpec = CFURLCopyResourceSpecifier( url );
	}
	if ( fsPath ) {
	    CFStringRef urlPath = _replacePathIllegalCharacters( fsPath, alloc, true );
            
            CFStringAppendBuffer appendBuffer;
            CFStringInitAppendBuffer(alloc, &appendBuffer);
            CFStringAppendStringToAppendBuffer(&appendBuffer, CFSTR(FILE_PREFIX));
            if ( netLoc ) {
                CFStringAppendStringToAppendBuffer(&appendBuffer, netLoc);
            }
            CFStringAppendStringToAppendBuffer(&appendBuffer, urlPath);
            // if original url had a directory path and the path isn't "/", append a slash
            if ( CFURLHasDirectoryPath(url) && (CFStringCompare(urlPath, CFSTR("/"), 0) != kCFCompareEqualTo) ) {
                UniChar slashUniChar = '/';
                CFStringAppendCharactersToAppendBuffer(&appendBuffer, &slashUniChar, 1);
            }
            if ( rSpec ) {
                CFStringAppendStringToAppendBuffer(&appendBuffer, rSpec);
            }
            newURLString = CFStringCreateMutableWithAppendBuffer(&appendBuffer);
            
	    result = CFURLCreateWithString( alloc, newURLString, NULL );
	    CFRelease( newURLString );
	    CFRelease( urlPath );
	    CFRelease( fsPath );	    
	} else {
	    if ( error ) { 
		// Would be better here to get an underlying error back from CFURLCreateStringWithFileSystemPath
		*error = CFErrorCreate( kCFAllocatorDefault, kCFErrorDomainCocoa, kCFURLNoSuchResourceError, NULL );
	    }
	    result = NULL;
	}
	if ( netLoc ) {
	    CFRelease( netLoc );
	}
	if ( rSpec ) {
	    CFRelease( rSpec );
	}
    }
    return result;
}

#endif


CFURLRef CFURLCreateFileReferenceURL(CFAllocatorRef alloc, CFURLRef url, CFErrorRef *error) { return NULL; }

