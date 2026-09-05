/*
 * Copyright (c) 2026 Zoe Knox.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "NSCFString.h"
#include <stdio.h>
#include <sys/param.h>

#import <Foundation/NSException.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSRaiseException.h>

#define TRACE(...) do { printf("%s: ", object_getClassName(self)); printf(__VA_ARGS__); } while(0);

extern unichar *NSUTF8ToUnicode(const char *,NSUInteger,NSUInteger *,NSZone *);
extern char *NSUnicodeToUTF8(const unichar *,NSUInteger,BOOL,NSUInteger *,NSZone *,BOOL);

@implementation NSCFString

NSString *NSCFStringNewWithCharacters(NSZone *zone, const unichar *characters,
        NSUInteger length, BOOL lossy)
{
    NSUInteger reslen;
    char *bytes = NSUnicodeToUTF8(characters, length, YES, &reslen, NULL, YES);
    NSString *string = NSCFStringNewWithBytes(zone, (const char *)bytes, reslen);
    return string;
}

NSString *NSCFStringNewWithBytes(NSZone *zone, const char *bytes,
         NSUInteger length)
{
    NSCFString *self = NSAllocateObject([NSCFString class], 
            length * sizeof(char), zone);
    if (self) {
        self->variants.notInlineImmutable1.length = length;
        self->variants.notInlineImmutable1.bytes = self->bytes;
        for (int i = 0; i < length; i++)
            self->bytes[i] = ((unsigned char *)bytes)[i];
        // cfinfo immutable, not inline, don't free, 8bit, no length byte, no NULL byte
        self->cfinfo[CF_INFO_BITS] = kNSCFDoNotFree;
        self->cfinfo[CF_RC_BITS] = 1;
    }
    return self;
}

NSUInteger NSGetCFStringWithMaxLength(const unichar *characters, 
        NSUInteger length, NSUInteger *location, char *cString, 
        NSUInteger maxLength, BOOL lossy)
{
    NSUInteger i, result = 0;

    if (length + 1 > maxLength) {
        cString[0] = '\0';
        return NSNotFound;
    }

    for (i = 0; i < length && result <= maxLength; i++) {
        const unichar code = characters[i];

        if (code < 256)
            cString[result++] = code;
        else {
            if(lossy)
                cString[result++] = '\0';
            else {
                return NSNotFound;
            }
        }
    }
    cString[result]='\0';

    if (location)
        *location = i;
    return result;
}

-copy {
    return self;
}

-copyWithZone:(NSZone *)zone {
    return self;
}

-retain {
    return self;
}

- (oneway void)release
{
}


-autorelease {
    return self;
}

-(void)dealloc {
    return;
    [super dealloc];
}

- initWithCharactersNoCopy:(unichar *)characters length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone
{
    NSDeallocateObject(self);
    return NSCFStringNewWithCharacters(NULL, characters, length, freeWhenDone);
}

-(NSUInteger)length {
    uint8_t infobits = self->cfinfo[CF_INFO_BITS];

    if (infobits & kNSCFHasLength) {
        return (self->variants.inline1.length & 0xff);
    } else if (infobits & kNSCFHasNull) {
        if (infobits & kNSCFUnichar) {
            uint16_t *ch = self->variants.notInlineImmutable1.bytes;
            NSUInteger len = 0;
            while (*ch++)
                len++;
            return len;
        } else {
            uint8_t *ch = self->variants.notInlineImmutable1.bytes;
            NSUInteger len = 0;
            while (*ch++)
                len++;
            return len;
        }
    }
    return (self->variants.notInlineImmutable1.length & 0xff);
}

-(unichar)characterAtIndex:(NSUInteger)location {
    if (location >= [self length]) {
        NSRaiseException(NSRangeException, self, _cmd, 
            @"index %d beyond length %d", location,[self length]);
    }
    unichar buffer[1];
    [self getCharacters:buffer range:NSMakeRange(location, 1)];
    return buffer[0];
}

-(NSStringEncoding)fastestEncoding {
    return NSUTF8StringEncoding;
}

-(const char *)_fastCStringContents:(BOOL)getContents {
    uint8_t infobits = self->cfinfo[CF_INFO_BITS];
    const char *p;
    NSUInteger reslen;

    if (infobits & kNSCFHasLength)
        p = (const char *)(&self->variants.notInlineImmutable1.bytes);
    else
        p = (const char *)(self->variants.notInlineImmutable1.bytes);

    if (infobits & kNSCFUnichar)
        return NSUnicodeToUTF8(p, [self length], YES, &reslen, NULL, YES);
    else
        return p;
}

-(unichar *)_fastCharacterContents {
    uint8_t infobits = self->cfinfo[CF_INFO_BITS];
    const char *p;
    NSUInteger reslen;

    if (infobits & kNSCFHasLength)
        p = (const char *)(&self->variants.notInlineImmutable1.bytes);
    else
        p = (const char *)(self->variants.notInlineImmutable1.bytes);

    if (infobits & kNSCFUnichar)
        return (unichar *)p;
    else
        return NSUTF8ToUnicode(p, [self length], &reslen, NULL);
}

-(BOOL)_encodingCantBeStoredInEightBitCFString {
    uint8_t infobits = self->cfinfo[CF_INFO_BITS];
    if (infobits & kNSCFUnichar)
        return YES;
    return NO;
}


-(void)getCharacters:(unichar *)buffer {
    return [self getCharacters:buffer range:NSMakeRange(0, [self length])];
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
    NSInteger i = 0, loc = range.location, len = range.length;
    unichar *uch = 0;
    unsigned char *ch = 0;

    if (!buffer)
        return;

    if (NSMaxRange(range) > [self length]) {
        NSRaiseException(NSRangeException, self, _cmd,
            @"range %@ beyond length %d",
            NSStringFromRange(range), [self length]);
    }

    uint8_t infobits = self->cfinfo[CF_INFO_BITS];
    if (infobits & kNSCFHasLength)
        loc++;

    memset(buffer, 0, len * sizeof(unichar));

    switch (infobits & ED_MASK) {
        case 0x00: {
            ch = (unsigned char *)&(self->variants.notInlineImmutable1.bytes);
            break;
        }
        default: {
            ch = (unsigned char *)self->variants.notInlineImmutable1.bytes;
        }
    }

    if (infobits & kNSCFUnichar) { // unicode 16-bit chars
        for (i = 0; i < len; i++)
            buffer[i] = ((unichar *)ch)[i + loc];
    } else { // 8-bit UTF8 or ASCII
        for (i = 0; i < len; i++) 
            buffer[i] = ch[i + loc];
    }
}

-(NSUInteger)hash {
    return CFStringHashNSString(self);
}

@end
