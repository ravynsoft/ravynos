/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>
   Copyright (c) 2021 Zoe Knox <zoe@pixin.net>

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
// We support these using the libobjc2 "small object" interface

@implementation NSConstantString_tiny

static inline NSUInteger extractLength(NSConstantString_tiny *self){
   return ((unsigned long)self & 0x78) >> 3;
}

static inline char characterAt(NSConstantString_tiny *self, NSUInteger location) {
   return (((unsigned long)self & (0xFEL << (56-(location*7)))) >> (57-(location*7)));
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
   return extractLength(self);
}

-(NSUInteger)lengthOfBytesUsingEncoding:(NSStringEncoding)encoding {
    switch (encoding) {
        case NSUTF8StringEncoding:
        case NSASCIIStringEncoding:
            return extractLength(self);
        default:
            NSUnimplementedMethod();
            return 0;
    }
}

-(unichar)characterAtIndex:(NSUInteger)location {
   NSUInteger length=extractLength(self);

   if(location>=length)
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",location,length);

   return characterAt(self, location);
}

-(void)getCharacters:(unichar *)buffer {
   NSUInteger i,length=extractLength(self);
   for(i=0; i<length; i++)
    buffer[i] = characterAt(self, i);
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSUInteger length=extractLength(self);

   if(NSMaxRange(range)>length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",NSStringFromRange(range),length);
   }

   NSInteger i,loc=range.location,len=range.length;

   for(i=0;i<len;i++)
    buffer[i]=characterAt(self, loc+i);
}

-(NSUInteger)hash {
   NSUInteger length=extractLength(self);

   char buffer[9];
   for(int i=0; i<length; i++)
    buffer[i] = characterAt(self, i);
   return NSStringHashASCII(buffer,MIN(length,NSHashStringLength));
}

@end
