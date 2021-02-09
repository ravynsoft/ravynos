/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSStringUTF8.h>

/* Constant strings are stored as UTF8, but this is expensive for NSString's, so we optimize for ASCII only strings
   by marking the string as such.
 */
#define STORAGE_ASCII 0x40000000
#define STORAGE_UTF8  0x80000000
#define STORAGE_MASK  (~(STORAGE_ASCII|STORAGE_UTF8))


@implementation NSConstantString(Impl)

static inline void resolveStorageIfNeeded(NSConstantString *self){
// FIX: There might be some hazard here modifying self->_length with multiple threads
   uint32_t length=self->_length;

   if((length&(STORAGE_ASCII|STORAGE_UTF8))==0){
    NSInteger check=NSConvertUTF8toUTF16(self->_bytes,length,NULL);

    if(check==length)
     self->_length|=STORAGE_ASCII;
    else
     self->_length|=STORAGE_UTF8;
   }
}

static BOOL storageIsASCII(NSConstantString *self){
   return (self->_length&STORAGE_ASCII)?YES:NO;
}

static inline NSUInteger lengthOfBytes(NSConstantString *self){
   return self->_length&STORAGE_MASK;
}

static inline NSUInteger lengthOfASCII(NSConstantString *self){
   return self->_length&STORAGE_MASK;
}

static inline NSUInteger lengthOfUTF8(NSConstantString *self){
   return NSConvertUTF8toUTF16(self->_bytes,lengthOfBytes(self),NULL);
}

static inline NSUInteger lengthInUnicode(NSConstantString *self){
   resolveStorageIfNeeded(self);

   if(storageIsASCII(self))
    return lengthOfASCII(self);
   else
    return lengthOfUTF8(self);
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
    unichar   unicode[length];
    //NSInteger check=
    NSConvertUTF8toUTF16(self->_bytes,lengthOfBytes(self),unicode);

    return unicode[location];
   }
}

-(void)getCharacters:(unichar *)buffer {
   NSUInteger i,length=lengthInUnicode(self);

   if(storageIsASCII(self)){
    for(i=0;i<length;i++)
     buffer[i]=((uint8_t *)_bytes)[i];
   }
   else {
    unichar   unicode[length];
    //NSInteger check=
    NSConvertUTF8toUTF16(self->_bytes,lengthOfBytes(self),unicode);

    for(i=0;i<length;i++)
     buffer[i]=unicode[i];
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
    unichar   unicode[length];
    //NSInteger check=
    NSConvertUTF8toUTF16(self->_bytes,lengthOfBytes(self),unicode);

    for(i=0;i<len;i++)
     buffer[i]=unicode[loc+i];
   }
}

-(NSUInteger)hash {
   NSUInteger length=lengthInUnicode(self);

   if(storageIsASCII(self))
    return NSStringHashASCII(_bytes,MIN(length,NSHashStringLength));
   else {
    unichar   unicode[length];
    //NSInteger check=
    NSConvertUTF8toUTF16(self->_bytes,lengthOfBytes(self),unicode);

    return NSStringHashUnicode(unicode,MIN(length,NSHashStringLength));
   }
}

@end


