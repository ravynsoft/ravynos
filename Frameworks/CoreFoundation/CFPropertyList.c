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

/*	CFPropertyList.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Tony Parker
*/

#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFError_Private.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFStringEncodingConverter.h>
#include "CFInternal.h"
#include <CoreFoundation/CFBurstTrie.h>
#include <CoreFoundation/CFString.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
#include <CoreFoundation/CFStream.h>
#endif
#include <CoreFoundation/CFCalendar.h>
#include "CFLocaleInternal.h"
#include <limits.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>


CF_EXPORT CFNumberType _CFNumberGetType2(CFNumberRef number);

#define PLIST_IX    0
#define ARRAY_IX    1
#define DICT_IX     2
#define KEY_IX      3
#define STRING_IX   4
#define DATA_IX     5
#define DATE_IX     6
#define REAL_IX     7
#define INTEGER_IX  8
#define TRUE_IX     9
#define FALSE_IX    10
#define DOCTYPE_IX  11
#define CDSECT_IX   12

#define PLIST_TAG_LENGTH	5
#define ARRAY_TAG_LENGTH	5
#define DICT_TAG_LENGTH		4
#define KEY_TAG_LENGTH		3
#define STRING_TAG_LENGTH	6
#define DATA_TAG_LENGTH		4
#define DATE_TAG_LENGTH		4
#define REAL_TAG_LENGTH		4
#define INTEGER_TAG_LENGTH	7
#define TRUE_TAG_LENGTH		4
#define FALSE_TAG_LENGTH	5
#define DOCTYPE_TAG_LENGTH	7
#define CDSECT_TAG_LENGTH	9

#if !defined(new_cftype_array)
#define new_cftype_array(N, C) \
    size_t N ## _count__ = (C); \
    if (N ## _count__ > LONG_MAX / sizeof(CFTypeRef)) { \
        CRSetCrashLogMessage("CFPropertyList ran out of memory while attempting to allocate temporary storage."); \
        HALT; \
    } \
    Boolean N ## _is_stack__ = (N ## _count__ <= 256); \
    if (N ## _count__ == 0) N ## _count__ = 1; \
    STACK_BUFFER_DECL(CFTypeRef, N ## _buffer__, N ## _is_stack__ ? N ## _count__ : 1); \
    if (N ## _is_stack__) memset(N ## _buffer__, 0, N ## _count__ * sizeof(CFTypeRef)); \
    CFTypeRef * N = N ## _is_stack__ ? N ## _buffer__ : (CFTypeRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, (N ## _count__) * sizeof(CFTypeRef), __kCFAllocatorGCScannedMemory); \
    if (! N) { \
        CRSetCrashLogMessage("CFPropertyList ran out of memory while attempting to allocate temporary storage."); \
        HALT; \
    } \
    do {} while (0)
#endif

#if !defined(free_cftype_array)
#define free_cftype_array(N) \
    if (! N ## _is_stack__) { \
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, N); \
    } \
    do {} while (0)
#endif

// Used to reference an old-style plist parser error inside of a more general XML error
#define CFPropertyListOldStyleParserErrorKey     CFSTR("kCFPropertyListOldStyleParsingError")

static CFTypeID stringtype, datatype, numbertype, datetype;
static CFTypeID booltype, nulltype, dicttype, arraytype, settype;

static void initStatics() {
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        stringtype = CFStringGetTypeID();
        datatype = CFDataGetTypeID();        
        numbertype = CFNumberGetTypeID();
        booltype = CFBooleanGetTypeID();
        datetype = CFDateGetTypeID();
        dicttype = CFDictionaryGetTypeID();
        arraytype = CFArrayGetTypeID();
        settype = CFSetGetTypeID();
        nulltype = CFNullGetTypeID();
    });
}

CF_PRIVATE CFErrorRef __CFPropertyListCreateError(CFIndex code, CFStringRef debugString, ...) {    
    va_list argList;        
    CFErrorRef error = NULL;
    
    if (debugString != NULL) {
        CFStringRef debugMessage = NULL;
        va_start(argList, debugString);
        debugMessage = CFStringCreateWithFormatAndArguments(kCFAllocatorSystemDefault, NULL, debugString, argList);
        va_end(argList);
    
        CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks); 
        CFDictionarySetValue(userInfo, kCFErrorDebugDescriptionKey, debugMessage);
                
        error = CFErrorCreate(kCFAllocatorSystemDefault, kCFErrorDomainCocoa, code, userInfo);
        
        CFRelease(debugMessage);
        CFRelease(userInfo);
    } else {
        error = CFErrorCreate(kCFAllocatorSystemDefault, kCFErrorDomainCocoa, code, NULL);
    }
    
    return error;
}

static CFStringRef __copyErrorDebugDescription(CFErrorRef error) {
    CFStringRef result = NULL;
    if (error) {
        CFDictionaryRef userInfo = CFErrorCopyUserInfo(error);
        if (userInfo != NULL) {
            CFStringRef desc = (CFStringRef) CFDictionaryGetValue(userInfo, kCFErrorDebugDescriptionKey);
            if (desc != NULL) {
                result = CFStringCreateCopy(kCFAllocatorSystemDefault, desc);
            }
            CFRelease(userInfo);
        }
    }
    return result;
}

#pragma mark -
#pragma mark Property List Validation

// don't allow _CFKeyedArchiverUID here
#define __CFAssertIsPList(cf) CFAssert2(CFGetTypeID(cf) == CFStringGetTypeID() || CFGetTypeID(cf) == CFArrayGetTypeID() || CFGetTypeID(cf) == CFBooleanGetTypeID() || CFGetTypeID(cf) == CFNumberGetTypeID() || CFGetTypeID(cf) == CFDictionaryGetTypeID() || CFGetTypeID(cf) == CFDateGetTypeID() || CFGetTypeID(cf) == CFDataGetTypeID(), __kCFLogAssertion, "%s(): %p not of a property list type", __PRETTY_FUNCTION__, cf);

struct context {
    bool answer;
    CFMutableSetRef set;
    CFPropertyListFormat format;
    CFStringRef *error;
};

static bool __CFPropertyListIsValidAux(CFPropertyListRef plist, bool recursive, CFMutableSetRef set, CFPropertyListFormat format, CFStringRef *error);

static void __CFPropertyListIsArrayPlistAux(const void *value, void *context) {
    struct context *ctx = (struct context *)context;
    if (!ctx->answer) return;
    if (!value && !*(ctx->error)) {
	*(ctx->error) = (CFStringRef)CFRetain(CFSTR("property list arrays cannot contain NULL"));
    }
    ctx->answer = value && __CFPropertyListIsValidAux(value, true, ctx->set, ctx->format, ctx->error);
}

static void __CFPropertyListIsDictPlistAux(const void *key, const void *value, void *context) {
    struct context *ctx = (struct context *)context;
    if (!ctx->answer) return;
    if (!key && !*(ctx->error)) *(ctx->error) = (CFStringRef)CFRetain(CFSTR("property list dictionaries cannot contain NULL keys"));
    if (!value && !*(ctx->error)) *(ctx->error) = (CFStringRef)CFRetain(CFSTR("property list dictionaries cannot contain NULL values"));
    if (stringtype != CFGetTypeID(key) && !*(ctx->error)) {
	CFStringRef desc = CFCopyTypeIDDescription(CFGetTypeID(key));
	*(ctx->error) = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("property list dictionaries may only have keys which are CFStrings, not '%@'"), desc);
	CFRelease(desc);
    }
    ctx->answer = key && value && (stringtype == CFGetTypeID(key)) && __CFPropertyListIsValidAux(value, true, ctx->set, ctx->format, ctx->error);
}

static bool __CFPropertyListIsValidAux(CFPropertyListRef plist, bool recursive, CFMutableSetRef set, CFPropertyListFormat format, CFStringRef *error) {
    CFTypeID type;
    if (!plist) {
	*error = (CFStringRef)CFRetain(CFSTR("property lists cannot contain NULL"));
    	return false;
    }
    type = CFGetTypeID(plist);    
    if (stringtype == type) return true;
    if (datatype == type) return true;
    if (kCFPropertyListOpenStepFormat != format) {
	if (booltype == type) return true;
	if (numbertype == type) return true;
	if (datetype == type) return true;        
	if (_CFKeyedArchiverUIDGetTypeID() == type) return true;
    }
    if (!recursive && arraytype == type) return true;
    if (!recursive && dicttype == type) return true;
    // at any one invocation of this function, set should contain the objects in the "path" down to this object
    if (CFSetContainsValue(set, plist)) {
	*error = (CFStringRef)CFRetain(CFSTR("property lists cannot contain recursive container references"));
	return false;
    }
    if (arraytype == type) {
	struct context ctx = {true, set, format, error}; 
	CFSetAddValue(set, plist);
	CFArrayApplyFunction((CFArrayRef)plist, CFRangeMake(0, CFArrayGetCount((CFArrayRef)plist)), __CFPropertyListIsArrayPlistAux, &ctx);
	CFSetRemoveValue(set, plist);
	return ctx.answer;
    }
    if (dicttype == type) {
	struct context ctx = {true, set, format, error}; 
	CFSetAddValue(set, plist);
	CFDictionaryApplyFunction((CFDictionaryRef)plist, __CFPropertyListIsDictPlistAux, &ctx);
	CFSetRemoveValue(set, plist);
	return ctx.answer;
    }

    CFStringRef desc = CFCopyTypeIDDescription(type);
    *error = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("property lists cannot contain objects of type '%@'"), desc);
    CFRelease(desc);

    return false;
}

static Boolean _CFPropertyListIsValidWithErrorString(CFPropertyListRef plist, CFPropertyListFormat format, CFStringRef *error) {
    initStatics();
    CFAssert1(plist != NULL, __kCFLogAssertion, "%s(): NULL is not a property list", __PRETTY_FUNCTION__);
    CFMutableSetRef set = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, NULL);
    bool result = __CFPropertyListIsValidAux(plist, true, set, format, error);
    CFRelease(set);
    return result;
}

#pragma mark -
#pragma mark Writing Property Lists

static const char CFXMLPlistTags[13][10]= {
{'p', 'l', 'i', 's', 't',   '\0', '\0', '\0', '\0', '\0'},
{'a', 'r', 'r', 'a', 'y',   '\0', '\0', '\0', '\0', '\0'},
{'d', 'i', 'c', 't',  '\0', '\0', '\0', '\0', '\0', '\0'},
{'k', 'e', 'y', '\0', '\0', '\0', '\0', '\0', '\0', '\0'},
{'s', 't', 'r', 'i', 'n', 'g',    '\0', '\0', '\0', '\0'},
{'d', 'a', 't', 'a',  '\0', '\0', '\0', '\0', '\0', '\0'},
{'d', 'a', 't', 'e',  '\0', '\0', '\0', '\0', '\0', '\0'},
{'r', 'e', 'a', 'l',  '\0', '\0', '\0', '\0', '\0', '\0'},
{'i', 'n', 't', 'e', 'g', 'e', 'r',     '\0', '\0', '\0'},
{'t', 'r', 'u', 'e',  '\0', '\0', '\0', '\0', '\0', '\0'},
{'f', 'a', 'l', 's', 'e',   '\0', '\0', '\0', '\0', '\0'},
{'D', 'O', 'C', 'T', 'Y', 'P', 'E',     '\0', '\0', '\0'},
{'<', '!', '[', 'C', 'D', 'A', 'T', 'A', '[',       '\0'}
};

static const UniChar CFXMLPlistTagsUnicode[13][10]= {
    {'p', 'l', 'i', 's', 't',   '\0', '\0', '\0', '\0', '\0'},
    {'a', 'r', 'r', 'a', 'y',   '\0', '\0', '\0', '\0', '\0'},
    {'d', 'i', 'c', 't',  '\0', '\0', '\0', '\0', '\0', '\0'},
    {'k', 'e', 'y', '\0', '\0', '\0', '\0', '\0', '\0', '\0'},
    {'s', 't', 'r', 'i', 'n', 'g',    '\0', '\0', '\0', '\0'},
    {'d', 'a', 't', 'a',  '\0', '\0', '\0', '\0', '\0', '\0'},
    {'d', 'a', 't', 'e',  '\0', '\0', '\0', '\0', '\0', '\0'},
    {'r', 'e', 'a', 'l',  '\0', '\0', '\0', '\0', '\0', '\0'},
    {'i', 'n', 't', 'e', 'g', 'e', 'r',     '\0', '\0', '\0'},
    {'t', 'r', 'u', 'e',  '\0', '\0', '\0', '\0', '\0', '\0'},
    {'f', 'a', 'l', 's', 'e',   '\0', '\0', '\0', '\0', '\0'},
    {'D', 'O', 'C', 'T', 'Y', 'P', 'E',     '\0', '\0', '\0'},
    {'<', '!', '[', 'C', 'D', 'A', 'T', 'A', '[',       '\0'}
};

typedef struct {
    const char *begin; // first character of the XML to be parsed
    const char *curr;  // current parse location
    const char *end;   // the first character _after_ the end of the XML
    CFErrorRef error;
    CFAllocatorRef allocator;
    UInt32 mutabilityOption;
    CFBurstTrieRef stringTrie; // map of cached strings
    CFMutableArrayRef stringCache; // retaining array of strings
    Boolean allowNewTypes; // Whether to allow the new types supported by XML property lists, but not by the old, OPENSTEP ASCII property lists (CFNumber, CFBoolean, CFDate)
    CFSetRef keyPaths; // if NULL, no filtering
    Boolean skip; // if true, do not create any objects.
} _CFXMLPlistParseInfo;

CF_PRIVATE CFTypeRef __CFCreateOldStylePropertyListOrStringsFile(CFAllocatorRef allocator, CFDataRef xmlData, CFStringRef originalString, CFStringEncoding guessedEncoding, CFOptionFlags option, CFErrorRef *outError,CFPropertyListFormat *format);

CF_INLINE void __CFPListRelease(CFTypeRef cf, CFAllocatorRef allocator) {
    if (cf && !(0)) CFRelease(cf);
}


// The following set of _plist... functions append various things to a mutable data which is in UTF8 encoding. These are pretty general. Assumption is call characters and CFStrings can be converted to UTF8 and appeneded.

// Null-terminated, ASCII or UTF8 string
//
static void _plistAppendUTF8CString(CFMutableDataRef mData, const char *cString) {
    CFDataAppendBytes (mData, (const UInt8 *)cString, strlen(cString));
}

// UniChars
//
static void _plistAppendCharacters(CFMutableDataRef mData, const UniChar *chars, CFIndex length) {
    CFIndex curLoc = 0;

    do {	// Flush out ASCII chars, BUFLEN at a time
        #define BUFLEN 400
	UInt8 buf[BUFLEN], *bufPtr = buf;
	CFIndex cnt = 0;
        while (cnt < length && (cnt - curLoc < BUFLEN) && (chars[cnt] < 128)) *bufPtr++ = (UInt8)(chars[cnt++]);
        if (cnt > curLoc) {	// Flush any ASCII bytes
            CFDataAppendBytes(mData, buf, cnt - curLoc);
            curLoc = cnt;
        }
    } while (curLoc < length && (chars[curLoc] < 128));	// We will exit out of here when we run out of chars or hit a non-ASCII char

    if (curLoc < length) {	// Now deal with non-ASCII chars
        CFDataRef data = NULL;
        CFStringRef str = NULL;
        if ((str = CFStringCreateWithCharactersNoCopy(kCFAllocatorSystemDefault, chars + curLoc, length - curLoc, kCFAllocatorNull))) {
            if ((data = CFStringCreateExternalRepresentation(kCFAllocatorSystemDefault, str, kCFStringEncodingUTF8, 0))) {
                CFDataAppendBytes (mData, CFDataGetBytePtr(data), CFDataGetLength(data));
                CFRelease(data);
            }
            CFRelease(str);
        }
        CFAssert1(str && data, __kCFLogAssertion, "%s(): Error writing plist", __PRETTY_FUNCTION__); 
    }
}

// Append CFString
//
static void _plistAppendString(CFMutableDataRef mData, CFStringRef str) {
    const UniChar *chars;
    const char *cStr;
    CFDataRef data;
    if ((chars = CFStringGetCharactersPtr(str))) {
        _plistAppendCharacters(mData, chars, CFStringGetLength(str));
    } else if ((cStr = CFStringGetCStringPtr(str, kCFStringEncodingASCII)) || (cStr = CFStringGetCStringPtr(str, kCFStringEncodingUTF8))) {
        _plistAppendUTF8CString(mData, cStr);
    } else if ((data = CFStringCreateExternalRepresentation(kCFAllocatorSystemDefault, str, kCFStringEncodingUTF8, 0))) {
        CFDataAppendBytes (mData, CFDataGetBytePtr(data), CFDataGetLength(data));
        CFRelease(data);
    } else {
	CFAssert1(TRUE, __kCFLogAssertion, "%s(): Error in plist writing", __PRETTY_FUNCTION__);
    }
}


// Append CFString-style format + arguments
//
static void _plistAppendFormat(CFMutableDataRef mData, CFStringRef format, ...) {
    CFStringRef fStr; 
    va_list argList;

    va_start(argList, format);
    fStr = CFStringCreateWithFormatAndArguments(kCFAllocatorSystemDefault, NULL, format, argList);
    va_end(argList);

    CFAssert1(fStr, __kCFLogAssertion, "%s(): Error writing plist", __PRETTY_FUNCTION__);
    _plistAppendString(mData, fStr);
    CFRelease(fStr);
}



static void _appendIndents(CFIndex numIndents, CFMutableDataRef str) {
#define NUMTABS 4
    static const UniChar tabs[NUMTABS] = {'\t','\t','\t','\t'};
    for (; numIndents > 0; numIndents -= NUMTABS) _plistAppendCharacters(str, tabs, (numIndents >= NUMTABS) ? NUMTABS : numIndents);
}

/* Append the escaped version of origStr to mStr.
*/
static void _appendEscapedString(CFStringRef origStr, CFMutableDataRef mStr) {
#define BUFSIZE 64
    CFIndex i, length = CFStringGetLength(origStr);
    CFIndex bufCnt = 0;
    UniChar buf[BUFSIZE];
    CFStringInlineBuffer inlineBuffer;

    CFStringInitInlineBuffer(origStr, &inlineBuffer, CFRangeMake(0, length));

    for (i = 0; i < length; i ++) {
	UniChar ch = __CFStringGetCharacterFromInlineBufferQuick(&inlineBuffer, i);
	if (CFStringIsSurrogateHighCharacter(ch) && (bufCnt + 2 >= BUFSIZE)) {
	    // flush the buffer first so we have room for a low/high pair and do not split them
	    _plistAppendCharacters(mStr, buf, bufCnt);
	    bufCnt = 0;
	}
	
        switch(ch) {
            case '<':
		if (bufCnt) _plistAppendCharacters(mStr, buf, bufCnt);
		bufCnt = 0;
	  	_plistAppendUTF8CString(mStr, "&lt;");
                break;
            case '>':
		if (bufCnt) _plistAppendCharacters(mStr, buf, bufCnt);
		bufCnt = 0;
	  	_plistAppendUTF8CString(mStr, "&gt;");
                break;
            case '&':
		if (bufCnt) _plistAppendCharacters(mStr, buf, bufCnt);
		bufCnt = 0;
	  	_plistAppendUTF8CString(mStr, "&amp;");
                break;
            default:
		buf[bufCnt++] = ch;
		if (bufCnt == BUFSIZE) {
		    _plistAppendCharacters(mStr, buf, bufCnt);
		    bufCnt = 0;
		}
		break;
        }
    }
    if (bufCnt) _plistAppendCharacters(mStr, buf, bufCnt);
}



/* Base-64 encoding/decoding */

/* The base-64 encoding packs three 8-bit bytes into four 7-bit ASCII
 * characters.  If the number of bytes in the original data isn't divisable
 * by three, "=" characters are used to pad the encoded data.  The complete
 * set of characters used in base-64 are:
 *
 *      'A'..'Z' => 00..25
 *      'a'..'z' => 26..51
 *      '0'..'9' => 52..61
 *      '+'      => 62
 *      '/'      => 63
 *      '='      => pad
 */

// Write the inputData to the mData using Base 64 encoding

static void _XMLPlistAppendDataUsingBase64(CFMutableDataRef mData, CFDataRef inputData, CFIndex indent) {
    static const char __CFPLDataEncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    #define MAXLINELEN 76
    char buf[MAXLINELEN + 4 + 2];	// For the slop and carriage return and terminating NULL
    
    const uint8_t *bytes = CFDataGetBytePtr(inputData);
    CFIndex length = CFDataGetLength(inputData);
    CFIndex i, pos;
    const uint8_t *p;

    if (indent > 8) indent = 8; // refuse to indent more than 64 characters

    pos = 0;		// position within buf

    for (i = 0, p = bytes; i < length; i++, p++) {
        /* 3 bytes are encoded as 4 */
        switch (i % 3) {
            case 0:
                buf[pos++] = __CFPLDataEncodeTable [ ((p[0] >> 2) & 0x3f)];
                break;
            case 1:
                buf[pos++] = __CFPLDataEncodeTable [ ((((p[-1] << 8) | p[0]) >> 4) & 0x3f)];
                break;
            case 2:
                buf[pos++] = __CFPLDataEncodeTable [ ((((p[-1] << 8) | p[0]) >> 6) & 0x3f)];
                buf[pos++] = __CFPLDataEncodeTable [ (p[0] & 0x3f)];
                break;
        }
        /* Flush the line out every 76 (or fewer) chars --- indents count against the line length*/
        if (pos >= MAXLINELEN - 8 * indent) {
            buf[pos++] = '\n';
            buf[pos++] = 0;
            _appendIndents(indent, mData);
            _plistAppendUTF8CString(mData, buf);
            pos = 0;
        }
    }
        
    switch (i % 3) {
	case 0:
            break;
	case 1:
            buf[pos++] = __CFPLDataEncodeTable [ ((p[-1] << 4) & 0x30)];
            buf[pos++] = '=';
            buf[pos++] = '=';
            break;
	case 2:
            buf[pos++] =  __CFPLDataEncodeTable [ ((p[-1] << 2) & 0x3c)];
            buf[pos++] = '=';
            break;
    }
    
    if (pos > 0) {
        buf[pos++] = '\n';
        buf[pos++] = 0;
        _appendIndents(indent, mData);
        _plistAppendUTF8CString(mData, buf);
    }
}

extern CFStringRef __CFNumberCopyFormattingDescriptionAsFloat64(CFTypeRef cf);

static void _CFAppendXML0(CFTypeRef object, UInt32 indentation, CFMutableDataRef xmlString) {
    UInt32 typeID = CFGetTypeID(object);
    _appendIndents(indentation, xmlString);
    if (typeID == stringtype) {
        _plistAppendUTF8CString(xmlString, "<");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[STRING_IX], STRING_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">");
	_appendEscapedString((CFStringRef)object, xmlString);
        _plistAppendUTF8CString(xmlString, "</");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[STRING_IX], STRING_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
    } else if (typeID == arraytype) {
        UInt32 i, count = CFArrayGetCount((CFArrayRef)object);
        if (count == 0) {
            _plistAppendUTF8CString(xmlString, "<");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[ARRAY_IX], ARRAY_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, "/>\n");
            return;
        }
        _plistAppendUTF8CString(xmlString, "<");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[ARRAY_IX], ARRAY_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
        for (i = 0; i < count; i ++) {
            _CFAppendXML0(CFArrayGetValueAtIndex((CFArrayRef)object, i), indentation+1, xmlString);
        }
        _appendIndents(indentation, xmlString);
        _plistAppendUTF8CString(xmlString, "</");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[ARRAY_IX], ARRAY_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
    } else if (typeID == dicttype) {
        UInt32 i, count = CFDictionaryGetCount((CFDictionaryRef)object);
        CFMutableArrayRef keyArray;
        if (count == 0) {
            _plistAppendUTF8CString(xmlString, "<");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[DICT_IX], DICT_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, "/>\n");
            return;
        }
        _plistAppendUTF8CString(xmlString, "<");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[DICT_IX], DICT_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
        new_cftype_array(keys, count);
        CFDictionaryGetKeysAndValues((CFDictionaryRef)object, keys, NULL);
        keyArray = CFArrayCreateMutable(kCFAllocatorSystemDefault, count, &kCFTypeArrayCallBacks);
        CFArrayReplaceValues(keyArray, CFRangeMake(0, 0), keys, count);
        CFArraySortValues(keyArray, CFRangeMake(0, count), (CFComparatorFunction)CFStringCompare, NULL);
        CFArrayGetValues(keyArray, CFRangeMake(0, count), keys);
        CFRelease(keyArray);
        for (i = 0; i < count; i ++) {
            CFTypeRef key = keys[i];
            _appendIndents(indentation+1, xmlString);
            _plistAppendUTF8CString(xmlString, "<");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[KEY_IX], KEY_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, ">");
	    _appendEscapedString((CFStringRef)key, xmlString);
            _plistAppendUTF8CString(xmlString, "</");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[KEY_IX], KEY_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, ">\n");
            _CFAppendXML0(CFDictionaryGetValue((CFDictionaryRef)object, key), indentation+1, xmlString);
        }
        free_cftype_array(keys);
        _appendIndents(indentation, xmlString);
        _plistAppendUTF8CString(xmlString, "</");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[DICT_IX], DICT_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
    } else if (typeID == datatype) {
        _plistAppendUTF8CString(xmlString, "<");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[DATA_IX], DATA_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
        _XMLPlistAppendDataUsingBase64(xmlString, (CFDataRef)object, indentation);       
        _appendIndents(indentation, xmlString);
        _plistAppendUTF8CString(xmlString, "</");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[DATA_IX], DATA_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
    } else if (typeID == datetype) {
        // YYYY '-' MM '-' DD 'T' hh ':' mm ':' ss 'Z'
	int32_t y = 0, M = 0, d = 0, H = 0, m = 0, s = 0;
        CFAbsoluteTime at = CFDateGetAbsoluteTime((CFDateRef)object);
#if 0
        // Alternative to the CFAbsoluteTimeGetGregorianDate() code which works well
        struct timeval tv;
        struct tm mine;
        tv.tv_sec = floor(at + kCFAbsoluteTimeIntervalSince1970);
        gmtime_r(&tv.tv_sec, &mine);
	y = mine.tm_year + 1900;
	M = mine.tm_mon + 1;
	d = mine.tm_mday;
	H = mine.tm_hour;
	m = mine.tm_min;
	s = mine.tm_sec;
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        CFGregorianDate date = CFAbsoluteTimeGetGregorianDate(at, NULL);
#pragma GCC diagnostic pop

#if 0
if (date.year != y || date.month != M || date.day != d || date.hour != H || date.minute != m || (int32_t)date.second != s) {
    CFLog(4, CFSTR("DATE ERROR {%d, %d, %d, %d, %d, %d} != {%d,  %d, %d, %d, %d, %d}\n"), (int)date.year, (int)date.month, (int)date.day, (int)date.hour, (int)date.minute, (int32_t)date.second, y, M, d, H, m, s);
}
#endif
        y = date.year;
        M = date.month;
        d = date.day;
        H = date.hour;
        m = date.minute;
        s = (int32_t)date.second;

        _plistAppendUTF8CString(xmlString, "<");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[DATE_IX], DATE_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">");
        _plistAppendFormat(xmlString, CFSTR("%04d-%02d-%02dT%02d:%02d:%02dZ"), y, M, d, H, m, s);
        _plistAppendUTF8CString(xmlString, "</");
        _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[DATE_IX], DATE_TAG_LENGTH);
        _plistAppendUTF8CString(xmlString, ">\n");
    } else if (typeID == numbertype) {
        if (CFNumberIsFloatType((CFNumberRef)object)) {
            _plistAppendUTF8CString(xmlString, "<");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[REAL_IX], REAL_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, ">");
                CFStringRef s = __CFNumberCopyFormattingDescriptionAsFloat64(object);
                _plistAppendString(xmlString, s);
                CFRelease(s);
            _plistAppendUTF8CString(xmlString, "</");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[REAL_IX], REAL_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, ">\n");
        } else {
            _plistAppendUTF8CString(xmlString, "<");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[INTEGER_IX], INTEGER_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, ">");

            _plistAppendFormat(xmlString, CFSTR("%@"), object);

            _plistAppendUTF8CString(xmlString, "</");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[INTEGER_IX], INTEGER_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, ">\n");
        }
    } else if (typeID == booltype) {
        if (CFBooleanGetValue((CFBooleanRef)object)) {
            _plistAppendUTF8CString(xmlString, "<");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[TRUE_IX], TRUE_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, "/>\n");
        } else {
            _plistAppendUTF8CString(xmlString, "<");
            _plistAppendCharacters(xmlString, CFXMLPlistTagsUnicode[FALSE_IX], FALSE_TAG_LENGTH);
            _plistAppendUTF8CString(xmlString, "/>\n");
        }
    }
}

static void _CFGenerateXMLPropertyListToData(CFMutableDataRef xml, CFTypeRef propertyList) {
    _plistAppendUTF8CString(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE ");
    _plistAppendCharacters(xml, CFXMLPlistTagsUnicode[PLIST_IX], PLIST_TAG_LENGTH);
    _plistAppendUTF8CString(xml, " PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n<");
    _plistAppendCharacters(xml, CFXMLPlistTagsUnicode[PLIST_IX], PLIST_TAG_LENGTH);
    _plistAppendUTF8CString(xml, " version=\"1.0\">\n");

    _CFAppendXML0(propertyList, 0, xml);

    _plistAppendUTF8CString(xml, "</");
    _plistAppendCharacters(xml, CFXMLPlistTagsUnicode[PLIST_IX], PLIST_TAG_LENGTH);
    _plistAppendUTF8CString(xml, ">\n");
}

// ========================================================================
#pragma mark -
#pragma mark Exported Creation Functions

CFDataRef _CFPropertyListCreateXMLData(CFAllocatorRef allocator, CFPropertyListRef propertyList, Boolean checkValidPlist) {
    initStatics();
    CFMutableDataRef xml;
    CFAssert1(propertyList != NULL, __kCFLogAssertion, "%s(): Cannot be called with a NULL property list", __PRETTY_FUNCTION__);
    if (checkValidPlist && !CFPropertyListIsValid(propertyList, kCFPropertyListXMLFormat_v1_0)) {
        __CFAssertIsPList(propertyList);
        return NULL;
    }
    xml = CFDataCreateMutable(allocator, 0);
    _CFGenerateXMLPropertyListToData(xml, propertyList);
    return xml;
}

CFDataRef CFPropertyListCreateXMLData(CFAllocatorRef allocator, CFPropertyListRef propertyList) {
    return _CFPropertyListCreateXMLData(allocator, propertyList, true);
}

CF_EXPORT CFDataRef _CFPropertyListCreateXMLDataWithExtras(CFAllocatorRef allocator, CFPropertyListRef propertyList) {
    return _CFPropertyListCreateXMLData(allocator, propertyList, false);
}

Boolean CFPropertyListIsValid(CFPropertyListRef plist, CFPropertyListFormat format) {
    initStatics();
    CFAssert1(plist != NULL, __kCFLogAssertion, "%s(): NULL is not a property list", __PRETTY_FUNCTION__);
    CFMutableSetRef set = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, NULL);
    CFStringRef error = NULL;
    bool result = __CFPropertyListIsValidAux(plist, true, set, format, &error);
    if (error) {
#if defined(DEBUG)
	CFLog(kCFLogLevelWarning, CFSTR("CFPropertyListIsValid(): %@"), error);
#endif
	CFRelease(error);
    }
    CFRelease(set);
    return result;
}

// ========================================================================
#pragma mark -
#pragma mark Reading Plists

//
// ------------------------- Reading plists ------------------
// 

static void skipInlineDTD(_CFXMLPlistParseInfo *pInfo);
static Boolean parseXMLElement(_CFXMLPlistParseInfo *pInfo, Boolean *isKey, CFTypeRef *out);

// warning: doesn't have a good idea of Unicode line separators
static UInt32 lineNumber(_CFXMLPlistParseInfo *pInfo) {
    const char *p = pInfo->begin;
    UInt32 count = 1;
    while (p < pInfo->curr) {
        if (*p == '\r') {
            count ++;
            if (*(p + 1) == '\n')
                p ++;
        } else if (*p == '\n') {
            count ++;
        }
        p ++;
    }
    return count;
}

// warning: doesn't have a good idea of Unicode white space
CF_INLINE void skipWhitespace(_CFXMLPlistParseInfo *pInfo) {
    while (pInfo->curr < pInfo->end) {
        switch (*(pInfo->curr)) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                pInfo->curr ++;
                continue;
            default:
                return;
        }
    }
}

/* All of these advance to the end of the given construct and return a pointer to the first character beyond the construct.  If the construct doesn't parse properly, NULL is returned. */

// pInfo should be just past "<!--"
static void skipXMLComment(_CFXMLPlistParseInfo *pInfo) {
    const char *p = pInfo->curr;
    const char *end = pInfo->end - 3; // Need at least 3 characters to compare against
    while (p < end) {
        if (*p == '-' && *(p+1) == '-' && *(p+2) == '>') {
            pInfo->curr = p+3;
            return;
        }
        p ++; 
    }
    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unterminated comment started on line %d"), lineNumber(pInfo));
}

// pInfo should be set to the first character after "<?"
static void skipXMLProcessingInstruction(_CFXMLPlistParseInfo *pInfo) {
    const char *begin = pInfo->curr, *end = pInfo->end - 2; // Looking for "?>" so we need at least 2 characters
    while (pInfo->curr < end) {
        if (*(pInfo->curr) == '?' && *(pInfo->curr+1) == '>') {
            pInfo->curr += 2;
            return;
        }
        pInfo->curr ++; 
    }
    pInfo->curr = begin;
    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF while parsing the processing instruction begun on line %d"), lineNumber(pInfo));
}

// first character should be immediately after the "<!"
static void skipDTD(_CFXMLPlistParseInfo *pInfo) {
    // First pass "DOCTYPE"
    if (pInfo->end - pInfo->curr < DOCTYPE_TAG_LENGTH || memcmp(pInfo->curr, CFXMLPlistTags[DOCTYPE_IX], DOCTYPE_TAG_LENGTH)) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Malformed DTD on line %d"), lineNumber(pInfo));
        return;
    }
    pInfo->curr += DOCTYPE_TAG_LENGTH;
    skipWhitespace(pInfo);

    // Look for either the beginning of a complex DTD or the end of the DOCTYPE structure
    while (pInfo->curr < pInfo->end) {
        char ch = *(pInfo->curr);
        if (ch == '[') break; // inline DTD
        if (ch == '>') {  // End of the DTD
            pInfo->curr ++;
            return;
        }
        pInfo->curr ++;
    }
    if (pInfo->curr == pInfo->end) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF while parsing DTD"));
        return;
    }

    // *Sigh* Must parse in-line DTD
    skipInlineDTD(pInfo);
    if (pInfo->error)  return;
    skipWhitespace(pInfo);
    if (pInfo->error) return;
    if (pInfo->curr < pInfo->end) {
        if (*(pInfo->curr) == '>') {
            pInfo->curr ++;
        } else {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c on line %d while parsing DTD"), *(pInfo->curr), lineNumber(pInfo));
        }
    } else {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF while parsing DTD"));
    }
}

static void skipPERef(_CFXMLPlistParseInfo *pInfo) {
    const char *p = pInfo->curr;
    while (p < pInfo->end) {
        if (*p == ';') {
            pInfo->curr = p+1;
            return;
        }
        p ++;
    }
    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF while parsing percent-escape sequence begun on line %d"), lineNumber(pInfo));
}

// First character should be just past '['
static void skipInlineDTD(_CFXMLPlistParseInfo *pInfo) {
    while (!pInfo->error && pInfo->curr < pInfo->end) {
        UniChar ch;
        skipWhitespace(pInfo);
        ch = *pInfo->curr;
        if (ch == '%') {
            pInfo->curr ++;
            skipPERef(pInfo);
        } else if (ch == '<') {
            pInfo->curr ++;
            if (pInfo->curr >= pInfo->end) {
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF while parsing inline DTD"));
                return;
            }
            ch = *(pInfo->curr);
            if (ch == '?') {
                pInfo->curr ++;
                skipXMLProcessingInstruction(pInfo);
            } else if (ch == '!') {
                if (pInfo->curr + 2 < pInfo->end && (*(pInfo->curr+1) == '-' && *(pInfo->curr+2) == '-')) {
                    pInfo->curr += 3;
                    skipXMLComment(pInfo);
                } else {
                    // Skip the myriad of DTD declarations of the form "<!string" ... ">"
                    pInfo->curr ++; // Past both '<' and '!'
                    while (pInfo->curr < pInfo->end) {
                        if (*(pInfo->curr) == '>') break;
                        pInfo->curr ++;
                    }
                    if (*(pInfo->curr) != '>') {
                        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF while parsing inline DTD"));
                        return;
                    }
                    pInfo->curr ++;
                }
            } else {
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c on line %d while parsing inline DTD"), ch, lineNumber(pInfo));
                return;
            }
        } else if (ch == ']') {
            pInfo->curr ++;
            return;
        } else {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c on line %d while parsing inline DTD"), ch, lineNumber(pInfo));
            return;
        }
    }
    if (!pInfo->error) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF while parsing inline DTD"));
    }
}

// content ::== (element | CharData | Reference | CDSect | PI | Comment)*
// In the context of a plist, CharData, Reference and CDSect are not legal (they all resolve to strings).  Skipping whitespace, then, the next character should be '<'.  From there, we figure out which of the three remaining cases we have (element, PI, or Comment).
static Boolean getContentObject(_CFXMLPlistParseInfo *pInfo, Boolean *isKey, CFTypeRef *out) {
    if (isKey) *isKey = false;
    while (!pInfo->error && pInfo->curr < pInfo->end) {
        skipWhitespace(pInfo);
        if (pInfo->curr >= pInfo->end) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
            return false;
        }
        if (*(pInfo->curr) != '<') {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c on line %d while looking for open tag"), *(pInfo->curr), lineNumber(pInfo));
            return false;
        }
        pInfo->curr ++;
        if (pInfo->curr >= pInfo->end) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
            return false;
        }
        switch (*(pInfo->curr)) {
            case '?':
                // Processing instruction
                skipXMLProcessingInstruction(pInfo);
                break;
            case '!':
                // Could be a comment
                if (pInfo->curr+2 >= pInfo->end) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
                    return false;
                }
                if (*(pInfo->curr+1) == '-' && *(pInfo->curr+2) == '-') {
                    pInfo->curr += 2;
                    skipXMLComment(pInfo);
                } else {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
                    return false;
                }
                break;
            case '/':
                // Whoops!  Looks like we got to the end tag for the element whose content we're parsing
                pInfo->curr --; // Back off to the '<'
                return false;
            default:
                // Should be an element
                return parseXMLElement(pInfo, isKey, out);
        }
    }
    // Do not set the error string here; if it wasn't already set by one of the recursive parsing calls, the caller will quickly detect the failure (b/c pInfo->curr >= pInfo->end) and provide a more useful one of the form "end tag for <blah> not found"
    return false;
}

static void parseCDSect_pl(_CFXMLPlistParseInfo *pInfo, CFMutableDataRef stringData) {
    const char *end, *begin;
    if (pInfo->end - pInfo->curr < CDSECT_TAG_LENGTH) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
        return;
    }
    if (memcmp(pInfo->curr, CFXMLPlistTags[CDSECT_IX], CDSECT_TAG_LENGTH)) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered improper CDATA opening at line %d"), lineNumber(pInfo));
        return;
    }
    pInfo->curr += CDSECT_TAG_LENGTH;
    begin = pInfo->curr; // Marks the first character of the CDATA content
    end = pInfo->end-2; // So we can safely look 2 characters beyond p
    while (pInfo->curr < end) {
        if (*(pInfo->curr) == ']' && *(pInfo->curr+1) == ']' && *(pInfo->curr+2) == '>') {
            // Found the end!
            CFDataAppendBytes(stringData, (const UInt8 *)begin, pInfo->curr-begin);
            pInfo->curr += 3;
            return;
        }
        pInfo->curr ++;
    }
    // Never found the end mark
    pInfo->curr = begin;
    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Could not find end of CDATA started on line %d"), lineNumber(pInfo));
}

// Only legal references are {lt, gt, amp, apos, quote, #ddd, #xAAA}
static void parseEntityReference_pl(_CFXMLPlistParseInfo *pInfo, CFMutableDataRef stringData) {
    int len;
    pInfo->curr ++; // move past the '&';
    len = pInfo->end - pInfo->curr; // how many bytes we can safely scan
    if (len < 1) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
        return;
    }

    char ch;
    switch (*(pInfo->curr)) {
        case 'l':  // "lt"
            if (len >= 3 && *(pInfo->curr+1) == 't' && *(pInfo->curr+2) == ';') {
                ch = '<';
                pInfo->curr += 3;
                break;
            }
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(pInfo));
            return;
        case 'g': // "gt"
            if (len >= 3 && *(pInfo->curr+1) == 't' && *(pInfo->curr+2) == ';') {
                ch = '>';
                pInfo->curr += 3;
                break;
            }
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(pInfo));
            return;
        case 'a': // "apos" or "amp"
            if (len < 4) {   // Not enough characters for either conversion
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
                return;
            }
            if (*(pInfo->curr+1) == 'm') {
                // "amp"
                if (*(pInfo->curr+2) == 'p' && *(pInfo->curr+3) == ';') {
                    ch = '&';
                    pInfo->curr += 4;
                    break;
                }
            } else if (*(pInfo->curr+1) == 'p') {
                // "apos"
                if (len > 4 && *(pInfo->curr+2) == 'o' && *(pInfo->curr+3) == 's' && *(pInfo->curr+4) == ';') {
                    ch = '\'';
                    pInfo->curr += 5;
                    break;
                }
            }
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(pInfo));
            return;
        case 'q':  // "quote"
            if (len >= 5 && *(pInfo->curr+1) == 'u' && *(pInfo->curr+2) == 'o' && *(pInfo->curr+3) == 't' && *(pInfo->curr+4) == ';') {
                ch = '\"';
                pInfo->curr += 5;
                break;
            }
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(pInfo));
            return;
        case '#':
        {
            uint16_t num = 0;
            Boolean isHex = false;
            if ( len < 4) {  // Not enough characters to make it all fit!  Need at least "&#d;"
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
                return;
            }
            pInfo->curr ++;
            if (*(pInfo->curr) == 'x') {
                isHex = true;
                pInfo->curr ++;
            }
            while (pInfo->curr < pInfo->end) {
                ch = *(pInfo->curr);
                pInfo->curr ++;
                if (ch == ';') {
                    // The value in num always refers to the unicode code point. We'll have to convert since the calling function expects UTF8 data.
                    CFStringRef oneChar = CFStringCreateWithBytes(pInfo->allocator, (const uint8_t *)&num, 2, kCFStringEncodingUnicode, NO);
                    uint8_t tmpBuf[6]; // max of 6 bytes for UTF8
                    CFIndex tmpBufLength = 0;
                    CFStringGetBytes(oneChar, CFRangeMake(0, 1), kCFStringEncodingUTF8, 0, NO, tmpBuf, 6, &tmpBufLength);
                    CFDataAppendBytes(stringData, tmpBuf, tmpBufLength);
                    __CFPListRelease(oneChar, pInfo->allocator);
                    return;
                }
                if (!isHex) num = num*10;
                else num = num << 4;
                if (ch <= '9' && ch >= '0') {
                    num += (ch - '0');
                } else if (!isHex) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c at line %d while parsing data"), ch, lineNumber(pInfo));
                    return;
                } else if (ch >= 'a' && ch <= 'f') {
                    num += 10 + (ch - 'a');
                } else if (ch >= 'A' && ch <= 'F') {
                    num += 10 + (ch - 'A');
                } else {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c at line %d while parsing data"), ch, lineNumber(pInfo));
                    return;                    
                }
            }
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
            return;
        }
        default:
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(pInfo));
            return;
    }
    CFDataAppendBytes(stringData, (const UInt8 *)&ch, 1);
}

static void _createStringMap(_CFXMLPlistParseInfo *pInfo) {
    pInfo->stringTrie = CFBurstTrieCreate();
    pInfo->stringCache = CFArrayCreateMutable(pInfo->allocator, 0, &kCFTypeArrayCallBacks);
}

static void _cleanupStringMap(_CFXMLPlistParseInfo *pInfo) {
    CFBurstTrieRelease(pInfo->stringTrie);
    CFRelease(pInfo->stringCache);
    pInfo->stringTrie = NULL;
    pInfo->stringCache = NULL;
}

static CFStringRef _createUniqueStringWithUTF8Bytes(_CFXMLPlistParseInfo *pInfo, const char *base, CFIndex length) {
    if (length == 0) return !(0) ? (CFStringRef)CFRetain(CFSTR("")) : CFSTR("");    
    
    CFStringRef result = NULL;
    uint32_t payload = 0;
    Boolean uniqued = CFBurstTrieContainsUTF8String(pInfo->stringTrie, (UInt8 *)base, length, &payload);
    if (uniqued && payload > 0) {
        // The index is at payload - 1 (see below).
        result = (CFStringRef)CFArrayGetValueAtIndex(pInfo->stringCache, (CFIndex)payload - 1);
        CFRetain(result);
    } else {
        result = CFStringCreateWithBytes(pInfo->allocator, (const UInt8 *)base, length, kCFStringEncodingUTF8, NO);
        if (!result) return NULL;
        // Payload must be >0, so the actual index of the value is at payload - 1
        // We also get add to the array after we make sure that CFBurstTrieAddUTF8String succeeds (it can fail, if the string is too large, for example)
        payload = CFArrayGetCount(pInfo->stringCache) + 1;
        Boolean didAddToBurstTrie = CFBurstTrieAddUTF8String(pInfo->stringTrie, (UInt8 *)base, length, payload);
        if (didAddToBurstTrie) {
            CFArrayAppendValue(pInfo->stringCache, result);
        }
    }
    return result;
}

// String could be comprised of characters, CDSects, or references to one of the "well-known" entities ('<', '>', '&', ''', '"')
static Boolean parseStringTag(_CFXMLPlistParseInfo *pInfo, CFStringRef *out) {
    const char *mark = pInfo->curr;
    CFMutableDataRef stringData = NULL;
    while (!pInfo->error && pInfo->curr < pInfo->end) {
        char ch = *(pInfo->curr);
        if (ch == '<') {
	    if (pInfo->curr + 1 >= pInfo->end) break;
            // Could be a CDSect; could be the end of the string
            if (*(pInfo->curr+1) != '!') break; // End of the string
            if (!stringData) stringData = CFDataCreateMutable(pInfo->allocator, 0);
            CFDataAppendBytes(stringData, (const UInt8 *)mark, pInfo->curr - mark);
            parseCDSect_pl(pInfo, stringData); // TODO: move to return boolean
            mark = pInfo->curr;
        } else if (ch == '&') {
            if (!stringData) stringData = CFDataCreateMutable(pInfo->allocator, 0);
            CFDataAppendBytes(stringData, (const UInt8 *)mark, pInfo->curr - mark);
            parseEntityReference_pl(pInfo, stringData); // TODO: move to return boolean
            mark = pInfo->curr;
        } else {
            pInfo->curr ++;
        }
    }

    if (pInfo->error) {
        __CFPListRelease(stringData, pInfo->allocator);
        return false;
    }
        
    if (!stringData) {
        if (pInfo->skip) {
            *out = NULL;
        } else {
            if (pInfo->mutabilityOption != kCFPropertyListMutableContainersAndLeaves) {
                CFStringRef s = _createUniqueStringWithUTF8Bytes(pInfo, mark, pInfo->curr - mark);
                if (!s) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to convert string to correct encoding"));
                    return false;
                }                    
                *out = s;
            } else {
                CFStringRef s = CFStringCreateWithBytes(pInfo->allocator, (const UInt8 *)mark, pInfo->curr - mark, kCFStringEncodingUTF8, NO);
                if (!s) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to convert string to correct encoding"));
                    return false;
                }
                *out = CFStringCreateMutableCopy(pInfo->allocator, 0, s);
                __CFPListRelease(s, pInfo->allocator);
            }
        }
        return true;
    } else {
        if (pInfo->skip) {
            *out = NULL;
        } else {
            CFDataAppendBytes(stringData, (const UInt8 *)mark, pInfo->curr - mark);
            if (pInfo->mutabilityOption != kCFPropertyListMutableContainersAndLeaves) {
                CFStringRef s = _createUniqueStringWithUTF8Bytes(pInfo, (const char *)CFDataGetBytePtr(stringData), CFDataGetLength(stringData));
                if (!s) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to convert string to correct encoding"));
                    return false;
                }
                *out = s;
            } else {
                CFStringRef s = CFStringCreateWithBytes(pInfo->allocator, (const UInt8 *)CFDataGetBytePtr(stringData), CFDataGetLength(stringData), kCFStringEncodingUTF8, NO);
                if (!s) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to convert string to correct encoding"));
                    return false;
                }
                *out = CFStringCreateMutableCopy(pInfo->allocator, 0, s);
                __CFPListRelease(s, pInfo->allocator);
            }
        }
        __CFPListRelease(stringData, pInfo->allocator);
        return true;
    }
}

static Boolean checkForCloseTag(_CFXMLPlistParseInfo *pInfo, const char *tag, CFIndex tagLen) {
    if (pInfo->end - pInfo->curr < tagLen + 3) {
        if (!pInfo->error) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
        }
        return false;
    }
    if (*(pInfo->curr) != '<' || *(++pInfo->curr) != '/') {
        if (!pInfo->error) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c on line %d while looking for close tag"), *(pInfo->curr), lineNumber(pInfo));
        }
        return false;
    }
    pInfo->curr ++;
    if (memcmp(pInfo->curr, tag, tagLen)) {
        CFStringRef str = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (const UInt8 *)tag, tagLen, kCFStringEncodingUTF8, NO);
        if (!pInfo->error) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Close tag on line %d does not match open tag %@"), lineNumber(pInfo), str);
        }
        CFRelease(str);
        return false;
    }
    pInfo->curr += tagLen;
    skipWhitespace(pInfo);
    if (pInfo->curr == pInfo->end) {
        if (!pInfo->error) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
        }
        return false;
    }
    if (*(pInfo->curr) != '>') {
        if (!pInfo->error) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected character %c on line %d while looking for close tag"), *(pInfo->curr), lineNumber(pInfo));
        }
        return false;
    }
    pInfo->curr ++;
    return true;
}

// pInfo should be set to the first content character of the <plist>
static Boolean parsePListTag(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    CFTypeRef result = NULL;
    if (!getContentObject(pInfo, NULL, &result)) {
        if (!pInfo->error) pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty plist tag"));
        return false;
    }
    const char *save = pInfo->curr; // Save this in case the next step fails
    CFTypeRef tmp = NULL;
    if (getContentObject(pInfo, NULL, &tmp)) {
        // Got an extra object
        __CFPListRelease(tmp, pInfo->allocator);
        __CFPListRelease(result, pInfo->allocator);
        pInfo->curr = save;
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected element at line %d (plist can only include one object)"), lineNumber(pInfo));
        return false;
    }
    if (pInfo->error) {
        // Parse failed catastrophically
        __CFPListRelease(result, pInfo->allocator);
        return false;
    }
    if (!checkForCloseTag(pInfo, CFXMLPlistTags[PLIST_IX], PLIST_TAG_LENGTH)) {
        __CFPListRelease(result, pInfo->allocator);
        return false;
    }
    *out = result;
    return true;
}

static int allowImmutableCollections = -1;

static void checkImmutableCollections(void) {
    allowImmutableCollections = (NULL == __CFgetenv("CFPropertyListAllowImmutableCollections")) ? 0 : 1;
}

// This converts the input value, a set of strings, into the form that's efficient for using during recursive decent parsing, a set of arrays
static CFSetRef createTopLevelKeypaths(CFAllocatorRef allocator, CFSetRef keyPaths) {
    if (!keyPaths) return NULL;
    
    CFIndex count = CFSetGetCount(keyPaths);
    new_cftype_array(keyPathValues, count);
    CFSetGetValues(keyPaths, keyPathValues);
    CFMutableSetRef splitKeyPathSet = CFSetCreateMutable(allocator, count, &kCFTypeSetCallBacks);
    for (CFIndex i = 0; i < count; i++) {
        // Split each key path and add it to the split path set, which we will reference throughout parsing
        CFArrayRef split = CFStringCreateArrayBySeparatingStrings(allocator, (CFStringRef)(keyPathValues[i]), CFSTR(":"));
        CFSetAddValue(splitKeyPathSet, split);
        __CFPListRelease(split, allocator);
    }
    free_cftype_array(keyPathValues);
    return splitKeyPathSet;
}

// This splits up the keypaths into the ones relevant for this level (of array or dictionary), and the ones for the next level (of array or dictionary)
CF_PRIVATE void __CFPropertyListCreateSplitKeypaths(CFAllocatorRef allocator, CFSetRef currentKeys, CFSetRef *theseKeys, CFSetRef *nextKeys) {
    if (!currentKeys) { *theseKeys = NULL; *nextKeys = NULL; return; }
    
    CFIndex count = CFSetGetCount(currentKeys);
    
    // For each array in the current key path set, grab the item at the start of the list and put it into theseKeys. The rest of the array goes into nextKeys.
    CFMutableSetRef outTheseKeys = NULL;
    CFMutableSetRef outNextKeys = NULL;
    
    new_cftype_array(currentKeyPaths, count);
    CFSetGetValues(currentKeys, currentKeyPaths);
    for (CFIndex i = 0; i < count; i++) {
        CFArrayRef oneKeyPath = (CFArrayRef)currentKeyPaths[i];
        CFIndex keyPathCount = CFArrayGetCount(oneKeyPath);
        
        if (keyPathCount > 0) {
            if (!outTheseKeys) outTheseKeys = CFSetCreateMutable(allocator, 0, &kCFTypeSetCallBacks);
            
            CFSetAddValue(outTheseKeys, CFArrayGetValueAtIndex(oneKeyPath, 0));
        }
        
        if (keyPathCount > 1) {
            if (!outNextKeys) outNextKeys = CFSetCreateMutable(allocator, 0, &kCFTypeSetCallBacks);
            
            // Create an array with values from 1 - end of list
            new_cftype_array(restOfKeys, keyPathCount - 1);
            CFArrayGetValues(oneKeyPath, CFRangeMake(1, CFArrayGetCount(oneKeyPath) - 1), restOfKeys);
            CFArrayRef newNextKeys = CFArrayCreate(allocator, restOfKeys, CFArrayGetCount(oneKeyPath) - 1, &kCFTypeArrayCallBacks);
            CFSetAddValue(outNextKeys, newNextKeys);
            __CFPListRelease(newNextKeys, allocator);
            free_cftype_array(restOfKeys);
            
        }
    }
    
    *theseKeys = outTheseKeys;
    *nextKeys = outNextKeys;
}

static Boolean parseArrayTag(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    CFTypeRef tmp = NULL;

    if (pInfo->skip) {
        Boolean result = getContentObject(pInfo, NULL, &tmp);
        while (result) {
            if (tmp) {
                // Shouldn't happen (if skipping, all content values should be null), but just in case
                __CFPListRelease(tmp, pInfo->allocator);
            }
            result = getContentObject(pInfo, NULL, &tmp);
        }
        
        if (pInfo->error) {
            // getContentObject encountered a parse error
            return false;
        }
        if (!checkForCloseTag(pInfo, CFXMLPlistTags[ARRAY_IX], ARRAY_TAG_LENGTH)) {
            return false;
        } else {
            *out = NULL;
            return true;
        }
    }
    
    CFMutableArrayRef array = CFArrayCreateMutable(pInfo->allocator, 0, &kCFTypeArrayCallBacks);
    Boolean result;
    
    CFIndex count = 0;
    CFSetRef oldKeyPaths = pInfo->keyPaths;
    CFSetRef newKeyPaths, keys;
    __CFPropertyListCreateSplitKeypaths(pInfo->allocator, pInfo->keyPaths, &keys, &newKeyPaths);
    
    if (keys) {
        CFStringRef countString = CFStringCreateWithFormat(pInfo->allocator, NULL, CFSTR("%ld"), count);
        if (!CFSetContainsValue(keys, countString)) pInfo->skip = true;
        __CFPListRelease(countString, pInfo->allocator);
        count++;
        pInfo->keyPaths = newKeyPaths;
    }
    result = getContentObject(pInfo, NULL, &tmp);
    if (keys) {
        pInfo->keyPaths = oldKeyPaths;
        pInfo->skip = false;
    }

    while (result) {
        if (tmp) {
            CFArrayAppendValue(array, tmp);
            __CFPListRelease(tmp, pInfo->allocator);
        }
        
        if (keys) {
            // prep for getting next object
            CFStringRef countString = CFStringCreateWithFormat(pInfo->allocator, NULL, CFSTR("%ld"), count);
            if (!CFSetContainsValue(keys, countString)) pInfo->skip = true;
            __CFPListRelease(countString, pInfo->allocator);
            count++;
            pInfo->keyPaths = newKeyPaths;
        }
        result = getContentObject(pInfo, NULL, &tmp);
        if (keys) {
            // reset after getting object
            pInfo->keyPaths = oldKeyPaths;
            pInfo->skip = false;
        }

    }
    
    __CFPListRelease(newKeyPaths, pInfo->allocator);
    __CFPListRelease(keys, pInfo->allocator);

    if (pInfo->error) { // getContentObject encountered a parse error
        __CFPListRelease(array, pInfo->allocator);
        return false;
    }
    if (!checkForCloseTag(pInfo, CFXMLPlistTags[ARRAY_IX], ARRAY_TAG_LENGTH)) {
        __CFPListRelease(array, pInfo->allocator);
        return false;
    }
    if (-1 == allowImmutableCollections) {
        checkImmutableCollections();
    }
    if (1 == allowImmutableCollections) {
        if (pInfo->mutabilityOption == kCFPropertyListImmutable) {
            CFArrayRef newArray = CFArrayCreateCopy(pInfo->allocator, array);
            __CFPListRelease(array, pInfo->allocator);
            array = (CFMutableArrayRef)newArray;
        }
    }
    *out = array;
    return true;
}

static Boolean parseDictTag(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    Boolean gotKey;
    Boolean result;
    CFTypeRef key = NULL, value = NULL;
    
    if (pInfo->skip) {
        result = getContentObject(pInfo, &gotKey, &key);
        while (result) {
            if (!gotKey) { 
                if (!pInfo->error) pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Found non-key inside <dict> at line %d"), lineNumber(pInfo));
                return false;
            }
            result = getContentObject(pInfo, NULL, &value);
            if (!result) { 
                if (!pInfo->error) pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Value missing for key inside <dict> at line %d"), lineNumber(pInfo)); 
                return false; 
            }
            // key and value should be null, but we'll release just in case here
            __CFPListRelease(key, pInfo->allocator);
            key = NULL;
            __CFPListRelease(value, pInfo->allocator);
            value = NULL;            
            result = getContentObject(pInfo, &gotKey, &key);
        }
        if (checkForCloseTag(pInfo, CFXMLPlistTags[DICT_IX], DICT_TAG_LENGTH)) {
            *out = NULL;
            return true;
        } else {
            return false;
        }
    }
    
    CFSetRef oldKeyPaths = pInfo->keyPaths;
    CFSetRef nextKeyPaths, theseKeyPaths;
    __CFPropertyListCreateSplitKeypaths(pInfo->allocator, pInfo->keyPaths, &theseKeyPaths, &nextKeyPaths);
    
    CFMutableDictionaryRef dict = NULL;
    
    result = getContentObject(pInfo, &gotKey, &key);
    while (result && key) {
        if (!gotKey) { 
            if (!pInfo->error) pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Found non-key inside <dict> at line %d"), lineNumber(pInfo)); 
            __CFPListRelease(key, pInfo->allocator);
            __CFPListRelease(nextKeyPaths, pInfo->allocator);
            __CFPListRelease(theseKeyPaths, pInfo->allocator);
            __CFPListRelease(dict, pInfo->allocator);
            return false;
        }
        
        if (theseKeyPaths) {
            if (!CFSetContainsValue(theseKeyPaths, key)) pInfo->skip = true;
            pInfo->keyPaths = nextKeyPaths;
        }
        result = getContentObject(pInfo, NULL, &value);
        if (theseKeyPaths) {
            pInfo->keyPaths = oldKeyPaths;
            pInfo->skip = false;
        }
        
        if (!result) { 
            if (!pInfo->error) pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Value missing for key inside <dict> at line %d"), lineNumber(pInfo)); 
            __CFPListRelease(key, pInfo->allocator);
            __CFPListRelease(nextKeyPaths, pInfo->allocator);
            __CFPListRelease(theseKeyPaths, pInfo->allocator);
            __CFPListRelease(dict, pInfo->allocator);
            return false;
        }
        
        if (key && value) {
            if (NULL == dict) {
                dict = CFDictionaryCreateMutable(pInfo->allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                _CFDictionarySetCapacity(dict, 10);
            }
            CFDictionarySetValue(dict, key, value);
        }
        
        __CFPListRelease(key, pInfo->allocator);
        key = NULL;
        __CFPListRelease(value, pInfo->allocator);
        value = NULL;
        
        result = getContentObject(pInfo, &gotKey, &key);
    }
    
    __CFPListRelease(nextKeyPaths, pInfo->allocator);
    __CFPListRelease(theseKeyPaths, pInfo->allocator);

    if (checkForCloseTag(pInfo, CFXMLPlistTags[DICT_IX], DICT_TAG_LENGTH)) {
	if (NULL == dict) {
	    if (pInfo->mutabilityOption == kCFPropertyListImmutable) {
		dict = (CFMutableDictionaryRef)CFDictionaryCreate(pInfo->allocator, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	    } else {
		dict = CFDictionaryCreateMutable(pInfo->allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	    }
	} else {
	    CFIndex cnt = CFDictionaryGetCount(dict);
	    if (1 == cnt) {
		CFTypeRef val = CFDictionaryGetValue(dict, CFSTR("CF$UID"));
		if (val && CFGetTypeID(val) == numbertype) {
		    CFTypeRef uid;
		    uint32_t v;
		    CFNumberGetValue((CFNumberRef)val, kCFNumberSInt32Type, &v);
		    uid = (CFTypeRef)_CFKeyedArchiverUIDCreate(pInfo->allocator, v);
		    __CFPListRelease(dict, pInfo->allocator);
                    *out = uid;
                    return true;
		}
	    }
	    if (-1 == allowImmutableCollections) checkImmutableCollections();
	    if (1 == allowImmutableCollections) {
		if (pInfo->mutabilityOption == kCFPropertyListImmutable) {
		    CFDictionaryRef newDict = CFDictionaryCreateCopy(pInfo->allocator, dict);
		    __CFPListRelease(dict, pInfo->allocator);
		    dict = (CFMutableDictionaryRef)newDict;
		}
	    }
	}
        *out = dict;
        return true;
    }
    
    if (dict) __CFPListRelease(dict, pInfo->allocator);
    return false;
}

static Boolean parseDataTag(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    const char *base = pInfo->curr;
    static const signed char dataDecodeTable[128] = {
        /* 000 */ -1, -1, -1, -1, -1, -1, -1, -1,
        /* 010 */ -1, -1, -1, -1, -1, -1, -1, -1,
        /* 020 */ -1, -1, -1, -1, -1, -1, -1, -1,
        /* 030 */ -1, -1, -1, -1, -1, -1, -1, -1,
        /* ' ' */ -1, -1, -1, -1, -1, -1, -1, -1,
        /* '(' */ -1, -1, -1, 62, -1, -1, -1, 63,
        /* '0' */ 52, 53, 54, 55, 56, 57, 58, 59,
        /* '8' */ 60, 61, -1, -1, -1,  0, -1, -1,
        /* '@' */ -1,  0,  1,  2,  3,  4,  5,  6,
        /* 'H' */  7,  8,  9, 10, 11, 12, 13, 14,
        /* 'P' */ 15, 16, 17, 18, 19, 20, 21, 22,
        /* 'X' */ 23, 24, 25, -1, -1, -1, -1, -1,
        /* '`' */ -1, 26, 27, 28, 29, 30, 31, 32,
        /* 'h' */ 33, 34, 35, 36, 37, 38, 39, 40,
        /* 'p' */ 41, 42, 43, 44, 45, 46, 47, 48,
        /* 'x' */ 49, 50, 51, -1, -1, -1, -1, -1
    };
    
    int tmpbufpos = 0;
    int tmpbuflen = 256;
    uint8_t *tmpbuf = pInfo->skip ? NULL : (uint8_t *)CFAllocatorAllocate(pInfo->allocator, tmpbuflen, 0);
    int numeq = 0;
    int acc = 0;
    int cntr = 0;
    
    for (; pInfo->curr < pInfo->end; pInfo->curr++) {
        signed char c = *(pInfo->curr);
        if (c == '<') {
            break;
        }
        if ('=' == c) {
            numeq++;
        } else if (!isspace(c)) {
            numeq = 0;
        }
        if (dataDecodeTable[c] < 0)
            continue;
        cntr++;
        acc <<= 6;
        acc += dataDecodeTable[c];
        if (!pInfo->skip && 0 == (cntr & 0x3)) {
            if (tmpbuflen <= tmpbufpos + 2) {
                if (tmpbuflen < 256 * 1024) {
                    tmpbuflen *= 4;
                } else if (tmpbuflen < 16 * 1024 * 1024) {
                    tmpbuflen *= 2;
                } else {
                    // once in this stage, this will be really slow
                    // and really potentially fragment memory
                    tmpbuflen += 256 * 1024;
                }
                tmpbuf = (uint8_t *)CFAllocatorReallocate(pInfo->allocator, tmpbuf, tmpbuflen, 0);
                if (!tmpbuf) HALT; // out of memory
            }
            tmpbuf[tmpbufpos++] = (acc >> 16) & 0xff;
            if (numeq < 2) tmpbuf[tmpbufpos++] = (acc >> 8) & 0xff;
            if (numeq < 1) tmpbuf[tmpbufpos++] = acc & 0xff;
        }
    }
    
    CFDataRef result = NULL;
    if (!pInfo->skip) {
        if (pInfo->mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
            result = (CFDataRef)CFDataCreateMutable(pInfo->allocator, 0);
            CFDataAppendBytes((CFMutableDataRef)result, tmpbuf, tmpbufpos);
            CFAllocatorDeallocate(pInfo->allocator, tmpbuf);
        } else {
            result = CFDataCreateWithBytesNoCopy(pInfo->allocator, tmpbuf, tmpbufpos, pInfo->allocator);
        }
        if (!result) {
            pInfo->curr = base;
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Could not interpret <data> at line %d (should be base64-encoded)"), lineNumber(pInfo));
            return false;
        }
    }
    
    if (checkForCloseTag(pInfo, CFXMLPlistTags[DATA_IX], DATA_TAG_LENGTH)) {
        *out = result;
        return true;
    } else {
        __CFPListRelease(result, pInfo->allocator);
        return false;
    }
}

CF_INLINE Boolean read2DigitNumber(_CFXMLPlistParseInfo *pInfo, int32_t *result) {
    char ch1, ch2;
    if (pInfo->curr + 2 >= pInfo->end) {
        return false;
    }
    ch1 = *pInfo->curr;
    ch2 = *(pInfo->curr + 1);
    pInfo->curr += 2;
    if (!isdigit(ch1) || !isdigit(ch2)) {
        return false;
    }
    *result = (ch1 - '0')*10 + (ch2 - '0');
    return true;
}

// YYYY '-' MM '-' DD 'T' hh ':' mm ':' ss 'Z'
static Boolean parseDateTag(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    int32_t num = 0;
    Boolean badForm = false;
    Boolean yearIsNegative = false;
    
    if (pInfo->curr < pInfo->end && *pInfo->curr == '-') {
        yearIsNegative = true;
        pInfo->curr++;
    }
    
    while (pInfo->curr < pInfo->end && isdigit(*pInfo->curr)) {
        year = 10*year + (*pInfo->curr) - '0';
        pInfo->curr ++;
    }
    if (pInfo->curr >= pInfo->end || *pInfo->curr != '-') {
        badForm = true;
    } else {
        pInfo->curr ++;
    }

    if (!badForm && read2DigitNumber(pInfo, &month) && pInfo->curr < pInfo->end && *pInfo->curr == '-') {
        pInfo->curr ++;
    } else {
        badForm = true;
    }

    if (!badForm && read2DigitNumber(pInfo, &day) && pInfo->curr < pInfo->end && *pInfo->curr == 'T') {
        pInfo->curr ++;
    } else {
        badForm = true;
    }

    if (!badForm && read2DigitNumber(pInfo, &hour) && pInfo->curr < pInfo->end && *pInfo->curr == ':') {
        pInfo->curr ++;
    } else {
        badForm = true;
    }

    if (!badForm && read2DigitNumber(pInfo, &minute) && pInfo->curr < pInfo->end && *pInfo->curr == ':') {
        pInfo->curr ++;
    } else {
        badForm = true;
    }

    if (!badForm && read2DigitNumber(pInfo, &num) && pInfo->curr < pInfo->end && *pInfo->curr == 'Z') {
        second = num;
        pInfo->curr ++;
    } else {
        badForm = true;
    }

    if (badForm || !checkForCloseTag(pInfo, CFXMLPlistTags[DATE_IX], DATE_TAG_LENGTH)) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Could not interpret <date> at line %d"), lineNumber(pInfo));
        return false;
    }

    CFAbsoluteTime at = 0.0;

#if 0
    { // alternative to CFGregorianDateGetAbsoluteTime() below; also, cheaper than CFCalendar would be;
      // clearly not thread-safe with that environment variable having to be set;
      // timegm() could be used instead of mktime(), on platforms which have it
        struct tm mine;
        mine.tm_year = (yearIsNegative ? -year : year) - 1900;
        mine.tm_mon = month - 1;
        mine.tm_mday = day;
        mine.tm_hour = hour;
        mine.tm_min = minute;
        mine.tm_sec = second;
        char *tz = getenv("TZ");
        setenv("TZ", "", 1);
        tzset();
        at = mktime(tm) - kCFAbsoluteTimeIntervalSince1970;
        if (tz) {
            setenv("TZ", tz, 1);
        } else {
            unsetenv("TZ");
        }
        tzset();
    }
#endif

    // See <rdar://problem/5052483> Revisit the CFGregorianDate -> CFCalendar change in CFPropertyList.c
    // for why we can't use CFCalendar
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    CFGregorianDate date = {yearIsNegative ? -year : year, month, day, hour, minute, second};
    at = CFGregorianDateGetAbsoluteTime(date, NULL);
#pragma GCC diagnostic pop

    if (pInfo->skip) {
        *out = NULL;
    } else {
        *out = CFDateCreate(pInfo->allocator, at);
    }
    return true;
}

static Boolean parseRealTag(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    CFStringRef str = NULL;
    if (!parseStringTag(pInfo, &str)) {
        if (!pInfo->error) pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty <real> on line %d"), lineNumber(pInfo));
        return false;
    }
    
    CFNumberRef result = NULL;
    
    if (!pInfo->skip) {
        if (kCFCompareEqualTo == CFStringCompare(str, CFSTR("nan"), kCFCompareCaseInsensitive)) result = kCFNumberNaN;
        else if (kCFCompareEqualTo == CFStringCompare(str, CFSTR("+infinity"), kCFCompareCaseInsensitive)) result = kCFNumberPositiveInfinity;
        else if (kCFCompareEqualTo == CFStringCompare(str, CFSTR("-infinity"), kCFCompareCaseInsensitive)) result = kCFNumberNegativeInfinity;
        else if (kCFCompareEqualTo == CFStringCompare(str, CFSTR("infinity"), kCFCompareCaseInsensitive)) result = kCFNumberPositiveInfinity;
        else if (kCFCompareEqualTo == CFStringCompare(str, CFSTR("-inf"), kCFCompareCaseInsensitive)) result = kCFNumberNegativeInfinity;
        else if (kCFCompareEqualTo == CFStringCompare(str, CFSTR("inf"), kCFCompareCaseInsensitive)) result = kCFNumberPositiveInfinity;
        else if (kCFCompareEqualTo == CFStringCompare(str, CFSTR("+inf"), kCFCompareCaseInsensitive)) result = kCFNumberPositiveInfinity;
        
        if (result) {
            CFRetain(result);
        } else {
            CFIndex len = CFStringGetLength(str);
            CFStringInlineBuffer buf;
            CFStringInitInlineBuffer(str, &buf, CFRangeMake(0, len));
            SInt32 idx = 0;
            double val;
            if (!__CFStringScanDouble(&buf, NULL, &idx, &val) || idx != len) {
                __CFPListRelease(str, pInfo->allocator);
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered misformatted real on line %d"), lineNumber(pInfo));
                return false;
            }
            result = CFNumberCreate(pInfo->allocator, kCFNumberDoubleType, &val);
        }
    }
    
    __CFPListRelease(str, pInfo->allocator);
    if (checkForCloseTag(pInfo, CFXMLPlistTags[REAL_IX], REAL_TAG_LENGTH)) {
        *out = result;
        return true;
    } else {
        __CFPListRelease(result, pInfo->allocator);
        return false;
    }
}

#define GET_CH	if (pInfo->curr == pInfo->end) {	\
			pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Premature end of file after <integer> on line %d"), lineNumber(pInfo)); \
			return false;			\
		}					\
		ch = *(pInfo->curr)

typedef struct {
    int64_t high;
    uint64_t low;
} CFSInt128Struct;

enum {
    kCFNumberSInt128Type = 17
};

CF_INLINE bool isWhitespace(const char *utf8bytes, const char *end) {
    // Converted UTF-16 isWhitespace from CFString to UTF8 bytes to get full list of UTF8 whitespace
    /*
     0020 -> <20>
     0009 -> <09>
     00a0 -> <c2a0>
     1680 -> <e19a80>
     2000 -> <e28080>
     2001 -> <e28081>
     2002 -> <e28082>
     2003 -> <e28083>
     2004 -> <e28084>
     2005 -> <e28085>
     2006 -> <e28086>
     2007 -> <e28087>
     2008 -> <e28088>
     2009 -> <e28089>
     200a -> <e2808a>
     200b -> <e2808b>
     202f -> <e280af>
     205f -> <e2819f>
     3000 -> <e38080>
     */
    // Except we consider some additional values from 0x0 to 0x21 and 0x7E to 0xA1 as whitespace, for compatability
    unsigned char byte1 = *utf8bytes;
    if (byte1 < 0x21 || (byte1 > 0x7E && byte1 < 0xA1)) return true;
    if ((byte1 == 0xe2 || byte1 == 0xe3) && (end - utf8bytes >= 3)) {
        // Check other possibilities in the 3-bytes range
        unsigned char byte2 = *(utf8bytes + 1);
        unsigned char byte3 = *(utf8bytes + 2);
        if (byte1 == 0xe2 && byte2 == 0x80) {
            return ((byte3 >= 80 && byte3 <= 0x8b) || byte3 == 0xaf);
        } else if (byte1 == 0xe2 && byte2 == 0x81) {
            return byte3 == 0x9f;
        } else if (byte1 == 0xe3 && byte2 == 0x80 && byte3 == 0x80) {
            return true;
        }
    }
    return false;
}

static Boolean parseIntegerTag(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    bool isHex = false, isNeg = false, hadLeadingZero = false;
    char ch = 0;
    
    // decimal_constant         S*(-|+)?S*[0-9]+		(S == space)
    // hex_constant		S*(-|+)?S*0[xX][0-9a-fA-F]+	(S == space)
    
    while (pInfo->curr < pInfo->end && isWhitespace(pInfo->curr, pInfo->end)) pInfo->curr++;
    GET_CH;
    if ('<' == ch) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty <integer> on line %d"), lineNumber(pInfo));
        return false;
    }
    if ('-' == ch || '+' == ch) {
	isNeg = ('-' == ch);
	pInfo->curr++;
	while (pInfo->curr < pInfo->end && isWhitespace(pInfo->curr, pInfo->end)) pInfo->curr++;
    }
    GET_CH;
    if ('0' == ch) {
	if (pInfo->curr + 1 < pInfo->end && ('x' == *(pInfo->curr + 1) || 'X' == *(pInfo->curr + 1))) {
	    pInfo->curr++;
	    isHex = true;
	} else {
	    hadLeadingZero = true;
	}
	pInfo->curr++;
    }
    GET_CH;
    while ('0' == ch) {
	hadLeadingZero = true;
	pInfo->curr++;
	GET_CH;
    }
    if ('<' == ch && hadLeadingZero) {	// nothing but zeros
	int32_t val = 0;
        if (!checkForCloseTag(pInfo, CFXMLPlistTags[INTEGER_IX], INTEGER_TAG_LENGTH)) {
	    // checkForCloseTag() sets error string
	    return false;
        }
        if (pInfo->skip) {
            *out = NULL;
        } else {
            *out = CFNumberCreate(pInfo->allocator, kCFNumberSInt32Type, &val);
        }
        return true;
    }
    if ('<' == ch) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Incomplete <integer> on line %d"), lineNumber(pInfo));
	return false;
    }
    uint64_t value = 0;
    uint32_t multiplier = (isHex ? 16 : 10);
    while ('<' != ch) {
	uint32_t new_digit = 0;
	switch (ch) {
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                new_digit = (ch - '0');
                break;
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                new_digit = (ch - 'a' + 10);
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                new_digit = (ch - 'A' + 10);
                break;
            default:	// other character
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unknown character '%c' (0x%x) in <integer> on line %d"), ch, ch, lineNumber(pInfo));
                return false;
	}
	if (!isHex && new_digit > 9) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Hex digit in non-hex <integer> on line %d"), lineNumber(pInfo));
	    return false;
	}
	if (UINT64_MAX / multiplier < value) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Integer overflow in <integer> on line %d"), lineNumber(pInfo));
	    return false;
	}
	value = multiplier * value;
	if (UINT64_MAX - new_digit < value) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Integer overflow in <integer> on line %d"), lineNumber(pInfo));
	    return false;
	}
	value = value + new_digit;
	if (isNeg && (uint64_t)INT64_MAX + 1 < value) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Integer underflow in <integer> on line %d"), lineNumber(pInfo));
	    return false;
	}
	pInfo->curr++;
	GET_CH;
    }
    if (!checkForCloseTag(pInfo, CFXMLPlistTags[INTEGER_IX], INTEGER_TAG_LENGTH)) {
	// checkForCloseTag() sets error string
	return false;
    }
    
    if (pInfo->skip) {
        *out = NULL;
    } else {
        if (isNeg || value <= INT64_MAX) {
            int64_t v = value;
            if (isNeg) v = -v;	// no-op if INT64_MIN
            *out = CFNumberCreate(pInfo->allocator, kCFNumberSInt64Type, &v);
        } else {
            CFSInt128Struct val;
            val.high = 0;
            val.low = value;
            *out = CFNumberCreate(pInfo->allocator, kCFNumberSInt128Type, &val);
        }
    }
    return true;
}

#undef GET_CH

// Returned object is retained; caller must free.  pInfo->curr expected to point to the first character after the '<'
static Boolean parseXMLElement(_CFXMLPlistParseInfo *pInfo, Boolean *isKey, CFTypeRef *out) {
    const char *marker = pInfo->curr;
    int markerLength = -1;
    Boolean isEmpty;
    int markerIx = -1;
    
    if (isKey) *isKey = false;
    while (pInfo->curr < pInfo->end) {
        char ch = *(pInfo->curr);
        if (ch == ' ' || ch ==  '\t' || ch == '\n' || ch =='\r') {
            if (markerLength == -1) markerLength = pInfo->curr - marker;
        } else if (ch == '>') {
            break;
        }
        pInfo->curr ++;
    }
    if (pInfo->curr >= pInfo->end) {
        return false;
    }
    isEmpty = (*(pInfo->curr-1) == '/');
    if (markerLength == -1)
        markerLength = pInfo->curr - (isEmpty ? 1 : 0) - marker;
    pInfo->curr ++; // Advance past '>'
    if (markerLength == 0) {
        // Back up to the beginning of the marker
        pInfo->curr = marker;
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Malformed tag on line %d"), lineNumber(pInfo));
        return false;
    }
    switch (*marker) {
        case 'a':   // Array
            if (markerLength == ARRAY_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[ARRAY_IX], ARRAY_TAG_LENGTH))
                markerIx = ARRAY_IX;
            break;
        case 'd': // Dictionary, data, or date; Fortunately, they all have the same marker length....
            if (markerLength != DICT_TAG_LENGTH)
                break;
            if (!memcmp(marker, CFXMLPlistTags[DICT_IX], DICT_TAG_LENGTH))
                markerIx = DICT_IX;
            else if (!memcmp(marker, CFXMLPlistTags[DATA_IX], DATA_TAG_LENGTH))
                markerIx = DATA_IX;
            else if (!memcmp(marker, CFXMLPlistTags[DATE_IX], DATE_TAG_LENGTH))
                markerIx = DATE_IX;
            break;
        case 'f': // false (boolean)
            if (markerLength == FALSE_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[FALSE_IX], FALSE_TAG_LENGTH)) {
                markerIx = FALSE_IX;
            }
            break;
        case 'i': // integer
            if (markerLength == INTEGER_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[INTEGER_IX], INTEGER_TAG_LENGTH))
                markerIx = INTEGER_IX;
            break;
        case 'k': // Key of a dictionary
            if (markerLength == KEY_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[KEY_IX], KEY_TAG_LENGTH)) {
                markerIx = KEY_IX;
                if (isKey) *isKey = true;
            }
            break;
        case 'p': // Plist
            if (markerLength == PLIST_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[PLIST_IX], PLIST_TAG_LENGTH))
                markerIx = PLIST_IX;
            break;
        case 'r': // real
            if (markerLength == REAL_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[REAL_IX], REAL_TAG_LENGTH))
                markerIx = REAL_IX;
            break;
        case 's': // String
            if (markerLength == STRING_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[STRING_IX], STRING_TAG_LENGTH))
                markerIx = STRING_IX;
            break;
        case 't': // true (boolean)
            if (markerLength == TRUE_TAG_LENGTH && !memcmp(marker, CFXMLPlistTags[TRUE_IX], TRUE_TAG_LENGTH))
                markerIx = TRUE_IX;
            break;
    }

    if (!pInfo->allowNewTypes && markerIx != PLIST_IX && markerIx != ARRAY_IX && markerIx != DICT_IX && markerIx != STRING_IX && markerIx != KEY_IX && markerIx != DATA_IX) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered new tag when expecting only old-style property list objects"));
        return false;
    }

    switch (markerIx) {
        case PLIST_IX:
            if (isEmpty) {
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty plist tag"));
                return false;
            }
            return parsePListTag(pInfo, out);
        case ARRAY_IX: 
            if (isEmpty) {
                if (pInfo->skip) {
                    *out = NULL;
                } else {
                    if (pInfo->mutabilityOption == kCFPropertyListImmutable) {
                        *out = CFArrayCreate(pInfo->allocator, NULL, 0, &kCFTypeArrayCallBacks);
                    } else {
                        *out = CFArrayCreateMutable(pInfo->allocator, 0, &kCFTypeArrayCallBacks);
                    }
                }
                return true;
            } else {
                return parseArrayTag(pInfo, out);
            }
        case DICT_IX:
            if (isEmpty) {
                if (pInfo->skip) {
                    *out = NULL;
                } else {
                    if (pInfo->mutabilityOption == kCFPropertyListImmutable) {
                        *out = CFDictionaryCreate(pInfo->allocator, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                    } else {
                        *out = CFDictionaryCreateMutable(pInfo->allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                    }
                }
                return true;
            } else {
                return parseDictTag(pInfo, out);
            }
        case KEY_IX:
        case STRING_IX:
        {
            int tagLen = (markerIx == KEY_IX) ? KEY_TAG_LENGTH : STRING_TAG_LENGTH;
            if (isEmpty) {
                if (pInfo->skip) {
                    *out = NULL;
                } else {
                    if (pInfo->mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
                        *out = CFStringCreateMutable(pInfo->allocator, 0);
                    } else {
                        *out = CFStringCreateWithCharacters(pInfo->allocator, NULL, 0);
                    }
                }
                return true;
            }
            if (!parseStringTag(pInfo, (CFStringRef *)out)) {
                return false; // parseStringTag will already have set the error string
            }
            if (!checkForCloseTag(pInfo, CFXMLPlistTags[markerIx], tagLen)) {
                __CFPListRelease(*out, pInfo->allocator);
                return false;
            } else {
                return true;
            }
        }
        case DATA_IX:
            if (isEmpty) {
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty <data> on line %d"), lineNumber(pInfo));
                return false;
            } else {
                return parseDataTag(pInfo, out);
            }
        case DATE_IX:
            if (isEmpty) {
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty <date> on line %d"), lineNumber(pInfo));
                return false;
            } else {
                return parseDateTag(pInfo, out);
            }
        case TRUE_IX:
            if (!isEmpty) {
		if (!checkForCloseTag(pInfo, CFXMLPlistTags[TRUE_IX], TRUE_TAG_LENGTH)) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered non-empty <true> on line %d"), lineNumber(pInfo));
                    return false;
                }
	    }
            if (pInfo->skip) {
                *out = NULL;
            } else {
                *out = CFRetain(kCFBooleanTrue);
            }
            return true;
        case FALSE_IX:
            if (!isEmpty) {
		if (!checkForCloseTag(pInfo, CFXMLPlistTags[FALSE_IX], FALSE_TAG_LENGTH)) {
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered non-empty <false> on line %d"), lineNumber(pInfo));
                    return false;
                }
	    }
            if (pInfo->skip) {
                *out = NULL;
            } else {
                *out = CFRetain(kCFBooleanFalse);
            }
            return true;
        case REAL_IX:
            if (isEmpty) {
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty <real> on line %d"), lineNumber(pInfo));
                return false;
            } else {
                return parseRealTag(pInfo, out);
            }
        case INTEGER_IX:
            if (isEmpty) {
                pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered empty <integer> on line %d"), lineNumber(pInfo));
                return false;
            } else {
                return parseIntegerTag(pInfo, out);
            }
        default:  {
            CFStringRef markerStr = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (const UInt8 *)marker, markerLength, kCFStringEncodingUTF8, NO);
            pInfo->curr = marker;
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown tag %@ on line %d"), markerStr ? markerStr : CFSTR("<unknown>"), lineNumber(pInfo));
            if (markerStr) CFRelease(markerStr);
            return false;
        }
    }
}

static Boolean parseXMLPropertyList(_CFXMLPlistParseInfo *pInfo, CFTypeRef *out) {
    while (!pInfo->error && pInfo->curr < pInfo->end) {
        UniChar ch;
        skipWhitespace(pInfo);
        if (pInfo->curr+1 >= pInfo->end) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("No XML content found"));
            return false;
        }
        if (*(pInfo->curr) != '<') {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected character %c at line %d"), *(pInfo->curr), lineNumber(pInfo));
            return false;
        }
        ch = *(++ pInfo->curr);
        if (ch == '!') {
            // Comment or DTD
            ++ pInfo->curr;
            if (pInfo->curr+1 < pInfo->end && *pInfo->curr == '-' && *(pInfo->curr+1) == '-') {
                // Comment
                pInfo->curr += 2;
                skipXMLComment(pInfo);
            } else {
                skipDTD(pInfo);
            }
        } else if (ch == '?') {
            // Processing instruction
            pInfo->curr++;
            skipXMLProcessingInstruction(pInfo);
        } else {
            // Tag or malformed
            return parseXMLElement(pInfo, NULL, out);
            // Note we do not verify that there was only one element, so a file that has garbage after the first element will nonetheless successfully parse
        }
    }
    // Should never get here
    if (!(pInfo->error)) {
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unexpected EOF"));
    }
    return false;
}

static CFStringEncoding encodingForXMLData(CFDataRef data, CFErrorRef *error, CFIndex *skip) {
    const uint8_t *bytes = (uint8_t *)CFDataGetBytePtr(data);
    UInt32 length = CFDataGetLength(data);
    const uint8_t *idx, *end;
    char quote;
    
    // Check for the byte order mark first. If we find it, set the skip value so the parser doesn't attempt to parse the BOM itself.
    if (length > 4) {
        if (*bytes == 0x00 && *(bytes+1) == 0x00 && *(bytes+2) == 0xFE && *(bytes+3) == 0xFF) {
            *skip = 4;
            return kCFStringEncodingUTF32BE;
        } else if (*bytes == 0xFF && *(bytes+1) == 0xFE && *(bytes+2) == 0x00 && *(bytes+3) == 0x00) {
            *skip = 4;
            return kCFStringEncodingUTF32LE;
        }
    }
    
    if (length > 3) {
        if (*bytes == 0xEF && *(bytes+1) == 0xBB && *(bytes+2) == 0xBF) {
            *skip = 3;
            return kCFStringEncodingUTF8;
        }
    }

    if (length > 2) {
        if (*bytes == 0xFF && *(bytes+1) == 0xFE) {
            *skip = 2;
            return kCFStringEncodingUTF16LE;
        } else if (*bytes == 0xFE && *(bytes+1) == 0xFF) {
            *skip = 2;
            return kCFStringEncodingUTF16BE;            
        } else if (*bytes == 0x00 || *(bytes+1) == 0x00) { // This clause checks for a Unicode sequence lacking the byte order mark; technically an error, but this check is recommended by the XML spec
            *skip = 2;
            return kCFStringEncodingUnicode;
        }
    }
        
    // Scan for the <?xml.... ?> opening
    if (length < 5 || strncmp((char const *) bytes, "<?xml", 5) != 0) return kCFStringEncodingUTF8;
    idx = bytes + 5;
    end = bytes + length;
    // Found "<?xml"; now we scan for "encoding"
    while (idx < end) {
        uint8_t ch = *idx;
        const uint8_t *scan;
        if ( ch == '?' || ch == '>') return kCFStringEncodingUTF8;
        idx ++;
        scan = idx;
	if (idx + 8 >= end) {
	    if (error) *error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("End of buffer while looking for encoding name"));
	    return 0;
	}
        if (ch == 'e' && *scan++ == 'n' && *scan++ == 'c' && *scan++ == 'o' && *scan++ == 'd' && *scan++ == 'i'
            && *scan++ == 'n' && *scan++ == 'g' && *scan++ == '=') {
            idx = scan;
            break;
        }
    }
    if (idx >= end) return kCFStringEncodingUTF8;
    quote = *idx;
    if (quote != '\'' && quote != '\"') return kCFStringEncodingUTF8;
    else {
        CFStringRef encodingName;
        const uint8_t *base = idx+1; // Move past the quote character
        UInt32 len;
        idx ++;
        while (idx < end && *idx != quote) idx ++;
        if (idx >= end) return kCFStringEncodingUTF8;
        len = idx - base;
        if (len == 5 && (*base == 'u' || *base == 'U') && (base[1] == 't' || base[1] == 'T') && (base[2] == 'f' || base[2] == 'F') && (base[3] == '-') && (base[4] == '8'))
            return kCFStringEncodingUTF8;
        encodingName = CFStringCreateWithBytes(kCFAllocatorSystemDefault, base, len, kCFStringEncodingISOLatin1, false);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        CFStringEncoding enc = CFStringConvertIANACharSetNameToEncoding(encodingName);
        if (enc != kCFStringEncodingInvalidId) {
            CFRelease(encodingName);
            return enc;
        }
#endif

        if (error) {
            *error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown encoding (%@)"), encodingName);
            CFRelease(encodingName);
        }
        return 0;
    }
}

bool __CFTryParseBinaryPlist(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFPropertyListRef *plist, CFStringRef *errorString);

#define SAVE_PLISTS 0

#if SAVE_PLISTS
static Boolean __savePlistData(CFDataRef data, CFOptionFlags opt) {
    uint8_t pn[2048];
    uint8_t fn[2048];
    uint32_t pnlen = sizeof(pn);
    uint8_t *pnp = NULL;
    if (0 == _NSGetExecutablePath((char *)pn, &pnlen)) {
	pnp = strrchr((char *)pn, '/');
    }
    if (!pnp) {
	pnp = pn;
    } else {
	pnp++;
    }
    if (0 == strcmp((char *)pnp, "parse_plists")) return true;
    CFUUIDRef r = CFUUIDCreate(kCFAllocatorSystemDefault);
    CFStringRef s = CFUUIDCreateString(kCFAllocatorSystemDefault, r);
    CFStringRef p = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("/tmp/plists/%s#%@#0x%x"), pnp, s, opt);
    _CFStringGetFileSystemRepresentation(p, fn, sizeof(fn));
    CFRelease(r);
    CFRelease(s);
    CFRelease(p);
    int fd = open((const char *)fn, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (fd < 0) return false;
    int len = CFDataGetLength(data);
    int w = write(fd, CFDataGetBytePtr(data), len);
    fsync(fd);
    close(fd);
    if (w != len) return false;
    return true;
}
#endif

// If the data is from a converted string, then originalString is non-NULL. If originalString is NULL, then pass in guessedEncoding.
// keyPaths is a set of CFStrings, ':'-separated paths
static Boolean _CFPropertyListCreateFromUTF8Data(CFAllocatorRef allocator, CFDataRef xmlData, CFIndex skipBytes, CFStringRef originalString, CFStringEncoding guessedEncoding, CFOptionFlags option, CFErrorRef *outError, Boolean allowNewTypes, CFPropertyListFormat *format, CFSetRef keyPaths, CFTypeRef *out) {
    initStatics();
    
    CFAssert1(xmlData != NULL, __kCFLogAssertion, "%s(): NULL data not allowed", __PRETTY_FUNCTION__);
    CFAssert2(option == kCFPropertyListImmutable || option == kCFPropertyListMutableContainers || option == kCFPropertyListMutableContainersAndLeaves, __kCFLogAssertion, "%s(): Unrecognized option %d", __PRETTY_FUNCTION__, option);
    
    CFIndex length = CFDataGetLength(xmlData);
    if (!length) {
        if (outError) *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Conversion of string failed. The string is empty."));
        return false;
    }
    
    _CFXMLPlistParseInfo pInfoBuf;
    _CFXMLPlistParseInfo *pInfo = &pInfoBuf;
    CFTypeRef result;
    
    // Ensure that the data is not collected while we are using it
    CFRetain(xmlData);
    const char *buf = (const char *)CFDataGetBytePtr(xmlData);
    
    // We may have to skip over starting stuff like BOM markers.
    buf += skipBytes;
    
    pInfo->begin = buf;
    pInfo->end = buf+length;
    pInfo->curr = buf;
    pInfo->allocator = allocator;
    pInfo->error = NULL;
    _createStringMap(pInfo);
    pInfo->mutabilityOption = option;
    pInfo->allowNewTypes = allowNewTypes;
    pInfo->skip = false;
    pInfo->keyPaths = createTopLevelKeypaths(allocator, keyPaths);
    
    Boolean success = parseXMLPropertyList(pInfo, &result);
    if (success && result && format) *format = kCFPropertyListXMLFormat_v1_0;
    
    _cleanupStringMap(pInfo);
    if (pInfo->keyPaths && !(0)) CFRelease(pInfo->keyPaths);
    CFRelease(xmlData);

    if (success) {
        *out = result; // caller releases
        return true;
    }
    
    // Try again, old-style
    CFErrorRef oldStyleError = NULL;
    result = __CFCreateOldStylePropertyListOrStringsFile(allocator, xmlData, originalString, guessedEncoding, option, outError ? &oldStyleError : NULL, format);
    if (result) {
        // Release old error, return
        if (pInfo->error) CFRelease(pInfo->error);
        *out = result;
        return true;
    }
    
    // Failure, both ways. Set up the error to be given back to caller.
    if (!outError) {
        if (pInfo->error) CFRelease(pInfo->error);
        return false;
    }
    
    // Caller's responsibility to release outError
    if (pInfo->error && oldStyleError) {
        // Add the error from the old-style property list parser to the user info of the original error (pInfo->error), which had better exist
        CFDictionaryRef oldUserInfo = CFErrorCopyUserInfo(pInfo->error);
        CFMutableDictionaryRef newUserInfo = CFDictionaryCreateMutableCopy(kCFAllocatorSystemDefault, CFDictionaryGetCount(oldUserInfo) + 1, oldUserInfo);
        CFDictionaryAddValue(newUserInfo, CFPropertyListOldStyleParserErrorKey, oldStyleError);
        
        // Re-create the xml parser error with this new user info dictionary
        CFErrorRef newError = CFErrorCreate(kCFAllocatorSystemDefault, CFErrorGetDomain(pInfo->error), CFErrorGetCode(pInfo->error), newUserInfo);
        
        CFRelease(oldUserInfo);
        CFRelease(newUserInfo);
        CFRelease(oldStyleError);
        CFRelease(pInfo->error);
        *outError = newError;
        
    } else if (pInfo->error && !oldStyleError) {
        // Return original error
        *outError = pInfo->error;
    } else if (!pInfo->error && oldStyleError) {
        // Return only old-style error
        // Probably shouldn't get here
        *outError = oldStyleError;
    } else if (!pInfo->error && !oldStyleError) {
        // Return unknown error
        *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Encountered unknown error during parse"));
    }
    return false;    
}

static CFDataRef _createUTF8DataFromString(CFAllocatorRef allocator, CFStringRef str) {
    CFIndex bytesNeeded = 0;
    CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, false, NULL, 0, &bytesNeeded);

    const char *bytes = (const char *)CFAllocatorAllocate(allocator, bytesNeeded, 0);
    CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, false, (uint8_t *)bytes, bytesNeeded, NULL);
   
    CFDataRef utf8Data = CFDataCreateWithBytesNoCopy(allocator, (const UInt8 *)bytes, bytesNeeded, allocator);
    return utf8Data;
}

// Set topLevelKeys to a set of top level keys to decode. If NULL, all keys are decoded. If the top level object is not a dictionary, all objects are decoded. If the plist is not XML, all objects are decoded.
static Boolean _CFPropertyListCreateWithData(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFErrorRef *outError, Boolean allowNewTypes, CFPropertyListFormat *format, CFSetRef topLevelKeys, CFTypeRef *out) {
    initStatics();
    CFStringEncoding encoding;
    
    if (!data || CFDataGetLength(data) == 0) {
        if (outError) {
            *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Cannot parse a NULL or zero-length data"));
        }
        return false;
    }
    
#if SAVE_PLISTS
    __savePlistData(data, option);
#endif
    
    // Ignore the error from CFTryParseBinaryPlist -- if it doesn't work, we're going to try again anyway using the XML parser
    if (__CFTryParseBinaryPlist(allocator, data, option, out, NULL)) {
	if (format) *format = kCFPropertyListBinaryFormat_v1_0;
        return true;
    }
    
    // Use our own error variable here so we can check it against NULL later
    CFErrorRef subError = NULL;
    CFIndex skip = 0;
    encoding = encodingForXMLData(data, &subError, &skip); // 0 is an error return, NOT MacRoman.
    
    if (encoding == 0) {
        // Couldn't find an encoding
        // Note that encodingForXMLData() will give us the right values for a standard plist, too.
        if (outError && subError == NULL) {
	    // encodingForXMLData didn't set an error, so we create a new one here
            *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Could not determine the encoding of the XML data"));
        } else if (outError && subError) {
	    // give the caller the subError, they will release
	    *outError = subError;
	} else if (!outError && subError) {
	    // Release the error
	    CFRelease(subError);
	}
        return false;
    }
    
    if (encoding == kCFStringEncodingUTF8) {
        // Use fast path
        return _CFPropertyListCreateFromUTF8Data(allocator, data, skip, NULL, encoding, option, outError, allowNewTypes, format, topLevelKeys, out);
    }
    
    // Convert to UTF8 first
    CFStringRef xmlString = CFStringCreateWithBytes(allocator, CFDataGetBytePtr(data) + skip, CFDataGetLength(data) - skip, encoding, false);
    if (!xmlString) {
        if (outError) *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Could not determine the encoding of the XML data (string creation failed)"));
        return false;
    }
    
    CFDataRef utf8Data = _createUTF8DataFromString(allocator, xmlString);

    Boolean result = _CFPropertyListCreateFromUTF8Data(allocator, utf8Data, 0, xmlString, 0, option, outError, allowNewTypes, format, topLevelKeys, out);
    
    if (xmlString && !(0)) CFRelease(xmlString);
    if (utf8Data && !(0)) CFRelease(utf8Data);
    
    return result;
}

// -----------------------------------------------------------------------------------------------------------------------

#pragma mark -
#pragma mark Exported Parsing Functions

CFTypeRef _CFPropertyListCreateFromXMLStringError(CFAllocatorRef allocator, CFStringRef xmlString, CFOptionFlags option, CFErrorRef *error, Boolean allowNewTypes, CFPropertyListFormat *format) {
    // Convert to UTF8 first
    CFDataRef utf8Data = _createUTF8DataFromString(allocator, xmlString);
    CFTypeRef result = NULL;
    _CFPropertyListCreateFromUTF8Data(allocator, utf8Data, 0, xmlString, 0, option, error, allowNewTypes, format, NULL, &result);    
    if (utf8Data && !(0)) CFRelease(utf8Data);

    return result;
}

CFTypeRef _CFPropertyListCreateFromXMLString(CFAllocatorRef allocator, CFStringRef xmlString, CFOptionFlags option, CFStringRef *errorString, Boolean allowNewTypes, CFPropertyListFormat *format) {
    initStatics();
    if (errorString) *errorString = NULL;
    CFErrorRef error = NULL;
    CFTypeRef result = _CFPropertyListCreateFromXMLStringError(allocator, xmlString, option, &error, allowNewTypes, format);

    if (errorString && error) {
        // The caller is interested in receiving an error message. Pull the debug string out of the CFError returned above
        CFDictionaryRef userInfo = CFErrorCopyUserInfo(error);
        CFStringRef debugString = NULL;
        
        // Handle a special-case for compatibility - if the XML parse failed and the old-style plist parse failed, construct a special string
        CFErrorRef underlyingError = NULL;

        Boolean oldStyleFailed = CFDictionaryGetValueIfPresent(userInfo, CFPropertyListOldStyleParserErrorKey, (const void **)&underlyingError);
        
        if (oldStyleFailed) {
            CFStringRef xmlParserErrorString, oldStyleParserErrorString;
            xmlParserErrorString = (CFStringRef)CFDictionaryGetValue(userInfo, kCFErrorDebugDescriptionKey);
            
            CFDictionaryRef oldStyleParserUserInfo = CFErrorCopyUserInfo(underlyingError);
            oldStyleParserErrorString = (CFStringRef)CFDictionaryGetValue(userInfo, kCFErrorDebugDescriptionKey);
            
            // Combine the two strings into a single one that matches the previous implementation
            debugString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("XML parser error:\n\t%@\nOld-style plist parser error:\n\t%@\n"), xmlParserErrorString, oldStyleParserErrorString);
            
            CFRelease(oldStyleParserUserInfo);
        } else {
            debugString = (CFStringRef)CFDictionaryGetValue(userInfo, kCFErrorDebugDescriptionKey);
            if (debugString) CFRetain(debugString);
        }
        
        // Give the debugString to the caller, who is responsible for releasing it
        CFRelease(userInfo);
        *errorString = debugString;
    }
    
    if (error) {
        CFRelease(error);
    }
    
    return result;
}

CF_PRIVATE bool __CFBinaryPlistCreateObjectFiltered(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFAllocatorRef allocator, CFOptionFlags mutabilityOption, CFMutableDictionaryRef objects, CFMutableSetRef set, CFIndex curDepth, CFSetRef keyPaths, CFPropertyListRef *plist);

// Returns a subset of the property list, only including the key paths in the CFSet.
bool _CFPropertyListCreateFiltered(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFSetRef keyPaths, CFPropertyListRef *value, CFErrorRef *error) {
    
    initStatics();
    
    if (!keyPaths || !data) {
        return false;
    }
    
    uint8_t marker;    
    CFBinaryPlistTrailer trailer;
    uint64_t offset;
    const uint8_t *databytes = CFDataGetBytePtr(data);
    uint64_t datalen = CFDataGetLength(data);
    Boolean success = false;
    CFTypeRef out = NULL;

    // First check to see if it is a binary property list
    if (8 <= datalen && __CFBinaryPlistGetTopLevelInfo(databytes, datalen, &marker, &offset, &trailer)) {        
        uint64_t valueOffset = offset;
        
        // Split up the key path
        CFSetRef splitKeyPaths = createTopLevelKeypaths(allocator, keyPaths);

        // Create a dictionary to cache objects in
        CFMutableDictionaryRef objects = CFDictionaryCreateMutable(allocator, 0, NULL, &kCFTypeDictionaryValueCallBacks);
        success = __CFBinaryPlistCreateObjectFiltered(databytes, datalen, valueOffset, &trailer, allocator, option, objects, NULL, 0, splitKeyPaths, &out);
        
        CFRelease(splitKeyPaths);
        CFRelease(objects);        
    } else {
	// Try an XML property list
        success = _CFPropertyListCreateWithData(allocator, data, option, error, true, NULL, keyPaths, &out);
    }
    
    if (success && value) {
        *value = out; // caller releases
    } else if (out) {
        CFRelease(out);
    }
    return success;
}

/* Get a single value for a given key in a top-level dictionary in a property list.
 @param allocator The allocator to use.
 @param data The property list data.
 @param option Currently unused, set to 0.
 @param keyPath The keyPath to search for in the property list. Keys are colon-separated. Indexes into arrays are specified with an integer (zero-based).
 @param value If the key is found and the parameter is non-NULL, this will be set to a reference to the value. It is the caller's responsibility to release the object. If no object is found, will be set to NULL. If this parameter is NULL, this function can be used to check for the existence of a key without creating it by just checking the return value.
 @param error If an error occurs, will be set to a valid CFErrorRef. It is the caller's responsibility to release this value.
 @return True if the key is found, false otherwise.
 */
bool _CFPropertyListCreateSingleValue(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFStringRef keyPath, CFPropertyListRef *value, CFErrorRef *error) {
    
    initStatics();
    
    if (!keyPath || CFStringGetLength(keyPath) == 0) {
        return false;
    }
    
    uint8_t marker;    
    CFBinaryPlistTrailer trailer;
    uint64_t offset;
    const uint8_t *databytes = CFDataGetBytePtr(data);
    uint64_t datalen = CFDataGetLength(data);
    Boolean success = false;
    
    // First check to see if it is a binary property list
    if (8 <= datalen && __CFBinaryPlistGetTopLevelInfo(databytes, datalen, &marker, &offset, &trailer)) {
        // Split up the key path
        CFArrayRef keyPathArray = CFStringCreateArrayBySeparatingStrings(kCFAllocatorSystemDefault, keyPath, CFSTR(":"));
        uint64_t keyOffset, valueOffset = offset;
        
        // Create a dictionary to cache objects in
        CFMutableDictionaryRef objects = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
        CFIndex keyPathCount = CFArrayGetCount(keyPathArray);
        _CFDictionarySetCapacity(objects, keyPathCount + 1);

        for (CFIndex i = 0; i < keyPathCount; i++) {
            CFStringRef oneKey = (CFStringRef)CFArrayGetValueAtIndex(keyPathArray, i);
            SInt32 intValue = CFStringGetIntValue(oneKey);
            if ((intValue == 0 && CFStringCompare(CFSTR("0"), oneKey, 0) != kCFCompareEqualTo) || intValue == INT_MAX || intValue == INT_MIN || intValue < 0) {
                // Treat as a string key into a dictionary
                success = __CFBinaryPlistGetOffsetForValueFromDictionary3(databytes, datalen, valueOffset, &trailer, (CFTypeRef)oneKey, &keyOffset, &valueOffset, false, objects);
            } else {
                // Treat as integer index into an array
                success = __CFBinaryPlistGetOffsetForValueFromArray2(databytes, datalen, valueOffset, &trailer, intValue, &valueOffset, objects);
            }
            
            if (!success) {
                break;
            }
        }
        
        // value could be null if the caller wanted to check for the existence of a key but not bother creating it
        if (success && value) {
            CFPropertyListRef pl;
	    success = __CFBinaryPlistCreateObjectFiltered(databytes, datalen, valueOffset, &trailer, allocator, option, objects, NULL, 0, NULL, &pl);
	    if (success) {
		// caller's responsibility to release the created object
		*value = pl;
	    }
	}
        
        CFRelease(keyPathArray);
        CFRelease(objects);
    } else {
	// Try an XML property list
	// Note: This is currently not any more efficient than grabbing the whole thing. This could be improved in the future.
	CFPropertyListRef plist = CFPropertyListCreateWithData(allocator, data, option, NULL, error);
        if (plist) {
            CFPropertyListRef nextObject = plist;
            success = true;
            CFArrayRef keyPathArray = CFStringCreateArrayBySeparatingStrings(kCFAllocatorSystemDefault, keyPath, CFSTR(":"));
            for (CFIndex i = 0;  i < CFArrayGetCount(keyPathArray); i++) {
                CFStringRef oneKey = (CFStringRef)CFArrayGetValueAtIndex(keyPathArray, i);
                SInt32 intValue = CFStringGetIntValue(oneKey);
                if (((intValue == 0 && CFStringCompare(CFSTR("0"), oneKey, 0) != kCFCompareEqualTo) || intValue == INT_MAX || intValue == INT_MIN) && CFGetTypeID((CFTypeRef)nextObject) == dicttype) {
                    // Treat as a string key into a dictionary
                    nextObject = (CFPropertyListRef)CFDictionaryGetValue((CFDictionaryRef)nextObject, oneKey);
                } else if (CFGetTypeID((CFTypeRef)nextObject) == arraytype) {
                    // Treat as integer index into an array
                    nextObject = (CFPropertyListRef)CFArrayGetValueAtIndex((CFArrayRef)nextObject, intValue);
                } else {
                    success = false;
                    break;
                }
            }
            
            if (success && nextObject && value) {
                *value = nextObject;
                // caller's responsibility to release the created object
                CFRetain(*value);
            } else if (!nextObject) {
                success = false;
            }
            
            CFRelease(keyPathArray);
            CFRelease(plist);
        }
    }
    
    return success;
}

// Legacy
CFTypeRef _CFPropertyListCreateFromXMLData(CFAllocatorRef allocator, CFDataRef xmlData, CFOptionFlags option, CFStringRef *errorString, Boolean allowNewTypes, CFPropertyListFormat *format) {
    initStatics();
    CFTypeRef out = NULL;
    if (errorString) *errorString = NULL;
    CFErrorRef error = NULL;
    Boolean result = _CFPropertyListCreateWithData(allocator, xmlData, option, &error, allowNewTypes, format, NULL, &out);
    if (!result && error && errorString) {
        *errorString = __copyErrorDebugDescription(error);
    }
    if (error) CFRelease(error);
    return out;
}

CFPropertyListRef CFPropertyListCreateWithData(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags options, CFPropertyListFormat *format, CFErrorRef *error) {
    initStatics();
    CFAssert1(data != NULL, __kCFLogAssertion, "%s(): NULL data not allowed", __PRETTY_FUNCTION__);
    CFAssert2(options == kCFPropertyListImmutable || options == kCFPropertyListMutableContainers || options == kCFPropertyListMutableContainersAndLeaves, __kCFLogAssertion, "%s(): Unrecognized option %d", __PRETTY_FUNCTION__, options);
    CFPropertyListRef out = NULL;
    _CFPropertyListCreateWithData(allocator, data, options, error, true, format, NULL, &out);
    return out;
}

CFPropertyListRef CFPropertyListCreateFromXMLData(CFAllocatorRef allocator, CFDataRef xmlData, CFOptionFlags option, CFStringRef *errorString) {
    initStatics();
    if (errorString) *errorString = NULL;
    CFErrorRef error = NULL;
    CFPropertyListRef result = CFPropertyListCreateWithData(allocator, xmlData, option, NULL, &error);
    if (error && errorString) {
        *errorString = __copyErrorDebugDescription(error);
    }
    if (error) CFRelease(error);
    return result;
}

CFDataRef CFPropertyListCreateData(CFAllocatorRef allocator, CFPropertyListRef propertyList, CFPropertyListFormat format, CFOptionFlags options, CFErrorRef *error) {
    initStatics();
    CFAssert1(format != kCFPropertyListOpenStepFormat, __kCFLogAssertion, "%s(): kCFPropertyListOpenStepFormat not supported for writing", __PRETTY_FUNCTION__);
    CFAssert2(format == kCFPropertyListXMLFormat_v1_0 || format == kCFPropertyListBinaryFormat_v1_0, __kCFLogAssertion, "%s(): Unrecognized option %d", __PRETTY_FUNCTION__, format);
    CFAssert1(propertyList != NULL, __kCFLogAssertion, "%s(): Cannot be called with a NULL property list", __PRETTY_FUNCTION__);
    __CFAssertIsPList(propertyList);
    
    CFDataRef data = NULL;
    
    
    CFStringRef validErr = NULL;
    if (!_CFPropertyListIsValidWithErrorString(propertyList, format, &validErr)) {
        CFLog(kCFLogLevelError, CFSTR("Property list invalid for format: %d (%@)"), format, validErr);
	if (validErr) CFRelease(validErr);
        return NULL;
    }
    
    if (format == kCFPropertyListOpenStepFormat) {
        CFLog(kCFLogLevelError, CFSTR("Property list format kCFPropertyListOpenStepFormat not supported for writing"));
        return NULL;
    } else if (format == kCFPropertyListXMLFormat_v1_0) {
        data = _CFPropertyListCreateXMLData(allocator, propertyList, true);
    } else if (format == kCFPropertyListBinaryFormat_v1_0) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
        // TODO: Is it more efficient to create a stream here or just use a mutable data?
        CFWriteStreamRef stream = CFWriteStreamCreateWithAllocatedBuffers(kCFAllocatorSystemDefault, allocator);
        CFWriteStreamOpen(stream);
        CFIndex len = CFPropertyListWrite(propertyList, stream, format, options, error);
        if (0 < len) {
            data = (CFDataRef)CFWriteStreamCopyProperty(stream, kCFStreamPropertyDataWritten);
        }
        CFWriteStreamClose(stream);
	CFRelease(stream);
#else
        CFMutableDataRef dataForPlist = CFDataCreateMutable(allocator, 0);
        __CFBinaryPlistWrite(propertyList, dataForPlist, 0, options, error);
        return dataForPlist;
#endif
    } else {
	CFLog(kCFLogLevelError, CFSTR("Unknown format option"));
    }
    
    return data;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS

CFIndex CFPropertyListWrite(CFPropertyListRef propertyList, CFWriteStreamRef stream, CFPropertyListFormat format, CFOptionFlags options, CFErrorRef *error) {
    initStatics();
    CFAssert1(stream != NULL, __kCFLogAssertion, "%s(): NULL stream not allowed", __PRETTY_FUNCTION__);
    CFAssert1(format != kCFPropertyListOpenStepFormat, __kCFLogAssertion, "%s(): kCFPropertyListOpenStepFormat not supported for writing", __PRETTY_FUNCTION__);
    CFAssert2(format == kCFPropertyListXMLFormat_v1_0 || format == kCFPropertyListBinaryFormat_v1_0, __kCFLogAssertion, "%s(): Unrecognized option %d", __PRETTY_FUNCTION__, format);
    CFAssert1(propertyList != NULL, __kCFLogAssertion, "%s(): Cannot be called with a NULL property list", __PRETTY_FUNCTION__);
    __CFAssertIsPList(propertyList);
    CFAssert1(CFWriteStreamGetTypeID() == CFGetTypeID(stream), __kCFLogAssertion, "%s(): stream argument is not a write stream", __PRETTY_FUNCTION__);
    CFAssert1(kCFStreamStatusOpen == CFWriteStreamGetStatus(stream) || kCFStreamStatusWriting == CFWriteStreamGetStatus(stream), __kCFLogAssertion, "%s():  stream is not open", __PRETTY_FUNCTION__);
    
    CFStringRef validErr = NULL;
    if (!_CFPropertyListIsValidWithErrorString(propertyList, format, &validErr)) {
        CFLog(kCFLogLevelError, CFSTR("Property list invalid for format: %d (%@)"), format, validErr);
	if (validErr) CFRelease(validErr);
        return 0;
    }
    if (format == kCFPropertyListOpenStepFormat) {
        CFLog(kCFLogLevelError, CFSTR("Property list format kCFPropertyListOpenStepFormat not supported for writing"));
        return 0;
    }
    if (format == kCFPropertyListXMLFormat_v1_0) {
        CFDataRef data = _CFPropertyListCreateXMLData(kCFAllocatorSystemDefault, propertyList, true);
        if (!data) {
            CFLog(kCFLogLevelError, CFSTR("Property list format kCFPropertyListXMLFormat_v1_0 specified but was not a valid property list type"));
            return 0;
        }
        CFIndex len = CFDataGetLength(data);
	const uint8_t *ptr = CFDataGetBytePtr(data);
	while (0 < len) {
	    CFIndex ret = CFWriteStreamWrite(stream, ptr, len);
	    if (ret == 0) {
                if (error) *error = __CFPropertyListCreateError(kCFPropertyListWriteStreamError, CFSTR("Property list writing could not be completed because stream is full."));
                CFRelease(data);
	        return 0;
	    }
	    if (ret < 0) {
		CFErrorRef underlyingError = CFWriteStreamCopyError(stream);
                if (underlyingError) {
                    if (error) {
                        // Wrap the error from CFWriteStreamCopy in a new error
                        CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks); 
                        CFDictionarySetValue(userInfo, kCFErrorDebugDescriptionKey, CFSTR("Property list writing could not be completed because the stream had an unknown error."));
                        CFDictionarySetValue(userInfo, kCFErrorUnderlyingErrorKey, underlyingError);
                        *error = CFErrorCreate(kCFAllocatorSystemDefault, kCFErrorDomainCocoa, kCFPropertyListWriteStreamError, userInfo);
                        CFRelease(userInfo);
                    }
                    CFRelease(underlyingError);
                }
		CFRelease(data);
		return 0;
	    }
	    ptr += ret;
	    len -= ret;
	}
	len = CFDataGetLength(data);
	CFRelease(data);
        return len;
    }
    if (format == kCFPropertyListBinaryFormat_v1_0) {
        CFIndex len = __CFBinaryPlistWrite(propertyList, stream, 0, options, error);
        return len;
    }
    CFLog(kCFLogLevelError, CFSTR("Unknown format option"));
    return 0;
}

CFIndex CFPropertyListWriteToStream(CFPropertyListRef propertyList, CFWriteStreamRef stream, CFPropertyListFormat format, CFStringRef *errorString) {
    initStatics();
    if (errorString) *errorString = NULL;
    CFErrorRef error = NULL;
    
    // For backwards compatibility, we check the format parameter up front since these do not have CFError counterparts in the newer API
    CFStringRef validErr = NULL;
    if (!_CFPropertyListIsValidWithErrorString(propertyList, format, &validErr)) {
	if (errorString) *errorString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("Property list invalid for format (%@)"), validErr);
	if (validErr) CFRelease(validErr);
        return 0;
    }
    if (format == kCFPropertyListOpenStepFormat) {
        if (errorString) *errorString = (CFStringRef)CFRetain(CFSTR("Property list format kCFPropertyListOpenStepFormat not supported for writing"));
        return 0;
    }
    if (format != kCFPropertyListBinaryFormat_v1_0 && format != kCFPropertyListXMLFormat_v1_0) {
        if (errorString) *errorString = (CFStringRef)CFRetain(CFSTR("Unknown format option"));
        return 0;
    }
    
    CFIndex result = CFPropertyListWrite(propertyList, stream, format, 0, &error);
    if (error && errorString) {
        *errorString = __copyErrorDebugDescription(error);
    }
    if (error) CFRelease(error);
    return result;    
}

static bool __convertReadStreamToBytes(CFReadStreamRef stream, CFIndex max, uint8_t **buffer, CFIndex *length, CFErrorRef *error) {
    int32_t buflen = 0, bufsize = 0, retlen;
    uint8_t *buf = NULL, sbuf[8192];
    for (;;) {
	retlen = CFReadStreamRead(stream, sbuf, __CFMin(8192, max));
        if (retlen <= 0) {
            *buffer = buf;
            *length = buflen;
            
            if (retlen < 0 && error) {
                // Copy the error out
                *error = CFReadStreamCopyError(stream);
                return false;
            }
            
	    return true;
	}
        if (bufsize < buflen + retlen) {
	    if (bufsize < 256 * 1024) {
		bufsize *= 4;
	    } else if (bufsize < 16 * 1024 * 1024) {
		bufsize *= 2;
	    } else {
		// once in this stage, this will be really slow
		// and really potentially fragment memory
		bufsize += 256 * 1024;
	    }
	    if (bufsize < buflen + retlen) bufsize = buflen + retlen;
	    buf = (uint8_t *)CFAllocatorReallocate(kCFAllocatorSystemDefault, buf, bufsize, 0);
	    if (!buf) HALT;
	}
	memmove(buf + buflen, sbuf, retlen);
	buflen += retlen;
        max -= retlen;
	if (max <= 0) {
	    *buffer = buf;
	    *length = buflen;
	    return true;
	}
    }
    return true;
}

CFPropertyListRef CFPropertyListCreateWithStream(CFAllocatorRef allocator, CFReadStreamRef stream, CFIndex streamLength, CFOptionFlags mutabilityOption, CFPropertyListFormat *format, CFErrorRef *error) {
    initStatics();
    
    CFAssert1(stream != NULL, __kCFLogAssertion, "%s(): NULL stream not allowed", __PRETTY_FUNCTION__);
    CFAssert1(CFReadStreamGetTypeID() == CFGetTypeID(stream), __kCFLogAssertion, "%s(): stream argument is not a read stream", __PRETTY_FUNCTION__);
    CFAssert1(kCFStreamStatusOpen == CFReadStreamGetStatus(stream) || kCFStreamStatusReading == CFReadStreamGetStatus(stream), __kCFLogAssertion, "%s():  stream is not open", __PRETTY_FUNCTION__);
    CFAssert2(mutabilityOption == kCFPropertyListImmutable || mutabilityOption == kCFPropertyListMutableContainers || mutabilityOption == kCFPropertyListMutableContainersAndLeaves, __kCFLogAssertion, "%s(): Unrecognized option %d", __PRETTY_FUNCTION__, mutabilityOption);
    
    if (0 == streamLength) streamLength = LONG_MAX;
    CFErrorRef underlyingError = NULL;
    CFIndex buflen = 0;
    uint8_t *buffer = NULL;
    if (!__convertReadStreamToBytes(stream, streamLength, &buffer, &buflen, &underlyingError)) {
        if (error) {
            // Wrap the error from CFReadStream in a new error in the cocoa domain
            CFMutableDictionaryRef userInfo = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks); 
            CFDictionarySetValue(userInfo, kCFErrorDebugDescriptionKey, CFSTR("Property list reading could not be completed because the stream had an unknown error. Did you forget to open the stream?"));
            if (underlyingError) {
                CFDictionarySetValue(userInfo, kCFErrorUnderlyingErrorKey, underlyingError);
            }
            *error = CFErrorCreate(kCFAllocatorSystemDefault, kCFErrorDomainCocoa, kCFPropertyListReadStreamError, userInfo);
            CFRelease(userInfo);
        }
        if (underlyingError) {
            CFRelease(underlyingError);
        }
        return NULL;
    }
    
    if (!buffer || buflen < 6) {
        if (buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, buffer);
        if (error) *error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("stream had too few bytes"));
        return NULL;
    }
    
    CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorSystemDefault, buffer, buflen, kCFAllocatorSystemDefault);
    CFPropertyListRef pl = NULL; // initialize to null, because if the following call fails we must return NULL
    _CFPropertyListCreateWithData(allocator, data, mutabilityOption, error, true, format, NULL, &pl);
    CFRelease(data);
    
    return pl;
}

CFPropertyListRef CFPropertyListCreateFromStream(CFAllocatorRef allocator, CFReadStreamRef stream, CFIndex length, CFOptionFlags mutabilityOption, CFPropertyListFormat *format, CFStringRef *errorString) {
    initStatics();
    if (errorString) *errorString = NULL;
    CFErrorRef error = NULL;
    CFPropertyListRef result = CFPropertyListCreateWithStream(allocator, stream, length, mutabilityOption, format, &error);
    if (error && errorString) {
        *errorString = __copyErrorDebugDescription(error);
    }
    if (error) CFRelease(error);
    return result;
}

#endif //DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS

#pragma mark -
#pragma mark Property List Copies

static CFArrayRef _arrayDeepImmutableCopy(CFAllocatorRef allocator, CFArrayRef array, CFOptionFlags mutabilityOption) {
    CFArrayRef result = NULL;
    CFIndex i, c = CFArrayGetCount(array);
    if (c == 0) {
        result = CFArrayCreate(allocator, NULL, 0, &kCFTypeArrayCallBacks);
    } else {
        new_cftype_array(values, c);
        CFArrayGetValues(array, CFRangeMake(0, c), values);
        for (i = 0; i < c; i ++) {
            CFTypeRef newValue = CFPropertyListCreateDeepCopy(allocator, values[i], mutabilityOption);
            if (newValue == NULL) {
                break;
            }
            __CFAssignWithWriteBarrier((void **)values + i, (void *)newValue);
        }
        result = (i == c) ? CFArrayCreate(allocator, values, c, &kCFTypeArrayCallBacks) : NULL;
        c = i;
        for (i = 0; i < c; i ++) CFRelease(values[i]);
        free_cftype_array(values);
    }
    return result;
}

static CFMutableArrayRef _arrayDeepMutableCopy(CFAllocatorRef allocator, CFArrayRef array, CFOptionFlags mutabilityOption) {
    CFIndex i, c = CFArrayGetCount(array);
    CFMutableArrayRef result = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);
    if (result) {
        for (i = 0; i < c; i ++) {
            CFTypeRef newValue = CFPropertyListCreateDeepCopy(allocator, CFArrayGetValueAtIndex(array, i), mutabilityOption);
            if (!newValue) break;
            CFArrayAppendValue(result, newValue);
            CFRelease(newValue);
        }
        if (i != c) {
            CFRelease(result);
            result = NULL;
        }
    }
    return result;
}

CFPropertyListRef CFPropertyListCreateDeepCopy(CFAllocatorRef allocator, CFPropertyListRef propertyList, CFOptionFlags mutabilityOption) {
    initStatics();
    CFPropertyListRef result = NULL;
    CFAssert1(propertyList != NULL, __kCFLogAssertion, "%s(): cannot copy a NULL property list", __PRETTY_FUNCTION__);
    __CFAssertIsPList(propertyList);
    CFAssert2(mutabilityOption == kCFPropertyListImmutable || mutabilityOption == kCFPropertyListMutableContainers || mutabilityOption == kCFPropertyListMutableContainersAndLeaves, __kCFLogAssertion, "%s(): Unrecognized option %d", __PRETTY_FUNCTION__, mutabilityOption);
	if (!CFPropertyListIsValid(propertyList, kCFPropertyListBinaryFormat_v1_0)) return NULL;
    
    CFTypeID typeID = CFGetTypeID(propertyList);
    if (typeID == dicttype) {
        CFDictionaryRef dict = (CFDictionaryRef)propertyList;
        Boolean isMutable = (mutabilityOption != kCFPropertyListImmutable);
        CFIndex count = CFDictionaryGetCount(dict);
        if (count == 0) {
            result = isMutable ? CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks): CFDictionaryCreate(allocator, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        } else {
            new_cftype_array(keys, 2 * count);
            CFTypeRef *values;
            CFIndex i;
            values = keys+count;
            CFDictionaryGetKeysAndValues(dict, keys, values);
            for (i = 0; i < count; i ++) {
                CFTypeRef newKey = CFStringCreateCopy(allocator, (CFStringRef)keys[i]);
                if (newKey == NULL) {
                    break;
                }
                __CFAssignWithWriteBarrier((void **)keys + i, (void *)newKey);
                CFTypeRef newValue = CFPropertyListCreateDeepCopy(allocator, values[i], mutabilityOption);
                if (newValue == NULL) {
                    CFRelease(keys[i]);
                    break;
                }
                __CFAssignWithWriteBarrier((void **)values + i, (void *)newValue);
            }
            if (i == count) {
                result = isMutable ? CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) : CFDictionaryCreate(allocator, keys, values, count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                for (i = 0; i < count; i ++) {
                    if (isMutable) {
                        CFDictionarySetValue((CFMutableDictionaryRef)result, keys[i], values[i]);
                    }
                    CFRelease(keys[i]);
                    CFRelease(values[i]);
                }
            } else {
                result = NULL;
                count = i;
                for (i = 0; i < count; i ++) {
                    CFRelease(keys[i]);
                    CFRelease(values[i]);
                }
            }
            free_cftype_array(keys);
        }
    } else if (typeID == arraytype) {
        if (mutabilityOption == kCFPropertyListImmutable) {
            result = _arrayDeepImmutableCopy(allocator, (CFArrayRef)propertyList, mutabilityOption);
        } else {
            result = _arrayDeepMutableCopy(allocator, (CFArrayRef)propertyList, mutabilityOption);
        }
    } else if (typeID == datatype) {
        if (mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
            result = CFDataCreateMutableCopy(allocator, 0, (CFDataRef)propertyList);
        } else {
            result = CFDataCreateCopy(allocator, (CFDataRef)propertyList);
        }
    } else if (typeID == numbertype) {
        // Warning - this will break if byteSize is ever greater than 128
        uint8_t bytes[128];
        CFNumberType numType = _CFNumberGetType2((CFNumberRef)propertyList);
        CFNumberGetValue((CFNumberRef)propertyList, numType, (void *)bytes);
        result = CFNumberCreate(allocator, numType, (void *)bytes);
    } else if (typeID == booltype) {
        // Booleans are immutable & shared instances
        CFRetain(propertyList);
        result = propertyList;
    } else if (typeID == datetype) {
        // Dates are immutable
        result = CFDateCreate(allocator, CFDateGetAbsoluteTime((CFDateRef)propertyList));
    } else if (typeID == stringtype) {
        if (mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
            result = CFStringCreateMutableCopy(allocator, 0, (CFStringRef)propertyList);
        } else {
            result = CFStringCreateCopy(allocator, (CFStringRef)propertyList);
        }
    } else {
        CFAssert2(false, __kCFLogAssertion, "%s(): %p is not a property list type", __PRETTY_FUNCTION__, propertyList);
        result = NULL;
    }
    return result;
}

