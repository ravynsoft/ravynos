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

/*	CFXMLInputStream.h
	Copyright (c) 2000-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFXMLINPUTSTREAM__)
#define __COREFOUNDATION_CFXMLINPUTSTREAM__ 1

#include <CoreFoundation/CFBase.h>
#include "CFInternal.h"
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFXMLNode.h>

struct __CFXMLNode {
    // additionalData currently always points off the bottom of this struct; we could just eliminate it.  Also, we may want to add a flags/version argument, and could use it to mark whether the node was the special one that CFXMLParser mucks with, as well as whether the allocator was "special" (could save us the alloc instance variable, below) -- REW, 3/8/2000
    CFRuntimeBase _cfBase;
    CFIndex version;
    CFXMLNodeTypeCode dataTypeID;
    CFStringRef dataString;
    void *additionalData;
};

struct __CFXMLInputStream {
    CFDataRef data;               // The XML data
    CFURLRef url;                 // the source URL for the data
    CFStringEncoding encoding;    // the data's encoding
    const UInt8 *currentByte;     // pointer into data at the first byte not yet translated to a character

    UniChar *charBuffer;          // the buffer of characters translated from data
    UniChar *currentChar;         // pointer into charBuffer at the current stream location.  MUST be NULL if there are no more characters in charBuffer to consume.
    UniChar *mark;                // The point at which the mark was dropped.  NULL if the mark is currently unset.
    UniChar *parserMark;          // mark available only for the parser's use
    CFIndex bufferLength;         // The number of meaningful characters in charBuffer
    CFIndex bufferCapacity;       // The current maximum capacity of charBuffer in UniChars
 
    CFIndex charIndex, lineNum;   // location in the file
    UInt32 flags;                 // See #defines below for bit flag meanings
    CFMutableSetRef nameSet;             // set of all names we've encountered; used for uniquing
    CFMutableStringRef tempString;

    CFAllocatorRef allocator; // This is unfortunate; this is always the same as the parser's allocator.  We'd like to get rid of it at some point, but that would mean adding an allocator to all the function calls, which means risking that the allocator passed in gets out-of-sync.  Maybe once we have CFStreams, we can encapsulate it all in that.  REW, 5/22/2000
};

// whether the stream has been opened for reading
#define STREAM_OPEN            0x1
// whether the encoding matches ASCII over 0x0-0x7F
#define ENCODING_MATCHES_ASCII 0x2
// whether the encoding is Unicode with the "natural" byte ordering
#define ENCODING_IS_UNICODE_NATURAL    0x4
// whether the encoding is Unicode with the bytes swapped
#define ENCODING_IS_UNICODE_SWAPPED    0x8
// whether the stream has encountered an error in encodings.
#define ENCODING_COMPOSITION_ERROR	0x10

typedef struct __CFXMLInputStream _CFXMLInputStream;

void _initializeInputStream(_CFXMLInputStream *stream, CFAllocatorRef alloc, CFURLRef dataSource, CFDataRef xmlData);
Boolean _openInputStream(_CFXMLInputStream *stream); // None of the subsequent calls will work until the input stream has been opened
void _freeInputStream(_CFXMLInputStream *stream);

CFStringEncoding _inputStreamGetEncoding(_CFXMLInputStream *stream);
CFIndex _inputStreamCurrentLocation(_CFXMLInputStream *stream);
CFIndex _inputStreamCurrentLine(_CFXMLInputStream *stream);
Boolean _inputStreamAtEOF(_CFXMLInputStream *stream);
Boolean _inputStreamComposingErrorOccurred(_CFXMLInputStream *stream);

Boolean _inputStreamPeekCharacter(_CFXMLInputStream *stream, UniChar *ch);
Boolean _inputStreamGetCharacter(_CFXMLInputStream *stream, UniChar *ch);
Boolean _inputStreamReturnCharacter(_CFXMLInputStream *stream, UniChar ch);
void _inputStreamSetMark(_CFXMLInputStream *stream);
void _inputStreamClearMark(_CFXMLInputStream *stream);
void _inputStreamGetCharactersFromMark(_CFXMLInputStream *stream, CFMutableStringRef string);
void _inputStreamBackUpToMark(_CFXMLInputStream *stream);
void _inputStringInitialize(_CFXMLInputStream *stream, UniChar *characters, CFIndex length);
CFIndex _inputStreamSkipWhitespace(_CFXMLInputStream *stream, CFMutableStringRef str);
Boolean _inputStreamScanToCharacters(_CFXMLInputStream *stream, const UniChar *scanChars, CFIndex numChars, CFMutableStringRef str);
Boolean _inputStreamMatchString(_CFXMLInputStream *stream, const UniChar *stringToMatch, CFIndex length);
Boolean _inputStreamScanQuotedString(_CFXMLInputStream *stream, CFMutableStringRef str);
Boolean _inputStreamScanXMLName(_CFXMLInputStream *stream, Boolean isNMToken, CFStringRef *str);

/* Returns the character index within the current line of the current parse location */
/* To add someday -- CF_EXPORT
CFIndex CFXMLParserGetOffsetInCurrentLine(CFXMLParserRef parser); */

#endif /* ! __COREFOUNDATION_CFXMLINPUTSTREAM__ */

