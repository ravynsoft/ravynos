/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSMutableData_concrete.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSData_concrete.h>
#import <Foundation/NSRaiseException.h>

@implementation NSMutableData

+allocWithZone:(NSZone *)zone {
   if(self==objc_lookUpClass("NSMutableData"))
    return NSAllocateObject(objc_lookUpClass("NSMutableData_concrete"),0,zone);

   return NSAllocateObject(self,0,zone);
}

-initWithCapacity:(NSUInteger)capacity {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithLength:(NSUInteger)length {
   self=[self initWithCapacity:length];
   [self setLength:length];
   return self;
}

-initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone {
   self=[self initWithCapacity:length];

   [self appendBytes:bytes length:length];

   if(freeWhenDone)
   NSZoneFree(NSZoneFromPointer(bytes),bytes);

   return self;
}

-initWithContentsOfMappedFile:(NSString *)path {
    NSUnimplementedMethod();
    return nil;
}

+dataWithCapacity:(NSUInteger)capacity {
   return [[[self allocWithZone:NULL] initWithCapacity:capacity] autorelease];
}

+dataWithLength:(NSUInteger)length {
   return [[[self allocWithZone:NULL] initWithLength:length] autorelease];
}

-copyWithZone:(NSZone *)zone {
   return [[NSData allocWithZone:zone] initWithBytes:[self bytes] length:[self length]];
}

- (Class)classForKeyedArchiver
{
	// Keyed archiver doesn't like mutable data
	return [NSData class];
}

-(Class)classForCoder {
   return [NSMutableData class];
}

-(void *)mutableBytes {
   NSInvalidAbstractInvocation();
   return NULL;
}

-(void)setLength:(NSUInteger)length {
   NSInvalidAbstractInvocation();
}

-(void)increaseLengthBy:(NSUInteger)delta {
   [self setLength:[self length]+delta];
}

-(void)appendBytes:(const void *)bytes length:(NSUInteger)length {
   NSUInteger selfLength=[self length];
   NSRange  range=NSMakeRange(selfLength,length);

   [self setLength:selfLength+length];
   [self replaceBytesInRange:range withBytes:bytes];
}

-(void)appendData:(NSData *)data {
   [self appendBytes:[data bytes] length:[data length]];
}

-(void)replaceBytesInRange:(NSRange)range withBytes:(const void *)bytes {
   NSInteger   i,length=[self length];
   void *mutableBytes;

   if(range.location>length)
    NSRaiseException(NSRangeException,self,_cmd,@"location %d beyond length %d",range.location,[self length]);

   if(range.location+range.length>length)
    [self setLength:range.location+range.length];
    
   mutableBytes=[self mutableBytes];

   for(i=0;i<range.length;i++)
    ((char *)mutableBytes)[range.location+i]=((char *)bytes)[i];
}

-(void)replaceBytesInRange:(NSRange)range withBytes:(const void *)bytes length:(NSUInteger)bytesLength {
   NSUInteger i,delta,length=[self length];
   char      *mutableBytes;

   if(range.location>length)
    NSRaiseException(NSRangeException,self,_cmd,@"location %d beyond length %d",range.location,length);

   if(bytesLength>range.length){
    delta=bytesLength-range.length;
    
    length+=delta;
    [self setLength:length];
    
    mutableBytes=[self mutableBytes];

    for(i=0;i<length-(NSMaxRange(range)+delta);i++)
     mutableBytes[length-1-i]=mutableBytes[length-1-i-delta];
   }
   else if(bytesLength<range.length){
    delta=range.length-bytesLength;
    
    mutableBytes=[self mutableBytes];

    for(i=range.location+bytesLength;i<length-delta;i++)
     mutableBytes[i]=mutableBytes[i+delta];
    
    length-=delta;
    [self setLength:length];

    mutableBytes=[self mutableBytes];
   }
   else {
    mutableBytes=[self mutableBytes];
   }
   
   for(i=0;i<bytesLength;i++)
    mutableBytes[range.location+i]=((char *)bytes)[i];
}

-(void)setData:(NSData *)data {
   [self setLength:[data length]];
   [self replaceBytesInRange:NSMakeRange(0,[data length]) withBytes:[data bytes]];
}

-(void)resetBytesInRange:(NSRange)range {
   if(NSMaxRange(range)>[self length])
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",NSStringFromRange(range),[self length]);

   NSByteZeroRange([self mutableBytes],range);
}

@end
