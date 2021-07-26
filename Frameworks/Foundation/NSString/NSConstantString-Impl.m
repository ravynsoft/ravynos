/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSStringUTF8.h>

// clang support:
// Tiny strings are only used on 64-bit platforms.  They store 8 7-bit
// ASCII characters in the high 56 bits, followed by a 4-bit length and a
// 3-bit tag (which is always 4).

// clang with GNU runtime stores NSConstantString as either ASCII or UTF-16
// we use the GNUstep 2.0 runtime (libobjc2) which uses the following class
//  struct
//  {
//    Class isa;
//    uint32_t flags;
//    uint32_t length; // Number of codepoints
//    uint32_t size; // Number of bytes
//    uint32_t hash;
//    const char *data;
//  };


@implementation NSConstantString(Impl)

static BOOL storageIsASCII(NSConstantString *self){
   return ((self->flags & 3) == 0 )?YES:NO;
}

static inline NSUInteger lengthOfBytes(NSConstantString *self){
   return self->size;
}

static inline NSUInteger lengthInUnicode(NSConstantString *self){
   return self->_length;
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

-(void)release {
}

-autorelease {
   return self;
}

-(NSUInteger)retainCount {
   return UINT_MAX;
}

-(void)dealloc {
   return;
   [super dealloc];
}

-(NSUInteger)length {
   return lengthInUnicode(self);
}

-(NSUInteger)lengthOfBytesUsingEncoding:(NSStringEncoding)encoding {
    switch (encoding) {
        case NSUTF8StringEncoding:
	    if(self->flags == 0) // ASCII string
	        return self->_length;
	    return NSConvertUTF16toUTF8(self->_bytes,self->_length,NULL);
        case NSASCIIStringEncoding:
            return lengthOfBytes(self);
        default:
            NSUnimplementedMethod();
            return 0;
    }
}

-(unichar)characterAtIndex:(NSUInteger)location {
   NSUInteger length=lengthInUnicode(self);

   if(location>=length)
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",location,length);

   if(storageIsASCII(self))
    return ((uint8_t *)_bytes)[location];
   else {
    return ((unichar *)_bytes)[location];
   }
}

-(void)getCharacters:(unichar *)buffer {
   NSUInteger i,length=lengthInUnicode(self);

   if(storageIsASCII(self)){
    for(i=0;i<length;i++)
     buffer[i]=((uint8_t *)_bytes)[i];
   }
   else {
    for(i=0;i<length;i++)
     buffer[i]=((unichar *)_bytes)[i];
   }

}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSUInteger length=lengthInUnicode(self);

   if(NSMaxRange(range)>length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",NSStringFromRange(range),length);
   }

   NSInteger i,loc=range.location,len=range.length;

   if(storageIsASCII(self)){
    for(i=0;i<len;i++)
     buffer[i]=((uint8_t *)_bytes)[loc+i];
   }
   else {
    for(i=0;i<len;i++)
     buffer[i]=((unichar *)_bytes)[loc+i];
   }
}

-(NSUInteger)hash {
   NSUInteger length=lengthInUnicode(self);

   if(storageIsASCII(self))
    return NSStringHashASCII(_bytes,MIN(length,NSHashStringLength));
   else {
    return NSStringHashUnicode(_bytes,MIN(length,NSHashStringLength));
   }
}

@end
