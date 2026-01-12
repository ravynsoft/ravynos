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

/*	CFXMLInputStream.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: David Smith
*/

#include "CFXMLInputStream.h"
#include <CoreFoundation/CFCharacterSet.h>
#include <string.h>
#include "CFStringEncodingConverter.h"
#include "CFUniChar.h"

/* Utility functions used in parsing */
static Boolean determineEncoding(_CFXMLInputStream *stream) {
    const uint8_t *bytes = (uint8_t *)CFDataGetBytePtr(stream->data);
    UInt32 length = CFDataGetLength(stream->data);
    const uint8_t *idx = 0L, *end = 0L;
    const uint8_t *base = 0L;
    char quote = ' ';
    Boolean useUTF8 = false;
    
    // Check for the byte order mark first
    if (length > 2) {
        // This clause checks for the unicode byte order mark, or a Unicode sequence lacking the BOM; technically an error, but this check is recommended by the XML spec
        if ((*bytes == 0xFF && *(bytes+1) == 0xFE) ||*(bytes+1) == 0x00) {
#if __BIG_ENDIAN__
            stream->flags |= ENCODING_IS_UNICODE_SWAPPED;
#else
            stream->flags |= ENCODING_IS_UNICODE_NATURAL;
#endif
            if (*bytes == 0xFF) {
                stream->currentByte = bytes + 2;
            }
            stream->encoding = kCFStringEncodingUnicode;
            return true;
        } else if ((*bytes == 0xFE && *(bytes+1) == 0xFF) || *bytes == 0x00) {
#if __BIG_ENDIAN__
            stream->flags |= ENCODING_IS_UNICODE_NATURAL;
#else
            stream->flags |= ENCODING_IS_UNICODE_SWAPPED;
#endif
            if (*bytes == 0xFE) {
                stream->currentByte = bytes + 2;
            }
            stream->encoding = kCFStringEncodingUnicode;
            return true;
        } else if(*bytes == 0xEF && *(bytes+1) == 0xBB && *(bytes+2) == 0xBF) {
            if(*bytes == 0xEF) {
                stream->currentByte = bytes + 3;
            }
            stream->encoding = kCFStringEncodingUTF8;
            stream->flags |= ENCODING_MATCHES_ASCII;
            return true;
        }
    }
    // Scan for the <?xml.... ?> opening
    if (length < 5 || strncmp((char const *) bytes, "<?xml", 5) != 0) {
        useUTF8 = true;
    }
    if (!useUTF8) {
        idx = bytes + 5;
        end = bytes + length;
        // Found "<?xml"; now we scan for "encoding"
        while (idx < end) {
            uint8_t ch = *idx;
            const uint8_t *scan;
            if ( ch == '?' || ch == '>') {
                useUTF8 = true;
                break;
            }
            idx ++;
            scan = idx;
            if (ch == 'e' && *scan++ == 'n' && *scan++ == 'c' && *scan++ == 'o' && *scan++ == 'd' && *scan++ == 'i' && *scan++ == 'n' && *scan++ == 'g' && *scan++ == '=') {
                idx = scan;
                break;
            }
        }
        if (!useUTF8 && idx >= end) {
            useUTF8 = true;
        }
    }
    if (!useUTF8) {
        // Found "encoding="; see if we've got an honest-to-goodness encoding name
        quote = *idx;
        if (quote != '\'' && quote != '\"') {
            useUTF8 = true;
        }
    }
    if (!useUTF8) {
        base = idx + 1; // Move past the quote character
        idx ++;
        while (idx < end && *idx != quote) idx ++;
        if (idx >= end) {
            useUTF8 = true;
        }
    }
    if (!useUTF8) {
        UInt32 len = idx - base;
        if (len == 5 && (*base == 'u' || *base == 'U') && (base[1] == 't' || base[1] == 'T') && (base[2] == 'f' || base[2] == 'F') && (base[3] == '-') && (base[4] == '8')) {
            useUTF8 = true;
        } else {
            CFStringRef encodingName = CFStringCreateWithBytes(stream->allocator, base, len, kCFStringEncodingISOLatin1, false);
            stream->encoding = CFStringConvertIANACharSetNameToEncoding(encodingName);
            CFRelease(encodingName);
        }
    }
    if (useUTF8) {
        stream->encoding = kCFStringEncodingUTF8;
        stream->flags |= ENCODING_MATCHES_ASCII;
        return true;
    } else if (stream->encoding == kCFStringEncodingInvalidId) {
        return false;
    } else if (__CFStringEncodingIsSupersetOfASCII(stream->encoding)) {
        stream->flags |= ENCODING_MATCHES_ASCII;
    }
    return true;
}

CF_INLINE void _fillStringWithCharacters(CFMutableStringRef string, UniChar *characters, CFIndex numChars) {
    CFStringDelete(string, CFRangeMake(0, CFStringGetLength(string)));
    if (numChars) {
        CFStringAppendCharacters(string, characters, numChars);
    }
}

CF_PRIVATE Boolean _openInputStream(_CFXMLInputStream *stream) {
    if (NULL == stream->data) {
        return false;
    } else {
        stream->currentByte = CFDataGetBytePtr(stream->data);
        if (determineEncoding(stream)) {
            stream->flags |= STREAM_OPEN;
            return true;
        } else {
            return false;
        }
    }
}

CF_PRIVATE void _initializeInputStream(_CFXMLInputStream *stream, CFAllocatorRef alloc, CFURLRef dataSource, CFDataRef xmlData) {
    stream->data = xmlData ? (CFDataRef)CFRetain(xmlData) : NULL;
    stream->url = dataSource ? (CFURLRef)CFRetain(dataSource) : NULL;
    stream->encoding = kCFStringEncodingInvalidId;
    stream->currentByte = NULL;
    
    stream->allocator = (CFAllocatorRef)CFRetain(alloc);
    stream->charBuffer = NULL;
    stream->currentChar = NULL;
    stream->mark = NULL;
    stream->parserMark = NULL;
    stream->bufferLength = 0;
    stream->bufferCapacity = 0;
    
    stream->charIndex = 1;
    stream->lineNum = 1;

    stream->flags = 0;
    stream->nameSet = NULL;
    stream->tempString = NULL;
}


CF_PRIVATE void _freeInputStream(_CFXMLInputStream *stream) {
    if (stream->data) CFRelease(stream->data);
    if (stream->url) CFRelease(stream->url);
    if (stream->charBuffer) CFAllocatorDeallocate(stream->allocator, stream->charBuffer);
    if (stream->nameSet) CFRelease(stream->nameSet);
    if (stream->tempString) CFRelease(stream->tempString);
    CFRelease(stream->allocator);
}

CF_PRIVATE CFStringEncoding _inputStreamGetEncoding(_CFXMLInputStream *stream) {
    return stream->encoding;
}

CF_PRIVATE CFIndex _inputStreamCurrentLocation(_CFXMLInputStream *stream) {
    return stream->charIndex;
}

CF_PRIVATE CFIndex _inputStreamCurrentLine(_CFXMLInputStream *stream) {
    return stream->lineNum;
}

CF_PRIVATE Boolean _inputStreamAtEOF(_CFXMLInputStream *stream) {
    if (!(stream->flags & STREAM_OPEN)) return false;
    if (stream->currentChar) return false;
    if (stream->currentByte - CFDataGetBytePtr(stream->data) < CFDataGetLength(stream->data)) return false;
    return true;
}

CF_PRIVATE Boolean _inputStreamComposingErrorOccurred(_CFXMLInputStream *stream) {
    return stream->flags & ENCODING_COMPOSITION_ERROR;
}

#define INITIAL_BUFFER_SIZE 64
static void growCharacterBuffer(_CFXMLInputStream *stream) {
    if (!stream->charBuffer) {
        stream->charBuffer = (UniChar *)CFAllocatorAllocate(stream->allocator, INITIAL_BUFFER_SIZE*sizeof(UniChar), 0);
        stream->bufferCapacity = INITIAL_BUFFER_SIZE;
    } else {
        CFIndex currCharDelta = stream->currentChar ? stream->currentChar - stream->charBuffer : -1;
        CFIndex markDelta = stream->mark ? stream->mark - stream->charBuffer: -1;
        CFIndex parserMarkDelta = stream->parserMark ? stream->parserMark - stream->charBuffer: -1;
        UniChar *newBuffer = (UniChar *)CFAllocatorReallocate(stream->allocator, stream->charBuffer, stream->bufferCapacity * 2 * sizeof(UniChar), 0);
        stream->bufferCapacity *= 2;
        if (newBuffer != stream->charBuffer) {
            stream->charBuffer = newBuffer;
            if (currCharDelta != -1) {
                stream->currentChar = newBuffer + currCharDelta;
            }
            if (markDelta != -1) {
                stream->mark = newBuffer + markDelta;
            }
            if (parserMarkDelta != -1) {
                stream->parserMark = newBuffer + parserMarkDelta;
            }
        }
    }
}

static CFIndex loadCharacters(UniChar *base, CFIndex maxLength, _CFXMLInputStream *stream) {
    const uint8_t *dataEnd = CFDataGetBytePtr(stream->data) + CFDataGetLength(stream->data);
    if (stream->flags & (ENCODING_IS_UNICODE_NATURAL|ENCODING_IS_UNICODE_SWAPPED) ) {
        CFIndex charsToTranslate = (dataEnd - stream->currentByte) / sizeof(UniChar);
        if (charsToTranslate > maxLength) {
            charsToTranslate = maxLength;
        }
        if (stream->flags & ENCODING_IS_UNICODE_NATURAL) {
            memmove(base, stream->currentByte, charsToTranslate * sizeof(UniChar));
            stream->currentByte += (charsToTranslate * sizeof(UniChar));
        } else {
            CFIndex i;
            uint8_t *baseBytePtr = (uint8_t *)base;
            for (i = 0; i < charsToTranslate; i ++) {
                *(baseBytePtr + 1) = *stream->currentByte;
                *baseBytePtr = *(stream->currentByte + 1);
                baseBytePtr += 2;
                stream->currentByte += 2;
            }
        }
        return charsToTranslate;
    } else {
        CFIndex lengthConsumed = 0;
        CFIndex usedByteLength, usedCharLength;
        UInt32 conversionResult;
        if (stream->flags & ENCODING_MATCHES_ASCII) {
            while (stream->currentByte < dataEnd && lengthConsumed < maxLength) {
                if (*stream->currentByte > 0x7f) break;
                *base = *stream->currentByte;
                base ++;
                stream->currentByte ++;
                lengthConsumed ++;
            }
            if (stream->currentByte == dataEnd || lengthConsumed == maxLength) {
                return lengthConsumed;
            }
        }
        conversionResult = CFStringEncodingBytesToUnicode(stream->encoding, 0, stream->currentByte, dataEnd - stream->currentByte, &usedByteLength, base, maxLength-lengthConsumed, &usedCharLength);
        if(kCFStringEncodingConversionSuccess != conversionResult) {
            switch(conversionResult) {
                case kCFStringEncodingConverterUnavailable:
                case kCFStringEncodingInvalidInputStream:
                    stream->flags |= ENCODING_COMPOSITION_ERROR;
                    break;
                case kCFStringEncodingInsufficientOutputBufferLength:
                default:
                    break;
            }
        }
        if (usedByteLength > 0) {
            stream->currentByte += usedByteLength;
            lengthConsumed += usedCharLength;
        }
        return lengthConsumed;
    }
}

// returns number of characters filled
CF_INLINE CFIndex fillToCapacity(_CFXMLInputStream *stream) {
    CFIndex numFilled;
    if (stream->bufferLength >= stream->bufferCapacity) return 0;
    // Try and fill in the remaining characters
    numFilled = loadCharacters(stream->charBuffer+stream->bufferLength, stream->bufferCapacity - stream->bufferLength, stream);
    if (numFilled != 0) {
        stream->currentChar = stream->charBuffer + stream->bufferLength;
        stream->bufferLength += numFilled;
    }
    return numFilled;
}

// we are expected to move mark & parserMark relative to any moved characters, set currentChar to the first new character fetched, update bufferLength, and advance currentByte as appropriate.  Does not check for EOF; it is the caller's responsibility to verify this.
static void fillCharacterBuffer(_CFXMLInputStream *stream) {
    if (!stream->charBuffer) {
        growCharacterBuffer(stream);
    }
    if (!stream->mark && !stream->parserMark) {
        // This is the easy case; we can freely overwrite the buffer; if either mark or parserMark is set, we must not remove any characters from those marks and the end of the buffer
        CFIndex fillLength = stream->bufferCapacity-5; // We leave a few characters at the end, b/c we don't want to reallocate (doubling the amount of memory used) just b/c we're matching a small string near the end of the filled buffer
        stream->bufferLength = loadCharacters(stream->charBuffer, fillLength, stream);
        CFAssert(stream->bufferLength != 0, __kCFLogAssertion, "CF internal error: XML parser input stream corruption");
        stream->currentChar = stream->charBuffer;
    } else {
        // We do everything we can not to allocate; first we fill any remaining characters.  If that doesn't work, we try shifting the characters starting at the earlier of mark or parserMark to the beginning of buffer, then filling the newly-freed characters.
        Boolean done;

        // First try just filling the remaining capacity
        done = (fillToCapacity(stream) != 0);
        if (!done) {
            const UniChar *leftMostMark;
            if (stream->mark && !stream->parserMark) {
                leftMostMark = stream->mark;
            } else if (stream->parserMark && !stream->mark) {
                leftMostMark = stream->parserMark;
            } else if (stream->parserMark < stream->mark) {
                leftMostMark = stream->parserMark;
            } else {
                leftMostMark = stream->mark;
            }
            if (leftMostMark > stream->charBuffer) {
                CFIndex delta = leftMostMark - stream->charBuffer;
                memmove(stream->charBuffer, leftMostMark, (stream->bufferLength - delta) * sizeof(UniChar));
                stream->bufferLength -= delta;
                if (stream->mark) {
                    stream->mark -= delta;
                }
                if (stream->parserMark) {
                    stream->parserMark -= delta;
                }
                // Now try to fill the newly-opened space
                done = (fillToCapacity(stream) != 0);
                delta = loadCharacters(stream->charBuffer + stream->bufferLength, stream->bufferCapacity - stream->bufferLength, stream);
            }
        }
        if (!done) {
            // No help for it; now we must allocate
            growCharacterBuffer(stream);
            fillToCapacity(stream); // If this doesn't work, we give up.
        }
    }
}

/* The guts of getCharacter() have been separated in order to allow getCharacter() to be small and more easily inline-able. Note that a check late in the 10.3 development cycle indicates that getCharacter() should call getCharacterGuts() less than 2% of the time.  (In 29000 calls, less than 400 called this. Note that a majority of calls have advanceStream set, so that was left in the inline version.  Also note that some calls to getCharacter() were changed to go through the functions _inputStreamGetCharacter() or _inputStreamPeekCharacter(), as the expansion in using the inline version didn't seem worthwhile. See 3275503 for some data supporting this.
*/
static Boolean getCharacterGuts(_CFXMLInputStream *stream, UniChar *ch, Boolean advanceStream) {
    if (stream->currentByte - CFDataGetBytePtr(stream->data) >= CFDataGetLength(stream->data)) {
        return false; // EOF
    } else if (!((stream->mark || stream->parserMark) && advanceStream) &&
               (((stream->flags & ENCODING_MATCHES_ASCII) && *(stream->currentByte) < 0x7F) ||
                (stream->flags & (ENCODING_IS_UNICODE_NATURAL | ENCODING_IS_UNICODE_SWAPPED)))) {
        // We can only perform optimizations if neither mark is set (if the mark is set, we must fill the character buffer so we can retrieve the characters later), and the encoding is Unicode, or the encoding matches ASCII and we're looking at a low-byte character.
        if (stream->flags & ENCODING_MATCHES_ASCII) {
            *ch = (UniChar)*(stream->currentByte);
            if (advanceStream) {
                stream->currentByte ++;
            }
        } else if (stream->flags & ENCODING_IS_UNICODE_NATURAL) {
            *ch = *(UniChar *)(stream->currentByte);
            if (advanceStream) {
                stream->currentByte += 2;
            }
        } else {
            // Unicode with swapped bytes
            *ch = CFSwapInt16(*(UniChar *)(stream->currentByte));
            if (advanceStream) {
                stream->currentByte += 2;
            }
        }
    } else {
        fillCharacterBuffer(stream); // this takes into account markIsSet to make sure and do the right thing
        if (!stream->charBuffer || !stream->currentChar) {
            return false;
        } else {
            *ch = *(stream->currentChar);
            if (advanceStream) {
                stream->currentChar ++;
                if (stream->currentChar == stream->charBuffer + stream->bufferLength) {
                    stream->currentChar = NULL;
                }
            }
        }
    }
    return true;
}

/* See comments above getCharacterGuts()
*/
CF_INLINE Boolean getCharacter(_CFXMLInputStream *stream, UniChar *ch, Boolean advanceStream) {
    if (!(stream->flags & STREAM_OPEN)) {
        return false;
    } else if (stream->currentChar) {
        *ch = *stream->currentChar;
        if (advanceStream) {
            stream->currentChar ++;
            if (stream->currentChar == stream->charBuffer + stream->bufferLength) {
                stream->currentChar = NULL;
            }
        }
    } else {
        if (!getCharacterGuts(stream, ch, advanceStream)) return false;
    }
    if (advanceStream) {
        UniChar nextChar;
        stream->charIndex ++;
        if ((*ch == '\n') || ((*ch == '\r') && (!_inputStreamPeekCharacter(stream, &nextChar) || nextChar != '\n'))) stream->lineNum ++;
    }
    return true;
}

CF_PRIVATE Boolean _inputStreamPeekCharacter(_CFXMLInputStream *stream, UniChar *ch) {
    return getCharacter(stream, ch, false);
}

CF_PRIVATE Boolean _inputStreamGetCharacter(_CFXMLInputStream *stream, UniChar *ch) {
    return getCharacter(stream, ch, true);
}

CF_PRIVATE Boolean _inputStreamReturnCharacter(_CFXMLInputStream *stream, UniChar ch) {
    Boolean decrementLineNum = false;
    if (ch == '\n') {
        decrementLineNum = true;
    } else if (ch == '\r') {
        UniChar nextChar;
        if (!_inputStreamPeekCharacter(stream, &nextChar) || nextChar != '\n') {
            decrementLineNum = true;
        }
    }

    if (!(stream->flags & STREAM_OPEN)) {
        return false;
    } else if (stream->currentChar) {
        if (stream->currentChar != stream->charBuffer) {
            stream->currentChar --;
        } else {
            // Yuck; we're unlucky and are returning a character _before_ the first character in charBuffer
            if (stream->bufferLength >= stream->bufferCapacity) {
                growCharacterBuffer(stream);
            }
            memmove(stream->charBuffer + 1, stream->charBuffer, stream->bufferLength * sizeof(UniChar));
            *stream->charBuffer = ch;
            stream->bufferLength ++;
            if (stream->mark) {
                stream->mark ++;
            }
            if (stream->parserMark) {
                stream->parserMark ++;
            }
        }
    } else if ((stream->mark || stream->parserMark) && stream->bufferLength) {
        // We've been collecting characters in charBuffer; the only reason stream->currentChar is NULL is that we've processed the last character thusfar translated from data.  That last character is the one being returned.
        stream->currentChar = stream->charBuffer + stream->bufferLength - 1;
    } else if (stream->charBuffer) {
        // We have processed all the meaningful characters from charBuffer and have no reason to preserve them.  We use charBuffer to hold this one character that has been returned to us.
        *stream->charBuffer = ch;
        stream->currentChar = stream->charBuffer;
        stream->bufferLength = 1;
        if (stream->mark) {
            stream->mark ++;
        }
        if (stream->parserMark) {
            stream->parserMark ++;
        } 
    } else if (stream->currentByte > CFDataGetBytePtr(stream->data)) {
        // We have no character buffer available, so that means one of two things - either we've never needed a character buffer because all the characters could come directly out of the byte stream, or we've not yet processed the first character.  The former means we can just back up the byte pointer; the latter means Bad Things have happened.
        if (stream->flags & ENCODING_MATCHES_ASCII) {
            stream->currentByte --;
        } else {  // Must be Unicode
            stream->currentByte -= 2;
        }
    } else {
        return false;
    }
    stream->charIndex --;
    if (decrementLineNum) {
        stream->lineNum --;
    }
    return true;
}

// Returns the pointer to hold as the mark
static UniChar *dropMark(_CFXMLInputStream *stream) {
    if (stream->currentChar) {
        return stream->currentChar;
    } else if (stream->mark || stream->parserMark) {
        return stream->charBuffer + stream->bufferLength;
    } else {
        if (!stream->charBuffer) {
            growCharacterBuffer(stream);
        }
        stream->bufferLength = 0; // This will be sufficient to force a fetch into the buffer when the next character is requested
        return stream->charBuffer;
    }

}

CF_PRIVATE void _inputStreamSetMark(_CFXMLInputStream *stream) {
    CFAssert(stream->mark == NULL, __kCFLogAssertion, "CF internal error: parser input stream malformed");
    stream->mark = dropMark(stream);
}

CF_PRIVATE void _inputStreamClearMark(_CFXMLInputStream *stream) {
    CFAssert(stream->mark != NULL, __kCFLogAssertion, "CF internal error: parser input stream malformed");
    stream->mark = NULL;
}

CF_PRIVATE void _inputStreamGetCharactersFromMark(_CFXMLInputStream *stream, CFMutableStringRef string) {
    UniChar *end = stream->currentChar ? stream->currentChar : stream->charBuffer + stream->bufferLength;
    CFIndex numChars = end - stream->mark;
    CFAssert(stream->mark, __kCFLogAssertion, "CF internal error: malformed XML input stream");
    _fillStringWithCharacters(string, stream->mark, numChars);
}

static void restoreToMark(_CFXMLInputStream *stream, UniChar *mark) {
    UniChar *end = stream->currentChar ? stream->currentChar : stream->charBuffer + stream->bufferLength;
    if (end > mark) {
        CFIndex numChars = end - mark;
        stream->charIndex -= numChars;
        stream->currentChar = mark;

        // This is annoying; to keep the line count accurate, if the first character we are returning is a CR, we must decrement the line count iff the next character is NOT a LF
        if (*(end - 1) == '\r') {
            UniChar nextChar;
            if (_inputStreamPeekCharacter(stream, &nextChar) && nextChar == '\n') {
                end --;
            }
        }
        while (end != mark) {
            end --;
            if (*end == '\r') {
                stream->lineNum --;
            } else if (*end == '\n') {
                stream->lineNum --;
                if (end != mark && *(end - 1) == '\r') {
                    end --;
                }
            }
        }
    }
}

CF_PRIVATE void _inputStreamBackUpToMark(_CFXMLInputStream *stream) {
    CFAssert(stream->mark != NULL || stream->charBuffer == NULL, __kCFLogAssertion, "CF internal error: malformed XML input stream");
    restoreToMark(stream, stream->mark);
}

CF_INLINE Boolean isWhitespaceChar(UniChar ch) {
    return (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t');
}

CF_PRIVATE CFIndex _inputStreamSkipWhitespace(_CFXMLInputStream *stream, CFMutableStringRef str) {
    UniChar ch;
    CFIndex len = 0;
    if (str) {
        stream->parserMark = dropMark(stream);
    }
    while (getCharacter(stream, &ch, true) && isWhitespaceChar(ch)) {
        len ++;
    }
    if (!isWhitespaceChar(ch)) {
        _inputStreamReturnCharacter(stream, ch);
    }
    if (str) {
        _fillStringWithCharacters(str, stream->parserMark, len);
        stream->parserMark = NULL;
    }
    return len;
}

// false return means EOF was encountered without finding scanChars
CF_PRIVATE Boolean _inputStreamScanToCharacters(_CFXMLInputStream *stream, const UniChar *scanChars, CFIndex numChars, CFMutableStringRef str) {
    Boolean done = false;
    CFIndex firstRepeatIndex = -1;
    CFIndex len = 0;
    stream->parserMark = dropMark(stream);
    do {
        UniChar ch;
        while (_inputStreamGetCharacter(stream, &ch) && ch != scanChars[0]) {
            len ++;
        }
        if (ch != scanChars[0]) {
            restoreToMark(stream, stream->parserMark);
            stream->parserMark = NULL;
            return false;
        } else {
            CFIndex i;
            for (i = 1; i < numChars; i ++) {
                if (!_inputStreamGetCharacter(stream, &ch)) break;
                if (ch != scanChars[i]) break;
            }
            if (i == numChars) {
                done = true;
            } else {
                if (firstRepeatIndex == -1) {
                    CFIndex j;
                    for (j = 1; j < numChars; j ++) {
                        if (scanChars[0] == scanChars[j]) {
                            break;
                        }
                    }
                    firstRepeatIndex = j;
                }
                _inputStreamReturnCharacter(stream, ch);
                while (i > firstRepeatIndex) {
                    i --;
                    _inputStreamReturnCharacter(stream, scanChars[i]);
                }
                len += i;
            }
        }
    } while (!done);
    if (str) {
        _fillStringWithCharacters(str, stream->parserMark, len);
    }
    stream->parserMark = NULL;
    return true;
}

CF_PRIVATE Boolean _inputStreamMatchString(_CFXMLInputStream *stream, const UniChar *stringToMatch, CFIndex length) {
    const UniChar *end = stringToMatch+length;
    const UniChar *sPtr=stringToMatch;
    stream->parserMark = dropMark(stream);
    while (sPtr < end) {
        UniChar ch;
        if (!_inputStreamGetCharacter(stream, &ch)) break;
        if (ch != *sPtr) break;
        sPtr ++;
    }
    if (sPtr != end) {
        restoreToMark(stream, stream->parserMark);
        stream->parserMark = NULL;
        return false;
    } else {
        stream->parserMark = NULL;
        return true;
    }
}

CF_PRIVATE Boolean _inputStreamScanQuotedString(_CFXMLInputStream *stream, CFMutableStringRef str) {
    UniChar ch;
    if (!_inputStreamPeekCharacter(stream, &ch)) return false;
    if (ch != '\'' && ch != '\"')  return false;

    _inputStreamGetCharacter(stream, &ch);
    if (!_inputStreamScanToCharacters(stream, &ch, 1, str)) {
        return false;
    }
    return true;
}

/*
 [4]  NameChar ::= Letter | Digit | '.' | '-' | '_' | ':' | CombiningChar | Extender
 [5]  Name ::= (Letter | '_' | ':') (NameChar)*
 [7]  Nmtoken ::= (NameChar)+
 [84] Letter ::= BaseChar | Ideographic

 We don't do this quite right; we rely on the Unicode charsets to do this analysis.  While
 the productions in the XML spec are based on the Unicode character sets, the definitions
 differ slightly to avoid those areas where the Unicode standard is still being resolved.
 At any rate, I'd lay money that using the Unicode charsets, we will be more correct than
 the vast majority of parsers out there.

 Letter == kCFUniCharLetterCharacterSet
 Digit == kCFUniCharDecimalDigitCharacterSet
 CombiningChar == kCFUniCharNonBaseCharacterSet
 Extender - complex, and not represented by a uniform character set.
 */
CF_PRIVATE Boolean _inputStreamScanXMLName(_CFXMLInputStream *stream, Boolean isNMToken, CFStringRef *str) {
    UniChar ch;
    Boolean success = true;
    stream->parserMark = dropMark(stream);
    if (!isNMToken) {
        // Only difference between an NMToken and a Name is Names have a stricter condition on the first character
        if (!getCharacter(stream, &ch, false)) {
            success = false;
        } else if (!CFUniCharIsMemberOf(ch, kCFUniCharLetterCharacterSet) && ch != '_' && ch != ':') {
            success = false;
        } else {
            getCharacter(stream, &ch, true);
        }
    }
    if (success) {
        while (getCharacter(stream, &ch, true)) {
            if (!CFUniCharIsMemberOf(ch, kCFUniCharLetterCharacterSet) && !CFUniCharIsMemberOf(ch, kCFUniCharDecimalDigitCharacterSet)  && ch != '.' && ch != '-' && ch != '_' && ch != ':' && !CFUniCharIsMemberOf(ch, kCFUniCharNonBaseCharacterSet)) {
                _inputStreamReturnCharacter(stream, ch);
                break;
            }
        }
        if (NULL == stream->currentChar || stream->currentChar == stream->parserMark) {
            success = false; // Must have processed at least one character
        }
    }
    if (success) {
        if (str) {
            if (!stream->nameSet) {
                stream->nameSet = CFSetCreateMutable(stream->allocator, 0, &kCFTypeSetCallBacks);
                stream->tempString = CFStringCreateMutableWithExternalCharactersNoCopy(stream->allocator, NULL, 0, 0, kCFAllocatorNull);
            }
            CFStringSetExternalCharactersNoCopy(stream->tempString, stream->parserMark, stream->currentChar-stream->parserMark, stream->currentChar-stream->parserMark);
            if (!CFSetGetValueIfPresent(stream->nameSet, stream->tempString, (const void **)str)) {
                *str = (CFStringRef)CFStringCreateCopy(stream->allocator, stream->tempString);
                CFSetAddValue(stream->nameSet, *str);
                CFRelease(*str);
            }
        }
    } else {
        restoreToMark(stream, stream->parserMark);
    }
    stream->parserMark = NULL;
    return success;
}


