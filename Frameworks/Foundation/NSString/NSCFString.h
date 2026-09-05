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

#ifndef __FOUNDATION_NSCFSTRING_H__
#define __FOUNDATION_NSCFSTRING_H__

#include <Foundation/Foundation.h>
#include <Foundation/NSCFTypeID.h>

/* Keep this in sync with CFString.c */
#define CF_STRING_VARIANTS \
    union { \
        struct _inline1 { \
            NSUInteger length; \
        } inline1; \
        struct _notInlineImmutable1 { \
            unsigned char *bytes; \
            NSUInteger length; \
            void *contentsDeallocator; \
        } notInlineImmutable1; \
        struct _notInlineImmutable2 { \
            unsigned char *bytes; \
            void *contentsDeallocator; \
        } notInlineImmutable2; \
        struct _notInlineMutable notInlineMutable; \
    } variants

#if __LITTLE_ENDIAN__
#define CF_INFO_BITS 0
#define CF_RC_BITS 3
#else
#define CF_INFO_BITS 3
#define CF_RC_BITS 0
#endif

#define kCFStringEncodingUTF8  0x08000100

struct _notInlineMutable {
    const char *bytes;
    NSUInteger length;
    NSUInteger capacity;
    unsigned int flags;
    unsigned long descap;
    void *contentsAllocator;
};

enum {
    kNSCFMutable = 0x1,
    kNSCFHasLength = 0x4,
    kNSCFHasNull = 0x8,
    kNSCFUnichar = 0x10,
    kNSCFDefaultFree = 0x20,
    kNSCFDoNotFree = 0x40
};
#define ED_MASK 0x60

NSString *NSCFStringNewWithCharacters(NSZone *zone, const unichar *characters,
        NSUInteger length, BOOL lossy);
NSString *NSCFStringNewWithBytes(NSZone *zone, const char *bytes,
        NSUInteger length);
NSUInteger NSGetCFStringWithMaxLength(const unichar *characters, 
        NSUInteger length, NSUInteger *location, char *cString, 
        NSUInteger maxLength, BOOL lossy);

@interface NSCFString : NSString {
    CF_STRING_VARIANTS;
    unsigned char bytes[];
}

-(NSCFString *)initWithBytes:(const void *)bytes length:(NSUInteger)length
                 encoding:(NSStringEncoding)encoding;
-(NSCFString *)initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length
                encoding:(NSStringEncoding)encoding 
                freeWhenDone:(BOOL)freeWhenDone;
-(NSUInteger)length;
-(NSStringEncoding)fastestEncoding;
-(unichar)characterAtIndex:(NSUInteger)location;
-(void)getCharacters:(unichar *)buffer range:(NSRange)range;
-(void)getCharacters:(unichar *)buffer;
-(NSUInteger)hash;

-(uint8_t *)_cfinfo;
-(BOOL)_encodingCantBeStoredInEightBitCFString;
-(unichar *)_fastCharacterContents;
-(const char *)_fastCStringContents:(BOOL)getContents;
@end

#endif /* __FOUNDATION_NSCFSTRING_H__ */
