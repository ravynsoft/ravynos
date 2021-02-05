/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSString_placeholder.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSString_unicode.h>
#import <Foundation/NSString_unicodePtr.h>
#import <Foundation/NSString_defaultEncoding.h>
#import <Foundation/NSUnicodeCaseMapping.h>
#import <Foundation/NSString_nextstep.h>
#import <Foundation/NSString_isoLatin1.h>
#import <Foundation/NSString_isoLatin2.h>
#import <Foundation/NSString_win1252.h>
#import <Foundation/NSString_macOSRoman.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSStringFileIO.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSStringUTF8.h>
#import <Foundation/NSStringSymbol.h>
#import <Foundation/NSRaiseException.h>

#import "NSStringEncoder.h"

#import <Foundation/NSData.h>
#import <Foundation/NSCoder.h>
#include <string.h>

@implementation NSString_placeholder

- init
{
    NSDeallocateObject(self);
    return (NSString_placeholder *)@"";
}


- initWithCharactersNoCopy:(unichar *)characters length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone
{
    NSDeallocateObject(self);
    return (NSString_placeholder *)NSString_unicodePtrNewNoCopy(NULL, characters, length, freeWhenDone);
}


// Copied from former -initWithData:(NSData *)data encoding:(NSStringEncoding)encoding;
- initWithBytes:(const void *)bytes length:(NSUInteger)length encoding:(NSStringEncoding)encoding
{
    NSDeallocateObject(self);

    if (encoding == defaultEncoding()) {
        return (NSString_placeholder *)NSString_cStringNewWithBytes(NULL, bytes, length);
    }

    switch(encoding) {
        NSUInteger resultLength;
        unichar *characters;

        case NSUnicodeStringEncoding:
            characters = NSUnicodeFromBytes(bytes, length, &resultLength);
            return (NSString_placeholder *)NSString_unicodePtrNewNoCopy(NULL, characters, resultLength, YES);

        case NSNEXTSTEPStringEncoding:
            return (NSString_placeholder *)NSNEXTSTEPStringNewWithBytes(NULL, bytes, length);

// FIX, not nextstep
        case NSASCIIStringEncoding:
        case NSNonLossyASCIIStringEncoding:
            return (NSString_placeholder *)NSNEXTSTEPStringNewWithBytes(NULL, bytes, length);

        case NSISOLatin1StringEncoding:
            return (NSString_placeholder *)NSString_isoLatin1NewWithBytes(NULL, bytes, length);

        case NSSymbolStringEncoding:
            characters = NSSymbolToUnicode(bytes, length, &resultLength, NULL);
            return (NSString_placeholder *)NSString_unicodePtrNewNoCopy(NULL, characters, resultLength, YES);

        case NSUTF8StringEncoding:
            characters = NSUTF8ToUnicode(bytes, length, &resultLength, NULL);
            return (NSString_placeholder *)NSString_unicodePtrNewNoCopy(NULL, characters, resultLength, YES);

        case NSWindowsCP1252StringEncoding:
            return (NSString_placeholder *)NSString_win1252NewWithBytes(NULL, bytes, length);

        case NSMacOSRomanStringEncoding:
            return (NSString_placeholder *)NSString_macOSRomanNewWithBytes(NULL, bytes, length);

        case NSUTF16LittleEndianStringEncoding:
            characters = NSUnicodeFromBytesUTF16LittleEndian(bytes, length, &resultLength);
            return (NSString_placeholder *)NSString_unicodePtrNewNoCopy(NULL, characters, resultLength, YES);

        case NSUTF16BigEndianStringEncoding:
            characters = NSUnicodeFromBytesUTF16BigEndian(bytes, length, &resultLength);
            return (NSString_placeholder *)NSString_unicodePtrNewNoCopy(NULL, characters, resultLength, YES);

        default: {
            // Let's convert the encoding to unicode and use that
            unichar *unicodePtr = NSBytesToUnicode(bytes, length, encoding, &resultLength, NULL);
            if (unicodePtr) {
                return (NSString_placeholder *)NSString_unicodePtrNewNoCopy(NULL, unicodePtr, resultLength, YES);
            }
        }
            break;
    }

    NSRaiseException(NSInvalidArgumentException, nil, _cmd, @"encoding %d not (yet) implemented", encoding);
    return nil;
}


- initWithFormat:(NSString *)format locale:(NSDictionary *)locale arguments:(va_list)arguments
{
    NSDeallocateObject(self);

    return (NSString_placeholder *)NSStringNewWithFormat(format, locale, arguments, NULL);
}


@end
