/* Copyright (c) 2007 Michael Ash
   Copyright (c) 2007 Jens Ayton (uid decoding)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSPropertyListReader_binary1.h"
#import <Foundation/NSData.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/CFUID.h>
#include <assert.h>
#include <string.h>

@implementation NSPropertyListReader_binary1

static id _readInlineObjectAtOffset(NSPropertyListReader_binary1 *self,NSUInteger *offset);

+propertyListFromData:(NSData *)data {
  NSPropertyListReader_binary1 *reader=[[self alloc] initWithData:data];

  if(reader==nil)
   return nil;

  id result=[reader read];

  [reader release];

  return result;
}

#define MAGIC "bplist"
#define FORMAT "00"
#define TRAILER_SIZE (sizeof( uint8_t ) * 2 + sizeof( uint64_t ) * 3)

-initWithData: (NSData *)data {
   size_t magiclen = strlen( MAGIC FORMAT );

   BOOL good = YES;
   if( good && [data length] < magiclen + TRAILER_SIZE )
    good = NO;
   if( good && strncmp( [data bytes], MAGIC FORMAT, magiclen ) != 0 )
    good = NO;

   if( !good ){
    [self release];
    return nil;
   }

   _data = [data copy];
   _length=[_data length];
   _bytes=[_data bytes];

   return self;
}

-(void)dealloc {
   [_data release];
   [super dealloc];
}

static inline uint64_t _readIntOfSize(NSPropertyListReader_binary1 *self,size_t size,NSUInteger *offsetPtr) {
        uint64_t ret = 0;
        const uint8_t *ptr = self->_bytes + *offsetPtr;
        size_t i;
        for( i = 0; i < size; i++ )
        {
                ret <<= 8;
                ret |= *ptr;
                ptr++;
        }

        *offsetPtr += size;

        return ret;
}


static inline double _readFloatOfSize(NSPropertyListReader_binary1 *self, size_t size, NSUInteger *offsetPtr)
{
    uint64_t val = _readIntOfSize(self, size, offsetPtr);

    if (size == 4) {
        uint32_t val32 = (uint32_t)val;
        void *p = &val32;
        return *((float *)p);
    } else if (size == 8) {
        void *p = &val;
        return *((double *)p);
    }

    [NSException raise: @"Invalid size" format: @"Don't know how to read float of size %u", size];
    return 0.0;
}


- (void)_readHeader
{
        NSUInteger trailerStart = _length - TRAILER_SIZE;

        _trailerOffsetIntSize= _readIntOfSize(self, sizeof( _trailerOffsetIntSize ), &trailerStart);
        _trailerOffsetRefSize= _readIntOfSize(self, sizeof( _trailerOffsetRefSize ), &trailerStart);
        _trailerNumObjects= _readIntOfSize(self, sizeof( _trailerNumObjects ), &trailerStart);
        _trailerTopObject= _readIntOfSize(self, sizeof( _trailerTopObject ), &trailerStart);
        _trailerOffsetTableOffset = _readIntOfSize(self, sizeof( _trailerOffsetTableOffset ), &trailerStart);
}

static uint64_t ReadSizedInt(NSPropertyListReader_binary1 *bplist, uint64_t offset, uint8_t size)
{
        const uint8_t   *ptr = bplist->_bytes;
        NSUInteger        length=bplist->_length;

        assert(ptr != NULL && size >= 1 && size <= 8 && offset + size <= length);

        uint64_t                result = 0;
        const uint8_t        *byte = ptr + offset;

        do
        {
                result = (result << 8) | *byte++;
        } while (--size);

        return result;
}

static BOOL ReadSelfSizedInt(NSPropertyListReader_binary1 *bplist, uint64_t offset, uint64_t *outValue, size_t *outSize)
{
        const uint8_t   *ptr = bplist->_bytes;
        NSUInteger        length=bplist->_length;

        uint32_t                        size;
        int64_t                                value;

        assert(ptr != NULL && offset < length);

        size = 1 << (ptr[offset] & 0x0F);
        if (size > 8)
        {
                // Maximum allowable size in this implementation is 1<<3 = 8 bytes.
                // This also happens to be the biggest NSNumber can handle.
                return NO;
        }

        if (offset + 1+size > length)
        {
                // Out of range.
                return NO;
        }

        value = ReadSizedInt(bplist, offset +1, size);

        if (outValue != NULL)  *outValue = value;
        if (outSize != NULL)  *outSize = size + 1; // +1 for tag byte.
        return YES;
}

static id ExtractUID(NSPropertyListReader_binary1 *bplist, uint64_t offset) {
        /*        UIDs are used by Cocoa's key-value coder.
                When writing other plist formats, they are expanded to dictionaries of
                the form <dict><key>CF$UID</key><integer>value</integer></dict>, so we
                do the same here on reading. This results in plists identical to what
                running plutil -convert xml1 gives us. However, this is not the same
                result as [Core]Foundation's plist parser, which extracts them as un-
                introspectable CF objects. In fact, it even seems to convert the CF$UID
                dictionaries from XML plists on the fly.
        */

        uint64_t                        value;

        if (!ReadSelfSizedInt(bplist, offset, &value, NULL))
        {
                NSLog(@"Bad binary plist: invalid UID object.");
                return nil;
        }

        return [[CFUID alloc] initWithUnsignedLongLong:value];
}


static id _readObjectAtOffset(NSPropertyListReader_binary1 *self,NSUInteger *offset)
{
    const uint8_t *ptr = self->_bytes;
    uint8_t marker = ptr[*offset];

    (*offset)++;

    if (marker == 0x00) {
        return [NSNull null];
    }
    if (marker == 0x08) {
        return (id)kCFBooleanFalse;
    }
    if (marker == 0x09) {
        return (id)kCFBooleanTrue;
    }

    uint8_t topNibble = marker >> 4;
    uint8_t botNibble = marker & 0x0F;

    if (topNibble == 0x1) {
        return [[NSNumber alloc] initWithLongLong: _readIntOfSize(self, 1 << botNibble, offset)];
    }
    if (topNibble == 0x2) {
        size_t size = 1 << botNibble;
        uint64_t val = _readIntOfSize(self, size, offset);

        if (size == 4) {
            uint32_t val32 = (uint32_t)val;
            void *p = &val32;
            return [[NSNumber alloc] initWithFloat: *(float*)p];
        }
        if (size == 8) {
            void *p = &val;
            return [[NSNumber alloc] initWithDouble: *(double*)p];
        }
        return [[NSNumber alloc] initWithDouble:0.0];
    }
    if (topNibble == 0x3) {
        return [[NSDate alloc] initWithTimeIntervalSinceReferenceDate:_readFloatOfSize(self, 8, offset)];
    }
    if (topNibble == 0x4 || topNibble == 0x5 || topNibble == 0x6 || topNibble == 0x8 || topNibble == 0xA || topNibble == 0xD) {
        uint64_t length = 0;
        if (botNibble != 0xF) {
            length = botNibble;
        } else {
            NSNumber *number=_readObjectAtOffset(self, offset);
            length = [number unsignedLongLongValue];
            [number release];
        }

        if (topNibble == 0x4) {
            return [[self->_data subdataWithRange: NSMakeRange(*offset, length)] copy];
        }
        if (topNibble == 0x5) {
            return [[NSString alloc] initWithBytes:self->_bytes + *offset length:length encoding: NSASCIIStringEncoding];
        }
        if (topNibble == 0x6) {
            return [[NSString alloc] initWithBytes:self->_bytes+*offset length:length*2 encoding: NSUTF16BigEndianStringEncoding];
        }
        if (topNibble == 0x8) {
            return ExtractUID(self, (*offset) - 1);
        }

        if (topNibble == 0xA) {
            id result;
            id *objs = NSZoneMalloc(NULL, length * sizeof(*objs));
            uint64_t i;
            for (i = 0; i < length; i++) {
                objs[i] = _readInlineObjectAtOffset(self, offset);
            }

            result = [[NSArray alloc] initWithObjects: objs count: length];
            for (i = 0; i < length; i++) {
                [objs[i] release];
            }
            free(objs);
            return result;
        }

        if (topNibble == 0xD) {
            id result;
            id *keys = NSZoneMalloc(NULL, length * sizeof(*keys));
            id *objs = NSZoneMalloc(NULL, length * sizeof(*objs));
            uint64_t i;
            for (i = 0; i < length; i++) {
                keys[i] = _readInlineObjectAtOffset(self, offset);
            }
            for (i = 0; i < length; i++) {
                objs[i] = _readInlineObjectAtOffset(self, offset);
            }

            result = [[NSDictionary alloc] initWithObjects: objs forKeys: keys count: length];
            for (i = 0; i < length; i++){
                [keys[i] release];
                [objs[i] release];
            }
            free(keys);
            free(objs);
            return result;
        }
    }

    [NSException raise: @"Unknown marker in plist" format: @"Unable to read marker 0x%uX", marker];
    return nil;
}


static id _readInlineObjectAtOffset(NSPropertyListReader_binary1 *self,NSUInteger *offset) {
        // first read the offset table index out of the file
        NSUInteger objOffset = _readIntOfSize(self, self->_trailerOffsetRefSize , offset);

        // then transform the index into an offset in the file which points to
        // that offset table entry
        objOffset = self->_trailerOffsetTableOffset + objOffset * self->_trailerOffsetIntSize;

        // lastly read the offset stored at that entry
        objOffset = _readIntOfSize(self, self->_trailerOffsetIntSize , &objOffset);

        // and read the object stored there
        return _readObjectAtOffset(self, &objOffset);

}

- (id)read {
        id result=nil;

        @try {
                [self _readHeader];

                NSUInteger offset = _trailerTopObject + strlen( MAGIC FORMAT );
                 result= _readObjectAtOffset(self,&offset);
        }
        @catch( id exception ) {
                NSLog( @"Unable to read binary plist: %@", exception );
        }

   return [result autorelease];
}

@end
