/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSMutableData_concrete.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSData_concrete.h>
#import <Foundation/NSRaiseException.h>

@implementation NSMutableData_concrete

-(const void *)bytes {
   return _bytes;
}

-(NSUInteger)length {
   return _length;
}

-(void *)mutableBytes {
   return _bytes;
}

static inline void setLength(NSMutableData_concrete *self,NSUInteger length){
   if(length>self->_length){
    if(length>self->_capacity){
     self->_capacity=length*2;
     self->_bytes=NSZoneRealloc(NSZoneFromPointer(self->_bytes),self->_bytes,self->_capacity);
    }

    NSByteZeroRange(self->_bytes,NSMakeRange(self->_length,length-self->_length));
   }

   self->_length=length;
}

static inline void replaceBytesInRange(NSMutableData_concrete *self,NSRange range,const void *bytes){
   NSInteger   i,loc=range.location,len=range.length;

   if(loc>self->_length)
    NSRaiseException(NSRangeException,self,@selector(replaceBytesInRange:withBytes:),@"location %d beyond length %d",loc,self->_length);

   if(loc+len>self->_length)
    setLength(self,loc+len);

   for(i=0;i<len;i++)
    ((char *)self->_bytes)[i+loc]=((char *)bytes)[i];
}

-(void)setLength:(NSUInteger)length {
   setLength(self,length);
}

-init {
   return [self initWithCapacity:0];
}

-initWithCapacity:(NSUInteger)capacity {
   _length=0;
   _capacity=(capacity<4)?4:capacity;
   _bytes=NSZoneMalloc(NSZoneFromPointer(self),_capacity);
   return self;
}

-(void)dealloc {
   NSZoneFree(NSZoneFromPointer(_bytes),_bytes);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(void)appendBytes:(const void *)bytes length:(NSUInteger)length {
   NSRange  range=NSMakeRange(_length,length);

   setLength(self,_length+length);

   replaceBytesInRange(self,range,bytes);
}

-(void)replaceBytesInRange:(NSRange)range withBytes:(const void *)bytes {
   replaceBytesInRange(self,range,bytes);
}

@end
