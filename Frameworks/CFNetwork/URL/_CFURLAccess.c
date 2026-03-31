/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
/* This was part of CoreFoundation; CF now dynamically loads us and calls through to us for any non-file scheme.  If/when CFNetwork has its own URL->stream/data/whatever APIs, this should be re-implemented in terms of that.   Note that, for the time being, CoreFoundation still exports the property names */

/*	CFURLAccess.c
	Copyright 1999-2000, Apple, Inc. All rights reserved.
	Responsibility: Becky Willrich
*/

#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFHTTPStream.h>
#include <CFNetwork/CFHTTPMessage.h>
#include <CFNetwork/CFFTPStream.h>
#include <CFNetwork/CFFTPStreamPriv.h>

// Internal prototypes.  What does it mean that these functions are exported from
// CFNetwork, but not used in any other file, or present in any header file? ¥DCJ¥
CFNetwork_EXPORT
CFHTTPMessageRef _CFHTTPMessageSendRequest(CFHTTPMessageRef request);

extern
Boolean _CFURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *fetchedData, CFDictionaryRef *fetchedProperties, CFArrayRef desiredProperties, SInt32 *errorCode);

extern
Boolean _CFURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef data, CFDictionaryRef propertyDict, SInt32 *errorCode);

extern
Boolean _CFURLDestroyResource(CFURLRef url, SInt32 *errorCode);


#define DATA_CHUNK_SIZE 512
CFNetwork_EXPORT
CFHTTPMessageRef _CFHTTPMessageSendRequest(CFHTTPMessageRef request) {
    CFAllocatorRef alloc = CFGetAllocator(request);
    CFReadStreamRef readStream = CFReadStreamCreateForHTTPRequest(alloc, request);
    CFWriteStreamRef writeStream = CFWriteStreamCreateWithAllocatedBuffers(alloc, alloc);
    Boolean fail = FALSE;
    int status;
    CFHTTPMessageRef response = NULL;

    if (!readStream || !writeStream) return NULL;
    CFReadStreamSetProperty(readStream, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue);
    if (!CFReadStreamOpen(readStream) || !CFWriteStreamOpen(writeStream)) {
        fail = TRUE;
    }
    if (!fail) {
        for (status = CFReadStreamGetStatus(readStream); !fail && status != kCFStreamStatusAtEnd && status != kCFStreamStatusError; status = CFReadStreamGetStatus(readStream)) {
            UInt8 buf[DATA_CHUNK_SIZE];
            CFIndex bytesRead = CFReadStreamRead(readStream, buf, DATA_CHUNK_SIZE);
            if (bytesRead > 0) {
                CFIndex bytesWritten = CFWriteStreamWrite(writeStream, buf, bytesRead);
                if (bytesWritten != bytesRead) {
                    fail = TRUE;
                }
            }
        }
        if (!fail && CFReadStreamGetStatus(readStream) == kCFStreamStatusError) {
            fail = TRUE;
        }
        CFReadStreamClose(readStream);
        CFWriteStreamClose(writeStream);
    }
    if (!fail) {
        CFDataRef messageBody = CFWriteStreamCopyProperty(writeStream, kCFStreamPropertyDataWritten);
        response = (CFHTTPMessageRef)CFReadStreamCopyProperty(readStream, kCFStreamPropertyHTTPResponseHeader);
        if (response) {
            CFHTTPMessageSetBody(response, messageBody);
        }
        if (messageBody) CFRelease(messageBody);
    }
    if (readStream) CFRelease(readStream);
    if (writeStream) CFRelease(writeStream);
    return response;
}

/*************************/
/* http: access routines */
/*************************/

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFURLAccessGETMethod				CFSTR("GET")
#define _kCFURLAccessHEADMethod				CFSTR("HEAD")
#define _kCFURLAccessPUTMethod				CFSTR("PUT")
#define _kCFURLAccessDELETEMethod			CFSTR("DELETE")
#define _kCFURLAccessContentLengthHeader	CFSTR("Content-Length")
#define _kCFURLAccessContentLengthFormat	CFSTR("%d")
#else
static CONST_STRING_DECL(_kCFURLAccessGETMethod, "GET")
static CONST_STRING_DECL(_kCFURLAccessHEADMethod, "HEAD")
static CONST_STRING_DECL(_kCFURLAccessPUTMethod, "PUT")
static CONST_STRING_DECL(_kCFURLAccessDELETEMethod, "DELETE")
static CONST_STRING_DECL(_kCFURLAccessContentLengthHeader, "Content-Length")
static CONST_STRING_DECL(_kCFURLAccessContentLengthFormat, "%d")
#endif	/* __CONSTANT_CFSTRINGS__ */

static Boolean _CFHTTPURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *fetchedData, CFArrayRef desiredProperties, CFDictionaryRef *fetchedProperties, SInt32 *errorCode) {
    CFHTTPMessageRef request = NULL;
    CFHTTPMessageRef response;
    Boolean success = TRUE;
    if (errorCode) *errorCode = 0;
    if (fetchedData) {
        request = CFHTTPMessageCreateRequest(alloc, _kCFURLAccessGETMethod, url, kCFHTTPVersion1_0);
    } else if (fetchedProperties) {
        if (desiredProperties && !CFArrayGetCount(desiredProperties)) {
            *fetchedProperties = NULL;
            return TRUE;
        }
        request = CFHTTPMessageCreateRequest(alloc, _kCFURLAccessHEADMethod, url, kCFHTTPVersion1_0);
    } else {
        return TRUE;
    }
    response = _CFHTTPMessageSendRequest(request);
    CFRelease(request);
    if (!response) {
        if (fetchedData) *fetchedData = NULL;
        if (fetchedProperties) *fetchedProperties = NULL;
        if (errorCode) *errorCode = kCFURLRemoteHostUnavailableError;
        return FALSE;
    }

    if (fetchedData) {
        *fetchedData = CFHTTPMessageCopyBody(response);
    }

    if (fetchedProperties) {
        if (!desiredProperties) {
            SInt32 code = CFHTTPMessageGetResponseStatusCode(response);
            CFNumberRef num = CFNumberCreate(alloc, kCFNumberSInt32Type, &code);
            CFDictionaryRef dict = CFHTTPMessageCopyAllHeaderFields(response);
            CFStringRef status;
            if (dict) {
                *fetchedProperties = CFDictionaryCreateMutableCopy(alloc, CFDictionaryGetCount(dict)+2,  dict);
                CFRelease(dict);
            } else {
                *fetchedProperties = CFDictionaryCreateMutable(alloc, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            }
            status = CFHTTPMessageCopyResponseStatusLine(response);
            if (status) {
                CFDictionarySetValue((CFMutableDictionaryRef)(*fetchedProperties), kCFURLHTTPStatusLine, status);
                CFRelease(status);
            }
            CFDictionarySetValue((CFMutableDictionaryRef)(*fetchedProperties), kCFURLHTTPStatusCode, num);
            CFRelease(num);
        } else {
            SInt32 idx, cnt = CFArrayGetCount(desiredProperties);
            *fetchedProperties = CFDictionaryCreateMutable(alloc, cnt, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            for (idx = 0; idx < cnt; idx ++) {
                CFStringRef key = CFArrayGetValueAtIndex(desiredProperties, idx);
                if (key == kCFURLHTTPStatusLine) {
                    CFStringRef status = CFHTTPMessageCopyResponseStatusLine(response);
                    if (status) {
                        CFDictionarySetValue((CFMutableDictionaryRef)(*fetchedProperties), kCFURLHTTPStatusLine, status);
                        CFRelease(status);
                    } else {
                        if (errorCode) *errorCode = kCFURLPropertyKeyUnavailableError;
                        success = FALSE;
                    }
                } else if (key == kCFURLHTTPStatusCode) {
                    SInt32 code = CFHTTPMessageGetResponseStatusCode(response);
                    CFNumberRef num = CFNumberCreate(alloc, kCFNumberSInt32Type, &code);
                    CFDictionarySetValue((CFMutableDictionaryRef)(*fetchedProperties), key, num);
                    CFRelease(num);
                } else {
                    CFStringRef value = CFHTTPMessageCopyHeaderFieldValue(response, key);
                    if (value) {
                        CFDictionarySetValue((CFMutableDictionaryRef)(*fetchedProperties), key, value);
                        CFRelease(value);
                    } else {
                        if (errorCode) *errorCode = kCFURLPropertyKeyUnavailableError;
                        success = FALSE;
                    }
                }
            }
        }
    }
    CFRelease(response);
    return success;
}

static Boolean _CFHTTPURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef data, CFDictionaryRef propertyDict, SInt32 *errorCode) {
    CFAllocatorRef alloc = CFGetAllocator(url);
    CFHTTPMessageRef response, request;
    CFStringRef lenStr;
    CFIndex count;
    if (!data) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        return FALSE;
    }
    request = CFHTTPMessageCreateRequest(alloc, _kCFURLAccessPUTMethod, url, kCFHTTPVersion1_0);
    lenStr = CFStringCreateWithFormat(alloc, NULL, _kCFURLAccessContentLengthFormat, CFDataGetLength(data));
    CFHTTPMessageSetHeaderFieldValue(request, _kCFURLAccessContentLengthHeader, lenStr);
    CFRelease(lenStr);
    if (propertyDict && (count = CFDictionaryGetCount(propertyDict)) > 0) {
        CFStringRef *keys, *values, *currentValue;
        keys = CFAllocatorAllocate(alloc, sizeof(CFStringRef) * 2 * count, 0);
        values = keys + count;
        CFDictionaryGetKeysAndValues(propertyDict, (const void **)keys, (const void **)values);
        for (currentValue = values; keys < values; currentValue ++, keys ++) {
            if (CFGetTypeID(*currentValue) == CFStringGetTypeID()) {
                CFHTTPMessageSetHeaderFieldValue(request, *keys, *currentValue);
            }
        }
        CFAllocatorDeallocate(alloc, keys);
    }
    response = _CFHTTPMessageSendRequest(request);
    CFRelease(request);
    if (response) {
        UInt32 status = CFHTTPMessageGetResponseStatusCode(response);
        CFRelease(response);
        if (status < 300 && status > 199) {
            if (errorCode) *errorCode = 0;
            return TRUE;
        } else {
            if (errorCode) *errorCode = status;
            return FALSE;
        }
    }
    if (errorCode) *errorCode = kCFURLRemoteHostUnavailableError;
    return FALSE;
}

static Boolean _CFHTTPURLDestroyResource(CFURLRef url, SInt32 *errorCode) {
    CFHTTPMessageRef response, request = CFHTTPMessageCreateRequest(CFGetAllocator(url), _kCFURLAccessDELETEMethod, url, kCFHTTPVersion1_0);
    response = _CFHTTPMessageSendRequest(request);
    CFRelease(request);
    if (response) {
        UInt32 status = CFHTTPMessageGetResponseStatusCode(response);
        CFRelease(request);
        if  (status < 300 && status > 199) {
            if (errorCode) *errorCode = 0;
            return TRUE;
        } else {
            if (errorCode) *errorCode = status;
            return FALSE;
        }
    }
    if (errorCode) *errorCode = kCFURLRemoteHostUnavailableError;
    return FALSE;
}

/**********************/
/* FTP routines       */
/**********************/
static void _ApplyWriteStreamProperties(CFTypeRef key, CFTypeRef value, CFWriteStreamRef stream) {
	
	if (CFGetTypeID(key) == CFStringGetTypeID())
		CFWriteStreamSetProperty((CFWriteStreamRef)stream, (CFStringRef)key, value);
}

static Boolean _CFFTPURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *fetchedData, CFArrayRef desiredProperties, CFDictionaryRef *fetchedProperties, SInt32 *errorCode) {
	
	SInt32 extra;
	CFStreamError error;
	CFReadStreamRef readStream;
	
	if (!errorCode) errorCode = &extra;
	
	if (!fetchedData) {
		*errorCode = kCFURLImproperArgumentsError;
		return FALSE;
	}
	
	*fetchedData = (CFDataRef)CFDataCreateMutable(alloc, 0);
	readStream = CFReadStreamCreateWithFTPURL(alloc, url);
	
	// Don't use persistence and this request won't get stuck behind another.
	CFReadStreamSetProperty(readStream, kCFStreamPropertyFTPAttemptPersistentConnection, kCFBooleanFalse); 
	
	if (CFReadStreamOpen(readStream)) {
		
		CFIndex read;
		
		do {
			UInt8 buffer[32768];
			read = CFReadStreamRead(readStream, buffer, sizeof(buffer));
			
			if (read <= 0)
				break;
			
			CFDataAppendBytes(*((CFMutableDataRef*)fetchedData), buffer, read);
				
		} while (1);
		
		CFReadStreamClose(readStream);			
	}
	
	error = CFReadStreamGetError(readStream);
	*errorCode = error.error;
	
	CFRelease(readStream);
	
    return (*errorCode == 0) ? TRUE : FALSE;
}

static Boolean _CFFTPURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef data, CFDictionaryRef propertyDict, SInt32 *errorCode) {
	
	SInt32 extra;
	CFStreamError error;
	CFWriteStreamRef writeStream;
	const UInt8* buffer;
	CFIndex left;
	
	if (!errorCode) errorCode = &extra;
	
	if (!data) {
		*errorCode = kCFURLImproperArgumentsError;
		return FALSE;
	}
	
	buffer = CFDataGetBytePtr(data);
	left = CFDataGetLength(data);
	
	writeStream = CFWriteStreamCreateWithFTPURL(CFGetAllocator(url), url);
	if (propertyDict)
		CFDictionaryApplyFunction(propertyDict, (CFDictionaryApplierFunction)_ApplyWriteStreamProperties, writeStream);
	
	// Don't use persistence and this request won't get stuck behind another.
	CFWriteStreamSetProperty(writeStream, kCFStreamPropertyFTPAttemptPersistentConnection, kCFBooleanFalse); 
	
	if (CFWriteStreamOpen(writeStream)) {
		
		while (left) {
			
			CFIndex written = CFWriteStreamWrite(writeStream, buffer, left);
		
			if (written <= 0)
				break;
			
			buffer += written;
			left -= written;
		}
		
		CFWriteStreamClose(writeStream);
	}
	
	error = CFWriteStreamGetError(writeStream);
	*errorCode = error.error;
	
	CFRelease(writeStream);
	
    return (*errorCode == 0) ? TRUE : FALSE;
}

static Boolean _CFFTPURLDestroyResource(CFURLRef url, SInt32 *errorCode) {
	
	SInt32 extra;
	CFStreamError error;
	CFWriteStreamRef writeStream = CFWriteStreamCreateWithFTPURL(CFGetAllocator(url), url);
	
	if (!errorCode) errorCode = &extra;
	
	// Don't use persistence and this request won't get stuck behind another.
	CFWriteStreamSetProperty(writeStream, kCFStreamPropertyFTPAttemptPersistentConnection, kCFBooleanFalse); 
	
	// Use magic property to indicate to remove the resource.
	CFWriteStreamSetProperty(writeStream, _kCFStreamPropertyFTPRemoveResource, kCFBooleanTrue);
	
	if (CFWriteStreamOpen(writeStream)) {
		CFWriteStreamWrite(writeStream, "a", 1);
		CFWriteStreamClose(writeStream);
	}
	
	error = CFWriteStreamGetError(writeStream);
	*errorCode = error.error;
	
	CFRelease(writeStream);
	
    return (*errorCode == 0) ? TRUE : FALSE;
}

/*************************/
/* Public routines       */
/*************************/

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFURLAccessHTTPScheme		CFSTR("http")
#define _kCFURLAccessHTTPSScheme	CFSTR("https")
#define _kCFURLAccessFTPScheme		CFSTR("ftp")
#else
static CONST_STRING_DECL(_kCFURLAccessHTTPScheme, "http")
static CONST_STRING_DECL(_kCFURLAccessHTTPSScheme, "https")
static CONST_STRING_DECL(_kCFURLAccessFTPScheme, "ftp")
#endif	/* __CONSTANT_CFSTRINGS__ */

extern
Boolean _CFURLCreateDataAndPropertiesFromResource(CFAllocatorRef alloc, CFURLRef url, CFDataRef *fetchedData, CFDictionaryRef *fetchedProperties, CFArrayRef desiredProperties, SInt32 *errorCode) {

    CFStringRef scheme = CFURLCopyScheme(url);

    if (!scheme) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        if (fetchedData) *fetchedData = NULL;
        if (fetchedProperties) *fetchedProperties = NULL;
        return FALSE;
    } else {
        Boolean result;
        if (CFStringCompare(scheme, _kCFURLAccessHTTPScheme, 0) == kCFCompareEqualTo || CFStringCompare(scheme, _kCFURLAccessHTTPSScheme, 0) == kCFCompareEqualTo) {
            result = _CFHTTPURLCreateDataAndPropertiesFromResource(alloc, url, fetchedData, desiredProperties, fetchedProperties, errorCode);
        } else if (CFStringCompare(scheme, _kCFURLAccessFTPScheme, 0) == kCFCompareEqualTo) {
            result = _CFFTPURLCreateDataAndPropertiesFromResource(alloc, url, fetchedData, desiredProperties, fetchedProperties, errorCode);
        } else {
            if (fetchedData) *fetchedData = NULL;
            if (fetchedProperties) *fetchedProperties = NULL;
            if (errorCode) *errorCode = kCFURLUnknownSchemeError;
            result = FALSE;
        }
        CFRelease(scheme);
        return result;
    }
}

extern
Boolean _CFURLWriteDataAndPropertiesToResource(CFURLRef url, CFDataRef data, CFDictionaryRef propertyDict, SInt32 *errorCode) {
    CFStringRef scheme = CFURLCopyScheme(url);
    Boolean result;
    if (!scheme) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        return FALSE;
    }
    
    if (CFStringCompare(scheme, _kCFURLAccessHTTPScheme, 0) == kCFCompareEqualTo || CFStringCompare(scheme, _kCFURLAccessHTTPSScheme, 0) == kCFCompareEqualTo) {
        result = _CFHTTPURLWriteDataAndPropertiesToResource(url, data, propertyDict, errorCode);
    } else if (CFStringCompare(scheme, _kCFURLAccessFTPScheme, 0) == kCFCompareEqualTo) {
        result = _CFFTPURLWriteDataAndPropertiesToResource(url, data, propertyDict, errorCode);
    } else {
        if (errorCode) *errorCode = kCFURLUnknownSchemeError;
        result = FALSE;
    }
    CFRelease(scheme);
    return result;
}

extern
Boolean _CFURLDestroyResource(CFURLRef url, SInt32 *errorCode) {
    CFStringRef scheme = CFURLCopyScheme(url);
    Boolean result;
    if (!scheme) {
        if (errorCode) *errorCode = kCFURLImproperArgumentsError;
        return FALSE;
    }
    
    if (CFStringCompare(scheme, _kCFURLAccessHTTPScheme, 0) == kCFCompareEqualTo || CFStringCompare(scheme, _kCFURLAccessHTTPSScheme, 0) == kCFCompareEqualTo) {
        result = _CFHTTPURLDestroyResource(url, errorCode);
    } else if (CFStringCompare(scheme, _kCFURLAccessFTPScheme, 0) == kCFCompareEqualTo) {
        result = _CFFTPURLDestroyResource(url, errorCode);
    } else {
        if (errorCode) *errorCode = kCFURLUnknownSchemeError;
        result = FALSE;
    }
    CFRelease(scheme);
    return result;
}
