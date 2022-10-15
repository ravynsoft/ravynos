/*	CFOldStylePList.c
 Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 Responsibility: Tony Parker
 */

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFStringEncodingConverter.h>
#include "CFInternal.h"
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFSet.h>

#include <ctype.h>
#include "CFOverflow.h"

//
// Old NeXT-style property lists
//

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

static void parseInfo_setError(_CFStringsFileParseInfo *const pInfo, CFErrorRef _Nonnull const CF_RELEASES_ARGUMENT error) {
    CFErrorRef oldError = pInfo->error;
    if (oldError) {
        CFRelease(oldError);
    }
    pInfo->error = error;
}

static void parseInfo_clearError(_CFStringsFileParseInfo *const pInfo) {
    if (pInfo->error) {
        CFRelease(pInfo->error);
        pInfo->error = NULL;
    }
}

// warning: doesn't have a good idea of Unicode line separators
static UInt32 lineNumberStrings(_CFStringsFileParseInfo *pInfo) {
    const UniChar *p = pInfo->begin;
    UInt32 count = 1;
    while (p < pInfo->end && p < pInfo->curr) {
        if (*p == '\r') {
            count ++;
            if (p + 1 < pInfo->end && p + 1 < pInfo->curr && *(p + 1) == '\n') {
                p ++;
            }
        } else if (*p == '\n') {
            count ++;
        }
        p ++;
    }
    return count;
}

static CFTypeRef parsePlistObject(_CFStringsFileParseInfo *pInfo, bool requireObject, const uint32_t depth) CF_RETURNS_RETAINED;

#define isValidUnquotedStringCharacter(x) (((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z') || ((x) >= '0' && (x) <= '9') || (x) == '_' || (x) == '$' || (x) == '/' || (x) == ':' || (x) == '.' || (x) == '-')

// Returns true if the advance found something before the end of the buffer, false otherwise
// AFTER-INVARIANT: pInfo->curr <= pInfo->end
//                  However result will be false when pInfo->curr == pInfo->end
static Boolean advanceToNonSpace(_CFStringsFileParseInfo *pInfo) {
    UniChar ch2;
    while (pInfo->curr < pInfo->end) {
        ch2 = *(pInfo->curr);
        pInfo->curr ++;
        if (ch2 >= 9 && ch2 <= 0x0d) continue;	// tab, newline, vt, form feed, carriage return
        if (ch2 == ' ' || ch2 == 0x2028 || ch2 == 0x2029) continue;	// space and Unicode line sep, para sep
        if (ch2 == '/') {
            if (pInfo->curr >= pInfo->end) {
                // TODO: this could possibly be made more robust (imagine if pInfo->curr is stomped; pInfo->end - 1 might be safer.  Unless its smashed too :-/
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
            } else if (*(pInfo->curr) == '*') {        // handle /* ... */
                pInfo->curr ++;
                while (pInfo->curr < pInfo->end) {
                    ch2 = *(pInfo->curr);
                    pInfo->curr ++;
                    if (ch2 == '*' && pInfo->curr < pInfo->end && *(pInfo->curr) == '/') {
                        pInfo->curr ++; // advance past the '/'
                        break;
                    }
                }
            } else { // this looked like the start of a comment, but wasn't
                pInfo->curr --;
                return true;
            }
        } else { // this didn't look like a comment, we've found non-whitespace
            pInfo->curr --;
            return true;
        }
    }
    return false;
}

static UniChar getSlashedChar(_CFStringsFileParseInfo *pInfo) {
    CFAssert(pInfo->curr < pInfo->end, __kCFLogAssertion, "Cannot read character after '\\' if at end.");
    
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
            // We've already read the first character here, so repeat at most two more times.
            for (uint8_t i = 0; i < 2; i += 1) {
                // Hitting the end of the plist is not a meaningful error here.
                // We parse the characters we have and allow the parent context (parseQuotedPlistString, the only current call site of getSlashedChar) to produce a more meaningful error message (e.g. it will at least expect a close quote after this character).
                if (pInfo->curr >= pInfo->end) {
                    break;
                }

                if ((ch = *(pInfo->curr)) >= '0' && ch <= '7') {
                    num = (num << 3) + ch - '0';
                    pInfo->curr ++;
                } else {
                    // Non-octal characters are not explicitly an error either: "\05" is a valid character which evaluates to 005. (We read a '0', a '5', and then a '"'; we can't bail on seeing '"' though.)
                    // Is this an ambiguous format? Probably. But it has to remain this way for backwards compatibility.
                    // See <rdar://problem/34321354>
                    break;
                }
            }
            
            const uint32_t conversionResult = CFStringEncodingBytesToUnicode(kCFStringEncodingNextStepLatin, 0, &num, sizeof(uint8_t), NULL,  &result, 1, &usedCharLen);
            if (conversionResult != kCFStringEncodingConversionSuccess) {
                parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to convert octet-stream while parsing plist")));
                return 0;
            } else {
                return (usedCharLen == 1) ? result : 0;
            }
        } break;
        case 'U': {
            unsigned num = 0, numDigits = 4;    /* Parse four digits */
            while (pInfo->curr < pInfo->end && numDigits--) {
                if (((ch = *(pInfo->curr)) < 128) && isxdigit(ch)) {
                    pInfo->curr ++;
                    num = (num << 4) + ((ch <= '9') ? (ch - '0') : ((ch <= 'F') ? (ch - 'A' + 10) : (ch - 'a' + 10)));
                }
            }
            return num;
        } break;
        case 'a':    return '\a';    // Note: the meaning of '\a' varies with -traditional to gcc
        case 'b':    return '\b';
        case 'f':    return '\f';
        case 'n':    return '\n';
        case 'r':    return '\r';
        case 't':    return '\t';
        case 'v':    return '\v';
        case '"':    return '\"';
        case '\n':    return '\n';
    }
    return ch;
}

static CFStringRef _uniqueStringForCharacters(_CFStringsFileParseInfo *pInfo, const UniChar *base, CFIndex length) CF_RETURNS_RETAINED {
    if (0 == length) { return (CFStringRef)CFRetain(CFSTR("")); }
    // This is to avoid having to promote the buffers of all the strings compared against
    // during the set probe; if a Unicode string is passed in, that's what happens.
    CFStringRef stringToUnique = NULL;
    const Boolean use_stack = (length < 2048);
    STACK_BUFFER_DECL(uint8_t, buffer, use_stack ? length + 1 : 1);
    uint8_t *ascii = use_stack ? buffer : (uint8_t *)CFAllocatorAllocate(kCFAllocatorSystemDefault, length + 1, 0);
    if (ascii == NULL) {
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate string while parsing plist")));
        return NULL;
    }
    
    BOOL gotString = NO;
    for (CFIndex idx = 0; idx < length; idx++) {
        UniChar ch = base[idx];
        if (ch < 0x80) {
            ascii[idx] = (uint8_t)ch;
        } else {
            stringToUnique = CFStringCreateWithCharacters(pInfo->allocator, base, length);
            if (stringToUnique == NULL) {
                parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate pre-unique string while parsing plist")));
                return NULL;
            } else {
                gotString = YES;
            }
            break;
        }
    }
    if (!gotString) {
        ascii[length] = '\0';
        stringToUnique = CFStringCreateWithBytes(pInfo->allocator, ascii, length, kCFStringEncodingASCII, false);
        if (stringToUnique == NULL) {
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate ascii string while parsing plist")));
            return NULL;
        }
    }
    if (ascii != buffer) { CFAllocatorDeallocate(kCFAllocatorSystemDefault, ascii); }
    CFStringRef uniqued = (CFStringRef)CFSetGetValue(pInfo->stringSet, stringToUnique);
    if (!uniqued) {
        CFSetAddValue(pInfo->stringSet, stringToUnique);
        uniqued = stringToUnique;
    }
    if (stringToUnique) { CFRelease(stringToUnique); }
    if (uniqued) { CFRetain(uniqued); }
    return uniqued;
}

static CFStringRef _uniqueStringForString(_CFStringsFileParseInfo *pInfo, CFStringRef stringToUnique) CF_RETURNS_RETAINED {
    CFStringRef uniqued = (CFStringRef)CFSetGetValue(pInfo->stringSet, stringToUnique);
    if (!uniqued) {
        uniqued = (CFStringRef)__CFStringCollectionCopy(pInfo->allocator, stringToUnique);
        if (uniqued == NULL) {
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to copy string while parsing plist")));
            return NULL;
        } else {
            CFSetAddValue(pInfo->stringSet, uniqued);
            __CFTypeCollectionRelease(pInfo->allocator, uniqued);
        }
    }
    if (uniqued) CFRetain(uniqued);
        return uniqued;
}

static CFStringRef parseQuotedPlistString(_CFStringsFileParseInfo *pInfo, UniChar quote) CF_RETURNS_RETAINED {
    CFMutableStringRef str = NULL;
    const UniChar *startMark = pInfo->curr;
    const UniChar *mark = pInfo->curr;
    while (pInfo->curr < pInfo->end) {
        UniChar ch = *(pInfo->curr);
        if (ch == quote) break;
        if (ch == '\\') {
            if (!str) { str = CFStringCreateMutable(pInfo->allocator, 0); }
            if (str == NULL) {
                parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate quoted string while parsing plist")));
                return NULL;
            }
            CFStringAppendCharacters(str, mark, pInfo->curr - mark);
            pInfo->curr ++;
            
            if (pInfo->curr == pInfo->end) {
                if (str) { CFRelease(str); }
                parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unterminated backslash sequence on line %d"), lineNumberStrings(pInfo)));
                return NULL;
            }
            
            ch = getSlashedChar(pInfo);
            CFStringAppendCharacters(str, &ch, 1);
            mark = pInfo->curr;
        } else {
            // Note that the original NSParser code was much more complex at this point, but it had to deal with 8-bit characters in a non-UniChar stream.  We always have UniChar (we translated the data by the system encoding at the very beginning, hopefully), so this is safe.
            pInfo->curr ++;
        }
    }
    if (pInfo->end <= pInfo->curr) {
        if (str) { CFRelease(str); }
        pInfo->curr = startMark;
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unterminated quoted string starting on line %d"), lineNumberStrings(pInfo)));
        return NULL;
    }
    if (!str) {
        if (pInfo->mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
            str = CFStringCreateMutable(pInfo->allocator, 0);
            if (str == NULL) {
                parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate mutable string while parsing plist")));
                return NULL;
            }
            if (mark != pInfo->curr) {
                CFStringAppendCharacters(str, mark, pInfo->curr - mark);
            }
        } else {
            str = (CFMutableStringRef)_uniqueStringForCharacters(pInfo, mark, pInfo->curr-mark);
        }
    } else {
        if (mark != pInfo->curr) {
            CFStringAppendCharacters(str, mark, pInfo->curr - mark);
        }
        if (pInfo->mutabilityOption != kCFPropertyListMutableContainersAndLeaves) {
            CFStringRef uniqueString = _uniqueStringForString(pInfo, str);
            if (str) CFRelease(str);
            str = (CFMutableStringRef)uniqueString;
        }
    }
    pInfo->curr ++;  // Advance past the quote character before returning.
    
    // this is a success path, so clear errors (NOTE: this seems weird, but is historical)
    parseInfo_clearError(pInfo);
    return str;
}

static CFStringRef parseUnquotedPlistString(_CFStringsFileParseInfo *pInfo) CF_RETURNS_RETAINED {
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
            if (str == NULL) {
                parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate unquoted string while parsing plist")));
                return NULL;
            }
            CFStringAppendCharacters(str, mark, pInfo->curr - mark);
            return str;
        }
    }
    parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected EOF")));
    return NULL;
}

static CFStringRef parsePlistString(_CFStringsFileParseInfo *pInfo, bool requireObject) CF_RETURNS_RETAINED {
    UniChar ch;
    Boolean foundChar = advanceToNonSpace(pInfo);
    if (!foundChar) {
        if (requireObject) {
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected EOF while parsing string")));
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
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Invalid string character at line %d"), lineNumberStrings(pInfo)));
        }
        return NULL;
    }
}

// when this returns yes, pInfo->error will be set
static BOOL depthIsInvalid(_CFStringsFileParseInfo *pInfo, const uint32_t depth) {
    BOOL invalid = NO;
#if TARGET_RT_64_BIT
#define MAX_DEPTH 512
#else
#define MAX_DEPTH 256
#endif
    if (depth > MAX_DEPTH) {
        invalid = YES;
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Too many nested arrays or dictionaries at line %d"), lineNumberStrings(pInfo)));
    }
    return invalid;
}


static CFTypeRef parsePlistArray(_CFStringsFileParseInfo *pInfo, uint32_t depth) CF_RETURNS_RETAINED {
    CFMutableArrayRef array = CFArrayCreateMutable(pInfo->allocator, 0, &kCFTypeArrayCallBacks);
    if (array == NULL) {
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate array string while parsing plist")));
        return NULL;
    }
    CFTypeRef tmp = parsePlistObject(pInfo, false, depth + 1);
    Boolean foundChar;
    while (tmp) {
        CFArrayAppendValue(array, tmp);
        if (tmp) { CFRelease(tmp); }
        foundChar = advanceToNonSpace(pInfo);
        if (!foundChar) {
            if (array) CFRelease(array);
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected ',' for array at line %d"), lineNumberStrings(pInfo)));
            return NULL;
        }
        if (*pInfo->curr != ',') {
            tmp = NULL;
        } else {
            pInfo->curr ++;
            tmp = parsePlistObject(pInfo, false, depth + 1);
        }
    }
    foundChar = advanceToNonSpace(pInfo);
    if (!foundChar || *pInfo->curr != ')') {
        if (array) { CFRelease(array); }
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected terminating ')' for array at line %d"), lineNumberStrings(pInfo)));
        return NULL;
    }
    
    // this is a success path, so clear errors (NOTE: this seems weird, but is historical)
    parseInfo_clearError(pInfo);
    
    pInfo->curr ++; // consume the )
    return array;
}

__attribute__((noinline)) void _CFPropertyListMissingSemicolon(UInt32 line) {
    CFLog(kCFLogLevelWarning, CFSTR("CFPropertyListCreateFromXMLData(): Old-style plist parser: missing semicolon in dictionary on line %d. Parsing will be abandoned. Break on _CFPropertyListMissingSemicolon to debug."), (unsigned int)line);
}

__attribute__((noinline)) void _CFPropertyListMissingSemicolonOrValue(UInt32 line) {
    CFLog(kCFLogLevelWarning, CFSTR("CFPropertyListCreateFromXMLData(): Old-style plist parser: missing semicolon or value in dictionary on line %d. Parsing will be abandoned. Break on _CFPropertyListMissingSemicolonOrValue to debug."), (unsigned int)line);
}

static CFDictionaryRef parsePlistDictContent(_CFStringsFileParseInfo *pInfo, const uint32_t depth) CF_RETURNS_RETAINED {
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(pInfo->allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (dict == NULL) {
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate dictionary while parsing plist at line %d"), lineNumberStrings(pInfo)));
        return NULL;
    }
    
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
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Missing ';' on line %d"), line));
            break;
        }
        
        if (*pInfo->curr == ';') {
            /* This is a strings file using the shortcut format */
            /* although this check here really applies to all plists. */
            value = CFRetain(key);
        } else if (*pInfo->curr == '=') {
            pInfo->curr ++;
            value = parsePlistObject(pInfo, true, depth + 1);
            if (!value) {
                failedParse = true;
                break;
            }
        } else {
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected ';' or '=' after key at line %d"), lineNumberStrings(pInfo)));
            failedParse = true;
            break;
        }
        CFDictionarySetValue(dict, key, value);
        if (key) CFRelease(key);
        key = NULL;
        if (value) CFRelease(value);
        value = NULL;
        foundChar = advanceToNonSpace(pInfo);
        if (foundChar && *pInfo->curr == ';') {
            pInfo->curr ++;
            key = parsePlistString(pInfo, false);
        } else {
            UInt32 line = lineNumberStrings(pInfo);
            _CFPropertyListMissingSemicolon(line);
            failedParse = true;
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Missing ';' on line %d"), line));
        }
    }
    
    if (failedParse) {
        if (key) CFRelease(key);
        if (dict) CFRelease(dict);
        return NULL;
    }
    
    // this is a success path, so clear errors (NOTE: this seems weird, but is historical)
    parseInfo_clearError(pInfo);
    
    return dict;
}

static CFTypeRef parsePlistDict(_CFStringsFileParseInfo *pInfo, const uint32_t depth) CF_RETURNS_RETAINED {
    CFDictionaryRef dict = parsePlistDictContent(pInfo, depth);
    if (!dict) { return NULL; }
    Boolean foundChar = advanceToNonSpace(pInfo);
    if (!foundChar || *pInfo->curr != '}') {
        if (dict) { CFRelease(dict); }
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected terminating '}' for dictionary at line %d"), lineNumberStrings(pInfo)));
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
        if (first != 0xff) {    // If the first char is a hex, then try to read a second hex
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
static CFTypeRef parsePlistData(_CFStringsFileParseInfo *pInfo) CF_RETURNS_RETAINED {
    CFMutableDataRef result = CFDataCreateMutable(pInfo->allocator, 0);
    if (result == NULL) {
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unable to allocate data while parsing property list at line %d"), lineNumberStrings(pInfo)));
        return NULL;
    }
    
    // Read hex bytes and append them to result
    while (1) {
        unsigned char bytes[numBytes];
        int numBytesRead = getDataBytes(pInfo, bytes, numBytes);
        if (numBytesRead < 0) {
            if (result) CFRelease(result);
            switch (numBytesRead) {
                case -2:
                    parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Malformed data byte group at line %d; uneven length"), lineNumberStrings(pInfo)));
                    break;
                default:
                    parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Malformed data byte group at line %d; invalid hex"), lineNumberStrings(pInfo)));
                    break;
            }
            return NULL;
        }
        if (numBytesRead == 0) break;
        CFDataAppendBytes(result, bytes, numBytesRead);
    }
    
    // this is a success path, so clear errors (NOTE: this seems weird, but is historical)
    parseInfo_clearError(pInfo);
    
    if (pInfo->curr < pInfo->end && *(pInfo->curr) == '>') {
        pInfo->curr ++; // Move past '>'
        return result;
    } else {
        if (result) CFRelease(result);
        parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Expected terminating '>' for data at line %d"), lineNumberStrings(pInfo)));
        return NULL;
    }
}
#undef numBytes

// Returned object is retained; caller must free.
static CFTypeRef parsePlistObject(_CFStringsFileParseInfo *pInfo, bool requireObject, const uint32_t depth) CF_RETURNS_RETAINED {
    if (depthIsInvalid(pInfo, depth)) {
        return NULL;
    }
    
    UniChar ch;
    Boolean foundChar = advanceToNonSpace(pInfo);
    if (!foundChar) {
        if (requireObject) {
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected EOF while parsing plist")));
        }
        return NULL;
    }
    ch = *(pInfo->curr);
    pInfo->curr ++;
    if (ch == '{') {
        return parsePlistDict(pInfo, depth);
    } else if (ch == '(') {
        return parsePlistArray(pInfo, depth);
    } else if (ch == '<') {
        return parsePlistData(pInfo);
    } else if (ch == '\'' || ch == '\"') {
        return parseQuotedPlistString(pInfo, ch);
    } else if (isValidUnquotedStringCharacter(ch)) {
        pInfo->curr --;
        return parseUnquotedPlistString(pInfo);
    } else {
        pInfo->curr --;  // Must back off the character we just read
        if (requireObject) {
            parseInfo_setError(pInfo, __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Unexpected character '0x%x' at line %d"), ch, lineNumberStrings(pInfo)));
        }
        return NULL;
    }
}

// CFAllocatorRef allocator, CFDataRef xmlData, CFStringRef originalString, CFStringEncoding guessedEncoding, CFOptionFlags option, CFErrorRef *outError, Boolean allowNewTypes, CFPropertyListFormat *format, CFSetRef keyPaths

CF_PRIVATE CFTypeRef __CFCreateOldStylePropertyListOrStringsFile(CFAllocatorRef allocator, CFDataRef xmlData, CFStringRef originalString, CFStringEncoding guessedEncoding, CFOptionFlags option, CFErrorRef *outError,CFPropertyListFormat *format) {
    
    CFStringRef plistString = NULL;
    
    // Convert the string to UTF16 for parsing old-style
    if (originalString) {
        plistString = CFRetain(originalString);
    } else {
        plistString = CFStringCreateWithBytesNoCopy(kCFAllocatorSystemDefault, CFDataGetBytePtr(xmlData), CFDataGetLength(xmlData), guessedEncoding, false, kCFAllocatorNull);
        if (!plistString) {
            // Couldn't convert
            if (outError) *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Conversion of string failed."));
            return NULL;
        }
    }
    
    
    Boolean createdBuffer = false;
    const CFIndex length = CFStringGetLength(plistString);
    if (!length) {
        if (outError) *outError = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Conversion of string failed. The string is empty."));
        CFRelease(plistString);
        return NULL;
    }
    
    UniChar *buf = (UniChar *)CFStringGetCharactersPtr(plistString);
    if (!buf) {
        buf = (UniChar *)CFAllocatorAllocate(allocator, length * sizeof(UniChar), 0);
        if (!buf) {
            CRSetCrashLogMessage("CFPropertyList ran out of memory while attempting to allocate temporary storage.");
            CFRelease(plistString);
            return NULL;
        }
        CFStringGetCharacters(plistString, CFRangeMake(0, length), buf);
        createdBuffer = true;
        CFRelease(plistString);
        plistString = NULL;
    }
    
    _CFStringsFileParseInfo stringsPInfo;
    stringsPInfo.begin = buf;
    
    CFIndex unused;
    if (os_add_overflow((CFIndex)buf, (CFIndex)length, &unused)) {
        CRSetCrashLogMessage("Unable to address entirety of CFPropertyList");
        if (createdBuffer) {
            CFAllocatorDeallocate(allocator, buf);
        } else {
            CFRelease(plistString);
        }
        return NULL;
    }
    stringsPInfo.end = buf+length;
    stringsPInfo.curr = buf;
    stringsPInfo.allocator = allocator;
    stringsPInfo.mutabilityOption = option;
    stringsPInfo.stringSet = CFSetCreateMutable(allocator, 0, &kCFTypeSetCallBacks);
    if (stringsPInfo.stringSet == NULL) {
        CRSetCrashLogMessage("CFPropertyList ran out of memory while attempting to allocate temporary storage.");
        if (createdBuffer) {
            CFAllocatorDeallocate(allocator, buf);
        } else {
            CFRelease(plistString);
        }
        return NULL;
    }
    stringsPInfo.error = NULL;
    
    const UniChar *begin = stringsPInfo.curr;
    CFTypeRef result = NULL;
    Boolean foundChar = advanceToNonSpace(&stringsPInfo);
    if (!foundChar) {
        // A file consisting only of whitespace (or empty) is now defined to be an empty dictionary
        result = CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    } else {
        result = parsePlistObject(&stringsPInfo, true, /*depth*/ 0);
        if (result) {
            foundChar = advanceToNonSpace(&stringsPInfo);
            if (foundChar) {
                if (CFGetTypeID(result) != CFStringGetTypeID()) {
                    if (result) {
                        CFRelease(result);
                    }
                    result = NULL;
                    if (stringsPInfo.error) {
                        CFRelease(stringsPInfo.error);
                    }
                    stringsPInfo.error = __CFPropertyListCreateError(kCFPropertyListReadCorruptError, CFSTR("Junk after plist at line %d"), lineNumberStrings(&stringsPInfo));
                } else {
                    // Reset info and keep parsing
                    if (result) {
                        CFRelease(result);
                    }
                    if (stringsPInfo.error) {
                        CFRelease(stringsPInfo.error);
                        stringsPInfo.error = NULL;
                    }
                    // Check for a strings file (looks like a dictionary without the opening/closing curly braces)
                    stringsPInfo.curr = begin;
                    result = parsePlistDictContent(&stringsPInfo, 0);
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
            stringsPInfo.error = NULL;
        }
    }
    
    if (result && format) {
        *format = kCFPropertyListOpenStepFormat;
    }
    
    if (createdBuffer) {
        CFAllocatorDeallocate(allocator, buf);
    } else {
        CFRelease(plistString);
    }
    if (stringsPInfo.stringSet) { CFRelease(stringsPInfo.stringSet); }
    return result;
}

#undef isValidUnquotedStringCharacter
