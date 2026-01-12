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

/*	CFURLAccess.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Jim Luther/Chris Linn
*/

/*------
CFData read/write routines
-------*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"

#include "CFInternal.h"
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFURLAccess.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFNumber.h>
#include <string.h>
#include <ctype.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#elif DEPLOYMENT_TARGET_WINDOWS
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <dlfcn.h>
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED


DEFINE_WEAK_CFNETWORK_FUNC_FAIL(Boolean, _CFURLCreateDataAndPropertiesFromResource, (CFAllocatorRef A, CFURLRef B, CFDataRef *C, CFDictionaryRef *D, CFArrayRef E, SInt32 *F), (A, B, C, D, E, F), { if(C) *C=NULL; if (D) *D=NULL; if(F) *F=kCFURLImproperArgumentsError; }, false)
DEFINE_WEAK_CFNETWORK_FUNC_FAIL(Boolean, _CFURLWriteDataAndPropertiesToResource, (CFURLRef A, CFDataRef B, CFDictionaryRef C, SInt32 *D), (A, B, C, D), if (D) *D = kCFURLImproperArgumentsError, false)

DEFINE_WEAK_CFNETWORK_FUNC_FAIL(Boolean, _CFURLDestroyResource, (CFURLRef A, SInt32 *B), (A, B), if(B) *B = kCFURLImproperArgumentsError, false)

#endif

typedef struct __NSString__ *NSString;

/*
    Pre-10.6 property keys
*/

CONST_STRING_DECL(kCFURLFileExists, "kCFURLFileExists")
CONST_STRING_DECL(kCFURLFilePOSIXMode, "kCFURLFilePOSIXMode")
CONST_STRING_DECL(kCFURLFileDirectoryContents, "kCFURLFileDirectoryContents")
CONST_STRING_DECL(kCFURLFileLength, "kCFURLFileLength")
CONST_STRING_DECL(kCFURLFileLastModificationTime, "kCFURLFileLastModificationTime")
CONST_STRING_DECL(kCFURLFileOwnerID, "kCFURLFileOwnerID")
CONST_STRING_DECL(kCFURLHTTPStatusCode, "kCFURLHTTPStatusCode")
CONST_STRING_DECL(kCFURLHTTPStatusLine, "kCFURLHTTPStatusLine")

CONST_STRING_DECL(kCFDataURLDataLength, "kCFDataURLDataLength")
CONST_STRING_DECL(kCFDataURLMimeType, "kCFDataURLMimeType")
CONST_STRING_DECL(kCFDataURLTextEncodingName, "kCFDataURLTextEncodingName")

// Compatibility property strings -- we obsoleted these names pre-DP4. REW, 5/22/2000
CONST_STRING_DECL(kCFFileURLExists, "kCFURLFileExists")
CONST_STRING_DECL(kCFFileURLPOSIXMode, "kCFURLFilePOSIXMode")
CONST_STRING_DECL(kCFFileURLDirectoryContents, "kCFURLFileDirectoryContents")
CONST_STRING_DECL(kCFFileURLSize, "kCFURLFileLength")
CONST_STRING_DECL(kCFFileURLLastModificationTime, "kCFURLFileLastModificationTime")
CONST_STRING_DECL(kCFHTTPURLStatusCode, "kCFURLHTTPStatusCode")
CONST_STRING_DECL(kCFHTTPURLStatusLine, "kCFURLHTTPStatusLine")

// Copied pretty much verbatim from NSData; note that files are still special cased in this code.  Ultimately, we probably want to treat file URLs the same way as any other URL (go through the URL Access layer).  -- REW, 10/21/98

/*************************/
/* file: access routines */
/*************************/

//#warning CF:For the moment file access failures are ill defined and set the error code to kCFURLUnknownError

static CFDictionaryRef _CFFileURLCreatePropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFArrayRef desiredProperties, SInt32 *errorCode) {
    // MF:!!! This could/should be changed to use _CFGetFileProperties() to do the actual figuring.
    static CFArrayRef _allProps = NULL;
    CFRange arrayRange;
    SInt32 idx;
    CFMutableDictionaryRef propertyDict = NULL;

    Boolean exists;
    SInt32 posixMode;
    int64_t size;
    CFDateRef modTime = NULL, *modTimePtr = NULL;
    CFArrayRef contents = NULL, *contentsPtr = NULL;
    SInt32 ownerID;

    if (errorCode) *errorCode = 0;
    if (!desiredProperties) {
        // Cheap and dirty hack to make this work for the moment; ultimately we need to do something more sophisticated.  This will result in an error return whenever a property key is defined which isn't applicable to all file URLs.  REW, 3/2/99
        if (!_allProps) {
            const void *values[9];
            values[0] = kCFURLFileExists;
            values[1] = kCFURLFilePOSIXMode;
            values[2] = kCFURLFileDirectoryContents;
            values[3] = kCFURLFileLength;
            values[4] = kCFURLFileLastModificationTime;
            values[5] = kCFURLFileOwnerID;
            _allProps = CFArrayCreate(kCFAllocatorSystemDefault, values, 6, &kCFTypeArrayCallBacks);
        }
        desiredProperties = _allProps;
    }

    arrayRange.location = 0;
    arrayRange.length = CFArrayGetCount(desiredProperties);
    propertyDict = CFDictionaryCreateMutable(alloc, 0, & kCFTypeDictionaryKeyCallBacks, & kCFTypeDictionaryValueCallBacks);
    if (arrayRange.length == 0) return propertyDict;

    if (CFArrayContainsValue(desiredProperties, arrayRange, kCFURLFileDirectoryContents)) {
        contentsPtr = &contents;
    }
    if (CFArrayContainsValue(desiredProperties, arrayRange, kCFURLFileLastModificationTime)) {
        modTimePtr = &modTime;
    }

    if (_CFGetFileProperties(alloc, url, &exists, &posixMode, &size, modTimePtr, &ownerID, contentsPtr) != 0) {
	
	//  If all they've asked for is whether this file exists, then any error will just make
	//  this return kCFURLFileExists = kCFBooleanFalse, which handles the case where the filename is invalid or too long or something.
	if ( arrayRange.length == 1 && CFArrayContainsValue( desiredProperties, arrayRange, kCFURLFileExists ) ) {
	    CFDictionarySetValue(propertyDict, kCFURLFileExists, kCFBooleanFalse);
	}
	else if (errorCode) {
            *errorCode = kCFURLUnknownError;
        }
        return propertyDict;
    }
    
    for (idx = 0; idx < arrayRange.length; idx ++) {
        CFStringRef key = (CFMutableStringRef )CFArrayGetValueAtIndex(desiredProperties, idx);
        if (key == kCFURLFilePOSIXMode || CFEqual(kCFURLFilePOSIXMode, key)) {
            if (exists) {
                CFNumberRef num = CFNumberCreate(alloc, kCFNumberSInt32Type, &posixMode);
                CFDictionarySetValue(propertyDict, kCFURLFilePOSIXMode, num);
                CFRelease(num);
            } else if (errorCode) {
                *errorCode = kCFURLUnknownError;
            }
        } else if (key == kCFURLFileDirectoryContents || CFEqual(kCFURLFileDirectoryContents, key)) {
            if (exists && (posixMode & S_IFMT) == S_IFDIR && contents) {
                CFDictionarySetValue(propertyDict, kCFURLFileDirectoryContents, contents);
            } else if (errorCode) {
                *errorCode = kCFURLUnknownError;
            }
        } else if (key == kCFURLFileLength || CFEqual(kCFURLFileLength, key)) {
            if (exists) {
                CFNumberRef num = CFNumberCreate(alloc, kCFNumberSInt64Type, &size);
                CFDictionarySetValue(propertyDict, kCFURLFileLength, num);
                CFRelease(num);
            } else if (errorCode) {
                *errorCode = kCFURLUnknownError;
            }
        } else if (key == kCFURLFileLastModificationTime || CFEqual(kCFURLFileLastModificationTime, key)) {
            if (exists && modTime) {
                CFDictionarySetValue(propertyDict, kCFURLFileLastModificationTime, modTime);
            } else if (errorCode) {
                *errorCode = kCFURLUnknownError;
            }
        } else if (key == kCFURLFileExists || CFEqual(kCFURLFileExists, key)) {
            if (exists) {
                CFDictionarySetValue(propertyDict, kCFURLFileExists, kCFBooleanTrue);
            } else {
                CFDictionarySetValue(propertyDict, kCFURLFileExists, kCFBooleanFalse);
            }
        } else if (key == kCFURLFileOwnerID || CFEqual(kCFURLFileOwnerID, key)) {
            if (exists) {
                CFNumberRef num  = CFNumberCreate(alloc, kCFNumberSInt32Type, &ownerID);
                CFDictionarySetValue(propertyDict, kCFURLFileOwnerID, num);
                CFRelease(num);
            } else if (errorCode) {
                *errorCode = kCFURLUnknownError;
            }
        // Add more properties here
        } else if (errorCode) {
            *errorCode = kCFURLUnknownPropertyKeyError;
        }
    }
    if (modTime) CFRelease(modTime);
    if (contents) CFRelease(contents);
    return propertyDict;
}

static Boolean _CFFileURLWritePropertiesToResource(CFURLRef url, CFDictionaryRef propertyDict, SInt32 *errorCode) {
    CFTypeRef buffer[16];
    CFTypeRef *keys;
    CFTypeRef *values;
    Boolean result = true;
    SInt32 idx, count;
    char cPath[CFMaxPathSize];

    if (!CFURLGetFileSystemRepresentation(url, true, (unsigned char *)cPath, CFMaxPathSize)) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        return false;
    }

    count = CFDictionaryGetCount(propertyDict);
    if (count < 8) {
        keys = buffer;
        values = buffer+8;
    } else {
        keys = (CFTypeRef *)CFAllocatorAllocate(CFGetAllocator(url), sizeof(void *) * count * 2, 0);
        values = keys + count;
    }
    CFDictionaryGetKeysAndValues(propertyDict, keys, values);

    for (idx = 0; idx < count; idx ++) {
        CFStringRef key = (CFStringRef)keys[idx];
        CFTypeRef value = values[idx];
        if (key == kCFURLFilePOSIXMode || CFEqual(kCFURLFilePOSIXMode, key)) {
            SInt32 mode;
            int err;
            if (CFNumberGetTypeID() == CFGetTypeID(value)) {
                CFNumberRef modeNum = (CFNumberRef)value;
                CFNumberGetValue(modeNum, kCFNumberSInt32Type, &mode);
            } else {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#define MODE_TYPE mode_t
#elif DEPLOYMENT_TARGET_WINDOWS
#define MODE_TYPE uint16_t
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
                const MODE_TYPE *modePtr = (const MODE_TYPE *)CFDataGetBytePtr((CFDataRef)value);
                mode = *modePtr;
            }
            err = chmod(cPath, mode);
            if (err != 0) result = false;
        } else {
            result = false;
        }
    }

    if ((CFTypeRef)keys != buffer) CFAllocatorDeallocate(CFGetAllocator(url), keys);

    if (errorCode) *errorCode = result ? 0 : kCFURLUnknownError;
    return result;
}

static Boolean _CFFileURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *fetchedData, CFArrayRef desiredProperties, CFDictionaryRef *fetchedProperties, SInt32 *errorCode) {
    Boolean success = true;

    if (errorCode) *errorCode = 0;
    if (fetchedData) {
        void *bytes;
        CFIndex length;
        Boolean releaseAlloc = false;
        
        if (alloc == NULL) {
            // We need a real allocator to pass to _CFReadBytesFromFile so that the CFDataRef we create with
			//	CFDataCreateWithBytesNoCopy() can free up the object _CFReadBytesFromFile() returns.
            alloc = (CFAllocatorRef)CFRetain(__CFGetDefaultAllocator());
            releaseAlloc = true;
        }
        if (!_CFReadBytesFromFile(alloc, url, &bytes, &length, 0, 0)) {
            if (errorCode) *errorCode = kCFURLUnknownError;
            *fetchedData = NULL;
            success = false;
        } else {
            *fetchedData = CFDataCreateWithBytesNoCopy(alloc, (const UInt8 *)bytes, length, alloc);
        }
        if (releaseAlloc) {
            // Now the CFData should be hanging on to it.
            CFRelease(alloc);
        }
    }

    if (fetchedProperties) {
        *fetchedProperties = _CFFileURLCreatePropertiesFromResource(alloc, url, desiredProperties, errorCode);
        if (!*fetchedProperties)
	    success = false;
    }

    if ( ! success && fetchedData && *fetchedData ) {
	CFRelease( *fetchedData );
	*fetchedData = NULL;
    }
    
    return success;
}

/*
 * Support for data: URLs - RFC 2397
 * Currently this is spi for CFNetwork, to make it API, just put these constants in CFURLAccess.h
 */

/*
CF_EXPORT
const CFStringRef kCFDataURLDataLength;
CF_EXPORT
const CFStringRef kCFDataURLMimeType;
CF_EXPORT
const CFStringRef kCFDataURLTextEncodingName;
*/

/* Properties for the data: scheme. */
/* kCFDataURLDataLength is a CFNumber giving the data's length in bytes. */
/* kCFDataURLMimeType is a CFString. */
/* kCFDataURLTextEncodingName is a CFString. */

/* REMINDSMZ: From CFURLResponse.c */
static CFStringRef mimeTypeFromContentTypeComponent(CFStringRef component) {
    CFIndex compLen = CFStringGetLength(component);
    CFStringInlineBuffer buf;
    CFIndex idx;
    CFIndex firstChar = -1, lastChar = -1;
    CFCharacterSetRef whitespaceSet = CFCharacterSetGetPredefined(kCFCharacterSetWhitespace);
    CFStringInitInlineBuffer(component, &buf, CFRangeMake(0, compLen));
    
    for (idx = 0; idx < compLen; idx ++) {
		UniChar ch = CFStringGetCharacterFromInlineBuffer(&buf, idx);
		if (ch == ';') {
			// Delimits the charset
			break;
		} else if (firstChar == -1) {
			if (!CFCharacterSetIsCharacterMember(whitespaceSet, ch)) {
				firstChar = idx;
			}
		} else if (!CFCharacterSetIsCharacterMember(whitespaceSet, ch)) {
			lastChar = idx;
		}
    }
    if (firstChar != -1 && lastChar != -1) {
		CFMutableStringRef newContentType = CFStringCreateMutableCopy(CFGetAllocator(component), compLen, component);
		if (lastChar != compLen - 1) {
			CFStringDelete(newContentType, CFRangeMake(lastChar + 1, compLen - lastChar - 1));
		}
		if (firstChar > 0) {
			CFStringDelete(newContentType, CFRangeMake(0, firstChar));
		}
		CFStringLowercase(newContentType, NULL);
		return newContentType;
    }
    return NULL;
}

/* REMINDSMZ: From CFURLResponse.c */
static CFStringRef charsetFromContentTypeHeader(CFStringRef contentType) {
    // FIXME: Should this use KeyValuePair parsing to handle quoting properly?
    CFRange range;
    CFIndex compLen = CFStringGetLength(contentType);
    CFIndex start, end, idx;
    CFCharacterSetRef whitespaceSet;
    CFMutableStringRef result;
    
	CFStringRef kCFURLResponseCharsetPrefix = CFSTR("charset=");
	
    if (!CFStringFindWithOptions(contentType, kCFURLResponseCharsetPrefix, CFRangeMake(0, compLen), kCFCompareCaseInsensitive, &range) || range.length == 0) return NULL;
	
    whitespaceSet = CFCharacterSetGetPredefined(kCFCharacterSetWhitespace);
    start = -1; 
    end = -1;
    for (idx = range.location + range.length; idx < compLen; idx ++) {
		UniChar ch = CFStringGetCharacterAtIndex(contentType, idx);
		if (ch == ';' || ch == ',') break;
		if (start == -1) {
			if (!CFCharacterSetIsCharacterMember(whitespaceSet, ch)) {
				start = idx;
				end = idx;
			} 
		} else if (!CFCharacterSetIsCharacterMember(whitespaceSet,ch)) {
			end = idx;
		}
    }
	
    if (start == -1) return NULL;
	
    result = CFStringCreateMutableCopy(CFGetAllocator(contentType), compLen,contentType);
    if (end != compLen) {
		CFStringDelete(result, CFRangeMake(end+1, compLen-end-1));
    }
    CFStringDelete(result, CFRangeMake(0, start));
    CFStringLowercase(result, NULL);
    return result;
}

#define STATIC_BUFFER_SIZE 1024

static BOOL isHexDigit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static UInt8 hexDigitValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    // NSURL_ERROR("illegal hex digit");
    return 0;
}

static CFDataRef percentEscapeDecodeBuffer(CFAllocatorRef alloc, const UInt8* srcBuffer, CFRange range, Boolean stripWhitespace)
{
    UInt8* dstBuffer;
    UInt8 staticDstBuffer[STATIC_BUFFER_SIZE];
	
    if (range.length > STATIC_BUFFER_SIZE) {
		dstBuffer = (UInt8*) malloc(range.length);
    } else {
		dstBuffer = staticDstBuffer;
    }
	
    CFIndex end = range.location + range.length;
	
    CFIndex i;
    CFIndex j;
    for (i = range.location, j = 0; i < end; ++i) {
		char value;
		
		if (srcBuffer[i] == '%' && end > i + 2 && isHexDigit(srcBuffer[i+1]) && isHexDigit(srcBuffer[i+2])) {
			value = hexDigitValue(srcBuffer[i+1]) * 16 + hexDigitValue(srcBuffer[i+2]);
			i += 2;
		} else {
			value = srcBuffer[i];
		}
		
		if (!stripWhitespace || !isspace(value)) {
			dstBuffer[j++] = value;
		}
    }
	
    CFDataRef result = CFDataCreate(alloc, dstBuffer, j);
	
    if (dstBuffer != staticDstBuffer) {
		free(dstBuffer);
    }
	
    return result;
}


// base 64 digits: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

static BOOL isBase64Digit(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '+') || (c == '/');
}

static BOOL isBase64DigitOrEqualSign(char c)
{
    return isBase64Digit(c) || c == '=';
}

static UInt8 base64DigitValue(char c)
{
    if (c >= 'A' && c <= 'Z') {
		return c - 'A';
    } else if (c >= 'a' && c <= 'z') {
		return 26 + c - 'a';
    } else if (c >= '0' && c <= '9') {
		return 52 + c - '0';
    } else if (c == '+') {
		return 62;
    } else if (c == '/') {
		return 63;
    } else {
		return 0;
    }
}

static CFDataRef base64DecodeData(CFAllocatorRef alloc, CFDataRef data)
{
    const UInt8 *srcBuffer = CFDataGetBytePtr(data);
    CFIndex length = CFDataGetLength(data);
    UInt8 *dstBuffer = NULL;
    UInt8 staticDstBuffer[STATIC_BUFFER_SIZE];
	CFDataRef result = NULL;
	
    // base64 encoded data length must be multiple of 4
    if (length % 4 != 0) {
		goto done;
    }
	
    if (length > STATIC_BUFFER_SIZE) {
		dstBuffer = (UInt8*) malloc(length);
    } else {
		dstBuffer = staticDstBuffer;
    }
	
    CFIndex i;
    CFIndex j;
    for (i = 0, j = 0; i < length; i+=4) {
		if (!(isBase64Digit(srcBuffer[i]) &&
			  isBase64Digit(srcBuffer[i+1]) &&
			  isBase64DigitOrEqualSign(srcBuffer[i+2]) &&
			  isBase64DigitOrEqualSign(srcBuffer[i+3]))) {
			if (dstBuffer != staticDstBuffer) {
				free(dstBuffer);
			}
			return NULL;
		}
		
		dstBuffer[j++] = (base64DigitValue(srcBuffer[i]) << 2) + (base64DigitValue(srcBuffer[i+1]) >> 4);
		if (srcBuffer[i+2] != '=') {
			dstBuffer[j++] = ((base64DigitValue(srcBuffer[i+1]) & 0xf) << 4) + (base64DigitValue(srcBuffer[i+2]) >> 2);
		}
		if (srcBuffer[i+3] != '=') {
			dstBuffer[j++] = ((base64DigitValue(srcBuffer[i+2]) & 0x3) << 6) + (base64DigitValue(srcBuffer[i+3]));
		}
    }
    
    result = CFDataCreate(alloc, dstBuffer, j);
	
done:
    if (dstBuffer != staticDstBuffer) {
		free(dstBuffer);
    }
	
    return result;
}

static inline CFStringRef percentExpandAndTrimContentType(CFAllocatorRef alloc, CFStringRef str, CFRange range)
{
    CFMutableStringRef contentTypeHeader = NULL;
    CFStringRef contentTypeUnexpanded = CFStringCreateWithSubstring(alloc, str, range);
    CFStringRef contentTypeExpanded = CFURLCreateStringByReplacingPercentEscapes(alloc, contentTypeUnexpanded, CFSTR(""));
    
    if (NULL == contentTypeExpanded) {
	contentTypeHeader = CFStringCreateMutableCopy(alloc, 0, contentTypeUnexpanded);
    } else {
	contentTypeHeader = CFStringCreateMutableCopy(alloc, 0, contentTypeExpanded);
	CFRelease(contentTypeExpanded);
    }
    CFRelease(contentTypeUnexpanded);
    CFStringTrimWhitespace(contentTypeHeader);
    
    return contentTypeHeader;
}

static Boolean parseDataRequestURL(CFURLRef url, CFDataRef* outData, CFStringRef* outMimeType, CFStringRef* outTextEncodingName)
{
    Boolean result = FALSE;
    CFAllocatorRef alloc = CFGetAllocator(url);
    CFStringRef str = CFURLCopyResourceSpecifier(url);
    if (str != NULL) {
		CFRange commaRange = CFStringFind(str, CFSTR(","), 0);
		
		if (commaRange.location != kCFNotFound) {
			CFStringRef contentTypeHeader = percentExpandAndTrimContentType(alloc, str, CFRangeMake(0, commaRange.location));
			CFStringRef mimeType = mimeTypeFromContentTypeComponent(contentTypeHeader);
			CFStringRef textEncodingName = charsetFromContentTypeHeader(contentTypeHeader);

			Boolean base64 = CFStringFind(contentTypeHeader, CFSTR(";base64"), kCFCompareCaseInsensitive).location != kCFNotFound;

			if (mimeType == NULL) {
				mimeType = (CFStringRef) CFRetain(CFSTR("text/plain"));
			}
			
			CFIndex bufferSize = CFURLGetBytes(url, NULL, 0);
			UInt8* srcBuffer = (UInt8*) malloc(bufferSize);
			CFURLGetBytes(url, srcBuffer, bufferSize);
			
			CFRange dataRange = CFURLGetByteRangeForComponent(url, kCFURLComponentResourceSpecifier, NULL);
			while (srcBuffer[dataRange.location] != ',') {
				dataRange.location++;
				dataRange.length--;
			}
			dataRange.location++;
			dataRange.length--;
			
			CFDataRef dataRef = NULL;
			
			if (! base64) {
				dataRef = percentEscapeDecodeBuffer(alloc, srcBuffer, dataRange, false);
			} else {
				CFDataRef unescapedAndStripped = percentEscapeDecodeBuffer(alloc, srcBuffer, dataRange, true);
				if (unescapedAndStripped) {
					dataRef = base64DecodeData(alloc, unescapedAndStripped);
					CFRelease(unescapedAndStripped);
				}
			}
			
			if (dataRef != NULL)  {
				*outData = dataRef;
				*outMimeType = (CFStringRef) mimeType == NULL? NULL : CFStringCreateCopy(alloc, mimeType);
				*outTextEncodingName = (CFStringRef) textEncodingName == NULL? NULL : CFStringCreateCopy(alloc, textEncodingName);
				result = true;
			}

			free(srcBuffer);
			
			if (contentTypeHeader) CFRelease(contentTypeHeader);
			if (mimeType) CFRelease(mimeType);
			if (textEncodingName) CFRelease(textEncodingName);
		}
		
		CFRelease(str);
    }
    
    return result;
}

static Boolean _CFDataURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *fetchedData, CFArrayRef desiredProperties, CFDictionaryRef *fetchedProperties, SInt32 *errorCode) {
    Boolean success = true;
	
    if (errorCode) *errorCode = 0;
	
	// We always need to fetch the data...
	CFDataRef data = NULL;
	CFStringRef mimeType = NULL;
	CFStringRef textEncodingName = NULL;

	if (! parseDataRequestURL(url, &data, &mimeType, &textEncodingName)) {
		if (errorCode)
			*errorCode = kCFURLUnknownError;
		*fetchedData = NULL;
		success = false;
	} else {
		if (fetchedData) {
			*fetchedData = CFDataCreateCopy(alloc, data);
		}
		
		if (fetchedProperties) {
			const void* propKeys[] = {
				kCFDataURLDataLength,
				kCFDataURLMimeType,
				kCFDataURLTextEncodingName,
			};
			const CFIndex propKeysCount = sizeof(propKeys) / sizeof(propKeys[0]);
			
			if (desiredProperties == NULL) {
				static CFArrayRef sAllProps = NULL;
				if (sAllProps == NULL) {
					sAllProps = CFArrayCreate(kCFAllocatorSystemDefault, propKeys, propKeysCount, &kCFTypeArrayCallBacks);
				}
				desiredProperties = sAllProps;
			}
				
			const void* vals[propKeysCount];
			const void* keys[propKeysCount];
			int ixVal = 0;
			
			CFIndex count = CFArrayGetCount(desiredProperties);
			for (CFIndex i = 0;  i < count;  i++) {
				CFStringRef key = (CFStringRef) CFArrayGetValueAtIndex(desiredProperties, i);
				
				if (CFEqual(key, kCFDataURLDataLength)) {
					CFIndex len = CFDataGetLength(data);
					keys[ixVal] = key;
					vals[ixVal++] = CFNumberCreate(alloc, kCFNumberCFIndexType, &len);
				} else if (CFEqual(key, kCFDataURLMimeType)) {
					if (mimeType != NULL) {
						keys[ixVal] = key;
						vals[ixVal++] = CFStringCreateCopy(alloc, mimeType);
					}
				} else if (CFEqual(key, kCFDataURLTextEncodingName)) {
					if (textEncodingName != NULL) {
						keys[ixVal] = key;
						vals[ixVal++] = CFStringCreateCopy(alloc, textEncodingName);
					}
				}
			}
			
			*fetchedProperties = CFDictionaryCreate(alloc, keys, vals, ixVal, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			for (CFIndex i = 0;  i < ixVal; i++) 
				CFRelease(vals[i]);
			if (*fetchedProperties == NULL)
				success = false;
		}
		
		if (data) CFRelease(data);
		if (mimeType) CFRelease(mimeType);
		if (textEncodingName) CFRelease(textEncodingName);
    }
	
	
    return success;
}

/*************************/
/* Public routines       */
/*************************/

Boolean CFURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *fetchedData, CFDictionaryRef *fetchedProperties, CFArrayRef desiredProperties, SInt32 *errorCode) {
    CFStringRef scheme = CFURLCopyScheme(url);

    if (!scheme) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        if (fetchedData) *fetchedData = NULL;
        if (fetchedProperties) *fetchedProperties = NULL;
        return false;
    } else {
        Boolean result;
        if (CFStringCompare(scheme, CFSTR("file"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            result = _CFFileURLCreateDataAndPropertiesFromResource(alloc, url, fetchedData, desiredProperties, fetchedProperties, errorCode);
        } else if (CFStringCompare(scheme, CFSTR("data"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
	    result = _CFDataURLCreateDataAndPropertiesFromResource(alloc, url, fetchedData, desiredProperties, fetchedProperties, errorCode);
	} else {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
            result = __CFNetwork__CFURLCreateDataAndPropertiesFromResource(alloc, url, fetchedData, fetchedProperties, desiredProperties, errorCode);
#else
            if (fetchedData) *fetchedData = NULL;
            if (fetchedProperties) *fetchedProperties = NULL;
            if (errorCode) *errorCode = kCFURLUnknownSchemeError;
            result = false;
#endif
        }
        CFRelease(scheme);
        return result;
    }
}

CFTypeRef CFURLCreatePropertyFromResource(CFAllocatorRef alloc, CFURLRef url, CFStringRef property, SInt32 *errorCode) {
    CFArrayRef array = CFArrayCreate(alloc, (const void **)&property, 1, &kCFTypeArrayCallBacks);
    CFDictionaryRef dict;

    if (CFURLCreateDataAndPropertiesFromResource(alloc, url, NULL, &dict, array, errorCode)) {
        CFTypeRef result = CFDictionaryGetValue(dict, property);
        if (result) CFRetain(result);
        CFRelease(array);
        CFRelease(dict);
        return result;
    } else {
        if (dict) CFRelease(dict);
        CFRelease(array);
        return NULL;
    }
}

Boolean CFURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef data, CFDictionaryRef propertyDict, SInt32 *errorCode) {
    CFStringRef scheme = CFURLCopyScheme(url);
    if (!scheme) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        return false;
    } else if (CFStringCompare(scheme, CFSTR("file"), 0) == kCFCompareEqualTo) {
        Boolean success = true;
        CFRelease(scheme);
        if (errorCode) *errorCode = 0;
        if (data) {
            if (CFURLHasDirectoryPath(url)) {
                // Create a directory
                char cPath[CFMaxPathSize];
                if (!CFURLGetFileSystemRepresentation(url, true, (unsigned char *)cPath, CFMaxPathSize))
                {
                    if (errorCode) *errorCode = kCFURLImproperArgumentsError;
                    success = false;
                } else {
                    success = _CFCreateDirectory(cPath);
                    if (!success && errorCode) *errorCode = kCFURLUnknownError;
                }
            } else {
               // Write data
                SInt32 length = CFDataGetLength(data);
                const void *bytes = (0 == length) ? (const void *)"" : CFDataGetBytePtr(data);
                success = _CFWriteBytesToFile(url, bytes, length);
                if (!success && errorCode) *errorCode = kCFURLUnknownError;
            }
        }
        if (propertyDict) {
            if (!_CFFileURLWritePropertiesToResource(url, propertyDict, errorCode))
                success = false;
        }
        return success;
    } else {
        CFRelease(scheme);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        Boolean result = __CFNetwork__CFURLWriteDataAndPropertiesToResource(url, data, propertyDict, errorCode);
	if (!result) {
	    if (errorCode) *errorCode = kCFURLUnknownSchemeError;
	}
	return result;
#else
        if (errorCode) *errorCode = kCFURLUnknownSchemeError;
        return false;
#endif
    }
}

Boolean CFURLDestroyResource(CFURLRef url, SInt32 *errorCode) {
    CFStringRef scheme = CFURLCopyScheme(url);
    char cPath[CFMaxPathSize];

    if (!scheme) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        return false;
    } else if (CFStringCompare(scheme, CFSTR("file"), 0) == kCFCompareEqualTo) {
        CFRelease(scheme);
        if (!CFURLGetFileSystemRepresentation(url, true, (unsigned char *)cPath, CFMaxPathSize)) {
            if (errorCode) *errorCode = kCFURLImproperArgumentsError;
            return false;
        }

        if (CFURLHasDirectoryPath(url)) {
            if (_CFRemoveDirectory(cPath)) {
                if (errorCode) *errorCode = 0;
                return true;
            } else {
                if (errorCode) *errorCode = kCFURLUnknownError;
                return false;
            }
        } else {
            if (_CFDeleteFile(cPath)) {
                if (errorCode) *errorCode = 0;
                return true;
            } else {
                if (errorCode) *errorCode = kCFURLUnknownError;
                return false;
            }
        }
    } else {
        CFRelease(scheme);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        Boolean result = __CFNetwork__CFURLDestroyResource(url, errorCode);
	if (!result) {
	    if (errorCode) *errorCode = kCFURLUnknownSchemeError;
	}
	return result;
#else
        if (errorCode) *errorCode = kCFURLUnknownSchemeError;
        return false;
#endif
    }
}
#pragma GCC diagnostic pop


