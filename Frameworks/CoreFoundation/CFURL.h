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

/*	CFURL.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFURL__)
#define __COREFOUNDATION_CFURL__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFString.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef CF_ENUM(CFIndex, CFURLPathStyle) {
    kCFURLPOSIXPathStyle = 0,
    kCFURLHFSPathStyle CF_ENUM_DEPRECATED(10_0, 10_9, 2_0, 7_0), /* The use of kCFURLHFSPathStyle is deprecated. The Carbon File Manager, which uses HFS style paths, is deprecated. HFS style paths are unreliable because they can arbitrarily refer to multiple volumes if those volumes have identical volume names. You should instead use kCFURLPOSIXPathStyle wherever possible. */
    kCFURLWindowsPathStyle
};
    
typedef const struct CF_BRIDGED_TYPE(NSURL) __CFURL * CFURLRef;

/* CFURLs are composed of two fundamental pieces - their string, and a */
/* (possibly NULL) base URL.  A relative URL is one in which the string */
/* by itself does not fully specify the URL (for instance "myDir/image.tiff"); */
/* an absolute URL is one in which the string does fully specify the URL */
/* ("file://localhost/myDir/image.tiff").  Absolute URLs always have NULL */
/* base URLs; however, it is possible for a URL to have a NULL base, and still */
/* not be absolute.  Such a URL has only a relative string, and cannot be */
/* resolved.  Two CFURLs are considered equal if and only if their strings */
/* are equal and their bases are equal.  In other words, */
/* "file://localhost/myDir/image.tiff" is NOT equal to the URL with relative */
/* string "myDir/image.tiff" and base URL "file://localhost/".  Clients that */
/* need these less strict form of equality should convert all URLs to their */
/* absolute form via CFURLCopyAbsoluteURL(), then compare the absolute forms. */

CF_EXPORT
CFTypeID CFURLGetTypeID(void);

/* encoding will be used both to interpret the bytes of URLBytes, and to */
/* interpret any percent-escapes within the bytes. */
/* Using a string encoding which isn't a superset of ASCII encoding is not */
/* supported because CFURLGetBytes and CFURLGetByteRangeForComponent require */
/* 7-bit ASCII characters to be stored in a single 8-bit byte. */
/* CFStringEncodings which are a superset of ASCII encoding include MacRoman, */
/* WindowsLatin1, ISOLatin1, NextStepLatin, ASCII, and UTF8. */
CF_EXPORT
CFURLRef CFURLCreateWithBytes(CFAllocatorRef allocator, const UInt8 *URLBytes, CFIndex length, CFStringEncoding encoding, CFURLRef baseURL);

/* Escapes any character that is not 7-bit ASCII with the byte-code */
/* for the given encoding.  If escapeWhitespace is true, whitespace */
/* characters (' ', '\t', '\r', '\n') will be escaped also (desirable */
/* if embedding the URL into a larger text stream like HTML) */
CF_EXPORT
CFDataRef CFURLCreateData(CFAllocatorRef allocator, CFURLRef url, CFStringEncoding encoding, Boolean escapeWhitespace);

/* Any escape sequences in URLString will be interpreted via UTF-8. */
CF_EXPORT
CFURLRef CFURLCreateWithString(CFAllocatorRef allocator, CFStringRef URLString, CFURLRef baseURL);

/* Create an absolute URL directly, without requiring the extra step */
/* of calling CFURLCopyAbsoluteURL().  If useCompatibilityMode is  */
/* true, the rules historically used on the web are used to resolve */
/* relativeString against baseURL - these rules are generally listed */
/* in the RFC as optional or alternate interpretations.  Otherwise, */
/* the strict rules from the RFC are used.  The major differences are */
/* that in compatibility mode, we are lenient of the scheme appearing */
/* in relative portion, leading "../" components are removed from the */
/* final URL's path, and if the relative portion contains only */
/* resource specifier pieces (query, parameters, and fragment), then */
/* the last path component of the base URL will not be deleted.  */
/* Using a string encoding which isn't a superset of ASCII encoding is not */
/* supported because CFURLGetBytes and CFURLGetByteRangeForComponent require */
/* 7-bit ASCII characters to be stored in a single 8-bit byte. */
/* CFStringEncodings which are a superset of ASCII encoding include MacRoman, */
/* WindowsLatin1, ISOLatin1, NextStepLatin, ASCII, and UTF8. */
CF_EXPORT
CFURLRef CFURLCreateAbsoluteURLWithBytes(CFAllocatorRef alloc, const UInt8 *relativeURLBytes, CFIndex length, CFStringEncoding encoding, CFURLRef baseURL, Boolean useCompatibilityMode);

/* filePath should be the URL's path expressed as a path of the type */
/* fsType.  If filePath is not absolute, the resulting URL will be */
/* considered relative to the current working directory (evaluated */
/* at creation time).  isDirectory determines whether filePath is */
/* treated as a directory path when resolving against relative path */
/* components */
CF_EXPORT
CFURLRef CFURLCreateWithFileSystemPath(CFAllocatorRef allocator, CFStringRef filePath, CFURLPathStyle pathStyle, Boolean isDirectory);

CF_EXPORT
CFURLRef CFURLCreateFromFileSystemRepresentation(CFAllocatorRef allocator, const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory);

/* The path style of the baseURL must match the path style of the relative */
/* url or the results are undefined.  If the provided filePath looks like an */
/* absolute path ( starting with '/' if pathStyle is kCFURLPosixPathStyle, */
/* not starting with ':' for kCFURLHFSPathStyle, or starting with what looks */
/* like a drive letter and colon for kCFURLWindowsPathStyle ) then the baseURL */
/* is ignored. */
CF_EXPORT
CFURLRef CFURLCreateWithFileSystemPathRelativeToBase(CFAllocatorRef allocator, CFStringRef filePath, CFURLPathStyle pathStyle, Boolean isDirectory, CFURLRef baseURL); 

CF_EXPORT
CFURLRef CFURLCreateFromFileSystemRepresentationRelativeToBase(CFAllocatorRef allocator, const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory, CFURLRef baseURL);
                                                                         
/* Fills buffer with the file system's native representation of */
/* url's path. No more than maxBufLen bytes are written to buffer. */
/* The buffer should be at least the maximum path length for */
/* the file system in question to avoid failures for insufficiently */
/* large buffers.  If resolveAgainstBase is true, the url's relative */
/* portion is resolved against its base before the path is computed. */
/* Returns success or failure. */
CF_EXPORT
Boolean CFURLGetFileSystemRepresentation(CFURLRef url, Boolean resolveAgainstBase, UInt8 *buffer, CFIndex maxBufLen);

/* Creates a new URL by resolving the relative portion of relativeURL against its base. */
CF_EXPORT
CFURLRef CFURLCopyAbsoluteURL(CFURLRef relativeURL);

/* Returns the URL's string. */
CF_EXPORT
CFStringRef CFURLGetString(CFURLRef anURL);

/* Returns the base URL if it exists */
CF_EXPORT
CFURLRef CFURLGetBaseURL(CFURLRef anURL);

/*
All URLs can be broken into two pieces - the scheme (preceding the
first colon) and the resource specifier (following the first colon).
Most URLs are also "standard" URLs conforming to RFC 1808 (available
from www.w3c.org).  This category includes URLs of the file, http,
https, and ftp schemes, to name a few.  Standard URLs start the
resource specifier with two slashes ("//"), and can be broken into
four distinct pieces - the scheme, the net location, the path, and
further resource specifiers (typically an optional parameter, query,
and/or fragment).  The net location appears immediately following
the two slashes and goes up to the next slash; it's format is
scheme-specific, but is usually composed of some or all of a username,
password, host name, and port.  The path is a series of path components
separated by slashes; if the net location is present, the path always
begins with a slash.  Standard URLs can be relative to another URL,
in which case at least the scheme and possibly other pieces as well
come from the base URL (see RFC 1808 for precise details when resolving
a relative URL against its base).  The full URL is therefore

<scheme> "://" <net location> <path, always starting with slash> <add'l resource specifiers>

If a given CFURL can be decomposed (that is, conforms to RFC 1808), you
can ask for each of the four basic pieces (scheme, net location, path,
and resource specifer) separately, as well as for its base URL.  The
basic pieces are returned with any percent escape sequences still in
place (although note that the scheme may not legally include any
percent escapes); this is to allow the caller to distinguish between
percent sequences that may have syntactic meaning if replaced by the
character being escaped (for instance, a '/' in a path component).
Since only the individual schemes know which characters are
syntactically significant, CFURL cannot safely replace any percent
escape sequences.  However, you can use
CFURLCreateStringByReplacingPercentEscapes() to create a new string with
the percent escapes removed; see below.

If a given CFURL can not be decomposed, you can ask for its scheme and its
resource specifier; asking it for its net location or path will return NULL.

To get more refined information about the components of a decomposable
CFURL, you may ask for more specific pieces of the URL, expressed with
the percent escapes removed.  The available functions are CFURLCopyHostName(),
CFURLGetPortNumber() (returns an Int32), CFURLCopyUserName(),
CFURLCopyPassword(), CFURLCopyQuery(), CFURLCopyParameters(), and
CFURLCopyFragment().  Because the parameters, query, and fragment of an
URL may contain scheme-specific syntaxes, these methods take a second
argument, giving a list of characters which should NOT be replaced if
percent escaped.  For instance, the ftp parameter syntax gives simple
key-value pairs as "<key>=<value>;"  Clearly if a key or value includes
either '=' or ';', it must be escaped to avoid corrupting the meaning of
the parameters, so the caller may request the parameter string as

CFStringRef myParams = CFURLCopyParameters(ftpURL, CFSTR("=;%"));

requesting that all percent escape sequences be replaced by the represented
characters, except for escaped '=', '%' or ';' characters.  Pass the empty
string (CFSTR("")) to request that all percent escapes be replaced, or NULL
to request that none be.
*/

/* Returns true if anURL conforms to RFC 1808 */
CF_EXPORT
Boolean CFURLCanBeDecomposed(CFURLRef anURL); 

/* The next several methods leave any percent escape sequences intact */

CF_EXPORT
CFStringRef CFURLCopyScheme(CFURLRef anURL);

/* NULL if CFURLCanBeDecomposed(anURL) is false */
CF_EXPORT
CFStringRef CFURLCopyNetLocation(CFURLRef anURL); 

/* NULL if CFURLCanBeDecomposed(anURL) is false; also does not resolve the URL */
/* against its base.  See also CFURLCopyAbsoluteURL().  Note that, strictly */
/* speaking, any leading '/' is not considered part of the URL's path, although */
/* its presence or absence determines whether the path is absolute. */
/* CFURLCopyPath()'s return value includes any leading slash (giving the path */
/* the normal POSIX appearance); CFURLCopyStrictPath()'s return value omits any */
/* leading slash, and uses isAbsolute to report whether the URL's path is absolute. */

/* CFURLCopyFileSystemPath() returns the URL's path as a file system path for the */
/* given path style.  All percent escape sequences are replaced.  The URL is not */
/* resolved against its base before computing the path. */
CF_EXPORT
CFStringRef CFURLCopyPath(CFURLRef anURL);

CF_EXPORT
CFStringRef CFURLCopyStrictPath(CFURLRef anURL, Boolean *isAbsolute);

CF_EXPORT
CFStringRef CFURLCopyFileSystemPath(CFURLRef anURL, CFURLPathStyle pathStyle);

/* Returns whether anURL's path represents a directory */
/* (true returned) or a simple file (false returned) */
CF_EXPORT
Boolean CFURLHasDirectoryPath(CFURLRef anURL);

/* Any additional resource specifiers after the path.  For URLs */
/* that cannot be decomposed, this is everything except the scheme itself. */
CF_EXPORT
CFStringRef CFURLCopyResourceSpecifier(CFURLRef anURL); 

CF_EXPORT
CFStringRef CFURLCopyHostName(CFURLRef anURL);

CF_EXPORT
SInt32 CFURLGetPortNumber(CFURLRef anURL); /* Returns -1 if no port number is specified */

CF_EXPORT
CFStringRef CFURLCopyUserName(CFURLRef anURL);

CF_EXPORT
CFStringRef CFURLCopyPassword(CFURLRef anURL);

/* These remove all percent escape sequences except those for */
/* characters in charactersToLeaveEscaped.  If charactersToLeaveEscaped */
/* is empty (""), all percent escape sequences are replaced by their */
/* corresponding characters.  If charactersToLeaveEscaped is NULL, */
/* then no escape sequences are removed at all */
CF_EXPORT
CFStringRef CFURLCopyParameterString(CFURLRef anURL, CFStringRef charactersToLeaveEscaped);

CF_EXPORT
CFStringRef CFURLCopyQueryString(CFURLRef anURL, CFStringRef charactersToLeaveEscaped);

CF_EXPORT
CFStringRef CFURLCopyFragment(CFURLRef anURL, CFStringRef charactersToLeaveEscaped);

CF_EXPORT
CFStringRef CFURLCopyLastPathComponent(CFURLRef url);

CF_EXPORT
CFStringRef CFURLCopyPathExtension(CFURLRef url);

/* These functions all treat the base URL of the supplied url as */
/* invariant.  In other words, the URL returned will always have */
/* the same base as the URL supplied as an argument. */

CF_EXPORT
CFURLRef CFURLCreateCopyAppendingPathComponent(CFAllocatorRef allocator, CFURLRef url, CFStringRef pathComponent, Boolean isDirectory);

CF_EXPORT
CFURLRef CFURLCreateCopyDeletingLastPathComponent(CFAllocatorRef allocator, CFURLRef url);

CF_EXPORT
CFURLRef CFURLCreateCopyAppendingPathExtension(CFAllocatorRef allocator, CFURLRef url, CFStringRef extension);

CF_EXPORT
CFURLRef CFURLCreateCopyDeletingPathExtension(CFAllocatorRef allocator, CFURLRef url);

/* Fills buffer with the bytes for url, returning the number of bytes */
/* filled.  If buffer is of insufficient size, returns -1 and no bytes */
/* are placed in buffer.  If buffer is NULL, the needed length is */
/* computed and returned.  The returned bytes are the original bytes */ 
/* from which the URL was created; if the URL was created from a */
/* string, the bytes will be the bytes of the string encoded via UTF-8  */
CF_EXPORT
CFIndex CFURLGetBytes(CFURLRef url, UInt8 *buffer, CFIndex bufferLength);

typedef CF_ENUM(CFIndex, CFURLComponentType) {
	kCFURLComponentScheme = 1,
	kCFURLComponentNetLocation = 2,
	kCFURLComponentPath = 3,
	kCFURLComponentResourceSpecifier = 4,

	kCFURLComponentUser = 5,
	kCFURLComponentPassword = 6,
	kCFURLComponentUserInfo = 7,
	kCFURLComponentHost = 8,
	kCFURLComponentPort = 9,
	kCFURLComponentParameterString = 10,
	kCFURLComponentQuery = 11,
	kCFURLComponentFragment = 12
};
 
/* 
Gets the  range of the requested component in the bytes of url, as
returned by CFURLGetBytes().  This range is only good for use in the
bytes returned by CFURLGetBytes!

If non-NULL, rangeIncludingSeparators gives the range of component
including the sequences that separate component from the previous and
next components.  If there is no previous or next component, that end of
rangeIncludingSeparators will match the range of the component itself.
If url does not contain the given component type, (kCFNotFound, 0) is
returned, and rangeIncludingSeparators is set to the location where the
component would be inserted.  Some examples -

For the URL http://www.apple.com/hotnews/

Component           returned range      rangeIncludingSeparators
scheme              (0, 4)              (0, 7)
net location        (7, 13)             (4, 16)
path                (20, 9)             (20, 9)    
resource specifier  (kCFNotFound, 0)    (29, 0)
user                (kCFNotFound, 0)    (7, 0)
password            (kCFNotFound, 0)    (7, 0)
user info           (kCFNotFound, 0)    (7, 0)
host                (7, 13)             (4, 16)
port                (kCFNotFound, 0)    (20, 0)
parameter           (kCFNotFound, 0)    (29, 0)
query               (kCFNotFound, 0)    (29, 0)
fragment            (kCFNotFound, 0)    (29, 0)


For the URL ./relPath/file.html#fragment

Component           returned range      rangeIncludingSeparators
scheme              (kCFNotFound, 0)    (0, 0)
net location        (kCFNotFound, 0)    (0, 0)
path                (0, 19)             (0, 20)
resource specifier  (20, 8)             (19, 9)
user                (kCFNotFound, 0)    (0, 0)
password            (kCFNotFound, 0)    (0, 0)
user info           (kCFNotFound, 0)    (0, 0)
host                (kCFNotFound, 0)    (0, 0)
port                (kCFNotFound, 0)    (0, 0)
parameter           (kCFNotFound, 0)    (19, 0)
query               (kCFNotFound, 0)    (19, 0)
fragment            (20, 8)             (19, 9)


For the URL scheme://user:pass@host:1/path/path2/file.html;params?query#fragment

Component           returned range      rangeIncludingSeparators
scheme              (0, 6)              (0, 9)
net location        (9, 16)             (6, 19)
path                (25, 21)            (25, 22) 
resource specifier  (47, 21)            (46, 22)
user                (9, 4)              (6, 8)
password            (14, 4)             (13, 6)
user info           (9, 9)              (6, 13)
host                (19, 4)             (18, 6)
port                (24, 1)             (23, 2)
parameter           (47, 6)             (46, 8)
query               (54, 5)             (53, 7)
fragment            (60, 8)             (59, 9)
*/
CF_EXPORT
CFRange CFURLGetByteRangeForComponent(CFURLRef url, CFURLComponentType component, CFRange *rangeIncludingSeparators);

/* Returns a string with any percent escape sequences that do NOT */
/* correspond to characters in charactersToLeaveEscaped with their */
/* equivalent.  Returns NULL on failure (if an invalid percent sequence */
/* is encountered), or the original string (retained) if no characters */
/* need to be replaced. Pass NULL to request that no percent escapes be */
/* replaced, or the empty string (CFSTR("")) to request that all percent */
/* escapes be replaced.  Uses UTF8 to interpret percent escapes. */
CF_EXPORT
CFStringRef CFURLCreateStringByReplacingPercentEscapes(CFAllocatorRef allocator, CFStringRef originalString, CFStringRef charactersToLeaveEscaped);

/* As above, but allows you to specify the encoding to use when interpreting percent escapes */
CF_EXPORT
CFStringRef CFURLCreateStringByReplacingPercentEscapesUsingEncoding(CFAllocatorRef allocator, CFStringRef origString, CFStringRef charsToLeaveEscaped, CFStringEncoding encoding);

/* Creates a copy or originalString, replacing certain characters with */
/* the equivalent percent escape sequence based on the encoding specified. */
/* If the originalString does not need to be modified (no percent escape */
/* sequences are missing), may retain and return originalString. */
/* If you are uncertain of the correct encoding, you should use UTF-8, */
/* which is the encoding designated by RFC 2396 as the correct encoding */
/* for use in URLs.  The characters so escaped are all characters that */
/* are not legal URL characters (based on RFC 2396), plus any characters */
/* in legalURLCharactersToBeEscaped, less any characters in */
/* charactersToLeaveUnescaped.  To simply correct any non-URL characters */
/* in an otherwise correct URL string, do: */

/* newString = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, origString, NULL, NULL, kCFStringEncodingUTF8); */
CF_EXPORT
CFStringRef CFURLCreateStringByAddingPercentEscapes(CFAllocatorRef allocator, CFStringRef originalString, CFStringRef charactersToLeaveUnescaped, CFStringRef legalURLCharactersToBeEscaped, CFStringEncoding encoding);


#if (TARGET_OS_MAC || TARGET_OS_EMBEDDED || TARGET_OS_IPHONE) || CF_BUILDING_CF || NSBUILDINGFOUNDATION
CF_IMPLICIT_BRIDGING_DISABLED

/*
    CFURLIsFileReferenceURL

    Returns whether the URL is a file reference URL.

    Parameters
        url
            The URL specifying the resource.
 */
CF_EXPORT
Boolean CFURLIsFileReferenceURL(CFURLRef url) CF_AVAILABLE(10_9, 7_0);

/*
    CFURLCreateFileReferenceURL
    
    Returns a new file reference URL that refers to the same resource as a specified URL.

    Parameters
        allocator
            The memory allocator for creating the new URL.
        url
            The file URL specifying the resource.
        error
            On output when the result is NULL, the error that occurred. This parameter is optional; if you do not wish the error returned, pass NULL here. The caller is responsible for releasing a valid output error.

    Return Value
        The new file reference URL, or NULL if an error occurs.

    Discussion
        File reference URLs use a URL path syntax that identifies a file system object by reference, not by path. This form of file URL remains valid when the file system path of the URL’s underlying resource changes. An error will occur if the url parameter is not a file URL. File reference URLs cannot be created to file system objects which do not exist or are not reachable. In some areas of the file system hierarchy, file reference URLs cannot be generated to the leaf node of the URL path. A file reference URL's path should never be persistently stored because is not valid across system restarts, and across remounts of volumes -- if you want to create a persistent reference to a file system object, use a bookmark (see CFURLCreateBookmarkData). If this function returns NULL, the optional error is populated. This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
CFURLRef CFURLCreateFileReferenceURL(CFAllocatorRef allocator, CFURLRef url, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);


/*
    CFURLCreateFilePathURL
    
    Returns a new file path URL that refers to the same resource as a specified URL.

    Parameters
        allocator
            The memory allocator for creating the new URL.
        url
            The file URL specifying the resource.
        error
            On output when the result is NULL, the error that occurred. This parameter is optional; if you do not wish the error returned, pass NULL here. The caller is responsible for releasing a valid output error.

    Return Value
        The new file path URL, or NULL if an error occurs.

    Discussion
        File path URLs use a file system style path. An error will occur if the url parameter is not a file URL. A file reference URL's resource must exist and be reachable to be converted to a file path URL. If this function returns NULL, the optional error is populated. This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
CFURLRef CFURLCreateFilePathURL(CFAllocatorRef allocator, CFURLRef url, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);

CF_IMPLICIT_BRIDGING_ENABLED
#endif



#if (TARGET_OS_MAC || TARGET_OS_EMBEDDED || TARGET_OS_IPHONE) || CF_BUILDING_CF || NSBUILDINGFOUNDATION
CF_IMPLICIT_BRIDGING_DISABLED

/* Resource access

    The behavior of resource value caching is slightly different between the NSURL and CFURL API.

    When the NSURL methods which get, set, or use cached resource values are used from the main thread, resource values cached by the URL (except those added as temporary properties) are invalidated the next time the main thread's run loop runs.

    The CFURL functions do not automatically clear any resource values cached by the URL. The client has complete control over the cache lifetime. If you are using CFURL API, you must use CFURLClearResourcePropertyCacheForKey or CFURLClearResourcePropertyCache to clear cached resource values.
 */


/*
    CFURLCopyResourcePropertyForKey
    
    Returns the resource value identified by a given resource key.

    Parameters
        url
            The URL specifying the resource.
        key
            The resource key that identifies the resource property.
        propertyValueTypeRefPtr
            On output when the result is true, the resource value or NULL.
        error
            On output when the result is false, the error that occurred. This parameter is optional; if you do not wish the error returned, pass NULL here. The caller is responsible for releasing a valid output error.

    Return Value
        true if propertyValueTypeRefPtr is successfully populated; false if an error occurs.

    Discussion
        CFURLCopyResourcePropertyForKey first checks if the URL object already caches the resource value. If so, it returns the cached resource value to the caller. If not, then CFURLCopyResourcePropertyForKey synchronously obtains the resource value from the backing store, adds the resource value to the URL object's cache, and returns the resource value to the caller. The type of the resource value varies by resource property (see resource key definitions). If this function returns true and propertyValueTypeRefPtr is populated with NULL, it means the resource property is not available for the specified resource and no errors occurred when determining the resource property was not available. If this function returns false, the optional error is populated. This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
Boolean CFURLCopyResourcePropertyForKey(CFURLRef url, CFStringRef key, void *propertyValueTypeRefPtr, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);


/*
    CFURLCopyResourcePropertiesForKeys
    
    Returns the resource values identified by specified array of resource keys.

    Parameters
        url
            The URL specifying the resource.
        keys
            An array of resource keys that identify the resource properties.
        error
            On output when the result is NULL, the error that occurred. This parameter is optional; if you do not wish the error returned, pass NULL here. The caller is responsible for releasing a valid output error.

    Return Value
        A dictionary of resource values indexed by resource key; NULL if an error occurs.

    Discussion
        CFURLCopyResourcePropertiesForKeys first checks if the URL object already caches the resource values. If so, it returns the cached resource values to the caller. If not, then CFURLCopyResourcePropertyForKey synchronously obtains the resource values from the backing store, adds the resource values to the URL object's cache, and returns the resource values to the caller. The type of the resource values vary by property (see resource key definitions). If the result dictionary does not contain a resource value for one or more of the requested resource keys, it means those resource properties are not available for the specified resource and no errors occurred when determining those resource properties were not available. If this function returns NULL, the optional error is populated. This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
CFDictionaryRef CFURLCopyResourcePropertiesForKeys(CFURLRef url, CFArrayRef keys, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);


/*
    CFURLSetResourcePropertyForKey
    
    Sets the resource value identified by a given resource key.

    Parameters
        url
            The URL specifying the resource.
        key
            The resource key that identifies the resource property.
        propertyValue
            The resource value.
        error
            On output when the result is false, the error that occurred. This parameter is optional; if you do not wish the error returned, pass NULL here. The caller is responsible for releasing a valid output error.

    Return Value
        true if the attempt to set the resource value completed with no errors; otherwise, false.

    Discussion
        CFURLSetResourcePropertyForKey writes the new resource value out to the backing store. Attempts to set a read-only resource property or to set a resource property not supported by the resource are ignored and are not considered errors. If this function returns false, the optional error is populated. This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
Boolean CFURLSetResourcePropertyForKey(CFURLRef url, CFStringRef key, CFTypeRef propertyValue, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);


/*
    CFURLSetResourcePropertiesForKeys
    
    Sets any number of resource values of a URL's resource.

    Parameters
        url
            The URL specifying the resource.
        keyedPropertyValues
            A dictionary of resource values indexed by resource keys.
        error
            On output when the result is false, the error that occurred. This parameter is optional; if you do not wish the error returned, pass NULL here. The caller is responsible for releasing a valid output error.

    Return Value
        true if the attempt to set the resource values completed with no errors; otherwise, false.

    Discussion
        CFURLSetResourcePropertiesForKeys writes the new resource values out to the backing store. Attempts to set read-only resource properties or to set resource properties not supported by the resource are ignored and are not considered errors. If an error occurs after some resource properties have been successfully changed, the userInfo dictionary in the returned error contains an array of resource keys that were not set with the key kCFURLKeysOfUnsetValuesKey. The order in which the resource values are set is not defined. If you need to guarantee the order resource values are set, you should make multiple requests to CFURLSetResourcePropertiesForKeys or CFURLSetResourcePropertyForKey to guarantee the order. If this function returns false, the optional error is populated. This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
Boolean CFURLSetResourcePropertiesForKeys(CFURLRef url, CFDictionaryRef keyedPropertyValues, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);


CF_EXPORT
const CFStringRef kCFURLKeysOfUnsetValuesKey CF_AVAILABLE(10_7, 5_0);
    /* Key for the resource properties that have not been set after the CFURLSetResourcePropertiesForKeys function returns an error, returned as an array of of CFString objects. */


/*
    CFURLClearResourcePropertyCacheForKey
    
    Discards a cached resource value of a URL.

    Parameters
        url
            The URL specifying the resource.
        key
            The resource key that identifies the resource property.

    Discussion
        Discarding a cached resource value may discard other cached resource values, because some resource values are cached as a set of values and because some resource values depend on other resource values (temporary properties have no dependencies). This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
void CFURLClearResourcePropertyCacheForKey(CFURLRef url, CFStringRef key) CF_AVAILABLE(10_6, 4_0);


/*
    CFURLClearResourcePropertyCache
    
    Discards all cached resource values of a URL.

    Parameters
        url
            The URL specifying the resource.

    Discussion
        All temporary properties are also cleared from the URL object's cache. This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
void CFURLClearResourcePropertyCache(CFURLRef url) CF_AVAILABLE(10_6, 4_0);


/*
    CFURLSetTemporaryResourcePropertyForKey
    
    Sets a temporary resource value on the URL object.

    Parameters
        url
            The URL object.
        key
            The resource key that identifies the temporary resource property.
        propertyValue
            The resource value.

    Discussion
        Temporary properties are for client use. Temporary properties exist only in memory and are never written to the resource's backing store. Once set, a temporary value can be copied from the URL object with CFURLCopyResourcePropertyForKey and CFURLCopyResourcePropertiesForKeys. To remove a temporary value from the URL object, use CFURLClearResourcePropertyCacheForKey. Temporary values must be valid Core Foundation types, and will be retained by CFURLSetTemporaryResourcePropertyForKey. Care should be taken to ensure the key that identifies a temporary resource property is unique and does not conflict with system defined keys (using reverse domain name notation in your temporary resource property keys is recommended). This function is currently applicable only to URLs for file system resources.
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
void CFURLSetTemporaryResourcePropertyForKey(CFURLRef url, CFStringRef key, CFTypeRef propertyValue) CF_AVAILABLE(10_6, 4_0);


/*
    CFURLResourceIsReachable
    
    Returns whether the URL's resource exists and is reachable.

    Parameters
        url
            The URL object.
        error
            On output when the result is false, the error that occurred. This parameter is optional; if you do not wish the error returned, pass NULL here. The caller is responsible for releasing a valid output error.

    Return Value
        true if the resource is reachable; otherwise, false.

    Discussion
        CFURLResourceIsReachable synchronously checks if the resource's backing store is reachable. Checking reachability is appropriate when making decisions that do not require other immediate operations on the resource, e.g. periodic maintenance of UI state that depends on the existence of a specific document. When performing operations such as opening a file or copying resource properties, it is more efficient to simply try the operation and handle failures. This function is currently applicable only to URLs for file system resources. If this function returns false, the optional error is populated. For other URL types, false is returned. 
        Symbol is present in iOS 4, but performs no operation.
 */
CF_EXPORT
Boolean CFURLResourceIsReachable(CFURLRef url, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);

CF_IMPLICIT_BRIDGING_ENABLED


/* Properties of File System Resources */

CF_EXPORT 
const CFStringRef kCFURLNameKey CF_AVAILABLE(10_6, 4_0);
    /* The resource name provided by the file system (Read-write, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLLocalizedNameKey CF_AVAILABLE(10_6, 4_0);
    /* Localized or extension-hidden name as displayed to users (Read-only, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLIsRegularFileKey CF_AVAILABLE(10_6, 4_0);
    /* True for regular files (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsDirectoryKey CF_AVAILABLE(10_6, 4_0);
    /* True for directories (Read-only, CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsSymbolicLinkKey CF_AVAILABLE(10_6, 4_0);
    /* True for symlinks (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsVolumeKey CF_AVAILABLE(10_6, 4_0);
    /* True for the root directory of a volume (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsPackageKey CF_AVAILABLE(10_6, 4_0);
    /* True for packaged directories (Read-only 10_6 and 10_7, read-write 10_8, value type CFBoolean). Note: You can only set or clear this property on directories; if you try to set this property on non-directory objects, the property is ignored. If the directory is a package for some other reason (extension type, etc), setting this property to false will have no effect. */

CF_EXPORT
const CFStringRef kCFURLIsSystemImmutableKey CF_AVAILABLE(10_6, 4_0);
    /* True for system-immutable resources (Read-write, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsUserImmutableKey CF_AVAILABLE(10_6, 4_0);
    /* True for user-immutable resources (Read-write, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsHiddenKey CF_AVAILABLE(10_6, 4_0);
    /* True for resources normally not displayed to users (Read-write, value type CFBoolean). Note: If the resource is a hidden because its name starts with a period, setting this property to false will not change the property. */

CF_EXPORT
const CFStringRef kCFURLHasHiddenExtensionKey CF_AVAILABLE(10_6, 4_0);
    /* True for resources whose filename extension is removed from the localized name property (Read-write, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLCreationDateKey CF_AVAILABLE(10_6, 4_0);
    /* The date the resource was created (Read-write, value type CFDate) */

CF_EXPORT
const CFStringRef kCFURLContentAccessDateKey CF_AVAILABLE(10_6, 4_0);
    /* The date the resource was last accessed (Read-only, value type CFDate) */

CF_EXPORT
const CFStringRef kCFURLContentModificationDateKey CF_AVAILABLE(10_6, 4_0);
    /* The time the resource content was last modified (Read-write, value type CFDate) */

CF_EXPORT
const CFStringRef kCFURLAttributeModificationDateKey CF_AVAILABLE(10_6, 4_0);
    /* The time the resource's attributes were last modified (Read-write, value type CFDate) */

CF_EXPORT
const CFStringRef kCFURLLinkCountKey CF_AVAILABLE(10_6, 4_0);
    /* Number of hard links to the resource (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLParentDirectoryURLKey CF_AVAILABLE(10_6, 4_0);
    /* The resource's parent directory, if any (Read-only, value type CFURL) */

CF_EXPORT
const CFStringRef kCFURLVolumeURLKey CF_AVAILABLE(10_6, 4_0);
    /* URL of the volume on which the resource is stored (Read-only, value type CFURL) */

CF_EXPORT
const CFStringRef kCFURLTypeIdentifierKey CF_AVAILABLE(10_6, 4_0);
    /* Uniform type identifier (UTI) for the resource (Read-only, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLLocalizedTypeDescriptionKey CF_AVAILABLE(10_6, 4_0);
    /* User-visible type or "kind" description (Read-only, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLLabelNumberKey CF_AVAILABLE(10_6, 4_0);
    /* The label number assigned to the resource (Read-write, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLLabelColorKey CF_AVAILABLE(10_6, 4_0);
    /* The color of the assigned label (Currently not implemented, value type CGColorRef, must link with Application Services) */

CF_EXPORT
const CFStringRef kCFURLLocalizedLabelKey CF_AVAILABLE(10_6, 4_0);
    /* The user-visible label text (Read-only, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLEffectiveIconKey CF_AVAILABLE(10_6, 4_0);
    /* The icon normally displayed for the resource (Read-only, value type CGImageRef, must link with Application Services) */

CF_EXPORT
const CFStringRef kCFURLCustomIconKey CF_AVAILABLE(10_6, 4_0);
    /* The custom icon assigned to the resource, if any (Currently not implemented, value type CGImageRef, must link with Application Services) */

CF_EXPORT
const CFStringRef kCFURLFileResourceIdentifierKey CF_AVAILABLE(10_7, 5_0);
    /* An identifier which can be used to compare two file system objects for equality using CFEqual (i.e, two object identifiers are equal if they have the same file system path or if the paths are linked to same inode on the same file system). This identifier is not persistent across system restarts. (Read-only, value type CFType) */

CF_EXPORT
const CFStringRef kCFURLVolumeIdentifierKey CF_AVAILABLE(10_7, 5_0);
    /* An identifier that can be used to identify the volume the file system object is on. Other objects on the same volume will have the same volume identifier and can be compared using for equality using CFEqual. This identifier is not persistent across system restarts. (Read-only, value type CFType) */

CF_EXPORT
const CFStringRef kCFURLPreferredIOBlockSizeKey CF_AVAILABLE(10_7, 5_0);
    /* The optimal block size when reading or writing this file's data, or NULL if not available. (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLIsReadableKey CF_AVAILABLE(10_7, 5_0);
    /* true if this process (as determined by EUID) can read the resource. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsWritableKey CF_AVAILABLE(10_7, 5_0);
    /* true if this process (as determined by EUID) can write to the resource. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLIsExecutableKey CF_AVAILABLE(10_7, 5_0);
    /* true if this process (as determined by EUID) can execute a file resource or search a directory resource. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLFileSecurityKey CF_AVAILABLE(10_7, 5_0);
    /* The file system object's security information encapsulated in a CFFileSecurity object. (Read-write, value type CFFileSecurity) */

CF_EXPORT
const CFStringRef kCFURLIsExcludedFromBackupKey CF_AVAILABLE(10_8, 5_1);
    /* true if resource should be excluded from backups, false otherwise (Read-write, value type CFBoolean). This property is only useful for excluding cache and other application support files which are not needed in a backup. Some operations commonly made to user documents will cause this property to be reset to false and so this property should not be used on user documents. */

CF_EXPORT
const CFStringRef kCFURLTagNamesKey CF_AVAILABLE(10_9, NA);
    /* The array of Tag names (Read-write, value type CFArray of CFString) */
    
CF_EXPORT
const CFStringRef kCFURLPathKey CF_AVAILABLE(10_8, 6_0);
    /* the URL's path as a file system path (Read-only, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLIsMountTriggerKey CF_AVAILABLE(10_7, 4_0);
    /* true if this URL is a file system trigger directory. Traversing or opening a file system trigger will cause an attempt to mount a file system on the trigger directory. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLGenerationIdentifierKey CF_AVAILABLE(10_10, 8_0);
    /* An opaque generation identifier which can be compared using CFEqual() to determine if the data in a document has been modified. For URLs which refer to the same file inode, the generation identifier will change when the data in the file's data fork is changed (changes to extended attributes or other file system metadata do not change the generation identifier). For URLs which refer to the same directory inode, the generation identifier will change when direct children of that directory are added, removed or renamed (changes to the data of the direct children of that directory will not change the generation identifier). The generation identifier is persistent across system restarts. The generation identifier is tied to a specific document on a specific volume and is not transferred when the document is copied to another volume. This property is not supported by all volumes. (Read-only, value type CFType) */

CF_EXPORT
const CFStringRef kCFURLDocumentIdentifierKey CF_AVAILABLE(10_10, 8_0);
    /* The document identifier -- a value assigned by the kernel to a document (which can be either a file or directory) and is used to identify the document regardless of where it gets moved on a volume. The document identifier survives "safe save” operations; i.e it is sticky to the path it was assigned to (-replaceItemAtURL:withItemAtURL:backupItemName:options:resultingItemURL:error: is the preferred safe-save API). The document identifier is persistent across system restarts. The document identifier is not transferred when the file is copied. Document identifiers are only unique within a single volume. This property is not supported by all volumes. (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLAddedToDirectoryDateKey CF_AVAILABLE(10_10, 8_0);
    /* The date the resource was created, or renamed into or within its parent directory. Note that inconsistent behavior may be observed when this attribute is requested on hard-linked items. This property is not supported by all volumes. (Read-only, value type CFDate) */

CF_EXPORT
const CFStringRef kCFURLQuarantinePropertiesKey CF_AVAILABLE(10_10, NA);
    /* The quarantine properties as defined in LSQuarantine.h. To remove quarantine information from a file, pass kCFNull as the value when setting this property. (Read-write, value type CFDictionary) */

CF_EXPORT
const CFStringRef kCFURLFileResourceTypeKey CF_AVAILABLE(10_7, 5_0);
    /* Returns the file system object type. (Read-only, value type CFString) */

/* The file system object type values returned for the kCFURLFileResourceTypeKey */
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeNamedPipe CF_AVAILABLE(10_7, 5_0);
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeCharacterSpecial CF_AVAILABLE(10_7, 5_0);
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeDirectory CF_AVAILABLE(10_7, 5_0);
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeBlockSpecial CF_AVAILABLE(10_7, 5_0);
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeRegular CF_AVAILABLE(10_7, 5_0);
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeSymbolicLink CF_AVAILABLE(10_7, 5_0);
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeSocket CF_AVAILABLE(10_7, 5_0);
CF_EXPORT
const CFStringRef kCFURLFileResourceTypeUnknown CF_AVAILABLE(10_7, 5_0);

/* File Properties */

CF_EXPORT
const CFStringRef kCFURLFileSizeKey CF_AVAILABLE(10_6, 4_0);
    /* Total file size in bytes (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLFileAllocatedSizeKey CF_AVAILABLE(10_6, 4_0);
    /* Total size allocated on disk for the file in bytes (number of blocks times block size) (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLTotalFileSizeKey CF_AVAILABLE(10_7, 5_0);
    /* Total displayable size of the file in bytes (this may include space used by metadata), or NULL if not available. (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLTotalFileAllocatedSizeKey CF_AVAILABLE(10_7, 5_0);
    /* Total allocated size of the file in bytes (this may include space used by metadata), or NULL if not available. This can be less than the value returned by kCFURLTotalFileSizeKey if the resource is compressed. (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLIsAliasFileKey CF_AVAILABLE(10_6, 4_0);
    /*  true if the resource is a Finder alias file or a symlink, false otherwise ( Read-only, value type CFBooleanRef) */


/* Volume Properties */

/* As a convenience, volume properties can be requested from any file system URL. The value returned will reflect the property value for the volume on which the resource is located. */

CF_EXPORT
const CFStringRef kCFURLVolumeLocalizedFormatDescriptionKey CF_AVAILABLE(10_6, 4_0);
    /* The user-visible volume format (Read-only, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLVolumeTotalCapacityKey CF_AVAILABLE(10_6, 4_0);
    /* Total volume capacity in bytes (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLVolumeAvailableCapacityKey CF_AVAILABLE(10_6, 4_0);
    /* Total free space in bytes (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLVolumeResourceCountKey CF_AVAILABLE(10_6, 4_0);
    /* Total number of resources on the volume (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsPersistentIDsKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume format supports persistent object identifiers and can look up file system objects by their IDs (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsSymbolicLinksKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume format supports symbolic links (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsHardLinksKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume format supports hard links (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsJournalingKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume format supports a journal used to speed recovery in case of unplanned restart (such as a power outage or crash). This does not necessarily mean the volume is actively using a journal. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsJournalingKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume is currently using a journal for speedy recovery after an unplanned restart. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsSparseFilesKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume format supports sparse files, that is, files which can have 'holes' that have never been written to, and thus do not consume space on disk. A sparse file may have an allocated size on disk that is less than its logical length. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsZeroRunsKey CF_AVAILABLE(10_6, 4_0);
    /* For security reasons, parts of a file (runs) that have never been written to must appear to contain zeroes. true if the volume keeps track of allocated but unwritten runs of a file so that it can substitute zeroes without actually writing zeroes to the media. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsCaseSensitiveNamesKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume format treats upper and lower case characters in file and directory names as different. Otherwise an upper case character is equivalent to a lower case character, and you can't have two names that differ solely in the case of the characters. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsCasePreservedNamesKey CF_AVAILABLE(10_6, 4_0);
    /* true if the volume format preserves the case of file and directory names.  Otherwise the volume may change the case of some characters (typically making them all upper or all lower case). (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsRootDirectoryDatesKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume supports reliable storage of times for the root directory. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsVolumeSizesKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume supports returning volume size values (kCFURLVolumeTotalCapacityKey and kCFURLVolumeAvailableCapacityKey). (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsRenamingKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume can be renamed. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsAdvisoryFileLockingKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume implements whole-file flock(2) style advisory locks, and the O_EXLOCK and O_SHLOCK flags of the open(2) call. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeSupportsExtendedSecurityKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume implements extended security (ACLs). (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsBrowsableKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume should be visible via the GUI (i.e., appear on the Desktop as a separate volume). (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeMaximumFileSizeKey CF_AVAILABLE(10_7, 5_0);
    /* The largest file size (in bytes) supported by this file system, or NULL if this cannot be determined. (Read-only, value type CFNumber) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsEjectableKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume's media is ejectable from the drive mechanism under software control. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsRemovableKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume's media is removable from the drive mechanism. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsInternalKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume's device is connected to an internal bus, false if connected to an external bus, or NULL if not available. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsAutomountedKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume is automounted. Note: do not mistake this with the functionality provided by kCFURLVolumeSupportsBrowsingKey. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsLocalKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume is stored on a local device. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeIsReadOnlyKey CF_AVAILABLE(10_7, 5_0);
    /* true if the volume is read-only. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLVolumeCreationDateKey CF_AVAILABLE(10_7, 5_0);
    /* The volume's creation date, or NULL if this cannot be determined. (Read-only, value type CFDate) */

CF_EXPORT
const CFStringRef kCFURLVolumeURLForRemountingKey CF_AVAILABLE(10_7, 5_0);
    /* The CFURL needed to remount a network volume, or NULL if not available. (Read-only, value type CFURL) */

CF_EXPORT
const CFStringRef kCFURLVolumeUUIDStringKey CF_AVAILABLE(10_7, 5_0);
    /* The volume's persistent UUID as a string, or NULL if a persistent UUID is not available for the volume. (Read-only, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLVolumeNameKey CF_AVAILABLE(10_7, 5_0);
    /* The name of the volume (Read-write, settable if kCFURLVolumeSupportsRenamingKey is true and permissions allow, value type CFString) */

CF_EXPORT
const CFStringRef kCFURLVolumeLocalizedNameKey CF_AVAILABLE(10_7, 5_0);
    /* The user-presentable name of the volume (Read-only, value type CFString) */

/* UbiquitousItem Properties */

CF_EXPORT
const CFStringRef kCFURLIsUbiquitousItemKey CF_AVAILABLE(10_7, 5_0);
    /* true if this item is synced to the cloud, false if it is only a local file. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemHasUnresolvedConflictsKey CF_AVAILABLE(10_7, 5_0);
    /* true if this item has conflicts outstanding. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemIsDownloadedKey CF_DEPRECATED(10_7, 10_9, 5_0, 7_0, "Use kCFURLUbiquitousItemDownloadingStatusKey instead");
    /* Equivalent to NSURLUbiquitousItemDownloadingStatusKey == NSURLUbiquitousItemDownloadingStatusCurrent. Has never behaved as documented in earlier releases, hence deprecated. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemIsDownloadingKey CF_AVAILABLE(10_7, 5_0);
    /* true if data is being downloaded for this item. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemIsUploadedKey CF_AVAILABLE(10_7, 5_0);
    /* true if there is data present in the cloud for this item. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemIsUploadingKey CF_AVAILABLE(10_7, 5_0);
    /* true if data is being uploaded for this item. (Read-only, value type CFBoolean) */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemPercentDownloadedKey CF_DEPRECATED(10_7, 10_8, 5_0, 6_0, "Use NSMetadataQuery and NSMetadataUbiquitousItemPercentDownloadedKey on NSMetadataItem instead");
    /* Use NSMetadataQuery and NSMetadataUbiquitousItemPercentDownloadedKey on NSMetadataItem instead */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemPercentUploadedKey CF_DEPRECATED(10_7, 10_8, 5_0, 6_0, "Use NSMetadataQuery and NSMetadataUbiquitousItemPercentUploadedKey on NSMetadataItem instead");
    /* Use NSMetadataQuery and NSMetadataUbiquitousItemPercentUploadedKey on NSMetadataItem instead */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemDownloadingStatusKey CF_AVAILABLE(10_9, 7_0);
    /* Returns the download status of this item. (Read-only, value type CFString). Possible values below. */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemDownloadingErrorKey CF_AVAILABLE(10_9, 7_0);
    /* returns the error when downloading the item from iCloud failed. See the NSUbiquitousFile section in FoundationErrors.h. (Read-only, value type CFError) */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemUploadingErrorKey CF_AVAILABLE(10_9, 7_0);
    /* returns the error when uploading the item to iCloud failed. See the NSUbiquitousFile section in FoundationErrors.h. (Read-only, value type CFError) */

/* The values returned for kCFURLUbiquitousItemDownloadingStatusKey
 */
CF_EXPORT
const CFStringRef kCFURLUbiquitousItemDownloadingStatusNotDownloaded CF_AVAILABLE(10_9, 7_0);
    /* this item has not been downloaded yet. Use NSFileManager's startDownloadingUbiquitousItemAtURL:error: to download it */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemDownloadingStatusDownloaded CF_AVAILABLE(10_9, 7_0);
    /* there is a local version of this item available. The most current version will get downloaded as soon as possible. */

CF_EXPORT
const CFStringRef kCFURLUbiquitousItemDownloadingStatusCurrent CF_AVAILABLE(10_9, 7_0);
    /* there is a local version of this item and it is the most up-to-date version known to this device. */


typedef CF_OPTIONS(CFOptionFlags, CFURLBookmarkCreationOptions) {
    kCFURLBookmarkCreationMinimalBookmarkMask = ( 1UL << 9 ), // creates bookmark data with "less" information, which may be smaller but still be able to resolve in certain ways
    kCFURLBookmarkCreationSuitableForBookmarkFile = ( 1UL << 10 ), // include the properties required by CFURLWriteBookmarkDataToFile() in the bookmark data created
    kCFURLBookmarkCreationWithSecurityScope CF_ENUM_AVAILABLE(10_7, NA) = ( 1UL << 11 ), // Mac OS X 10.7.3 and later, include information in the bookmark data which allows the same sandboxed process to access the resource after being relaunched
    kCFURLBookmarkCreationSecurityScopeAllowOnlyReadAccess CF_ENUM_AVAILABLE(10_7, NA) = ( 1UL << 12 ), // Mac OS X 10.7.3 and later, if used with kCFURLBookmarkCreationWithSecurityScope, at resolution time only read access to the resource will be granted
    
    // deprecated
    kCFURLBookmarkCreationPreferFileIDResolutionMask CF_ENUM_DEPRECATED(10_6, 10_9, 4_0, 7_0, "kCFURLBookmarkCreationPreferFileIDResolutionMask does nothing and has no effect on bookmark resolution" ) = ( 1UL << 8 ),
} CF_ENUM_AVAILABLE(10_6, 4_0);

typedef CF_OPTIONS(CFOptionFlags, CFURLBookmarkResolutionOptions) {
    kCFURLBookmarkResolutionWithoutUIMask = ( 1UL << 8 ), // don't perform any user interaction during bookmark resolution
    kCFURLBookmarkResolutionWithoutMountingMask = ( 1UL << 9 ), // don't mount a volume during bookmark resolution
    kCFURLBookmarkResolutionWithSecurityScope CF_ENUM_AVAILABLE(10_7, NA) = ( 1UL << 10 ), // Mac OS X 10.7.3 and later, use the secure information included at creation time to provide the ability to access the resource in a sandboxed process.
    
    kCFBookmarkResolutionWithoutUIMask = kCFURLBookmarkResolutionWithoutUIMask,
    kCFBookmarkResolutionWithoutMountingMask = kCFURLBookmarkResolutionWithoutMountingMask,
} CF_ENUM_AVAILABLE(10_6, 4_0);

typedef CFOptionFlags CFURLBookmarkFileCreationOptions;

CF_IMPLICIT_BRIDGING_DISABLED

/* Returns bookmark data for the URL, created with specified options and resource properties. If this function returns NULL, the optional error is populated.
 */
CF_EXPORT
CFDataRef CFURLCreateBookmarkData ( CFAllocatorRef allocator, CFURLRef url, CFURLBookmarkCreationOptions options, CFArrayRef resourcePropertiesToInclude, CFURLRef relativeToURL, CFErrorRef* error ) CF_AVAILABLE(10_6, 4_0);

/* Return a URL that refers to a location specified by resolving bookmark data. If this function returns NULL, the optional error is populated.
 */
CF_EXPORT
CFURLRef CFURLCreateByResolvingBookmarkData ( CFAllocatorRef allocator, CFDataRef bookmark, CFURLBookmarkResolutionOptions options, CFURLRef relativeToURL, CFArrayRef resourcePropertiesToInclude, Boolean* isStale, CFErrorRef* error ) CF_AVAILABLE(10_6, 4_0);

/* Returns the resource propertyies identified by a specified array of keys contained in specified bookmark data. If the result dictionary does not contain a resource value for one or more of the requested resource keys, it means those resource properties are not available in the bookmark data.
 */
CF_EXPORT
CFDictionaryRef CFURLCreateResourcePropertiesForKeysFromBookmarkData ( CFAllocatorRef allocator, CFArrayRef resourcePropertiesToReturn, CFDataRef bookmark ) CF_AVAILABLE(10_6, 4_0);

/*  Returns the resource property identified by a given resource key contained in specified bookmark data. If this function returns NULL, it means the resource property is not available in the bookmark data.
 */
CF_EXPORT
CFTypeRef  CFURLCreateResourcePropertyForKeyFromBookmarkData( CFAllocatorRef allocator, CFStringRef resourcePropertyKey, CFDataRef bookmark ) CF_AVAILABLE(10_6, 4_0);

/* Returns bookmark data derived from an alias file referred to by fileURL. If fileURL refers to an alias file created prior to OS X v10.6 that contains Alias Manager information but no bookmark data, this method synthesizes bookmark data for the file. If this method returns NULL, the optional error is populated.
 */
CF_EXPORT
CFDataRef CFURLCreateBookmarkDataFromFile(CFAllocatorRef allocator, CFURLRef fileURL, CFErrorRef *errorRef ) CF_AVAILABLE(10_6, 5_0);

/* Creates an alias file on disk at a specified location with specified bookmark data. The bookmark data must have been created with the kCFURLBookmarkCreationSuitableForBookmarkFile option. fileURL must either refer to an existing file (which will be overwritten), or to location in an existing directory. If this method returns FALSE, the optional error is populated.
 */
CF_EXPORT
Boolean CFURLWriteBookmarkDataToFile( CFDataRef bookmarkRef, CFURLRef fileURL, CFURLBookmarkFileCreationOptions options, CFErrorRef *errorRef ) CF_AVAILABLE(10_6, 5_0);

/* Returns bookmark data derived from an alias record.
 */
CF_EXPORT
CFDataRef CFURLCreateBookmarkDataFromAliasRecord ( CFAllocatorRef allocatorRef, CFDataRef aliasRecordDataRef ) CF_AVAILABLE_MAC(10_6);

CF_IMPLICIT_BRIDGING_ENABLED

/* Given a CFURLRef created by resolving a bookmark data created with security scope, make the resource referenced by the url accessible to the process. When access to this resource is no longer needed the client must call CFURLStopAccessingSecurityScopedResource(). Each call to CFURLStartAccessingSecurityScopedResource() must be balanced with a call to CFURLStopAccessingSecurityScopedResource() (Note: this is not reference counted).
 */
CF_EXPORT
Boolean CFURLStartAccessingSecurityScopedResource(CFURLRef url) CF_AVAILABLE(10_7, 8_0); // On OSX, available in MacOS X 10.7.3 and later

/* Revokes the access granted to the url by a prior successful call to CFURLStartAccessingSecurityScopedResource().
 */
CF_EXPORT
void CFURLStopAccessingSecurityScopedResource(CFURLRef url) CF_AVAILABLE(10_7, 8_0); // On OSX, available in MacOS X 10.7.3 and later

#endif /* TARGET_OS_MAC || TARGET_OS_EMBEDDED || TARGET_OS_IPHONE */

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFURL__ */

