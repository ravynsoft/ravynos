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

/*	CFOldStylePList.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Tony Parker
*/

#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFStringEncodingConverter.h>
#include "CFInternal.h"
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFSet.h>

#include <ctype.h>

//
// Old NeXT-style property lists
//

CF_INLINE void __CFPListRelease(CFTypeRef cf, CFAllocatorRef allocator) {
    if (cf && !(0)) CFRelease(cf);
}

CF_PRIVATE CFErrorRef __CFPropertyListCreateError(CFIndex code, CFStringRef debugString, ...);

typedef struct {
    const UniChar *begin;
    const UniChar *curr;
    const UniChar *end;
    CFErrorRef error;
    CFAllocatorRef allocator;
    UInt32 mutabilityOption;
    CFMutableSetRef stringSet;  // set of all strings involved in this parse; allows us to share non-mutable strings in the returned plist
} _CFStringsFileParseInfo;

// warning: doesn't have a good idea of Unicode line separators
static UInt32 lineNumberStrings(_CFStringsFileParseInfo *pInfo) {
    const UniChar *p = pInfo->begin;
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

static CFTypeRef parsePlistObject(_CFStringsFileParseInfo *pInfo, bool requireObject);

#define isValidUnquotedStringCharacter(x) (((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z') || ((x) >= '0' && (x) <= '9') || (x) == '_' || (x) == '$' || (x) == '/' || (x) == ':' || (x) == '.' || (x) == '-')

// Returns true if the advance found something before the end of the buffer, false otherwise
static Boolean advanceToNonSpace(_CFStringsFileParseInfo *pInfo) {
    UniChar ch2;
    while (pInfo->curr < pInfo->end) {
	ch2 = *(pInfo->curr);
        pInfo->curr ++;
        if (ch2 >= 9 && ch2 <= 0x0d) continue;	// tab, newline, vt, form feed, carriage return
        if (ch2 == ' ' || ch2 == 0x2028 || ch2 == 0x2029) continue;	// space and Unicode line sep, para sep
	if (ch2 == '/') {
            if (pInfo->curr >= pInfo->end) {
                // whoops; back up and return
                pInfo->curr --;
                return true;
            } else if (*(pInfo->curr) == '/') {
                pInfo->curr ++;
                while (pInfo->curr < pInfo->end) {	// go to end of comment line
                    UniChar ch3 = *(pInfo->curr);
                    if (ch3 == '\n' || ch3 == '\r' || ch3 == 0x2028 || ch3 == 0x2029) break;
                    pInfo->curr ++;
		}
	    } else if (*(pInfo->curr) == '*') {		// handle /* ... */
                pInfo->curr ++;
		while (pInfo->curr < pInfo->end) {
		    ch2 = *(pInfo->curr);
                    pInfo->curr ++;
		    if (ch2 == '*' && pInfo->curr < pInfo->end && *(pInfo->curr) == '/') {
                        pInfo->curr ++; // advance past the '/'
                        break;
                    }
                }
            } else {
                pInfo->curr --;
                return true;
	    }
        } else {
            pInfo->curr --;
            return true;
        }
    }
    return false;
}

static UniChar getSlashedChar(_CFStringsFileParseInfo *pInfo) {
    UniChar ch = *(pInfo->curr);
    pInfo->curr ++;
    switch (ch) {
	case '0':
	case '1':	
	case '2':	
	case '3':	
	case '4':	
	case '5':	
	case '6':	
	case '7':  {
            uint8_t num = ch - '0';
            UniChar result;
            CFIndex usedCharLen;
	    /* three digits maximum to avoid reading \000 followed by 5 as \5 ! */
	    if ((ch = *(pInfo->curr)) >= '0' && ch <= '7') { // we use in this test the fact that the buffer is zero-terminated
                pInfo->curr ++;
		num = (num << 3) + ch - '0';
		if ((pInfo->curr < pInfo->end) && (ch = *(pInfo->curr)) >= '0' && ch <= '7') {
                    pInfo->curr ++;
		    num = (num << 3) + ch - '0';
		}
	    }
            CFStringEncodingBytesToUnicode(kCFStringEncodingNextStepLatin, 0, &num, sizeof(uint8_t), NULL,  &result, 1, &usedCharLen);
            return (usedCharLen == 1) ? result : 0;
	}
	case 'U': {
	    unsigned num = 0, numDigits = 4;	/* Parse four digits */
	    while (pInfo->curr < pInfo->end && numDigits--) {
                if (((ch = *(pInfo->curr)) < 128) && isxdigit(ch)) { 
                    pInfo->curr ++;
		    num = (num << 4) + ((ch <= '9') ? (ch - '0') : ((ch <= 'F') ? (ch - 'A' + 10) : (ch - 'a' + 10)));
		}
	    }
	    return num;
	}
	case 'a':	return '\a';	// Note: the meaning of '\a' varies with -traditional to gcc
	case 'b':	return '\b';
	case 'f':	return '\f';
	case 'n':	return '\n';
	case 'r':	return '\r';
	case 't':	return '\t';
	case 'v':	return '\v';
	case '"':	return '\"';
	case '\n':	return '\n';
    }
    return ch;
}

static CFStringRef _uniqueStringForCharacters(_CFStringsFileParseInfo *pInfo, const UniChar *base, CFIndex length) {
    if (0 == length) return !(0) ? (CFStringRef)CFRetain(CFSTR("")) : CFSTR("");
    // This is to avoid having to promote the buffers of all the strings compared against
    // during the set probe; if a Unicode string is passed in, that's what happens.
    CFStringRef stringToUnique = NULL;
    Boolean use_stack = (length < 2048);
    STACK_BUFFER_DECL(uint8_t, buffer, use_stack ? length + 1 : 1);
    uint8_t *ascii = use_stack ? buffer : (uint8_t *)CFAllocatorAllocate(kCFAllocatorSystemDefault, length + 1, 0);
    for (CFIndex idx = 0; idx < length; idx++) {
        UniChar ch = base[idx];
	if (ch < 0x80) {
	    ascii[idx] = (uint8_t)ch;
        } else {
	    stringToUnique = CFStringCreateWithCharacters(pInfo->allocator, base, length);
	    break;
	}
    }
    if (!stringToUnique) {
        ascii[length] = '\0';
        stringToUnique = CFStringCreateWithBytes(pInfo->allocator, ascii, length, kCFStringEncodingASCII, false);
    }
    if (ascii != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, ascii);
    CFStringRef uniqued = (CFStringRef)CFSetGetValue(pInfo->stringSet, stringToUnique);
    if (!uniqued) {
        CFSetAddValue(pInfo->stringSet, stringToUnique);
	uniqued = stringToUnique;
    }
    __CFPListRelease(stringToUnique, pInfo->allocator);
    if (uniqued && !(0)) CFRetain(uniqued);
    return uniqued;
}

static CFStringRef _uniqueStringForString(_CFStringsFileParseInfo *pInfo, CFStringRef stringToUnique) {
    CFStringRef uniqued = (CFStringRef)CFSetGetValue(pInfo->stringSet, stringToUnique);
    if (!uniqued) {
        uniqued = (CFStringRef)__CFStringCollectionCopy(pInfo->allocator, stringToUnique);
        CFSetAddValue(pInfo->stringSet, uniqued);
        __CFTypeCollectionRelease(pInfo->allocator, uniqued);
    }
    if (uniqued && !(0)) CFRetain(uniqued);
    return uniqued;
}

static CFStringRef parseQuotedPlistString(_CFStringsFileParseInfo *pInfo, UniChar quote) {
    CFMutableStringRef str = NULL;
    const UniChar *startMark = pInfo->curr;
    const UniChar *mark = pInfo->curr;
    while (pInfo->curr < pInfo->end) {
	UniChar ch = *(pInfo->curr);
        if (ch == quote) break;
        if (ch == '\\') {
            if (!str) str = CFStringCreateMutable(pInfo->allocator, 0);
            CFStringAppendCharacters(str, mark, pInfo->curr - mark);
            pInfo->curr ++;
            ch = getSlashedChar(pInfo);
            CFStringAppendCharacters(str, &ch, 1);
            mark = pInfo->curr;
	} else {
            // Note that the original NSParser code was much more complex at this point, but it had to deal with 8-bit characters in a non-UniChar stream.  We always have UniChar (we translated the data by the system encoding at the very beginning, hopefully), so this is safe.
            pInfo->curr ++;
        }
    }
    if (pInfo->end <= pInfo->curr) {
        __CFPListRelease(str, pInfo->allocator);
        pInfo->curr = startMark;
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unterminated quoted string starting on line %d"), lineNumberStrings(pInfo));
        return NULL;
    }
    if (!str) {
        if (pInfo->mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
            str = CFStringCreateMutable(pInfo->allocator, 0);
            CFStringAppendCharacters(str, mark, pInfo->curr - mark);
        } else {
            str = (CFMutableStringRef)_uniqueStringForCharacters(pInfo, mark, pInfo->curr-mark);
        }
    } else {
        if (mark != pInfo->curr) {
            CFStringAppendCharacters(str, mark, pInfo->curr - mark);
        }
        if (pInfo->mutabilityOption != kCFPropertyListMutableContainersAndLeaves) {
            CFStringRef uniqueString = _uniqueStringForString(pInfo, str);
            __CFPListRelease(str, pInfo->allocator);
            str = (CFMutableStringRef)uniqueString;
        }
    }
    pInfo->curr ++;  // Advance past the quote character before returning.
    if (pInfo->error) {
        CFRelease(pInfo->error);
        pInfo->error = NULL;
    }
    return str;
}

static CFStringRef parseUnquotedPlistString(_CFStringsFileParseInfo *pInfo) {
    const UniChar *mark = pInfo->curr;
    while (pInfo->curr < pInfo->end) {
        UniChar ch = *pInfo->curr;
        if (isValidUnquotedStringCharacter(ch))
            pInfo->curr ++;
        else break;
    }
    if (pInfo->curr != mark) {
        if (pInfo->mutabilityOption != kCFPropertyListMutableContainersAndLeaves) {
            CFStringRef str = _uniqueStringForCharacters(pInfo, mark, pInfo->curr-mark);
            return str;
        } else {
            CFMutableStringRef str = CFStringCreateMutable(pInfo->allocator, 0);
            CFStringAppendCharacters(str, mark, pInfo->curr - mark);
            return str;
        }
    }
    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected EOF"));
    return NULL;
}

static CFStringRef parsePlistString(_CFStringsFileParseInfo *pInfo, bool requireObject) {
    UniChar ch;
    Boolean foundChar = advanceToNonSpace(pInfo);
    if (!foundChar) {
        if (requireObject) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected EOF while parsing string"));
        }
        return NULL;
    }
    ch = *(pInfo->curr);
    if (ch == '\'' || ch == '\"') {
        pInfo->curr ++;
        return parseQuotedPlistString(pInfo, ch);
    } else if (isValidUnquotedStringCharacter(ch)) {
        return parseUnquotedPlistString(pInfo);
    } else {
        if (requireObject) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Invalid string character at line %d"), lineNumberStrings(pInfo));
	}
        return NULL;
    }
}

static CFTypeRef parsePlistArray(_CFStringsFileParseInfo *pInfo) {
    CFMutableArrayRef array = CFArrayCreateMutable(pInfo->allocator, 0, &kCFTypeArrayCallBacks);
    CFTypeRef tmp = parsePlistObject(pInfo, false);
    Boolean foundChar;
    while (tmp) {
        CFArrayAppendValue(array, tmp);
        __CFPListRelease(tmp, pInfo->allocator);
        foundChar = advanceToNonSpace(pInfo);
	if (!foundChar) {
	    __CFPListRelease(array, pInfo->allocator);
	    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected ',' for array at line %d"), lineNumberStrings(pInfo));
	    return NULL;
	}
        if (*pInfo->curr != ',') {
            tmp = NULL;
        } else {
            pInfo->curr ++;
            tmp = parsePlistObject(pInfo, false);
        }
    }
    foundChar = advanceToNonSpace(pInfo);
    if (!foundChar || *pInfo->curr != ')') {
        __CFPListRelease(array, pInfo->allocator);
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected terminating ')' for array at line %d"), lineNumberStrings(pInfo));
        return NULL;
    }
    if (pInfo->error) {
        CFRelease(pInfo->error);
        pInfo->error = NULL;
    }
    pInfo->curr ++;
    return array;
}

__attribute__((noinline)) void _CFPropertyListMissingSemicolon(UInt32 line) {
    CFLog(kCFLogLevelWarning, CFSTR("CFPropertyListCreateFromXMLData(): Old-style plist parser: missing semicolon in dictionary on line %d. Parsing will be abandoned. Break on _CFPropertyListMissingSemicolon to debug."), line);
}

__attribute__((noinline)) void _CFPropertyListMissingSemicolonOrValue(UInt32 line) {
    CFLog(kCFLogLevelWarning, CFSTR("CFPropertyListCreateFromXMLData(): Old-style plist parser: missing semicolon or value in dictionary on line %d. Parsing will be abandoned. Break on _CFPropertyListMissingSemicolonOrValue to debug."), line);
}

static CFDictionaryRef parsePlistDictContent(_CFStringsFileParseInfo *pInfo) {
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(pInfo->allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFStringRef key = NULL;
    Boolean failedParse = false;
    key = parsePlistString(pInfo, false);
    while (key) {
        CFTypeRef value;
        Boolean foundChar = advanceToNonSpace(pInfo);
        if (!foundChar) {
            UInt32 line = lineNumberStrings(pInfo);
            _CFPropertyListMissingSemicolonOrValue(line);
            failedParse = true;
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Missing ';' on line %d"), line);
            break;
        }
	
	if (*pInfo->curr == ';') {
	    /* This is a strings file using the shortcut format */
	    /* although this check here really applies to all plists. */
	    value = CFRetain(key);
	} else if (*pInfo->curr == '=') {
	    pInfo->curr ++;
	    value = parsePlistObject(pInfo, true);
	    if (!value) {
		failedParse = true;
		break;
	    }
	} else {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected ';' or '=' after key at line %d"), lineNumberStrings(pInfo));
	    failedParse = true;
	    break;
	}
	CFDictionarySetValue(dict, key, value);
	__CFPListRelease(key, pInfo->allocator);
	key = NULL;
	__CFPListRelease(value, pInfo->allocator);
	value = NULL;
	foundChar = advanceToNonSpace(pInfo);
	if (foundChar && *pInfo->curr == ';') {
	    pInfo->curr ++;
	    key = parsePlistString(pInfo, false);
	} else if (true || !foundChar) {
            UInt32 line = lineNumberStrings(pInfo);
            _CFPropertyListMissingSemicolon(line);
	    failedParse = true;
	    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Missing ';' on line %d"), line);
	}
    }
    
    if (failedParse) {
        __CFPListRelease(key, pInfo->allocator);
        __CFPListRelease(dict, pInfo->allocator);
        return NULL;
    }
    if (pInfo->error) {
        CFRelease(pInfo->error);
        pInfo->error = NULL;
    }
    return dict;
}

static CFTypeRef parsePlistDict(_CFStringsFileParseInfo *pInfo) {
    CFDictionaryRef dict = parsePlistDictContent(pInfo);
    if (!dict) return NULL;
    Boolean foundChar = advanceToNonSpace(pInfo);
    if (!foundChar || *pInfo->curr != '}') {
        __CFPListRelease(dict, pInfo->allocator);
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected terminating '}' for dictionary at line %d"), lineNumberStrings(pInfo));
        return NULL;
    }
    pInfo->curr ++;
    return dict;
}

CF_INLINE unsigned char fromHexDigit(unsigned char ch) {
    if (isdigit(ch)) return ch - '0';
    if ((ch >= 'a') && (ch <= 'f')) return ch - 'a' + 10;
    if ((ch >= 'A') && (ch <= 'F')) return ch - 'A' + 10;
    return 0xff; // Just choose a large number for the error code
}

/* Gets up to bytesSize bytes from a plist data. Returns number of bytes actually read. Leaves cursor at first non-space, non-hex character.
 -1 is returned for unexpected char, -2 for uneven number of hex digits
 */
static int getDataBytes(_CFStringsFileParseInfo *pInfo, unsigned char *bytes, int bytesSize) {
    int numBytesRead = 0;
    while ((pInfo->curr < pInfo->end) && (numBytesRead < bytesSize)) {
	int first, second;
	UniChar ch1 = *pInfo->curr;
	if (ch1 == '>') return numBytesRead;  // Meaning we're done
	first = fromHexDigit((unsigned char)ch1);
	if (first != 0xff) {	// If the first char is a hex, then try to read a second hex
	    pInfo->curr++;
	    if (pInfo->curr >= pInfo->end) return -2;   // Error: uneven number of hex digits
	    UniChar ch2 = *pInfo->curr;
	    second = fromHexDigit((unsigned char)ch2);
	    if (second == 0xff) return -2;  // Error: uneven number of hex digits
	    bytes[numBytesRead++] = (first << 4) + second;
	    pInfo->curr++;
	} else if (ch1 == ' ' || ch1 == '\n' || ch1 == '\t' || ch1 == '\r' || ch1 == 0x2028 || ch1 == 0x2029) {
	    pInfo->curr++;
	} else {
	    return -1;  // Error: unexpected character
	}
    }
    return numBytesRead;    // This does likely mean we didn't encounter a '>', but we'll let the caller deal with that
}

#define numBytes 400
static CFTypeRef parsePlistData(_CFStringsFileParseInfo *pInfo) {
    CFMutableDataRef result = CFDataCreateMutable(pInfo->allocator, 0);
    
    // Read hex bytes and append them to result
    while (1) {
	unsigned char bytes[numBytes];
	int numBytesRead = getDataBytes(pInfo, bytes, numBytes);
	if (numBytesRead < 0) {
	    __CFPListRelease(result, pInfo->allocator);
            switch (numBytesRead) {
                case -2: 
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Malformed data byte group at line %d; uneven length"), lineNumberStrings(pInfo));
                    break;
                default: 
                    pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Malformed data byte group at line %d; invalid hex"), lineNumberStrings(pInfo));
                    break;
            }
	    return NULL;
	}
	if (numBytesRead == 0) break;
	CFDataAppendBytes(result, bytes, numBytesRead);
    }
    
    if (pInfo->error) {
        CFRelease(pInfo->error);
        pInfo->error = NULL;
    }
    
    if (*(pInfo->curr) == '>') {
        pInfo->curr ++; // Move past '>'
        return result;
    } else {
        __CFPListRelease(result, pInfo->allocator);
        pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected terminating '>' for data at line %d"), lineNumberStrings(pInfo));
        return NULL;
    }
}
#undef numBytes

// Returned object is retained; caller must free.
static CFTypeRef parsePlistObject(_CFStringsFileParseInfo *pInfo, bool requireObject) {
    UniChar ch;
    Boolean foundChar = advanceToNonSpace(pInfo);
    if (!foundChar) {
        if (requireObject) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected EOF while parsing plist"));
        }
        return NULL;
    }
    ch = *(pInfo->curr);
    pInfo->curr ++;
    if (ch == '{') {
        return parsePlistDict(pInfo);
    } else if (ch == '(') {
        return parsePlistArray(pInfo);
    } else if (ch == '<') {
        return parsePlistData(pInfo);
    } else if (ch == '\'' || ch == '\"') {
        return parseQuotedPlistString(pInfo, ch);
    } else if (isValidUnquotedStringCharacter(ch)) {
        pInfo->curr --;
        return parseUnquotedPlistString(pInfo);
    } else {
        pInfo->curr --;  // Must back off the charcter we just read
        if (requireObject) {
            pInfo->error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected character '0x%x' at line %d"), ch, lineNumberStrings(pInfo));
        }
        return NULL;
    }
}

// CFAllocatorRef allocator, CFDataRef xmlData, CFStringRef originalString, CFStringEncoding guessedEncoding, CFOptionFlags option, CFErrorRef *outError, Boolean allowNewTypes, CFPropertyListFormat *format, CFSetRef keyPaths

CF_PRIVATE CFTypeRef __CFCreateOldStylePropertyListOrStringsFile(CFAllocatorRef allocator, CFDataRef xmlData, CFStringRef originalString, CFStringEncoding guessedEncoding, CFOptionFlags option, CFErrorRef *outError,CFPropertyListFormat *format) {
    
    // Convert the string to UTF16 for parsing old-style
    if (originalString) {
        // Ensure that originalString is not collected while we are using it
        CFRetain(originalString);
    } else {
        originalString = CFStringCreateWithBytes(kCFAllocatorSystemDefault, CFDataGetBytePtr(xmlData), CFDataGetLength(xmlData), guessedEncoding, NO);
        if (!originalString) {
            // Couldn't convert
            if (outError) *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Conversion of string failed."));
            return NULL;
        }
    }
        
    UInt32 length;
    Boolean createdBuffer = false;
    length = CFStringGetLength(originalString);
    if (!length) {
        if (outError) *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Conversion of string failed. The string is empty."));
        return NULL;
    }
    
    UniChar *buf = (UniChar *)CFStringGetCharactersPtr(originalString);
    if (!buf) {
        buf = (UniChar *)CFAllocatorAllocate(allocator, length * sizeof(UniChar), 0);
        if (!buf) {
            CRSetCrashLogMessage("CFPropertyList ran out of memory while attempting to allocate temporary storage.");
            return NULL;
        }
        CFStringGetCharacters(originalString, CFRangeMake(0, length), buf);
        createdBuffer = true;
    }
    
    _CFStringsFileParseInfo stringsPInfo;
    stringsPInfo.begin = buf;
    stringsPInfo.end = buf+length;
    stringsPInfo.curr = buf;
    stringsPInfo.allocator = allocator;
    stringsPInfo.mutabilityOption = option;
    stringsPInfo.stringSet = CFSetCreateMutable(allocator, 0, &kCFTypeSetCallBacks);
    stringsPInfo.error = NULL;
    
    const UniChar *begin = stringsPInfo.curr;
    CFTypeRef result = NULL;
    Boolean foundChar = advanceToNonSpace(&stringsPInfo);
    if (!foundChar) {
        // A file consisting only of whitespace (or empty) is now defined to be an empty dictionary
        result = CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    } else {
        result = parsePlistObject(&stringsPInfo, true);
        if (result) {
            foundChar = advanceToNonSpace(&stringsPInfo);
            if (foundChar) {
                if (CFGetTypeID(result) != CFStringGetTypeID()) {
                    __CFPListRelease(result, allocator);
                    result = NULL;
                    if (stringsPInfo.error) CFRelease(stringsPInfo.error);
                    stringsPInfo.error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Junk after plist at line %d"), lineNumberStrings(&stringsPInfo));
                } else {
                    // Reset info and keep parsing
                    __CFPListRelease(result, allocator);
                    if (stringsPInfo.error) CFRelease(stringsPInfo.error);
                    stringsPInfo.error = NULL;
                    
                    // Check for a strings file (looks like a dictionary without the opening/closing curly braces)
                    stringsPInfo.curr = begin;
                    result = parsePlistDictContent(&stringsPInfo);
                }
            }
        }
    }
    
    if (!result) {
        // Must return some kind of error if requested
        if (outError) {
            if (stringsPInfo.error) {
                // Transfer ownership
                *outError = stringsPInfo.error;
            } else {
                *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unknown error parsing property list around line %d"), lineNumberStrings(&stringsPInfo));
            }
        } else if (stringsPInfo.error) {
            // Caller doesn't want it, so we need to free it
            CFRelease(stringsPInfo.error);
        }
    }
    
    if (result && format) *format = kCFPropertyListOpenStepFormat;
    
    if (createdBuffer && !(0)) CFAllocatorDeallocate(allocator, buf);
    CFRelease(stringsPInfo.stringSet);
    CFRelease(originalString);
    return result;
}

#undef isValidUnquotedStringCharacter
